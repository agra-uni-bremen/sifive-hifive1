#!/bin/bash

make build-spi_oled
killall vp-breadboard
echo "Doing inline disconnected test..."
{ time hifive-vp spi_oled/spi_oled ; } &> test_run_inline_disco.txt
vp-breadboard &
echo "Doing inline connected test..."
sleep 1
{ time hifive-vp spi_oled/spi_oled ; } &> test_run_inline_connected.txt
killall vp-breadboard


echo "Doing new protocol disconnected test..."
{ time hifive-vp --disable-inline-oled spi_oled/spi_oled ; } &> test_run_NP_disco.txt
vp-breadboard -c :/conf/oled_iof.json &
echo "Doing new protocol connected test..."
sleep 1
{ time hifive-vp --disable-inline-oled --wait-for-gpio spi_oled/spi_oled ; } &> test_run_NP_connected.txt
killall vp-breadboard


tail -n 6 test_run*
