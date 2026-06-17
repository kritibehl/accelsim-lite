module register_file #(
  parameter WIDTH = 32,
  parameter DEPTH = 8
)(
  input  logic clk,
  input  logic write_en,
  input  logic [$clog2(DEPTH)-1:0] write_addr,
  input  logic [$clog2(DEPTH)-1:0] read_addr,
  input  logic [WIDTH-1:0] write_data,
  output logic [WIDTH-1:0] read_data
);
  logic [WIDTH-1:0] regs [DEPTH-1:0];

  always_ff @(posedge clk) begin
    if (write_en) begin
      regs[write_addr] <= write_data;
    end
  end

  assign read_data = regs[read_addr];
endmodule
