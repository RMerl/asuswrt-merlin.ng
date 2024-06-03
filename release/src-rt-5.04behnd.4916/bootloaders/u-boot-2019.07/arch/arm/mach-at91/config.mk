ifeq ($(CONFIG_CPU_ARM926EJS),y)
PLATFORM_CPPFLAGS += $(call cc-option,-mtune=arm926ejs,)
endif

ifeq ($(CONFIG_CPU_V7A),y)
ifndef CONFIG_SPL_BUILD
ALL-y	+= u-boot.img
endif
endif
