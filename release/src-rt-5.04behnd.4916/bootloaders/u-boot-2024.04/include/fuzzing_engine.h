/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#ifndef __FUZZING_ENGINE_H
#define __FUZZING_ENGINE_H

struct udevice;

/**
 * dm_fuzzing_engine_get_input() - get an input from the fuzzing engine device
 *
 * The function will return a pointer to the input data and the size of the
 * data pointed to. The pointer will remain valid until the next invocation of
 * this function.
 *
 * @dev:	fuzzing engine device
 * @data:	output pointer to input data
 * @size	output size of input data
 * Return:	0 if OK, -ve on error
 */
int dm_fuzzing_engine_get_input(struct udevice *dev,
				const uint8_t **data,
				size_t *size);

/**
 * struct dm_fuzzing_engine_ops - operations for the fuzzing engine uclass
 *
 * This contains the functions implemented by a fuzzing engine device.
 */
struct dm_fuzzing_engine_ops {
	/**
	 * @get_input() - get an input
	 *
	 * The function will return a pointer to the input data and the size of
	 * the data pointed to. The pointer will remain valid until the next
	 * invocation of this function.
	 *
	 * @get_input.dev:	fuzzing engine device
	 * @get_input.data:	output pointer to input data
	 * @get_input.size	output size of input data
	 * @get_input.Return:	0 if OK, -ve on error
	 */
	int (*get_input)(struct udevice *dev,
			 const uint8_t **data,
			 size_t *size);
};

#endif /* __FUZZING_ENGINE_H */
