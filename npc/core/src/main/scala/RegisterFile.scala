package flow.components

import chisel3._
import chisel3.util.log2Ceil
import chisel3.util.UIntToOH
import chisel3.util.MuxLookup
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
}

class RegFileData[T <: Data](size:Int, tpe: T, numReadPorts: Int, numWritePorts: Int) extends Bundle {
  val write = new Bundle {
    val addr = Input(UInt(size.W))
    val data = Vec(numWritePorts, Input(tpe))
  }
  val read = Vec(numReadPorts, new Bundle {
    val rs = Input(UInt(size.W))
    val src = Output(tpe)
  })
}

class RegFileInterface[T <: Data](size: Int, tpe: T, numReadPorts: Int, numWritePorts: Int) extends Bundle {
  val control = new RegControl
  // val data = new RegFileData(size, tpe, numReadPorts, numWritePorts)
  val in = new Bundle {
    val writeAddr = Input(UInt(size.W))
    val writeData = Input(Vec(numWritePorts, tpe))
    val rs = Input(Vec(numReadPorts, UInt(size.W)))
  }
  val out = new Bundle {
    val src = Output(Vec(numReadPorts, tpe))
  }
}

class RegisterFileCore[T <: Data](size: Int, tpe: T, numReadPorts: Int) extends Module {
  require(numReadPorts >= 0)
  val writePort = IO(new Bundle {
    val enable = Input(Bool())
    val addr = Input(UInt(log2Ceil(size).W))
    val data = Input(tpe)
  })
  val readPorts = IO(Vec(numReadPorts, new Bundle {
    val addr = Input(UInt(log2Ceil(size).W))
    val data = Output(tpe)
  }))

  val regFile = RegInit(VecInit(Seq.fill(size)(0.U(tpe.getWidth.W))))
  val writeAddrOH = UIntToOH(writePort.addr)
  for ((reg, i) <- regFile.zipWithIndex.tail) {
    reg := Mux(writeAddrOH(i) && writePort.enable, writePort.data, reg)
  }
  regFile(0) := 0.U

  for (readPort <- readPorts) {
    readPort.data := regFile(readPort.addr)
  }
  dontTouch(regFile)
}

object RegisterFile {
  def apply[T <: Data](size: Int, tpe: T, numReadPorts: Int, numWritePorts: Int): RegFileInterface[T] = {
    val core = Module(new RegisterFileCore(size, tpe, numReadPorts))
    val _out = Wire(new RegFileInterface(size, tpe, numReadPorts, numWritePorts))
    val clock = core.clock
    for (i <- 0 until numReadPorts) {
      core.readPorts(i).addr := _out.in.rs(i)
      _out.out.src(i) := core.readPorts(i).data
    }
    core.writePort.addr := _out.in.writeAddr
    core.writePort.data := _out.in.writeData(_out.control.writeSelect.asUInt)
    core.writePort.enable := _out.control.writeEnable
    _out
  }
}
