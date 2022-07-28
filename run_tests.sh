#!/bin/bash

make build-spi_oled
killall vp-breadboard
echo "Doing inline (SytemC) disconnected test..."
{ time hifive-vp spi_oled/spi_oled ; } &> test_run_inline_disco.txt
vp-breadboard -c :/conf/oled_shield.json &
echo "Doing inline (SystemC) connected test..."
sleep 1
{ time hifive-vp spi_oled/spi_oled ; } &> test_run_inline_connected.txt
killall vp-breadboard


echo "Doing new protocol disconnected test..."
{ time hifive-vp --disable-inline-oled spi_oled/spi_oled ; } &> test_run_NP_disco.txt
vp-breadboard -c :/conf/oled_iof_bidirectional.json &
echo "Doing new protocol connected (C++-Device) test, bidirectional..."
sleep 1
{ time hifive-vp --disable-inline-oled --wait-for-gpio spi_oled/spi_oled ; } &> test_run_NP_C++_connected_bidirectional.txt
killall vp-breadboard

vp-breadboard -c :/conf/oled_iof_noresponse.json &
echo "Doing new protocol connected (C++-Device) test, one-way spi (noresponse mode)..."
sleep 1
{ time hifive-vp --disable-inline-oled --wait-for-gpio spi_oled/spi_oled ; } &> test_run_NP_C++_connected_noresponse.txt
killall vp-breadboard

vp-breadboard -c :/conf/oled_iof_noresponse_lua.json &
echo "Doing new protocol connected (Lua-Device) test, one-way spi (noresponse mode)..."
sleep 1
{ time hifive-vp --disable-inline-oled --wait-for-gpio spi_oled/spi_oled ; } &> test_run_NP_Lua_connected_noresponse.txt
killall vp-breadboard

tail -n 6 test_run*
