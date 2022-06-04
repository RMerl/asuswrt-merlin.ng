obj-$(CONFIG_PHONET) += phonet.o pn_pep.o

phonet-y := \
	pn_dev.o \
	pn_netlink.o \
	socket.o \
	datagram.o \
	sysctl.o \
	af_phonet.o

pn_pep-y := pep.o pep-gprs.o

ifdef BCM_KF # defined(CONFIG_BCM_KF_PHONET)
obj-$(CONFIG_LD_PHONET) += ld_phonet.o
obj-$(CONFIG_LD_TMODEM) += ld_tmodem.o

subdir-ccflags-y += -Werror
endif # BCM_KF
