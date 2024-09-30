module Behavioral(
	input A,
	input B,
	input C,
	input D,
	output W,
	output Y
);

	assign W = ~((A & B) | (C & D));
	assign Y = ~(C & D);

endmodule
