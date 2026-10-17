/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#ifndef _STM32PROG_H_
#define _STM32PROG_H_

#include <linux/printk.h>

/* - phase defines ------------------------------------------------*/
#define PHASE_FLASHLAYOUT	0x00
#define PHASE_FIRST_USER	0x10
#define PHASE_LAST_USER		0xF0
#define PHASE_CMD		0xF1
#define PHASE_OTP		0xF2
#define PHASE_PMIC		0xF4
#define PHASE_END		0xFE
#define PHASE_RESET		0xFF
#define PHASE_DO_RESET		0x1FF

#define DEFAULT_ADDRESS		0xFFFFFFFF

#define CMD_SIZE		512
/* SMC is only supported in SPMIN for STM32MP15x */
#ifdef CONFIG_STM32MP15X
#define OTP_SIZE_SMC		1024
#else
#define OTP_SIZE_SMC		0
#endif
/* size of the OTP struct in NVMEM PTA */
#define _OTP_SIZE_TA(otp)	(((otp) * 2 + 2) * 4)
#if defined(CONFIG_STM32MP13X) || defined(CONFIG_STM32MP15X)
/* STM32MP1 with BSEC2 */
#define OTP_SIZE_TA		_OTP_SIZE_TA(96)
#else
/* STM32MP2 with BSEC3 */
#define OTP_SIZE_TA		_OTP_SIZE_TA(368)
#endif
#define PMIC_SIZE		8

enum stm32prog_target {
	STM32PROG_NONE,
	STM32PROG_MMC,
	STM32PROG_NAND,
	STM32PROG_NOR,
	STM32PROG_SPI_NAND,
	STM32PROG_RAM
};

enum stm32prog_link_t {
	LINK_SERIAL,
	LINK_USB,
	LINK_UNDEFINED,
};

enum stm32prog_header_t {
	HEADER_NONE,
	HEADER_STM32IMAGE,
	HEADER_STM32IMAGE_V2,
	HEADER_FIP,
};

struct image_header_s {
	enum stm32prog_header_t type;
	u32	image_checksum;
	u32	image_length;
	u32	length;
};

struct stm32_header_v1 {
	u32 magic_number;
	u8 image_signature[64];
	u32 image_checksum;
	u32 header_version;
	u32 image_length;
	u32 image_entry_point;
	u32 reserved1;
	u32 load_address;
	u32 reserved2;
	u32 version_number;
	u32 option_flags;
	u32 ecdsa_algorithm;
	u8 ecdsa_public_key[64];
	u8 padding[83];
	u8 binary_type;
};

struct stm32_header_v2 {
	u32 magic_number;
	u8 image_signature[64];
	u32 image_checksum;
	u32 header_version;
	u32 image_length;
	u32 image_entry_point;
	u32 reserved1;
	u32 load_address;
	u32 reserved2;
	u32 version_number;
	u32 extension_flags;
	u32 extension_headers_length;
	u32 binary_type;
	u8 padding[16];
	u32 extension_header_type;
	u32 extension_header_length;
	u8 extension_padding[376];
};

/*
 * partition type in flashlayout file
 * SYSTEM = linux partition, bootable
 * FILESYSTEM = linux partition
 * ESP = EFI system partition
 */
enum stm32prog_part_type {
	PART_BINARY,
	PART_FIP,
	PART_FWU_MDATA,
	PART_ENV,
	PART_SYSTEM,
	PART_FILESYSTEM,
	PART_ESP,
	RAW_IMAGE,
};

/* device information */
struct stm32prog_dev_t {
	enum stm32prog_target	target;
	char			dev_id;
	u32			erase_size;
	struct mmc		*mmc;
	struct mtd_info		*mtd;
	/* list of partition for this device / ordered in offset */
	struct list_head	part_list;
	bool			full_update;
};

/* partition information build from FlashLayout and device */
struct stm32prog_part_t {
	/* FlashLayout information */
	int			option;
	int			id;
	enum stm32prog_part_type part_type;
	enum stm32prog_target	target;
	char			dev_id;

	/* partition name
	 * (16 char in gpt, + 1 for null terminated string
	 */
	char			name[16 + 1];
	u64			addr;
	u64			size;
	enum stm32prog_part_type bin_nb;	/* SSBL repeatition */

	/* information on associated device */
	struct stm32prog_dev_t	*dev;		/* pointer to device */
	s16			part_id;	/* partition id in device */
	int			alt_id;		/* alt id in usb/dfu */

	struct list_head	list;
};

#define STM32PROG_MAX_DEV 5
struct stm32prog_data {
	/* Layout information */
	int			dev_nb;		/* device number*/
	struct stm32prog_dev_t	dev[STM32PROG_MAX_DEV];	/* array of device */
	int			part_nb;	/* nb of partition */
	struct stm32prog_part_t	*part_array;	/* array of partition */
	bool			fsbl_nor_detected;

	/* command internal information */
	unsigned int		phase;
	u32			offset;
	char			error[255];
	struct stm32prog_part_t	*cur_part;
	void			*otp_part;
	u8			pmic_part[PMIC_SIZE];

	/* SERIAL information */
	u32	cursor;
	u32	packet_number;
	u8	*buffer; /* size = USART_RAM_BUFFER_SIZE*/
	int	dfu_seq;
	u8	read_phase;

	/* bootm information */
	uintptr_t	uimage;
	uintptr_t	dtb;
	uintptr_t	initrd;
	size_t		initrd_size;

	uintptr_t	script;

	/* OPTEE PTA NVMEM */
	struct udevice *tee;
	u32 tee_session;
};

extern struct stm32prog_data *stm32prog_data;

/* OTP access */
int stm32prog_otp_write(struct stm32prog_data *data, u32 offset,
			u8 *buffer, long *size);
int stm32prog_otp_read(struct stm32prog_data *data, u32 offset,
		       u8 *buffer, long *size);
int stm32prog_otp_start(struct stm32prog_data *data);

/* PMIC access */
int stm32prog_pmic_write(struct stm32prog_data *data, u32 offset,
			 u8 *buffer, long *size);
int stm32prog_pmic_read(struct stm32prog_data *data, u32 offset,
			u8 *buffer, long *size);
int stm32prog_pmic_start(struct stm32prog_data *data);

/* generic part*/
void stm32prog_header_check(uintptr_t raw_header, struct image_header_s *header);
int stm32prog_dfu_init(struct stm32prog_data *data);
void stm32prog_next_phase(struct stm32prog_data *data);
void stm32prog_do_reset(struct stm32prog_data *data);

char *stm32prog_get_error(struct stm32prog_data *data);

#define stm32prog_err(args...) {\
	if (data->phase != PHASE_RESET) { \
		sprintf(data->error, args); \
		data->phase = PHASE_RESET; \
		log_err("Error: %s\n", data->error); } \
	}

/* Main function */
int stm32prog_init(struct stm32prog_data *data, uintptr_t addr, ulong size);
void stm32prog_clean(struct stm32prog_data *data);

#ifdef CONFIG_CMD_STM32PROG_SERIAL
int stm32prog_serial_init(struct stm32prog_data *data, int link_dev);
bool stm32prog_serial_loop(struct stm32prog_data *data);
#else
static inline int stm32prog_serial_init(struct stm32prog_data *data, int link_dev)
{
	return -ENOSYS;
}

static inline bool stm32prog_serial_loop(struct stm32prog_data *data)
{
	return false;
}
#endif

#ifdef CONFIG_CMD_STM32PROG_USB
bool stm32prog_usb_loop(struct stm32prog_data *data, int dev);
#else
static inline bool stm32prog_usb_loop(struct stm32prog_data *data, int dev)
{
	return false;
}
#endif

#endif
