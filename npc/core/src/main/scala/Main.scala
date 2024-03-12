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

  srcList.zip(dstList).foreach({
    case (src, dst) => dst := src.asTypeOf(dst)
  })
}

import flow.components.{ RegisterFile, RegFileInterface, ProgramCounter, ALU }
class Flow extends Module {
  val dataType = UInt(32.W)

  val ram = SRAM(size=128*1024*1024, tpe=dataType, numReadPorts=2, numWritePorts=1,numReadwritePorts=0)
  val control = Module(new Control(32))
  val reg = RegisterFile(32, dataType, 2, 2)
  val pc = Module(new ProgramCounter(dataType))
  val alu = Module(new ALU(dataType))

  ram.readPorts(0).enable := true.B
  ram.readPorts(0).address := pc.out
  val inst = ram.readPorts(0).data

  import control.pc.SrcSelect._

  pc.in.pcSrcs(pStaticNpc.litValue.toInt) := pc.out + 4.U
  pc.in.pcSrcs(pBranchResult.litValue.toInt) := alu.out.result

  control.inst := inst
  reg.control <> control.reg
  pc.control <> control.pc
  alu.control <> control.alu

  import control.reg.WriteSelect._
  reg.in.writeData(rAluOut.litValue.toInt) := alu.out.result
  // TODO: Read address in load command goes here
  ram.readPorts(1).enable := false.B
  ram.readPorts(1).address := 0.U
  reg.in.writeData(rMemOut.litValue.toInt) := ram.readPorts(1).data
  reg.in.writeAddr := inst(11, 7)
  reg.in.rs(0) := inst(19, 15)
  reg.in.rs(1) := inst(24, 20)

  // TODO: Memory write goes here
  ram.writePorts(0).address := 1.U
  ram.writePorts(0).data := 1.U
  ram.writePorts(0).enable := false.B

  alu.in.a := reg.out.src(0)
  alu.in.b := reg.out.src(1)
}
