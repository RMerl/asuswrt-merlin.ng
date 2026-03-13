/*
 * Copyright (C) 2025 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "kernel_listener.h"

/*
 * Described in header
 */
kernel_acquire_data_t *kernel_acquire_data_clone(kernel_acquire_data_t *data)
{
	kernel_acquire_data_t *clone;

	INIT(clone);

	*clone = *data;

	if (clone->src)
	{
		clone->src = clone->src->clone(clone->src);
	}
	if (clone->dst)
	{
		clone->dst = clone->dst->clone(clone->dst);
	}
	if (clone->label)
	{
		clone->label = clone->label->clone(clone->label);
	}
	return clone;
}

/*
 * Described in header
 */
void kernel_acquire_data_destroy(kernel_acquire_data_t *data)
{
	if (data)
	{
		DESTROY_IF(data->src);
		DESTROY_IF(data->dst);
		DESTROY_IF(data->label);
		free(data);
	}
}
