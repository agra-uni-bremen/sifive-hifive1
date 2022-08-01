How to build bare metal applications on HiFive1-Board
=====================================================

This Repository contains some software examples for use with the HiFive1-Board of SiFive and our binary-compatible [Riscv-VP](systemc-verification.org/riscv-vp) ([repo](https://github.com/agra-uni-bremen/riscv-vp)).
Here, an *old* version of the [freedom-e-sdk](https://github.com/sifive/freedom-e-sdk) of SiFive is pinned and used for building/uploading programs.

The main idea of this repo is to build small and easy bare metal programs to be used in conjunction with the `vp-breadboard` which is a breadboard/pcb simulator with some buttons, oled display and leds. It is to be found in the `riscv-vp` repo under `env/hifive/vp-breadboard` and can be opened before calling any of the `make sim-*` targets.

1) Prerequisites on ubuntu (see needed packages at *https://github.com/sifive/freedom-e-sdk/tree/baeeb8f*):

	`sudo apt-get install autoconf automake libmpc-dev libmpfr-dev libgmp-dev gawk bison flex texinfo libtool libusb-1.0-0-dev make g++ pkg-config libexpat1-dev zlib1g-dev`

2) Clone and update the *sifive--hifive1* repository:

	```bash
	git clone https://github.com/agra-uni-bremen/sifive-hifive1
	cd sifive-hifive1/freedom-e-sdk
	git submodule update --init --recursive # may take a bit
	```

3) Either:
    a) Download and unpack a pre-build *riscv-multilib* toolchain, e.g. available from:

	http://satisfy.informatik.uni-bremen.de/gnu-toolchain_riscv-multilib/latest-gnu-toolchain_riscv-multilib.tar.gz
	And make sure that the env variable RISCV_PATH is pointing to your toolchain-dir (without `/bin`)
	
	example:
	```bash
	wget http://satisfy.informatik.uni-bremen.de/gnu-toolchain_riscv-multilib/latest-gnu-toolchain_riscv-multilib.tar.gz
	tar xzf latest-gnu-toolchain_riscv-multilib.tar.gz
	sudo mv riscv-multilib /opt
	export RISCV_PATH="/opt/riscv-multilib" #  you may add this line to your .bashrc
	```

    b) Or build the toolchain directly:

	```bash
	cd freedom-e-sdk
	make riscv-gnu-toolchain -j$(nproc) # may take a bit
	```

4) Build openocd (if you want to load a program on the real board) in *sifive--hifive1/freedom-e-sdk/*:

	`make openocd # need only to be done once`


5) Build and upload a program to the board:

	```bash
	make upload-snake
	```
   Optional: If you have the `hifive-vp` from the [riscv-vp](https://github.com/agra-uni-bremen/riscv-vp) repository in your `$PATH`, you may also start the simulation by typing
	```bash
	make sim-snake
	```

6) Show program output:

	`screen /dev/ttyUSB1 115200`

In case screen does not work use (this may happen when running screen a second time):

	`cat /dev/ttyUSB1	# NOTE: need to run screen once before this command works`

Press the reset button on the board to restart (and thus see output).
NOTE: /dev/ttyUSB0 is the debug interface.



ZEPHYR OS
=========

To use Zephyr, see zephyr/RREADME.rst. (Install OS Packages, pip packages)
To build for HiFive1, use example zephyrrc file, modify for your paths, rename to ~/.zephyrrc and source it(?). Source zephyr-env.sh. Then build your project:

```bash
mkdir build && cmake .. -DBOARD=$BOARD
make -j6
```


Uploading Zephyr-Elfs to board
------------------------------

Basically the same procedure as freedom-e-sdk, but manual.

```bash
$ ${SDK_PATH}/work/build/openocd/prefix/bin/openocd -f ${SDK_PATH}/bsp/env/freedom-e300-hifive1/openocd.cfg &
$ riscv32-unknown-elf-gdb
 set remotetimeout 240
 target extended-remote localhost:3333
 monitor reset halt
 monitor flash protect 0 64 last off
 load zephyr/zephyr.elf
 monitor resume
```

Other Doc
=========

FreeRTOS example uses 14.7kiB out of 16kiB RAM (with 2k stack):

	size RISCV_HiFive1_GCC.elf -d
	   text	   data	    bss	    dec	    hex	filename
	  17079	   1076	  13953	  32108	   7d6c	RISCV_HiFive1_GCC.elf

Zephyr uses with the philosophers demo:

	Memory region         Used Size  Region Size  %age Used
		         ROM:       22960 B        12 MB      0.18%
		         RAM:        9440 B        16 KB     57.62%
		    IDT_LIST:         553 B         2 KB     27.00%

		    
#### Acknowledgements:

This work was supported in part by the German Federal Ministry of Education and Research (BMBF) within the project SATiSFy under contract no. 16KIS0821K.
