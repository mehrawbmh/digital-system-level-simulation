module Behavioral(
	input A,
	input B,
	input C,
	input D,
	output W,
	output V,
	output G,
	output H
);

	assign W = ~((A & B) | (C & D));
	assign V = (A & B);
	assign G = (A | B);
	assign H = ~C;

endmodule
