#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

BINS	= mkimage
OBJS	= mkimage.o 

#
# Use native tools and options
#
CPPFLAGS   = -idirafter ./include -DTEXT_BASE=$(TEXT_BASE) -DUSE_HOSTCC
CFLAGS     = $(HOST_CFLAGS) $(CPPFLAGS) -O
AFLAGS	   = -D__ASSEMBLY__ $(CPPFLAGS)
MAKEDEPEND = makedepend
CC	   = gcc
STRIP	   = strip

all:	.depend $(BINS) 

mkimage:mkimage.o crc32.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

crc32.o: crc32.c	
	$(CC) $(CFLAGS) -c $<

mkimage.o: mkimage.c
	$(CC) $(CFLAGS) -c $<

.depend: Makefile $(OBJS:.o=.c)
	$(CC) -M $(HOST_CFLAGS) $(CPPFLAGS) $(OBJS:.o=.c) > $@

clean:
	rm -f *.o mkimage


