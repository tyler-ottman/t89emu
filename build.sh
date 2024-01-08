#!/bin/bash
echo "Building CPU emulator..."
cd ./t89emu
make
cd ../firmware
make
