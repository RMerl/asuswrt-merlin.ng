#
# Makefile
#
# Version:      $Id$
#
TARGET		:= libfreeradius-radius.a

SOURCES		:= dict.c filters.c hash.c hmac.c hmacsha1.c isaac.c log.c \
		  misc.c missing.c md4.c md5.c print.c radius.c rbtree.c \
		  sha1.c snprintf.c strlcat.c strlcpy.c token.c udpfromto.c \
		  valuepair.c fifo.c packet.c event.c getaddrinfo.c \
		  heap.c tcp.c base64.c

SRC_CFLAGS	:= -D_LIBRADIUS -I$(top_builddir)/src

# System libraries discovered by our top level configure script, links things
# like pthread and the regexp libraries.
TGT_LDLIBS	:= $(LIBS)
