ifeq ($(CONFIG_ARCH_OCTEONTX2),y)
PLATFORM_CPPFLAGS += $(call cc-option,-march=armv8.2-a,)
PLATFORM_CPPFLAGS += $(call cc-option,-mtune=octeontx2,)
endif
