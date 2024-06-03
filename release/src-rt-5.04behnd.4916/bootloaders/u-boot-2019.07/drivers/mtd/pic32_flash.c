// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015
 * Cristian Birsan <cristian.birsan@microchip.com>
 * Purna Chandra Mandal <purna.mandal@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <flash.h>
#include <mach/pic32.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

/* NVM Controller registers */
struct pic32_reg_nvm {
	struct pic32_reg_atomic ctrl;
	struct pic32_reg_atomic key;
	struct pic32_reg_atomic addr;
	struct pic32_reg_atomic data;
};

/* NVM operations */
#define NVMOP_NOP		0
#define NVMOP_WORD_WRITE	1
#define NVMOP_PAGE_ERASE	4

/* NVM control bits */
#define NVM_WR			BIT(15)
#define NVM_WREN		BIT(14)
#define NVM_WRERR		BIT(13)
#define NVM_LVDERR		BIT(12)

/* NVM programming unlock register */
#define LOCK_KEY		0x0
#define UNLOCK_KEY1		0xaa996655
#define UNLOCK_KEY2		0x556699aa

/*
 * PIC32 flash banks consist of number of pages, each page
 * into number of rows and rows into number of words.
 * Here we will maintain page information instead of sector.
 */
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];
static struct pic32_reg_nvm *nvm_regs_p;

static inline void flash_initiate_operation(u32 nvmop)
{
	/* set operation */
	writel(nvmop, &nvm_regs_p->ctrl.raw);

	/* enable flash write */
	writel(NVM_WREN, &nvm_regs_p->ctrl.set);

	/* unlock sequence */
	writel(LOCK_KEY, &nvm_regs_p->key.raw);
	writel(UNLOCK_KEY1, &nvm_regs_p->key.raw);
	writel(UNLOCK_KEY2, &nvm_regs_p->key.raw);

	/* initiate operation */
	writel(NVM_WR, &nvm_regs_p->ctrl.set);
}

static int flash_wait_till_busy(const char *func, ulong timeout)
{
	int ret = wait_for_bit_le32(&nvm_regs_p->ctrl.raw,
				    NVM_WR, false, timeout, false);

	return ret ? ERR_TIMEOUT : ERR_OK;
}

static inline int flash_complete_operation(void)
{
	u32 tmp;

	tmp = readl(&nvm_regs_p->ctrl.raw);
	if (tmp & NVM_WRERR) {
		printf("Error in Block Erase - Lock Bit may be set!\n");
		flash_initiate_operation(NVMOP_NOP);
		return ERR_PROTECTED;
	}

	if (tmp & NVM_LVDERR) {
		printf("Error in Block Erase - low-vol detected!\n");
		flash_initiate_operation(NVMOP_NOP);
		return ERR_NOT_ERASED;
	}

	/* disable flash write or erase operation */
	writel(NVM_WREN, &nvm_regs_p->ctrl.clr);

	return ERR_OK;
}

/*
 * Erase flash sectors, returns:
 * ERR_OK - OK
 * ERR_INVAL - invalid sector arguments
 * ERR_TIMEOUT - write timeout
 * ERR_NOT_ERASED - Flash not erased
 * ERR_UNKNOWN_FLASH_VENDOR - incorrect flash
 */
int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	ulong sect_start, sect_end, flags;
	int prot, sect;
	int rc;

	if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_MCHP) {
		printf("Can't erase unknown flash type %08lx - aborted\n",
		       info->flash_id);
		return ERR_UNKNOWN_FLASH_VENDOR;
	}

	if ((s_first < 0) || (s_first > s_last)) {
		printf("- no sectors to erase\n");
		return ERR_INVAL;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	else
		printf("\n");

	/* erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect])
			continue;

		/* disable interrupts */
		flags = disable_interrupts();

		/* write destination page address (physical) */
		sect_start = CPHYSADDR(info->start[sect]);
		writel(sect_start, &nvm_regs_p->addr.raw);

		/* page erase */
		flash_initiate_operation(NVMOP_PAGE_ERASE);

		/* wait */
		rc = flash_wait_till_busy(__func__,
					  CONFIG_SYS_FLASH_ERASE_TOUT);

		/* re-enable interrupts if necessary */
		if (flags)
			enable_interrupts();

		if (rc != ERR_OK)
			return rc;

		rc = flash_complete_operation();
		if (rc != ERR_OK)
			return rc;

		/*
		 * flash content is updated but cache might contain stale
		 * data, so invalidate dcache.
		 */
		sect_end = info->start[sect] + info->size / info->sector_count;
		invalidate_dcache_range(info->start[sect], sect_end);
	}

	printf(" done\n");
	return ERR_OK;
}

int page_erase(flash_info_t *info, int sect)
{
	return 0;
}

/* Write a word to flash */
static int write_word(flash_info_t *info, ulong dest, ulong word)
{
	ulong flags;
	int rc;

	/* read flash to check if it is sufficiently erased */
	if ((readl((void __iomem *)dest) & word) != word) {
		printf("Error, Flash not erased!\n");
		return ERR_NOT_ERASED;
	}

	/* disable interrupts */
	flags = disable_interrupts();

	/* update destination page address (physical) */
	writel(CPHYSADDR(dest), &nvm_regs_p->addr.raw);
	writel(word, &nvm_regs_p->data.raw);

	/* word write */
	flash_initiate_operation(NVMOP_WORD_WRITE);

	/* wait for operation to complete */
	rc = flash_wait_till_busy(__func__, CONFIG_SYS_FLASH_WRITE_TOUT);

	/* re-enable interrupts if necessary */
	if (flags)
		enable_interrupts();

	if (rc != ERR_OK)
		return rc;

	return flash_complete_operation();
}

/*
 * Copy memory to flash, returns:
 * ERR_OK - OK
 * ERR_TIMEOUT - write timeout
 * ERR_NOT_ERASED - Flash not erased
 */
int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong dst, tmp_le, len = cnt;
	int i, l, rc;
	uchar *cp;

	/* get lower word aligned address */
	dst = (addr & ~3);

	/* handle unaligned start bytes */
	l = addr - dst;
	if (l != 0) {
		tmp_le = 0;
		for (i = 0, cp = (uchar *)dst; i < l; ++i, ++cp)
			tmp_le |= *cp << (i * 8);

		for (; (i < 4) && (cnt > 0); ++i, ++src, --cnt, ++cp)
			tmp_le |= *src << (i * 8);

		for (; (cnt == 0) && (i < 4); ++i, ++cp)
			tmp_le |= *cp << (i * 8);

		rc = write_word(info, dst, tmp_le);
		if (rc)
			goto out;

		dst += 4;
	}

	/* handle word aligned part */
	while (cnt >= 4) {
		tmp_le = src[0] | src[1] << 8 | src[2] << 16 | src[3] << 24;
		rc = write_word(info, dst, tmp_le);
		if (rc)
			goto out;
		src += 4;
		dst += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		rc = ERR_OK;
		goto out;
	}

	/* handle unaligned tail bytes */
	tmp_le = 0;
	for (i = 0, cp = (uchar *)dst; (i < 4) && (cnt > 0); ++i, ++cp) {
		tmp_le |= *src++ << (i * 8);
		--cnt;
	}

	for (; i < 4; ++i, ++cp)
		tmp_le |= *cp << (i * 8);

	rc = write_word(info, dst, tmp_le);
out:
	/*
	 * flash content updated by nvm controller but CPU cache might
	 * have stale data, so invalidate dcache.
	 */
	invalidate_dcache_range(addr, addr + len);

	printf(" done\n");
	return rc;
}

void flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_MCHP:
		printf("Microchip Technology ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_MCHP100T:
		printf("Internal (8 Mbit, 64 x 16k)\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		break;
	}

	printf("  Size: %ld MB in %d Sectors\n",
	       info->size >> 20, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");

		printf(" %08lX%s", info->start[i],
		       info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
}

unsigned long flash_init(void)
{
	unsigned long size = 0;
	struct udevice *dev;
	int bank;

	/* probe every MTD device */
	for (uclass_first_device(UCLASS_MTD, &dev); dev;
	     uclass_next_device(&dev)) {
		/* nop */
	}

	/* calc total flash size */
	for (bank = 0; bank < CONFIG_SYS_MAX_FLASH_BANKS; ++bank)
		size += flash_info[bank].size;

	return size;
}

static void pic32_flash_bank_init(flash_info_t *info,
				  ulong base, ulong size)
{
	ulong sect_size;
	int sect;

	/* device & manufacturer code */
	info->flash_id = FLASH_MAN_MCHP | FLASH_MCHP100T;
	info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	info->size = size;

	/* update sector (i.e page) info */
	sect_size = info->size / info->sector_count;
	for (sect = 0; sect < info->sector_count; sect++) {
		info->start[sect] = base;
		/* protect each sector by default */
		info->protect[sect] = 1;
		base += sect_size;
	}
}

static int pic32_flash_probe(struct udevice *dev)
{
	void *blob = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	const char *list, *end;
	const fdt32_t *cell;
	unsigned long addr, size;
	int parent, addrc, sizec;
	flash_info_t *info;
	int len, idx;

	/*
	 * decode regs. there are multiple reg tuples, and they need to
	 * match with reg-names.
	 */
	parent = fdt_parent_offset(blob, node);
	fdt_support_default_count_cells(blob, parent, &addrc, &sizec);
	list = fdt_getprop(blob, node, "reg-names", &len);
	if (!list)
		return -ENOENT;

	end = list + len;
	cell = fdt_getprop(blob, node, "reg", &len);
	if (!cell)
		return -ENOENT;

	for (idx = 0, info = &flash_info[0]; list < end;) {
		addr = fdt_translate_address((void *)blob, node, cell + idx);
		size = fdt_addr_to_cpu(cell[idx + addrc]);
		len = strlen(list);
		if (!strncmp(list, "nvm", len)) {
			/* NVM controller */
			nvm_regs_p = ioremap(addr, size);
		} else if (!strncmp(list, "bank", 4)) {
			/* Flash bank: use kseg0 cached address */
			pic32_flash_bank_init(info, CKSEG0ADDR(addr), size);
			info++;
		}
		idx += addrc + sizec;
		list += len + 1;
	}

	/* disable flash write/erase operations */
	writel(NVM_WREN, &nvm_regs_p->ctrl.clr);

#if (CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE)
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
		      &flash_info[0]);
#endif

#ifdef CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
		      &flash_info[0]);
#endif
	return 0;
}

static const struct udevice_id pic32_flash_ids[] = {
	{ .compatible = "microchip,pic32mzda-flash" },
	{}
};

U_BOOT_DRIVER(pic32_flash) = {
	.name	= "pic32_flash",
	.id	= UCLASS_MTD,
	.of_match = pic32_flash_ids,
	.probe	= pic32_flash_probe,
};
