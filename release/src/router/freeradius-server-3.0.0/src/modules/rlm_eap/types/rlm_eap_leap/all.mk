TARGETNAME	:= rlm_eap_leap

ifneq "$(TARGETNAME)" ""
TARGET		:= $(TARGETNAME).a
endif

SOURCES		:= $(TARGETNAME).c eap_leap.c smbdes.c

SRC_CFLAGS	:=
TGT_LDLIBS	:=
SRC_INCDIRS	:= ../../ ../../libeap/
TGT_PREREQS	:= libfreeradius-eap.a
