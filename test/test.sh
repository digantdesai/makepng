#!/bin/bash
dir=$(dirname "$(readlink -f "$0")")
bin=${dir}/../makepng
log=${dir}/test.log

rm -f $log

cd ..
make rebuild_debug &> $log

cd $dir

if [[ ! -e $bin ]]; then echo "Missing binary. Make sure \"$(readlink -f $bin)\" exists before running the tests"; exit 1; fi

# Encode
# 0 - original file
# 1 - generated png file
# validated.data
# Decode
# 2 - new original file
# Pass is 0 = 2 = validated.data

echo "" 2>&1 >> $log

for f0 in $(ls $dir/testdata* );
do
	f1=${f0}_1
	f2=${f0}_2
	fv=${dir}/validated.data

	# Delete validation data and other intermediate files before doing anything
	rm -f $fv $f1 $f2

	# Encode and Validate
	$bin -v -m encode -i $f0 -o $f1 2>&1 >> $log

	# Decode
	$bin -m decode -i $f1 -o $f2 2>&1 >> $log

	# Get all the check sums
	sum0=$(echo -n $(cat $f0) | shasum -a 256)
	sumv=$(echo -n $(cat $fv) | shasum -a 256)
	sum2=$(echo -n $(cat $f2) | shasum -a 256)

	# no sum1 since its the intermediate state
	if [[ "$sum0" == "$sumv" && "$sum0" == "$sum2" ]]; then
		echo $sum0 $f0 2>&1 >> $log
		echo $sumv $fv 2>&1 >> $log
		echo $sum2 $f2 2>&1 >> $log
		echo "Success: $f0" | tee -a  $log
		echo "" 2>&1 >> $log
	else
		echo "Failure: $f0"
		exit 1
	fi

	# Cleanup
	rm -f $fv $f1 $f2
done

exit 0;
