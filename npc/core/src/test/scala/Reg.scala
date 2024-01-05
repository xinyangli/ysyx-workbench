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

class MuxGeneratorSpec extends AnyFreeSpec with ChiselScalatestTester {
  "MuxGenerator should work" - {
    "when there are 2 inputs" in {
      test(new MuxGenerator(8, 2)) { c =>
        c.io.in(0).poke(0.U)
        c.io.in(1).poke(1.U)
        c.io.sel.poke(0.U)
        c.io.out.expect(0.U)
        c.io.sel.poke(1.U)
        c.io.out.expect(1.U)
      }
    }
    "when there are 1024 inputs" in {
      test(new MuxGenerator(32, 1024)) { c =>
        for (i <- 0 until 1024) {
          c.io.in(i).poke(i.U)
        }
        for (i <- 0 until 1024) {
          c.io.sel.poke(i.U)
          c.io.out.expect(i.U)
        }
      }
    }
  }
  "MuxGenerator should raise exception" - {
    "when nInput is not 2^n" in {
      assertThrows[IllegalArgumentException] {
        test(new MuxGenerator(8, 3)) { c => }
      }
    }
  }
}
