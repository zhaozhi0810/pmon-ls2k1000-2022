#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*

 	2024-03-02  by dazhi
	特别注意： ls2k1000 的pmon 不能大于1m ，因为它映射的空间就是1m以内

	env的起始位置0x1fcff000,大小498字节以内，还有2个字节的校验和，最大不能超过500字节
	dtb的起始位置0x1fcfb000,大小16k-8字节，8个字节留出来做校验和，最大不能超过16k
	gzrom的起始位置0x1fc00000，最大1004k字节。

	选项
	-o gzrom-dtb-new.bin 读出flash中的程序（1m以内）
	-e env.bin 写入env数据：要求有校验和+正确的格式
	-d dtb.bin  写入dtb
	-c gzrom-dtb.bin  比较flash中的与文件是否相同
	gzrom-dtb.bin 直接写入gzrom-dtb.bin 

 */


#define BIN_FILE_SIZE 1044980   //这是编译的gzrom-dtb.bin的大小



//extern int spiflash_main(char *cmd, unsigned long flashbase, unsigned long spiregbase, unsigned long spimem_base_map, ...);
//off 就是flash内部的偏移地址
//extern int fl_erase_device(int off, int size, int verbose);

//off 就是flash内部的偏移地址
//extern int fl_program_device(int off, void *data_base, int data_size, int verbose);

extern int set_spiflash_regaddr(unsigned long long spireg_base,void* spimem_base);
extern void tgt_flashprogram(int p, int size, void *s);
// off 是flash中的偏移地址，起始地址0
// data_base 是文件内容的缓存起始地址
// data_size 是需要比较的数据大小
// verbose 是否打印信息
extern int fl_verify_device(int off, void *data_base, int data_size, int verbose);


int PAGE_SIZE;
int PAGE_MASK;

#define FLASH_SIZE 0x500000



void printf_usage(char* name)
{
	printf("USAGE:\n");
	printf("%s [-o gzrom-dtb-new.bin] : read flash(1M) to  file gzrom-dtb-new.bin\n",name);
	printf("%s <-e env.bin> : write env.bin to flash offset 0xff000,size 500Bytes\n",name);
	printf("%s <-d dtb.bin> : write dtb.bin to flash offset 0xfb000,size 16KBytes\n",name);
	printf("%s <-c gzrom-dtb.bin> : compare flash(ahout 600K) and file gzrom-dtb.bin,the same or not\n",name);
	printf("%s <-w gzrom-dtb.bin> : write gzrom-dtb.bin to flash offset 0,size 1044980Bytes\n",name);
	printf("%s <gzrom-dtb.bin> : the same with -w\n",name);
	printf("others ,not support!!!!\n");
}





unsigned int flash_buf[FLASH_SIZE];
int main(int argc, char **argv)
{
	int fd,mem_fd,spimem_physaddr,spimem_map_len,spireg_physaddr,spireg_map_len,spireg_offset;
	void *spimem_base = NULL,*spireg_base= NULL,*buf=NULL;
	int i;
	int err;
	unsigned char* pbuf;
	int option = 0;   //0表示读出来
	char* filename = "gzrom-dtb-new.bin";   //文件名
	struct stat statbuf;


	if(argc > 3) //参数多余3个
	{
		printf_usage(argv[0]);
		return -1;
	}
	else if(argc == 2)
	{
		printf("len = %d\n",strlen(argv[1]));
		if(strlen(argv[1]) > 8)
		{
			printf("name = %s ,%s\n",argv[1],argv[1]+(strlen(argv[1])-4));
			if(strcmp(".bin",argv[1]+(strlen(argv[1])-4)) != 0)  //是.bin 结尾的吗
			{
				printf_usage(argv[0]);
				return -1;
			}
		}
		else
		{
			printf_usage(argv[0]);
			return -1;
		}

		option = 4;  //write gzrom-dtb ro flash
		filename = argv[1];  //保存文件名；
		
	}
	else if(argc == 1)
	{
		option = 0;
	}
	else  //argc == 3
	{
		filename = argv[2];  //保存文件名；
		if(strcmp(argv[1], "-o")==0)
		{
			option = 0;
	
		}
		else if(strcmp(argv[1], "-e")==0)
		{
			option = 1;  //写环境变量 env

		}
		else if(strcmp(argv[1], "-d")==0)
		{
			option = 2;  //写dtb文件 

		}
		else if(strcmp(argv[1], "-c")==0)
		{
			option = 3;  //比较文件

		}
		else if(strcmp(argv[1], "-w")==0)
		{
			option = 4;  //write gzrom-dtb ro flash	
		}
		else{  //其他不能识别
			printf_usage(argv[0]);
			return -1;
		} 
	}

	printf("opt = %d filename = %s\n",option,filename);


    spimem_physaddr = 0x1fc00000;   //0x1d000000; //3a5000的地址   //0x1fc00000 2k1000的地址
    spimem_map_len = 0x100000;//1M


    PAGE_SIZE = getpagesize();
    PAGE_MASK = (~(PAGE_SIZE-1));


	mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
    if(mem_fd == -1)
        error(-1,errno,"error open /dev/mem\n");

    //spi 内存读写方式
    spimem_base = mmap(0, spimem_map_len, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, spimem_physaddr&PAGE_MASK);
    if(spimem_base == MAP_FAILED)
        error(err,errno,"spimem_base map failed.\n");


    spireg_physaddr = 0x1fff0220;//0x1fe001f0;    //0x1fff0220 2k1000的地址
    spireg_map_len = 0x1000; //4K
    spireg_offset = spireg_physaddr & (PAGE_SIZE-1);   //偏移地址

	spireg_base = (unsigned char*)mmap(NULL, spireg_map_len, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, spireg_physaddr&PAGE_MASK);
    if(spireg_base == MAP_FAILED){
        error(err,errno,"spireg_base map failed.\n");
        return -1;
    }



    close(mem_fd);

    pbuf = spimem_base;

    //printf("spireg_base = %p\n",spireg_base);
    // 需要设置一下才能用啊。。。。
	set_spiflash_regaddr((unsigned long long) spireg_base + spireg_offset,spimem_base);

    // for(i=0;i<100;i++)
    // {
    //     printf("%02hhx ",pbuf[i]);
    //     if(i%16 ==15)
    //         printf("\n");
    // }

    //是读操作
    if(option == 0)  //读出flash的内容，大小BIN_FILE_SIZE个字节
    {
    	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,0666);
		if(fd == -1)
		    error(err,errno,"error open file.\n");

		int ret =	write(fd,pbuf,BIN_FILE_SIZE);  //一次性写进去
		printf("read size = %d\n",ret);

		close(fd);
    }
    else if(option == 1) //写环境变量 env
    {
    	//1 打开env。bin文件，限制文件不能大于500字节
    	fd = open(filename, O_RDONLY);
	    if(fd == -1)
	        error(err,errno,"error open file. %s\n",filename);
    	
	    int size = lseek(fd,0,SEEK_END);  //长度
	    if(size > 500)
	    {
	    	printf("file size too large (len=%d > 500Bytes)\n",size);
	    	munmap(spireg_base,spireg_map_len);
	    	munmap(spimem_base,spimem_map_len);  
	    	close(fd);
	    	return -1;
	    }

	    lseek(fd,0,SEEK_SET);  //

		buf = mmap(0, 500, PROT_READ, MAP_SHARED, fd, 0);
		if(buf == MAP_FAILED)
			error(err,errno,"map failed.%s\n",filename);

		// 第一个参数是 flash中的地址，从0开始
		// 第二个参数是 写入字节数
		// 第三个参数需要写入内容缓存的起始地址
		tgt_flashprogram(0xff000, size, buf);

		close(fd);
		//取消映射
		munmap(buf,500);

		printf("erase and program down\n");
    }
    else if(option == 4) //write gzrom-dtb ro flash
    {
    	if(strncmp("gzrom",filename,5) != 0 )
    	{
    		printf_usage(argv[0]);
    		munmap(spireg_base,spireg_map_len);
	    	munmap(spimem_base,spimem_map_len);
			return -1;
    	}


    	//1 打开gzrom-dtb.bin文件，限制文件大小
    	fd = open(filename, O_RDONLY);
	    if(fd == -1)
	        error(err,errno,"error open file. %s\n",filename);
    	
	    int size = lseek(fd,0,SEEK_END);  //长度
	    if(size != BIN_FILE_SIZE)
	    {
	    	printf("file size !=  %d\n",size,BIN_FILE_SIZE);
	    	munmap(spireg_base,spireg_map_len);
	    	munmap(spimem_base,spimem_map_len);  
	    	close(fd);
	    	return -1;
	    }

	    lseek(fd,0,SEEK_SET);  //

		buf = mmap(0, BIN_FILE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
		if(buf == MAP_FAILED)
			error(err,errno,"map failed.%s\n",filename);

		// 第一个参数是 flash中的地址，从0开始
		// 第二个参数是 写入字节数
		// 第三个参数需要写入内容缓存的起始地址
		tgt_flashprogram(0, BIN_FILE_SIZE, buf);   //起始地址是0
		printf("program %s done\n",filename);

		close(fd);
		//取消映射
		munmap(buf,BIN_FILE_SIZE);

		//printf("erase and program down\n");
    }


    else if(option == 3)  //比较文件，只比较前面0~0xfb000,后面的dtb没有必要比较
    {
    	if(strncmp("gzrom",filename,5) != 0 )
    	{
    		printf_usage(argv[0]);
    		munmap(spireg_base,spireg_map_len);
	    	munmap(spimem_base,spimem_map_len);
			return -1;
    	}


    	//1 打开gzrom-dtb.bin文件，限制文件大小
    	fd = open(filename, O_RDONLY);
	    if(fd == -1)
	        error(err,errno,"error open file. %s\n",filename);
    	
	    int size = lseek(fd,0,SEEK_END);  //长度
	    if(size != BIN_FILE_SIZE)
	    {
	    	printf("file size !=  %d\n",size,BIN_FILE_SIZE);
	    	munmap(spireg_base,spireg_map_len);
	    	munmap(spimem_base,spimem_map_len);  
	    	close(fd);
	    	return -1;
	    }

	    lseek(fd,0,SEEK_SET);  //

		buf = mmap(0, BIN_FILE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
		if(buf == MAP_FAILED)
			error(err,errno,"map failed.%s\n",filename);


		int ret = fl_verify_device(0, buf, 0xfb000, 1);

		printf("compare %s done\n",filename);
		if(ret == 1)
			printf("file in system is the same with the file %s\n",filename);
		else if(ret == 0)
			printf("you can update this %s now!!!\n",filename);

		close(fd);
		//取消映射
		munmap(buf,BIN_FILE_SIZE);
    }



//     int fd,mem_fd,physaddr,spimem_physaddr,spireg_physaddr;
//     void *mem = NULL, *spimem_base = NULL;
//     unsigned char *spireg_base;
//     int err,i,map_len,len,spimem_map_len,spireg_map_len;
//     char *filename = NULL, *buf = NULL, *devname = "/dev/mem";
//     struct stat statbuf;
//     if(argc < 2)
//         goto usage;
	
//     filename = argv[1];
    
//     PAGE_SIZE = getpagesize();
//     PAGE_MASK = (~(PAGE_SIZE-1));
    
//     physaddr = 0x1fc00000;
//     map_len = 0x100000;//1M
	
//     spimem_physaddr = 0x1fc00000;   //0x1d000000; //3a5000的地址   //0x1fc00000 2k1000的地址
//     spimem_map_len = 0x1000000;//16M
	
//     spireg_physaddr = 0x1fff0220;//0x1fe001f0;    //0x1fff0220 2k1000的地址
//     spireg_map_len = 0x1000; //4K
	
//     mem_fd = open(devname, O_RDWR|O_SYNC);
//     if(mem_fd == -1)
//         error(-1,errno,"error open /dev/mem\n");
	
//     mem = mmap(0, map_len, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, physaddr&PAGE_MASK);
//     if(mem == MAP_FAILED)
//         error(err,errno,"mem map failed.\n");
	
//     spimem_base = mmap(0, spimem_map_len, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, spimem_physaddr&PAGE_MASK);
//     if(spimem_base == MAP_FAILED)
//         error(err,errno,"spimem_base map failed.\n");
	
//     spireg_base = (unsigned char*)mmap(NULL, spireg_map_len, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, spireg_physaddr&PAGE_MASK);
//     if(spireg_base == MAP_FAILED)
//         error(err,errno,"spireg_base map failed.\n");
    
// 	close(mem_fd);
	
//     printf("file name is: %s\n",filename);
//     if((err = stat(filename, &statbuf)) != 0)
//         error(err,errno,"error find file.\n");
    
// 	if(statbuf.st_size > FLASH_SIZE){
//         error(-1,0,"File size is too large.\n");
//         return -1;
//     }
	
//     fd = open(filename, O_RDONLY);
//     if(fd == -1)
//         error(err,errno,"error open file.\n");
	
//     buf = mmap(0, map_len, PROT_READ, MAP_SHARED, fd, 0);
//     if(buf == MAP_FAILED)
//         error(err,errno,"map failed.\n");
	
//     close(fd);
	
//     printf("file size is: %ld KB\n",(long)((statbuf.st_size)/1024));
//     //spiflash_main("iep",(unsigned long)mem, (unsigned long)(spireg_base+0x1f0),(unsigned long)spimem_base, 0, (unsigned long)statbuf.st_size, 0x1000, buf, 0, (unsigned long)statbuf.st_size);
	
// 	spiflash_main("i", (unsigned long)mem, (unsigned long)(spireg_base+0x220), (unsigned long)spimem_base, 0, (unsigned long)statbuf.st_size, 0x1000, buf, 0, (unsigned long)statbuf.st_size);

//     //memcpy(flash_buf,mem,statbuf.st_size);
//     memcpy(flash_buf, spimem_base, statbuf.st_size);
	
//     for(i=0;i<100;i++)
//     {
//         printf("%#hhx ",flash_buf[i]);
//         if(i%10 ==9)
//             printf("\n");
//     }

//     for(i=0;i<100;i++)
//     {
//         printf("%#hhx ",buf[i]);
//         if(i%10 ==9)
//             printf("\n");
//     }
    
//     if (memcmp(flash_buf, buf, statbuf.st_size))
//     {
// 	    printf("verify miscompare,exit flash program,please retry!!\r\n");
// 	    //exit(0);
//     }
// 	else
//     {

// 	    printf("verify mach OK, you can reboot!!\r\n");
//     }
	
//     munmap(mem,map_len);
//     munmap(spimem_base,spimem_map_len);
//     munmap(spireg_base,spireg_map_len);
//     munmap(buf,map_len);  
	
//     return 0;
	
// usage:
//     printf("usage: proparm filename.\n");
//     return -1;
    munmap(spireg_base,spireg_map_len);
	munmap(spimem_base,spimem_map_len);     
    return 0;
}
