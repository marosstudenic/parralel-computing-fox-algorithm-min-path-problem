rm -rf output/*

make && mpirun --allow-run-as-root -np 4 ./fox < tests/inputs/input300 > output/fox.out