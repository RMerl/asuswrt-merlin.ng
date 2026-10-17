// SPDX-License-Identifier: GPL-2.0+
/*
 * SDHCI ADMA2 helper functions.
 */

#include <common.h>
#include <cpu_func.h>
#include <sdhci.h>
#include <malloc.h>
#include <asm/cache.h>

static void sdhci_adma_desc(struct sdhci_adma_desc *desc,
			    dma_addr_t addr, u16 len, bool end)
{
	u8 attr;

	attr = ADMA_DESC_ATTR_VALID | ADMA_DESC_TRANSFER_DATA;
	if (end)
		attr |= ADMA_DESC_ATTR_END;

	desc->attr = attr;
	desc->len = len;
	desc->reserved = 0;
	desc->addr_lo = lower_32_bits(addr);
#ifdef CONFIG_DMA_ADDR_T_64BIT
	desc->addr_hi = upper_32_bits(addr);
#endif
}

/**
 * sdhci_prepare_adma_table() - Populate the ADMA table
 *
 * @table:	Pointer to the ADMA table
 * @data:	Pointer to MMC data
 * @addr:	DMA address to write to or read from
 *
 * Fill the ADMA table according to the MMC data to read from or write to the
 * given DMA address.
 * Please note, that the table size depends on CONFIG_SYS_MMC_MAX_BLK_COUNT and
 * we don't have to check for overflow.
 */
void sdhci_prepare_adma_table(struct sdhci_adma_desc *table,
			      struct mmc_data *data, dma_addr_t addr)
{
	uint trans_bytes = data->blocksize * data->blocks;
	uint desc_count = DIV_ROUND_UP(trans_bytes, ADMA_MAX_LEN);
	struct sdhci_adma_desc *desc = table;
	int i = desc_count;

	while (--i) {
		sdhci_adma_desc(desc, addr, ADMA_MAX_LEN, false);
		addr += ADMA_MAX_LEN;
		trans_bytes -= ADMA_MAX_LEN;
		desc++;
	}

	sdhci_adma_desc(desc, addr, trans_bytes, true);

	flush_cache((dma_addr_t)table,
		    ROUND(desc_count * sizeof(struct sdhci_adma_desc),
			  ARCH_DMA_MINALIGN));
}

/**
 * sdhci_adma_init() - initialize the ADMA descriptor table
 *
 * Return: pointer to the allocated descriptor table or NULL in case of an
 * error.
 */
struct sdhci_adma_desc *sdhci_adma_init(void)
{
	return memalign(ARCH_DMA_MINALIGN, ADMA_TABLE_SZ);
}
