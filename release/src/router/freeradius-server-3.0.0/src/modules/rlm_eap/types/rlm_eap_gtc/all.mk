TARGETNAME	:= rlm_eap_gtc

ifneq "$(TARGETNAME)" ""
TARGET		:= $(TARGETNAME).a
endif

SOURCES		:= $(TARGETNAME).c

SRC_CFLAGS	:=
TGT_LDLIBS	:=
SRC_INCDIRS	:= ../../ ../../libeap/
TGT_PREREQS	:= libfreeradius-eap.a
