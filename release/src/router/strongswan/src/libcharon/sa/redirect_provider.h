/*
 * Copyright (C) 2015 Tobias Brunner
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
 * @defgroup redirect_provider redirect_provider
 * @{ @ingroup sa
 */

#ifndef REDIRECT_PROVIDER_H_
#define REDIRECT_PROVIDER_H_

typedef struct redirect_provider_t redirect_provider_t;

#include <library.h>
#include <sa/ike_sa.h>

/**
 * Interface that allows implementations to decide whether a client is
 * redirected during IKE_SA_INIT or IKE_AUTH using RFC 5685.
 */
struct redirect_provider_t {

	/**
	 * Decide whether a client is redirect directly upon receipt of the
	 * IKE_SA_INIT message.
	 *
	 * @param ike_sa		IKE_SA for which this is called
	 * @param gateway[out]	new IKE gateway (IP or FQDN)
	 * @return				TRUE if client should be redirected, FALSE otherwise
	 */
	bool (*redirect_on_init)(redirect_provider_t *this, ike_sa_t *ike_sa,
							 identification_t **gateway);

	/**
	 * Decide whether a client is redirect after the IKE_AUTH has been
	 * handled.  This is called after the client is authenticated and when the
	 * server authenticates itself.
	 *
	 * @param ike_sa		IKE_SA for which this is called
	 * @param gateway[out]	new IKE gateway (IP or FQDN)
	 * @return				TRUE if client should be redirected, FALSE otherwise
	 */
	bool (*redirect_on_auth)(redirect_provider_t *this, ike_sa_t *ike_sa,
							 identification_t **gateway);
};

#endif /** REDIRECT_PROVIDER_H_ @}*/
