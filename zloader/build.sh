export LD_LIBRARY_PATH=/home/jc/ls2k1000-20240208/gcc-4.4.0-pmon/lib:$LD_LIBRARY_PATH
make tgt=rom CROSS_COMPILE=/home/jc/ls2k1000-20240208/gcc-4.4.0-pmon/bin/mipsel-linux-
if [ $? -ne 0 ]
then
	exit
fi
make dtb CROSS_COMPILE=/home/jc/ls2k1000-20240208/gcc-4.4.0-pmon/bin/mipsel-linux-
if [ $? -ne 0 ]
then
        exit
fi
/home/jc/ls2k1000-20240208/gcc-4.4.0-pmon/bin/mipsel-linux-readelf -a ../Targets/LS2K-hj20004/compile/ls2k/pmon > pmon.System
cp gzrom-dtb.bin /mnt/hgfs/share-win/
