ifdef BCM_KF # defined(CONFIG_BCM_KF_BLOG)
EXTRA_CFLAGS	+= -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS	+= -I$(INC_BRCMSHARED_PUB_PATH)/bcm963xx
endif # BCM_KF

obj-$(CONFIG_MHI_L3MHI)      += af_mhi.o

af_mhi-objs		     := mhi_proto.o mhi_socket.o mhi_dgram.o mhi_raw.o

obj-$(CONFIG_MHI_L2MUX)      += l2mux.o
obj-$(CONFIG_MHI_L3MHI)      += l3mhi.o
obj-$(CONFIG_MHI_L3MHDP)     += l3mhdp.o
obj-$(CONFIG_MHI_L3PHONET)   += l3phonet.o
subdir-ccflags-y	     += -Werror

