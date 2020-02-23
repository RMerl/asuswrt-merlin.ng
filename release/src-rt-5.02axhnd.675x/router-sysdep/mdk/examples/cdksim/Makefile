# $Id: Makefile,v 1.4 Broadcom SDK $
# $Copyright: Copyright 2013 Broadcom Corporation.
# This program is the proprietary software of Broadcom Corporation
# and/or its licensors, and may only be used, duplicated, modified
# or distributed pursuant to the terms and conditions of a separate,
# written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized
# License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software
# and all intellectual property rights therein.  IF YOU HAVE
# NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
# IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
# ALL USE OF THE SOFTWARE.  
#  
# Except as expressly set forth in the Authorized License,
#  
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of
# Broadcom integrated circuit products.
#  
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
# PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
# OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
# INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
# ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
# TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
# THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
# ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
#
# Simple CDK shell (cdksim)
#
# General notes:
#
# This makefile is intended for POSIX compatible operating systems
# (e.g. Linux) with the GNU make utility (gmake) installed.
#
# The MDK environment variable must be set to point to the root
# of the MDK source files, e.g. (bash):
#
# export MDK=~/projects/broadcom/mdk
#
#
# Make options:
#
# NO_PERL
# If your host system (build platform) does not have Perl installed
# you must specify NO_PERL=1 to avoid build errors.
#
# SYS_NO_TTY
# If your target system does not provide a character-based terminal
# interface, you must specify SYS_NO_TTY=1. Note that this will also
# disable advanced command line editing and history.
#
#
# Example invokations:
#
# gmake
# gmake CFLAGS="-g" BLDDIR=`pwd`/linux64 NO_PERL=1
# gmake CFLAGS="-g" BLDDIR=`pwd`/linux64 SYS_NO_TTY=1
#
#

ifndef MDK
nocdk:; @echo 'The $$MDK environment variable is not set'; exit 1
else

ifndef CDK
CDK = $(MDK)/cdk
export CDK
endif

################################################################################
#
# Configure build environment
#
################################################################################

# Tools prefix
export CROSS_COMPILE =

# Build tools
COMPILER = gcc
LINKER   = ld
ARCHIVE  = ar

CC = $(CROSS_COMPILE)$(COMPILER)
LD = $(CROSS_COMPILE)$(LINKER)
AR = $(CROSS_COMPILE)$(ARCHIVE)

# Toolchain compiler flags
CFLAGS += -fPIC

# Make tools
MAKE = gmake
ECHO = echo

# Build directory
ifndef BLDDIR
BLDDIR = $(CURDIR)
endif

# Destination directory for application
ifndef DSTDIR
DSTDIR = $(BLDDIR)
endif

################################################################################
#
# Configure compiler and linker options
#
################################################################################

ifndef SYS_NO_TTY
CPPFLAGS += -DSYS_HAS_TTY
endif

# Configure include path
CPPFLAGS += -I$(CDK)/include -I$(CURDIR)

# Use CDK custom config file and system C library
# (see cdk_custom_config.h for details)
CPPFLAGS += -DCDK_INCLUDE_CUSTOM_CONFIG -DUSE_SYSTEM_LIBC

# Default application flags
CFLAGS += -Wall -Werror

################################################################################
#
# Configure CDK Libraries
#
#  1) Once you import $(CDK)/targets.mk, $(CDK_LIBS) will contain the list
#     of CDK libraries in correct link order.
#
#  2) To build the CDK, the following should be specified:
#
#     CDK_BLDDIR   - the directory where the libraries will be placed
#     CDK_CPPFLAGS - C preprocessor flags
#     CDK_CFLAGS   - C compiler flags
#     CDK_CC       - compiler
#     CDK_LD       - linker (currently not used)
#     CDK_AR       - archiver (for creating libraries)
#
################################################################################

# Check for Perl support
ifdef NO_PERL
export CDK_PERL   = n/a
endif
# Configure CDK libraries
export CDK_BLDDIR = $(BLDDIR)/cdk
export CDK_CFLAGS = $(CFLAGS)
export CDK_CPPFLAGS = $(CPPFLAGS)
# Configure CDK toolchain
export CDK_CC = $(CC)
export CDK_LD = $(LD)
export CDK_AR = $(AR)
# CDK_LIBS is defined in $(CDK)/targets.mk
include $(CDK)/targets.mk
CDK_BLIBS = $(addprefix $(CDK_BLDDIR)/,$(CDK_LIBS))

################################################################################
#
# Configure main application
#
################################################################################

OBJS = cdksim.o
BOBJS = $(addprefix $(BLDDIR)/,$(OBJS))
APPL = $(DSTDIR)/cdksim

################################################################################
#
# Make rules
#
################################################################################

.PHONY: all cdk clean cleanall

all: cdk $(APPL)

cdk:
	$(MAKE) -C $(CDK)

$(APPL): $(BOBJS) $(CDK_BLIBS)
	$(CC) $(BOBJS) $(CDK_BLIBS) $(LDFLAGS) -o $@

$(BLDDIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C $(CDK) clean
	rm -f $(APPL) $(BOBJS)

cleanall: clean
	$(MAKE) -C $(CDK) cleanpkgs

# endif $CDK
endif
