package flowpc.components

import chisel3._
import chisel3.util.log2Ceil
import chisel3.util.UIntToOH
import chisel3.util.MuxLookup

class RegControl extends Bundle {
  val writeEnable = Input(Bool()) 

  object WriteSelect extends ChiselEnum {
    val rAluOut, rMemOut = Value
  }
  val writeSelect = Input(WriteSelect())
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
  val data = new RegFileData(size, tpe, numReadPorts, numWritePorts)
}

class RegisterFileCore[T <: Data](size: Int, tpe: T, numReadPorts: Int) extends Module {
  printf("$numReadPorts\n")
  require(numReadPorts >= 0)
  val writePort = IO(new Bundle {
    val enable = Input(Bool())
    val addr = Input(UInt(size.W))
    val data = Input(tpe)
  })
  val readPorts = IO(Vec(numReadPorts, new Bundle {
    val addr = Input(UInt(size.W))
    val data = Output(tpe)
  }))

  // val regFile = RegInit(VecInit(Seq.fill(size)(0.U)))
  // val writeAddrOH = UIntToOH(writePort.addr)
  // for ((reg, i) <- regFile.zipWithIndex) {
  //   reg := Mux(writeAddrOH(i) && writePort.enable, writePort.data, reg)
  // }

  // for (readPort <- readPorts) {
  //   readPort.data := regFile(readPort.addr)
  // }
}

object RegisterFile {
  def apply[T <: Data](size: Int, tpe: T, numReadPorts: Int, numWritePorts: Int): RegFileInterface[T] = {
    val core = new RegisterFileCore(size, tpe, numReadPorts)
    val _out = Wire(new RegFileInterface(size, tpe, numReadPorts, numWritePorts))
    val clock = core.clock
    for (i <- 0 to numReadPorts) {
      core.readPorts(i).addr := _out.data.read(i).rs
      core.readPorts(i).data := _out.data.read(i).src
    }
    core.writePort.addr := _out.data.write.addr
    core.writePort.data := MuxLookup(_out.control.writeSelect, 0.U)(
      _out.control.WriteSelect.all.map(x => (x -> _out.data.write.data(x.asUInt).asUInt))
    )
    _out
  }
}
