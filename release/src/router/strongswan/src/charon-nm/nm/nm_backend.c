/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
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

#include "nm_service.h"
#include "nm_creds.h"
#include "nm_handler.h"

#include <daemon.h>
#include <processing/jobs/callback_job.h>

typedef struct nm_backend_t nm_backend_t;

/**
 * Data for the NetworkManager backend.
 */
struct nm_backend_t {

	/**
	 * NetworkManager service (VPNPlugin)
	 */
	NMStrongswanPlugin *plugin;

	/**
	 * Glib main loop for a thread, handles DBUS calls
	 */
	GMainLoop *loop;

	/**
	 * credential set registered at the daemon
	 */
	nm_creds_t *creds;

	/**
	 * attribute handler regeisterd at the daemon
	 */
	nm_handler_t *handler;
};

/**
 * Global (but private) instance of the NM backend.
 */
static nm_backend_t *nm_backend = NULL;

/**
 * NM plugin processing routine, creates and handles NMVpnServicePlugin
 */
static job_requeue_t run(nm_backend_t *this)
{
	this->loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(this->loop);
	return JOB_REQUEUE_NONE;
}

/**
 * Cancel the GLib Main Event Loop
 */
static bool cancel(nm_backend_t *this)
{
	if (this->loop)
	{
		if (g_main_loop_is_running(this->loop))
		{
			g_main_loop_quit(this->loop);
		}
		g_main_loop_unref(this->loop);
	}
	return TRUE;
}

/**
 * Deinitialize NetworkManager backend
 */
static void nm_backend_deinit()
{
	nm_backend_t *this = nm_backend;

	if (!this)
	{
		return;
	}
	if (this->plugin)
	{
		g_object_unref(this->plugin);
	}
	lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
	charon->attributes->remove_handler(charon->attributes,
									   &this->handler->handler);
	this->creds->destroy(this->creds);
	this->handler->destroy(this->handler);
	free(this);

	nm_backend = NULL;
}

/**
 * Initialize NetworkManager backend
 */
static bool nm_backend_init()
{
	nm_backend_t *this;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init ();
#endif

#if !GLIB_CHECK_VERSION(2,23,0)
	if (!g_thread_supported())
	{
		g_thread_init(NULL);
	}
#endif

	INIT(this,
		.creds = nm_creds_create(),
		.handler = nm_handler_create(),
	);
	this->plugin = nm_strongswan_plugin_new(this->creds, this->handler);
	nm_backend = this;

	charon->attributes->add_handler(charon->attributes, &this->handler->handler);
	lib->credmgr->add_set(lib->credmgr, &this->creds->set);
	if (!this->plugin)
	{
		DBG1(DBG_CFG, "DBUS binding failed");
		nm_backend_deinit();
		return FALSE;
	}

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)run, this,
				NULL, (callback_job_cancel_t)cancel, JOB_PRIO_CRITICAL));
	return TRUE;
}

/**
 * Initialize/deinitialize NetworkManager backend
 */
static bool nm_backend_cb(void *plugin,
						  plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		return nm_backend_init();
	}
	nm_backend_deinit();
	return TRUE;
}

/*
 * see header file
 */
void nm_backend_register()
{
	static plugin_feature_t features[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)nm_backend_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "NetworkManager backend"),
				PLUGIN_DEPENDS(CUSTOM, "libcharon"),
				PLUGIN_SDEPEND(PRIVKEY, KEY_RSA),
				PLUGIN_SDEPEND(PRIVKEY, KEY_ECDSA),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_ANY),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
	};
	lib->plugins->add_static_features(lib->plugins, "nm-backend", features,
									  countof(features), TRUE, NULL, NULL);
}
