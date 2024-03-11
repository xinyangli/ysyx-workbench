package flowpc.components
import chisel3._
import chisel3.util.{Valid, log2Ceil}
import chisel3.util.MuxLookup

class ProgramCounter[T <: Data](tpe: T, numPcSrc: Int)  extends Module {
  val io = new Bundle {
    val pc_srcs = Input(Vec(numPcSrc, tpe))
    val select = Input(UInt(log2Ceil(numPcSrc).W))
    val pc = Output(tpe)
  }
    io.pc := io.pc_srcs(io.select)
}
