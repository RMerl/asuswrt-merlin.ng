// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <dfu.h>
#include <malloc.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/arch/sys_proto.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <asm/global_data.h>
#include "stm32prog.h"

/* - configuration part -----------------------------*/
#define USART_BL_VERSION	0x40	/* USART bootloader version V4.0*/
#define UBOOT_BL_VERSION	0x03	/* bootloader version V0.3*/

#define USART_RAM_BUFFER_SIZE	256	/* Size of USART_RAM_Buf buffer*/

/* - Commands -----------------------------*/
#define GET_CMD_COMMAND		0x00	/* Get CMD command*/
#define GET_VER_COMMAND		0x01	/* Get Version command*/
#define GET_ID_COMMAND		0x02	/* Get ID command*/
#define GET_PHASE_COMMAND	0x03	/* Get Phase command*/
#define RM_COMMAND		0x11	/* Read Memory command*/
#define READ_PART_COMMAND	0x12	/* Read Partition command*/
#define START_COMMAND		0x21	/* START command (Go)*/
#define DOWNLOAD_COMMAND	0x31	/* Download command*/
/* existing command for other STM32 but not used */
/* ERASE			0x43 */
/* EXTENDED_ERASE		0x44 */
/* WRITE_UNPROTECTED		0x73 */
/* READOUT_PROTECT		0x82 */
/* READOUT_UNPROTECT		0x92 */

/* - miscellaneous defines ----------------------------------------*/
#define INIT_BYTE		0x7F	/*Init Byte ID*/
#define ACK_BYTE		0x79	/*Acknowlede Byte ID*/
#define NACK_BYTE		0x1F	/*No Acknowlede Byte ID*/
#define ABORT_BYTE		0x5F	/*ABORT*/

struct udevice *down_serial_dev;

const u8 cmd_id[] = {
	GET_CMD_COMMAND,
	GET_VER_COMMAND,
	GET_ID_COMMAND,
	GET_PHASE_COMMAND,
	RM_COMMAND,
	READ_PART_COMMAND,
	START_COMMAND,
	DOWNLOAD_COMMAND
};

#define NB_CMD sizeof(cmd_id)

/* with 115200 bauds, 20 ms allow to receive the 256 bytes buffer */
#define TIMEOUT_SERIAL_BUFFER	30

/* DFU support for serial *********************************************/
static struct dfu_entity *stm32prog_get_entity(struct stm32prog_data *data)
{
	int alt_id;

	if (!data->cur_part)
		if (data->phase == PHASE_FLASHLAYOUT)
			alt_id = 0;
		else
			return NULL;
	else
		alt_id = data->cur_part->alt_id;

	return dfu_get_entity(alt_id);
}

static int stm32prog_write(struct stm32prog_data *data, u8 *buffer,
			   u32 buffer_size)
{
	struct dfu_entity *dfu_entity;
	u8 ret = 0;

	dfu_entity = stm32prog_get_entity(data);
	if (!dfu_entity)
		return -ENODEV;

	ret = dfu_write(dfu_entity,
			buffer,
			buffer_size,
			data->dfu_seq);

	if (ret) {
		stm32prog_err("DFU write failed [%d] cnt: %d",
			      ret, data->dfu_seq);
	}
	data->dfu_seq++;
	/* handle rollover as in driver/dfu/dfu.c */
	data->dfu_seq &= 0xffff;
	if (buffer_size == 0)
		data->dfu_seq = 0; /* flush done */

	return ret;
}

static int stm32prog_read(struct stm32prog_data *data, u8 phase, u32 offset,
			  u8 *buffer, u32 buffer_size)
{
	struct dfu_entity *dfu_entity;
	struct stm32prog_part_t *part;
	u32 size;
	int ret, i;

	if (data->dfu_seq) {
		stm32prog_err("DFU write pending for phase %d, seq %d",
			      data->phase, data->dfu_seq);
		return -EINVAL;
	}
	if (phase == PHASE_FLASHLAYOUT || phase > PHASE_LAST_USER) {
		stm32prog_err("read failed : phase %d is invalid", phase);
		return -EINVAL;
	}
	if (data->read_phase <= PHASE_LAST_USER &&
	    phase != data->read_phase) {
		/* clear previous read session */
		dfu_entity = dfu_get_entity(data->read_phase - 1);
		if (dfu_entity)
			dfu_transaction_cleanup(dfu_entity);
	}

	dfu_entity = NULL;
	/* found partition for the expected phase */
	for (i = 0; i < data->part_nb; i++) {
		part = &data->part_array[i];
		if (part->id == phase)
			dfu_entity = dfu_get_entity(part->alt_id);
	}
	if (!dfu_entity) {
		stm32prog_err("read failed : phase %d is unknown", phase);
		return -ENODEV;
	}

	/* clear pending read before to force offset */
	if (dfu_entity->inited &&
	    (data->read_phase != phase || data->offset != offset))
		dfu_transaction_cleanup(dfu_entity);

	/* initiate before to force offset */
	if (!dfu_entity->inited) {
		ret = dfu_transaction_initiate(dfu_entity, true);
			if (ret < 0) {
				stm32prog_err("DFU read init failed [%d] phase = %d offset = 0x%08x",
					      ret, phase, offset);
			return ret;
		}
	}
	/* force new offset */
	if (dfu_entity->offset != offset)
		dfu_entity->offset = offset;
	data->offset = offset;
	data->read_phase = phase;
	log_debug("\nSTM32 download read %s offset=0x%x\n",
		  dfu_entity->name, offset);
	ret = dfu_read(dfu_entity, buffer, buffer_size,
		       dfu_entity->i_blk_seq_num);
	if (ret < 0) {
		stm32prog_err("DFU read failed [%d] phase = %d offset = 0x%08x",
			      ret, phase, offset);
		return ret;
	}

	size = ret;

	if (size < buffer_size) {
		data->offset = 0;
		data->read_phase = PHASE_END;
		memset(buffer + size, 0, buffer_size - size);
	} else {
		data->offset += size;
	}

	return ret;
}

/* UART access ***************************************************/
int stm32prog_serial_init(struct stm32prog_data *data, int link_dev)
{
	struct udevice *dev = NULL;
	struct dm_serial_ops *ops;
	/* no parity, 8 bits, 1 stop */
	u32 serial_config = SERIAL_DEFAULT_CONFIG;

	down_serial_dev = NULL;

	if (uclass_get_device_by_seq(UCLASS_SERIAL, link_dev, &dev)) {
		log_err("serial %d device not found\n", link_dev);
		return -ENODEV;
	}

	down_serial_dev = dev;

	/* force silent console on uart only when used */
	if (gd->cur_serial_dev == down_serial_dev)
		gd->flags |= GD_FLG_DISABLE_CONSOLE | GD_FLG_SILENT;
	else
		gd->flags &= ~(GD_FLG_DISABLE_CONSOLE | GD_FLG_SILENT);

	ops = serial_get_ops(down_serial_dev);

	if (!ops) {
		log_err("serial %d = %s missing ops\n", link_dev, dev->name);
		return -ENODEV;
	}
	if (!ops->setconfig) {
		log_err("serial %d = %s missing setconfig\n", link_dev, dev->name);
		return -ENODEV;
	}

	clrsetbits_le32(&serial_config, SERIAL_PAR_MASK, SERIAL_PAR_EVEN);

	data->buffer = memalign(CONFIG_SYS_CACHELINE_SIZE,
				USART_RAM_BUFFER_SIZE);

	return ops->setconfig(down_serial_dev, serial_config);
}

static void stm32prog_serial_flush(void)
{
	struct dm_serial_ops *ops = serial_get_ops(down_serial_dev);
	int err;

	do {
		err = ops->getc(down_serial_dev);
	} while (err != -EAGAIN);
}

static int stm32prog_serial_getc_err(void)
{
	struct dm_serial_ops *ops = serial_get_ops(down_serial_dev);
	int err;

	do {
		err = ops->getc(down_serial_dev);
		if (err == -EAGAIN) {
			ctrlc();
			schedule();
		}
	} while ((err == -EAGAIN) && (!had_ctrlc()));

	return err;
}

static u8 stm32prog_serial_getc(void)
{
	int err;

	err = stm32prog_serial_getc_err();

	return err >= 0 ? err : 0;
}

static bool stm32prog_serial_get_buffer(u8 *buffer, u32 *count)
{
	struct dm_serial_ops *ops = serial_get_ops(down_serial_dev);
	int err;
	ulong start = get_timer(0);

	do {
		err = ops->getc(down_serial_dev);
		if (err >= 0) {
			*buffer++ = err;
			*count -= 1;
		} else if (err == -EAGAIN) {
			ctrlc();
			schedule();
			if (get_timer(start) > TIMEOUT_SERIAL_BUFFER) {
				err = -ETIMEDOUT;
				break;
			}
		} else {
			break;
		}
	} while (*count && !had_ctrlc());

	return !!(err < 0);
}

static void stm32prog_serial_putc(u8 w_byte)
{
	struct dm_serial_ops *ops = serial_get_ops(down_serial_dev);
	int err;

	do {
		err = ops->putc(down_serial_dev, w_byte);
	} while (err == -EAGAIN);
}

/* Helper function ************************************************/
static u8 stm32prog_start(struct stm32prog_data *data, uintptr_t address)
{
	u8 ret = 0;
	struct dfu_entity *dfu_entity;

	if (address < 0x100) {
		if (address == PHASE_OTP)
			return stm32prog_otp_start(data);

		if (address == PHASE_PMIC)
			return stm32prog_pmic_start(data);

		if (address == PHASE_RESET || address == PHASE_END) {
			data->cur_part = NULL;
			data->dfu_seq = 0;
			data->phase = address;
			return 0;
		}
		if (address != data->phase) {
			stm32prog_err("invalid received phase id %d, current phase is %d",
				      (u8)address, (u8)data->phase);
			return -EINVAL;
		}
	}
	/* check the last loaded partition */
	if (address == DEFAULT_ADDRESS || address == data->phase) {
		switch (data->phase) {
		case PHASE_END:
		case PHASE_RESET:
		case PHASE_DO_RESET:
			data->cur_part = NULL;
			data->phase = PHASE_DO_RESET;
			return 0;
		}
		dfu_entity = stm32prog_get_entity(data);
		if (!dfu_entity)
			return -ENODEV;

		ret = dfu_flush(dfu_entity, NULL, 0, data->dfu_seq);
		if (ret) {
			stm32prog_err("DFU flush failed [%d]", ret);
			return ret;
		}
		data->dfu_seq = 0;

		printf("\n  received length = 0x%x\n", data->cursor);

		/* update DFU with received flashlayout */
		if (data->phase == PHASE_FLASHLAYOUT)
			stm32prog_dfu_init(data);
	} else {
		void (*entry)(void) = (void *)address;

		printf("## Starting application at 0x%p ...\n", (void *)address);
		(*entry)();
		printf("## Application terminated\n");
		ret = -ENOEXEC;
	}

	return ret;
}

/**
 * get_address() - Get address if it is valid
 *
 * @tmp_xor:		Current xor value to update
 * Return: The address area
 */
static uintptr_t get_address(u8 *tmp_xor)
{
	uintptr_t address = 0x0;
	u8 data;

	data = stm32prog_serial_getc();
	*tmp_xor ^= data;
	address |= ((u32)data) << 24;

	data = stm32prog_serial_getc();
	address |= ((u32)data) << 16;
	*tmp_xor ^= data;

	data = stm32prog_serial_getc();
	address |= ((u32)data) << 8;
	*tmp_xor ^= data;

	data = stm32prog_serial_getc();
	address |= ((u32)data);
	*tmp_xor ^= data;

	return address;
}

static void stm32prog_serial_result(u8 result)
{
	/* always flush fifo before to send result */
	stm32prog_serial_flush();
	stm32prog_serial_putc(result);
}

/* Command -----------------------------------------------*/
/**
 * get_cmd_command() - Respond to Get command
 *
 * @data:		Current command context
 */
static void get_cmd_command(struct stm32prog_data *data)
{
	u32 counter = 0x0;

	stm32prog_serial_putc(NB_CMD);
	stm32prog_serial_putc(USART_BL_VERSION);

	for (counter = 0; counter < NB_CMD; counter++)
		stm32prog_serial_putc(cmd_id[counter]);

	stm32prog_serial_result(ACK_BYTE);
}

/**
 * get_version_command() - Respond to Get Version command
 *
 * @data:		Current command context
 */
static void get_version_command(struct stm32prog_data *data)
{
	stm32prog_serial_putc(UBOOT_BL_VERSION);
	stm32prog_serial_result(ACK_BYTE);
}

/**
 * get_id_command() - Respond to Get ID command
 *
 * @data:		Current command context
 */
static void get_id_command(struct stm32prog_data *data)
{
	u32 cpu = get_cpu_dev();

	/* Send Device IDCode */
	stm32prog_serial_putc(0x1);
	stm32prog_serial_putc((cpu >> 8) & 0xFF);
	stm32prog_serial_putc(cpu & 0xFF);
	stm32prog_serial_result(ACK_BYTE);
}

/**
 * get_phase_command() - Respond to Get phase
 *
 * @data:		Current command context
 */
static void get_phase_command(struct stm32prog_data *data)
{
	char *err_msg = NULL;
	u8 i, length = 0;
	u32 destination = DEFAULT_ADDRESS; /* destination address */
	int phase = data->phase;

	if (phase == PHASE_RESET || phase == PHASE_DO_RESET) {
		err_msg = stm32prog_get_error(data);
		length = strlen(err_msg);
	}
	if (phase == PHASE_FLASHLAYOUT)
		destination = CONFIG_SYS_LOAD_ADDR;

	stm32prog_serial_putc(length + 5);           /* Total length */
	stm32prog_serial_putc(phase & 0xFF);         /* partition ID */
	stm32prog_serial_putc(destination);          /* byte 1 of address */
	stm32prog_serial_putc(destination >> 8);     /* byte 2 of address */
	stm32prog_serial_putc(destination >> 16);    /* byte 3 of address */
	stm32prog_serial_putc(destination >> 24);    /* byte 4 of address */

	stm32prog_serial_putc(length);               /* Information length */
	for (i = 0; i < length; i++)
		stm32prog_serial_putc(err_msg[i]);
	stm32prog_serial_result(ACK_BYTE);

	if (phase == PHASE_RESET)
		stm32prog_do_reset(data);
}

/**
 * read_memory_command() - Read data from memory
 *
 * @data:		Current command context
 */
static void read_memory_command(struct stm32prog_data *data)
{
	uintptr_t address = 0x0;
	u8 rcv_data = 0x0, tmp_xor = 0x0;
	u32 counter = 0x0;

	/* Read memory address */
	address = get_address(&tmp_xor);

	/* If address memory is not received correctly */
	rcv_data = stm32prog_serial_getc();
	if (rcv_data != tmp_xor) {
		stm32prog_serial_result(NACK_BYTE);
		return;
	}

	stm32prog_serial_result(ACK_BYTE);

	/* Read the number of bytes to be received:
	 * Max NbrOfData = Data + 1 = 256
	 */
	rcv_data = stm32prog_serial_getc();
	tmp_xor = ~rcv_data;
	if (stm32prog_serial_getc() != tmp_xor) {
		stm32prog_serial_result(NACK_BYTE);
		return;
	}

	/* If checksum is correct send ACK */
	stm32prog_serial_result(ACK_BYTE);

	/* Send data to the host:
	 * Number of data to read = data + 1
	 */
	for (counter = (rcv_data + 1); counter != 0; counter--)
		stm32prog_serial_putc(*(u8 *)(address++));
}

/**
 * start_command() - Respond to start command
 *
 * Jump to user application in RAM or partition check
 *
 * @data:		Current command context
 */
static void start_command(struct stm32prog_data *data)
{
	uintptr_t address = 0;
	u8 tmp_xor = 0x0;
	u8 ret, rcv_data;

	/* Read memory address */
	address = get_address(&tmp_xor);

	/* If address memory is not received correctly */
	rcv_data = stm32prog_serial_getc();
	if (rcv_data != tmp_xor) {
		stm32prog_serial_result(NACK_BYTE);
		return;
	}
	/* validate partition */
	ret = stm32prog_start(data, address);

	if (ret)
		stm32prog_serial_result(ABORT_BYTE);
	else
		stm32prog_serial_result(ACK_BYTE);
}

/**
 * download_command() - Respond to download command
 *
 * Write data to not volatile memory, Flash
 *
 * @data:		Current command context
 */
static void download_command(struct stm32prog_data *data)
{
	u32 address = 0x0;
	u8 my_xor = 0x0;
	u8 rcv_xor;
	u32 counter = 0x0, codesize = 0x0;
	u8 *ramaddress = 0;
	u8 rcv_data = 0x0;
	u32 cursor = data->cursor;
	long size = 0;
	u8 operation;
	u32 packet_number;
	u32 result = ACK_BYTE;
	u8 ret;
	bool error;
	int rcv;

	address = get_address(&my_xor);

	/* If address memory is not received correctly */
	rcv_xor = stm32prog_serial_getc();
	if (rcv_xor != my_xor) {
		result = NACK_BYTE;
		goto end;
	}

	/* If address valid send ACK */
	stm32prog_serial_result(ACK_BYTE);

	/* get packet number and operation type */
	operation = (u8)((u32)address >> 24);
	packet_number = ((u32)(((u32)address << 8))) >> 8;

	switch (operation) {
	/* supported operation */
	case PHASE_FLASHLAYOUT:
	case PHASE_OTP:
	case PHASE_PMIC:
		break;
	default:
		result = NACK_BYTE;
		goto end;
	}
	/* check the packet number */
	if (packet_number == 0) {
		/* erase: re-initialize the image_header struct */
		data->packet_number = 0;
		cursor = 0;
		data->cursor = 0;
		/*idx = cursor;*/
	} else {
		data->packet_number++;
	}

	/* Check with the number of current packet if the device receive
	 * the true packet
	 */
	if (packet_number != data->packet_number) {
		data->packet_number--;
		result = NACK_BYTE;
		goto end;
	}

	/*-- Read number of bytes to be written and data -----------*/

	/* Read the number of bytes to be written:
	 * Max NbrOfData = data + 1 <= 256
	 */
	rcv_data = stm32prog_serial_getc();

	/* NbrOfData to write = data + 1 */
	codesize = rcv_data + 0x01;

	if (codesize > USART_RAM_BUFFER_SIZE) {
		result = NACK_BYTE;
		goto end;
	}

	/* Checksum Initialization */
	my_xor = rcv_data;

	/* UART receive data and send to Buffer */
	counter = codesize;
	error = stm32prog_serial_get_buffer(data->buffer, &counter);

	/* read checksum */
	if (!error) {
		rcv = stm32prog_serial_getc_err();
		error = !!(rcv < 0);
		rcv_xor = rcv;
	}

	if (error) {
		printf("transmission error on packet %d, byte %d\n",
		       packet_number, codesize - counter);
		/* waiting end of packet before flush & NACK */
		mdelay(TIMEOUT_SERIAL_BUFFER);
		data->packet_number--;
		result = NACK_BYTE;
		goto end;
	}

	/* Compute Checksum */
	ramaddress = data->buffer;
	for (counter = codesize; counter != 0; counter--)
		my_xor ^= *(ramaddress++);

	/* If Checksum is incorrect */
	if (rcv_xor != my_xor) {
		printf("checksum error on packet %d\n",
		       packet_number);
		/* wait to be sure that all data are received
		 * in the FIFO before flush
		 */
		mdelay(TIMEOUT_SERIAL_BUFFER);
		data->packet_number--;
		result = NACK_BYTE;
		goto end;
	}

	switch (operation) {
	case PHASE_OTP:
		size = codesize;
		ret = stm32prog_otp_write(data, cursor, data->buffer, &size);
		break;

	case PHASE_PMIC:
		size = codesize;
		ret = stm32prog_pmic_write(data, cursor, data->buffer, &size);
		break;

	default:
		ret = stm32prog_write(data, data->buffer, codesize);
		break;
	}

	if (ret)
		result = ABORT_BYTE;
	else
		/* Update current position in buffer */
		data->cursor += codesize;

end:
	stm32prog_serial_result(result);
}

/**
 * read_partition() - Respond to read command
 *
 * Read data from not volatile memory, Flash
 *
 * @data:		Current command context
 */
static void read_partition_command(struct stm32prog_data *data)
{
	u32 i, part_id, codesize, offset = 0, rcv_data;
	long size;
	u8 tmp_xor;
	int res;
	u8 buffer[256];

	part_id = stm32prog_serial_getc();
	tmp_xor = part_id;

	offset = get_address(&tmp_xor);

	rcv_data = stm32prog_serial_getc();
	if (rcv_data != tmp_xor) {
		log_debug("1st checksum received = %x, computed %x\n",
			  rcv_data, tmp_xor);
		goto error;
	}
	stm32prog_serial_putc(ACK_BYTE);

	/* NbrOfData to read = data + 1 */
	rcv_data = stm32prog_serial_getc();
	codesize = rcv_data + 0x01;
	tmp_xor = rcv_data;

	rcv_data = stm32prog_serial_getc();
	if ((rcv_data ^ tmp_xor) != 0xFF) {
		log_debug("2nd checksum received = %x, computed %x\n",
			  rcv_data, tmp_xor);
		goto error;
	}

	log_debug("%s : %x\n", __func__, part_id);
	rcv_data = 0;
	switch (part_id) {
	case PHASE_OTP:
		size = codesize;
		if (!stm32prog_otp_read(data, offset, buffer, &size))
			rcv_data = size;
		break;
	case PHASE_PMIC:
		size = codesize;
		if (!stm32prog_pmic_read(data, offset, buffer, &size))
			rcv_data = size;
		break;
	default:
		res = stm32prog_read(data, part_id, offset,
				     buffer, codesize);
		if (res > 0)
			rcv_data = res;
		break;
	}
	if (rcv_data > 0) {
		stm32prog_serial_putc(ACK_BYTE);
		/*----------- Send data to the host -----------*/
		for (i = 0; i < rcv_data; i++)
			stm32prog_serial_putc(buffer[i]);
		/*----------- Send filler to the host -----------*/
		for (; i < codesize; i++)
			stm32prog_serial_putc(0x0);
		return;
	}
	stm32prog_serial_result(ABORT_BYTE);
	return;

error:
	stm32prog_serial_result(NACK_BYTE);
}

/* MAIN function = SERIAL LOOP ***********************************************/

/**
 * stm32prog_serial_loop() - USART bootloader Loop routine
 *
 * @data:		Current command context
 * Return: true if reset is needed after loop
 */
bool stm32prog_serial_loop(struct stm32prog_data *data)
{
	u32 counter = 0x0;
	u8 command = 0x0;
	u8 found;
	int phase = data->phase;

	/* element of cmd_func need to aligned with cmd_id[]*/
	void (*cmd_func[NB_CMD])(struct stm32prog_data *) = {
		/* GET_CMD_COMMAND */	get_cmd_command,
		/* GET_VER_COMMAND */	get_version_command,
		/* GET_ID_COMMAND */	get_id_command,
		/* GET_PHASE_COMMAND */	get_phase_command,
		/* RM_COMMAND */	read_memory_command,
		/* READ_PART_COMMAND */	read_partition_command,
		/* START_COMMAND */	start_command,
		/* DOWNLOAD_COMMAND */	download_command
	};

	/* flush and NACK pending command received during u-boot init
	 * request command reemit
	 */
	stm32prog_serial_result(NACK_BYTE);

	clear_ctrlc(); /* forget any previous Control C */
	while (!had_ctrlc()) {
		phase = data->phase;

		if (phase == PHASE_DO_RESET)
			return true;

		/* Get the user command: read first byte */
		command = stm32prog_serial_getc();

		if (command == INIT_BYTE) {
			puts("\nConnected\n");
			stm32prog_serial_result(ACK_BYTE);
			continue;
		}

		found = 0;
		for (counter = 0; counter < NB_CMD; counter++)
			if (cmd_id[counter] == command) {
				found = 1;
				break;
			}
		if (found)
			if ((command ^ stm32prog_serial_getc()) != 0xFF)
				found = 0;
		if (!found) {
			/* wait to be sure that all data are received
			 * in the FIFO before flush (CMD and XOR)
			 */
			mdelay(3);
			stm32prog_serial_result(NACK_BYTE);
		} else {
			stm32prog_serial_result(ACK_BYTE);
			cmd_func[counter](data);
		}
		schedule();
	}

	/* clean device */
	if (gd->cur_serial_dev == down_serial_dev) {
		/* restore console on uart */
		gd->flags &= ~(GD_FLG_DISABLE_CONSOLE | GD_FLG_SILENT);
	}
	down_serial_dev = NULL;

	return false; /* no reset after ctrlc */
}
