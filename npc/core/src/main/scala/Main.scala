package npc

import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
import chisel3.stage.ChiselOption

class Switch extends Module {
  val io = IO(new Bundle {
    val sw = Input(Vec(2, Bool()))
    val out = Output(Bool())
  })

  io.out := io.sw(0) ^ io.sw(1)
}

import npc.util.{PS2Port, KeyboardController, SegGenerator}

class Keyboard extends Module {
  val io = IO(new Bundle {
    val ps2 = PS2Port()
    val segs = Output(Vec(8, UInt(8.W)))
  })

  val seg_handler = Module(new SegGenerator(seg_count = 8))
  val keyboard_controller = Module(new KeyboardController)

  seg_handler.io.keycode <> keyboard_controller.io.out

  keyboard_controller.io.ps2 := io.ps2
  io.segs := seg_handler.io.segs
}

class SegHandler extends SegGenerator(8) { }
