# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# Authors:
#   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>

# build the generic FF-A methods
obj-y += arm-ffa-uclass.o
ifeq ($(CONFIG_SANDBOX),y)
# build the FF-A sandbox emulator and driver
obj-y += ffa-emul-uclass.o sandbox_ffa.o
else
# build the Arm64 FF-A driver
obj-y += arm-ffa.o
endif
