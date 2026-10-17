PLATFORM_CPPFLAGS += -mcpu=arcem -mlittle-endian -mcode-density -mdiv-rem \
                     -mswap -mnorm -mmpy-option=6 -mbarrel-shifter
LDSCRIPT = $(srctree)/board/synopsys/iot_devkit/u-boot.lds
