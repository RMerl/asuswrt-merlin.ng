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
 * @defgroup ipsec_sa_mgr ipsec_sa_mgr
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_SA_MGR_H_
#define IPSEC_SA_MGR_H_

#include "ipsec_sa.h"

#include <library.h>
#include <ipsec/ipsec_types.h>
#include <selectors/traffic_selector.h>
#include <networking/host.h>

typedef struct ipsec_sa_mgr_t ipsec_sa_mgr_t;

/**
 * IPsec SA manager
 *
 * The first methods are modeled after those in kernel_ipsec_t.
 */
struct ipsec_sa_mgr_t {

	/**
	 * Allocate an SPI for an inbound IPsec SA
	 *
	 * @param src			source address of the SA
	 * @param dst			destination address of the SA
	 * @param protocol		protocol of the SA (only ESP supported)
	 * @param reqid			reqid for the SA
	 * @param spi			the allocated SPI
	 * @return				SUCCESS of operation successful
	 */
	status_t (*get_spi)(ipsec_sa_mgr_t *this, host_t *src, host_t *dst,
						u_int8_t protocol, u_int32_t reqid, u_int32_t *spi);

	/**
	 * Add a new SA
	 *
	 * @param src			source address for this SA (gets cloned)
	 * @param dst			destination address for this SA (gets cloned)
	 * @param spi			SPI for this SA
	 * @param protocol		protocol for this SA (only ESP is supported)
	 * @param reqid			reqid for this SA
	 * @param mark			mark for this SA (ignored)
	 * @param tfc			Traffic Flow Confidentiality (not yet supported)
	 * @param lifetime		lifetime for this SA
	 * @param enc_alg		encryption algorithm for this SA
	 * @param enc_key		encryption key for this SA
	 * @param int_alg		integrity protection algorithm
	 * @param int_key		integrity protection key
	 * @param mode			mode for this SA (only tunnel mode is supported)
	 * @param ipcomp		IPcomp transform (not supported, use IPCOMP_NONE)
	 * @param cpi			CPI for IPcomp (ignored)
	 * @param initiator		TRUE if initiator of the exchange creating this SA
	 * @param encap			enable UDP encapsulation (must be TRUE)
	 * @param esn			Extended Sequence Numbers (currently not supported)
	 * @param inbound		TRUE if this is an inbound SA, FALSE otherwise
	 * @param src_ts		source traffic selector
	 * @param dst_ts		destination traffic selector
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_sa)(ipsec_sa_mgr_t *this, host_t *src, host_t *dst,
					   u_int32_t spi, u_int8_t protocol, u_int32_t reqid,
					   mark_t mark, u_int32_t tfc,	lifetime_cfg_t *lifetime,
					   u_int16_t enc_alg, chunk_t enc_key, u_int16_t int_alg,
					   chunk_t int_key, ipsec_mode_t mode, u_int16_t ipcomp,
					   u_int16_t cpi, bool initiator, bool encap, bool esn,
					   bool inbound, traffic_selector_t *src_ts,
					   traffic_selector_t *dst_ts);

	/**
	 * Update the hosts on an installed SA.
	 *
	 * @param spi			SPI of the SA
	 * @param protocol		protocol for this SA (ESP/AH)
	 * @param cpi			CPI for IPComp, 0 if no IPComp is used
	 * @param src			current source address
	 * @param dst			current destination address
	 * @param new_src		new source address
	 * @param new_dst		new destination address
	 * @param encap			current use of UDP encapsulation
	 * @param new_encap		new use of UDP encapsulation
	 * @param mark			optional mark for this SA
	 * @return				SUCCESS if operation completed
	 */
	status_t (*update_sa)(ipsec_sa_mgr_t *this,
						  u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						  host_t *src, host_t *dst,
						  host_t *new_src, host_t *new_dst,
						  bool encap, bool new_encap, mark_t mark);

	/**
	 * Query the number of bytes processed by an SA from the SAD.
	 *
	 * @param src			source address for this SA
	 * @param dst			destination address for this SA
	 * @param spi			SPI allocated by us or remote peer
	 * @param protocol		protocol for this SA (ESP/AH)
	 * @param mark			optional mark for this SA
	 * @param[out] bytes	the number of bytes processed by SA
	 * @param[out] packets	number of packets processed by SA
	 * @param[out] time		last (monotonic) time of SA use
	 * @return				SUCCESS if operation completed
	 */
	status_t (*query_sa)(ipsec_sa_mgr_t *this, host_t *src, host_t *dst,
						 u_int32_t spi, u_int8_t protocol, mark_t mark,
						 u_int64_t *bytes, u_int64_t *packets, time_t *time);

	/**
	 * Delete a previously added SA
	 *
	 * @param spi			SPI of the SA
	 * @param src			source address of the SA
	 * @param dst			destination address of the SA
	 * @param protocol		protocol of the SA
	 * @param cpi			CPI for IPcomp
	 * @param mark			optional mark
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_sa)(ipsec_sa_mgr_t *this, host_t *src, host_t *dst,
					   u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
					   mark_t mark);

	/**
	 * Flush all SAs
	 *
	 * @return				SUCCESS if operation completed
	 */
	status_t (*flush_sas)(ipsec_sa_mgr_t *this);

	/**
	 * Checkout an installed IPsec SA by SPI and destination address
	 * Can be used to find the correct SA for an inbound packet.
	 *
	 * The matching SA is locked until it is checked in using checkin().
	 * If the matching SA is already checked out, this call blocks until the
	 * SA is checked in.
	 *
	 * Since other threads may be waiting for the checked out SA, it should be
	 * checked in as soon as possible after use.
	 *
	 * @param spi			SPI (e.g. of an inbound packet)
	 * @param dst			destination address (e.g. of an inbound packet)
	 * @return				the matching IPsec SA, or NULL if none is found
	 */
	ipsec_sa_t *(*checkout_by_spi)(ipsec_sa_mgr_t *this, u_int32_t spi,
								   host_t *dst);

	/**
	 * Checkout an installed IPsec SA by its reqid and inbound/outbound flag.
	 * Can be used to find the correct SA for an outbound packet.
	 *
	 * The matching SA is locked until it is checked in using checkin().
	 * If the matching SA is already checked out, this call blocks until the
	 * SA is checked in.
	 *
	 * Since other threads may be waiting for a checked out SA, it should be
	 * checked in as soon as possible after use.
	 *
	 * @param reqid			reqid of the SA
	 * @param inbound		TRUE for an inbound SA, FALSE for an outbound SA
	 * @return				the matching IPsec SA, or NULL if none is found
	 */
	ipsec_sa_t *(*checkout_by_reqid)(ipsec_sa_mgr_t *this, u_int32_t reqid,
									 bool inbound);

	/**
	 * Checkin an SA after use.
	 *
	 * @param sa			checked out SA
	 */
	void (*checkin)(ipsec_sa_mgr_t *this, ipsec_sa_t *sa);

	/**
	 * Destroy an ipsec_sa_mgr_t
	 */
	void (*destroy)(ipsec_sa_mgr_t *this);

};

/**
 * Create an ipsec_sa_mgr instance
 *
 * @return					IPsec SA manager instance
 */
ipsec_sa_mgr_t *ipsec_sa_mgr_create();

#endif /** IPSEC_SA_MGR_H_ @}*/
