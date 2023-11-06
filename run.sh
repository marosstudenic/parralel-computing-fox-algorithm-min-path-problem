rm -rf output/*

make && mpirun --allow-run-as-root -np 4 ./fox < tests/inputs/input300


# test output agains expected output in tests/outputs/output300
diff output/proc-0.out tests/outputs/output300 > /dev/null
# write ok if no difference
if [ $? -eq 0 ]
then
    echo "nic sa nepokazilo"
else
    echo "fuck it"
fi