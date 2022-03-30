// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#include "vsc3316_3308.h"

#define REVISION_ID_REG		0x7E
#define INTERFACE_MODE_REG		0x79
#define CURRENT_PAGE_REGISTER		0x7F
#define CONNECTION_CONFIG_PAGE		0x00
#define INPUT_STATE_REG		0x13
#define GLOBAL_INPUT_ISE1		0x51
#define GLOBAL_INPUT_ISE2		0x52
#define GLOBAL_INPUT_GAIN		0x53
#define GLOBAL_INPUT_LOS		0x55
#define GLOBAL_OUTPUT_PE1		0x56
#define GLOBAL_OUTPUT_PE2		0x57
#define GLOBAL_OUTPUT_LEVEL		0x58
#define GLOBAL_OUTPUT_TERMINATION	0x5A
#define GLOBAL_CORE_CNTRL		0x5D
#define OUTPUT_MODE_PAGE		0x23
#define CORE_CONTROL_PAGE		0x25
#define CORE_CONFIG_REG		0x75

int vsc_if_enable(unsigned int vsc_addr)
{
	u8 data;

	debug("VSC:Configuring VSC at I2C address 0x%2x"
			" for 2-wire interface\n", vsc_addr);

	/* enable 2-wire Serial InterFace (I2C) */
	data = 0x02;
	return i2c_write(vsc_addr, INTERFACE_MODE_REG, 1, &data, 1);
}

int vsc3316_config(unsigned int vsc_addr, int8_t con_arr[][2],
		unsigned int num_con)
{
	unsigned int i;
	u8 rev_id = 0;
	int ret;

	debug("VSC:Initializing VSC3316 at I2C address 0x%2x"
		" for Tx\n", vsc_addr);

	ret = i2c_read(vsc_addr, REVISION_ID_REG, 1, &rev_id, 1);
	if (ret < 0) {
		printf("VSC:0x%x could not read REV_ID from device.\n",
			vsc_addr);
		return ret;
	}

	if (rev_id != 0xab) {
		printf("VSC: device at address 0x%x is not VSC3316/3308.\n",
			vsc_addr);
		return -ENODEV;
	}

	ret = vsc_if_enable(vsc_addr);
	if (ret) {
		printf("VSC:0x%x could not configured for 2-wire I/F.\n",
			vsc_addr);
		return ret;
	}

	/* config connections - page 0x00 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, CONNECTION_CONFIG_PAGE);

	/* Making crosspoint connections, by connecting required
	 * input to output */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][1], con_arr[i][0]);

	/* input state - page 0x13 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, INPUT_STATE_REG);
	/* Configuring the required input of the switch */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][0], 0x80);

	/* Setting Global Input LOS threshold value */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_LOS, 0x60);

	/* config output mode - page 0x23 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, OUTPUT_MODE_PAGE);
	/* Turn ON the Output driver correspond to required output*/
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr,  con_arr[i][1], 0);

	/* configure global core control register, Turn on Global core power */
	i2c_reg_write(vsc_addr, GLOBAL_CORE_CNTRL, 0);

	vsc_wp_config(vsc_addr);

	return 0;
}

#ifdef CONFIG_SYS_FSL_B4860QDS_XFI_ERR
int vsc3308_config_adjust(unsigned int vsc_addr, const int8_t con_arr[][2],
		unsigned int num_con)
{
	unsigned int i;
	u8 rev_id = 0;
	int ret;

	debug("VSC:Initializing VSC3308 at I2C address 0x%x for Tx\n",
	      vsc_addr);

	ret = i2c_read(vsc_addr, REVISION_ID_REG, 1, &rev_id, 1);
	if (ret < 0) {
		printf("VSC:0x%x could not read REV_ID from device.\n",
		       vsc_addr);
		return ret;
	}

	if (rev_id != 0xab) {
		printf("VSC: device at address 0x%x is not VSC3316/3308.\n",
		       vsc_addr);
		return -ENODEV;
	}

	ret = vsc_if_enable(vsc_addr);
	if (ret) {
		printf("VSC:0x%x could not configured for 2-wire I/F.\n",
		       vsc_addr);
		return ret;
	}

	/* config connections - page 0x00 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, CONNECTION_CONFIG_PAGE);

	/* Configure Global Input ISE */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_ISE1, 0);
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_ISE2, 0);

	/* Configure Tx/Rx Global Output PE1 */
	i2c_reg_write(vsc_addr, GLOBAL_OUTPUT_PE1, 0);

	/* Configure Tx/Rx Global Output PE2 */
	i2c_reg_write(vsc_addr, GLOBAL_OUTPUT_PE2, 0);

	/* Configure Tx/Rx Global Input GAIN */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_GAIN, 0x3F);

	/* Setting Global Input LOS threshold value */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_LOS, 0xE0);

	/* Setting Global output termination */
	i2c_reg_write(vsc_addr, GLOBAL_OUTPUT_TERMINATION, 0);

	/* Configure Tx/Rx Global Output level */
	if (vsc_addr == VSC3308_TX_ADDRESS)
		i2c_reg_write(vsc_addr, GLOBAL_OUTPUT_LEVEL, 4);
	else
		i2c_reg_write(vsc_addr, GLOBAL_OUTPUT_LEVEL, 2);

	/* Making crosspoint connections, by connecting required
	 * input to output */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][1], con_arr[i][0]);

	/* input state - page 0x13 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, INPUT_STATE_REG);
	/* Turning off all the required input of the switch */
	for (i = 0; i < num_con; i++)
		i2c_reg_write(vsc_addr, con_arr[i][0], 1);

	/* only turn on specific Tx/Rx requested by the XFI erratum */
	if (vsc_addr == VSC3308_TX_ADDRESS) {
		i2c_reg_write(vsc_addr, 2, 0);
		i2c_reg_write(vsc_addr, 3, 0);
	} else {
		i2c_reg_write(vsc_addr, 0, 0);
		i2c_reg_write(vsc_addr, 1, 0);
	}

	/* config output mode - page 0x23 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, OUTPUT_MODE_PAGE);
	/* Turn off the Output driver correspond to required output*/
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr,  con_arr[i][1], 1);

	/* only turn on specific Tx/Rx requested by the XFI erratum */
	if (vsc_addr == VSC3308_TX_ADDRESS) {
		i2c_reg_write(vsc_addr, 0, 0);
		i2c_reg_write(vsc_addr, 1, 0);
	} else {
		i2c_reg_write(vsc_addr, 3, 0);
		i2c_reg_write(vsc_addr, 4, 0);
	}

	/* configure global core control register, Turn on Global core power */
	i2c_reg_write(vsc_addr, GLOBAL_CORE_CNTRL, 0);

	vsc_wp_config(vsc_addr);

	return 0;
}
#endif

int vsc3308_config(unsigned int vsc_addr, const int8_t con_arr[][2],
		unsigned int num_con)
{
	unsigned int i;
	u8 rev_id = 0;
	int ret;

	debug("VSC:Initializing VSC3308 at I2C address 0x%x"
		" for Tx\n", vsc_addr);

	ret = i2c_read(vsc_addr, REVISION_ID_REG, 1, &rev_id, 1);
	if (ret < 0) {
		printf("VSC:0x%x could not read REV_ID from device.\n",
			vsc_addr);
		return ret;
	}

	if (rev_id != 0xab) {
		printf("VSC: device at address 0x%x is not VSC3316/3308.\n",
			vsc_addr);
		return -ENODEV;
	}

	ret = vsc_if_enable(vsc_addr);
	if (ret) {
		printf("VSC:0x%x could not configured for 2-wire I/F.\n",
			vsc_addr);
		return ret;
	}

	/* config connections - page 0x00 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, CONNECTION_CONFIG_PAGE);

	/* Making crosspoint connections, by connecting required
	 * input to output */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][1], con_arr[i][0]);

	/*Configure Global Input ISE and gain */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_ISE1, 0x12);
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_ISE2, 0x12);

	/* input state - page 0x13 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, INPUT_STATE_REG);
	/* Turning ON the required input of the switch */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][0], 0);

	/* Setting Global Input LOS threshold value */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_LOS, 0x60);

	/* config output mode - page 0x23 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, OUTPUT_MODE_PAGE);
	/* Turn ON the Output driver correspond to required output*/
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr,  con_arr[i][1], 0);

	/* configure global core control register, Turn on Global core power */
	i2c_reg_write(vsc_addr, GLOBAL_CORE_CNTRL, 0);

	vsc_wp_config(vsc_addr);

	return 0;
}

void vsc_wp_config(unsigned int vsc_addr)
{
	debug("VSC:Configuring VSC at address:0x%x for WP\n", vsc_addr);

	/* For new crosspoint configuration to occur, WP bit of
	 * CORE_CONFIG_REG should be set 1 and then reset to 0 */
	i2c_reg_write(vsc_addr, CORE_CONFIG_REG, 0x01);
	i2c_reg_write(vsc_addr, CORE_CONFIG_REG, 0x0);
}
