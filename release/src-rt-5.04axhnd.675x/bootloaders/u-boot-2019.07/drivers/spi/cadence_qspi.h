/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012
 * Altera Corporation <www.altera.com>
 */

#ifndef __CADENCE_QSPI_H__
#define __CADENCE_QSPI_H__

#include <reset.h>

#define CQSPI_IS_ADDR(cmd_len)		(cmd_len > 1 ? 1 : 0)

#define CQSPI_NO_DECODER_MAX_CS		4
#define CQSPI_DECODER_MAX_CS		16
#define CQSPI_READ_CAPTURE_MAX_DELAY	16

struct cadence_spi_platdata {
	unsigned int	max_hz;
	void		*regbase;
	void		*ahbbase;
	bool		is_decoded_cs;
	u32		fifo_depth;
	u32		fifo_width;
	u32		trigger_address;

	/* Flash parameters */
	u32		page_size;
	u32		block_size;
	u32		tshsl_ns;
	u32		tsd2d_ns;
	u32		tchsh_ns;
	u32		tslch_ns;
};

struct cadence_spi_priv {
	void		*regbase;
	void		*ahbbase;
	size_t		cmd_len;
	u8		cmd_buf[32];
	size_t		data_len;

	int		qspi_is_init;
	unsigned int	qspi_calibrated_hz;
	unsigned int	qspi_calibrated_cs;
	unsigned int	previous_hz;

	struct reset_ctl_bulk resets;
};

/* Functions call declaration */
void cadence_qspi_apb_controller_init(struct cadence_spi_platdata *plat);
void cadence_qspi_apb_controller_enable(void *reg_base_addr);
void cadence_qspi_apb_controller_disable(void *reg_base_addr);

int cadence_qspi_apb_command_read(void *reg_base_addr,
	unsigned int cmdlen, const u8 *cmdbuf, unsigned int rxlen, u8 *rxbuf);
int cadence_qspi_apb_command_write(void *reg_base_addr,
	unsigned int cmdlen, const u8 *cmdbuf,
	unsigned int txlen,  const u8 *txbuf);

int cadence_qspi_apb_indirect_read_setup(struct cadence_spi_platdata *plat,
	unsigned int cmdlen, unsigned int rx_width, const u8 *cmdbuf);
int cadence_qspi_apb_indirect_read_execute(struct cadence_spi_platdata *plat,
	unsigned int rxlen, u8 *rxbuf);
int cadence_qspi_apb_indirect_write_setup(struct cadence_spi_platdata *plat,
	unsigned int cmdlen, unsigned int tx_width, const u8 *cmdbuf);
int cadence_qspi_apb_indirect_write_execute(struct cadence_spi_platdata *plat,
	unsigned int txlen, const u8 *txbuf);

void cadence_qspi_apb_chipselect(void *reg_base,
	unsigned int chip_select, unsigned int decoder_enable);
void cadence_qspi_apb_set_clk_mode(void *reg_base, uint mode);
void cadence_qspi_apb_config_baudrate_div(void *reg_base,
	unsigned int ref_clk_hz, unsigned int sclk_hz);
void cadence_qspi_apb_delay(void *reg_base,
	unsigned int ref_clk, unsigned int sclk_hz,
	unsigned int tshsl_ns, unsigned int tsd2d_ns,
	unsigned int tchsh_ns, unsigned int tslch_ns);
void cadence_qspi_apb_enter_xip(void *reg_base, char xip_dummy);
void cadence_qspi_apb_readdata_capture(void *reg_base,
	unsigned int bypass, unsigned int delay);

#endif /* __CADENCE_QSPI_H__ */
