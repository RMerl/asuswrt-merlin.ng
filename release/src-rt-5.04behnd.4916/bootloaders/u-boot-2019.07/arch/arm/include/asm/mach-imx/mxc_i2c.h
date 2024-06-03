/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 */
#ifndef __ASM_ARCH_MXC_MXC_I2C_H__
#define __ASM_ARCH_MXC_MXC_I2C_H__
#include <asm-generic/gpio.h>
#include <asm/mach-imx/iomux-v3.h>

struct i2c_pin_ctrl {
	iomux_v3_cfg_t i2c_mode;
	iomux_v3_cfg_t gpio_mode;
	unsigned char gp;
	unsigned char spare;
};

struct i2c_pads_info {
	struct i2c_pin_ctrl scl;
	struct i2c_pin_ctrl sda;
};

/*
 * Information about i2c controller
 * struct mxc_i2c_bus - information about the i2c[x] bus
 * @index: i2c bus index
 * @base: Address of I2C bus controller
 * @driver_data: Flags for different platforms, such as I2C_QUIRK_FLAG.
 * @speed: Speed of I2C bus
 * @pads_info: pinctrl info for this i2c bus, will be used when pinctrl is ok.
 * The following two is only to be compatible with non-DM part.
 * @idle_bus_fn: function to force bus idle
 * @idle_bus_data: parameter for idle_bus_fun
 * For DM:
 * bus: The device structure for i2c bus controller
 * scl-gpio: specify the gpio related to SCL pin
 * sda-gpio: specify the gpio related to SDA pin
 */
struct mxc_i2c_bus {
	/*
	 * board file can use this index to locate which i2c_pads_info is for
	 * i2c_idle_bus. When pinmux is implement, this entry can be
	 * discarded. Here we do not use dev->seq, because we do not want to
	 * export device to board file.
	 */
	int index;
	ulong base;
	ulong driver_data;
	int speed;
	struct i2c_pads_info *pads_info;
#ifndef CONFIG_DM_I2C
	int (*idle_bus_fn)(void *p);
	void *idle_bus_data;
#else
	struct udevice *bus;
	/* Use gpio to force bus idle when bus state is abnormal */
	struct gpio_desc scl_gpio;
	struct gpio_desc sda_gpio;
#endif
};

#if defined(CONFIG_MX6QDL)
#define I2C_PADS(name, scl_i2c, scl_gpio, scl_gp, sda_i2c, sda_gpio, sda_gp) \
	struct i2c_pads_info mx6q_##name = {		\
		.scl = {				\
			.i2c_mode = MX6Q_##scl_i2c,	\
			.gpio_mode = MX6Q_##scl_gpio,	\
			.gp = scl_gp,			\
		},					\
		.sda = {				\
			.i2c_mode = MX6Q_##sda_i2c,	\
			.gpio_mode = MX6Q_##sda_gpio,	\
			.gp = sda_gp,			\
		}					\
	};						\
	struct i2c_pads_info mx6s_##name = {		\
		.scl = {				\
			.i2c_mode = MX6DL_##scl_i2c,	\
			.gpio_mode = MX6DL_##scl_gpio,	\
			.gp = scl_gp,			\
		},					\
		.sda = {				\
			.i2c_mode = MX6DL_##sda_i2c,	\
			.gpio_mode = MX6DL_##sda_gpio,	\
			.gp = sda_gp,			\
		}					\
	};


#define I2C_PADS_INFO(name)	\
	(is_mx6dq() || is_mx6dqp()) ? &mx6q_##name : &mx6s_##name
#endif

int setup_i2c(unsigned i2c_index, int speed, int slave_addr,
	      struct i2c_pads_info *p);
void bus_i2c_init(int index, int speed, int slave_addr,
		int (*idle_bus_fn)(void *p), void *p);
int force_idle_bus(void *priv);
int i2c_idle_bus(struct mxc_i2c_bus *i2c_bus);
#endif
