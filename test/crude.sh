dir=$(dirname "$(readlink -f "$0")")
bin=$(readlink -f ${dir}/../makepng)

for i in $(ls /bin); do
	echo $(ls -ahs /bin/$i)
	\time -f "encode: %e sec" $bin -i /bin/$i -o /tmp/$i.png -e
	\time -f "decode: %e sec" $bin -d -i /tmp/$i.png -o /tmp/$i.orig

    sum0=$(echo -n $(cat /tmp/$i.orig) | sha1sum)
    sum2=$(echo -n $(cat /bin/$i) | sha1sum)

    if [[ "$sum0" == "$sum2" ]]; then
        tput setaf 34; echo "Success !!!"; tput sgr0;
    else
        tput setaf 1; echo "Failure !!!"; tput sgr1;
    fi

    rm /tmp/$i.png /tmp/$i.orig
done
