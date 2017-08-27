TARGET		:= radconf2xml
SOURCES		:= radconf2xml.c

TGT_PREREQS	:= libfreeradius-server.a libfreeradius-radius.a
TGT_LDLIBS	:= $(LIBS)
