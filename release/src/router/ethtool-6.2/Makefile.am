AM_CFLAGS = -Wall -Wextra -D_POSIX_C_SOURCE=200809L
AM_CPPFLAGS = -I$(top_srcdir)/uapi
LDADD = -lm

man_MANS = ethtool.8
EXTRA_DIST = LICENSE ethtool.8 ethtool.spec.in aclocal.m4 ChangeLog autogen.sh

sbin_PROGRAMS = ethtool
ethtool_SOURCES = ethtool.c uapi/linux/ethtool.h internal.h \
		  uapi/linux/net_tstamp.h uapi/linux/if.h uapi/linux/hdlc/ioctl.h \
		  rxclass.c common.c common.h \
		  json_writer.c json_writer.h json_print.c json_print.h \
		  list.h
if ETHTOOL_ENABLE_PRETTY_DUMP
ethtool_SOURCES += \
		  amd8111e.c de2104x.c dsa.c e100.c e1000.c et131x.c igb.c	\
		  fec.c fec_8xx.c fsl_enetc.c ibm_emac.c ixgb.c ixgbe.c \
		  natsemi.c pcnet32.c realtek.c tg3.c marvell.c vioc.c \
		  smsc911x.c at76c50x-usb.c sfc.c stmmac.c	\
		  sff-common.c sff-common.h sfpid.c sfpdiag.c	\
		  ixgbevf.c tse.c vmxnet3.c qsfp.c qsfp.h fjes.c lan78xx.c \
		  igc.c cmis.c cmis.h bnxt.c cpsw.c lan743x.c
endif

if ENABLE_BASH_COMPLETION
bashcompletiondir = $(BASH_COMPLETION_DIR)
dist_bashcompletion_DATA = shell-completion/bash/ethtool
endif

if ETHTOOL_ENABLE_NETLINK
ethtool_SOURCES += \
		  netlink/netlink.c netlink/netlink.h netlink/extapi.h \
		  netlink/msgbuff.c netlink/msgbuff.h netlink/nlsock.c \
		  netlink/nlsock.h netlink/strset.c netlink/strset.h \
		  netlink/monitor.c netlink/bitset.c netlink/bitset.h \
		  netlink/settings.c netlink/parser.c netlink/parser.h \
		  netlink/permaddr.c netlink/prettymsg.c netlink/prettymsg.h \
		  netlink/features.c netlink/privflags.c netlink/rings.c \
		  netlink/channels.c netlink/coalesce.c netlink/pause.c \
		  netlink/eee.c netlink/tsinfo.c netlink/fec.c \
		  netlink/stats.c \
		  netlink/desc-ethtool.c netlink/desc-genlctrl.c \
		  netlink/module-eeprom.c netlink/module.c netlink/rss.c \
		  netlink/desc-rtnl.c netlink/cable_test.c netlink/tunnels.c \
		  uapi/linux/ethtool_netlink.h \
		  uapi/linux/netlink.h uapi/linux/genetlink.h \
		  uapi/linux/rtnetlink.h uapi/linux/if_link.h \
		  uapi/linux/if.h uapi/linux/hdlc/ioctl.h
AM_CPPFLAGS += @MNL_CFLAGS@
LDADD += @MNL_LIBS@
endif

TESTS = test-cmdline
check_PROGRAMS = test-cmdline
test_cmdline_SOURCES = test-cmdline.c test-common.c $(ethtool_SOURCES) 
test_cmdline_CFLAGS = -DTEST_ETHTOOL
if !ETHTOOL_ENABLE_NETLINK
TESTS += test-features
check_PROGRAMS += test-features
test_features_SOURCES = test-features.c test-common.c $(ethtool_SOURCES) 
test_features_CFLAGS = -DTEST_ETHTOOL
endif

dist-hook:
	cp $(top_srcdir)/ethtool.spec $(distdir)

