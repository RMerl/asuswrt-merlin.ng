ifneq (,$(filter $(BRCM_CHIP),6858 6846 6856 6878 6855))
ifneq ($(wildcard $(BRCMDRIVERS_DIR)/opensource/net/enet/impl7/enet_ingress),)
obj-y += enet_ingress/
endif
endif

obj-$(CONFIG_BCM_ENET) := bcm_enet.o

bcm_enet-objs += bcm_enet_dep.o

$(obj)/bcm_enet_dep.o:
	cp $(obj)/bcm_enet.o $(obj)/bcm_enet_dep.o
