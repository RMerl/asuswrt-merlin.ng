/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */
#ifndef _ASM_ARCH_SPL_H_
#define _ASM_ARCH_SPL_H_

#ifdef CONFIG_SOC_K3_AM654
#include "am6_spl.h"
#endif

#ifdef CONFIG_SOC_K3_J721E
#include "j721e_spl.h"
#endif

#ifdef CONFIG_SOC_K3_J721S2
#include "j721s2_spl.h"
#endif

#ifdef CONFIG_SOC_K3_AM642
#include "am64_spl.h"
#endif

#ifdef CONFIG_SOC_K3_AM625
#include "am62_spl.h"
#endif

#ifdef CONFIG_SOC_K3_AM62A7
#include "am62a_spl.h"
#endif

#endif /* _ASM_ARCH_SPL_H_ */
