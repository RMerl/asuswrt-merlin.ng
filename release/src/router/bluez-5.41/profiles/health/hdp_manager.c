/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>

#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "btio/btio.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/uuid-helper.h"
#include "src/log.h"

#include "hdp_types.h"
#include "hdp_manager.h"
#include "hdp.h"

static int hdp_adapter_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	return hdp_adapter_register(adapter);
}

static void hdp_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	hdp_adapter_unregister(adapter);
}

static int hdp_driver_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	return hdp_device_register(device);
}

static void hdp_driver_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	hdp_device_unregister(device);
}

static struct btd_profile hdp_source_profile = {
	.name		= "hdp-source",
	.remote_uuid	= HDP_SOURCE_UUID,

	.device_probe	= hdp_driver_probe,
	.device_remove	= hdp_driver_remove,

	.adapter_probe	= hdp_adapter_probe,
	.adapter_remove	= hdp_adapter_remove,
};

static struct btd_profile hdp_sink_profile = {
	.name		= "hdp-sink",
	.remote_uuid	= HDP_SINK_UUID,

	.device_probe	= hdp_driver_probe,
	.device_remove	= hdp_driver_remove,
};

int hdp_manager_init(void)
{
	if (hdp_manager_start() < 0)
		return -1;

	btd_profile_register(&hdp_source_profile);
	btd_profile_register(&hdp_sink_profile);

	return 0;
}

void hdp_manager_exit(void)
{
	btd_profile_unregister(&hdp_sink_profile);
	btd_profile_unregister(&hdp_source_profile);

	hdp_manager_stop();
}
