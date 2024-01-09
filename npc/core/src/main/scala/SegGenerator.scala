package npc.seg

import chisel3._
import chisel3.util.{Decoupled}
import chisel3.util.log2Ceil

class SegInput(width: Int) extends Bundle {
  require(width > 0)
  val addr = UInt(width.W)
  val value = UInt(log2Ceil(width).W)
}

object SegInput {
  def apply(width: Int): SegInput = {
    return new SegInput(width)
  }
}

class SegGenerator(width: Int) extends {
  val io = IO(new Bundle {
    val write = Flipped(Decoupled(SegInput(8)))
    val segs = Output(Vec(width, UInt(8.W)))
  })

  val seg_regs = RegInit(VecInit(Seq.fill(width)(0.U(8.W))))
  io.segs := seg_regs

  when(io.write.valid) {
    val data = io.write.bits
    seg_regs(data.addr) := data.value
    io.write.ready := true.B
  }.otherwise {
    io.write.ready := false.B
  }
}
