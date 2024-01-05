package npc

import chisel3._
import chisel3.stage.ChiselOption

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

class MuxGenerator(width: Int, nInput: Int) extends Module {
  require(width >= 0)
  require(nInput >= 1)
  require(nInput.toBinaryString.map(_ - '0').sum == 1)
  
  val io = IO(new Bundle {
    val in = Input(Vec(nInput, UInt(width.W)))
    val sel = Input(UInt(nInput.toBinaryString.reverse.indexOf('1').W))
    val out = Output(UInt(width.W))
  })

  io.out := io.in(io.sel)
}

class Test extends Module {
  val io = IO(new Bundle {
    val in = Input(UInt(32.W))
    val out = Output(UInt(32.W))
  })

  val regFile = Module(new RegisterFile(2))
  regFile.io.writeEnable := true.B
  regFile.io.writeAddr := 1.U
  regFile.io.writeData := io.in
  regFile.io.readAddr(0) := 0.U
  regFile.io.readAddr(1) := 1.U
  io.out := regFile.io.readData(1)
}

class Switch extends Module {
  val io = IO(new Bundle {
    val sw = Input(Vec(2, Bool()))
    val out = Output(Bool())
  })

  io.out := io.sw(0) ^ io.sw(1)
}
