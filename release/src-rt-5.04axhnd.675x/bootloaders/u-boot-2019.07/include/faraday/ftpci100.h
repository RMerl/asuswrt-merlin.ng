/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday FTPCI100 PCI Bridge Controller Device Driver Implementation
 *
 * Copyright (C) 2010 Andes Technology Corporation
 * Gavin Guo, Andes Technology Corporation <gavinguo@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __FTPCI100_H
#define __FTPCI100_H

/* AHB Control Registers */
struct ftpci100_ahbc {
	unsigned int iosize;		/* 0x00 - I/O Space Size Signal */
	unsigned int prot;		/* 0x04 - AHB Protection */
	unsigned int rsved[8];		/* 0x08-0x24 - Reserved */
	unsigned int conf;		/* 0x28 - PCI Configuration */
	unsigned int data;		/* 0x2c - PCI Configuration DATA */
};

/*
 * FTPCI100_IOSIZE_REG's constant definitions
 */
#define FTPCI100_BASE_IO_SIZE(x)	(ffs(x) - 1)	/* 1M - 2048M */

/*
 * PCI Configuration Register
 */
#define PCI_INT_MASK			0x4c
#define PCI_MEM_BASE_SIZE1		0x50
#define PCI_MEM_BASE_SIZE2		0x54
#define PCI_MEM_BASE_SIZE3		0x58

/*
 * PCI_INT_MASK's bit definitions
 */
#define PCI_INTA_ENABLE			(1 << 22)
#define PCI_INTB_ENABLE			(1 << 23)
#define PCI_INTC_ENABLE			(1 << 24)
#define PCI_INTD_ENABLE			(1 << 25)

/*
 * PCI_MEM_BASE_SIZE1's constant definitions
 */
#define FTPCI100_BASE_ADR_SIZE(x)	((ffs(x) - 1) << 16)	/* 1M - 2048M */

#define FTPCI100_MAX_FUNCTIONS		20
#define PCI_IRQ_LINES			4

#define MAX_BUS_NUM			256
#define MAX_DEV_NUM			32
#define MAX_FUN_NUM			8

#define PCI_MAX_BAR_PER_FUNC		6

/*
 * PCI_MEM_SIZE
 */
#define FTPCI100_MEM_SIZE(x)		(ffs(x) << 24)

/* This definition is used by pci_ftpci_init() */
#define FTPCI100_BRIDGE_VENDORID		0x159b
#define FTPCI100_BRIDGE_DEVICEID		0x4321

void pci_ftpci_init(void);

struct pcibar {
	unsigned int size;
	unsigned int addr;
};

struct pci_config {
	unsigned int bus;
	unsigned int dev;				/* device */
	unsigned int func;
	unsigned int pin;
	unsigned short v_id;				/* vendor id */
	unsigned short d_id;				/* device id */
	struct pcibar bar[PCI_MAX_BAR_PER_FUNC + 1];
};

#endif
