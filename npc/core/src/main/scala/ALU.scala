package npc.util

import chisel3._
import chisel3.util._

object ALUSel extends ChiselEnum {
  val add, sub, not, and, or, xor, slt, eq, nop = Value
}

class ALUGenerator[T <: ChiselEnum](width: Int, tpe: T = ALUSel) extends Module {
  val io = IO(new Bundle {
    val a = Input(UInt(tpe.getWidth.W))
    val b = Input(UInt(tpe.getWidth.W))
    val op = Input(ALUSel())
    val out = Output(UInt(tpe.getWidth.W))
  })

  // val adder_b = (Fill(tpe.getWidth, io.op(0)) ^ io.b) + io.op(0)  // take (-b) if sub
  val add = io.a + io.b 
  val sub = io.a - io.b
  val and = io.a & io.b
  val not = ~io.a
  val or = io.a | io.b
  val xor = io.a ^ io.b
  val slt = io.a < io.b
  val eq = io.a === io.b

  io.out := MuxLookup(io.op, ALUSel.nop.asUInt)(Seq(
    ALUSel.add -> add,
    ALUSel.sub -> sub,
    ALUSel.not -> not,
    ALUSel.and -> and,
    ALUSel.or  -> or,
    ALUSel.xor -> xor,
    ALUSel.slt -> slt,
    ALUSel.eq -> eq
  ))
}
