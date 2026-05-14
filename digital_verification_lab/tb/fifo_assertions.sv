module fifo_assertions #(
    parameter DEPTH = 4,
    parameter ADDR_WIDTH = 2
) (
    input logic                clk,
    input logic                rst_n,
    input logic                wr_en,
    input logic                rd_en,
    input logic                full,
    input logic                empty,
    input logic [ADDR_WIDTH:0] count
);

    property count_never_exceeds_depth;
        @(posedge clk) disable iff (!rst_n)
        count <= DEPTH;
    endproperty

    property empty_matches_zero_count;
        @(posedge clk) disable iff (!rst_n)
        empty == (count == 0);
    endproperty

    property full_matches_depth_count;
        @(posedge clk) disable iff (!rst_n)
        full == (count == DEPTH);
    endproperty

    property no_underflow_attempt;
        @(posedge clk) disable iff (!rst_n)
        !(rd_en && empty);
    endproperty

    property no_overflow_attempt;
        @(posedge clk) disable iff (!rst_n)
        !(wr_en && full);
    endproperty

    assert property (count_never_exceeds_depth)
        else $error("FIFO count exceeded DEPTH");

    assert property (empty_matches_zero_count)
        else $error("FIFO empty flag inconsistent with count");

    assert property (full_matches_depth_count)
        else $error("FIFO full flag inconsistent with count");

    assert property (no_underflow_attempt)
        else $warning("Read attempted while FIFO empty");

    assert property (no_overflow_attempt)
        else $warning("Write attempted while FIFO full");

endmodule
