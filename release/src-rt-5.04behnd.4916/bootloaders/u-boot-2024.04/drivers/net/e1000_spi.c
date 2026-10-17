#include <common.h>
#include <command.h>
#include <console.h>
#include <linux/delay.h>
#include "e1000.h"
#include <malloc.h>
#include <linux/compiler.h>

/*-----------------------------------------------------------------------
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 *
 * This may be interrupted with Ctrl-C if "intr" is true, otherwise it will
 * never return an error.
 */
static int e1000_spi_xfer(struct e1000_hw *hw, unsigned int bitlen,
		const void *dout_mem, void *din_mem, bool intr)
{
	const uint8_t *dout = dout_mem;
	uint8_t *din = din_mem;

	uint8_t mask = 0;
	uint32_t eecd;
	unsigned long i;

	/* Pre-read the control register */
	eecd = E1000_READ_REG(hw, EECD);

	/* Iterate over each bit */
	for (i = 0, mask = 0x80; i < bitlen; i++, mask = (mask >> 1)?:0x80) {
		/* Check for interrupt */
		if (intr && ctrlc())
			return -1;

		/* Determine the output bit */
		if (dout && dout[i >> 3] & mask)
			eecd |=  E1000_EECD_DI;
		else
			eecd &= ~E1000_EECD_DI;

		/* Write the output bit and wait 50us */
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(50);

		/* Poke the clock (waits 50us) */
		e1000_raise_ee_clk(hw, &eecd);

		/* Now read the input bit */
		eecd = E1000_READ_REG(hw, EECD);
		if (din) {
			if (eecd & E1000_EECD_DO)
				din[i >> 3] |=  mask;
			else
				din[i >> 3] &= ~mask;
		}

		/* Poke the clock again (waits 50us) */
		e1000_lower_ee_clk(hw, &eecd);
	}

	/* Now clear any remaining bits of the input */
	if (din && (i & 7))
		din[i >> 3] &= ~((mask << 1) - 1);

	return 0;
}

#ifdef CONFIG_E1000_SPI_GENERIC
static inline struct e1000_hw *e1000_hw_from_spi(struct spi_slave *spi)
{
	return container_of(spi, struct e1000_hw, spi);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	/* Find the right PCI device */
	struct e1000_hw *hw = e1000_find_card(bus);
	if (!hw) {
		printf("ERROR: No such e1000 device: e1000#%u\n", bus);
		return NULL;
	}

	/* Make sure it has an SPI chip */
	if (hw->eeprom.type != e1000_eeprom_spi) {
		E1000_ERR(hw, "No attached SPI EEPROM found!\n");
		return NULL;
	}

	/* Argument sanity checks */
	if (cs != 0) {
		E1000_ERR(hw, "No such SPI chip: %u\n", cs);
		return NULL;
	}
	if (mode != SPI_MODE_0) {
		E1000_ERR(hw, "Only SPI MODE-0 is supported!\n");
		return NULL;
	}

	/* TODO: Use max_hz somehow */
	E1000_DBG(hw->nic, "EEPROM SPI access requested\n");
	return &hw->spi;
}

void spi_free_slave(struct spi_slave *spi)
{
	__maybe_unused struct e1000_hw *hw = e1000_hw_from_spi(spi);
	E1000_DBG(hw->nic, "EEPROM SPI access released\n");
}

int spi_claim_bus(struct spi_slave *spi)
{
	struct e1000_hw *hw = e1000_hw_from_spi(spi);

	if (e1000_acquire_eeprom(hw)) {
		E1000_ERR(hw, "EEPROM SPI cannot be acquired!\n");
		return -1;
	}

	return 0;
}

void spi_release_bus(struct spi_slave *spi)
{
	struct e1000_hw *hw = e1000_hw_from_spi(spi);
	e1000_release_eeprom(hw);
}

/* Skinny wrapper around e1000_spi_xfer */
int spi_xfer(struct spi_slave *spi, unsigned int bitlen,
		const void *dout_mem, void *din_mem, unsigned long flags)
{
	struct e1000_hw *hw = e1000_hw_from_spi(spi);
	int ret;

	if (flags & SPI_XFER_BEGIN)
		e1000_standby_eeprom(hw);

	ret = e1000_spi_xfer(hw, bitlen, dout_mem, din_mem, true);

	if (flags & SPI_XFER_END)
		e1000_standby_eeprom(hw);

	return ret;
}

#endif /* not CONFIG_E1000_SPI_GENERIC */

#ifdef CONFIG_CMD_E1000

/* The EEPROM opcodes */
#define SPI_EEPROM_ENABLE_WR	0x06
#define SPI_EEPROM_DISABLE_WR	0x04
#define SPI_EEPROM_WRITE_STATUS	0x01
#define SPI_EEPROM_READ_STATUS	0x05
#define SPI_EEPROM_WRITE_PAGE	0x02
#define SPI_EEPROM_READ_PAGE	0x03

/* The EEPROM status bits */
#define SPI_EEPROM_STATUS_BUSY	0x01
#define SPI_EEPROM_STATUS_WREN	0x02

static int e1000_spi_eeprom_enable_wr(struct e1000_hw *hw, bool intr)
{
	u8 op[] = { SPI_EEPROM_ENABLE_WR };
	e1000_standby_eeprom(hw);
	return e1000_spi_xfer(hw, 8*sizeof(op), op, NULL, intr);
}

/*
 * These have been tested to perform correctly, but they are not used by any
 * of the EEPROM commands at this time.
 */
static __maybe_unused int e1000_spi_eeprom_disable_wr(struct e1000_hw *hw,
						      bool intr)
{
	u8 op[] = { SPI_EEPROM_DISABLE_WR };
	e1000_standby_eeprom(hw);
	return e1000_spi_xfer(hw, 8*sizeof(op), op, NULL, intr);
}

static __maybe_unused int e1000_spi_eeprom_write_status(struct e1000_hw *hw,
							u8 status, bool intr)
{
	u8 op[] = { SPI_EEPROM_WRITE_STATUS, status };
	e1000_standby_eeprom(hw);
	return e1000_spi_xfer(hw, 8*sizeof(op), op, NULL, intr);
}

static int e1000_spi_eeprom_read_status(struct e1000_hw *hw, bool intr)
{
	u8 op[] = { SPI_EEPROM_READ_STATUS, 0 };
	e1000_standby_eeprom(hw);
	if (e1000_spi_xfer(hw, 8*sizeof(op), op, op, intr))
		return -1;
	return op[1];
}

static int e1000_spi_eeprom_write_page(struct e1000_hw *hw,
		const void *data, u16 off, u16 len, bool intr)
{
	u8 op[] = {
		SPI_EEPROM_WRITE_PAGE,
		(off >> (hw->eeprom.address_bits - 8)) & 0xff, off & 0xff
	};

	e1000_standby_eeprom(hw);

	if (e1000_spi_xfer(hw, 8 + hw->eeprom.address_bits, op, NULL, intr))
		return -1;
	if (e1000_spi_xfer(hw, len << 3, data, NULL, intr))
		return -1;

	return 0;
}

static int e1000_spi_eeprom_read_page(struct e1000_hw *hw,
		void *data, u16 off, u16 len, bool intr)
{
	u8 op[] = {
		SPI_EEPROM_READ_PAGE,
		(off >> (hw->eeprom.address_bits - 8)) & 0xff, off & 0xff
	};

	e1000_standby_eeprom(hw);

	if (e1000_spi_xfer(hw, 8 + hw->eeprom.address_bits, op, NULL, intr))
		return -1;
	if (e1000_spi_xfer(hw, len << 3, NULL, data, intr))
		return -1;

	return 0;
}

static int e1000_spi_eeprom_poll_ready(struct e1000_hw *hw, bool intr)
{
	int status;
	while ((status = e1000_spi_eeprom_read_status(hw, intr)) >= 0) {
		if (!(status & SPI_EEPROM_STATUS_BUSY))
			return 0;
	}
	return -1;
}

static int e1000_spi_eeprom_dump(struct e1000_hw *hw,
		void *data, u16 off, unsigned int len, bool intr)
{
	/* Interruptibly wait for the EEPROM to be ready */
	if (e1000_spi_eeprom_poll_ready(hw, intr))
		return -1;

	/* Dump each page in sequence */
	while (len) {
		/* Calculate the data bytes on this page */
		u16 pg_off = off & (hw->eeprom.page_size - 1);
		u16 pg_len = hw->eeprom.page_size - pg_off;
		if (pg_len > len)
			pg_len = len;

		/* Now dump the page */
		if (e1000_spi_eeprom_read_page(hw, data, off, pg_len, intr))
			return -1;

		/* Otherwise go on to the next page */
		len  -= pg_len;
		off  += pg_len;
		data += pg_len;
	}

	/* We're done! */
	return 0;
}

static int e1000_spi_eeprom_program(struct e1000_hw *hw,
		const void *data, u16 off, u16 len, bool intr)
{
	/* Program each page in sequence */
	while (len) {
		/* Calculate the data bytes on this page */
		u16 pg_off = off & (hw->eeprom.page_size - 1);
		u16 pg_len = hw->eeprom.page_size - pg_off;
		if (pg_len > len)
			pg_len = len;

		/* Interruptibly wait for the EEPROM to be ready */
		if (e1000_spi_eeprom_poll_ready(hw, intr))
			return -1;

		/* Enable write access */
		if (e1000_spi_eeprom_enable_wr(hw, intr))
			return -1;

		/* Now program the page */
		if (e1000_spi_eeprom_write_page(hw, data, off, pg_len, intr))
			return -1;

		/* Otherwise go on to the next page */
		len  -= pg_len;
		off  += pg_len;
		data += pg_len;
	}

	/* Wait for the last write to complete */
	if (e1000_spi_eeprom_poll_ready(hw, intr))
		return -1;

	/* We're done! */
	return 0;
}

static int do_e1000_spi_show(struct cmd_tbl *cmdtp, struct e1000_hw *hw,
			     int argc, char *const argv[])
{
	unsigned int length = 0;
	u16 i, offset = 0;
	u8 *buffer;
	int err;

	if (argc > 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Parse the offset and length */
	if (argc >= 1)
		offset = simple_strtoul(argv[0], NULL, 0);
	if (argc == 2)
		length = simple_strtoul(argv[1], NULL, 0);
	else if (offset < (hw->eeprom.word_size << 1))
		length = (hw->eeprom.word_size << 1) - offset;

	/* Extra sanity checks */
	if (!length) {
		E1000_ERR(hw, "Requested zero-sized dump!\n");
		return 1;
	}
	if ((0x10000 < length) || (0x10000 - length < offset)) {
		E1000_ERR(hw, "Can't dump past 0xFFFF!\n");
		return 1;
	}

	/* Allocate a buffer to hold stuff */
	buffer = malloc(length);
	if (!buffer) {
		E1000_ERR(hw, "Out of Memory!\n");
		return 1;
	}

	/* Acquire the EEPROM and perform the dump */
	if (e1000_acquire_eeprom(hw)) {
		E1000_ERR(hw, "EEPROM SPI cannot be acquired!\n");
		free(buffer);
		return 1;
	}
	err = e1000_spi_eeprom_dump(hw, buffer, offset, length, true);
	e1000_release_eeprom(hw);
	if (err) {
		E1000_ERR(hw, "Interrupted!\n");
		free(buffer);
		return 1;
	}

	/* Now hexdump the result */
	printf("%s: ===== Intel e1000 EEPROM (0x%04hX - 0x%04hX) =====",
			hw->name, offset, offset + length - 1);
	for (i = 0; i < length; i++) {
		if ((i & 0xF) == 0)
			printf("\n%s: %04hX: ", hw->name, offset + i);
		else if ((i & 0xF) == 0x8)
			printf(" ");
		printf(" %02hx", buffer[i]);
	}
	printf("\n");

	/* Success! */
	free(buffer);
	return 0;
}

static int do_e1000_spi_dump(struct cmd_tbl *cmdtp, struct e1000_hw *hw,
			     int argc, char *const argv[])
{
	unsigned int length;
	u16 offset;
	void *dest;

	if (argc != 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Parse the arguments */
	dest = (void *)hextoul(argv[0], NULL);
	offset = simple_strtoul(argv[1], NULL, 0);
	length = simple_strtoul(argv[2], NULL, 0);

	/* Extra sanity checks */
	if (!length) {
		E1000_ERR(hw, "Requested zero-sized dump!\n");
		return 1;
	}
	if ((0x10000 < length) || (0x10000 - length < offset)) {
		E1000_ERR(hw, "Can't dump past 0xFFFF!\n");
		return 1;
	}

	/* Acquire the EEPROM */
	if (e1000_acquire_eeprom(hw)) {
		E1000_ERR(hw, "EEPROM SPI cannot be acquired!\n");
		return 1;
	}

	/* Perform the programming operation */
	if (e1000_spi_eeprom_dump(hw, dest, offset, length, true) < 0) {
		E1000_ERR(hw, "Interrupted!\n");
		e1000_release_eeprom(hw);
		return 1;
	}

	e1000_release_eeprom(hw);
	printf("%s: ===== EEPROM DUMP COMPLETE =====\n", hw->name);
	return 0;
}

static int do_e1000_spi_program(struct cmd_tbl *cmdtp, struct e1000_hw *hw,
				int argc, char *const argv[])
{
	unsigned int length;
	const void *source;
	u16 offset;

	if (argc != 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Parse the arguments */
	source = (const void *)hextoul(argv[0], NULL);
	offset = simple_strtoul(argv[1], NULL, 0);
	length = simple_strtoul(argv[2], NULL, 0);

	/* Acquire the EEPROM */
	if (e1000_acquire_eeprom(hw)) {
		E1000_ERR(hw, "EEPROM SPI cannot be acquired!\n");
		return 1;
	}

	/* Perform the programming operation */
	if (e1000_spi_eeprom_program(hw, source, offset, length, true) < 0) {
		E1000_ERR(hw, "Interrupted!\n");
		e1000_release_eeprom(hw);
		return 1;
	}

	e1000_release_eeprom(hw);
	printf("%s: ===== EEPROM PROGRAMMED =====\n", hw->name);
	return 0;
}

static int do_e1000_spi_checksum(struct cmd_tbl *cmdtp, struct e1000_hw *hw,
				 int argc, char *const argv[])
{
	uint16_t i, length, checksum = 0, checksum_reg;
	uint16_t *buffer;
	bool upd;

	if (argc == 0)
		upd = 0;
	else if ((argc == 1) && !strcmp(argv[0], "update"))
		upd = 1;
	else {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Allocate a temporary buffer */
	length = sizeof(uint16_t) * (EEPROM_CHECKSUM_REG + 1);
	buffer = malloc(length);
	if (!buffer) {
		E1000_ERR(hw, "Unable to allocate EEPROM buffer!\n");
		return 1;
	}

	/* Acquire the EEPROM */
	if (e1000_acquire_eeprom(hw)) {
		E1000_ERR(hw, "EEPROM SPI cannot be acquired!\n");
		return 1;
	}

	/* Read the EEPROM */
	if (e1000_spi_eeprom_dump(hw, buffer, 0, length, true) < 0) {
		E1000_ERR(hw, "Interrupted!\n");
		e1000_release_eeprom(hw);
		return 1;
	}

	/* Compute the checksum and read the expected value */
	for (i = 0; i < EEPROM_CHECKSUM_REG; i++)
		checksum += le16_to_cpu(buffer[i]);
	checksum = ((uint16_t)EEPROM_SUM) - checksum;
	checksum_reg = le16_to_cpu(buffer[i]);

	/* Verify it! */
	if (checksum_reg == checksum) {
		printf("%s: INFO: EEPROM checksum is correct! (0x%04hx)\n",
				hw->name, checksum);
		e1000_release_eeprom(hw);
		return 0;
	}

	/* Hrm, verification failed, print an error */
	E1000_ERR(hw, "EEPROM checksum is incorrect!\n");
	E1000_ERR(hw, "  ...register was 0x%04hx, calculated 0x%04hx\n",
		  checksum_reg, checksum);

	/* If they didn't ask us to update it, just return an error */
	if (!upd) {
		e1000_release_eeprom(hw);
		return 1;
	}

	/* Ok, correct it! */
	printf("%s: Reprogramming the EEPROM checksum...\n", hw->name);
	buffer[i] = cpu_to_le16(checksum);
	if (e1000_spi_eeprom_program(hw, &buffer[i], i * sizeof(uint16_t),
			sizeof(uint16_t), true)) {
		E1000_ERR(hw, "Interrupted!\n");
		e1000_release_eeprom(hw);
		return 1;
	}

	e1000_release_eeprom(hw);
	return 0;
}

int do_e1000_spi(struct cmd_tbl *cmdtp, struct e1000_hw *hw,
		 int argc, char *const argv[])
{
	if (argc < 1) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Make sure it has an SPI chip */
	if (hw->eeprom.type != e1000_eeprom_spi) {
		E1000_ERR(hw, "No attached SPI EEPROM found (%d)!\n",
			  hw->eeprom.type);
		return 1;
	}

	/* Check the eeprom sub-sub-command arguments */
	if (!strcmp(argv[0], "show"))
		return do_e1000_spi_show(cmdtp, hw, argc - 1, argv + 1);

	if (!strcmp(argv[0], "dump"))
		return do_e1000_spi_dump(cmdtp, hw, argc - 1, argv + 1);

	if (!strcmp(argv[0], "program"))
		return do_e1000_spi_program(cmdtp, hw, argc - 1, argv + 1);

	if (!strcmp(argv[0], "checksum"))
		return do_e1000_spi_checksum(cmdtp, hw, argc - 1, argv + 1);

	cmd_usage(cmdtp);
	return 1;
}

#endif /* not CONFIG_CMD_E1000 */
