/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 *
 * based on AT91RM9200 datasheet revision I (36. Ethernet MAC (EMAC))
 */

#ifndef AT91_H
#define AT91_H

typedef struct at91_emac {
	u32	 ctl;
	u32	 cfg;
	u32	 sr;
	u32	 tar;
	u32	 tcr;
	u32	 tsr;
	u32	 rbqp;
	u32	 reserved0;
	u32	 rsr;
	u32	 isr;
	u32	 ier;
	u32	 idr;
	u32	 imr;
	u32	 man;
	u32	 reserved1[2];
	u32	 fra;
	u32	 scol;
	u32	 mocl;
	u32	 ok;
	u32	 seqe;
	u32	 ale;
	u32	 dte;
	u32	 lcol;
	u32	 ecol;
	u32	 cse;
	u32	 tue;
	u32	 cde;
	u32	 elr;
	u32	 rjb;
	u32	 usf;
	u32	 sqee;
	u32	 drfc;
	u32	 reserved2[3];
	u32	 hsh;
	u32	 hsl;
	u32	 sa1l;
	u32	 sa1h;
	u32	 sa2l;
	u32	 sa2h;
	u32	 sa3l;
	u32	 sa3h;
	u32	 sa4l;
	u32	 sa4h;
} at91_emac_t;

#define AT91_EMAC_CTL_LB	0x0001
#define AT91_EMAC_CTL_LBL	0x0002
#define AT91_EMAC_CTL_RE	0x0004
#define AT91_EMAC_CTL_TE	0x0008
#define AT91_EMAC_CTL_MPE	0x0010
#define AT91_EMAC_CTL_CSR	0x0020
#define AT91_EMAC_CTL_ISR	0x0040
#define AT91_EMAC_CTL_WES	0x0080
#define AT91_EMAC_CTL_BP	0x1000

#define AT91_EMAC_CFG_SPD	0x0001
#define AT91_EMAC_CFG_FD	0x0002
#define AT91_EMAC_CFG_BR	0x0004
#define AT91_EMAC_CFG_CAF	0x0010
#define AT91_EMAC_CFG_NBC	0x0020
#define AT91_EMAC_CFG_MTI	0x0040
#define AT91_EMAC_CFG_UNI	0x0080
#define AT91_EMAC_CFG_BIG	0x0100
#define AT91_EMAC_CFG_EAE	0x0200
#define AT91_EMAC_CFG_CLK_MASK	0xFFFFF3FF
#define AT91_EMAC_CFG_MCLK_8	0x0000
#define AT91_EMAC_CFG_MCLK_16	0x0400
#define AT91_EMAC_CFG_MCLK_32	0x0800
#define AT91_EMAC_CFG_MCLK_64	0x0C00
#define AT91_EMAC_CFG_RTY	0x1000
#define AT91_EMAC_CFG_RMII	0x2000

#define AT91_EMAC_SR_LINK	0x0001
#define AT91_EMAC_SR_MDIO	0x0002
#define AT91_EMAC_SR_IDLE	0x0004

#define AT91_EMAC_TCR_LEN(x)	(x & 0x7FF)
#define AT91_EMAC_TCR_NCRC	0x8000

#define AT91_EMAC_TSR_OVR	0x0001
#define AT91_EMAC_TSR_COL	0x0002
#define AT91_EMAC_TSR_RLE	0x0004
#define AT91_EMAC_TSR_TXIDLE	0x0008
#define AT91_EMAC_TSR_BNQ	0x0010
#define AT91_EMAC_TSR_COMP	0x0020
#define AT91_EMAC_TSR_UND	0x0040

#define AT91_EMAC_RSR_BNA	0x0001
#define AT91_EMAC_RSR_REC	0x0002
#define AT91_EMAC_RSR_OVR	0x0004

/*  ISR, IER, IDR, IMR use the same bits */
#define AT91_EMAC_IxR_DONE	0x0001
#define AT91_EMAC_IxR_RCOM	0x0002
#define AT91_EMAC_IxR_RBNA	0x0004
#define AT91_EMAC_IxR_TOVR	0x0008
#define AT91_EMAC_IxR_TUND	0x0010
#define AT91_EMAC_IxR_RTRY	0x0020
#define AT91_EMAC_IxR_TBRE	0x0040
#define AT91_EMAC_IxR_TCOM	0x0080
#define AT91_EMAC_IxR_TIDLE	0x0100
#define AT91_EMAC_IxR_LINK	0x0200
#define AT91_EMAC_IxR_ROVR	0x0400
#define AT91_EMAC_IxR_HRESP	0x0800

#define AT91_EMAC_MAN_DATA_MASK		0xFFFF
#define AT91_EMAC_MAN_CODE_802_3	0x00020000
#define AT91_EMAC_MAN_REGA(reg)		((reg & 0x1F) << 18)
#define AT91_EMAC_MAN_PHYA(phy)		((phy & 0x1F) << 23)
#define AT91_EMAC_MAN_RW_R		0x20000000
#define AT91_EMAC_MAN_RW_W		0x10000000
#define AT91_EMAC_MAN_HIGH		0x40000000
#define AT91_EMAC_MAN_LOW		0x80000000

#endif
