@PHONY: env

projects = sevensegment \
	   led_example \
	   morse \
	   spi_can \
	   spi_oled \
	   snake

all: build-all

define template
freedom-e-sdk/software/$(1): freedom-e-sdk/software
	cd freedom-e-sdk/software && ln -sd ../../$(1) || true
build-$(1): freedom-e-sdk/software/$(1) env
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

freedom-e-sdk/software:
	git submodule update --init # --recursive

env:
    ifneq ("$(shell which riscv64-unknown-elf-gcc)","")
	@echo "existing compiler found"
	export RISCV_PATH="$(shell dirname $(shell which riscv64-unknown-elf-gcc))/../"
	@echo "if this ->$(shell echo $$RISCV_PATH)<- is empty, then just copy/paste the expression into your terminal. Makefiles are hard."
    else
	@echo "No compiler found, using internal"
	git submodule update --init --recursive
	cd freedom-e-sdk && make riscv-gnu-toolchain
    endif
    ifeq ("$(shell which riscv-vp)","")
	@echo "No riscv-vp found!"
    endif
	#export RISCV_OPENOCD="/home/dwd/dev/sifive--hifive1/freedom-e-sdk/work/build/openocd/src/openocd"
