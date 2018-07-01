argc=$#
if [ $argc -ne 3 ]
then echo "usage: $0 <start> <len> <file_name>"
else
	xxd -u -a -g 1 -c 16 -s $1 -l $2 $3
fi
