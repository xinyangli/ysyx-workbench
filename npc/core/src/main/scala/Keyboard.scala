package npc.keyboard

import chisel3._
import chisel3.util.{Counter, Decoupled, Queue, Reverse, MuxLookup}

import npc.seg._

class PS2Port extends Bundle {
  val clk = Input(Bool())
  val data = Input(UInt(1.W))
}

object PS2Port {
  def apply(): PS2Port = {
    new PS2Port
  }
}

class KeyboardController extends Module {
  val io = IO(new Bundle {
    val ps2 = PS2Port()
    val out = Decoupled(UInt(8.W))
  })
  // valid only on the clock negedge of ps2_clk
  val ps2_clk_valid = RegNext(io.ps2.clk, false.B) & ~io.ps2.clk
  val cycle_counter = Counter(11)
  val concated_data = RegInit(0.U(8.W))

  val queue_io = Wire(Flipped(Decoupled(UInt(8.W))))
  val queue = Queue(queue_io, entries = 8)
  val received = RegInit(Bool(), false.B)
  val pushed = RegNext(queue_io.valid && queue_io.ready, false.B)
  queue_io.valid := false.B
  queue_io.bits := Reverse(concated_data)
  io.out <> queue

  when(cycle_counter.value === 0.U) {
    concated_data := 0.U
    received := false.B
  }

  when(ps2_clk_valid) {
    when(cycle_counter.value < 9.U && cycle_counter.value >= 1.U) {
      concated_data := (concated_data << 1) | io.ps2.data
    }.elsewhen(cycle_counter.value === 9.U) {
      received := true.B
    }
    cycle_counter.inc()
  }

  when(!pushed && received) {
    queue_io.valid := true.B
  }.elsewhen(pushed && received) {
    queue_io.valid := false.B
    received := false.B
  }
}

class SegHandler(seg_count: Int) extends Module {
  val io = IO(new Bundle {
    val keycode = Flipped(Decoupled(UInt(8.W)))
    val segs = Output(Vec(seg_count / 2, UInt(16.W)))
  })


  val seg_regs = RegInit(VecInit(Seq.fill(seg_count / 2)(0.U(16.W))))
  val last_keycode = RegInit(0.U(8.W))
  val counter = Counter(0xFF)
  val digit_to_seg = Seq(
    0.U -> "b0111111".U, // 0
    1.U ->"b0000110".U, // 1
    2.U -> "b1011011".U, // 2
    3.U -> "b1001111".U, // 3
    4.U -> "b1100110".U, // 4
    5.U -> "b1101101".U, // 5
    6.U -> "b1111101".U, // 6
    7.U -> "b0000111".U, // 7
    8.U -> "b1111111".U, // 8
    9.U -> "b1101111".U, // 9
    10.U -> "b1110111".U, // A
    11.U -> "b1111100".U, // B
    12.U -> "b0111001".U, // C
    13.U -> "b1011110".U, // D
    14.U -> "b1111001".U, // E
    15.U -> "b1110001".U  // F
  )
  io.segs := seg_regs

  when(io.keycode.valid) {
    val data = io.keycode.bits
    val state_f0_received = RegNext(data === 0xF0.U, false.B)
    io.keycode.ready := true.B
    // Handle keycode based on current state
    // (keyboard press counter) :: (ASCII code) :: (Keycode)
    when(state_f0_received) {
      // Release code
    }.otherwise{
      counter.inc()
      last_keycode := io.keycode.bits
    }
  }.otherwise {
    io.keycode.ready := false.B
  }
  
  seg_regs := Seq(counter.value, last_keycode, last_keycode).map(d => {
    MuxLookup(d & 0xF.U, 0.U)(digit_to_seg) | (MuxLookup((d >> 4.U) & 0xF.U, 0.U)(digit_to_seg) << 8.U)
  })
}


