/*
 * Copyright (C) 2011 Sansar Choinyambuu
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

#include "pts_dh_group.h"

#include <utils/debug.h>

/**
 * Described in header.
 */
bool pts_dh_group_probe(pts_dh_group_t *dh_groups, bool mandatory_dh_groups)
{
	enumerator_t *enumerator;
	key_exchange_method_t dh_group;
	const char *plugin_name;

	*dh_groups = PTS_DH_GROUP_NONE;

	enumerator = lib->crypto->create_ke_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &dh_group, &plugin_name))
	{
		pts_dh_group_t mapped = PTS_DH_GROUP_NONE;

		switch (dh_group)
		{
			case MODP_1024_BIT:
				mapped = PTS_DH_GROUP_IKE2;
				break;
			case MODP_1536_BIT:
				mapped = PTS_DH_GROUP_IKE5;
				break;
			case MODP_2048_BIT:
				mapped = PTS_DH_GROUP_IKE14;
				break;
			case ECP_256_BIT:
				mapped = PTS_DH_GROUP_IKE19;
				break;
			case ECP_384_BIT:
				mapped = PTS_DH_GROUP_IKE20;
				break;
			default:
				break;
		}
		if (mapped != PTS_DH_GROUP_NONE)
		{
			*dh_groups |= mapped;
			DBG2(DBG_PTS, "  %s PTS DH group %N[%s] available",
				 mapped == PTS_DH_GROUP_IKE19 ? "mandatory" : "optional ",
				 key_exchange_method_names, dh_group, plugin_name);
		}
	}
	enumerator->destroy(enumerator);

	if (*dh_groups & PTS_DH_GROUP_IKE19)
	{
		/* mandatory PTS DH group is available */
		return TRUE;
	}
	if (*dh_groups == PTS_DH_GROUP_NONE)
	{
		DBG1(DBG_PTS, "no PTS DH group available");
		return FALSE;
	}
	if (mandatory_dh_groups)
	{
		DBG1(DBG_PTS, "  mandatory PTS DH group %N[%s] available",
			 key_exchange_method_names, ECP_256_BIT);
		return FALSE;
	}

	/* at least one optional PTS DH group is available */
	return TRUE;
}

/**
 * Described in header.
 */
bool pts_dh_group_update(char *dh_group, pts_dh_group_t *dh_groups)
{
	if (strcaseeq(dh_group, "ecp384"))
	{
		/* nothing to update, all groups are supported */
		return TRUE;
	}
	if (strcaseeq(dh_group, "ecp256"))
	{
		/* remove DH group 20 */
		*dh_groups &= ~PTS_DH_GROUP_IKE20;
		return TRUE;
	}
	if (strcaseeq(dh_group, "modp2048"))
	{
		/* remove DH groups 19 and 20 */
		*dh_groups &= ~(PTS_DH_GROUP_IKE20 | PTS_DH_GROUP_IKE19);
		return TRUE;
	}
	if (strcaseeq(dh_group, "modp1536"))
	{
		/* remove DH groups 14, 19 and 20 */
		*dh_groups &= ~(PTS_DH_GROUP_IKE20 | PTS_DH_GROUP_IKE19 |
						PTS_DH_GROUP_IKE14);
		return TRUE;
	}
	if (strcaseeq(dh_group, "modp1024"))
	{
		/* remove DH groups 5, 14, 19 and 20 */
		*dh_groups &= ~(PTS_DH_GROUP_IKE20 | PTS_DH_GROUP_IKE19 |
						PTS_DH_GROUP_IKE14 | PTS_DH_GROUP_IKE5);
		return TRUE;
	}
	DBG1(DBG_PTS, "unknown DH group '%s' configured", dh_group);
	return FALSE;
}

/**
 * Described in header.
 */
pts_dh_group_t pts_dh_group_select(pts_dh_group_t supported_dh_groups,
								   pts_dh_group_t offered_dh_groups)
{
	if ((supported_dh_groups & PTS_DH_GROUP_IKE20) &&
		(offered_dh_groups   & PTS_DH_GROUP_IKE20))
	{
		return PTS_DH_GROUP_IKE20;
	}
	if ((supported_dh_groups & PTS_DH_GROUP_IKE19) &&
		(offered_dh_groups   & PTS_DH_GROUP_IKE19))
	{
		return PTS_DH_GROUP_IKE19;
	}
	if ((supported_dh_groups & PTS_DH_GROUP_IKE14) &&
		(offered_dh_groups   & PTS_DH_GROUP_IKE14))
	{
		return PTS_DH_GROUP_IKE14;
	}
	if ((supported_dh_groups & PTS_DH_GROUP_IKE5) &&
		(offered_dh_groups   & PTS_DH_GROUP_IKE5))
	{
		return PTS_DH_GROUP_IKE5;
	}
	if ((supported_dh_groups & PTS_DH_GROUP_IKE2) &&
		(offered_dh_groups   & PTS_DH_GROUP_IKE2))
	{
		return PTS_DH_GROUP_IKE2;
	}
	return PTS_DH_GROUP_NONE;
}

/**
 * Described in header.
 */
key_exchange_method_t pts_dh_group_to_ike(pts_dh_group_t dh_group)
{
	switch (dh_group)
	{
		case PTS_DH_GROUP_IKE2:
			return MODP_1024_BIT;
		case PTS_DH_GROUP_IKE5:
			return MODP_1536_BIT;
		case PTS_DH_GROUP_IKE14:
			return MODP_2048_BIT;
		case PTS_DH_GROUP_IKE19:
			return ECP_256_BIT;
		case PTS_DH_GROUP_IKE20:
			return ECP_384_BIT;
		default:
			return KE_NONE;
	}
}
