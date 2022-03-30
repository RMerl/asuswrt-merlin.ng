// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include "vct.h"

/*
 * EBI initialization for SMC911x access
 */
int ebi_init_smc911x(void)
{
	reg_write(EBI_DEV1_CONFIG1(EBI_BASE), 0x00003020);
	reg_write(EBI_DEV1_CONFIG2(EBI_BASE), 0x0000004F);

	reg_write(EBI_DEV1_TIM1_RD1(EBI_BASE), 0x00501100);
	reg_write(EBI_DEV1_TIM1_RD2(EBI_BASE), 0x0FF02111);

	reg_write(EBI_DEV1_TIM_EXT(EBI_BASE), 0xFFF00000);
	reg_write(EBI_DEV1_EXT_ACC(EBI_BASE), 0x0FFFFFFF);

	reg_write(EBI_DEV1_TIM1_WR1(EBI_BASE), 0x05001100);
	reg_write(EBI_DEV1_TIM1_WR2(EBI_BASE), 0x3FC21110);

	return 0;
}

/*
 * Accessor functions replacing the "weak" functions in
 * drivers/net/smc911x.c
 */
u32 smc911x_reg_read(struct eth_device *dev, u32 addr)
{
	volatile u32 data;

	addr += dev->iobase;
	reg_write(EBI_DEV1_CONFIG2(EBI_BASE), 0x0000004F);
	ebi_wait();
	reg_write(EBI_CPU_IO_ACCS(EBI_BASE), (EXT_DEVICE_CHANNEL_1 | addr));
	ebi_wait();
	data = reg_read(EBI_IO_ACCS_DATA(EBI_BASE));

	return (data);
}

void smc911x_reg_write(struct eth_device *dev, u32 addr, u32 data)
{
	addr += dev->iobase;
	reg_write(EBI_DEV1_CONFIG2(EBI_BASE), 0x0000004F);
	ebi_wait();
	reg_write(EBI_IO_ACCS_DATA(EBI_BASE), data);
	reg_write(EBI_CPU_IO_ACCS(EBI_BASE),
		  EXT_DEVICE_CHANNEL_1 | EBI_CPU_WRITE | addr);
	ebi_wait();
}

void pkt_data_push(struct eth_device *dev, u32 addr, u32 data)
{
	addr += dev->iobase;
	reg_write(EBI_DEV1_CONFIG2(EBI_BASE), 0x0000004A);
	ebi_wait();
	reg_write(EBI_IO_ACCS_DATA(EBI_BASE), data);
	reg_write(EBI_CPU_IO_ACCS(EBI_BASE),
		  EXT_DEVICE_CHANNEL_1 | EBI_CPU_WRITE | addr);
	ebi_wait();

	return;
}

u32 pkt_data_pull(struct eth_device *dev, u32 addr)
{
	volatile u32 data;

	addr += dev->iobase;
	reg_write(EBI_DEV1_CONFIG2(EBI_BASE), 0x0000004A);
	ebi_wait();
	reg_write(EBI_CPU_IO_ACCS(EBI_BASE), (EXT_DEVICE_CHANNEL_1 | addr));
	ebi_wait();
	data = reg_read(EBI_IO_ACCS_DATA(EBI_BASE));

	return data;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_DRIVER_SMC911X_BASE);
#endif
	return rc;
}
