#!/bin/bash

make build-spi_oled
killall vp-breadboard
echo "Doing disconnected test..."
{ time hifive-vp spi_oled/spi_oled ; } &> test_run_inline_disco.txt
vp-breadboard &
echo "Doing connected test..."
sleep 1
{ time hifive-vp spi_oled/spi_oled ; } &> test_run_inline_connected.txt
killall vp-breadboard


tail -n 20 test_run*
