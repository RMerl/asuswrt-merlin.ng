/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "if-main.h"
#include "../hal-utils.h"

const btmce_interface_t *if_mce = NULL;

/*
 *  Callback for get_remote_mas_instances
 */
static void btmce_remote_mas_instances_cb(bt_status_t status,
						bt_bdaddr_t *bd_addr,
						int num_instances,
						btmce_mas_instance_t *instances)
{
	int i;

	haltest_info("%s: status=%s bd_addr=%s num_instance=%d\n", __func__,
				bt_status_t2str(status), bdaddr2str(bd_addr),
				num_instances);

	for (i = 0; i < num_instances; i++)
		haltest_info("id=%d scn=%d msg_types=%d name=%s\n",
				instances[i].id, instances[i].scn,
				instances[i].msg_types, instances[i].p_name);
}

static btmce_callbacks_t mce_cbacks = {
	.size = sizeof(mce_cbacks),
	.remote_mas_instances_cb = btmce_remote_mas_instances_cb,
};

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_mce);

	EXEC(if_mce->init, &mce_cbacks);
}

static void get_remote_mas_instances_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	}
}

/* search for MAS instances on remote device */

static void get_remote_mas_instances_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_mce);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_mce->get_remote_mas_instances, &addr);
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(get_remote_mas_instances, "<addr>"),
	END_METHOD
};

const struct interface mce_if = {
	.name = "mce",
	.methods = methods
};
