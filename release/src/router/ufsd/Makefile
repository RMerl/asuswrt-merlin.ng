include ../common.mak

ifeq ($(RTCONFIG_RALINK),y)
ifeq ($(CONFIG_LINUX30),y)
SRCFILE = ralink_3.0.0
else
SRCFILE = ralink_2.6.2219
endif
DSTDIR = $(INSTALLDIR)/lib/modules/$(LINUX_KERNEL)/kernel/fs
else
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(ARMCPUSMP),up)
SRCFILE = broadcom_arm_up
else
SRCFILE = broadcom_arm
endif
DSTDIR = $(INSTALLDIR)/lib/modules/2.6.36.4brcmarm/kernel/fs
else
SRCFILE = broadcom
DSTDIR = $(INSTALLDIR)/lib/modules/2.6.22.19/kernel/fs
endif
endif

ifeq ($(RTCONFIG_RECVFILE),y)
SRCFILE := $(SRCFILE).recvfile
endif

vpath broadcom ralink

all:

clean:

install:
	@echo "[ufsd] Installing..."
	@install -d $(INSTALLDIR)/usr/sbin
	@install -d $(DSTDIR)
ifeq ($(or $(RTCONFIG_PARAGON_NTFS),$(RTCONFIG_PARAGON_HFS)),y)
	@[ ! -f $(SRCFILE)/ufsd.ko ] || cp -f $(SRCFILE)/ufsd.ko $(DSTDIR) || chmod 664 $(DSTDIR)/ufsd.ko
	@[ ! -f $(SRCFILE)/jnl.ko ] || cp -f $(SRCFILE)/jnl.ko $(DSTDIR) || chmod 664 $(DSTDIR)/jnl.ko
ifneq ($(NO_MKTOOLS),y)
	@[ ! -f $(SRCFILE)/memalloc ] || cp -f $(SRCFILE)/memalloc $(INSTALLDIR)/usr/sbin || chmod 775 $(INSTALLDIR)/usr/sbin/memalloc
	@[ ! -f $(SRCFILE)/test_system ] || cp -f $(SRCFILE)/test_system $(INSTALLDIR)/usr/sbin || chmod 775 $(INSTALLDIR)/usr/sbin/test_system
endif
endif
ifeq ($(RTCONFIG_PARAGON_NTFS),y)
	@[ ! -f $(SRCFILE)/chkntfs ] || cp -f $(SRCFILE)/chkntfs $(INSTALLDIR)/usr/sbin || chmod 775 $(INSTALLDIR)/usr/sbin/chkntfs
ifneq ($(NO_MKTOOLS),y)
	@[ ! -f $(SRCFILE)/mkntfs ] || cp -f $(SRCFILE)/mkntfs $(INSTALLDIR)/usr/sbin || chmod 775 $(INSTALLDIR)/usr/sbin/mkntfs
endif
endif
ifeq ($(RTCONFIG_PARAGON_HFS),y)
	@[ ! -f $(SRCFILE)/chkhfs ] || cp -f $(SRCFILE)/chkhfs $(INSTALLDIR)/usr/sbin || chmod 775 $(INSTALLDIR)/usr/sbin/chkhfs
ifneq ($(NO_MKTOOLS),y)
	@[ ! -f $(SRCFILE)/mkhfs ] || cp -f $(SRCFILE)/mkhfs $(INSTALLDIR)/usr/sbin || chmod 775 $(INSTALLDIR)/usr/sbin/mkhfs
endif
endif

.phony: all clean install
