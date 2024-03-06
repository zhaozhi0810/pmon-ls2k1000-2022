# pmon-ls2k1000-2022

编译命令
cd pmon-loongson3-hj20004/zloader.ls2k-hj20004
make cfg
./build.sh   (编译的命令都sh中了，交叉编译工具的路径需要注意，根据自己的修改)

编译后可以使用vmlinux或者vmlinuz



2024-0229  pmon for ls2k1000 (hd43)
1. 对 g 命令帮助信息进行了完善，  h g 查看
2. 对load命令帮助信息进行了完善，  h load 查看
3. c 打断自动启动时，自动喂狗
4. 配置默认IP，syn0 192.168.1.12 ，syn1：192.168.0.12
5. 环境变量设置在Makefile.inc （顶层）中，可以修改，目前已经调整为linux启动的环境变量 



2024-03-04
1. 通过调试，网卡的mac地址已经可以通过dts中设置
	gmac0: ethernet@0x40040000 {
			compatible = "snps,dwmac-3.70a", "ls,ls-gmac";
			reg = <0x40040000 0x10000>;
			interrupt-parent = <&icu>;
			interrupts = <20 21>;
			interrupt-names = "macirq", "eth_wake_irq";
			mac-address = [ 64 48 48 48 48 60 ];/* [>mac 64:48:48:48:48:60 <]*/
			phy-mode = "rgmii";
			bus_id = <0x0>;
			phy_addr = <0xffffffff>;
			dma-mask = <0xffffffff 0xffffffff>;
		}; 	
	源码 load-dtb.c 中的update_mac 函数	
2. 修改pmon的生成脚本（pmonenv.py），让gzrom-dtb.bin中大部分空白的区域填充0xff ,但是最后500字节的env区域还没有修改好
	因为pmon运行后本身会重写env的区域，这个地方所以还要追一下源码。

3. 但是使用dts中的mac本身也有问题，也会造成所有的板卡的mac都会相同，而且不好单独设置
	可以考虑设置到pmon的flash中，以为25q64 只用了前面1M的空间，还有很多剩余空间可以利用。（没有去实现哈）




2024-03-06
1. pmon 会更新 dtb中的一些数据，比如pci的地址分配，mac地址等。
	内核改为直接使用自己的dtb的话，我是直接在pmon中把dtb打印出来，然后把pci相关的地址分配写道内核的dts中
	这样ch368的驱动才能正常加载。
2. pmon 修改dtb的时候，它的填充值改为了0xff   函数verify_dtb(void) （load_dtb.c） 这样基本满足了不用的空间保持为0xff了。
3. 根据pmon的代码，修改了一个在Linux下直接烧写pmon的工具。（tools/program-2k1000-pmon 下直接make）
	目前只在ls2k1000+ gd25q64上测试了



