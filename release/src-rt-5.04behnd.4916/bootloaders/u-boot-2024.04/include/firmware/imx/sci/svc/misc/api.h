/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_MISC_API_H
#define SC_MISC_API_H
/* Defines for type widths */
#define SC_MISC_DMA_GRP_W       5U      /* Width of sc_misc_dma_group_t */

/* Max DMA channel priority group */
#define SC_MISC_DMA_GRP_MAX     31U
/* Defines for sc_misc_boot_status_t */
#define SC_MISC_BOOT_STATUS_SUCCESS	0U	/* Success */
#define SC_MISC_BOOT_STATUS_SECURITY	1U	/* Security violation */

/* Defines for sc_misc_temp_t */
#define SC_MISC_TEMP                    0U   /* Temp sensor */
#define SC_MISC_TEMP_HIGH               1U   /* Temp high alarm */
#define SC_MISC_TEMP_LOW                2U   /* Temp low alarm */

/* Defines for sc_misc_bt_t */
#define SC_MISC_BT_PRIMARY              0U   /* Primary boot */
#define SC_MISC_BT_SECONDARY            1U   /* Secondary boot */
#define SC_MISC_BT_RECOVERY             2U   /* Recovery boot */
#define SC_MISC_BT_MANUFACTURE          3U   /* Manufacture boot */
#define SC_MISC_BT_SERIAL               4U   /* Serial boot */
/* Types */

/*
 * This type is used to store a DMA channel priority group.
 */
typedef u8 sc_misc_dma_group_t;

/*
 * This type is used report boot status.
 */
typedef u8 sc_misc_boot_status_t;

/*
 * This type is used report boot status.
 */
typedef u8 sc_misc_temp_t;

/*
 * This type is used report the boot type.
 */
typedef u8 sc_misc_bt_t;
#endif /* SC_MISC_API_H */
