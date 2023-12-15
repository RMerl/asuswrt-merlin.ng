noinst_LIBRARIES = libfragroute.a
libfragroute_a_SOURCES = fragroute.c bget.c mod.c pkt.c argv.c \
						 randutil.c mod_delay.c mod_drop.c mod_dup.c \
						 mod_echo.c mod_ip_chaff.c mod_ip_frag.c mod_ip_opt.c \
						 mod_ip_ttl.c mod_ip_tos.c mod_order.c mod_print.c \
						 mod_tcp_chaff.c mod_tcp_opt.c mod_tcp_seg.c \
						 iputil.c mod_ip6_opt.c mod_ip6_qos.c


libfragroute_a_CFLAGS = -I$(srcdir)/.. -I$(srcdir)/../.. @LDNETINC@

# libfragroute_a_LIBS = @LDNETLIB@

noinst_HEADERS = bget.h mod.h pkt.h randutil.h iputil.h fragroute.h argv.h \
				 LICENSE README

MOSTLYCLEANFILES = *~

MAINTAINERCLEANFILES = Makefiles.in
