package npc.util

import chisel3._
import chisel3.util._

class ALUGenerator(width: Int) extends Module {
  require(width >= 0)
  val io = IO(new Bundle {
    val a = Input(UInt(width.W))
    val b = Input(UInt(width.W))
    val op = Input(UInt(4.W))
    val out = Output(UInt(width.W))
  })

  val adder_b = (Fill(width, io.op(0)) ^ io.b) + io.op(0)  // take (-b) if sub
  val add = io.a + adder_b
  val and = io.a & io.b
  val not = ~io.a
  val or = io.a | io.b
  val xor = io.a ^ io.b
  val slt = io.a < io.b
  val eq = io.a === io.b

  io.out := MuxLookup(io.op, 0.U)(Seq(
    0.U -> add,
    1.U -> add, // add with b reversed
    2.U -> not,
    3.U -> and,
    4.U -> or,
    5.U -> xor,
    6.U -> slt,
    7.U -> eq,
  ))
}
