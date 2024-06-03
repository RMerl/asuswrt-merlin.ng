/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2015-2017, STMicroelectronics - All Rights Reserved
 */

/* ID = Device Version (bit31:16) + Device Part Number (RPN) (bit15:0)*/
#define CPU_STM32MP157Cxx	0x05000000
#define CPU_STM32MP157Axx	0x05000001
#define CPU_STM32MP153Cxx	0x05000024
#define CPU_STM32MP153Axx	0x05000025
#define CPU_STM32MP151Cxx	0x0500002E
#define CPU_STM32MP151Axx	0x0500002F

/* return CPU_STMP32MP...Xxx constants */
u32 get_cpu_type(void);

#define CPU_REVA	0x1000
#define CPU_REVB	0x2000

/* return CPU_REV constants */
u32 get_cpu_rev(void);
/* return boot mode */
u32 get_bootmode(void);
