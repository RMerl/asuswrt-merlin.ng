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
# Copyright 2014, ASUSTeK Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

PWD=$(shell pwd)

all:
ifeq ($(MT798X),y)
	$(MAKE) -C $(LINUXDIR) CROSS_COMPILE=$(patsubst %-gcc,%-,$(KERNELCC)) EXTRA_CFLAGS=-I$(TOP)/shared M=$(PWD)/driver modules
else
	$(MAKE) -C $(LINUXDIR) CROSS_COMPILE=$(patsubst %-gcc,%-,$(KERNELCC)) EXTRA_CFLAGS=-I$(TOP)/shared SUBDIRS=$(PWD)/driver modules
endif

install:
ifeq ($(MT798X),y)
	$(MAKE) -C $(LINUXDIR) CROSS_COMPILE=$(patsubst %-gcc,%-,$(KERNELCC)) EXTRA_CFLAGS=-I$(TOP)/shared M=$(PWD)/driver INSTALL_MOD_PATH=$(INSTALLDIR) modules_install
else
	$(MAKE) -C $(LINUXDIR) CROSS_COMPILE=$(patsubst %-gcc,%-,$(KERNELCC)) EXTRA_CFLAGS=-I$(TOP)/shared SUBDIRS=$(PWD)/driver INSTALL_MOD_PATH=$(INSTALLDIR) modules_install
endif
	@find $(INSTALLDIR) -name "modules.*" | xargs rm -f
	@find $(INSTALLDIR) -name "*.ko" | xargs $(STRIPX)

clean:
	$(RM) *.o *.ko driver/*.o driver/*.ko driver/*.mod.*
