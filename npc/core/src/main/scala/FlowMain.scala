package flow

import scala.reflect.runtime.universe._
import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
import chisel3.util.{SRAM}
import chisel3.util.experimental.decode.{decoder, TruthTable}
import chisel3.util.log2Ceil
import chisel3.util.BitPat
import chisel3.util.Enum
import chisel3.experimental.Trace._
import shapeless.{HNil, ::}
import shapeless.HList
import shapeless.ops.coproduct.Prepend
import chisel3.util.{ BinaryMemoryFile, HexMemoryFile }

import chisel3.experimental.Trace

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

import flow.components.{RegControl, PcControlInterface, ALUControlInterface}
class Control(width: Int) extends Module {
  val inst = IO(Input(UInt(width.W)))

  val reg = IO(Flipped(new RegControl))
  val pc = IO(Flipped(new PcControlInterface))
  val alu = IO(Flipped(new ALUControlInterface))

  // TODO: Add .ctrlTypes together instead of writing them by hand.
  type T =
    Bool :: reg.WriteSelect.Type :: pc.SrcSelect.Type :: alu.OpSelect.Type :: alu.SrcSelect.Type :: HNil
  val dst: T = reg.ctrlBindPorts ++ pc.ctrlBindPorts ++ alu.ctrlBindPorts

  val dstList = dst.toList
  val reversePrefixSum = dstList.scanLeft(0)(_ + _.getWidth).reverse
  val slices = reversePrefixSum.zip(reversePrefixSum.tail)

  import reg.WriteSelect._
  import pc.SrcSelect._
  import alu.OpSelect._
  import alu.SrcSelect._
  import RV32Inst._
  val ControlMapping: Array[(BitPat, T)] = Array(
    //     Regs                       :: PC         :: Exe
    //     writeEnable :: writeSelect :: srcSelect  ::
    (addi, true.B      :: rAluOut     :: pStaticNpc :: aOpAdd :: aSrcImm :: HNil),
  )
  val default = BitPat.dontCare(dstList.map(_.getWidth).reduce(_ + _))

  def toBits(t: T): BitPat = {
    val list: List[Data] = t.toList
    list.map(x => BitPat(x.litValue.toInt.U(x.getWidth.W))).reduceLeft(_ ## _)
  }
  val out = decoder(
    inst,
    TruthTable(ControlMapping.map(it => (it._1 -> toBits(it._2))), default))
  val srcList = slices.map(s => out(s._1 - 1, s._2))

  srcList
    .zip(dstList.reverse)
    .foreach({ case (src, dst) =>
      dst := src.asTypeOf(dst)
    })
}

import flow.components.{RegisterFile, ProgramCounter, ALU, RamDpi}
import chisel3.util.experimental.loadMemoryFromFileInline
class Flow extends Module {
  val dataType = UInt(32.W)
  val ram = Module(new RamDpi)
  val control = Module(new Control(32))
  val reg = Module(new RegisterFile(dataType, 32, 2))
  val pc = Module(new ProgramCounter(dataType))
  val alu = Module(new ALU(dataType))

  ram.io.readAddr := pc.out
  val inst = ram.io.readData

  Trace.traceName(reg.control.writeEnable)
  dontTouch(reg.control.writeEnable)

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
  reg.in.writeData(rMemOut.litValue.toInt) := DontCare

  reg.in.writeAddr := inst(11, 7)
  reg.in.rs(0) := inst(19, 15)
  reg.in.rs(1) := inst(24, 20)

  // TODO: Memory write goes here
  ram.io.writeAddr := DontCare
  ram.io.writeData := DontCare
  ram.io.writeMask := DontCare
  ram.io.writeEnable := false.B
  ram.io.valid := true.B

  import control.alu.SrcSelect._
  alu.in.a(aSrcRs1.litValue.toInt) := reg.out.src(0)
  alu.in.a(aSrcImm.litValue.toInt) := inst(31, 20)
  alu.in.b := reg.out.src(1)


  dontTouch(control.out)
}
