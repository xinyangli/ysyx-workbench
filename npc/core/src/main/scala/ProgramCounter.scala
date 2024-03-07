package flowpc.components
import chisel3._
import chisel3.util.{Valid}

class ProgramCounter (width: Int) extends Module {
  val io = new Bundle {
    val next_pc = Input(Flipped(Valid(UInt(width.W))))
    val pc = Output(UInt(width.W))
  }
    io.pc := Mux(io.next_pc.valid, io.next_pc.bits, io.pc)
}
