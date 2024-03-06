#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#define UNCACHED_MEMORY_ADDR	0x8000000000000000

#define SPCR      0x0
#define SPSR      0x1
#define TXFIFO    0x2
#define RXFIFO    0x2
#define SPER      0x3
#define PARAM     0x4
#define SOFTCS    0x5
#define PARAM2    0x6

#define PAGE_SIZE_W 0x100

#define RFEMPTY 1
#define KSEG1_STORE8(addr,val)	(*(volatile unsigned char *)(addr) = val)
#define KSEG1_LOAD8(addr)	(*(volatile unsigned char *)(addr))

#define SET_SPI(addr,val)	KSEG1_STORE8((spireg_base + addr),val)
#define GET_SPI(addr)		KSEG1_LOAD8((spireg_base + addr))


#define SET_CE(x)		SET_SPI(SOFTCS,x)


unsigned long  base,spimem_base,spireg_base;
int sectorsize = 0x1000;
unsigned char manufacturer_id = 0xEF;    // MANUFACTURER_ID 
unsigned char device_id = 0x15;          //Device_ID
unsigned char mem_type_id = 0x00;


/*************************************************************
 *  dotik(rate,use_ret)
 */
void dotik (int rate, int use_ret)
{
	static	int             tik_cnt;
	static	const char      more_tiks[] = "|/-\\";
	static	const char     *more_tik;

	tik_cnt -= rate;
	if (tik_cnt > 0) {
		return;
	}
	tik_cnt = 256000;
	if (more_tik == 0) {
		more_tik = more_tiks;
	}
	if (*more_tik == 0) {
		more_tik = more_tiks;
	}
	if (use_ret) {
		printf (" %c\r", *more_tik);
	} else {
		printf ("\b%c", *more_tik);
	}
	more_tik++;
}

int delay(int cnt)
{
	int i;
	for(i=0;i<cnt;i++);
	return 0;
}

void spi_initw(void)
{
	//ls3a4000 need change this code?
	SET_SPI(SPSR  , 0xc0);
	SET_SPI(PARAM , 0x40);	//espr:0100
#ifdef LOONGSON_3C5000
	SET_SPI(SPER  , 0x06);
#else
	SET_SPI(SPER  , 0x05);	//spre:01
#endif
	SET_SPI(PARAM2, 0x01);
	SET_SPI(SPCR  , 0x50);
}

void spi_initr(void)
{
#ifdef LOONGSON_3C5000
	SET_SPI(PARAM, 0x27);
#else
	SET_SPI(PARAM, 0x47);	//espr:0100
#endif
}

int send_spi_cmd(unsigned char command)
{
	int timeout = 1000;
	unsigned char val;
	SET_SPI(TXFIFO,command);
	while (((GET_SPI(SPSR)) & RFEMPTY) && timeout--);
	val = GET_SPI(RXFIFO);

	if (timeout < 0) {
		printf("wait rfempty timeout!\n");
		return -1;
	}
	return val;
}

int read_sr(void)
{
	int val;
	SET_CE(0x01);
	send_spi_cmd(0x05);
	val = send_spi_cmd(0x00);
	SET_CE(0x11);

	return val;
}

int spi_wait_busy(void)
{
	int timeout = 1000;
	unsigned char res;

	do {
		res = read_sr();
	} while ((res & 0x01) && timeout--);

	if (timeout < 0) {
		printf("wait status register busy bit time out!\n");
		return -1;
	}
	return 0;
}

/*****************************set write enable*******************************/
int set_wren(void)
{
	if (spi_wait_busy() < 0) {
		return -1;
	}
	SET_CE(0x01);
	send_spi_cmd(0x6);
	SET_CE(0x11);

	return 1;
}

/***********************Enable-Write-Status-Register************************/
int en_write_sr(void)
{
	if (spi_wait_busy() < 0)
		return -1;
	SET_CE(0x01);
	send_spi_cmd(0x50);
	SET_CE(0x11);

	return 1;
}

/*****************************write status reg*******************************/
int write_sr(char val)
{
	/*this command do'nt used to check busy bit otherwise cause error*/
	en_write_sr();

	SET_CE(0x01);
	send_spi_cmd(0x01);
	/*set status register*/
	send_spi_cmd(val);
	SET_CE(0x11);

	return 1;
}

/*****************************erase all memory*******************************/
int erase_all(void)
{
	int i = 1;
	spi_initw();
	set_wren();
	if (spi_wait_busy() < 0)
		return -1;

	SET_CE(0x01);
	send_spi_cmd(0xc7);
	SET_CE(0x11);
	while (i++) {
		if (read_sr() & 0x1) {
			if (i % 10000 == 0)
				printf(".");
		} else {
			printf("done...\n");
			break;
		}
	}
	return 1;
}

void spi_read_id(void)
{ 
	char i;

	spi_initw();
	if (spi_wait_busy() < 0)
		return;

	/*CE 0*/
	SET_CE(0x01);
	/*READ ID CMD*/
	send_spi_cmd(0x90);

	/*address bit [23-0]*/
	for (i = 0; i < 3; i++) {
		send_spi_cmd(0);
	}

	/*Manufacturer’s ID*/
	manufacturer_id = send_spi_cmd(0);
	printf("Manufacturer's ID:         %x\n", manufacturer_id);
	/*Device ID*/
	device_id = send_spi_cmd(0);
	printf("Device ID:                 %x\n", device_id);
	/*CE 1*/
	SET_CE(0x11);
}

void spi_jedec_id(void)
{
	unsigned char val;
	spi_initw();

	if (spi_wait_busy() < 0)
		return;
	/*CE 0*/
	SET_CE(0x01);
	/*JEDEC ID CMD*/
	send_spi_cmd(0x9f);

	/*Manufacturer’s ID*/
	val = send_spi_cmd(0x00);
	printf("Manufacturer's ID:         %x\n", val);

	/*Device ID:Memory Type*/
	val = send_spi_cmd(0x00);
	mem_type_id = val;
	
	printf("Device ID-memory_type:     %x\n", val);

	/*Device ID:Memory Capacity*/
	val = send_spi_cmd(0x00);
	printf("Device ID-memory_capacity: %x\n", val);

	/*CE 1*/
	SET_CE(0x11);
}

static void spi_flash_check_busy(void)
{
	unsigned char res;

	do{
		res = read_sr();  //读flash状态寄存器
	}while((res&0x01));  //忙则继续等
}

static int spi_write_pagebytes(unsigned int addr, unsigned char *data, int len)
{
	unsigned int i = 0;

	//	printf("1 addr = %#x i = %u, len = %d data[0] = %hhx\n",addr,i,len,data[0]);

	if(len > PAGE_SIZE_W)
		len = PAGE_SIZE_W;   //最多一次编程1page

	i = addr & (0xff);  //起始地址是不是256的整数倍
	if(len + i > PAGE_SIZE_W) //页内有偏移，从写入的位置开始，到结束不能超过页的边界
		len = PAGE_SIZE_W - i; //写入页内字节数

	//	printf("addr = %#x i = %u, len = %d data[0] = %hhx\n",addr,i,len,data[0]);
	write_sr(0x0);	
	//1. 写使能
	set_wren();
	
	if (spi_wait_busy() < 0)
		return 0;

	//2 .片选，页编程命令
	SET_CE(0x01);/*CE 0*/
	send_spi_cmd(0x02);  //写页编程指令

	//3. 发送地址
	send_spi_cmd(((addr)>>16) & 0xff);  //写地址
	send_spi_cmd(((addr)>>8) & 0xff);  //写地址
	send_spi_cmd(addr & 0xff);  //写地址

	//4. 发送数据
	for(i=0;i<len;i++)
	{
		send_spi_cmd(data[i]);  //写地址
	}

	//5.取消片选  /*CE 1*/
	SET_CE(0x11);  //取消片选

	spi_flash_check_busy(); //等待数据写入完成

	return len;   //返回实际写入的字节数
}

static void spi_write_bytes(unsigned int addr, unsigned char *data, int len)
{
	int tmp,ret = 0;
	int i = 0;
	char buffer[101] = {0};//存储进度条字符
	char arr[5] = {"-/|\\"};//存储基本的变化字幕
	spi_initw();
	SET_CE(0x10);
	write_sr(0x00);
	tmp = len/100;  
	
	while( len>0 )
	{
		delay(300000);    //必须延时，否则写失败
		ret = spi_write_pagebytes(addr, data, len);  //返回写入了多少个字节
		addr += ret;  //指针向后移动
		data += ret;  //指针向后移动
		len -= ret;

		if(len < (100*tmp - tmp*i))
		{
			buffer[i] = '#';
			printf("[%-100s][%d%%][%c]\r",buffer, i+1, arr[i%4]);
			fflush(stdout);
			i++;
		}	
	}
	SET_CE(0x11);
	while (read_sr() & 1);
	SET_CE(0x11);
	delay(10);
}

void spi_write_byte(unsigned int addr, unsigned char data)
{
	write_sr(0x0);
	set_wren();
	if (spi_wait_busy() < 0)
		return;
	SET_CE(0x01);	/*CE 0*/

	send_spi_cmd(0x2);

	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	/*send data(one byte)*/
	send_spi_cmd(data);

	/*CE 1*/
	SET_CE(0x11);
}

int spi_write_area(u_int64_t flashaddr, char *buffer, int size)
{
	int j;
	spi_initw();
	SET_CE(0x10);
	write_sr(0x00);
	for (j = 0; size > 0; flashaddr++, size--, j++) {
		spi_write_byte(flashaddr, buffer[j]);
		printf("%d\n",j);
		dotik(32, 0);
	}

	SET_CE(0x11);
	while (read_sr() & 1);
	SET_CE(0x11);
	delay(10);
	return 0;
}

int spi_erase_area(u_int64_t saddr, unsigned int eaddr, unsigned sectorsize)
{
	unsigned int addr,tmp,i=0;
	char buffer[101] = {0};//存储进度条字符
	char arr[5] = {"-/|\\"};//存储基本的变化字幕
	tmp = eaddr/100;
	spi_initw();

	for (addr = saddr; addr < eaddr; addr += sectorsize)
	{
		SET_CE(0x11);

		set_wren();
		write_sr(0x00);
		while (read_sr() & 1);
		set_wren();
		SET_CE(0x01);
		/*
		 * 0x20 erase 4kbyte of memory array
		 * 0x52 erase 32kbyte of memory array
		 * 0xd8 erase 64kbyte of memory array
		 */
		send_spi_cmd(0x20);

		/*send addr [23 16]*/
		send_spi_cmd((addr >> 16) & 0xff);
		/*send addr [15 8]*/
		send_spi_cmd((addr >> 8) & 0xff);
		/*send addr [8 0]*/
		send_spi_cmd(addr & 0xff);
		SET_CE(0x11);

		while (read_sr() & 1);
		if(addr > i*tmp)
		{
			buffer[i] = '#';
			printf("[%-100s][%d%%][%c]\r",buffer,i+1,arr[i%4]);
			fflush(stdout);
			i++;    
		}
	}
	SET_CE(0x11);
	delay(10);

	return 0;
}


int identify_verify()
{
	printf("identify...\n");
	spi_jedec_id();
	spi_read_id();
	
	if(!(((manufacturer_id  ==  0xef)&&(device_id == 0x16))||((manufacturer_id == 0xef)&&(device_id == 0x15)) || ((manufacturer_id == 0xc8)&&(device_id == 0x16)&&(mem_type_id == 0x65 || mem_type_id == 0x40))))
	{
		printf("Unsuportted device type.\n");
		return(0);
	}
	
	printf("identify end.\n");
	return(1);
}

int erase_sect(int soff, int eoff, int sectorsize)
{
	//printf("erase soff = %x,eoff = %d ,sectorsize = %x\n",soff,eoff,sectorsize);    
	printf("erase sect...\n");    
	spi_erase_area(soff, eoff, sectorsize);//0 size 0x10000
	spi_initr();
	printf("\nerase done.\n");
	
	return 0;
}

int erase_chip()
{
	erase_all();
	spi_initr();
	return 0;
}

int program(char *buf, int soff, int eoff)
{
	printf("download program...\n");
	spi_write_bytes(soff, buf, eoff);    //pagebytes write
	spi_initr();
	printf("\ndownload end.\n");
	return 0;
}

int spiflash_main(char *cmd, unsigned long flashbase, unsigned long spiregbase, unsigned long spimem_base_map,...)
{
	void *ap,**arg;
	int i,ret;
	
	__builtin_va_start(ap,spimem_base_map);
	arg = ap;
	base = flashbase;
	spimem_base = spimem_base_map;
	spireg_base = spiregbase;
	
	printf("erase before %x \n",*((unsigned int *)flashbase));
	for(i=0; cmd[i]; i++)
	{
		switch(cmd[i])
		{
			case 'i'://识别并校验设备
				ret = identify_verify();
				if(ret)
					break;
				else 
					return 0;    
				
			case 'e'://区域擦除
				erase_sect((int)(*(int*)arg++), (int)(*(int*)arg++), (int)(*(int*)arg++));
				printf("erase_sect %x \n",*((unsigned int *)flashbase));
				break;
				
			case 'E'://全部擦除
				erase_chip();
				break;
				
			case 'p'://下载程序
				program(*arg++, (int)(*(int*)arg++), (int)(*(int*)arg++));
				printf("program %x \n", *((unsigned int*)flashbase));
				
			default:
				break;
		}
	}
	
	return 0;
}
