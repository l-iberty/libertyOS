if [ $# -ne 2 ]
then echo "usage: $0 <filename> <ch>"
else
    cat $1 | sed -e "s/\([ABC]\)/\1\n/g" | grep $2 |wc -l
fi
