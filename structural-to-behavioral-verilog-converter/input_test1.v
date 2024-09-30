module Structural(
    input A,
    input B,
    input C,
    input D,
    output W
);

    wire w1;
    wire w2;
    wire w3;
    wire w4;
    wire w5;

    and i1(w1, A, B);
    and i2(w2, C, D);
    or i3(w3, w1, w2);
    not i4(w4, w3);
    not i5(w5, w4);
    not i6(W, w5);
    
endmodule