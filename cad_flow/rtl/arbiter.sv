module arbiter #(
  parameter N = 4
)(
  input  logic [N-1:0] req,
  output logic [N-1:0] grant
);
  always_comb begin
    grant = '0;
    for (int i = 0; i < N; i++) begin
      if (req[i] && grant == '0) begin
        grant[i] = 1'b1;
      end
    end
  end
endmodule
