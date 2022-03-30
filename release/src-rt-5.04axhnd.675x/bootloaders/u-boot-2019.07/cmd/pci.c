// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
 */

/*
 * PCI routines
 */

#include <common.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

struct pci_reg_info {
	const char *name;
	enum pci_size_t size;
	u8 offset;
};

static int pci_byte_size(enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return 1;
	case PCI_SIZE_16:
		return 2;
	case PCI_SIZE_32:
	default:
		return 4;
	}
}

static int pci_field_width(enum pci_size_t size)
{
	return pci_byte_size(size) * 2;
}

#ifdef CONFIG_DM_PCI
static void pci_show_regs(struct udevice *dev, struct pci_reg_info *regs)
{
	for (; regs->name; regs++) {
		unsigned long val;

		dm_pci_read_config(dev, regs->offset, &val, regs->size);
		printf("  %s =%*s%#.*lx\n", regs->name,
		       (int)(28 - strlen(regs->name)), "",
		       pci_field_width(regs->size), val);
	}
}
#else
static unsigned long pci_read_config(pci_dev_t dev, int offset,
				     enum pci_size_t size)
{
	u32 val32;
	u16 val16;
	u8 val8;

	switch (size) {
	case PCI_SIZE_8:
		pci_read_config_byte(dev, offset, &val8);
		return val8;
	case PCI_SIZE_16:
		pci_read_config_word(dev, offset, &val16);
		return val16;
	case PCI_SIZE_32:
	default:
		pci_read_config_dword(dev, offset, &val32);
		return val32;
	}
}

static void pci_show_regs(pci_dev_t dev, struct pci_reg_info *regs)
{
	for (; regs->name; regs++) {
		printf("  %s =%*s%#.*lx\n", regs->name,
		       (int)(28 - strlen(regs->name)), "",
		       pci_field_width(regs->size),
		       pci_read_config(dev, regs->offset, regs->size));
	}
}
#endif

#ifdef CONFIG_DM_PCI
int pci_bar_show(struct udevice *dev)
{
	u8 header_type;
	int bar_cnt, bar_id, mem_type;
	bool is_64, is_io;
	u32 base_low, base_high;
	u32 size_low, size_high;
	u64 base, size;
	u32 reg_addr;
	int prefetchable;

	dm_pci_read_config8(dev, PCI_HEADER_TYPE, &header_type);

	if (header_type == PCI_HEADER_TYPE_CARDBUS) {
		printf("CardBus doesn't support BARs\n");
		return -ENOSYS;
	}

	bar_cnt = (header_type == PCI_HEADER_TYPE_NORMAL) ? 6 : 2;

	printf("ID   Base                Size                Width  Type\n");
	printf("----------------------------------------------------------\n");

	bar_id = 0;
	reg_addr = PCI_BASE_ADDRESS_0;
	while (bar_cnt) {
		dm_pci_read_config32(dev, reg_addr, &base_low);
		dm_pci_write_config32(dev, reg_addr, 0xffffffff);
		dm_pci_read_config32(dev, reg_addr, &size_low);
		dm_pci_write_config32(dev, reg_addr, base_low);
		reg_addr += 4;

		base = base_low & ~0xf;
		size = size_low & ~0xf;
		base_high = 0x0;
		size_high = 0xffffffff;
		is_64 = 0;
		prefetchable = base_low & PCI_BASE_ADDRESS_MEM_PREFETCH;
		is_io = base_low & PCI_BASE_ADDRESS_SPACE_IO;
		mem_type = base_low & PCI_BASE_ADDRESS_MEM_TYPE_MASK;

		if (mem_type == PCI_BASE_ADDRESS_MEM_TYPE_64) {
			dm_pci_read_config32(dev, reg_addr, &base_high);
			dm_pci_write_config32(dev, reg_addr, 0xffffffff);
			dm_pci_read_config32(dev, reg_addr, &size_high);
			dm_pci_write_config32(dev, reg_addr, base_high);
			bar_cnt--;
			reg_addr += 4;
			is_64 = 1;
		}

		base = base | ((u64)base_high << 32);
		size = size | ((u64)size_high << 32);

		if ((!is_64 && size_low) || (is_64 && size)) {
			size = ~size + 1;
			printf(" %d   %#016llx  %#016llx  %d     %s   %s\n",
			       bar_id, (unsigned long long)base,
			       (unsigned long long)size, is_64 ? 64 : 32,
			       is_io ? "I/O" : "MEM",
			       prefetchable ? "Prefetchable" : "");
		}

		bar_id++;
		bar_cnt--;
	}

	return 0;
}
#endif

static struct pci_reg_info regs_start[] = {
	{ "vendor ID", PCI_SIZE_16, PCI_VENDOR_ID },
	{ "device ID", PCI_SIZE_16, PCI_DEVICE_ID },
	{ "command register ID", PCI_SIZE_16, PCI_COMMAND },
	{ "status register", PCI_SIZE_16, PCI_STATUS },
	{ "revision ID", PCI_SIZE_8, PCI_REVISION_ID },
	{},
};

static struct pci_reg_info regs_rest[] = {
	{ "sub class code", PCI_SIZE_8, PCI_CLASS_SUB_CODE },
	{ "programming interface", PCI_SIZE_8, PCI_CLASS_PROG },
	{ "cache line", PCI_SIZE_8, PCI_CACHE_LINE_SIZE },
	{ "latency time", PCI_SIZE_8, PCI_LATENCY_TIMER },
	{ "header type", PCI_SIZE_8, PCI_HEADER_TYPE },
	{ "BIST", PCI_SIZE_8, PCI_BIST },
	{ "base address 0", PCI_SIZE_32, PCI_BASE_ADDRESS_0 },
	{},
};

static struct pci_reg_info regs_normal[] = {
	{ "base address 1", PCI_SIZE_32, PCI_BASE_ADDRESS_1 },
	{ "base address 2", PCI_SIZE_32, PCI_BASE_ADDRESS_2 },
	{ "base address 3", PCI_SIZE_32, PCI_BASE_ADDRESS_3 },
	{ "base address 4", PCI_SIZE_32, PCI_BASE_ADDRESS_4 },
	{ "base address 5", PCI_SIZE_32, PCI_BASE_ADDRESS_5 },
	{ "cardBus CIS pointer", PCI_SIZE_32, PCI_CARDBUS_CIS },
	{ "sub system vendor ID", PCI_SIZE_16, PCI_SUBSYSTEM_VENDOR_ID },
	{ "sub system ID", PCI_SIZE_16, PCI_SUBSYSTEM_ID },
	{ "expansion ROM base address", PCI_SIZE_32, PCI_ROM_ADDRESS },
	{ "interrupt line", PCI_SIZE_8, PCI_INTERRUPT_LINE },
	{ "interrupt pin", PCI_SIZE_8, PCI_INTERRUPT_PIN },
	{ "min Grant", PCI_SIZE_8, PCI_MIN_GNT },
	{ "max Latency", PCI_SIZE_8, PCI_MAX_LAT },
	{},
};

static struct pci_reg_info regs_bridge[] = {
	{ "base address 1", PCI_SIZE_32, PCI_BASE_ADDRESS_1 },
	{ "primary bus number", PCI_SIZE_8, PCI_PRIMARY_BUS },
	{ "secondary bus number", PCI_SIZE_8, PCI_SECONDARY_BUS },
	{ "subordinate bus number", PCI_SIZE_8, PCI_SUBORDINATE_BUS },
	{ "secondary latency timer", PCI_SIZE_8, PCI_SEC_LATENCY_TIMER },
	{ "IO base", PCI_SIZE_8, PCI_IO_BASE },
	{ "IO limit", PCI_SIZE_8, PCI_IO_LIMIT },
	{ "secondary status", PCI_SIZE_16, PCI_SEC_STATUS },
	{ "memory base", PCI_SIZE_16, PCI_MEMORY_BASE },
	{ "memory limit", PCI_SIZE_16, PCI_MEMORY_LIMIT },
	{ "prefetch memory base", PCI_SIZE_16, PCI_PREF_MEMORY_BASE },
	{ "prefetch memory limit", PCI_SIZE_16, PCI_PREF_MEMORY_LIMIT },
	{ "prefetch memory base upper", PCI_SIZE_32, PCI_PREF_BASE_UPPER32 },
	{ "prefetch memory limit upper", PCI_SIZE_32, PCI_PREF_LIMIT_UPPER32 },
	{ "IO base upper 16 bits", PCI_SIZE_16, PCI_IO_BASE_UPPER16 },
	{ "IO limit upper 16 bits", PCI_SIZE_16, PCI_IO_LIMIT_UPPER16 },
	{ "expansion ROM base address", PCI_SIZE_32, PCI_ROM_ADDRESS1 },
	{ "interrupt line", PCI_SIZE_8, PCI_INTERRUPT_LINE },
	{ "interrupt pin", PCI_SIZE_8, PCI_INTERRUPT_PIN },
	{ "bridge control", PCI_SIZE_16, PCI_BRIDGE_CONTROL },
	{},
};

static struct pci_reg_info regs_cardbus[] = {
	{ "capabilities", PCI_SIZE_8, PCI_CB_CAPABILITY_LIST },
	{ "secondary status", PCI_SIZE_16, PCI_CB_SEC_STATUS },
	{ "primary bus number", PCI_SIZE_8, PCI_CB_PRIMARY_BUS },
	{ "CardBus number", PCI_SIZE_8, PCI_CB_CARD_BUS },
	{ "subordinate bus number", PCI_SIZE_8, PCI_CB_SUBORDINATE_BUS },
	{ "CardBus latency timer", PCI_SIZE_8, PCI_CB_LATENCY_TIMER },
	{ "CardBus memory base 0", PCI_SIZE_32, PCI_CB_MEMORY_BASE_0 },
	{ "CardBus memory limit 0", PCI_SIZE_32, PCI_CB_MEMORY_LIMIT_0 },
	{ "CardBus memory base 1", PCI_SIZE_32, PCI_CB_MEMORY_BASE_1 },
	{ "CardBus memory limit 1", PCI_SIZE_32, PCI_CB_MEMORY_LIMIT_1 },
	{ "CardBus IO base 0", PCI_SIZE_16, PCI_CB_IO_BASE_0 },
	{ "CardBus IO base high 0", PCI_SIZE_16, PCI_CB_IO_BASE_0_HI },
	{ "CardBus IO limit 0", PCI_SIZE_16, PCI_CB_IO_LIMIT_0 },
	{ "CardBus IO limit high 0", PCI_SIZE_16, PCI_CB_IO_LIMIT_0_HI },
	{ "CardBus IO base 1", PCI_SIZE_16, PCI_CB_IO_BASE_1 },
	{ "CardBus IO base high 1", PCI_SIZE_16, PCI_CB_IO_BASE_1_HI },
	{ "CardBus IO limit 1", PCI_SIZE_16, PCI_CB_IO_LIMIT_1 },
	{ "CardBus IO limit high 1", PCI_SIZE_16, PCI_CB_IO_LIMIT_1_HI },
	{ "interrupt line", PCI_SIZE_8, PCI_INTERRUPT_LINE },
	{ "interrupt pin", PCI_SIZE_8, PCI_INTERRUPT_PIN },
	{ "bridge control", PCI_SIZE_16, PCI_CB_BRIDGE_CONTROL },
	{ "subvendor ID", PCI_SIZE_16, PCI_CB_SUBSYSTEM_VENDOR_ID },
	{ "subdevice ID", PCI_SIZE_16, PCI_CB_SUBSYSTEM_ID },
	{ "PC Card 16bit base address", PCI_SIZE_32, PCI_CB_LEGACY_MODE_BASE },
	{},
};

/**
 * pci_header_show() - Show the header of the specified PCI device.
 *
 * @dev: Bus+Device+Function number
 */
#ifdef CONFIG_DM_PCI
void pci_header_show(struct udevice *dev)
#else
void pci_header_show(pci_dev_t dev)
#endif
{
#ifdef CONFIG_DM_PCI
	unsigned long class, header_type;

	dm_pci_read_config(dev, PCI_CLASS_CODE, &class, PCI_SIZE_8);
	dm_pci_read_config(dev, PCI_HEADER_TYPE, &header_type, PCI_SIZE_8);
#else
	u8 class, header_type;

	pci_read_config_byte(dev, PCI_CLASS_CODE, &class);
	pci_read_config_byte(dev, PCI_HEADER_TYPE, &header_type);
#endif
	pci_show_regs(dev, regs_start);
	printf("  class code =                  0x%.2x (%s)\n", (int)class,
	       pci_class_str(class));
	pci_show_regs(dev, regs_rest);

	switch (header_type & 0x03) {
	case PCI_HEADER_TYPE_NORMAL:	/* "normal" PCI device */
		pci_show_regs(dev, regs_normal);
		break;
	case PCI_HEADER_TYPE_BRIDGE:	/* PCI-to-PCI bridge */
		pci_show_regs(dev, regs_bridge);
		break;
	case PCI_HEADER_TYPE_CARDBUS:	/* PCI-to-CardBus bridge */
		pci_show_regs(dev, regs_cardbus);
		break;

	default:
		printf("unknown header\n");
		break;
    }
}

void pciinfo_header(int busnum, bool short_listing)
{
	printf("Scanning PCI devices on bus %d\n", busnum);

	if (short_listing) {
		printf("BusDevFun  VendorId   DeviceId   Device Class       Sub-Class\n");
		printf("_____________________________________________________________\n");
	}
}

#ifdef CONFIG_DM_PCI
/**
 * pci_header_show_brief() - Show the short-form PCI device header
 *
 * Reads and prints the header of the specified PCI device in short form.
 *
 * @dev: PCI device to show
 */
static void pci_header_show_brief(struct udevice *dev)
{
	ulong vendor, device;
	ulong class, subclass;

	dm_pci_read_config(dev, PCI_VENDOR_ID, &vendor, PCI_SIZE_16);
	dm_pci_read_config(dev, PCI_DEVICE_ID, &device, PCI_SIZE_16);
	dm_pci_read_config(dev, PCI_CLASS_CODE, &class, PCI_SIZE_8);
	dm_pci_read_config(dev, PCI_CLASS_SUB_CODE, &subclass, PCI_SIZE_8);

	printf("0x%.4lx     0x%.4lx     %-23s 0x%.2lx\n",
	       vendor, device,
	       pci_class_str(class), subclass);
}

static void pciinfo(struct udevice *bus, bool short_listing)
{
	struct udevice *dev;

	pciinfo_header(bus->seq, short_listing);

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		struct pci_child_platdata *pplat;

		pplat = dev_get_parent_platdata(dev);
		if (short_listing) {
			printf("%02x.%02x.%02x   ", bus->seq,
			       PCI_DEV(pplat->devfn), PCI_FUNC(pplat->devfn));
			pci_header_show_brief(dev);
		} else {
			printf("\nFound PCI device %02x.%02x.%02x:\n", bus->seq,
			       PCI_DEV(pplat->devfn), PCI_FUNC(pplat->devfn));
			pci_header_show(dev);
		}
	}
}

#else

/**
 * pci_header_show_brief() - Show the short-form PCI device header
 *
 * Reads and prints the header of the specified PCI device in short form.
 *
 * @dev: Bus+Device+Function number
 */
void pci_header_show_brief(pci_dev_t dev)
{
	u16 vendor, device;
	u8 class, subclass;

	pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
	pci_read_config_word(dev, PCI_DEVICE_ID, &device);
	pci_read_config_byte(dev, PCI_CLASS_CODE, &class);
	pci_read_config_byte(dev, PCI_CLASS_SUB_CODE, &subclass);

	printf("0x%.4x     0x%.4x     %-23s 0x%.2x\n",
	       vendor, device,
	       pci_class_str(class), subclass);
}

/**
 * pciinfo() - Show a list of devices on the PCI bus
 *
 * Show information about devices on PCI bus. Depending on @short_pci_listing
 * the output will be more or less exhaustive.
 *
 * @bus_num: The number of the bus to be scanned
 * @short_pci_listing: true to use short form, showing only a brief header
 * for each device
 */
void pciinfo(int bus_num, int short_pci_listing)
{
	struct pci_controller *hose = pci_bus_to_hose(bus_num);
	int device;
	int function;
	unsigned char header_type;
	unsigned short vendor_id;
	pci_dev_t dev;
	int ret;

	if (!hose)
		return;

	pciinfo_header(bus_num, short_pci_listing);

	for (device = 0; device < PCI_MAX_PCI_DEVICES; device++) {
		header_type = 0;
		vendor_id = 0;
		for (function = 0; function < PCI_MAX_PCI_FUNCTIONS;
		     function++) {
			/*
			 * If this is not a multi-function device, we skip
			 * the rest.
			 */
			if (function && !(header_type & 0x80))
				break;

			dev = PCI_BDF(bus_num, device, function);

			if (pci_skip_dev(hose, dev))
				continue;

			ret = pci_read_config_word(dev, PCI_VENDOR_ID,
						   &vendor_id);
			if (ret)
				goto error;
			if ((vendor_id == 0xFFFF) || (vendor_id == 0x0000))
				continue;

			if (!function) {
				pci_read_config_byte(dev, PCI_HEADER_TYPE,
						     &header_type);
			}

			if (short_pci_listing) {
				printf("%02x.%02x.%02x   ", bus_num, device,
				       function);
				pci_header_show_brief(dev);
			} else {
				printf("\nFound PCI device %02x.%02x.%02x:\n",
				       bus_num, device, function);
				pci_header_show(dev);
			}
		}
	}

	return;
error:
	printf("Cannot read bus configuration: %d\n", ret);
}
#endif

/**
 * get_pci_dev() - Convert the "bus.device.function" identifier into a number
 *
 * @name: Device string in the form "bus.device.function" where each is in hex
 * @return encoded pci_dev_t or -1 if the string was invalid
 */
static pci_dev_t get_pci_dev(char *name)
{
	char cnum[12];
	int len, i, iold, n;
	int bdfs[3] = {0,0,0};

	len = strlen(name);
	if (len > 8)
		return -1;
	for (i = 0, iold = 0, n = 0; i < len; i++) {
		if (name[i] == '.') {
			memcpy(cnum, &name[iold], i - iold);
			cnum[i - iold] = '\0';
			bdfs[n++] = simple_strtoul(cnum, NULL, 16);
			iold = i + 1;
		}
	}
	strcpy(cnum, &name[iold]);
	if (n == 0)
		n = 1;
	bdfs[n] = simple_strtoul(cnum, NULL, 16);

	return PCI_BDF(bdfs[0], bdfs[1], bdfs[2]);
}

#ifdef CONFIG_DM_PCI
static int pci_cfg_display(struct udevice *dev, ulong addr,
			   enum pci_size_t size, ulong length)
#else
static int pci_cfg_display(pci_dev_t bdf, ulong addr, enum pci_size_t size,
			   ulong length)
#endif
{
#define DISP_LINE_LEN	16
	ulong i, nbytes, linebytes;
	int byte_size;
	int rc = 0;

	byte_size = pci_byte_size(size);
	if (length == 0)
		length = 0x40 / byte_size; /* Standard PCI config space */

	/* Print the lines.
	 * once, and all accesses are with the specified bus width.
	 */
	nbytes = length * byte_size;
	do {
		printf("%08lx:", addr);
		linebytes = (nbytes > DISP_LINE_LEN) ? DISP_LINE_LEN : nbytes;
		for (i = 0; i < linebytes; i += byte_size) {
			unsigned long val;

#ifdef CONFIG_DM_PCI
			dm_pci_read_config(dev, addr, &val, size);
#else
			val = pci_read_config(bdf, addr, size);
#endif
			printf(" %0*lx", pci_field_width(size), val);
			addr += byte_size;
		}
		printf("\n");
		nbytes -= linebytes;
		if (ctrlc()) {
			rc = 1;
			break;
		}
	} while (nbytes > 0);

	return (rc);
}

#ifndef CONFIG_DM_PCI
static int pci_cfg_write (pci_dev_t bdf, ulong addr, ulong size, ulong value)
{
	if (size == 4) {
		pci_write_config_dword(bdf, addr, value);
	}
	else if (size == 2) {
		ushort val = value & 0xffff;
		pci_write_config_word(bdf, addr, val);
	}
	else {
		u_char val = value & 0xff;
		pci_write_config_byte(bdf, addr, val);
	}
	return 0;
}
#endif

#ifdef CONFIG_DM_PCI
static int pci_cfg_modify(struct udevice *dev, ulong addr, ulong size,
			  ulong value, int incrflag)
#else
static int pci_cfg_modify(pci_dev_t bdf, ulong addr, ulong size, ulong value,
			  int incrflag)
#endif
{
	ulong	i;
	int	nbytes;
	ulong val;

	/* Print the address, followed by value.  Then accept input for
	 * the next value.  A non-converted value exits.
	 */
	do {
		printf("%08lx:", addr);
#ifdef CONFIG_DM_PCI
		dm_pci_read_config(dev, addr, &val, size);
#else
		val = pci_read_config(bdf, addr, size);
#endif
		printf(" %0*lx", pci_field_width(size), val);

		nbytes = cli_readline(" ? ");
		if (nbytes == 0 || (nbytes == 1 && console_buffer[0] == '-')) {
			/* <CR> pressed as only input, don't modify current
			 * location and move to next. "-" pressed will go back.
			 */
			if (incrflag)
				addr += nbytes ? -size : size;
			nbytes = 1;
			/* good enough to not time out */
			bootretry_reset_cmd_timeout();
		}
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (nbytes == -2) {
			break;	/* timed out, exit the command	*/
		}
#endif
		else {
			char *endp;
			i = simple_strtoul(console_buffer, &endp, 16);
			nbytes = endp - console_buffer;
			if (nbytes) {
				/* good enough to not time out
				 */
				bootretry_reset_cmd_timeout();
#ifdef CONFIG_DM_PCI
				dm_pci_write_config(dev, addr, i, size);
#else
				pci_cfg_write(bdf, addr, size, i);
#endif
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	return 0;
}

#ifdef CONFIG_DM_PCI
static const struct pci_flag_info {
	uint flag;
	const char *name;
} pci_flag_info[] = {
	{ PCI_REGION_IO, "io" },
	{ PCI_REGION_PREFETCH, "prefetch" },
	{ PCI_REGION_SYS_MEMORY, "sysmem" },
	{ PCI_REGION_RO, "readonly" },
	{ PCI_REGION_IO, "io" },
};

static void pci_show_regions(struct udevice *bus)
{
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	const struct pci_region *reg;
	int i, j;

	if (!hose) {
		printf("Bus '%s' is not a PCI controller\n", bus->name);
		return;
	}

	printf("#   %-16s %-16s %-16s  %s\n", "Bus start", "Phys start", "Size",
	       "Flags");
	for (i = 0, reg = hose->regions; i < hose->region_count; i++, reg++) {
		printf("%d   %#016llx %#016llx %#016llx  ", i,
		       (unsigned long long)reg->bus_start,
		       (unsigned long long)reg->phys_start,
		       (unsigned long long)reg->size);
		if (!(reg->flags & PCI_REGION_TYPE))
			printf("mem ");
		for (j = 0; j < ARRAY_SIZE(pci_flag_info); j++) {
			if (reg->flags & pci_flag_info[j].flag)
				printf("%s ", pci_flag_info[j].name);
		}
		printf("\n");
	}
}
#endif

/* PCI Configuration Space access commands
 *
 * Syntax:
 *	pci display[.b, .w, .l] bus.device.function} [addr] [len]
 *	pci next[.b, .w, .l] bus.device.function [addr]
 *      pci modify[.b, .w, .l] bus.device.function [addr]
 *      pci write[.b, .w, .l] bus.device.function addr value
 */
static int do_pci(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr = 0, value = 0, cmd_size = 0;
	enum pci_size_t size = PCI_SIZE_32;
#ifdef CONFIG_DM_PCI
	struct udevice *dev, *bus;
#else
	pci_dev_t dev;
#endif
	int busnum = 0;
	pci_dev_t bdf = 0;
	char cmd = 's';
	int ret = 0;

	if (argc > 1)
		cmd = argv[1][0];

	switch (cmd) {
	case 'd':		/* display */
	case 'n':		/* next */
	case 'm':		/* modify */
	case 'w':		/* write */
		/* Check for a size specification. */
		cmd_size = cmd_get_data_size(argv[1], 4);
		size = (cmd_size == 4) ? PCI_SIZE_32 : cmd_size - 1;
		if (argc > 3)
			addr = simple_strtoul(argv[3], NULL, 16);
		if (argc > 4)
			value = simple_strtoul(argv[4], NULL, 16);
	case 'h':		/* header */
#ifdef CONFIG_DM_PCI
	case 'b':		/* bars */
#endif
		if (argc < 3)
			goto usage;
		if ((bdf = get_pci_dev(argv[2])) == -1)
			return 1;
		break;
#if defined(CONFIG_DM_PCI)
	case 'e':
		pci_init();
		return 0;
#endif
	case 'r': /* no break */
	default:		/* scan bus */
		value = 1; /* short listing */
		if (argc > 1) {
			if (cmd != 'r' && argv[argc-1][0] == 'l') {
				value = 0;
				argc--;
			}
			if (argc > 1)
				busnum = simple_strtoul(argv[1], NULL, 16);
		}
#ifdef CONFIG_DM_PCI
		ret = uclass_get_device_by_seq(UCLASS_PCI, busnum, &bus);
		if (ret) {
			printf("No such bus\n");
			return CMD_RET_FAILURE;
		}
		if (cmd == 'r')
			pci_show_regions(bus);
		else
			pciinfo(bus, value);
#else
		pciinfo(busnum, value);
#endif
		return 0;
	}

#ifdef CONFIG_DM_PCI
	ret = dm_pci_bus_find_bdf(bdf, &dev);
	if (ret) {
		printf("No such device\n");
		return CMD_RET_FAILURE;
	}
#else
	dev = bdf;
#endif

	switch (argv[1][0]) {
	case 'h':		/* header */
		pci_header_show(dev);
		break;
	case 'd':		/* display */
		return pci_cfg_display(dev, addr, size, value);
	case 'n':		/* next */
		if (argc < 4)
			goto usage;
		ret = pci_cfg_modify(dev, addr, size, value, 0);
		break;
	case 'm':		/* modify */
		if (argc < 4)
			goto usage;
		ret = pci_cfg_modify(dev, addr, size, value, 1);
		break;
	case 'w':		/* write */
		if (argc < 5)
			goto usage;
#ifdef CONFIG_DM_PCI
		ret = dm_pci_write_config(dev, addr, value, size);
#else
		ret = pci_cfg_write(dev, addr, size, value);
#endif
		break;
#ifdef CONFIG_DM_PCI

	case 'b':		/* bars */
		return pci_bar_show(dev);
#endif
	default:
		ret = CMD_RET_USAGE;
		break;
	}

	return ret;
 usage:
	return CMD_RET_USAGE;
}

/***************************************************/

#ifdef CONFIG_SYS_LONGHELP
static char pci_help_text[] =
	"[bus] [long]\n"
	"    - short or long list of PCI devices on bus 'bus'\n"
#if defined(CONFIG_DM_PCI)
	"pci enum\n"
	"    - Enumerate PCI buses\n"
#endif
	"pci header b.d.f\n"
	"    - show header of PCI device 'bus.device.function'\n"
#ifdef CONFIG_DM_PCI
	"pci bar b.d.f\n"
	"    - show BARs base and size for device b.d.f'\n"
	"pci regions\n"
	"    - show PCI regions\n"
#endif
	"pci display[.b, .w, .l] b.d.f [address] [# of objects]\n"
	"    - display PCI configuration space (CFG)\n"
	"pci next[.b, .w, .l] b.d.f address\n"
	"    - modify, read and keep CFG address\n"
	"pci modify[.b, .w, .l] b.d.f address\n"
	"    -  modify, auto increment CFG address\n"
	"pci write[.b, .w, .l] b.d.f address value\n"
	"    - write to CFG address";
#endif

U_BOOT_CMD(
	pci,	5,	1,	do_pci,
	"list and access PCI Configuration Space", pci_help_text
);
