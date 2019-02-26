/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "ita_attr.h"
#include "ita/ita_attr_command.h"
#include "ita/ita_attr_dummy.h"
#include "ita/ita_attr_get_settings.h"
#include "ita/ita_attr_settings.h"
#include "ita/ita_attr_angel.h"
#include "generic/generic_attr_string.h"

ENUM(ita_attr_names, ITA_ATTR_COMMAND, ITA_ATTR_DEVICE_ID,
	"Command",
	"Dummy",
	"Get Settings",
	"Settings",
	"Start Angel",
	"Stop Angel",
	"Echo",
	"Device ID"
);

/**
 * See header
 */
pa_tnc_attr_t* ita_attr_create_from_data(uint32_t type, size_t length,
										 chunk_t value)
{
	switch (type)
	{
		case ITA_ATTR_COMMAND:
			return ita_attr_command_create_from_data(length, value);
		case ITA_ATTR_DUMMY:
			return ita_attr_dummy_create_from_data(length, value);
		case ITA_ATTR_GET_SETTINGS:
			return ita_attr_get_settings_create_from_data(length, value);
		case ITA_ATTR_SETTINGS:
			return ita_attr_settings_create_from_data(length, value);
		case ITA_ATTR_START_ANGEL:
			return ita_attr_angel_create_from_data(TRUE);
		case ITA_ATTR_STOP_ANGEL:
			return ita_attr_angel_create_from_data(FALSE);
		case ITA_ATTR_DEVICE_ID:
			return generic_attr_string_create_from_data(length, value,
									pen_type_create(PEN_ITA, type));
		default:
			return NULL;
	}
}
