/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _63138_OTP_H
#define _63138_OTP_H


#define JTAG_OTP_BASE    0xfffebb00

struct Jtag_Otp {
    uint32_t ctrl0;       /* 0x00 */
#define JTAG_OTP_CTRL_BYPASS_OTP_CLK        (1 << 31)
#define JTAG_OTP_CTRL_READ_FOUT         (1 << 30)
#define JTAG_OTP_CTRL_TESTCOL           (1 << 29)
#define JTAG_OTP_CTRL_CPU_DEBUG_SEL     (0xf << 25)
#define JTAG_OTP_CTRL_BURST_UART_SEL        (1 << 24)
#define JTAG_OTP_CTRL_ACCESS_MODE       (0x2 << 22)
#define JTAG_OTP_CTRL_PROG_EN           (1 << 21)
#define JTAG_OTP_CTRL_DEBUG_MDE         (1 << 20)
#define JTAG_OTP_CTRL_WRP_CONTINUE_ON_FAIL  (1 << 19)
#define JTAG_OTP_CTRL_WRP_PROGRAM_VERIFY_FLAG   (1 << 18)
#define JTAG_OTP_CTRL_WRP_DOUBLE_WORD       (1 << 17)
#define JTAG_OTP_CTRL_WRP_REGC_SEL      (0x7 << 14)
#define JTAG_OTP_CTRL_WRP_QUADFUSE      (1 << 13)
#define JTAG_OTP_CTRL_WRP_DOUBLEFUSE        (1 << 12)
#define JTAG_OTP_CTRL_WRP_READ4X        (1 << 11)
#define JTAG_OTP_CTRL_WRP_READ2X        (1 << 10)
#define JTAG_OTP_CTRL_WRP_IN_DEBUG      (1 << 9)
#define JTAG_OTP_CTRL_COMMAND           (0x1f << 1)
#define JTAG_OTP_CTRL_CMD_READ      (0x00 << 1)
#define JTAG_OTP_CTRL_CMD_READBURST (0x01 << 1)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_EN   (0x02 << 1)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_DIS  (0x03 << 1)
#define JTAG_OTP_CTRL_CMD_PRESCREEN (0x04 << 1)
#define JTAG_OTP_CTRL_CMD_PRESCREEN_RP  (0x05 << 1)
#define JTAG_OTP_CTRL_CMD_FLUSH     (0x06 << 1)
#define JTAG_OTP_CTRL_CMD_NOP       (0x07 << 1)
#define JTAG_OTP_CTRL_CMD_PROG      (0x0a << 1)
#define JTAG_OTP_CTRL_CMD_PROG_RP   (0x0b << 1)
#define JTAG_OTP_CTRL_CMD_PROG_OVST (0x0c << 1)
#define JTAG_OTP_CTRL_CMD_RELOAD    (0x0d << 1)
#define JTAG_OTP_CTRL_CMD_ERASE     (0x0e << 1)
#define JTAG_OTP_CTRL_CMD_LOAD_RF   (0x0f << 1)
#define JTAG_OTP_CTRL_CMD_CTRL_WR   (0x10 << 1)
#define JTAG_OTP_CTRL_CMD_CTRL_RD   (0x11 << 1)
#define JTAG_OTP_CTRL_CMD_READ_HP   (0x12 << 1)
#define JTAG_OTP_CTRL_CMD_READ_OVST (0x13 << 1)
#define JTAG_OTP_CTRL_CMD_READ_VERIFY0  (0x14 << 1)
#define JTAG_OTP_CTRL_CMD_READ_VERIFY1  (0x15 << 1)
#define JTAG_OTP_CTRL_CMD_READ_FORCE0   (0x16 << 1)
#define JTAG_OTP_CTRL_CMD_READ_FORCE1   (0x17 << 1)
#define JTAG_OTP_CTRL_CMD_BURNING   (0x18 << 1)
#define JTAG_OTP_CTRL_CMD_PROG_LOCK (0x19 << 1)
#define JTAG_OTP_CTRL_CMD_PROG_TESTCOL  (0x1a << 1)
#define JTAG_OTP_CTRL_CMD_READ_TESTCOL  (0x1b << 1)
#define JTAG_OTP_CTRL_CMD_READ_FOUT (0x1e << 1)
#define JTAG_OTP_CTRL_CMD_SFT_RESET (0x1f << 1)

#define JTAG_OTP_CTRL_START         (1 << 0)
    uint32_t ctrl1;       /* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE          (1 << 0)
    uint32_t ctrl2;       /* 0x08 */
    uint32_t ctrl3;       /* 0x0c */
    uint32_t ctrl4;       /* 0x10 */
    uint32_t status0;     /* 0x14 */
    uint32_t status1;     /* 0x18 */
#define JTAG_OTP_STATUS_1_PROG_OK       (1 << 2)
#define JTAG_OTP_STATUS_1_CMD_DONE      (1 << 1)
    uint32_t status2;     /* 0x1c */
    uint32_t status3;     /* 0x20 */
#define OTP_SAR_DISABLE       0
#define OTP_VDSL_DSLDISABLE   1
#define OTP_DIS2LINE          2
#define OTP_VDSL_DISVDSL      3
#define OTP_VDSL_DIS6BND      4
#define OTP_VDSL_DISRNC       5
    uint32_t status4;     /* 0x24 */
#define OTP_SF2_DISABLE       32
#define OTP_PCIE_RCAL_VALID 33
    uint32_t status5;     /* 0x28 */
#define OTP_SPHY_DISABLE      64
#define OTP_RNR_DISIPSEC      75
#define OTP_RNR_DISSHA1       76
#define OTP_RNR_DISSHA2       77
#define OTP_PRNR_DISAES256    78
#define OTP_PRNR_DISAES192    79
#define OTP_PRNR_DISAES128    80
#define OTP_USBH_DISEHCI      81
#define OTP_USBH_XHCIDIS      82
#define OTP_USBD_DISABLE      83
#define OTP_PCIE_DISABLE0     84
#define OTP_PCIE_DISABLE1     85
#define OTP_A9_DISC1          86
#define OTP_A9_DISNEON        87
#define OTP_SATA_DISABLE      88
#define OTP_DECT_DISABLE      89
    uint32_t status6;     /* 0x2c */
#define OTP_TBUS_DISABLE      120
#define OTP_PMC_BOOT          121
    uint32_t status7;     /* 0x30 */
#define OTP_PMC_BOOTROM       132
#define OTP_PMC_SECUREREG     133
#define OTP_PBMU_DISABLE      134
    uint32_t status8;     /* 0x34 */
    uint32_t status9;     /* 0x38 */
#define OTP_DDR_SECUREACC     211
#define OTP_DDR_SECIRELCK     212
#define OTP_ECC_MREPAIR_EN    217
};

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW       17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          1 

#define OTP_BRCM_MEK_MIV_ROW                17
#define OTP_BRCM_MEK_MIV_SHIFT              7
#define OTP_BRCM_MEK_MIV_MASK               7

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW       18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          7

/* row 24 */
#define OTP_CUST_MFG_MRKTID_ROW         24
#define OTP_CUST_MFG_MRKTID_SHIFT       0
#define OTP_CUST_MFG_MRKTID_MASK        0xffff

#define OTP_CUST_OP_INUSE_ROW               24
#define OTP_CUST_OP_INUSE_SHIFT             16
#define OTP_CUST_OP_INUSE_MASK              1

/* row 25 */
#define OTP_CUST_OP_MRKTID_ROW          25
#define OTP_CUST_OP_MRKTID_SHIFT        0
#define OTP_CUST_OP_MRKTID_MASK         0xffff

/* A row initializer that maps actual row number with mask and shift to a feature name;
 * this allows to use features vs. rows for common functionality, 
 * such as secure boot handling frequency, chipid and so on 
 * prevent ifdef dependencies when used outside of arch directories for common among SoCs logic
 * */
#define	DEFINE_OTP_MAP_ROW_INITLR(__VV__)									\
	static otp_hw_cmn_row_t __VV__[ ] = {									\
	{OTP_MAP_BRCM_BTRM_BOOT_ENABLE, OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_MASK, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT, 1},\
	{OTP_MAP_CUST_BTRM_BOOT_ENABLE, OTP_CUST_BTRM_BOOT_ENABLE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_MASK, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT, 1},\
	{OTP_MAP_CUST_MFG_MRKTID, OTP_CUST_MFG_MRKTID_ROW, OTP_CUST_MFG_MRKTID_MASK, OTP_CUST_MFG_MRKTID_SHIFT, 1},				\
	{OTP_MAP_CUST_OP_INUSE, OTP_CUST_OP_INUSE_ROW, OTP_CUST_OP_INUSE_MASK, OTP_CUST_OP_INUSE_SHIFT, 1},				\
}

#endif
