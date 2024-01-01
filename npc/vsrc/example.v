module top(
  input [1:0] sw,
  output ledr
);
  assign ledr = sw[1] ^ sw[0];
endmodule