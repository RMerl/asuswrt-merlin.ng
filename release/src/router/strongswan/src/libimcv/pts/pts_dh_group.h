/*
 * Copyright (C) 2011 Sansar Choinyambuu
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
 * @defgroup pts_dh_group pts_dh_group
 * @{ @ingroup pts
 */

#ifndef PTS_DH_GROUP_H_
#define PTS_DH_GROUP_H_

#include <library.h>
#include <crypto/diffie_hellman.h>

typedef enum pts_dh_group_t pts_dh_group_t;

/**
 * PTS Diffie Hellman Group Values
 */
enum pts_dh_group_t {
	/** No DH Group */
	PTS_DH_GROUP_NONE  =					0,
	/** IKE Group 2 */
	PTS_DH_GROUP_IKE2  =				 (1<<15),
	/** IKE Group 5 */
	PTS_DH_GROUP_IKE5  =				 (1<<14),
	/** IKE Group 14 */
	PTS_DH_GROUP_IKE14 =				 (1<<13),
	/** IKE Group 19 */
	PTS_DH_GROUP_IKE19 =				 (1<<12),
	/** IKE Group 20 */
	PTS_DH_GROUP_IKE20 =				 (1<<11),
};

/**
 * Diffie-Hellman Group Values
 * see section 3.8.6 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *                       1
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |1|2|3|4|5|R|R|R|R|R|R|R|R|R|R|R|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * Probe available PTS Diffie-Hellman groups
 *
 * @param dh_groups				returns set of available DH groups
 * @param mandatory_dh_groups	if TRUE enforce mandatory PTS DH groups
 * @return						TRUE if mandatory DH groups are available
 *								or at least one optional DH group if 
 *								mandatory_dh_groups is set to FALSE.
 */
bool pts_dh_group_probe(pts_dh_group_t *dh_groups, bool mandatory_dh_groups);

/**
 * Update supported Diffie-Hellman groups according to configuration
 *
 * modp1024: PTS_DH_GROUP_IKE2
 * modp1536: PTS_DH_GROUP_IKE2  | PTS_DH_GROUP_IKE5
 * modp2048: PTS_DH_GROUP_IKE2  | PTS_DH_GROUP_IKE5  | PTS_DH_GROUP_IKE14
 * ecp256:   PTS_DH_GROUP_IKE2  | PTS_DH_GROUP_IKE5  | PTS_DH_GROUP_IKE14 |
 *           PTS_DH_GROUP_IKE19
 * ecp384:   PTS_DH_GROUP_IKE2  | PTS_DH_GROUP_IKE5  | PTS_DH_GROUP_IKE14 |
 *           PTS_DH_GROUP_IKE19 | PTS_DH_GROUP_IKE20
 *
 * The PTS-IMC is expected to select the strongest supported group
 *
 * @param dh_group			configured DH group
 * @param dh_groups			returns set of available DH groups
 */
bool pts_dh_group_update(char *dh_group, pts_dh_group_t *dh_groups);

/**
 * Select the strongest supported Diffie-Hellman group
 * among a set of offered DH groups
 *
 * @param supported_groups	set of supported DH groups
 * @param offered_groups	set of offered DH groups
 * @return					selected DH group
 */
pts_dh_group_t pts_dh_group_select(pts_dh_group_t supported_groups,
								   pts_dh_group_t offered_groups);

/**
 * Convert pts_dh_group_t to diffie_hellman_group_t
 *
 * @param dh_group			PTS DH group type
 * @return					IKE DH group type
 */
diffie_hellman_group_t pts_dh_group_to_ike(pts_dh_group_t dh_group);

#endif /** PTS_DH_GROUP_H_ @}*/
