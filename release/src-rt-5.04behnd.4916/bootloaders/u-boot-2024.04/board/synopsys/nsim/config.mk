# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2020 Synopsys, Inc. All rights reserved.

# CONFIG_NSIM_BOARD_CPPFLAGS is a string variable which comes from defconfig
# with double quotes. We use echo to remove them so CONFIG_NSIM_BOARD_CPPFLAGS
# won't be treated by compiler as a single option.
PLATFORM_CPPFLAGS += $(shell echo $(CONFIG_NSIM_BOARD_CPPFLAGS))
