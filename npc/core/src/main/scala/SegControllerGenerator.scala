package npc.util

import chisel3._
import chisel3.util._
import chisel3.util.log2Ceil

class SegControllerGenerator[T <: Data](seg_count: Int, t: T) extends Module {
  val io = IO(new Bundle {
    val in_segs = Input(Vec(seg_count / ((t.getWidth + 3) / 4), t))
    val segs = Output(Vec(seg_count, UInt(8.W)))
  })
  val digit_to_seg = ((0 until 16).map(_.U)).zip(Seq(
    "b00000011".U, "b10011111".U, "b00100101".U, "b00001101".U,
    "b10011001".U, "b01001001".U, "b01000001".U, "b00011111".U,
    "b00000001".U, "b00001001".U, "b00010001".U, "b11000001".U,
    "b01100011".U, "b10000101".U, "b01100001".U, "b01110001".U,
  ))
  val vec = io.in_segs.asTypeOf(Vec(seg_count, UInt(4.W)))

  val segs = VecInit(Seq.fill(seg_count)(0.U(8.W)))
  segs := vec.map(MuxLookup(_, 0xFF.U)(digit_to_seg))

  io.segs := segs
}
