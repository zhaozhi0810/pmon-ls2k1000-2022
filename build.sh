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
proID=$2 #设置项目编号

cd zloader.${cputype}-${proID} && \
make CROSS_COMPILE=mipsel-linux- clean && \
make CROSS_COMPILE=mipsel-linux- cfg && \
make CROSS_COMPILE=mipsel-linux- tgt=rom && \
make CROSS_COMPILE=mipsel-linux- dtb && \
sleep 2 && \
sync && \
cp gzrom.bin ${proID}-gzrom-${cputype}.bin && \
cp gzrom-dtb.bin ${proID}-gzrom-${cputype}-dtb.bin && \
cd -
