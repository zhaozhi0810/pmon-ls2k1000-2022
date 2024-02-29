# 1 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"




/dts-v1/;
/ {
 model = "loongson,LS2k1000-EVP";
 compatible = "loongson,ls2k";
 #address-cells = <2>;
 #size-cells = <2>;


 memory {
  name = "memory";
  device_type = "memory";

  reg = <0 0x00200000 0 0x0ce00000




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
# 71 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
  dc@0x400c0000 {
   compatible = "loongson,ls-fb";
   reg = <0x400c0000 0x10000

       0x0d000000 0x2000000>;



   interrupt-parent = <&icu>;
   interrupts = <36>;
  };
# 93 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
  ohci@0x40070000 {
   compatible = "loongson,ls2k-ohci", "usb-ohci", "generic-ohci";
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

  otg@0x40000000 {
   compatible = "loongson,dwc-otg", "dwc-otg";
   reg = <0x40000000 0x40000>;
   interrupt-parent = <&icu>;
   interrupts = <57>;
   dma-mask = <0x0 0xffffffff>;
  };

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



  pcie0_port0@40100000 {
   compatible = "loongson,ls-pcie";
   interrupt-parent = <&icu>;
   interrupts = <40>;
   #address-cells = <3>;
   #size-cells = <2>;
   bus-range = <0x1 0x3>;




     ranges = <0x02000000 0x0 0x40260000 0x40260000 0x0 0x8000000
      0x01000000 0x0 0x4028c000 0x4028c000 0x0 0x0ff000>;

  };

  pcie0_port1@50000000 {
   compatible = "loongson,ls-pcie";
   interrupt-parent = <&icu>;
   interrupts = <41>;
   #address-cells = <3>;
   #size-cells = <2>;
   bus-range = <0x4 0x6>;
     ranges = <0x02000000 0x0 0x50000000 0x50000000 0x0 0x4000000
      0x01000000 0x0 0x18100000 0x18100000 0x0 0x100000>;
  };

  pcie0_port2@54000000 {
   compatible = "loongson,ls-pcie";
   interrupt-parent = <&icu>;
   interrupts = <42>;
   #address-cells = <3>;
   #size-cells = <2>;
   bus-range = <0x8 0xa>;
     ranges = <0x02000000 0x0 0x54000000 0x54000000 0x0 0x4000000
      0x01000000 0x0 0x18200000 0x18200000 0x0 0x100000>;
  };

  pcie0_port3@58000000 {
   compatible = "loongson,ls-pcie";
   interrupt-parent = <&icu>;
   interrupts = <43>;
   #address-cells = <3>;
   #size-cells = <2>;
   bus-range = <0xc 0xe>;
     ranges = <0x02000000 0x0 0x58000000 0x58000000 0x0 0x8000000
      0x01000000 0x0 0x18300000 0x18300000 0x0 0x100000>;
  };

  pcie1_port0@60000000 {
   compatible = "loongson,ls-pcie";
   interrupt-parent = <&icu>;
   interrupts = <44>;
   #address-cells = <3>;
   #size-cells = <2>;
   bus-range = <0x10 0x12>;
     ranges = <0x02000000 0x0 0x60000000 0x60000000 0x0 0x18000000
      0x01000000 0x0 0x18400000 0x18400000 0x0 0x100000>;
  };

  pcie1_port1@78000000 {
   compatible = "loongson,ls-pcie";
   interrupt-parent = <&icu>;
   interrupts = <45>;
   #address-cells = <3>;
   #size-cells = <2>;
   bus-range = <0x14 0x16>;
     ranges = <0x02000000 0x0 0x78000000 0x78000000 0x0 0x8000000
      0x01000000 0x0 0x18500000 0x18500000 0x0 0x100000>;
  };

  serial0x@0x1fe00000{
   device_type = "serial";
   compatible = "ns16550";
   reg = <0x1fe00000 0x100>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <8>;
  };
# 250 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
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

   rtc@51{
    compatible = "nxp,pcf8563";
    reg = <0x51>;
   };
# 296 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
  };
# 343 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
  spi0: spi@1fff0220{
   compatible = "loongson,ls2k-spi";
   #address-cells = <1>;
   #size-cells = <0>;
   reg = <0x1fff0220 0x10>;
   spidev@0{
    compatible = "rohm,dh2228fv";
    spi-max-frequency = <100000000>;
    reg = <0>;
   };
  };
# 368 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
  can0: can@1fe00c00{
   compatible = "nxp,sja1000";
   reg = <0x1fe00c00 0xff>;
   nxp,external-clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <24>;
  };
  can1: can@1fe00d00{
   compatible = "nxp,sja1000";
   reg = <0x1fe00d00 0xff>;
   nxp,external-clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <25>;
  };
# 415 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
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
# 533 "/home/jc/ls2k1000-20240208/pmon-loongson3-hj20004/zloader.ls2k-hj20004/../Targets/LS2K-hj20004/conf/LS2K-hj20004.dts"
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
