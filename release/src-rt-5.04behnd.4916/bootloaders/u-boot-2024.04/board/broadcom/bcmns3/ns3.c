// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom.
 *
 */

#include <common.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/gic-v3.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-bcmns3/bl33_info.h>
#include <dt-bindings/memory/bcm-ns3-mc.h>
#include <broadcom/chimp.h>

#define BANK_OFFSET(bank)      ((u64)BCM_NS3_DDR_INFO_BASE + 8 + ((bank) * 16))

/*
 * ns3_dram_bank - DDR bank details
 *
 * @start: DDR bank start address
 * @len: DDR bank length
 */
struct ns3_dram_bank {
	u64 start[BCM_NS3_MAX_NR_BANKS];
	u64 len[BCM_NS3_MAX_NR_BANKS];
};

/*
 * ns3_dram_hdr - DDR header info
 *
 * @sig: DDR info signature
 * @bank: DDR bank details
 */
struct ns3_dram_hdr {
	u32 sig;
	struct ns3_dram_bank bank;
};

static struct mm_region ns3_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = BCM_NS3_MEM_START,
		.phys = BCM_NS3_MEM_START,
		.size = BCM_NS3_MEM_LEN,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = BCM_NS3_BANK_1_MEM_START,
		.phys = BCM_NS3_BANK_1_MEM_START,
		.size = BCM_NS3_BANK_1_MEM_LEN,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = ns3_mem_map;

DECLARE_GLOBAL_DATA_PTR;

/*
 * Force the bl33_info to the data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
struct bl33_info *bl33_info __section(".data");

/*
 * Run modulo 256 checksum calculation and return the calculated checksum
 */
static u8 checksum_calc(u8 *p, unsigned int len)
{
	unsigned int i;
	u8 chksum = 0;

	for (i = 0; i < len; i++)
		chksum += p[i];

	return chksum;
}

/*
 * This function parses the memory layout information from a reserved area in
 * DDR, and then fix up the FDT before passing it to Linux.
 *
 * In the case of error, do nothing and the default memory layout in DT will
 * be used
 */
static int mem_info_parse_fixup(void *fdt)
{
	struct ns3_dram_hdr hdr;
	u32 *p32, i, nr_banks;
	u64 *p64;

	/* validate signature */
	p32 = (u32 *)BCM_NS3_DDR_INFO_BASE;
	hdr.sig = *p32;
	if (hdr.sig != BCM_NS3_DDR_INFO_SIG) {
		printf("DDR info signature 0x%x invalid\n", hdr.sig);
		return -EINVAL;
	}

	/* run checksum test to validate data  */
	if (checksum_calc((u8 *)p32, BCM_NS3_DDR_INFO_LEN) != 0) {
		printf("Checksum on DDR info failed\n");
		return -EINVAL;
	}

	/* parse information for each bank */
	nr_banks = 0;
	for (i = 0; i < BCM_NS3_MAX_NR_BANKS; i++) {
		/* skip banks with a length of zero */
		p64 = (u64 *)BANK_OFFSET(i);
		if (*(p64 + 1) == 0)
			continue;

		hdr.bank.start[i] = *p64;
		hdr.bank.len[i] = *(p64 + 1);

		printf("mem[%u] 0x%llx - 0x%llx\n", i, hdr.bank.start[i],
		       hdr.bank.start[i] + hdr.bank.len[i] - 1);
		nr_banks++;
	}

	if (!nr_banks) {
		printf("No DDR banks detected\n");
		return -ENOMEM;
	}

	return fdt_fixup_memory_banks(fdt, hdr.bank.start, hdr.bank.len,
				      nr_banks);
}

int board_init(void)
{
	/* Setup memory using "memory" node from DTB */
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;
	fdtdec_setup_memory_banksize();

	if (bl33_info->version != BL33_INFO_VERSION)
		printf("*** warning: ATF BL31 and U-Boot not in sync! ***\n");
#if IS_ENABLED(CONFIG_BNXT_ETH)
	if (chimp_fastboot_optee() != 0)
		printf("*** warning: secure chimp fastboot failed! ***\n");
#endif
	return 0;
}

int board_late_init(void)
{
	return 0;
}

int dram_init(void)
{
	/*
	 * Mark ram base as the last 16MB of 2GB DDR, which is 0xFF00_0000.
	 * So that relocation happens with in the last 16MB memory.
	 */
	gd->ram_base = (phys_size_t)(BCM_NS3_MEM_END - SZ_16M);
	gd->ram_size = (unsigned long)SZ_16M;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = (BCM_NS3_MEM_END - SZ_16M);
	gd->bd->bi_dram[0].size = SZ_16M;

	return 0;
}

/* Limit RAM used by U-Boot to the DDR first bank End region */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	return BCM_NS3_MEM_END;
}

void reset_cpu(void)
{
	/* Perform a level 3 reset */
	psci_system_reset2(3, 0);
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *fdt, struct bd_info *bd)
{
	u32 chimp_hs = CHIMP_HANDSHAKE_WAIT_TIMEOUT;

	/* FIXME: Need to call gic_lpi_tables_init correctly now */
	printf("%s: failed to init gic-lpi-tables\n", __func__);

	/*
	 * Check for chimp handshake status.
	 * Zero timeout value will actually fall to default timeout.
	 *
	 * System boot is independent of chimp handshake.
	 * chimp handshake failure is not a catastrophic error.
	 * Hence continue booting if chimp handshake fails.
	 */
	chimp_handshake_status_optee(0, &chimp_hs);
	if (chimp_hs == CHIMP_HANDSHAKE_SUCCESS)
		printf("ChiMP handshake successful\n");
	else
		printf("ERROR: ChiMP handshake status 0x%x\n", chimp_hs);

	return mem_info_parse_fixup(fdt);
}
#endif /* CONFIG_OF_BOARD_SETUP */
