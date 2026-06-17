module fifo #(
  parameter WIDTH = 8,
  parameter DEPTH = 4
)(
  input  logic clk,
  input  logic rst_n,
  input  logic push,
  input  logic pop,
  input  logic [WIDTH-1:0] data_in,
  output logic [WIDTH-1:0] data_out,
  output logic full,
  output logic empty
);

  logic [WIDTH-1:0] mem [DEPTH-1:0];
  logic [$clog2(DEPTH)-1:0] rd_ptr;
  logic [$clog2(DEPTH)-1:0] wr_ptr;
  logic [$clog2(DEPTH+1)-1:0] count;

  assign full = count == DEPTH;
  assign empty = count == 0;
  assign data_out = mem[rd_ptr];

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      rd_ptr <= 0;
      wr_ptr <= 0;
      count <= 0;
    end else begin
      if (push && !full) begin
        mem[wr_ptr] <= data_in;
        wr_ptr <= wr_ptr + 1;
      end

      if (pop && !empty) begin
        rd_ptr <= rd_ptr + 1;
      end

      case ({push && !full, pop && !empty})
        2'b10: count <= count + 1;
        2'b01: count <= count - 1;
        default: count <= count;
      endcase
    end
  end
endmodule
