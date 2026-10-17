// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <uboot_aes.h>
#include <asm/io.h>
#include <asm/arch/aes.h>
#include <asm/arch/otp.h>
#include <malloc.h>

#define ONE_SECOND 0xC00000

struct npcm_aes_priv {
	struct npcm_aes_regs *regs;
};

static struct npcm_aes_priv *aes_priv;
static u8 fkeyind_to_set = 0xff;

static int second_timeout(u32 *addr, u32 bitmask, u32 bitpol)
{
	ulong time, i = 0;

	time = get_timer(0);

	/* default 1 second timeout */
	while (((readl(addr) & bitmask) == bitpol) && i < ONE_SECOND)
		i++;

	if (i == ONE_SECOND) {
		printf("%xms timeout: addr = %x, mask = %x\n", (u32)get_timer(time),
		       *addr, bitmask);
		return -1;
	}

	return 0;
}

int npcm_aes_select_key(u8 fkeyind)
{
	if (npcm_otp_is_fuse_array_disabled(NPCM_KEY_SA)) {
		printf("AES key access denied\n");
		return -EACCES;
	}

	if (fkeyind < 4)
		fkeyind_to_set = fkeyind;

	return 0;
}

static int npcm_aes_init(u8 dec_enc)
{
	struct npcm_aes_regs *regs = aes_priv->regs;
	u32 ctrl, orgctrlval, wrtimeout;

	/* reset hw */
	writel(readl(&regs->aes_sw_reset) | SW_RESET_BIT, &regs->aes_sw_reset);
	writel(readl(&regs->aes_fifo_status) | DIN_FIFO_OVERFLOW, &regs->aes_fifo_status);
	writel(readl(&regs->aes_fifo_status) | DOUT_FIFO_UNDERFLOW, &regs->aes_fifo_status);

	/* Workaround to over come Errata #648 */
	orgctrlval = readl(&regs->aes_control);
	ctrl = (0x00002004 | dec_enc);    /* AES256(CBC) */

	if (ctrl != orgctrlval) {
		writel(ctrl, &regs->aes_control);

		if (ctrl != readl(&regs->aes_control)) {
			u32 read_ctrl;
			int intwr;

			for (wrtimeout = 0; wrtimeout < 1000; wrtimeout++) {
				for (intwr = 0 ; intwr < 10; intwr++) {
					writel(ctrl, &regs->aes_control);
					writew(ctrl, (u16 *)&regs->aes_control + 1);
					/* Write configurable info in a single write operation */
					mb();
				}

				read_ctrl = readl(&regs->aes_control);
				if (ctrl == read_ctrl)
					break;
			}

			if (wrtimeout == 1000) {
				printf("\nTIMEOUT expected data=0x%x Actual AES_CONTROL data 0x%x\n\n",
				       ctrl, read_ctrl);
				return -EAGAIN;
			}

			printf("Workaround success, wrtimeout = %d\n", wrtimeout);
		}
	}

	if (second_timeout(&regs->aes_busy, AES_BUSY_BIT, AES_BUSY_BIT))
		return -EAGAIN;

	return 0;
}

static inline void npcm_aes_load_iv(u8 *iv)
{
	struct npcm_aes_regs *regs = aes_priv->regs;
	u32 *p = (u32 *)iv;
	u32 i;

	/* Initialization Vector is loaded in 32-bit chunks */
	for (i = 0; i < (SIZE_AES_BLOCK / sizeof(u32)); i++)
		writel(p[i], &regs->aes_iv_0 + i);
}

static inline void npcm_aes_load_key(u8 *key)
{
	struct npcm_aes_regs *regs = aes_priv->regs;
	u32 *p = (u32 *)key;
	u32 i;

	/* The key can be loaded either via the configuration or by using sideband
	 *  key port (aes_select_key).
	 *  If aes_select_key has been called ('fkeyind_to_set' was set to desired
	 *  key index) and no key is specified (key is NULL), we should use the
	 *  key index. Otherwise, we write the given key to the registers.
	 */
	if (!key && fkeyind_to_set < 4) {
		npcm_otp_select_key(fkeyind_to_set);

		/* Sample the new key */
		writel(readl(&regs->aes_sk) | AES_SK_BIT, &regs->aes_sk);

	} else {
		/* Initialization Vector is loaded in 32-bit chunks */
		for (i = 0; i < (2 * SIZE_AES_BLOCK / sizeof(u32)); i++)
			writel(p[i], &regs->aes_key_0 + i);

		fkeyind_to_set = 0xff;
	}
}

static inline void npcm_aes_write(u32 *in)
{
	struct npcm_aes_regs *regs = aes_priv->regs;
	u32 i;

	/* 16 Byte AES Block is written in 32-bit chunks */
	for (i = 0; i < (SIZE_AES_BLOCK / sizeof(u32)); i++)
		writel(in[i], &regs->aes_fifo_data);
}

static inline void npcm_aes_read(u32 *out)
{
	struct npcm_aes_regs *regs = aes_priv->regs;
	u32 i;

	/* Data is read in 32-bit chunks */
	for (i = 0; i < (SIZE_AES_BLOCK / sizeof(u32)); i++)
		out[i] = readl(&regs->aes_fifo_data);
}

static void npcm_aes_feed(u32 num_aes_blocks, u32 *datain, u32 *dataout)
{
	struct npcm_aes_regs *regs = aes_priv->regs;
	u32 aes_datablk;
	u32 total_blocks = num_aes_blocks;
	u32 blocks_left = num_aes_blocks;

	/* data mode */
	writel(readl(&regs->aes_busy) | AES_BUSY_BIT, &regs->aes_busy);

	/* Clear overflow and underflow */
	writel(readl(&regs->aes_fifo_status) | DIN_FIFO_OVERFLOW, &regs->aes_fifo_status);
	writel(readl(&regs->aes_fifo_status) | DOUT_FIFO_UNDERFLOW, &regs->aes_fifo_status);

	/* datain/dataout is advanced in 32-bit chunks */
	aes_datablk = (SIZE_AES_BLOCK / sizeof(u32));

	/* Quit if there is no complete blocks */
	if (total_blocks == 0)
		return;

	/* Write the first block */
	if (total_blocks > 1) {
		npcm_aes_write(datain);
		datain += aes_datablk;
		blocks_left--;
	}

	/* Write the second block */
	if (total_blocks > 2) {
		second_timeout(&regs->aes_fifo_status, DIN_FIFO_EMPTY, 0);
		npcm_aes_write(datain);
		datain += aes_datablk;
		blocks_left--;
	}

	/* Write & read available blocks */
	while (blocks_left > 0) {
		second_timeout(&regs->aes_fifo_status, DIN_FIFO_FULL, DIN_FIFO_FULL);

		/* Write next block */
		npcm_aes_write(datain);
		datain  += aes_datablk;

		/* Wait till DOUT FIFO is empty */
		second_timeout(&regs->aes_fifo_status, DOUT_FIFO_EMPTY, DOUT_FIFO_EMPTY);

		/* Read next block */
		npcm_aes_read(dataout);
		dataout += aes_datablk;

		blocks_left--;
	}

	if (total_blocks > 2) {
		second_timeout(&regs->aes_fifo_status, DOUT_FIFO_FULL, 0);

		/* Read next block */
		npcm_aes_read(dataout);
		dataout += aes_datablk;

		second_timeout(&regs->aes_fifo_status, DOUT_FIFO_FULL, 0);

		/* Read next block */
		npcm_aes_read(dataout);
		dataout += aes_datablk;
	} else if (total_blocks > 1) {
		second_timeout(&regs->aes_fifo_status, DOUT_FIFO_FULL, 0);

		/* Read next block */
		npcm_aes_read(dataout);
		dataout += aes_datablk;
	}
}

void aes_expand_key(u8 *key, u32 key_size, u8 *expkey)
{
	/* npcm hw expands the key automatically, just copy it */
	memcpy(expkey, key, SIZE_AES_BLOCK * 2);
}

void aes_cbc_encrypt_blocks(u32 key_size, u8 *key_exp, u8 *iv, u8 *src, u8 *dst,
			    u32 num_aes_blocks)
{
	if (npcm_aes_init(AES_OP_ENCRYPT))
		return;

	npcm_aes_load_iv(iv);

	npcm_aes_load_key(key_exp);

	npcm_aes_feed(num_aes_blocks, (u32 *)src, (u32 *)dst);
}

void aes_cbc_decrypt_blocks(u32 key_size, u8 *key_exp, u8 *iv, u8 *src, u8 *dst,
			    u32 num_aes_blocks)
{
	if (npcm_aes_init(AES_OP_DECRYPT))
		return;

	npcm_aes_load_iv(iv);

	npcm_aes_load_key(key_exp);

	npcm_aes_feed(num_aes_blocks, (u32 *)src, (u32 *)dst);
}

static int npcm_aes_bind(struct udevice *dev)
{
	aes_priv = calloc(1, sizeof(struct npcm_aes_priv));
	if (!aes_priv) {
		printf("%s: %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	aes_priv->regs = dev_read_addr_ptr(dev);
	if (!aes_priv->regs) {
		printf("Cannot find aes reg address, binding failed\n");
		return -EINVAL;
	}

	printf("AES: NPCM AES module bind OK\n");

	return 0;
}

static const struct udevice_id npcm_aes_ids[] = {
	{ .compatible = "nuvoton,npcm845-aes" },
	{ .compatible = "nuvoton,npcm750-aes" },
	{ }
};

U_BOOT_DRIVER(npcm_aes) = {
	.name = "npcm_aes",
	.id = UCLASS_MISC,
	.of_match = npcm_aes_ids,
	.priv_auto = sizeof(struct npcm_aes_priv),
	.bind = npcm_aes_bind,
};
