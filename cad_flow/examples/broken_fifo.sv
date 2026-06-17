// Intentional broken example for failure-injection documentation.
// Missing pointer update and incomplete boundary behavior.
module broken_fifo (
  input logic clk,
  input logic rst_n,
  input logic push,
  input logic pop,
  output logic full,
  output logic empty
);
  assign full = push;
  assign empty = pop;
endmodule
