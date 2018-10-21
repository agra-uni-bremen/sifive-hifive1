Debugging:
==========

see: https://forums.sifive.com/t/how-can-i-debug-the-hifive-board-by-jtag/649

1) In one terminal:
	
	make run_openocd
	
2) Then, in another terminal:

	make run_gdb PROGRAM=hello
	
Press the reset button on the board before doing step 2 (exit and restart gdb to re-run) in order to break before the program is started (thus breakpoints can be set).



The bootcode disassemble is retrieved by running:

	(gdb) set pagination off
	(gdb) disassemble 0x20000000, 0x20001000


The bootcode binary code by:

	dump binary memory bootcode.bin 0x20000000 0x20001000

