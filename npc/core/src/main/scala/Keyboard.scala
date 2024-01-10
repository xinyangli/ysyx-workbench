package npc.util

import chisel3._
import chisel3.util.{Counter, Decoupled, Queue, Reverse, MuxLookup}

class PS2Port extends Bundle {
  val clk = Input(Bool())
  val data = Input(UInt(1.W))
}

object PS2Port {
  def apply(): PS2Port = {
    new PS2Port
  }
}

class KeyboardController extends Module {
  val io = IO(new Bundle {
    val ps2 = PS2Port()
    val out = Decoupled(UInt(8.W))
  })
  // valid only on the clock negedge of ps2_clk
  val ps2_clk_valid = RegNext(io.ps2.clk, false.B) & ~io.ps2.clk
  val cycle_counter = Counter(11)
  val concated_data = RegInit(0.U(8.W))

  val queue_in = Wire(Flipped(Decoupled(UInt(8.W))))
  val queue = Queue(queue_in, entries = 8)
  val received = RegInit(Bool(), false.B)
  val pushed = RegNext(queue_in.valid && queue_in.ready, false.B)
  queue_in.valid := false.B
  queue_in.bits := Reverse(concated_data)
  io.out <> queue

  when(cycle_counter.value === 0.U) {
    concated_data := 0.U
    received := false.B
  }

  when(ps2_clk_valid) {
    when(cycle_counter.value < 9.U && cycle_counter.value >= 1.U) {
      concated_data := (concated_data << 1) | io.ps2.data
    }.elsewhen(cycle_counter.value === 9.U) {
      received := true.B
    }
    cycle_counter.inc()
  }

  when(!pushed && received) {
    queue_in.valid := true.B
  }.elsewhen(pushed && received) {
    queue_in.valid := false.B
    received := false.B
  }
}
