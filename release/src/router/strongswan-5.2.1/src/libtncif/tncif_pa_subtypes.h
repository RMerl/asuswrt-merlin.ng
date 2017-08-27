/*
 * Copyright (C) 2011 Andreas Steffen, HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup tncif_pa_subtypes tncif_pa_subtypes
 * @{ @ingroup libtncif
 */

#ifndef TNCIF_PA_SUBTYPES_H_
#define TNCIF_PA_SUBTYPES_H_

typedef enum pa_subtype_ietf_t pa_subtype_ietf_t;
typedef enum pa_subtype_tcg_t pa_subtype_tcg_t;
typedef enum pa_subtype_fhh_t pa_subtype_fhh_t;
typedef enum pa_subtype_ita_t pa_subtype_ita_t;

#include <library.h>
#include <pen/pen.h>

/**
 * PA-TNC IETF Standard Subtypes as defined in section 3.5 of RFC 5792
 */
 enum pa_subtype_ietf_t {
	PA_SUBTYPE_IETF_TESTING =			0x00,
	PA_SUBTYPE_IETF_OPERATING_SYSTEM =	0x01,
	PA_SUBTYPE_IETF_ANTI_VIRUS =		0x02,
	PA_SUBTYPE_IETF_ANTI_SPYWARE =		0x03,
	PA_SUBTYPE_IETF_ANTI_MALWARE =		0x04,
	PA_SUBTYPE_IETF_FIREWALL =			0x05,
	PA_SUBTYPE_IETF_IDPS =				0x06,
	PA_SUBTYPE_IETF_VPN =				0x07,
	PA_SUBTYPE_IETF_NEA_CLIENT =		0x08,
	PA_SUBTYPE_IETF_ANY =				0xff
};

/**
 * enum name for pa_subtype_ietf_t.
 */
extern enum_name_t *pa_subtype_ietf_names;

/**
 * PA-TNC TCG Subtypes
 */
 enum pa_subtype_tcg_t {
	PA_SUBTYPE_TCG_PTS =				0x01,
	PA_SUBTYPE_TCG_SCAP =				0x02,
	PA_SUBTYPE_TCG_SWID =				0x03,
	PA_SUBTYPE_TCG_ANY =				0xff
};

/**
 * enum name for pa_subtype_tcg_t.
 */
extern enum_name_t *pa_subtype_tcg_names;

/**
 * PA-TNC FHH Subtypes
 */
 enum pa_subtype_fhh_t {
	PA_SUBTYPE_FHH_HOSTSCANNER =		0x30,
	PA_SUBTYPE_FHH_DUMMY =				0x31,
	PA_SUBTYPE_FHH_PLATID =				0x33,
	PA_SUBTYPE_FHH_ATTESTATION =		0x34,
	PA_SUBTYPE_FHH_CLAMAV =				0x41,
	PA_SUBTYPE_FHH_ANY =				0xff
};

/**
 * enum name for pa_subtype_fhh_t.
 */
extern enum_name_t *pa_subtype_fhh_names;

/**
 * PA-TNC ITA-HSR Subtypes
 */
 enum pa_subtype_ita_t {
	PA_SUBTYPE_ITA_TEST =				0x01,
	PA_SUBTYPE_ITA_ECHO =				0x02,
	PA_SUBTYPE_ITA_ANY =				0xff
};

/**
 * enum name for pa_subtype_ita_t.
 */
extern enum_name_t *pa_subtype_ita_names;

/**
 * Return the pa_subtype_names for a given PEN
 *
 * @param pen		Private Enterprise Number (PEN)
 * @return			pa_subtype_names if found, NULL else
 */
extern enum_name_t* get_pa_subtype_names(pen_t pen);

#endif /** TNCIF_PA_SUBTYPES_H_ @}*/
