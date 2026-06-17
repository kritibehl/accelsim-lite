`timescale 1ns/1ps

module arbiter_tb;
  reg [3:0] req;
  wire [3:0] grant;

  arbiter dut(.req(req), .grant(grant));

  initial begin
    $display("arbiter_tb start");

    req = 4'b0000;
    #1;
    if (grant !== 4'b0000) begin
      $display("FAIL: grant should be zero");
      $finish;
    end

    req = 4'b1010;
    #1;
    if (grant !== 4'b0010) begin
      $display("FAIL: priority grant mismatch");
      $finish;
    end

    $display("PASS: arbiter priority smoke test");
    $finish;
  end
endmodule
