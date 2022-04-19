#!/bin/bash

make build-spi_oled
killall vp-breadboard
echo "Doing disconnected test..."
{ time hifive-vp --disable-inline-oled spi_oled/spi_oled ; } &> test_run_NP_disco.txt
vp-breadboard -c :/conf/oled_iof.json &
echo "Doing connected test..."
sleep 1
{ time hifive-vp --disable-inline-oled --wait-for-gpio spi_oled/spi_oled ; } &> test_run_NP_connected.txt
killall vp-breadboard


tail -n 20 test_run*
