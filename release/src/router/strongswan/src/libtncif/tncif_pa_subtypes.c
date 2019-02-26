/*
 * Copyright (C) 2010-2017 Andreas Steffen
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

/* IETF PA Subtype names */
ENUM_BEGIN(pa_subtype_ietf_names, PA_SUBTYPE_IETF_TESTING, PA_SUBTYPE_IETF_SWIMA,
	"Testing",
	"Operating System",
	"Anti-Virus",
	"Anti-Spyware",
	"Anti-Malware",
	"Firewall",
	"IDPS",
	"VPN",
	"NEA Client",
	"SWIMA"
);
ENUM_NEXT(pa_subtype_ietf_names, PA_SUBTYPE_IETF_ANY, PA_SUBTYPE_IETF_ANY,
								PA_SUBTYPE_IETF_SWIMA,
	"ANY"
);
ENUM_END(pa_subtype_ietf_names, PA_SUBTYPE_IETF_ANY);

/* TCG PA Subtype names */
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

/* PWG PA Subtype names */
ENUM_BEGIN(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_TESTING,
								 PA_SUBTYPE_PWG_HCD_UNKNOWN,
	"HCD Testing",
	"HCD Other",
	"HCD Unknown"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_CONSOLE,
								PA_SUBTYPE_PWG_HCD_COVER,
								PA_SUBTYPE_PWG_HCD_UNKNOWN,
	"HCD Console",
	"HCD System",
	"HCD Cover"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_INPUT_TRAY,
								PA_SUBTYPE_PWG_HCD_MARKER,
								PA_SUBTYPE_PWG_HCD_COVER,
	"HCD Input Tray",
	"HCD Output Tray",
	"HCD Marker"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_MEDIA_PATH,
								PA_SUBTYPE_PWG_HCD_INTERPRETER,
								PA_SUBTYPE_PWG_HCD_MARKER,
	"HCD Media Path",
	"HCD Channel",
	"HCD Interpreter"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_FINISHER,
								PA_SUBTYPE_PWG_HCD_FINISHER,
								PA_SUBTYPE_PWG_HCD_INTERPRETER,
	"HCD Finisher"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_INTERFACE,
								PA_SUBTYPE_PWG_HCD_INTERFACE,
								PA_SUBTYPE_PWG_HCD_FINISHER,
	"HCD Interface"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_HCD_SCANNER,
								PA_SUBTYPE_PWG_HCD_SCANNER,
								PA_SUBTYPE_PWG_HCD_INTERFACE,
	"HCD Scanner"
);
ENUM_NEXT(pa_subtype_pwg_names, PA_SUBTYPE_PWG_ANY, PA_SUBTYPE_PWG_ANY,
								PA_SUBTYPE_PWG_HCD_SCANNER,
	"ANY"
);
ENUM_END(pa_subtype_pwg_names, PA_SUBTYPE_PWG_ANY);

/* FHH PA Subtype names */
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

/* ITA-HSR PA Subtype names */
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
		case PEN_PWG:
			return pa_subtype_pwg_names;
		case PEN_FHH:
			return pa_subtype_fhh_names;
		case PEN_ITA:
			return pa_subtype_ita_names;
		default:
			break;
	}
	return NULL;
}
