#! /bin/bash
# 统计文件的空行数

echo -n "filename: "
read exname

files=`find . -name "$exname"`
total=0 # 总空行数
x=0 # 每个文件的空行数

for file in $files
do
    x=`awk 'BEGIN{x=0}/^$/{x++}END{print x}' $file`
    echo -e " $x\t$file"
    let total+=x
done

echo "total: $total"
