ifndef IP_SET_MAX
$(error "Use the toplevel Makefile, not the ones in the kernel/ subdirectory")
endif

ifneq ($(KERNELRELEASE),)
include Kbuild
else
KERNELDIR := /lib/modules/`uname -r`/build
all::
	$(MAKE) -C $(KERNELDIR) M=`pwd` $@
endif
