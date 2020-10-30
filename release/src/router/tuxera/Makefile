include ../common.mak
include $(SRCBASE)/.config

TNSA_SMBD :=
DSTDIR = $(INSTALLDIR)/lib/modules/$(LINUX_KERNEL)/kernel/fs

ifeq ($(RTCONFIG_RALINK),y)
# Ralink/MTK platform
SRCFILE = ralink

else ifeq ($(RTCONFIG_QCA),y)
# QCA platform
ifeq ($(RTCONFIG_SOC_QCA9557),y)
SRCFILE = $(if $(RT4GAC55U),qca_4gac55u,qca)
else ifeq ($(RTCONFIG_SOC_IPQ8064),y)
SRCFILE = qca_ipq806x
TNSA_SMBD = $(if $(RTCONFIG_SAMBA36X),,$(SRCFILE)/tnsa/smbd)
else ifeq ($(RTCONFIG_SOC_IPQ8074),y)
ifneq ($(findstring $(_BUILD_NAME_),$(SPF8_MODEL_LIST)),)
SRCFILE = qca_ipq807x.spf8
else ifneq ($(findstring $(_BUILD_NAME_),$(SPF11.0_MODEL_LIST)),)
SRCFILE = qca_ipq807x.spf11.0
else ifneq ($(findstring $(_BUILD_NAME_),$(IPQ807X_MODEL_LIST)),)
SRCFILE = $(if $(RTCONFIG_MUSL_LIBC),qca_ipq807x.musl64,qca_ipq807x)
endif
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
ifeq ($(RTCONFIG_QSDK6PLUS),y)
SRCFILE = qca_ipq40xx_ILQ6101CS
else ifeq ($(QSDK_VER),SPF10CS)
SRCFILE = qca_ipq40xx_SPF10CS
else # IPQ40XX
SRCFILE = qca_ipq40xx
endif # IPQ40XX
else
$(error Tuxera driver is not available)
endif

else ifeq ($(RTCONFIG_REALTEK),y)
# Realtek platform
SRCFILE = realtek
DSTDIR = $(INSTALLDIR)/lib/modules/3.10.24/kernel/fs

else ifeq ($(RTCONFIG_ALPINE),y)
SRCFILE = alpine_gt_ac9600
else ifeq ($(RTCONFIG_LANTIQ),y)
SRCFILE = lantiq_grx350
else
# Broadcom platform

ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
SRCFILE = broadcom_arm_7
else
ifeq ($(RTCONFIG_BCM_7114),y)
ifneq ($(RTCONFIG_GMAC3),y)
SRCFILE = broadcom_arm_7114.nogmac3
else
SRCFILE = broadcom_arm_7114
endif
else
ifeq ($(ARMCPUSMP),up)
SRCFILE = broadcom_arm_up
else
SRCFILE = broadcom_arm
endif # ARMCPUSMP
endif # RTCONFIG_BCM_7114
endif # RTCONFIG_BCM7

ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
ifeq ($(HND_ROUTER_AX_675X),y)
ifneq ($(BCM_CHIP),)
SRCFILE = broadcom_arm_ax_$(BCM_CHIP)
endif
else
SRCFILE = broadcom_arm_ax
endif
ifeq ($(HND_ROUTER_AX_675X),y)
DSTDIR = $(INSTALLDIR)/lib/modules/$(LINUX_KERNEL)/kernel/fs
else
DSTDIR = $(INSTALLDIR)/lib/modules/4.1.51/kernel/fs
endif
else ifeq ($(RTCONFIG_HND_ROUTER),y)
SRCFILE = broadcom_arm_hnd
DSTDIR = $(INSTALLDIR)/lib/modules/4.1.27/kernel/fs
else
DSTDIR = $(INSTALLDIR)/lib/modules/2.6.36.4brcmarm/kernel/fs
endif
else # RTCONFIG_BCMARM
SRCFILE = broadcom
DSTDIR = $(INSTALLDIR)/lib/modules/2.6.22.19/kernel/fs
endif
endif

#ifeq ($(BUILD_NAME),DSL-AC68U)
#SRCFILE = broadcom_arm_dsl_ac68u
#DSTDIR = $(INSTALLDIR)/lib/modules/2.6.36.4brcmarm/kernel/fs
#endif

export DSTDIR

vpath broadcom ralink

all:
	@[ ! -f $(SRCFILE)/Makefile ] || $(MAKE) -C $(SRCFILE) $@

clean:
	@[ ! -f $(SRCFILE)/Makefile ] || $(MAKE) -C $(SRCFILE) $@

install:
	@echo "[tuxera] Installing..."
	@install -d $(DSTDIR)
	@install -d $(INSTALLDIR)/usr/sbin
ifeq ($(RTCONFIG_TFAT),y)
	@cp -f $(SRCFILE)/fat/kernel-module/tfat.ko $(DSTDIR)
	install -D $(SRCFILE)/fat/tools/fatfsck $(INSTALLDIR)/usr/sbin/fatfsck
ifneq ($(NO_MKTOOLS),y)
	install -D $(SRCFILE)/fat/tools/mkfatfs $(INSTALLDIR)/usr/sbin/mkfatfs
	#install -D $(SRCFILE)/fat/tools/fatdebug $(INSTALLDIR)/usr/sbin/fatdebug
	#install -D $(SRCFILE)/fat/tools/fatinfo $(INSTALLDIR)/usr/sbin/fatinfo
	#install -D $(SRCFILE)/fat/tools/fatlabel $(INSTALLDIR)/usr/sbin/fatlabel
	#install -D $(SRCFILE)/fat/tools/fatdump $(INSTALLDIR)/usr/sbin/fatdump
endif
endif
ifeq ($(RTCONFIG_TUXERA_NTFS),y)
	@cp -f $(SRCFILE)/ntfs/kernel-module/tntfs.ko $(DSTDIR)
	install -D $(SRCFILE)/ntfs/tools/ntfsck $(INSTALLDIR)/usr/sbin/ntfsck
ifneq ($(NO_MKTOOLS),y)
	install -D $(SRCFILE)/ntfs/tools/mkntfs $(INSTALLDIR)/usr/sbin/mkntfs
	#install -D $(SRCFILE)/ntfs/tools/ntfsdebug $(INSTALLDIR)/usr/sbin/ntfsdebug
	#install -D $(SRCFILE)/ntfs/tools/ntfsinfo $(INSTALLDIR)/usr/sbin/ntfsinfo
	#install -D $(SRCFILE)/ntfs/tools/ntfslabel $(INSTALLDIR)/usr/sbin/ntfslabel
endif
endif
ifeq ($(RTCONFIG_TUXERA_HFS),y)
	cp -f $(SRCFILE)/hfsplus/kernel-module/thfsplus.ko $(DSTDIR)
	install -D $(SRCFILE)/hfsplus/tools/fsck_hfs $(INSTALLDIR)/usr/sbin/fsck_hfs
ifneq ($(NO_MKTOOLS),y)
	install -D $(SRCFILE)/hfsplus/tools/newfs_hfs $(INSTALLDIR)/usr/sbin/newfs_hfs
	#install -D $(SRCFILE)/hfsplus/tools/thfslabel $(INSTALLDIR)/usr/sbin/thfslabel
endif
endif
ifeq ($(or $(RTCONFIG_TFAT),$(RTCONFIG_TUXERA_NTFS),$(RTCONFIG_TUXERA_HFS)),y)
	install  -m 0644 -D $(SRCFILE)/VERSION-INFO $(INSTALLDIR)/rom/Tuxera-VERSION-INFO
ifeq ($(and $(RTCONFIG_BCMARM),$(RTCONFIG_SAMBA3)),y)
#	install -D $(SRCFILE)/samba/lib/libstdc++.so.6.0.14 $(INSTALLDIR)/usr/lib/libstdc++.so.6.0.14
#	cd $(INSTALLDIR)/usr/lib && ln -sf libstdc++.so.6.0.14 libstdc++.so.6 && ln -sf libstdc++.so.6.0.14 libstdc++.so
ifeq ($(RTCONFIG_BCM_7114),y)
	install -D $(SRCFILE)/samba/smbd $(INSTALLDIR)/usr/bin/smbd
endif
endif
endif
ifneq ($(TNSA_SMBD),)
	# NOTE: Destination path of smbd is different from Broadcom platform!
	[ -e $(TNSA_SMBD) ] && install -D $(TNSA_SMBD) $(INSTALLDIR)/usr/bin/smbd
endif

.phony: all clean install

