1) Prerequisites on ubuntu (see *https://github.com/sifive/freedom-e-sdk*):

	sudo apt-get install autoconf automake libmpc-dev libmpfr-dev libgmp-dev gawk bison flex texinfo libtool libusb-1.0-0-dev make g++ pkg-config libexpat1-dev zlib1g-dev


2) Clone and update the *sifive--hifive1* repository:

	git clone git@gitlab.informatik.uni-bremen.de:ppieper/sifive--hifive1.git
	cd sifive--hifive1

	cd freedom-e-sdk
	git submodule update --init --recursive        # may take some time


3) Download and unpack the pre-build *riscv-multilib* toolchain available from:

	http://raws13/gnu-toolchain_riscv-multilib/latest-gnu-toolchain_riscv-multilib.tar.gz
	


4) In *sifive--hifive1/freedom-e-sdk/Makefile* adapt the following paths to point to the above *riscv-multilib*:

	"toolchain_srcdir := riscv-gnu-toolchain"
	"toolchain_builddir := $(builddir)/riscv-gnu-toolchain/riscv64-unknown-elf"
	"RISCV_PATH ?= $(toolchain_prefix)"
	
==>> use your own local path of *riscv-multilib* ==>>

	"toolchain_srcdir := /home/vladi/work/riscv/toolchain/latest-gnu-toolchain_riscv-multilib/"
	"toolchain_builddir := /home/vladi/work/riscv/toolchain/latest-gnu-toolchain_riscv-multilib/"
	"RISCV_PATH ?= /home/vladi/work/riscv/toolchain/latest-gnu-toolchain_riscv-multilib/"	



5) Build openocd (to load a program on the board) in *sifive--hifive1/freedom-e-sdk/*:

	make openocd						# need only to be done once
	
	
6) Build and upload a program to the board:

	make software PROGRAM=hello			# replace hello with other programs accordingly (e.g. *fade_led*, etc., see *software* folder)

	sudo make upload PROGRAM=hello		# may not be necessary to use *sudo*, checkout the *sifive--hifive1/doc/hifive1-getting-started-v1.0.2.pdf* on additional details


7) Show program output:

	sudo screen /dev/ttyUSB1 115200

In case screen does not work use (this may happen when running screen a second time):

	sudo cat /dev/ttyUSB1	# NOTE: need to run screen once before this command works

Press the reset button on the board to restart (and thus see output).
NOTE: /dev/ttyUSB0 is the debug interface.


======================================================================================

Disable compressed instructions:

	In *sifive--hifive1/freedom-e-sdk/bsp/env/freedom-e300-hifive1/* open *setting.mk* and set *RISCV_ARCH := rv32im* (set to rv32imac to re-enable). NOTE: *RISCV_ARCH := rv32ima* might also be possible, however the RISC-V multilib toolchain seems to not support it for now!?

