#
# Copyright 2019 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# $Id: igs.mk 768736 2018-10-23 21:53:02Z $
#

CROSS_COMPILE = mipsel-linux-
KSRC := $(shell /bin/pwd)/../../..
KINCLUDE = -I$(KSRC)/include -I$(LINUXDIR)/include/asm/gcc \
	   -I$(KSRC)/router/emf/igs -I$(KSRC)/router/emf/emf

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

OFILES = igsc.o igs_linux.o igsc_sdb.o osl_linux.o

IFLAGS1 = -I$(LINUXDIR)/include
IFLAGS2 = $(KINCLUDE)
DFLAGS += -DMODULE -D__KERNEL__ $(IFLAGS) -G 0 -mno-abicalls \
	  -fno-pic -pipe -gstabs+ -mcpu=r4600 -mips2 -Wa,--trap \
	  -m4710a0kern -mlong-calls -fno-common -nostdinc \
	  -iwithprefix include

WFLAGS = $(IFLAGS1) -Wall -Wstrict-prototypes -Wno-trigraphs -O2 \
	 -fno-strict-aliasing -fno-common -fomit-frame-pointer $(IFLAGS2)

CFLAGS += $(WFLAGS) $(DFLAGS) $(IGSFLAGS)

TARGETS = igs.o

all: $(TARGETS)

igs.o: $(OFILES)
	$(LD) -r -o $@ $(OFILES)

clean:
	rm -f $(OFILES) $(TARGETS) *~

.PHONY: all clean

include $(LINUXDIR)/Rules.make
