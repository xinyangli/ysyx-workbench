package npc

import scala.reflect.runtime.universe._
import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
import chisel3.util.{SRAM}
import chisel3.util.experimental.decode.{decoder, TruthTable, QMCMinimizer}
import chisel3.stage.ChiselOption
import npc.util.{ KeyboardSegController }
import flowpc.components.RegisterFile
import chisel3.util.log2Ceil
import chisel3.util.BitPat
import chisel3.util.Enum
import chisel3.experimental.prefix
import shapeless.{ HNil, :: }

class Switch extends Module {
  val io = IO(new Bundle {
    val sw = Input(Vec(2, Bool()))
    val out = Output(Bool())
  })

  io.out := io.sw(0) ^ io.sw(1)
}

import npc.util.{PS2Port, KeyboardController, SegControllerGenerator}

class Keyboard extends Module {
  val io = IO(new Bundle {
    val ps2 = PS2Port()
    val segs = Output(Vec(8, UInt(8.W)))
  })

  val seg_handler = Module(new KeyboardSegController)
  val keyboard_controller = Module(new KeyboardController)

  seg_handler.io.keycode <> keyboard_controller.io.out

  keyboard_controller.io.ps2 := io.ps2
  io.segs := seg_handler.io.segs
}

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


import flowpc.components.{ RegisterFile }
class Control(width: Int) extends Module {
  val reg = Flipped(RegisterFile(32, UInt(32.W), 2, 2))
  val pc = new PcControl(width)
  val inst = IO(Input(UInt(width.W)))

  type T = Bool :: reg.control.WriteSelect.Type :: HNil
  val dst: T = reg.control.writeEnable :: reg.control.writeSelect :: HNil
  val dstList: List[Data] = dst.toList
  val reversePrefixSum = dstList.scanLeft(0)(_ + _.getWidth).reverse
  val slices = reversePrefixSum.zip(reversePrefixSum.tail)

  import reg.control.WriteSelect._
  import pc.SrcSelect._
  import RV32Inst._
  val ControlMapping: Array[(BitPat, T)] = Array(
    //     Regs                       :: PC          :: Exe
    //     writeEnable :: writeSelect :: srcSelect   :: 
    (addi, false.B     :: rAluOut     ::               HNil)
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

class Flowpc extends Module {
  val io = IO(new Bundle { })
  val ram = SRAM(size=128*1024*1024, tpe=UInt(32.W), numReadPorts=2, numWritePorts=1,numReadwritePorts=0)

  // Instruction Fetch
  ram.readPorts(0).enable := true.B
  val instruction = ram.readPorts(0).address

}
