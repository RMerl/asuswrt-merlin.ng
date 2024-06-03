#***********************************************************************
#
#  Copyright (c) 2013  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

#
# THIS MAKEFILE IS NOT CALLED DURING A NORMAL BUILD!
# this makefile is a convinience which allows you to run make from the
# userspace/private directory.

.PHONY: all

all :
	$(MAKE) -C .. private-libs private-apps

clean: 
	$(MAKE) -C libs clean
	$(MAKE) -C apps clean


