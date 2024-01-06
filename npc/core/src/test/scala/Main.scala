package npc

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec

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
    "add should work" in {
      test(new ALUGenerator(32)) { c =>
        c.io.op.poke(0.U)
        c.io.a.poke(5.U)
        c.io.b.poke(3.U)
        c.io.out.expect(8.U)
      }
    }
    "sub should work" - {
      "with positive result" in {
        test(new ALUGenerator(32)) { c =>
          c.io.op.poke(1.U)
          c.io.a.poke(5.U)
          c.io.b.poke(3.U)
          c.io.out.expect(2)
        }
      }
      "with negative result" in {
        test(new ALUGenerator(32)) { c =>
          c.io.op.poke(1.U)
          c.io.a.poke(3.U)
          c.io.b.poke(5.U)
          c.io.out.expect(BigInt("FFFFFFFF", 16) - 1)
        }
      }
    }
    "not should work" in {
      // test(new ALUGenerator(32)) { c =>
      //   c.io.op.poke(2.U)
      //   c.io.a.poke(5.U)
      //   c.io.b.poke(3.U)
      //   c.io.out.expect(((1 << 32) - 1 - 5).U)
      // }
    }
    "and should work" in {
      test(new ALUGenerator(32)) { c =>
        c.io.op.poke(3.U)
        c.io.a.poke(5.U)
        c.io.b.poke(3.U)
        c.io.out.expect(1.U)
      }
    }
    "or should work" in {
      test(new ALUGenerator(32)) { c =>
        c.io.op.poke(4.U)
        c.io.a.poke(5.U)
        c.io.b.poke(3.U)
        c.io.out.expect(7.U)
      }
    }
    "xor should work" in {
      test(new ALUGenerator(32)) { c =>
        c.io.op.poke(5.U)
        c.io.a.poke(5.U)
        c.io.b.poke(3.U)
        c.io.out.expect(6.U)
      }
    }
    "compare should work" in {
      test(new ALUGenerator(32)) { c =>
        c.io.op.poke(6)

        c.io.a.poke(62.U)
        c.io.b.poke(3.U)
        c.io.out.expect(0.U)

        c.io.a.poke(2.U)
        c.io.b.poke(103.U)
        c.io.out.expect(1.U)

        c.io.a.poke(16.U)
        c.io.b.poke(16.U)
        c.io.out.expect(0.U)
      }
    }
    "equal should work" in {
      test(new ALUGenerator(32)) { c =>
        c.io.op.poke(7)

        c.io.a.poke(62.U)
        c.io.b.poke(3.U)
        c.io.out.expect(0.U)

        c.io.a.poke(2.U)
        c.io.b.poke(103.U)
        c.io.out.expect(0.U)

        c.io.a.poke(16.U)
        c.io.b.poke(16.U)
        c.io.out.expect(1.U)
      }
    }
  }
}
