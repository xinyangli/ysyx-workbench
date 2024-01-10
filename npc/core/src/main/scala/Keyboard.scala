package npc.keyboard

import chisel3._
import chisel3.util.{Counter, Decoupled, Queue, Reverse, MuxLookup}

import npc.seg._
import upickle.implicits.key

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

class SegGenerator(seg_count: Int) extends Module {
  val io = IO(new Bundle {
    val keycode = Flipped(Decoupled(UInt(8.W)))
    val segs = Output(Vec(seg_count, UInt(8.W)))
  })
  io.keycode.ready := DontCare

  val seg_regs = RegInit(VecInit(Seq.fill(seg_count)(0.U(8.W))))
  val last_keycode = RegInit(0.U(8.W))
  val counter = Counter(0xFF)
  val digit_to_seg = ((0 until 16).map(_.U)).zip(Seq(
    "b00000010".U, "b10011111".U, "b00100101".U, "b00001101".U,
    "b10011001".U, "b01001001".U, "b01000001".U, "b00011111".U,
    "b00000001".U, "b00001001".U, "b00010001".U, "b11000001".U,
    "b01100011".U, "b10000101".U, "b01100001".U, "b01110001".U
  ))

  val keycode_digits = VecInit(io.keycode.bits(3,0)) ++ VecInit(io.keycode.bits(7,4))
  val keycode_seg = keycode_digits.map(MuxLookup(_, 0.U)(digit_to_seg))

  seg_regs := keycode_seg ++ keycode_seg ++ keycode_seg ++ keycode_seg

  io.segs := seg_regs
}


