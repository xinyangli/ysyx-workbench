package flow

import scala.reflect.runtime.universe._
import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
import chisel3.util.{SRAM}
import chisel3.util.experimental.decode.{decoder, TruthTable}
import chisel3.stage.ChiselOption
import chisel3.util.log2Ceil
import chisel3.util.BitPat
import chisel3.util.Enum
import chisel3.experimental.prefix
import shapeless.{HNil, ::}
import shapeless.HList
import shapeless.ops.coproduct.Prepend
import chisel3.util.{ BinaryMemoryFile, HexMemoryFile }

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
    Bool :: reg.WriteSelect.Type :: pc.SrcSelect.Type :: alu.OpSelect.Type :: HNil
  val dst: T = reg.ctrlBindPorts ++ pc.ctrlBindPorts ++ alu.ctrlBindPorts
  import reg.WriteSelect._
  import pc.SrcSelect._
  import alu.OpSelect._
  import RV32Inst._
  val ControlMapping: Array[(BitPat, T)] = Array(
    //     Regs                       :: PC         :: Exe
    //     writeEnable :: writeSelect :: srcSelect  ::
    (addi, false.B :: rAluOut :: pStaticNpc :: aOpAdd :: HNil),
    // ("", false.B :: rAluOut :: pStaticNpc :: aOpNop :: HNil),
  )
  def toBits(t: T): BitPat = {
    val list: List[Data] = t.toList
    list.map(x => BitPat(x.litValue.toInt.U(x.getWidth.W))).reduce(_ ## _)
  }

  // val default = toBits(false.B :: rAluOut :: pStaticNpc :: aOpAdd :: HNil).getWidth
  val default = BitPat("b???????")

  reg.writeEnable := false.B
  reg.writeSelect := reg.WriteSelect(0.U)
  alu.op := alu.OpSelect(0.U)
  pc.srcSelect := pc.SrcSelect(0.U)

  // val out = decoder(
  //   inst,
  //   TruthTable(ControlMapping.map(it => (it._1 -> toBits(it._2))), default))
  val table = TruthTable(
    (0 until 16).map { i =>
      BitPat(i.U(4.W)) -> BitPat((1 << i).U(16.W))
    },
    BitPat.dontCare(16)
  )

  val out = decoder.qmc(inst(3,0), table)

  // val dstList = dst.toList
  // val reversePrefixSum = dstList.scanLeft(0)(_ + _.getWidth).reverse
  // val slices = reversePrefixSum.zip(reversePrefixSum.tail)
  // val srcList = slices.map(s => out(s._1 - 1, s._2))

  // srcList
  //   .zip(dstList)
  //   .foreach({ case (src, dst) =>
  //     dst := src.asTypeOf(dst)
  //   })
}

import flow.components.{RegisterFile, RegFileInterface, ProgramCounter, ALU}
import chisel3.util.experimental.loadMemoryFromFileInline
class Flow(memoryFile: String) extends Module {
  // val dataType = UInt(32.W)

  val ram = SRAM(
    size = 128 * 1024 * 1024,
    tpe = UInt(32.W),
    numReadPorts = 2,
    numWritePorts = 1,
    numReadwritePorts = 0,
    memoryFile = HexMemoryFile(memoryFile)
  )
  val control = Module(new Control(32))
  val reg = RegisterFile(32, UInt(32.W), 2, 2)
  val pc = Module(new ProgramCounter(UInt(32.W)))
  val alu = Module(new ALU(UInt(32.W)))

  ram.readPorts(0).enable := true.B
  ram.readPorts(0).address := pc.out - 0x80000000L.U
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
