/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "imcv.h"
#include "pts/components/pts_comp_func_name.h"

#include <utils/debug.h>

typedef struct private_pts_comp_func_name_t private_pts_comp_func_name_t;

/**
 * Private data of a pts_comp_func_name_t object.
 *
 */
struct private_pts_comp_func_name_t {

	/**
	 * Public pts_comp_func_name_t interface.
	 */
	pts_comp_func_name_t public;

	/**
	 * PTS Component Functional Name Vendor ID
	 */
	uint32_t vid;

	/**
	 * PTS Component Functional Name
	 */
	uint32_t name;

	/**
	 * PTS Component Functional Name Qualifier
	 */
	uint8_t qualifier;

};

METHOD(pts_comp_func_name_t, get_vendor_id, uint32_t,
	private_pts_comp_func_name_t *this)
{
	return this->vid;
}

METHOD(pts_comp_func_name_t, get_name, uint32_t,
	private_pts_comp_func_name_t *this)
{
	return this->name;
}

METHOD(pts_comp_func_name_t, get_qualifier, uint8_t,
	private_pts_comp_func_name_t *this)
{
	return this->qualifier;
}

METHOD(pts_comp_func_name_t, set_qualifier, void,
	private_pts_comp_func_name_t *this, uint8_t qualifier)
{
	this->qualifier = qualifier;
}

static bool equals(private_pts_comp_func_name_t *this,
				   private_pts_comp_func_name_t *other)
{
	if (this->vid != other->vid || this->name != other->name)
	{
		return FALSE;
	}
	if (this->qualifier == PTS_QUALIFIER_UNKNOWN ||
		other->qualifier == PTS_QUALIFIER_UNKNOWN)
	{
		return TRUE;
	}
	/* TODO handle qualifier wildcards */

	return this->qualifier == other->qualifier;
}

METHOD(pts_comp_func_name_t, clone_, pts_comp_func_name_t*,
	private_pts_comp_func_name_t *this)
{
	private_pts_comp_func_name_t *clone;

	clone = malloc_thing(private_pts_comp_func_name_t);
	memcpy(clone, this, sizeof(private_pts_comp_func_name_t));

	return &clone->public;
}

METHOD(pts_comp_func_name_t, log_, void,
	private_pts_comp_func_name_t *this, char *label)
{
	enum_name_t *names, *types;
	char flags[8];
	int type;

	names = imcv_pts_components->get_comp_func_names(imcv_pts_components,
											this->vid);
	types = imcv_pts_components->get_qualifier_type_names(imcv_pts_components,
											this->vid);
	type =  imcv_pts_components->get_qualifier(imcv_pts_components,
											&this->public, flags);

	if (names && types)
	{
		DBG3(DBG_PTS, "%s%N functional component '%N' [%s] '%N'",
			 label, pen_names, this->vid, names, this->name, flags, types, type);
	}
	else
	{
		DBG3(DBG_PTS, "%s0x%06x functional component 0x%08x 0x%02x",
			 label, this->vid, this->name, this->qualifier);
	}
}

METHOD(pts_comp_func_name_t, destroy, void,
	private_pts_comp_func_name_t *this)
{
	free(this);
}

/**
 * See header
 */
pts_comp_func_name_t* pts_comp_func_name_create(uint32_t vid, uint32_t name,
												uint8_t qualifier)
{
	private_pts_comp_func_name_t *this;

	INIT(this,
		.public = {
			.get_vendor_id = _get_vendor_id,
			.get_name = _get_name,
			.get_qualifier = _get_qualifier,
			.set_qualifier = _set_qualifier,
			.equals = (bool(*)(pts_comp_func_name_t*,pts_comp_func_name_t*))equals,
			.clone = _clone_,
			.log = _log_,
			.destroy = _destroy,
		},
		.vid = vid,
		.name = name,
		.qualifier = qualifier,
	);

	return &this->public;
}

