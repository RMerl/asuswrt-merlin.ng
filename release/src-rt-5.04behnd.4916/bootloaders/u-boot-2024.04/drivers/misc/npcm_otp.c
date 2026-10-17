// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fuse.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/arch/otp.h>

struct npcm_otp_priv {
	struct npcm_otp_regs *regs[2];
};

static struct npcm_otp_priv *otp_priv;

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_check_inputs                                     */
/*                                                                            */
/* Parameters:      arr - fuse array number to check                          */
/*                  word - fuse word (offset) to check                        */
/* Returns:         int                                                       */
/* Side effects:                                                              */
/* Description:     Checks is arr and word are illegal and do not exceed      */
/*                  their range. Return 0 if they are legal, -1 if not        */
/*----------------------------------------------------------------------------*/
static int npcm_otp_check_inputs(u32 arr, u32 word)
{
	if (arr >= NPCM_NUM_OF_SA) {
		if (IS_ENABLED(CONFIG_ARCH_NPCM8XX))
			printf("\nError: npcm8XX otp includs only one bank: 0\n");
		if (IS_ENABLED(CONFIG_ARCH_NPCM7xx))
			printf("\nError: npcm7XX otp includs only two banks: 0 and 1\n");
		return -1;
	}

	if (word >= NPCM_OTP_ARR_BYTE_SIZE) {
		printf("\nError: npcm otp array comprises only %d bytes, numbered from 0 to %d\n",
		       NPCM_OTP_ARR_BYTE_SIZE, NPCM_OTP_ARR_BYTE_SIZE - 1);
		return -1;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_wait_for_otp_ready                               */
/*                                                                            */
/* Parameters:      array - fuse array to wait for                            */
/* Returns:         int                                                       */
/* Side effects:                                                              */
/* Description:     Initialize the Fuse HW module.                            */
/*----------------------------------------------------------------------------*/
static int npcm_otp_wait_for_otp_ready(u32 arr, u32 timeout)
{
	struct npcm_otp_regs *regs = otp_priv->regs[arr];
	u32 time = timeout;

	/*------------------------------------------------------------------------*/
	/* check parameters validity                                              */
	/*------------------------------------------------------------------------*/
	if (arr > NPCM_FUSE_SA)
		return -EINVAL;

	while (--time > 1) {
		if (readl(&regs->fst) & FST_RDY) {
			/* fuse is ready, clear the status. */
			writel(readl(&regs->fst) | FST_RDST, &regs->fst);
			return 0;
		}
	}

	/* try to clear the status in case it was set */
	writel(readl(&regs->fst) | FST_RDST, &regs->fst);

	return -EINVAL;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_read_byte                                        */
/*                                                                            */
/* Parameters:      arr  - Storage Array type [input].                        */
/*                  addr - Byte-address to read from [input].                 */
/*                  data - Pointer to result [output].                        */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     Read 8-bit data from an OTP storage array.                */
/*----------------------------------------------------------------------------*/
static void npcm_otp_read_byte(u32 arr, u32 addr, u8 *data)
{
	struct npcm_otp_regs *regs = otp_priv->regs[arr];

	/* Wait for the Fuse Box Idle */
	npcm_otp_wait_for_otp_ready(arr, 0xDEADBEEF);

	/* Configure the byte address in the fuse array for read operation */
	writel(FADDR_VAL(addr, 0), &regs->faddr);

	/* Initiate a read cycle */
	writel(READ_INIT, &regs->fctl);

	/* Wait for read operation completion */
	npcm_otp_wait_for_otp_ready(arr, 0xDEADBEEF);

	/* Read the result */
	*data = readl(&regs->fdata) & FDATA_MASK;

	/* Clean FDATA contents to prevent unauthorized software from reading
	 * sensitive information
	 */
	writel(FDATA_CLEAN_VALUE, &regs->fdata);
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_bit_is_programmed                                */
/*                                                                            */
/* Parameters:      arr     - Storage Array type [input].                     */
/*                  byte_offset - Byte offset in array [input].               */
/*                  bit_offset  - Bit offset in byte [input].                 */
/* Returns:         Nonzero if bit is programmed, zero otherwise.             */
/* Side effects:                                                              */
/* Description:     Check if a bit is programmed in an OTP storage array.     */
/*----------------------------------------------------------------------------*/
static bool npcm_otp_bit_is_programmed(u32  arr,
				       u32 byte_offset, u8 bit_offset)
{
	u32 data = 0;

	/* Read the entire byte you wish to program */
	npcm_otp_read_byte(arr, byte_offset, (u8 *)&data);

	/* Check whether the bit is already programmed */
	if (data & (1 << bit_offset))
		return true;

	return false;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_program_bit                                      */
/*                                                                            */
/* Parameters:      arr     - Storage Array type [input].                     */
/*                  byte)offset - Byte offset in array [input].               */
/*                  bit_offset  - Bit offset in byte [input].                 */
/* Returns:         int                                                       */
/* Side effects:                                                              */
/* Description:     Program (set to 1) a bit in an OTP storage array.         */
/*----------------------------------------------------------------------------*/
static int npcm_otp_program_bit(u32 arr, u32 byte_offset,
				u8 bit_offset)
{
	struct npcm_otp_regs *regs = otp_priv->regs[arr];
	int count;
	u8 read_data;

	/* Wait for the Fuse Box Idle */
	npcm_otp_wait_for_otp_ready(arr, 0xDEADBEEF);

	/* Make sure the bit is not already programmed */
	if (npcm_otp_bit_is_programmed(arr, byte_offset, bit_offset))
		return 0;

	/* Configure the bit address in the fuse array for program operation */
	writel(FADDR_VAL(byte_offset, bit_offset), &regs->faddr);
	writel(readl(&regs->faddr) | FADDR_IN_PROG, &regs->faddr);

	// program up to MAX_PROGRAM_PULSES
	for (count = 1; count <= MAX_PROGRAM_PULSES; count++) {
		/* Initiate a program cycle */
		writel(PROGRAM_ARM, &regs->fctl);
		writel(PROGRAM_INIT, &regs->fctl);

		/* Wait for program operation completion */
		npcm_otp_wait_for_otp_ready(arr, 0xDEADBEEF);

		// after MIN_PROGRAM_PULSES start verifying the result
		if (count >= MIN_PROGRAM_PULSES) {
			/* Initiate a read cycle */
			writel(READ_INIT, &regs->fctl);

			/* Wait for read operation completion */
			npcm_otp_wait_for_otp_ready(arr, 0xDEADBEEF);

			/* Read the result */
			read_data = readl(&regs->fdata) & FDATA_MASK;

			/* If the bit is set the sequence ended correctly */
			if (read_data & (1 << bit_offset))
				break;
		}
	}

	// check if programmking failed
	if (count > MAX_PROGRAM_PULSES) {
		printf("program fail\n");
		return -EINVAL;
	}

	/*
	 * Clean FDATA contents to prevent unauthorized software from reading
	 * sensitive information
	 */
	writel(FDATA_CLEAN_VALUE, &regs->fdata);

	return 0;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_program_byte                                     */
/*                                                                            */
/* Parameters:      arr     - Storage Array type [input].                     */
/*                  byte_offset - Byte offset in array [input].               */
/*                  value   - Byte to program [input].                        */
/* Returns:         int                                                       */
/* Side effects:                                                              */
/* Description:     Program (set to 1) a given byte's relevant bits in an     */
/*                  OTP storage array.                                        */
/*----------------------------------------------------------------------------*/
static int npcm_otp_program_byte(u32 arr, u32 byte_offset,
				 u8 value)
{
	int status = 0;
	unsigned int i;
	u8 data = 0;
	int rc;

	rc = npcm_otp_check_inputs(arr, byte_offset);
	if (rc != 0)
		return rc;

	/* Wait for the Fuse Box Idle */
	npcm_otp_wait_for_otp_ready(arr, 0xDEADBEEF);

	/* Read the entire byte you wish to program */
	npcm_otp_read_byte(arr, byte_offset, &data);

	/* In case all relevant bits are already programmed - nothing to do */
	if ((~data & value) == 0)
		return status;

	/* Program unprogrammed bits. */
	for (i = 0; i < 8; i++) {
		if (value & (1 << i)) {
			/* Program (set to 1) the relevant bit */
			int last_status = npcm_otp_program_bit(arr, byte_offset, (u8)i);

			if (last_status != 0)
				status = last_status;
		}
	}
	return status;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_is_fuse_array_disabled                           */
/*                                                                            */
/* Parameters:      arr - Storage Array type [input].                         */
/* Returns:         bool                                                      */
/* Side effects:                                                              */
/* Description:     Return true if access to the first 2048 bits of the       */
/*                  specified fuse array is disabled, false if not            */
/*----------------------------------------------------------------------------*/
bool npcm_otp_is_fuse_array_disabled(u32 arr)
{
	struct npcm_otp_regs *regs = otp_priv->regs[arr];

	return (readl(&regs->fcfg) & FCFG_FDIS) != 0;
}

int npcm_otp_select_key(u8 key_index)
{
	struct npcm_otp_regs *regs = otp_priv->regs[NPCM_KEY_SA];
	u32 idx = 0;
	u32 time = 0xDAEDBEEF;

	if (key_index >= 4)
		return -1;

	/* Do not destroy ECCDIS bit */
	idx = readl(&regs->fustrap_fkeyind);

	/* Configure the key size */
	idx &= ~FKEYIND_KSIZE_MASK;
	idx |= FKEYIND_KSIZE_256;

	/* Configure the key index (0 to 3) */
	idx &= ~FKEYIND_KIND_MASK;
	idx |= FKEYIND_KIND_KEY(key_index);

	writel(idx, &regs->fustrap_fkeyind);

	/* Wait for selection completetion */
	while (--time > 1) {
		if (readl(&regs->fustrap_fkeyind) & FKEYIND_KVAL)
			return 0;
		udelay(1);
	}

	return -1;
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_nibble_parity_ecc_encode                         */
/*                                                                            */
/* Parameters:      datain - pointer to decoded data buffer                   */
/*                  dataout - pointer to encoded data buffer (buffer size     */
/*                            should be 2 x dataout)                          */
/*                  size - size of encoded data (decoded data x 2)            */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     Decodes the data according to nibble parity ECC scheme.   */
/*                  Size specifies the encoded data size.                     */
/*                  Decodes whole bytes only                                  */
/*----------------------------------------------------------------------------*/
void npcm_otp_nibble_parity_ecc_encode(u8 *datain, u8 *dataout, u32 size)
{
	u32 i, idx;
	u8 E0, E1, E2, E3;

	for (i = 0; i < (size / 2); i++) {
		E0 = (datain[i] >> 0) & 0x01;
		E1 = (datain[i] >> 1) & 0x01;
		E2 = (datain[i] >> 2) & 0x01;
		E3 = (datain[i] >> 3) & 0x01;

		idx = i * 2;
		dataout[idx] = datain[i] & 0x0f;
		dataout[idx] |= (E0 ^ E1) << 4;
		dataout[idx] |= (E2 ^ E3) << 5;
		dataout[idx] |= (E0 ^ E2) << 6;
		dataout[idx] |= (E1 ^ E3) << 7;

		E0 = (datain[i] >> 4) & 0x01;
		E1 = (datain[i] >> 5) & 0x01;
		E2 = (datain[i] >> 6) & 0x01;
		E3 = (datain[i] >> 7) & 0x01;

		idx = i * 2 + 1;
		dataout[idx] = (datain[i] & 0xf0) >> 4;
		dataout[idx] |= (E0 ^ E1) << 4;
		dataout[idx] |= (E2 ^ E3) << 5;
		dataout[idx] |= (E0 ^ E2) << 6;
		dataout[idx] |= (E1 ^ E3) << 7;
	}
}

/*----------------------------------------------------------------------------*/
/* Function:        npcm_otp_majority_rule_ecc_encode                         */
/*                                                                            */
/* Parameters:      datain - pointer to decoded data buffer                   */
/*                  dataout - pointer to encoded data buffer (buffer size     */
/*                            should be 3 x dataout)                          */
/*                  size - size of encoded data (decoded data x 3)            */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     Decodes the data according to Major Rule ECC scheme.      */
/*                  Size specifies the encoded data size.                     */
/*                  Decodes whole bytes only                                  */
/*----------------------------------------------------------------------------*/
void npcm_otp_majority_rule_ecc_encode(u8 *datain, u8 *dataout, u32 size)
{
	u32 byte;
	u32 bit;
	u8 bit_val;
	u32 decoded_size = size / 3;

	for (byte = 0; byte < decoded_size; byte++) {
		for (bit = 0; bit < 8; bit++) {
			bit_val = (datain[byte] >> bit) & 0x01;

			if (bit_val) {
				dataout[byte] |= (1 << bit);
				dataout[decoded_size + byte] |= (1 << bit);
				dataout[decoded_size * 2 + byte] |= (1 << bit);
			} else {
				dataout[byte] &= ~(1 << bit);
				dataout[decoded_size + byte] &= ~(1 << bit);
				dataout[decoded_size * 2 + byte] &= ~(1 << bit);
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
/* Function:        fuse_program_data                                         */
/*                                                                            */
/* Parameters:      bank - Storage Array type [input].                        */
/*                  word - Byte offset in array [input].                      */
/*                  data - Pointer to data buffer to program.                 */
/*                  size - Number of bytes to program.                        */
/* Returns:         none                                                      */
/* Side effects:                                                              */
/* Description:     Programs the given byte array (size bytes) to the given   */
/*                  OTP storage array, starting from offset word.             */
/*----------------------------------------------------------------------------*/
int fuse_program_data(u32 bank, u32 word, u8 *data, u32 size)
{
	u32 arr = (u32)bank;
	u32 byte;
	int rc;

	rc = npcm_otp_check_inputs(bank, word + size - 1);
	if (rc != 0)
		return rc;

	for (byte = 0; byte < size; byte++) {
		u8 val;

		val = data[byte];
		if (val == 0) // optimization
			continue;

		rc = npcm_otp_program_byte(arr, word + byte, data[byte]);
		if (rc != 0)
			return rc;

		// verify programming of every '1' bit
		val = 0;
		npcm_otp_read_byte((u32)bank, byte, &val);
		if ((data[byte] & ~val) != 0)
			return -1;
	}

	return 0;
}

int fuse_prog_image(u32 bank, uintptr_t address)
{
	return fuse_program_data(bank, 0, (u8 *)address, NPCM_OTP_ARR_BYTE_SIZE);
}

int fuse_read(u32 bank, u32 word, u32 *val)
{
	int rc = npcm_otp_check_inputs(bank, word);

	if (rc != 0)
		return rc;

	*val = 0;
	npcm_otp_read_byte((u32)bank, word, (u8 *)val);

	return 0;
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	/* We do not support overriding */
	return -EINVAL;
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	int rc;

	rc = npcm_otp_check_inputs(bank, word);
	if (rc != 0)
		return rc;

	return npcm_otp_program_byte(bank, word, (u8)val);
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	/* We do not support overriding */
	return -EINVAL;
}

static int npcm_otp_bind(struct udevice *dev)
{
	struct npcm_otp_regs *regs;

	otp_priv = calloc(1, sizeof(struct npcm_otp_priv));
	if (!otp_priv)
		return -ENOMEM;

	regs = dev_remap_addr_index(dev, 0);
	if (!regs) {
		printf("Cannot find reg address (arr #0), binding failed\n");
		return -EINVAL;
	}
	otp_priv->regs[0] = regs;

	if (IS_ENABLED(CONFIG_ARCH_NPCM7xx)) {
		regs = dev_remap_addr_index(dev, 1);
		if (!regs) {
			printf("Cannot find reg address (arr #1), binding failed\n");
			return -EINVAL;
		}
		otp_priv->regs[1] = regs;
	}
	printf("OTP: NPCM OTP module bind OK\n");

	return 0;
}

static const struct udevice_id npcm_otp_ids[] = {
	{ .compatible = "nuvoton,npcm845-otp" },
	{ .compatible = "nuvoton,npcm750-otp" },
	{ }
};

U_BOOT_DRIVER(npcm_otp) = {
	.name = "npcm_otp",
	.id = UCLASS_MISC,
	.of_match = npcm_otp_ids,
	.priv_auto = sizeof(struct npcm_otp_priv),
	.bind = npcm_otp_bind,
};
