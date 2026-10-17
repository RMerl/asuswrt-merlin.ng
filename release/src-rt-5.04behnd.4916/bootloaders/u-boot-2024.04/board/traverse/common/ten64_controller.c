// SPDX-License-Identifier: GPL-2.0+

/* Ten64 Board Microcontroller Driver
 * Copyright 2021 Traverse Technologies Australia
 *
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <i2c.h>
#include <hexdump.h>
#include <dm/device_compat.h>
#include <inttypes.h>
#include <linux/delay.h>

#include "ten64-controller.h"

/* Microcontroller command set and structure
 * These should not be used outside this file
 */

#define T64_UC_DATA_MAX_SIZE            128U
#define T64_UC_API_MSG_HEADER_SIZE      4U
#define T64_UC_API_HEADER_PREAMB        0xcabe

enum {
	TEN64_UC_CMD_SET_BOARD_MAC = 0x10,
	TEN64_UC_CMD_GET_BOARD_INFO = 0x11,
	TEN64_UC_CMD_GET_STATE = 0x20,
	TEN64_UC_CMD_SET_RESET_BTN_HOLD_TIME = 0x21,
	TEN64_UC_CMD_ENABLE_RESET_BUTTON = 0x22,
	TEN64_UC_CMD_SET_NEXT_BOOTSRC = 0x23,
	TEN64_UC_CMD_ENABLE_10G = 0x24,
	TEN64_UC_CMD_FWUP_GET_INFO = 0xA0,
	TEN64_UC_CMD_FWUP_INIT = 0xA1,
	TEN64_UC_CMD_FWUP_XFER = 0xA2,
	TEN64_UC_CMD_FWUP_CHECK = 0xA3,
	TEN64_UC_CMD_FWUPBOOT = 0x0A
};

/** struct t64uc_message - Wire Format for microcontroller messages
 * @preamb: Message preamble (always 0xcabe)
 * @cmd: Command to invoke
 * @len: Length of data
 * @data: Command data, up to 128 bytes
 */
struct t64uc_message {
	u16 preamb;
	u8 cmd;
	u8 len;
	u8 data[T64_UC_DATA_MAX_SIZE];
}  __packed;

enum {
	T64_CTRL_IO_SET = 1U,
	T64_CTRL_IO_CLEAR = 2U,
	T64_CTRL_IO_TOGGLE = 3U,
	T64_CTRL_IO_RESET = 4U,
	T64_CTRL_IO_UNKNOWN = 5U
};

/** struct t64uc_board_10g_enable - Wrapper for 10G enable command
 * @control: state to set the 10G retimer - either
 *	     T64_CTRL_IO_CLEAR (0x02) for off or
 *	     T64_CTRL_IO_SET (0x01) for on.
 *
 * This struct exists to simplify the wrapping of the
 * command value into a microcontroller message and passing into
 * functions.
 */
struct t64uc_board_10g_enable {
	u8 control;
} __packed;

/** ten64_controller_send_recv_command() - Wrapper function to
 * send a command to the microcontroller.
 * @uc_chip: the DM I2C chip handle for the microcontroller
 * @uc_cmd: the microcontroller API command code
 * @uc_cmd_data: pointer to the data struct for this command
 * @uc_data_len: size of command data struct
 * @return_data: place to store response from microcontroller, NULL if not expected
 * @expected_return_len: expected size of microcontroller command response
 * @return_message_wait: wait this long (in us) before reading the response
 *
 * Invoke a microcontroller command and receive a response.
 * This function includes communicating with the microcontroller over
 * I2C and encoding a message in the wire format.
 *
 * Return: 0 if successful, error code otherwise.
 * Returns -EBADMSG if the microcontroller response could not be validated,
 * other error codes may be passed from dm_i2c_xfer()
 */
static int ten64_controller_send_recv_command(struct udevice *ucdev, u8 uc_cmd,
					      void *uc_cmd_data, u8 cmd_data_len,
					      void *return_data, u8 expected_return_len,
					      u16 return_message_wait)
{
	int ret;
	struct t64uc_message send, recv;
	struct i2c_msg command_message, return_message;
	struct dm_i2c_chip *chip = dev_get_parent_plat(ucdev);

	dev_dbg(ucdev, "%s sending cmd %02X len %d\n", __func__, uc_cmd, cmd_data_len);

	send.preamb = T64_UC_API_HEADER_PREAMB;
	send.cmd = uc_cmd;
	send.len = cmd_data_len;
	if (uc_cmd_data && cmd_data_len > 0)
		memcpy(send.data, uc_cmd_data, cmd_data_len);

	command_message.addr = chip->chip_addr;
	command_message.len = T64_UC_API_MSG_HEADER_SIZE + send.len;
	command_message.buf = (void *)&send;
	command_message.flags = I2C_M_STOP;

	ret = dm_i2c_xfer(ucdev, &command_message, 1);
	if (!return_data)
		return ret;

	udelay(return_message_wait);

	return_message.addr = chip->chip_addr;
	return_message.len = T64_UC_API_MSG_HEADER_SIZE + expected_return_len;
	return_message.buf = (void *)&recv;
	return_message.flags = I2C_M_RD;

	ret = dm_i2c_xfer(ucdev, &return_message, 1);
	if (ret)
		return ret;

	if (recv.preamb != T64_UC_API_HEADER_PREAMB) {
		dev_err(ucdev, "%s: No preamble received in microcontroller response\n",
			__func__);
		return -EBADMSG;
	}
	if (recv.cmd != uc_cmd) {
		dev_err(ucdev, "%s: command response mismatch, got %02X expecting %02X\n",
			__func__, recv.cmd, uc_cmd);
		return -EBADMSG;
	}
	if (recv.len != expected_return_len) {
		dev_err(ucdev, "%s: received message has unexpected length, got %d expected %d\n",
			__func__, recv.len, expected_return_len);
		return -EBADMSG;
	}
	memcpy(return_data, recv.data, expected_return_len);
	return ret;
}

/** ten64_controller_send_command() - Send command to microcontroller without
 * expecting a response (for example, invoking a control command)
 * @uc_chip: the DM I2C chip handle for the microcontroller
 * @uc_cmd: the microcontroller API command code
 * @uc_cmd_data: pointer to the data struct for this command
 * @uc_data_len: size of command data struct
 */
static int ten64_controller_send_command(struct udevice *ucdev, u8 uc_cmd,
					 void *uc_cmd_data, u8 cmd_data_len)
{
	return ten64_controller_send_recv_command(ucdev, uc_cmd,
						  uc_cmd_data, cmd_data_len,
						  NULL, 0, 0);
}

/** ten64_controller_get_board_info() -Get board information from microcontroller
 * @dev: The microcontroller device handle
 * @out: Pointer to a t64uc_board_info struct that has been allocated by the caller
 */
static int ten64_controller_get_board_info(struct udevice *dev, struct t64uc_board_info *out)
{
	int ret;

	ret = ten64_controller_send_recv_command(dev, TEN64_UC_CMD_GET_BOARD_INFO,
						 NULL, 0, out,
						 sizeof(struct t64uc_board_info),
						 10000);
	if (ret) {
		dev_err(dev, "%s unable to send board info command: %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

/**
 * ten64_controller_10g_enable_command() - Sends a 10G (Retimer) enable command
 * to the microcontroller.
 * @ucdev: The microcontroller udevice
 * @value: The value flag for the 10G state
 */
static int ten64_controller_10g_enable_command(struct udevice *ucdev, u8 value)
{
	int ret;
	struct t64uc_board_10g_enable enable_msg;

	enable_msg.control = value;

	ret = ten64_controller_send_command(ucdev, TEN64_UC_CMD_ENABLE_10G,
					    &enable_msg, sizeof(enable_msg));
	if (ret) {
		dev_err(ucdev, "ERROR sending uC 10G Enable message: %d\n", ret);
		return -1;
	}

	return 0;
}

int ten64_controller_call(struct udevice *dev, int msgid, void *tx_msg, int tx_size,
			  void *rx_msg, int rx_size)
{
	switch (msgid) {
	case TEN64_CNTRL_GET_BOARD_INFO:
		return ten64_controller_get_board_info(dev, (struct t64uc_board_info *)rx_msg);
	case TEN64_CNTRL_10G_OFF:
		return ten64_controller_10g_enable_command(dev, T64_CTRL_IO_CLEAR);
	case TEN64_CNTRL_10G_ON:
		return ten64_controller_10g_enable_command(dev, T64_CTRL_IO_SET);
	default:
		dev_err(dev, "%s: Unknown operation %d\n", __func__, msgid);
	}
	return -EINVAL;
}

static struct misc_ops ten64_ctrl_ops  = {
	.call = ten64_controller_call
};

static const struct udevice_id ten64_controller_ids[] = {
	{.compatible = "traverse,ten64-controller"},
	{}
};

U_BOOT_DRIVER(ten64_controller) = {
	.name = "ten64-controller-i2c",
	.id = UCLASS_MISC,
	.of_match = ten64_controller_ids,
	.ops = &ten64_ctrl_ops
};
