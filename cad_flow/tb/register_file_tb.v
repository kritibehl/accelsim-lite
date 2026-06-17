`timescale 1ns/1ps

module register_file_tb;
  reg clk = 0;
  reg write_en = 0;
  reg [2:0] write_addr = 0;
  reg [2:0] read_addr = 0;
  reg [31:0] write_data = 0;
  wire [31:0] read_data;

  register_file dut (
    .clk(clk),
    .write_en(write_en),
    .write_addr(write_addr),
    .read_addr(read_addr),
    .write_data(write_data),
    .read_data(read_data)
  );

  always #5 clk = ~clk;

  initial begin
    $display("register_file_tb start");

    write_addr = 3'd2;
    read_addr = 3'd2;
    write_data = 32'hDEADBEEF;
    write_en = 1;
    #10;
    write_en = 0;
    #1;

    if (read_data !== 32'hDEADBEEF) begin
      $display("FAIL: register read/write mismatch");
      $finish;
    end

    $display("PASS: register file write/read smoke test");
    $finish;
  end
endmodule
