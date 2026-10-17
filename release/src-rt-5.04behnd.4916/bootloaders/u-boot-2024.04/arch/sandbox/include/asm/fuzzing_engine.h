/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#ifndef __ASM_FUZZING_ENGINE_H
#define __ASM_FUZZING_ENGINE_H

/** Function to get fuzzing engine input data. */
/**
 * sandbox_fuzzing_engine_get_input() - get an input from the sandbox fuzzing
 * 					engine
 *
 * The function will return a pointer to the input data and the size of the
 * data pointed to. The pointer will remain valid until the next invocation of
 * this function.
 *
 * @data:	output pointer to input data
 * @size	output size of input data
 * Return:	0 if OK, -ve on error
 */
int sandbox_fuzzing_engine_get_input(const uint8_t **data, size_t *size);

#endif /* __ASM_FUZZING_ENGINE_H */
