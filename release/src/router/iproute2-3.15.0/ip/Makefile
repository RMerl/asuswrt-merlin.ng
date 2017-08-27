IPOBJ=ip.o ipaddress.o ipaddrlabel.o iproute.o iprule.o ipnetns.o \
    rtm_map.o iptunnel.o ip6tunnel.o tunnel.o ipneigh.o ipntable.o iplink.o \
    ipmaddr.o ipmonitor.o ipmroute.o ipprefix.o iptuntap.o iptoken.o \
    ipxfrm.o xfrm_state.o xfrm_policy.o xfrm_monitor.o \
    iplink_vlan.o link_veth.o link_gre.o iplink_can.o \
    iplink_macvlan.o iplink_macvtap.o ipl2tp.o link_vti.o \
    iplink_vxlan.o tcp_metrics.o iplink_ipoib.o ipnetconf.o link_ip6tnl.o \
    link_iptnl.o link_gre6.o iplink_bond.o iplink_bond_slave.o iplink_hsr.o

RTMONOBJ=rtmon.o

include ../Config

ifeq ($(IP_CONFIG_SETNS),y)
	CFLAGS += -DHAVE_SETNS
endif

STATIC_SYM_FILTER:=
ifeq ($(IP_CONFIG_TINY),y)
  STATIC_SYM_FILTER:=iplink_can.c iplink_ipoib.c iplink_vxlan.c
  CFLAGS += -DIPROUTE2_TINY
endif
STATIC_SYM_SOURCES:=$(filter-out $(STATIC_SYM_FILTER),$(wildcard *.c))

ALLOBJ=$(IPOBJ) $(RTMONOBJ)
SCRIPTS=ifcfg rtpr routel routef
TARGETS=ip rtmon

all: $(TARGETS) $(SCRIPTS)

ip: $(IPOBJ) $(LIBNETLINK)

rtmon: $(RTMONOBJ)

install: all
	install -m 0755 $(TARGETS) $(DESTDIR)$(SBINDIR)
	install -m 0755 $(SCRIPTS) $(DESTDIR)$(SBINDIR)

clean:
	rm -f $(ALLOBJ) $(TARGETS)

SHARED_LIBS ?= y
ifeq ($(SHARED_LIBS),y)

LDLIBS += -ldl
LDFLAGS += -Wl,-export-dynamic

else

ip: static-syms.o
static-syms.o: static-syms.h
static-syms.h: $(STATIC_SYM_SOURCES)
	files="$^" ; \
	for s in `grep -B 3 '\<dlsym' $$files | sed -n '/snprintf/{s:.*"\([^"]*\)".*:\1:;s:%s::;p}'` ; do \
		sed -n '/'$$s'[^ ]* =/{s:.* \([^ ]*'$$s'[^ ]*\) .*:extern char \1[] __attribute__((weak)); if (!strcmp(sym, "\1")) return \1;:;p}' $$files ; \
	done > $@

endif
