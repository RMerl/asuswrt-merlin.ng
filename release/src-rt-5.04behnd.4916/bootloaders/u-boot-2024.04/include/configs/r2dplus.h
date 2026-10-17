#ifndef __CONFIG_H
#define __CONFIG_H

#define __LITTLE_ENDIAN__	1

/* SCIF */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		0x8C000000
#define CFG_SYS_SDRAM_SIZE		0x04000000

/* Address of u-boot image in Flash */
#define CFG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

/*
 * NOR Flash ( Spantion S29GL256P )
 */
#define CFG_SYS_FLASH_BASE		(0xA0000000)
#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE }

#endif /* __CONFIG_H */
