// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003 Stefan Roese, stefan.roese@esd-electronics.com
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <pci.h>

#include <universe.h>

#define PCI_VENDOR PCI_VENDOR_ID_TUNDRA
#define PCI_DEVICE PCI_DEVICE_ID_TUNDRA_CA91C042


typedef struct _UNI_DEV UNI_DEV;

struct _UNI_DEV {
	int            bus;
	pci_dev_t      busdevfn;
	UNIVERSE       *uregs;
	unsigned int   pci_bs;
};

static UNI_DEV   *dev;


int universe_init(void)
{
	int j, result;
	pci_dev_t busdevfn;
	unsigned int val;

	busdevfn = pci_find_device(PCI_VENDOR, PCI_DEVICE, 0);
	if (busdevfn == -1) {
		puts("No Tundra Universe found!\n");
		return -1;
	}

	/* Lets turn Latency off */
	pci_write_config_dword(busdevfn, 0x0c, 0);

	dev = malloc(sizeof(*dev));
	if (NULL == dev) {
		puts("UNIVERSE: No memory!\n");
		result = -1;
		goto break_20;
	}

	memset(dev, 0, sizeof(*dev));
	dev->busdevfn = busdevfn;

	pci_read_config_dword(busdevfn, PCI_BASE_ADDRESS_1, &val);
	if (val & 1) {
		pci_read_config_dword(busdevfn, PCI_BASE_ADDRESS_0, &val);
	}
	val &= ~0xf;
	dev->uregs = (UNIVERSE *)val;

	debug ("UNIVERSE-Base    : %p\n", dev->uregs);

	/* check mapping  */
	debug (" Read via mapping, PCI_ID = %08X\n", readl(&dev->uregs->pci_id));
	if (((PCI_DEVICE <<16) | PCI_VENDOR) !=  readl(&dev->uregs->pci_id)) {
		printf ("UNIVERSE: Cannot read PCI-ID via Mapping: %08x\n",
			readl(&dev->uregs->pci_id));
		result = -1;
		goto break_30;
	}

	debug ("PCI_BS = %08X\n", readl(&dev->uregs->pci_bs));

	dev->pci_bs = readl(&dev->uregs->pci_bs);

	/* turn off windows */
	for (j=0; j <4; j ++) {
		writel(0x00800000, &dev->uregs->lsi[j].ctl);
		writel(0x00800000, &dev->uregs->vsi[j].ctl);
	}

	/*
	 * Write to Misc Register
	 * Set VME Bus Time-out
	 *   Arbitration Mode
	 *   DTACK Enable
	 */
	writel(0x15040000 | (readl(&dev->uregs->misc_ctl) & 0x00020000), &dev->uregs->misc_ctl);

	if (readl(&dev->uregs->misc_ctl) & 0x00020000) {
		debug ("System Controller!\n"); /* test-only */
	} else {
		debug ("Not System Controller!\n"); /* test-only */
	}

	/*
	 * Lets turn off interrupts
	 */
	writel(0x00000000,&dev->uregs->lint_en);   /* Disable interrupts in the Universe first */
	writel(0x0000FFFF,&dev->uregs->lint_stat); /* Clear Any Pending Interrupts */
	eieio();
	writel(0x0000, &dev->uregs->lint_map0);  /* Map all ints to 0 */
	writel(0x0000, &dev->uregs->lint_map1);  /* Map all ints to 0 */
	eieio();

	return 0;

 break_30:
	free(dev);
 break_20:
	return result;
}


/*
 * Create pci slave window (access: pci -> vme)
 */
int universe_pci_slave_window(unsigned int pciAddr, unsigned int vmeAddr, int size, int vam, int pms, int vdw)
{
	int result, i;
	unsigned int ctl = 0;

	if (NULL == dev) {
		result = -1;
		goto exit_10;
	}

	for (i = 0; i < 4; i++) {
		if (0x00800000 == readl(&dev->uregs->lsi[i].ctl))
			break;
	}

	if (i == 4) {
		printf ("universe: No Image available\n");
		result = -1;
		goto exit_10;
	}

	debug ("universe: Using image %d\n", i);

	writel(pciAddr , &dev->uregs->lsi[i].bs);
	writel((pciAddr + size), &dev->uregs->lsi[i].bd);
	writel((vmeAddr - pciAddr), &dev->uregs->lsi[i].to);

	switch (vam & VME_AM_Axx) {
	case VME_AM_A16:
		ctl = 0x00000000;
		break;
	case VME_AM_A24:
		ctl = 0x00010000;
		break;
	case VME_AM_A32:
		ctl = 0x00020000;
		break;
	}

	switch (vam & VME_AM_Mxx) {
	case VME_AM_DATA:
		ctl |= 0x00000000;
		break;
	case VME_AM_PROG:
		ctl |= 0x00008000;
		break;
	}

	if (vam & VME_AM_SUP) {
		ctl |= 0x00001000;

	}

	switch (vdw & VME_FLAG_Dxx) {
	case VME_FLAG_D8:
		ctl |= 0x00000000;
		break;
	case VME_FLAG_D16:
		ctl |= 0x00400000;
		break;
	case VME_FLAG_D32:
		ctl |= 0x00800000;
		break;
	}

	switch (pms & PCI_MS_Mxx) {
	case PCI_MS_MEM:
		ctl |= 0x00000000;
		break;
	case PCI_MS_IO:
		ctl |= 0x00000001;
		break;
	case PCI_MS_CONFIG:
		ctl |= 0x00000002;
		break;
	}

	ctl |= 0x80000000;    /* enable */

	writel(ctl, &dev->uregs->lsi[i].ctl);

	debug ("universe: window-addr=%p\n", &dev->uregs->lsi[i].ctl);
	debug ("universe: pci slave window[%d] ctl=%08x\n", i, readl(&dev->uregs->lsi[i].ctl));
	debug ("universe: pci slave window[%d] bs=%08x\n", i, readl(&dev->uregs->lsi[i].bs));
	debug ("universe: pci slave window[%d] bd=%08x\n", i, readl(&dev->uregs->lsi[i].bd));
	debug ("universe: pci slave window[%d] to=%08x\n", i, readl(&dev->uregs->lsi[i].to));

	return 0;

 exit_10:
	return -result;
}


/*
 * Create vme slave window (access: vme -> pci)
 */
int universe_vme_slave_window(unsigned int vmeAddr, unsigned int pciAddr, int size, int vam, int pms)
{
	int result, i;
	unsigned int ctl = 0;

	if (NULL == dev) {
		result = -1;
		goto exit_10;
	}

	for (i = 0; i < 4; i++) {
		if (0x00800000 == readl(&dev->uregs->vsi[i].ctl))
			break;
	}

	if (i == 4) {
		printf ("universe: No Image available\n");
		result = -1;
		goto exit_10;
	}

	debug ("universe: Using image %d\n", i);

	writel(vmeAddr , &dev->uregs->vsi[i].bs);
	writel((vmeAddr + size), &dev->uregs->vsi[i].bd);
	writel((pciAddr - vmeAddr), &dev->uregs->vsi[i].to);

	switch (vam & VME_AM_Axx) {
	case VME_AM_A16:
		ctl = 0x00000000;
		break;
	case VME_AM_A24:
		ctl = 0x00010000;
		break;
	case VME_AM_A32:
		ctl = 0x00020000;
		break;
	}

	switch (vam & VME_AM_Mxx) {
	case VME_AM_DATA:
		ctl |= 0x00000000;
		break;
	case VME_AM_PROG:
		ctl |= 0x00800000;
		break;
	}

	if (vam & VME_AM_SUP) {
		ctl |= 0x00100000;

	}

	switch (pms & PCI_MS_Mxx) {
	case PCI_MS_MEM:
		ctl |= 0x00000000;
		break;
	case PCI_MS_IO:
		ctl |= 0x00000001;
		break;
	case PCI_MS_CONFIG:
		ctl |= 0x00000002;
		break;
	}

	ctl |= 0x80f00000;    /* enable */

	writel(ctl, &dev->uregs->vsi[i].ctl);

	debug ("universe: window-addr=%p\n", &dev->uregs->vsi[i].ctl);
	debug ("universe: vme slave window[%d] ctl=%08x\n", i, readl(&dev->uregs->vsi[i].ctl));
	debug ("universe: vme slave window[%d] bs=%08x\n", i, readl(&dev->uregs->vsi[i].bs));
	debug ("universe: vme slave window[%d] bd=%08x\n", i, readl(&dev->uregs->vsi[i].bd));
	debug ("universe: vme slave window[%d] to=%08x\n", i, readl(&dev->uregs->vsi[i].to));

	return 0;

 exit_10:
	return -result;
}


/*
 * Tundra Universe configuration
 */
int do_universe(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr1 = 0, addr2 = 0, size = 0, vam = 0, pms = 0, vdw = 0;
	char cmd = 'x';

	/* get parameter */
	if (argc > 1)
		cmd = argv[1][0];
	if (argc > 2)
		addr1 = hextoul(argv[2], NULL);
	if (argc > 3)
		addr2 = hextoul(argv[3], NULL);
	if (argc > 4)
		size = hextoul(argv[4], NULL);
	if (argc > 5)
		vam = hextoul(argv[5], NULL);
	if (argc > 6)
		pms = hextoul(argv[6], NULL);
	if (argc > 7)
		vdw = hextoul(argv[7], NULL);

	switch (cmd) {
	case 'i':		/* init */
		universe_init();
		break;
	case 'v':		/* vme */
		printf("Configuring Universe VME Slave Window (VME->PCI):\n");
		printf("  vme=%08lx pci=%08lx size=%08lx vam=%02lx pms=%02lx\n",
		       addr1, addr2, size, vam, pms);
		universe_vme_slave_window(addr1, addr2, size, vam, pms);
		break;
	case 'p':		/* pci */
		printf("Configuring Universe PCI Slave Window (PCI->VME):\n");
		printf("  pci=%08lx vme=%08lx size=%08lx vam=%02lx pms=%02lx vdw=%02lx\n",
		       addr1, addr2, size, vam, pms, vdw);
		universe_pci_slave_window(addr1, addr2, size, vam, pms, vdw);
		break;
	default:
		printf("Universe command %s not supported!\n", argv[1]);
	}

	return 0;
}


U_BOOT_CMD(
	universe,	8,	1,	do_universe,
	"initialize and configure Turndra Universe",
	"init\n"
	"    - initialize universe\n"
	"universe vme [vme_addr] [pci_addr] [size] [vam] [pms]\n"
	"    - create vme slave window (access: vme->pci)\n"
	"universe pci [pci_addr] [vme_addr] [size] [vam] [pms] [vdw]\n"
	"    - create pci slave window (access: pci->vme)\n"
	"    [vam] = VMEbus Address-Modifier:  01 -> A16 Address Space\n"
	"                                      02 -> A24 Address Space\n"
	"                                      03 -> A32 Address Space\n"
	"                                      04 -> Supervisor AM Code\n"
	"                                      10 -> Data AM Code\n"
	"                                      20 -> Program AM Code\n"
	"    [pms] = PCI Memory Space:         01 -> Memory Space\n"
	"                                      02 -> I/O Space\n"
	"                                      03 -> Configuration Space\n"
	"    [vdw] = VMEbus Maximum Datawidth: 01 -> D8 Data Width\n"
	"                                      02 -> D16 Data Width\n"
	"                                      03 -> D32 Data Width"
);
