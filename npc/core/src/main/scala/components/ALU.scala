package flow.components

import chisel3._
import chisel3.util._
import shapeless.{HNil, ::}

class ALUControlInterface extends Bundle {
  object OpSelect extends ChiselEnum {
    val aOpAdd, aOpSub, aOpNot, aOpAnd, aOpOr, aOpXor, aOpSlt, aOpSll, aOpSrl, aOpSra = Value
  }
  object SrcASelect extends ChiselEnum {
    val aSrcARs1, aSrcAPc, aSrcAZero = Value
  }
  object SrcBSelect extends ChiselEnum {
    val aSrcBRs2, aSrcBImmI, aSrcBImmJ, aSrcBImmB, aSrcBImmS = Value
  }
  val op = Input(OpSelect())
  val srcASelect = Input(SrcASelect())
  val srcBSelect = Input(SrcBSelect())
  val signExt = Input(Bool())

  def ctrlBindPorts = {
    op :: srcASelect :: srcBSelect :: signExt :: HNil
  }
}

class ALU[T <: UInt](tpe: T) extends Module {
  val control = IO(new ALUControlInterface)
  val in = IO(new Bundle {
    val a = Input(Vec(control.SrcASelect.all.length, tpe))
    val b = Input(Vec(control.SrcBSelect.all.length, tpe))
  })
  val out = IO(new Bundle {
    val eq = Output(Bool())
    val result = Output(tpe)
  })

  val a = in.a(control.srcASelect.asUInt)
  val b = in.b(control.srcBSelect.asUInt)

  // val adder_b = (Fill(tpe.getWidth, io.op(0)) ^ io.b) + io.op(0)  // take (-b) if sub
  val add = a + b
  val sub = a - b
  val and = a & b
  val not = ~a
  val or = a | b
  val xor = a ^ b
  val slt = a < b
  val sll = a << b(log2Ceil(tpe.getWidth), 0)
  val srl = a >> b(log2Ceil(tpe.getWidth), 0)
  val sra = a.asSInt >> b(log2Ceil(tpe.getWidth), 0)
  out.eq := a === b

  import control.OpSelect._

  out.result := MuxLookup(control.op, 0.U)(Seq(
    aOpAdd -> add,
    aOpSub -> sub,
    aOpNot -> not,
    aOpAnd -> and,
    aOpOr  -> or,
    aOpXor -> xor,
    aOpSlt -> slt,
    aOpSll -> sll,
    aOpSrl -> srl,
    aOpSra -> sra.asUInt
  ))
}

object ALU {
  def apply[T <: UInt](tpe: T): ALU[T] = {
    Module(new ALU(tpe))
  }
}
