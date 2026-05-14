module simple_fifo #(
    parameter DATA_WIDTH = 8,
    parameter DEPTH = 4,
    parameter ADDR_WIDTH = 2
) (
    input  logic                  clk,
    input  logic                  rst_n,
    input  logic                  wr_en,
    input  logic                  rd_en,
    input  logic [DATA_WIDTH-1:0] din,
    output logic [DATA_WIDTH-1:0] dout,
    output logic                  full,
    output logic                  empty,
    output logic [ADDR_WIDTH:0]   count
);

    logic [DATA_WIDTH-1:0] mem [DEPTH-1:0];
    logic [ADDR_WIDTH-1:0] wr_ptr;
    logic [ADDR_WIDTH-1:0] rd_ptr;

    assign full  = (count == DEPTH);
    assign empty = (count == 0);

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wr_ptr <= '0;
            rd_ptr <= '0;
            count  <= '0;
            dout   <= '0;
        end else begin
            case ({wr_en && !full, rd_en && !empty})
                2'b10: begin
                    mem[wr_ptr] <= din;
                    wr_ptr <= wr_ptr + 1'b1;
                    count <= count + 1'b1;
                end

                2'b01: begin
                    dout <= mem[rd_ptr];
                    rd_ptr <= rd_ptr + 1'b1;
                    count <= count - 1'b1;
                end

                2'b11: begin
                    mem[wr_ptr] <= din;
                    wr_ptr <= wr_ptr + 1'b1;
                    dout <= mem[rd_ptr];
                    rd_ptr <= rd_ptr + 1'b1;
                end

                default: begin
                    count <= count;
                end
            endcase
        end
    end

endmodule
