/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003 Stefan Roese, stefan.roese@esd-electronics.com
 */

#ifndef _universe_h
#define _universe_h

typedef struct _UNIVERSE UNIVERSE;
typedef struct _SLAVE_IMAGE SLAVE_IMAGE;
typedef struct _TDMA_CMD_PACKET TDMA_CMD_PACKET;

struct _SLAVE_IMAGE {
	unsigned int ctl;      /* Control     */
	unsigned int bs;       /* Base        */
	unsigned int bd;       /* Bound       */
	unsigned int to;       /* Translation */
	unsigned int reserved;
};

struct _UNIVERSE {
	unsigned int pci_id;
	unsigned int pci_csr;
	unsigned int pci_class;
	unsigned int pci_misc0;
	unsigned int pci_bs;
	unsigned int spare0[10];
	unsigned int pci_misc1;
	unsigned int spare1[48];
	SLAVE_IMAGE  lsi[4];
	unsigned int spare2[8];
	unsigned int scyc_ctl;
	unsigned int scyc_addr;
	unsigned int scyc_en;
	unsigned int scyc_cmp;
	unsigned int scyc_swp;
	unsigned int lmisc;
	unsigned int slsi;
	unsigned int l_cmderr;
	unsigned int laerr;
	unsigned int spare3[27];
	unsigned int dctl;
	unsigned int dtbc;
	unsigned int dla;
	unsigned int spare4[1];
	unsigned int dva;
	unsigned int spare5[1];
	unsigned int dcpp;
	unsigned int spare6[1];
	unsigned int dgcs;
	unsigned int d_llue;
	unsigned int spare7[54];
	unsigned int lint_en;
	unsigned int lint_stat;
	unsigned int lint_map0;
	unsigned int lint_map1;
	unsigned int vint_en;
	unsigned int vint_stat;
	unsigned int vint_map0;
	unsigned int vint_map1;
	unsigned int statid;
	unsigned int vx_statid[7];
	unsigned int spare8[48];
	unsigned int mast_ctl;
	unsigned int misc_ctl;
	unsigned int misc_stat;
	unsigned int user_am;
	unsigned int spare9[700];
	SLAVE_IMAGE  vsi[4];
	unsigned int spare10[8];
	unsigned int vrai_ctl;
	unsigned int vrai_bs;
	unsigned int spare11[2];
	unsigned int vcsr_ctl;
	unsigned int vcsr_to;
	unsigned int v_amerr;
	unsigned int vaerr;
	unsigned int spare12[25];
	unsigned int vcsr_clr;
	unsigned int vcsr_set;
	unsigned int vcsr_bs;
};

#define IRQ_VOWN    0x0001
#define IRQ_VIRQ1   0x0002
#define IRQ_VIRQ2   0x0004
#define IRQ_VIRQ3   0x0008
#define IRQ_VIRQ4   0x0010
#define IRQ_VIRQ5   0x0020
#define IRQ_VIRQ6   0x0040
#define IRQ_VIRQ7   0x0080
#define IRQ_DMA     0x0100
#define IRQ_LERR    0x0200
#define IRQ_VERR    0x0400
#define IRQ_res     0x0800
#define IRQ_IACK    0x1000
#define IRQ_SWINT   0x2000
#define IRQ_SYSFAIL 0x4000
#define IRQ_ACFAIL  0x8000

struct _TDMA_CMD_PACKET {
	unsigned int dctl;   /* DMA Control         */
	unsigned int dtbc;   /* Transfer Byte Count */
	unsigned int dlv;    /* PCI Address         */
	unsigned int res1;   /* Reserved            */
	unsigned int dva;    /* Vme Address         */
	unsigned int res2;   /* Reserved            */
	unsigned int dcpp;   /* Pointer to Numed Cmd Packet with rPN */
	unsigned int res3;   /* Reserved                             */
};

#define VME_AM_A16		0x01
#define VME_AM_A24		0x02
#define VME_AM_A32		0x03
#define VME_AM_Axx		0x03
#define VME_AM_SUP		0x04
#define VME_AM_DATA		0x10
#define VME_AM_PROG		0x20
#define VME_AM_Mxx		0x30

#define VME_FLAG_D8             0x01
#define VME_FLAG_D16            0x02
#define VME_FLAG_D32            0x03
#define VME_FLAG_Dxx		0x03

#define PCI_MS_MEM		0x01
#define PCI_MS_IO		0x02
#define PCI_MS_CONFIG		0x03
#define PCI_MS_Mxx		0x03

#endif
