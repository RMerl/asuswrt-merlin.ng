#
# Makefile for the PCMHAL driver
#
ifneq ($(strip $(BRCM_BCM_PCM_VER)),)
$(info PCM driver is linking)
obj-$(CONFIG_BCM_PCM) += pcmhal.o
pcmhal-objs := pcmhal_pcm.o 

subdir-ccflags-y += -I$(src)
endif
