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
  val addi = this.bp("b??b?????_?????_?????_000_?????_00100_11")
  val inv = this.bp("b???????_?????_?????_???_?????_?????_??")
}

class PcControl(width: Int) extends Bundle {
  object SrcSelect extends ChiselEnum {
      val pPC, pExeResult = Value
  }
  val srcSelect = Output(SrcSelect())
}


import flow.components.{ RegisterFile, ProgramCounter, ALU }
class Control(width: Int) extends Module {
  val inst = Input(UInt(width.W))
  val reg = Flipped(RegisterFile(32, UInt(width.W), 2, 2))
  val pc = ProgramCounter(UInt(width.W))
  val alu = ALU(UInt(width.W))

  // TODO: Add .ctrlTypes together instead of write them by hand.
  type T = Bool :: reg.control.WriteSelect.Type :: pc.SrcSelect.Type :: alu.control.OpSelect.Type :: HNil
  val dst: T = reg.ctrlBindPorts ++ pc.ctrlBindPorts ++ alu.ctrlBindPorts
  val dstList: List[Data] = dst.toList
  val reversePrefixSum = dstList.scanLeft(0)(_ + _.getWidth).reverse
  val slices = reversePrefixSum.zip(reversePrefixSum.tail)

  import reg.control.WriteSelect._
  import pc.SrcSelect._
  import alu.control.OpSelect._
  import RV32Inst._
  val ControlMapping: Array[(BitPat, T)] = Array(
    //     Regs                       :: PC          :: Exe
    //     writeEnable :: writeSelect :: srcSelect   :: 
    (addi, false.B     :: rAluOut     :: pStaticNpc  :: aOpAdd :: HNil)
  )

  def toBits(t: T): BitPat = {
    val list: List[Data] = t.toList
    list.map(x => x.asUInt).map(x => BitPat(x)).reduce(_ ## _)
  }

  val out = decoder(QMCMinimizer, inst, TruthTable(
    ControlMapping.map(it => (it._1, toBits(it._2))), inv))

  val srcList = slices.map(s => out(s._1 - 1, s._2))
  srcList.zip(dstList).foreach({ case (src, dst) => dst := src })
}

class Flow extends Module {
  val io = IO(new Bundle { })
  val ram = SRAM(size=128*1024*1024, tpe=UInt(32.W), numReadPorts=2, numWritePorts=1,numReadwritePorts=0)
  val control = new Control(32)

  // Instruction Fetch
  ram.readPorts(0).enable := true.B
  val instruction = ram.readPorts(0).address

}
