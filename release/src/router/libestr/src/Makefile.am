
AM_CPPFLAGS = -I${top_srcdir}/include
AM_CFLAGS = ${my_CFLAGS}

DEBUG = -g

# Uncomment for debugging TODO: configure option
#AM_CFLAGS += $(DEBUG)

lib_LTLIBRARIES = libestr.la

libestr_la_SOURCES = \
	libestr.c \
	string.c

libestr_la_LIBADD = 
libestr_la_LDFLAGS = -version-info 0:0:0

include_HEADERS = 
