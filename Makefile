all:	build

pre:
	cd freedom-e-sdk/software && ln -sd ../../sevensegment || true
	cd freedom-e-sdk/software && ln -sd ../../led_example || true
	cd freedom-e-sdk/software && ln -sd ../../morse || true
	cd freedom-e-sdk/software && ln -sd ../../spi_can || true
	cd freedom-e-sdk/software && ln -sd ../../spi_oled || true
env:
	export RISCV_PATH="/opt/riscv64"
	#export RISCV_OPENOCD="/home/dwd/dev/sifive--hifive1/freedom-e-sdk/work/build/openocd/src/openocd"

build-morse:
	cd freedom-e-sdk && make software PROGRAM=morse

build-led_example:
	cd freedom-e-sdk && make software PROGRAM=led_example

build-sevensegment:
	cd freedom-e-sdk && make software PROGRAM=sevensegment

build-spi_can:
	cd freedom-e-sdk && make software PROGRAM=spi_can

build-spi_oled:
	cd freedom-e-sdk && make software PROGRAM=spi_oled

upload-morse: build-morse
	cd freedom-e-sdk && make upload PROGRAM=morse

upload-led_example: build-led_example
	cd freedom-e-sdk && make upload PROGRAM=led_example

upload-sevensegment: build-sevensegment
	cd freedom-e-sdk && make upload PROGRAM=sevensegment

upload-spi_can: build-spi_can
	cd freedom-e-sdk && make upload PROGRAM=spi_can

upload-spi_oled: build-spi_oled
	cd freedom-e-sdk && make upload PROGRAM=spi_oled

sim-morse: build-morse
	hifive-vp morse/morse

sim-led_example: build-led_example
	hifive-vp led_example/led_example

sim-sevensegment: build-sevensegment
	hifive-vp sevensegment/sevensegment

sim-spi_can: build-spi_can
	hifive-vp spi_can/spi_can

sim-spi_oled: build-spi_oled
	hifive-vp spi_oled/spi_oled
