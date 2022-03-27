/*
 * Copyright (C) 2013 Intel Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>
#include <poll.h>

#include <hardware/audio.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_av.h>
#include <hardware/bt_hh.h>
#include <hardware/bt_pan.h>
#include <hardware/bt_sock.h>
#include <hardware/bt_hf.h>
#include <hardware/bt_hl.h>

#include "hal.h"

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
#include <hardware/bt_hf_client.h>
#include <hardware/bt_mce.h>
#endif

#include <hardware/bt_rc.h>
#include <hardware/bt_gatt.h>
#include <hardware/bt_gatt_types.h>
#include <hardware/bt_gatt_client.h>
#include <hardware/bt_gatt_server.h>

extern audio_hw_device_t *if_audio;

/* Interfaces from hal that can be populated during application lifetime */
extern const bt_interface_t *if_bluetooth;
extern const btav_interface_t *if_av;
extern const btrc_interface_t *if_rc;
extern const bthf_interface_t *if_hf;
extern const bthh_interface_t *if_hh;
extern const btpan_interface_t *if_pan;
extern const bthl_interface_t *if_hl;
extern const btsock_interface_t *if_sock;
extern const btgatt_interface_t *if_gatt;
extern const btgatt_server_interface_t *if_gatt_server;
extern const btgatt_client_interface_t *if_gatt_client;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
extern const btrc_ctrl_interface_t *if_rc_ctrl;
extern const bthf_client_interface_t *if_hf_client;
extern const btmce_interface_t *if_mce;
extern const btav_interface_t *if_av_sink;
#endif

/*
 * Structure defines top level interfaces that can be used in test tool
 * this will contain values as: bluetooth, av, gatt, socket, pan...
 */
struct interface {
	const char *name; /* interface name */
	struct method *methods; /* methods available for this interface */
};

extern const struct interface audio_if;
extern const struct interface sco_if;
extern const struct interface bluetooth_if;
extern const struct interface av_if;
extern const struct interface rc_if;
extern const struct interface gatt_if;
extern const struct interface gatt_client_if;
extern const struct interface gatt_server_if;
extern const struct interface pan_if;
extern const struct interface sock_if;
extern const struct interface hf_if;
extern const struct interface hh_if;
extern const struct interface hl_if;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
extern const struct interface ctrl_rc_if;
extern const struct interface hf_client_if;
extern const struct interface mce_if;
extern const struct interface av_sink_if;
#endif

/* Interfaces that will show up in tool (first part of command line) */
extern const struct interface *interfaces[];

#define METHOD(name, func, comp, help) {name, func, comp, help}
#define STD_METHOD(m) {#m, m##_p, NULL, NULL}
#define STD_METHODC(m) {#m, m##_p, m##_c, NULL}
#define STD_METHODH(m, h) {#m, m##_p, NULL, h}
#define STD_METHODCH(m, h) {#m, m##_p, m##_c, h}
#define END_METHOD {"", NULL, NULL, NULL}

/*
 * Function to parse argument for function, argv[0] and argv[1] are already
 * parsed before this function is called and contain interface and method name
 * up to argc - 1 arguments are finished and should be used to decide which
 * function enumeration function to return
 */
typedef void (*parse_and_call)(int argc, const char **argv);

/*
 * This is prototype of function that will return string for given number.
 * Purpose is to enumerate string for auto completion.
 * Function of this type will always be called in loop.
 * First time function is called i = 0, then if function returns non-NULL
 * it will be called again till for some value of i it will return NULL
 */
typedef const char *(*enum_func)(void *user, int i);

/*
 * This is prototype of function that when given argc, argv will
 * fill enum_func with pointer to function that will enumerate
 * parameters for argc argument, user will be passed to enum_func.
 */
typedef void (*tab_complete)(int argc, const char **argv, enum_func *enum_func,
								void **user);

/*
 * For each method there is name and two functions to parse command line
 * and call proper hal function on.
 */
struct method {
	const char *name;
	parse_and_call func;
	tab_complete complete;
	const char *help;
};

int haltest_error(const char *format, ...)
					__attribute__((format(printf, 1, 2)));
int haltest_info(const char *format, ...)__attribute__((format(printf, 1, 2)));
int haltest_warn(const char *format, ...)__attribute__((format(printf, 1, 2)));

/* Enumerator for discovered devices, to be used as tab completion enum_func */
const char *enum_devices(void *v, int i);
const char *interface_name(void *v, int i);
const char *command_name(void *v, int i);
void add_remote_device(const bt_bdaddr_t *addr);
bool close_hw_bt_dev(void);

const struct interface *get_interface(const char *name);
struct method *get_method(struct method *methods, const char *name);
struct method *get_command(const char *name);
const struct method *get_interface_method(const char *iname,
							const char *mname);

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

/* Helper macro for executing function on interface and printing BT_STATUS */
#define EXEC(f, ...) \
	{ \
		if (f) { \
			int err = f(__VA_ARGS__); \
			haltest_info("%s: %s\n", #f, bt_status_t2str(err)); \
		} else { \
			haltest_info("%s is NULL\n", #f); \
		} \
	}

/* Helper macro for executing void function on interface */
#define EXECV(f, ...) \
	{ \
		(void) f(__VA_ARGS__); \
		haltest_info("%s: void\n", #f); \
	}

#define RETURN_IF_NULL(x) \
	do { if (!x) { haltest_error("%s is NULL\n", #x); return; } } while (0)

#define VERIFY_ADDR_ARG(n, adr) \
	do { \
		if (n < argc) {\
			str2bt_bdaddr_t(argv[n], adr); \
		} else { \
			haltest_error("No address specified\n");\
			return;\
		} \
	} while (0)
