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
    "b00000010".U, "b10011110".U, "b00100100".U, "b00001100".U,
    "b10011000".U, "b01001000".U, "b01000000".U, "b00011110".U,
    "b00000000".U, "b00001000".U, "b00010000".U, "b11000000".U,
    "b01100010".U, "b10000100".U, "b01100000".U, "b01110000".U
  ))

  val keycode_to_ascii = (((0x41 to 0x5A) ++ (0x30 to 0x39)).map(_.U)).zip(Seq(
    "0x1C".U, "0x32".U, "0x21".U, "0x23".U, "0x24".U, "0x2B".U,
    "0x34".U, "0x33".U, "0x43".U, "0x3B".U, "0x42".U, "0x4B".U,
    "0x3A".U, "0x31".U, "0x44".U, "0x4D".U, "0x15".U, "0x2D".U,
    "0x1B".U, "0x2C".U, "0x3C".U, "0x2A".U, "0x1D".U, "0x22".U,
    "0x35".U, "0x1A".U, "0x45".U, "0x16".U, "0x1E".U, "0x26".U,
    "0x25".U, "0x2E".U, "0x36".U, "0x3D".U, "0x3E".U, "0x46".U,
  ))

  println(keycode_to_ascii)

  val keycode_digits = VecInit(io.keycode.bits(3,0)) ++ VecInit(io.keycode.bits(7,4))
  val keycode_seg = keycode_digits.map(MuxLookup(_, 0.U)(digit_to_seg))

  seg_regs := keycode_seg ++ keycode_seg ++ keycode_seg ++ keycode_seg

  io.segs := seg_regs
}


