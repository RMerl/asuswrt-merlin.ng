ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
include Kbuild

else
# normal makefile
ifeq ($(KDIR),)
  $(error Please set KDIR to point to your kernel build directory)
endif

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif
