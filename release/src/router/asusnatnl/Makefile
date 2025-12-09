include ../common.mak
CFLAGS += $(if $(RTCONFIG_DEBUG),-g)
#CFLAGS += -Werror
#CXXFLAGS += -Werror


# check if 64bit
CHECK_64BIT := $(shell $(CC) -dM -E - < /dev/null | grep -q '__x86_64__\|__aarch64__\|__PPC64__\|__sparc64__\|__ia64__\|__mips64' && echo 1 || echo 0)
ifeq ($(or $(RTCONFIG_BCMARM),$(RTCONFIG_QCA_ARM),$(RTCONFIG_ALPINE),$(MUSL_LIBC)),y)
	ifeq ($(CHECK_64BIT), 1)
		CONFIGURE=configure-router-arm64
		ARCH=ARM
	else
		CONFIGURE=configure-router-arm
		ARCH=ARM64
	endif
else
CONFIGURE=configure-router-mips
ARCH=MIPS
endif

all: pjproject-1.12/stamp-h1-$(ARCH)
	$(MAKE) -C pjproject-1.12 UCLIBC_NG=$(UCLIBC_NG)
	$(MAKE) -C udt os=LINUX arch=$(ARCH)
	$(MAKE) -C natnl linux-lib router=yes RTCONFIG_SW_HW_AUTH=$(RTCONFIG_SW_HW_AUTH)
#	$(MAKE) -C natnl/msvc_prjs/asusnatnl_test

clean:
	-$(MAKE) -C pjproject-1.12 distclean
	-$(MAKE) -C udt clean
	-$(MAKE) -C natnl clean
	@rm -f pjproject-1.12/stamp-h1-*

pjproject-1.12/stamp-h1-%:
	cd pjproject-1.12 && AR="$(AR) -r -v" \
	./$(CONFIGURE)
	@touch $@
