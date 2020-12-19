include ../common.mak

include $(SRCBASE)/.config

model = $(subst -,,$(subst +,P,$(BUILD_NAME)))
modelstr = $(shell echo $(model) | tr '[:upper:]' '[:lower:]')

all:

install: all
	install -d $(INSTALLDIR)/usr/sbin
ifneq ($(HND_ROUTER),y)
	install lld2d $(INSTALLDIR)/usr/sbin
else
ifeq ($(HND_ROUTER_AX_675X),y)
	install lld2d.6755axhnd $(INSTALLDIR)/usr/sbin/lld2d
else
	install lld2d.hnd $(INSTALLDIR)/usr/sbin/lld2d
endif
endif
ifeq ($(BUILD_NAME),RT-AC3200)
	install icon.rtac3200.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac3200.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC56U)
	install icon.rtac56u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac56u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC87U)
	install icon.rtac87u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac87u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC68U)
	mkdir -p $(INSTALLDIR)/usr/sbin/lltd
	install icon.rtac68u.ico $(INSTALLDIR)/usr/sbin/lltd/icon.ico
	install icon.large.rtac68u.ico $(INSTALLDIR)/usr/sbin/lltd/icon.large.ico
	install icon.rtac66u_v2.ico $(INSTALLDIR)/usr/sbin/lltd/icon_alt.ico
	install icon.large.rtac66u_v2.ico $(INSTALLDIR)/usr/sbin/lltd/icon_alt.large.ico
	install icon.rpac1900.ico $(INSTALLDIR)/usr/sbin/lltd/icon_alt2.ico
	install icon.large.rpac1900.ico $(INSTALLDIR)/usr/sbin/lltd/icon_alt2.large.ico
else ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68A 4G-AC68U))
	install icon.rtac68u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac68u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC88U)
	install icon.rtac88u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac88u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC86U)
	install icon.rtac86u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac86u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),GT-AC2900)
	install icon.gtac2900.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.gtac2900.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC3100)
	install icon.rtac3100.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac3100.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC5300)
	install icon.rtac5300.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac5300.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),GT-AC5300)
	install icon.rtac5300.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac5300.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX88U)
	install icon.rtax88u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax88u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), GT-AX11000)
	install icon.gtax11000.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.gtax11000.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX92U)
	install icon.rtax92u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax92u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX95Q)
	install icon.rtax95q.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax95q.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX56_XD4)
	install icon.rtax56_xd4.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax56_xd4.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), CT-AX56_XD4)
	install icon.ctax56_xd4.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.ctax56_xd4.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX58U)
	install icon.rtax58u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax58u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX56U)
	install icon.rtax56u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax56u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), RT-AX68U)
	install icon.rtax68u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtax68u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico	
else ifeq ($(BUILD_NAME),RT-AC1200G)
	install icon.rtac1200g.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac1200g.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AC1200G+)
	install icon.rtac1200g+.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac1200g+.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),DSL-AC68U)
	install icon.rtac68u.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.rtac68u.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME), GT-AXE11000)
	install icon.gtaxe11000.ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.gtaxe11000.ico $(INSTALLDIR)/usr/sbin/icon.large.ico
else ifeq ($(BUILD_NAME),RT-AX82U)
	install icon.rtax82u.ico $(INSTALLDIR)/usr/sbin/icon_default.ico
	install icon.large.rtax82u.ico $(INSTALLDIR)/usr/sbin/icon_default.large.ico
	install icon.rtax82u_gd.ico $(INSTALLDIR)/usr/sbin/icon_gd.ico
	install icon.large.rtax82u_gd.ico $(INSTALLDIR)/usr/sbin/icon_gd.large.ico
else
	install icon.$(modelstr).ico $(INSTALLDIR)/usr/sbin/icon.ico
	install icon.large.$(modelstr).ico $(INSTALLDIR)/usr/sbin/icon.large.ico
endif
ifneq ($(RTCONFIG_BCMARM),y)
	install lld2d.conf $(INSTALLDIR)/usr/sbin
else
	cd $(INSTALLDIR)/usr/sbin && ln -sf /tmp/lld2d.conf lld2d.conf
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC68U RT-AX82U))
	cd $(INSTALLDIR)/usr/sbin && ln -sf /tmp/icon.ico icon.ico
	cd $(INSTALLDIR)/usr/sbin && ln -sf /tmp/icon.large.ico icon.large.ico
endif
endif

clean:
