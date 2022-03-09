# 1 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"
# 1 "<built-in>"
# 1 "<命令行>"
# 1 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"




/dts-v1/;
/ {
 model = "loongson,LS2k1000-EVP";
 compatible = "loongson,ls2k";
 #address-cells = <2>;
 #size-cells = <2>;


 memory {
  name = "memory";
  device_type = "memory";



  reg = <0 0x00200000 0 0x0ee00000


   1 0x10000000 0 0xd0000000>;
 };

 aliases {
  ethernet0 = &gmac0;
  ethernet1 = &gmac1;
 };



 soc {
  compatible = "ls,nbus", "simple-bus";
  #address-cells = <1>;
  #size-cells = <1>;
  ranges = <0x10000000 0 0x10000000 0x10000000
     0x40000000 0 0x40000000 0x40000000
     0x20000000 0 0x20000000 0x20000000
                  0x0d000000 0 0x0d000000 0x02000000>;

  dma-coherent;
  nr_cpus_loongson = <2>;
  cores_per_package = <2>;

  icu: interrupt-controller@1fe11400 {
   compatible = "loongson,2k-icu";
   interrupt-controller;
   #interrupt-cells = <1>;
   reg = <0x1fe11400 0x40>;
  };

  msi: pci-msi-controller@1fe114a0 {
   compatible = "loongson,2k-pci-msi";
   msi-controller;
   interrupt-parent = <&icu>;
   msi-mask = <0x000000c0 0x40000c00>;
   reg = <0x1fe114a0 0x60>;
  };
# 71 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"
  dc@0x400c0000 {
   compatible = "loongson,ls-fb";
   reg = <0x400c0000 0x10000



       0x20000000 0x8000000>;

   interrupt-parent = <&icu>;
   interrupts = <36>;
  };


  gpu@0x40080000 {
   compatible = "loongson,galcore";
   reg = <0x40080000 0x40000
    0x2a000000 0x15000000>;
   interrupt-parent = <&icu>;
   interrupts = <37>;
  };


  ohci@0x40070000 {
   compatible = "loongson,ls2k-ohci", "usb-ohci";
   reg = <0x40070000 0x10000>;
   interrupt-parent = <&icu>;
   interrupts = <59>;
   dma-mask = <0x0 0xffffffff>;
  };

  ehci@0x40060000 {
   compatible = "loongson,ls2k-ehci", "usb-ehci";
   reg = <0x40060000 0x10000>;
   interrupt-parent = <&icu>;
   interrupts = <58>;
   dma-mask = <0xffffffff 0xffffffff>;
  };
# 117 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"
  ahci@0x400e0000{
   compatible = "snps,spear-ahci";
   reg = <0x400e0000 0x10000>;
   interrupt-parent = <&icu>;
   interrupts = <27>;
   dma-mask = <0x0 0xffffffff>;
  };

  gmac0: ethernet@0x40040000 {
   compatible = "snps,dwmac-3.70a", "ls,ls-gmac";
   reg = <0x40040000 0x10000>;
   interrupt-parent = <&icu>;
   interrupts = <20 21>;
   interrupt-names = "macirq", "eth_wake_irq";
   mac-address = [ 64 48 48 48 48 60 ];
   phy-mode = "rgmii";
   bus_id = <0x0>;
   phy_addr = <0xffffffff>;
   dma-mask = <0xffffffff 0xffffffff>;
  };

  gmac1: ethernet@0x40050000 {
   compatible = "snps,dwmac-3.70a", "ls,ls-gmac";
   reg = <0x40050000 0x10000>;
   interrupt-parent = <&icu>;
   interrupts = <22 23>;
   interrupt-names = "macirq", "eth_wake_irq";
   mac-address = [ 64 48 48 48 48 61 ];
   phy-mode = "rgmii";
   bus_id = <0x1>;
   phy_addr = <0xffffffff>;
   dma-mask = <0xffffffff 0xffffffff>;
  };
# 222 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"
  serial0x@0x1fe00000{
   device_type = "serial";
   compatible = "ns16550";
   reg = <0x1fe00000 0x100>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <8>;
  };

  serial3x@0x1fe00300{
   device_type = "serial";
   compatible = "ns16550";
   reg = <0x1fe00300 0x100>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <8>;
  };
  serial4x@0x1fe00400{
   device_type = "serial";
   compatible = "ns16550";
   reg = <0x1fe00400 0x100>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <9>;
  };
  serial5x@0x1fe00500{
   device_type = "serial";
   compatible = "ns16550";
   reg = <0x1fe00500 0x100>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <9>;
  };

  pioA:gpio@0x1fe10500{
   compatible = "ls,ls2k-gpio", "ls,ls-gpio";
   reg = <0x1fe10500 0x38>;
   ngpios = <64>;
   gpio-controller;
   #gpio-cells = <2>;
  };

  i2c0: i2c@1fe01000{
   compatible = "loongson,ls-i2c";
   reg = <0x1fe01000 0x8>;
   interrupt-parent = <&icu>;
   interrupts = <30>;
   #address-cells = <1>;
   #size-cells = <0>;
   rtc@68{
    compatible = "dallas,ds1338";
    reg = <0x68>;
   };

   tmp421@4d{
    compatible = "tmp422";
    reg = <0x4d>;
   };







  };
  i2c1: i2c@1fe01800{
   #address-cells = <1>;
   #size-cells = <0>;

   compatible = "loongson,ls-i2c";
   reg = <0x1fe01800 0x8>;
   interrupt-parent = <&icu>;
   interrupts = <31>;





  };
# 406 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"
  apbdma: apbdma@1fe10438{
   compatible = "loongson,ls-apbdma";
   reg = <0x1fe10438 0x8>;
   #config-nr = <2>;
  };




  dma0: dma@1fe10c00 {
   compatible = "loongson,ls-apbdma-0";
   reg = <0x1fe10c00 0x8>;
   apbdma-sel = <&apbdma 0x0 0x0>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };
  dma1: dma@1fe10c10 {
   compatible = "loongson,ls-apbdma-1";
   reg = <0x1fe10c10 0x8>;
   apbdma-sel = <&apbdma 0x5 0x1>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };
  dma2: dma@1fe10c20 {
   compatible = "loongson,ls-apbdma-2";
   reg = <0x1fe10c20 0x8>;
   apbdma-sel = <&apbdma 0x6 0x2>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };
  dma3: dma@1fe10c30 {
   compatible = "loongson,ls-apbdma-3";
   reg = <0x1fe10c30 0x8>;
   apbdma-sel = <&apbdma 0x7 0x3>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };
# 524 "/home/lifeng/loongson/pmon-loongson3-all/zloader.ls2k-hm19047/../Targets/LS2K-hm19047/conf/LS2K-hm19047.dts"
  hwmon@0x1fe07000{
   #address-cells = <1>;
   #size-cells = <1>;

   compatible = "loongson,ls-hwmon";
   reg = <0x1fe07000 0x1>;
   max-id = <0>;
   id = <0>;
  };
 };

 suspend_to_ram {
  suspend_addr = <0x1fc00500>;
 };

 chosen {


 };
};
