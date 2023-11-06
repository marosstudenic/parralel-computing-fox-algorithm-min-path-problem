rm -rf output/*

make && mpirun --allow-run-as-root -np 4 ./fox < basic-input.in