/*
 * Copyright (C) 2017-2018 Andreas Steffen
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

#include "swima/swima_data_model.h"

/**
 * ISO/IEC 19770-2-2015: Information Technology - Software Asset Management -
 * Part 2: Software Identification Tag
 */
pen_type_t swima_data_model_iso_2015_swid_xml = { PEN_IETF, 0 };

/**
 * ISO/IEC 19770-2-2009: Information Technology - Software Asset Management -
 * Part 2: Software Identification Tag
 */
pen_type_t swima_data_model_iso_2009_swid_xml = { PEN_IETF, 1 };
