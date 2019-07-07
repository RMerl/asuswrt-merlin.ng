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
#include <batch/pb_tnc_batch.h>
#include <messages/ietf/pb_error_msg.h>
#include <state_machine/pb_tnc_state_machine.h>
#include <utils/debug.h>


int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
	pb_tnc_batch_t *batch;
	pb_tnc_state_machine_t *state;
	pb_tnc_msg_t *msg;
	pb_error_msg_t *error;
	enumerator_t *enumerator;
	bool from_server;
	chunk_t chunk;

	dbg_default_set_level(-1);
	library_init(NULL, "fuzz_pb_tnc");
	plugin_loader_add_plugindirs(PLUGINDIR, PLUGINS);
	if (!lib->plugins->load(lib->plugins, PLUGINS))
	{
		return 1;
	}
	chunk = chunk_create((u_char*)buf, len);

	INIT(state,
		.receive_batch = (void*)return_true,
		.set_empty_cdata = (void*)nop,
	);

	/* parse incoming PB-TNC batch */
	batch = pb_tnc_batch_create_from_data(chunk);
	if (batch->process_header(batch, TRUE, FALSE, &from_server) == SUCCESS ||
		batch->process_header(batch, TRUE, TRUE, &from_server) == SUCCESS)
	{
		batch->process(batch, state);
	}

	/* enumerate correctly decoded PB-TNC messages */
	enumerator = batch->create_msg_enumerator(batch);
	while (enumerator->enumerate(enumerator, &msg))
	{
		msg->get_type(msg);
	}
	enumerator->destroy(enumerator);

	/* enumerate errors detected while parsing PB-TNC batch and messages */
	enumerator = batch->create_error_enumerator(batch);
	while (enumerator->enumerate(enumerator, &msg))
	{
		error = (pb_error_msg_t*)msg;
		error->get_error_code(error);
	}
	enumerator->destroy(enumerator);

	batch->destroy(batch);

	free(state);
	lib->plugins->unload(lib->plugins);
	library_deinit();
	return 0;
}
