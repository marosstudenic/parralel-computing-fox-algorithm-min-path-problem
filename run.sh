rm -rf output/*

make && mpirun --hostfile hostfile --allow-run-as-root -np 9 ./fox < tests/inputs/input300 > output/fox.out


# test output agains expected output in tests/outputs/output300
diff output/fox.out tests/outputs/output300 > /dev/null
# write ok if no difference
if [ $? -eq 0 ]
then
    echo "nic sa nepokazilo"
else
    echo "fuck it"
fi