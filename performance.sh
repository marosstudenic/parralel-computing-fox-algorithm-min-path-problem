#!/bin/bash

# Compile the floyd-warshal executable
gcc floyd-warshal.c -o floyd-warshal
make

# Array of input file sizes
inputs=("6" "300" "600" "900" "1200")

# Function to run tests for floyd-warshal
run_floyd_warshal_tests() {
  for input in "${inputs[@]}"; do
    echo "Running floyd-warshal with input size ${input}"
    output_file="tests/outputs/output${input}"
    test_output="/tmp/output${input}"
    /usr/bin/time -f "Execution time: %E" ./floyd-warshal < "tests/inputs/input${input}" > "${test_output}"
    if diff "${output_file}" "${test_output}" > /dev/null; then
      echo "Test passed: Output matches for input size ${input}"
    else
      echo "Test failed: Output does not match for input size ${input}"
    fi
    rm "${test_output}"  # Clean up temporary output file
  done
}

# Function to run tests for fox executable with different number of processes
run_fox_tests() {
  for procs in 16 9 4 1; do
    for input in "${inputs[@]}"; do
      echo "Running fox with ${procs} processes and input size ${input}"
      output_file="tests/outputs/output${input}"
      test_output="/tmp/output${input}_${procs}"
      /usr/bin/time -f "Execution time: %E" mpirun --hostfile hostfile --allow-run-as-root -np ${procs} ./fox < "tests/inputs/input${input}" > "${test_output}"
      if diff "${output_file}" "${test_output}" > /dev/null; then
        echo "Test passed: Output matches for ${procs} processes and input size ${input}"
      else
        echo "Test failed: Output does not match for ${procs} processes and input size ${input}"
      fi
      rm "${test_output}"  # Clean up temporary output file
    done
  done
}

# Run the tests for floyd-warshal
run_floyd_warshal_tests

# Run the tests for fox
run_fox_tests