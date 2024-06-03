# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2000
# Murray Jensen <Murray.Jensen@csiro.au>

ifneq ($(HOSTOS),cygwin)

# Location of a usable BFD library, where we define "usable" as
# "built for ${HOST}, supports ${TARGET}".  Sensible values are
# - When cross-compiling: the root of the cross-environment
# - Linux/ppc (native): /usr
# - NetBSD/ppc (native): you lose ... (must extract these from the
#   binutils build directory, plus the native and U-Boot include
#   files don't like each other)

ifeq ($(HOSTOS),darwin)
BFD_ROOT_DIR =		/usr/local/tools
else
ifeq ($(HOSTARCH),$(ARCH))
# native
BFD_ROOT_DIR =		/usr
else
#BFD_ROOT_DIR =		/LinuxPPC/CDK		# Linux/i386
#BFD_ROOT_DIR =		/usr/pkg/cross		# NetBSD/i386
BFD_ROOT_DIR =		/opt/powerpc
endif
endif

#
# Use native tools and options
#
HOST_EXTRACFLAGS := -I$(BFD_ROOT_DIR)/include -pedantic

hostprogs-y := gdbsend gdbcont

gdbsend-objs := gdbsend.o error.o remote.o serial.o
gdbcont-objs := gdbcont.o error.o remote.o serial.o

always := $(hostprogs-y)

endif	# cygwin
