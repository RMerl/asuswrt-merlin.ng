// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Old PCI routines
 *
 * Do not change this file. Instead, convert your board to use CONFIG_DM_PCI
 * and change pci-uclass.c.
 */

#include <common.h>

#include <command.h>
#include <errno.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

DECLARE_GLOBAL_DATA_PTR;

#define PCI_HOSE_OP(rw, size, type)					\
int pci_hose_##rw##_config_##size(struct pci_controller *hose,		\
				  pci_dev_t dev,			\
				  int offset, type value)		\
{									\
	return hose->rw##_##size(hose, dev, offset, value);		\
}

PCI_HOSE_OP(read, byte, u8 *)
PCI_HOSE_OP(read, word, u16 *)
PCI_HOSE_OP(read, dword, u32 *)
PCI_HOSE_OP(write, byte, u8)
PCI_HOSE_OP(write, word, u16)
PCI_HOSE_OP(write, dword, u32)

#define PCI_OP(rw, size, type, error_code)				\
int pci_##rw##_config_##size(pci_dev_t dev, int offset, type value)	\
{									\
	struct pci_controller *hose = pci_bus_to_hose(PCI_BUS(dev));	\
									\
	if (!hose)							\
	{								\
		error_code;						\
		return -1;						\
	}								\
									\
	return pci_hose_##rw##_config_##size(hose, dev, offset, value);	\
}

PCI_OP(read, byte, u8 *, *value = 0xff)
PCI_OP(read, word, u16 *, *value = 0xffff)
PCI_OP(read, dword, u32 *, *value = 0xffffffff)
PCI_OP(write, byte, u8, )
PCI_OP(write, word, u16, )
PCI_OP(write, dword, u32, )

#define PCI_READ_VIA_DWORD_OP(size, type, off_mask)			\
int pci_hose_read_config_##size##_via_dword(struct pci_controller *hose,\
					pci_dev_t dev,			\
					int offset, type val)		\
{									\
	u32 val32;							\
									\
	if (pci_hose_read_config_dword(hose, dev, offset & 0xfc, &val32) < 0) {	\
		*val = -1;						\
		return -1;						\
	}								\
									\
	*val = (val32 >> ((offset & (int)off_mask) * 8));		\
									\
	return 0;							\
}

#define PCI_WRITE_VIA_DWORD_OP(size, type, off_mask, val_mask)		\
int pci_hose_write_config_##size##_via_dword(struct pci_controller *hose,\
					     pci_dev_t dev,		\
					     int offset, type val)	\
{									\
	u32 val32, mask, ldata, shift;					\
									\
	if (pci_hose_read_config_dword(hose, dev, offset & 0xfc, &val32) < 0)\
		return -1;						\
									\
	shift = ((offset & (int)off_mask) * 8);				\
	ldata = (((unsigned long)val) & val_mask) << shift;		\
	mask = val_mask << shift;					\
	val32 = (val32 & ~mask) | ldata;				\
									\
	if (pci_hose_write_config_dword(hose, dev, offset & 0xfc, val32) < 0)\
		return -1;						\
									\
	return 0;							\
}

PCI_READ_VIA_DWORD_OP(byte, u8 *, 0x03)
PCI_READ_VIA_DWORD_OP(word, u16 *, 0x02)
PCI_WRITE_VIA_DWORD_OP(byte, u8, 0x03, 0x000000ff)
PCI_WRITE_VIA_DWORD_OP(word, u16, 0x02, 0x0000ffff)

/*
 *
 */

static struct pci_controller* hose_head;

struct pci_controller *pci_get_hose_head(void)
{
	if (gd->hose)
		return gd->hose;

	return hose_head;
}

void pci_register_hose(struct pci_controller* hose)
{
	struct pci_controller **phose = &hose_head;

	while(*phose)
		phose = &(*phose)->next;

	hose->next = NULL;

	*phose = hose;
}

struct pci_controller *pci_bus_to_hose(int bus)
{
	struct pci_controller *hose;

	for (hose = pci_get_hose_head(); hose; hose = hose->next) {
		if (bus >= hose->first_busno && bus <= hose->last_busno)
			return hose;
	}

	printf("pci_bus_to_hose() failed\n");
	return NULL;
}

struct pci_controller *find_hose_by_cfg_addr(void *cfg_addr)
{
	struct pci_controller *hose;

	for (hose = pci_get_hose_head(); hose; hose = hose->next) {
		if (hose->cfg_addr == cfg_addr)
			return hose;
	}

	return NULL;
}

int pci_last_busno(void)
{
	struct pci_controller *hose = pci_get_hose_head();

	if (!hose)
		return -1;

	while (hose->next)
		hose = hose->next;

	return hose->last_busno;
}

pci_dev_t pci_find_devices(struct pci_device_id *ids, int index)
{
	struct pci_controller * hose;
	pci_dev_t bdf;
	int bus;

	for (hose = pci_get_hose_head(); hose; hose = hose->next) {
		for (bus = hose->first_busno; bus <= hose->last_busno; bus++) {
			bdf = pci_hose_find_devices(hose, bus, ids, &index);
			if (bdf != -1)
				return bdf;
		}
	}

	return -1;
}

static int pci_hose_config_device(struct pci_controller *hose, pci_dev_t dev,
				  ulong io, pci_addr_t mem, ulong command)
{
	u32 bar_response;
	unsigned int old_command;
	pci_addr_t bar_value;
	pci_size_t bar_size;
	unsigned char pin;
	int bar, found_mem64;

	debug("PCI Config: I/O=0x%lx, Memory=0x%llx, Command=0x%lx\n", io,
		(u64)mem, command);

	pci_hose_write_config_dword(hose, dev, PCI_COMMAND, 0);

	for (bar = PCI_BASE_ADDRESS_0; bar <= PCI_BASE_ADDRESS_5; bar += 4) {
		pci_hose_write_config_dword(hose, dev, bar, 0xffffffff);
		pci_hose_read_config_dword(hose, dev, bar, &bar_response);

		if (!bar_response)
			continue;

		found_mem64 = 0;

		/* Check the BAR type and set our address mask */
		if (bar_response & PCI_BASE_ADDRESS_SPACE) {
			bar_size = ~(bar_response & PCI_BASE_ADDRESS_IO_MASK) + 1;
			/* round up region base address to a multiple of size */
			io = ((io - 1) | (bar_size - 1)) + 1;
			bar_value = io;
			/* compute new region base address */
			io = io + bar_size;
		} else {
			if ((bar_response & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
				PCI_BASE_ADDRESS_MEM_TYPE_64) {
				u32 bar_response_upper;
				u64 bar64;
				pci_hose_write_config_dword(hose, dev, bar + 4,
					0xffffffff);
				pci_hose_read_config_dword(hose, dev, bar + 4,
					&bar_response_upper);

				bar64 = ((u64)bar_response_upper << 32) | bar_response;

				bar_size = ~(bar64 & PCI_BASE_ADDRESS_MEM_MASK) + 1;
				found_mem64 = 1;
			} else {
				bar_size = (u32)(~(bar_response & PCI_BASE_ADDRESS_MEM_MASK) + 1);
			}

			/* round up region base address to multiple of size */
			mem = ((mem - 1) | (bar_size - 1)) + 1;
			bar_value = mem;
			/* compute new region base address */
			mem = mem + bar_size;
		}

		/* Write it out and update our limit */
		pci_hose_write_config_dword (hose, dev, bar, (u32)bar_value);

		if (found_mem64) {
			bar += 4;
#ifdef CONFIG_SYS_PCI_64BIT
			pci_hose_write_config_dword(hose, dev, bar,
				(u32)(bar_value >> 32));
#else
			pci_hose_write_config_dword(hose, dev, bar, 0x00000000);
#endif
		}
	}

	/* Configure Cache Line Size Register */
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

	/* Configure Latency Timer */
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);

	/* Disable interrupt line, if device says it wants to use interrupts */
	pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_PIN, &pin);
	if (pin != 0) {
		pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE,
					   PCI_INTERRUPT_LINE_DISABLE);
	}

	pci_hose_read_config_dword(hose, dev, PCI_COMMAND, &old_command);
	pci_hose_write_config_dword(hose, dev, PCI_COMMAND,
				     (old_command & 0xffff0000) | command);

	return 0;
}

/*
 *
 */

struct pci_config_table *pci_find_config(struct pci_controller *hose,
					 unsigned short class,
					 unsigned int vendor,
					 unsigned int device,
					 unsigned int bus,
					 unsigned int dev,
					 unsigned int func)
{
	struct pci_config_table *table;

	for (table = hose->config_table; table && table->vendor; table++) {
		if ((table->vendor == PCI_ANY_ID || table->vendor == vendor) &&
		    (table->device == PCI_ANY_ID || table->device == device) &&
		    (table->class  == PCI_ANY_ID || table->class  == class)  &&
		    (table->bus    == PCI_ANY_ID || table->bus    == bus)    &&
		    (table->dev    == PCI_ANY_ID || table->dev    == dev)    &&
		    (table->func   == PCI_ANY_ID || table->func   == func)) {
			return table;
		}
	}

	return NULL;
}

void pci_cfgfunc_config_device(struct pci_controller *hose,
			       pci_dev_t dev,
			       struct pci_config_table *entry)
{
	pci_hose_config_device(hose, dev, entry->priv[0], entry->priv[1],
		entry->priv[2]);
}

void pci_cfgfunc_do_nothing(struct pci_controller *hose,
			    pci_dev_t dev, struct pci_config_table *entry)
{
}

/*
 * HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
extern int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev);

#ifdef CONFIG_PCI_SCAN_SHOW
__weak int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	if (dev == PCI_BDF(hose->first_busno, 0, 0))
		return 0;

	return 1;
}
#endif /* CONFIG_PCI_SCAN_SHOW */

int pci_hose_scan_bus(struct pci_controller *hose, int bus)
{
	unsigned int sub_bus, found_multi = 0;
	unsigned short vendor, device, class;
	unsigned char header_type;
#ifndef CONFIG_PCI_PNP
	struct pci_config_table *cfg;
#endif
	pci_dev_t dev;
#ifdef CONFIG_PCI_SCAN_SHOW
	static int indent = 0;
#endif

	sub_bus = bus;

	for (dev =  PCI_BDF(bus,0,0);
	     dev <  PCI_BDF(bus, PCI_MAX_PCI_DEVICES - 1,
				PCI_MAX_PCI_FUNCTIONS - 1);
	     dev += PCI_BDF(0, 0, 1)) {

		if (pci_skip_dev(hose, dev))
			continue;

		if (PCI_FUNC(dev) && !found_multi)
			continue;

		pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &header_type);

		pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

		if (vendor == 0xffff || vendor == 0x0000)
			continue;

		if (!PCI_FUNC(dev))
			found_multi = header_type & 0x80;

		debug("PCI Scan: Found Bus %d, Device %d, Function %d\n",
			PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));

		pci_hose_read_config_word(hose, dev, PCI_DEVICE_ID, &device);
		pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);

#ifdef CONFIG_PCI_FIXUP_DEV
		board_pci_fixup_dev(hose, dev, vendor, device, class);
#endif

#ifdef CONFIG_PCI_SCAN_SHOW
		indent++;

		/* Print leading space, including bus indentation */
		printf("%*c", indent + 1, ' ');

		if (pci_print_dev(hose, dev)) {
			printf("%02x:%02x.%-*x - %04x:%04x - %s\n",
			       PCI_BUS(dev), PCI_DEV(dev), 6 - indent, PCI_FUNC(dev),
			       vendor, device, pci_class_str(class >> 8));
		}
#endif

#ifdef CONFIG_PCI_PNP
		sub_bus = max((unsigned int)pciauto_config_device(hose, dev),
			      sub_bus);
#else
		cfg = pci_find_config(hose, class, vendor, device,
				      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
		if (cfg) {
			cfg->config_device(hose, dev, cfg);
			sub_bus = max(sub_bus,
				      (unsigned int)hose->current_busno);
		}
#endif

#ifdef CONFIG_PCI_SCAN_SHOW
		indent--;
#endif

		if (hose->fixup_irq)
			hose->fixup_irq(hose, dev);
	}

	return sub_bus;
}

int pci_hose_scan(struct pci_controller *hose)
{
#if defined(CONFIG_PCI_BOOTDELAY)
	char *s;
	int i;

	if (!gd->pcidelay_done) {
		/* wait "pcidelay" ms (if defined)... */
		s = env_get("pcidelay");
		if (s) {
			int val = simple_strtoul(s, NULL, 10);
			for (i = 0; i < val; i++)
				udelay(1000);
		}
		gd->pcidelay_done = 1;
	}
#endif /* CONFIG_PCI_BOOTDELAY */

#ifdef CONFIG_PCI_SCAN_SHOW
	puts("PCI:\n");
#endif

	/*
	 * Start scan at current_busno.
	 * PCIe will start scan at first_busno+1.
	 */
	/* For legacy support, ensure current >= first */
	if (hose->first_busno > hose->current_busno)
		hose->current_busno = hose->first_busno;
#ifdef CONFIG_PCI_PNP
	pciauto_config_init(hose);
#endif
	return pci_hose_scan_bus(hose, hose->current_busno);
}

void pci_init(void)
{
	hose_head = NULL;

	/* allow env to disable pci init/enum */
	if (env_get("pcidisable") != NULL)
		return;

	/* now call board specific pci_init()... */
	pci_init_board();
}

/* Returns the address of the requested capability structure within the
 * device's PCI configuration space or 0 in case the device does not
 * support it.
 * */
int pci_hose_find_capability(struct pci_controller *hose, pci_dev_t dev,
			     int cap)
{
	int pos;
	u8 hdr_type;

	pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &hdr_type);

	pos = pci_hose_find_cap_start(hose, dev, hdr_type & 0x7F);

	if (pos)
		pos = pci_find_cap(hose, dev, pos, cap);

	return pos;
}

/* Find the header pointer to the Capabilities*/
int pci_hose_find_cap_start(struct pci_controller *hose, pci_dev_t dev,
			    u8 hdr_type)
{
	u16 status;

	pci_hose_read_config_word(hose, dev, PCI_STATUS, &status);

	if (!(status & PCI_STATUS_CAP_LIST))
		return 0;

	switch (hdr_type) {
	case PCI_HEADER_TYPE_NORMAL:
	case PCI_HEADER_TYPE_BRIDGE:
		return PCI_CAPABILITY_LIST;
	case PCI_HEADER_TYPE_CARDBUS:
		return PCI_CB_CAPABILITY_LIST;
	default:
		return 0;
	}
}

int pci_find_cap(struct pci_controller *hose, pci_dev_t dev, int pos, int cap)
{
	int ttl = PCI_FIND_CAP_TTL;
	u8 id;
	u8 next_pos;

	while (ttl--) {
		pci_hose_read_config_byte(hose, dev, pos, &next_pos);
		if (next_pos < CAP_START_POS)
			break;
		next_pos &= ~3;
		pos = (int) next_pos;
		pci_hose_read_config_byte(hose, dev,
					  pos + PCI_CAP_LIST_ID, &id);
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos += PCI_CAP_LIST_NEXT;
	}
	return 0;
}

/**
 * pci_find_next_ext_capability - Find an extended capability
 *
 * Returns the address of the next matching extended capability structure
 * within the device's PCI configuration space or 0 if the device does
 * not support it.  Some capabilities can occur several times, e.g., the
 * vendor-specific capability, and this provides a way to find them all.
 */
int pci_find_next_ext_capability(struct pci_controller *hose, pci_dev_t dev,
				 int start, int cap)
{
	u32 header;
	int ttl, pos = PCI_CFG_SPACE_SIZE;

	/* minimum 8 bytes per capability */
	ttl = (PCI_CFG_SPACE_EXP_SIZE - PCI_CFG_SPACE_SIZE) / 8;

	if (start)
		pos = start;

	pci_hose_read_config_dword(hose, dev, pos, &header);
	if (header == 0xffffffff || header == 0)
		return 0;

	while (ttl-- > 0) {
		if (PCI_EXT_CAP_ID(header) == cap && pos != start)
			return pos;

		pos = PCI_EXT_CAP_NEXT(header);
		if (pos < PCI_CFG_SPACE_SIZE)
			break;

		pci_hose_read_config_dword(hose, dev, pos, &header);
		if (header == 0xffffffff || header == 0)
			break;
	}

	return 0;
}

/**
 * pci_hose_find_ext_capability - Find an extended capability
 *
 * Returns the address of the requested extended capability structure
 * within the device's PCI configuration space or 0 if the device does
 * not support it.
 */
int pci_hose_find_ext_capability(struct pci_controller *hose, pci_dev_t dev,
				 int cap)
{
	return pci_find_next_ext_capability(hose, dev, 0, cap);
}
