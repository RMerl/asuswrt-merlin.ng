include ../../.config

FLTR_OBJS := fltr.o

# src-rt-5.02hnd
#HND_MODELS = GT-AC5300 GT-AC2900 RT-AC86U

# src-rt-5.02axhnd
#HND_MODELS += RT-AX88U RT-AX92U GT-AX11000

# src-rt-5.02axhnd.675x
#HND_MODELS += RT-AX95Q RT-AX82U RT-AX58U RT-AX56U TUF-AX3000

# src-rt-5.04axhnd.675x
#HND_MODELS += GT-AX11000_PRO ET12 XT12 GT-AXE16000 XT8PRO BT12 BT10 BQ16 BQ16_PRO BM68 XT8_V2 ET8PRO ET8_V2 XD6_V2

obj-$(CONFIG_NETFILTER_ASUS_FILTER) += $(FLTR_OBJS)

#ifneq ($(findstring $(BUILD_NAME), $(HND_MODELS)), )
ifneq ($(CONFIG_BCM_CHIP_NUMBER),)
 CFLAGS_MODULE += -DHAVE_BRCM_FC
 SRCBASE := ../..
 CFLAGS_MODULE += -I$(SRCBASE)/shared/opensource/include/bcm963xx
 CFLAGS_MODULE += -I$(SRCBASE)/bcmdrivers/broadcom/include/bcm963xx
 CFLAGS_MODULE += -I$(SRCBASE)/bcmdrivers/opensource/include/bcm963xx
else
 CFLAGS_MODULE += -DNO_BRCM_FC
endif

CFLAGS_MODULE += -DBUILD_NAME=\"$(BUILD_NAME)\"

clean-files += *.o .*.o.cmd
