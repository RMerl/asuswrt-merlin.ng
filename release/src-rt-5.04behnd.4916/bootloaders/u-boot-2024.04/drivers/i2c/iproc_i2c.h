/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Broadcom
 *
 */

#ifndef __IPROC_I2C_H__
#define __IPROC_I2C_H__

/* Registers */
#define CCB_SMB_CFG_REG 0x0

#define CCB_SMB_CFG_RST_MASK                         0x80000000
#define CCB_SMB_CFG_RST_SHIFT                        31

#define CCB_SMB_CFG_SMBEN_MASK                       0x40000000
#define CCB_SMB_CFG_SMBEN_SHIFT                      30

#define CCB_SMB_CFG_BITBANGEN_MASK                   0x20000000
#define CCB_SMB_CFG_BITBANGEN_SHIFT                  29

#define CCB_SMB_CFG_EN_NIC_SMBADDR0_MASK             0x10000000
#define CCB_SMB_CFG_EN_NIC_SMBADDR0_SHIFT            28

#define CCB_SMB_CFG_PROMISCMODE_MASK                 0x08000000
#define CCB_SMB_CFG_PROMISCMODE_SHIFT                27

#define CCB_SMB_CFG_TSTMPCNTEN_MASK                  0x04000000
#define CCB_SMB_CFG_TSTMPCNTEN_SHIFT                 26

#define CCB_SMB_CFG_MSTRRTRYCNT_MASK                 0x000F0000
#define CCB_SMB_CFG_MSTRRTRYCNT_SHIFT                16

#define CCB_SMB_TIMGCFG_REG 0x4

#define CCB_SMB_TIMGCFG_MODE400_MASK                 0x80000000
#define CCB_SMB_TIMGCFG_MODE400_SHIFT                31

#define CCB_SMB_TIMGCFG_RNDSLVSTR_MASK               0x7F000000
#define CCB_SMB_TIMGCFG_RNDSLVSTR_SHIFT              24

#define CCB_SMB_TIMGCFG_PERSLVSTR_MASK               0x00FF0000
#define CCB_SMB_TIMGCFG_PERSLVSTR_SHIFT              16

#define CCB_SMB_TIMGCFG_IDLTIME_MASK                 0x0000FF00
#define CCB_SMB_TIMGCFG_IDLTIME_SHIFT                8

#define CCB_SMB_ADDR_REG 0x8

#define CCB_SMB_EN_NIC_SMBADDR3_MASK                 0x80000000
#define CCB_SMB_EN_NIC_SMBADDR3_SHIFT                31

#define CCB_SMB_NIC_SMBADDR3_MASK                    0x7F000000
#define CCB_SMB_NIC_SMBADDR3_SHIFT                   24

#define CCB_SMB_EN_NIC_SMBADDR2_MASK                 0x00800000
#define CCB_SMB_EN_NIC_SMBADDR2_SHIFT                23

#define CCB_SMB_NIC_SMBADDR2_MASK                    0x007F0000
#define CCB_SMB_NIC_SMBADDR2_SHIFT                   16

#define CCB_SMB_EN_NIC_SMBADDR1_MASK                 0x00008000
#define CCB_SMB_EN_NIC_SMBADDR1_SHIFT                15

#define CCB_SMB_NIC_SMBADDR1_MASK                    0x00007F00
#define CCB_SMB_NIC_SMBADDR1_SHIFT                   8

#define CCB_SMB_EN_NIC_SMBADDR0_MASK                 0x00000080
#define CCB_SMB_EN_NIC_SMBADDR0_SHIFT                7

#define CCB_SMB_NIC_SMBADDR0_MASK                    0x0000007F
#define CCB_SMB_NIC_SMBADDR0_SHIFT                   0

#define CCB_SMB_MSTRFIFOCTL_REG 0xC

#define CCB_SMB_MSTRRXFIFOFLSH_MASK                  0x80000000
#define CCB_SMB_MSTRRXFIFOFLSH_SHIFT                 31

#define CCB_SMB_MSTRTXFIFOFLSH_MASK                  0x40000000
#define CCB_SMB_MSTRTXFIFOFLSH_SHIFT                 30

#define CCB_SMB_MSTRRXPKTCNT_MASK                    0x007F0000
#define CCB_SMB_MSTRRXPKTCNT_SHIFT                   16

#define CCB_SMB_MSTRRXFIFOTHR_MASK                   0x00003F00
#define CCB_SMB_MSTRRXFIFOTHR_SHIFT                  8

#define CCB_SMB_SLVFIFOCTL_REG 0x10

#define CCB_SMB_SLVRXFIFOFLSH_MASK                   0x80000000
#define CCB_SMB_SLVRXFIFOFLSH_SHIFT                  31

#define CCB_SMB_SLVTXFIFOFLSH_MASK                   0x40000000
#define CCB_SMB_SLVTXFIFOFLSH_SHIFT                  30

#define CCB_SMB_SLVRXPKTCNT_MASK                     0x007F0000
#define CCB_SMB_SLVRXPKTCNT_SHIFT                    16

#define CCB_SMB_SLVRXFIFOTHR_MASK                    0x00003F00
#define CCB_SMB_SLVRXFIFOTHR_SHIFT                   8

#define CCB_SMB_BITBANGCTL_REG 0x14

#define CCB_SMB_SMBCLKIN_MASK                        0x80000000
#define CCB_SMB_SMBCLKIN_SHIFT                       31

#define CCB_SMB_SMBCLKOUTEN_MASK                     0x40000000
#define CCB_SMB_SMBCLKOUTEN_SHIFT                    30

#define CCB_SMB_SMBDATAIN_MASK                       0x20000000
#define CCB_SMB_SMBDATAIN_SHIFT                      29

#define CCB_SMB_SMBDATAOUTEN_MASK                    0x10000000
#define CCB_SMB_SMBDATAOUTEN_SHIFT                   28

#define CCB_SMB_MSTRCMD_REG 0x30

#define CCB_SMB_MSTRSTARTBUSYCMD_MASK                0x80000000
#define CCB_SMB_MSTRSTARTBUSYCMD_SHIFT               31

#define CCB_SMB_MSTRABORT_MASK                       0x40000000
#define CCB_SMB_MSTRABORT_SHIFT                      30

#define CCB_SMB_MSTRSTS_MASK                         0x0E000000
#define CCB_SMB_MSTRSTS_SHIFT                        25

#define CCB_SMB_MSTRSMBUSPROTO_MASK                  0x00001E00
#define CCB_SMB_MSTRSMBUSPROTO_SHIFT                 9

#define CCB_SMB_MSTRPEC_MASK                         0x00000100
#define CCB_SMB_MSTRPEC_SHIFT                        8

#define CCB_SMB_MSTRRDBYTECNT_MASK                   0x000000FF
#define CCB_SMB_MSTRRDBYTECNT_SHIFT                  0

#define CCB_SMB_SLVCMD_REG 0x34

#define CCB_SMB_SLVSTARTBUSYCMD_MASK                 0x80000000
#define CCB_SMB_SLVSTARTBUSYCMD_SHIFT                31

#define CCB_SMB_SLVABORT_MASK                        0x40000000
#define CCB_SMB_SLVABORT_SHIFT                       30

#define CCB_SMB_SLVSTS_MASK                          0x03800000
#define CCB_SMB_SLVSTS_SHIFT                         23

#define CCB_SMB_SLVPEC_MASK                          0x00000100
#define CCB_SMB_SLVPEC_SHIFT                         8

#define CCB_SMB_EVTEN_REG 0x38

#define CCB_SMB_MSTRRXFIFOFULLEN_MASK                0x80000000
#define CCB_SMB_MSTRRXFIFOFULLEN_SHIFT               31

#define CCB_SMB_MSTRRXFIFOTHRHITEN_MASK              0x40000000
#define CCB_SMB_MSTRRXFIFOTHRHITEN_SHIFT             30

#define CCB_SMB_MSTRRXEVTEN_MASK                     0x20000000
#define CCB_SMB_MSTRRXEVTEN_SHIFT                    29

#define CCB_SMB_MSTRSTARTBUSYEN_MASK                 0x10000000
#define CCB_SMB_MSTRSTARTBUSYEN_SHIFT                28

#define CCB_SMB_MSTRTXUNDEN_MASK                     0x08000000
#define CCB_SMB_MSTRTXUNDEN_SHIFT                    27

#define CCB_SMB_SLVRXFIFOFULLEN_MASK                 0x04000000
#define CCB_SMB_SLVRXFIFOFULLEN_SHIFT                26

#define CCB_SMB_SLVRXFIFOTHRHITEN_MASK               0x02000000
#define CCB_SMB_SLVRXFIFOTHRHITEN_SHIFT              25

#define CCB_SMB_SLVRXEVTEN_MASK                      0x01000000
#define CCB_SMB_SLVRXEVTEN_SHIFT                     24

#define CCB_SMB_SLVSTARTBUSYEN_MASK                  0x00800000
#define CCB_SMB_SLVSTARTBUSYEN_SHIFT                 23

#define CCB_SMB_SLVTXUNDEN_MASK                      0x00400000
#define CCB_SMB_SLVTXUNDEN_SHIFT                     22

#define CCB_SMB_SLVRDEVTEN_MASK                      0x00200000
#define CCB_SMB_SLVRDEVTEN_SHIFT                     21

#define CCB_SMB_EVTSTS_REG 0x3C

#define CCB_SMB_MSTRRXFIFOFULLSTS_MASK               0x80000000
#define CCB_SMB_MSTRRXFIFOFULLSTS_SHIFT              31

#define CCB_SMB_MSTRRXFIFOTHRHITSTS_MASK             0x40000000
#define CCB_SMB_MSTRRXFIFOTHRHITSTS_SHIFT            30

#define CCB_SMB_MSTRRXEVTSTS_MASK                    0x20000000
#define CCB_SMB_MSTRRXEVTSTS_SHIFT                   29

#define CCB_SMB_MSTRSTARTBUSYSTS_MASK                0x10000000
#define CCB_SMB_MSTRSTARTBUSYSTS_SHIFT               28

#define CCB_SMB_MSTRTXUNDSTS_MASK                    0x08000000
#define CCB_SMB_MSTRTXUNDSTS_SHIFT                   27

#define CCB_SMB_SLVRXFIFOFULLSTS_MASK                0x04000000
#define CCB_SMB_SLVRXFIFOFULLSTS_SHIFT               26

#define CCB_SMB_SLVRXFIFOTHRHITSTS_MASK              0x02000000
#define CCB_SMB_SLVRXFIFOTHRHITSTS_SHIFT             25

#define CCB_SMB_SLVRXEVTSTS_MASK                     0x01000000
#define CCB_SMB_SLVRXEVTSTS_SHIFT                    24

#define CCB_SMB_SLVSTARTBUSYSTS_MASK                 0x00800000
#define CCB_SMB_SLVSTARTBUSYSTS_SHIFT                23

#define CCB_SMB_SLVTXUNDSTS_MASK                     0x00400000
#define CCB_SMB_SLVTXUNDSTS_SHIFT                    22

#define CCB_SMB_SLVRDEVTSTS_MASK                     0x00200000
#define CCB_SMB_SLVRDEVTSTS_SHIFT                    21

#define CCB_SMB_MSTRDATAWR_REG 0x40

#define CCB_SMB_MSTRWRSTS_MASK                       0x80000000
#define CCB_SMB_MSTRWRSTS_SHIFT                      31

#define CCB_SMB_MSTRWRDATA_MASK                      0x000000FF
#define CCB_SMB_MSTRWRDATA_SHIFT                     0

#define CCB_SMB_MSTRDATARD_REG 0x44

#define CCB_SMB_MSTRRDSTS_MASK                       0xC0000000
#define CCB_SMB_MSTRRDSTS_SHIFT                      30

#define CCB_SMB_MSTRRDPECERR_MASK                    0x20000000
#define CCB_SMB_MSTRRDPECERR_SHIFT                   29

#define CCB_SMB_MSTRRDDATA_MASK                      0x000000FF
#define CCB_SMB_MSTRRDDATA_SHIFT                     0

#define CCB_SMB_SLVDATAWR_REG 0x48

#define CCB_SMB_SLVWRSTS_MASK                        0x80000000
#define CCB_SMB_SLVWRSTS_SHIFT                       31

#define CCB_SMB_SLVWRDATA_MASK                       0x000000FF
#define CCB_SMB_SLVWRDATA_SHIFT                      0

#define CCB_SMB_SLVDATARD_REG 0x4C

#define CCB_SMB_SLVRDSTS_MASK                        0xC0000000
#define CCB_SMB_SLVRDSTS_SHIFT                       30

#define CCB_SMB_SLVRDERRSTS_MASK                     0x30000000
#define CCB_SMB_SLVRDERRSTS_SHIFT                    28

#define CCB_SMB_SLVRDDATA_MASK                       0x000000FF
#define CCB_SMB_SLVRDDATA_SHIFT                      0

/* --Registers-- */

/* Transaction error codes defined in Master command register (0x30) */
#define MSTR_STS_XACT_SUCCESS          0
#define MSTR_STS_LOST_ARB              1
#define MSTR_STS_NACK_FIRST_BYTE       2

/* NACK on a byte other than
 * the first byte
 */
#define MSTR_STS_NACK_NON_FIRST_BYTE   3

#define MSTR_STS_TTIMEOUT_EXCEEDED     4
#define MSTR_STS_TX_TLOW_MEXT_EXCEEDED 5
#define MSTR_STS_RX_TLOW_MEXT_EXCEEDED 6

/* SMBUS protocol values defined in register 0x30 */
#define SMBUS_PROT_QUICK_CMD               0
#define SMBUS_PROT_SEND_BYTE               1
#define SMBUS_PROT_RECV_BYTE               2
#define SMBUS_PROT_WR_BYTE                 3
#define SMBUS_PROT_RD_BYTE                 4
#define SMBUS_PROT_WR_WORD                 5
#define SMBUS_PROT_RD_WORD                 6
#define SMBUS_PROT_BLK_WR                  7
#define SMBUS_PROT_BLK_RD                  8
#define SMBUS_PROT_PROC_CALL               9
#define SMBUS_PROT_BLK_WR_BLK_RD_PROC_CALL 10

/* SMBUS Block speed mode */
#define SMBUS_BLOCK_MODE_100		0
#define SMBUS_BLOCK_MODE_400		1

#define BUS_BUSY_COUNT         100000 /* Number can be changed later */
#define IPROC_I2C_INVALID_ADDR 0xFF
#define IPROC_SMB_MAX_RETRIES   35
#define I2C_SMBUS_BLOCK_MAX     32
#define GETREGFLDVAL(regval, mask, startbit) \
		    (((regval) & (mask)) >> (startbit))

/* This enum will be used to notify the user of status of a data transfer
 * request
 */
enum iproc_smb_error_code {
	I2C_NO_ERR = 0,
	I2C_TIMEOUT_ERR = 1,

	/* Invalid parameter(s) passed to the driver */
	I2C_INVALID_PARAM_ERR = 2,

	/* The driver API was called before the present
	 * transfer was completed
	 */
	I2C_OPER_IN_PROGRESS = 3,

	/* Transfer aborted unexpectedly, for example a NACK
	 * received, before last byte was read/written
	 */
	I2C_OPER_ABORT_ERR = 4,

	/* Feature or function not supported
	 * (e.g., 10-bit addresses, or clock speeds
	 * other than 100KHz, 400KHz)
	 */
	I2C_FUNC_NOT_SUPPORTED = 5,
};

/* Structure used to pass information to read/write functions. */
struct iproc_xact_info {
	unsigned char command;
	unsigned char *data;
	unsigned int size;
	unsigned short flags; /* used for specifying PEC, 10-bit addresses */
	unsigned char smb_proto; /* SMBus protocol */
	unsigned int cmd_valid; /* true if command is valid else false */
};

#endif /* __IPROC_I2C_H__ */
