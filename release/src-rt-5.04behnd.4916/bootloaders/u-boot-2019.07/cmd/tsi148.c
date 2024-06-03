// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Reinhard Arlt, reinhard.arlt@esd-electronics.com
 *
 * base on universe.h by
 *
 * (C) Copyright 2003 Stefan Roese, stefan.roese@esd-electronics.com
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <pci.h>

#include <tsi148.h>

#define LPCI_VENDOR PCI_VENDOR_ID_TUNDRA
#define LPCI_DEVICE PCI_DEVICE_ID_TUNDRA_TSI148

typedef struct _TSI148_DEV TSI148_DEV;

struct _TSI148_DEV {
	int           bus;
	pci_dev_t     busdevfn;
	TSI148       *uregs;
	unsigned int  pci_bs;
};

static TSI148_DEV *dev;

/*
 * Most of the TSI148 register are BIGENDIAN
 * This is the reason for the __raw_writel(htonl(x), x) usage!
 */

int tsi148_init(void)
{
	int j, result;
	pci_dev_t busdevfn;
	unsigned int val;

	busdevfn = pci_find_device(LPCI_VENDOR, LPCI_DEVICE, 0);
	if (busdevfn == -1) {
		puts("Tsi148: No Tundra Tsi148 found!\n");
		return -1;
	}

	/* Lets turn Latency off */
	pci_write_config_dword(busdevfn, 0x0c, 0);

	dev = malloc(sizeof(*dev));
	if (NULL == dev) {
		puts("Tsi148: No memory!\n");
		return -1;
	}

	memset(dev, 0, sizeof(*dev));
	dev->busdevfn = busdevfn;

	pci_read_config_dword(busdevfn, PCI_BASE_ADDRESS_0, &val);
	val &= ~0xf;
	dev->uregs = (TSI148 *)val;

	debug("Tsi148: Base    : %p\n", dev->uregs);

	/* check mapping */
	debug("Tsi148: Read via mapping, PCI_ID = %08X\n",
	      readl(&dev->uregs->pci_id));
	if (((LPCI_DEVICE << 16) | LPCI_VENDOR) != readl(&dev->uregs->pci_id)) {
		printf("Tsi148: Cannot read PCI-ID via Mapping: %08x\n",
		       readl(&dev->uregs->pci_id));
		result = -1;
		goto break_30;
	}

	debug("Tsi148: PCI_BS = %08X\n", readl(&dev->uregs->pci_mbarl));

	dev->pci_bs = readl(&dev->uregs->pci_mbarl);

	/* turn off windows */
	for (j = 0; j < 8; j++) {
		__raw_writel(htonl(0x00000000), &dev->uregs->outbound[j].otat);
		__raw_writel(htonl(0x00000000), &dev->uregs->inbound[j].itat);
	}

	/* Tsi148 VME timeout etc */
	__raw_writel(htonl(0x00000084), &dev->uregs->vctrl);

#ifdef DEBUG
	if ((__raw_readl(&dev->uregs->vstat) & 0x00000100) != 0)
		printf("Tsi148: System Controller!\n");
	else
		printf("Tsi148: Not System Controller!\n");
#endif

	/*
	 * Lets turn off interrupts
	 */
	/* Disable interrupts in Tsi148 first */
	__raw_writel(htonl(0x00000000), &dev->uregs->inten);
	/* Disable interrupt out */
	__raw_writel(htonl(0x00000000), &dev->uregs->inteo);
	eieio();
	/* Reset all IRQ's */
	__raw_writel(htonl(0x03ff3f00), &dev->uregs->intc);
	/* Map all ints to 0 */
	__raw_writel(htonl(0x00000000), &dev->uregs->intm1);
	__raw_writel(htonl(0x00000000), &dev->uregs->intm2);
	eieio();

	val = __raw_readl(&dev->uregs->vstat);
	val &= ~(0x00004000);
	__raw_writel(val, &dev->uregs->vstat);
	eieio();

	debug("Tsi148: register struct size %08x\n", sizeof(TSI148));

	return 0;

 break_30:
	free(dev);
	dev = NULL;

	return result;
}

/*
 * Create pci slave window (access: pci -> vme)
 */
int tsi148_pci_slave_window(unsigned int pciAddr, unsigned int vmeAddr,
			    int size, int vam, int vdw)
{
	int result, i;
	unsigned int ctl = 0;

	if (NULL == dev) {
		result = -1;
		goto exit_10;
	}

	for (i = 0; i < 8; i++) {
		if (0x00000000 == readl(&dev->uregs->outbound[i].otat))
			break;
	}

	if (i > 7) {
		printf("Tsi148: No Image available\n");
		result = -1;
		goto exit_10;
	}

	debug("Tsi148: Using image %d\n", i);

	printf("Tsi148: Pci addr %08x\n", pciAddr);

	__raw_writel(htonl(pciAddr), &dev->uregs->outbound[i].otsal);
	__raw_writel(0x00000000, &dev->uregs->outbound[i].otsau);
	__raw_writel(htonl(pciAddr + size), &dev->uregs->outbound[i].oteal);
	__raw_writel(0x00000000, &dev->uregs->outbound[i].oteau);
	__raw_writel(htonl(vmeAddr - pciAddr), &dev->uregs->outbound[i].otofl);
	__raw_writel(0x00000000, &dev->uregs->outbound[i].otofu);

	switch (vam & VME_AM_Axx) {
	case VME_AM_A16:
		ctl = 0x00000000;
		break;
	case VME_AM_A24:
		ctl = 0x00000001;
		break;
	case VME_AM_A32:
		ctl = 0x00000002;
		break;
	}

	switch (vam & VME_AM_Mxx) {
	case VME_AM_DATA:
		ctl |= 0x00000000;
		break;
	case VME_AM_PROG:
		ctl |= 0x00000010;
		break;
	}

	if (vam & VME_AM_SUP)
		ctl |= 0x00000020;

	switch (vdw & VME_FLAG_Dxx) {
	case VME_FLAG_D16:
		ctl |= 0x00000000;
		break;
	case VME_FLAG_D32:
		ctl |= 0x00000040;
		break;
	}

	ctl |= 0x80040000;	/* enable, no prefetch */

	__raw_writel(htonl(ctl), &dev->uregs->outbound[i].otat);

	debug("Tsi148: window-addr                =%p\n",
	      &dev->uregs->outbound[i].otsau);
	debug("Tsi148: pci slave window[%d] attr  =%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->outbound[i].otat)));
	debug("Tsi148: pci slave window[%d] start =%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->outbound[i].otsal)));
	debug("Tsi148: pci slave window[%d] end   =%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->outbound[i].oteal)));
	debug("Tsi148: pci slave window[%d] offset=%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->outbound[i].otofl)));

	return 0;

 exit_10:
	return -result;
}

unsigned int tsi148_eval_vam(int vam)
{
	unsigned int ctl = 0;

	switch (vam & VME_AM_Axx) {
	case VME_AM_A16:
		ctl = 0x00000000;
		break;
	case VME_AM_A24:
		ctl = 0x00000010;
		break;
	case VME_AM_A32:
		ctl = 0x00000020;
		break;
	}
	switch (vam & VME_AM_Mxx) {
	case VME_AM_DATA:
		ctl |= 0x00000001;
		break;
	case VME_AM_PROG:
		ctl |= 0x00000002;
		break;
	case (VME_AM_PROG | VME_AM_DATA):
		ctl |= 0x00000003;
		break;
	}

	if (vam & VME_AM_SUP)
		ctl |= 0x00000008;
	if (vam & VME_AM_USR)
		ctl |= 0x00000004;

	return ctl;
}

/*
 * Create vme slave window (access: vme -> pci)
 */
int tsi148_vme_slave_window(unsigned int vmeAddr, unsigned int pciAddr,
			    int size, int vam)
{
	int result, i;
	unsigned int ctl = 0;

	if (NULL == dev) {
		result = -1;
		goto exit_10;
	}

	for (i = 0; i < 8; i++) {
		if (0x00000000 == readl(&dev->uregs->inbound[i].itat))
			break;
	}

	if (i > 7) {
		printf("Tsi148: No Image available\n");
		result = -1;
		goto exit_10;
	}

	debug("Tsi148: Using image %d\n", i);

	__raw_writel(htonl(vmeAddr), &dev->uregs->inbound[i].itsal);
	__raw_writel(0x00000000, &dev->uregs->inbound[i].itsau);
	__raw_writel(htonl(vmeAddr + size), &dev->uregs->inbound[i].iteal);
	__raw_writel(0x00000000, &dev->uregs->inbound[i].iteau);
	__raw_writel(htonl(pciAddr - vmeAddr), &dev->uregs->inbound[i].itofl);
	if (vmeAddr > pciAddr)
		__raw_writel(0xffffffff, &dev->uregs->inbound[i].itofu);
	else
		__raw_writel(0x00000000, &dev->uregs->inbound[i].itofu);

	ctl = tsi148_eval_vam(vam);
	ctl |= 0x80000000;	/* enable */
	__raw_writel(htonl(ctl), &dev->uregs->inbound[i].itat);

	debug("Tsi148: window-addr                =%p\n",
	      &dev->uregs->inbound[i].itsau);
	debug("Tsi148: vme slave window[%d] attr  =%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->inbound[i].itat)));
	debug("Tsi148: vme slave window[%d] start =%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->inbound[i].itsal)));
	debug("Tsi148: vme slave window[%d] end   =%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->inbound[i].iteal)));
	debug("Tsi148: vme slave window[%d] offset=%08x\n",
	      i, ntohl(__raw_readl(&dev->uregs->inbound[i].itofl)));

	return 0;

 exit_10:
	return -result;
}

/*
 * Create vme slave window (access: vme -> gcsr)
 */
int tsi148_vme_gcsr_window(unsigned int vmeAddr, int vam)
{
	int result;
	unsigned int ctl;

	result = 0;

	if (NULL == dev) {
		result = 1;
	} else {
		__raw_writel(htonl(vmeAddr), &dev->uregs->gbal);
		__raw_writel(0x00000000, &dev->uregs->gbau);

		ctl = tsi148_eval_vam(vam);
		ctl |= 0x00000080;	/* enable */
		__raw_writel(htonl(ctl), &dev->uregs->gcsrat);
	}

	return result;
}

/*
 * Create vme slave window (access: vme -> crcsr)
 */
int tsi148_vme_crcsr_window(unsigned int vmeAddr)
{
	int result;
	unsigned int ctl;

	result = 0;

	if (NULL == dev) {
		result = 1;
	} else {
		__raw_writel(htonl(vmeAddr), &dev->uregs->crol);
		__raw_writel(0x00000000, &dev->uregs->crou);

		ctl = 0x00000080;	/* enable */
		__raw_writel(htonl(ctl), &dev->uregs->crat);
	}

	return result;
}

/*
 * Create vme slave window (access: vme -> crg)
 */
int tsi148_vme_crg_window(unsigned int vmeAddr, int vam)
{
	int result;
	unsigned int ctl;

	result = 0;

	if (NULL == dev) {
		result = 1;
	} else {
		__raw_writel(htonl(vmeAddr), &dev->uregs->cbal);
		__raw_writel(0x00000000, &dev->uregs->cbau);

		ctl = tsi148_eval_vam(vam);
		ctl |= 0x00000080;	/* enable */
		__raw_writel(htonl(ctl), &dev->uregs->crgat);
	}

	return result;
}

/*
 * Tundra Tsi148 configuration
 */
int do_tsi148(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr1 = 0, addr2 = 0, size = 0, vam = 0, vdw = 0;
	char cmd = 'x';

	/* get parameter */
	if (argc > 1)
		cmd = argv[1][0];
	if (argc > 2)
		addr1 = simple_strtoul(argv[2], NULL, 16);
	if (argc > 3)
		addr2 = simple_strtoul(argv[3], NULL, 16);
	if (argc > 4)
		size = simple_strtoul(argv[4], NULL, 16);
	if (argc > 5)
		vam = simple_strtoul(argv[5], NULL, 16);
	if (argc > 6)
		vdw = simple_strtoul(argv[6], NULL, 16);

	switch (cmd) {
	case 'c':
		if (strcmp(argv[1], "crg") == 0) {
			vam = addr2;
			printf("Tsi148: Configuring VME CRG Window "
			       "(VME->CRG):\n");
			printf("  vme=%08lx vam=%02lx\n", addr1, vam);
			tsi148_vme_crg_window(addr1, vam);
		} else {
			printf("Tsi148: Configuring VME CR/CSR Window "
			       "(VME->CR/CSR):\n");
			printf("  pci=%08lx\n", addr1);
			tsi148_vme_crcsr_window(addr1);
		}
		break;
	case 'i':		/* init */
		tsi148_init();
		break;
	case 'g':
		vam = addr2;
		printf("Tsi148: Configuring VME GCSR Window (VME->GCSR):\n");
		printf("  vme=%08lx vam=%02lx\n", addr1, vam);
		tsi148_vme_gcsr_window(addr1, vam);
		break;
	case 'v':		/* vme */
		printf("Tsi148: Configuring VME Slave Window (VME->PCI):\n");
		printf("  vme=%08lx pci=%08lx size=%08lx vam=%02lx\n",
		       addr1, addr2, size, vam);
		tsi148_vme_slave_window(addr1, addr2, size, vam);
		break;
	case 'p':		/* pci */
		printf("Tsi148: Configuring PCI Slave Window (PCI->VME):\n");
		printf("  pci=%08lx vme=%08lx size=%08lx vam=%02lx vdw=%02lx\n",
		       addr1, addr2, size, vam, vdw);
		tsi148_pci_slave_window(addr1, addr2, size, vam, vdw);
		break;
	default:
		printf("Tsi148: Command %s not supported!\n", argv[1]);
	}

	return 0;
}

U_BOOT_CMD(
	tsi148,	7,	1,	do_tsi148,
	"initialize and configure Turndra Tsi148\n",
	"init\n"
	"    - initialize tsi148\n"
	"tsi148 vme   [vme_addr] [pci_addr] [size] [vam]\n"
	"    - create vme slave window (access: vme->pci)\n"
	"tsi148 pci   [pci_addr] [vme_addr] [size] [vam] [vdw]\n"
	"    - create pci slave window (access: pci->vme)\n"
	"tsi148 crg   [vme_addr] [vam]\n"
	"    - create vme slave window: (access vme->CRG\n"
	"tsi148 crcsr [pci_addr]\n"
	"    - create vme slave window: (access vme->CR/CSR\n"
	"tsi148 gcsr  [vme_addr] [vam]\n"
	"    - create vme slave window: (access vme->GCSR\n"
	"    [vam] = VMEbus Address-Modifier:  01 -> A16 Address Space\n"
	"                                      02 -> A24 Address Space\n"
	"                                      03 -> A32 Address Space\n"
	"                                      04 -> Usr        AM Code\n"
	"                                      08 -> Supervisor AM Code\n"
	"                                      10 -> Data AM Code\n"
	"                                      20 -> Program AM Code\n"
	"    [vdw] = VMEbus Maximum Datawidth: 02 -> D16 Data Width\n"
	"                                      03 -> D32 Data Width\n"
);
