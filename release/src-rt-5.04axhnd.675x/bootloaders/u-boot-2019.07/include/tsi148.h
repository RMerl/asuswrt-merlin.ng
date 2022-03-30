/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Reinhard Arlt, reinhard.arlt@esd-electronics.com
 *
 * base on universe.h by
 *
 * (C) Copyright 2003 Stefan Roese, stefan.roese@esd-electronics.com
 */

#ifndef _tsi148_h
#define _tsi148_h

#ifndef PCI_DEVICE_ID_TUNDRA_TSI148
#define PCI_DEVICE_ID_TUNDRA_TSI148 0x0148
#endif

typedef struct _TSI148 TSI148;
typedef struct _OUTBOUND OUTBOUND;
typedef struct _INBOUND  INBOUND;
typedef struct _TDMA_CMD_PACKET TDMA_CMD_PACKET;

struct _OUTBOUND {
	unsigned int otsau;                   /* 0x000 Outbound start       upper */
	unsigned int otsal;                   /* 0x004 Outbouud start       lower */
	unsigned int oteau;                   /* 0x008 Outbound end         upper */
	unsigned int oteal;                   /* 0x00c Outbound end         lower */
	unsigned int otofu;                   /* 0x010 Outbound translation upper */
	unsigned int otofl;                   /* 0x014 Outbound translation lower */
	unsigned int otbs;                    /* 0x018 Outbound translation 2eSST */
	unsigned int otat;                    /* 0x01c Outbound translation attr  */
};

struct _INBOUND {
	unsigned int itsau;                   /* 0x000 inbound  start       upper */
	unsigned int itsal;                   /* 0x004 inbouud  start       lower */
	unsigned int iteau;                   /* 0x008 inbound  end         upper */
	unsigned int iteal;                   /* 0x00c inbound  end         lower */
	unsigned int itofu;                   /* 0x010 inbound  translation upper */
	unsigned int itofl;                   /* 0x014 inbound  translation lower */
	unsigned int itat;                    /* 0x018 inbound  translation attr  */
	unsigned int spare;                   /* 0x01c not used                   */
};

struct _TSI148 {
	unsigned int pci_id;                  /* 0x000         */
	unsigned int pci_csr;                 /* 0x004         */
	unsigned int pci_class;               /* 0x008         */
	unsigned int pci_misc0;               /* 0x00c         */
	unsigned int pci_mbarl;               /* 0x010         */
	unsigned int pci_mbarh;               /* 0x014         */
	unsigned int spare0[(0x03c-0x018)/4]; /* 0x018         */
	unsigned int pci_misc1;               /* 0x03c         */
	unsigned int pci_pcixcap;             /* 0x040         */
	unsigned int pci_pcixstat;            /* 0x044         */
	unsigned int spare1[(0x100-0x048)/4]; /* 0x048         */
	OUTBOUND     outbound[8];             /* 0x100         */
	unsigned int viack[8];                /* 0x204         */
	unsigned int rmwau;                   /* 0x220         */
	unsigned int rmwal;                   /* 0x224         */
	unsigned int rmwen;                   /* 0x228         */
	unsigned int rmwc;                    /* 0x22c         */
	unsigned int rmws;                    /* 0x230         */
	unsigned int vmctrl;                  /* 0x234         */
	unsigned int vctrl;                   /* 0x238         */
	unsigned int vstat;                   /* 0x23c         */
	unsigned int pcsr;                    /* 0x240         */
	unsigned int spare2[3];               /* 0x244 - 0x24c */
	unsigned int vmefl;                   /* 0x250         */
	unsigned int spare3[3];               /* 0x254 - 0x25c */
	unsigned int veau;                    /* 0x260         */
	unsigned int veal;                    /* 0x264         */
	unsigned int veat;                    /* 0x268         */
	unsigned int spare4[1];               /* 0x26c         */
	unsigned int edpau;                   /* 0x270         */
	unsigned int edpal;                   /* 0x274         */
	unsigned int edpxa;                   /* 0x278         */
	unsigned int edpxs;                   /* 0x27c         */
	unsigned int edpat;                   /* 0x280         */
	unsigned int spare5[31];              /* 0x284 - 0x2fc */
	INBOUND      inbound[8];              /* 0x100         */
	unsigned int gbau;                    /* 0x400         */
	unsigned int gbal;                    /* 0x404         */
	unsigned int gcsrat;                  /* 0x408         */
	unsigned int cbau;                    /* 0x40c         */
	unsigned int cbal;                    /* 0x410         */
	unsigned int crgat;                   /* 0x414         */
	unsigned int crou;                    /* 0x418         */
	unsigned int crol;                    /* 0x41c         */
	unsigned int crat;                    /* 0x420         */
	unsigned int lmbau;                   /* 0x424         */
	unsigned int lmbal;                   /* 0x428         */
	unsigned int lmat;                    /* 0x42c         */
	unsigned int r64bcu;                  /* 0x430         */
	unsigned int r64bcl;                  /* 0x434         */
	unsigned int bpgtr;                   /* 0x438         */
	unsigned int bpctr;                   /* 0x43c         */
	unsigned int vicr;                    /* 0x440         */
	unsigned int spare6[1];               /* 0x444         */
	unsigned int inten;                   /* 0x448         */
	unsigned int inteo;                   /* 0x44c         */
	unsigned int ints;                    /* 0x450         */
	unsigned int intc;                    /* 0x454         */
	unsigned int intm1;                   /* 0x458         */
	unsigned int intm2;                   /* 0x45c         */
	unsigned int spare7[40];              /* 0x460 - 0x4fc */
	unsigned int dctl0;                   /* 0x500         */
	unsigned int dsta0;                   /* 0x504         */
	unsigned int dcsau0;                  /* 0x508         */
	unsigned int dcsal0;                  /* 0x50c         */
	unsigned int dcdau0;                  /* 0x510         */
	unsigned int dcdal0;                  /* 0x514         */
	unsigned int dclau0;                  /* 0x518         */
	unsigned int dclal0;                  /* 0x51c         */
	unsigned int dsau0;                   /* 0x520         */
	unsigned int dsal0;                   /* 0x524         */
	unsigned int ddau0;                   /* 0x528         */
	unsigned int ddal0;                   /* 0x52c         */
	unsigned int dsat0;                   /* 0x530         */
	unsigned int ddat0;                   /* 0x534         */
	unsigned int dnlau0;                  /* 0x538         */
	unsigned int dnlal0;                  /* 0x53c         */
	unsigned int dcnt0;                   /* 0x540         */
	unsigned int ddbs0;                   /* 0x544         */
	unsigned int r20[14];                 /* 0x548 - 0x57c */
	unsigned int dctl1;                   /* 0x580         */
	unsigned int dsta1;                   /* 0x584         */
	unsigned int dcsau1;                  /* 0x588         */
	unsigned int dcsal1;                  /* 0x58c         */
	unsigned int dcdau1;                  /* 0x590         */
	unsigned int dcdal1;                  /* 0x594         */
	unsigned int dclau1;                  /* 0x598         */
	unsigned int dclal1;                  /* 0x59c         */
	unsigned int dsau1;                   /* 0x5a0         */
	unsigned int dsal1;                   /* 0x5a4         */
	unsigned int ddau1;                   /* 0x5a8         */
	unsigned int ddal1;                   /* 0x5ac         */
	unsigned int dsat1;                   /* 0x5b0         */
	unsigned int ddat1;                   /* 0x5b4         */
	unsigned int dnlau1;                  /* 0x5b8         */
	unsigned int dnlal1;                  /* 0x5bc         */
	unsigned int dcnt1;                   /* 0x5c0         */
	unsigned int ddbs1;                   /* 0x5c4         */
	unsigned int r21[14];                 /* 0x5c8 - 0x5fc */
	unsigned int devi_veni_2;             /* 0x600         */
	unsigned int gctrl_ga_revid;          /* 0x604         */
	unsigned int semaphore0_1_2_3;        /* 0x608         */
	unsigned int semaphore4_5_6_7;        /* 0x60c         */
	unsigned int mbox0;                   /* 0x610         */
	unsigned int mbox1;                   /* 0x614         */
	unsigned int mbox2;                   /* 0x618         */
	unsigned int mbox3;                   /* 0x61c         */
	unsigned int r22[629];                /* 0x620 - 0xff0 */
	unsigned int csrbcr;                  /* 0xff4         */
	unsigned int csrbsr;                  /* 0xff8         */
	unsigned int cbar;                    /* 0xffc         */
};

#define IRQ_VOWN	0x0001
#define IRQ_VIRQ1	0x0002
#define IRQ_VIRQ2	0x0004
#define IRQ_VIRQ3	0x0008
#define IRQ_VIRQ4	0x0010
#define IRQ_VIRQ5	0x0020
#define IRQ_VIRQ6	0x0040
#define IRQ_VIRQ7	0x0080
#define IRQ_DMA		0x0100
#define IRQ_LERR	0x0200
#define IRQ_VERR	0x0400
#define IRQ_res		0x0800
#define IRQ_IACK	0x1000
#define IRQ_SWINT	0x2000
#define IRQ_SYSFAIL	0x4000
#define IRQ_ACFAIL	0x8000

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
#define VME_AM_USR		0x04
#define VME_AM_SUP		0x08
#define VME_AM_DATA		0x10
#define VME_AM_PROG		0x20
#define VME_AM_Mxx		(VME_AM_DATA | VME_AM_PROG)

#define VME_FLAG_D8		0x01
#define VME_FLAG_D16		0x02
#define VME_FLAG_D32		0x03
#define VME_FLAG_Dxx		0x03

#endif
