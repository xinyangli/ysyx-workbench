package npc.util

import chisel3._
import chisel3.util._
import chisel3.util.log2Ceil

class SegGenerator(seg_count: Int) extends Module {
  val io = IO(new Bundle {
    val keycode = Flipped(Decoupled(UInt(8.W)))
    val segs = Output(Vec(seg_count, UInt(8.W)))
  })
  io.keycode.ready := false.B
  when(io.keycode.valid) {
    io.keycode.ready := true.B
  }

  val seg_regs = RegInit(VecInit(Seq.fill(seg_count)(0.U(8.W))))
  val last_keycode = RegInit(0.U(8.W))
  val digit_to_seg = ((0 until 16).map(_.U)).zip(Seq(
    "b00000011".U, "b10011111".U, "b00100101".U, "b00001101".U,
    "b10011001".U, "b01001001".U, "b01000001".U, "b00011111".U,
    "b00000001".U, "b00001001".U, "b00010001".U, "b11000001".U,
    "b01100011".U, "b10000101".U, "b01100001".U, "b01110001".U,
  ))

  val keycode_to_ascii = Seq(
    0x1C.U, 0x32.U, 0x21.U, 0x23.U, 0x24.U, 0x2B.U,
    0x34.U, 0x33.U, 0x43.U, 0x3B.U, 0x42.U, 0x4B.U,
    0x3A.U, 0x31.U, 0x44.U, 0x4D.U, 0x15.U, 0x2D.U,
    0x1B.U, 0x2C.U, 0x3C.U, 0x2A.U, 0x1D.U, 0x22.U,
    0x35.U, 0x1A.U, 0x45.U, 0x16.U, 0x1E.U, 0x26.U,
    0x25.U, 0x2E.U, 0x36.U, 0x3D.U, 0x3E.U, 0x46.U,
  ).zip(((0x41 to 0x5A) ++ (0x30 to 0x39)).map(_.U))

  val keycode = RegInit(0.U(8.W))
  val counter = Counter(0xFF)
  val release_state = false.B
  when(io.keycode.ready && io.keycode.valid) {
    when(io.keycode.bits === 0xF0.U) {
      release_state := true.B
    }.elsewhen(!release_state) {
      keycode := io.keycode.bits
      counter.inc()
    }
  }

  val keycode_digits = VecInit(keycode(3,0)) ++ VecInit(keycode(7,4))
  val keycode_seg = keycode_digits.map(MuxLookup(_, 0xFF.U)(digit_to_seg))

  val ascii = MuxLookup(keycode, 0.U)(keycode_to_ascii)
  val ascii_digits = VecInit(ascii(3,0)) ++ VecInit(ascii(6,4))
  val ascii_seg = ascii_digits.map(MuxLookup(_, 0xFF.U)(digit_to_seg))

  val count_digits = VecInit(counter.value(3,0)) ++ VecInit(counter.value(7,4))
  val count_seg = count_digits.map(MuxLookup(_, 0xFF.U)(digit_to_seg))
  seg_regs := keycode_seg ++ ascii_seg ++ count_seg ++ Seq(0xFF.U, 0xFF.U)

  io.segs := seg_regs
}
