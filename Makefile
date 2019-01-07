all:	build

pre:
	cd freedom-e-sdk/software && ln -sd ../../sevensegment || true
	cd freedom-e-sdk/software && ln -sd ../../led_example || true
env:
	export RISCV_PATH="/opt/riscv64"
	#export RISCV_OPENOCD="/home/dwd/dev/sifive--hifive1/freedom-e-sdk/work/build/openocd/src/openocd"

build-sevensegment:
	cd freedom-e-sdk && make software PROGRAM=sevensegment

upload-sevensegment: build-sevensegment
	cd freedom-e-sdk && make upload PROGRAM=sevensegment

sim-sevensegment: build-sevensegment
	hifive-vp sevensegment/sevensegment

build-led_example:
	cd freedom-e-sdk && make software PROGRAM=led_example

upload-led_example: build-led_example
	cd freedom-e-sdk && make upload PROGRAM=led_example

sim-led_example: build-led_example
	hifive-vp led_example/led_example

