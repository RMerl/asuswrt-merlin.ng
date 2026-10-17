# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (C) 2020, STMicroelectronics - All Rights Reserved
#

obj-$(CONFIG_CMD_STBOARD) += cmd_stboard.o
obj-$(CONFIG_PMIC_STPMIC1) += stpmic1.o

ifeq ($(CONFIG_ARCH_STM32MP),y)
obj-$(CONFIG_SET_DFU_ALT_INFO) += stm32mp_dfu.o
obj-$(CONFIG_$(SPL_)DFU_VIRT) += stm32mp_dfu_virt.o
endif

obj-$(CONFIG_TYPEC_STUSB160X) += stusb160x.o
