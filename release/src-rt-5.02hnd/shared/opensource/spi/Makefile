# 3.2+ Kernel
ifeq ($(strip $(BRCM_CHIP)),6362)
obj-y +=               \
	bcmHsSpi.o        
endif

ifeq ($(strip $(BRCM_CHIP)),6818)
obj-y +=               \
	bcmHsSpi.o        
endif

ifeq ($(strip $(BRCM_CHIP)),6816)
obj-y +=               \
	bcmHsSpi.o        
endif
ifeq ($(strip $(BRCM_CHIP)),6818)
obj-y +=               \
	bcmHsSpi.o        
endif
ifeq ($(strip $(BRCM_CHIP)),6328)
obj-y +=               \
	bcmHsSpi.o        
endif

ifeq ($(strip $(BRCM_CHIP)),63268)
obj-y +=               \
	bcmHsSpi.o        
endif

ifeq ($(strip $(BRCM_CHIP)),6828)
obj-y +=               \
	bcmHsSpi.o        
endif
ifeq ($(strip $(BRCM_CHIP)),6838)
obj-y +=               \
    bcmHsSpi.o
endif
ifeq ($(strip $(BRCM_CHIP)),6318)
obj-y +=               \
    bcmHsSpi.o
endif
ifeq ($(strip $(BRCM_CHIP)),60333)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),63138)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),63148)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),4908)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),6858)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),68360)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),63381)
obj-y +=               \
    bcmHsSpi.o
endif

ifeq ($(strip $(BRCM_CHIP)),6848)
obj-y +=               \
    bcmHsSpi.o
endif

ifneq ($(strip $(BRCM_CHIP)),6328)
obj-y +=               \
    bcmLegSpi.o
endif

obj-y +=               \
    bcmSpiRes.o 

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/pmc
