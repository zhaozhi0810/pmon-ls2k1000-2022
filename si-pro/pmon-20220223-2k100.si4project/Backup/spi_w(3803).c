#include <stdio.h>
//#include "include/fcr.h"
#include <stdlib.h>
#include <ctype.h>
#undef _KERNEL
#include <errno.h>
#include <pmon.h>
//#include <include/types.h>
#include <pflash.h>

#define SPI_BASE  0x1fff0220
#define PMON_ADDR 0xa1000000
#define FLASH_ADDR 0x000000

#define SPCR      0x0
#define SPSR      0x1
#define TXFIFO    0x2
#define RXFIFO    0x2
#define SPER      0x3
#define PARAM     0x4
#define SOFTCS    0x5
#define PARAM2    0x6

#define WFFULL (1<<3)   //发送缓存满
#define RFEMPTY 1
#define KSEG1_STORE8(addr,val)	 *(volatile char *)(0xffffffffa0000000 | addr) = val
#define KSEG1_LOAD8(addr)	 *(volatile char *)(0xffffffffa0000000 | addr) 

#define SET_SPI(addr,val)        KSEG1_STORE8(SPI_BASE+addr,val)
#define GET_SPI(addr)            KSEG1_LOAD8(SPI_BASE+addr)
#define NEW_SPI_ZZ


int write_sr(char val);
void spi_initw()
{ 
  	SET_SPI(SPSR, 0xc0);   
  	SET_SPI(PARAM, 0x40);    //这里没有读使能了         //espr:0100
 	SET_SPI(SPER, 0x05); //spre:01 
  	SET_SPI(PARAM2,0x01); 
  	SET_SPI(SPCR, 0x50);
}

void spi_initr()
{
  	SET_SPI(PARAM, 0x47);             //espr:0100
}


#ifdef NEW_SPI_ZZ

//发送数据，需配合写使能，片选操作。
static unsigned char spi_send_byte(unsigned char val)
{	
	while((GET_SPI(SPSR))&WFFULL);  //发送缓存满！！，等待
	SET_SPI(TXFIFO,val);
	while((GET_SPI(SPSR))&RFEMPTY); //等待发送结束
	return GET_SPI(RXFIFO); //读缓存
}
#endif


///////////////////read status reg /////////////////

int read_sr(void)
{
	int val;
	
	SET_SPI(SOFTCS,0x01);  //设置片选

#ifdef NEW_SPI_ZZ
	spi_send_byte(0x05);
	val = spi_send_byte(0x00);
#else
	SET_SPI(TXFIFO,0x05);  //发送命令
	while((GET_SPI(SPSR))&RFEMPTY);  //等待发送结束

	val = GET_SPI(RXFIFO);  //读缓存
	SET_SPI(TXFIFO,0x00);	//写数据0，是为了读取一个数据

	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY); //等待发送结束

	val = GET_SPI(RXFIFO); //读缓存
#endif	
	SET_SPI(SOFTCS,0x11);  //取消片选
      
	return val;
}

#ifdef NEW_SPI_ZZ
static void spi_flash_check_busy(void)
{
	int res;

	do{
		res = read_sr();  //读flash状态寄存器
	}while((res&0x01) == 1);  //忙则继续等
}
#endif


////////////set write enable//////////
int set_wren(void)
{
	int res;

#ifdef NEW_SPI_ZZ
	//spi_flash_check_busy();
	SET_SPI(SOFTCS,0x01);  //片选
	spi_send_byte(0x06);  //写使能	
	SET_SPI(SOFTCS,0x11);   //取消片选
	spi_flash_check_busy();
	return 1;
#else	
	res = read_sr();  //读flash状态寄存器
	while(res&0x01 == 1)  //忙则继续等
	{
		res = read_sr();  
	}
	
	SET_SPI(SOFTCS,0x01);  //片选
	
	SET_SPI(TXFIFO,0x6);    //发出命令0x6
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){  //等待发送接收
	}
	GET_SPI(RXFIFO);  //读接收缓存，数据丢掉

	SET_SPI(SOFTCS,0x11);   //取消片选

	return 1;
#endif
}

///////////////////////write status reg///////////////////////
int write_sr(char val)
{
	int res;
#ifdef NEW_SPI_ZZ
	set_wren(); //flash写使能操作

	//spi_flash_check_busy();
	SET_SPI(SOFTCS,0x01);  //片选
	spi_send_byte(0x01);  //写状态寄存器	
	spi_send_byte(val);  //写入值
#else	
	set_wren();	//flash写使能操作
	res = read_sr();  //读flash状态寄存器
	while(res&0x01 == 1)  //忙则继续等
	{
		res = read_sr();
	}
	
	SET_SPI(SOFTCS,0x01);  //片选
	SET_SPI(TXFIFO,0x01);  //发出命令0x1，这里是写发送缓存，写入发送缓存的数据，就会发送给flash
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){ //读控制器的状态，读缓存为空吗？没收完整就是空     			
	}				//发送是串行的，数据写入缓存，到发送完是有个时间的。
	GET_SPI(RXFIFO);  //读接收缓存，数据丢掉

	SET_SPI(TXFIFO,val);  //再发送值，由参数传入
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){   //等待发送完    			
	}
	GET_SPI(RXFIFO);  //读接收缓存，数据丢掉
#endif
	SET_SPI(SOFTCS,0x11);  //取消片选
	
	return 1;	
}

///////////erase all memory/////////////
int erase_all(void)
{
	int res;
	int i=1;

	spi_initw();
	set_wren();
	res = read_sr();
	while(res&0x01 == 1)
	{
		res = read_sr();
	}
	SET_SPI(SOFTCS,0x1);
	
	SET_SPI(TXFIFO,0xC7);
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){      			
	}
	GET_SPI(RXFIFO);
	
	SET_SPI(SOFTCS,0x11);
    while(i++){
        if(read_sr() & 0x1 == 0x1){
            if(i % 10000 == 0)
                printf(".");
        }else{
            printf("done...\n");
            break;
        }   
    }
	return 1;
}



void spi_read_id(void)
{
    unsigned char val;
    spi_initw();
    val = read_sr();
    while(val&0x01 == 1)
    {
        val = read_sr();
    }
    /*CE 0*/
    SET_SPI(SOFTCS,0x01);
    /*READ ID CMD*/
    SET_SPI(TXFIFO,0x9f);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    GET_SPI(RXFIFO);

    /*Manufacturer’s ID*/
    SET_SPI(TXFIFO,0x00);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    val = GET_SPI(RXFIFO);
    printf("Manufacturer's ID:         %x\n",val);

    /*Device ID:Memory Type*/
    SET_SPI(TXFIFO,0x00);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    val = GET_SPI(RXFIFO);
    printf("Device ID-memory_type:     %x\n",val);
    
    /*Device ID:Memory Capacity*/
    SET_SPI(TXFIFO,0x00);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    val = GET_SPI(RXFIFO);
    printf("Device ID-memory_capacity: %x\n",val);
    
    /*CE 1*/
    SET_SPI(SOFTCS,0x11);

}

#ifdef NEW_SPI_ZZ
#define PAGE_SIZE 0x100  //# 256B
//返回写入的字节数
static int spi_write_pagebytes(unsigned int addr,unsigned char *data,int len)
{
	unsigned int i = 0;
	
	if(len > PAGE_SIZE)
		len = PAGE_SIZE;   //最多一次编程1page

	i = addr & (0xff);  //起始地址是不是256的整数倍
	if(len + i > PAGE_SIZE) //页内有偏移，从写入的位置开始，到结束不能超过页的边界
		len = PAGE_SIZE - i; //写入页内字节数
	
	
	//1. 写使能
	set_wren();

	//2 .片选，页编程命令
	SET_SPI(SOFTCS,0x01);/*CE 0*/
	spi_send_byte(0x02);  //写页编程指令

	//3. 发送地址
	spi_send_byte((addr)>>16);  //写地址
	spi_send_byte((addr)>>8);  //写地址
	spi_send_byte(addr);  //写地址

	//4. 发送数据
	for(i=0;i<len;i++)
	{
		spi_send_byte(data[i]);  //写地址
	}
	
	//5.取消片选  /*CE 1*/
	SET_SPI(SOFTCS,0x11);  //取消片选

	spi_flash_check_busy(); //等待数据写入完成

	return len;   //返回实际写入的字节数
}

//写入数据
static void spi_write_bytes(unsigned int addr,unsigned char *data,int len)
{
	int ret = 0;

	while(len>0)
	{		
		ret = spi_write_pagebytes(addr,data,len);  //返回写入了多少个字节
		printf("spi_write_bytes ret = %d\n",ret);
		addr+=ret;  //指针向后移动
		data+=ret;  //指针向后移动
		len -= ret;
	}	
}
#endif

void spi_write_byte(unsigned int addr,unsigned char data)
{
#ifdef NEW_SPI_ZZ
	spi_write_pagebytes(addr,&data,1);
#else
    /*byte_program,CE 0, cmd 0x2,addr2,addr1,addr0,data in,CE 1*/
	unsigned char addr2,addr1,addr0;
        unsigned char val;
	addr2 = (addr & 0xff0000)>>16;
    	addr1 = (addr & 0x00ff00)>>8;
	addr0 = (addr & 0x0000ff);
        set_wren();
        val = read_sr();
        while(val&0x01 == 1)
        {
            val = read_sr();
        }
		SET_SPI(SOFTCS,0x01);/*CE 0*/

        SET_SPI(TXFIFO,0x2);/*byte_program */
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*send addr2*/
        SET_SPI(TXFIFO,addr2);     
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*send addr1*/
        SET_SPI(TXFIFO,addr1);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*send addr0*/
        SET_SPI(TXFIFO,addr0);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);

        /*send data(one byte)*/
       	SET_SPI(TXFIFO,data);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*CE 1*/
	SET_SPI(SOFTCS,0x11);
#endif
}
int write_pmon_byte(int argc,char ** argv)
{
    unsigned int addr;
   unsigned char val; 
    if(argc != 3){
        printf("\nuse: write_pmon_byte  dst(flash addr) data\n");
        return -1;
    }
    addr = strtoul(argv[1],0,0);
    val = strtoul(argv[2],0,0);
    spi_write_byte(addr,val);
return 0;

}


int write_pmon(int argc,char **argv)
{
	long int j=0;
        unsigned char val;
        unsigned int ramaddr,flashaddr,size;
	if(argc != 4){
        printf("\nuse: write_pmon src(ram addr) dst(flash addr) size\n");
        return -1;
    }

    ramaddr = strtoul(argv[1],0,0);
    flashaddr = strtoul(argv[2],0,0);
    size = strtoul(argv[3],0,0);
        
	spi_initw();
    write_sr(0);
// read flash id command
    spi_read_id();
	val = GET_SPI(SPSR);
	printf("====spsr value:%x\n",val);
	
	SET_SPI(0x5,0x10);
// erase the flash     
	write_sr(0x00);
//	erase_all();
    printf("\nfrom ram 0x%08x  to flash 0x%08x size 0x%08x \n\nprogramming            ",ramaddr,flashaddr,size);
    for(j=0;size > 0;flashaddr++,ramaddr++,size--,j++)
    {
        spi_write_byte(flashaddr,*((unsigned char*)ramaddr));
        if(j % 0x1000 == 0)
            printf("\b\b\b\b\b\b\b\b\b\b0x%08x",j);
    }
    printf("\b\b\b\b\b\b\b\b\b\b0x%08x end...\n",j);

    SET_SPI(0x5,0x11);
	return 1;
}

int read_pmon_byte(unsigned int addr,unsigned int num)
{
        unsigned char val,data;
	val = read_sr();
	while(val&0x01 == 1)
	{
		val = read_sr();
	}
	
	SET_SPI(0x5,0x01);
// read flash command 
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
// addr
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
        GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
        
        SET_SPI(TXFIFO,0x00);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        data = GET_SPI(RXFIFO);
	SET_SPI(0x5,0x11);
        return data;
}

int read_pmon(int argc,char **argv)
{
	unsigned char addr2,addr1,addr0;
	unsigned char data;
	int val,base=0;
	int addr;
	int i;
        if(argc != 3)
        {
            printf("\nuse: read_pmon addr(flash) size\n");
            return -1;
        }
        addr = strtoul(argv[1],0,0);
        i = strtoul(argv[2],0,0);
	spi_initw();
	val = read_sr();
	while(val&0x01 == 1)
	{
		val = read_sr();
	}
	
	SET_SPI(0x5,0x01);
// read flash command 
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
// addr
	SET_SPI(TXFIFO,((addr >> 16)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
        GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,((addr >> 8)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,(addr & 0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
// addr end
	
        
        printf("\n");
        while(i--)
	{
		SET_SPI(TXFIFO,0x00);
		while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
		}
	        data = GET_SPI(RXFIFO);
                if(base % 16 == 0 ){
                    printf("0x%08x    ",base);
                }
                printf("%02x ",data);
                if(base % 16 == 7)
                    printf("  ");
                if(base % 16 == 15)
                    printf("\n");
		base++;	
	}
        printf("\n");
	return 1;
	
}

int spi_erase_area(unsigned int saddr,unsigned int eaddr,unsigned sectorsize)
{
	unsigned int addr;
       	spi_initw(); 

	for(addr=saddr;addr<eaddr;addr+=sectorsize)
	{

	SET_SPI(SOFTCS,0x11);

	set_wren();

	write_sr(0x00);

	while(read_sr()&1);

	set_wren();

	SET_SPI(SOFTCS,0x01);

        /* 
         * 0x20 erase 4kbyte of memory array
         * 0x52 erase 32kbyte of memory array
         * 0xd8 erase 64kbyte of memory array                                                                                                           
         */
	SET_SPI(TXFIFO,0x20);
       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);
	SET_SPI(TXFIFO,addr >> 16);

       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,addr >> 8);
       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,addr);
	
       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);
	
	SET_SPI(SOFTCS,0x11);

	while(read_sr()&1);
	}
	SET_SPI(SOFTCS,0x11);
	delay(10);

	return 0;
}

int spi_write_area(int flashaddr,char *buffer,int size)
{
	int j;
	spi_initw();    //spi控制设置为写模式
//	SET_SPI(0x5,0x10); //spi控制器，设置片选，输出低，低有效
	write_sr(0x00);  //写flash的状态寄存器，不是控制器的
#ifdef NEW_SPI_ZZ
	spi_write_bytes(flashaddr,buffer,size);
#else
    for(j=0;size > 0;flashaddr++,size--,j++)
    {
        spi_write_byte(flashaddr,buffer[j]);   //写入数据，一个字节一个字节
    	dotik(32, 0);			//延时
    }
#endif
//	SET_SPI(SOFTCS,0x11); //取消片选，之前写入的数据是先到flash的缓存，然后才会编程到flash中，这样速度快一些
	delay(10);	//延时，结束，写入数据之后，flash会忙一阵子（把缓存的数据编程到flash中去）
	return 0;
}


int spi_read_area(int flashaddr,char *buffer,int size)
{
	int i;
	spi_initw();

	SET_SPI(SOFTCS,0x01);

	SET_SPI(TXFIFO,0x03);

        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);
        
        SET_SPI(TXFIFO,flashaddr>>16);     
        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);

        SET_SPI(TXFIFO,flashaddr>>8);     
        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);

        SET_SPI(TXFIFO,flashaddr);     
        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);
        

        for(i=0;i<size;i++)
        {
        SET_SPI(TXFIFO,0);     
        while((GET_SPI(SPSR))&RFEMPTY);
        buffer[i] = GET_SPI(RXFIFO);
        }

        SET_SPI(SOFTCS,0x11);
	delay(10);
	return 0;
}

struct fl_device myflash = {
	.fl_name="spiflash",
	.fl_size=0x100000,
	.fl_secsize=0x10000,
};

struct fl_device *fl_devident(void *base, struct fl_map **m)
{
	if(m)
	*m = fl_find_map(base);
	return &myflash;
}

int fl_program_device(void *fl_base, void *data_base, int data_size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_write_area(off,data_base,data_size);
	spi_initr();
	return 0;
}


int fl_erase_device(void *fl_base, int size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_erase_area(off,off+size,0x1000);
	spi_initr();
	return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"spi_initw","",0,"spi_initw(sst25vf080b)",spi_initw,0,99,CMD_REPEAT},
	{"read_pmon","",0,"read_pmon(sst25vf080b)",read_pmon,0,99,CMD_REPEAT},
	{"write_pmon","",0,"write_pmon(sst25vf080b)",write_pmon,0,99,CMD_REPEAT},
	{"erase_all","",0,"erase_all(sst25vf080b)",erase_all,0,99,CMD_REPEAT},
	{"write_pmon_byte","",0,"write_pmon_byte(sst25vf080b)",write_pmon_byte,0,99,CMD_REPEAT},
	{"read_flash_id","",0,"read_flash_id(sst25vf080b)",spi_read_id,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds,1);
}

