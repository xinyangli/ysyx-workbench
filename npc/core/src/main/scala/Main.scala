package flow

import scala.reflect.runtime.universe._
import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
import chisel3.util.{SRAM}
import chisel3.util.experimental.decode.{decoder, TruthTable, QMCMinimizer}
import chisel3.stage.ChiselOption
import chisel3.util.log2Ceil
import chisel3.util.BitPat
import chisel3.util.Enum
import chisel3.experimental.prefix
import shapeless.{ HNil, :: }
import shapeless.HList
import shapeless.ops.coproduct.Prepend

object RV32Inst {
  private val bp = BitPat
  val addi = this.bp("b???????_?????_?????_000_?????_00100_11")
  val inv = this.bp("b???????_?????_?????_???_?????_?????_??")
}

class PcControl(width: Int) extends Bundle {
  object SrcSelect extends ChiselEnum {
      val pPC, pExeResult = Value
  }
  val srcSelect = Output(SrcSelect())
}


import flow.components.{ RegisterFile, RegFileInterface, ProgramCounter, ALU }
import flow.components.{ RegControl, PcControlInterface, ALUControlInterface }
class Control(width: Int) extends Module {
  val inst = IO(Input(UInt(width.W)))

  val reg = IO(Flipped(new RegControl))
  val pc = IO(Flipped(new PcControlInterface))
  val alu = IO(Flipped(new ALUControlInterface))


  // TODO: Add .ctrlTypes together instead of writing them by hand.
  type T = Bool :: reg.WriteSelect.Type :: pc.SrcSelect.Type :: alu.OpSelect.Type :: HNil
  val dst: T = reg.ctrlBindPorts ++ pc.ctrlBindPorts ++ alu.ctrlBindPorts
  import reg.WriteSelect._
  import pc.SrcSelect._
  import alu.OpSelect._
  import RV32Inst._
  val ControlMapping: Array[(BitPat, T)] = Array(
    //     Regs                       :: PC         :: Exe
    //     writeEnable :: writeSelect :: srcSelect  :: 
    (addi, false.B     :: rAluOut     :: pStaticNpc :: aOpAdd :: HNil)
  )
  def toBits(t: T): BitPat = {
    val list: List[Data] = t.toList
    list.map(x => BitPat(x.litValue.toInt.U(x.getWidth.W))).reduce(_ ## _)
  }

  val default = toBits(false.B     :: rAluOut     :: pStaticNpc :: aOpSlt:: HNil)

  val out = decoder(QMCMinimizer, inst, TruthTable(
    ControlMapping.map(it => (it._1, toBits(it._2))), default))

  val dstList = dst.toList
  val reversePrefixSum = dstList.scanLeft(0)(_ + _.getWidth).reverse
  val slices = reversePrefixSum.zip(reversePrefixSum.tail)
  val srcList = slices.map(s => out(s._1 - 1, s._2))

  // def m[T <: Data](src: UInt, dst: T) = dst match {
  //   case dst: EnumType  => dst := src.asTypeOf(chiselTypeOf(dst)) 
  //   case dst: Data      => dst := src
  // }

  srcList.zip(dstList).foreach({
    case (src, dst) => dst := src.asTypeOf(dst)
  })
}

class Flow extends Module {
  val io = IO(new Bundle { })
  val ram = SRAM(size=128*1024*1024, tpe=UInt(32.W), numReadPorts=2, numWritePorts=1,numReadwritePorts=0)
  val control = Module(new Control(32))

  // Instruction Fetch
  ram.readPorts(0).enable := true.B
  val instruction = ram.readPorts(0).address

}
