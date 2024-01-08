package npc

import chisel3._
import chisel3.util.{MuxLookup, Fill, Decoupled, Counter, Queue, Reverse}
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

class ALUGenerator(width: Int) extends Module {
  require(width >= 0)
  val io = IO(new Bundle {
    val a = Input(UInt(width.W))
    val b = Input(UInt(width.W))
    val op = Input(UInt(4.W))
    val out = Output(UInt(width.W))
  })

  val adder_b = (Fill(width, io.op(0)) ^ io.b) + io.op(0)  // take (-b) if sub
  val add = io.a + adder_b
  val and = io.a & io.b
  val not = ~io.a
  val or = io.a | io.b
  val xor = io.a ^ io.b
  val slt = io.a < io.b
  val eq = io.a === io.b

  io.out := MuxLookup(io.op, 0.U)(Seq(
    0.U -> add,
    1.U -> add, // add with b reversed
    2.U -> not,
    3.U -> and,
    4.U -> or,
    5.U -> xor,
    6.U -> slt,
    7.U -> eq,
  ))
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

class KeyboardController extends Module {
  val io = IO(new Bundle {
    val ps2_clk = Input(Bool())
    val ps2_data = Input(UInt(1.W))
    val out = Decoupled(UInt(8.W))
  })
  val queue_io = Wire(Flipped(Decoupled(UInt(8.W))))
  queue_io.valid := true.B
  queue_io.bits := 0.B
  val queue = Queue(queue_io, entries = 8)
  io.out <> queue

  // valid only on the clock negedge of ps2_clk
  val ps2_clk_valid = RegNext(io.ps2_clk, false.B) & ~io.ps2_clk
  val cycle_counter = Counter(11)
  val concated_data = RegInit(0.U(8.W))
  val is_receiving = RegInit(Bool(), false.B)

  when(io.ps2_clk && io.ps2_data === 1.U) {
    // Start receiving data
    is_receiving := true.B
  }

  when(is_receiving && ps2_clk_valid) {
    cycle_counter.inc()
    when(cycle_counter.value < 9.U && cycle_counter.value > 0.U) {
      concated_data := (concated_data << 1) | io.ps2_data
    } 
    when(cycle_counter.value === 10.U) {
      is_receiving := false.B
    }
  }

  when(is_receiving) {
    queue_io.noenq()
  }.otherwise{
    queue_io.enq(Reverse(concated_data))
  }
}

class Switch extends Module {
  val io = IO(new Bundle {
    val sw = Input(Vec(2, Bool()))
    val out = Output(Bool())
  })

  io.out := io.sw(0) ^ io.sw(1)
}
