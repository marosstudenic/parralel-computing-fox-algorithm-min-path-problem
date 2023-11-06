rm -rf output/*

make && mpirun --allow-run-as-root -np 4 ./fox < tests/inputs/input300 > output/fox.out


# test output agains expected output in tests/outputs/output300
diff output/fox.out tests/outputs/output300 > /dev/null
# write ok if no difference
if [ $? -eq 0 ]
then
    echo "test sucess"
else
    echo "test failed"
fi