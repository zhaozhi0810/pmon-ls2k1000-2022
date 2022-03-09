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

##将Targets目录下的相关目录拷贝一份
#判断字符串是否相等，与上面的=等价 
if [ "ls2k" == "${cputype}" ];then #ls2k
cp Targets/LS2K Targets/LS2K-${proID} -rf
cd Targets/LS2K-${proID}/conf
mv files.LS2K files.LS2K-${proID}
mv LS2K.dts LS2K-${proID}.dts
mv Makefile.LS2K Makefile.LS2K-${proID}

#替换字符串
sed -i "s/LS2K/LS2K-${proID}/g"  ls2k
sed -i "s/LS2K-${proID}_STR/LS2K_STR/g"  ls2k
sed -i "s/LS2K/LS2K-${proID}/g"  Makefile.LS2K-${proID}
sed -i "s/LS2K/LS2K-${proID}/g"  files.LS2K-${proID}

cd -

elif [ "3a3000_7a" == "${cputype}" ];then #ls3a3000
cp Targets/Bonito3a3000_7a Targets/Bonito3a3000_7a-${proID} -rf
cd Targets/Bonito3a3000_7a-${proID}/conf
mv files.Bonito3a3000_7a files.Bonito3a3000_7a-${proID}
mv Bonito3a3000_7a.dts Bonito3a3000_7a-${proID}.dts
mv Makefile.Bonito3a3000_7a Makefile.Bonito3a3000_7a-${proID}

#替换字符串
sed -i "s/Bonito3a3000_7a/Bonito3a3000_7a-${proID}/g"  Bonito
sed -i "s/Bonito3a3000_7a/Bonito3a3000_7a-${proID}/g"  Bonito.3a3000_7a
sed -i "s/Bonito3a3000_7a/Bonito3a3000_7a-${proID}/g"  Makefile.Bonito3a3000_7a-${proID}
sed -i "s/Bonito3a3000_7a/Bonito3a3000_7a-${proID}/g"  files.Bonito3a3000_7a-${proID}

cd -

else
echo "输入cputype: unknow"
exit 1;
fi


#:<<!
cd zloader  #进入zloader目录

#判断字符串是否相等，与上面的=等价 
if [ "ls2k" == "${cputype}" ];then #ls2k
echo "TARGET=LS2K-${proID}" > Makefile.${cputype}-${proID}
echo "TARGETEL=${cputype}" >> Makefile.${cputype}-${proID}
echo "export START=start.o" >> Makefile.${cputype}-${proID}
elif [ "3a3000_7a" == "${cputype}" ];then #ls3a3000
echo "TARGET=Bonito3a3000_7a-${proID}" > Makefile.${cputype}-${proID}
echo "TARGETEL=Bonito" >> Makefile.${cputype}-${proID}
echo "START=start.o" >> Makefile.${cputype}-${proID}
else
echo "输入cputype: unknow"
exit 1;
fi

echo "MEMSIZE=128" >> Makefile.${cputype}-${proID}
echo "ZLOADER_OPTIONS=-mips3" >> Makefile.${cputype}-${proID}
echo "" >> Makefile.${cputype}-${proID}
echo "include Makefile.inc" >> Makefile.${cputype}-${proID}

cd ..

ln -s zloader zloader.${cputype}-${proID}  #创建软链接

##将新建的项目名称写到文件里
echo "${cputype}-${proID}"  >> proID.txt

echo "已创建项目${cputype}-${proID}"
echo 
#!