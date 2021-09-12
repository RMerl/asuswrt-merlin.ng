/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2015 Vincent Bernat <vincent@bernat.im>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>

#include "../lldpctl.h"
#include "../../log.h"
#include "../atom.h"
#include "../helpers.h"
#include "../fixedpoint.h"

#ifdef ENABLE_LLDPMED

static lldpctl_map_t port_med_location_map[] = {
	{ LLDP_MED_LOCFORMAT_COORD, "Coordinates" },
	{ LLDP_MED_LOCFORMAT_CIVIC, "Civic address" },
	{ LLDP_MED_LOCFORMAT_ELIN, "ELIN" },
	{ 0, NULL },
};

static lldpctl_map_t port_med_pow_devicetype_map[] = {
	{ LLDP_MED_POW_TYPE_PSE, "PSE" },
	{ LLDP_MED_POW_TYPE_PD,  "PD" },
	{ 0, NULL },
};

static lldpctl_map_t port_med_pow_source_map[] = {
	{ LLDP_MED_POW_SOURCE_PRIMARY, "Primary Power Source" },
	{ LLDP_MED_POW_SOURCE_BACKUP,  "Backup Power Source / Power Conservation Mode" },
	{ LLDP_MED_POW_SOURCE_PSE,     "PSE" },
	{ LLDP_MED_POW_SOURCE_LOCAL,   "Local"},
	{ LLDP_MED_POW_SOURCE_BOTH,    "PSE + Local"},
	{ 0, NULL },
};

static lldpctl_map_t port_med_pow_source_map2[] = {
	{ 0,                           "unknown" },
	{ LLDP_MED_POW_SOURCE_PRIMARY, "primary" },
	{ LLDP_MED_POW_SOURCE_BACKUP,  "backup" },
	{ LLDP_MED_POW_SOURCE_PSE,     "pse" },
	{ LLDP_MED_POW_SOURCE_LOCAL,   "local" },
	{ LLDP_MED_POW_SOURCE_BOTH,    "both" },
	{ 0, NULL },
};

static struct atom_map port_med_geoid_map = {
	.key = lldpctl_k_med_location_geoid,
	.map = {
		{ LLDP_MED_LOCATION_GEOID_WGS84, "WGS84" },
		{ LLDP_MED_LOCATION_GEOID_NAD83, "NAD83" },
		{ LLDP_MED_LOCATION_GEOID_NAD83_MLLW, "NAD83/MLLW" },
		{ 0, NULL },
	},
};

static struct atom_map civic_address_type_map = {
	.key = lldpctl_k_med_civicaddress_type,
	.map = {
        { 0,    "Language" },
        { 1,    "Country subdivision" },
        { 2,    "County" },
        { 3,    "City" },
        { 4,    "City division" },
        { 5,    "Block" },
        { 6,    "Street" },
        { 16,   "Direction" },
        { 17,   "Trailing street suffix" },
        { 18,   "Street suffix" },
        { 19,   "Number" },
        { 20,   "Number suffix" },
        { 21,   "Landmark" },
        { 22,   "Additional" },
        { 23,   "Name" },
        { 24,   "ZIP" },
        { 25,   "Building" },
        { 26,   "Unit" },
        { 27,   "Floor" },
        { 28,   "Room" },
        { 29,   "Place type" },
        { 128,  "Script" },
        { 0, NULL },
	},
};

static struct atom_map port_med_policy_map = {
	.key = lldpctl_k_med_policy_type,
	.map = {
		{ LLDP_MED_APPTYPE_VOICE ,           "Voice"},
		{ LLDP_MED_APPTYPE_VOICESIGNAL,      "Voice Signaling"},
		{ LLDP_MED_APPTYPE_GUESTVOICE,       "Guest Voice"},
		{ LLDP_MED_APPTYPE_GUESTVOICESIGNAL, "Guest Voice Signaling"},
		{ LLDP_MED_APPTYPE_SOFTPHONEVOICE,   "Softphone Voice"},
		{ LLDP_MED_APPTYPE_VIDEOCONFERENCE,  "Video Conferencing"},
		{ LLDP_MED_APPTYPE_VIDEOSTREAM,      "Streaming Video"},
		{ LLDP_MED_APPTYPE_VIDEOSIGNAL,      "Video Signaling"},
		{ 0, NULL },
	}
};

static struct atom_map port_med_policy_prio_map = {
	.key = lldpctl_k_med_policy_priority,
	.map = {
		{ 1, "Background" },
		{ 0, "Best effort" },
		{ 2, "Excellent effort" },
		{ 3, "Critical applications" },
		{ 4, "Video" },
		{ 5, "Voice" },
		{ 6, "Internetwork control" },
		{ 7, "Network control" },
		{ 0, NULL },
	},
};

static struct atom_map port_med_pow_priority_map = {
	.key = lldpctl_k_med_power_priority,
	.map = {
		{ 0,                          "unknown" },
		{ LLDP_MED_POW_PRIO_CRITICAL, "critical" },
		{ LLDP_MED_POW_PRIO_HIGH,     "high" },
		{ LLDP_MED_POW_PRIO_LOW,      "low" },
		{ 0, NULL },
	},
};

ATOM_MAP_REGISTER(port_med_geoid_map,        7);
ATOM_MAP_REGISTER(civic_address_type_map,    8);
ATOM_MAP_REGISTER(port_med_policy_map,       9);
ATOM_MAP_REGISTER(port_med_policy_prio_map,  10);
ATOM_MAP_REGISTER(port_med_pow_priority_map, 11);

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_med_policies_list(lldpctl_atom_t *atom)
{
	int i;
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	for (i = 0; i < LLDP_MED_APPTYPE_LAST; i++)
		vlist->parent->port->p_med_policy[i].index = i;
	return (lldpctl_atom_iter_t*)&vlist->parent->port->p_med_policy[0];
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_med_policies_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_med_policy *policy = (struct lldpd_med_policy *)iter;
	if (policy->index == LLDP_MED_APPTYPE_LAST - 1) return NULL;
	return (lldpctl_atom_iter_t*)(++policy);
}

static lldpctl_atom_t*
_lldpctl_atom_value_med_policies_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	struct lldpd_med_policy *policy = (struct lldpd_med_policy *)iter;
	return _lldpctl_new_atom(atom->conn, atom_med_policy, vlist->parent, policy);
}

static int
_lldpctl_atom_new_med_policy(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_med_policy_t *policy =
	    (struct _lldpctl_atom_med_policy_t *)atom;
	policy->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	policy->policy = va_arg(ap, struct lldpd_med_policy *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)policy->parent);
	return 1;
}

static void
_lldpctl_atom_free_med_policy(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_med_policy_t *policy =
	    (struct _lldpctl_atom_med_policy_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)policy->parent);
}

static long int
_lldpctl_atom_get_int_med_policy(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_policy_t *m =
	    (struct _lldpctl_atom_med_policy_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_policy_type:
		return m->policy->type;
	case lldpctl_k_med_policy_unknown:
		return m->policy->unknown;
	case lldpctl_k_med_policy_tagged:
		return m->policy->tagged;
	case lldpctl_k_med_policy_vid:
		return m->policy->vid;
	case lldpctl_k_med_policy_dscp:
		return m->policy->dscp;
	case lldpctl_k_med_policy_priority:
		return m->policy->priority;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_med_policy(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_med_policy_t *m =
	    (struct _lldpctl_atom_med_policy_t *)atom;

	/* Only local port can be modified */
	if (!m->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_policy_type:
		/* We let set any policy type, including one whose are not
		 * compatible with the index. If a policy type is set, the index
		 * will be ignored. If a policy type is 0, the index will be
		 * used to know which policy to "erase". */
		if (value < 0 || value > LLDP_MED_APPTYPE_LAST) goto bad;
		m->policy->type = value;
		return atom;
	case lldpctl_k_med_policy_unknown:
		if (value != 0 && value != 1) goto bad;
		m->policy->unknown = value;
		return atom;
	case lldpctl_k_med_policy_tagged:
		if (value != 0 && value != 1) goto bad;
		m->policy->tagged = value;
		return atom;
	case lldpctl_k_med_policy_vid:
		if (value < 0 || value > 4094) goto bad;
		m->policy->vid = value;
		return atom;
	case lldpctl_k_med_policy_dscp:
		if (value < 0 || value > 63) goto bad;
		m->policy->dscp = value;
		return atom;
	case lldpctl_k_med_policy_priority:
		if (value < 0 || value > 7) goto bad;
		m->policy->priority = value;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static const char*
_lldpctl_atom_get_str_med_policy(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_policy_t *m =
	    (struct _lldpctl_atom_med_policy_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_policy_type:
		return map_lookup(port_med_policy_map.map, m->policy->type);
	case lldpctl_k_med_policy_priority:
		return map_lookup(port_med_policy_prio_map.map, m->policy->priority);
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_med_policy(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_policy_type:
		return _lldpctl_atom_set_int_med_policy(atom, key,
		    map_reverse_lookup(port_med_policy_map.map, value));
	case lldpctl_k_med_policy_priority:
		return _lldpctl_atom_set_int_med_policy(atom, key,
		    map_reverse_lookup(port_med_policy_prio_map.map, value));
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_med_locations_list(lldpctl_atom_t *atom)
{
	int i;
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	for (i = 0; i < LLDP_MED_LOCFORMAT_LAST; i++)
		vlist->parent->port->p_med_location[i].index = i;
	return (lldpctl_atom_iter_t*)&vlist->parent->port->p_med_location[0];
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_med_locations_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_med_loc *location = (struct lldpd_med_loc *)iter;
	if (location->index == LLDP_MED_LOCFORMAT_LAST - 1) return NULL;
	return (lldpctl_atom_iter_t*)(++location);
}

static lldpctl_atom_t*
_lldpctl_atom_value_med_locations_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	struct lldpd_med_loc *location = (struct lldpd_med_loc *)iter;
	return _lldpctl_new_atom(atom->conn, atom_med_location, vlist->parent, location);
}

static int
_lldpctl_atom_new_med_location(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_med_location_t *location =
	    (struct _lldpctl_atom_med_location_t *)atom;
	location->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	location->location = va_arg(ap, struct lldpd_med_loc *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)location->parent);
	return 1;
}

static void
_lldpctl_atom_free_med_location(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_med_location_t *location =
	    (struct _lldpctl_atom_med_location_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)location->parent);
}

static long int
_lldpctl_atom_get_int_med_location(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_location_t *m =
	    (struct _lldpctl_atom_med_location_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_location_format:
		switch (m->location->format) {
		case LLDP_MED_LOCFORMAT_COORD:
			if (m->location->data_len != 16) break;
			return LLDP_MED_LOCFORMAT_COORD;
		case LLDP_MED_LOCFORMAT_CIVIC:
			if ((m->location->data_len < 3) ||
			    (m->location->data_len - 1 <
				m->location->data[0])) break;
			return LLDP_MED_LOCFORMAT_CIVIC;
		case LLDP_MED_LOCFORMAT_ELIN:
			return LLDP_MED_LOCFORMAT_ELIN;
		default:
			return 0;
		}
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	case lldpctl_k_med_location_geoid:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD)
			return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return m->location->data[15];
	case lldpctl_k_med_location_altitude_unit:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD)
			return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return (m->location->data[10] & 0xf0) >> 4;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_med_location(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_med_location_t *mloc =
	    (struct _lldpctl_atom_med_location_t *)atom;

	/* Only local port can be modified */
	if (!mloc->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_location_format:
		switch (value) {
		case 0:		/* Disabling */
		case LLDP_MED_LOCFORMAT_COORD:
			mloc->location->format = value;
			free(mloc->location->data);
			mloc->location->data = calloc(1, 16);
			if (mloc->location->data == NULL) {
				mloc->location->data_len = 0;
				SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
				return NULL;
			}
			mloc->location->data_len = 16;
			return atom;
		case LLDP_MED_LOCFORMAT_CIVIC:
			mloc->location->format = value;
			free(mloc->location->data);
			mloc->location->data = calloc(1, 4);
			if (mloc->location->data == NULL) {
				mloc->location->data_len = 0;
				SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
				return NULL;
			}
			mloc->location->data_len = 4;
			mloc->location->data[0] = 3;
			mloc->location->data[1] = 2; /* Client */
			mloc->location->data[2] = 'U';
			mloc->location->data[3] = 'S';
			return atom;
		case LLDP_MED_LOCFORMAT_ELIN:
			mloc->location->format = value;
			free(mloc->location->data);
			mloc->location->data = NULL;
			mloc->location->data_len = 0;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_med_location_geoid:
		if (mloc->location->format != LLDP_MED_LOCFORMAT_COORD) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len != 16) goto bad;
		switch (value) {
		case 0:
		case LLDP_MED_LOCATION_GEOID_WGS84:
		case LLDP_MED_LOCATION_GEOID_NAD83:
		case LLDP_MED_LOCATION_GEOID_NAD83_MLLW:
			mloc->location->data[15] = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_med_location_altitude_unit:
		if (mloc->location->format != LLDP_MED_LOCFORMAT_COORD) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len != 16) goto bad;
		switch (value) {
		case 0:
		case LLDP_MED_LOCATION_ALTITUDE_UNIT_METER:
		case LLDP_MED_LOCATION_ALTITUDE_UNIT_FLOOR:
			mloc->location->data[10] &= 0x0f;
			mloc->location->data[10] |= value << 4;
			return atom;
		default: goto bad;
		}
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;

}

static const char*
read_fixed_precision(lldpctl_atom_t *atom,
    char *buffer, unsigned shift,
    unsigned intbits, unsigned fltbits, const char *suffix)
{
	struct fp_number fp = fp_buftofp((unsigned char*)buffer, intbits, fltbits, shift);
	char *result = fp_fptostr(fp, suffix);
	if (result == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
		return NULL;
	}

	size_t len = strlen(result) + 1;
	char *stored = _lldpctl_alloc_in_atom(atom, len);
	if (stored == NULL) {
		free(result);
		return NULL;
	}
	strlcpy(stored, result, len);
	free(result);
	return stored;
}

static const char*
_lldpctl_atom_get_str_med_location(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_location_t *m =
	    (struct _lldpctl_atom_med_location_t *)atom;
	char *value;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_location_format:
		return map_lookup(port_med_location_map, m->location->format);
	case lldpctl_k_med_location_geoid:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD) break;
		return map_lookup(port_med_geoid_map.map,
		    m->location->data[15]);
	case lldpctl_k_med_location_latitude:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD) break;
		return read_fixed_precision(atom, m->location->data,
		    0, 9, 25, "NS");
	case lldpctl_k_med_location_longitude:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD) break;
		return read_fixed_precision(atom, m->location->data,
		    40, 9, 25, "EW");
	case lldpctl_k_med_location_altitude:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD) break;
		return read_fixed_precision(atom, m->location->data,
		    84, 22, 8, NULL);
	case lldpctl_k_med_location_altitude_unit:
		if (m->location->format != LLDP_MED_LOCFORMAT_COORD) break;
		switch (m->location->data[10] & 0xf0) {
		case (LLDP_MED_LOCATION_ALTITUDE_UNIT_METER << 4):
			return "m";
		case (LLDP_MED_LOCATION_ALTITUDE_UNIT_FLOOR << 4):
			return "floor";
		}
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	case lldpctl_k_med_location_country:
		if (m->location->format != LLDP_MED_LOCFORMAT_CIVIC) break;
		if (m->location->data_len < 4) return NULL;
		value = _lldpctl_alloc_in_atom(atom, 3);
		if (!value) return NULL;
		memcpy(value, m->location->data + 2, 2);
		return value;
	case lldpctl_k_med_location_elin:
		if (m->location->format != LLDP_MED_LOCFORMAT_ELIN) break;
		value = _lldpctl_alloc_in_atom(atom, m->location->data_len + 1);
		if (!value) return NULL;
		memcpy(value, m->location->data, m->location->data_len);
		return value;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	return NULL;
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_med_location(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	struct _lldpctl_atom_med_location_t *mloc =
	    (struct _lldpctl_atom_med_location_t *)atom;
	struct fp_number fp;
	char *end = NULL;

	/* Only local port can be modified */
	if (!mloc->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_location_latitude:
		if (mloc->location->format != LLDP_MED_LOCFORMAT_COORD) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len != 16) goto bad;
		if (value) fp = fp_strtofp(value, &end, 9, 25);
		if (!end) goto bad;
		if (end && *end != '\0') {
			if (*(end+1) != '\0') goto bad;
			if (*end == 'S') fp = fp_negate(fp);
			else if (*end != 'N') goto bad;
		}
		fp_fptobuf(fp, (unsigned char*)mloc->location->data, 0);
		return atom;
	case lldpctl_k_med_location_longitude:
		if (mloc->location->format != LLDP_MED_LOCFORMAT_COORD) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len != 16) goto bad;
		if (value) fp = fp_strtofp(value, &end, 9, 25);
		if (!end) goto bad;
		if (end && *end != '\0') {
			if (*(end+1) != '\0') goto bad;
			if (*end == 'W') fp = fp_negate(fp);
			else if (*end != 'E') goto bad;
		}
		fp_fptobuf(fp, (unsigned char*)mloc->location->data, 40);
		return atom;
	case lldpctl_k_med_location_altitude:
		if (mloc->location->format != LLDP_MED_LOCFORMAT_COORD) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len != 16) goto bad;
		if (value) fp = fp_strtofp(value, &end, 22, 8);
		if (!end || *end != '\0') goto bad;
		fp_fptobuf(fp, (unsigned char*)mloc->location->data, 84);
		return atom;
	case lldpctl_k_med_location_altitude_unit:
		if (!value) goto bad;
		if (mloc->location->format != LLDP_MED_LOCFORMAT_COORD) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len != 16) goto bad;
		if (!strcmp(value, "m"))
			return _lldpctl_atom_set_int_med_location(atom, key,
			    LLDP_MED_LOCATION_ALTITUDE_UNIT_METER);
		if (!strcmp(value, "f") ||
		    (!strcmp(value, "floor")))
			return _lldpctl_atom_set_int_med_location(atom, key,
			    LLDP_MED_LOCATION_ALTITUDE_UNIT_FLOOR);
		goto bad;
		break;
	case lldpctl_k_med_location_geoid:
		return _lldpctl_atom_set_int_med_location(atom, key,
		    map_reverse_lookup(port_med_geoid_map.map, value));
	case lldpctl_k_med_location_country:
		if (mloc->location->format != LLDP_MED_LOCFORMAT_CIVIC) goto bad;
		if (mloc->location->data == NULL || mloc->location->data_len < 3) goto bad;
		if (!value || strlen(value) != 2) goto bad;
		memcpy(mloc->location->data + 2, value, 2);
		return atom;
	case lldpctl_k_med_location_elin:
		if (!value) goto bad;
		if (mloc->location->format != LLDP_MED_LOCFORMAT_ELIN) goto bad;
		free(mloc->location->data);
		mloc->location->data = calloc(1, strlen(value));
		if (mloc->location->data == NULL) {
			mloc->location->data_len = 0;
			SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
			return NULL;
		}
		mloc->location->data_len = strlen(value);
		memcpy(mloc->location->data, value,
		    mloc->location->data_len);
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;

}

static lldpctl_atom_t*
_lldpctl_atom_get_atom_med_location(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_location_t *m =
	    (struct _lldpctl_atom_med_location_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_location_ca_elements:
		if (m->location->format != LLDP_MED_LOCFORMAT_CIVIC) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
			return NULL;
		}
		return _lldpctl_new_atom(atom->conn, atom_med_caelements_list, m);
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_atom_med_location(lldpctl_atom_t *atom, lldpctl_key_t key,
    lldpctl_atom_t *value)
{
	struct _lldpctl_atom_med_location_t *m =
	    (struct _lldpctl_atom_med_location_t *)atom;
	struct _lldpctl_atom_med_caelement_t *el;
	uint8_t *new;

	/* Only local port can be modified */
	if (!m->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_location_ca_elements:
		if (value->type != atom_med_caelement) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
			return NULL;
		}
		if (m->location->format != LLDP_MED_LOCFORMAT_CIVIC) goto bad;
		if (m->location->data == NULL || m->location->data_len < 3) goto bad;

		/* We append this element. */
		el = (struct _lldpctl_atom_med_caelement_t *)value;
		new = malloc(m->location->data_len + 2 + el->len);
		if (new == NULL) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
			return NULL;
		}
		memcpy(new, m->location->data, m->location->data_len);
		new[m->location->data_len] = el->type;
		new[m->location->data_len + 1] = el->len;
		memcpy(new + m->location->data_len + 2, el->value, el->len);
		new[0] += 2 + el->len;
		free(m->location->data);
		m->location->data = (char*)new;
		m->location->data_len += 2 + el->len;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

struct ca_iter {
	uint8_t *data;
	size_t data_len;
};

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_med_caelements_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_med_caelements_list_t *plist =
	    (struct _lldpctl_atom_med_caelements_list_t *)atom;
	struct ca_iter *iter;
	if (plist->parent->location->data_len < 4 ||
	    *(uint8_t*)plist->parent->location->data < 3 ||
	    !(iter = _lldpctl_alloc_in_atom(atom, sizeof(struct ca_iter))))
		return NULL;
	iter->data = (uint8_t*)plist->parent->location->data + 4;
	iter->data_len = *(uint8_t*)plist->parent->location->data - 3;
	return (lldpctl_atom_iter_t*)iter;
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_med_caelements_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct ca_iter *cai = (struct ca_iter *)iter;
	int len;
	if (cai->data_len < 2) return NULL;
	len = *((uint8_t *)cai->data + 1);
	if (cai->data_len < 2 + len) return NULL;
	cai->data += 2 + len;
	cai->data_len -= 2 + len;
	return (lldpctl_atom_iter_t*)cai;
}

static lldpctl_atom_t*
_lldpctl_atom_value_med_caelements_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_med_caelements_list_t *plist =
	    (struct _lldpctl_atom_med_caelements_list_t *)atom;
	struct ca_iter *cai = (struct ca_iter *)iter;
	size_t len;
	if (cai->data_len < 2) return NULL;
	len = *((uint8_t *)cai->data + 1);
	if (cai->data_len < 2 + len) return NULL;
	return _lldpctl_new_atom(atom->conn, atom_med_caelement, plist->parent,
	    (int)*cai->data, cai->data + 2, len);
}

static lldpctl_atom_t*
_lldpctl_atom_create_med_caelements_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_med_caelements_list_t *plist =
	    (struct _lldpctl_atom_med_caelements_list_t *)atom;
	return _lldpctl_new_atom(atom->conn, atom_med_caelement, plist->parent,
	    -1, NULL, 0);
}

static int
_lldpctl_atom_new_med_caelement(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_med_caelement_t *el =
	    (struct _lldpctl_atom_med_caelement_t *)atom;
	el->parent = va_arg(ap, struct _lldpctl_atom_med_location_t *);
	el->type   = va_arg(ap, int);
	el->value  = va_arg(ap, uint8_t*);
	el->len    = va_arg(ap, size_t);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)el->parent);
	return 1;
}

static void
_lldpctl_atom_free_med_caelement(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_med_caelement_t *el =
	    (struct _lldpctl_atom_med_caelement_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)el->parent);
}

static long int
_lldpctl_atom_get_int_med_caelement(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_caelement_t *m =
	    (struct _lldpctl_atom_med_caelement_t *)atom;

	switch (key) {
	case lldpctl_k_med_civicaddress_type:
		return m->type;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_med_caelement(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_med_caelement_t *el =
	    (struct _lldpctl_atom_med_caelement_t *)atom;

	/* Only local port can be modified */
	if (!el->parent->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_civicaddress_type:
		if (value < 0 || value > 128) goto bad;
		el->type = value;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static const char*
_lldpctl_atom_get_str_med_caelement(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	char *value = NULL;
	struct _lldpctl_atom_med_caelement_t *m =
	    (struct _lldpctl_atom_med_caelement_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_civicaddress_type:
		return map_lookup(civic_address_type_map.map, m->type);
	case lldpctl_k_med_civicaddress_value:
		value = _lldpctl_alloc_in_atom(atom, m->len + 1);
		if (!value) return NULL;
		memcpy(value, m->value, m->len);
		return value;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_med_caelement(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	struct _lldpctl_atom_med_caelement_t *el =
	    (struct _lldpctl_atom_med_caelement_t *)atom;
	size_t len;

	/* Only local port can be modified */
	if (!el->parent->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_civicaddress_value:
		if (!value) goto bad;
		len = strlen(value) + 1;
		if (len > 251) goto bad;
		el->value = _lldpctl_alloc_in_atom(atom, len);
		if (el->value == NULL) return NULL;
		strlcpy((char*)el->value, value, len);
		el->len = strlen(value);
		return atom;
	case lldpctl_k_med_civicaddress_type:
		return _lldpctl_atom_set_int_med_caelement(atom, key,
		    map_reverse_lookup(civic_address_type_map.map, value));
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static int
_lldpctl_atom_new_med_power(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_med_power_t *mpow =
	    (struct _lldpctl_atom_med_power_t *)atom;
	mpow->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)mpow->parent);
	return 1;
}

static void
_lldpctl_atom_free_med_power(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_med_power_t *mpow =
	    (struct _lldpctl_atom_med_power_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)mpow->parent);
}

static const char*
_lldpctl_atom_get_str_med_power(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_power_t *mpow =
	    (struct _lldpctl_atom_med_power_t *)atom;
	struct lldpd_port *port = mpow->parent->port;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_power_type:
		return map_lookup(port_med_pow_devicetype_map,
		    port->p_med_power.devicetype);
	case lldpctl_k_med_power_source:
		return map_lookup(port_med_pow_source_map,
		    port->p_med_power.source);
	case lldpctl_k_med_power_priority:
		return map_lookup(port_med_pow_priority_map.map,
		    port->p_med_power.priority);
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static long int
_lldpctl_atom_get_int_med_power(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_med_power_t *dpow =
	    (struct _lldpctl_atom_med_power_t *)atom;
	struct lldpd_port     *port     = dpow->parent->port;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_med_power_type:
		return port->p_med_power.devicetype;
	case lldpctl_k_med_power_source:
		return port->p_med_power.source;
	case lldpctl_k_med_power_priority:
		return port->p_med_power.priority;
	case lldpctl_k_med_power_val:
		return port->p_med_power.val * 100;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_med_power(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_med_power_t *dpow =
	    (struct _lldpctl_atom_med_power_t *)atom;
	struct lldpd_port *port = dpow->parent->port;

	/* Only local port can be modified */
	if (!dpow->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_med_power_type:
		switch (value) {
		case 0:
		case LLDP_MED_POW_TYPE_PSE:
		case LLDP_MED_POW_TYPE_PD:
			port->p_med_power.devicetype = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_med_power_source:
		switch (value) {
		case LLDP_MED_POW_SOURCE_PRIMARY:
		case LLDP_MED_POW_SOURCE_BACKUP:
			if (port->p_med_power.devicetype != LLDP_MED_POW_TYPE_PSE)
				goto bad;
			port->p_med_power.source = value;
			return atom;
		case LLDP_MED_POW_SOURCE_PSE:
		case LLDP_MED_POW_SOURCE_LOCAL:
		case LLDP_MED_POW_SOURCE_BOTH:
			if (port->p_med_power.devicetype != LLDP_MED_POW_TYPE_PD)
				goto bad;
			port->p_med_power.source = value;
			return atom;
		case LLDP_MED_POW_SOURCE_UNKNOWN:
			port->p_med_power.source = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_med_power_priority:
		if (value < 0 || value > 3) goto bad;
		port->p_med_power.priority = value;
		return atom;
	case lldpctl_k_med_power_val:
		if (value < 0) goto bad;
		port->p_med_power.val = value / 100;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_med_power(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	switch (key) {
	case lldpctl_k_med_power_type:
		return _lldpctl_atom_set_int_med_power(atom, key,
		    map_reverse_lookup(port_med_pow_devicetype_map, value));
	case lldpctl_k_med_power_source:
		return _lldpctl_atom_set_int_med_power(atom, key,
		    map_reverse_lookup(port_med_pow_source_map2, value));
	case lldpctl_k_med_power_priority:
		return _lldpctl_atom_set_int_med_power(atom, key,
		    map_reverse_lookup(port_med_pow_priority_map.map, value));
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static struct atom_builder med_policies_list =
	{ atom_med_policies_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_med_policies_list,
	  .next = _lldpctl_atom_next_med_policies_list,
	  .value = _lldpctl_atom_value_med_policies_list };

static struct atom_builder med_policy =
	{ atom_med_policy, sizeof(struct _lldpctl_atom_med_policy_t),
	  .init = _lldpctl_atom_new_med_policy,
	  .free = _lldpctl_atom_free_med_policy,
	  .get_int = _lldpctl_atom_get_int_med_policy,
	  .set_int = _lldpctl_atom_set_int_med_policy,
	  .get_str = _lldpctl_atom_get_str_med_policy,
	  .set_str = _lldpctl_atom_set_str_med_policy };

static struct atom_builder med_locations_list =
	{ atom_med_locations_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_med_locations_list,
	  .next = _lldpctl_atom_next_med_locations_list,
	  .value = _lldpctl_atom_value_med_locations_list };

static struct atom_builder med_location =
	{ atom_med_location, sizeof(struct _lldpctl_atom_med_location_t),
	  .init = _lldpctl_atom_new_med_location,
	  .free = _lldpctl_atom_free_med_location,
	  .get     = _lldpctl_atom_get_atom_med_location,
	  .set     = _lldpctl_atom_set_atom_med_location,
	  .get_int = _lldpctl_atom_get_int_med_location,
	  .set_int = _lldpctl_atom_set_int_med_location,
	  .get_str = _lldpctl_atom_get_str_med_location,
	  .set_str = _lldpctl_atom_set_str_med_location };

static struct atom_builder med_caelements_list =
	{ atom_med_caelements_list, sizeof(struct _lldpctl_atom_med_caelements_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_med_caelements_list,
	  .next = _lldpctl_atom_next_med_caelements_list,
	  .value = _lldpctl_atom_value_med_caelements_list,
	  .create = _lldpctl_atom_create_med_caelements_list };

static struct atom_builder med_caelement =
	{ atom_med_caelement, sizeof(struct _lldpctl_atom_med_caelement_t),
	  .init = _lldpctl_atom_new_med_caelement,
	  .free = _lldpctl_atom_free_med_caelement,
	  .get_int = _lldpctl_atom_get_int_med_caelement,
	  .set_int = _lldpctl_atom_set_int_med_caelement,
	  .get_str = _lldpctl_atom_get_str_med_caelement,
	  .set_str = _lldpctl_atom_set_str_med_caelement };

static struct atom_builder med_power =
	{ atom_med_power, sizeof(struct _lldpctl_atom_med_power_t),
	  .init = _lldpctl_atom_new_med_power,
	  .free = _lldpctl_atom_free_med_power,
	  .get_int = _lldpctl_atom_get_int_med_power,
	  .set_int = _lldpctl_atom_set_int_med_power,
	  .get_str = _lldpctl_atom_get_str_med_power,
	  .set_str = _lldpctl_atom_set_str_med_power };

ATOM_BUILDER_REGISTER(med_policies_list,   15);
ATOM_BUILDER_REGISTER(med_policy,          16);
ATOM_BUILDER_REGISTER(med_locations_list,  17);
ATOM_BUILDER_REGISTER(med_location,        18);
ATOM_BUILDER_REGISTER(med_caelements_list, 19);
ATOM_BUILDER_REGISTER(med_caelement,       20);
ATOM_BUILDER_REGISTER(med_power,           21);

#endif

