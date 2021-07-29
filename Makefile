all:
	verilator --build --cc --exe --vpi -o vpitest one.sv two.sv main.cc -CFLAGS -g --timescale 1us/1ns --public-flat-rw --public
