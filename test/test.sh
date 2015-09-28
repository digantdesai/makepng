#!/bin/bash
dir=$(dirname "$(readlink -f "$0")")
bin=${dir}/../makepng
log=${dir}/test.log

rm -f $log

cd $dir/..
echo -n "Building application.." | tee -a $log
make rebuild_debug &> $log

cd $dir

if [[ $? -ne 0 ]]; then
    echo "failed, exiting." | tee -a  $log
    exit 1
else
    echo "succeeded." | tee -a  $log
fi

# Generate temp test files in $dir starting with testdata
touch $dir/testdata_a
for i in $(seq 1 10); do echo dmesg >> $dir/testdata_a; done

touch $dir/testdata_b
for i in $(seq 1 100); do cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 10 | head -n 1 >> $dir/testdata_b; done

touch $dir/testdata_c
cp /vmlinuz-linux $dir/testdata_c

echo "Generating random test files: a, b, c..done." | tee -a  $log

# Encode
# 0 - original file
# 1 - generated png file
# validated.data
# Decode
# 2 - new original file
# Pass is 0 = 2 = validated.data

echo "" 2>&1 >> $log

echo "Starting tests.." | tee -a  $log
for f0 in $(ls $dir/testdata* );
do
    echo "New testfile: $f0" | tee -a  $log
    echo "" 2>&1 >> $log

    f1=${f0}_1
    f2=${f0}_2
    fv=${dir}/validated.data

    # Delete validation data and other intermediate files before doing anything
    rm -f $fv $f1 $f2

    # Encode and Validate
    echo "Encoding test.." | tee -a  $log
    $bin -v -m encode -i $f0 -o $f1 2>&1 >> $log

    echo "" 2>&1 >> $log
    # Decode
    echo "Decoding test.." | tee -a  $log
    $bin -m decode -i $f1 -o $f2 2>&1 >> $log

    wait &&

    # Get all the check sums
    sum0=$(echo -n $(cat $f0) | shasum -a 256)
    sumv=$(echo -n $(cat $fv) | shasum -a 256)
    sum2=$(echo -n $(cat $f2) | shasum -a 256)

    echo "" 2>&1 >> $log
    # no sum1 since its the intermediate state
    if [[ "$sum0" == "$sumv" && "$sum0" == "$sum2" ]]; then
        echo $sum0 $f0 2>&1 >> $log
        echo $sumv $fv 2>&1 >> $log
        echo $sum2 $f2 2>&1 >> $log
        echo "Success !!!" | tee -a  $log
        echo "" 2>&1 >> $log
    else
        echo "Failure !!!" | tee -a  $log
    fi

    # Cleanup
    rm -f $fv $f1 $f2
done


# remove newly created temp files
rm -f $dir/testdata_a $dir/testdata_b $dir/testdata_c

echo "Testing complete." | tee -a  $log
exit 0;
