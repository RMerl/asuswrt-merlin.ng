TARGETNAME	:= rlm_eap_peap

ifneq "$(OPENSSL_LIBS)" ""
TARGET		:= $(TARGETNAME).a
endif

SOURCES		:= $(TARGETNAME).c peap.c

SRC_CFLAGS	:=
TGT_LDLIBS	:=
TGT_LDLIBS	+= $(OPENSSL_LIBS)

SRC_INCDIRS	:= ../../ ../../libeap/
TGT_PREREQS	:= libfreeradius-eap.a
