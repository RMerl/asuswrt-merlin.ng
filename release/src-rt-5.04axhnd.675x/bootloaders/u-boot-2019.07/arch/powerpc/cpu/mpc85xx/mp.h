#ifndef __MPC85XX_MP_H_
#define __MPC85XX_MP_H_

#include <asm/mp.h>

phys_addr_t get_spin_phys_addr(void);
u32 get_my_id(void);
int hold_cores_in_reset(int verbose);

#define BOOT_ENTRY_ADDR_UPPER	0
#define BOOT_ENTRY_ADDR_LOWER	1
#define BOOT_ENTRY_R3_UPPER	2
#define BOOT_ENTRY_R3_LOWER	3
#define BOOT_ENTRY_RESV		4
#define BOOT_ENTRY_PIR		5
#define BOOT_ENTRY_R6_UPPER	6
#define BOOT_ENTRY_R6_LOWER	7
#define NUM_BOOT_ENTRY		16	/* pad to 64 bytes */
#define SIZE_BOOT_ENTRY		(NUM_BOOT_ENTRY * sizeof(u32))

#endif
