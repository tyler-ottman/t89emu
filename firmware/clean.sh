#!/bin/bash
echo "Cleaning CPU emulator"
cd ../t89_cpu_components
make clean
cd ../t89_cpu
make clean