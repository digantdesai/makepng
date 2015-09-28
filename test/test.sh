#!/bin/bash
dir=$(dirname "$(readlink -f "$0")")
bin=$(readlink -f ${dir}/../makepng)
log=$(readlink -f ${dir}/test.log)

rm -f $log
touch $log

cd $dir/..

echo "Testing begin." | tee $log
echo -n "Building application.." | tee -a $log
make rebuild_debug >> $log

cd $dir
if [[ $? -ne 0 ]]; then
    echo "failed, exiting." | tee -a  $log
    exit 1
else
    echo "succeeded." | tee -a  $log
fi
echo "" | tee -a $log

# Generate temp test files in $dir starting with testdata
echo -n "Generating random test files..." | tee -a  $log
touch $dir/testdata_a; echo "" > $dir/testdata_a
for i in $(seq 1 10); do echo dmesg >> $dir/testdata_a; done

touch $dir/testdata_b; echo "" > $dir/testdata_b
for i in $(seq 1 200); do cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 10 | head -n 1 >> $dir/testdata_b; done

touch $dir/testdata_c; echo "" > $dir/testdata_c
for i in $(seq 1 20); do cat $dir/testdata_b  >> $dir/testdata_c; done

touch $dir/testdata_d; echo "" > $dir/testdata_d
cp /vmlinuz-linux $dir/testdata_d

echo "done." | tee -a  $log
echo "" | tee -a $log

# Encode
# 0 - original file
# 1 - generated png file
# validated.data
# Decode
# 2 - new original file
# Pass is 0 = 2 = validated.data

echo "" >> $log

echo "Starting tests.." | tee -a  $log
for f0 in $(ls $dir/testdata* );
do
    echo "New testfile: $f0" | tee -a  $log
    echo "" >> $log

    f1=${f0}_1
    f2=${f0}_2
    fv=${dir}/validated.data

    # Delete validation data and other intermediate files before doing anything
    rm -f $fv $f1 $f2

    # Encode and Validate
    echo -n "Encoding test.." | tee -a  $log
    $bin -v -m encode -i $f0 -o $f1 >> $log
    echo "Done." | tee -a $log

    echo "" 2>&1 >> $log
    # Decode
    echo -n "Decoding test.." | tee -a  $log
    $bin -m decode -i $f1 -o $f2 >> $log
    echo "Done." | tee -a $log

    wait &&

    # Get all the check sums
    sum0=$(echo -n $(cat $f0) | sha1sum)
    sumv=$(echo -n $(cat $fv) | sha1sum)
    sum2=$(echo -n $(cat $f2) | sha1sum)

    echo "" 2>&1 >> $log
    # no sum1 since its the intermediate state
    if [[ "$sum0" == "$sumv" && "$sum0" == "$sum2" ]]; then
        echo $sum0 $f0 >> $log
        echo $sumv $fv >> $log
        echo $sum2 $f2 >> $log
        echo "Success !!!" | tee -a  $log
        echo "" | tee -a $log
    else
        echo "Failure !!!" | tee -a  $log
        echo "" | tee -a $log
    fi

    # Cleanup
    rm -f $fv $f1 $f2
done


# remove newly created temp files
rm -f $dir/testdata_a $dir/testdata_b $dir/testdata_c $dir/testdata_d

echo "Testing complete." | tee -a  $log
exit 0;
