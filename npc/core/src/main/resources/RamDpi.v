import "DPI-C" function int pmem_read(input int addr);
import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

module RamDpi (
  input writeEnable,
  input valid,
  input [31:0] writeAddr,
  input [31:0] writeData,
  input [3:0] writeMask,
  input reg [31:0] readAddr,
  output reg [31:0] readData
);
  always @(*) begin
    if (valid) begin // 有读写请求时
      readData = pmem_read(readAddr);
      if (writeEnable) begin // 有写请求时
        pmem_write(writeAddr, writeData, { 4'h0, writeMask });
      end
    end
    else begin
      readData = 0;
    end
  end
endmodule