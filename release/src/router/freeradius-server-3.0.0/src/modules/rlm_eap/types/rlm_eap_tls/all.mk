TARGETNAME	:= rlm_eap_tls

ifneq "$(OPENSSL_LIBS)" ""
TARGET		:= $(TARGETNAME).a
endif

SOURCES		:= $(TARGETNAME).c

TGT_LDLIBS	+= $(OPENSSL_LIBS)

SRC_INCDIRS	:= ../../ ../../libeap/
TGT_PREREQS	:= libfreeradius-eap.a
