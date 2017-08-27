TARGET		:= radwho
SOURCES		:= radwho.c

TGT_PREREQS	:= libfreeradius-server.a libfreeradius-radius.a
TGT_LDLIBS	:= $(LIBS)
