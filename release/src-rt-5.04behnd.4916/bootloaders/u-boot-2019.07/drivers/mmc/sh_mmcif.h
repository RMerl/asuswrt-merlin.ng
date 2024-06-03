/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MMCIF driver.
 *
 * Copyright (C)  2011 Renesas Solutions Corp.
 */

#ifndef _SH_MMCIF_H_
#define _SH_MMCIF_H_

struct sh_mmcif_regs {
	unsigned long ce_cmd_set;
	unsigned long reserved;
	unsigned long ce_arg;
	unsigned long ce_arg_cmd12;
	unsigned long ce_cmd_ctrl;
	unsigned long ce_block_set;
	unsigned long ce_clk_ctrl;
	unsigned long ce_buf_acc;
	unsigned long ce_resp3;
	unsigned long ce_resp2;
	unsigned long ce_resp1;
	unsigned long ce_resp0;
	unsigned long ce_resp_cmd12;
	unsigned long ce_data;
	unsigned long reserved2[2];
	unsigned long ce_int;
	unsigned long ce_int_mask;
	unsigned long ce_host_sts1;
	unsigned long ce_host_sts2;
	unsigned long reserved3[11];
	unsigned long ce_version;
};

/* CE_CMD_SET */
#define CMD_MASK		0x3f000000
#define CMD_SET_RTYP_NO		((0 << 23) | (0 << 22))
/* R1/R1b/R3/R4/R5 */
#define CMD_SET_RTYP_6B		((0 << 23) | (1 << 22))
/* R2 */
#define CMD_SET_RTYP_17B	((1 << 23) | (0 << 22))
/* R1b */
#define CMD_SET_RBSY		(1 << 21)
#define CMD_SET_CCSEN		(1 << 20)
/* 1: on data, 0: no data */
#define CMD_SET_WDAT		(1 << 19)
/* 1: write to card, 0: read from card */
#define CMD_SET_DWEN		(1 << 18)
/* 1: multi block trans, 0: single */
#define CMD_SET_CMLTE		(1 << 17)
/* 1: CMD12 auto issue */
#define CMD_SET_CMD12EN		(1 << 16)
/* index check */
#define CMD_SET_RIDXC_INDEX	((0 << 15) | (0 << 14))
/* check bits check */
#define CMD_SET_RIDXC_BITS	((0 << 15) | (1 << 14))
/* no check */
#define CMD_SET_RIDXC_NO	((1 << 15) | (0 << 14))
/* 1: CRC7 check*/
#define CMD_SET_CRC7C		((0 << 13) | (0 << 12))
/* 1: check bits check*/
#define CMD_SET_CRC7C_BITS	((0 << 13) | (1 << 12))
/* 1: internal CRC7 check*/
#define CMD_SET_CRC7C_INTERNAL	((1 << 13) | (0 << 12))
/* 1: CRC16 check*/
#define CMD_SET_CRC16C		(1 << 10)
/* 1: not receive CRC status */
#define CMD_SET_CRCSTE		(1 << 8)
/* 1: tran mission bit "Low" */
#define CMD_SET_TBIT		(1 << 7)
/* 1: open/drain */
#define CMD_SET_OPDM		(1 << 6)
#define CMD_SET_CCSH		(1 << 5)
/* 1bit */
#define CMD_SET_DATW_1		((0 << 1) | (0 << 0))
/* 4bit */
#define CMD_SET_DATW_4		((0 << 1) | (1 << 0))
/* 8bit */
#define CMD_SET_DATW_8		((1 << 1) | (0 << 0))

/* CE_CMD_CTRL */
#define CMD_CTRL_BREAK		(1 << 0)

/* CE_BLOCK_SET */
#define BLOCK_SIZE_MASK		0x0000ffff

/* CE_CLK_CTRL */
#define CLK_ENABLE		(1 << 24)
#define CLK_CLEAR		((1 << 19) | (1 << 18) | (1 << 17) | (1 << 16))
#define CLK_PCLK		((1 << 19) | (1 << 18) | (1 << 17) | (1 << 16))
/* respons timeout */
#define SRSPTO_256		((1 << 13) | (0 << 12))
/* respons busy timeout */
#define SRBSYTO_29		((1 << 11) | (1 << 10) | (1 << 9) | (1 << 8))
/* read/write timeout */
#define SRWDTO_29		((1 << 7) | (1 << 6) | (1 << 5) | (1 << 4))
/* ccs timeout */
#define SCCSTO_29		((1 << 3) | (1 << 2) | (1 << 1) | (1 << 0))

/* CE_BUF_ACC */
#define BUF_ACC_DMAWEN		(1 << 25)
#define BUF_ACC_DMAREN		(1 << 24)
#define BUF_ACC_BUSW_32		(0 << 17)
#define BUF_ACC_BUSW_16		(1 << 17)
#define BUF_ACC_ATYP		(1 << 16)

/* CE_INT */
#define INT_CCSDE		(1 << 29)
#define INT_CMD12DRE		(1 << 26)
#define INT_CMD12RBE		(1 << 25)
#define INT_CMD12CRE		(1 << 24)
#define INT_DTRANE		(1 << 23)
#define INT_BUFRE		(1 << 22)
#define INT_BUFWEN		(1 << 21)
#define INT_BUFREN		(1 << 20)
#define INT_CCSRCV		(1 << 19)
#define INT_RBSYE		(1 << 17)
#define INT_CRSPE		(1 << 16)
#define INT_CMDVIO		(1 << 15)
#define INT_BUFVIO		(1 << 14)
#define INT_WDATERR		(1 << 11)
#define INT_RDATERR		(1 << 10)
#define INT_RIDXERR		(1 << 9)
#define INT_RSPERR		(1 << 8)
#define INT_CCSTO		(1 << 5)
#define INT_CRCSTO		(1 << 4)
#define INT_WDATTO		(1 << 3)
#define INT_RDATTO		(1 << 2)
#define INT_RBSYTO		(1 << 1)
#define INT_RSPTO		(1 << 0)
#define INT_ERR_STS		(INT_CMDVIO | INT_BUFVIO | INT_WDATERR |  \
				 INT_RDATERR | INT_RIDXERR | INT_RSPERR | \
				 INT_CCSTO | INT_CRCSTO | INT_WDATTO |	  \
				 INT_RDATTO | INT_RBSYTO | INT_RSPTO)
#define INT_START_MAGIC		0xD80430C0

/* CE_INT_MASK */
#define MASK_ALL		0x00000000
#define MASK_MCCSDE		(1 << 29)
#define MASK_MCMD12DRE		(1 << 26)
#define MASK_MCMD12RBE		(1 << 25)
#define MASK_MCMD12CRE		(1 << 24)
#define MASK_MDTRANE		(1 << 23)
#define MASK_MBUFRE		(1 << 22)
#define MASK_MBUFWEN		(1 << 21)
#define MASK_MBUFREN		(1 << 20)
#define MASK_MCCSRCV		(1 << 19)
#define MASK_MRBSYE		(1 << 17)
#define MASK_MCRSPE		(1 << 16)
#define MASK_MCMDVIO		(1 << 15)
#define MASK_MBUFVIO		(1 << 14)
#define MASK_MWDATERR		(1 << 11)
#define MASK_MRDATERR		(1 << 10)
#define MASK_MRIDXERR		(1 << 9)
#define MASK_MRSPERR		(1 << 8)
#define MASK_MCCSTO		(1 << 5)
#define MASK_MCRCSTO		(1 << 4)
#define MASK_MWDATTO		(1 << 3)
#define MASK_MRDATTO		(1 << 2)
#define MASK_MRBSYTO		(1 << 1)
#define MASK_MRSPTO		(1 << 0)

/* CE_HOST_STS1 */
#define STS1_CMDSEQ		(1 << 31)

/* CE_HOST_STS2 */
#define STS2_CRCSTE		(1 << 31)
#define STS2_CRC16E		(1 << 30)
#define STS2_AC12CRCE		(1 << 29)
#define STS2_RSPCRC7E		(1 << 28)
#define STS2_CRCSTEBE		(1 << 27)
#define STS2_RDATEBE		(1 << 26)
#define STS2_AC12REBE		(1 << 25)
#define STS2_RSPEBE		(1 << 24)
#define STS2_AC12IDXE		(1 << 23)
#define STS2_RSPIDXE		(1 << 22)
#define STS2_CCSTO		(1 << 15)
#define STS2_RDATTO		(1 << 14)
#define STS2_DATBSYTO		(1 << 13)
#define STS2_CRCSTTO		(1 << 12)
#define STS2_AC12BSYTO		(1 << 11)
#define STS2_RSPBSYTO		(1 << 10)
#define STS2_AC12RSPTO		(1 << 9)
#define STS2_RSPTO		(1 << 8)

#define STS2_CRC_ERR		(STS2_CRCSTE | STS2_CRC16E |		\
				 STS2_AC12CRCE | STS2_RSPCRC7E | STS2_CRCSTEBE)
#define STS2_TIMEOUT_ERR	(STS2_CCSTO | STS2_RDATTO |		\
				 STS2_DATBSYTO | STS2_CRCSTTO |		\
				 STS2_AC12BSYTO | STS2_RSPBSYTO |	\
				 STS2_AC12RSPTO | STS2_RSPTO)

/* CE_VERSION */
#define SOFT_RST_ON		(1 << 31)
#define SOFT_RST_OFF		(0 << 31)

#define CLKDEV_EMMC_DATA	52000000	/* 52MHz */
#ifdef CONFIG_ARCH_RMOBILE
#define MMC_CLK_DIV_MIN(clk)	(clk / (1 << 9))
#define MMC_CLK_DIV_MAX(clk)	(clk / (1 << 1))
#else
#define MMC_CLK_DIV_MIN(clk)	(clk / (1 << 8))
#define MMC_CLK_DIV_MAX(clk)	CLKDEV_EMMC_DATA
#endif

#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3

struct sh_mmcif_host {
	struct mmc_data		*data;
	struct sh_mmcif_regs	*regs;
	unsigned int		clk;
	int			bus_width;
	u16			wait_int;
	u16			sd_error;
	u8			last_cmd;
};

static inline u32 sh_mmcif_read(unsigned long *reg)
{
	return readl(reg);
}

static inline void sh_mmcif_write(u32 val, unsigned long *reg)
{
	writel(val, reg);
}

static inline void sh_mmcif_bitset(u32 val, unsigned long *reg)
{
	sh_mmcif_write(val | sh_mmcif_read(reg), reg);
}

static inline void sh_mmcif_bitclr(u32 val, unsigned long *reg)
{
	sh_mmcif_write(~val & sh_mmcif_read(reg), reg);
}

#endif /* _SH_MMCIF_H_ */
