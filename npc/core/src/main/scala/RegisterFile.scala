package npc.util

import chisel3._

class RegisterFile(readPorts: Int) extends Module {
  require(readPorts >= 0)
  val io = IO(new Bundle {
    val writeEnable = Input(Bool())
    val writeAddr = Input(UInt(5.W))
    val writeData = Input(UInt(32.W))
    val readAddr = Input(Vec(readPorts, UInt(5.W)))
    val readData = Output(Vec(readPorts, UInt(32.W)))
  })

  val regFile = RegInit(VecInit(Seq.fill(32)(0.U(32.W))))
  for (i <- 1 until 32) {
    regFile(i) := regFile(i)
  }
  regFile(io.writeAddr) := Mux(io.writeEnable, io.writeData, regFile(io.writeAddr))
  regFile(0) := 0.U

  for (i <- 0 until readPorts) {
    io.readData(i) := regFile(io.readAddr(i))
  }
}
