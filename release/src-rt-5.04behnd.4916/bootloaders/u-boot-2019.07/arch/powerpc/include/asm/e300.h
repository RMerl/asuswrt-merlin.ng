/*
 * Copyright 2004 Freescale Semiconductor, Inc.
 * Liberty Eran (liberty@freescale.com)
 */

#ifndef	__E300_H__
#define __E300_H__

#define PVR_E300C1	0x80830000
#define PVR_E300C2	0x80840000
#define PVR_E300C3	0x80850000
#define PVR_E300C4	0x80860000

/*
 * Hardware Implementation-Dependent Register 0 (HID0)
 */

/* #define HID0 1008 already defined in processor.h */
#define HID0_MASK_MACHINE_CHECK		     0x00000000
#define HID0_ENABLE_MACHINE_CHECK	     0x80000000

#define HID0_DISABLE_CACHE_PARITY	     0x00000000
#define HID0_ENABLE_CACHE_PARITY	     0x40000000

#define HID0_DISABLE_ADDRESS_PARITY	     0x00000000 /* on mpc8349ads must be disabled */
#define HID0_ENABLE_ADDRESS_PARITY	     0x20000000

#define HID0_DISABLE_DATA_PARITY	     0x00000000 /* on mpc8349ads must be disabled */
#define HID0_ENABLE_DATE_PARITY		     0x10000000

#define HID0_CORE_CLK_OUT		     0x00000000
#define HID0_CORE_CLK_OUT_DIV_2		     0x08000000

#define HID0_ENABLE_ARTRY_OUT_PRECHARGE      0x00000000 /* on mpc8349ads must be enabled */
#define HID0_DISABLE_ARTRY_OUT_PRECHARGE     0x01000000

#define HID0_DISABLE_DOSE_MODE		     0x00000000
#define HID0_ENABLE_DOSE_MODE		     0x00800000

#define HID0_DISABLE_NAP_MODE		     0x00000000
#define HID0_ENABLE_NAP_MODE		     0x00400000

#define HID0_DISABLE_SLEEP_MODE		     0x00000000
#define HID0_ENABLE_SLEEP_MODE		     0x00200000

#define HID0_DISABLE_DYNAMIC_POWER_MANAGMENT 0x00000000
#define HID0_ENABLE_DYNAMIC_POWER_MANAGMENT  0x00100000

#define HID0_SOFT_RESET			     0x00010000

#define HID0_DISABLE_INSTRUCTION_CACHE	     0x00000000
#define HID0_ENABLE_INSTRUCTION_CACHE	     0x00008000

#define HID0_DISABLE_DATA_CACHE		     0x00000000
#define HID0_ENABLE_DATA_CACHE		     0x00004000

#define HID0_LOCK_INSTRUCTION_CACHE	     0x00002000

#define HID0_LOCK_DATA_CACHE		     0x00001000

#define HID0_INVALIDATE_INSTRUCTION_CACHE    0x00000800

#define HID0_INVALIDATE_DATA_CACHE	     0x00000400

#define HID0_DISABLE_M_BIT		     0x00000000
#define HID0_ENABLE_M_BIT		     0x00000080

#define HID0_FBIOB			     0x00000010

#define HID0_DISABLE_ADDRESS_BROADCAST	     0x00000000
#define HID0_ENABLE_ADDRESS_BROADCAST	     0x00000008

#define HID0_ENABLE_NOOP_DCACHE_INSTRUCTION  0x00000000
#define HID0_DISABLE_NOOP_DCACHE_INSTRUCTION 0x00000001

/*
 * Hardware Implementation-Dependent Register 2 (HID2)
 */
#define HID2		1011

#define HID2_LET       0x08000000
#define HID2_HBE       0x00040000
#define HID2_IWLCK_000 0x00000000 /* no ways locked */
#define HID2_IWLCK_001 0x00002000 /* way 0 locked */
#define HID2_IWLCK_010 0x00004000 /* way 0 through way 1 locked */
#define HID2_IWLCK_011 0x00006000 /* way 0 through way 2 locked */
#define HID2_IWLCK_100 0x00008000 /* way 0 through way 3 locked */
#define HID2_IWLCK_101 0x0000A000 /* way 0 through way 4 locked */
#define HID2_IWLCK_110 0x0000C000 /* way 0 through way 5 locked */

#endif	/* __E300_H__ */
