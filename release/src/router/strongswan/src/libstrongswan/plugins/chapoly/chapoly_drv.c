/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include "chapoly_drv.h"
#include "chapoly_drv_portable.h"
#include "chapoly_drv_ssse3.h"

typedef chapoly_drv_t*(*chapoly_drv_create)();

/**
 * See header.
 */
chapoly_drv_t *chapoly_drv_probe()
{
	chapoly_drv_create drivers[] = {
		chapoly_drv_ssse3_create,
		chapoly_drv_portable_create,
	};
	chapoly_drv_t *driver;
	int i;

	for (i = 0; i < countof(drivers); i++)
	{
		driver = drivers[i]();
		if (driver)
		{
			return driver;
		}
	}
	return NULL;
}
