package flow.components

import chisel3._
import chisel3.util._
import shapeless.{HNil, ::}

class ALUControlInterface extends Bundle {
  object OpSelect extends ChiselEnum {
    val aOpAdd, aOpSub, aOpNot, aOpAnd, aOpOr, aOpXor, aOpSlt, aOpEq, aOpNop = Value
  }
  object SrcSelect extends ChiselEnum {
    val aSrcRs2, aSrcImm = Value
  }
  val op = Input(OpSelect())
  val src = Input(SrcSelect())

  type CtrlTypes = OpSelect.Type :: SrcSelect.Type :: HNil
  def ctrlBindPorts: CtrlTypes = {
    op :: src :: HNil
  }
}

class ALU[T <: UInt](tpe: T) extends Module {
  val control = IO(new ALUControlInterface)
  val in = IO(new Bundle {
    val a = Input(Vec(control.SrcSelect.getWidth, tpe))
    val b = Input(tpe)
  })
  val out = IO(new Bundle {
    val result = Output(tpe)
  })

  val a = in.a(control.src.asUInt)

  // val adder_b = (Fill(tpe.getWidth, io.op(0)) ^ io.b) + io.op(0)  // take (-b) if sub
  val add = a + in.b 
  val sub = a - in.b
  val and = a & in.b
  val not = ~a
  val or = a | in.b
  val xor = a ^ in.b
  val slt = a < in.b
  val eq = a === in.b

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
