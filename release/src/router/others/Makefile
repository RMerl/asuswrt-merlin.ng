include ../common.mak

all:

install:
	install -d $(INSTALLDIR)/sbin
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/lib

# K3 wl setup script
	install -m 0700 wlk $(INSTALLDIR)/usr/sbin

ifeq ($(RTCONFIG_USB),y)
# Optware perl symlink
	install -d $(INSTALLDIR)/usr/bin
	ln -sf /opt/bin/perl $(INSTALLDIR)/usr/bin/perl
endif

# cron helper
	install -m 0700 cru $(INSTALLDIR)/usr/sbin

# playing with ipkg / from OpenWrt svn
#!!TB	install -m 0700 ipkg $(INSTALLDIR)/usr/sbin

# system info
	install -m 0700 sysinfo $(INSTALLDIR)/usr/sbin

# keep winscp happy
#	install -m 0700 groups $(INSTALLDIR)/usr/bin

# chpasswd script
	install -m 0700 chpasswd.sh $(INSTALLDIR)/usr/sbin

# OpenVPN
ifeq ($(RTCONFIG_OPENVPN),y)
	install -m 0700 setuprsa.sh $(INSTALLDIR)/usr/sbin
endif

# Postconf helper script
	install -m 0755 helper.sh $(INSTALLDIR)/usr/sbin

# amtm script
	install -m 0700 amtm $(INSTALLDIR)/usr/sbin

ifeq ($(RTN14U)$(RTAC52U)$(RTAC51U)$(RTN11P)$(RTN300)$(RTN54U)$(RTAC1200HP)$(RTN56UB1)$(RTN56UB2)$(RTAC54U)$(RTAC1200)$(RTAC1200V2)$(RTAC1200GA1)$(RTAC1200GU)$(RTN11PB1)$(RTAC85U)$(RTAC85P)$(RPAC87)$(RTN800HP)$(RTACRH26),y)
# for read/write Ralink PHY registers
	install -D mii_mgr $(INSTALLDIR)/bin/mii_mgr
endif

ifeq ($(RTCONFIG_RALINK),y)
# for adjust smp_affinity and rps_cpus
ifeq ($(RTCONFIG_RALINK_MT7621),y)
ifeq ($(RTAC1200GU),y)
	install smp_4300.sh $(INSTALLDIR)/sbin/smp.sh
else ifeq ($(RTAC85U)$(RTAC85P)$(RPAC87)$(RTN800HP)$(RTACRH26),y)
	install smp_5010.sh $(INSTALLDIR)/sbin/smp.sh
endif
endif
endif

# Temporary fix for missing ld.so.cache
ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX_675X),y)
	install ldconfig_armv7 $(INSTALLDIR)/sbin/ldconfig
else
	install ldconfig_armv8 $(INSTALLDIR)/sbin/ldconfig
endif
endif

clean:
