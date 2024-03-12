package flow.components

import chisel3._
import chisel3.util._
import shapeless.{HNil, ::}

class ALUControlInterface extends Bundle {
  object OpSelect extends ChiselEnum {
    val aOpAdd, aOpSub, aOpNot, aOpAnd, aOpOr, aOpXor, aOpSlt, aOpEq, aOpNop = Value
  }
  val op = Input(OpSelect())

  type ctrlTypes = OpSelect.Type :: HNil
  def ctrlBindPorts: ctrlTypes = {
    op :: HNil
  }
}

class ALU[T <: UInt](tpe: T) extends Module {
  val control = IO(new ALUControlInterface)
  val in = IO(new Bundle {
    val a = Input(tpe)
    val b = Input(tpe)
  })
  val out = IO(new Bundle {
    val result = Output(tpe)
  })

  // val adder_b = (Fill(tpe.getWidth, io.op(0)) ^ io.b) + io.op(0)  // take (-b) if sub
  val add = in.a + in.b 
  val sub = in.a - in.b
  val and = in.a & in.b
  val not = ~in.a
  val or = in.a | in.b
  val xor = in.a ^ in.b
  val slt = in.a < in.b
  val eq = in.a === in.b

  import control.OpSelect._

  out.result := MuxLookup(control.op, 0.U)(Seq(
    aOpAdd -> add,
    aOpSub -> sub,
    aOpNot -> not,
    aOpAnd -> and,
    aOpOr  -> or,
    aOpXor -> xor,
    aOpSlt -> slt,
    aOpEq -> eq
  ))
}

object ALU {
  def apply[T <: UInt](tpe: T): ALU[T] = {
    Module(new ALU(tpe))
  }
}
