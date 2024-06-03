#
# THIS MAKEFILE IS NOT CALLED DURING A NORMAL BUILD!
# this makefile is a convinience which allows you to run make from the
# userspace/public directory.

.PHONY: all

all :
	$(MAKE) -C .. public-include public-libs public-apps

clean: 
	$(MAKE) -C libs clean
	$(MAKE) -C apps clean
	$(MAKE) -C include clean



