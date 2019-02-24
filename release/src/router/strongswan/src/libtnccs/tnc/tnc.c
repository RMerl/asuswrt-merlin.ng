/*
 * Copyright (C) 2011 Andreas Steffen
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

#define _GNU_SOURCE /* for stdndup() */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "tnc.h"

#include <utils/lexparser.h>
#include <utils/debug.h>

#ifdef WIN32
# define DEFAULT_TNC_CONFIG "tnc_config"
#else
# define DEFAULT_TNC_CONFIG "/etc/tnc_config"
#endif

typedef struct private_tnc_t private_tnc_t;

typedef tnccs_manager_t *(*tnc_create_tnccs_manager_t)(void);
typedef imc_manager_t *(*tnc_create_imc_manager_t)(void);
typedef imv_manager_t *(*tnc_create_imv_manager_t)(void);

/**
 * Private additions to tnc_t.
 */
struct private_tnc_t {

	/**
	 * Public members of tnc_t.
	 */
	tnc_t public;

	/**
	 * Number of times we have been initialized
	 */
	refcount_t ref;
};

/**
 * Register plugins if built statically
 */
#ifdef STATIC_PLUGIN_CONSTRUCTORS
#include "plugin_constructors.c"
#endif

/**
 * Single instance of tnc_t.
 */
tnc_t *tnc;

/**
 * Described in header.
 */
void libtnccs_init(void)
{
	private_tnc_t *this;

	if (tnc)
	{	/* already initialized, increase refcount */
		this = (private_tnc_t*)tnc;
		ref_get(&this->ref);
		return;
	}

	INIT(this,
		.public = {
		},
		.ref = 1,
	);
	tnc = &this->public;
	lib->settings->add_fallback(lib->settings, "%s.tnc", "libtnccs", lib->ns);
	lib->settings->add_fallback(lib->settings, "%s.plugins", "libtnccs.plugins",
								lib->ns);
}

/**
 * Described in header.
 */
void libtnccs_deinit(void)
{
	private_tnc_t *this = (private_tnc_t*)tnc;

	if (!this || !ref_put(&this->ref))
	{	/* have more users */
		return;
	}

	free(this);
	tnc = NULL;
}

static bool load_imcvs_from_config(char *filename, bool is_imc)
{
	bool success = FALSE;
	int line_nr = 0;
	chunk_t *src, line;
	char *label;

	if (!filename || !*filename)
	{
		return TRUE;
	}

	label = is_imc ? "IMC" : "IMV";

	DBG1(DBG_TNC, "loading %ss from '%s'", label, filename);
	src = chunk_map(filename, FALSE);
	if (!src)
	{
		DBG1(DBG_TNC, "opening configuration file '%s' failed: %s", filename,
			 strerror(errno));
		return FALSE;
	}

	while (fetchline(src, &line))
	{
		char *name, *path;
		chunk_t token;

		line_nr++;

		/* skip comments or empty lines */
		if (*line.ptr == '#' || !eat_whitespace(&line))
		{
			continue;
		}

		/* determine keyword */
		if (!extract_token(&token, ' ', &line))
		{
			DBG1(DBG_TNC, "line %d: keyword must be followed by a space",
						   line_nr);
			break;
		}

		/* only interested in IMCs or IMVs depending on label */
		if (!match(label, &token))
		{
			continue;
		}

		/* advance to the IMC/IMV name and extract it */
		if (!extract_token(&token, '"', &line) ||
			!extract_token(&token, '"', &line))
		{
			DBG1(DBG_TNC, "line %d: %s name must be set in double quotes",
						   line_nr, label);
			break;
		}

		/* copy the IMC/IMV name */
		name = strndup(token.ptr, token.len);

		/* advance to the IMC/IMV path and extract it */
		if (!eat_whitespace(&line))
		{
			DBG1(DBG_TNC, "line %d: %s path is missing", line_nr, label);
			free(name);
			break;
		}
		if (!extract_token(&token, ' ', &line))
		{
			token = line;
		}

		/* copy the IMC/IMV path */
		path = strndup(token.ptr, token.len);

		/* load and register an IMC/IMV instance */
		if (is_imc)
		{
			success = tnc->imcs->load(tnc->imcs, name, path);
		}
		else
		{
			success = tnc->imvs->load(tnc->imvs, name, path);
		}
		free(name);
		free(path);
		if (!success)
		{
			break;
		}
	}
	chunk_unmap(src);
	return success;
}

/**
 * Described in header.
 */
bool tnc_manager_register(plugin_t *plugin, plugin_feature_t *feature,
						  bool reg, void *data)
{
	bool load_imcvs = FALSE;
	bool is_imc = FALSE;

	if (feature->type == FEATURE_CUSTOM)
	{
		if (streq(feature->arg.custom, "tnccs-manager"))
		{
			if (reg)
			{
				tnc->tnccs = ((tnc_create_tnccs_manager_t)data)();
			}
			else
			{
				tnc->tnccs->destroy(tnc->tnccs);
				tnc->tnccs = NULL;
			}
		}
		else if (streq(feature->arg.custom, "imc-manager"))
		{
			if (reg)
			{
				tnc->imcs = ((tnc_create_imc_manager_t)data)();
				is_imc = TRUE;
				load_imcvs = TRUE;
			}
			else
			{
				tnc->imcs->destroy(tnc->imcs);
				tnc->imcs = NULL;
			}
		}
		else if (streq(feature->arg.custom, "imv-manager"))
		{
			if (reg)
			{
				tnc->imvs = ((tnc_create_imv_manager_t)data)();
				is_imc = FALSE;
				load_imcvs = TRUE;
			}
			else
			{
				tnc->imvs->destroy(tnc->imvs);
				tnc->imvs = NULL;
			}
		}
		else
		{
			return FALSE;
		}

		if (load_imcvs)
		{
			load_imcvs_from_config(
						lib->settings->get_str(lib->settings,
							"%s.tnc.tnc_config", DEFAULT_TNC_CONFIG, lib->ns),
						is_imc);
		}
	}
	return TRUE;
}
