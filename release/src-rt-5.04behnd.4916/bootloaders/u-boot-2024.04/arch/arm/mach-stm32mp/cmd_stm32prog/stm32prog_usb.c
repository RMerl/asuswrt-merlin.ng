// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dfu.h>
#include <g_dnl.h>
#include <usb.h>
#include <asm/arch/stm32prog.h>
#include <asm/arch/sys_proto.h>
#include <linux/printk.h>
#include "stm32prog.h"

static int stm32prog_set_phase(struct stm32prog_data *data, u8 phase,
			       u32 offset)
{
	struct stm32prog_part_t *part;
	int i;

	if (phase == data->phase) {
		data->offset = offset;
		data->dfu_seq = 0;
		return 0;
	}

	/* found partition for phase */
	for (i = 0; i < data->part_nb; i++) {
		part = &data->part_array[i];
		if (part->id == phase) {
			data->cur_part = part;
			data->phase = phase;
			data->offset = offset;
			data->dfu_seq = 0;
			return 0;
		}
	}

	return  -EINVAL;
}

static int stm32prog_cmd_write(u64 offset, void *buf, long *len)
{
	u8 phase;
	uintptr_t address;
	u8 *pt = buf;
	void (*entry)(void);
	int ret;

	if (*len < 5) {
		log_err("size not allowed\n");
		return  -EINVAL;
	}
	if (offset) {
		log_err("invalid offset\n");
		return  -EINVAL;
	}
	phase = pt[0];
	address = (pt[1] << 24) | (pt[2] << 16) | (pt[3] << 8) | pt[4];
	if (phase == PHASE_RESET) {
		entry = (void *)address;
		printf("## Starting application at 0x%p ...\n", entry);
		(*entry)();
		printf("## Application terminated\n");
		return 0;
	}
	/* set phase and offset */
	ret = stm32prog_set_phase(stm32prog_data, phase, address);
	if (ret)
		log_err("failed: %d\n", ret);
	return ret;
}

#define PHASE_MIN_SIZE	9
static int stm32prog_cmd_read(u64 offset, void *buf, long *len)
{
	u32 destination = DEFAULT_ADDRESS; /* destination address */
	u32 dfu_offset;
	u8 *pt_buf = buf;
	int phase;
	char *err_msg;
	int length;

	if (*len < PHASE_MIN_SIZE) {
		log_err("request exceeds allowed area\n");
		return  -EINVAL;
	}
	if (offset) {
		*len = 0; /* EOF for second request */
		return 0;
	}
	phase = stm32prog_data->phase;
	if (phase == PHASE_FLASHLAYOUT)
		destination = CONFIG_SYS_LOAD_ADDR;
	dfu_offset = stm32prog_data->offset;

	/* mandatory header, size = PHASE_MIN_SIZE */
	*pt_buf++ = (u8)(phase & 0xFF);
	*pt_buf++ = (u8)(destination);
	*pt_buf++ = (u8)(destination >> 8);
	*pt_buf++ = (u8)(destination >> 16);
	*pt_buf++ = (u8)(destination >> 24);
	*pt_buf++ = (u8)(dfu_offset);
	*pt_buf++ = (u8)(dfu_offset >> 8);
	*pt_buf++ = (u8)(dfu_offset >> 16);
	*pt_buf++ = (u8)(dfu_offset >> 24);

	if (phase == PHASE_RESET || phase == PHASE_DO_RESET) {
		err_msg = stm32prog_get_error(stm32prog_data);
		length = strlen(err_msg);
		if (length + PHASE_MIN_SIZE > *len)
			length = *len - PHASE_MIN_SIZE;

		memcpy(pt_buf, err_msg, length);
		*len = PHASE_MIN_SIZE + length;
		stm32prog_do_reset(stm32prog_data);
	} else if (phase == PHASE_FLASHLAYOUT) {
		*pt_buf++ = stm32prog_data->part_nb ? 1 : 0;
		*len = PHASE_MIN_SIZE + 1;
	} else {
		*len = PHASE_MIN_SIZE;
	}

	return 0;
}

int stm32prog_write_medium_virt(struct dfu_entity *dfu, u64 offset,
				void *buf, long *len)
{
	if (dfu->dev_type != DFU_DEV_VIRT)
		return -EINVAL;

	switch (dfu->data.virt.dev_num) {
	case PHASE_CMD:
		return stm32prog_cmd_write(offset, buf, len);

	case PHASE_OTP:
		return stm32prog_otp_write(stm32prog_data, (u32)offset,
					   buf, len);

	case PHASE_PMIC:
		return stm32prog_pmic_write(stm32prog_data, (u32)offset,
					    buf, len);
	}
	*len = 0;
	return 0;
}

int stm32prog_read_medium_virt(struct dfu_entity *dfu, u64 offset,
			       void *buf, long *len)
{
	if (dfu->dev_type != DFU_DEV_VIRT)
		return -EINVAL;

	switch (dfu->data.virt.dev_num) {
	case PHASE_CMD:
		return stm32prog_cmd_read(offset, buf, len);

	case PHASE_OTP:
		return stm32prog_otp_read(stm32prog_data, (u32)offset,
					  buf, len);

	case PHASE_PMIC:
		return stm32prog_pmic_read(stm32prog_data, (u32)offset,
					   buf, len);
	}
	*len = 0;
	return 0;
}

int stm32prog_get_medium_size_virt(struct dfu_entity *dfu, u64 *size)
{
	if (dfu->dev_type != DFU_DEV_VIRT) {
		*size = 0;
		log_debug("%s, invalid dev_type = %d\n",
			  __func__, dfu->dev_type);
		return -EINVAL;
	}

	switch (dfu->data.virt.dev_num) {
	case PHASE_CMD:
		*size = CMD_SIZE;
		break;
	case PHASE_OTP:
		*size = stm32prog_data->tee ? OTP_SIZE_TA : OTP_SIZE_SMC;
		break;
	case PHASE_PMIC:
		*size = PMIC_SIZE;
		break;
	}

	return 0;
}

bool stm32prog_usb_loop(struct stm32prog_data *data, int dev)
{
	int ret;
	bool result;
	/* USB download gadget for STM32 Programmer */
	char product[128];
	char name[SOC_NAME_SIZE];

	get_soc_name(name);
	snprintf(product, sizeof(product),
		 "USB download gadget@Device ID /0x%03X, @Revision ID /0x%04X, @Name /%s,",
		 get_cpu_dev(), get_cpu_rev(), name);
	g_dnl_set_product(product);

	if (stm32prog_data->phase == PHASE_FLASHLAYOUT) {
		/* forget any previous Control C */
		clear_ctrlc();
		ret = run_usb_dnl_gadget(dev, "usb_dnl_dfu");
		/* DFU reset received, no error or CtrlC */
		if (ret || stm32prog_data->phase != PHASE_FLASHLAYOUT || had_ctrlc())
			return ret; /* true = reset on DFU error */
		/* prepare the second enumeration with the FlashLayout */
		stm32prog_dfu_init(data);
	}

	ret = run_usb_dnl_gadget(dev, "usb_dnl_dfu");

	result = !!(ret) || (stm32prog_data->phase == PHASE_DO_RESET);

	g_dnl_set_product(NULL);

	return result;
}

int g_dnl_get_board_bcd_device_number(int gcnum)
{
	log_debug("%s\n", __func__);
	return 0x200;
}
