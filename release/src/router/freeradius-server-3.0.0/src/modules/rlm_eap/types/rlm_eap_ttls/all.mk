TARGETNAME	:= rlm_eap_ttls

ifneq "$(OPENSSL_LIBS)" ""
TARGET		:= $(TARGETNAME).a
endif

SOURCES		:= $(TARGETNAME).c ttls.c

SRC_CFLAGS	:=
TGT_LDLIBS	:=
TGT_LDLIBS	+= $(OPENSSL_LIBS)

SRC_INCDIRS	:= ../../ ../../libeap/
TGT_PREREQS	:= libfreeradius-eap.a
