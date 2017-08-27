#
# Extra run-time libraries
#
# Note: this file is only used if LIBOPT=n, called from libcreduction Makefile
# 
# Copyright (C) 2004 Broadcom Corporation
#


TARGETS := $(LIBDIR)/$(LIBC) $(LIBDIR)/$(LINKER)

ifneq ($(strip $(BRCM_UCLIBC)),)

# Extra libs required for uClibc
	  
TARGETS += $(LIBDIR)/libresolv.so.0
TARGETS += $(EXTRALIBDIR)/libgcc_s.so.1

TARGETS += $(LIBDIR)/libcrypt.so.0
TARGETS += $(LIBDIR)/libutil.so.0
TARGETS += $(LIBDIR)/libm.so.0
TARGETS += $(LIBDIR)/librt.so.0
TARGETS += $(LIBDIR)/libpthread.so.0


#
# These following libraries will only be included if certain features are
# enabled.
#

INCLUDE_LIBTHREAD := n

ifneq ($(strip $(BUILD_GDBSERVER)),)
  INCLUDE_LIBTHREAD := y
endif

ifeq ($(strip $(INCLUDE_LIBTHREAD)),y)
  TARGETS += $(LIBDIR)/libthread_db.so.1
endif

else

# Extra libs required for glibc
	
TARGETS += $(LIBDIR)/libresolv.so.2
TARGETS += $(EXTRALIBDIR)/libgcc_s.so.1

TARGETS += $(LIBDIR)/libcrypt.so.1
TARGETS += $(LIBDIR)/libutil.so.1
TARGETS += $(LIBDIR)/libm.so.6
TARGETS += $(LIBDIR)/librt.so.1
TARGETS += $(LIBDIR)/libpthread.so.0

TARGETS += $(LIBDIR)/libnss_dns.so.2
TARGETS += $(LIBDIR)/libnss_files.so.2

#
# These following libraries will only be included if certain features are
# enabled.
#

INCLUDE_LIBTHREAD := n
INCLIDE_LIBNSL := n


ifneq ($(strip $(BUILD_GDBSERVER)),)
  INCLUDE_LIBTHREAD := y
endif

ifneq ($(strip $(BUILD_SAMBA)),)
  INCLUDE_LIBNSL := y
endif

ifeq ($(strip $(INCLUDE_LIBTHREAD)),y)
  TARGETS += $(LIBDIR)/libthread_db.so.1
endif

ifeq ($(strip $(INCLUDE_LIBNSL)),y)
  TARGETS += $(LIBDIR)/libnsl.so.1
endif

endif
