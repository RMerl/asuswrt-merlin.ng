/*
 * Copyright (C) 2012 Reto Buerki
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

/**
 * @defgroup tkm-eescallbacks ees callbacks
 * @{ @ingroup tkm
 *
 * ESP SA Event Service (EES) callbacks.
 * The xfrm-proxy forwards acquire and expire events from the kernel to
 * charon-tkm using the EES interface. Upon reception of an event the
 * corresponding callback is executed.
 */

#ifndef EES_CALLBACKS_H_
#define EES_CALLBACKS_H_

/**
 * Process Acquire event for given security policy.
 */
void charon_esa_acquire(result_type *res, const sp_id_type sp_id);

/**
 * Process Expire event for given security policy.
 */
void charon_esa_expire(result_type *res, const sp_id_type sp_id,
					   const esp_spi_type spi_rem, const protocol_type protocol,
					   const expiry_flag_type hard);

#endif /** EES_CALLBACKS_H_ @}*/
