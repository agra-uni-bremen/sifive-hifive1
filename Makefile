projects = sevensegment \
	   led_example \
	   morse \
	   spi_can \
	   spi_oled \
	   snake

all: build-all

define template
pre-$(1): 
	cd freedom-e-sdk/software && ln -sd ../../$(1) || true
build-$(1): pre-$(1)
	cd freedom-e-sdk && make software PROGRAM=$(1)
upload-$(1): build-$(1)
	cd freedom-e-sdk && make upload PROGRAM=$(1)
sim-$(1): build-$(1)
	 hifive-vp $(1)/$(1)
endef

$(foreach project,$(projects),$(eval $(call template,$(project))))

pres = $(addprefix pre-, $(projects))
pre:	$(pres)

build-all: $(addprefix build-, $(projects))

env:
	echo "Set:"
	export RISCV_PATH="/opt/riscv64-multi"
	#export RISCV_OPENOCD="/home/dwd/dev/sifive--hifive1/freedom-e-sdk/work/build/openocd/src/openocd"
