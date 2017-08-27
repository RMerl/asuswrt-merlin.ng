TARGET		:= smbencrypt
SOURCES		:= smbencrypt.c smbdes.c

TGT_PREREQS	:= libfreeradius-radius.a
TGT_PRLIBS	:= ${LIBS}

SRC_CFLAGS	:= 
TGT_LDLIBS	:= $(LIBS)


