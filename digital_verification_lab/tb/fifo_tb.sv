`timescale 1ns/1ps

module fifo_tb;

    localparam DATA_WIDTH = 8;
    localparam DEPTH = 4;
    localparam ADDR_WIDTH = 2;

    logic clk;
    logic rst_n;
    logic wr_en;
    logic rd_en;
    logic [DATA_WIDTH-1:0] din;
    logic [DATA_WIDTH-1:0] dout;
    logic full;
    logic empty;
    logic [ADDR_WIDTH:0] count;

    simple_fifo #(
        .DATA_WIDTH(DATA_WIDTH),
        .DEPTH(DEPTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) dut (
        .clk(clk),
        .rst_n(rst_n),
        .wr_en(wr_en),
        .rd_en(rd_en),
        .din(din),
        .dout(dout),
        .full(full),
        .empty(empty),
        .count(count)
    );

    fifo_assertions #(
        .DEPTH(DEPTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) assertions (
        .clk(clk),
        .rst_n(rst_n),
        .wr_en(wr_en),
        .rd_en(rd_en),
        .full(full),
        .empty(empty),
        .count(count)
    );

    always #5 clk = ~clk;

    task reset_fifo;
        begin
            rst_n = 0;
            wr_en = 0;
            rd_en = 0;
            din = '0;
            repeat (2) @(posedge clk);
            rst_n = 1;
            @(posedge clk);
        end
    endtask

    task write_fifo(input logic [DATA_WIDTH-1:0] value);
        begin
            @(posedge clk);
            din = value;
            wr_en = 1;
            rd_en = 0;
            @(posedge clk);
            wr_en = 0;
        end
    endtask

    task read_fifo;
        begin
            @(posedge clk);
            wr_en = 0;
            rd_en = 1;
            @(posedge clk);
            rd_en = 0;
        end
    endtask

    initial begin
        clk = 0;
        reset_fifo();

        // Scenario 1: reset behavior
        if (!empty || full || count != 0) begin
            $error("Reset state failed");
        end

        // Scenario 2: normal write/read ordering
        write_fifo(8'hA1);
        write_fifo(8'hB2);

        read_fifo();
        if (dout !== 8'hA1) begin
            $error("FIFO ordering failed for first read");
        end

        read_fifo();
        if (dout !== 8'hB2) begin
            $error("FIFO ordering failed for second read");
        end

        // Scenario 3: fill FIFO
        write_fifo(8'h11);
        write_fifo(8'h22);
        write_fifo(8'h33);
        write_fifo(8'h44);

        if (!full) begin
            $error("FIFO should be full after DEPTH writes");
        end

        // Scenario 4: attempted overfill
        @(posedge clk);
        din = 8'h55;
        wr_en = 1;
        rd_en = 0;
        @(posedge clk);
        wr_en = 0;

        if (count > DEPTH) begin
            $error("FIFO count exceeded depth after overfill attempt");
        end

        // Scenario 5: drain FIFO
        read_fifo();
        read_fifo();
        read_fifo();
        read_fifo();

        if (!empty) begin
            $error("FIFO should be empty after draining");
        end

        // Scenario 6: attempted under-read
        @(posedge clk);
        wr_en = 0;
        rd_en = 1;
        @(posedge clk);
        rd_en = 0;

        if (count != 0) begin
            $error("FIFO count changed after under-read attempt");
        end

        // Scenario 7: simultaneous read/write when not full/empty
        write_fifo(8'hC3);
        @(posedge clk);
        din = 8'hD4;
        wr_en = 1;
        rd_en = 1;
        @(posedge clk);
        wr_en = 0;
        rd_en = 0;

        $display("FIFO verification testbench completed");
        $finish;
    end

endmodule
