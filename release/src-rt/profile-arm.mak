ifeq ($(RTCONFIG_ALPINE)$(RTCONFIG_LANTIQ),)
EXTRACFLAGS := -DLINUX26 -DCONFIG_BCMWL5 -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -DBCMWPA2
endif

ifeq ($(RTCONFIG_BCMARM),y)
EXTRACFLAGS += -DBCMARM -fno-strict-aliasing -marm
else ifeq ($(RTCONFIG_ALPINE),y)
else ifeq ($(RTCONFIG_LANTIQ),y)
else
EXTRACFLAGS += -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32
endif

ifeq ($(RTCONFIG_NVRAM_64K), y)
EXTRACFLAGS += -DRTCONFIG_NVRAM_64K
endif
ifeq ($(RTCONFIG_NV128), y)
EXTRACFLAGS += -DCONFIG_NVSIZE_128
endif

ifeq ($(RTCONFIG_ALPINE),y)
EXTRACFLAGS := -DCONFIG_ALPINE -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -funit-at-a-time -Wno-pointer-sign -marm
endif

ifeq ($(RTCONFIG_LANTIQ),y)
EXTRACFLAGS := -DCONFIG_LANTIQ -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -funit-at-a-time -Wno-pointer-sign -mips32r2 -mno-branch-likely -mtune=1004kc
endif

ifneq ($(findstring linux-3,$(LINUXDIR)),)
EXTRACFLAGS += -DLINUX30
endif

export EXTRACFLAGS
