/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#ifndef __TPM_COMMON_H
#define __TPM_COMMON_H

enum tpm_duration {
	TPM_SHORT = 0,
	TPM_MEDIUM = 1,
	TPM_LONG = 2,
	TPM_UNDEFINED,

	TPM_DURATION_COUNT,
};

/*
 * Here is a partial implementation of TPM commands.  Please consult TCG Main
 * Specification for definitions of TPM commands.
 */

#define TPM_HEADER_SIZE		10

/* Max buffer size supported by our tpm */
#define TPM_DEV_BUFSIZE		1260

#define TPM_PCR_MINIMUM_DIGEST_SIZE 20

/**
 * enum tpm_version - The version of the TPM stack to be used
 * @TPM_V1:		Use TPM v1.x stack
 * @TPM_V2:		Use TPM v2.x stack
 */
enum tpm_version {
	TPM_V1 = 0,
	TPM_V2,
};

/**
 * struct tpm_chip_priv - Information about a TPM, stored by the uclass
 *
 * These values must be set up by the device's probe() method before
 * communcation is attempted. If the device has an xfer() method, this is
 * not needed. There is no need to set up @buf.
 *
 * @version:		TPM stack to be used
 * @duration_ms:	Length of each duration type in milliseconds
 * @retry_time_ms:	Time to wait before retrying receive
 * @buf:		Buffer used during the exchanges with the chip
 * @pcr_count:		Number of PCR per bank
 * @pcr_select_min:	Minimum size in bytes of the pcrSelect array
 */
struct tpm_chip_priv {
	enum tpm_version version;

	uint duration_ms[TPM_DURATION_COUNT];
	uint retry_time_ms;
	u8 buf[TPM_DEV_BUFSIZE + sizeof(u8)];  /* Max buffer size + addr */

	/* TPM v2 specific data */
	uint pcr_count;
	uint pcr_select_min;
};

/**
 * struct tpm_ops - low-level TPM operations
 *
 * These are designed to avoid loops and delays in the driver itself. These
 * should be handled in the uclass.
 *
 * In gneral you should implement everything except xfer(). Where you need
 * complete control of the transfer, then xfer() can be provided and will
 * override the other methods.
 *
 * This interface is for low-level TPM access. It does not understand the
 * concept of localities or the various TPM messages. That interface is
 * defined in the functions later on in this file, but they all translate
 * to bytes which are sent and received.
 */
struct tpm_ops {
	/**
	 * open() - Request access to locality 0 for the caller
	 *
	 * After all commands have been completed the caller should call
	 * close().
	 *
	 * @dev:	Device to open
	 * @return 0 ok OK, -ve on error
	 */
	int (*open)(struct udevice *dev);

	/**
	 * close() - Close the current session
	 *
	 * Releasing the locked locality. Returns 0 on success, -ve 1 on
	 * failure (in case lock removal did not succeed).
	 *
	 * @dev:	Device to close
	 * @return 0 ok OK, -ve on error
	 */
	int (*close)(struct udevice *dev);

	/**
	 * get_desc() - Get a text description of the TPM
	 *
	 * @dev:	Device to check
	 * @buf:	Buffer to put the string
	 * @size:	Maximum size of buffer
	 * @return length of string, or -ENOSPC it no space
	 */
	int (*get_desc)(struct udevice *dev, char *buf, int size);

	/**
	 * send() - send data to the TPM
	 *
	 * @dev:	Device to talk to
	 * @sendbuf:	Buffer of the data to send
	 * @send_size:	Size of the data to send
	 *
	 * Returns 0 on success or -ve on failure.
	 */
	int (*send)(struct udevice *dev, const u8 *sendbuf, size_t send_size);

	/**
	 * recv() - receive a response from the TPM
	 *
	 * @dev:	Device to talk to
	 * @recvbuf:	Buffer to save the response to
	 * @max_size:	Maximum number of bytes to receive
	 *
	 * Returns number of bytes received on success, -EAGAIN if the TPM
	 * response is not ready, -EINTR if cancelled, or other -ve value on
	 * failure.
	 */
	int (*recv)(struct udevice *dev, u8 *recvbuf, size_t max_size);

	/**
	 * cleanup() - clean up after an operation in progress
	 *
	 * This is called if receiving times out. The TPM may need to abort
	 * the current transaction if it did not complete, and make itself
	 * ready for another.
	 *
	 * @dev:	Device to talk to
	 */
	int (*cleanup)(struct udevice *dev);

	/**
	 * xfer() - send data to the TPM and get response
	 *
	 * This method is optional. If it exists it is used in preference
	 * to send(), recv() and cleanup(). It should handle all aspects of
	 * TPM communication for a single transfer.
	 *
	 * @dev:	Device to talk to
	 * @sendbuf:	Buffer of the data to send
	 * @send_size:	Size of the data to send
	 * @recvbuf:	Buffer to save the response to
	 * @recv_size:	Pointer to the size of the response buffer
	 *
	 * Returns 0 on success (and places the number of response bytes at
	 * recv_size) or -ve on failure.
	 */
	int (*xfer)(struct udevice *dev, const u8 *sendbuf, size_t send_size,
		    u8 *recvbuf, size_t *recv_size);
};

#define tpm_get_ops(dev)        ((struct tpm_ops *)device_get_ops(dev))

#define MAKE_TPM_CMD_ENTRY(cmd) \
	U_BOOT_CMD_MKENT(cmd, 0, 1, do_tpm_ ## cmd, "", "")

#define TPM_COMMAND_NO_ARG(cmd)				\
int do_##cmd(cmd_tbl_t *cmdtp, int flag,		\
	     int argc, char * const argv[])		\
{							\
	struct udevice *dev;				\
	int rc;						\
							\
	rc = get_tpm(&dev);				\
	if (rc)						\
		return rc;				\
	if (argc != 1)					\
		return CMD_RET_USAGE;			\
	return report_return_code(cmd(dev));		\
}

/**
 * tpm_open() - Request access to locality 0 for the caller
 *
 * After all commands have been completed the caller is supposed to
 * call tpm_close().
 *
 * @dev - TPM device
 * Returns 0 on success, -ve on failure.
 */
int tpm_open(struct udevice *dev);

/**
 * tpm_close() - Close the current session
 *
 * Releasing the locked locality. Returns 0 on success, -ve 1 on
 * failure (in case lock removal did not succeed).
 *
 * @dev - TPM device
 * Returns 0 on success, -ve on failure.
 */
int tpm_close(struct udevice *dev);

/**
 * tpm_clear_and_reenable() - Force clear the TPM and reenable it
 *
 * @dev: TPM device
 * @return 0 on success, -ve on failure
 */
u32 tpm_clear_and_reenable(struct udevice *dev);

/**
 * tpm_get_desc() - Get a text description of the TPM
 *
 * @dev:	Device to check
 * @buf:	Buffer to put the string
 * @size:	Maximum size of buffer
 * @return length of string, or -ENOSPC it no space
 */
int tpm_get_desc(struct udevice *dev, char *buf, int size);

/**
 * tpm_xfer() - send data to the TPM and get response
 *
 * This first uses the device's send() method to send the bytes. Then it calls
 * recv() to get the reply. If recv() returns -EAGAIN then it will delay a
 * short time and then call recv() again.
 *
 * Regardless of whether recv() completes successfully, it will then call
 * cleanup() to finish the transaction.
 *
 * Note that the outgoing data is inspected to determine command type
 * (ordinal) and a timeout is used for that command type.
 *
 * @dev - TPM device
 * @sendbuf - buffer of the data to send
 * @send_size size of the data to send
 * @recvbuf - memory to save the response to
 * @recv_len - pointer to the size of the response buffer
 *
 * Returns 0 on success (and places the number of response bytes at
 * recv_len) or -ve on failure.
 */
int tpm_xfer(struct udevice *dev, const u8 *sendbuf, size_t send_size,
	     u8 *recvbuf, size_t *recv_size);

/**
 * Initialize TPM device.  It must be called before any TPM commands.
 *
 * @dev - TPM device
 * @return 0 on success, non-0 on error.
 */
int tpm_init(struct udevice *dev);

/**
 * Retrieve the array containing all the v1 (resp. v2) commands.
 *
 * @return a cmd_tbl_t array.
 */
#if defined(CONFIG_TPM_V1)
cmd_tbl_t *get_tpm1_commands(unsigned int *size);
#else
static inline cmd_tbl_t *get_tpm1_commands(unsigned int *size)
{
	return NULL;
}
#endif
#if defined(CONFIG_TPM_V2)
cmd_tbl_t *get_tpm2_commands(unsigned int *size);
#else
static inline cmd_tbl_t *get_tpm2_commands(unsigned int *size)
{
	return NULL;
}
#endif

/**
 * tpm_get_version() - Find the version of a TPM
 *
 * This checks the uclass data for a TPM device and returns the version number
 * it supports.
 *
 * @dev: TPM device
 * @return version number (TPM_V1 or TPMV2)
 */
enum tpm_version tpm_get_version(struct udevice *dev);

#endif /* __TPM_COMMON_H */
