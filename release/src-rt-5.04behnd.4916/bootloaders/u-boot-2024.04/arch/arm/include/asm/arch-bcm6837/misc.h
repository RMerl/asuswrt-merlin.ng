/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2021 Broadcom Ltd.
 */

#ifndef _6837_MISC_H
#define _6837_MISC_H

/*
 * Gpio Controller
 */
typedef struct GpioControl {
	uint32_t GPIODir[8];             /* 0x00-0x1c */
	uint32_t GPIOio[8];              /* 0x20-0x3c */
	uint32_t PadCtrl;                 /* 0x40 */
	uint32_t SpiSlaveCfg;             /* 0x44 */
	uint32_t TestControl;             /* 0x48 */
	uint32_t TestPortBlockEnMSB;      /* 0x4c */
	uint32_t TestPortBlockEnLSB;      /* 0x50 */
	uint32_t TestPortBlockDataMSB;    /* 0x54 */
	uint32_t TestPortBlockDataLSB;    /* 0x58 */
#define PINMUX_DATA_SHIFT       12
#define PINMUX_0                0
#define PINMUX_1                1
#define PINMUX_2                2
#define PINMUX_3                3
#define PINMUX_4                4
#define PINMUX_5                5
#define PINMUX_6                6
#define PINMUX_7                7
#define PAD_CTRL_SHIFT          12
#define PAD_CTRL_MASK           (0x3f<<PAD_CTRL_SHIFT)
#define PAD_SEL_SHIFT           12
#define PAD_AMP_SHIFT           15
#define PAD_IND_SHIFT           16
#define PAD_GMII_SHIFT          17
	uint32_t TestPortCmd;             /* 0x5c */
#define LOAD_MUX_REG_CMD        0x21
#define LOAD_PAD_CTRL_CMD       0x22
	uint32_t DiagReadBack;            /* 0x60 */
	uint32_t DiagReadBackHi;          /* 0x64 */
	uint32_t GeneralPurpose;          /* 0x68 */
	uint32_t spare[3];
} GpioControl;

#define GPIO_BASE          0xff800500
#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/*
 ** Misc Register Set Definitions.
 */
typedef struct Misc {
	uint32_t  miscStrapBus;                       /* 0x00 *//* NOT USED - defined for Backward compatability */ 
	uint32_t  miscStrapOverride;                  /* 0x04 *//* NOT USED - defined for Backward compatability */ 
	uint32_t  miscMaskUBUSErr;                    /* 0x08 *//* NOT USED - defined for Backward compatability */ 
	uint32_t  miscPeriphCtrl;                     /* 0x0c */
	uint32_t  miscSPImasterCtrl;                  /* 0x10 */
	uint32_t  miscDierevid;                       /* 0x14 */
	uint32_t  miscPeriphMiscCtrl;                 /* 0x18 */
	uint32_t  miscPeriphMiscStat;                 /* 0x1c */
	uint32_t  miscSoftResetB;                     /* 0x20 */
	uint32_t  miscSWdebugNW[3];                   /* 0x24 */
	uint32_t  miscWDresetCtrl;                    /* 0x30 */
} Misc;

#define MISC_BASE          0xff802600
#define MISC ((volatile Misc * const) MISC_BASE)

// PERF
typedef struct PerfControl { /* GenInt */
	uint32_t        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   8
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff
} PerfControl;

#define PERF_BASE              0x84210000
#define PERF ((volatile PerfControl * const) PERF_BASE)

typedef struct TopControl {
	uint32_t      MiiFiberDetect;     // 0x0
	uint32_t      DgSensePadCtl;      // 0x4
#define DG_CTRL_SHIFT   4
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
	uint32_t      TpDirOverride0;     // 0x8
	uint32_t      TpDirOverride1;     // 0xc
	uint32_t      RescalReadData0;    // 0x10
	uint32_t      RescalReadData1;    // 0x14
	uint32_t      RescalIpCtrl;       // 0x18
	uint32_t      ResetStatus;        // 0x1c
	uint32_t      TodSync;            // 0x20
	uint32_t      MdioMasterSelect;   // 0x24
	uint32_t      AdslClkSample;      // 0x28
	uint32_t      OrionIntPending;    // 0x2c
	uint32_t      EdmSealRngStat;     // 0x30
	uint32_t      Uart3SdinCtrl;      // 0x34
	uint32_t      reserved[3];        // 0x38
	uint32_t      XphyTestEn;         // 0x44
	uint32_t      OtpChipidLC;        // 0x48
#define CHIP_ID_LC_SIZE         4
#define CHIP_ID_LC_MASK         0x0000000f
} TopControl;

#define TOP_CONTROL_BASE 0xff85a000
#define TOPCTRL ((volatile TopControl * const) TOP_CONTROL_BASE)
#define TOP ((volatile TopControl * const) TOP_CONTROL_BASE)

#endif
