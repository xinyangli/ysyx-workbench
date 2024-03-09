package flowpc.components

import chisel3._
import chisel3.util.log2Ceil
import chisel3.util.UIntToOH

class RegisterFile(width: Int, numRegisters: Int, numReadPorts: Int) extends Module {
  require(numReadPorts >= 0)
  val writePort = IO(new Bundle {
    val enable = Input(Bool())
    val addr = Input(UInt(log2Ceil(width).W))
    val data = Input(UInt(width.W))
  })
  val readPorts = IO(Vec(numReadPorts, new Bundle {
    val addr = Input(UInt(log2Ceil(width).W))
    val data = Output(UInt(width.W))
  }))

  val regFile = RegInit(VecInit(Seq.fill(numRegisters)(0.U(32.W))))
  val writeAddrOH = UIntToOH(writePort.addr)
  for ((reg, i) <- regFile.zipWithIndex) {
    reg := Mux(writeAddrOH(i) && writePort.enable, writePort.data, reg)
  }

  for (readPort <- readPorts) {
    readPort.data := regFile(readPort.addr)
  }
}
