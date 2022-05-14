#!/bin/bash
echo "Cleaning CPU emulator"
cd ../modules
make clean
cd ../top-level
make clean
