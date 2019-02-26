/*
 * Copyright (C) 2012-2014 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * HSR Hochschule fuer Technik Rapperswil
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

#include <daemon.h>
#include <utils/debug.h>
#include <tkm/constants.h>
#include <tkm/types.h>

#include "tkm.h"
#include "ees_callbacks.h"

void charon_esa_acquire(result_type *res, const sp_id_type sp_id)
{
	DBG1(DBG_KNL, "ees: acquire received for reqid %u", sp_id);
	charon->kernel->acquire(charon->kernel, sp_id, NULL, NULL);
	*res = TKM_OK;
}

void charon_esa_expire(result_type *res, const sp_id_type sp_id,
					   const esp_spi_type spi_rem, const protocol_type protocol,
					   const expiry_flag_type hard)
{
	host_t *dst;

	dst = tkm->sad->get_dst_host(tkm->sad, sp_id, spi_rem, protocol);
	*res = TKM_OK;
	if (dst == NULL)
	{
		DBG3(DBG_KNL, "ees: destination host not found for reqid %u, spi %x, "
			 "proto %u", sp_id, ntohl(spi_rem), protocol);
		return;
	}

	DBG1(DBG_KNL, "ees: expire received for reqid %u, spi %x, dst %H", sp_id,
		 ntohl(spi_rem), dst);
	charon->kernel->expire(charon->kernel, protocol, spi_rem, dst, hard != 0);
	dst->destroy(dst);
}
