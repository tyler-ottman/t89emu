#!/bin/bash
echo "Cleaning CPU emulator"
cd ./t89emu
make clean
cd ../firmware
make clean
