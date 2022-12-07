// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <getopt.h>
#include <stdbool.h>
#include <wordexp.h>
#include <ctype.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "src/uuid-helper.h"
#include "lib/mgmt.h"

#include "src/shared/mainloop.h"
#include "src/shared/io.h"
#include "src/shared/util.h"
#include "src/shared/mgmt.h"
#include "src/shared/shell.h"

#define SCAN_TYPE_BREDR (1 << BDADDR_BREDR)
#define SCAN_TYPE_LE ((1 << BDADDR_LE_PUBLIC) | (1 << BDADDR_LE_RANDOM))
#define SCAN_TYPE_DUAL (SCAN_TYPE_BREDR | SCAN_TYPE_LE)

static struct mgmt *mgmt = NULL;
static uint16_t mgmt_index = MGMT_INDEX_NONE;

static bool discovery = false;
static bool resolve_names = true;

static struct {
	uint16_t index;
	uint16_t req;
	struct mgmt_addr_info addr;
} prompt = {
	.index = MGMT_INDEX_NONE,
};


static int pending_index = 0;

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define PROMPT_ON	COLOR_BLUE "[mgmt]" COLOR_OFF "# "

static void set_index(const char *arg)
{
	if (!arg || !strcmp(arg, "none") || !strcmp(arg, "any") ||
						!strcmp(arg, "all"))
		mgmt_index = MGMT_INDEX_NONE;
	else if(strlen(arg) > 3 && !strncasecmp(arg, "hci", 3))
		mgmt_index = atoi(&arg[3]);
	else
		mgmt_index = atoi(arg);
}

static bool parse_setting(int argc, char **argv, uint8_t *val)
{
	if (strcasecmp(argv[1], "on") == 0 || strcasecmp(argv[1], "yes") == 0)
		*val = 1;
	else if (strcasecmp(argv[1], "off") == 0)
		*val = 0;
	else
		*val = atoi(argv[1]);
	return true;
}

static void update_prompt(uint16_t index)
{
	char str[32];

	if (index == MGMT_INDEX_NONE)
		snprintf(str, sizeof(str), "%s# ",
					COLOR_BLUE "[mgmt]" COLOR_OFF);
	else
		snprintf(str, sizeof(str),
				COLOR_BLUE "[hci%u]" COLOR_OFF "# ", index);

	bt_shell_set_prompt(str);
}

#define print(fmt, arg...) do { \
		bt_shell_printf(fmt "\n", ## arg); \
} while (0)

#define error(fmt, arg...) do { \
		bt_shell_printf(COLOR_RED fmt "\n" COLOR_OFF, ## arg); \
} while (0)

static size_t hex2bin(const char *hexstr, uint8_t *buf, size_t buflen)
{
	size_t i, len;

	len = MIN((strlen(hexstr) / 2), buflen);
	memset(buf, 0, len);

	for (i = 0; i < len; i++)
		sscanf(hexstr + (i * 2), "%02hhX", &buf[i]);

	return len;
}

static size_t bin2hex(const uint8_t *buf, size_t buflen, char *str,
								size_t strlen)
{
	size_t i;

	for (i = 0; i < buflen && i < (strlen / 2); i++)
		sprintf(str + (i * 2), "%02x", buf[i]);

	return i;
}

static void print_eir(const uint8_t *eir, uint16_t eir_len)
{
	uint16_t parsed = 0;
	char str[33];

	while (parsed < eir_len - 1) {
		uint8_t field_len = eir[0];

		if (field_len == 0)
			break;

		parsed += field_len + 1;

		if (parsed > eir_len)
			break;

		switch (eir[1]) {
		case 0x01:
			print("Flags: 0x%02x", eir[2]);
			break;
		case 0x0d:
			print("Class of Device: 0x%02x%02x%02x",
						eir[4], eir[3], eir[2]);
			break;
		case 0x0e:
			bin2hex(eir + 2, 16, str, sizeof(str));
			print("SSP Hash C-192: %s", str);
			break;
		case 0x0f:
			bin2hex(eir + 2, 16, str, sizeof(str));
			print("SSP Rand R-192: %s", str);
			break;
		case 0x1b:
			ba2str((bdaddr_t *) (eir + 2), str);
			print("LE Device Address: %s (%s)", str,
					eir[8] ? "random" : "public");
			break;
		case 0x1c:
			print("LE Role: 0x%02x", eir[2]);
			break;
		case 0x1d:
			bin2hex(eir + 2, 16, str, sizeof(str));
			print("SSP Hash C-256: %s", str);
			break;
		case 0x1e:
			bin2hex(eir + 2, 16, str, sizeof(str));
			print("SSP Rand R-256: %s", str);
			break;
		case 0x22:
			bin2hex(eir + 2, 16, str, sizeof(str));
			print("LE SC Confirmation Value: %s", str);
			break;
		case 0x23:
			bin2hex(eir + 2, 16, str, sizeof(str));
			print("LE SC Random Value: %s", str);
			break;
		default:
			print("Type %u: %u byte%s", eir[1], field_len - 1,
					(field_len - 1) == 1 ? "" : "s");
			break;
		}

		eir += field_len + 1;
	}
}

static bool load_identity(const char *path, struct mgmt_irk_info *irk)
{
	char *addr, *key;
	unsigned int type;
	int n;
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp) {
		error("Failed to open identity file: %s", strerror(errno));
		return false;
	}

	n = fscanf(fp, "%m[0-9a-f:] (type %u) %m[0-9a-f]", &addr, &type, &key);

	fclose(fp);

	if (n != 3)
		return false;

	str2ba(addr, &irk->addr.bdaddr);
	hex2bin(key, irk->val, sizeof(irk->val));

	free(addr);
	free(key);

	switch (type) {
	case 0:
		irk->addr.type = BDADDR_LE_PUBLIC;
		break;
	case 1:
		irk->addr.type = BDADDR_LE_RANDOM;
		break;
	default:
		error("Invalid address type %u", type);
		return false;
	}

	return true;
}

static void controller_error(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	const struct mgmt_ev_controller_error *ev = param;

	if (len < sizeof(*ev)) {
		error("Too short (%u bytes) controller error event", len);
		return;
	}

	print("hci%u error 0x%02x", index, ev->error_code);
}

static void index_added(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	print("hci%u added", index);
}

static void index_removed(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	print("hci%u removed", index);
}

static void unconf_index_added(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	print("hci%u added (unconfigured)", index);
}

static void unconf_index_removed(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	print("hci%u removed (unconfigured)", index);
}

static void ext_index_added(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	const struct mgmt_ev_ext_index_added *ev = param;

	print("hci%u added (type %u bus %u)", index, ev->type, ev->bus);
}

static void ext_index_removed(uint16_t index, uint16_t len,
				const void *param, void *user_data)
{
	const struct mgmt_ev_ext_index_removed *ev = param;

	print("hci%u removed (type %u bus %u)", index, ev->type, ev->bus);
}

static const char *options_str[] = {
				"external",
				"public-address",
};

static const char *options2str(uint32_t options)
{
	static char str[256];
	unsigned i;
	int off;

	off = 0;
	str[0] = '\0';

	for (i = 0; i < NELEM(options_str); i++) {
		if ((options & (1 << i)) != 0)
			off += snprintf(str + off, sizeof(str) - off, "%s ",
							options_str[i]);
	}

	return str;
}

static void new_config_options(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	const uint32_t *ev = param;

	if (len < sizeof(*ev)) {
		error("Too short new_config_options event (%u)", len);
		return;
	}

	print("hci%u new_config_options: %s", index, options2str(get_le32(ev)));
}

static const char *settings_str[] = {
				"powered",
				"connectable",
				"fast-connectable",
				"discoverable",
				"bondable",
				"link-security",
				"ssp",
				"br/edr",
				"hs",
				"le",
				"advertising",
				"secure-conn",
				"debug-keys",
				"privacy",
				"configuration",
				"static-addr",
				"phy-configuration",
				"wide-band-speech",
};

static const char *settings2str(uint32_t settings)
{
	static char str[256];
	unsigned i;
	int off;

	off = 0;
	str[0] = '\0';

	for (i = 0; i < NELEM(settings_str); i++) {
		if ((settings & (1 << i)) != 0)
			off += snprintf(str + off, sizeof(str) - off, "%s ",
							settings_str[i]);
	}

	return str;
}

static void new_settings(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	const uint32_t *ev = param;

	if (len < sizeof(*ev)) {
		error("Too short new_settings event (%u)", len);
		return;
	}

	print("hci%u new_settings: %s", index, settings2str(get_le32(ev)));
}

static void discovering(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_discovering *ev = param;

	if (len < sizeof(*ev)) {
		error("Too short (%u bytes) discovering event", len);
		return;
	}

	print("hci%u type %u discovering %s", index, ev->type,
					ev->discovering ? "on" : "off");

	if (ev->discovering == 0 && discovery)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void new_link_key(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_new_link_key *ev = param;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid new_link_key length (%u bytes)", len);
		return;
	}

	ba2str(&ev->key.addr.bdaddr, addr);
	print("hci%u new_link_key %s type 0x%02x pin_len %d store_hint %u",
		index, addr, ev->key.type, ev->key.pin_len, ev->store_hint);
}

static const char *typestr(uint8_t type)
{
	static const char *str[] = { "BR/EDR", "LE Public", "LE Random" };

	if (type <= BDADDR_LE_RANDOM)
		return str[type];

	return "(unknown)";
}

static void connected(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_device_connected *ev = param;
	uint16_t eir_len;
	char addr[18];

	if (len < sizeof(*ev)) {
		error("Invalid connected event length (%u bytes)", len);
		return;
	}

	eir_len = get_le16(&ev->eir_len);
	if (len != sizeof(*ev) + eir_len) {
		error("Invalid connected event length (%u != eir_len %u)",
								len, eir_len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s type %s connected eir_len %u", index, addr,
					typestr(ev->addr.type), eir_len);
}

static void disconnected(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_device_disconnected *ev = param;
	char addr[18];
	uint8_t reason;

	if (len < sizeof(struct mgmt_addr_info)) {
		error("Invalid disconnected event length (%u bytes)", len);
		return;
	}

	if (len < sizeof(*ev))
		reason = MGMT_DEV_DISCONN_UNKNOWN;
	else
		reason = ev->reason;

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s type %s disconnected with reason %u",
			index, addr, typestr(ev->addr.type), reason);
}

static void conn_failed(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_connect_failed *ev = param;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid connect_failed event length (%u bytes)", len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s type %s connect failed (status 0x%02x, %s)",
			index, addr, typestr(ev->addr.type), ev->status,
			mgmt_errstr(ev->status));
}

static void auth_failed(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_auth_failed *ev = param;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid auth_failed event length (%u bytes)", len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s auth failed with status 0x%02x (%s)",
			index, addr, ev->status, mgmt_errstr(ev->status));
}

static void class_of_dev_changed(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_ev_class_of_dev_changed *ev = param;

	if (len != sizeof(*ev)) {
		error("Invalid class_of_dev_changed length (%u bytes)", len);
		return;
	}

	print("hci%u class of device changed: 0x%02x%02x%02x", index,
			ev->dev_class[2], ev->dev_class[1], ev->dev_class[0]);
}

static void local_name_changed(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_local_name_changed *ev = param;

	if (len != sizeof(*ev)) {
		error("Invalid local_name_changed length (%u bytes)", len);
		return;
	}

	print("hci%u name changed: %s", index, ev->name);
}

static void confirm_name_rsp(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_confirm_name *rp = param;
	char addr[18];

	if (len == 0 && status != 0) {
		error("confirm_name failed with status 0x%02x (%s)", status,
							mgmt_errstr(status));
		return;
	}

	if (len != sizeof(*rp)) {
		error("confirm_name rsp length %u instead of %zu",
							len, sizeof(*rp));
		return;
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status != 0)
		error("confirm_name for %s failed: 0x%02x (%s)",
			addr, status, mgmt_errstr(status));
	else
		print("confirm_name succeeded for %s", addr);
}

static char *eir_get_name(const uint8_t *eir, uint16_t eir_len)
{
	uint8_t parsed = 0;

	if (eir_len < 2)
		return NULL;

	while (parsed < eir_len - 1) {
		uint8_t field_len = eir[0];

		if (field_len == 0)
			break;

		parsed += field_len + 1;

		if (parsed > eir_len)
			break;

		/* Check for short of complete name */
		if (eir[1] == 0x09 || eir[1] == 0x08)
			return strndup((char *) &eir[2], field_len - 1);

		eir += field_len + 1;
	}

	return NULL;
}

static unsigned int eir_get_flags(const uint8_t *eir, uint16_t eir_len)
{
	uint8_t parsed = 0;

	if (eir_len < 2)
		return 0;

	while (parsed < eir_len - 1) {
		uint8_t field_len = eir[0];

		if (field_len == 0)
			break;

		parsed += field_len + 1;

		if (parsed > eir_len)
			break;

		/* Check for flags */
		if (eir[1] == 0x01)
			return eir[2];

		eir += field_len + 1;
	}

	return 0;
}

static void device_found(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_device_found *ev = param;
	struct mgmt *mgmt = user_data;
	uint16_t eir_len;
	uint32_t flags;

	if (len < sizeof(*ev)) {
		error("Too short device_found length (%u bytes)", len);
		return;
	}

	flags = btohl(ev->flags);

	eir_len = get_le16(&ev->eir_len);
	if (len != sizeof(*ev) + eir_len) {
		error("dev_found: expected %zu bytes, got %u bytes",
						sizeof(*ev) + eir_len, len);
		return;
	}

	if (discovery) {
		char addr[18], *name;

		ba2str(&ev->addr.bdaddr, addr);
		print("hci%u dev_found: %s type %s rssi %d "
			"flags 0x%04x ", index, addr,
			typestr(ev->addr.type), ev->rssi, flags);

		if (ev->addr.type != BDADDR_BREDR)
			print("AD flags 0x%02x ",
					eir_get_flags(ev->eir, eir_len));

		name = eir_get_name(ev->eir, eir_len);
		if (name)
			print("name %s", name);
		else
			print("eir_len %u", eir_len);

		free(name);
	}

	if (discovery && (flags & MGMT_DEV_FOUND_CONFIRM_NAME)) {
		struct mgmt_cp_confirm_name cp;

		memset(&cp, 0, sizeof(cp));
		memcpy(&cp.addr, &ev->addr, sizeof(cp.addr));
		if (resolve_names)
			cp.name_known = 0;
		else
			cp.name_known = 1;

		mgmt_reply(mgmt, MGMT_OP_CONFIRM_NAME, index, sizeof(cp), &cp,
						confirm_name_rsp, NULL, NULL);
	}
}

static void pin_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("PIN Code reply failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("PIN Reply successful");
}

static int mgmt_pin_reply(uint16_t index, const struct mgmt_addr_info *addr,
					const char *pin, size_t len)
{
	struct mgmt_cp_pin_code_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, addr, sizeof(cp.addr));
	cp.pin_len = len;
	memcpy(cp.pin_code, pin, len);

	return mgmt_reply(mgmt, MGMT_OP_PIN_CODE_REPLY, index,
				sizeof(cp), &cp, pin_rsp, NULL, NULL);
}

static void pin_neg_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("PIN Neg reply failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("PIN Negative Reply successful");
}

static int mgmt_pin_neg_reply(uint16_t index, const struct mgmt_addr_info *addr)
{
	struct mgmt_cp_pin_code_neg_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, addr, sizeof(cp.addr));

	return mgmt_reply(mgmt, MGMT_OP_PIN_CODE_NEG_REPLY, index,
				sizeof(cp), &cp, pin_neg_rsp, NULL, NULL);
}

static void confirm_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("User Confirm reply failed. status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("User Confirm Reply successful");
}

static int mgmt_confirm_reply(uint16_t index, const struct mgmt_addr_info *addr)
{
	struct mgmt_cp_user_confirm_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, addr, sizeof(*addr));

	return mgmt_reply(mgmt, MGMT_OP_USER_CONFIRM_REPLY, index,
				sizeof(cp), &cp, confirm_rsp, NULL, NULL);
}

static void confirm_neg_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("Confirm Neg reply failed. status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("User Confirm Negative Reply successful");
}

static int mgmt_confirm_neg_reply(uint16_t index,
					const struct mgmt_addr_info *addr)
{
	struct mgmt_cp_user_confirm_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, addr, sizeof(*addr));

	return mgmt_reply(mgmt, MGMT_OP_USER_CONFIRM_NEG_REPLY, index,
				sizeof(cp), &cp, confirm_neg_rsp, NULL, NULL);
}

static void passkey_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("User Passkey reply failed. status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("User Passkey Reply successful");
}

static int mgmt_passkey_reply(uint16_t index, const struct mgmt_addr_info *addr,
						uint32_t passkey)
{
	struct mgmt_cp_user_passkey_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, addr, sizeof(*addr));
	put_le32(passkey, &cp.passkey);

	return mgmt_reply(mgmt, MGMT_OP_USER_PASSKEY_REPLY, index,
				sizeof(cp), &cp, passkey_rsp, NULL, NULL);
}

static void passkey_neg_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("Passkey Neg reply failed. status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("User Passkey Negative Reply successful");
}

static int mgmt_passkey_neg_reply(uint16_t index,
					const struct mgmt_addr_info *addr)
{
	struct mgmt_cp_user_passkey_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, addr, sizeof(*addr));

	return mgmt_reply(mgmt, MGMT_OP_USER_PASSKEY_NEG_REPLY, index,
				sizeof(cp), &cp, passkey_neg_rsp, NULL, NULL);
}

static void prompt_input(const char *input, void *user_data)
{
	size_t len;

	len = strlen(input);

	switch (prompt.req) {
	case MGMT_EV_PIN_CODE_REQUEST:
		if (len)
			mgmt_pin_reply(prompt.index, &prompt.addr, input, len);
		else
			mgmt_pin_neg_reply(prompt.index, &prompt.addr);
		break;
	case MGMT_EV_USER_PASSKEY_REQUEST:
		if (strlen(input) > 0)
			mgmt_passkey_reply(prompt.index, &prompt.addr,
								atoi(input));
		else
			mgmt_passkey_neg_reply(prompt.index,
								&prompt.addr);
		break;
	case MGMT_EV_USER_CONFIRM_REQUEST:
		if (input[0] == 'y' || input[0] == 'Y')
			mgmt_confirm_reply(prompt.index, &prompt.addr);
		else
			mgmt_confirm_neg_reply(prompt.index, &prompt.addr);
		break;
	}
}

static void ask(uint16_t index, uint16_t req, const struct mgmt_addr_info *addr,
						const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	int off;

	prompt.index = index;
	prompt.req = req;
	memcpy(&prompt.addr, addr, sizeof(*addr));

	va_start(ap, fmt);
	off = vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	snprintf(msg + off, sizeof(msg) - off, " %s ",
					COLOR_BOLDGRAY ">>" COLOR_OFF);

	bt_shell_prompt_input("", msg, prompt_input, NULL);
}

static void request_pin(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_pin_code_request *ev = param;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid pin_code request length (%u bytes)", len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s request PIN", index, addr);

	ask(index, MGMT_EV_PIN_CODE_REQUEST, &ev->addr,
				"PIN Request (press enter to reject)");
}

static void user_confirm(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_user_confirm_request *ev = param;
	uint32_t val;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid user_confirm request length (%u)", len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	val = get_le32(&ev->value);

	print("hci%u %s User Confirm %06u hint %u", index, addr,
							val, ev->confirm_hint);

	if (ev->confirm_hint)
		ask(index, MGMT_EV_USER_CONFIRM_REQUEST, &ev->addr,
				"Accept pairing with %s (yes/no)", addr);
	else
		ask(index, MGMT_EV_USER_CONFIRM_REQUEST, &ev->addr,
			"Confirm value %06u for %s (yes/no)", val, addr);
}

static void request_passkey(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_user_passkey_request *ev = param;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid passkey request length (%u bytes)", len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s request passkey", index, addr);

	ask(index, MGMT_EV_USER_PASSKEY_REQUEST, &ev->addr,
			"Passkey Request (press enter to reject)");
}

static void passkey_notify(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_passkey_notify *ev = param;
	char addr[18];

	if (len != sizeof(*ev)) {
		error("Invalid passkey request length (%u bytes)", len);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	print("hci%u %s request passkey", index, addr);

	print("Passkey Notify: %06u (entered %u)", get_le32(&ev->passkey),
								ev->entered);
}

static void local_oob_data_updated(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_ev_local_oob_data_updated *ev = param;
	uint16_t eir_len;

	if (len < sizeof(*ev)) {
		error("Too small (%u bytes) local_oob_updated event", len);
		return;
	}

	eir_len = le16_to_cpu(ev->eir_len);
	if (len != sizeof(*ev) + eir_len) {
		error("local_oob_updated: expected %zu bytes, got %u bytes",
						sizeof(*ev) + eir_len, len);
		return;
	}

	print("hci%u oob data updated: type %u len %u", index,
						ev->type, eir_len);
}

static void advertising_added(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_ev_advertising_added *ev = param;

	if (len < sizeof(*ev)) {
		error("Too small (%u bytes) advertising_added event", len);
		return;
	}

	print("hci%u advertising_added: instance %u", index, ev->instance);
}

static void advertising_removed(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_ev_advertising_removed *ev = param;

	if (len < sizeof(*ev)) {
		error("Too small (%u bytes) advertising_removed event", len);
		return;
	}

	print("hci%u advertising_removed: instance %u", index, ev->instance);
}

static void advmon_added(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_adv_monitor_added *ev = param;

	if (len < sizeof(*ev)) {
		error("Too small (%u bytes) %s event", len, __func__);
		return;
	}

	print("hci%u %s: handle %u", index, __func__,
					le16_to_cpu(ev->monitor_handle));
}

static void advmon_removed(uint16_t index, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_ev_adv_monitor_removed *ev = param;

	if (len < sizeof(*ev)) {
		error("Too small (%u bytes) %s event", len, __func__);
		return;
	}

	print("hci%u %s: handle %u", index, __func__,
					le16_to_cpu(ev->monitor_handle));
}

static void version_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_version *rp = param;

	if (status != 0) {
		error("Reading mgmt version failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small version reply (%u bytes)", len);
		goto done;
	}

	print("MGMT Version %u, revision %u", rp->version,
						get_le16(&rp->revision));

done:
	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_revision(int argc, char **argv)
{
	if (mgmt_send(mgmt, MGMT_OP_READ_VERSION, MGMT_INDEX_NONE,
				0, NULL, version_rsp, NULL, NULL) == 0) {
		error("Unable to send read_version cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void commands_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_commands *rp = param;
	uint16_t num_commands, num_events;
	size_t expected_len;
	int i;

	if (status != 0) {
		error("Read Supported Commands failed: status 0x%02x (%s)",
						status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small commands reply (%u bytes)", len);
		goto done;
	}

	num_commands = get_le16(&rp->num_commands);
	num_events = get_le16(&rp->num_events);

	expected_len = sizeof(*rp) + num_commands * sizeof(uint16_t) +
						num_events * sizeof(uint16_t);

	if (len < expected_len) {
		error("Too small commands reply (%u != %zu)",
							len, expected_len);
		goto done;
	}

	print("%u commands:", num_commands);
	for (i = 0; i < num_commands; i++) {
		uint16_t op = get_le16(rp->opcodes + i);
		print("\t%s (0x%04x)", mgmt_opstr(op), op);
	}

	print("%u events:", num_events);
	for (i = 0; i < num_events; i++) {
		uint16_t ev = get_le16(rp->opcodes + num_commands + i);
		print("\t%s (0x%04x)", mgmt_evstr(ev), ev);
	}

done:
	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_commands(int argc,
								char **argv)
{
	if (mgmt_send(mgmt, MGMT_OP_READ_COMMANDS, MGMT_INDEX_NONE,
				0, NULL, commands_rsp, NULL, NULL) == 0) {
		error("Unable to send read_commands cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void config_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_config_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t supported_options, missing_options;

	if (status != 0) {
		error("Reading hci%u config failed with status 0x%02x (%s)",
					index, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small info reply (%u bytes)", len);
		goto done;
	}

	print("hci%u:\tUnconfigured controller", index);

	print("\tmanufacturer %u", le16_to_cpu(rp->manufacturer));

	supported_options = le32_to_cpu(rp->supported_options);
	print("\tsupported options: %s", options2str(supported_options));

	missing_options = le32_to_cpu(rp->missing_options);
	print("\tmissing options: %s", options2str(missing_options));

done:
	pending_index--;

	if (pending_index > 0)
		return;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void unconf_index_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_unconf_index_list *rp = param;
	uint16_t count;
	unsigned int i;

	if (status != 0) {
		error("Reading index list failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small index list reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = le16_to_cpu(rp->num_controllers);

	if (len < sizeof(*rp) + count * sizeof(uint16_t)) {
		error("Index count (%u) doesn't match reply length (%u)",
								count, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Unconfigured index list with %u item%s",
					count, count != 1 ? "s" : "");

	for (i = 0; i < count; i++) {
		uint16_t index = le16_to_cpu(rp->index[i]);

		if (!mgmt_send(mgmt, MGMT_OP_READ_CONFIG_INFO, index, 0, NULL,
				config_info_rsp, UINT_TO_PTR(index), NULL)) {
			error("Unable to send read_config_info cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		pending_index++;
	}

	if (!count)
		bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_config(int argc, char **argv)
{
	if (mgmt_index == MGMT_INDEX_NONE) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_UNCONF_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					unconf_index_rsp, mgmt, NULL)) {
			error("Unable to send unconf_index_list cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		return;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_CONFIG_INFO, mgmt_index, 0, NULL,
				config_info_rsp, UINT_TO_PTR(index), NULL)) {
		error("Unable to send read_config_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void config_options_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_config_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t supported_options, missing_options;

	if (status != 0) {
		error("Reading hci%u config failed with status 0x%02x (%s)",
					index, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small info reply (%u bytes)", len);
		goto done;
	}

	print("hci%u:\tConfiguration options", index);

	supported_options = le32_to_cpu(rp->supported_options);
	print("\tsupported options: %s", options2str(supported_options));

	missing_options = le32_to_cpu(rp->missing_options);
	print("\tmissing options: %s", options2str(missing_options));

done:
	pending_index--;

	if (pending_index > 0)
		return;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t supported_settings, current_settings;
	char addr[18];

	if (status != 0) {
		error("Reading hci%u info failed with status 0x%02x (%s)",
					index, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small info reply (%u bytes)", len);
		goto done;
	}

	print("hci%u:\tPrimary controller", index);

	ba2str(&rp->bdaddr, addr);
	print("\taddr %s version %u manufacturer %u class 0x%02x%02x%02x",
			addr, rp->version, le16_to_cpu(rp->manufacturer),
			rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);

	supported_settings = le32_to_cpu(rp->supported_settings);
	print("\tsupported settings: %s", settings2str(supported_settings));

	current_settings = le32_to_cpu(rp->current_settings);
	print("\tcurrent settings: %s", settings2str(current_settings));

	print("\tname %s", rp->name);
	print("\tshort name %s", rp->short_name);

	if (supported_settings & MGMT_SETTING_CONFIGURATION) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_CONFIG_INFO,
					index, 0, NULL, config_options_rsp,
					UINT_TO_PTR(index), NULL)) {
			error("Unable to send read_config cmd");
			goto done;
		}
		return;
	}

done:
	pending_index--;

	if (pending_index > 0)
		return;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void ext_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_ext_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t supported_settings, current_settings;
	char addr[18];

	if (status != 0) {
		error("Reading hci%u info failed with status 0x%02x (%s)",
					index, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small info reply (%u bytes)", len);
		goto done;
	}

	print("hci%u:\tPrimary controller", index);

	ba2str(&rp->bdaddr, addr);
	print("\taddr %s version %u manufacturer %u",
			addr, rp->version, le16_to_cpu(rp->manufacturer));

	supported_settings = le32_to_cpu(rp->supported_settings);
	print("\tsupported settings: %s", settings2str(supported_settings));

	current_settings = le32_to_cpu(rp->current_settings);
	print("\tcurrent settings: %s", settings2str(current_settings));

	if (supported_settings & MGMT_SETTING_CONFIGURATION) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_CONFIG_INFO,
					index, 0, NULL, config_options_rsp,
					UINT_TO_PTR(index), NULL)) {
			error("Unable to send read_config cmd");
			goto done;
		}
		return;
	}

done:
	pending_index--;

	if (pending_index > 0)
		return;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void index_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	struct mgmt *mgmt = user_data;
	uint16_t count;
	unsigned int i;

	if (status != 0) {
		error("Reading index list failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small index list reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = le16_to_cpu(rp->num_controllers);

	if (len < sizeof(*rp) + count * sizeof(uint16_t)) {
		error("Index count (%u) doesn't match reply length (%u)",
								count, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Index list with %u item%s", count, count != 1 ? "s" : "");

	for (i = 0; i < count; i++) {
		uint16_t index = le16_to_cpu(rp->index[i]);

		if (!mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
					info_rsp, UINT_TO_PTR(index), NULL)) {
			error("Unable to send read_info cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		pending_index++;
	}

	if (!count)
		bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_info(int argc, char **argv)
{
	if (mgmt_index == MGMT_INDEX_NONE) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					index_rsp, mgmt, NULL)) {
			error("Unable to send index_list cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		return;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_INFO, mgmt_index, 0, NULL, info_rsp,
					UINT_TO_PTR(mgmt_index), NULL)) {
		error("Unable to send read_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void ext_index_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_ext_index_list *rp = param;
	uint16_t count;
	unsigned int i;

	if (status != 0) {
		error("Reading ext index list failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small ext index list reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = get_le16(&rp->num_controllers);

	if (len < sizeof(*rp) + count * (sizeof(uint16_t) + sizeof(uint8_t))) {
		error("Index count (%u) doesn't match reply length (%u)",
								count, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Extended index list with %u item%s",
					count, count != 1 ? "s" : "");

	for (i = 0; i < count; i++) {
		uint16_t index = le16_to_cpu(rp->entry[i].index);
		char *busstr = hci_bustostr(rp->entry[i].bus);

		switch (rp->entry[i].type) {
		case 0x00:
			print("Primary controller (hci%u,%s)", index, busstr);
			if (!mgmt_send(mgmt, MGMT_OP_READ_EXT_INFO,
						index, 0, NULL, ext_info_rsp,
						UINT_TO_PTR(index), NULL)) {
				error("Unable to send read_ext_info cmd");
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			pending_index++;
			break;
		case 0x01:
			print("Unconfigured controller (hci%u,%s)",
								index, busstr);
			if (!mgmt_send(mgmt, MGMT_OP_READ_CONFIG_INFO,
						index, 0, NULL, config_info_rsp,
						UINT_TO_PTR(index), NULL)) {
				error("Unable to send read_config cmd");
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			pending_index++;
			break;
		case 0x02:
			print("AMP controller (hci%u,%s)", index, busstr);
			break;
		default:
			print("Type %u controller (hci%u,%s)",
					rp->entry[i].type, index, busstr);
			break;
		}
	}

	print("");

	if (!count)
		bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_extinfo(int argc, char **argv)
{
	if (mgmt_index == MGMT_INDEX_NONE) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_EXT_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					ext_index_rsp, mgmt, NULL)) {
			error("Unable to send ext_index_list cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		return;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_EXT_INFO, mgmt_index, 0, NULL,
					ext_info_rsp,
					UINT_TO_PTR(mgmt_index), NULL)) {
		error("Unable to send ext_read_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void sec_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_controller_cap *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);

	if (status != 0) {
		error("Reading hci%u security failed with status 0x%02x (%s)",
					index, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small info reply (%u bytes)", len);
		goto done;
	}

	print("Primary controller (hci%u)", index);
	print("\tSecurity info length: %u", le16_to_cpu(rp->cap_len));

done:
	pending_index--;

	if (pending_index > 0)
		return;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void sec_index_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_ext_index_list *rp = param;
	uint16_t count;
	unsigned int i;

	if (status != 0) {
		error("Reading ext index list failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small ext index list reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = get_le16(&rp->num_controllers);

	if (len < sizeof(*rp) + count * (sizeof(uint16_t) + sizeof(uint8_t))) {
		error("Index count (%u) doesn't match reply length (%u)",
								count, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	for (i = 0; i < count; i++) {
		uint16_t index = le16_to_cpu(rp->entry[i].index);

		if (rp->entry[i].type != 0x00)
			continue;

		if (!mgmt_send(mgmt, MGMT_OP_READ_CONTROLLER_CAP,
						index, 0, NULL, sec_info_rsp,
						UINT_TO_PTR(index), NULL)) {
			error("Unable to send read_security_info cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
		pending_index++;
	}

	if (!count)
		bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_secinfo(int argc, char **argv)
{
	if (mgmt_index == MGMT_INDEX_NONE) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_EXT_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					sec_index_rsp, mgmt, NULL)) {
			error("Unable to send ext_index_list cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		return;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_CONTROLLER_CAP, mgmt_index, 0, NULL,
					sec_info_rsp,
					UINT_TO_PTR(mgmt_index), NULL)) {
		error("Unable to send read_security_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void exp_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_exp_features_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);

	if (status != 0) {
		error("Reading hci%u exp features failed with status 0x%02x (%s)",
					index, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small info reply (%u bytes)", len);
		goto done;
	}

	if (index == MGMT_INDEX_NONE)
		print("Global");
	else
		print("Primary controller (hci%u)", index);

	print("\tNumber of experimental features: %u",
					le16_to_cpu(rp->feature_count));

done:
	pending_index--;

	if (pending_index > 0)
		return;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void exp_index_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_ext_index_list *rp = param;
	uint16_t count;
	unsigned int i;

	if (status != 0) {
		error("Reading ext index list failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small ext index list reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = get_le16(&rp->num_controllers);

	if (len < sizeof(*rp) + count * (sizeof(uint16_t) + sizeof(uint8_t))) {
		error("Index count (%u) doesn't match reply length (%u)",
								count, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	for (i = 0; i < count; i++) {
		uint16_t index = le16_to_cpu(rp->entry[i].index);

		if (rp->entry[i].type != 0x00)
			continue;

		if (!mgmt_send(mgmt, MGMT_OP_READ_EXP_FEATURES_INFO,
						index, 0, NULL, exp_info_rsp,
						UINT_TO_PTR(index), NULL)) {
				error("Unable to send read_exp_features_info cmd");
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
		pending_index++;
	}
}

static void cmd_expinfo(int argc, char **argv)
{
	if (mgmt_index == MGMT_INDEX_NONE) {
		if (!mgmt_send(mgmt, MGMT_OP_READ_EXT_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					exp_index_rsp, mgmt, NULL)) {
			error("Unable to send ext_index_list cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		if (!mgmt_send(mgmt, MGMT_OP_READ_EXP_FEATURES_INFO,
					MGMT_INDEX_NONE, 0, NULL,
					exp_info_rsp,
					UINT_TO_PTR(MGMT_INDEX_NONE), NULL)) {
			error("Unable to send read_exp_features_info cmd");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		pending_index++;
		return;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_EXP_FEATURES_INFO, mgmt_index,
					0, NULL, exp_info_rsp,
					UINT_TO_PTR(mgmt_index), NULL)) {
		error("Unable to send read_exp_features_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void exp_debug_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Set debug feature failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Debug feature successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_exp_debug(int argc, char **argv)
{
	/* d4992530-b9ec-469f-ab01-6c481c47da1c */
	static const uint8_t uuid[16] = {
				0x1c, 0xda, 0x47, 0x1c, 0x48, 0x6c, 0x01, 0xab,
				0x9f, 0x46, 0xec, 0xb9, 0x30, 0x25, 0x99, 0xd4,
	};
	struct mgmt_cp_set_exp_feature cp;
	uint8_t val;

	if (parse_setting(argc, argv, &val) == false)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	memset(&cp, 0, sizeof(cp));
	memcpy(cp.uuid, uuid, 16);
	cp.action = val;

	if (mgmt_send(mgmt, MGMT_OP_SET_EXP_FEATURE, mgmt_index,
			sizeof(cp), &cp, exp_debug_rsp, NULL, NULL) == 0) {
		error("Unable to send debug feature cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void print_mgmt_tlv(void *data, void *user_data)
{
	const struct mgmt_tlv *entry = data;
	char buf[256];

	bin2hex(entry->value, entry->length, buf, sizeof(buf));
	print("Type: 0x%04x\tLength: %02hhu\tValue: %s", entry->type,
							entry->length, buf);
}

static void read_sysconfig_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	struct mgmt_tlv_list *tlv_list;

	if (status != 0) {
		error("Read system configuration failed with status "
				"0x%02x (%s)", status, mgmt_errstr(status));
		return;
	}

	tlv_list = mgmt_tlv_list_load_from_buf(param, len);
	if (!tlv_list) {
		error("Unable to parse response of read system configuration");
		return;
	}

	mgmt_tlv_list_foreach(tlv_list, print_mgmt_tlv, NULL);
	mgmt_tlv_list_free(tlv_list);
}

static void cmd_read_sysconfig(int argc, char **argv)
{
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (!mgmt_send(mgmt, MGMT_OP_READ_DEF_SYSTEM_CONFIG, index,
			0, NULL, read_sysconfig_rsp, NULL, NULL)) {
		error("Unable to send read system configuration cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static bool parse_mgmt_tlv(const char *input, uint16_t *type, uint8_t *length,
								uint8_t *value)
{
	int i, value_starting_pos;

	if (sscanf(input, "%4hx:%1hhu:%n", type, length,
						&value_starting_pos) < 2) {
		return false;
	}

	input += value_starting_pos;

	if (*length * 2 != strlen(input))
		return false;

	for (i = 0; i < *length; i++) {
		if (sscanf(input + i * 2, "%2hhx", &value[i]) < 1)
			return false;
	}

	return true;
}

static void set_sysconfig_rsp(uint8_t status, uint16_t len, const void *param,
								void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		error("Could not set default system configuration with status "
				"0x%02x (%s)", status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Set default system configuration success");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static bool set_sysconfig(int argc, char **argv)
{
	struct mgmt_tlv_list *tlv_list = NULL;
	int i;
	uint16_t index, type;
	uint8_t length;
	uint8_t value[256] = {};
	bool success = false;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	tlv_list = mgmt_tlv_list_new();
	if (!tlv_list) {
		error("tlv_list failed to init");
		goto failed;
	}

	for (i = 0; i < argc; i++) {
		if (!parse_mgmt_tlv(argv[i], &type, &length, value)) {
			error("failed to parse");
			goto failed;
		}

		if (!mgmt_tlv_add(tlv_list, type, length, value)) {
			error("failed to add");
			goto failed;
		}
	}

	if (!mgmt_send_tlv(mgmt, MGMT_OP_SET_DEF_SYSTEM_CONFIG, index,
				tlv_list, set_sysconfig_rsp, NULL, NULL)) {
		error("Failed to send \"Set Default System Configuration\""
								" command");
		goto failed;
	}

	success = true;

failed:
	if (tlv_list)
		mgmt_tlv_list_free(tlv_list);

	return success;
}

static void set_sysconfig_usage(void)
{
	bt_shell_usage();
	print("Parameters:\n\t-v <type:length:value>...\n"
		"e.g.:\n\tset-sysconfig -v 001a:2:1234 001f:1:00");
}

static void cmd_set_sysconfig(int argc, char **argv)
{
	bool success = false;

	if (strcasecmp(argv[1], "-v") == 0 && argc > 2) {
		argc -= 2;
		argv += 2;
		success = set_sysconfig(argc, argv);
	}

	if (!success) {
		set_sysconfig_usage();
		bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void auto_power_enable_rsp(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	uint16_t index = PTR_TO_UINT(user_data);

	print("Successfully enabled controller with index %u", index);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void auto_power_info_rsp(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t supported_settings, current_settings, missing_settings;
	uint8_t val = 0x01;

	if (status) {
		error("Reading info failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	supported_settings = le32_to_cpu(rp->supported_settings);
	current_settings = le32_to_cpu(rp->current_settings);
	missing_settings = current_settings ^ supported_settings;

	if (missing_settings & MGMT_SETTING_BREDR)
		mgmt_send(mgmt, MGMT_OP_SET_BREDR, index, sizeof(val), &val,
							NULL, NULL, NULL);

	if (missing_settings & MGMT_SETTING_SSP)
		mgmt_send(mgmt, MGMT_OP_SET_SSP, index, sizeof(val), &val,
							NULL, NULL, NULL);

	if (missing_settings & MGMT_SETTING_LE)
		mgmt_send(mgmt, MGMT_OP_SET_LE, index, sizeof(val), &val,
							NULL, NULL, NULL);

	if (missing_settings & MGMT_SETTING_SECURE_CONN)
		mgmt_send(mgmt, MGMT_OP_SET_SECURE_CONN, index,
							sizeof(val), &val,
							NULL, NULL, NULL);

	if (missing_settings & MGMT_SETTING_BONDABLE)
		mgmt_send(mgmt, MGMT_OP_SET_BONDABLE, index, sizeof(val), &val,
							NULL, NULL, NULL);

	if (current_settings & MGMT_SETTING_POWERED)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	if (!mgmt_send(mgmt, MGMT_OP_SET_POWERED, index, sizeof(val), &val,
						auto_power_enable_rsp,
						UINT_TO_PTR(index), NULL)) {
		error("Unable to send set powerd cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void auto_power_index_evt(uint16_t index, uint16_t len,
					const void *param, void *user_data)
{
	uint16_t index_filter = PTR_TO_UINT(user_data);

	if (index != index_filter)
		return;

	print("New controller with index %u", index);

	if (!mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
						auto_power_info_rsp,
						UINT_TO_PTR(index), NULL)) {
		error("Unable to send read info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void auto_power_index_rsp(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint16_t i, count;
	bool found = false;

	if (status) {
		error("Reading index list failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = le16_to_cpu(rp->num_controllers);
	for (i = 0; i < count; i++) {
		if (le16_to_cpu(rp->index[i]) == index)
			found = true;
	}

	if (!found) {
		print("Waiting for index %u to appear", index);

		mgmt_register(mgmt, MGMT_EV_INDEX_ADDED, index,
						auto_power_index_evt,
						UINT_TO_PTR(index), NULL);
		return;
	}

	print("Found controller with index %u", index);

	if (!mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
						auto_power_info_rsp,
						UINT_TO_PTR(index), NULL)) {
		error("Unable to send read info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_auto_power(int argc, char **argv)
{
	int index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (!mgmt_send(mgmt, MGMT_OP_READ_INDEX_LIST, MGMT_INDEX_NONE, 0, NULL,
						auto_power_index_rsp,
						UINT_TO_PTR(index), NULL)) {
		error("Unable to send read index list cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

/* Wrapper to get the index and opcode to the response callback */
struct command_data {
	uint16_t id;
	uint16_t op;
	void (*callback) (uint16_t id, uint16_t op, uint8_t status,
					uint16_t len, const void *param);
};

static void cmd_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	struct command_data *data = user_data;

	data->callback(data->op, data->id, status, len, param);
}

static unsigned int send_cmd(struct mgmt *mgmt, uint16_t op, uint16_t id,
				uint16_t len, const void *param,
				void (*cb)(uint16_t id, uint16_t op,
						uint8_t status, uint16_t len,
						const void *param))
{
	struct command_data *data;
	unsigned int send_id;

	data = new0(struct command_data, 1);
	if (!data)
		return 0;

	data->id = id;
	data->op = op;
	data->callback = cb;

	send_id = mgmt_send(mgmt, op, id, len, param, cmd_rsp, data, free);
	if (send_id == 0)
		free(data);

	return send_id;
}

static void setting_rsp(uint16_t op, uint16_t id, uint8_t status, uint16_t len,
							const void *param)
{
	const uint32_t *rp = param;

	if (status != 0) {
		error("%s for hci%u failed with status 0x%02x (%s)",
			mgmt_opstr(op), id, status, mgmt_errstr(status));
		goto done;
	}

	if (len < sizeof(*rp)) {
		error("Too small %s response (%u bytes)",
							mgmt_opstr(op), len);
		goto done;
	}

	print("hci%u %s complete, settings: %s", id, mgmt_opstr(op),
						settings2str(get_le32(rp)));

done:
	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_setting(uint16_t op, int argc, char **argv)
{
	int index;
	uint8_t val;

	if (parse_setting(argc, argv, &val) == false)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (send_cmd(mgmt, op, index, sizeof(val), &val, setting_rsp) == 0) {
		error("Unable to send %s cmd", mgmt_opstr(op));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_power(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_POWERED, argc, argv);
}

static void cmd_discov(int argc, char **argv)
{
	struct mgmt_cp_set_discoverable cp;
	uint16_t index;

	memset(&cp, 0, sizeof(cp));

	if (strcasecmp(argv[1], "on") == 0 || strcasecmp(argv[1], "yes") == 0)
		cp.val = 1;
	else if (strcasecmp(argv[1], "off") == 0)
		cp.val = 0;
	else if (strcasecmp(argv[1], "limited") == 0)
		cp.val = 2;
	else
		cp.val = atoi(argv[1]);

	if (argc > 2)
		cp.timeout = htobs(atoi(argv[2]));

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (send_cmd(mgmt, MGMT_OP_SET_DISCOVERABLE, index, sizeof(cp), &cp,
							setting_rsp) == 0) {
		error("Unable to send set_discoverable cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_connectable(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_CONNECTABLE, argc, argv);
}

static void cmd_fast_conn(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_FAST_CONNECTABLE, argc, argv);
}

static void cmd_bondable(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_BONDABLE, argc, argv);
}

static void cmd_linksec(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_LINK_SECURITY, argc, argv);
}

static void cmd_ssp(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_SSP, argc, argv);
}

static void cmd_sc(int argc, char **argv)
{
	uint8_t val;
	uint16_t index;

	if (strcasecmp(argv[1], "on") == 0 || strcasecmp(argv[1], "yes") == 0)
		val = 1;
	else if (strcasecmp(argv[1], "off") == 0)
		val = 0;
	else if (strcasecmp(argv[1], "only") == 0)
		val = 2;
	else
		val = atoi(argv[1]);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (send_cmd(mgmt, MGMT_OP_SET_SECURE_CONN, index,
					sizeof(val), &val, setting_rsp) == 0) {
		error("Unable to send set_secure_conn cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_hs(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_HS, argc, argv);
}

static void cmd_le(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_LE, argc, argv);
}

static void cmd_advertising(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_ADVERTISING, argc, argv);
}

static void cmd_bredr(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_BREDR, argc, argv);
}

static void cmd_privacy(int argc, char **argv)
{
	struct mgmt_cp_set_privacy cp;
	uint16_t index;

	if (parse_setting(argc, argv, &cp.privacy) == false)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (argc > 2) {
		if (hex2bin(argv[2], cp.irk,
					sizeof(cp.irk)) != sizeof(cp.irk)) {
			error("Invalid key format");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	} else {
		int fd;

		fd = open("/dev/urandom", O_RDONLY);
		if (fd < 0) {
			error("open(/dev/urandom): %s", strerror(errno));
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		if (read(fd, cp.irk, sizeof(cp.irk)) != sizeof(cp.irk)) {
			error("Reading from urandom failed");
			close(fd);
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		close(fd);
	}

	if (send_cmd(mgmt, MGMT_OP_SET_PRIVACY, index, sizeof(cp), &cp,
							setting_rsp) == 0) {
		error("Unable to send Set Privacy command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void class_rsp(uint16_t op, uint16_t id, uint8_t status, uint16_t len,
							const void *param)
{
	const struct mgmt_ev_class_of_dev_changed *rp = param;

	if (len == 0 && status != 0) {
		error("%s failed, status 0x%02x (%s)",
				mgmt_opstr(op), status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Unexpected %s len %u", mgmt_opstr(op), len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("%s succeeded. Class 0x%02x%02x%02x", mgmt_opstr(op),
		rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_class(int argc, char **argv)
{
	uint8_t class[2];
	uint16_t index;

	class[0] = atoi(argv[1]);
	class[1] = atoi(argv[2]);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (send_cmd(mgmt, MGMT_OP_SET_DEV_CLASS, index, sizeof(class), class,
							class_rsp) == 0) {
		error("Unable to send set_dev_class cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void disconnect_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_disconnect *rp = param;
	char addr[18];

	if (len == 0 && status != 0) {
		error("Disconnect failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Invalid disconnect response length (%u)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status == 0)
		print("%s disconnected", addr);
	else
		error("Disconnecting %s failed with status 0x%02x (%s)",
				addr, status, mgmt_errstr(status));

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option disconnect_options[] = {
	{ "help",	0, 0, 'h' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_disconnect(int argc, char **argv)
{
	struct mgmt_cp_disconnect cp;
	uint8_t type = BDADDR_BREDR;
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+t:h", disconnect_options,
								NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;

	if (mgmt_send(mgmt, MGMT_OP_DISCONNECT, index, sizeof(cp), &cp,
					disconnect_rsp, NULL, NULL) == 0) {
		error("Unable to send disconnect cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void con_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_get_connections *rp = param;
	uint16_t count, i;

	if (len < sizeof(*rp)) {
		error("Too small (%u bytes) get_connections rsp", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	count = get_le16(&rp->conn_count);
	if (len != sizeof(*rp) + count * sizeof(struct mgmt_addr_info)) {
		error("Invalid get_connections length (count=%u, len=%u)",
								count, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	for (i = 0; i < count; i++) {
		char addr[18];

		ba2str(&rp->addr[i].bdaddr, addr);

		print("%s type %s", addr, typestr(rp->addr[i].type));
	}

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_con(int argc, char **argv)
{
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (mgmt_send(mgmt, MGMT_OP_GET_CONNECTIONS, index, 0, NULL,
						con_rsp, NULL, NULL) == 0) {
		error("Unable to send get_connections cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void find_service_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("Start Service Discovery failed: status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Service discovery started");
	discovery = true;
}

static struct option find_service_options[] = {
	{ "help",	no_argument, 0, 'h' },
	{ "le-only",	no_argument, 0, 'l' },
	{ "bredr-only",	no_argument, 0, 'b' },
	{ "uuid",	required_argument, 0, 'u' },
	{ "rssi",	required_argument, 0, 'r' },
	{ 0, 0, 0, 0 }
};

static void uuid_to_uuid128(uuid_t *uuid128, const uuid_t *uuid)
{
	if (uuid->type == SDP_UUID16)
		sdp_uuid16_to_uuid128(uuid128, uuid);
	else if (uuid->type == SDP_UUID32)
		sdp_uuid32_to_uuid128(uuid128, uuid);
	else
		memcpy(uuid128, uuid, sizeof(*uuid));
}

#define MAX_UUIDS 4

static void cmd_find_service(int argc, char **argv)
{
	struct mgmt_cp_start_service_discovery *cp;
	uint8_t buf[sizeof(*cp) + 16 * MAX_UUIDS];
	uuid_t uuid;
	uint128_t uint128;
	uuid_t uuid128;
	uint8_t type = SCAN_TYPE_DUAL;
	int8_t rssi;
	uint16_t count;
	int opt;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	rssi = 127;
	count = 0;

	while ((opt = getopt_long(argc, argv, "+lbu:r:h",
					find_service_options, NULL)) != -1) {
		switch (opt) {
		case 'l':
			type &= ~SCAN_TYPE_BREDR;
			type |= SCAN_TYPE_LE;
			break;
		case 'b':
			type |= SCAN_TYPE_BREDR;
			type &= ~SCAN_TYPE_LE;
			break;
		case 'u':
			if (count == MAX_UUIDS) {
				print("Max %u UUIDs supported", MAX_UUIDS);
				optind = 0;
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}

			if (bt_string2uuid(&uuid, optarg) < 0) {
				print("Invalid UUID: %s", optarg);
				optind = 0;
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			cp = (void *) buf;
			uuid_to_uuid128(&uuid128, &uuid);
			ntoh128((uint128_t *) uuid128.value.uuid128.data,
				&uint128);
			htob128(&uint128, (uint128_t *) cp->uuids[count++]);
			break;
		case 'r':
			rssi = atoi(optarg);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	cp = (void *) buf;
	cp->type = type;
	cp->rssi = rssi;
	cp->uuid_count = cpu_to_le16(count);

	if (mgmt_send(mgmt, MGMT_OP_START_SERVICE_DISCOVERY, index,
				sizeof(*cp) + count * 16, cp,
				find_service_rsp, NULL, NULL) == 0) {
		error("Unable to send start_service_discovery cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void find_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("Unable to start discovery. status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Discovery started");
	discovery = true;
}

static struct option find_options[] = {
	{ "help",	0, 0, 'h' },
	{ "le-only",	1, 0, 'l' },
	{ "bredr-only",	1, 0, 'b' },
	{ "limited",	1, 0, 'L' },
	{ 0, 0, 0, 0 }
};

static void cmd_find(int argc, char **argv)
{
	struct mgmt_cp_start_discovery cp;
	uint8_t op = MGMT_OP_START_DISCOVERY;
	uint8_t type = SCAN_TYPE_DUAL;
	int opt;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	while ((opt = getopt_long(argc, argv, "+lbLh", find_options,
								NULL)) != -1) {
		switch (opt) {
		case 'l':
			type &= ~SCAN_TYPE_BREDR;
			type |= SCAN_TYPE_LE;
			break;
		case 'b':
			type |= SCAN_TYPE_BREDR;
			type &= ~SCAN_TYPE_LE;
			break;
		case 'L':
			op = MGMT_OP_START_LIMITED_DISCOVERY;
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	memset(&cp, 0, sizeof(cp));
	cp.type = type;

	if (mgmt_send(mgmt, op, index, sizeof(cp), &cp, find_rsp,
							NULL, NULL) == 0) {
		error("Unable to send start_discovery cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void stop_find_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("Stop Discovery failed: status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	print("Discovery stopped");
	discovery = false;

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option stop_find_options[] = {
	{ "help",	0, 0, 'h' },
	{ "le-only",	1, 0, 'l' },
	{ "bredr-only",	1, 0, 'b' },
	{ 0, 0, 0, 0 }
};

static void cmd_stop_find(int argc, char **argv)
{
	struct mgmt_cp_stop_discovery cp;
	uint8_t type = SCAN_TYPE_DUAL;
	int opt;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	while ((opt = getopt_long(argc, argv, "+lbh", stop_find_options,
								NULL)) != -1) {
		switch (opt) {
		case 'l':
			type &= ~SCAN_TYPE_BREDR;
			type |= SCAN_TYPE_LE;
			break;
		case 'b':
			type |= SCAN_TYPE_BREDR;
			type &= ~SCAN_TYPE_LE;
			break;
		case 'h':
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	memset(&cp, 0, sizeof(cp));
	cp.type = type;

	if (mgmt_send(mgmt, MGMT_OP_STOP_DISCOVERY, index, sizeof(cp), &cp,
					     stop_find_rsp, NULL, NULL) == 0) {
		error("Unable to send stop_discovery cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void name_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Unable to set local name with status 0x%02x (%s)",
						status, mgmt_errstr(status));

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_name(int argc, char **argv)
{
	struct mgmt_cp_set_local_name cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	strncpy((char *) cp.name, argv[1], HCI_MAX_NAME_LENGTH);
	if (argc > 2)
		strncpy((char *) cp.short_name, argv[2],
					MGMT_MAX_SHORT_NAME_LENGTH - 1);

	if (mgmt_send(mgmt, MGMT_OP_SET_LOCAL_NAME, index, sizeof(cp), &cp,
						name_rsp, NULL, NULL) == 0) {
		error("Unable to send set_name cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void pair_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_pair_device *rp = param;
	char addr[18];

	if (len == 0 && status != 0) {
		error("Pairing failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Unexpected pair_rsp len %u", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status)
		error("Pairing with %s (%s) failed. status 0x%02x (%s)",
			addr, typestr(rp->addr.type), status,
			mgmt_errstr(status));
	else
		print("Paired with %s (%s)", addr, typestr(rp->addr.type));

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option pair_options[] = {
	{ "help",	0, 0, 'h' },
	{ "capability",	1, 0, 'c' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_pair(int argc, char **argv)
{
	struct mgmt_cp_pair_device cp;
	uint8_t cap = 0x01;
	uint8_t type = BDADDR_BREDR;
	char addr[18];
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+c:t:h", pair_options,
								NULL)) != -1) {
		switch (opt) {
		case 'c':
			cap = strtol(optarg, NULL, 0);
			break;
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;
	cp.io_cap = cap;

	ba2str(&cp.addr.bdaddr, addr);
	print("Pairing with %s (%s)", addr, typestr(cp.addr.type));

	if (mgmt_send(mgmt, MGMT_OP_PAIR_DEVICE, index, sizeof(cp), &cp,
						pair_rsp, NULL, NULL) == 0) {
		error("Unable to send pair_device cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cancel_pair_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_addr_info *rp = param;
	char addr[18];

	if (len == 0 && status != 0) {
		error("Cancel Pairing failed with 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Unexpected cancel_pair_rsp len %u", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ba2str(&rp->bdaddr, addr);

	if (status)
		error("Cancel Pairing with %s (%s) failed. 0x%02x (%s)",
			addr, typestr(rp->type), status,
			mgmt_errstr(status));
	else
		print("Pairing Cancelled with %s", addr);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option cancel_pair_options[] = {
	{ "help",	0, 0, 'h' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_cancel_pair(int argc, char **argv)
{
	struct mgmt_addr_info cp;
	uint8_t type = BDADDR_BREDR;
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+t:h", cancel_pair_options,
								NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.bdaddr);
	cp.type = type;

	if (mgmt_reply(mgmt, MGMT_OP_CANCEL_PAIR_DEVICE, index, sizeof(cp), &cp,
					cancel_pair_rsp, NULL, NULL) == 0) {
		error("Unable to send cancel_pair_device cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void unpair_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_unpair_device *rp = param;
	char addr[18];

	if (len == 0 && status != 0) {
		error("Unpair device failed. status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Unexpected unpair_device_rsp len %u", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status)
		error("Unpairing %s failed. status 0x%02x (%s)",
				addr, status, mgmt_errstr(status));
	else
		print("%s unpaired", addr);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option unpair_options[] = {
	{ "help",	0, 0, 'h' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_unpair(int argc, char **argv)
{
	struct mgmt_cp_unpair_device cp;
	uint8_t type = BDADDR_BREDR;
	int opt;
	uint16_t index = mgmt_index;

	while ((opt = getopt_long(argc, argv, "+t:h", unpair_options,
								NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;
	cp.disconnect = 1;

	if (mgmt_send(mgmt, MGMT_OP_UNPAIR_DEVICE, index, sizeof(cp), &cp,
						unpair_rsp, NULL, NULL) == 0) {
		error("Unable to send unpair_device cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void keys_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Load keys failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Keys successfully loaded");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_keys(int argc, char **argv)
{
	struct mgmt_cp_load_link_keys cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));

	if (mgmt_send(mgmt, MGMT_OP_LOAD_LINK_KEYS, index, sizeof(cp), &cp,
						keys_rsp, NULL, NULL) == 0) {
		error("Unable to send load_keys cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void ltks_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Load keys failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Long term keys successfully loaded");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ltks(int argc, char **argv)
{
	struct mgmt_cp_load_long_term_keys cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));

	if (mgmt_send(mgmt, MGMT_OP_LOAD_LONG_TERM_KEYS, index, sizeof(cp), &cp,
						ltks_rsp, NULL, NULL) == 0) {
		error("Unable to send load_ltks cmd");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}
}

static void irks_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Load IRKs failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Identity Resolving Keys successfully loaded");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option irks_options[] = {
	{ "help",	0, 0, 'h' },
	{ "local",	1, 0, 'l' },
	{ "file",	1, 0, 'f' },
	{ 0, 0, 0, 0 }
};

#define MAX_IRKS 4

static void cmd_irks(int argc, char **argv)
{
	struct mgmt_cp_load_irks *cp;
	uint8_t buf[sizeof(*cp) + 23 * MAX_IRKS];
	uint16_t count, local_index;
	char path[PATH_MAX];
	int opt;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp = (void *) buf;
	count = 0;

	while ((opt = getopt_long(argc, argv, "+l:f:h",
					irks_options, NULL)) != -1) {
		switch (opt) {
		case 'l':
			if (count >= MAX_IRKS) {
				error("Number of IRKs exceeded");
				optind = 0;
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			if (strlen(optarg) > 3 &&
					strncasecmp(optarg, "hci", 3) == 0)
				local_index = atoi(optarg + 3);
			else
				local_index = atoi(optarg);
			snprintf(path, sizeof(path),
				"/sys/kernel/debug/bluetooth/hci%u/identity",
				local_index);
			if (!load_identity(path, &cp->irks[count])) {
				error("Unable to load identity");
				optind = 0;
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			count++;
			break;
		case 'f':
			if (count >= MAX_IRKS) {
				error("Number of IRKs exceeded");
				optind = 0;
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			if (!load_identity(optarg, &cp->irks[count])) {
				error("Unable to load identities");
				optind = 0;
				return bt_shell_noninteractive_quit(EXIT_FAILURE);
			}
			count++;
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	cp->irk_count = cpu_to_le16(count);

	if (mgmt_send(mgmt, MGMT_OP_LOAD_IRKS, index,
					sizeof(*cp) + count * 23, cp,
					irks_rsp, NULL, NULL) == 0) {
		error("Unable to send load_irks cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void block_rsp(uint16_t op, uint16_t id, uint8_t status, uint16_t len,
							const void *param)
{
	const struct mgmt_addr_info *rp = param;
	char addr[18];

	if (len == 0 && status != 0) {
		error("%s failed, status 0x%02x (%s)",
				mgmt_opstr(op), status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Unexpected %s len %u", mgmt_opstr(op), len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ba2str(&rp->bdaddr, addr);

	if (status)
		error("%s %s (%s) failed. status 0x%02x (%s)",
				mgmt_opstr(op), addr, typestr(rp->type),
				status, mgmt_errstr(status));
	else
		print("%s %s succeeded", mgmt_opstr(op), addr);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option block_options[] = {
	{ "help",	0, 0, 'h' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_block(int argc, char **argv)
{
	struct mgmt_cp_block_device cp;
	uint8_t type = BDADDR_BREDR;
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+t:h", block_options,
							NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;

	if (send_cmd(mgmt, MGMT_OP_BLOCK_DEVICE, index, sizeof(cp), &cp,
							block_rsp) == 0) {
		error("Unable to send block_device cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_unblock(int argc, char **argv)
{
	struct mgmt_cp_unblock_device cp;
	uint8_t type = BDADDR_BREDR;
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+t:h", block_options,
							NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;

	if (send_cmd(mgmt, MGMT_OP_UNBLOCK_DEVICE, index, sizeof(cp), &cp,
							block_rsp) == 0) {
		error("Unable to send unblock_device cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_add_uuid(int argc, char **argv)
{
	struct mgmt_cp_add_uuid cp;
	uint128_t uint128;
	uuid_t uuid, uuid128;
	uint16_t index;

	if (argc < 3) {
		print("UUID and service hint needed");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (bt_string2uuid(&uuid, argv[1]) < 0) {
		print("Invalid UUID: %s", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	memset(&cp, 0, sizeof(cp));

	uuid_to_uuid128(&uuid128, &uuid);
	ntoh128((uint128_t *) uuid128.value.uuid128.data, &uint128);
	htob128(&uint128, (uint128_t *) cp.uuid);

	cp.svc_hint = atoi(argv[2]);

	if (send_cmd(mgmt, MGMT_OP_ADD_UUID, index, sizeof(cp), &cp,
							class_rsp) == 0) {
		error("Unable to send add_uuid cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_remove_uuid(int argc, char **argv)
{
	struct mgmt_cp_remove_uuid cp;
	uint128_t uint128;
	uuid_t uuid, uuid128;
	uint16_t index;

	if (argc < 2) {
		print("UUID needed");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (bt_string2uuid(&uuid, argv[1]) < 0) {
		print("Invalid UUID: %s", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	memset(&cp, 0, sizeof(cp));

	uuid_to_uuid128(&uuid128, &uuid);
	ntoh128((uint128_t *) uuid128.value.uuid128.data, &uint128);
	htob128(&uint128, (uint128_t *) cp.uuid);

	if (send_cmd(mgmt, MGMT_OP_REMOVE_UUID, index, sizeof(cp), &cp,
							class_rsp) == 0) {
		error("Unable to send remove_uuid cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_clr_uuids(int argc, char **argv)
{
	char *uuid_any = "00000000-0000-0000-0000-000000000000";
	char *rm_argv[] = { "rm-uuid", uuid_any, NULL };

	cmd_remove_uuid(2, rm_argv);
}

static void local_oob_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_local_oob_data *rp = param;
	char str[33];

	if (status != 0) {
		error("Read Local OOB Data failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small (%u bytes) read_local_oob rsp", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bin2hex(rp->hash192, 16, str, sizeof(str));
	print("Hash C from P-192: %s", str);

	bin2hex(rp->rand192, 16, str, sizeof(str));
	print("Randomizer R with P-192: %s", str);

	if (len < sizeof(*rp))
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	bin2hex(rp->hash256, 16, str, sizeof(str));
	print("Hash C from P-256: %s", str);

	bin2hex(rp->rand256, 16, str, sizeof(str));
	print("Randomizer R with P-256: %s", str);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_local_oob(int argc, char **argv)
{
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (mgmt_send(mgmt, MGMT_OP_READ_LOCAL_OOB_DATA, index, 0, NULL,
					local_oob_rsp, NULL, NULL) == 0) {
		error("Unable to send read_local_oob cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void remote_oob_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_addr_info *rp = param;
	char addr[18];

	if (status != 0) {
		error("Add Remote OOB Data failed: 0x%02x (%s)",
						status, mgmt_errstr(status));
		return;
	}

	if (len < sizeof(*rp)) {
		error("Too small (%u bytes) add_remote_oob rsp", len);
		return;
	}

	ba2str(&rp->bdaddr, addr);
	print("Remote OOB data added for %s (%u)", addr, rp->type);
}

static struct option remote_oob_opt[] = {
	{ "help",	0, 0, '?' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_remote_oob(int argc, char **argv)
{
	struct mgmt_cp_add_remote_oob_data cp;
	int opt;
	uint16_t index;

	memset(&cp, 0, sizeof(cp));
	cp.addr.type = BDADDR_BREDR;

	while ((opt = getopt_long(argc, argv, "+t:r:R:h:H:",
					remote_oob_opt, NULL)) != -1) {
		switch (opt) {
		case 't':
			cp.addr.type = strtol(optarg, NULL, 0);
			break;
		case 'r':
			hex2bin(optarg, cp.rand192, 16);
			break;
		case 'h':
			hex2bin(optarg, cp.hash192, 16);
			break;
		case 'R':
			hex2bin(optarg, cp.rand256, 16);
			break;
		case 'H':
			hex2bin(optarg, cp.hash256, 16);
			break;
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	str2ba(argv[0], &cp.addr.bdaddr);

	print("Adding OOB data for %s (%s)", argv[0], typestr(cp.addr.type));

	if (mgmt_send(mgmt, MGMT_OP_ADD_REMOTE_OOB_DATA, index,
				sizeof(cp), &cp, remote_oob_rsp,
				NULL, NULL) == 0) {
		error("Unable to send add_remote_oob cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void did_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Set Device ID failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Device ID successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_did(int argc, char **argv)
{
	struct mgmt_cp_set_device_id cp;
	uint16_t vendor, product, version , source;
	int result;
	uint16_t index;

	result = sscanf(argv[1], "bluetooth:%4hx:%4hx:%4hx", &vendor, &product,
								&version);
	if (result == 3) {
		source = 0x0001;
		goto done;
	}

	result = sscanf(argv[1], "usb:%4hx:%4hx:%4hx", &vendor, &product,
								&version);
	if (result == 3) {
		source = 0x0002;
		goto done;
	}

	return;
done:
	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp.source = htobs(source);
	cp.vendor = htobs(vendor);
	cp.product = htobs(product);
	cp.version = htobs(version);

	if (mgmt_send(mgmt, MGMT_OP_SET_DEVICE_ID, index, sizeof(cp), &cp,
						did_rsp, NULL, NULL) == 0) {
		error("Unable to send set_device_id cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void static_addr_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Set static address failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Static address successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_static_addr(int argc, char **argv)
{
	struct mgmt_cp_set_static_address cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	str2ba(argv[1], &cp.bdaddr);

	if (mgmt_send(mgmt, MGMT_OP_SET_STATIC_ADDRESS, index, sizeof(cp), &cp,
					static_addr_rsp, NULL, NULL) == 0) {
		error("Unable to send set_static_address cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void options_rsp(uint16_t op, uint16_t id, uint8_t status,
					uint16_t len, const void *param)
{
	const uint32_t *rp = param;

	if (status != 0) {
		error("%s for hci%u failed with status 0x%02x (%s)",
			mgmt_opstr(op), id, status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small %s response (%u bytes)",
							mgmt_opstr(op), len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("hci%u %s complete, options: %s", id, mgmt_opstr(op),
						options2str(get_le32(rp)));

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_public_addr(int argc, char **argv)
{
	struct mgmt_cp_set_public_address cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	str2ba(argv[1], &cp.bdaddr);

	if (send_cmd(mgmt, MGMT_OP_SET_PUBLIC_ADDRESS, index, sizeof(cp), &cp,
							options_rsp) == 0) {
		error("Unable to send Set Public Address cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_ext_config(int argc, char **argv)
{
	struct mgmt_cp_set_external_config cp;
	uint16_t index;

	if (parse_setting(argc, argv, &cp.config) == false)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (send_cmd(mgmt, MGMT_OP_SET_EXTERNAL_CONFIG, index, sizeof(cp), &cp,
							options_rsp) == 0) {
		error("Unable to send Set External Config cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_debug_keys(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_DEBUG_KEYS, argc, argv);
}

static void conn_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_get_conn_info *rp = param;	char addr[18];

	if (len == 0 && status != 0) {
		error("Get Conn Info failed, status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Unexpected Get Conn Info len %u", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status) {
		error("Get Conn Info for %s (%s) failed. status 0x%02x (%s)",
						addr, typestr(rp->addr.type),
						status, mgmt_errstr(status));
	} else {
		print("Connection Information for %s (%s)",
						addr, typestr(rp->addr.type));
		print("\tRSSI %d\tTX power %d\tmaximum TX power %d",
				rp->rssi, rp->tx_power, rp->max_tx_power);
	}

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option conn_info_options[] = {
	{ "help",       0, 0, 'h' },
	{ "type",       1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_conn_info(int argc, char **argv)
{
	struct mgmt_cp_get_conn_info cp;
	uint8_t type = BDADDR_BREDR;
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+t:h", conn_info_options,
								NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;

	if (mgmt_send(mgmt, MGMT_OP_GET_CONN_INFO, index, sizeof(cp), &cp,
					conn_info_rsp, NULL, NULL) == 0) {
		error("Unable to send get_conn_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void io_cap_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Could not set IO Capability with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("IO Capabilities successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_io_cap(int argc, char **argv)
{
	struct mgmt_cp_set_io_capability cp;
	uint8_t cap;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cap = strtol(argv[1], NULL, 0);
	memset(&cp, 0, sizeof(cp));
	cp.io_capability = cap;

	if (mgmt_send(mgmt, MGMT_OP_SET_IO_CAPABILITY, index, sizeof(cp), &cp,
					io_cap_rsp, NULL, NULL) == 0) {
		error("Unable to send set-io-cap cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void scan_params_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Set scan parameters failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Scan parameters successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_scan_params(int argc, char **argv)
{
	struct mgmt_cp_set_scan_params cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp.interval = strtol(argv[1], NULL, 0);
	cp.window = strtol(argv[2], NULL, 0);

	if (mgmt_send(mgmt, MGMT_OP_SET_SCAN_PARAMS, index, sizeof(cp), &cp,
					scan_params_rsp, NULL, NULL) == 0) {
		error("Unable to send set_scan_params cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void clock_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_get_clock_info *rp = param;

	if (len < sizeof(*rp)) {
		error("Unexpected Get Clock Info len %u", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (status) {
		error("Get Clock Info failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Local Clock:   %u", le32_to_cpu(rp->local_clock));
	print("Piconet Clock: %u", le32_to_cpu(rp->piconet_clock));
	print("Accurary:      %u", le16_to_cpu(rp->accuracy));

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_clock_info(int argc, char **argv)
{
	struct mgmt_cp_get_clock_info cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));

	if (argc > 1)
		str2ba(argv[1], &cp.addr.bdaddr);

	if (mgmt_send(mgmt, MGMT_OP_GET_CLOCK_INFO, index, sizeof(cp), &cp,
					clock_info_rsp, NULL, NULL) == 0) {
		error("Unable to send get_clock_info cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void add_device_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Add device failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option add_device_options[] = {
	{ "help",	0, 0, 'h' },
	{ "action",	1, 0, 'a' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_add_device(int argc, char **argv)
{
	struct mgmt_cp_add_device cp;
	uint8_t action = 0x00;
	uint8_t type = BDADDR_BREDR;
	char addr[18];
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+a:t:h", add_device_options,
								NULL)) != -1) {
		switch (opt) {
		case 'a':
			action = strtol(optarg, NULL, 0);
			break;
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;
	cp.action = action;

	ba2str(&cp.addr.bdaddr, addr);
	print("Adding device with %s (%s)", addr, typestr(cp.addr.type));

	if (mgmt_send(mgmt, MGMT_OP_ADD_DEVICE, index, sizeof(cp), &cp,
					add_device_rsp, NULL, NULL) == 0) {
		error("Unable to send add device command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void remove_device_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Remove device failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct option del_device_options[] = {
	{ "help",	0, 0, 'h' },
	{ "type",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static void cmd_del_device(int argc, char **argv)
{
	struct mgmt_cp_remove_device cp;
	uint8_t type = BDADDR_BREDR;
	char addr[18];
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+t:h", del_device_options,
								NULL)) != -1) {
		switch (opt) {
		case 't':
			type = strtol(optarg, NULL, 0);
			break;
		case 'h':
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_SUCCESS);
		default:
			bt_shell_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		bt_shell_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}


	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));
	str2ba(argv[0], &cp.addr.bdaddr);
	cp.addr.type = type;

	ba2str(&cp.addr.bdaddr, addr);
	print("Removing device with %s (%s)", addr, typestr(cp.addr.type));

	if (mgmt_send(mgmt, MGMT_OP_REMOVE_DEVICE, index, sizeof(cp), &cp,
					remove_device_rsp, NULL, NULL) == 0) {
		error("Unable to send remove device command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_clr_devices(int argc, char **argv)
{
	char *bdaddr_any = "00:00:00:00:00:00";
	char *rm_argv[] = { "del-device", bdaddr_any, NULL };

	cmd_del_device(2, rm_argv);
}

static void local_oob_ext_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_local_oob_ext_data *rp = param;
	uint16_t eir_len;

	if (status != 0) {
		error("Read Local OOB Ext Data failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small (%u bytes) read_local_oob_ext rsp", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	eir_len = le16_to_cpu(rp->eir_len);
	if (len != sizeof(*rp) + eir_len) {
		error("local_oob_ext: expected %zu bytes, got %u bytes",
						sizeof(*rp) + eir_len, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print_eir(rp->eir, eir_len);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_bredr_oob(int argc, char **argv)
{
	struct mgmt_cp_read_local_oob_ext_data cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp.type = SCAN_TYPE_BREDR;

	if (!mgmt_send(mgmt, MGMT_OP_READ_LOCAL_OOB_EXT_DATA,
					index, sizeof(cp), &cp,
					local_oob_ext_rsp, NULL, NULL)) {
		error("Unable to send read_local_oob_ext cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_le_oob(int argc, char **argv)
{
	struct mgmt_cp_read_local_oob_ext_data cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp.type = SCAN_TYPE_LE;

	if (!mgmt_send(mgmt, MGMT_OP_READ_LOCAL_OOB_EXT_DATA,
					index, sizeof(cp), &cp,
					local_oob_ext_rsp, NULL, NULL)) {
		error("Unable to send read_local_oob_ext cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static const char *adv_flags_str[] = {
				"connectable",
				"general-discoverable",
				"limited-discoverable",
				"managed-flags",
				"tx-power",
				"scan-rsp-appearance",
				"scan-rsp-local-name",
				"Secondary-channel-1M",
				"Secondary-channel-2M",
				"Secondary-channel-CODED",
};

static const char *adv_flags2str(uint32_t flags)
{
	static char str[256];
	unsigned i;
	int off;

	off = 0;
	str[0] = '\0';

	for (i = 0; i < NELEM(adv_flags_str); i++) {
		if ((flags & (1 << i)) != 0)
			off += snprintf(str + off, sizeof(str) - off, "%s ",
							adv_flags_str[i]);
	}

	return str;
}

static void adv_features_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_adv_features *rp = param;
	uint32_t supported_flags;

	if (status != 0) {
		error("Reading adv features failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small adv features reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp) + rp->num_instances * sizeof(uint8_t)) {
		error("Instances count (%u) doesn't match reply length (%u)",
							rp->num_instances, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	supported_flags = le32_to_cpu(rp->supported_flags);
	print("Supported flags: %s", adv_flags2str(supported_flags));
	print("Max advertising data len: %u", rp->max_adv_data_len);
	print("Max scan response data len: %u", rp->max_scan_rsp_len);
	print("Max instances: %u", rp->max_instances);

	print("Instances list with %u item%s", rp->num_instances,
					rp->num_instances != 1 ? "s" : "");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_advinfo(int argc, char **argv)
{
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (!mgmt_send(mgmt, MGMT_OP_READ_ADV_FEATURES, index, 0, NULL,
					adv_features_rsp, NULL, NULL)) {
		error("Unable to send advertising features command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void adv_size_info_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_get_adv_size_info *rp = param;
	uint32_t flags;

	if (status != 0) {
		error("Reading adv size info failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small adv size info reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	flags = le32_to_cpu(rp->flags);
	print("Instance: %u", rp->instance);
	print("Flags: %s", adv_flags2str(flags));
	print("Max advertising data len: %u", rp->max_adv_data_len);
	print("Max scan response data len: %u", rp->max_scan_rsp_len);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void advsize_usage(void)
{
	bt_shell_usage();
	print("Options:\n"
		"\t -c, --connectable         \"connectable\" flag\n"
		"\t -g, --general-discov      \"general-discoverable\" flag\n"
		"\t -l, --limited-discov      \"limited-discoverable\" flag\n"
		"\t -m, --managed-flags       \"managed-flags\" flag\n"
		"\t -p, --tx-power            \"tx-power\" flag\n"
		"\t -a, --appearance          \"appearance\" flag\n"
		"\t -n, --local-name          \"local-name\" flag");
}

static struct option advsize_options[] = {
	{ "help",		0, 0, 'h' },
	{ "connectable",	0, 0, 'c' },
	{ "general-discov",	0, 0, 'g' },
	{ "limited-discov",	0, 0, 'l' },
	{ "managed-flags",	0, 0, 'm' },
	{ "tx-power",		0, 0, 'p' },
	{ "appearance",		0, 0, 'a' },
	{ "local-name",		0, 0, 'n' },
	{ 0, 0, 0, 0}
};

static void cmd_advsize(int argc, char **argv)
{
	struct mgmt_cp_get_adv_size_info cp;
	uint8_t instance;
	uint32_t flags = 0;
	int opt;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+cglmphna",
						advsize_options, NULL)) != -1) {
		switch (opt) {
		case 'c':
			flags |= MGMT_ADV_FLAG_CONNECTABLE;
			break;
		case 'g':
			flags |= MGMT_ADV_FLAG_DISCOV;
			break;
		case 'l':
			flags |= MGMT_ADV_FLAG_LIMITED_DISCOV;
			break;
		case 'm':
			flags |= MGMT_ADV_FLAG_MANAGED_FLAGS;
			break;
		case 'p':
			flags |= MGMT_ADV_FLAG_TX_POWER;
			break;
		case 'a':
			flags |= MGMT_ADV_FLAG_APPEARANCE;
			break;
		case 'n':
			flags |= MGMT_ADV_FLAG_LOCAL_NAME;
			break;
		default:
			advsize_usage();
			optind = 0;
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc != 1) {
		advsize_usage();
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	instance = strtol(argv[0], NULL, 0);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));

	cp.instance = instance;
	cp.flags = cpu_to_le32(flags);

	if (!mgmt_send(mgmt, MGMT_OP_GET_ADV_SIZE_INFO, index, sizeof(cp), &cp,
					adv_size_info_rsp, NULL, NULL)) {
		error("Unable to send advertising size info command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void add_adv_rsp(uint8_t status, uint16_t len, const void *param,
								void *user_data)
{
	const struct mgmt_rp_add_advertising *rp = param;

	if (status != 0) {
		error("Add Advertising failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Invalid Add Advertising response length (%u)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Instance added: %u", rp->instance);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void add_adv_usage(void)
{
	bt_shell_usage();
	print("Options:\n"
		"\t -u, --uuid <uuid>         Service UUID\n"
		"\t -d, --adv-data <data>     Advertising Data bytes\n"
		"\t -s, --scan-rsp <data>     Scan Response Data bytes\n"
		"\t -t, --timeout <timeout>   Timeout in seconds\n"
		"\t -D, --duration <duration> Duration in seconds\n"
		"\t -P, --phy <phy>           Phy type, Specify 1M/2M/CODED\n"
		"\t -c, --connectable         \"connectable\" flag\n"
		"\t -g, --general-discov      \"general-discoverable\" flag\n"
		"\t -l, --limited-discov      \"limited-discoverable\" flag\n"
		"\t -n, --scan-rsp-local-name \"local-name\" flag\n"
		"\t -a, --scan-rsp-appearance \"appearance\" flag\n"
		"\t -m, --managed-flags       \"managed-flags\" flag\n"
		"\t -p, --tx-power            \"tx-power\" flag\n"
		"e.g.:\n"
		"\tadd-adv -u 180d -u 180f -d 080954657374204C45 1");
}

static struct option add_adv_options[] = {
	{ "help",		0, 0, 'h' },
	{ "uuid",		1, 0, 'u' },
	{ "adv-data",		1, 0, 'd' },
	{ "scan-rsp",		1, 0, 's' },
	{ "timeout",		1, 0, 't' },
	{ "duration",		1, 0, 'D' },
	{ "phy",		1, 0, 'P' },
	{ "connectable",	0, 0, 'c' },
	{ "general-discov",	0, 0, 'g' },
	{ "limited-discov",	0, 0, 'l' },
	{ "managed-flags",	0, 0, 'm' },
	{ "tx-power",		0, 0, 'p' },
	{ 0, 0, 0, 0}
};

static bool parse_bytes(char *optarg, uint8_t **bytes, size_t *len)
{
	unsigned i;

	if (!optarg) {
		add_adv_usage();
		return false;
	}

	*len = strlen(optarg);

	if (*len % 2) {
		error("Malformed data");
		return false;
	}

	*len /= 2;
	if (*len > UINT8_MAX) {
		error("Data too long");
		return false;
	}

	*bytes = malloc(*len);
	if (!*bytes) {
		error("Failed to allocate memory");
		return false;
	}

	for (i = 0; i < *len; i++) {
		if (sscanf(optarg + (i * 2), "%2hhx", *bytes + i) != 1) {
			error("Invalid data");
			free(*bytes);
			*bytes = NULL;
			return false;
		}
	}

	return true;
}

#define MAX_AD_UUID_BYTES 32

static void cmd_add_adv(int argc, char **argv)
{
	struct mgmt_cp_add_advertising *cp = NULL;
	int opt;
	uint8_t *adv_data = NULL, *scan_rsp = NULL;
	size_t adv_len = 0, scan_rsp_len = 0;
	size_t cp_len;
	uint8_t uuids[MAX_AD_UUID_BYTES];
	size_t uuid_bytes = 0;
	uint8_t uuid_type = 0;
	uint16_t timeout = 0, duration = 0;
	uint8_t instance;
	uuid_t uuid;
	bool success = false;
	bool quit = true;
	uint32_t flags = 0;
	uint16_t index;

	while ((opt = getopt_long(argc, argv, "+u:d:s:t:D:P:cglmphna",
						add_adv_options, NULL)) != -1) {
		switch (opt) {
		case 'u':
			if (bt_string2uuid(&uuid, optarg) < 0) {
				print("Invalid UUID: %s", optarg);
				goto done;
			}

			if (uuid_type && uuid_type != uuid.type) {
				print("UUID types must be consistent");
				goto done;
			}

			if (uuid.type == SDP_UUID16) {
				if (uuid_bytes + 2 >= MAX_AD_UUID_BYTES) {
					print("Too many UUIDs");
					goto done;
				}

				put_le16(uuid.value.uuid16, uuids + uuid_bytes);
				uuid_bytes += 2;
			} else if (uuid.type == SDP_UUID128) {
				if (uuid_bytes + 16 >= MAX_AD_UUID_BYTES) {
					print("Too many UUIDs");
					goto done;
				}

				bswap_128(uuid.value.uuid128.data,
							uuids + uuid_bytes);
				uuid_bytes += 16;
			} else {
				printf("Unsupported UUID type");
				goto done;
			}

			if (!uuid_type)
				uuid_type = uuid.type;

			break;
		case 'd':
			if (adv_len) {
				print("Only one adv-data option allowed");
				goto done;
			}

			if (!parse_bytes(optarg, &adv_data, &adv_len))
				goto done;
			break;
		case 's':
			if (scan_rsp_len) {
				print("Only one scan-rsp option allowed");
				goto done;
			}

			if (!parse_bytes(optarg, &scan_rsp, &scan_rsp_len))
				goto done;
			break;
		case 't':
			timeout = strtol(optarg, NULL, 0);
			break;
		case 'D':
			duration = strtol(optarg, NULL, 0);
			break;
		case 'c':
			flags |= MGMT_ADV_FLAG_CONNECTABLE;
			break;
		case 'g':
			flags |= MGMT_ADV_FLAG_DISCOV;
			break;
		case 'l':
			flags |= MGMT_ADV_FLAG_LIMITED_DISCOV;
			break;
		case 'm':
			flags |= MGMT_ADV_FLAG_MANAGED_FLAGS;
			break;
		case 'p':
			flags |= MGMT_ADV_FLAG_TX_POWER;
			break;
		case 'n':
			flags |= MGMT_ADV_FLAG_LOCAL_NAME;
			break;
		case 'a':
			flags |= MGMT_ADV_FLAG_APPEARANCE;
			break;
		case 'P':
			if (strcasecmp(optarg, "1M") == 0)
				flags |= MGMT_ADV_FLAG_SEC_1M;
			else if (strcasecmp(optarg, "2M") == 0)
				flags |= MGMT_ADV_FLAG_SEC_2M;
			else if (strcasecmp(optarg, "CODED") == 0)
				flags |= MGMT_ADV_FLAG_SEC_CODED;
			else
				goto done;
			break;
		case 'h':
			success = true;
			/* fall through */
		default:
			add_adv_usage();
			optind = 0;
			goto done;
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc != 1) {
		add_adv_usage();
		goto done;
	}

	if (uuid_bytes)
		uuid_bytes += 2;

	instance = strtol(argv[0], NULL, 0);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp_len = sizeof(*cp) + uuid_bytes + adv_len + scan_rsp_len;
	cp = malloc0(cp_len);
	if (!cp)
		goto done;

	cp->instance = instance;
	put_le32(flags, &cp->flags);
	put_le16(timeout, &cp->timeout);
	put_le16(duration, &cp->duration);
	cp->adv_data_len = adv_len + uuid_bytes;
	cp->scan_rsp_len = scan_rsp_len;

	if (uuid_bytes) {
		cp->data[0] = uuid_bytes - 1;
		cp->data[1] = uuid_type == SDP_UUID16 ? 0x03 : 0x07;
		memcpy(cp->data + 2, uuids, uuid_bytes - 2);
	}

	memcpy(cp->data + uuid_bytes, adv_data, adv_len);
	memcpy(cp->data + uuid_bytes + adv_len, scan_rsp, scan_rsp_len);

	if (!mgmt_send(mgmt, MGMT_OP_ADD_ADVERTISING, index, cp_len, cp,
						add_adv_rsp, NULL, NULL)) {
		error("Unable to send \"Add Advertising\" command");
		goto done;
	}

	quit = false;

done:
	free(adv_data);
	free(scan_rsp);
	free(cp);

	if (quit)
		bt_shell_noninteractive_quit(success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void rm_adv_rsp(uint8_t status, uint16_t len, const void *param,
								void *user_data)
{
	const struct mgmt_rp_remove_advertising *rp = param;

	if (status != 0) {
		error("Remove Advertising failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len != sizeof(*rp)) {
		error("Invalid Remove Advertising response length (%u)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Instance removed: %u", rp->instance);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_rm_adv(int argc, char **argv)
{
	struct mgmt_cp_remove_advertising cp;
	uint8_t instance;
	uint16_t index;

	instance = strtol(argv[1], NULL, 0);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	memset(&cp, 0, sizeof(cp));

	cp.instance = instance;

	if (!mgmt_send(mgmt, MGMT_OP_REMOVE_ADVERTISING, index, sizeof(cp), &cp,
						rm_adv_rsp, NULL, NULL)) {
		error("Unable to send \"Remove Advertising\" command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_clr_adv(int argc, char **argv)
{
	char *all_instances = "0";
	char *rm_argv[] = { "rm-adv", all_instances, NULL };

	cmd_rm_adv(2, rm_argv);
}

static void appearance_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0)
		error("Could not set Appearance with status 0x%02x (%s)",
						status, mgmt_errstr(status));
	else
		print("Appearance successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_appearance(int argc, char **argv)
{
	struct mgmt_cp_set_appearance cp;
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	cp.appearance = cpu_to_le16(strtol(argv[1], NULL, 0));

	if (mgmt_send(mgmt, MGMT_OP_SET_APPEARANCE, index, sizeof(cp), &cp,
					appearance_rsp, NULL, NULL) == 0) {
		error("Unable to send appearance cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static const char *phys_str[] = {
	"BR1M1SLOT",
	"BR1M3SLOT",
	"BR1M5SLOT",
	"EDR2M1SLOT",
	"EDR2M3SLOT",
	"EDR2M5SLOT",
	"EDR3M1SLOT",
	"EDR3M3SLOT",
	"EDR3M5SLOT",
	"LE1MTX",
	"LE1MRX",
	"LE2MTX",
	"LE2MRX",
	"LECODEDTX",
	"LECODEDRX",
};

static const char *phys2str(uint32_t phys)
{
	static char str[256];
	unsigned int i;
	int off;

	off = 0;
	str[0] = '\0';

	for (i = 0; i < NELEM(phys_str); i++) {
		if ((phys & (1 << i)) != 0)
			off += snprintf(str + off, sizeof(str) - off, "%s ",
							phys_str[i]);
	}

	return str;
}

static bool str2phy(const char *phy_str, uint32_t *phy_val)
{
	unsigned int i;

	for (i = 0; i < NELEM(phys_str); i++) {
		if (strcasecmp(phys_str[i], phy_str) == 0) {
			*phy_val = (1 << i);
			return true;
		}
	}

	return false;
}

static void get_phy_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_get_phy_confguration *rp = param;
	uint32_t supported_phys, selected_phys, configurable_phys;

	if (status != 0) {
		error("Get PHY Configuration failed with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small get-phy reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	supported_phys = get_le32(&rp->supported_phys);
	configurable_phys = get_le32(&rp->configurable_phys);
	selected_phys = get_le32(&rp->selected_phys);

	print("Supported phys: %s", phys2str(supported_phys));
	print("Configurable phys: %s", phys2str(configurable_phys));
	print("Selected phys: %s", phys2str(selected_phys));

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void get_phy(void)
{
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (mgmt_send(mgmt, MGMT_OP_GET_PHY_CONFIGURATION, index, 0, NULL,
					get_phy_rsp, NULL, NULL) == 0) {
		error("Unable to send Get PHY cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void set_phy_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	if (status != 0) {
		error("Could not set PHY Configuration with status 0x%02x (%s)",
						status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("PHY Configuration successfully set");

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_phy(int argc, char **argv)
{
	struct mgmt_cp_set_phy_confguration cp;
	int i;
	uint32_t phys = 0;
	uint16_t index;

	if (argc < 2)
		return get_phy();

	for (i = 1; i < argc; i++) {
		uint32_t phy_val;

		if (str2phy(argv[i], &phy_val))
			phys |= phy_val;
	}

	cp.selected_phys = cpu_to_le32(phys);

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (mgmt_send(mgmt, MGMT_OP_SET_PHY_CONFIGURATION, index, sizeof(cp),
					&cp, set_phy_rsp, NULL, NULL) == 0) {
		error("Unable to send %s cmd",
				mgmt_opstr(MGMT_OP_GET_PHY_CONFIGURATION));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_wbs(int argc, char **argv)
{
	cmd_setting(MGMT_OP_SET_WIDEBAND_SPEECH, argc, argv);
}

static const char * const advmon_features_str[] = {
	"Pattern monitor with logic OR.",
};

static const char *advmon_features2str(uint32_t features)
{
	static char str[512];
	unsigned int off, i;

	off = 0;
	snprintf(str, sizeof(str), "\n\tNone");

	for (i = 0; i < NELEM(advmon_features_str); i++) {
		if ((features & (1 << i)) != 0 && off < sizeof(str))
			off += snprintf(str + off, sizeof(str) - off, "\n\t%s",
						advmon_features_str[i]);
	}

	return str;
}

static void advmon_features_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_adv_monitor_features *rp = param;
	uint32_t supported_features, enabled_features;
	uint16_t num_handles;
	int i;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Reading adv monitor features failed with status 0x%02x "
					"(%s)", status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (len < sizeof(*rp)) {
		error("Too small adv monitor features reply (%u bytes)", len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	supported_features = le32_to_cpu(rp->supported_features);
	enabled_features = le32_to_cpu(rp->enabled_features);
	num_handles = le16_to_cpu(rp->num_handles);

	if (len < sizeof(*rp) + num_handles * sizeof(uint16_t)) {
		error("Handles count (%u) doesn't match reply length (%u)",
							num_handles, len);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Supported features:%s", advmon_features2str(supported_features));
	print("Enabled features:%s", advmon_features2str(enabled_features));
	print("Max number of handles: %u", le16_to_cpu(rp->max_num_handles));
	print("Max number of patterns: %u", rp->max_num_patterns);
	print("Handles list with %u item%s", num_handles,
			num_handles == 0 ? "" : num_handles == 1 ? ":" : "s:");
	for (i = 0; i < num_handles; i++)
		print("\t0x%04x ", le16_to_cpu(rp->handles[i]));

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_advmon_features(int argc, char **argv)
{
	uint16_t index;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (!mgmt_send(mgmt, MGMT_OP_READ_ADV_MONITOR_FEATURES, index, 0, NULL,
					advmon_features_rsp, NULL, NULL)) {
		error("Unable to send advertising monitor features command");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void advmon_add_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_add_adv_patterns_monitor *rp = param;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Could not add advertisement monitor with status "
				"0x%02x (%s)", status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Advertisement monitor with handle:0x%04x added",
					le16_to_cpu(rp->monitor_handle));
	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static bool str2pattern(struct mgmt_adv_pattern *pattern, const char *str)
{
	int type_len, offset_len, offset_end_pos, str_len;
	int i, j;
	char pattern_str[62] = { 0 };
	char tmp;

	if (sscanf(str, "%2hhx%n:%2hhx%n:%s", &pattern->ad_type, &type_len,
			&pattern->offset, &offset_end_pos, pattern_str) != 3)
		return false;

	offset_len = offset_end_pos - type_len - 1;
	str_len = strlen(pattern_str);
	pattern->length = str_len / 2 + str_len % 2;

	if (type_len > 2 || offset_len > 2 ||
					pattern->offset + pattern->length > 31)
		return false;

	for (i = 0, j = 0; i < str_len; i++, j++) {
		if (sscanf(&pattern_str[i++], "%2hhx", &pattern->value[j])
									!= 1)
			return false;
		if (i < str_len && sscanf(&pattern_str[i], "%1hhx", &tmp) != 1)
			return false;
	}

	return true;
}

static struct option add_monitor_rssi_options[] = {
	{ "help",		0, 0, 'h' },
	{ "high-threshold",	1, 0, 'R' },
	{ "low-threshold",	1, 0, 'r' },
	{ "high-timeout",	1, 0, 'T' },
	{ "low-timeout",	1, 0, 't' },
	{ "sampling",		1, 0, 's' },
	{ 0, 0, 0, 0 }
};

static void advmon_add_pattern_usage(void)
{
	bt_shell_usage();
	print("patterns format:\n"
		"\t<ad_type:offset:pattern> [patterns]\n"
		"e.g.:\n"
		"\tadd-pattern 0:1:c504 ff:a:9a55beef");
}

static void advmon_add_pattern_rssi_usage(void)
{
	bt_shell_usage();
	print("RSSI options:\n"
		"\t -R, --high-threshold <dBm>  "
			"RSSI high threshold. Default: -70\n"
		"\t -r, --low-threshold <dBm>   "
			"RSSI low threshold. Default: -50\n"
		"\t -T, --high-timeout <s>      "
			"RSSI high threshold duration. Default: 0\n"
		"\t -t, --low-timeout <s>       "
			"RSSI low threshold duration. Default: 5\n"
		"\t -s, --sampling <N * 100ms>  "
			"RSSI sampling period. Default: 0\n"
		"patterns format:\n"
		"\t<ad_type:offset:pattern> [patterns]\n"
		"e.g.:\n"
		"\tadd-pattern-rssi -R 0xb2 -r -102 0:1:c504 ff:a:9a55beef");
}

static void cmd_advmon_add_pattern(int argc, char **argv)
{
	bool success = true;
	uint16_t index;
	int i, cp_len;
	struct mgmt_cp_add_adv_monitor *cp = NULL;

	if (!strcmp(argv[1], "-h"))
		goto done;

	argc -= 1;
	argv += 1;

	cp_len = sizeof(*cp) + argc * sizeof(struct mgmt_adv_pattern);
	cp = malloc0(cp_len);
	if (!cp) {
		error("Failed to alloc patterns.");
		success = false;
		goto done;
	}

	cp->pattern_count = argc;

	for (i = 0; i < argc; i++) {
		if (!str2pattern(&cp->patterns[i], argv[i])) {
			error("Failed to parse monitor patterns.");
			success = false;
			goto done;
		}
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (!mgmt_send(mgmt, MGMT_OP_ADD_ADV_PATTERNS_MONITOR, index,
				cp_len, cp, advmon_add_rsp, NULL, NULL)) {
		error("Unable to send Add Advertising Monitor command");
		success = false;
		goto done;
	}

	free(cp);
	return;

done:
	free(cp);
	advmon_add_pattern_usage();
	bt_shell_noninteractive_quit(success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void cmd_advmon_add_pattern_rssi(int argc, char **argv)
{
	bool success = true;
	int opt;
	int8_t rssi_low = -70;
	int8_t rssi_high = -50;
	uint16_t rssi_low_timeout = 5;
	uint16_t rssi_high_timeout = 0;
	uint8_t rssi_sampling_period = 0;
	uint16_t index;
	int i, cp_len;
	struct mgmt_cp_add_adv_patterns_monitor_rssi *cp = NULL;

	while ((opt = getopt_long(argc, argv, "+hr:R:t:T:s:",
				add_monitor_rssi_options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			goto done;
		case 'r':
			rssi_low = strtol(optarg, NULL, 0);
			break;
		case 'R':
			rssi_high = strtol(optarg, NULL, 0);
			break;
		case 't':
			rssi_low_timeout = strtol(optarg, NULL, 0);
			break;
		case 'T':
			rssi_high_timeout = strtol(optarg, NULL, 0);
			break;
		case 's':
			rssi_sampling_period = strtol(optarg, NULL, 0);
			break;
		default:
			success = false;
			goto done;
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	cp_len = sizeof(*cp) + argc * sizeof(struct mgmt_adv_pattern);
	cp = malloc0(cp_len);
	if (!cp) {
		error("Failed to alloc patterns.");
		success = false;
		goto done;
	}

	cp->pattern_count = argc;
	cp->rssi.high_threshold = rssi_high;
	cp->rssi.low_threshold = rssi_low;
	cp->rssi.high_threshold_timeout = htobs(rssi_high_timeout);
	cp->rssi.low_threshold_timeout = htobs(rssi_low_timeout);
	cp->rssi.sampling_period = rssi_sampling_period;

	for (i = 0; i < argc; i++) {
		if (!str2pattern(&cp->patterns[i], argv[i])) {
			error("Failed to parse monitor patterns.");
			success = false;
			goto done;
		}
	}

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (!mgmt_send(mgmt, MGMT_OP_ADD_ADV_PATTERNS_MONITOR_RSSI, index,
				cp_len, cp, advmon_add_rsp, NULL, NULL)) {
		error("Unable to send Add Advertising Monitor RSSI command");
		success = false;
		goto done;
	}

	free(cp);
	return;

done:
	free(cp);
	optind = 0;
	advmon_add_pattern_rssi_usage();
	bt_shell_noninteractive_quit(success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void advmon_remove_rsp(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_remove_adv_monitor *rp = param;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Could not remove advertisement monitor with status "
				"0x%02x (%s)", status, mgmt_errstr(status));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	print("Advertisement monitor with handle: 0x%04x removed",
					le16_to_cpu(rp->monitor_handle));
	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_advmon_remove(int argc, char **argv)
{
	struct mgmt_cp_remove_adv_monitor cp;
	uint16_t index, monitor_handle;

	index = mgmt_index;
	if (index == MGMT_INDEX_NONE)
		index = 0;

	if (sscanf(argv[1], "%hx", &monitor_handle) != 1) {
		error("Wrong formatted handle argument");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cp.monitor_handle = cpu_to_le16(monitor_handle);
	if (mgmt_send(mgmt, MGMT_OP_REMOVE_ADV_MONITOR, index, sizeof(cp), &cp,
					advmon_remove_rsp, NULL, NULL) == 0) {
		error("Unable to send appearance cmd");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void register_mgmt_callbacks(struct mgmt *mgmt, uint16_t index)
{
	mgmt_register(mgmt, MGMT_EV_CONTROLLER_ERROR, index, controller_error,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_INDEX_ADDED, index, index_added,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_INDEX_REMOVED, index, index_removed,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_SETTINGS, index, new_settings,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DISCOVERING, index, discovering,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_LINK_KEY, index, new_link_key,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DEVICE_CONNECTED, index, connected,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DEVICE_DISCONNECTED, index, disconnected,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_CONNECT_FAILED, index, conn_failed,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_AUTH_FAILED, index, auth_failed,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_CLASS_OF_DEV_CHANGED, index,
					class_of_dev_changed, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_LOCAL_NAME_CHANGED, index,
					local_name_changed, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DEVICE_FOUND, index, device_found,
								mgmt, NULL);
	mgmt_register(mgmt, MGMT_EV_PIN_CODE_REQUEST, index, request_pin,
								mgmt, NULL);
	mgmt_register(mgmt, MGMT_EV_USER_CONFIRM_REQUEST, index, user_confirm,
								mgmt, NULL);
	mgmt_register(mgmt, MGMT_EV_USER_PASSKEY_REQUEST, index,
						request_passkey, mgmt, NULL);
	mgmt_register(mgmt, MGMT_EV_PASSKEY_NOTIFY, index,
						passkey_notify, mgmt, NULL);
	mgmt_register(mgmt, MGMT_EV_UNCONF_INDEX_ADDED, index,
					unconf_index_added, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_UNCONF_INDEX_REMOVED, index,
					unconf_index_removed, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_CONFIG_OPTIONS, index,
					new_config_options, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_EXT_INDEX_ADDED, index,
					ext_index_added, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_EXT_INDEX_REMOVED, index,
					ext_index_removed, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_LOCAL_OOB_DATA_UPDATED, index,
					local_oob_data_updated, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_ADVERTISING_ADDED, index,
						advertising_added, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_ADVERTISING_REMOVED, index,
					advertising_removed, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_ADV_MONITOR_ADDED, index, advmon_added,
								NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_ADV_MONITOR_REMOVED, index, advmon_removed,
								NULL, NULL);
}

static void cmd_select(int argc, char **argv)
{
	mgmt_cancel_all(mgmt);
	mgmt_unregister_all(mgmt);

	set_index(argv[1]);

	register_mgmt_callbacks(mgmt, mgmt_index);

	print("Selected index %u", mgmt_index);

	update_prompt(mgmt_index);
}

static const struct bt_shell_menu monitor_menu = {
	.name = "monitor",
	.desc = "Advertisement Monitor Submenu",
	.entries = {
	{ "features",		NULL,
		cmd_advmon_features,	"Show advertisement monitor "
					"features"			},
	{ "remove",		"<handle>",
		cmd_advmon_remove,	"Remove advertisement monitor "	},
	{ "add-pattern",	"[-h] <patterns>",
		cmd_advmon_add_pattern,	"Add advertisement monitor pattern" },
	{ "add-pattern-rssi",	"[options] <patterns>",
		cmd_advmon_add_pattern_rssi,
		"Add advertisement monitor pattern with RSSI options"    },
	{ } },
};

static const struct bt_shell_menu main_menu = {
	.name = "main",
	.entries = {
	{ "select",		"<index>",
		cmd_select,		"Select a different index"	},
	{ "revision",		NULL,
		cmd_revision,		"Get the MGMT Revision"		},
	{ "commands",		NULL,
		cmd_commands,		"List supported commands"	},
	{ "config",		NULL,
		cmd_config,		"Show configuration info"	},
	{ "info",		NULL,
		cmd_info,		"Show controller info"		},
	{ "extinfo",		NULL,
		cmd_extinfo,		"Show extended controller info"	},
	{ "auto-power",		NULL,
		cmd_auto_power,		"Power all available features"	},
	{ "power",		"<on/off>",
		cmd_power,		"Toggle powered state"		},
	{ "discov",		"<yes/no/limited> [timeout]",
		cmd_discov,		"Toggle discoverable state"	},
	{ "connectable",	"<on/off>",
	cmd_connectable,		"Toggle connectable state"	},
	{ "fast-conn",		"<on/off>",
		cmd_fast_conn,		"Toggle fast connectable state"	},
	{ "bondable",		"<on/off>",
		cmd_bondable,		"Toggle bondable state"		},
	{ "pairable",		"<on/off>",
		cmd_bondable,		"Toggle bondable state"		},
	{ "linksec",		"<on/off>",
		cmd_linksec,		"Toggle link level security"	},
	{ "ssp",		"<on/off>",
		cmd_ssp,		"Toggle SSP mode"		},
	{ "sc",			"<on/off/only>",
		cmd_sc,			"Toogle SC support"		},
	{ "hs",			"<on/off>",
		cmd_hs,			"Toggle HS support"		},
	{ "le",			"<on/off>",
		cmd_le,			"Toggle LE support"		},
	{ "advertising",	"<on/off>",
	cmd_advertising,		"Toggle LE advertising",	},
	{ "bredr",		"<on/off>",
		cmd_bredr,		"Toggle BR/EDR support",	},
	{ "privacy",		"<on/off>",
		cmd_privacy,		"Toggle privacy support"	},
	{ "class",		"<major> <minor>",
		cmd_class,		"Set device major/minor class"	},
	{ "disconnect", 	"[-t type] <remote address>",
		cmd_disconnect,		"Disconnect device"		},
	{ "con",		NULL,
		cmd_con,		"List connections"		},
	{ "find",		"[-l|-b] [-L]",
		cmd_find,		"Discover nearby devices"	},
	{ "find-service",	"[-u UUID] [-r RSSI_Threshold] [-l|-b]",
		cmd_find_service,	"Discover nearby service"	},
	{ "stop-find",		"[-l|-b]",
		cmd_stop_find,		"Stop discovery"		},
	{ "name",		"<name> [shortname]",
		cmd_name,		"Set local name"		},
	{ "pair",		"[-c cap] [-t type] <remote address>",
		cmd_pair,		"Pair with a remote device"	},
	{ "cancelpair",		"[-t type] <remote address>",
		cmd_cancel_pair,	"Cancel pairing"		},
	{ "unpair",		"[-t type] <remote address>",
		cmd_unpair,		"Unpair device"			},
	{ "keys",		NULL,
		cmd_keys,		"Load Link Keys"		},
	{ "ltks",		NULL,
		cmd_ltks,		"Load Long Term Keys"		},
	{ "irks",		"[--local index] [--file file path]",
		cmd_irks,		"Load Identity Resolving Keys"	},
	{ "block",		"[-t type] <remote address>",
		cmd_block,		"Block Device"			},
	{ "unblock",		"[-t type] <remote address>",
		cmd_unblock,		"Unblock Device"		},
	{ "add-uuid",		"<UUID> <service class hint>",
		cmd_add_uuid,		"Add UUID"			},
	{ "rm-uuid",		"<UUID>",
		cmd_remove_uuid,	"Remove UUID"			},
	{ "clr-uuids",		NULL,
		cmd_clr_uuids,		"Clear UUIDs"			},
	{ "local-oob",		NULL,
		cmd_local_oob,		"Local OOB data"		},
	{ "remote-oob",		"[-t <addr_type>] [-r <rand192>] "
				"[-h <hash192>] [-R <rand256>] "
				"[-H <hash256>] <addr>",
		cmd_remote_oob,		"Remote OOB data"		},
	{ "did",		"<source>:<vendor>:<product>:<version>",
		cmd_did,		"Set Device ID"			},
	{ "static-addr",	"<address>",
		cmd_static_addr,	"Set static address"		},
	{ "public-addr",	"<address>",
		cmd_public_addr,	"Set public address"		},
	{ "ext-config",		"<on/off>",
		cmd_ext_config,		"External configuration"	},
	{ "debug-keys",		"<on/off>",
		cmd_debug_keys,		"Toogle debug keys"		},
	{ "conn-info",		"[-t type] <remote address>",
		cmd_conn_info,		"Get connection information"	},
	{ "io-cap",		"<cap>",
		cmd_io_cap,		"Set IO Capability"		},
	{ "scan-params",	"<interval> <window>",
		cmd_scan_params,	"Set Scan Parameters"		},
	{ "get-clock",		"[address]",
		cmd_clock_info,		"Get Clock Information"		},
	{ "add-device", 	"[-a action] [-t type] <address>",
		cmd_add_device,		"Add Device"			},
	{ "del-device", 	"[-t type] <address>",
		cmd_del_device,		"Remove Device"			},
	{ "clr-devices",	NULL,
		cmd_clr_devices,	"Clear Devices"			},
	{ "bredr-oob",		NULL,
		cmd_bredr_oob,		"Local OOB data (BR/EDR)"	},
	{ "le-oob",		NULL,
		cmd_le_oob,		"Local OOB data (LE)"		},
	{ "advinfo",		NULL,
		cmd_advinfo,		"Show advertising features"	},
	{ "advsize",		"[options] <instance_id>",
		cmd_advsize,		"Show advertising size info"	},
	{ "add-adv",		"[options] <instance_id>",
		cmd_add_adv,		"Add advertising instance"	},
	{ "rm-adv",		"<instance_id>",
		cmd_rm_adv,		"Remove advertising instance"	},
	{ "clr-adv",		NULL,
		cmd_clr_adv,		"Clear advertising instances"	},
	{ "appearance",		"<appearance>",
		cmd_appearance,		"Set appearance"		},
	{ "phy",		"[LE1MTX] [LE1MRX] [LE2MTX] [LE2MRX] "
				"[LECODEDTX] [LECODEDRX] "
				"[BR1M1SLOT] [BR1M3SLOT] [BR1M5SLOT]"
				"[EDR2M1SLOT] [EDR2M3SLOT] [EDR2M5SLOT]"
				"[EDR3M1SLOT] [EDR3M3SLOT] [EDR3M5SLOT]",
		cmd_phy,		"Get/Set PHY Configuration"	},
	{ "wbs",		"<on/off>",
		cmd_wbs,		"Toggle Wideband-Speech support"},
	{ "secinfo",		NULL,
		cmd_secinfo,		"Show security information"	},
	{ "expinfo",		NULL,
		cmd_expinfo,		"Show experimental features"	},
	{ "exp-debug",		"<on/off>",
		cmd_exp_debug,		"Set debug feature"		},
	{ "read-sysconfig",	NULL,
		cmd_read_sysconfig,	"Read System Configuration"	},
	{ "set-sysconfig",	"<-v|-h> [options...]",
		cmd_set_sysconfig,	"Set System Configuration"	},
	{} },
};

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	print("%s%s", prefix, str);
}

static const char *index_option;

static struct option main_options[] = {
	{ "index",	1, 0, 'i' },
	{ 0, 0, 0, 0 }
};

static const char **optargs[] = {
	&index_option
};

static const char *help[] = {
	"Specify adapter index\n"
};

static const struct bt_shell_opt opt = {
	.options = main_options,
	.optno = sizeof(main_options) / sizeof(struct option),
	.optstr = "i:V",
	.optarg = optargs,
	.help = help,
};

int main(int argc, char *argv[])
{
	int status;

	bt_shell_init(argc, argv, &opt);
	bt_shell_set_menu(&main_menu);
	bt_shell_add_submenu(&monitor_menu);

	mgmt = mgmt_new_default();
	if (!mgmt) {
		fprintf(stderr, "Unable to open mgmt_socket\n");
		return EXIT_FAILURE;
	}

	if (getenv("MGMT_DEBUG"))
		mgmt_set_debug(mgmt, mgmt_debug, "mgmt: ", NULL);

	if (index_option)
		set_index(index_option);

	register_mgmt_callbacks(mgmt, mgmt_index);

	bt_shell_attach(fileno(stdin));
	update_prompt(mgmt_index);
	status = bt_shell_run();

	mgmt_cancel_all(mgmt);
	mgmt_unregister_all(mgmt);
	mgmt_unref(mgmt);

	return status;
}
