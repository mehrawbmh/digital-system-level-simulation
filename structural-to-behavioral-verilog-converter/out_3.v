module Behavioral(
	input A,
	input B,
	input C,
	input D,
	output W,
	output V
);

	assign W = ~((A & B) | (C & D));
	assign V = ((~((A & B) | (C & D))) & A);

endmodule
