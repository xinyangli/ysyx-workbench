package npc.keyboard

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec
import chiseltest.simulator.WriteVcdAnnotation

import npc.util._

class KeyboardControllerSpec extends AnyFreeSpec with ChiselScalatestTester {
  def transfer(keycode: Int, clock: Clock, ps2: PS2Port) : Unit = {
    require(keycode >= 0 && keycode < 0xFF)
    var cycle = 0
    var keycode_remain = keycode << 1   // Shift 1 to do nothing at cycle 0
    var keycode_collect = 0

    ps2.data.poke(1)
    ps2.clk.poke(true)
    clock.step(1)
    for (cycle <- 0 until 9) {
      val last_digit = keycode_remain & 1
      ps2.clk.poke(true)
      ps2.data.poke(last_digit)
      clock.step(32)
      keycode_collect = keycode_collect | (last_digit << cycle)
      keycode_remain = keycode_remain >> 1
      ps2.clk.poke(false)
      clock.step(32)
    }
    for (_ <- 9 until 11) {
      ps2.clk.poke(true)
      clock.step(32)
      ps2.clk.poke(false)
      clock.step(32)
    }
    assert(keycode_collect >> 1 == keycode)
    clock.step(32)
  }
  "Simple test" in {
    test(new KeyboardController).withAnnotations(Seq(WriteVcdAnnotation)) { c =>
      val data = Array(0xE4, 0xD4, 0xC4, 0xA9)
      data.foreach(d => {
        transfer(d, c.clock, c.io.ps2)
        c.io.out.valid.expect(1.U)
        c.io.out.bits.expect(d)
        c.io.out.ready.poke(1)
        c.clock.step(1)
        c.io.out.ready.poke(0)
      })
      data.foreach(d => {
        transfer(d, c.clock, c.io.ps2)
      })
      data.foreach(d => {
        c.io.out.valid.expect(1.U)
        c.io.out.bits.expect(d)
        c.io.out.ready.poke(1)
        c.clock.step(1)
        c.io.out.ready.poke(0)
      })
    }
  }
}
