/*
 * Copyright (C) 2011-2013 Andreas Steffen
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

#include "tnc_ifmap_listener.h"
#include "tnc_ifmap_soap.h"
#include "tnc_ifmap_renew_session_job.h"

#include <daemon.h>
#include <hydra.h>
#include <utils/debug.h>

#define IFMAP_RENEW_SESSION_INTERVAL	150

typedef struct private_tnc_ifmap_listener_t private_tnc_ifmap_listener_t;

/**
 * Private data of an tnc_ifmap_listener_t object.
 */
struct private_tnc_ifmap_listener_t {

	/**
	 * Public tnc_ifmap_listener_t interface.
	 */
	tnc_ifmap_listener_t public;

	/**
	 * TNC IF-MAP 2.0 SOAP interface
	 */
	tnc_ifmap_soap_t *ifmap;

};

/**
 * Publish PEP device-ip metadata
 */
static bool publish_device_ip_addresses(private_tnc_ifmap_listener_t *this)
{
	enumerator_t *enumerator;
	host_t *host;
	bool success = TRUE;

	enumerator = hydra->kernel_interface->create_address_enumerator(
									hydra->kernel_interface, ADDR_TYPE_REGULAR);
	while (enumerator->enumerate(enumerator, &host))
	{
		if (!this->ifmap->publish_device_ip(this->ifmap, host))
		{
			success = FALSE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return success;
}

/**
 * Publish all IKE_SA metadata
 */
static bool reload_metadata(private_tnc_ifmap_listener_t *this)
{
	ike_sa_t *ike_sa;
	enumerator_t *enumerator;
	bool success = TRUE;

	enumerator = charon->controller->create_ike_sa_enumerator(
												charon->controller, FALSE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (ike_sa->get_state(ike_sa) != IKE_ESTABLISHED)
		{
			continue;
		}
		if (!this->ifmap->publish_ike_sa(this->ifmap, ike_sa, TRUE) ||
			!this->ifmap->publish_virtual_ips(this->ifmap, ike_sa, TRUE))
		{
			success = FALSE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return success;
}

METHOD(listener_t, ike_updown, bool,
	private_tnc_ifmap_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	if (ike_sa->get_state(ike_sa) != IKE_CONNECTING)
	{
		this->ifmap->publish_ike_sa(this->ifmap, ike_sa, up);
	}
	return TRUE;
}

METHOD(listener_t, assign_vips, bool,
	private_tnc_ifmap_listener_t *this, ike_sa_t *ike_sa, bool assign)
{
	this->ifmap->publish_virtual_ips(this->ifmap, ike_sa, assign);
	return TRUE;
}

METHOD(listener_t, alert, bool,
	private_tnc_ifmap_listener_t *this, ike_sa_t *ike_sa, alert_t alert,
	va_list args)
{
	if (alert == ALERT_PEER_AUTH_FAILED)
	{
		this->ifmap->publish_enforcement_report(this->ifmap,
							ike_sa->get_other_host(ike_sa),
							"block", "authentication failed");
	}
	return TRUE;
}

METHOD(tnc_ifmap_listener_t, destroy, void,
	private_tnc_ifmap_listener_t *this)
{
	if (this->ifmap)
	{
		if (this->ifmap->get_session_id(this->ifmap))
		{
			this->ifmap->endSession(this->ifmap);
		}
		this->ifmap->destroy(this->ifmap);
	}
	free(this);
}

/**
 * See header
 */
tnc_ifmap_listener_t *tnc_ifmap_listener_create(bool reload)
{
	private_tnc_ifmap_listener_t *this;
	job_t *job;
	u_int32_t reschedule;

	INIT(this,
		.public = {
			.listener = {
				.ike_updown = _ike_updown,
				.assign_vips = _assign_vips,
				.alert = _alert,
			},
			.destroy = _destroy,
		},
		.ifmap = tnc_ifmap_soap_create(),
	);

	if (!this->ifmap)
	{
		destroy(this);
		return NULL;
	}
	if (!this->ifmap->newSession(this->ifmap))
	{
		destroy(this);
		return NULL;
	}
	if (!this->ifmap->purgePublisher(this->ifmap))
	{
		destroy(this);
		return NULL;
	}
	if (!publish_device_ip_addresses(this))
	{
		destroy(this);
		return NULL;
	}
	if (reload)
	{
		if (!reload_metadata(this))
		{
			destroy(this);
			return NULL;
		}
	}

	/* schedule periodic transmission of IF-MAP renewSession request */
	reschedule =  lib->settings->get_int(lib->settings,
								"%s.plugins.tnc-ifmap.renew_session_interval",
								 IFMAP_RENEW_SESSION_INTERVAL, lib->ns);

	job = (job_t*)tnc_ifmap_renew_session_job_create(
						this->ifmap->get_ref(this->ifmap), reschedule);
	lib->scheduler->schedule_job(lib->scheduler, job, reschedule);

	return &this->public;
}

