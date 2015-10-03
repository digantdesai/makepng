#!/bin/bash -e
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#0. compile the application
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
make rebuild > /dev/null # Release compile
#make rebuild_debug # for a debug build

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#1. encode the file and generate a png file
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#1.1 input file: Simple encoding demo with a small text file.
inputfile="test/testdata_input_short.txt"

#1.2 output png file
output_png_file="/tmp/my_output.png"

#1.3 optional metadata, for example sha1sum of the input data
metadata="sha1sum=$(cat $inputfile | sha1sum | sed -r 's/([0-9a-zA-Z]*).*/\1/g')"

#1.4 lets encode
echo "1. Encoding and adding meta data:"
echo "$metadata"
./makepng -e -i $inputfile -o $output_png_file -m $metadata

# run with -v for optional validation
#./makpng -e -i $inputfile -o $output_png_file -m $metadata -v
# run without -m or -v
#./makpng -e -i $inputfile -o $output_png_file

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#2. decoding the png, getting the original file back
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#2.1 output file extracted from the png file (should be = inputfile)
output_orig_file="/tmp/my_file.orig"

#2.2 lets decode
echo ""
echo "2. Decoding and retrieved meta data from the image: "
./makepng -d -i $output_png_file -o $output_orig_file -p
# printing meta data is optional, run without -p
#./makepng -d -i $output_png_file -o $output_orig_file

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#3. did it really work??
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo ""
echo "3. Validation:"

echo -n "Sha1sum original  : "
echo $(sha1sum $inputfile)
echo -n "Sha1sum extracted : "
echo $(sha1sum $output_orig_file)

echo -n "Original file : "
cat $inputfile
echo -n "Extracted file: "
cat $output_orig_file

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#4. cleanup
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
rm -rf $output_png_file
rm -rf $output_orig_file

make clean > /dev/null
