all:	build

pre:
	cd freedom-e-sdk/software && ln -sd ../../sevensegment || true
	cd freedom-e-sdk/software && ln -sd ../../led_example || true
	cd freedom-e-sdk/software && ln -sd ../../morse || true
env:
	export RISCV_PATH="/opt/riscv64"
	#export RISCV_OPENOCD="/home/dwd/dev/sifive--hifive1/freedom-e-sdk/work/build/openocd/src/openocd"

build-morse:
	cd freedom-e-sdk && make software PROGRAM=morse

build-led_example:
	cd freedom-e-sdk && make software PROGRAM=led_example

build-sevensegment:
	cd freedom-e-sdk && make software PROGRAM=sevensegment

upload-morse: build-morse
	cd freedom-e-sdk && make upload PROGRAM=morse

upload-led_example: build-led_example
	cd freedom-e-sdk && make upload PROGRAM=led_example

upload-sevensegment: build-sevensegment
	cd freedom-e-sdk && make upload PROGRAM=sevensegment

sim-morse: build-morse
	hifive-vp morse/morse

sim-led_example: build-led_example
	hifive-vp led_example/led_example

sim-sevensegment: build-sevensegment
	hifive-vp sevensegment/sevensegment

