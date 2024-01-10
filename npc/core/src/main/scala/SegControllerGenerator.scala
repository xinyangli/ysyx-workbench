package npc.util

import chisel3._
import chisel3.util._
import chisel3.util.log2Ceil

class SegControllerGenerator(seg_count: Int) extends Module {
  val io = IO(new Bundle {
    val in_segs = Input(Vec(seg_count, UInt()))
    val segs = Output(Vec(seg_count, UInt(8.W)))
  })
  val digit_to_seg = ((0 until 16).map(_.U)).zip(Seq(
    "b00000011".U, "b10011111".U, "b00100101".U, "b00001101".U,
    "b10011001".U, "b01001001".U, "b01000001".U, "b00011111".U,
    "b00000001".U, "b00001001".U, "b00010001".U, "b11000001".U,
    "b01100011".U, "b10000101".U, "b01100001".U, "b01110001".U,
  ))
  val vec_size = (io.in_segs.getWidth + 3) / 4
  val vec = io.in_segs.asTypeOf(Vec(vec_size, UInt(4.W)))

  val seg_regs = RegInit(VecInit(Seq.fill(seg_count)(0.U(8.W))))
  seg_regs := vec.map(MuxLookup(_, 0xFF.U)(digit_to_seg)) ++ Seq(0xFF.U, 0xFF.U)

  io.segs := seg_regs
}

object SegControllerGenerator {
  def apply(seg_count: Int): SegControllerGenerator = {
    new SegControllerGenerator(seg_count)
  }
}
