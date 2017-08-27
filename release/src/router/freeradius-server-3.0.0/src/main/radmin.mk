TARGET 		:= radmin

SOURCES		:= radmin.c

TGT_INSTALLDIR  := ${sbindir}
TGT_PREREQS	:= libfreeradius-server.a libfreeradius-radius.a
TGT_LDLIBS	:= $(LIBS) $(LIBREADLINE) -ltalloc
