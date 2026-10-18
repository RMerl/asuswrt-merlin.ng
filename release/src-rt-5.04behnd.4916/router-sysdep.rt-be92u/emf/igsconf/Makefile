#
# Copyright (C) 2025, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual:>>
#
# $Id: Makefile 832689 2023-11-10 16:41:06Z $
#

CROSS =
CC = $(CROSS)gcc
AR = $(CROSS)ar
STRIP = $(CROSS)strip

ALL_OBJS = igsu.o igsu_linux.o

CCFLAGS = -Os -Wall -I$(SRCBASE)/include ${WLAN_StdIncPathA} ${WLAN_ComponentIncPathA} -I$(SRCBASE)/common/include -I$(SRCBASE_ROUTER)/shared -I$(HND_SRC)/router-sysdep/bcmdrv/include/emf/igs/

CCFLAGS += $(USR_CFLAGS)

ifneq ($(wildcard ./prebuilt),)
all:
	-cp prebuilt/igs igs
else
all: igs
endif

igs: $(ALL_OBJS)
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $@ $(ALL_OBJS) $(LDLIBS)
	$(STRIP) $@

$(ALL_OBJS): %.o: %.c
	@echo " "
	@echo "Making $<"
	$(CC) $(CCFLAGS) -c $<

install: all
	install -d $(INSTALLDIR)/igs/usr/sbin
	install -m 755 igs $(INSTALLDIR)/igs/usr/sbin

.PHONY:	clean
clean:
	-rm -f *.o *~ core igs

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: makefile-gmake
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 noet
