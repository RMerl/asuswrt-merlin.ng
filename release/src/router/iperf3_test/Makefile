
all:
	arm-brcm-linux-uclibcgnueabi-gcc main.c  -lpthread  -I/home/markcool/asuswrt/release/src-rt-6.x.4708/router/nvram_arm -I/home/markcool/asuswrt/release/src-rt-6.x.4708/include -I/home/markcool/asuswrt/release/src-rt-6.x.4708/router/shared -o iperf3_client -L/home/markcool/asuswrt/release/src-rt-6.x.4708/router/nvram_arm -L/home/markcool/asuswrt/release/src-rt-6.x.4708/router/shared -lnvram -lshared 
	arm-brcm-linux-uclibcgnueabi-gcc main.c  -lpthread  -I/home/markcool/asuswrt/release/src-rt-6.x.4708/router/nvram_arm -I/home/markcool/asuswrt/release/src-rt-6.x.4708/include -I/home/markcool/asuswrt/release/src-rt-6.x.4708/router/shared -o iperf3_server -L/home/markcool/asuswrt/release/src-rt-6.x.4708/router/nvram_arm -L/home/markcool/asuswrt/release/src-rt-6.x.4708/router/shared -lnvram -lshared 

clean:
	rm -f main