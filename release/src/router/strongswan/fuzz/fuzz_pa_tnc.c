/*
 * Copyright (C) 2018 Andreas Steffen
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

#include <library.h>
#include <imcv.h>
#include <pa_tnc/pa_tnc_msg.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <utils/debug.h>

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
	pa_tnc_msg_t *msg;
	pa_tnc_attr_t *attr;
	ietf_attr_pa_tnc_error_t *error;
	linked_list_t *non_fatal_types;
	enumerator_t *enumerator;
	chunk_t chunk;

	dbg_default_set_level(-1);
	library_init(NULL, "fuzz_pa_tnc");
	plugin_loader_add_plugindirs(PLUGINDIR, PLUGINS);
	if (!lib->plugins->load(lib->plugins, PLUGINS))
	{
		return 1;
	}
	libimcv_init(FALSE);
	chunk = chunk_create((u_char*)buf, len);

	/* Parse incoming PA-TNC message */
	msg = pa_tnc_msg_create_from_data(chunk);
	if (msg->process(msg) == SUCCESS)
	{
		non_fatal_types = linked_list_create();
		msg->process_ietf_std_errors(msg, non_fatal_types);
		non_fatal_types->destroy(non_fatal_types);
	}

	/* enumerate correctly decoded attributes */
	enumerator = msg->create_attribute_enumerator(msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		attr->get_noskip_flag(attr);
	}
	enumerator->destroy(enumerator);

	/* enumerate errors detected while parsing PA-TNC message and attributes */
	enumerator = msg->create_error_enumerator(msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		error = (ietf_attr_pa_tnc_error_t*)attr;
		error->get_error_code(error);
	}
	enumerator->destroy(enumerator);

	msg->destroy(msg);

	libimcv_deinit();
	lib->plugins->unload(lib->plugins);
	library_deinit();
	return 0;
}
