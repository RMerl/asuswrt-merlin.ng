// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

__maybe_unused
static void print_num(const char *name, ulong value)
{
	printf("%-12s= 0x%0*lx\n", name, 2 * (int)sizeof(value), value);
}

__maybe_unused
static void print_eth(int idx)
{
	char name[10], *val;
	if (idx)
		sprintf(name, "eth%iaddr", idx);
	else
		strcpy(name, "ethaddr");
	val = env_get(name);
	if (!val)
		val = "(not set)";
	printf("%-12s= %s\n", name, val);
}

#ifndef CONFIG_DM_ETH
__maybe_unused
static void print_eths(void)
{
	struct eth_device *dev;
	int i = 0;

	do {
		dev = eth_get_dev_by_index(i);
		if (dev) {
			printf("eth%dname    = %s\n", i, dev->name);
			print_eth(i);
			i++;
		}
	} while (dev);

	printf("current eth = %s\n", eth_get_name());
	printf("ip_addr     = %s\n", env_get("ipaddr"));
}
#endif

__maybe_unused
static void print_lnum(const char *name, unsigned long long value)
{
	printf("%-12s= 0x%.8llX\n", name, value);
}

__maybe_unused
static void print_mhz(const char *name, unsigned long hz)
{
	char buf[32];

	printf("%-12s= %6s MHz\n", name, strmhz(buf, hz));
}


static inline void print_bi_boot_params(const bd_t *bd)
{
	print_num("boot_params",	(ulong)bd->bi_boot_params);
}

static inline void print_bi_mem(const bd_t *bd)
{
#if defined(CONFIG_SH)
	print_num("mem start      ",	(ulong)bd->bi_memstart);
	print_lnum("mem size       ",	(u64)bd->bi_memsize);
#elif defined(CONFIG_ARC)
	print_num("mem start",		(ulong)bd->bi_memstart);
	print_lnum("mem size",		(u64)bd->bi_memsize);
#else
	print_num("memstart",		(ulong)bd->bi_memstart);
	print_lnum("memsize",		(u64)bd->bi_memsize);
#endif
}

static inline void print_bi_dram(const bd_t *bd)
{
#ifdef CONFIG_NR_DRAM_BANKS
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			print_num("DRAM bank",	i);
			print_num("-> start",	bd->bi_dram[i].start);
			print_num("-> size",	bd->bi_dram[i].size);
		}
	}
#endif
}

static inline void print_bi_flash(const bd_t *bd)
{
#if defined(CONFIG_MICROBLAZE) || defined(CONFIG_SH)
	print_num("flash start    ",	(ulong)bd->bi_flashstart);
	print_num("flash size     ",	(ulong)bd->bi_flashsize);
	print_num("flash offset   ",	(ulong)bd->bi_flashoffset);

#elif defined(CONFIG_NIOS2)
	print_num("flash start",	(ulong)bd->bi_flashstart);
	print_num("flash size",		(ulong)bd->bi_flashsize);
	print_num("flash offset",	(ulong)bd->bi_flashoffset);
#else
	print_num("flashstart",		(ulong)bd->bi_flashstart);
	print_num("flashsize",		(ulong)bd->bi_flashsize);
	print_num("flashoffset",	(ulong)bd->bi_flashoffset);
#endif
}

static inline void print_eth_ip_addr(void)
{
#if defined(CONFIG_CMD_NET)
	print_eth(0);
#if defined(CONFIG_HAS_ETH1)
	print_eth(1);
#endif
#if defined(CONFIG_HAS_ETH2)
	print_eth(2);
#endif
#if defined(CONFIG_HAS_ETH3)
	print_eth(3);
#endif
#if defined(CONFIG_HAS_ETH4)
	print_eth(4);
#endif
#if defined(CONFIG_HAS_ETH5)
	print_eth(5);
#endif
	printf("IP addr     = %s\n", env_get("ipaddr"));
#endif
}

static inline void print_baudrate(void)
{
#if defined(CONFIG_PPC)
	printf("baudrate    = %6u bps\n", gd->baudrate);
#else
	printf("baudrate    = %u bps\n", gd->baudrate);
#endif
}

static inline void __maybe_unused print_std_bdinfo(const bd_t *bd)
{
	print_bi_boot_params(bd);
	print_bi_mem(bd);
	print_bi_flash(bd);
	print_eth_ip_addr();
	print_baudrate();
}

#if defined(CONFIG_PPC)
void __weak board_detail(void)
{
	/* Please define board_detail() for your platform */
}

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

#ifdef DEBUG
	print_num("bd address",		(ulong)bd);
#endif
	print_bi_mem(bd);
	print_bi_flash(bd);
	print_num("sramstart",		bd->bi_sramstart);
	print_num("sramsize",		bd->bi_sramsize);
#if	defined(CONFIG_MPC8xx) || defined(CONFIG_E500)
	print_num("immr_base",		bd->bi_immr_base);
#endif
	print_num("bootflags",		bd->bi_bootflags);
#if defined(CONFIG_CPM2)
	print_mhz("vco",		bd->bi_vco);
	print_mhz("sccfreq",		bd->bi_sccfreq);
	print_mhz("brgfreq",		bd->bi_brgfreq);
#endif
	print_mhz("intfreq",		bd->bi_intfreq);
#if defined(CONFIG_CPM2)
	print_mhz("cpmfreq",		bd->bi_cpmfreq);
#endif
	print_mhz("busfreq",		bd->bi_busfreq);

#ifdef CONFIG_ENABLE_36BIT_PHYS
#ifdef CONFIG_PHYS_64BIT
	puts("addressing  = 36-bit\n");
#else
	puts("addressing  = 32-bit\n");
#endif
#endif

	print_eth_ip_addr();
	print_baudrate();
	print_num("relocaddr", gd->relocaddr);
	board_detail();
	return 0;
}

#elif defined(CONFIG_NIOS2)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_dram(bd);
	print_bi_flash(bd);

#if defined(CONFIG_SYS_SRAM_BASE)
	print_num ("sram start",	(ulong)bd->bi_sramstart);
	print_num ("sram size",		(ulong)bd->bi_sramsize);
#endif

	print_eth_ip_addr();
	print_baudrate();

	return 0;
}

#elif defined(CONFIG_MICROBLAZE)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_dram(bd);
	print_bi_flash(bd);
#if defined(CONFIG_SYS_SRAM_BASE)
	print_num("sram start     ",	(ulong)bd->bi_sramstart);
	print_num("sram size      ",	(ulong)bd->bi_sramsize);
#endif
#if defined(CONFIG_CMD_NET) && !defined(CONFIG_DM_ETH)
	print_eths();
#endif
	print_baudrate();
	print_num("relocaddr", gd->relocaddr);
	print_num("reloc off", gd->reloc_off);
	print_num("fdt_blob", (ulong)gd->fdt_blob);
	print_num("new_fdt", (ulong)gd->new_fdt);
	print_num("fdt_size", (ulong)gd->fdt_size);

	return 0;
}

#elif defined(CONFIG_M68K)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_mem(bd);
	print_bi_flash(bd);
#if defined(CONFIG_SYS_INIT_RAM_ADDR)
	print_num("sramstart",		(ulong)bd->bi_sramstart);
	print_num("sramsize",		(ulong)bd->bi_sramsize);
#endif
#if defined(CONFIG_SYS_MBAR)
	print_num("mbar",		bd->bi_mbar_base);
#endif
	print_mhz("cpufreq",		bd->bi_intfreq);
	print_mhz("busfreq",		bd->bi_busfreq);
#ifdef CONFIG_PCI
	print_mhz("pcifreq",		bd->bi_pcifreq);
#endif
#ifdef CONFIG_EXTRA_CLOCK
	print_mhz("flbfreq",		bd->bi_flbfreq);
	print_mhz("inpfreq",		bd->bi_inpfreq);
	print_mhz("vcofreq",		bd->bi_vcofreq);
#endif
	print_eth_ip_addr();
	print_baudrate();

	return 0;
}

#elif defined(CONFIG_MIPS)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	print_std_bdinfo(gd->bd);
	print_num("relocaddr", gd->relocaddr);
	print_num("reloc off", gd->reloc_off);

	return 0;
}

#elif defined(CONFIG_ARM)

static int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	bd_t *bd = gd->bd;

	print_num("arch_number",	bd->bi_arch_number);
	print_bi_boot_params(bd);
	print_bi_dram(bd);

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	if (gd->arch.secure_ram & MEM_RESERVE_SECURE_SECURED) {
		print_num("Secure ram",
			  gd->arch.secure_ram & MEM_RESERVE_SECURE_ADDR_MASK);
	}
#endif
#ifdef CONFIG_RESV_RAM
	if (gd->arch.resv_ram)
		print_num("Reserved ram", gd->arch.resv_ram);
#endif
#if defined(CONFIG_CMD_NET) && !defined(CONFIG_DM_ETH)
	print_eths();
#endif
	print_baudrate();
#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
	print_num("TLB addr", gd->arch.tlb_addr);
#endif
	print_num("relocaddr", gd->relocaddr);
	print_num("reloc off", gd->reloc_off);
	print_num("irq_sp", gd->irq_sp);	/* irq stack pointer */
	print_num("sp start ", gd->start_addr_sp);
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	print_num("FB base  ", gd->fb_base);
#endif
	/*
	 * TODO: Currently only support for davinci SOC's is added.
	 * Remove this check once all the board implement this.
	 */
#ifdef CONFIG_CLOCKS
	printf("ARM frequency = %ld MHz\n", gd->bd->bi_arm_freq);
	printf("DSP frequency = %ld MHz\n", gd->bd->bi_dsp_freq);
	printf("DDR frequency = %ld MHz\n", gd->bd->bi_ddr_freq);
#endif
#ifdef CONFIG_BOARD_TYPES
	printf("Board Type  = %ld\n", gd->board_type);
#endif
#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	printf("Early malloc usage: %lx / %x\n", gd->malloc_ptr,
	       CONFIG_VAL(SYS_MALLOC_F_LEN));
#endif
	if (gd->fdt_blob)
		print_num("fdt_blob", (ulong)gd->fdt_blob);

	return 0;
}

#elif defined(CONFIG_SH)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_mem(bd);
	print_bi_flash(bd);
	print_eth_ip_addr();
	print_baudrate();
	return 0;
}

#elif defined(CONFIG_X86)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_boot_params(bd);

	print_bi_dram(bd);

	print_num("relocaddr", gd->relocaddr);
	print_num("reloc off", gd->reloc_off);
#if defined(CONFIG_CMD_NET)
	print_eth_ip_addr();
	print_mhz("ethspeed",	    bd->bi_ethspeed);
#endif
	print_baudrate();

	return 0;
}

#elif defined(CONFIG_SANDBOX)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_boot_params(bd);
	print_bi_dram(bd);
	print_eth_ip_addr();

#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	print_num("FB base  ", gd->fb_base);
#endif
	return 0;
}

#elif defined(CONFIG_NDS32)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_num("arch_number",	bd->bi_arch_number);
	print_bi_boot_params(bd);
	print_bi_dram(bd);
	print_eth_ip_addr();
	print_baudrate();

	return 0;
}

#elif defined(CONFIG_RISCV)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_boot_params(bd);
	print_bi_dram(bd);
	print_num("relocaddr", gd->relocaddr);
	print_num("reloc off", gd->reloc_off);
	print_eth_ip_addr();
	print_baudrate();

	return 0;
}

#elif defined(CONFIG_ARC)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t *bd = gd->bd;

	print_bi_mem(bd);
	print_eth_ip_addr();
	print_baudrate();

	return 0;
}

#elif defined(CONFIG_XTENSA)

int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	print_std_bdinfo(gd->bd);
	return 0;
}

#else
 #error "a case for this architecture does not exist!"
#endif

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	bdinfo,	1,	1,	do_bdinfo,
	"print Board Info structure",
	""
);
