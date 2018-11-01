all:	build

pre:
	cd freedom-e-sdk/software && ln -sd ../../sevensegment
env:
	export RISCV_PATH="/opt/riscv"
	export RISCV_OPENOCD="/home/dwd/dev/sifive--hifive1/freedom-e-sdk/work/build/openocd/src/openocd"

build:
	cd freedom-e-sdk && make software PROGRAM=sevensegment

upload: build
	cd freedom-e-sdk && make upload PROGRAM=sevensegment
