/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2011 Infineon Technologies
 *
 * Authors:
 * Peter Huewe <huewe.external@infineon.com>
 *
 * Version: 2.1.1
 *
 * Description:
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * It is based on the Linux kernel driver tpm.c from Leendert van
 * Dorn, Dave Safford, Reiner Sailer, and Kyleen Hall.
 */

#ifndef _TPM_TIS_I2C_H
#define _TPM_TIS_I2C_H

#include <linux/compiler.h>
#include <linux/types.h>

/**
 * struct tpm_tis_phy_ops - low-level TPM bus operations
 */
struct tpm_tis_phy_ops {
	/* read_bytes() - Read a number of bytes from the device
	 *
	 * @udev:   TPM device
	 * @addr:   offset from device base
	 * @len:    len to read
	 * @result: data read
	 *
	 * @return: 0 on success, negative on failure
	 */
	int (*read_bytes)(struct udevice *udev, u32 addr, u16 len,
			  u8 *result);
	/* write_bytes() - Read a number of bytes from the device
	 *
	 * @udev:   TPM device
	 * @addr:   offset from device base
	 * @len:    len to read
	 * @value:  data to write
	 *
	 * @return: 0 on success, negative on failure
	 */
	int (*write_bytes)(struct udevice *udev, u32 addr, u16 len,
			   const u8 *value);
	/* read32() - Read a 32bit value of the device
	 *
	 * @udev:   TPM device
	 * @addr:   offset from device base
	 * @result: data read
	 *
	 * @return: 0 on success, negative on failure
	 */
	int (*read32)(struct udevice *udev, u32 addr, u32 *result);
	/* write32() - write a 32bit value to the device
	 *
	 * @udev: TPM device
	 * @addr: offset from device base
	 * @src:  data to write
	 *
	 * @return: 0 on success, negative on failure
	 */
	int (*write32)(struct udevice *udev, u32 addr, u32 src);
};

enum tis_int_flags {
	TPM_GLOBAL_INT_ENABLE = 0x80000000,
	TPM_INTF_BURST_COUNT_STATIC = 0x100,
	TPM_INTF_CMD_READY_INT = 0x080,
	TPM_INTF_INT_EDGE_FALLING = 0x040,
	TPM_INTF_INT_EDGE_RISING = 0x020,
	TPM_INTF_INT_LEVEL_LOW = 0x010,
	TPM_INTF_INT_LEVEL_HIGH = 0x008,
	TPM_INTF_LOCALITY_CHANGE_INT = 0x004,
	TPM_INTF_STS_VALID_INT = 0x002,
	TPM_INTF_DATA_AVAIL_INT = 0x001,
};

#define TPM_ACCESS(l)                   (0x0000 | ((l) << 12))
#define TPM_INT_ENABLE(l)               (0x0008 | ((l) << 12))
#define TPM_STS(l)                      (0x0018 | ((l) << 12))
#define TPM_DATA_FIFO(l)                (0x0024 | ((l) << 12))
#define TPM_DID_VID(l)                  (0x0f00 | ((l) << 12))
#define TPM_RID(l)                      (0x0f04 | ((l) << 12))
#define TPM_INTF_CAPS(l)                (0x0014 | ((l) << 12))

enum tpm_timeout {
	TPM_TIMEOUT_MS			= 5,
	TIS_SHORT_TIMEOUT_MS		= 750,
	TIS_LONG_TIMEOUT_MS		= 2000,
	SLEEP_DURATION_US		= 60,
	SLEEP_DURATION_LONG_US		= 210,
};

/* Size of external transmit buffer (used in tpm_transmit)*/
#define TPM_BUFSIZE 4096

/* Index of Count field in TPM response buffer */
#define TPM_RSP_SIZE_BYTE	2
#define TPM_RSP_RC_BYTE		6

struct tpm_chip {
	int is_open;
	int locality;
	u32 vend_dev;
	u8 rid;
	unsigned long timeout_a, timeout_b, timeout_c, timeout_d;  /* msec */
	ulong chip_type;
	struct tpm_tis_phy_ops *phy_ops;
};

struct tpm_input_header {
	__be16 tag;
	__be32 length;
	__be32 ordinal;
} __packed;

struct tpm_output_header {
	__be16 tag;
	__be32 length;
	__be32 return_code;
} __packed;

struct timeout_t {
	__be32 a;
	__be32 b;
	__be32 c;
	__be32 d;
} __packed;

struct duration_t {
	__be32 tpm_short;
	__be32 tpm_medium;
	__be32 tpm_long;
} __packed;

union cap_t {
	struct timeout_t timeout;
	struct duration_t duration;
};

struct tpm_getcap_params_in {
	__be32 cap;
	__be32 subcap_size;
	__be32 subcap;
} __packed;

struct tpm_getcap_params_out {
	__be32 cap_size;
	union cap_t cap;
} __packed;

union tpm_cmd_header {
	struct tpm_input_header in;
	struct tpm_output_header out;
};

union tpm_cmd_params {
	struct tpm_getcap_params_out getcap_out;
	struct tpm_getcap_params_in getcap_in;
};

struct tpm_cmd_t {
	union tpm_cmd_header header;
	union tpm_cmd_params params;
} __packed;

/* Max number of iterations after i2c NAK */
#define MAX_COUNT		3

#ifndef __TPM_V2_H
/*
 * Max number of iterations after i2c NAK for 'long' commands
 *
 * We need this especially for sending TPM_READY, since the cleanup after the
 * transtion to the ready state may take some time, but it is unpredictable
 * how long it will take.
 */
#define MAX_COUNT_LONG		50

enum tis_access {
	TPM_ACCESS_VALID		= 0x80,
	TPM_ACCESS_ACTIVE_LOCALITY	= 0x20,
	TPM_ACCESS_REQUEST_PENDING	= 0x04,
	TPM_ACCESS_REQUEST_USE		= 0x02,
};

enum tis_status {
	TPM_STS_VALID			= 0x80,
	TPM_STS_COMMAND_READY		= 0x40,
	TPM_STS_GO			= 0x20,
	TPM_STS_DATA_AVAIL		= 0x10,
	TPM_STS_DATA_EXPECT		= 0x08,
};
#endif

/**
 * tpm_tis_open - Open the device and request locality 0
 *
 * @dev:  TPM device
 *
 * @return: 0 on success, negative on failure
 */
int tpm_tis_open(struct udevice *udev);
/**
 * tpm_tis_close - Close the device and release locality
 *
 * @dev:  TPM device
 *
 * @return: 0 on success, negative on failure
 */
int tpm_tis_close(struct udevice *udev);
/** tpm_tis_cleanup - Get the device in ready state and release locality
 *
 * @dev:  TPM device
 *
 * @return: always 0
 */
int tpm_tis_cleanup(struct udevice *udev);
/**
 * tpm_tis_send - send data to the device
 *
 * @dev:  TPM device
 * @buf:  buffer to send
 * @len:  size of the buffer
 *
 * @return: number of bytes sent or negative on failure
 */
int tpm_tis_send(struct udevice *udev, const u8 *buf, size_t len);
/**
 * tpm_tis_recv_data - Receive data from a device. Wrapper for tpm_tis_recv
 *
 * @dev:  TPM device
 * @buf:  buffer to copy data
 * @size: buffer size
 *
 * @return: bytes read or negative on failure
 */
int tpm_tis_recv(struct udevice *udev, u8 *buf, size_t count);
/**
 * tpm_tis_get_desc - Get the TPM description
 *
 * @dev:  TPM device
 * @buf:  buffer to fill data
 * @size: buffer size
 *
 * @return: Number of characters written (or would have been written) in buffer
 */
int tpm_tis_get_desc(struct udevice *udev, char *buf, int size);
/**
 * tpm_tis_init - inititalize the device
 *
 * @dev:  TPM device
 *
 * @return: 0 on success, negative on failure
 */
int tpm_tis_init(struct udevice *udev);
/**
 * tpm_tis_ops_register - register the PHY ops for the device
 *
 * @dev: TPM device
 * @ops: tpm_tis_phy_ops ops for the device
 */
void tpm_tis_ops_register(struct udevice *udev, struct tpm_tis_phy_ops *ops);
#endif
