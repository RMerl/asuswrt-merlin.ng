include $(TOP)/.config


obj-m := bled.o

bled-objs := common.o check-netdev.o check-interrupt.o

bled-objs += $(if $(RTCONFIG_USB),check-usbbus.o)
bled-objs += $(if $(CONFIG_GPIOLIB),gpio-gpiolib.o)

QCA8337_MODEL_LIST = $(addprefix _,$(addsuffix _,RT-AC55U RT-AC55UHP 4G-AC55U RT-N19 RT-AC59U RT-AC59U_V2 PL-AC56 PL-AC66U MAP-AC1750 RT-AC59_CD6R RT-AC59_CD6N GT-AXY16000 RT-AX89U GT-AX6000N))
QCA9531ESW_MODEL_LIST = $(addprefix _,$(addsuffix _,PL-N12))
MT7620ESW_MODEL_LIST = $(addprefix _,$(addsuffix _,RT-N11P RT-N300 RT-N14U RT-AC51U RT-AC52U RT-AC1200HP))
MT7621ESW_MODEL_LIST = $(addprefix _,$(addsuffix _,RT-N56UB1 RT-N56UB2 RT-AC85U RT-AC65U RT-N800HP))
MT7628ESW_MODEL_LIST = $(addprefix _,$(addsuffix _,RT-N11P_B1 RT-N10P_V3))
RTL8370M_MODEL_LIST = $(addprefix _,$(addsuffix _,BRT-AC828 RT-AC88Q RT-AC88S RT-AD7200))
DAKOTA_MODEL_LIST = $(addprefix _,$(addsuffix _,RT-AC58U 4G-AC53U RT-AC82U))
DUMMY_SWITCH_MODEL_LIST = $(addprefix _,$(addsuffix _,RT-AC88N PL-AX56_XP4))

_BUILD_NAME_ = $(addprefix _,$(addsuffix _,$(BUILD_NAME)))

# QCA platform
# AR8216 phy + QCA8337N switch
ifneq ($(findstring $(_BUILD_NAME_),$(QCA8337_MODEL_LIST)),)
bled-objs += check-ar8216-qca8337.o
endif

# QCA9531 embedded switch
ifneq ($(findstring $(_BUILD_NAME_),$(QCA9531ESW_MODEL_LIST)),)
bled-objs += check-ar7240-qca9531esw.o
endif

ifneq ($(findstring $(_BUILD_NAME_),$(DAKOTA_MODEL_LIST)),)
bled-objs += check-dakota.o
endif

# Ralink/MTK platform
# MT7620 embedded switch
ifneq ($(findstring $(_BUILD_NAME_),$(MT7620ESW_MODEL_LIST)),)
bled-objs += gpio-mt7620.o check-mt7620esw.o
endif

# MT7621 embedded switch
ifneq ($(findstring $(_BUILD_NAME_),$(MT7621ESW_MODEL_LIST)),)
bled-objs += gpio-mt7621.o check-mt7621esw.o
endif

# MT7628 embedded switch, exclude RT-N11P that uses MT7620ESW
ifeq ($(RTCONFIG_RALINK_MT7628),y)
ifneq ($(findstring $(_BUILD_NAME_),$(MT7628ESW_MODEL_LIST)),)
bled-objs += gpio-mt7628.o check-mt7628esw.o
endif
endif

# RTL8370M/RTL8370MB switch
ifneq ($(findstring $(_BUILD_NAME_),$(RTL8370M_MODEL_LIST)),)
bled-objs += check-rtl8370m.o
endif

# Dummy switch
bled-objs += $(if $(findstring $(_BUILD_NAME_),$(DUMMY_SWITCH_MODEL_LIST)),check-dummy.o)
