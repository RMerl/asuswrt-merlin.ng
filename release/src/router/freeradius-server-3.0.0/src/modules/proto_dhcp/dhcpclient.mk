TARGET		:= dhcpclient
SOURCES		:= dhcpclient.c dhcp.c

TGT_PREREQS	:= libfreeradius-radius.a
TGT_LDLIBS	:= $(LIBS)
