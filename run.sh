rm -rf output/*

make && mpirun --allow-run-as-root -np 4 ./fox < tests/inputs/input1200 > output/fox.out


# test output agains expected output in tests/outputs/output300
diff output/fox.out tests/outputs/output1200 > /dev/null
# write ok if no difference
if [ $? -eq 0 ]
then
    echo "nic sa nepokazilo"
else
    echo "fuck it"
fi