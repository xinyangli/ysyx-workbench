package flow.components

import chisel3._
import chisel3.util.log2Ceil
import chisel3.util.UIntToOH
import chisel3.util.MuxLookup
import chisel3.experimental.Trace._
import shapeless.{ HNil, :: }

class RegControl extends Bundle {
  object WriteSelect extends ChiselEnum {
    val rAluOut, rMemOut = Value
  }

  val writeEnable = Input(Bool())
  val writeSelect = Input(WriteSelect())

  type CtrlTypes = Bool :: WriteSelect.Type :: HNil
  def ctrlBindPorts: CtrlTypes = {
    writeEnable :: writeSelect :: HNil
  }
  traceName(writeEnable)
}

class RegisterFile[T <: Data](tpe: T, regCount: Int, numReadPorts: Int) extends Module {
  require(numReadPorts >= 0)
  val control = IO(new RegControl)
  val dataAddrWidth = log2Ceil(regCount).W
  val in = IO(new Bundle {
    val writeAddr = Input(UInt(dataAddrWidth))
    val writeData = Input(Vec(control.WriteSelect.all.length, tpe))
    val rs = Input(Vec(numReadPorts, UInt(dataAddrWidth)))
  })
  val out = IO(new Bundle {
    val src = Output(Vec(numReadPorts, tpe))
  })

  val regResetValue = 0.U(tpe.getWidth.W)
  val regFile = RegInit(VecInit(Seq.fill(regCount)(regResetValue)))
  val writeAddrOH = UIntToOH(in.writeAddr)

  for ((reg, i) <- regFile.zipWithIndex.tail) {
    reg := Mux(writeAddrOH(i.U(log2Ceil(regCount).W)) && control.writeEnable, in.writeData(control.writeSelect.asUInt), reg)
  }
  regFile(0) := 0.U

  for (port <- 0 until numReadPorts) {
    out.src(port) := regFile(in.rs(port).asUInt)
  }
  traceName(regFile)
  dontTouch(regFile)
}
