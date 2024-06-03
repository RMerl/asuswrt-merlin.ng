// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, Linaro Limited
 */

#if !defined _RNG_H_
#define _RNG_H_

struct udevice;

/**
 * dm_rng_read() - read a random number seed from the rng device
 *
 * The function blocks until the requested number of bytes is read.
 *
 * @dev:	random number generator device
 * @buffer:	input buffer to put the read random seed into
 * @size:	number of random bytes to read
 * Return:	0 if OK, -ve on error
 */
int dm_rng_read(struct udevice *dev, void *buffer, size_t size);

/**
 * struct dm_rng_ops - operations for the hwrng uclass
 *
 * This structures contains the function implemented by a hardware random
 * number generation device.
 */
struct dm_rng_ops {
	/**
	 * @read:	read a random bytes
	 *
	 * The function blocks until the requested number of bytes is read.
	 *
	 * @read.dev:		random number generator device
	 * @read.data:		input buffer to read the random seed into
	 * @read.max:		number of random bytes to read
	 * @read.Return:	0 if OK, -ve on error
	 */
	int (*read)(struct udevice *dev, void *data, size_t max);
};

#endif /* _RNG_H_ */
