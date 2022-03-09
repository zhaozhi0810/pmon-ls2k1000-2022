#!/bin/bash
echo "已经创建的ID:"
cat proID.txt
echo 

if [ $# != 2 ] ; then  #判断输入参数是否2个
echo "输入格式: $0 <cputype> <proID>"
echo " 例如1: $0 ls2k ky18008"
echo " 例如2: $0 3a3000_7a ky18008"
exit 1;
fi

cputype=$1  #处理器型号
proID=$2 	#设置项目编号

cd zloader.${cputype}-${proID} && \
make CROSS_COMPILE=mipsel-linux- clean && \
cd ..

##删除Targets目录下的相关目录
#判断字符串是否相等，与上面的=等价 
if [ "ls2k" == "${cputype}" ];then #ls2k
rm Targets/LS2K-${proID} -rf
elif [ "3a3000_7a" == "${cputype}" ];then #ls3a3000
rm Targets/Bonito3a3000_7a-${proID} -rf
else
echo "输入cputype: unknow"
exit 1;
fi


#:<<!

rm zloader/Makefile.${cputype}-${proID} #删除Makefile
rm zloader/${proID}-gzrom-${cputype}.bin 
rm zloader/${proID}-gzrom-${cputype}-dtb.bin 
rm zloader.${cputype}-${proID}  #删除软链接

##将项目名称从文件里删除
sed -i "/${cputype}-${proID}/d" proID.txt

echo "已删除项目${cputype}-${proID}"
echo 
#!