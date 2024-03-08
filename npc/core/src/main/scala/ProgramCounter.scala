package flowpc.components
import chisel3._
import chisel3.util.{Valid, log2Ceil}
import chisel3.util.MuxLookup

object ProgramCounterSel extends ChiselEnum {
    val selectPC, selectResult = Value
}

class ProgramCounter (width: Int) extends Module {
  val io = new Bundle {
    val pc_srcs = Input(Vec(1 << (ProgramCounterSel.getWidth - 1), UInt(width.W)))
    val select = Input(UInt(ProgramCounterSel.getWidth.W))
    val pc = Output(UInt(width.W))
  }
    io.pc := io.pc_srcs(io.select)
}
