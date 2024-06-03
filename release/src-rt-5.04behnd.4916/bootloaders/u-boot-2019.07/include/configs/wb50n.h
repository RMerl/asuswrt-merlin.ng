/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the WB50N CPU Module.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000	/* from 12 MHz crystal */

#define CONFIG_ARCH_CPU_INIT

#define CONFIG_CMDLINE_TAG	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/* general purpose I/O */
#define CONFIG_AT91_GPIO

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE       ATMEL_BASE_DBGU
#define CONFIG_USART_ID         ATMEL_ID_DBGU

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE       ATMEL_BASE_DDRCS
#define CONFIG_SYS_SDRAM_SIZE       0x04000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR     0x310000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
    (CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

#define CONFIG_SYS_MEMTEST_START    0x21000000
#define CONFIG_SYS_MEMTEST_END      0x22000000

/* NAND flash */
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_SYS_NAND_BASE        ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE    (1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE    (1 << 22)
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* Ethernet Hardware */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT      20
#define CONFIG_MACB_SEARCH_PHY
#define CONFIG_RGMII
#define CONFIG_ETHADDR              C0:EE:40:00:00:00
#define CONFIG_ENV_OVERWRITE        1

#define CONFIG_SYS_LOAD_ADDR        0x22000000	/* load address */

#define CONFIG_EXTRA_ENV_SETTINGS \
    "autoload=no\0" \
    "autostart=no\0"

/* bootstrap + u-boot + env in nandflash */
#define CONFIG_ENV_OFFSET           0xA0000
#define CONFIG_ENV_OFFSET_REDUND    0xC0000
#define CONFIG_ENV_SIZE             0x20000
#define CONFIG_BOOTCOMMAND \
    "nand read 0x22000000 0x000e0000 0x500000; " \
    "bootm"

#define CONFIG_BOOTARGS \
    "rw rootfstype=ubifs ubi.mtd=6 root=ubi0:rootfs"

#define CONFIG_BAUDRATE             115200

#define CONFIG_SYS_CBSIZE           1024
#define CONFIG_SYS_MAXARGS          16
#define CONFIG_SYS_PBSIZE \
    (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN       (2 * 1024 * 1024)

/* SPL */
#define CONFIG_SPL_MAX_SIZE         0x10000
#define CONFIG_SPL_BSS_START_ADDR   0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE     0x80000
#define CONFIG_SYS_SPL_MALLOC_START 0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE  0x80000

#define CONFIG_SYS_MONITOR_LEN      (512 << 10)

#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS 0x20000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE   0x800
#define CONFIG_SYS_NAND_PAGE_COUNT  64
#define CONFIG_SYS_NAND_OOBSIZE     64
#define CONFIG_SYS_NAND_BLOCK_SIZE  0x20000
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   0x0

#endif
