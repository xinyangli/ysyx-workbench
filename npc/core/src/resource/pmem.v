import "DPI-C" function int pmem_read(input int addr);
import "DPI-C" function void pmem_write(
  input int waddr, input int wdata, input byte wmask);

