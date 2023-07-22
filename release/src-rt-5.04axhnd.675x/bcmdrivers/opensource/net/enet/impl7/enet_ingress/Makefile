obj-$(CONFIG_BCM_ENET) := bcm_enet_ingress.o

bcm_enet_ingress-objs += bcm_enet_ingress_dep.o

$(obj)/bcm_enet_ingress_dep.o:
	cp $(obj)/bcm_enet_ingress.o $(obj)/bcm_enet_ingress_dep.o
