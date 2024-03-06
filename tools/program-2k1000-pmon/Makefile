CC= mips64el-loongson-linux-gcc

all: mymap.o spi_w.o
	$(CC) --static  -o  program_pmon_ls2k1000 $^

%.o:%.c
	$(CC) -c $^


clean:
	rm -f program_pmon_ls2k1000 *.o
