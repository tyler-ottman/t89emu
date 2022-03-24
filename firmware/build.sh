#!/bin/bash
echo "Building CPU emulator..."
cd ../t89_cpu_components
make
make tester
cd ../t89_cpu
make
make test_driver