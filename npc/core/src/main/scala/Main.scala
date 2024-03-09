package npc

import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
import chisel3.util.{SRAM}
import chisel3.util.experimental.decode.{decoder, TruthTable, QMCMinimizer}
import chisel3.stage.ChiselOption
import npc.util.{ KeyboardSegController }
import flowpc.components.RegisterFile
import chisel3.util.log2Ceil
import chisel3.util.BitPat

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

object RV32Inst extends ChiselEnum {
  val addi = Value("b0010011".U)
  val inv = Value("b0000000".U)
}

object InstType extends ChiselEnum {
  val R, I, S, B, U, J = Value
}

class RegControl(width: Int) extends Bundle {
  object RegWriteDataSel extends ChiselEnum {
    val ALUOut = Value
  }
  val T = RegWriteDataSel
  val writeEnable = Output(Bool()) 
  val writeSelect = Output(this.T())
}

class Control(width: Int) extends Bundle {
  val inst = IO(Input(UInt(width.W)))
  val out = decoder(QMCMinimizer, inst, TruthTable(
    Map(
      BitPat(Opcode.addi.asUInt) -> BitPat("b00001")
    ), BitPat("b?????")))
  val regControl = new RegControl(width)

}

class Flowpc extends Module {
  val io = IO(new Bundle { })
  val register_file = new RegisterFile(readPorts = 2)
  val pc = new ProgramCounter(32)
  val ram = SRAM(size=128*1024*1024, tpe=UInt(32.W), numReadPorts=2, numWritePorts=1,numReadwritePorts=0)

  // Instruction Fetch
  ram.readPorts(0).address := pc.io.pc
  ram.readPorts(0).enable := true.B
  val instruction = ram.readPorts(0).address

  // Instruction Decode
  val opcode = Opcode(instruction(7,0))

  // Execution

  // Next PC
  pc.io.pc_srcs(ProgramCounterSel.selectPC.asUInt) := pc.io.pc + 4.U
  // pc.io.pc_srcs(ProgramCounterSel.selectResult.asUInt) := 
}
