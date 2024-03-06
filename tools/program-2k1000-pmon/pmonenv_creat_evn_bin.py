"""
python pmonenv.py -f gzrom.bin -o 0x70000 -s 512 al=/dev/mtd0 append="'root=/dev/mtdblock0'"
python ../tools/pmonenv.py -f gzrom-dtb.bin -d ls2k.dtb -w  al=/dev/ram@p0x110000000 al1=/dev/ram@p0x110000000 append="'console=ttyS0,115200 initcall_debug=1 loglevel=20 nosmp'" FR=1
"""

# cmd = {"al=/dev/ram@p0x110000000", "al1=/dev/ram@p0x110000000", "append='console=ttyS0,115200 initcall_debug=1 loglevel=20 nosmp' FR=1"}
cmd = {"al=(usb0,0)/boot/vmlinuz", "al1=(wd0,0)/vmlinuz", "append='console=ttyS0,115200 noinitrd root=/dev/sda2  rootfstype=ext4 rootwait rw'", "FR=1","author=zhaodazhi"}


import struct
import sys
import getopt
def readenv(argv): 
	d={}
	t = argv

	print(argv)
	for i in t:
		# print("i=",i)
		a=i.split('=',1)  
		# print("6 a = ",a)   # 7
		if len(a) > 1:     
			d[a[0]] = a[1]   
		elif a[0] in d:   
			del d[a[0]]
	# print("for end*******************\n")
	return d



# 2024-03-04
def creat_env_bin(fname,fsz,d):
	a=b'\x00\x00'   
	for i in d.keys():   
		a += i+b'='+d[i]+b'\x00' 
		# print("8 a = ",a)   # 9
	a += b'\x00'
	a=a.ljust(fsz,b'\xff')   
	b = struct.pack('!H',(-sum(struct.unpack('!'+str(len(a)//2)+'H',a)))&0xffff)
	# print("9 b = ",b)   # 10
	a=b+a[2:]
	# print("10 a = ",a)   # 11
	f=open(fname,'wb+')
	# f.seek(foff,0)
	f.write(a)  
	f.close()


    
if __name__ == '__main__':
	d=readenv(cmd)
	print(d) 
	creat_env_bin('env.bin',500,d)   

           

