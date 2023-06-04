#!/bin/bash

# Compile the C++ test
g++ -mavx2 -mfma -O3 test.cc -o test

# Get the CPU frequency
CPU_FREQ=$(cat /proc/cpuinfo | grep Hz)

echo "CPU Frequency: $CPU_FREQ MHz"

# Run the test and measure the time
/usr/bin/time -v ./test 

# Collect some statistics
sudo perf stat -e cycles,instructions,cache-references,cache-misses,faults,migrations ./test 
