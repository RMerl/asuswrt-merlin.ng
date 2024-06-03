// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Microelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <flash.h>
#include <linux/err.h>
#include <linux/mtd/st_smi.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>

#if defined(CONFIG_MTD_NOR_FLASH)

static struct smi_regs *const smicntl =
    (struct smi_regs * const)CONFIG_SYS_SMI_BASE;
static ulong bank_base[CONFIG_SYS_MAX_FLASH_BANKS] =
    CONFIG_SYS_FLASH_ADDR_BASE;
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

/* data structure to maintain flash ids from different vendors */
struct flash_device {
	char *name;
	u8 erase_cmd;
	u32 device_id;
	u32 pagesize;
	unsigned long sectorsize;
	unsigned long size_in_bytes;
};

#define FLASH_ID(n, es, id, psize, ssize, size)	\
{				\
	.name = n,		\
	.erase_cmd = es,	\
	.device_id = id,	\
	.pagesize = psize,	\
	.sectorsize = ssize,	\
	.size_in_bytes = size	\
}

/*
 * List of supported flash devices.
 * Currently the erase_cmd field is not used in this driver.
 */
static struct flash_device flash_devices[] = {
	FLASH_ID("st m25p16"     , 0xd8, 0x00152020, 0x100, 0x10000, 0x200000),
	FLASH_ID("st m25p32"     , 0xd8, 0x00162020, 0x100, 0x10000, 0x400000),
	FLASH_ID("st m25p64"     , 0xd8, 0x00172020, 0x100, 0x10000, 0x800000),
	FLASH_ID("st m25p128"    , 0xd8, 0x00182020, 0x100, 0x40000, 0x1000000),
	FLASH_ID("st m25p05"     , 0xd8, 0x00102020, 0x80 , 0x8000 , 0x10000),
	FLASH_ID("st m25p10"     , 0xd8, 0x00112020, 0x80 , 0x8000 , 0x20000),
	FLASH_ID("st m25p20"     , 0xd8, 0x00122020, 0x100, 0x10000, 0x40000),
	FLASH_ID("st m25p40"     , 0xd8, 0x00132020, 0x100, 0x10000, 0x80000),
	FLASH_ID("st m25p80"     , 0xd8, 0x00142020, 0x100, 0x10000, 0x100000),
	FLASH_ID("st m45pe10"    , 0xd8, 0x00114020, 0x100, 0x10000, 0x20000),
	FLASH_ID("st m45pe20"    , 0xd8, 0x00124020, 0x100, 0x10000, 0x40000),
	FLASH_ID("st m45pe40"    , 0xd8, 0x00134020, 0x100, 0x10000, 0x80000),
	FLASH_ID("st m45pe80"    , 0xd8, 0x00144020, 0x100, 0x10000, 0x100000),
	FLASH_ID("sp s25fl004"   , 0xd8, 0x00120201, 0x100, 0x10000, 0x80000),
	FLASH_ID("sp s25fl008"   , 0xd8, 0x00130201, 0x100, 0x10000, 0x100000),
	FLASH_ID("sp s25fl016"   , 0xd8, 0x00140201, 0x100, 0x10000, 0x200000),
	FLASH_ID("sp s25fl032"   , 0xd8, 0x00150201, 0x100, 0x10000, 0x400000),
	FLASH_ID("sp s25fl064"   , 0xd8, 0x00160201, 0x100, 0x10000, 0x800000),
	FLASH_ID("mac 25l512"    , 0xd8, 0x001020C2, 0x010, 0x10000, 0x10000),
	FLASH_ID("mac 25l1005"   , 0xd8, 0x001120C2, 0x010, 0x10000, 0x20000),
	FLASH_ID("mac 25l2005"   , 0xd8, 0x001220C2, 0x010, 0x10000, 0x40000),
	FLASH_ID("mac 25l4005"   , 0xd8, 0x001320C2, 0x010, 0x10000, 0x80000),
	FLASH_ID("mac 25l4005a"  , 0xd8, 0x001320C2, 0x010, 0x10000, 0x80000),
	FLASH_ID("mac 25l8005"   , 0xd8, 0x001420C2, 0x010, 0x10000, 0x100000),
	FLASH_ID("mac 25l1605"   , 0xd8, 0x001520C2, 0x100, 0x10000, 0x200000),
	FLASH_ID("mac 25l1605a"  , 0xd8, 0x001520C2, 0x010, 0x10000, 0x200000),
	FLASH_ID("mac 25l3205"   , 0xd8, 0x001620C2, 0x100, 0x10000, 0x400000),
	FLASH_ID("mac 25l3205a"  , 0xd8, 0x001620C2, 0x100, 0x10000, 0x400000),
	FLASH_ID("mac 25l6405"   , 0xd8, 0x001720C2, 0x100, 0x10000, 0x800000),
	FLASH_ID("wbd w25q128" , 0xd8, 0x001840EF, 0x100, 0x10000, 0x1000000),
};

/*
 * smi_wait_xfer_finish - Wait until TFF is set in status register
 * @timeout:	 timeout in milliseconds
 *
 * Wait until TFF is set in status register
 */
static int smi_wait_xfer_finish(int timeout)
{
	ulong start = get_timer(0);

	while (get_timer(start) < timeout) {
		if (readl(&smicntl->smi_sr) & TFF)
			return 0;

		/* Try after 10 ms */
		udelay(10);
	};

	return -1;
}

/*
 * smi_read_id - Read flash id
 * @info:	 flash_info structure pointer
 * @banknum:	 bank number
 *
 * Read the flash id present at bank #banknum
 */
static unsigned int smi_read_id(flash_info_t *info, int banknum)
{
	unsigned int value;

	writel(readl(&smicntl->smi_cr1) | SW_MODE, &smicntl->smi_cr1);
	writel(READ_ID, &smicntl->smi_tr);
	writel((banknum << BANKSEL_SHIFT) | SEND | TX_LEN_1 | RX_LEN_3,
	       &smicntl->smi_cr2);

	if (smi_wait_xfer_finish(XFER_FINISH_TOUT))
		return -EIO;

	value = (readl(&smicntl->smi_rr) & 0x00FFFFFF);

	writel(readl(&smicntl->smi_sr) & ~TFF, &smicntl->smi_sr);
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE, &smicntl->smi_cr1);

	return value;
}

/*
 * flash_get_size - Detect the SMI flash by reading the ID.
 * @base:	 Base address of the flash area bank #banknum
 * @banknum:	 Bank number
 *
 * Detect the SMI flash by reading the ID. Initializes the flash_info structure
 * with size, sector count etc.
 */
static ulong flash_get_size(ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int value;
	int i;

	value = smi_read_id(info, banknum);

	if (value < 0) {
		printf("Flash id could not be read\n");
		return 0;
	}

	/* Matches chip-id to entire list of 'serial-nor flash' ids */
	for (i = 0; i < ARRAY_SIZE(flash_devices); i++) {
		if (flash_devices[i].device_id == value) {
			info->size = flash_devices[i].size_in_bytes;
			info->flash_id = value;
			info->start[0] = base;
			info->sector_count =
					info->size/flash_devices[i].sectorsize;

			return info->size;
		}
	}

	return 0;
}

/*
 * smi_read_sr - Read status register of SMI
 * @bank:	 bank number
 *
 * This routine will get the status register of the flash chip present at the
 * given bank
 */
static int smi_read_sr(int bank)
{
	u32 ctrlreg1, val;

	/* store the CTRL REG1 state */
	ctrlreg1 = readl(&smicntl->smi_cr1);

	/* Program SMI in HW Mode */
	writel(readl(&smicntl->smi_cr1) & ~(SW_MODE | WB_MODE),
	       &smicntl->smi_cr1);

	/* Performing a RSR instruction in HW mode */
	writel((bank << BANKSEL_SHIFT) | RD_STATUS_REG, &smicntl->smi_cr2);

	if (smi_wait_xfer_finish(XFER_FINISH_TOUT))
		return -1;

	val = readl(&smicntl->smi_sr);

	/* Restore the CTRL REG1 state */
	writel(ctrlreg1, &smicntl->smi_cr1);

	return val;
}

/*
 * smi_wait_till_ready - Wait till last operation is over.
 * @bank:	 bank number shifted.
 * @timeout:	 timeout in milliseconds.
 *
 * This routine checks for WIP(write in progress)bit in Status register(SMSR-b0)
 * The routine checks for #timeout loops, each at interval of 1 milli-second.
 * If successful the routine returns 0.
 */
static int smi_wait_till_ready(int bank, int timeout)
{
	int sr;
	ulong start = get_timer(0);

	/* One chip guarantees max 5 msec wait here after page writes,
	   but potentially three seconds (!) after page erase. */
	while (get_timer(start) < timeout) {
		sr = smi_read_sr(bank);
		if ((sr >= 0) && (!(sr & WIP_BIT)))
			return 0;

		/* Try again after 10 usec */
		udelay(10);
	} while (timeout--);

	printf("SMI controller is still in wait, timeout=%d\n", timeout);
	return -EIO;
}

/*
 * smi_write_enable - Enable the flash to do write operation
 * @bank:	 bank number
 *
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static int smi_write_enable(int bank)
{
	u32 ctrlreg1;
	u32 start;
	int timeout = WMODE_TOUT;
	int sr;

	/* Store the CTRL REG1 state */
	ctrlreg1 = readl(&smicntl->smi_cr1);

	/* Program SMI in H/W Mode */
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE, &smicntl->smi_cr1);

	/* Give the Flash, Write Enable command */
	writel((bank << BANKSEL_SHIFT) | WE, &smicntl->smi_cr2);

	if (smi_wait_xfer_finish(XFER_FINISH_TOUT))
		return -1;

	/* Restore the CTRL REG1 state */
	writel(ctrlreg1, &smicntl->smi_cr1);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		sr = smi_read_sr(bank);
		if ((sr >= 0) && (sr & (1 << (bank + WM_SHIFT))))
			return 0;

		/* Try again after 10 usec */
		udelay(10);
	};

	return -1;
}

/*
 * smi_init - SMI initialization routine
 *
 * SMI initialization routine. Sets SMI control register1.
 */
void smi_init(void)
{
	/* Setting the fast mode values. SMI working at 166/4 = 41.5 MHz */
	writel(HOLD1 | FAST_MODE | BANK_EN | DSEL_TIME | PRESCAL4,
	       &smicntl->smi_cr1);
}

/*
 * smi_sector_erase - Erase flash sector
 * @info:	 flash_info structure pointer
 * @sector:	 sector number
 *
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static int smi_sector_erase(flash_info_t *info, unsigned int sector)
{
	int bank;
	unsigned int sect_add;
	unsigned int instruction;

	switch (info->start[0]) {
	case SMIBANK0_BASE:
		bank = BANK0;
		break;
	case SMIBANK1_BASE:
		bank = BANK1;
		break;
	case SMIBANK2_BASE:
		bank = BANK2;
		break;
	case SMIBANK3_BASE:
		bank = BANK3;
		break;
	default:
		return -1;
	}

	sect_add = sector * (info->size / info->sector_count);
	instruction = ((sect_add >> 8) & 0x0000FF00) | SECTOR_ERASE;

	writel(readl(&smicntl->smi_sr) & ~(ERF1 | ERF2), &smicntl->smi_sr);

	/* Wait until finished previous write command. */
	if (smi_wait_till_ready(bank, CONFIG_SYS_FLASH_ERASE_TOUT))
		return -EBUSY;

	/* Send write enable, before erase commands. */
	if (smi_write_enable(bank))
		return -EIO;

	/* Put SMI in SW mode */
	writel(readl(&smicntl->smi_cr1) | SW_MODE, &smicntl->smi_cr1);

	/* Send Sector Erase command in SW Mode */
	writel(instruction, &smicntl->smi_tr);
	writel((bank << BANKSEL_SHIFT) | SEND | TX_LEN_4,
		       &smicntl->smi_cr2);
	if (smi_wait_xfer_finish(XFER_FINISH_TOUT))
		return -EIO;

	if (smi_wait_till_ready(bank, CONFIG_SYS_FLASH_ERASE_TOUT))
		return -EBUSY;

	/* Put SMI in HW mode */
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE,
		       &smicntl->smi_cr1);

	return 0;
}

/*
 * smi_write - Write to SMI flash
 * @src_addr:	 source buffer
 * @dst_addr:	 destination buffer
 * @length:	 length to write in bytes
 * @bank:	 bank base address
 *
 * Write to SMI flash
 */
static int smi_write(unsigned int *src_addr, unsigned int *dst_addr,
		     unsigned int length, ulong bank_addr)
{
	u8 *src_addr8 = (u8 *)src_addr;
	u8 *dst_addr8 = (u8 *)dst_addr;
	int banknum;
	int i;

	switch (bank_addr) {
	case SMIBANK0_BASE:
		banknum = BANK0;
		break;
	case SMIBANK1_BASE:
		banknum = BANK1;
		break;
	case SMIBANK2_BASE:
		banknum = BANK2;
		break;
	case SMIBANK3_BASE:
		banknum = BANK3;
		break;
	default:
		return -1;
	}

	if (smi_wait_till_ready(banknum, CONFIG_SYS_FLASH_WRITE_TOUT))
		return -EBUSY;

	/* Set SMI in Hardware Mode */
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE, &smicntl->smi_cr1);

	if (smi_write_enable(banknum))
		return -EIO;

	/* Perform the write command */
	for (i = 0; i < length; i += 4) {
		if (((ulong) (dst_addr) % SFLASH_PAGE_SIZE) == 0) {
			if (smi_wait_till_ready(banknum,
						CONFIG_SYS_FLASH_WRITE_TOUT))
				return -EBUSY;

			if (smi_write_enable(banknum))
				return -EIO;
		}

		if (length < 4) {
			int k;

			/*
			 * Handle special case, where length < 4 (redundant env)
			 */
			for (k = 0; k < length; k++)
				*dst_addr8++ = *src_addr8++;
		} else {
			/* Normal 32bit write */
			*dst_addr++ = *src_addr++;
		}

		if ((readl(&smicntl->smi_sr) & (ERF1 | ERF2)))
			return -EIO;
	}

	if (smi_wait_till_ready(banknum, CONFIG_SYS_FLASH_WRITE_TOUT))
		return -EBUSY;

	writel(readl(&smicntl->smi_sr) & ~(WCF), &smicntl->smi_sr);

	return 0;
}

/*
 * write_buff - Write to SMI flash
 * @info:	 flash info structure
 * @src:	 source buffer
 * @dest_addr:	 destination buffer
 * @length:	 length to write in words
 *
 * Write to SMI flash
 */
int write_buff(flash_info_t *info, uchar *src, ulong dest_addr, ulong length)
{
	return smi_write((unsigned int *)src, (unsigned int *)dest_addr,
			 length, info->start[0]);
}

/*
 * flash_init - SMI flash initialization
 *
 * SMI flash initialization
 */
unsigned long flash_init(void)
{
	unsigned long size = 0;
	int i, j;

	smi_init();

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		size += flash_info[i].size = flash_get_size(bank_base[i], i);
	}

	for (j = 0; j < CONFIG_SYS_MAX_FLASH_BANKS; j++) {
		for (i = 1; i < flash_info[j].sector_count; i++)
			flash_info[j].start[i] =
			    flash_info[j].start[i - 1] +
			    flash_info->size / flash_info->sector_count;

	}

	return size;
}

/*
 * flash_print_info - Print SMI flash information
 *
 * Print SMI flash information
 */
void flash_print_info(flash_info_t *info)
{
	int i;
	if (info->flash_id == FLASH_UNKNOWN) {
		puts("missing or unknown FLASH type\n");
		return;
	}

	if (info->size >= 0x100000)
		printf("  Size: %ld MB in %d Sectors\n",
		       info->size >> 20, info->sector_count);
	else
		printf("  Size: %ld KB in %d Sectors\n",
		       info->size >> 10, info->sector_count);

	puts("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
		int size;
		int erased;
		u32 *flash;

		/*
		 * Check if whole sector is erased
		 */
		size = (info->size) / (info->sector_count);
		flash = (u32 *) info->start[i];
		size = size / sizeof(int);

		while ((size--) && (*flash++ == ~0))
			;

		size++;
		if (size)
			erased = 0;
		else
			erased = 1;

		if ((i % 5) == 0)
			printf("\n");

		printf(" %08lX%s%s",
		       info->start[i],
		       erased ? " E" : "  ", info->protect[i] ? "RO " : "   ");
#else
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i], info->protect[i] ? " (RO)  " : "     ");
#endif
	}
	putc('\n');
	return;
}

/*
 * flash_erase - Erase SMI flash
 *
 * Erase SMI flash
 */
int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int rcode = 0;
	int prot = 0;
	flash_sect_t sect;

	if ((s_first < 0) || (s_first > s_last)) {
		puts("- no sectors to erase\n");
		return 1;
	}

	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}
	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	} else {
		putc('\n');
	}

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {
			if (smi_sector_erase(info, sect))
				rcode = 1;
			else
				putc('.');
		}
	}
	puts(" done\n");
	return rcode;
}
#endif
