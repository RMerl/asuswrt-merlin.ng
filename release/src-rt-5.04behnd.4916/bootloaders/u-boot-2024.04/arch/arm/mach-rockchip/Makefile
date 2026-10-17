# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2014 Google, Inc
# Copyright (c) 2019 Rockchip Electronics Co., Ltd.

# We don't want the bootrom-helper present in a full U-Boot build, as
# this may have entered from ATF with the stack-pointer pointing to
# inaccessible/protected memory (and the bootrom-helper assumes that
# the stack-pointer is valid before switching to the U-Boot stack).
obj-spl-$(CONFIG_ROCKCHIP_BROM_HELPER) += bootrom.o
obj-spl-$(CONFIG_SPL_ROCKCHIP_COMMON_BOARD) += spl.o spl-boot-order.o
obj-tpl-$(CONFIG_ROCKCHIP_BROM_HELPER) += bootrom.o
obj-tpl-$(CONFIG_TPL_ROCKCHIP_COMMON_BOARD) += tpl.o
obj-tpl-$(CONFIG_ROCKCHIP_PX30) += px30-board-tpl.o

obj-spl-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board-spl.o

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)

# Always include boot_mode.o, as we bypass it (i.e. turn it off)
# inside of boot_mode.c when CONFIG_ROCKCHIP_BOOT_MODE_REG is 0.  This way,
# we can have the preprocessor correctly recognise both 0x0 and 0
# meaning "turn it off".
obj-y += boot_mode.o
obj-$(CONFIG_ROCKCHIP_COMMON_BOARD) += board.o
obj-$(CONFIG_MISC_INIT_R) += misc.o
endif

ifeq ($(CONFIG_TPL_BUILD),)
obj-$(CONFIG_DISPLAY_CPUINFO) += cpu-info.o
endif

obj-$(CONFIG_$(SPL_TPL_)RAM) += sdram.o

obj-$(CONFIG_ROCKCHIP_PX30) += px30/
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036/
obj-$(CONFIG_ROCKCHIP_RK3066) += rk3066/
obj-$(CONFIG_ROCKCHIP_RK3128) += rk3128/
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188/
obj-$(CONFIG_ROCKCHIP_RK322X) += rk322x/
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288/
obj-$(CONFIG_ROCKCHIP_RK3308) += rk3308/
obj-$(CONFIG_ROCKCHIP_RK3328) += rk3328/
obj-$(CONFIG_ROCKCHIP_RK3368) += rk3368/
obj-$(CONFIG_ROCKCHIP_RK3399) += rk3399/
obj-$(CONFIG_ROCKCHIP_RK3568) += rk3568/
obj-$(CONFIG_ROCKCHIP_RK3588) += rk3588/
obj-$(CONFIG_ROCKCHIP_RV1108) += rv1108/
obj-$(CONFIG_ROCKCHIP_RV1126) += rv1126/

# Clear out SPL objects, in case this is a TPL build
obj-spl-$(CONFIG_TPL_BUILD) =

# Now add SPL/TPL objects back into the main build
obj-$(CONFIG_SPL_BUILD) += $(obj-spl-y)
obj-$(CONFIG_TPL_BUILD) += $(obj-tpl-y)
