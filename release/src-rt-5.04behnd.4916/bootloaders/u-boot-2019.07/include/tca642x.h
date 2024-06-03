/*
 * Copyright 2013 Texas Instruments, Inc.
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from the pca953x.c driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __TCA642X_H_
#define __TCA642X_H_

#ifdef CONFIG_CMD_TCA642X
enum {
	TCA642X_CMD_INFO,
	TCA642X_CMD_DEVICE,
	TCA642X_CMD_OUTPUT,
	TCA642X_CMD_INPUT,
	TCA642X_CMD_INVERT,
};
#endif

#define TCA642X_OUT_LOW		0
#define TCA642X_OUT_HIGH	1
#define TCA642X_POL_NORMAL	0
#define TCA642X_POL_INVERT	1
#define TCA642X_DIR_OUT		0
#define TCA642X_DIR_IN		1

/* Default to an address that hopefully won't corrupt other i2c devices */
#ifndef CONFIG_SYS_I2C_TCA642X_ADDR
#define CONFIG_SYS_I2C_TCA642X_ADDR	(~0)
#endif

/* Default to an address that hopefully won't corrupt other i2c devices */
#ifndef CONFIG_SYS_I2C_TCA642X_BUS_NUM
#define CONFIG_SYS_I2C_TCA642X_BUS_NUM	(0)
#endif

struct tca642x_bank_info {
	uint8_t input_reg;
	uint8_t output_reg;
	uint8_t polarity_reg;
	uint8_t configuration_reg;
};

int tca642x_set_val(uchar chip, uint8_t gpio_bank,
			uint8_t reg_bit, uint8_t data);
int tca642x_set_pol(uchar chip, uint8_t gpio_bank,
			uint8_t reg_bit, uint8_t data);
int tca642x_set_dir(uchar chip, uint8_t gpio_bank,
			uint8_t reg_bit, uint8_t data);
int tca642x_get_val(uchar chip, uint8_t gpio_bank);
int tca642x_set_inital_state(uchar chip, struct tca642x_bank_info init_data[]);

#endif /* __TCA642X_H_ */
