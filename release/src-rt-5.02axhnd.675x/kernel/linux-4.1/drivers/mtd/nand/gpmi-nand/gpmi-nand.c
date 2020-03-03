/*
 * Freescale GPMI NAND Flash Driver
 *
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 * Copyright (C) 2008 Embedded Alley Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_mtd.h>
#include "gpmi-nand.h"
#include "bch-regs.h"

/* Resource names for the GPMI NAND driver. */
#define GPMI_NAND_GPMI_REGS_ADDR_RES_NAME  "gpmi-nand"
#define GPMI_NAND_BCH_REGS_ADDR_RES_NAME   "bch"
#define GPMI_NAND_BCH_INTERRUPT_RES_NAME   "bch"

/* add our owner bbt descriptor */
static uint8_t scan_ff_pattern[] = { 0xff };
static struct nand_bbt_descr gpmi_bbt_descr = {
	.options	= 0,
	.offs		= 0,
	.len		= 1,
	.pattern	= scan_ff_pattern
};

/*
 * We may change the layout if we can get the ECC info from the datasheet,
 * else we will use all the (page + OOB).
 */
static struct nand_ecclayout gpmi_hw_ecclayout = {
	.eccbytes = 0,
	.eccpos = { 0, },
	.oobfree = { {.offset = 0, .length = 0} }
};

static const struct gpmi_devdata gpmi_devdata_imx23 = {
	.type = IS_MX23,
	.bch_max_ecc_strength = 20,
	.max_chain_delay = 16,
};

static const struct gpmi_devdata gpmi_devdata_imx28 = {
	.type = IS_MX28,
	.bch_max_ecc_strength = 20,
	.max_chain_delay = 16,
};

static const struct gpmi_devdata gpmi_devdata_imx6q = {
	.type = IS_MX6Q,
	.bch_max_ecc_strength = 40,
	.max_chain_delay = 12,
};

static const struct gpmi_devdata gpmi_devdata_imx6sx = {
	.type = IS_MX6SX,
	.bch_max_ecc_strength = 62,
	.max_chain_delay = 12,
};

static irqreturn_t bch_irq(int irq, void *cookie)
{
	struct gpmi_nand_data *this = cookie;

	gpmi_clear_bch(this);
	complete(&this->bch_done);
	return IRQ_HANDLED;
}

/*
 *  Calculate the ECC strength by hand:
 *	E : The ECC strength.
 *	G : the length of Galois Field.
 *	N : The chunk count of per page.
 *	O : the oobsize of the NAND chip.
 *	M : the metasize of per page.
 *
 *	The formula is :
 *		E * G * N
 *	      ------------ <= (O - M)
 *                  8
 *
 *      So, we get E by:
 *                    (O - M) * 8
 *              E <= -------------
 *                       G * N
 */
static inline int get_ecc_strength(struct gpmi_nand_data *this)
{
	struct bch_geometry *geo = &this->bch_geometry;
	struct mtd_info	*mtd = &this->mtd;
	int ecc_strength;

	ecc_strength = ((mtd->oobsize - geo->metadata_size) * 8)
			/ (geo->gf_len * geo->ecc_chunk_count);

	/* We need the minor even number. */
	return round_down(ecc_strength, 2);
}

static inline bool gpmi_check_ecc(struct gpmi_nand_data *this)
{
	struct bch_geometry *geo = &this->bch_geometry;

	/* Do the sanity check. */
	if (GPMI_IS_MX23(this) || GPMI_IS_MX28(this)) {
		/* The mx23/mx28 only support the GF13. */
		if (geo->gf_len == 14)
			return false;
	}
	return geo->ecc_strength <= this->devdata->bch_max_ecc_strength;
}

/*
 * If we can get the ECC information from the nand chip, we do not
 * need to calculate them ourselves.
 *
 * We may have available oob space in this case.
 */
static bool set_geometry_by_ecc_info(struct gpmi_nand_data *this)
{
	struct bch_geometry *geo = &this->bch_geometry;
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = mtd->priv;
	struct nand_oobfree *of = gpmi_hw_ecclayout.oobfree;
	unsigned int block_mark_bit_offset;

	if (!(chip->ecc_strength_ds > 0 && chip->ecc_step_ds > 0))
		return false;

	switch (chip->ecc_step_ds) {
	case SZ_512:
		geo->gf_len = 13;
		break;
	case SZ_1K:
		geo->gf_len = 14;
		break;
	default:
		dev_err(this->dev,
			"unsupported nand chip. ecc bits : %d, ecc size : %d\n",
			chip->ecc_strength_ds, chip->ecc_step_ds);
		return false;
	}
	geo->ecc_chunk_size = chip->ecc_step_ds;
	geo->ecc_strength = round_up(chip->ecc_strength_ds, 2);
	if (!gpmi_check_ecc(this))
		return false;

	/* Keep the C >= O */
	if (geo->ecc_chunk_size < mtd->oobsize) {
		dev_err(this->dev,
			"unsupported nand chip. ecc size: %d, oob size : %d\n",
			chip->ecc_step_ds, mtd->oobsize);
		return false;
	}

	/* The default value, see comment in the legacy_set_geometry(). */
	geo->metadata_size = 10;

	geo->ecc_chunk_count = mtd->writesize / geo->ecc_chunk_size;

	/*
	 * Now, the NAND chip with 2K page(data chunk is 512byte) shows below:
	 *
	 *    |                          P                            |
	 *    |<----------------------------------------------------->|
	 *    |                                                       |
	 *    |                                        (Block Mark)   |
	 *    |                      P'                      |      | |     |
	 *    |<-------------------------------------------->|  D   | |  O' |
	 *    |                                              |<---->| |<--->|
	 *    V                                              V      V V     V
	 *    +---+----------+-+----------+-+----------+-+----------+-+-----+
	 *    | M |   data   |E|   data   |E|   data   |E|   data   |E|     |
	 *    +---+----------+-+----------+-+----------+-+----------+-+-----+
	 *                                                   ^              ^
	 *                                                   |      O       |
	 *                                                   |<------------>|
	 *                                                   |              |
	 *
	 *	P : the page size for BCH module.
	 *	E : The ECC strength.
	 *	G : the length of Galois Field.
	 *	N : The chunk count of per page.
	 *	M : the metasize of per page.
	 *	C : the ecc chunk size, aka the "data" above.
	 *	P': the nand chip's page size.
	 *	O : the nand chip's oob size.
	 *	O': the free oob.
	 *
	 *	The formula for P is :
	 *
	 *	            E * G * N
	 *	       P = ------------ + P' + M
	 *                      8
	 *
	 * The position of block mark moves forward in the ECC-based view
	 * of page, and the delta is:
	 *
	 *                   E * G * (N - 1)
	 *             D = (---------------- + M)
	 *                          8
	 *
	 * Please see the comment in legacy_set_geometry().
	 * With the condition C >= O , we still can get same result.
	 * So the bit position of the physical block mark within the ECC-based
	 * view of the page is :
	 *             (P' - D) * 8
	 */
	geo->page_size = mtd->writesize + geo->metadata_size +
		(geo->gf_len * geo->ecc_strength * geo->ecc_chunk_count) / 8;

	/* The available oob size we have. */
	if (geo->page_size < mtd->writesize + mtd->oobsize) {
		of->offset = geo->page_size - mtd->writesize;
		of->length = mtd->oobsize - of->offset;
	}

	geo->payload_size = mtd->writesize;

	geo->auxiliary_status_offset = ALIGN(geo->metadata_size, 4);
	geo->auxiliary_size = ALIGN(geo->metadata_size, 4)
				+ ALIGN(geo->ecc_chunk_count, 4);

	if (!this->swap_block_mark)
		return true;

	/* For bit swap. */
	block_mark_bit_offset = mtd->writesize * 8 -
		(geo->ecc_strength * geo->gf_len * (geo->ecc_chunk_count - 1)
				+ geo->metadata_size * 8);

	geo->block_mark_byte_offset = block_mark_bit_offset / 8;
	geo->block_mark_bit_offset  = block_mark_bit_offset % 8;
	return true;
}

static int legacy_set_geometry(struct gpmi_nand_data *this)
{
	struct bch_geometry *geo = &this->bch_geometry;
	struct mtd_info *mtd = &this->mtd;
	unsigned int metadata_size;
	unsigned int status_size;
	unsigned int block_mark_bit_offset;

	/*
	 * The size of the metadata can be changed, though we set it to 10
	 * bytes now. But it can't be too large, because we have to save
	 * enough space for BCH.
	 */
	geo->metadata_size = 10;

	/* The default for the length of Galois Field. */
	geo->gf_len = 13;

	/* The default for chunk size. */
	geo->ecc_chunk_size = 512;
	while (geo->ecc_chunk_size < mtd->oobsize) {
		geo->ecc_chunk_size *= 2; /* keep C >= O */
		geo->gf_len = 14;
	}

	geo->ecc_chunk_count = mtd->writesize / geo->ecc_chunk_size;

	/* We use the same ECC strength for all chunks. */
	geo->ecc_strength = get_ecc_strength(this);
	if (!gpmi_check_ecc(this)) {
		dev_err(this->dev,
			"required ecc strength of the NAND chip: %d is not supported by the GPMI controller (%d)\n",
			geo->ecc_strength,
			this->devdata->bch_max_ecc_strength);
		return -EINVAL;
	}

	geo->page_size = mtd->writesize + mtd->oobsize;
	geo->payload_size = mtd->writesize;

	/*
	 * The auxiliary buffer contains the metadata and the ECC status. The
	 * metadata is padded to the nearest 32-bit boundary. The ECC status
	 * contains one byte for every ECC chunk, and is also padded to the
	 * nearest 32-bit boundary.
	 */
	metadata_size = ALIGN(geo->metadata_size, 4);
	status_size   = ALIGN(geo->ecc_chunk_count, 4);

	geo->auxiliary_size = metadata_size + status_size;
	geo->auxiliary_status_offset = metadata_size;

	if (!this->swap_block_mark)
		return 0;

	/*
	 * We need to compute the byte and bit offsets of
	 * the physical block mark within the ECC-based view of the page.
	 *
	 * NAND chip with 2K page shows below:
	 *                                             (Block Mark)
	 *                                                   |      |
	 *                                                   |  D   |
	 *                                                   |<---->|
	 *                                                   V      V
	 *    +---+----------+-+----------+-+----------+-+----------+-+
	 *    | M |   data   |E|   data   |E|   data   |E|   data   |E|
	 *    +---+----------+-+----------+-+----------+-+----------+-+
	 *
	 * The position of block mark moves forward in the ECC-based view
	 * of page, and the delta is:
	 *
	 *                   E * G * (N - 1)
	 *             D = (---------------- + M)
	 *                          8
	 *
	 * With the formula to compute the ECC strength, and the condition
	 *       : C >= O         (C is the ecc chunk size)
	 *
	 * It's easy to deduce to the following result:
	 *
	 *         E * G       (O - M)      C - M         C - M
	 *      ----------- <= ------- <=  --------  <  ---------
	 *           8            N           N          (N - 1)
	 *
	 *  So, we get:
	 *
	 *                   E * G * (N - 1)
	 *             D = (---------------- + M) < C
	 *                          8
	 *
	 *  The above inequality means the position of block mark
	 *  within the ECC-based view of the page is still in the data chunk,
	 *  and it's NOT in the ECC bits of the chunk.
	 *
	 *  Use the following to compute the bit position of the
	 *  physical block mark within the ECC-based view of the page:
	 *          (page_size - D) * 8
	 *
	 *  --Huang Shijie
	 */
	block_mark_bit_offset = mtd->writesize * 8 -
		(geo->ecc_strength * geo->gf_len * (geo->ecc_chunk_count - 1)
				+ geo->metadata_size * 8);

	geo->block_mark_byte_offset = block_mark_bit_offset / 8;
	geo->block_mark_bit_offset  = block_mark_bit_offset % 8;
	return 0;
}

int common_nfc_set_geometry(struct gpmi_nand_data *this)
{
	if (of_property_read_bool(this->dev->of_node, "fsl,use-minimum-ecc")
		&& set_geometry_by_ecc_info(this))
		return 0;
	return legacy_set_geometry(this);
}

struct dma_chan *get_dma_chan(struct gpmi_nand_data *this)
{
	/* We use the DMA channel 0 to access all the nand chips. */
	return this->dma_chans[0];
}

/* Can we use the upper's buffer directly for DMA? */
void prepare_data_dma(struct gpmi_nand_data *this, enum dma_data_direction dr)
{
	struct scatterlist *sgl = &this->data_sgl;
	int ret;

	/* first try to map the upper buffer directly */
	if (virt_addr_valid(this->upper_buf) &&
		!object_is_on_stack(this->upper_buf)) {
		sg_init_one(sgl, this->upper_buf, this->upper_len);
		ret = dma_map_sg(this->dev, sgl, 1, dr);
		if (ret == 0)
			goto map_fail;

		this->direct_dma_map_ok = true;
		return;
	}

map_fail:
	/* We have to use our own DMA buffer. */
	sg_init_one(sgl, this->data_buffer_dma, this->upper_len);

	if (dr == DMA_TO_DEVICE)
		memcpy(this->data_buffer_dma, this->upper_buf, this->upper_len);

	dma_map_sg(this->dev, sgl, 1, dr);

	this->direct_dma_map_ok = false;
}

/* This will be called after the DMA operation is finished. */
static void dma_irq_callback(void *param)
{
	struct gpmi_nand_data *this = param;
	struct completion *dma_c = &this->dma_done;

	switch (this->dma_type) {
	case DMA_FOR_COMMAND:
		dma_unmap_sg(this->dev, &this->cmd_sgl, 1, DMA_TO_DEVICE);
		break;

	case DMA_FOR_READ_DATA:
		dma_unmap_sg(this->dev, &this->data_sgl, 1, DMA_FROM_DEVICE);
		if (this->direct_dma_map_ok == false)
			memcpy(this->upper_buf, this->data_buffer_dma,
				this->upper_len);
		break;

	case DMA_FOR_WRITE_DATA:
		dma_unmap_sg(this->dev, &this->data_sgl, 1, DMA_TO_DEVICE);
		break;

	case DMA_FOR_READ_ECC_PAGE:
	case DMA_FOR_WRITE_ECC_PAGE:
		/* We have to wait the BCH interrupt to finish. */
		break;

	default:
		dev_err(this->dev, "in wrong DMA operation.\n");
	}

	complete(dma_c);
}

int start_dma_without_bch_irq(struct gpmi_nand_data *this,
				struct dma_async_tx_descriptor *desc)
{
	struct completion *dma_c = &this->dma_done;
	unsigned long timeout;

	init_completion(dma_c);

	desc->callback		= dma_irq_callback;
	desc->callback_param	= this;
	dmaengine_submit(desc);
	dma_async_issue_pending(get_dma_chan(this));

	/* Wait for the interrupt from the DMA block. */
	timeout = wait_for_completion_timeout(dma_c, msecs_to_jiffies(1000));
	if (!timeout) {
		dev_err(this->dev, "DMA timeout, last DMA :%d\n",
			this->last_dma_type);
		gpmi_dump_info(this);
		return -ETIMEDOUT;
	}
	return 0;
}

/*
 * This function is used in BCH reading or BCH writing pages.
 * It will wait for the BCH interrupt as long as ONE second.
 * Actually, we must wait for two interrupts :
 *	[1] firstly the DMA interrupt and
 *	[2] secondly the BCH interrupt.
 */
int start_dma_with_bch_irq(struct gpmi_nand_data *this,
			struct dma_async_tx_descriptor *desc)
{
	struct completion *bch_c = &this->bch_done;
	unsigned long timeout;

	/* Prepare to receive an interrupt from the BCH block. */
	init_completion(bch_c);

	/* start the DMA */
	start_dma_without_bch_irq(this, desc);

	/* Wait for the interrupt from the BCH block. */
	timeout = wait_for_completion_timeout(bch_c, msecs_to_jiffies(1000));
	if (!timeout) {
		dev_err(this->dev, "BCH timeout, last DMA :%d\n",
			this->last_dma_type);
		gpmi_dump_info(this);
		return -ETIMEDOUT;
	}
	return 0;
}

static int acquire_register_block(struct gpmi_nand_data *this,
				  const char *res_name)
{
	struct platform_device *pdev = this->pdev;
	struct resources *res = &this->resources;
	struct resource *r;
	void __iomem *p;

	r = platform_get_resource_byname(pdev, IORESOURCE_MEM, res_name);
	p = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(p))
		return PTR_ERR(p);

	if (!strcmp(res_name, GPMI_NAND_GPMI_REGS_ADDR_RES_NAME))
		res->gpmi_regs = p;
	else if (!strcmp(res_name, GPMI_NAND_BCH_REGS_ADDR_RES_NAME))
		res->bch_regs = p;
	else
		dev_err(this->dev, "unknown resource name : %s\n", res_name);

	return 0;
}

static int acquire_bch_irq(struct gpmi_nand_data *this, irq_handler_t irq_h)
{
	struct platform_device *pdev = this->pdev;
	const char *res_name = GPMI_NAND_BCH_INTERRUPT_RES_NAME;
	struct resource *r;
	int err;

	r = platform_get_resource_byname(pdev, IORESOURCE_IRQ, res_name);
	if (!r) {
		dev_err(this->dev, "Can't get resource for %s\n", res_name);
		return -ENODEV;
	}

	err = devm_request_irq(this->dev, r->start, irq_h, 0, res_name, this);
	if (err)
		dev_err(this->dev, "error requesting BCH IRQ\n");

	return err;
}

static void release_dma_channels(struct gpmi_nand_data *this)
{
	unsigned int i;
	for (i = 0; i < DMA_CHANS; i++)
		if (this->dma_chans[i]) {
			dma_release_channel(this->dma_chans[i]);
			this->dma_chans[i] = NULL;
		}
}

static int acquire_dma_channels(struct gpmi_nand_data *this)
{
	struct platform_device *pdev = this->pdev;
	struct dma_chan *dma_chan;

	/* request dma channel */
	dma_chan = dma_request_slave_channel(&pdev->dev, "rx-tx");
	if (!dma_chan) {
		dev_err(this->dev, "Failed to request DMA channel.\n");
		goto acquire_err;
	}

	this->dma_chans[0] = dma_chan;
	return 0;

acquire_err:
	release_dma_channels(this);
	return -EINVAL;
}

static char *extra_clks_for_mx6q[GPMI_CLK_MAX] = {
	"gpmi_apb", "gpmi_bch", "gpmi_bch_apb", "per1_bch",
};

static int gpmi_get_clks(struct gpmi_nand_data *this)
{
	struct resources *r = &this->resources;
	char **extra_clks = NULL;
	struct clk *clk;
	int err, i;

	/* The main clock is stored in the first. */
	r->clock[0] = devm_clk_get(this->dev, "gpmi_io");
	if (IS_ERR(r->clock[0])) {
		err = PTR_ERR(r->clock[0]);
		goto err_clock;
	}

	/* Get extra clocks */
	if (GPMI_IS_MX6(this))
		extra_clks = extra_clks_for_mx6q;
	if (!extra_clks)
		return 0;

	for (i = 1; i < GPMI_CLK_MAX; i++) {
		if (extra_clks[i - 1] == NULL)
			break;

		clk = devm_clk_get(this->dev, extra_clks[i - 1]);
		if (IS_ERR(clk)) {
			err = PTR_ERR(clk);
			goto err_clock;
		}

		r->clock[i] = clk;
	}

	if (GPMI_IS_MX6(this))
		/*
		 * Set the default value for the gpmi clock.
		 *
		 * If you want to use the ONFI nand which is in the
		 * Synchronous Mode, you should change the clock as you need.
		 */
		clk_set_rate(r->clock[0], 22000000);

	return 0;

err_clock:
	dev_dbg(this->dev, "failed in finding the clocks.\n");
	return err;
}

static int acquire_resources(struct gpmi_nand_data *this)
{
	int ret;

	ret = acquire_register_block(this, GPMI_NAND_GPMI_REGS_ADDR_RES_NAME);
	if (ret)
		goto exit_regs;

	ret = acquire_register_block(this, GPMI_NAND_BCH_REGS_ADDR_RES_NAME);
	if (ret)
		goto exit_regs;

	ret = acquire_bch_irq(this, bch_irq);
	if (ret)
		goto exit_regs;

	ret = acquire_dma_channels(this);
	if (ret)
		goto exit_regs;

	ret = gpmi_get_clks(this);
	if (ret)
		goto exit_clock;
	return 0;

exit_clock:
	release_dma_channels(this);
exit_regs:
	return ret;
}

static void release_resources(struct gpmi_nand_data *this)
{
	release_dma_channels(this);
}

static int init_hardware(struct gpmi_nand_data *this)
{
	int ret;

	/*
	 * This structure contains the "safe" GPMI timing that should succeed
	 * with any NAND Flash device
	 * (although, with less-than-optimal performance).
	 */
	struct nand_timing  safe_timing = {
		.data_setup_in_ns        = 80,
		.data_hold_in_ns         = 60,
		.address_setup_in_ns     = 25,
		.gpmi_sample_delay_in_ns =  6,
		.tREA_in_ns              = -1,
		.tRLOH_in_ns             = -1,
		.tRHOH_in_ns             = -1,
	};

	/* Initialize the hardwares. */
	ret = gpmi_init(this);
	if (ret)
		return ret;

	this->timing = safe_timing;
	return 0;
}

static int read_page_prepare(struct gpmi_nand_data *this,
			void *destination, unsigned length,
			void *alt_virt, dma_addr_t alt_phys, unsigned alt_size,
			void **use_virt, dma_addr_t *use_phys)
{
	struct device *dev = this->dev;

	if (virt_addr_valid(destination)) {
		dma_addr_t dest_phys;

		dest_phys = dma_map_single(dev, destination,
						length, DMA_FROM_DEVICE);
		if (dma_mapping_error(dev, dest_phys)) {
			if (alt_size < length) {
				dev_err(dev, "Alternate buffer is too small\n");
				return -ENOMEM;
			}
			goto map_failed;
		}
		*use_virt = destination;
		*use_phys = dest_phys;
		this->direct_dma_map_ok = true;
		return 0;
	}

map_failed:
	*use_virt = alt_virt;
	*use_phys = alt_phys;
	this->direct_dma_map_ok = false;
	return 0;
}

static inline void read_page_end(struct gpmi_nand_data *this,
			void *destination, unsigned length,
			void *alt_virt, dma_addr_t alt_phys, unsigned alt_size,
			void *used_virt, dma_addr_t used_phys)
{
	if (this->direct_dma_map_ok)
		dma_unmap_single(this->dev, used_phys, length, DMA_FROM_DEVICE);
}

static inline void read_page_swap_end(struct gpmi_nand_data *this,
			void *destination, unsigned length,
			void *alt_virt, dma_addr_t alt_phys, unsigned alt_size,
			void *used_virt, dma_addr_t used_phys)
{
	if (!this->direct_dma_map_ok)
		memcpy(destination, alt_virt, length);
}

static int send_page_prepare(struct gpmi_nand_data *this,
			const void *source, unsigned length,
			void *alt_virt, dma_addr_t alt_phys, unsigned alt_size,
			const void **use_virt, dma_addr_t *use_phys)
{
	struct device *dev = this->dev;

	if (virt_addr_valid(source)) {
		dma_addr_t source_phys;

		source_phys = dma_map_single(dev, (void *)source, length,
						DMA_TO_DEVICE);
		if (dma_mapping_error(dev, source_phys)) {
			if (alt_size < length) {
				dev_err(dev, "Alternate buffer is too small\n");
				return -ENOMEM;
			}
			goto map_failed;
		}
		*use_virt = source;
		*use_phys = source_phys;
		return 0;
	}
map_failed:
	/*
	 * Copy the content of the source buffer into the alternate
	 * buffer and set up the return values accordingly.
	 */
	memcpy(alt_virt, source, length);

	*use_virt = alt_virt;
	*use_phys = alt_phys;
	return 0;
}

static void send_page_end(struct gpmi_nand_data *this,
			const void *source, unsigned length,
			void *alt_virt, dma_addr_t alt_phys, unsigned alt_size,
			const void *used_virt, dma_addr_t used_phys)
{
	struct device *dev = this->dev;
	if (used_virt == source)
		dma_unmap_single(dev, used_phys, length, DMA_TO_DEVICE);
}

static void gpmi_free_dma_buffer(struct gpmi_nand_data *this)
{
	struct device *dev = this->dev;

	if (this->page_buffer_virt && virt_addr_valid(this->page_buffer_virt))
		dma_free_coherent(dev, this->page_buffer_size,
					this->page_buffer_virt,
					this->page_buffer_phys);
	kfree(this->cmd_buffer);
	kfree(this->data_buffer_dma);
	kfree(this->raw_buffer);

	this->cmd_buffer	= NULL;
	this->data_buffer_dma	= NULL;
	this->page_buffer_virt	= NULL;
	this->page_buffer_size	=  0;
}

/* Allocate the DMA buffers */
static int gpmi_alloc_dma_buffer(struct gpmi_nand_data *this)
{
	struct bch_geometry *geo = &this->bch_geometry;
	struct device *dev = this->dev;
	struct mtd_info *mtd = &this->mtd;

	/* [1] Allocate a command buffer. PAGE_SIZE is enough. */
	this->cmd_buffer = kzalloc(PAGE_SIZE, GFP_DMA | GFP_KERNEL);
	if (this->cmd_buffer == NULL)
		goto error_alloc;

	/*
	 * [2] Allocate a read/write data buffer.
	 *     The gpmi_alloc_dma_buffer can be called twice.
	 *     We allocate a PAGE_SIZE length buffer if gpmi_alloc_dma_buffer
	 *     is called before the nand_scan_ident; and we allocate a buffer
	 *     of the real NAND page size when the gpmi_alloc_dma_buffer is
	 *     called after the nand_scan_ident.
	 */
	this->data_buffer_dma = kzalloc(mtd->writesize ?: PAGE_SIZE,
					GFP_DMA | GFP_KERNEL);
	if (this->data_buffer_dma == NULL)
		goto error_alloc;

	/*
	 * [3] Allocate the page buffer.
	 *
	 * Both the payload buffer and the auxiliary buffer must appear on
	 * 32-bit boundaries. We presume the size of the payload buffer is a
	 * power of two and is much larger than four, which guarantees the
	 * auxiliary buffer will appear on a 32-bit boundary.
	 */
	this->page_buffer_size = geo->payload_size + geo->auxiliary_size;
	this->page_buffer_virt = dma_alloc_coherent(dev, this->page_buffer_size,
					&this->page_buffer_phys, GFP_DMA);
	if (!this->page_buffer_virt)
		goto error_alloc;

	this->raw_buffer = kzalloc(mtd->writesize + mtd->oobsize, GFP_KERNEL);
	if (!this->raw_buffer)
		goto error_alloc;

	/* Slice up the page buffer. */
	this->payload_virt = this->page_buffer_virt;
	this->payload_phys = this->page_buffer_phys;
	this->auxiliary_virt = this->payload_virt + geo->payload_size;
	this->auxiliary_phys = this->payload_phys + geo->payload_size;
	return 0;

error_alloc:
	gpmi_free_dma_buffer(this);
	return -ENOMEM;
}

static void gpmi_cmd_ctrl(struct mtd_info *mtd, int data, unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;
	int ret;

	/*
	 * Every operation begins with a command byte and a series of zero or
	 * more address bytes. These are distinguished by either the Address
	 * Latch Enable (ALE) or Command Latch Enable (CLE) signals being
	 * asserted. When MTD is ready to execute the command, it will deassert
	 * both latch enables.
	 *
	 * Rather than run a separate DMA operation for every single byte, we
	 * queue them up and run a single DMA operation for the entire series
	 * of command and data bytes. NAND_CMD_NONE means the END of the queue.
	 */
	if ((ctrl & (NAND_ALE | NAND_CLE))) {
		if (data != NAND_CMD_NONE)
			this->cmd_buffer[this->command_length++] = data;
		return;
	}

	if (!this->command_length)
		return;

	ret = gpmi_send_command(this);
	if (ret)
		dev_err(this->dev, "Chip: %u, Error %d\n",
			this->current_chip, ret);

	this->command_length = 0;
}

static int gpmi_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;

	return gpmi_is_ready(this, this->current_chip);
}

static void gpmi_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;

	if ((this->current_chip < 0) && (chipnr >= 0))
		gpmi_begin(this);
	else if ((this->current_chip >= 0) && (chipnr < 0))
		gpmi_end(this);

	this->current_chip = chipnr;
}

static void gpmi_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;

	dev_dbg(this->dev, "len is %d\n", len);
	this->upper_buf	= buf;
	this->upper_len	= len;

	gpmi_read_data(this);
}

static void gpmi_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;

	dev_dbg(this->dev, "len is %d\n", len);
	this->upper_buf	= (uint8_t *)buf;
	this->upper_len	= len;

	gpmi_send_data(this);
}

static uint8_t gpmi_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;
	uint8_t *buf = this->data_buffer_dma;

	gpmi_read_buf(mtd, buf, 1);
	return buf[0];
}

/*
 * Handles block mark swapping.
 * It can be called in swapping the block mark, or swapping it back,
 * because the the operations are the same.
 */
static void block_mark_swapping(struct gpmi_nand_data *this,
				void *payload, void *auxiliary)
{
	struct bch_geometry *nfc_geo = &this->bch_geometry;
	unsigned char *p;
	unsigned char *a;
	unsigned int  bit;
	unsigned char mask;
	unsigned char from_data;
	unsigned char from_oob;

	if (!this->swap_block_mark)
		return;

	/*
	 * If control arrives here, we're swapping. Make some convenience
	 * variables.
	 */
	bit = nfc_geo->block_mark_bit_offset;
	p   = payload + nfc_geo->block_mark_byte_offset;
	a   = auxiliary;

	/*
	 * Get the byte from the data area that overlays the block mark. Since
	 * the ECC engine applies its own view to the bits in the page, the
	 * physical block mark won't (in general) appear on a byte boundary in
	 * the data.
	 */
	from_data = (p[0] >> bit) | (p[1] << (8 - bit));

	/* Get the byte from the OOB. */
	from_oob = a[0];

	/* Swap them. */
	a[0] = from_data;

	mask = (0x1 << bit) - 1;
	p[0] = (p[0] & mask) | (from_oob << bit);

	mask = ~0 << bit;
	p[1] = (p[1] & mask) | (from_oob >> (8 - bit));
}

static int gpmi_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct gpmi_nand_data *this = chip->priv;
	struct bch_geometry *nfc_geo = &this->bch_geometry;
	void          *payload_virt;
	dma_addr_t    payload_phys;
	void          *auxiliary_virt;
	dma_addr_t    auxiliary_phys;
	unsigned int  i;
	unsigned char *status;
	unsigned int  max_bitflips = 0;
	int           ret;

	dev_dbg(this->dev, "page number is : %d\n", page);
	ret = read_page_prepare(this, buf, nfc_geo->payload_size,
					this->payload_virt, this->payload_phys,
					nfc_geo->payload_size,
					&payload_virt, &payload_phys);
	if (ret) {
		dev_err(this->dev, "Inadequate DMA buffer\n");
		ret = -ENOMEM;
		return ret;
	}
	auxiliary_virt = this->auxiliary_virt;
	auxiliary_phys = this->auxiliary_phys;

	/* go! */
	ret = gpmi_read_page(this, payload_phys, auxiliary_phys);
	read_page_end(this, buf, nfc_geo->payload_size,
			this->payload_virt, this->payload_phys,
			nfc_geo->payload_size,
			payload_virt, payload_phys);
	if (ret) {
		dev_err(this->dev, "Error in ECC-based read: %d\n", ret);
		return ret;
	}

	/* Loop over status bytes, accumulating ECC status. */
	status = auxiliary_virt + nfc_geo->auxiliary_status_offset;

	for (i = 0; i < nfc_geo->ecc_chunk_count; i++, status++) {
		if ((*status == STATUS_GOOD) || (*status == STATUS_ERASED))
			continue;

		if (*status == STATUS_UNCORRECTABLE) {
			mtd->ecc_stats.failed++;
			continue;
		}
		mtd->ecc_stats.corrected += *status;
		max_bitflips = max_t(unsigned int, max_bitflips, *status);
	}

	/* handle the block mark swapping */
	block_mark_swapping(this, buf, auxiliary_virt);

	if (oob_required) {
		/*
		 * It's time to deliver the OOB bytes. See gpmi_ecc_read_oob()
		 * for details about our policy for delivering the OOB.
		 *
		 * We fill the caller's buffer with set bits, and then copy the
		 * block mark to th caller's buffer. Note that, if block mark
		 * swapping was necessary, it has already been done, so we can
		 * rely on the first byte of the auxiliary buffer to contain
		 * the block mark.
		 */
		memset(chip->oob_poi, ~0, mtd->oobsize);
		chip->oob_poi[0] = ((uint8_t *) auxiliary_virt)[0];
	}

	read_page_swap_end(this, buf, nfc_geo->payload_size,
			this->payload_virt, this->payload_phys,
			nfc_geo->payload_size,
			payload_virt, payload_phys);

	return max_bitflips;
}

/* Fake a virtual small page for the subpage read */
static int gpmi_ecc_read_subpage(struct mtd_info *mtd, struct nand_chip *chip,
			uint32_t offs, uint32_t len, uint8_t *buf, int page)
{
	struct gpmi_nand_data *this = chip->priv;
	void __iomem *bch_regs = this->resources.bch_regs;
	struct bch_geometry old_geo = this->bch_geometry;
	struct bch_geometry *geo = &this->bch_geometry;
	int size = chip->ecc.size; /* ECC chunk size */
	int meta, n, page_size;
	u32 r1_old, r2_old, r1_new, r2_new;
	unsigned int max_bitflips;
	int first, last, marker_pos;
	int ecc_parity_size;
	int col = 0;
	int old_swap_block_mark = this->swap_block_mark;

	/* The size of ECC parity */
	ecc_parity_size = geo->gf_len * geo->ecc_strength / 8;

	/* Align it with the chunk size */
	first = offs / size;
	last = (offs + len - 1) / size;

	if (this->swap_block_mark) {
		/*
		 * Find the chunk which contains the Block Marker.
		 * If this chunk is in the range of [first, last],
		 * we have to read out the whole page.
		 * Why? since we had swapped the data at the position of Block
		 * Marker to the metadata which is bound with the chunk 0.
		 */
		marker_pos = geo->block_mark_byte_offset / size;
		if (last >= marker_pos && first <= marker_pos) {
			dev_dbg(this->dev,
				"page:%d, first:%d, last:%d, marker at:%d\n",
				page, first, last, marker_pos);
			return gpmi_ecc_read_page(mtd, chip, buf, 0, page);
		}
	}

	meta = geo->metadata_size;
	if (first) {
		col = meta + (size + ecc_parity_size) * first;
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);

		meta = 0;
		buf = buf + first * size;
	}

	/* Save the old environment */
	r1_old = r1_new = readl(bch_regs + HW_BCH_FLASH0LAYOUT0);
	r2_old = r2_new = readl(bch_regs + HW_BCH_FLASH0LAYOUT1);

	/* change the BCH registers and bch_geometry{} */
	n = last - first + 1;
	page_size = meta + (size + ecc_parity_size) * n;

	r1_new &= ~(BM_BCH_FLASH0LAYOUT0_NBLOCKS |
			BM_BCH_FLASH0LAYOUT0_META_SIZE);
	r1_new |= BF_BCH_FLASH0LAYOUT0_NBLOCKS(n - 1)
			| BF_BCH_FLASH0LAYOUT0_META_SIZE(meta);
	writel(r1_new, bch_regs + HW_BCH_FLASH0LAYOUT0);

	r2_new &= ~BM_BCH_FLASH0LAYOUT1_PAGE_SIZE;
	r2_new |= BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(page_size);
	writel(r2_new, bch_regs + HW_BCH_FLASH0LAYOUT1);

	geo->ecc_chunk_count = n;
	geo->payload_size = n * size;
	geo->page_size = page_size;
	geo->auxiliary_status_offset = ALIGN(meta, 4);

	dev_dbg(this->dev, "page:%d(%d:%d)%d, chunk:(%d:%d), BCH PG size:%d\n",
		page, offs, len, col, first, n, page_size);

	/* Read the subpage now */
	this->swap_block_mark = false;
	max_bitflips = gpmi_ecc_read_page(mtd, chip, buf, 0, page);

	/* Restore */
	writel(r1_old, bch_regs + HW_BCH_FLASH0LAYOUT0);
	writel(r2_old, bch_regs + HW_BCH_FLASH0LAYOUT1);
	this->bch_geometry = old_geo;
	this->swap_block_mark = old_swap_block_mark;

	return max_bitflips;
}

static int gpmi_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required)
{
	struct gpmi_nand_data *this = chip->priv;
	struct bch_geometry *nfc_geo = &this->bch_geometry;
	const void *payload_virt;
	dma_addr_t payload_phys;
	const void *auxiliary_virt;
	dma_addr_t auxiliary_phys;
	int        ret;

	dev_dbg(this->dev, "ecc write page.\n");
	if (this->swap_block_mark) {
		/*
		 * If control arrives here, we're doing block mark swapping.
		 * Since we can't modify the caller's buffers, we must copy them
		 * into our own.
		 */
		memcpy(this->payload_virt, buf, mtd->writesize);
		payload_virt = this->payload_virt;
		payload_phys = this->payload_phys;

		memcpy(this->auxiliary_virt, chip->oob_poi,
				nfc_geo->auxiliary_size);
		auxiliary_virt = this->auxiliary_virt;
		auxiliary_phys = this->auxiliary_phys;

		/* Handle block mark swapping. */
		block_mark_swapping(this,
				(void *)payload_virt, (void *)auxiliary_virt);
	} else {
		/*
		 * If control arrives here, we're not doing block mark swapping,
		 * so we can to try and use the caller's buffers.
		 */
		ret = send_page_prepare(this,
				buf, mtd->writesize,
				this->payload_virt, this->payload_phys,
				nfc_geo->payload_size,
				&payload_virt, &payload_phys);
		if (ret) {
			dev_err(this->dev, "Inadequate payload DMA buffer\n");
			return 0;
		}

		ret = send_page_prepare(this,
				chip->oob_poi, mtd->oobsize,
				this->auxiliary_virt, this->auxiliary_phys,
				nfc_geo->auxiliary_size,
				&auxiliary_virt, &auxiliary_phys);
		if (ret) {
			dev_err(this->dev, "Inadequate auxiliary DMA buffer\n");
			goto exit_auxiliary;
		}
	}

	/* Ask the NFC. */
	ret = gpmi_send_page(this, payload_phys, auxiliary_phys);
	if (ret)
		dev_err(this->dev, "Error in ECC-based write: %d\n", ret);

	if (!this->swap_block_mark) {
		send_page_end(this, chip->oob_poi, mtd->oobsize,
				this->auxiliary_virt, this->auxiliary_phys,
				nfc_geo->auxiliary_size,
				auxiliary_virt, auxiliary_phys);
exit_auxiliary:
		send_page_end(this, buf, mtd->writesize,
				this->payload_virt, this->payload_phys,
				nfc_geo->payload_size,
				payload_virt, payload_phys);
	}

	return 0;
}

/*
 * There are several places in this driver where we have to handle the OOB and
 * block marks. This is the function where things are the most complicated, so
 * this is where we try to explain it all. All the other places refer back to
 * here.
 *
 * These are the rules, in order of decreasing importance:
 *
 * 1) Nothing the caller does can be allowed to imperil the block mark.
 *
 * 2) In read operations, the first byte of the OOB we return must reflect the
 *    true state of the block mark, no matter where that block mark appears in
 *    the physical page.
 *
 * 3) ECC-based read operations return an OOB full of set bits (since we never
 *    allow ECC-based writes to the OOB, it doesn't matter what ECC-based reads
 *    return).
 *
 * 4) "Raw" read operations return a direct view of the physical bytes in the
 *    page, using the conventional definition of which bytes are data and which
 *    are OOB. This gives the caller a way to see the actual, physical bytes
 *    in the page, without the distortions applied by our ECC engine.
 *
 *
 * What we do for this specific read operation depends on two questions:
 *
 * 1) Are we doing a "raw" read, or an ECC-based read?
 *
 * 2) Are we using block mark swapping or transcription?
 *
 * There are four cases, illustrated by the following Karnaugh map:
 *
 *                    |           Raw           |         ECC-based       |
 *       -------------+-------------------------+-------------------------+
 *                    | Read the conventional   |                         |
 *                    | OOB at the end of the   |                         |
 *       Swapping     | page and return it. It  |                         |
 *                    | contains exactly what   |                         |
 *                    | we want.                | Read the block mark and |
 *       -------------+-------------------------+ return it in a buffer   |
 *                    | Read the conventional   | full of set bits.       |
 *                    | OOB at the end of the   |                         |
 *                    | page and also the block |                         |
 *       Transcribing | mark in the metadata.   |                         |
 *                    | Copy the block mark     |                         |
 *                    | into the first byte of  |                         |
 *                    | the OOB.                |                         |
 *       -------------+-------------------------+-------------------------+
 *
 * Note that we break rule #4 in the Transcribing/Raw case because we're not
 * giving an accurate view of the actual, physical bytes in the page (we're
 * overwriting the block mark). That's OK because it's more important to follow
 * rule #2.
 *
 * It turns out that knowing whether we want an "ECC-based" or "raw" read is not
 * easy. When reading a page, for example, the NAND Flash MTD code calls our
 * ecc.read_page or ecc.read_page_raw function. Thus, the fact that MTD wants an
 * ECC-based or raw view of the page is implicit in which function it calls
 * (there is a similar pair of ECC-based/raw functions for writing).
 */
static int gpmi_ecc_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
				int page)
{
	struct gpmi_nand_data *this = chip->priv;

	dev_dbg(this->dev, "page number is %d\n", page);
	/* clear the OOB buffer */
	memset(chip->oob_poi, ~0, mtd->oobsize);

	/* Read out the conventional OOB. */
	chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	/*
	 * Now, we want to make sure the block mark is correct. In the
	 * non-transcribing case (!GPMI_IS_MX23()), we already have it.
	 * Otherwise, we need to explicitly read it.
	 */
	if (GPMI_IS_MX23(this)) {
		/* Read the block mark into the first byte of the OOB buffer. */
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		chip->oob_poi[0] = chip->read_byte(mtd);
	}

	return 0;
}

static int
gpmi_ecc_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	struct nand_oobfree *of = mtd->ecclayout->oobfree;
	int status = 0;

	/* Do we have available oob area? */
	if (!of->length)
		return -EPERM;

	if (!nand_is_slc(chip))
		return -EPERM;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize + of->offset, page);
	chip->write_buf(mtd, chip->oob_poi + of->offset, of->length);
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);
	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/*
 * This function reads a NAND page without involving the ECC engine (no HW
 * ECC correction).
 * The tricky part in the GPMI/BCH controller is that it stores ECC bits
 * inline (interleaved with payload DATA), and do not align data chunk on
 * byte boundaries.
 * We thus need to take care moving the payload data and ECC bits stored in the
 * page into the provided buffers, which is why we're using gpmi_copy_bits.
 *
 * See set_geometry_by_ecc_info inline comments to have a full description
 * of the layout used by the GPMI controller.
 */
static int gpmi_ecc_read_page_raw(struct mtd_info *mtd,
				  struct nand_chip *chip, uint8_t *buf,
				  int oob_required, int page)
{
	struct gpmi_nand_data *this = chip->priv;
	struct bch_geometry *nfc_geo = &this->bch_geometry;
	int eccsize = nfc_geo->ecc_chunk_size;
	int eccbits = nfc_geo->ecc_strength * nfc_geo->gf_len;
	u8 *tmp_buf = this->raw_buffer;
	size_t src_bit_off;
	size_t oob_bit_off;
	size_t oob_byte_off;
	uint8_t *oob = chip->oob_poi;
	int step;

	chip->read_buf(mtd, tmp_buf,
		       mtd->writesize + mtd->oobsize);

	/*
	 * If required, swap the bad block marker and the data stored in the
	 * metadata section, so that we don't wrongly consider a block as bad.
	 *
	 * See the layout description for a detailed explanation on why this
	 * is needed.
	 */
	if (this->swap_block_mark) {
		u8 swap = tmp_buf[0];

		tmp_buf[0] = tmp_buf[mtd->writesize];
		tmp_buf[mtd->writesize] = swap;
	}

	/*
	 * Copy the metadata section into the oob buffer (this section is
	 * guaranteed to be aligned on a byte boundary).
	 */
	if (oob_required)
		memcpy(oob, tmp_buf, nfc_geo->metadata_size);

	oob_bit_off = nfc_geo->metadata_size * 8;
	src_bit_off = oob_bit_off;

	/* Extract interleaved payload data and ECC bits */
	for (step = 0; step < nfc_geo->ecc_chunk_count; step++) {
		if (buf)
			gpmi_copy_bits(buf, step * eccsize * 8,
				       tmp_buf, src_bit_off,
				       eccsize * 8);
		src_bit_off += eccsize * 8;

		/* Align last ECC block to align a byte boundary */
		if (step == nfc_geo->ecc_chunk_count - 1 &&
		    (oob_bit_off + eccbits) % 8)
			eccbits += 8 - ((oob_bit_off + eccbits) % 8);

		if (oob_required)
			gpmi_copy_bits(oob, oob_bit_off,
				       tmp_buf, src_bit_off,
				       eccbits);

		src_bit_off += eccbits;
		oob_bit_off += eccbits;
	}

	if (oob_required) {
		oob_byte_off = oob_bit_off / 8;

		if (oob_byte_off < mtd->oobsize)
			memcpy(oob + oob_byte_off,
			       tmp_buf + mtd->writesize + oob_byte_off,
			       mtd->oobsize - oob_byte_off);
	}

	return 0;
}

/*
 * This function writes a NAND page without involving the ECC engine (no HW
 * ECC generation).
 * The tricky part in the GPMI/BCH controller is that it stores ECC bits
 * inline (interleaved with payload DATA), and do not align data chunk on
 * byte boundaries.
 * We thus need to take care moving the OOB area at the right place in the
 * final page, which is why we're using gpmi_copy_bits.
 *
 * See set_geometry_by_ecc_info inline comments to have a full description
 * of the layout used by the GPMI controller.
 */
static int gpmi_ecc_write_page_raw(struct mtd_info *mtd,
				   struct nand_chip *chip,
				   const uint8_t *buf,
				   int oob_required)
{
	struct gpmi_nand_data *this = chip->priv;
	struct bch_geometry *nfc_geo = &this->bch_geometry;
	int eccsize = nfc_geo->ecc_chunk_size;
	int eccbits = nfc_geo->ecc_strength * nfc_geo->gf_len;
	u8 *tmp_buf = this->raw_buffer;
	uint8_t *oob = chip->oob_poi;
	size_t dst_bit_off;
	size_t oob_bit_off;
	size_t oob_byte_off;
	int step;

	/*
	 * Initialize all bits to 1 in case we don't have a buffer for the
	 * payload or oob data in order to leave unspecified bits of data
	 * to their initial state.
	 */
	if (!buf || !oob_required)
		memset(tmp_buf, 0xff, mtd->writesize + mtd->oobsize);

	/*
	 * First copy the metadata section (stored in oob buffer) at the
	 * beginning of the page, as imposed by the GPMI layout.
	 */
	memcpy(tmp_buf, oob, nfc_geo->metadata_size);
	oob_bit_off = nfc_geo->metadata_size * 8;
	dst_bit_off = oob_bit_off;

	/* Interleave payload data and ECC bits */
	for (step = 0; step < nfc_geo->ecc_chunk_count; step++) {
		if (buf)
			gpmi_copy_bits(tmp_buf, dst_bit_off,
				       buf, step * eccsize * 8, eccsize * 8);
		dst_bit_off += eccsize * 8;

		/* Align last ECC block to align a byte boundary */
		if (step == nfc_geo->ecc_chunk_count - 1 &&
		    (oob_bit_off + eccbits) % 8)
			eccbits += 8 - ((oob_bit_off + eccbits) % 8);

		if (oob_required)
			gpmi_copy_bits(tmp_buf, dst_bit_off,
				       oob, oob_bit_off, eccbits);

		dst_bit_off += eccbits;
		oob_bit_off += eccbits;
	}

	oob_byte_off = oob_bit_off / 8;

	if (oob_required && oob_byte_off < mtd->oobsize)
		memcpy(tmp_buf + mtd->writesize + oob_byte_off,
		       oob + oob_byte_off, mtd->oobsize - oob_byte_off);

	/*
	 * If required, swap the bad block marker and the first byte of the
	 * metadata section, so that we don't modify the bad block marker.
	 *
	 * See the layout description for a detailed explanation on why this
	 * is needed.
	 */
	if (this->swap_block_mark) {
		u8 swap = tmp_buf[0];

		tmp_buf[0] = tmp_buf[mtd->writesize];
		tmp_buf[mtd->writesize] = swap;
	}

	chip->write_buf(mtd, tmp_buf, mtd->writesize + mtd->oobsize);

	return 0;
}

static int gpmi_ecc_read_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				 int page)
{
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	return gpmi_ecc_read_page_raw(mtd, chip, NULL, 1, page);
}

static int gpmi_ecc_write_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				 int page)
{
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0, page);

	return gpmi_ecc_write_page_raw(mtd, chip, NULL, 1);
}

static int gpmi_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	struct gpmi_nand_data *this = chip->priv;
	int ret = 0;
	uint8_t *block_mark;
	int column, page, status, chipnr;

	chipnr = (int)(ofs >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	column = !GPMI_IS_MX23(this) ? mtd->writesize : 0;

	/* Write the block mark. */
	block_mark = this->data_buffer_dma;
	block_mark[0] = 0; /* bad block marker */

	/* Shift to get page */
	page = (int)(ofs >> chip->page_shift);

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, column, page);
	chip->write_buf(mtd, block_mark, 1);
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);
	if (status & NAND_STATUS_FAIL)
		ret = -EIO;

	chip->select_chip(mtd, -1);

	return ret;
}

static int nand_boot_set_geometry(struct gpmi_nand_data *this)
{
	struct boot_rom_geometry *geometry = &this->rom_geometry;

	/*
	 * Set the boot block stride size.
	 *
	 * In principle, we should be reading this from the OTP bits, since
	 * that's where the ROM is going to get it. In fact, we don't have any
	 * way to read the OTP bits, so we go with the default and hope for the
	 * best.
	 */
	geometry->stride_size_in_pages = 64;

	/*
	 * Set the search area stride exponent.
	 *
	 * In principle, we should be reading this from the OTP bits, since
	 * that's where the ROM is going to get it. In fact, we don't have any
	 * way to read the OTP bits, so we go with the default and hope for the
	 * best.
	 */
	geometry->search_area_stride_exponent = 2;
	return 0;
}

static const char  *fingerprint = "STMP";
static int mx23_check_transcription_stamp(struct gpmi_nand_data *this)
{
	struct boot_rom_geometry *rom_geo = &this->rom_geometry;
	struct device *dev = this->dev;
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->nand;
	unsigned int search_area_size_in_strides;
	unsigned int stride;
	unsigned int page;
	uint8_t *buffer = chip->buffers->databuf;
	int saved_chip_number;
	int found_an_ncb_fingerprint = false;

	/* Compute the number of strides in a search area. */
	search_area_size_in_strides = 1 << rom_geo->search_area_stride_exponent;

	saved_chip_number = this->current_chip;
	chip->select_chip(mtd, 0);

	/*
	 * Loop through the first search area, looking for the NCB fingerprint.
	 */
	dev_dbg(dev, "Scanning for an NCB fingerprint...\n");

	for (stride = 0; stride < search_area_size_in_strides; stride++) {
		/* Compute the page addresses. */
		page = stride * rom_geo->stride_size_in_pages;

		dev_dbg(dev, "Looking for a fingerprint in page 0x%x\n", page);

		/*
		 * Read the NCB fingerprint. The fingerprint is four bytes long
		 * and starts in the 12th byte of the page.
		 */
		chip->cmdfunc(mtd, NAND_CMD_READ0, 12, page);
		chip->read_buf(mtd, buffer, strlen(fingerprint));

		/* Look for the fingerprint. */
		if (!memcmp(buffer, fingerprint, strlen(fingerprint))) {
			found_an_ncb_fingerprint = true;
			break;
		}

	}

	chip->select_chip(mtd, saved_chip_number);

	if (found_an_ncb_fingerprint)
		dev_dbg(dev, "\tFound a fingerprint\n");
	else
		dev_dbg(dev, "\tNo fingerprint found\n");
	return found_an_ncb_fingerprint;
}

/* Writes a transcription stamp. */
static int mx23_write_transcription_stamp(struct gpmi_nand_data *this)
{
	struct device *dev = this->dev;
	struct boot_rom_geometry *rom_geo = &this->rom_geometry;
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->nand;
	unsigned int block_size_in_pages;
	unsigned int search_area_size_in_strides;
	unsigned int search_area_size_in_pages;
	unsigned int search_area_size_in_blocks;
	unsigned int block;
	unsigned int stride;
	unsigned int page;
	uint8_t      *buffer = chip->buffers->databuf;
	int saved_chip_number;
	int status;

	/* Compute the search area geometry. */
	block_size_in_pages = mtd->erasesize / mtd->writesize;
	search_area_size_in_strides = 1 << rom_geo->search_area_stride_exponent;
	search_area_size_in_pages = search_area_size_in_strides *
					rom_geo->stride_size_in_pages;
	search_area_size_in_blocks =
		  (search_area_size_in_pages + (block_size_in_pages - 1)) /
				    block_size_in_pages;

	dev_dbg(dev, "Search Area Geometry :\n");
	dev_dbg(dev, "\tin Blocks : %u\n", search_area_size_in_blocks);
	dev_dbg(dev, "\tin Strides: %u\n", search_area_size_in_strides);
	dev_dbg(dev, "\tin Pages  : %u\n", search_area_size_in_pages);

	/* Select chip 0. */
	saved_chip_number = this->current_chip;
	chip->select_chip(mtd, 0);

	/* Loop over blocks in the first search area, erasing them. */
	dev_dbg(dev, "Erasing the search area...\n");

	for (block = 0; block < search_area_size_in_blocks; block++) {
		/* Compute the page address. */
		page = block * block_size_in_pages;

		/* Erase this block. */
		dev_dbg(dev, "\tErasing block 0x%x\n", block);
		chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page);
		chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);

		/* Wait for the erase to finish. */
		status = chip->waitfunc(mtd, chip);
		if (status & NAND_STATUS_FAIL)
			dev_err(dev, "[%s] Erase failed.\n", __func__);
	}

	/* Write the NCB fingerprint into the page buffer. */
	memset(buffer, ~0, mtd->writesize);
	memcpy(buffer + 12, fingerprint, strlen(fingerprint));

	/* Loop through the first search area, writing NCB fingerprints. */
	dev_dbg(dev, "Writing NCB fingerprints...\n");
	for (stride = 0; stride < search_area_size_in_strides; stride++) {
		/* Compute the page addresses. */
		page = stride * rom_geo->stride_size_in_pages;

		/* Write the first page of the current stride. */
		dev_dbg(dev, "Writing an NCB fingerprint in page 0x%x\n", page);
		chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);
		chip->ecc.write_page_raw(mtd, chip, buffer, 0);
		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

		/* Wait for the write to finish. */
		status = chip->waitfunc(mtd, chip);
		if (status & NAND_STATUS_FAIL)
			dev_err(dev, "[%s] Write failed.\n", __func__);
	}

	/* Deselect chip 0. */
	chip->select_chip(mtd, saved_chip_number);
	return 0;
}

static int mx23_boot_init(struct gpmi_nand_data  *this)
{
	struct device *dev = this->dev;
	struct nand_chip *chip = &this->nand;
	struct mtd_info *mtd = &this->mtd;
	unsigned int block_count;
	unsigned int block;
	int     chipnr;
	int     page;
	loff_t  byte;
	uint8_t block_mark;
	int     ret = 0;

	/*
	 * If control arrives here, we can't use block mark swapping, which
	 * means we're forced to use transcription. First, scan for the
	 * transcription stamp. If we find it, then we don't have to do
	 * anything -- the block marks are already transcribed.
	 */
	if (mx23_check_transcription_stamp(this))
		return 0;

	/*
	 * If control arrives here, we couldn't find a transcription stamp, so
	 * so we presume the block marks are in the conventional location.
	 */
	dev_dbg(dev, "Transcribing bad block marks...\n");

	/* Compute the number of blocks in the entire medium. */
	block_count = chip->chipsize >> chip->phys_erase_shift;

	/*
	 * Loop over all the blocks in the medium, transcribing block marks as
	 * we go.
	 */
	for (block = 0; block < block_count; block++) {
		/*
		 * Compute the chip, page and byte addresses for this block's
		 * conventional mark.
		 */
		chipnr = block >> (chip->chip_shift - chip->phys_erase_shift);
		page = block << (chip->phys_erase_shift - chip->page_shift);
		byte = block <<  chip->phys_erase_shift;

		/* Send the command to read the conventional block mark. */
		chip->select_chip(mtd, chipnr);
		chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
		block_mark = chip->read_byte(mtd);
		chip->select_chip(mtd, -1);

		/*
		 * Check if the block is marked bad. If so, we need to mark it
		 * again, but this time the result will be a mark in the
		 * location where we transcribe block marks.
		 */
		if (block_mark != 0xff) {
			dev_dbg(dev, "Transcribing mark in block %u\n", block);
			ret = chip->block_markbad(mtd, byte);
			if (ret)
				dev_err(dev,
					"Failed to mark block bad with ret %d\n",
					ret);
		}
	}

	/* Write the stamp that indicates we've transcribed the block marks. */
	mx23_write_transcription_stamp(this);
	return 0;
}

static int nand_boot_init(struct gpmi_nand_data  *this)
{
	nand_boot_set_geometry(this);

	/* This is ROM arch-specific initilization before the BBT scanning. */
	if (GPMI_IS_MX23(this))
		return mx23_boot_init(this);
	return 0;
}

static int gpmi_set_geometry(struct gpmi_nand_data *this)
{
	int ret;

	/* Free the temporary DMA memory for reading ID. */
	gpmi_free_dma_buffer(this);

	/* Set up the NFC geometry which is used by BCH. */
	ret = bch_set_geometry(this);
	if (ret) {
		dev_err(this->dev, "Error setting BCH geometry : %d\n", ret);
		return ret;
	}

	/* Alloc the new DMA buffers according to the pagesize and oobsize */
	return gpmi_alloc_dma_buffer(this);
}

static void gpmi_nand_exit(struct gpmi_nand_data *this)
{
	nand_release(&this->mtd);
	gpmi_free_dma_buffer(this);
}

static int gpmi_init_last(struct gpmi_nand_data *this)
{
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = mtd->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	struct bch_geometry *bch_geo = &this->bch_geometry;
	int ret;

	/* Set up the medium geometry */
	ret = gpmi_set_geometry(this);
	if (ret)
		return ret;

	/* Init the nand_ecc_ctrl{} */
	ecc->read_page	= gpmi_ecc_read_page;
	ecc->write_page	= gpmi_ecc_write_page;
	ecc->read_oob	= gpmi_ecc_read_oob;
	ecc->write_oob	= gpmi_ecc_write_oob;
	ecc->read_page_raw = gpmi_ecc_read_page_raw;
	ecc->write_page_raw = gpmi_ecc_write_page_raw;
	ecc->read_oob_raw = gpmi_ecc_read_oob_raw;
	ecc->write_oob_raw = gpmi_ecc_write_oob_raw;
	ecc->mode	= NAND_ECC_HW;
	ecc->size	= bch_geo->ecc_chunk_size;
	ecc->strength	= bch_geo->ecc_strength;
	ecc->layout	= &gpmi_hw_ecclayout;

	/*
	 * We only enable the subpage read when:
	 *  (1) the chip is imx6, and
	 *  (2) the size of the ECC parity is byte aligned.
	 */
	if (GPMI_IS_MX6(this) &&
		((bch_geo->gf_len * bch_geo->ecc_strength) % 8) == 0) {
		ecc->read_subpage = gpmi_ecc_read_subpage;
		chip->options |= NAND_SUBPAGE_READ;
	}

	/*
	 * Can we enable the extra features? such as EDO or Sync mode.
	 *
	 * We do not check the return value now. That's means if we fail in
	 * enable the extra features, we still can run in the normal way.
	 */
	gpmi_extra_init(this);

	return 0;
}

static int gpmi_nand_init(struct gpmi_nand_data *this)
{
	struct mtd_info  *mtd = &this->mtd;
	struct nand_chip *chip = &this->nand;
	struct mtd_part_parser_data ppdata = {};
	int ret;

	/* init current chip */
	this->current_chip	= -1;

	/* init the MTD data structures */
	mtd->priv		= chip;
	mtd->name		= "gpmi-nand";
	mtd->owner		= THIS_MODULE;

	/* init the nand_chip{}, we don't support a 16-bit NAND Flash bus. */
	chip->priv		= this;
	chip->select_chip	= gpmi_select_chip;
	chip->cmd_ctrl		= gpmi_cmd_ctrl;
	chip->dev_ready		= gpmi_dev_ready;
	chip->read_byte		= gpmi_read_byte;
	chip->read_buf		= gpmi_read_buf;
	chip->write_buf		= gpmi_write_buf;
	chip->badblock_pattern	= &gpmi_bbt_descr;
	chip->block_markbad	= gpmi_block_markbad;
	chip->options		|= NAND_NO_SUBPAGE_WRITE;

	/* Set up swap_block_mark, must be set before the gpmi_set_geometry() */
	this->swap_block_mark = !GPMI_IS_MX23(this);

	if (of_get_nand_on_flash_bbt(this->dev->of_node)) {
		chip->bbt_options |= NAND_BBT_USE_FLASH | NAND_BBT_NO_OOB;

		if (of_property_read_bool(this->dev->of_node,
						"fsl,no-blockmark-swap"))
			this->swap_block_mark = false;
	}
	dev_dbg(this->dev, "Blockmark swapping %sabled\n",
		this->swap_block_mark ? "en" : "dis");

	/*
	 * Allocate a temporary DMA buffer for reading ID in the
	 * nand_scan_ident().
	 */
	this->bch_geometry.payload_size = 1024;
	this->bch_geometry.auxiliary_size = 128;
	ret = gpmi_alloc_dma_buffer(this);
	if (ret)
		goto err_out;

	ret = nand_scan_ident(mtd, GPMI_IS_MX6(this) ? 2 : 1, NULL);
	if (ret)
		goto err_out;

	ret = gpmi_init_last(this);
	if (ret)
		goto err_out;

	chip->options |= NAND_SKIP_BBTSCAN;
	ret = nand_scan_tail(mtd);
	if (ret)
		goto err_out;

	ret = nand_boot_init(this);
	if (ret)
		goto err_out;
	ret = chip->scan_bbt(mtd);
	if (ret)
		goto err_out;

	ppdata.of_node = this->pdev->dev.of_node;
	ret = mtd_device_parse_register(mtd, NULL, &ppdata, NULL, 0);
	if (ret)
		goto err_out;
	return 0;

err_out:
	gpmi_nand_exit(this);
	return ret;
}

static const struct of_device_id gpmi_nand_id_table[] = {
	{
		.compatible = "fsl,imx23-gpmi-nand",
		.data = &gpmi_devdata_imx23,
	}, {
		.compatible = "fsl,imx28-gpmi-nand",
		.data = &gpmi_devdata_imx28,
	}, {
		.compatible = "fsl,imx6q-gpmi-nand",
		.data = &gpmi_devdata_imx6q,
	}, {
		.compatible = "fsl,imx6sx-gpmi-nand",
		.data = &gpmi_devdata_imx6sx,
	}, {}
};
MODULE_DEVICE_TABLE(of, gpmi_nand_id_table);

static int gpmi_nand_probe(struct platform_device *pdev)
{
	struct gpmi_nand_data *this;
	const struct of_device_id *of_id;
	int ret;

	this = devm_kzalloc(&pdev->dev, sizeof(*this), GFP_KERNEL);
	if (!this)
		return -ENOMEM;

	of_id = of_match_device(gpmi_nand_id_table, &pdev->dev);
	if (of_id) {
		this->devdata = of_id->data;
	} else {
		dev_err(&pdev->dev, "Failed to find the right device id.\n");
		return -ENODEV;
	}

	platform_set_drvdata(pdev, this);
	this->pdev  = pdev;
	this->dev   = &pdev->dev;

	ret = acquire_resources(this);
	if (ret)
		goto exit_acquire_resources;

	ret = init_hardware(this);
	if (ret)
		goto exit_nfc_init;

	ret = gpmi_nand_init(this);
	if (ret)
		goto exit_nfc_init;

	dev_info(this->dev, "driver registered.\n");

	return 0;

exit_nfc_init:
	release_resources(this);
exit_acquire_resources:

	return ret;
}

static int gpmi_nand_remove(struct platform_device *pdev)
{
	struct gpmi_nand_data *this = platform_get_drvdata(pdev);

	gpmi_nand_exit(this);
	release_resources(this);
	return 0;
}

static struct platform_driver gpmi_nand_driver = {
	.driver = {
		.name = "gpmi-nand",
		.of_match_table = gpmi_nand_id_table,
	},
	.probe   = gpmi_nand_probe,
	.remove  = gpmi_nand_remove,
};
module_platform_driver(gpmi_nand_driver);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("i.MX GPMI NAND Flash Controller Driver");
MODULE_LICENSE("GPL");
