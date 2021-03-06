"""
python pmonenv.py -f gzrom.bin -o 0x70000 -s 512 al=/dev/mtd0 append="'root=/dev/mtdblock0'"
python ../tools/pmonenv.py -f gzrom-dtb.bin -d ls2k.dtb -w  al=/dev/ram@p0x110000000 al1=/dev/ram@p0x110000000 append="'console=ttyS0,115200 initcall_debug=1 loglevel=20 nosmp'" FR=1
"""
import struct
import sys
import getopt
def readenv(fname,foff,fsz,argv):
    f=open(fname,'rb+') # 使用二进制的方式打开文件
    f.seek(foff,0)  # 设置文件偏移地址 从文件开始位置偏移foff
    a=f.read(fsz);  # 读取文件的内容，长度fsz
    a=a.ljust(fsz,b'\x00')  # a的内容左对齐，长度为fsz，不够的地方使用"\0"填充
    f.close()  # 关闭文件
    d={}
    b = struct.unpack('!'+str(len(a)//2)+'H',a) # 按照参数1的格式解包为结构体成员 !表示网络字节序，H表示short类型2字节，len(a)/2,表示总字节数除2
    if(sum(b)&0xffff):  # 校验和不为0 出错
     print('checksum error, rebuild env')
     t = argv
    else:
     e = a[2:].find(b'\x00\x00')  # 找到第二个字节开始，出现两个‘\0’的地方
     t = a[2:2+e].split(b'\x00')+list(map(lambda x:x.encode('utf8'),argv))
    for i in t:
       a=i.split(b'=',1) # 找到等号分开 
       if len(a) > 1:    # 多余一项就是设置值
         d[a[0]] = a[1]   # 内容写到字典中，
       elif a[0] in d:   # 只有一项就是要删除这个值
         del d[a[0]]
    return d

def writeenv(fname,foff,fsz,d):
    
    a=b'\x00\x00'  # a 赋值两个‘\0’
    for i in d.keys():   # i 是key
     a += i+b'='+d[i]+b'\x00' # key = ... '\0'
    a=a.ljust(fsz,b'\x00')
    
    b = struct.pack('!H',(-sum(struct.unpack('!'+str(len(a)//2)+'H',a)))&0xffff)
    a=b+a[2:]
    
    f=open(fname,'rb+')
    f.seek(foff,0)
    f.write(a)
    f.close()

def writehexenv(fname,hexbin):
    f=open(fname,'rb+')
    f.seek(foff+fsz, 0)
    f.write('\xff'*256)
    for b in hexbin.split(','):
      i,v = b.split(':')
      f.seek(foff+int(i,0),0)
      f.write(v.decode('hex'))
    f.close()

def writedtb(fname,dtb,foff):
    f=open(dtb,'rb')
    a=f.read();
    f.close()
    a=a.ljust(0x4000-8,'\x00')
    b = struct.pack('I',(-sum(struct.unpack(''+str(len(a)//4)+'I',a)))&0xffffffff)
    a=b+a+b
    
    f=open(fname,'rb+')
    f.seek(foff-0x4000,0)
    f.write(a)
    f.close()
    
if __name__ == '__main__':
    opt,argv=getopt.getopt(sys.argv[1:],'b:o:s:f:wd:')
    opt=dict(opt)
    foff = int(opt['-o'],0) if '-o' in opt  else 0x000ff000
    fsz = int(opt['-s'],0) if '-s' in opt else 500
    fname = opt['-f'] if '-f' in opt else 'gzrom.bin'
    
    d=readenv(fname,foff,fsz,argv)
    print(d)
    if '-w' in opt:
     writeenv(fname,foff,fsz,d)
     if '-b' in opt: writehexenv(fname, opt['-b'])
     if '-d' in opt: writedtb(fname, opt['-d'], foff)
           

