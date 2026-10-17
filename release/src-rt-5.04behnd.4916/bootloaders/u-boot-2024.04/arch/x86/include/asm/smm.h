/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SMM definitions (U-Boot does not support SMM itself)
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot smm.h
 */

#ifndef _ASM_SMM_H
#define _ASM_SMM_H

#define APM_CNT			0xb2
#define APM_CNT_CST_CONTROL	0x85
#define APM_CNT_PST_CONTROL	0x80
#define APM_CNT_ACPI_DISABLE	0x1e
#define APM_CNT_ACPI_ENABLE	0xe1
#define APM_CNT_MBI_UPDATE	0xeb
#define APM_CNT_GNVS_UPDATE	0xea
#define APM_CNT_FINALIZE	0xcb
#define APM_CNT_LEGACY		0xcc
#define APM_CNT_SMMSTORE	0xed
#define APM_CNT_ELOG_GSMI	0xef
#define APM_STS			0xb3

#endif /* _ASM_SMM_H */
