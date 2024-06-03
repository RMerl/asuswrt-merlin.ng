/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_TYPES_H
#define SC_TYPES_H

/* Includes */
#include <linux/types.h>

/* Defines */
/*
 * This type is used to declare a handle for an IPC communication
 * channel. Its meaning is specific to the IPC implementation.
 */
typedef u64 sc_ipc_t;

/* Defines for common frequencies */
#define SC_32KHZ            32768U   /* 32KHz */
#define SC_10MHZ         10000000U   /* 10MHz */
#define SC_20MHZ         20000000U   /* 20MHz */
#define SC_25MHZ         25000000U   /* 25MHz */
#define SC_27MHZ         27000000U   /* 27MHz */
#define SC_40MHZ         40000000U   /* 40MHz */
#define SC_45MHZ         45000000U   /* 45MHz */
#define SC_50MHZ         50000000U   /* 50MHz */
#define SC_60MHZ         60000000U   /* 60MHz */
#define SC_66MHZ         66666666U   /* 66MHz */
#define SC_74MHZ         74250000U   /* 74.25MHz */
#define SC_80MHZ         80000000U   /* 80MHz */
#define SC_83MHZ         83333333U   /* 83MHz */
#define SC_84MHZ         84375000U   /* 84.37MHz */
#define SC_100MHZ       100000000U   /* 100MHz */
#define SC_125MHZ       125000000U   /* 125MHz */
#define SC_133MHZ       133333333U   /* 133MHz */
#define SC_135MHZ       135000000U   /* 135MHz */
#define SC_150MHZ       150000000U   /* 150MHz */
#define SC_160MHZ       160000000U   /* 160MHz */
#define SC_166MHZ       166666666U   /* 166MHz */
#define SC_175MHZ       175000000U   /* 175MHz */
#define SC_180MHZ       180000000U   /* 180MHz */
#define SC_200MHZ       200000000U   /* 200MHz */
#define SC_250MHZ       250000000U   /* 250MHz */
#define SC_266MHZ       266666666U   /* 266MHz */
#define SC_300MHZ       300000000U   /* 300MHz */
#define SC_312MHZ       312500000U   /* 312.5MHZ */
#define SC_320MHZ       320000000U   /* 320MHz */
#define SC_325MHZ       325000000U   /* 325MHz */
#define SC_333MHZ       333333333U   /* 333MHz */
#define SC_350MHZ       350000000U   /* 350MHz */
#define SC_372MHZ       372000000U   /* 372MHz */
#define SC_375MHZ       375000000U   /* 375MHz */
#define SC_400MHZ       400000000U   /* 400MHz */
#define SC_500MHZ       500000000U   /* 500MHz */
#define SC_594MHZ       594000000U   /* 594MHz */
#define SC_625MHZ       625000000U   /* 625MHz */
#define SC_640MHZ       640000000U   /* 640MHz */
#define SC_650MHZ       650000000U   /* 650MHz */
#define SC_667MHZ       666666667U   /* 667MHz */
#define SC_675MHZ       675000000U   /* 675MHz */
#define SC_700MHZ       700000000U   /* 700MHz */
#define SC_720MHZ       720000000U   /* 720MHz */
#define SC_750MHZ       750000000U   /* 750MHz */
#define SC_800MHZ       800000000U   /* 800MHz */
#define SC_850MHZ       850000000U   /* 850MHz */
#define SC_900MHZ       900000000U   /* 900MHz */
#define SC_1000MHZ     1000000000U   /* 1GHz */
#define SC_1060MHZ     1060000000U   /* 1.06GHz */
#define SC_1188MHZ     1188000000U   /* 1.188GHz */
#define SC_1260MHZ     1260000000U   /* 1.26GHz */
#define SC_1280MHZ     1280000000U   /* 1.28GHz */
#define SC_1300MHZ     1300000000U   /* 1.3GHz */
#define SC_1400MHZ     1400000000U   /* 1.4GHz */
#define SC_1500MHZ     1500000000U   /* 1.5GHz */
#define SC_1600MHZ     1600000000U   /* 1.6GHz */
#define SC_1800MHZ     1800000000U   /* 1.8GHz */
#define SC_2000MHZ     2000000000U   /* 2.0GHz */
#define SC_2112MHZ     2112000000U   /* 2.12GHz */

/* Defines for 24M related frequencies */
#define SC_8MHZ           8000000U   /* 8MHz */
#define SC_12MHZ         12000000U   /* 12MHz */
#define SC_19MHZ         19800000U   /* 19.8MHz */
#define SC_24MHZ         24000000U   /* 24MHz */
#define SC_48MHZ         48000000U   /* 48MHz */
#define SC_120MHZ       120000000U   /* 120MHz */
#define SC_132MHZ       132000000U   /* 132MHz */
#define SC_144MHZ       144000000U   /* 144MHz */
#define SC_192MHZ       192000000U   /* 192MHz */
#define SC_211MHZ       211200000U   /* 211.2MHz */
#define SC_240MHZ       240000000U   /* 240MHz */
#define SC_264MHZ       264000000U   /* 264MHz */
#define SC_352MHZ       352000000U   /* 352MHz */
#define SC_360MHZ       360000000U   /* 360MHz */
#define SC_384MHZ       384000000U   /* 384MHz */
#define SC_396MHZ       396000000U   /* 396MHz */
#define SC_432MHZ       432000000U   /* 432MHz */
#define SC_480MHZ       480000000U   /* 480MHz */
#define SC_600MHZ       600000000U   /* 600MHz */
#define SC_744MHZ       744000000U   /* 744MHz */
#define SC_792MHZ       792000000U   /* 792MHz */
#define SC_864MHZ       864000000U   /* 864MHz */
#define SC_960MHZ       960000000U   /* 960MHz */
#define SC_1056MHZ     1056000000U   /* 1056MHz */
#define SC_1104MHZ     1104000000U   /* 1104MHz */
#define SC_1200MHZ     1200000000U   /* 1.2GHz */
#define SC_1464MHZ     1464000000U   /* 1.464GHz */
#define SC_2400MHZ     2400000000U   /* 2.4GHz */

/* Defines for A/V related frequencies */
#define SC_62MHZ         62937500U   /* 62.9375MHz */
#define SC_755MHZ       755250000U   /* 755.25MHz */

/* Defines for type widths */
#define SC_FADDR_W      36U          /* Width of sc_faddr_t */
#define SC_BOOL_W       1U           /* Width of sc_bool_t */
#define SC_ERR_W        4U           /* Width of sc_err_t */
#define SC_RSRC_W       10U          /* Width of sc_rsrc_t */
#define SC_CTRL_W       6U           /* Width of sc_ctrl_t */

/* Defines for sc_bool_t */
#define SC_FALSE        ((sc_bool_t)0U)
#define SC_TRUE         ((sc_bool_t)1U)

/* Defines for sc_err_t */
#define SC_ERR_NONE         0U      /* Success */
#define SC_ERR_VERSION      1U      /* Incompatible API version */
#define SC_ERR_CONFIG       2U      /* Configuration error */
#define SC_ERR_PARM         3U      /* Bad parameter */
#define SC_ERR_NOACCESS     4U      /* Permission error (no access) */
#define SC_ERR_LOCKED       5U      /* Permission error (locked) */
#define SC_ERR_UNAVAILABLE  6U      /* Unavailable (out of resources) */
#define SC_ERR_NOTFOUND     7U      /* Not found */
#define SC_ERR_NOPOWER      8U      /* No power */
#define SC_ERR_IPC          9U      /* Generic IPC error */
#define SC_ERR_BUSY         10U     /* Resource is currently busy/active */
#define SC_ERR_FAIL         11U     /* General I/O failure */
#define SC_ERR_LAST         12U

/* Defines for sc_ctrl_t. */
#define SC_C_TEMP                       0U
#define SC_C_TEMP_HI                    1U
#define SC_C_TEMP_LOW                   2U
#define SC_C_PXL_LINK_MST1_ADDR         3U
#define SC_C_PXL_LINK_MST2_ADDR         4U
#define SC_C_PXL_LINK_MST_ENB           5U
#define SC_C_PXL_LINK_MST1_ENB          6U
#define SC_C_PXL_LINK_MST2_ENB          7U
#define SC_C_PXL_LINK_SLV1_ADDR         8U
#define SC_C_PXL_LINK_SLV2_ADDR         9U
#define SC_C_PXL_LINK_MST_VLD           10U
#define SC_C_PXL_LINK_MST1_VLD          11U
#define SC_C_PXL_LINK_MST2_VLD          12U
#define SC_C_SINGLE_MODE                13U
#define SC_C_ID                         14U
#define SC_C_PXL_CLK_POLARITY           15U
#define SC_C_LINESTATE                  16U
#define SC_C_PCIE_G_RST                 17U
#define SC_C_PCIE_BUTTON_RST            18U
#define SC_C_PCIE_PERST                 19U
#define SC_C_PHY_RESET                  20U
#define SC_C_PXL_LINK_RATE_CORRECTION   21U
#define SC_C_PANIC                      22U
#define SC_C_PRIORITY_GROUP             23U
#define SC_C_TXCLK                      24U
#define SC_C_CLKDIV                     25U
#define SC_C_DISABLE_50                 26U
#define SC_C_DISABLE_125                27U
#define SC_C_SEL_125                    28U
#define SC_C_MODE                       29U
#define SC_C_SYNC_CTRL0                 30U
#define SC_C_KACHUNK_CNT                31U
#define SC_C_KACHUNK_SEL                32U
#define SC_C_SYNC_CTRL1                 33U
#define SC_C_DPI_RESET                  34U
#define SC_C_MIPI_RESET                 35U
#define SC_C_DUAL_MODE                  36U
#define SC_C_VOLTAGE                    37U
#define SC_C_PXL_LINK_SEL               38U
#define SC_C_OFS_SEL                    39U
#define SC_C_OFS_AUDIO                  40U
#define SC_C_OFS_PERIPH                 41U
#define SC_C_OFS_IRQ                    42U
#define SC_C_RST0                       43U
#define SC_C_RST1                       44U
#define SC_C_SEL0                       45U
#define SC_C_LAST                       46U

#define SC_P_ALL        ((sc_pad_t)UINT16_MAX)   /* All pads */

/* Types */

/* This type is used to store a boolean */
typedef u8 sc_bool_t;

/* This type is used to store a system (full-size) address.  */
typedef u64 sc_faddr_t;

/* This type is used to indicate error response for most functions.  */
typedef u8 sc_err_t;

/*
 * This type is used to indicate a resource. Resources include peripherals
 * and bus masters (but not memory regions). Note items from list should
 * never be changed or removed (only added to at the end of the list).
 */
typedef u16 sc_rsrc_t;

/* This type is used to indicate a control.  */
typedef u8 sc_ctrl_t;

/*
 * This type is used to indicate a pad. Valid values are SoC specific.
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
typedef u16 sc_pad_t;

#endif /* SC_TYPES_H */
