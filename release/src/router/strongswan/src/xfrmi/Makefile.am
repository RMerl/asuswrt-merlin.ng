ipsec_PROGRAMS = xfrmi

xfrmi_SOURCES = xfrmi.c

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libcharon/plugins/kernel_netlink

xfrmi_LDADD = \
	$(top_builddir)/src/libcharon/plugins/kernel_netlink/libstrongswan-kernel-netlink.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libcharon/libcharon.la
