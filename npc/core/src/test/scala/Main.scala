package npc

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec
import chiseltest.simulator.ChiselBridge
import chiseltest.simulator.WriteVcdAnnotation

class RegisterFileSpec extends AnyFreeSpec with ChiselScalatestTester {
  "RegisterFile should work" - {
    "with 2 read ports" in {
      test(new RegisterFile(2)) { c =>
        def readExpect(addr: Int, value: Int, port: Int = 0): Unit = {
          c.io.readAddr(port).poke(addr.U)
          c.io.readData(port).expect(value.U)
        }
        def write(addr: Int, value: Int): Unit = {
          c.io.writeEnable.poke(true.B)
          c.io.writeData.poke(value.U)
          c.io.writeAddr.poke(addr.U)
          c.clock.step(1)
          c.io.writeEnable.poke(false.B)
        }
        // everything should be 0 on init
        for (i <- 0 until 32) {
          readExpect(i, 0, port = 0)
          readExpect(i, 0, port = 1)
        }

        // write 5 * addr + 3
        for (i <- 0 until 32) {
          write(i, 5 * i + 3)
        }

        // check that the writes worked
        for (i <- 0 until 32) {
          readExpect(i, if (i == 0) 0 else 5 * i + 3, port = i % 2)
        }
      }
    }
  }
}

class ALUGeneratorSpec extends AnyFreeSpec with ChiselScalatestTester {
  "With 32 width, " - {
    val neg = (x: BigInt) => BigInt("FFFFFFFF", 16) - x + 1
    val not = (x: BigInt) => x ^ BigInt("FFFFFFFF", 16)
    val mask = BigInt("FFFFFFFF", 16)
    val oprands: List[(BigInt, BigInt)] = List(
      (5, 3), (101010, 101010), (0xFFFFFFFCL, 0xFFFFFFFFL), (4264115, 2)
    )
    val operations: Map[Int, (BigInt, BigInt) => BigInt] = Map(
      0 -> ((a: BigInt, b: BigInt) => (a + b) & mask),
      1 -> ((a: BigInt, b: BigInt) => (a + neg(b)) & mask),
      2 -> ((a, _) => not(a)),
      3 -> (_ & _),
      4 -> (_ | _),
      5 -> (_ ^ _),
      6 -> ((a, b) => if (a < b) 1 else 0),
      7 -> ((a, b) => if (a == b) 1 else 0),
    )
    val validate = (c: ALUGenerator,op: Int, oprands: List[(BigInt, BigInt)]) => {
      c.io.op.poke(op.U)
      oprands.foreach({ case (a, b) =>
        c.io.a.poke(a.U)
        c.io.b.poke(b.U)
        c.io.out.expect(operations(op)(a, b))
      })
    }
    "add should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 0, oprands) }
    }
    "sub should work" - {
      "with positive result" in {
        test(new ALUGenerator(32)) { c =>
          validate(c, 1, oprands.filter({case (a, b) => a >= b}))
        }
      }
      "with negative result" in {
        test(new ALUGenerator(32)) { c =>
          validate(c, 1, oprands.filter({case (a, b) => a < b}))
        }
      }
    }
    "not should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 2, oprands) }
    }
    "and should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 3, oprands) }
    }
    "or should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 4, oprands) }
    }
    "xor should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 5, oprands) }
    }
    "compare should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 6, oprands) }
    }
    "equal should work" in {
      test(new ALUGenerator(32)) { c => validate(c, 7, oprands) }
    }
  }
}

class KeyboardControllerSpec extends AnyFreeSpec with ChiselScalatestTester {
  def transfer(keycode: Int, c: KeyboardController) : Unit = {
    require(keycode >= 0 && keycode < 0xFF)
    var cycle = 0
    var ps2_clk = true
    var keycode_remain = keycode << 1   // Shift 1 to do nothing at cycle 1
    var keycode_collect = 0

    c.io.ps2_clk.poke(ps2_clk)
    c.io.ps2_data.poke(1)
    for (cycle <- 0 until 9) {
      c.io.ps2_clk.poke(true)
      c.clock.step(32)
      val last_digit = keycode_remain & 1
      c.io.ps2_data.poke(last_digit)
      keycode_collect = keycode_collect | (last_digit << cycle)
      keycode_remain = keycode_remain >> 1
      c.io.ps2_clk.poke(false)
      c.clock.step(32)
    }
    for (_ <- 9 until 11) {
      c.io.ps2_clk.poke(true)
      c.clock.step(32)
      c.io.ps2_clk.poke(ps2_clk)
      ps2_clk = !ps2_clk
      c.io.ps2_clk.poke(false)
      c.clock.step(32)
    }
    assert(keycode_collect >> 1 == keycode)
    c.io.out.ready.poke(1)
    c.clock.step(32)
    c.io.out.bits.expect(keycode)
  }
  "Simple test" in {
    test(new KeyboardController).withAnnotations(Seq(WriteVcdAnnotation)) { c =>
      transfer(0xE4, c)
      transfer(0xE4, c)
      transfer(0xE4, c)
      transfer(0xE4, c)
    }
  }
}
