#!/bin/bash
echo "Building CPU emulator..."
cd ../modules
make
cd ../top-level
make
