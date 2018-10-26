/*
 Copyright 2000-2010 Broadcom Corporation

 <:label-BRCM:2011:DUAL/GPL:standard    
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
 :>
*/

#ifndef __BCM6838_MAP_PART_H
#define __BCM6838_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#if !defined(REG_BASE)
#define REG_BASE                 0xa0000000
#endif

#define CHIP_FAMILY_ID_HEX	 0x6838


#define MEMC_BASE                 ( REG_BASE +  0x12000000 )  /* DDR Memory Controller base */
#define PCIE_0_BASE               ( REG_BASE +  0x12800000 )  /* PCIe-0 Top */
#define PCIE_1_BASE               ( REG_BASE +  0x12a00000 )  /* PCIe-1 Top */
#define APM_BASE                  ( REG_BASE +  0x13a00000 )  /* APM Control block */
#define BMU_BASE                  ( REG_BASE +  0x13a01000 )  /* BMU Control Block */
#define PMC_BASE                  ( REG_BASE +  0x13e00000 )  /* PMC Block */ 
#define PROC_MON_BASE             ( REG_BASE +  0x13e80000 )  /* PMC Process Monitor Block */ 
#define IC_BASE                   ( REG_BASE +  0x14e00000 )   /* chip control interrupt register */
/* Blocks of PERIPHERAL BLOCK
   IC has the base of PERF_BASE */
#define PERF_BASE                 ( REG_BASE +  0x14e00000 )	/* INTERRUPT registers */
#define TIMR_BASE                 ( REG_BASE +  0x14e000c0 )  /* TIMER registers */
#define NAND_INTR_BASE            ( REG_BASE +  0x14e000f0 )  /* NAND Flash Interrupt base */
#define GPIO_BASE                 ( REG_BASE +  0x14e00100 )  /* GPIO registers */
#define PG_CONTROL_PER            ( REG_BASE +  0x14e002bc )	/* VIPER alternate boot vector register */
#define PLL_CONTROL_REG           ( REG_BASE +  0x14e002c0 )  /* PLL control */
#define WATCHDOG_BASE             ( REG_BASE +  0x14e002d0 ) /* WATCHDOG registers */
#define PERF_EXT_INT              ( REG_BASE +  0x14e00304 )  /* Peripheral Extended interrupt register*/ 	
#define DBG_PERF                  ( REG_BASE +  0x14e003e0 )  /* Software debug register per*/
#define OTP_BASE                  ( REG_BASE +  0x14e00400 )  /* OTP registers */
#define UART_BASE                 ( REG_BASE +  0x14e00500 )  /* UART 0 registers */
#define UART1_BASE                ( REG_BASE +  0x14e00520 )  /* UART 1 registers */
#define UART2_BASE                ( REG_BASE +  0x14e00540 )  /* UART 2 registers */
#define MDIO_EXT_BASE             ( REG_BASE +  0x14e00600 )  /* External MDIO Registers PER*/
#define MDIO_EGPHY_BASE           ( REG_BASE +  0x14e00610 )  /* EGPHY MDIO Registers PER*/
#define MDIO_SATA_BASE            ( REG_BASE +  0x14e00620 )  /* SATA MDIO register per*/
#define MDIO_AE_BASE              ( REG_BASE +  0x14e00630 )	/* AE PCS MDIO Registers PER*/
#define USIM_BASE                 ( REG_BASE +  0x14e00700 )  /* USIM interface registers*/
#define I2C_BASE                  ( REG_BASE +  0x14e00e00 )  /* I2C interface register*/
#define LED_BASE                  ( REG_BASE +  0x14e00f00 )  /* LED control register*/
#define HSSPIM_BASE               ( REG_BASE +  0x14e01000 )  /* High Speed SPI Master registes*/
#define NAND_REG_BASE             ( REG_BASE +  0x14e02200 )  /* NAND flash controller registers*/
#define NAND_CACHE_BASE           ( REG_BASE +  0x14e02600 )  /* NAND flash cache buffer */

#define USBH_BASE                 ( REG_BASE +  0x15400000 )  /* USBH control block*/
#define USBD_BASE                 ( REG_BASE +  0x15600000 )  /* USBD control block*/
#define UBUS2_ERROR               ( REG_BASE +  0x15e00000 )  /* UBUS Error block*/

#define PSRAM_BASE                0xb30a0000
#define PSRAM_BASE_KSEG0          0x930a0000
#define PSRAM_SIZE                0x20000 /* 128KB */

#define HIGH_MEM_PHYS_ADDRESS     0x20000000

#ifndef __ASSEMBLER__

/***********************************************************/
/*                    MEMC block definition                */
/***********************************************************/
typedef struct DDRPhyControl {
    uint32 REVISION;               /* 0x00 */
    uint32 CLK_PM_CTRL;            /* 0x04 */
    uint32 unused0[2];             /* 0x08-0x10 */
    uint32 PLL_STATUS;             /* 0x10 */
    uint32 PLL_CONFIG;             /* 0x14 */
    uint32 PLL_CONTROL;	           /* 0x18 */
    uint32 PLL_DIVIDER;	           /* 0x1c */
    uint32 AUX_CONTROL;	           /* 0x20 */ 
    uint32 unused1[3];             /* 0x24-0x30 */
    uint32 VDL_OVERRIDE_BYTE;      /* 0x30 */
    uint32 VDL_OVERRIDE_BIT;       /* 0x34 */
    uint32 IDLE_PAD_CONTROL;	   /* 0x38 */
    uint32 ZQ_PVT_COMP;	           /* 0x3c */
    uint32 DRIVE_PAD;		   /* 0x40 */
    uint32 VDL_RD_DATA_DELAY_STATUS;    /* 0x44 */
    uint32 VDL_CALIBRATE;	   /* 0x48 */ 
    uint32 VDL_CALIB_STATUS;	   /* 0x4c */
    uint32 VDL_DQ_CALIB_STATUS;     /* 0x50*/
    uint32 VDL_WR_CHAN_CALIB_STATUS; /* 0x54 */
    uint32 VDL_RD_EN_CALIB_STATUS;   /* 0x58 */
    uint32 VIRTUAL_VTT_CONTROL;      /* 0x5c */
    uint32 VIRTUAL_VTT_STATUS;       /* 0x60 */
    uint32 VIRTUAL_VTT_CONNECTIONS;  /* 0x64 */
    uint32 VIRTUAL_OVERRIDE;	    /* 0x68 */
    uint32 VREF_DAC_CONTROL;         /* 0x6c */
    uint32 PHYBIST_CNTRL;	    /* 0x70 */
    uint32 PHYBIST_SEED;             /* 0x74 */
    uint32 PHYBIST_STATUS;           /* 0x78 */
    uint32 PHYBIST_CTL_STATUS;       /* 0x7c */
    uint32 PHYBIST_DQ_STATUS;        /* 0x80 */
    uint32 PHYBIST_MISC_STATUS;      /* 0x84 */
    uint32 unused2[2];	            /* 0x88-0x90 */
    uint32 COMMAND_REG;		    /* 0x90 */
    uint32 MODE_REG0;	            /* 0x94 */
    uint32 MODE_REG1;	            /* 0x98 */
    uint32 MODE_REG2;	            /* 0x9c */
    uint32 MODE_REG3;	            /* 0xa0 */
    uint32 STANDBY_CONTROL;	    /* 0xa4 */
    uint32 unused3[2];		    /* 0xa8-0xb0 */
    uint32 STRAP_CONTROL;            /* 0xb0 */
    uint32 STRAP_CONTROL2;           /* 0xb4 */
    uint32 STRAP_STATUS;	            /* 0xb8 */
    uint32 STRAP_STATUS2;            /* 0xbc */
    uint32 DEBUG_FREEZ_ENABLE;       /* 0xc0 */
    uint32 DATAPATH_LOOPBACK ;       /* 0xc4 */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
    uint32 VDL_OVRIDE_BYTE_RD_EN;	/*0x400 */
    uint32 VDL_OVRIDE_BYTE0_W;		/*0x404*/
    uint32 VDL_OVRIDE_BYTE0_R_P;	/*0x408*/
    uint32 VDL_OVRIDE_BYTE0_R_N;	/*0x40c*/
    uint32 VDL_OVRIDE_BYTE0_BIT0_W;	/*0x410*/
    uint32 VDL_OVRIDE_BYTE0_BIT1_W;	/*0x414*/
    uint32 VDL_OVRIDE_BYTE0_BIT2_W;	/*0x418*/
    uint32 VDL_OVRIDE_BYTE0_BIT3_W;	/*0x41c*/
    uint32 VDL_OVRIDE_BYTE0_BIT4_W;	/*0x420*/
    uint32 VDL_OVRIDE_BYTE0_BIT5_W;	/*0x424*/
    uint32 VDL_OVRIDE_BYTE0_BIT6_W;	/*0x428*/
    uint32 VDL_OVRIDE_BYTE0_BIT7_W;	/*0x42c*/
    uint32 VDL_OVRIDE_BYTE0_DM_W;	/*0x430*/
    uint32 VDL_OVRIDE_BYTE0_BIT0_R_P;	/*0x434*/
    uint32 VDL_OVRIDE_BYTE0_BIT0_R_N;	/*0x438*/
    uint32 VDL_OVRIDE_BYTE0_BIT1_R_P;	/*0x43c*/
    uint32 VDL_OVRIDE_BYTE0_BIT1_R_N;	/*0x440*/
    uint32 VDL_OVRIDE_BYTE0_BIT2_R_P;	/*0x444*/
    uint32 VDL_OVRIDE_BYTE0_BIT2_R_N;	/*0x448*/
    uint32 VDL_OVRIDE_BYTE0_BIT3_R_P;	/*0x44c*/
    uint32 VDL_OVRIDE_BYTE0_BIT3_R_N;	/*0x450*/
    uint32 VDL_OVRIDE_BYTE0_BIT4_R_P;	/*0x454*/
    uint32 VDL_OVRIDE_BYTE0_BIT4_R_N;	/*0x458*/
    uint32 VDL_OVRIDE_BYTE0_BIT5_R_P;	/*0x45c*/
    uint32 VDL_OVRIDE_BYTE0_BIT5_R_N;	/*0x460*/
    uint32 VDL_OVRIDE_BYTE0_BIT6_R_P;	/*0x464*/
    uint32 VDL_OVRIDE_BYTE0_BIT6_R_N;	/*0x468*/
    uint32 VDL_OVRIDE_BYTE0_BIT7_R_P;	/*0x46c*/
    uint32 VDL_OVRIDE_BYTE0_BIT7_R_N;	/*0x470*/
    uint32 VDL_OVRIDE_BYTE0_BIT_RD_EN;	/*0x474*/
    uint32 unused[11];
    uint32 VDL_OVRIDE_BYTE1_W;		/*0x4a4*/
    uint32 VDL_OVRIDE_BYTE1_R_P;	/*0x4a8*/
    uint32 VDL_OVRIDE_BYTE1_R_N;	/*0x4ac*/
    uint32 VDL_OVRIDE_BYTE1_BIT0_W;	/*0x4b0*/
    uint32 VDL_OVRIDE_BYTE1_BIT1_W;	/*0x4b4*/
    uint32 VDL_OVRIDE_BYTE1_BIT2_W;	/*0x4b8*/
    uint32 VDL_OVRIDE_BYTE1_BIT3_W;	/*0x4bc*/
    uint32 VDL_OVRIDE_BYTE1_BIT4_W;	/*0x4c0*/
    uint32 VDL_OVRIDE_BYTE1_BIT5_W;	/*0x4c4*/
    uint32 VDL_OVRIDE_BYTE1_BIT6_W;	/*0x4c8*/
    uint32 VDL_OVRIDE_BYTE1_BIT7_W;	/*0x4cc*/
    uint32 VDL_OVRIDE_BYTE1_DM_W;	/*0x4d0*/
    uint32 VDL_OVRIDE_BYTE1_BIT0_R_P;	/*0x4d4*/
    uint32 VDL_OVRIDE_BYTE1_BIT0_R_N;	/*0x4d8*/
    uint32 VDL_OVRIDE_BYTE1_BIT1_R_P;	/*0x4dc*/
    uint32 VDL_OVRIDE_BYTE1_BIT1_R_N;	/*0x4e0*/
    uint32 VDL_OVRIDE_BYTE1_BIT2_R_P;	/*0x4e4*/
    uint32 VDL_OVRIDE_BYTE1_BIT2_R_N; 	/*0x4e8*/
    uint32 VDL_OVRIDE_BYTE1_BIT3_R_P;	/*0x4ec*/
    uint32 VDL_OVRIDE_BYTE1_BIT3_R_N;	/*0x4f0*/
    uint32 VDL_OVRIDE_BYTE1_BIT4_R_P;	/*0x4f4*/
    uint32 VDL_OVRIDE_BYTE1_BIT4_R_N;	/*0x4f8*/
    uint32 VDL_OVRIDE_BYTE1_BIT5_R_P;	/*0x4fc*/
    uint32 VDL_OVRIDE_BYTE1_BIT5_R_N;	/*0x500*/
    uint32 VDL_OVRIDE_BYTE1_BIT6_R_P;	/*0x504*/
    uint32 VDL_OVRIDE_BYTE1_BIT6_R_N;	/*0x508*/
    uint32 VDL_OVRIDE_BYTE1_BIT7_R_P;	/*0x50c*/
    uint32 VDL_OVRIDE_BYTE1_BIT7_R_N;	/*0x510*/
    uint32 VDL_OVRIDE_BYTE1_BIT_RD_EN;	/*0x514*/
    uint32 unused1[4];
    uint32 DYN_VDL_OVRIDE_BYTE0_R_P;	/*0x528*/
    uint32 DYN_VDL_OVRIDE_BYTE0_R_N;	/*0x52c*/
    uint32 DYN_VDL_OVRIDE_BYTE0_BIT_R_P;/*0x530*/	
    uint32 DYN_VDL_OVRIDE_BYTE0_BIT_R_N;/*0x534*/
    uint32 DYN_VDL_OVRIDE_BYTE0_W;	/*0x538*/
    uint32 DYN_VDL_OVRIDE_BYTE0_BIT_W;	/*0x53c*/
    uint32 unused2[2];			
    uint32 DYN_VDL_OVRIDE_BYTE1_R_P;	/*0x548*/
    uint32 DYN_VDL_OVRIDE_BYTE1_R_N;	/*0x54c*/
    uint32 DYN_VDL_OVRIDE_BYTE1_BIT_R_P;/*0x550*/
    uint32 DYN_VDL_OVRIDE_BYTE1_BIT_R_N;/*0x554*/
    uint32 DYN_VDL_OVRIDE_BYTE1_W;	/*0x558*/
    uint32 DYN_VDL_OVRIDE_BYTE1_BIT_W;	/*0x55c*/
    uint32 READ_DATA_DLY;		/*0x560*/
    uint32 READ_CONTROL;		/*0x564*/
    uint32 unused3[2];
    uint32 READ_FIFO_DATA_BL0_0;	/*0x570*/
    uint32 READ_FIFO_DATA_BL0_1;	/*0x574*/	
    uint32 READ_FIFO_DATA_BL0_2;	/*0x578*/
    uint32 READ_FIFO_DATA_BL0_3;	/*0x57c*/
    uint32 READ_FIFO_DATA_BL1_0;	/*0x580*/
    uint32 READ_FIFO_DATA_BL1_1;	/*0x584*/
    uint32 READ_FIFO_DATA_BL1_2;	/*0x588*/
    uint32 READ_FIFO_DATA_BL1_3;	/*0x58c*/
    uint32 READ_FIFO_STATUS;		/*0x590*/
    uint32 READ_FIFO_CLEAR;		/*0x594*/
    uint32 unused4[2];			
    uint32 IDLE_PAD_CONTROL;		/*0x5a0*/
    uint32 DRIVE_PAD_CTL;		/*0x5a4*/
    uint32 CLOCK_PAD_DISABLE;		/*0x5a8*/
    uint32 WR_PREAMBLE_MODE;		/*0x5ac*/
    uint32 PHYBIST_VDL_ADJ;		/*0x5b0*/
} DDRPhyByteLaneControl;

typedef struct MEMCControl {
    uint32 CNFG;                            /* 0x000 */
    uint32 CSST;                            /* 0x004 */
    uint32 CSEND;                           /* 0x008 */
    uint32 unused; 	                    /* 0x00c */
    uint32 ROW00_0;                         /* 0x010 */
    uint32 ROW00_1;                         /* 0x014 */
    uint32 ROW01_0;                         /* 0x018 */
    uint32 ROW01_1;                         /* 0x01c */
    uint32 unused0[4];			    /* 0x20- 0x30 */
    uint32 ROW20_0;                         /* 0x030 */
    uint32 ROW20_1;                         /* 0x034 */
    uint32 ROW21_0;                         /* 0x038 */
    uint32 ROW21_1;                         /* 0x03c */
    uint32 unused1[4];
    uint32 COL00_0;                         /* 0x050 */
    uint32 COL00_1;                         /* 0x054 */
    uint32 COL01_0;                         /* 0x058 */
    uint32 COL01_1;                         /* 0x05c */
    uint32 unused2[4];
    uint32 COL20_0;                         /* 0x070 */
    uint32 COL20_1;                         /* 0x074 */
    uint32 COL21_0;                         /* 0x078 */
    uint32 COL21_1;                         /* 0x07c */
    uint32 unused3[4];
    uint32 BNK10;                           /* 0x090 */
    uint32 BNK32;                           /* 0x094 */
    uint32 unused4[26];
    uint32 DCMD;                            /* 0x100 */
#define DCMD_CS1          (1 << 9)
#define DCMD_CS0          (1 << 8)
#define DCMD_SET_SREF     4
    uint32 DMODE_0;                         /* 0x104 */
    uint32 DMODE_1;                         /* 0x108 */
#define DMODE_1_DRAMSLEEP (1 << 11)	
    uint32 CLKS;                            /* 0x10c */
    uint32 ODT;                             /* 0x110 */
    uint32 TIM1_0;                          /* 0x114 */
    uint32 TIM1_1;                          /* 0x118 */
    uint32 TIM2;                            /* 0x11c */
    uint32 CTL_CRC;                         /* 0x120 */
    uint32 DOUT_CRC;                        /* 0x124 */
    uint32 DIN_CRC;                         /* 0x128 */
    uint32 CRC_CTRL;			    /* 0x12c */
    uint32 PHY_ST;			    /* 0x130 */
    uint32 DRAM_CFG;			    /* 0x134 */
    uint32 STAT;			    /* 0x138 */
    uint32 unused5[49];			    /* 0x13c-0x200*/
    DDRPhyControl           PhyControl;     /* 0x200 */
    uint32 unused6[14];			    /*0x2c8 - 0x300*/
    DDRPhyByteLaneControl   PhyByteLane0Control;    /* 0x300 */
    uint32 unused7[147];
    uint32 GCFG;                            /* 0x800 */
    uint32 unused8;                         /* 0x804 */
    uint32 unused9;                         /* 0x808 */
    uint32 ARB;                             /* 0x80c */
    uint32 PI_GCF;                          /* 0x810 */
    uint32 PI_UBUS_CTL;                     /* 0x814 */
    uint32 PI_MIPS_CTL;                     /* 0x818 */
    uint32 PI_DSL_MIPS_CTL;                 /* 0x81c */
    uint32 PI_DSL_PHY_CTL;                  /* 0x820 */
    uint32 PI_UBUS_ST;                      /* 0x824 */
    uint32 PI_MIPS_ST;                      /* 0x828 */
    uint32 PI_DSL_MIPS_ST;                  /* 0x82c */
    uint32 PI_DSL_PHY_ST;                   /* 0x830 */
    uint32 PI_UBUS_SMPL;                    /* 0x834 */
    uint32 TESTMODE;                        /* 0x838 */
    uint32 TEST_CFG1;                       /* 0x83c */
    uint32 TEST_PAT;                        /* 0x840 */
    uint32 TEST_COUNT;                      /* 0x844 */
    uint32 TEST_CURR_COUNT;                 /* 0x848 */
    uint32 TEST_ADDR_UPDT;                  /* 0x84c */
    uint32 TEST_ADDR;                       /* 0x850 */
    uint32 TEST_DATA0_0;                    /* 0x854 */
    uint32 TEST_DATA0_1;                    /* 0x858 */
    uint32 TEST_DATA0_2;                    /* 0x85c */
    uint32 TEST_DATA0_3;                    /* 0x860 */
    uint32 TEST_DATA1_0;                    /* 0x864 */
    uint32 TEST_DATA1_1;                    /* 0x868 */
    uint32 TEST_DATA1_2;                    /* 0x86c */
    uint32 TEST_DATA1_3;                    /* 0x870 */
    uint32 REPLY_DATA0;                     /* 0x874 */
    uint32 REPLY_DATA1;                     /* 0x878 */
    uint32 REPLY_DATA2;                     /* 0x87c */
    uint32 REPLY_DATA3;                     /* 0x880 */
    uint32 REPLY_STAT;                      /* 0x884 */	
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) MEMC_BASE)
/********************** MEMC block end *********************/

/***********************************************************/
/*                    PCIE block definition                 */
/***********************************************************/
#define PCIE_BASE                       PCIE_0_BASE
#define UBUS2_PCIE
typedef struct PcieRegs{
  uint32 devVenID;
  uint16 command;
  uint16 status;
  uint32 revIdClassCode;			/* 0x08 */
  uint32 headerTypeLatCacheLineSize;
  uint32 bar1;
  uint32 bar2;
  uint32 priSecBusNo;				/* 0x18 */
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_MASK              0x00ff0000
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_SHIFT             16
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_MASK              0x0000ff00
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_SHIFT             8
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_PRI_BUS_NO_MASK              0x000000ff

  uint16 ioBaseLimit;
  uint16 secStatus;
  uint32 rcMemBaseLimit;
  uint32 rcPrefBaseLimit;
  uint32 rcPrefBaseHi;
  uint32 rcPrefLimitHi;
  uint32 rcIoBaseLimit;
  uint32 capPointer;
  uint32 expRomBase;
  uint16 brdigeCtrlIntPinIntLine;
  uint16 bridgeCtrl;/* 0x03c */
  uint32 unused1[2];
  uint32 pm_cap;				/* 0x048 */
  uint32 pm_csr;				/* 0x04c */
  uint32 unused2[23];	
  /* PcieExpressCtrlRegs */
  uint16 pciExpressCap;			/* 0x0ac */		
  uint16 pcieCapabilitiy;
  uint32 deviceCapability;		
  uint16 deviceControl;
  uint16 deviceStatus;
  uint32 linkCapability;
  uint16 linkControl;
  uint16 linkStatus;
  uint32 slotCapability;
  uint16 slotControl;
  uint16 slotStatus;
  uint16 rootControl;
  uint16 rootCap;
  uint32 rootStatus;
  uint32 deviceCapability2;
  uint16 deviceControl2;
  uint16 deviceStatus2;
  uint32 linkCapability2;
  uint16 linkControl2;
  uint16 linkStatus2;
  uint32 slotCapability2;
  uint16 slotControl2;
  uint16 slotStatus2;
  uint32 unused3[6];		/* 0x0e8 */

  /* PcieErrorRegs */
  uint16 advErrCapId;		/* 0x100 */
  uint16 advErrCapOff;
  uint32 ucErrStatus;
  uint32 ucorrErrMask;
  uint32 ucorrErrSevr;
  uint32 corrErrStatus;
  uint32 corrErrMask;
  uint32 advErrCapControl;
  uint32 headerLog1;
  uint32 headerLog2;
  uint32 headerLog3;
  uint32 headerLog4;
  uint32 rootErrorCommand;
  uint32 rootErrorStatus;
  uint32 rcCorrId;
  uint32 unused4[10];		/* 0x138 */

  /* PcieVcRegs */
  uint16 vcCapId;			/* 0x160 */
  uint16 vcCapOffset;
  uint32 prtVcCapability;
  uint32 portVcCapability2;
  uint16 portVcControl;
  uint16 portVcCtatus;
  uint32 portArbStatus;
  uint32 vcRsrcControl;
  uint32 vcRsrcStatus;
  uint32 unused5[1];		/* 0x17c */
} PcieRegs;

typedef struct PcieRcCfgVendorRegs{
	uint32 vendorCap;
	uint32 specificHeader;
	uint32 specificReg1;
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR1_SHIFT    0	
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_SHIFT    2
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR3_SHIFT    4
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_WORD_ALIGN    0x0	
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_HWORD_ALIGN   0x1	
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BYTE_ALIGN    0x2
}PcieRcCfgVendorRegs;

typedef struct PcieBlk404Regs{
  uint32 unused;          /* 0x404 */
  uint32 config2;         /* 0x408 */
#define PCIE_IP_BLK404_CONFIG_2_BAR1_SIZE_MASK         0x0000000f
#define PCIE_IP_BLK404_CONFIG_2_BAR1_DISABLE           0
  uint32 config3;         /* 0x40c */
  uint32 pmDataA;         /* 0x410 */
  uint32 pmDataB;         /* 0x414 */
} PcieBlk404Regs;

typedef struct PcieBlk428Regs{
  uint32 vpdIntf;        /* 0x428 */
  uint16 unused_g;       /* 0x42c */
  uint16 vpdAddrFlag;    /* 0x42e */
  uint32 vpdData;        /* 0x430 */
  uint32 idVal1;         /* 0x434 */
  uint32 idVal2;         /* 0x438 */
  uint32 idVal3;         /* 0x43c */
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_MASK                    0xff000000
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_SHIFT                   24
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_MASK                     0x00ffffff
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_SHIFT                    16
#define PCIE_IP_BLK428_ID_VAL3_SUB_CLASS_CODE_SHIFT                 8

  uint32 idVal4;
  uint32 idVal5;
  uint32 unused_h;
  uint32 idVal6;
  uint32 msiData;
  uint32 msiAddr_h;
  uint32 msiAddr_l;
  uint32 msiMask;
  uint32 msiPend;
  uint32 pmData_c;
  uint32 msixControl;
  uint32 msixTblOffBir;
  uint32 msixPbaOffBit;
  uint32 unused_k;
  uint32 pcieCapability;
  uint32 deviceCapability;
  uint32 deviceControl;
  uint32 linkCapability;
  uint32 bar2Config;
  uint32 pcieDeviceCapability2;
  uint32 pcieLinkCapability2;
  uint32 pcieLinkControl;
  uint32 pcieLinkCapabilityRc;
  uint32 bar3Config;
  uint32 rootCap;
  uint32 devSerNumCapId;
  uint32 lowerSerNum;
  uint32 upperSerNum;
  uint32 advErrCap;
  uint32 pwrBdgtData0;
  uint32 pwrBdgtData1;
  uint32 pwrBdgtData2;
  uint32 pwdBdgtData3;
  uint32 pwrBdgtData4;
  uint32 pwrBdgtData5;
  uint32 pwrBdgtData6;
  uint32 pwrBdgtData7;
  uint32 ext2CapAddr;
  uint32 pwrBdgtData8;
  uint32 pwrBdgtCapability;
  uint32 vsecHdr;
  uint32 rcUserMemLo1;
  uint32 rcUserMemHi1;
  uint32 rcUserMemLo2;
  uint32 rcUserMemHi2;
  uint32 tphCap;   
  uint32 resizebarCap;  
  uint32 ariCap;   
  uint32 initvf;   
  uint32 vfOffset;   
  uint32 vfBarReg;   
  uint32 vfSuppPageSize;   
  uint32 vfCap_en;  
  uint32 vfMsixTblBirOff;  
  uint32 vfMsixPbaOffBit;  
  uint32 vfMsixControl;  
  uint32 vfBar4Reg;   
  uint32 pfInitvf;   
  uint32 vfNsp;   
  uint32 atsInldQueueDepth; 
} PcieBlk428Regs;

typedef struct PcieBlk800Regs{
#define NUM_PCIE_BLK_800_CTRL_REGS  6
  uint32 tlControl[NUM_PCIE_BLK_800_CTRL_REGS];
  uint32 tlL1Ctrl;
#define NUM_PCIE_BLK_800_USER_CTRL_REGS  7
  uint32 tlUserControl[NUM_PCIE_BLK_800_USER_CTRL_REGS];
  uint32 crsClearTimer;   
  uint32 tlFunc345Mask;   
  uint32 tlFunc345Stat;   
  uint32 tlFunc678_mask;   
  uint32 tlFunc678Stat;   
  uint32 funcIntSel;   
  uint32 tlObffCtrl;   
  uint32 tlCtlStat0;
  uint32 pmStatus0;
  uint32 pmStatus1;
#define NUM_PCIE_BLK_800_TAGS       32
  uint32 tlStatus[NUM_PCIE_BLK_800_TAGS];
  uint32 tlHdrFcStatus;
  uint32 tlDataFcStatus;
  uint32 tlHdrFcconStatus;
  uint32 tlDataFcconStatus;
  uint32 tlTargetCreditStatus;
  uint32 tlCreditAllocStatus;
  uint32 tlSmlogicStatus;
} PcieBlk800Regs;


typedef struct PcieBlk1000Regs{
#define NUM_PCIE_BLK_1000_PDL_CTRL_REGS  16
  uint32 pdlControl[NUM_PCIE_BLK_1000_PDL_CTRL_REGS];
  uint32 dlattnVec;
  uint32 dlAttnMask;
  uint32 dlStatus;        /* 0x1048 */
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_MASK                   0x00002000
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_SHIFT                  13
  uint32 dlTxChecksum;
  uint32 dlForcedUpdateGen1;
  uint32 dlReg_spare0;  
  uint32 dlErrStatistic_ctl;   
  uint32 dlErrStatistic; 
  uint32 reserved[40];
  uint32 mdioAddr;
  uint32 mdioWrData;
  uint32 mdioRdData;
  uint32 dlAteTlpHdr_0;  
  uint32 dlAteTlpHdr_1;  
  uint32 dlAteTlpHdr_2;  
  uint32 dlAteTlpHdr_3;  
  uint32 dlAteTlpCfg;   
  uint32 dlAteTlpCtl; 
  uint32 dlRxPFcCl;
  uint32 dlRxCFcCl;
  uint32 dlRxAckNack;
  uint32 dlTxRxSeqnb;
  uint32 dlTxPFcAl;
  uint32 dlTxNpFcAl;
  uint32 regDlSpare;
  uint32 dlRegSpare;
  uint32 dlTxRxSeq;
  uint32 dlRxNpFcCl;
} PcieBlk1000Regs;

typedef struct PcieBlk1800Regs{
#define NUM_PCIE_BLK_1800_PHY_CTRL_REGS         8
  uint32 phyCtrl[NUM_PCIE_BLK_1800_PHY_CTRL_REGS];
#define REG_POWERDOWN_P1PLL_ENA                      (1<<12)
  uint32 phyErrorAttnVec;
  uint32 phyErrorAttnMask;
  uint32 reserved;
  uint32 phyCtl8;				/* 0x1830 */
  uint32 reserved1[243];
  uint32 phyReceivedMcpErrors; 	/* 0x1c00 */
  uint32 reserved2[3];
  uint32 phyTransmittedMcpErrors;/* 0x1c10 */
  uint32 reserved3[3];
  uint32 rxFtsLimit;			/* 0x1c20 */
  uint32 reserved4[46];
  uint32 ftsHist;				/* 0x1cd8 */
  uint32 phyGenDebug;
  uint32 phyRecoveryHist;
#define NUM_PCIE_BLK_1800_PHY_LTSSM_HIST_REGS   5
  uint32 phyltssmHist[NUM_PCIE_BLK_1800_PHY_LTSSM_HIST_REGS];
#define NUM_PCIE_BLK_1800_PHY_DBG_REGS          11
  uint32 phyDbg[NUM_PCIE_BLK_1800_PHY_DBG_REGS];
  uint32 phyAteLoopbkInfo;		/* 0x1d30 */
  uint32 reserved5[55];
#define NUM_PCIE_BLK_1800_PHY_DBG_CLK_REGS      4
  uint32 phyDbgClk[NUM_PCIE_BLK_1800_PHY_DBG_CLK_REGS]; /* 0x1e10 */
} PcieBlk1800Regs;

typedef struct PcieMiscRegs{
  uint32 reset_ctrl;                    /* 4000 Reset Control Register */
  uint32 eco_ctrl_core;                 /* 4004 ECO Core Reset Control Register */
  uint32 misc_ctrl;                     /* 4008 MISC Control Register */
#define PCIE_MISC_CTRL_CFG_READ_UR_MODE                            (1<<13)
  uint32 cpu_2_pcie_mem_win0_lo;        /* 400c CPU to PCIe Memory Window 0 Low */
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK              0xfff00000
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_SHIFT             20
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_NO_SWAP               0
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_HALF_WORD_SWAP        1
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_HALF_BYTE_SWAP        2
  uint32 cpu_2_pcie_mem_win0_hi;        /* 4010 CPU to PCIe Memory Window 0 High */
  uint32 cpu_2_pcie_mem_win1_lo;        /* 4014 CPU to PCIe Memory Window 1 Low */
  uint32 cpu_2_pcie_mem_win1_hi;        /* 4018 CPU to PCIe Memory Window 1 High */
  uint32 cpu_2_pcie_mem_win2_lo;        /* 401c CPU to PCIe Memory Window 2 Low */
  uint32 cpu_2_pcie_mem_win2_hi;        /* 4020 CPU to PCIe Memory Window 2 High */
  uint32 cpu_2_pcie_mem_win3_lo;        /* 4024 CPU to PCIe Memory Window 3 Low */
  uint32 cpu_2_pcie_mem_win3_hi;        /* 4028 CPU to PCIe Memory Window 3 High */
  uint32 rc_bar1_config_lo;             /* 402c RC BAR1 Configuration Low Register */
#define  PCIE_MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK             0xfff00000
#define  PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_256MB                     0xd
#define  PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_MAX                       0x11     /* max is 4GB */
  uint32 rc_bar1_config_hi;             /* 4030 RC BAR1 Configuration High Register */
  uint32 rc_bar2_config_lo;             /* 4034 RC BAR2 Configuration Low Register */
  uint32 rc_bar2_config_hi;             /* 4038 RC BAR2 Configuration High Register */
  uint32 rc_bar3_config_lo;             /* 403c RC BAR3 Configuration Low Register */
  uint32 rc_bar3_config_hi;             /* 4040 RC BAR3 Configuration High Register */
  uint32 msi_bar_config_lo;             /* 4044 Message Signaled Interrupt Base Address Low Register */
  uint32 msi_bar_config_hi;             /* 4048 Message Signaled Interrupt Base Address High Register */
  uint32 msi_data_config;               /* 404c Message Signaled Interrupt Data Configuration Register */
  uint32 rc_bad_address_lo;             /* 4050 RC Bad Address Register Low */
  uint32 rc_bad_address_hi;             /* 4054 RC Bad Address Register High */
  uint32 rc_bad_data;                   /* 4058 RC Bad Data Register */
  uint32 rc_config_retry_timeout;       /* 405c RC Configuration Retry Timeout Register */
  uint32 eoi_ctrl;                      /* 4060 End of Interrupt Control Register */
  uint32 pcie_ctrl;                     /* 4064 PCIe Control */
  uint32 pcie_status;                   /* 4068 PCIe Status */
  uint32 revision;                      /* 406c PCIe Revision */
  uint32 cpu_2_pcie_mem_win0_base_limit;/* 4070 CPU to PCIe Memory Window 0 base/limit */
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK        0xfff00000
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT       20
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_MASK         0x0000fff0
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT        4
  uint32 cpu_2_pcie_mem_win1_base_limit;/* 4074 CPU to PCIe Memory Window 1 base/limit */
  uint32 cpu_2_pcie_mem_win2_base_limit;/* 4078 CPU to PCIe Memory Window 2 base/limit */
  uint32 cpu_2_pcie_mem_win3_base_limit;/* 407c CPU to PCIe Memory Window 3 base/limit */
  uint32 ubus_ctrl;                     /* 4080 UBUS Control */
  uint32 ubus_timeout;                  /* 4084 UBUS Timeout */
  uint32 ubus_bar1_config_remap;        /* 4088 UBUS BAR1 System Bus Address Remap Register */
#define  PCIE_MISC_UBUS_BAR_CONFIG_OFFSET_MASK                      0xfff00000
#define  PCIE_MISC_UBUS_BAR_CONFIG_ACCESS_EN                        1
  uint32 ubus_bar2_config_remap;        /* 408c UBUS BAR2 System Bus Address Remap Register */
  uint32 ubus_bar3_config_remap;        /* 4090 UBUS BAR3 System Bus Address Remap Register */
  uint32 ubus_status;                   /* 4094 UBUS Status */
} PcieMiscRegs;

typedef struct PcieMiscPerstRegs{
  uint32 perst_eco_ctrl_perst;          /* 4100 ECO PCIE Reset Control Register */
  uint32 perst_eco_cce_status;          /* 4104 Config Copy Engine Status */
} PcieMiscPerstRegs;

typedef struct PcieMiscHardRegs{
  uint32 hard_eco_ctrl_hard;            /* 4200 ECO Hard Reset Control Register */
  uint32 hard_pcie_hard_debug;          /* 4204 PCIE Hard Debug Register */
#define PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ                   (1<<23)
#define PCIE_MISC_HARD_PCIE_HARD_DEBUG_CLKREQ_DEBUG_ENABLE           (1<<1)
} PcieMiscHardRegs;

typedef struct PcieL2IntrControl{
  uint32 Intr2CpuStatus;
  uint32 Intr2CpuSet;
  uint32 Intr2CpuClear;
  uint32 Intr2CpuMask_status;
  uint32 Intr2CpuMask_set;
  uint32 Intr2CpuMask_clear;
  uint32 Intr2PciStatus;
  uint32 Intr2PciSet;
  uint32 Intr2PciClear;
  uint32 Intr2PciMask_status;
  uint32 Intr2PciMask_set;
  uint32 Intr2PciMask_clear;
} PcieL2IntrControl;

typedef struct PcieDMAregs{
  uint32 TxFirstDesc_l_AddrList0;
  uint32 TxFirstDesc_u_AddrList0;
  uint32 TxFirstDesc_l_AddrList1;
  uint32 TxFirstDesc_u_AddrList1;
  uint32 TxSwDescListCtrlSts;
  uint32 TxWakeCtrl;
  uint32 TxErrorStatus;
  uint32 TxList0CurDesc_l_Addr;
  uint32 TxList0CurDesc_u_Addr;
  uint32 TxList0CurByteCnt;
  uint32 TxList1CurDesc_l_Addr;
  uint32 TxList1CurDesc_u_Addr;
  uint32 TxList1CurByteCnt;
  uint32 RxFirstDesc_l_AddrList0;
  uint32 RxFirstDesc_u_AddrList0;
  uint32 RxFirstDesc_l_AddrList1;
  uint32 RxFirstDesc_u_AddrList1;
  uint32 RxSwDescListCtrlSts;
  uint32 RxWakeCtrl;
  uint32 RxErrorStatus;
  uint32 RxList0CurDesc_l_Addr;
  uint32 RxList0CurDesc_u_Addr;
  uint32 RxList0CurByteCnt;
  uint32 RxList1CurDesc_l_Addr;
  uint32 RxList1CurDesc_u_Addr;
  uint32 RxList1CurByteCnt;
  uint32 Dma_debug_options_reg;
  uint32 ReadChannelErrorStatus;
} PcieDMAregs;

typedef struct PcieUBUSL2IntrControl{
  uint32 UBUSIntr2CPUStatus;
  uint32 UBUSIntr2CPUSet;
  uint32 UBUSIntr2CPUClear;
  uint32 UBUSIntr2CPUMaskStatus;
  uint32 UBUSIntr2CPUMaskSet;
  uint32 UBUSIntr2CPUMaskClear;
  uint32 UBUSIntr2PCIStatus;
  uint32 UBUSIntr2PCISet;
  uint32 UBUSIntr2PCIClear;
  uint32 UBUSIntr2PCIMaskStatus;
  uint32 UBUSIntr2PCIMaskSet;
  uint32 UBUSIntr2PCIMaskClear;
} PcieUBUSL2IntrControl;

typedef struct PcieIPIL2IntrControl{
  uint32 IPIIntr2CPUStatus;
  uint32 IPIIntr2CPUSet;
  uint32 IPIIntr2CPUClear;
  uint32 IPIIntr2CPUMask_status;
  uint32 IPIIntr2CPUMask_set;
  uint32 IPIIntr2CPUMask_clear;
  uint32 IPIIntr2PCIStatus;
  uint32 IPIIntr2PCISet;
  uint32 IPIIntr2PCIClear;
  uint32 IPIIntr2PCIMask_status;
  uint32 IPIIntr2PCIMask_set;
  uint32 IPIIntr2PCIMask_clear;
} PcieIPIL2IntrControl;

typedef struct PcieCpuIntr1Regs{
  uint32 status;
#define PCIE_CPU_INTR1_IPI_CPU_INTR                                (1<<8)
#define PCIE_CPU_INTR1_PCIE_UBUS_CPU_INTR                          (1<<7)
#define PCIE_CPU_INTR1_PCIE_NMI_CPU_INTR                           (1<<6)
#define PCIE_CPU_INTR1_PCIE_INTR_CPU_INTR                          (1<<5)
#define PCIE_CPU_INTR1_PCIE_INTD_CPU_INTR                          (1<<4)
#define PCIE_CPU_INTR1_PCIE_INTC_CPU_INTR                          (1<<3)
#define PCIE_CPU_INTR1_PCIE_INTB_CPU_INTR                          (1<<2)
#define PCIE_CPU_INTR1_PCIE_INTA_CPU_INTR                          (1<<1)
#define PCIE_CPU_INTR1_PCIE_ERR_ATTN_CPU_INTR                      (1<<0)
  uint32 maskStatus;
  uint32 maskSet;
  uint32 maskClear;  
} PcieCpuIntr1Regs;

typedef struct PcieExtCfgRegs{
  uint32 index;
#define PCIE_EXT_CFG_BUS_NUM_MASK                                  0x0ff00000
#define PCIE_EXT_CFG_BUS_NUM_SHIFT                                 20
#define PCIE_EXT_CFG_DEV_NUM_MASK                                  0x000f0000
#define PCIE_EXT_CFG_DEV_NUM_SHIFT                                 15
#define PCIE_EXT_CFG_FUNC_NUM_MASK                                 0x00007000
#define PCIE_EXT_CFG_FUNC_NUM_SHIFT                                12
  uint32 data;
  uint32 scratch;
} PcieExtCfgRegs;

typedef struct PcieExtCfgDirectAccess{
  uint32 ExtCfgDataExtCfgData_0[1024];
  uint32 ExtCfgDataExtCfgData_1023;
} PcieExtCfgDirectAccess;


#define PCIEH                         ((volatile uint32 * const) PCIE_0_BASE)
#define PCIEH_REGS                    ((volatile PcieRegs * const) PCIE_0_BASE)
#define PCIEH_RC_CFG_VENDOR_REGS        ((volatile PcieRcCfgVendorRegs * const) \
                                        (PCIE_0_BASE+0x180)) 
#define PCIEH_BLK_404_REGS            ((volatile PcieBlk404Regs * const) \
                                        (PCIE_0_BASE+0x404))
#define PCIEH_BLK_428_REGS            ((volatile PcieBlk428Regs * const) \
                                        (PCIE_0_BASE+0x428))
#define PCIEH_BLK_800_REGS            ((volatile PcieBlk800Regs * const) \
                                        (PCIE_0_BASE+0x800))
#define PCIEH_BLK_1000_REGS           ((volatile PcieBlk1000Regs * const) \
                                        (PCIE_0_BASE+0x1000))
#define PCIEH_BLK_1800_REGS           ((volatile PcieBlk1800Regs * const) \
                                        (PCIE_0_BASE+0x1800))

#define PCIEH_MISC_REGS           	  ((volatile PcieMiscRegs * const) \
                                        (PCIE_0_BASE+0x4000))

#define PCIEH_MISC_PERST_REGS           ((volatile PcieMiscPerstRegs * const)  \
                                        (PCIE_0_BASE+0x4100))
#define PCIEH_MISC_HARD_REGS            ((volatile PcieMiscHardRegs * const)  \
                                        (PCIE_0_BASE+0x4200))
#define PCIEH_L2_INTR_CTRL_REGS       ((volatile PcieL2IntrControl * const) \
                                        (PCIE_0_BASE+0x4300))
#define PCIEH_DMA_REGS           	  ((volatile PcieDMAregs * const) \
                                        (PCIE_0_BASE+0x4400))
#define PCIEH_UBUS_L2_INTR_CTRL_REGS  ((volatile PcieUBUSL2IntrControl * const) \
                                        (PCIE_0_BASE+0x8000))
#define PCIEH_IPI_L2_INTR_CTRL_REGS   ((volatile PcieIPIL2IntrControl * const) \
                                        (PCIE_0_BASE+0x8100))
#define PCIEH_CPU_INTR1_REGS            ((volatile PcieCpuIntr1Regs * const)  \
                                        (PCIE_0_BASE+0x8300))
#define PCIEH_PCIE_EXT_CFG_REGS         ((volatile PcieExtCfgRegs * const)  \
                                        (PCIE_0_BASE+0x8400))
#define PCIEH_EXT_CFG_DIRECT_REGS     ((volatile PcieExtCfgDirectAccess * const) \
                                        (PCIE_0_BASE+0x9000))

#define PCIEH_1                         ((volatile uint32 * const) PCIE_1_BASE)
#define PCIEH_1_REGS                    ((volatile PcieRegs * const) PCIE_1_BASE)

#define PCIEH_1_RC_CFG_VENDOR_REGS      ((volatile PcieRcCfgVendorRegs * const) \
                                        (PCIE_1_BASE+0x180))
#define PCIEH_1_BLK_404_REGS            ((volatile PcieBlk404Regs * const) \
                                        (PCIE_1_BASE+0x404))
#define PCIEH_1_BLK_428_REGS            ((volatile PcieBlk428Regs * const) \
                                        (PCIE_1_BASE+0x428))
#define PCIEH_1_BLK_800_REGS            ((volatile PcieBlk800Regs * const) \
                                        (PCIE_1_BASE+0x800))
#define PCIEH_1_BLK_1000_REGS           ((volatile PcieBlk1000Regs * const) \
                                        (PCIE_1_BASE+0x1000))
#define PCIEH_1_BLK_1800_REGS           ((volatile PcieBlk1800Regs * const) \
                                        (PCIE_1_BASE+0x1800))
#define PCIEH_1_MISC_REGS               ((volatile PcieMiscRegs * const) \
                                        (PCIE_1_BASE+0x4000))
#define PCIEH_1_MISC_PERST_REGS         ((volatile PcieMiscPerstRegs * const)  \
                                        (PCIE_1_BASE+0x4100))
#define PCIEH_1_MISC_HARD_REGS          ((volatile PcieMiscHardRegs * const)  \
                                        (PCIE_1_BASE+0x4200))
#define PCIEH_1_L2_INTR_CTRL_REGS       ((volatile PcieL2IntrControl * const) \
                                        (PCIE_1_BASE+0x4300))
#define PCIEH_1_DMA_REGS                ((volatile PcieDMAregs * const) \
                                        (PCIE_1_BASE+0x4400))
#define PCIEH_1_UBUS_L2_INTR_CTRL_REGS  ((volatile PcieUBUSL2IntrControl * const) \
                                        (PCIE_1_BASE+0x8000))
#define PCIEH_1_IPI_L2_INTR_CTRL_REGS   ((volatile PcieIPIL2IntrControl * const) \
                                        (PCIE_1_BASE+0x8100))
#define PCIEH_1_CPU_INTR1_REGS          ((volatile PcieCpuIntr1Regs * const)  \
                                        (PCIE_1_BASE+0x8300))
#define PCIEH_1_PCIE_EXT_CFG_REGS       ((volatile PcieExtCfgRegs * const)  \
                                        (PCIE_1_BASE+0x8400))
#define PCIEH_1_EXT_CFG_DIRECT_REGS     ((volatile PcieExtCfgDirectAccess * const) \
                                        (PCIE_1_BASE+0x9000))

#define PCIEH_DEV_OFFSET                0x9000

#define PCIEH_MEM1_BASE                 0xd0000000
#define PCIEH_MEM1_SIZE                 0x10000000

#define PCIEH_0_MEM1_BASE               PCIEH_MEM1_BASE
#define PCIEH_0_MEM1_SIZE               PCIEH_MEM1_SIZE


#define PCIEH_1_MEM1_BASE               0xe0000000
#define PCIEH_1_MEM1_SIZE               0x10000000

#define DDR_UBUS_ADDRESS_BASE         0
/********************** PCIE block end **********************/

/*
** DMA Buffer
*/
typedef struct DmaDesc {
  union {
    struct {
        uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
        uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
    };
    uint32      word0;
  };

  uint32        address;                /* address of data */
} DmaDesc;

/*
** 16 Byte DMA Buffer
*/
typedef struct DmaDesc16 {
  union {
    struct {
        uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM        0x8000
#define          DMA_DESC_MULTICAST     0x4000
#define          DMA_DESC_BUFLENGTH     0x0fff
        uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
    };
    uint32      word0;
  };

  uint32        address;                 /* address of data */
  uint32        control;
#define         GEM_ID_MASK             0x001F
  uint32        reserved;
} DmaDesc16;

/********************** APM block end **********************/

#define IRQ_BITS 32

typedef struct  {
    uint32         IrqMask;
    uint32         IrqStatus;
} IrqControl_t;

/***********************************************************/
/*                    IC block definition (PERF)           */
/***********************************************************/
typedef struct PerfControl {
     uint32        RevID;                         /* (00) word 0 */
#define CHIP_ID_SHIFT   0x4
#define CHIP_ID_MASK    (0xf << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xf  
     uint32        Unused[3];                     /* (04) word 1 */
     uint32        TimerControl;                 /* (10) word 4 */
#define SOFT_RESET_0			0x00000001		// viper watchdog
#define SOFT_RESET_1			0x00000002		// PMC watchdog

     uint32        RsvdIrqMask0;                  /* (14) word 5 */
     uint32        RsvdIrqStatus0;                /* (18) word 6 */
     uint32        RsvdrqMask1;                   /* (1c) word 7 */
     uint32        RsvdIrqStatus1;                /* (20) word 8 */
     uint32        RsvdIrqMask2;                  /* (24) word 9 */
     uint32        RsvdIrqStatus2;                /* (28) word 10 */
     uint32        unused2c;                      /* (2c) word 11 */
     uint32        PeriphIrqMask0;                /* (30) word 12 */
#define WAN_GPON_TX_IRQ	   	(1 << 31)	
#define WAN_GPON_RX_IRQ	   	(1 << 30)
#define WAN_EPON_IRQ		(1 << 29)
#define WAN_NCO_8KHZ_IRQ	(1 << 28)
#define RDP_UBUS_ERR_PORT_IRQ   (1 << 27)
#define RDP_UBUS_ERROR_IRQ	(1 << 26)
#define RDP_RUNNER_IRQ_OFFSET   16
#define RDP_RUNNER_IRQ_MASK     0x3FF00000
#define EXT_IRQ		        (1 << 15)
#define DSCRAM_RNG_READY_IRQ    (1 << 10)
#define DSCRAM_KEYDONE_IRQ      (1 << 9)
#define UART2_IRQ               (1 << 8)      
#define UBUS_REQOUT_ERR_IRQ     (1 << 7)      
#define PERIPH_ERROR_DET_IRQ    (1 << 6)
#define HSSPI_IRQ               (1 << 5)
#define I2CIRQ                  (1 << 4)      
#define NAND_FLASH_IRQ          (1 << 3)
#define UART1_IRQ               (1 << 2)      
#define UART_IRQ                (1 << 1) 
#define TIMRIRQ                 (1 << 0)
     uint32        PeriphIrqStatus0;              /* (34) word 13 */
     uint32        PeriphIrqMask1;                /* (38) word 14 */
     uint32        PeriphIrqStatus1;              /* (3c) word 15 */
	 union {
		 struct {
			 uint32        PeriphIrqMask2;                /* (40) word 16 */
			 uint32        PeriphIrqStatus2;              /* (44) word 17 */
			 uint32        PeriphIrqMask3;                /* (48) word 18 */
			 uint32        PeriphIrqStatus3;              /* (4c) word 19 */
		 };
		 IrqControl_t  IrqControl[2];    /* (40) (48)*/
	 };
     uint32        IopIrqMask0;                   /* (50) word 20 */
#define PMCIRQ_TP0			(1 << 5)
#define PMCIRQ_TP1		    (1 << 13)
     uint32        IopIrqStatus0;                 /* (54) word 21 */
     uint32        IopIrqMask1;                   /* (58) word 22 */
     uint32        IopIrqStatus1;                 /* (5c) word 23 */
     uint32        Rsvd0IrqSense;             	  /* (60) word 24 */
     uint32        PeriphIrqSense;                /* (64) word 25 */
     uint32        IopIrqSense;                   /* (68) word 26 */
     uint32        ExtIrqCfg;                  	  /* (6c) word 27 */
#define EI_SENSE_SHFT		0
#define EI_STATUS_SHFT		6
#define EI_CLEAR_SHFT		6
#define EI_MASK_SHFT		12
#define EI_INSENS_SHFT		18
#define EI_LEVEL_SHFT		24
#define EI_STATUS_MASK		0xfc0
#define EI_CLEAR_MASK		EI_STATUS_MASK
#define EI_MASK_MASK		0x3f000
     uint32        ExtIrqCfg1;    	              /* (70) word 28 */
     uint32        ExtIrqCfg2;                    /* (74) word 29 */
     uint32        IrqOutMask;                    /* (78) word 30 */
#define SYSIRQ_OUT_IRQ0_IOP          (1 << 0)
#define SYSIRQ_OUT_IRQ1_IOP          (1 << 1)
#define SYSIRQ_OUT_IRQ0_PERIPH       (1 << 2)
#define SYSIRQ_OUT_IRQ1_PERIPH       (1 << 3)
#define SYSIRQ_OUT_IRQ2_PERIPH       (1 << 4)
#define SYSIRQ_OUT_IRQ0_DOCSIS       (1 << 5)
#define SYSIRQ_OUT_IRQ1_DOCSIS       (1 << 6)
#define SYSIRQ_OUT_IRQ2_DOCSIS       (1 << 7)
#define TESTBUS_OUT_IRQ0_IOP         (1 << 8)
#define TESTBUS_OUT_IRQ1_IOP         (1 << 9)
#define TESTBUS_OUT_IRQ0_PERIPH      (1 << 10)
#define TESTBUS_OUT_IRQ1_PERIPH      (1 << 11)
#define TESTBUS_OUT_IRQ2_PERIPH      (1 << 12)
#define TESTBUS_OUT_IRQ0_DOCSIS      (1 << 13)
#define TESTBUS_OUT_IRQ1_DOCSIS      (1 << 14)
#define TESTBUS_OUT_IRQ2_DOCSIS      (1 << 15)
#define UBUSCAPTURE_OUT_IRQ0_IOP     (1 << 16)
#define UBUSCAPTURE_OUT_IRQ1_IOP     (1 << 17)
#define UBUSCAPTURE_OUT_IRQ0_PERIPH  (1 << 18)
#define UBUSCAPTURE_OUT_IRQ1_PERIPH  (1 << 19)
#define UBUSCAPTURE_OUT_IRQ2_PERIPH  (1 << 20)
#define UBUSCAPTURE_OUT_IRQ0_DOCSIS  (1 << 21)
#define UBUSCAPTURE_OUT_IRQ1_DOCSIS  (1 << 22)
#define UBUSCAPTURE_OUT_IRQ2_DOCSIS  (1 << 23)
#define PCIE_OUT_IRQ0_IOP            (1 << 24)
#define PCIE_OUT_IRQ1_IOP            (1 << 25)
#define PCIE_OUT_IRQ0_PERIPH         (1 << 26)
#define PCIE_OUT_IRQ1_PERIPH         (1 << 27)
#define PCIE_OUT_IRQ2_PERIPH         (1 << 28)
#define PCIE_OUT_IRQ0_DOCSIS         (1 << 29)
#define PCIE_OUT_IRQ1_DOCSIS         (1 << 30)
#define PCIE_OUT_IRQ2_DOCSIS         (1 << 31)
       uint32        diagSelControl;              /* (7c) word 31 */
#define DIAG_HI_SEL_MASK        0x0000ff00
#define DIAG_HI_SEL_SHFT        8
#define DIAG_LO_SEL_MASK        0x000000ff
#define DIAG_LO_SEL_SHFT        0
#define DIAG_CLK_PHS_MASK       0x003f0000
#define DIAG_CLK_PHS_SHIFT      16
#define DIAG_LO_ENABLED         (1 << 24)
#define DIAG_CLK_LO_ENABLED     (1 << 25)
#define DIAG_HI_ENABLED         (1 << 26)
#define DIAG_CLK_HI_ENABLED     (1 << 27)
#define DIAG_UBUS_OBS_ENABLED   (1 << 28)
#define DIAG_PINMUX_OVERRIDE    (1 << 29)
#define DIAG_SPI_OVERRIDE       (1 << 31)
     uint32        diagReadBack;                  /* (80) word 32 */
     uint32        diagReadBackHi;                /* (84) word 33 */
     uint32        diagMiscControl;               /* (88) word 34 */
     uint32        pcie_softResetB_lo;            /* (8c) word 35 */
#define SOFT_RST_PCIE1_CORE 	   	(1 << 1)
#define SOFT_RST_PCIE0_CORE 		(1 << 0)
     uint32        mdio_irq;                      /* (90) word 36 */
#define MDIO_EXT_DONE_IRQ 	    (1 << 0)
#define MDIO_EXT_ERR_IRQ 	    (1 << 1)
#define EGPHY_EXT_DONE_IRQ	    (1 << 2)
#define EGPHY_EXT_ERR_IRQ	    (1 << 3)
#define MDIO_SATA_DONE_IRQ	    (1 << 4)
#define MDIO_SATA_ERR_IRQ	    (1 << 5)
#define MDIO_AE_DONE_IRQ	    (1 << 6)
#define MDIO_AE_ERR_IRQ			(1 << 7)
#define MDIO_EXT_DONE_IEN  		(1 << 8)
#define MDIO_EXT_ERR_IEN   		(1 << 9)
#define EGPHY_EXT_DONE_IEN 		(1 << 10)
#define EGPHY_EXT_ERR_IEN  		(1 << 11)
#define MDIO_SATA_DONE_IEN 		(1 << 12)
#define MDIO_SATA_ERR_IEN  		(1 << 13)
#define MDIO_AE_DONE_IEN   		(1 << 14)
#define MDIO_AE_ERR_IEN    		(1 << 15)
     uint32        dsramIrqStatus;                /* (94) word 37 */
     uint32        ext_irq_muxsel0;               /* (98) word 38 */
#define  EXT_IRQ5_SEL	0x3E000000
#define  EXT_IRQ4_SEL	0x01F00000
#define  EXT_IRQ3_SEL	0x000F8000
#define  EXT_IRQ2_SEL	0x00007C00
#define  EXT_IRQ1_SEL	0x000002E0
#define  EXT_IRQ0_SEL	0x0000001F
#define  EXT_IRQ_MASK_LOW   0x0000001F
#define  EXT_IRQ_OFF_LOW    5
     uint32        IntPeriphIrqStatus;                 /* (9C) word 39 */
#define DIAG_IRQ                0x00000100      
#define RBUS_ERR_IRQ           0x00000080      
#define BRIDGE_TO_ERR_IRQ       0x00000040      
#define REQOUT_PLEN_ERR_IRQ     0x00000020      
#define U2R_REPOUT_ERR_IRQ      0x00000010      
#define BRIDGE_UBUS_ERR_IRQ     0x00000008      
#define DEVTIMEOUT_IRQ          0x00000004      
#define ERROR_PORT_IRQ          0x00000002      
#define BAD_BOOT_LOC_IRQ        0x00000001
     uint32        IntPeriphIrqMask;              /* (A0) word 40 */
     uint32        spare[6];  
     uint32        soft_no_watchdog_reset_b;      /* (BC) word 47 */
}PerfControl;
#define PERF ((volatile PerfControl * const) PERF_BASE)

/********************** IC block end ***********************/

/***********************************************************/
/*                    Timer block definition(PERF)         */
/***********************************************************/
typedef struct Timer {
    uint16			unused0;
    byte			TimerMask;
#define TIMER0EN		0x01
#define TIMER1EN		0x02
#define TIMER2EN		0x04
    byte			TimerInts;
#define TIMER0			0x01
#define TIMER1			0x02
#define TIMER2			0x04
#define WATCHDOG0		0x08
#define WATCHDOG		WATCHDOG0 /* compatible with other chips */
#define WATCHDOG1		0x10
    uint32			TimerCtl0;
    uint32			TimerCtl1;
    uint32			TimerCtl2;
#define TIMERENABLE     0x80000000
#define RSTCNTCLR       0x40000000
    uint32			TimerCnt0;
    uint32			TimerCnt1;
    uint32			TimerCnt2;
#define TIMER_COUNT_MASK    0x3FFFFFFF
	uint32			TimerMemTm;  //unused
	uint32			TimerEphyTestCtrl;
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)
/********************** Timer block end ********************/

/***********************************************************/
/*                    Watchdog block definition(PERF)      */
/***********************************************************/
typedef struct Watchdog {
	uint32			WD0DefCount;

    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32			WD0Ctl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32			WD0ResetCount;

	uint32			WD1DefCount;
	uint32			WD1Ctl;
	uint32			WD1ResetCount;
}		Watchdog;

#define WDTIMER ((volatile Watchdog * const) WATCHDOG_BASE)
/********************** Watchdog block end *****************/

/***********************************************************/
/*                    NAND Intr block definition(PERF)     */
/***********************************************************/
typedef struct NandIntrReg {
    uint32 NandInt;			//  nand flash Interrupt 
    uint32 NandIntBase_0;  	//	nand flash Base Address 0 
    uint32 NandIntBase_1;   //	nand flash Base Address 1 
    uint32 NandIntSoftnowatchdogreset;  //Soft No Watchdog Reset Register 
} NandIntrReg;

#define NANINTR ((volatile NandIntrReg * const) NAND_INTR_BASE)

/********************** NAND Intr block end *****************/

/***********************************************************/
/*                    GPIO  block definition(PERF)         */
/***********************************************************/
typedef struct GpioControl {
  /* High in bit location enables output */
  uint32		GPIODir_low;		// 0
  uint32		GPIODir_mid0;		// 4
  uint32		GPIODir_mid1;		// 8
  uint32		reserved_GPIODir[7];// c
  uint32		unused0;			// 24
  uint32		GPIOData_low;		// 2C;
  uint32		GPIOData_mid0;		// 30 :
  uint32		GPIOData_mid1;		// 34:
  uint32		resered_GPIOData[7];// 38
  uint32		unused1;			// 54;
#define USB1_PULLUP			(1 << 23)
#define USB1_PULLDOWN		(1 << 22)
#define USB1_DISABLE_INPUT	(1 << 21)
#define USB1_HYS_ENABLE		(1 << 20)
#define USB0_PULLUP			(1 << 19)
#define USB0_PULLDOWN		(1 << 18)
#define USB0_DISABLE_INPUT	(1 << 17)
#define USB0_HYS_ENABLE		(1 << 16)
  uint32		PadControl;			// 58:
  uint32		SpiSlaveCfg;		// 5C
  uint32		reserved_diag1[3];	// 60
  uint32		TestControl;		// 6C
  uint32		USimControl;		// 70
  uint32		reserved_diag2[8];	// 74
  uint32		strap_bus;			// 94
#define MISC_STRAP_BUS_BOOT_SEL_MASK	0x000003800
#define	MISC_STRAP_BUS_BOOT_SEL_SHIFT	11
#define MISC_STRAP_SPI_3BYTE_ADDR_MASK	0x4
#define MISC_STRAP_SPI_4BYTE_ADDR_MASK	0x5
#define MISC_STRAP_BUS_SPI_NAND_BOOT	1
#define MISC_STRAP_BUS_BOOT_CFG_MASK	(0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_PAGE_SIZE_SHIFT	9
#define MISC_STRAP_BUS_PAGE_SIZE_MASK	(0x3 << MISC_STRAP_BUS_PAGE_SIZE_SHIFT)
  uint32		strap_override;		// 98
  uint32		qam_pll_status;		// 9C - reserved
  uint32		strap_out_bus;		// A0
  uint32		led_input_xor;		// A4
  uint32		reserved_diag3[12];	// A8
  uint32		ddr_pll_override;	// D8
  uint32        diag_kick_timer;    // DC:
  uint32        DieRevID;           // E0:
#define DIEID 0x00000047
  uint32		spi_master_control;	// E4
  uint32		clk_rst_misc;		// E8 - reserved
  uint32		dg_control;			// EC
#define DG_IRQ_SHIFT    6
#define DG_TRIM_SHIFT   3
#define DG_CTRL_SHIFT   1
#define DG_EN_SHIFT     0
  uint32		reserved_diag4[36];	// F0
  uint32		IRQ_out_mask1;		// 180
  uint32		sdram_space;		// 184
  uint32		ddr_16_en;			// 188
  uint32		memc_phy_control;	// 18C
  uint32		memc_control;		// 190
  uint32		port_reg_tpIn1;		// 194
  uint32		port_reg_tpIn2;		// 198
  uint32		port_block_en1;		// 19C
  uint32		port_block_en2;		// 1A0
  uint32		port_block_data1;	// 1A4
  uint32		port_block_data2;	// 1A8
#define PINMUX_DATA_SHIFT	12
#define PINMUX_0           0
#define PINMUX_1           1
#define PINMUX_2           2
#define PINMUX_3           3
#define PINMUX_4           4
#define PINMUX_5           5
#define PINMUX_6           6
#define PINMUX_7           7
#define PINMUX_8           8  
#define PINMUX_14          14
#define PINMUX_15          15
#define PINMUX_16          16
#define PINMUX_17          17
#define PINMUX_18          18
#define PINMUX_35          35
#define PINMUX_36          36
#define PINMUX_37          37
#define PINMUX_39          39

#define PINMUX_SIM_FUNC    3
#define PINMUX_GPIO_FUNC   5

#define PINMUX_MSPI                     PINMUX_1
#define PINMUX_PCM                      PINMUX_1
#define PINMUX_APM                      PINMUX_1
#define PINMUX_ZAR_IF_D                 PINMUX_2    
#define PINMUX_ZAR_IF_E                 PINMUX_2  
#define PINMUX_ZAR_IF_C                 PINMUX_3
#define PINMUX_GPIO                     PINMUX_5
#define PINMUX_SIM_DAT                  PINMUX_6
#define PINMUX_ZAR_IF_A                 PINMUX_7    
#define PINMUX_ZAR_IF_B                 PINMUX_7    
#define PINMUX_SIM_CLK                  PINMUX_7
#define PINMUX_SIM_PRESENCE             PINMUX_8
#define PINMUX_SIM_A_VCC_EN_PIN         PINMUX_14
#define PINMUX_SIM_A_VCC_VOL_SEL_PIN    PINMUX_15
#define PINMUX_SIM_A_VCC_RST_PIN        PINMUX_16
#define PINMUX_SIM_A_VPP_EN_PIN         PINMUX_17

#define PINMUX_SIM_B_VCC_EN_PIN         PINMUX_35 
#define PINMUX_SIM_B_VCC_VOL_SEL_PIN    PINMUX_36
#define PINMUX_SIM_B_VCC_RST_PIN        PINMUX_37
#define PINMUX_SIM_B_VPP_EN_PIN         PINMUX_39


  uint32		port_command;		// 1AC
#define LOAD_MUX_REG_CMD	0x21
} GpioControl;
#define GPIO ((volatile GpioControl * const) GPIO_BASE)
#define GPIO_NUM_MAX            74

#define USIM_CTRL               &GPIO->USimControl

#define GPIO_NUM_TO_MASK(X)		((uint32)1 << (X))
/********************** GPIO block end ********************/

/***********************************************************/
/*                    PageControl block definition(PERF)   */
/***********************************************************/
 typedef struct PageControl {
	 uint32 AltBootConfig;
#define ALT_BOOT_MASK   0xFFF00000
#define ALT_BOOT_EN		(1 << 19)
} PageControl;
#define PGCTRL ((volatile PageControl * const) PG_CONTROL_PER)
/********************** PageControl block end **************/

/***********************************************************/
/*                    PLL Control block definition(PERF)   */
/***********************************************************/
 typedef struct PLLControl {
	 uint32 PllControlReg;
	 uint32 PllControlOsc;
#define PLL_DIV2_SEL_MASK   (1 << 22)
#define PWR_DN_CML			(1 << 21)
#define PWR_SAVE			(1 << 20)
} PLLControl;
#define PLLCTRL ((volatile PLLControl * const) PLL_CONTROL_REG)
/********************** PLL Control block end **************/

/***********************************************************/
/*                    Perf ext intr block definition(PERF) */
/***********************************************************/
 typedef struct ExtIntr {
    uint32 ExtRsvd0irqmask3;  		//rsVD0 Interrupt Mask Register3 
    uint32 ExtRsvd0irqstatus3;  	//RSVD0 Interrupt Status Register3 
    uint32 ExtIopirqmask2;  		//iop Interrupt Mask Register2 
    uint32 ExtIopirqstatus2;  		//ioP Interrupt Status Register2 
    uint32 ExtRsvd0irqmask0_2;  	//RSVD0 Interrupt Mask Register0_2 
    uint32 ExtRsvd0irqstatus0_2;  	//RSVD0 Interrupt Status Register0_2 
    uint32 ExtRsvd0irqmask1_2;  	//RSVD0 Interrupt Mask Register1_2 
    uint32 ExtRsvd0irqstatus1_2;  	//RSVD0 Interrupt Status Register1_2 
    uint32 ExtRsvd0irqmask2_2;  	//RSVD0 Interrupt Mask Register2_2 
    uint32 ExtRsvd0irqstatus2_2;  	//RSVD0 Interrupt Status Register2_2 
    uint32 ExtRsvd0irqmask3_2;  	//RSVD0 Interrupt Mask Register3_2 
    uint32 ExtRsvd0irqstatus3_2;  	//RSVD0 Interrupt Status Register3_2 
    uint32 ExtRsvd0irqsense2;  		//rSVD0 Interrupt Sense Register2 
    uint32 ExtPeriphirqmask0_2;  	//Periph Interrupt Mask Register0_2 
    uint32 ExtPeriphirqstatus0_2;   //Periph Interrupt Status Register0_2 
    uint32 ExtPeriphirqmask1_2;  	//Periph Interrupt Mask Register1_2 
    uint32 ExtPeriphirqstatus1_2;  	//Periph Interrupt Status Register1_2 
	union {
	 struct {
		uint32 ExtPeriphirqmask2_2;  	//Periph Interrupt Mask Register2_2 
		uint32 ExtPeriphirqstatus2_2;  	//Periph Interrupt Status Register2_2 
		uint32 ExtPeriphirqmask3_2;  	//RSVD0 Interrupt Mask Register3_2 
		uint32 ExtPeriphirqstatus3_2;  	//Periph Interrupt Status Register3_2 
	 };
	 IrqControl_t  IrqControl[2];    /* (40) (48)*/
	};
    uint32 ExtPeriphirqsense2;  	//Periph Interrupt Sense Register2 
} ExtIntr;
#define PERFEXT ((volatile ExtIntr * const) PERF_EXT_INT)
/********************** Perf ext intr block end **************/

/***********************************************************/
/*                    Dbg perf block definition(PERF)      */
/***********************************************************/
 typedef struct DebugPerf {
   uint32 Dbg_Features;  	   // Features Register for Software debug 
   uint32 Dbg_SoftwareDebug1;  //	Software debug register 1 
   uint32 Dbg_SoftwareDebug2;  //	Software debug register 2 
   uint32 Dbg_SoftwareDebug3;  //	Software debug register 3 
   uint32 Dbg_SoftwareDebug4;  //	Software debug register 4 
   uint32 Dbg_extirqmuxsel0_1; // ExtIrq Mux select 1 
#define  EXT_IRQ_MASK_HIGH   0x000001E0
#define  EXT_IRQ_OFF_HIGH    4
} DebugPerf;
#define DBGPERF ((volatile DebugPerf * const) DBG_PERF)
/********************** Dbg perf block end **************/

/***********************************************************/
/*                    UART block definition(PERF)          */
/***********************************************************/
typedef struct Uart {
  byte          unused0;
  byte          control;
#define BRGEN           0x80    /* Control register bit defs */
#define TXEN            0x40
#define RXEN            0x20
#define LOOPBK          0x10
#define TXPARITYEN      0x08
#define TXPARITYEVEN    0x04
#define RXPARITYEN      0x02
#define RXPARITYEVEN    0x01

  byte          config;
#define XMITBREAK       0x40
#define BITS5SYM        0x00
#define BITS6SYM        0x10
#define BITS7SYM        0x20
#define BITS8SYM        0x30
#define ONESTOP         0x07
#define TWOSTOP         0x0f
  /* 4-LSBS represent STOP bits/char
   * in 1/8 bit-time intervals.  Zero
   * represents 1/8 stop bit interval.
   * Fifteen represents 2 stop bits.
   */
  byte          fifoctl;
#define RSTTXFIFOS      0x80
#define RSTRXFIFOS      0x40
#define RSTTXCHARDONE   0x20
#define UART_RXTIMEOUT  0x1f

  /* 5-bit TimeoutCnt is in low bits of this register.
   *  This count represents the number of characters 
   *  idle times before setting receive Irq when below threshold
   */
  uint32        baudword;
  /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
   */

  byte          txf_levl;       /* Read-only fifo depth */
  byte          rxf_levl;       /* Read-only fifo depth */
  byte          fifocfg;        /* Upper 4-bits are TxThresh, Lower are
                                 *      RxThreshold.  Irq can be asserted
                                 *      when rx fifo> thresh, txfifo<thresh
                                 */
  byte          prog_out;       /* Set value of DTR (Bit0), RTS (Bit1)
                                 *  if these bits are also enabled to GPIO_o
                                 */
#define UART_DTR_OUT    0x01
#define UART_RTS_OUT    0x02
  byte          unused1;
  byte          DeltaIPEdgeNoSense;     /* Low 4-bits, set corr bit to 1 to 
                                         * detect irq on rising AND falling 
                                         * edges for corresponding GPIO_i
                                         * if enabled (edge insensitive)
                                         */
  byte          DeltaIPConfig_Mask;     /* Upper 4 bits: 1 for posedge sense
                                         *      0 for negedge sense if
                                         *      not configured for edge
                                         *      insensitive (see above)
                                         * Lower 4 bits: Mask to enable change
                                         *  detection IRQ for corresponding
                                         *  GPIO_i
                                         */
  byte          DeltaIP_SyncIP;         /* Upper 4 bits show which bits
                                         *  have changed (may set IRQ).
                                         *  read automatically clears bit
                                         * Lower 4 bits are actual status
                                         */

  uint16        intMask;          /* Same Bit defs for Mask and status */
  uint16        intStatus;
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000
#define TXCHARDONE      0x8000

  uint16        unused2;
  uint16        Data;                   /* Write to TX, Read from RX */
  uint32        unused3;
  uint32        unused4;

} Uart;
#define UART ((volatile Uart * const) UART_BASE)
/********************** Uart block end ********************/

/***********************************************************/
/*                    MDIO block definition(PERF)          */
/***********************************************************/
typedef struct MDIOExtBase {
  uint32 MDIO_PerCmd;  //MDIO Command Register 
#define MDIO_DATA_ADDR_MASK	0x0000FFFF
#define	MDIO_REGISTER_MASK  0X001F0000				   
#define	MDIO_PHY_MASK  		0X03E00000
#define MDIO_OPCODE_MASK	0x0C000000
#define MDIO_FAIL			(1 << 28)				   
#define MDIO_BUSY			(1 << 29)				   
  uint32 MDIO_PerCfg;  //MDIO Configuration Register
#define MDIO_CLAUSE_MASK		1
#define MDIO_CLK_DIVIDER_MASK 	0x000007F0
#define MDIO_SUPRESS_PREAMBLE   (1 << 12)					    
} MDIOExtBase;

#define MDIOEXTERN ((volatile MDIOExtBase * const) MDIO_EXT_BASE)
#define MDIOEGPHY ((volatile MDIOExtBase * const) (MDIO_EXT_BASE + 0x10))
#define MDIOSATA ((volatile MDIOExtBase * const) (MDIO_EXT_BASE + 0x20))
#define MDIOAE ((volatile MDIOExtBase * const) (MDIO_EXT_BASE + 0x30))
/********************** MDIO block end ********************/

/***********************************************************/
/*                    USIM block definition(PERF)          */
/***********************************************************/
typedef struct UsimBase {

    uint32 UsimScr;  		//sim Control Register 
    uint32 UsimSsr;  		//sim Status Register 
    uint32 UsimSdr;  		//sim Data Register 
    uint32 UsimSier;  		//siM Interrupt Enable Register 
    uint32 UsimSfcr;  		//siM FIFO Control Register 
    uint32 UsimSecgtr;  	//SIM Extra Character Guard Time Register 
    uint32 UsimStgtr;  		//sIM Turnaround Guard Time Register 
    uint32 UsimSgccr;  		//sIM Generic Counter Compare Register 
    uint32 UsimSgcvr;  		//sIM Generic Counter Value Register 
    uint32 UsimScdr;  		//siM Clock Divide Register 
    uint32 UsimSfdrr;  		//sIM F/D Ratio Register 
    uint32 UsimSesr;  		//siM Extra Sample Register 
    uint32 UsimSimdebug;	//SIM Debug Register 
    uint32 UsimSrtor;  		//sIM Received Time Out Register 
    uint32 UsimSipver;  	//SIM Controller IP version 
    uint32 UsimSesdcr;  	//SIM Card Detection and Emergency Shutdown Control Register 
    uint32 UsimSesdisr;  	//SIM Card Detection and Emergency Shutdown Interrupt Status Register 
    uint32 UsimScardsr;  	//SIM Card Status Control and Status Register 
    uint32 UsimSldocr;  	//SIM LDO Controler Register 
} UsimBase;
#define USIM ((volatile UsimBase * const) USIM_BASE)
/********************** USIM block end *********************/

/***********************************************************/
/*                    I2C block definition(PERF)           */
/***********************************************************/
typedef struct I2CControl {
  uint32        ChipAddress;            /* 0x0 */
#define I2C_CHIP_ADDRESS_MASK           0x000000f7
#define I2C_CHIP_ADDRESS_SHIFT          0x1
  uint32        DataIn0;                /* 0x4 */
  uint32        DataIn1;                /* 0x8 */
  uint32        DataIn2;                /* 0xc */
  uint32        DataIn3;                /* 0x10 */
  uint32        DataIn4;                /* 0x14 */
  uint32        DataIn5;                /* 0x18 */
  uint32        DataIn6;                /* 0x1c */
  uint32        DataIn7;                /* 0x20 */
  uint32        CntReg;                 /* 0x24 */
#define I2C_CNT_REG1_SHIFT              0x0
#define I2C_CNT_REG2_SHIFT              0x6
  uint32        CtlReg;                 /* 0x28 */
#define I2C_CTL_REG_DTF_MASK            0x00000003
#define I2C_CTL_REG_DTF_WRITE           0x0
#define I2C_CTL_REG_DTF_READ            0x1
#define I2C_CTL_REG_DTF_READ_AND_WRITE  0x2
#define I2C_CTL_REG_DTF_WRITE_AND_READ  0x3
#define I2C_CTL_REG_DEGLITCH_DISABLE    0x00000004
#define I2C_CTL_REG_DELAY_DISABLE       0x00000008
#define I2C_CTL_REG_SCL_SEL_MASK        0x00000030
#define I2C_CTL_REG_SCL_CLK_375KHZ      0x00000000
#define I2C_CTL_REG_SCL_CLK_390KHZ      0x00000010
#define I2C_CTL_REG_SCL_CLK_187_5KHZ    0x00000020
#define I2C_CTL_REG_SCL_CLK_200KHZ      0x00000030
#define I2C_CTL_REG_INT_ENABLE          0x00000040
#define I2C_CTL_REG_DIV_CLK             0x00000080
  uint32        IICEnable;              /* 0x2c */
#define I2C_IIC_ENABLE                  0x00000001
#define I2C_IIC_INTRP                   0x00000002
#define I2C_IIC_NO_ACK                  0x00000004
#define I2C_IIC_NO_STOP                 0x00000010
#define I2C_IIC_NO_START                0x00000020
  uint32        DataOut0;               /* 0x30 */
  uint32        DataOut1;               /* 0x34 */
  uint32        DataOut2;               /* 0x38 */
  uint32        DataOut3;               /* 0x3c */
  uint32        DataOut4;               /* 0x40 */
  uint32        DataOut5;               /* 0x44 */
  uint32        DataOut6;               /* 0x48 */
  uint32        DataOut7;               /* 0x4c */
  uint32        CtlHiReg;               /* 0x50 */
#define I2C_CTLHI_REG_WAIT_DISABLE      0x00000001
#define I2C_CTLHI_REG_IGNORE_ACK        0x00000002
#define I2C_CTLHI_REG_DATA_REG_SIZE     0x00000040
  uint32        SclParam;               /* 0x54 */
} I2CControl;

#define I2C ((volatile I2CControl * const) I2C_BASE)
/********************** I2C block end **********************/

/***********************************************************/
/*                    LED block definition(PERF)           */
/***********************************************************/
#pragma pack(push, 4)
typedef struct LedControl {
    uint32  ledInit;
#define LED_LED_TEST                (1 << 31)
#define LED_SHIFT_TEST              (1 << 30)
#define LED_SER_SHIFT_CLK_SEL_SHIFT	24
#define LED_SER_SHIFT_CLK_SEL_MASK	(0x3 << LED_CLOCK_SEL_SHIFT)
#define LED_SERIAL_SHIFT_FRAME_POL	(1 << 23)
#define LED_SERIAL_SHIFT_FRAME_EN	(1 << 22)
#define LED_SERIAL_SHIFT_MODE_SHIFT	20
#define LED_SERIAL_SHIFT_MODE_MASK	(0x3 << LED_SERIAL_SHIFT_MODE_SHIFT)
#define LED_SERIAL_LED_SHIFT_DIR    (1 << 16)
#define LED_SERIAL_LED_DATA_PPOL    (1 << 15)
#define LEDSERIAL_LED_CLK_NPOL      (1 << 14)
#define LED_SERIAL_LED_MUX_SEL      (1 << 13)
#define LED_SERIAL_LED_EN           (1 << 12)
#define LED_FAST_INTV_SHIFT         6
#define LED_FAST_INTV_MASK          (0x3F<<LED_FAST_INTV_SHIFT)
#define LED_SLOW_INTV_SHIFT         0
#define LED_SLOW_INTV_MASK          (0x3F<<LED_SLOW_INTV_SHIFT)
#define LED_INTERVAL_20MS           1

    uint64  ledMode;
#define LED_MODE_MASK               (uint64)0x3
#define LED_MODE_OFF                (uint64)0x0
#define LED_MODE_FLASH              (uint64)0x1
#define LED_MODE_BLINK              (uint64)0x2
#define LED_MODE_ON                 (uint64)0x3

    uint32  ledHWDis;
#define LED_GPHY0_SPD0              0
#define LED_GPHY0_SPD1              1
#define LED_INET_ACT                8
#define LED_EPHY0_ACT               9
#define LED_EPHY1_ACT               10
#define LED_EPHY2_ACT               11
#define LED_GPHY0_ACT               12
#define LED_EPHY0_SPD               13
#define LED_EPHY1_SPD               14
#define LED_EPHY2_SPD               15
#define LED_USB_ACT                 23


    uint32  ledStrobe;
    uint32  ledLinkActSelHigh;
#define LED_4_ACT_SHIFT             0
#define LED_5_ACT_SHIFT             4
#define LED_6_ACT_SHIFT             8
#define LED_7_ACT_SHIFT             12
#define LED_4_LINK_SHIFT            16
#define LED_5_LINK_SHIFT            20
#define LED_6_LINK_SHIFT            24
#define LED_7_LINK_SHIFT            28
    uint32  ledLinkActSelLow;
#define LED_0_ACT_SHIFT             0
#define LED_1_ACT_SHIFT             4
#define LED_2_ACT_SHIFT             8
#define LED_3_ACT_SHIFT             12
#define LED_0_LINK_SHIFT            16
#define LED_1_LINK_SHIFT            20
#define LED_2_LINK_SHIFT            24
#define LED_3_LINK_SHIFT            28

    uint32  ledReadback;
    uint32  ledSerialMuxSelect;
	uint32	ledXor;
} LedControl;
#pragma pack(pop)

#define LED ((volatile LedControl * const) LED_BASE)

#define GPIO_NUM_TO_LED_MODE_SHIFT(X) \
    ((((X) & BP_GPIO_NUM_MASK) < 8) ? (32 + (((X) & BP_GPIO_NUM_MASK) << 1)) : \
    ((((X) & BP_GPIO_NUM_MASK) - 8) << 1))



/********************** Led control block end **************/

/***********************************************************/
/*                    HSSPI block definition(PERF)         */
/***********************************************************/
#define __mask(end, start)      (((1 << ((end - start) + 1)) - 1) << start)
typedef struct HsSpiControl {

  uint32    hs_spiGlobalCtrl;   // 0x0000
#define HS_SPI_MOSI_IDLE        (1 << 18)
#define HS_SPI_CLK_POLARITY      (1 << 17)
#define HS_SPI_CLK_GATE_SSOFF       (1 << 16)
#define HS_SPI_PLL_CLK_CTRL     (8)
#define HS_SPI_PLL_CLK_CTRL_MASK    __mask(15, HS_SPI_PLL_CLK_CTRL)
#define HS_SPI_SS_POLARITY      (0)
#define HS_SPI_SS_POLARITY_MASK     __mask(7, HS_SPI_SS_POLARITY)

  uint32    hs_spiExtTrigCtrl;  // 0x0004
#define HS_SPI_TRIG_RAW_STATE   (24)
#define HS_SPI_TRIG_RAW_STATE_MASK  __mask(31, HS_SPI_TRIG_RAW_STATE)
#define HS_SPI_TRIG_LATCHED     (16)
#define HS_SPI_TRIG_LATCHED_MASK    __mask(23, HS_SPI_TRIG_LATCHED)
#define HS_SPI_TRIG_SENSE       (8)
#define HS_SPI_TRIG_SENSE_MASK      __mask(15, HS_SPI_TRIG_SENSE)
#define HS_SPI_TRIG_TYPE        (0)
#define HS_SPI_TRIG_TYPE_MASK       __mask(7, HS_SPI_TRIG_TYPE)
#define HS_SPI_TRIG_TYPE_EDGE       (0)
#define HS_SPI_TRIG_TYPE_LEVEL      (1)

  uint32    hs_spiIntStatus;    // 0x0008
#define HS_SPI_IRQ_PING1_USER       (28)
#define HS_SPI_IRQ_PING1_USER_MASK  __mask(31, HS_SPI_IRQ_PING1_USER)
#define HS_SPI_IRQ_PING0_USER       (24)
#define HS_SPI_IRQ_PING0_USER_MASK  __mask(27, HS_SPI_IRQ_PING0_USER)

#define HS_SPI_IRQ_PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQ_PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQ_PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQ_PING1_RX_OVER    (1 << 9)
#define HS_SPI_IRQ_PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQ_PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQ_PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQ_PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQ_PING0_RX_OVER    (1 << 1)
#define HS_SPI_IRQ_PING0_CMD_DONE   (1 << 0)

  uint32    hs_spiIntStatusMasked;  // 0x000C
#define HS_SPI_IRQSM__PING1_USER    (28)
#define HS_SPI_IRQSM__PING1_USER_MASK   __mask(31, HS_SPI_IRQSM__PING1_USER)
#define HS_SPI_IRQSM__PING0_USER    (24)
#define HS_SPI_IRQSM__PING0_USER_MASK   __mask(27, HS_SPI_IRQSM__PING0_USER)

#define HS_SPI_IRQSM__PING1_CTRL_INV    (1 << 12)
#define HS_SPI_IRQSM__PING1_POLL_TOUT   (1 << 11)
#define HS_SPI_IRQSM__PING1_TX_UNDER    (1 << 10)
#define HS_SPI_IRQSM__PING1_RX_OVER (1 << 9)
#define HS_SPI_IRQSM__PING1_CMD_DONE    (1 << 8)

#define HS_SPI_IRQSM__PING0_CTRL_INV    (1 << 4)
#define HS_SPI_IRQSM__PING0_POLL_TOUT   (1 << 3)
#define HS_SPI_IRQSM__PING0_TX_UNDER    (1 << 2)
#define HS_SPI_IRQSM__PING0_RX_OVER     (1 << 1)
#define HS_SPI_IRQSM__PING0_CMD_DONE    (1 << 0)

  uint32    hs_spiIntMask;      // 0x0010
#define HS_SPI_IRQM_PING1_USER      (28)
#define HS_SPI_IRQM_PING1_USER_MASK __mask(31, HS_SPI_IRQM_PING1_USER)
#define HS_SPI_IRQM_PING0_USER      (24)
#define HS_SPI_IRQM_PING0_USER_MASK __mask(27, HS_SPI_IRQM_PING0_USER)

#define HS_SPI_IRQM_PING1_CTRL_INV  (1 << 12)
#define HS_SPI_IRQM_PING1_POLL_TOUT (1 << 11)
#define HS_SPI_IRQM_PING1_TX_UNDER  (1 << 10)
#define HS_SPI_IRQM_PING1_RX_OVER   (1 << 9)
#define HS_SPI_IRQM_PING1_CMD_DONE  (1 << 8)

#define HS_SPI_IRQM_PING0_CTRL_INV  (1 << 4)
#define HS_SPI_IRQM_PING0_POLL_TOUT (1 << 3)
#define HS_SPI_IRQM_PING0_TX_UNDER  (1 << 2)
#define HS_SPI_IRQM_PING0_RX_OVER   (1 << 1)
#define HS_SPI_IRQM_PING0_CMD_DONE  (1 << 0)

#define HS_SPI_INTR_CLEAR_ALL       (0xFF001F1F)

  uint32    hs_spiFlashCtrl;    // 0x0014
#define HS_SPI_FCTRL_MB_ENABLE      (23)
#define HS_SPI_FCTRL_SS_NUM         (20)
#define HS_SPI_FCTRL_SS_NUM_MASK    __mask(22, HS_SPI_FCTRL_SS_NUM)
#define HS_SPI_FCTRL_PROFILE_NUM    (16)
#define HS_SPI_FCTRL_PROFILE_NUM_MASK   __mask(18, HS_SPI_FCTRL_PROFILE_NUM)
#define HS_SPI_FCTRL_DUMMY_BYTES    (10)
#define HS_SPI_FCTRL_DUMMY_BYTES_MASK   __mask(11, HS_SPI_FCTRL_DUMMY_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES     (8)
#define HS_SPI_FCTRL_ADDR_BYTES_MASK    __mask(9, HS_SPI_FCTRL_ADDR_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES_2   (0)
#define HS_SPI_FCTRL_ADDR_BYTES_3   (1)
#define HS_SPI_FCTRL_ADDR_BYTES_4   (2)
#define HS_SPI_FCTRL_READ_OPCODE    (0)
#define HS_SPI_FCTRL_READ_OPCODE_MASK   __mask(7, HS_SPI_FCTRL_READ_OPCODE)

  uint32    hs_spiFlashAddrBase;    // 0x0018

} HsSpiControl;

typedef struct HsSpiPingPong {

    uint32 command;
#define HS_SPI_SS_NUM (12)
#define ZSI_SPI_DEV_ID           5     // SS_N[5] connected to APM/PCM block for use by MSIF/ZDS interfaces
#define HS_SPI_PROFILE_NUM (8)
#define HS_SPI_TRIGGER_NUM (4)
#define HS_SPI_COMMAND_VALUE (0)
    #define HS_SPI_COMMAND_NOOP (0)
    #define HS_SPI_COMMAND_START_NOW (1)
    #define HS_SPI_COMMAND_START_TRIGGER (2)
    #define HS_SPI_COMMAND_HALT (3)
    #define HS_SPI_COMMAND_FLUSH (4)

    uint32 status;
#define HS_SPI_ERROR_BYTE_OFFSET (16)
#define HS_SPI_WAIT_FOR_TRIGGER (2)
#define HS_SPI_SOURCE_BUSY (1)
#define HS_SPI_SOURCE_GNT (0)

    uint32 fifo_status;
    uint32 control;

} HsSpiPingPong;
typedef struct HsSpiProfile {

    uint32 clk_ctrl;
#define HS_SPI_ACCUM_RST_ON_LOOP (15)
#define HS_SPI_SPI_CLK_2X_SEL (14)
#define HS_SPI_FREQ_CTRL_WORD (0)

    uint32 signal_ctrl;
#define	HS_SPI_ASYNC_INPUT_PATH (1 << 16)
#define	HS_SPI_LAUNCH_RISING    (1 << 13)
#define	HS_SPI_LATCH_RISING     (1 << 12)

    uint32 mode_ctrl;
#define HS_SPI_PREPENDBYTE_CNT (24)
#define HS_SPI_MODE_ONE_WIRE (20)
#define HS_SPI_MULTIDATA_WR_SIZE (18)
#define HS_SPI_MULTIDATA_RD_SIZE (16)
#define HS_SPI_MULTIDATA_WR_STRT (12)
#define HS_SPI_MULTIDATA_RD_STRT (8)
#define HS_SPI_FILLBYTE (0)

    uint32 polling_config;
    uint32 polling_and_mask;
    uint32 polling_compare;
    uint32 polling_timeout;
    uint32 reserved;

} HsSpiProfile;
#define HS_SPI_OP_CODE 13
    #define HS_SPI_OP_SLEEP (0)
    #define HS_SPI_OP_READ_WRITE (1)
    #define HS_SPI_OP_WRITE (2)
    #define HS_SPI_OP_READ (3)
    #define HS_SPI_OP_SETIRQ (4)

#define HS_SPI ((volatile HsSpiControl * const) HSSPIM_BASE)
#define HS_SPI_PINGPONG0 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0x80))
#define HS_SPI_PINGPONG1 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0xc0))
#define HS_SPI_PROFILES ((volatile HsSpiProfile * const) (HSSPIM_BASE+0x100))
#define HS_SPI_FIFO0 ((volatile uint8 * const) (HSSPIM_BASE+0x200))
#define HS_SPI_FIFO1 ((volatile uint8 * const) (HSSPIM_BASE+0x400))
/********************** HSSPI block end ********************/

/***********************************************************/
/*                    NAND block definition(PERF)          */
/***********************************************************/
typedef struct NandCtrlRegs {
    uint32 NandRevision;            /* NAND Revision */
    uint32 NandCmdStart;            /* Nand Flash Command Start */
#define NCMD_MASK           0x1f000000
#define NCMD_LOW_LEVEL_OP   0x10000000
#define NCMD_PARAM_CHG_COL  0x0f000000
#define NCMD_PARAM_READ     0x0e000000
#define NCMD_BLK_LOCK_STS   0x0d000000
#define NCMD_BLK_UNLOCK     0x0c000000
#define NCMD_BLK_LOCK_DOWN  0x0b000000
#define NCMD_BLK_LOCK       0x0a000000
#define NCMD_FLASH_RESET    0x09000000
#define NCMD_BLOCK_ERASE    0x08000000
#define NCMD_DEV_ID_READ    0x07000000
#define NCMD_COPY_BACK      0x06000000
#define NCMD_PROGRAM_SPARE  0x05000000
#define NCMD_PROGRAM_PAGE   0x04000000
#define NCMD_STS_READ       0x03000000
#define NCMD_SPARE_READ     0x02000000
#define NCMD_PAGE_READ      0x01000000

    uint32 NandCmdExtAddr;          /* Nand Flash Command Extended Address */
    uint32 NandCmdAddr;             /* Nand Flash Command Address */
    uint32 NandCmdEndAddr;          /* Nand Flash Command End Address */
    uint32 NandNandBootConfig;      /* Nand Flash Boot Config */
#define NBC_CS_LOCK         0x80000000
#define NBC_AUTO_DEV_ID_CFG 0x40000000
#define NBC_WR_PROT_BLK0    0x10000000
#define NBC_EBI_CS7_USES_NAND (1<<15)
#define NBC_EBI_CS6_USES_NAND (1<<14)
#define NBC_EBI_CS5_USES_NAND (1<<13)
#define NBC_EBI_CS4_USES_NAND (1<<12)
#define NBC_EBI_CS3_USES_NAND (1<<11)
#define NBC_EBI_CS2_USES_NAND (1<<10)
#define NBC_EBI_CS1_USES_NAND (1<< 9)
#define NBC_EBI_CS0_USES_NAND (1<< 8)
#define NBC_EBC_CS7_SEL       (1<< 7)
#define NBC_EBC_CS6_SEL       (1<< 6)
#define NBC_EBC_CS5_SEL       (1<< 5)
#define NBC_EBC_CS4_SEL       (1<< 4)
#define NBC_EBC_CS3_SEL       (1<< 3)
#define NBC_EBC_CS2_SEL       (1<< 2)
#define NBC_EBC_CS1_SEL       (1<< 1)
#define NBC_EBC_CS0_SEL       (1<< 0)

    uint32 NandCsNandXor;           /* Nand Flash EBI CS Address XOR with */
                                    /*   1FC0 Control */
    uint32 NandReserved1;
    uint32 NandSpareAreaReadOfs0;   /* Nand Flash Spare Area Read Bytes 0-3 */
    uint32 NandSpareAreaReadOfs4;   /* Nand Flash Spare Area Read Bytes 4-7 */
    uint32 NandSpareAreaReadOfs8;   /* Nand Flash Spare Area Read Bytes 8-11 */
    uint32 NandSpareAreaReadOfsC;   /* Nand Flash Spare Area Read Bytes 12-15*/
    uint32 NandSpareAreaWriteOfs0;  /* Nand Flash Spare Area Write Bytes 0-3 */
    uint32 NandSpareAreaWriteOfs4;  /* Nand Flash Spare Area Write Bytes 4-7 */
    uint32 NandSpareAreaWriteOfs8;  /* Nand Flash Spare Area Write Bytes 8-11*/
    uint32 NandSpareAreaWriteOfsC;  /* Nand Flash Spare Area Write Bytes12-15*/
    uint32 NandAccControl;          /* Nand Flash Access Control */
#define NAC_RD_ECC_EN       0x80000000
#define NAC_WR_ECC_EN       0x40000000
#define NAC_RD_ECC_BLK0_EN  0x20000000
#define NAC_FAST_PGM_RDIN   0x10000000
#define NAC_RD_ERASED_ECC_EN 0x08000000
#define NAC_PARTIAL_PAGE_EN 0x04000000
#define NAC_WR_PREEMPT_EN   0x02000000
#define NAC_PAGE_HIT_EN     0x01000000
#define NAC_ECC_LVL_0_SHIFT 20
#define NAC_ECC_LVL_0_MASK  0x00f00000
#define NAC_ECC_LVL_SHIFT   16
#define NAC_ECC_LVL_MASK    0x000f0000
#define NAC_ECC_LVL_DISABLE 0
#define NAC_ECC_LVL_BCH_1   1
#define NAC_ECC_LVL_BCH_2   2
#define NAC_ECC_LVL_BCH_3   3
#define NAC_ECC_LVL_BCH_4   4
#define NAC_ECC_LVL_BCH_5   5
#define NAC_ECC_LVL_BCH_6   6
#define NAC_ECC_LVL_BCH_7   7
#define NAC_ECC_LVL_BCH_8   8
#define NAC_ECC_LVL_BCH_9   9
#define NAC_ECC_LVL_BCH_10  10
#define NAC_ECC_LVL_BCH_11  11
#define NAC_ECC_LVL_BCH_12  12
#define NAC_ECC_LVL_RESVD_1 13
#define NAC_ECC_LVL_RESVD_2 14
#define NAC_ECC_LVL_HAMMING 15
#define NAC_SPARE_SZ_0_SHIFT 8
#define NAC_SPARE_SZ_0_MASK 0x00003f00
#define NAC_SPARE_SZ_SHIFT  0
#define NAC_SPARE_SZ_MASK   0x0000003f
    uint32 NandReserved2;
    uint32 NandConfig;              /* Nand Flash Config */
#define NC_CONFIG_LOCK      0x80000000
#define NC_BLK_SIZE_MASK    0x70000000
#define NC_BLK_SIZE_2048K   0x60000000
#define NC_BLK_SIZE_1024K   0x50000000
#define NC_BLK_SIZE_512K    0x30000000
#define NC_BLK_SIZE_128K    0x10000000
#define NC_BLK_SIZE_16K     0x00000000
#define NC_BLK_SIZE_8K      0x20000000
#define NC_BLK_SIZE_256K    0x40000000
#define NC_DEV_SIZE_MASK    0x0f000000
#define NC_DEV_SIZE_SHIFT   24
#define NC_DEV_WIDTH_MASK   0x00800000
#define NC_DEV_WIDTH_16     0x00800000
#define NC_DEV_WIDTH_8      0x00000000
#define NC_PG_SIZE_MASK     0x00300000
#define NC_PG_SIZE_8K       0x00300000
#define NC_PG_SIZE_4K       0x00200000
#define NC_PG_SIZE_2K       0x00100000
#define NC_PG_SIZE_512B     0x00000000
#define NC_FUL_ADDR_MASK    0x00070000
#define NC_FUL_ADDR_SHIFT   16
#define NC_BLK_ADDR_MASK    0x00000700
#define NC_BLK_ADDR_SHIFT   8

    uint32 NandReserved3;
    uint32 NandTiming1;             /* Nand Flash Timing Parameters 1 */
#define NT_TREH_MASK        0x000f0000
#define NT_TREH_SHIFT       16
#define NT_TRP_MASK         0x00f00000
#define NT_TRP_SHIFT        20
    uint32 NandTiming2;             /* Nand Flash Timing Parameters 2 */
#define NT_TREAD_MASK       0x0000000f
#define NT_TREAD_SHIFT      0
    uint32 NandSemaphore;           /* Semaphore */
    uint32 NandReserved4;
    uint32 NandFlashDeviceId;       /* Nand Flash Device ID */
    uint32 NandFlashDeviceIdExt;    /* Nand Flash Extended Device ID */
    uint32 NandBlockLockStatus;     /* Nand Flash Block Lock Status */
    uint32 NandIntfcStatus;         /* Nand Flash Interface Status */
#define NIS_CTLR_READY      0x80000000
#define NIS_FLASH_READY     0x40000000
#define NIS_CACHE_VALID     0x20000000
#define NIS_SPARE_VALID     0x10000000
#define NIS_FLASH_STS_MASK  0x000000ff
#define NIS_WRITE_PROTECT   0x00000080
#define NIS_DEV_READY       0x00000040
#define NIS_PGM_ERASE_ERROR 0x00000001

    uint32 NandEccCorrExtAddr;      /* ECC Correctable Error Extended Address*/
    uint32 NandEccCorrAddr;         /* ECC Correctable Error Address */
    uint32 NandEccUncExtAddr;       /* ECC Uncorrectable Error Extended Addr */
    uint32 NandEccUncAddr;          /* ECC Uncorrectable Error Address */
    uint32 NandReadErrorCount;      /* Read Error Count */
    uint32 NandCorrStatThreshold;   /* Correctable Error Reporting Threshold */
    uint32 NandOnfiStatus;          /* ONFI Status */
    uint32 NandOnfiDebugData;       /* ONFI Debug Data */
    uint32 NandFlashReadExtAddr;    /* Flash Read Data Extended Address */
    uint32 NandFlashReadAddr;       /* Flash Read Data Address */
    uint32 NandProgramPageExtAddr;  /* Page Program Extended Address */
    uint32 NandProgramPageAddr;     /* Page Program Address */
    uint32 NandCopyBackExtAddr;     /* Copy Back Extended Address */
    uint32 NandCopyBackAddr;        /* Copy Back Address */
    uint32 NandBlockEraseExtAddr;   /* Block Erase Extended Address */
    uint32 NandBlockEraseAddr;      /* Block Erase Address */
    uint32 NandInvReadExtAddr;      /* Flash Invalid Data Extended Address */
    uint32 NandInvReadAddr;         /* Flash Invalid Data Address */
    uint32 NandReserved5[2];
    uint32 NandBlkWrProtect;        /* Block Write Protect Enable and Size */
                                    /*   for EBI_CS0b */
    uint32 NandReserved6[3];    
    uint32 NandAccControlCs1;       /* Nand Flash Access Control */
    uint32 NandConfigCs1;           /* Nand Flash Config */
    uint32 NandTiming1Cs1;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs1;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs2;       /* Nand Flash Access Control */
    uint32 NandConfigCs2;           /* Nand Flash Config */
    uint32 NandTiming1Cs2;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs2;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs3;       /* Nand Flash Access Control */
    uint32 NandConfigCs3;           /* Nand Flash Config */
    uint32 NandTiming1Cs3;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs3;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs4;       /* Nand Flash Access Control */
    uint32 NandConfigCs4;           /* Nand Flash Config */
    uint32 NandTiming1Cs4;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs4;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs5;       /* Nand Flash Access Control */
    uint32 NandConfigCs5;           /* Nand Flash Config */
    uint32 NandTiming1Cs5;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs5;          /* Nand Flash Timing Parameters 2 */
    uint32 NandReserved7[4];
    uint32 NandSpareAreaReadOfs10;  /* Nand Flash Spare Area Read Bytes 16-19 */
    uint32 NandSpareAreaReadOfs14;  /* Nand Flash Spare Area Read Bytes 20-23 */
    uint32 NandSpareAreaReadOfs18;  /* Nand Flash Spare Area Read Bytes 24-27 */
    uint32 NandSpareAreaReadOfs1C;  /* Nand Flash Spare Area Read Bytes 28-31 */
    uint32 NandSpareAreaWriteOfs10; /* Nand Flash Spare Area Write Bytes 16-19 */
    uint32 NandSpareAreaWriteOfs14; /* Nand Flash Spare Area Write Bytes 20-23 */
    uint32 NandSpareAreaWriteOfs18; /* Nand Flash Spare Area Write Bytes 24-27 */
    uint32 NandSpareAreaWriteOfs1C; /* Nand Flash Spare Area Write Bytes 28-31 */
    uint32 NandReserved8[10];
    uint32 NandLlOpNand;            /* Flash Low Level Operation */
    uint32 NandLlRdData;            /* Nand Flash Low Level Read Data */
} NandCtrlRegs;

#define NAND ((volatile NandCtrlRegs * const) NAND_REG_BASE)

#define NAND_CACHE_BUFFER ((volatile uint8 * const) NAND_CACHE_BASE);

/********************** NAND block end **************/

/***********************************************************/
/*                    IC block definition (PMC)           */
/***********************************************************/
typedef struct PmcCtrlReg {
	/* 0x00 */
	uint32 l1Irq4keMask;
	uint32 l1Irq4keStatus;
	uint32 l1IrqMipsMask;
	uint32 l1IrqMipsStatus;
	/* 0x10 */
	uint32 l2IrqGpMask;
	uint32 l2IrqGpStatus;
	uint32 gpTmr0Ctl;
	uint32 gpTmr0Cnt;
	/* 0x20 */
	uint32 gpTmr1Ctl;
	uint32 gpTmr1Cnt;
	uint32 hostMboxIn;
	uint32 hostMboxOut;
	/* 0x30 */
	uint32 gpOut;
	uint32 gpIn;
	uint32 gpInIrqMask;
	uint32 gpInIrqStatus;
	/* 0x40 */
	uint32 dmaCtrl;
	uint32 dmaStatus;
	uint32 dma0_3FifoStatus;
	uint32 unused0[3];	/* 0x4c-0x57 */
	/* 0x58 */
	uint32 l1IrqMips1Mask;
	uint32 diagControl;
	/* 0x60 */
	uint32 diagHigh;
	uint32 diagLow;
	uint32 badAddr;
	uint32 addr1WndwMask;
	/* 0x70 */
	uint32 addr1WndwBaseIn;
	uint32 addr1WndwBaseOut;
	uint32 addr2WndwMask;
	uint32 addr2WndwBaseIn;
	/* 0x80 */
	uint32 addr2WndwBaseOut;
	uint32 scratch;
	uint32 tm;
	uint32 softResets;
	/* 0x90 */
	uint32 eb2ubusTimeout;
	uint32 m4keCoreStatus;
	uint32 gpInIrqSense;
	uint32 ubSlaveTimeout;
	/* 0xa0 */
	uint32 diagEn;
	uint32 devTimeout;
	uint32 ubusErrorOutMask;
	uint32 diagCaptStopMask;
	/* 0xb0 */
	uint32 revId;
	uint32 gpTmr2Ctl;
	uint32 gpTmr2Cnt;
	uint32 legacyMode;
	/* 0xc0 */
	uint32 smisbMonitor;
	uint32 diagCtrl;
	uint32 diagStat;
	uint32 diagMask;
	/* 0xd0 */
	uint32 diagRslt;
	uint32 diagCmp;
	uint32 diagCapt;
	uint32 diagCnt;
	/* 0xe0 */
	uint32 diagEdgeCnt;
	uint32 unused1[4];	/* 0xe4-0xf3 */
	/* 0xf4 */
	uint32 iopPeriphBaseAddr;
	uint32 lfsr;
	uint32 unused2;		/* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
	uint32 msgCtrl;		/* 0x00 */
	uint32 msgSts;		/* 0x04 */
	uint32 unused[14];	/* 0x08-0x3f */
	uint32 msgData[16];	/* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
	uint32 msgCtrl;		/* 0x00 */
	uint32 msgSts;		/* 0x04 */
	uint32 unused[13];	/* 0x08-0x3b */
	uint32 msgLast;		/* 0x3c */
	uint32 msgData[16];	/* 0x40-0x7c */
} PmcInFifoReg;

typedef struct PmcDmaReg {
	/* 0x00 */
	uint32 src;
	uint32 dest;
	uint32 cmdList;
	uint32 lenCtl;
	/* 0x10 */
	uint32 rsltSrc;
	uint32 rsltDest;
	uint32 rsltHcs;
	uint32 rsltLenStat;
} PmcDmaReg;

typedef struct PmcTokenReg {
	/* 0x00 */
	uint32 bufSize;
	uint32 bufBase;
	uint32 idx2ptrIdx;
	uint32 idx2ptrPtr;
	/* 0x10 */
	uint32 unused[2];
	uint32 bufSize2;
} PmcTokenReg;

typedef struct PmcPerfPowReg {
	/* 0x00 */
	uint32 dcacheHit;
	uint32 dcacheMiss;
	uint32 icacheHit;
	uint32 icacheMiss;
	/* 0x10 */
	uint32 instnComplete;
	uint32 wtbMerge;
	uint32 wtbNoMerge;
	uint32 itlbHit;
	/* 0x20 */
	uint32 itlbMiss;
	uint32 dtlbHit;
	uint32 dtlbMiss;
	uint32 jtlbHit;
	/* 0x30 */
	uint32 jtlbMiss;
	uint32 powerSubZone;
	uint32 powerMemPda;
	uint32 freqScalarCtrl;
	/* 0x40 */
	uint32 freqScalarMask;
} PmcPerfPowReg;

typedef struct PmcDQMReg {
	/* 0x00 */
	uint32 cfg;
	uint32 _4keLowWtmkIrqMask;
	uint32 mipsLowWtmkIrqMask;
	uint32 lowWtmkIrqMask;
	/* 0x10 */
	uint32 _4keNotEmptyIrqMask;
	uint32 mipsNotEmptyIrqMask;
	uint32 notEmptyIrqSts;
	uint32 queueRst;
	/* 0x20 */
	uint32 notEmptySts;
	uint32 nextAvailMask;
	uint32 nextAvailQueue;
	uint32 mips1LowWtmkIrqMask;
	/* 0x30 */
	uint32 mips1NotEmptyIrqMask;
	uint32 autoSrcPidInsert;
} PmcDQMReg;

typedef struct PmcCntReg {
	uint32 cntr[10];
	uint32 unused[6];	/* 0x28-0x3f */
	uint32 cntrIrqMask;
	uint32 cntrIrqSts;
} PmcCntReg;

typedef struct PmcDqmQCtrlReg {
	uint32 size;
	uint32 cfga;
	uint32 cfgb;
	uint32 cfgc;
} PmcDqmQCtrlReg;

typedef struct PmcDqmQDataReg {
	uint32 word[4];
} PmcDqmQDataReg;

typedef struct PmcDqmQMibReg {
	uint32 qNumFull[32];
	uint32 qNumEmpty[32];
	uint32 qNumPushed[32];
} PmcDqmQMibReg;

typedef struct Pmc {
	uint32 baseReserved;		/* 0x0000 */
	uint32 unused0[1023];
	PmcCtrlReg ctrl;		/* 0x1000 */

	PmcOutFifoReg outFifo;		/* 0x1100 */
	uint32 unused1[32];		/* 0x1180-0x11ff */
	PmcInFifoReg inFifo;		/* 0x1200 */
	uint32 unused2[32];		/* 0x1280-0x12ff */

	PmcDmaReg dma[2];		/* 0x1300 */
	uint32 unused3[48];		/* 0x1340-0x13ff */

	PmcTokenReg token;		/* 0x1400 */
	uint32 unused4[121];		/* 0x141c-0x15ff */

	PmcPerfPowReg perfPower;	/* 0x1600 */
	uint32 unused5[47];		/* 0x1644-0x16ff */

	uint32 msgId[32];		/* 0x1700 */
	uint32 unused6[32];		/* 0x1780-0x17ff */

	PmcDQMReg dqm;			/* 0x1800 */
	uint32 unused7[50];		/* 0x1838-0x18ff */

	PmcCntReg hwCounter;		/* 0x1900 */
	uint32 unused8[46];		/* 0x1948-0x19ff */

	PmcDqmQCtrlReg dqmQCtrl[32];	/* 0x1a00 */
	PmcDqmQDataReg dqmQData[32];	/* 0x1c00 */
	uint32 unused9[64];		/* 0x1e00-0x1eff */

	uint32 qStatus[32];		/* 0x1f00 */
	uint32 unused10[32];		/* 0x1f80-0x1fff */

	PmcDqmQMibReg qMib;		/* 0x2000 */
	uint32 unused11[1952];		/* 0x2180-0x3ffff */

	uint32 sharedMem[8192];		/* 0x4000-0xbffc */
} Pmc;

#define PMC ((volatile Pmc * const) PMC_BASE)

/********************** PMC block end **************/

/***********************************************************/
/*                   PROCMON block definition(PMC)         */
/***********************************************************/
typedef struct PMRingOscillatorControl {
	uint32 control;
	uint32 en_lo;
	uint32 en_mid;
	uint32 en_hi;
	uint32 idle_lo;
	uint32 idle_mid;
	uint32 idle_hi;
} PMRingOscillatorControl;

typedef struct PMMiscControl {
	uint32 gp_out;
	uint32 clock_select;
} PMMiscControl;

typedef struct PMSSBMasterControl {
	uint32 control;
	uint32 wr_data;
	uint32 rd_data;
} PMSSBMasterControl;

typedef struct PMEctrControl {
	uint32 control;
	uint32 interval;
	uint32 thresh_lo;
	uint32 thresh_hi;
	uint32 count;
} PMEctrControl;

typedef struct PMRefclkGen {
	uint32 refclk_ctrl;
	uint32 clkgen_diag;
	uint32 clkgen_ctrl0;
	uint32 clkgen_ctrl1;
	uint32 fcw_est0;
	uint32 fcw_est1;
	uint32 fcw_slewed0;
	uint32 fcw_slewed1;
} PMRefclkGen;

typedef struct PMBMaster {
	uint32 ctrl;
#define PMC_PMBM_START		(1 << 31)
#define PMC_PMBM_TIMEOUT	(1 << 30)
#define PMC_PMBM_SLAVE_ERR	(1 << 29)
#define PMC_PMBM_BUSY		(1 << 28)
#define PMC_PMBM_Read		(0 << 20)
#define PMC_PMBM_Write		(1 << 20)
	uint32 wr_data;
	uint32 timeout;
	uint32 rd_data;
	uint32 unused[4];
} PMBMaster;

typedef struct PMAPVTMONControl {
	uint32 control;
	uint32 reserved;
	uint32 cfg_lo;
	uint32 cfg_hi;
	uint32 data;
	uint32 vref_data;
	uint32 unused[2];
	uint32 ascan_cfg;
	uint32 warn_temp;
	uint32 reset_temp;
	uint32 temp_value;
	uint32 data1_value;
	uint32 data2_value;
	uint32 data3_value;
} PMAPVTMONControl;

typedef struct PMUBUSCfg {
	uint32 window[8];
	uint32 control;
} PMUBUSCfg;

typedef struct ProcessMonitorRegs {
	uint32 MonitorCtrl;		/* 0x00 */
	uint32 unused0[7];
	PMRingOscillatorControl ROSC;	/* 0x20 */
	uint32 unused1;
	PMMiscControl Misc;		/* 0x40 */
	uint32 unused2[6];
	PMSSBMasterControl SSBMaster;	/* 0x60 */
	uint32 unused3[5];
	PMEctrControl Ectr;		/* 0x80 */
	uint32 unused4[3];
	PMRefclkGen RefclkGen;		/* 0xa0 */
	PMBMaster PMBM[2];		/* 0xc0 */
	PMAPVTMONControl APvtmonCtrl;	/* 0x100 */
} ProcessMonitorRegs;

#define PROCMON ((volatile ProcessMonitorRegs * const) PROC_MON_BASE)

/******************** PROCMON block end *************/

/***********************************************************/
/*                    USB block definition                 */
/***********************************************************/
typedef struct USB_CAPSControl {
    uint32 BrtEControl;
    uint32 BrtOControl;
    uint32 BrtXControl;
    uint32 BrtControl;
    uint32 BrtEControl1;
    uint32 BrtOControl1;
    uint32 BrtXControl1;
    uint32 BrtControl1;
    uint32 BrtEControl2;
    uint32 BrtOControl2;
    uint32 BrtXControl2;
    uint32 BrtControl2;
} USB_CAPSControl;

typedef struct USBControl {
	uint32 Setup;
#define USB_IOC                	(1<<4)
#define USB_IPP                	(1<<5)
#define USB_SOFT_RESET         	(1<<6)
#define USB_OC_DIS			   	0xF0000000
    uint32 PllControl1;
#define PLL_NDIV_MASK     		0x000003FF
#define PLL_NDIV_SHIFT    		0
#define PLL_PDIV_MASK       	0x00007000
#define PLL_PDIV_SHIFT      	12
#define PLL_Kp_MASK        		0x000F0000
#define PLL_Kp_SHIFT       		16
#define PLL_Ki_MASK        		0x00700000
#define PLL_Ki_SHIFT       		20
#define PLL_Ka_MASK        		0x07000000
#define PLL_Ka_SHIFT       		24
#define PLL_SUSPEND_EN			(1 << 27)
#define PHYPLL_BYP				(1 << 39)
#define PLL_RESETB				(1 << 30)
#define PLL_IDDQ_PWRDN			(1 << 31)
    uint32 FrameAdjustValue;
    uint32 SwapControl;
#define USB_DEVICE_SEL          (1<<6)
#define EHCI_ENDIAN_SWAP        (1<<4)
#define EHCI_DATA_SWAP          (1<<3)
#define OHCI_ENDIAN_SWAP        (1<<1)
#define OHCI_DATA_SWAP          (1<<0)

    uint32 PowerManagement;
    uint32 MDIO;
    uint32 MDIO32;
	uint32 TestPortControl;
    uint32 USBSimControl;
	uint32 TestCtl;
	uint32 TestMon;
	uint32 UTMIcontrol;
#define UTMI_SOFT_RESET   		(1<<1)
#define USB_PHY_MODE_MASK		0x0000000C
#define USB_PHY_MODE_SHIFT		2
#define UTMI_SOFT_RESET_1   	(1<<17)
#define USB_PHY_MODE_MASK_1		0x000C0000
#define USB_PHY_MODE_SHIFT_1	18
#define USB_PHY_BC10			3
#define USB_PHY_OTG				2
#define USB_PHY_DEVICE			1
#define USB_PHY_HOST			0

} USBControl;
typedef struct USB_EHCIControl {
	uint32 ehci_registers[42];
} USB_EHCIControl;

typedef struct USB_OHCIControl {
	uint32 ohci_registers[22];
} USB_OHCIControl;

#define USBH_CAPS_BASE ((volatile USB_CAPSControl * const) USBH_BASE)
#define USBH_CTRL_BASE ((volatile USBControl * const) (USBH_BASE + 0x200))
#define USBH_EHCI_BASE ((volatile USB_EHCIControl * const) (USBH_BASE + 0x300))
#define USBH_OHCI_BASE ((volatile USB_OHCIControl * const) (USBH_BASE + 0x400))
/* for compaitiilty define EHCI & OHCI bases in 0x1XXX XXXX range and they will be remmaped
 * by usb driver to 0xbXXX XXXX addresses*/
#define USB_EHCI_BASE  (uint32)0x15400300  /* USB host registers */
#define USB_OHCI_BASE  (uint32)0x15400400 /* USB host registers */

#define USBH USBH_CTRL_BASE

#define USBD_CAPS_BASE ((volatile USB_CAPSControl * const) USBD_BASE)
#define USBD_CTRL_BASE ((volatile USBControl * const) (USBD_BASE + 0x200))
#define USBD_EHCI_BASE ((volatile USB_EHCIControl * const) (USBD_BASE + 0x300))
#define USBD_OHCI_BASE ((volatile USB_OHCIControl * const) (USBD_BASE + 0x400))
#define USBD USBD_CAPS_BASE
/********************** USB block end ***********************/

/***********************************************************/
/*                    UBUS Error block definition          */
/***********************************************************/
typedef struct UbusErrorPort {
	uint32 ErrorPortMduleId;  		//Module ID Register 
	uint32 ErrorPortIntPending;  	//Interrupt Pending Register 
	uint32 ErrorPortIntMask;      	//Interrupt Mask Register 
	uint32 ErrorPortIntState;  		//Interrupt State Register 
	uint32 ErrorPortStatus;  	    //Port Error Status Register 
	uint32 ErrorPortMask;  	    	//Port Error Mask Register 
	uint32 ErrorPortCount;  	    //Port Error Count Register 
} UbusErrorPort;

typedef struct UbusCaptureEngine {
	uint32 CaptureId;      		//Capture Engine ID Register 
 	uint32 CaptureInable;  		//Capture Engine Enable Register 
 	uint32 CaptureIontrol;  	//Capture Engine Enable Register 
 	uint32 CaptureItatus;  		//Capture Engine Status Register 
 	uint32 IntIending;     		//Capture Engine Interrupt Pending Register 
 	uint32 IntMask;  	    	//Capture Engine Interrupt Mask Register 
 	uint32 IntState;  	    	//Capture Engine Interrupt State Register 
 	uint32 AccumHI;  	    	//Capture Engine Accululator High Register 
 	uint32 AccumLO;  	    	//Capture Engine Accululator High Register 
 	uint32 AccumHIThresh;  		//Capture Engine Accululator High Threshold Register 
 	uint32 AccumLOThresh;  		//Capture Engine Accululator Low Threshold Register 
 	uint32 RateIntv;  	    	//Capture Engine Rate Counter Interval Register. 
 	uint32 RateCounter;    		//Capture Engine Rate Counter Value Register. 
 	uint32 RateCntrThresh;  	//Capture Engine Rate Counter Threshold Register. 
 	uint32 F0Control;      		//Capture Engine Filter 0 Control Register. 
 	uint32 F0AddrMin;     		//Capture Engine Filter 0 Address Min Register. 
 	uint32 F0AddrMax;     		//Capture Engine Filter 0 Address Max Register. 
 	uint32 F0Oid;  	    		//Capture Engine Filter 0 PID Register. 
 	uint32 F0HdrFilter;  		//Capture Engine Filter 0 Header Filter Register. 
 	uint32 F0HdrMask;     		//Capture Engine Filter 0 Header Filter Mask Register. 
 	uint32 F1Control;      		//Capture Engine Filter 1 Control Register. 
 	uint32 F1AddrMin;     		//Capture Engine Filter 1 Address Min Register. 
 	uint32 F1AddrMax;     		//Capture Engine Filter 1 Address Max Register. 
 	uint32 F1Pid;		    	//Capture Engine Filter 1 PID Register. 
 	uint32 F1HdrFilter;  		//Capture Engine Filter 1 Header Filter Register. 
 	uint32 F1HdrMask;     		//Capture Engine Filter 1 Header Filter Mask Register. 
 	uint32 CaptBufWord3;  		//Capture Engine Capture Buffer Word 3 Register. 
 	uint32 CaptBufWord2;  		//Capture Engine Capture Buffer Word 2 Register. 
 	uint32 CaptBufWord1;  		//Capture Engine Capture Buffer Word 1 Register. 
 	uint32 CaptBufWord0;  		//Capture Engine Capture Buffer Word 0 Register. 
} UbusCaptureEngine;

#define UBUSERROR ((volatile UbusErrorPort * const) (UBUS2_ERROR))
#define UBUSREQ0 ((volatile UbusCaptureEngine * const) (UBUS2_ERROR + 0x4000))
#define UBUSREP0 ((volatile UbusCaptureEngine * const) (UBUS2_ERROR + 0x4100))
#define UBUSREQ1 ((volatile UbusCaptureEngine * const) (UBUS2_ERROR + 0x5000))
#define UBUSREP1 ((volatile UbusCaptureEngine * const) (UBUS2_ERROR + 0x5100))
#define UBUSREQ2 ((volatile UbusCaptureEngine * const) (UBUS2_ERROR + 0x6000))
#define UBUSREP2 ((volatile UbusCaptureEngine * const) (UBUS2_ERROR + 0x6100))
/********************** UBUS Error block end **************/

#define UNIMAC_BASE         0x130d4000
#define UNIMAC_CFG_BASE     UNIMAC_BASE + 0x00000000
#define UNIMAC_MIB_BASE     UNIMAC_BASE + 0x00006000
#define UNIMAC_TOP_BASE     UNIMAC_BASE + 0x00007800

/* Bootrom specifics */
#define OTP_SHADOW_ADDR_BTRM_ENABLE_BTRM_ROW    0x4c
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         28
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (0x1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)
#define OTP_SHADOW_ADDR_BTRM_ENABLE_CUST_ROW    0x60
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         4
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (0x1 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)
#define OTP_SHADOW_ADDR_MARKET_ID_CUST_ROW      0x6c
#define OTP_MFG_MRKTID_OTP_BITS_SHIFT           0
#define OTP_MFG_MRKTID_OTP_BITS_MASK            (0xffff << OTP_MFG_MRKTID_OTP_BITS_SHIFT)

/* WAN Block */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG    0xb30f8000 /* WAN_CFG Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG   0xb30f8004 /* SATA_CFG Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT  0xb30f8008 /* SATA_STAT Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_PCS_CFG    0xb30f800c /* PCS_CFG Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_PCS_STAT   0xb30f8010 /* PCS_STAT Register */
#define ESERDES_STAT_WAN_GPIO_PER_REG            0xb4e001e0 /* ESERDES_STAT_WAN_GPIO_PER_REG */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET 0xb30f8014 /* GPON_GEARBOX_SW_RESET Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0 0xb30f8018 /* GPON_GEARBOX_FIFO_CFG_0 Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1 0xb30f801c /* GPON_GEARBOX_FIFO_CFG_1 Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2 0xb30f8020 /* GPON_GEARBOX_FIFO_CFG_2 Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_STATUS 0xb30f8024 /* GPON_GEARBOX_FIFO_STATUS Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1 0xb30f8028 /* GPON_GEARBOX_PATTERN_CFG1 Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2 0xb30f802c /* GPON_GEARBOX_PATTERN_CFG2 Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_CFG 0xb30f8030 /* GPON_GEARBOX_BURST_CFG Register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_STATUS 0xb30f8034 /* GPON_GEARBOX_BURST_STATUS Register */
#define PERIPH_BLOCK_PLLCNTRL_PER_OSC_CTRL 0xB4E002C4 /*PERIPH_BLOCK.PLLNTRL_PER_OSC_CTRL register */
#define WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG 0xb30f8038 /* EARLY_TXEN_CFG Register */

#endif

#ifdef __cplusplus
}
#endif

#endif

