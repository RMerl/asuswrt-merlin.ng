/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/acpi/sleepstates.asl
 */

Name(\_S0, Package(){0x0,0x0,0x0,0x0})
#if !IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)
Name(\_S1, Package(){0x1,0x0,0x0,0x0})
#else
Name(\_S3, Package(){0x5,0x0,0x0,0x0})
#endif
Name(\_S4, Package(){0x6,0x0,0x0,0x0})
Name(\_S5, Package(){0x7,0x0,0x0,0x0})
