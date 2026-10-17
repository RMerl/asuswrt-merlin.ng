#
# Makefile for the BRCM DSL/PON voice slic power control driver
#

obj-$(CONFIG_BCM_VOICE_PWRCTL)	+= slic_pwrctl.o

subdir-ccflags-y += -Werror -Wfatal-errors

clean:
	rm -f core *.ko *.o *.a *.s
