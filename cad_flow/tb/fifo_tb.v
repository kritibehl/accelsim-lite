`timescale 1ns/1ps

module fifo_tb;
  reg clk = 0;
  reg rst_n = 0;
  reg push = 0;
  reg pop = 0;
  reg [7:0] data_in = 0;
  wire [7:0] data_out;
  wire full;
  wire empty;

  fifo dut (
    .clk(clk),
    .rst_n(rst_n),
    .push(push),
    .pop(pop),
    .data_in(data_in),
    .data_out(data_out),
    .full(full),
    .empty(empty)
  );

  always #5 clk = ~clk;

  initial begin
    $display("fifo_tb start");

    rst_n = 0;
    #20;
    rst_n = 1;

    if (!empty) begin
      $display("FAIL: fifo should be empty after reset");
      $finish;
    end

    data_in = 8'hA5;
    push = 1;
    #10;
    push = 0;

    pop = 1;
    #10;
    pop = 0;

    $display("PASS: fifo push/pop smoke test");
    $finish;
  end
endmodule
