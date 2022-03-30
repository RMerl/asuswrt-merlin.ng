/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the WB45N CPU Module.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK  32768
#define CONFIG_SYS_AT91_MAIN_CLOCK  12000000	/* 12 MHz crystal */

#define CONFIG_CMDLINE_TAG	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY	/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE   ATMEL_BASE_DBGU
#define CONFIG_USART_ID     ATMEL_ID_SYS

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE       0x20000000
#define CONFIG_SYS_SDRAM_SIZE       0x04000000	/* 64 MB */

#define CONFIG_SYS_INIT_SP_ADDR \
    (CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_SYS_NAND_BASE        0x40000000
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE    (1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE    (1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN  AT91_PIN_PD4
#define CONFIG_SYS_NAND_READY_PIN   AT91_PIN_PD5

#define CONFIG_RBTREE
#define CONFIG_LZO

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT      20
#define CONFIG_MACB_SEARCH_PHY
#define CONFIG_ETHADDR              C0:EE:40:00:00:00
#define CONFIG_ENV_OVERWRITE        1

/* System */
#define CONFIG_SYS_LOAD_ADDR        0x22000000	/* load address */
#define CONFIG_SYS_MEMTEST_START    CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END      0x23e00000

#ifdef CONFIG_SYS_USE_NANDFLASH
/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_OFFSET           0xa0000
#define CONFIG_ENV_OFFSET_REDUND    0xc0000
#define CONFIG_ENV_SIZE             0x20000	/* 1 block = 128 kB */

#define CONFIG_BOOTCOMMAND  "nand read 0x22000000 0xe0000 0x280000; " \
    "run _mtd; bootm"

#define MTDIDS_DEFAULT      "nand0=atmel_nand"
#define MTDPARTS_DEFAULT    "mtdparts=atmel_nand:" \
                            "128K(at91bs)," \
                            "512K(u-boot)," \
                            "128K(u-boot-env)," \
                            "128K(redund-env)," \
                            "2560K(kernel-a)," \
                            "2560K(kernel-b)," \
                            "38912K(rootfs-a)," \
                            "38912K(rootfs-b)," \
                            "46208K(user)," \
                            "512K(logs)"

#else
#error No boot method selected, please select 'CONFIG_SYS_USE_NANDFLASH'
#endif

#define CONFIG_BOOTARGS     "console=ttyS0,115200 earlyprintk " \
                            "rw noinitrd mem=64M "              \
                            "rootfstype=ubifs root=ubi0:rootfs ubi.mtd=6"

#define CONFIG_EXTRA_ENV_SETTINGS       \
    "_mtd=mtdparts default; setenv bootargs ${bootargs} ${mtdparts}\0" \
    "autoload=no\0" \
    "autostart=no\0" \
    "ethaddr=" __stringify(CONFIG_ETHADDR) "\0" \
    "\0"

#define CONFIG_SYS_CBSIZE   256
#define CONFIG_SYS_MAXARGS  16

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN       (512 * 1024 + 0x1000)

/* SPL */
#define CONFIG_SPL_MAX_SIZE         0x6000
#define CONFIG_SPL_STACK            0x308000

#define CONFIG_SPL_BSS_START_ADDR   0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE     0x80000
#define CONFIG_SYS_SPL_MALLOC_START 0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE  0x80000

#define CONFIG_SYS_MONITOR_LEN      (512 << 10)

#define CONFIG_SYS_MASTER_CLOCK     132096000
#define CONFIG_SYS_AT91_PLLA        0x20c73f03
#define CONFIG_SYS_MCKR             0x1301
#define CONFIG_SYS_MCKR_CSS         0x1302

#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS 0x20000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE   0x800
#define CONFIG_SYS_NAND_PAGE_COUNT  64
#define CONFIG_SYS_NAND_OOBSIZE     64
#define CONFIG_SYS_NAND_BLOCK_SIZE  0x20000
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   0x0

#endif				/* __CONFIG_H__ */
