#!/bin/bash
cd freedom-e-sdk/
sudo work/build/openocd/prefix/bin/openocd -f bsp/env/freedom-e300-hifive1/openocd.cfg &
/opt/riscv64-multi/bin/riscv64-unknown-elf-gdb software/spi_oled/spi_oled --batch -ex "set remotetimeout 240" -ex "target extended-remote localhost:3333" \
-ex "monitor reset halt" -ex "monitor flash protect 0 64 last off" -ex "load" -ex "monitor resume" -ex "monitor shutdown" -ex "quit" && echo "yay"
