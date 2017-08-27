/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup ipsec_sa ipsec_sa
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_SA_H_
#define IPSEC_SA_H_

#include "esp_context.h"

#include <library.h>
#include <networking/host.h>
#include <selectors/traffic_selector.h>
#include <ipsec/ipsec_types.h>

typedef struct ipsec_sa_t ipsec_sa_t;

/**
 * IPsec Security Association (SA)
 */
struct ipsec_sa_t {

	/**
	 * Get the source address for this SA
	 *
	 * @return			source address of this SA
	 */
	host_t *(*get_source)(ipsec_sa_t *this);

	/**
	 * Get the destination address for this SA
	 *
	 * @return			destination address of this SA
	 */
	host_t *(*get_destination)(ipsec_sa_t *this);

	/**
	 * Set the source address for this SA
	 *
	 * @param addr		source address of this SA (gets cloned)
	 */
	void (*set_source)(ipsec_sa_t *this, host_t *addr);

	/**
	 * Set the destination address for this SA
	 *
	 * @param addr		destination address of this SA (gets cloned)
	 */
	void (*set_destination)(ipsec_sa_t *this, host_t *addr);

	/**
	 * Get the SPI for this SA
	 *
	 * @return			SPI of this SA
	 */
	u_int32_t (*get_spi)(ipsec_sa_t *this);

	/**
	 * Get the reqid of this SA
	 *
	 * @return			reqid of this SA
	 */
	u_int32_t (*get_reqid)(ipsec_sa_t *this);

	/**
	 * Get the protocol (e.g. IPPROTO_ESP) of this SA
	 *
	 * @return			protocol of this SA
	 */
	u_int8_t (*get_protocol)(ipsec_sa_t *this);

	/**
	 * Returns whether this SA is inbound or outbound
	 *
	 * @return			TRUE if inbound, FALSE if outbound
	 */
	bool (*is_inbound)(ipsec_sa_t *this);

	/**
	 * Get the lifetime information for this SA
	 * Note that this information is always relative to the time when the
	 * SA was installed (i.e. it is not adjusted over time)
	 *
	 * @return			lifetime of this SA
	 */
	lifetime_cfg_t *(*get_lifetime)(ipsec_sa_t *this);

	/**
	 * Get the ESP context for this SA
	 *
	 * @return			ESP context of this SA
	 */
	esp_context_t *(*get_esp_context)(ipsec_sa_t *this);

	/**
	 * Get usage statistics for this SA.
	 *
	 * @param bytes		receives number of processed bytes, or NULL
	 * @param packets	receives number of processed packets, or NULL
	 * @param time		receives last use time of this SA, or NULL
	 */
	void (*get_usestats)(ipsec_sa_t *this, u_int64_t *bytes, u_int64_t *packets,
						 time_t *time);

	/**
	 * Record en/decryption of a packet to update usage statistics.
	 *
	 * @param bytes		length of packet processed
	 */
	void (*update_usestats)(ipsec_sa_t *this, u_int32_t bytes);

	/**
	 * Expire this SA, soft or hard.
	 *
	 * A soft expire triggers a rekey, a hard expire blocks the SA and
	 * triggers a delete for the SA.
	 *
	 * @param hard		TRUE for hard, FALSE for soft
	 */
	void (*expire)(ipsec_sa_t *this, bool hard);

	/**
	 * Check if this SA matches all given parameters
	 *
	 * Only matches if the SA has not yet expired.
	 *
	 * @param spi		SPI
	 * @param dst		destination address
	 * @return			TRUE if this SA matches all parameters, FALSE otherwise
	 */
	bool (*match_by_spi_dst)(ipsec_sa_t *this, u_int32_t spi, host_t *dst);

	/**
	 * Check if this SA matches all given parameters
	 *
	 * @param spi		SPI
	 * @param src		source address
	 * @param dst		destination address
	 * @return			TRUE if this SA matches all parameters, FALSE otherwise
	 */
	bool (*match_by_spi_src_dst)(ipsec_sa_t *this, u_int32_t spi, host_t *src,
								 host_t *dst);

	/**
	 * Check if this SA matches all given parameters
	 *
	 * Only matches if the SA has not yet expired.
	 *
	 * @param reqid		reqid
	 * @param inbound	TRUE for inbound SA, FALSE for outbound
	 * @return			TRUE if this SA matches all parameters, FALSE otherwise
	 */
	bool (*match_by_reqid)(ipsec_sa_t *this, u_int32_t reqid, bool inbound);

	/**
	 * Destroy an ipsec_sa_t
	 */
	void (*destroy)(ipsec_sa_t *this);

};

/**
 * Create an ipsec_sa_t instance
 *
 * @param spi			SPI for this SA
 * @param src			source address for this SA (gets cloned)
 * @param dst			destination address for this SA (gets cloned)
 * @param protocol		protocol for this SA (only ESP is supported)
 * @param reqid			reqid for this SA
 * @param mark			mark for this SA (ignored)
 * @param tfc			Traffic Flow Confidentiality (currently not supported)
 * @param lifetime		lifetime for this SA
 * @param enc_alg		encryption algorithm for this SA
 * @param enc_key		encryption key for this SA
 * @param int_alg		integrity protection algorithm
 * @param int_key		integrity protection key
 * @param mode			mode for this SA (only tunnel mode is supported)
 * @param ipcomp		IPcomp transform (not supported, use IPCOMP_NONE)
 * @param cpi			CPI for IPcomp (ignored)
 * @param encap			enable UDP encapsulation (must be TRUE)
 * @param esn			Extended Sequence Numbers (currently not supported)
 * @param inbound		TRUE if this is an inbound SA, FALSE otherwise
 * @param src_ts		source traffic selector
 * @param dst_ts		destination traffic selector
 * @return				the IPsec SA, or NULL if the creation failed
 */
ipsec_sa_t *ipsec_sa_create(u_int32_t spi, host_t *src, host_t *dst,
							u_int8_t protocol, u_int32_t reqid, mark_t mark,
							u_int32_t tfc, lifetime_cfg_t *lifetime,
							u_int16_t enc_alg, chunk_t enc_key,
							u_int16_t int_alg, chunk_t int_key,
							ipsec_mode_t mode, u_int16_t ipcomp, u_int16_t cpi,
							bool encap, bool esn, bool inbound,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts);

#endif /** IPSEC_SA_H_ @}*/
