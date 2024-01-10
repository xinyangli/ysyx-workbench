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
    "b11111101".U, // 0
    "b01100000".U, // 1
    "b11011010".U, // 2
    "b11110010".U, // 3
    "b01100110".U, // 4
    "b10110110".U, // 5
    "b10111110".U, // 6
    "b11100000".U, // 7
    "b11111110".U, // 8
    "b11110110".U, // 9
    "b11101110".U, // A
    "b00111110".U, // B
    "b10011100".U, // C
    "b01111010".U, // D
    "b10011110".U, // E
    "b10001110".U  // F
  ))

  val keycode_digits = VecInit(io.keycode.bits(3,0)) ++ VecInit(io.keycode.bits(7,4))
  val keycode_seg = keycode_digits.map(MuxLookup(_, 0.U)(digit_to_seg))

  seg_regs := keycode_seg ++ keycode_seg ++ keycode_seg ++ keycode_seg

  io.segs := seg_regs
}


