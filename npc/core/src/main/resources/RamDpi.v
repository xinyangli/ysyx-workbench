import "DPI-C" function int pmem_read(input int addr);
import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

module RamDpi (
  input clock,
  input reset,
  input writeEnable,
  input valid,
  input [31:0] writeAddr,
  input [31:0] writeData,
  input [3:0] writeMask,
  input reg [31:0] readAddr,
  output reg [31:0] readData,
  input reg [31:0] pc,
  output reg [31:0] inst
);
  always @(*) begin
    if (valid) begin // 有读写请求时
      readData = pmem_read(readAddr);
      // readData = {
      //   {readData[31:24] & {{7{writeMask[3]}}, writeMask[3] }},
      //   {readData[23:16] & {{7{writeMask[2]}}, writeMask[2] }},
      //   {readData[15:8] & {{7{writeMask[1]}}, writeMask[1] }},
      //   {readData[7:0] & {{7{writeMask[0]}}, writeMask[0] }} 
      // };
      if (writeEnable) begin // 有写请求时
        pmem_write(writeAddr, writeData, { 4'h0, writeMask });
      end
    end
    else begin
      readData = 0;
    end
    inst = pmem_read(pc);
  end
endmodule