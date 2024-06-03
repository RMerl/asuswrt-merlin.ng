# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
# Script to convert coreboot code to something similar to what U-Boot uses
# sed -f coreboot.sed <coreboot_file.c>
# Remember to add attribution to coreboot for new files added to U-Boot.
s/REG_RES_WRITE32(\(.*\), \(.*\), \(.*\)),/writel(\3, base + \2);/
s/REG_RES_POLL32(\(.*\), \(.*\), \(.*\), \(.*\), \(.*\)),/ret = poll32(base + \2, \3, \4, \5);/
s/REG_RES_OR32(\(.*\), \(.*\), \(.*\)),/setbits_le32(base + \2, \3);/
s/REG_RES_RMW32(\(.*\), \(.*\), \(.*\), \(.*\)),/clrsetbits_le32(base + \2, ~\3, \4);/
/REG_SCRIPT_END/d
s/read32/readl/
s/write32(\(.*\), \(.*\))/writel(\2, \1)/
s/conf->/plat->/
s/static const struct reg_script \(.*\)_script\[\] = {/static int \1(struct udevice *dev)/
