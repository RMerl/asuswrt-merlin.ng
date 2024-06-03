// SPDX-License-Identifier: GPL-2.0+
/*
 * MPC8560 FCC Fast Ethernet
 * Copyright (c) 2003 Motorola,Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * Copyright (c) 2000 MontaVista Software, Inc.   Dan Malek (dmalek@jlc.net)
 *
 * (C) Copyright 2000 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

/*
 * MPC8560 FCC Fast Ethernet
 * Basic ET HW initialization and packet RX/TX routines
 *
 * This code will not perform the IO port configuration. This should be
 * done in the iop_conf_t structure specific for the board.
 *
 * TODO:
 * add a PHY driver to do the negotiation
 * reflect negotiation results in FPSMR
 * look for ways to configure the board specific stuff elsewhere, eg.
 *    config_xxx.h or the board directory
 */

#include <common.h>
#include <malloc.h>
#include <asm/cpm_85xx.h>
#include <command.h>
#include <config.h>
#include <net.h>

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#include <miiphy.h>
#endif

#if defined(CONFIG_ETHER_ON_FCC) && defined(CONFIG_CMD_NET)

static struct ether_fcc_info_s
{
	int ether_index;
	int proff_enet;
	ulong cpm_cr_enet_sblock;
	ulong cpm_cr_enet_page;
	ulong cmxfcr_mask;
	ulong cmxfcr_value;
}
	ether_fcc_info[] =
{
#ifdef CONFIG_ETHER_ON_FCC1
{
	0,
	PROFF_FCC1,
	CPM_CR_FCC1_SBLOCK,
	CPM_CR_FCC1_PAGE,
	CONFIG_SYS_CMXFCR_MASK1,
	CONFIG_SYS_CMXFCR_VALUE1
},
#endif

#ifdef CONFIG_ETHER_ON_FCC2
{
	1,
	PROFF_FCC2,
	CPM_CR_FCC2_SBLOCK,
	CPM_CR_FCC2_PAGE,
	CONFIG_SYS_CMXFCR_MASK2,
	CONFIG_SYS_CMXFCR_VALUE2
},
#endif

#ifdef CONFIG_ETHER_ON_FCC3
{
	2,
	PROFF_FCC3,
	CPM_CR_FCC3_SBLOCK,
	CPM_CR_FCC3_PAGE,
	CONFIG_SYS_CMXFCR_MASK3,
	CONFIG_SYS_CMXFCR_VALUE3
},
#endif
};

/*---------------------------------------------------------------------*/

/* Maximum input DMA size.  Must be a should(?) be a multiple of 4. */
#define PKT_MAXDMA_SIZE         1520

/* The FCC stores dest/src/type, data, and checksum for receive packets. */
#define PKT_MAXBUF_SIZE         1518
#define PKT_MINBUF_SIZE         64

/* Maximum input buffer size.  Must be a multiple of 32. */
#define PKT_MAXBLR_SIZE         1536

#define TOUT_LOOP 1000000

#define TX_BUF_CNT 2

static uint rxIdx;	/* index of the current RX buffer */
static uint txIdx;	/* index of the current TX buffer */

/*
 * FCC Ethernet Tx and Rx buffer descriptors.
 * Provide for Double Buffering
 * Note: PKTBUFSRX is defined in net.h
 */

typedef volatile struct rtxbd {
    cbd_t rxbd[PKTBUFSRX];
    cbd_t txbd[TX_BUF_CNT];
} RTXBD;

/*  Good news: the FCC supports external BDs! */
#ifdef __GNUC__
static RTXBD rtx __attribute__ ((aligned(8)));
#else
#error "rtx must be 64-bit aligned"
#endif

#undef ET_DEBUG

static int fec_send(struct eth_device *dev, void *packet, int length)
{
    int i = 0;
    int result = 0;

    if (length <= 0) {
	printf("fec: bad packet size: %d\n", length);
	goto out;
    }

    for(i=0; rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= TOUT_LOOP) {
	    printf("fec: tx buffer not ready\n");
	    goto out;
	}
    }

    rtx.txbd[txIdx].cbd_bufaddr = (uint)packet;
    rtx.txbd[txIdx].cbd_datlen = length;
    rtx.txbd[txIdx].cbd_sc |= (BD_ENET_TX_READY | BD_ENET_TX_LAST | \
			       BD_ENET_TX_TC | BD_ENET_TX_PAD);

    for(i=0; rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= TOUT_LOOP) {
	    printf("fec: tx error\n");
	    goto out;
	}
    }

#ifdef ET_DEBUG
    printf("cycles: 0x%x txIdx=0x%04x status: 0x%04x\n", i, txIdx,rtx.txbd[txIdx].cbd_sc);
    printf("packets at 0x%08x, length_in_bytes=0x%x\n",(uint)packet,length);
    for(i=0;i<(length/16 + 1);i++) {
	 printf("%08x %08x %08x %08x\n",*((uint *)rtx.txbd[txIdx].cbd_bufaddr+i*4),\
    *((uint *)rtx.txbd[txIdx].cbd_bufaddr + i*4 + 1),*((uint *)rtx.txbd[txIdx].cbd_bufaddr + i*4 + 2), \
    *((uint *)rtx.txbd[txIdx].cbd_bufaddr + i*4 + 3));
    }
#endif

    /* return only status bits */
    result = rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_STATS;
    txIdx = (txIdx + 1) % TX_BUF_CNT;

out:
    return result;
}

static int fec_recv(struct eth_device* dev)
{
    int length;

    for (;;)
    {
	if (rtx.rxbd[rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
	    length = -1;
	    break;     /* nothing received - leave for() loop */
	}
	length = rtx.rxbd[rxIdx].cbd_datlen;

	if (rtx.rxbd[rxIdx].cbd_sc & 0x003f) {
	    printf("fec: rx error %04x\n", rtx.rxbd[rxIdx].cbd_sc);
	}
	else {
	    /* Pass the packet up to the protocol layers. */
	    net_process_received_packet(net_rx_packets[rxIdx], length - 4);
	}


	/* Give the buffer back to the FCC. */
	rtx.rxbd[rxIdx].cbd_datlen = 0;

	/* wrap around buffer index when necessary */
	if ((rxIdx + 1) >= PKTBUFSRX) {
	    rtx.rxbd[PKTBUFSRX - 1].cbd_sc = (BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY);
	    rxIdx = 0;
	}
	else {
	    rtx.rxbd[rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
	    rxIdx++;
	}
    }
    return length;
}


static int fec_init(struct eth_device* dev, bd_t *bis)
{
    struct ether_fcc_info_s * info = dev->priv;
    int i;
    volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
    volatile ccsr_cpm_cp_t *cp = &(cpm->im_cpm_cp);
    fcc_enet_t *pram_ptr;
    unsigned long mem_addr;

#if 0
    mii_discover_phy();
#endif

    /* 28.9 - (1-2): ioports have been set up already */

    /* 28.9 - (3): connect FCC's tx and rx clocks */
    cpm->im_cpm_mux.cmxuar = 0; /* ATM */
    cpm->im_cpm_mux.cmxfcr = (cpm->im_cpm_mux.cmxfcr & ~info->cmxfcr_mask) |
							info->cmxfcr_value;

    /* 28.9 - (4): GFMR: disable tx/rx, CCITT CRC, set Mode Ethernet */
    if(info->ether_index == 0) {
	cpm->im_cpm_fcc1.gfmr = FCC_GFMR_MODE_ENET | FCC_GFMR_TCRC_32;
    } else if (info->ether_index == 1) {
	cpm->im_cpm_fcc2.gfmr = FCC_GFMR_MODE_ENET | FCC_GFMR_TCRC_32;
    } else if (info->ether_index == 2) {
	cpm->im_cpm_fcc3.gfmr = FCC_GFMR_MODE_ENET | FCC_GFMR_TCRC_32;
    }

    /* 28.9 - (5): FPSMR: enable full duplex, select CCITT CRC for Ethernet,MII */
    if(info->ether_index == 0) {
	cpm->im_cpm_fcc1.fpsmr = CONFIG_SYS_FCC_PSMR | FCC_PSMR_ENCRC;
    } else if (info->ether_index == 1){
	cpm->im_cpm_fcc2.fpsmr = CONFIG_SYS_FCC_PSMR | FCC_PSMR_ENCRC;
    } else if (info->ether_index == 2){
	cpm->im_cpm_fcc3.fpsmr = CONFIG_SYS_FCC_PSMR | FCC_PSMR_ENCRC;
    }

    /* 28.9 - (6): FDSR: Ethernet Syn */
    if(info->ether_index == 0) {
	cpm->im_cpm_fcc1.fdsr = 0xD555;
    } else if (info->ether_index == 1) {
	cpm->im_cpm_fcc2.fdsr = 0xD555;
    } else if (info->ether_index == 2) {
	cpm->im_cpm_fcc3.fdsr = 0xD555;
    }

    /* reset indeces to current rx/tx bd (see eth_send()/eth_rx()) */
    rxIdx = 0;
    txIdx = 0;

    /* Setup Receiver Buffer Descriptors */
    for (i = 0; i < PKTBUFSRX; i++)
    {
      rtx.rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
      rtx.rxbd[i].cbd_datlen = 0;
      rtx.rxbd[i].cbd_bufaddr = (uint)net_rx_packets[i];
    }
    rtx.rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

    /* Setup Ethernet Transmitter Buffer Descriptors */
    for (i = 0; i < TX_BUF_CNT; i++)
    {
      rtx.txbd[i].cbd_sc = 0;
      rtx.txbd[i].cbd_datlen = 0;
      rtx.txbd[i].cbd_bufaddr = 0;
    }
    rtx.txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

    /* 28.9 - (7): initialize parameter ram */
    pram_ptr = (fcc_enet_t *)&(cpm->im_dprambase[info->proff_enet]);

    /* clear whole structure to make sure all reserved fields are zero */
    memset((void*)pram_ptr, 0, sizeof(fcc_enet_t));

    /*
     * common Parameter RAM area
     *
     * Allocate space in the reserved FCC area of DPRAM for the
     * internal buffers.  No one uses this space (yet), so we
     * can do this.  Later, we will add resource management for
     * this area.
     * CPM_FCC_SPECIAL_BASE:	0xB000 for MPC8540, MPC8560
     *				0x9000 for MPC8541, MPC8555
     */
    mem_addr = CPM_FCC_SPECIAL_BASE + ((info->ether_index) * 64);
    pram_ptr->fen_genfcc.fcc_riptr = mem_addr;
    pram_ptr->fen_genfcc.fcc_tiptr = mem_addr+32;
    /*
     * Set maximum bytes per receive buffer.
     * It must be a multiple of 32.
     */
    pram_ptr->fen_genfcc.fcc_mrblr = PKT_MAXBLR_SIZE; /* 1536 */
    /* localbus SDRAM should be preferred */
    pram_ptr->fen_genfcc.fcc_rstate = (CPMFCR_GBL | CPMFCR_EB |
				       CONFIG_SYS_CPMFCR_RAMTYPE) << 24;
    pram_ptr->fen_genfcc.fcc_rbase = (unsigned int)(&rtx.rxbd[rxIdx]);
    pram_ptr->fen_genfcc.fcc_rbdstat = 0;
    pram_ptr->fen_genfcc.fcc_rbdlen = 0;
    pram_ptr->fen_genfcc.fcc_rdptr = 0;
    /* localbus SDRAM should be preferred */
    pram_ptr->fen_genfcc.fcc_tstate = (CPMFCR_GBL | CPMFCR_EB |
				       CONFIG_SYS_CPMFCR_RAMTYPE) << 24;
    pram_ptr->fen_genfcc.fcc_tbase = (unsigned int)(&rtx.txbd[txIdx]);
    pram_ptr->fen_genfcc.fcc_tbdstat = 0;
    pram_ptr->fen_genfcc.fcc_tbdlen = 0;
    pram_ptr->fen_genfcc.fcc_tdptr = 0;

    /* protocol-specific area */
    pram_ptr->fen_statbuf = 0x0;
    pram_ptr->fen_cmask = 0xdebb20e3;	/* CRC mask */
    pram_ptr->fen_cpres = 0xffffffff;	/* CRC preset */
    pram_ptr->fen_crcec = 0;
    pram_ptr->fen_alec = 0;
    pram_ptr->fen_disfc = 0;
    pram_ptr->fen_retlim = 15;		/* Retry limit threshold */
    pram_ptr->fen_retcnt = 0;
    pram_ptr->fen_pper = 0;
    pram_ptr->fen_boffcnt = 0;
    pram_ptr->fen_gaddrh = 0;
    pram_ptr->fen_gaddrl = 0;
    pram_ptr->fen_mflr = PKT_MAXBUF_SIZE;   /* maximum frame length register */
    /*
     * Set Ethernet station address.
     *
     * This is supplied in the board information structure, so we
     * copy that into the controller.
     * So far we have only been given one Ethernet address. We make
     * it unique by setting a few bits in the upper byte of the
     * non-static part of the address.
     */
#define ea eth_get_ethaddr()
    pram_ptr->fen_paddrh = (ea[5] << 8) + ea[4];
    pram_ptr->fen_paddrm = (ea[3] << 8) + ea[2];
    pram_ptr->fen_paddrl = (ea[1] << 8) + ea[0];
#undef ea
    pram_ptr->fen_ibdcount = 0;
    pram_ptr->fen_ibdstart = 0;
    pram_ptr->fen_ibdend = 0;
    pram_ptr->fen_txlen = 0;
    pram_ptr->fen_iaddrh = 0;  /* disable hash */
    pram_ptr->fen_iaddrl = 0;
    pram_ptr->fen_minflr = PKT_MINBUF_SIZE; /* minimum frame length register: 64 */
    /* pad pointer. use tiptr since we don't need a specific padding char */
    pram_ptr->fen_padptr = pram_ptr->fen_genfcc.fcc_tiptr;
    pram_ptr->fen_maxd1 = PKT_MAXDMA_SIZE;	/* maximum DMA1 length:1520 */
    pram_ptr->fen_maxd2 = PKT_MAXDMA_SIZE;	/* maximum DMA2 length:1520 */

#if defined(ET_DEBUG)
    printf("parm_ptr(0xff788500) = %p\n",pram_ptr);
    printf("pram_ptr->fen_genfcc.fcc_rbase %08x\n",
	pram_ptr->fen_genfcc.fcc_rbase);
    printf("pram_ptr->fen_genfcc.fcc_tbase %08x\n",
	pram_ptr->fen_genfcc.fcc_tbase);
#endif

    /* 28.9 - (8)(9): clear out events in FCCE */
    /* 28.9 - (9): FCCM: mask all events */
    if(info->ether_index == 0) {
	cpm->im_cpm_fcc1.fcce = ~0x0;
	cpm->im_cpm_fcc1.fccm = 0;
    } else if (info->ether_index == 1) {
	cpm->im_cpm_fcc2.fcce = ~0x0;
	cpm->im_cpm_fcc2.fccm = 0;
    } else if (info->ether_index == 2) {
	cpm->im_cpm_fcc3.fcce = ~0x0;
	cpm->im_cpm_fcc3.fccm = 0;
    }

    /* 28.9 - (10-12): we don't use ethernet interrupts */

    /* 28.9 - (13)
     *
     * Let's re-initialize the channel now.  We have to do it later
     * than the manual describes because we have just now finished
     * the BD initialization.
     */
    cp->cpcr = mk_cr_cmd(info->cpm_cr_enet_page,
			    info->cpm_cr_enet_sblock,
			    0x0c,
			    CPM_CR_INIT_TRX) | CPM_CR_FLG;
    do {
	__asm__ __volatile__ ("eieio");
    } while (cp->cpcr & CPM_CR_FLG);

    /* 28.9 - (14): enable tx/rx in gfmr */
    if(info->ether_index == 0) {
	cpm->im_cpm_fcc1.gfmr |= FCC_GFMR_ENT | FCC_GFMR_ENR;
    } else if (info->ether_index == 1) {
	cpm->im_cpm_fcc2.gfmr |= FCC_GFMR_ENT | FCC_GFMR_ENR;
    } else if (info->ether_index == 2) {
	cpm->im_cpm_fcc3.gfmr |= FCC_GFMR_ENT | FCC_GFMR_ENR;
    }

    return 1;
}

static void fec_halt(struct eth_device* dev)
{
    struct ether_fcc_info_s * info = dev->priv;
    volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;

    /* write GFMR: disable tx/rx */
    if(info->ether_index == 0) {
	cpm->im_cpm_fcc1.gfmr &= ~(FCC_GFMR_ENT | FCC_GFMR_ENR);
    } else if(info->ether_index == 1) {
	cpm->im_cpm_fcc2.gfmr &= ~(FCC_GFMR_ENT | FCC_GFMR_ENR);
    } else if(info->ether_index == 2) {
	cpm->im_cpm_fcc3.gfmr &= ~(FCC_GFMR_ENT | FCC_GFMR_ENR);
    }
}

int fec_initialize(bd_t *bis)
{
	struct eth_device* dev;
	int i;

	for (i = 0; i < ARRAY_SIZE(ether_fcc_info); i++)
	{
		dev = (struct eth_device*) malloc(sizeof *dev);
		memset(dev, 0, sizeof *dev);

		sprintf(dev->name, "FCC%d",
			ether_fcc_info[i].ether_index + 1);
		dev->priv   = &ether_fcc_info[i];
		dev->init   = fec_init;
		dev->halt   = fec_halt;
		dev->send   = fec_send;
		dev->recv   = fec_recv;

		eth_register(dev);

#if (defined(CONFIG_MII) || defined(CONFIG_CMD_MII)) \
		&& defined(CONFIG_BITBANGMII)
		int retval;
		struct mii_dev *mdiodev = mdio_alloc();
		if (!mdiodev)
			return -ENOMEM;
		strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
		mdiodev->read = bb_miiphy_read;
		mdiodev->write = bb_miiphy_write;

		retval = mdio_register(mdiodev);
		if (retval < 0)
			return retval;
#endif
	}

	return 1;
}

#endif
