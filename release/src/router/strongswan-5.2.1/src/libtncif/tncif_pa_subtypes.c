/*
 * Copyright (C) 2010-2011 Andreas Steffen
 *
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

#include "tncif_pa_subtypes.h"

ENUM_BEGIN(pa_subtype_ietf_names, PA_SUBTYPE_IETF_TESTING, PA_SUBTYPE_IETF_NEA_CLIENT,
	"Testing",
	"Operating System",
	"Anti-Virus",
	"Anti-Spyware",
	"Anti-Malware",
	"Firewall",
	"IDPS",
	"VPN",
	"NEA Client"
);
ENUM_NEXT(pa_subtype_ietf_names, PA_SUBTYPE_IETF_ANY, PA_SUBTYPE_IETF_ANY,
								PA_SUBTYPE_IETF_NEA_CLIENT,
	"ANY"
);
ENUM_END(pa_subtype_ietf_names, PA_SUBTYPE_IETF_ANY);

ENUM_BEGIN(pa_subtype_tcg_names, PA_SUBTYPE_TCG_PTS, PA_SUBTYPE_TCG_SWID,
	"PTS",
	"SCAP",
	"SWID"
);
ENUM_NEXT(pa_subtype_tcg_names, PA_SUBTYPE_TCG_ANY, PA_SUBTYPE_TCG_ANY,
								PA_SUBTYPE_TCG_SWID,
	"ANY"
);
ENUM_END(pa_subtype_tcg_names, PA_SUBTYPE_TCG_ANY);

ENUM_BEGIN(pa_subtype_fhh_names, PA_SUBTYPE_FHH_HOSTSCANNER, PA_SUBTYPE_FHH_DUMMY,
	"HostScanner",
	"Dummy"
);
ENUM_NEXT(pa_subtype_fhh_names, PA_SUBTYPE_FHH_PLATID, PA_SUBTYPE_FHH_ATTESTATION,
								PA_SUBTYPE_FHH_DUMMY,
	"PlatformID",
	"Attestation"
);
ENUM_NEXT(pa_subtype_fhh_names, PA_SUBTYPE_FHH_CLAMAV, PA_SUBTYPE_FHH_CLAMAV,
								PA_SUBTYPE_FHH_ATTESTATION,
	"ClamAV"
);
ENUM_NEXT(pa_subtype_fhh_names, PA_SUBTYPE_FHH_ANY, PA_SUBTYPE_FHH_ANY,
								PA_SUBTYPE_FHH_CLAMAV,
	"ANY"
);
ENUM_END(pa_subtype_fhh_names, PA_SUBTYPE_FHH_ANY);

ENUM_BEGIN(pa_subtype_ita_names, PA_SUBTYPE_ITA_TEST, PA_SUBTYPE_ITA_ECHO,
	"Test",
	"Echo"
);
ENUM_NEXT(pa_subtype_ita_names, PA_SUBTYPE_ITA_ANY, PA_SUBTYPE_ITA_ANY,
								PA_SUBTYPE_ITA_ECHO,
	"ANY"
);
ENUM_END(pa_subtype_ita_names, PA_SUBTYPE_ITA_ANY);

/**
 * See header
 */
enum_name_t* get_pa_subtype_names(pen_t pen)
{
	switch (pen)
	{
		case PEN_IETF:
			return pa_subtype_ietf_names;
		case PEN_TCG:
			return pa_subtype_tcg_names;
		case PEN_FHH:
			return pa_subtype_fhh_names;
		case PEN_ITA:
			return pa_subtype_ita_names;
		default:
			break;
	}
	return NULL;
}
