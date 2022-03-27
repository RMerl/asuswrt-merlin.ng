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
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>

#include <sys/un.h>
#include <sys/wait.h>
#include <sys/signalfd.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"
#include "src/shared/util.h"
#include "src/shared/tester.h"
#include "src/shared/mgmt.h"
#include "src/shared/queue.h"
#include "emulator/bthost.h"
#include "monitor/bt.h"
#include "tester-main.h"

static char exec_dir[PATH_MAX + 1];

static gint scheduled_cbacks_num;

#define EMULATOR_SIGNAL_TIMEOUT 2 /* in seconds */
#define EMULATOR_SIGNAL "emulator_started"

#define BT_TRANSPORT_UNKNOWN	0x00

static struct {
	uint16_t cb_num;
	const char *str;
} cb_table[] = {
	DBG_CB(CB_BT_NONE),
	DBG_CB(CB_BT_ADAPTER_STATE_CHANGED),
	DBG_CB(CB_BT_ADAPTER_PROPERTIES),
	DBG_CB(CB_BT_REMOTE_DEVICE_PROPERTIES),
	DBG_CB(CB_BT_DEVICE_FOUND),
	DBG_CB(CB_BT_DISCOVERY_STATE_CHANGED),
	DBG_CB(CB_BT_PIN_REQUEST),
	DBG_CB(CB_BT_SSP_REQUEST),
	DBG_CB(CB_BT_BOND_STATE_CHANGED),
	DBG_CB(CB_BT_ACL_STATE_CHANGED),
	DBG_CB(CB_BT_THREAD_EVT),
	DBG_CB(CB_BT_DUT_MODE_RECV),
	DBG_CB(CB_BT_LE_TEST_MODE),

	/* Hidhost cb */
	DBG_CB(CB_HH_CONNECTION_STATE),
	DBG_CB(CB_HH_HID_INFO),
	DBG_CB(CB_HH_PROTOCOL_MODE),
	DBG_CB(CB_HH_IDLE_TIME),
	DBG_CB(CB_HH_GET_REPORT),
	DBG_CB(CB_HH_VIRTUAL_UNPLUG),

	/* PAN cb */
	DBG_CB(CB_PAN_CONTROL_STATE),
	DBG_CB(CB_PAN_CONNECTION_STATE),

	/* HDP cb */
	DBG_CB(CB_HDP_APP_REG_STATE),
	DBG_CB(CB_HDP_CHANNEL_STATE),

	/* A2DP cb */
	DBG_CB(CB_A2DP_CONN_STATE),
	DBG_CB(CB_A2DP_AUDIO_STATE),

	/* AVRCP */
	DBG_CB(CB_AVRCP_PLAY_STATUS_REQ),
	DBG_CB(CB_AVRCP_PLAY_STATUS_RSP),
	DBG_CB(CB_AVRCP_REG_NOTIF_REQ),
	DBG_CB(CB_AVRCP_REG_NOTIF_RSP),
	DBG_CB(CB_AVRCP_GET_ATTR_REQ),
	DBG_CB(CB_AVRCP_GET_ATTR_RSP),

	/* Gatt client */
	DBG_CB(CB_GATTC_REGISTER_CLIENT),
	DBG_CB(CB_GATTC_SCAN_RESULT),
	DBG_CB(CB_GATTC_OPEN),
	DBG_CB(CB_GATTC_CLOSE),
	DBG_CB(CB_GATTC_SEARCH_COMPLETE),
	DBG_CB(CB_GATTC_SEARCH_RESULT),
	DBG_CB(CB_GATTC_GET_CHARACTERISTIC),
	DBG_CB(CB_GATTC_GET_DESCRIPTOR),
	DBG_CB(CB_GATTC_GET_INCLUDED_SERVICE),
	DBG_CB(CB_GATTC_REGISTER_FOR_NOTIFICATION),
	DBG_CB(CB_GATTC_NOTIFY),
	DBG_CB(CB_GATTC_READ_CHARACTERISTIC),
	DBG_CB(CB_GATTC_WRITE_CHARACTERISTIC),
	DBG_CB(CB_GATTC_READ_DESCRIPTOR),
	DBG_CB(CB_GATTC_WRITE_DESCRIPTOR),
	DBG_CB(CB_GATTC_EXECUTE_WRITE),
	DBG_CB(CB_GATTC_READ_REMOTE_RSSI),
	DBG_CB(CB_GATTC_LISTEN),

	/* Gatt server */
	DBG_CB(CB_GATTS_REGISTER_SERVER),
	DBG_CB(CB_GATTS_CONNECTION),
	DBG_CB(CB_GATTS_SERVICE_ADDED),
	DBG_CB(CB_GATTS_INCLUDED_SERVICE_ADDED),
	DBG_CB(CB_GATTS_CHARACTERISTIC_ADDED),
	DBG_CB(CB_GATTS_DESCRIPTOR_ADDED),
	DBG_CB(CB_GATTS_SERVICE_STARTED),
	DBG_CB(CB_GATTS_SERVICE_STOPPED),
	DBG_CB(CB_GATTS_SERVICE_DELETED),
	DBG_CB(CB_GATTS_REQUEST_READ),
	DBG_CB(CB_GATTS_REQUEST_WRITE),
	DBG_CB(CB_GATTS_REQUEST_EXEC_WRITE),
	DBG_CB(CB_GATTS_RESPONSE_CONFIRMATION),
	DBG_CB(CB_GATTS_INDICATION_SEND),

	/* Map client */
	DBG_CB(CB_MAP_CLIENT_REMOTE_MAS_INSTANCES),

	/* Emulator callbacks */
	DBG_CB(CB_EMU_CONFIRM_SEND_DATA),
	DBG_CB(CB_EMU_ENCRYPTION_ENABLED),
	DBG_CB(CB_EMU_ENCRYPTION_DISABLED),
	DBG_CB(CB_EMU_CONNECTION_REJECTED),
	DBG_CB(CB_EMU_VALUE_INDICATION),
	DBG_CB(CB_EMU_VALUE_NOTIFICATION),
	DBG_CB(CB_EMU_READ_RESPONSE),
	DBG_CB(CB_EMU_WRITE_RESPONSE),
	DBG_CB(CB_EMU_ATT_ERROR),
};

static gboolean check_callbacks_called(gpointer user_data)
{
	/*
	 * Wait for all callbacks scheduled in current test context to execute
	 * in main loop. This will avoid late callback calls after test case has
	 * already failed or timed out.
	 */

	if (g_atomic_int_get(&scheduled_cbacks_num) == 0) {
		tester_teardown_complete();
		return FALSE;
	} else if (scheduled_cbacks_num < 0) {
		tester_warn("Unscheduled callback called!");
		return FALSE;
	}

	return TRUE;
}

static void check_daemon_term(void)
{
	int status;
	pid_t pid;
	struct test_data *data = tester_get_data();

	if (!data)
		return;

	pid = waitpid(data->bluetoothd_pid, &status, WNOHANG);
	if (pid != data->bluetoothd_pid)
		return;

	data->bluetoothd_pid = 0;

	if (WIFEXITED(status) && (WEXITSTATUS(status) == EXIT_SUCCESS)) {
		g_idle_add(check_callbacks_called, NULL);
		return;
	}

	tester_warn("Unexpected Daemon shutdown with status %d", status);
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGCHLD:
		check_daemon_term();
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
		return 0;

	fd = signalfd(-1, &mask, 0);
	if (fd < 0)
		return 0;

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

static void test_post_teardown(const void *test_data)
{
	struct test_data *data = tester_get_data();

	/* remove hook for encryption change */
	hciemu_del_hook(data->hciemu, HCIEMU_HOOK_POST_EVT, 0x08);

	hciemu_unref(data->hciemu);
	data->hciemu = NULL;

	g_source_remove(data->signalfd);
	data->signalfd = 0;
}

static void bluetoothd_start(int hci_index)
{
	char prg_name[PATH_MAX + 1];
	char index[8];
	char *prg_argv[5];

	snprintf(prg_name, sizeof(prg_name), "%s/%s", exec_dir, "bluetoothd");
	snprintf(index, sizeof(index), "%d", hci_index);

	prg_argv[0] = prg_name;
	prg_argv[1] = "-i";
	prg_argv[2] = index;
	prg_argv[3] = "-d";
	prg_argv[4] = NULL;

	if (!tester_use_debug())
		fclose(stderr);

	execve(prg_argv[0], prg_argv, NULL);
}

static void emulator(int pipe, int hci_index)
{
	static const char SYSTEM_SOCKET_PATH[] = "\0android_system";
	char buf[1024];
	struct sockaddr_un addr;
	struct timeval tv;
	int fd;
	ssize_t len;

	fd = socket(PF_LOCAL, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		goto failed;

	tv.tv_sec = EMULATOR_SIGNAL_TIMEOUT;
	tv.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, SYSTEM_SOCKET_PATH, sizeof(SYSTEM_SOCKET_PATH));

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind system socket");
		goto failed;
	}

	len = write(pipe, EMULATOR_SIGNAL, sizeof(EMULATOR_SIGNAL));
	if (len != sizeof(EMULATOR_SIGNAL))
		goto failed;

	memset(buf, 0, sizeof(buf));

	len = read(fd, buf, sizeof(buf));
	if (len <= 0 || strcmp(buf, "bluetooth.start=daemon"))
		goto failed;

	close(pipe);
	close(fd);
	return bluetoothd_start(hci_index);

failed:
	close(pipe);

	if (fd >= 0)
		close(fd);
}

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	tester_print("%s%s", prefix, str);
}

static bool hciemu_post_encr_hook(const void *data, uint16_t len,
							void *user_data)
{
	struct step *step;

	/*
	 * Expected data: status (1 octet) + conn. handle (2 octets) +
	 * encryption flag (1 octet)
	 */
	if (len < 4)
		return true;

	step = g_new0(struct step, 1);

	step->callback = ((uint8_t *)data)[3] ? CB_EMU_ENCRYPTION_ENABLED :
						CB_EMU_ENCRYPTION_DISABLED;

	schedule_callback_verification(step);
	return true;
}

static void read_info_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct mgmt_rp_read_info *rp = param;
	char addr[18];
	uint16_t manufacturer;
	uint32_t supported_settings, current_settings;

	tester_print("Read Info callback");
	tester_print("  Status: 0x%02x", status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	ba2str(&rp->bdaddr, addr);
	manufacturer = btohs(rp->manufacturer);
	supported_settings = btohl(rp->supported_settings);
	current_settings = btohl(rp->current_settings);

	tester_print("  Address: %s", addr);
	tester_print("  Version: 0x%02x", rp->version);
	tester_print("  Manufacturer: 0x%04x", manufacturer);
	tester_print("  Supported settings: 0x%08x", supported_settings);
	tester_print("  Current settings: 0x%08x", current_settings);
	tester_print("  Class: 0x%02x%02x%02x",
			rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);
	tester_print("  Name: %s", rp->name);
	tester_print("  Short name: %s", rp->short_name);

	if (strcmp(hciemu_get_address(data->hciemu), addr)) {
		tester_pre_setup_failed();
		return;
	}

	/* set hook for encryption change */
	hciemu_add_hook(data->hciemu, HCIEMU_HOOK_POST_EVT, 0x08,
						hciemu_post_encr_hook, NULL);

	tester_pre_setup_complete();
}

static void index_added_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Added callback");
	tester_print("  Index: 0x%04x", index);

	data->mgmt_index = index;

	mgmt_send(data->mgmt, MGMT_OP_READ_INFO, data->mgmt_index, 0, NULL,
					read_info_callback, NULL, NULL);
}

static void index_removed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Removed callback");
	tester_print("  Index: 0x%04x", index);

	if (index != data->mgmt_index)
		return;

	mgmt_unregister_index(data->mgmt, data->mgmt_index);

	mgmt_unref(data->mgmt);
	data->mgmt = NULL;

	tester_post_teardown_complete();
}

static void read_index_list_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Read Index List callback");
	tester_print("  Status: 0x%02x", status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	mgmt_register(data->mgmt, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
					index_added_callback, NULL, NULL);

	mgmt_register(data->mgmt, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
					index_removed_callback, NULL, NULL);

	data->hciemu = hciemu_new(data->hciemu_type);
	if (!data->hciemu) {
		tester_warn("Failed to setup HCI emulation");
		tester_pre_setup_failed();
		return;
	}

	tester_print("New hciemu instance created");
}

static void test_pre_setup(const void *test_data)
{
	struct test_data *data = tester_get_data();

	data->signalfd = setup_signalfd();
	if (!data->signalfd) {
		tester_warn("Failed to setup signalfd");
		tester_pre_setup_failed();
		return;
	}

	data->mgmt = mgmt_new_default();
	if (!data->mgmt) {
		tester_warn("Failed to setup management interface");
		tester_pre_setup_failed();
		return;
	}

	if (!tester_use_debug())
		fclose(stderr);
	else
		mgmt_set_debug(data->mgmt, mgmt_debug, "mgmt: ", NULL);

	mgmt_send(data->mgmt, MGMT_OP_READ_INDEX_LIST, MGMT_INDEX_NONE, 0,
				NULL, read_index_list_callback, NULL, NULL);
}

static bool match_property(bt_property_t *exp_prop, bt_property_t *rec_prop,
								int prop_num)
{
	if (exp_prop->type && (exp_prop->type != rec_prop->type))
		return 0;

	if (exp_prop->len && (exp_prop->len != rec_prop->len)) {
		tester_debug("Property [%d] len don't match! received=%d, "
					"expected=%d", prop_num, rec_prop->len,
					exp_prop->len);
		return 0;
	}

	if (exp_prop->val && memcmp(exp_prop->val, rec_prop->val,
							exp_prop->len)) {
		tester_debug("Property [%d] value don't match!", prop_num);
		return 0;
	}

	return 1;
}

static bool match_mas_inst(btmce_mas_instance_t *exp_inst,
				btmce_mas_instance_t *rec_inst, int inst_num)
{
	if (exp_inst->id && (exp_inst->id != rec_inst->id)) {
		tester_debug("MAS inst. [%d] id missmatch %d vs %d", inst_num,
						rec_inst->id, exp_inst->id);
		return 0;
	}

	if (exp_inst->scn && (exp_inst->scn != rec_inst->scn)) {
		tester_debug("MAS inst. [%d] scn missmatch %d vs %d", inst_num,
						rec_inst->scn, exp_inst->scn);
		return 0;
	}

	if (exp_inst->msg_types &&
			(exp_inst->msg_types != rec_inst->msg_types)) {
		tester_debug("Mas inst. [%d] mesg type missmatch %d vs %d",
					inst_num, rec_inst->scn, exp_inst->scn);
		return 0;
	}

	if (exp_inst->p_name && memcmp(exp_inst->p_name, rec_inst->p_name,
						strlen(exp_inst->p_name))) {
		tester_debug("Mas inst. [%d] name don't match!", inst_num);
		return 0;
	}

	return 1;
}

static int verify_property(bt_property_t *exp_props, int exp_num_props,
				bt_property_t *rec_props, int rec_num_props)
{
	int i, j;
	int exp_prop_to_find = exp_num_props;

	if (rec_num_props == 0)
		return 1;

	if (exp_num_props == 0) {
		tester_debug("Wrong number of expected properties given");
		tester_test_failed();
		return 1;
	}

	/* Get first exp prop to match and search for it */
	for (i = 0; i < exp_num_props; i++) {
		for (j = 0; j < rec_num_props; j++) {
			if (match_property(&exp_props[i], &rec_props[j], i)) {
				exp_prop_to_find--;
				break;
			}
		}
	}

	return exp_prop_to_find;
}

static int verify_mas_inst(btmce_mas_instance_t *exp_inst, int exp_num_inst,
						btmce_mas_instance_t *rec_inst,
						int rec_num_inst)
{
	int i, j;
	int exp_inst_to_find = exp_num_inst;

	if (rec_num_inst == 0)
		return 1;

	if (exp_num_inst == 0) {
		tester_debug("Wrong number of expected MAS instances given");
		tester_test_failed();
		return 1;
	}

	for (i = 0; i < exp_num_inst; i++) {
		for (j = 0; j < rec_num_inst; j++) {
			if (match_mas_inst(&exp_inst[i], &rec_inst[i], i)) {
				exp_inst_to_find--;
				break;
			}
		}
	}

	return exp_inst_to_find;
}

/*
 * Check each test case step if test case expected
 * data is set and match it with expected result.
 */

static bool verify_gatt_ids(btgatt_gatt_id_t *a, btgatt_gatt_id_t *b)
{

	if (memcmp(&a->uuid, &b->uuid, sizeof(bt_uuid_t)))
		return false;

	if (a->inst_id != b->inst_id)
		return false;

	return true;
}

static bool verify_services(btgatt_srvc_id_t *a, btgatt_srvc_id_t *b)
{
	if (a->is_primary != b->is_primary)
		return false;

	return verify_gatt_ids(&a->id, &b->id);
}

static bool match_data(struct step *step)
{
	struct test_data *data = tester_get_data();
	const struct step *exp;

	exp = queue_peek_head(data->steps);

	if (!exp) {
		/* Can occure while test passed already */
		tester_debug("Cannot get step to match");
		return false;
	}

	if (exp->action_status != step->action_status) {
		tester_debug("Action status don't match");
		return false;
	}

	if (!exp->callback && !step->callback)
		return true;

	if (exp->callback != step->callback) {
		tester_debug("Callback type mismatch: %s vs %s",
						cb_table[step->callback].str,
						cb_table[exp->callback].str);
		return false;
	}

	if (exp->callback_result.state != step->callback_result.state) {
		tester_debug("Callback state mismatch: %d vs %d",
						step->callback_result.state,
						exp->callback_result.state);
		return false;
	}

	if (exp->callback_result.status != step->callback_result.status) {
		tester_debug("Callback status mismatch: %d vs %d",
						step->callback_result.status,
						exp->callback_result.status);
		return false;
	}

	if (exp->callback_result.mode != step->callback_result.mode) {
		tester_debug("Callback mode mismatch: %02x vs %02x",
						step->callback_result.mode,
						exp->callback_result.mode);
		return false;
	}

	if (exp->callback_result.report_size !=
					step->callback_result.report_size) {
		tester_debug("Callback report size mismatch: %d vs %d",
					step->callback_result.report_size,
					exp->callback_result.report_size);
		return false;
	}

	if (exp->callback_result.ctrl_state !=
					step->callback_result.ctrl_state) {
		tester_debug("Callback ctrl state mismatch: %d vs %d",
					step->callback_result.ctrl_state,
					exp->callback_result.ctrl_state);
		return false;
	}

	if (exp->callback_result.conn_state !=
					step->callback_result.conn_state) {
		tester_debug("Callback connection state mismatch: %d vs %d",
					step->callback_result.conn_state,
					exp->callback_result.conn_state);
		return false;
	}

	if (exp->callback_result.local_role !=
					step->callback_result.local_role) {
		tester_debug("Callback local_role mismatch: %d vs %d",
					step->callback_result.local_role,
					exp->callback_result.local_role);
		return false;
	}

	if (exp->callback_result.remote_role !=
					step->callback_result.remote_role) {
		tester_debug("Callback remote_role mismatch: %d vs %d",
					step->callback_result.remote_role,
					exp->callback_result.remote_role);
		return false;
	}

	if (exp->callback_result.app_id != step->callback_result.app_id) {
		tester_debug("Callback app_id mismatch: %d vs %d",
						step->callback_result.app_id,
						exp->callback_result.app_id);
		return false;
	}

	if (exp->callback_result.channel_id !=
					step->callback_result.channel_id) {
		tester_debug("Callback channel_id mismatch: %d vs %d",
					step->callback_result.channel_id,
					exp->callback_result.channel_id);
		return false;
	}

	if (exp->callback_result.mdep_cfg_index !=
					step->callback_result.mdep_cfg_index) {
		tester_debug("Callback mdep_cfg_index mismatch: %d vs %d",
					step->callback_result.mdep_cfg_index,
					exp->callback_result.mdep_cfg_index);
		return false;
	}

	if (exp->callback_result.app_state != step->callback_result.app_state) {
		tester_debug("Callback app_state mismatch: %d vs %d",
						step->callback_result.app_state,
						exp->callback_result.app_state);
		return false;
	}

	if (exp->callback_result.channel_state !=
					step->callback_result.channel_state) {
		tester_debug("Callback channel_state mismatch: %d vs %d",
					step->callback_result.channel_state,
					exp->callback_result.channel_state);
		return false;
	}

	if (exp->callback_result.av_conn_state !=
					step->callback_result.av_conn_state) {
		tester_debug("Callback av conn state mismatch: 0x%x vs 0x%x",
					step->callback_result.av_conn_state,
					exp->callback_result.av_conn_state);
		return false;
	}

	if (exp->callback_result.av_audio_state !=
					step->callback_result.av_audio_state) {
		tester_debug("Callback av audio state mismatch: 0x%x vs 0x%x",
					step->callback_result.av_audio_state,
					exp->callback_result.av_audio_state);
		return false;
	}

	if (exp->callback_result.song_length !=
					step->callback_result.song_length) {
		tester_debug("Callback song_length mismatch: 0x%x vs 0x%x",
					step->callback_result.song_length,
					exp->callback_result.song_length);
		return false;
	}

	if (exp->callback_result.song_position !=
					step->callback_result.song_position) {
		tester_debug("Callback song_position mismatch: 0x%x vs 0x%x",
					step->callback_result.song_position,
					exp->callback_result.song_position);
		return false;
	}

	if (exp->callback_result.play_status !=
					step->callback_result.play_status) {
		tester_debug("Callback play_status mismatch: 0x%x vs 0x%x",
					step->callback_result.play_status,
					exp->callback_result.play_status);
		return false;
	}

	if (exp->callback_result.rc_index !=
					step->callback_result.rc_index) {
		tester_debug("Callback rc_index mismatch");
		return false;
	}

	if (exp->callback_result.num_of_attrs !=
					step->callback_result.num_of_attrs) {
		tester_debug("Callback rc num of attrs mismatch");
		return false;
	}

	if (exp->callback_result.attrs) {
		if (memcmp(step->callback_result.attrs,
				exp->callback_result.attrs,
				exp->callback_result.num_of_attrs *
				sizeof(btrc_element_attr_val_t))) {
			tester_debug("Callback rc element attributes doesn't match");
			return false;
		}
	}

	if (exp->callback_result.pairing_variant !=
					step->callback_result.pairing_variant) {
		tester_debug("Callback pairing result mismatch: %d vs %d",
					step->callback_result.pairing_variant,
					exp->callback_result.pairing_variant);
		return false;
	}

	if (exp->callback_result.adv_data != step->callback_result.adv_data) {
		tester_debug("Callback adv. data status mismatch: %d vs %d",
						step->callback_result.adv_data,
						exp->callback_result.adv_data);
		return false;
	}

	if (exp->callback_result.conn_id != step->callback_result.conn_id) {
		tester_debug("Callback conn_id mismatch: %d vs %d",
						step->callback_result.conn_id,
						exp->callback_result.conn_id);
		return false;
	}

	if (exp->callback_result.gatt_app_id !=
					step->callback_result.gatt_app_id) {
		tester_debug("Callback gatt_app_id mismatch: %d vs %d",
					step->callback_result.gatt_app_id,
					exp->callback_result.gatt_app_id);
		return false;
	}

	if (exp->callback_result.properties &&
			verify_property(exp->callback_result.properties,
					exp->callback_result.num_properties,
					step->callback_result.properties,
					step->callback_result.num_properties)) {
		tester_debug("Gatt properties don't match");
		return false;
	}

	if (exp->callback_result.service &&
			!verify_services(step->callback_result.service,
						exp->callback_result.service)) {
		tester_debug("Gatt service doesn't match");
		return false;
	}

	if (exp->callback_result.characteristic) {
		btgatt_gatt_id_t *a;
		btgatt_gatt_id_t *b;
		a = step->callback_result.characteristic;
		b = exp->callback_result.characteristic;

		if (!verify_gatt_ids(a, b)) {
			tester_debug("Gatt char doesn't match");
			return false;
		}
	}

	if (exp->callback_result.char_prop != step->callback_result.char_prop) {
		tester_debug("Gatt char prop mismatch: %d vs %d",
						step->callback_result.char_prop,
						exp->callback_result.char_prop);
		return false;
	}

	if (exp->callback_result.descriptor) {
		btgatt_gatt_id_t *a;
		btgatt_gatt_id_t *b;
		a = step->callback_result.descriptor;
		b = exp->callback_result.descriptor;

		if (!verify_gatt_ids(a, b)) {
			tester_debug("Gatt desc doesn't match");
			return false;
		}
	}

	if (exp->callback_result.included) {
		if (!verify_services(step->callback_result.included,
					exp->callback_result.included)) {
			tester_debug("Gatt include srvc doesn't match");
			return false;
		}
	}

	if (exp->callback_result.read_params) {
		if (memcmp(step->callback_result.read_params,
				exp->callback_result.read_params,
				sizeof(btgatt_read_params_t))) {
			tester_debug("Gatt read_param doesn't match");
			return false;
		}
	}

	if (exp->callback_result.write_params) {
		if (memcmp(step->callback_result.write_params,
				exp->callback_result.write_params,
				sizeof(btgatt_write_params_t))) {
			tester_debug("Gatt write_param doesn't match");
			return false;
		}

		if (exp->callback_result.notification_registered !=
				step->callback_result.notification_registered) {
			tester_debug("Gatt registered flag mismatch");
			return false;
		}

		if (exp->callback_result.notify_params) {
			if (memcmp(step->callback_result.notify_params,
					exp->callback_result.notify_params,
					sizeof(btgatt_notify_params_t))) {
				tester_debug("Gatt notify_param doesn't match");
				return false;
			}
		}
	}

	if (exp->callback_result.connected !=
				step->callback_result.connected) {
		tester_debug("Gatt server conn status mismatch: %d vs %d",
						step->callback_result.connected,
						exp->callback_result.connected);
		return false;
	}

	if (exp->callback_result.attr_handle &&
					step->callback_result.attr_handle)
		if (*exp->callback_result.attr_handle !=
					*step->callback_result.attr_handle) {
			tester_debug("Gatt attribute handle mismatch: %d vs %d",
					*step->callback_result.attr_handle,
					*exp->callback_result.attr_handle);
			return false;
		}

	if (exp->callback_result.srvc_handle &&
					step->callback_result.srvc_handle)
		if (*exp->callback_result.srvc_handle !=
					*step->callback_result.srvc_handle) {
			tester_debug("Gatt service handle mismatch: %d vs %d",
					*step->callback_result.srvc_handle,
					*exp->callback_result.srvc_handle);
			return false;
		}

	if (exp->callback_result.inc_srvc_handle &&
					step->callback_result.inc_srvc_handle)
		if (*exp->callback_result.inc_srvc_handle !=
				*step->callback_result.inc_srvc_handle) {
			tester_debug("Gatt inc. srvc handle mismatch: %d vs %d",
					*step->callback_result.inc_srvc_handle,
					*exp->callback_result.inc_srvc_handle);
			return false;
		}

	if (exp->callback_result.uuid && step->callback_result.uuid)
		if (memcmp(exp->callback_result.uuid,
					step->callback_result.uuid,
					sizeof(*exp->callback_result.uuid))) {
			tester_debug("Uuid mismatch");
			return false;
		}

	if (exp->callback_result.trans_id != step->callback_result.trans_id) {
		tester_debug("Gatt trans id mismatch: %d vs %d",
						exp->callback_result.trans_id,
						step->callback_result.trans_id);
		return false;
	}

	if (exp->callback_result.offset != step->callback_result.offset) {
		tester_debug("Gatt offset mismatch: %d vs %d",
						exp->callback_result.offset,
						step->callback_result.offset);
		return false;
	}

	if (exp->callback_result.is_long != step->callback_result.is_long) {
		tester_debug("Gatt is long attr value flag mismatch: %d vs %d",
						exp->callback_result.is_long,
						step->callback_result.is_long);
		return false;
	}

	if (exp->callback_result.length > 0) {
		if (exp->callback_result.length !=
						step->callback_result.length) {
			tester_debug("Gatt attr length mismatch: %d vs %d",
						exp->callback_result.length,
						step->callback_result.length);
			return false;
		}
		if (!exp->callback_result.value ||
						!step->callback_result.value) {
			tester_debug("Gatt attr values are wrong set");
			return false;
		}
		if (!memcmp(exp->callback_result.value,
						step->callback_result.value,
						exp->callback_result.length)) {
			tester_debug("Gatt attr value mismatch");
			return false;
		}
	}

	if (exp->callback_result.need_rsp != step->callback_result.need_rsp) {
		tester_debug("Gatt need response value flag mismatch: %d vs %d",
						exp->callback_result.need_rsp,
						step->callback_result.need_rsp);
		return false;
	}

	if (exp->callback_result.is_prep != step->callback_result.is_prep) {
		tester_debug("Gatt is prepared value flag mismatch: %d vs %d",
						exp->callback_result.is_prep,
						step->callback_result.is_prep);
		return false;
	}

	if (exp->callback_result.num_mas_instances !=
				step->callback_result.num_mas_instances) {
		tester_debug("Mas instance count mismatch: %d vs %d",
				exp->callback_result.num_mas_instances,
				step->callback_result.num_mas_instances);
		return false;
	}

	if (exp->callback_result.mas_instances &&
		verify_mas_inst(exp->callback_result.mas_instances,
				exp->callback_result.num_mas_instances,
				step->callback_result.mas_instances,
				step->callback_result.num_mas_instances)) {
		tester_debug("Mas instances don't match");
		return false;
	}

	if (exp->callback_result.error != step->callback_result.error) {
		tester_debug("Err mismatch: %d vs %d",
				exp->callback_result.error,
				step->callback_result.error);
		return false;
	}

	if (exp->store_srvc_handle)
		memcpy(exp->store_srvc_handle,
					step->callback_result.srvc_handle,
					sizeof(*exp->store_srvc_handle));

	if (exp->store_char_handle)
		memcpy(exp->store_char_handle,
					step->callback_result.char_handle,
					sizeof(*exp->store_char_handle));

	return true;
}

static void init_test_steps(struct test_data *data)
{
	const struct test_case *test_steps = data->test_data;
	int i = 0;

	for (i = 0; i < test_steps->step_num; i++)
		queue_push_tail(data->steps, (void *) &(test_steps->step[i]));

	tester_print("tester: Number of test steps=%d",
						queue_length(data->steps));
}

/*
 * Each test case step should be verified, if match with
 * expected result tester should go to next test step.
 */
static void verify_step(struct step *step, queue_destroy_func_t cleanup_cb)
{
	struct test_data *data = tester_get_data();
	const struct test_case *test_steps = data->test_data;
	struct step *next_step;

	tester_debug("tester: STEP[%d] check",
			test_steps->step_num-queue_length(data->steps) + 1);

	if (step && !match_data(step)) {
		if (cleanup_cb)
			cleanup_cb(step);

		return;
	}

	queue_pop_head(data->steps);

	if (cleanup_cb)
		cleanup_cb(step);

	tester_debug("tester: STEP[%d] pass",
			test_steps->step_num-queue_length(data->steps));

	if (queue_isempty(data->steps)) {
		tester_print("tester: All steps done, passing");
		tester_test_passed();

		return;
	}

	/* goto next step action if declared in step */
	next_step = queue_peek_head(data->steps);

	if (next_step->action)
		next_step->action();
}

/*
 * NOTICE:
 * Its mandatory for callback to set proper step.callback value so that
 * step verification could pass and move to next test step
 */

static void free_properties(struct step *step)
{
	bt_property_t *properties = step->callback_result.properties;
	int num_properties = step->callback_result.num_properties;
	int i;

	for (i = 0; i < num_properties; i++)
		g_free(properties[i].val);

	g_free(properties);
}

static void free_mas_instances(struct step *step)
{
	btmce_mas_instance_t *mas_instances;
	int num_instances;
	int i;

	mas_instances = step->callback_result.mas_instances;
	num_instances = step->callback_result.num_mas_instances;

	for (i = 0; i < num_instances; i++)
		g_free(mas_instances[i].p_name);

	g_free(mas_instances);
}

static void destroy_callback_step(void *data)
{
	struct step *step = data;

	if (step->callback_result.properties)
		free_properties(step);

	if (step->callback_result.service)
		free(step->callback_result.service);

	if (step->callback_result.characteristic)
		free(step->callback_result.characteristic);

	if (step->callback_result.descriptor)
		free(step->callback_result.descriptor);

	if (step->callback_result.included)
		free(step->callback_result.included);

	if (step->callback_result.read_params)
		free(step->callback_result.read_params);

	if (step->callback_result.write_params)
		free(step->callback_result.write_params);

	if (step->callback_result.notify_params)
		free(step->callback_result.notify_params);

	if (step->callback_result.srvc_handle)
		free(step->callback_result.srvc_handle);

	if (step->callback_result.inc_srvc_handle)
		free(step->callback_result.inc_srvc_handle);

	if (step->callback_result.uuid)
		free(step->callback_result.uuid);

	if (step->callback_result.char_handle)
		free(step->callback_result.char_handle);

	if (step->callback_result.desc_handle)
		free(step->callback_result.desc_handle);

	if (step->callback_result.attr_handle)
		free(step->callback_result.attr_handle);

	if (step->callback_result.value)
		free(step->callback_result.value);

	if (step->callback_result.mas_instances)
		free_mas_instances(step);

	g_free(step);
	g_atomic_int_dec_and_test(&scheduled_cbacks_num);
}

static gboolean verify_action(gpointer user_data)
{
	struct step *step = user_data;

	verify_step(step, g_free);

	return FALSE;
}

static gboolean verify_callback(gpointer user_data)
{
	struct test_data *data = tester_get_data();
	struct step *step = user_data;

	/* Return if callback came when all steps are already verified */
	if (queue_isempty(data->steps)) {
		destroy_callback_step(step);
		return FALSE;
	}

	/*
	 * TODO: This may call action from next step before callback data
	 * from previous step was freed.
	 */
	verify_step(step, destroy_callback_step);

	return FALSE;
}

void schedule_callback_verification(struct step *step)
{
	g_atomic_int_inc(&scheduled_cbacks_num);
	g_idle_add(verify_callback, step);
}

void schedule_action_verification(struct step *step)
{
	g_idle_add_full(G_PRIORITY_HIGH_IDLE, verify_action, step, NULL);
}

static void adapter_state_changed_cb(bt_state_t state)
{
	struct step *step = g_new0(struct step, 1);

	step->callback_result.state = state;
	step->callback = CB_BT_ADAPTER_STATE_CHANGED;

	schedule_callback_verification(step);
}

static bt_property_t *copy_properties(int num_properties,
						bt_property_t *properties)
{
	int i;
	bt_property_t *props = g_new0(bt_property_t, num_properties);

	for (i = 0; i < num_properties; i++) {
		props[i].type = properties[i].type;
		props[i].len = properties[i].len;
		props[i].val = g_memdup(properties[i].val, properties[i].len);
	}

	return props;
}

static bt_property_t *repack_properties(int num_properties,
						bt_property_t **properties)
{
	int i;
	bt_property_t *props = g_new0(bt_property_t, num_properties);

	for (i = 0; i < num_properties; i++) {
		props[i].type = properties[i]->type;
		props[i].len = properties[i]->len;
		props[i].val = g_memdup(properties[i]->val, properties[i]->len);
	}

	return props;
}

static bt_property_t *create_property(bt_property_type_t type, void *val,
								int len)
{
	bt_property_t *prop = g_new0(bt_property_t, 1);

	prop->type = type;
	prop->len = len;
	prop->val = g_memdup(val, len);

	return prop;
}

static void adapter_properties_cb(bt_status_t status, int num_properties,
						bt_property_t *properties)
{
	struct step *step = g_new0(struct step, 1);

	step->callback_result.status = status;
	step->callback_result.num_properties = num_properties;
	step->callback_result.properties = copy_properties(num_properties,
								properties);
	step->callback = CB_BT_ADAPTER_PROPERTIES;

	schedule_callback_verification(step);
}

static void discovery_state_changed_cb(bt_discovery_state_t state)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_BT_DISCOVERY_STATE_CHANGED;
	step->callback_result.state = state;

	schedule_callback_verification(step);
}

static void device_found_cb(int num_properties, bt_property_t *properties)
{
	struct step *step = g_new0(struct step, 1);

	step->callback_result.num_properties = num_properties;
	step->callback_result.properties = copy_properties(num_properties,
								properties);

	step->callback = CB_BT_DEVICE_FOUND;

	schedule_callback_verification(step);
}

static void remote_device_properties_cb(bt_status_t status,
				bt_bdaddr_t *bd_addr, int num_properties,
				bt_property_t *properties)
{
	struct step *step = g_new0(struct step, 1);

	step->callback_result.num_properties = num_properties;
	step->callback_result.properties = copy_properties(num_properties,
								properties);

	step->callback = CB_BT_REMOTE_DEVICE_PROPERTIES;

	schedule_callback_verification(step);
}

static void bond_state_changed_cb(bt_status_t status,
						bt_bdaddr_t *remote_bd_addr,
						bt_bond_state_t state)
{
	struct step *step = g_new0(struct step, 1);

	step->callback_result.status = status;
	step->callback_result.state = state;

	/* Utilize property verification mechanism for bdaddr */
	step->callback_result.num_properties = 1;
	step->callback_result.properties =
			create_property(BT_PROPERTY_BDADDR, remote_bd_addr,
						sizeof(*remote_bd_addr));

	step->callback = CB_BT_BOND_STATE_CHANGED;

	schedule_callback_verification(step);
}

static void pin_request_cb(bt_bdaddr_t *remote_bd_addr,
					bt_bdname_t *bd_name, uint32_t cod)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[3];

	step->callback = CB_BT_PIN_REQUEST;

	/* Utilize property verification mechanism for those */
	props[0] = create_property(BT_PROPERTY_BDADDR, remote_bd_addr,
						sizeof(*remote_bd_addr));
	props[1] = create_property(BT_PROPERTY_BDNAME, bd_name->name,
						strlen((char *) bd_name->name));
	props[2] = create_property(BT_PROPERTY_CLASS_OF_DEVICE, &cod,
								sizeof(cod));

	step->callback_result.num_properties = 3;
	step->callback_result.properties = repack_properties(3, props);

	g_free(props[0]->val);
	g_free(props[0]);
	g_free(props[1]->val);
	g_free(props[1]);
	g_free(props[2]->val);
	g_free(props[2]);

	schedule_callback_verification(step);
}

static void ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
					bt_bdname_t *bd_name, uint32_t cod,
					bt_ssp_variant_t pairing_variant,
					uint32_t pass_key)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[3];

	step->callback = CB_BT_SSP_REQUEST;

	/* Utilize property verification mechanism for those */
	props[0] = create_property(BT_PROPERTY_BDADDR, remote_bd_addr,
						sizeof(*remote_bd_addr));
	props[1] = create_property(BT_PROPERTY_BDNAME, bd_name->name,
						strlen((char *) bd_name->name));
	props[2] = create_property(BT_PROPERTY_CLASS_OF_DEVICE, &cod,
								sizeof(cod));

	step->callback_result.num_properties = 3;
	step->callback_result.properties = repack_properties(3, props);

	g_free(props[0]->val);
	g_free(props[0]);
	g_free(props[1]->val);
	g_free(props[1]);
	g_free(props[2]->val);
	g_free(props[2]);

	schedule_callback_verification(step);
}

static void acl_state_changed_cb(bt_status_t status,
					bt_bdaddr_t *remote_bd_addr,
					bt_acl_state_t state) {
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_BT_ACL_STATE_CHANGED;

	step->callback_result.status = status;
	step->callback_result.state = state;

	schedule_callback_verification(step);
}

static bt_callbacks_t bt_callbacks = {
	.size = sizeof(bt_callbacks),
	.adapter_state_changed_cb = adapter_state_changed_cb,
	.adapter_properties_cb = adapter_properties_cb,
	.remote_device_properties_cb = remote_device_properties_cb,
	.device_found_cb = device_found_cb,
	.discovery_state_changed_cb = discovery_state_changed_cb,
	.pin_request_cb = pin_request_cb,
	.ssp_request_cb = ssp_request_cb,
	.bond_state_changed_cb = bond_state_changed_cb,
	.acl_state_changed_cb = acl_state_changed_cb,
	.thread_evt_cb = NULL,
	.dut_mode_recv_cb = NULL,
	.le_test_mode_cb = NULL,
	.energy_info_cb = NULL,
};

static void hidhost_connection_state_cb(bt_bdaddr_t *bd_addr,
						bthh_connection_state_t state)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HH_CONNECTION_STATE;
	step->callback_result.state = state;

	schedule_callback_verification(step);
}

static void hidhost_virtual_unplug_cb(bt_bdaddr_t *bd_addr, bthh_status_t status)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HH_VIRTUAL_UNPLUG;
	step->callback_result.status = status;

	schedule_callback_verification(step);
}

static void hidhost_protocol_mode_cb(bt_bdaddr_t *bd_addr,
						bthh_status_t status,
						bthh_protocol_mode_t mode)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HH_PROTOCOL_MODE;
	step->callback_result.status = status;
	step->callback_result.mode = mode;

	/* TODO: add bdaddr to verify? */

	schedule_callback_verification(step);
}

static void hidhost_hid_info_cb(bt_bdaddr_t *bd_addr, bthh_hid_info_t hid)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HH_HID_INFO;

	schedule_callback_verification(step);
}

static void hidhost_get_report_cb(bt_bdaddr_t *bd_addr, bthh_status_t status,
						uint8_t *report, int size)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HH_GET_REPORT;

	step->callback_result.status = status;
	step->callback_result.report_size = size;

	schedule_callback_verification(step);
}

static bthh_callbacks_t bthh_callbacks = {
	.size = sizeof(bthh_callbacks),
	.connection_state_cb = hidhost_connection_state_cb,
	.hid_info_cb = hidhost_hid_info_cb,
	.protocol_mode_cb = hidhost_protocol_mode_cb,
	.idle_time_cb = NULL,
	.get_report_cb = hidhost_get_report_cb,
	.virtual_unplug_cb = hidhost_virtual_unplug_cb,
	.handshake_cb = NULL,
};

static void gattc_register_client_cb(int status, int client_if,
							bt_uuid_t *app_uuid)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_REGISTER_CLIENT;

	step->callback_result.status = status;

	schedule_callback_verification(step);
}

static void gattc_scan_result_cb(bt_bdaddr_t *bda, int rssi, uint8_t *adv_data)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[2];

	step->callback = CB_GATTC_SCAN_RESULT;
	step->callback_result.adv_data = adv_data ? TRUE : FALSE;

	/* Utilize property verification mechanism for those */
	props[0] = create_property(BT_PROPERTY_BDADDR, bda, sizeof(*bda));
	props[1] = create_property(BT_PROPERTY_REMOTE_RSSI, &rssi,
								sizeof(rssi));

	step->callback_result.num_properties = 2;
	step->callback_result.properties = repack_properties(2, props);

	g_free(props[0]->val);
	g_free(props[0]);
	g_free(props[1]->val);
	g_free(props[1]);

	schedule_callback_verification(step);
}

static void gattc_connect_cb(int conn_id, int status, int client_if,
							bt_bdaddr_t *bda)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[1];

	step->callback = CB_GATTC_OPEN;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.gatt_app_id = client_if;

	/* Utilize property verification mechanism for bdaddr */
	props[0] = create_property(BT_PROPERTY_BDADDR, bda, sizeof(*bda));

	step->callback_result.num_properties = 1;
	step->callback_result.properties = repack_properties(1, props);

	g_free(props[0]->val);
	g_free(props[0]);

	schedule_callback_verification(step);
}

static void gattc_disconnect_cb(int conn_id, int status, int client_if,
							bt_bdaddr_t *bda)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[1];

	step->callback = CB_GATTC_CLOSE;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.gatt_app_id = client_if;

	/* Utilize property verification mechanism for bdaddr */
	props[0] = create_property(BT_PROPERTY_BDADDR, bda, sizeof(*bda));

	step->callback_result.num_properties = 1;
	step->callback_result.properties = repack_properties(1, props);

	g_free(props[0]->val);
	g_free(props[0]);

	schedule_callback_verification(step);
}

static void gattc_listen_cb(int status, int server_if)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_LISTEN;
	step->callback_result.status = status;

	schedule_callback_verification(step);
}

static void gattc_search_result_cb(int conn_id, btgatt_srvc_id_t *srvc_id)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_SEARCH_RESULT;
	step->callback_result.conn_id = conn_id;
	step->callback_result.service = g_memdup(srvc_id, sizeof(*srvc_id));

	schedule_callback_verification(step);
}

static void gattc_search_complete_cb(int conn_id, int status)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_SEARCH_COMPLETE;
	step->callback_result.conn_id = conn_id;

	schedule_callback_verification(step);
}

static void gattc_get_characteristic_cb(int conn_id, int status,
			btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
			int char_prop)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_GET_CHARACTERISTIC;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.service = g_memdup(srvc_id, sizeof(*srvc_id));
	step->callback_result.characteristic = g_memdup(char_id,
							sizeof(*char_id));
	step->callback_result.char_prop = char_prop;

	schedule_callback_verification(step);
}

static void gattc_get_descriptor_cb(int conn_id, int status,
			btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
			btgatt_gatt_id_t *descr_id)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_GET_DESCRIPTOR;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.service = g_memdup(srvc_id, sizeof(*srvc_id));
	step->callback_result.characteristic = g_memdup(char_id,
							sizeof(*char_id));
	step->callback_result.descriptor = g_memdup(descr_id,
							sizeof(*descr_id));

	schedule_callback_verification(step);
}

static void gattc_get_included_service_cb(int conn_id, int status,
		btgatt_srvc_id_t *srvc_id, btgatt_srvc_id_t *incl_srvc_id)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_GET_INCLUDED_SERVICE;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.service = g_memdup(srvc_id, sizeof(*srvc_id));
	step->callback_result.included = g_memdup(incl_srvc_id,
							sizeof(*srvc_id));

	schedule_callback_verification(step);
}

static void gattc_read_characteristic_cb(int conn_id, int status,
						btgatt_read_params_t *p_data)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_READ_CHARACTERISTIC;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.read_params = g_memdup(p_data, sizeof(*p_data));

	schedule_callback_verification(step);
}

static void gattc_read_descriptor_cb(int conn_id, int status,
						btgatt_read_params_t *p_data)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_READ_DESCRIPTOR;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.read_params = g_memdup(p_data, sizeof(*p_data));

	schedule_callback_verification(step);
}

static void gattc_write_characteristic_cb(int conn_id, int status,
						btgatt_write_params_t *p_data)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_WRITE_CHARACTERISTIC;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.write_params = g_memdup(p_data, sizeof(*p_data));

	schedule_callback_verification(step);
}

static void gattc_write_descriptor_cb(int conn_id, int status,
						btgatt_write_params_t *p_data)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_WRITE_DESCRIPTOR;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.write_params = g_memdup(p_data, sizeof(*p_data));

	schedule_callback_verification(step);
}

static void gattc_register_for_notification_cb(int conn_id, int registered,
						int status,
						btgatt_srvc_id_t *srvc_id,
						btgatt_gatt_id_t *char_id)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_REGISTER_FOR_NOTIFICATION;
	step->callback_result.status = status;
	step->callback_result.conn_id = conn_id;
	step->callback_result.service = g_memdup(srvc_id, sizeof(*srvc_id));
	step->callback_result.characteristic = g_memdup(char_id,
							sizeof(*char_id));
	step->callback_result.notification_registered = registered;

	schedule_callback_verification(step);
}

static void gattc_notif_cb(int conn_id, btgatt_notify_params_t *p_data)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTC_NOTIFY;
	step->callback_result.conn_id = conn_id;
	step->callback_result.notify_params = g_memdup(p_data, sizeof(*p_data));

	schedule_callback_verification(step);
}

static const btgatt_client_callbacks_t btgatt_client_callbacks = {
	.register_client_cb = gattc_register_client_cb,
	.scan_result_cb = gattc_scan_result_cb,
	.open_cb = gattc_connect_cb,
	.close_cb = gattc_disconnect_cb,
	.search_complete_cb = gattc_search_complete_cb,
	.search_result_cb = gattc_search_result_cb,
	.get_characteristic_cb = gattc_get_characteristic_cb,
	.get_descriptor_cb = gattc_get_descriptor_cb,
	.get_included_service_cb = gattc_get_included_service_cb,
	.register_for_notification_cb = gattc_register_for_notification_cb,
	.notify_cb = gattc_notif_cb,
	.read_characteristic_cb = gattc_read_characteristic_cb,
	.write_characteristic_cb = gattc_write_characteristic_cb,
	.read_descriptor_cb = gattc_read_descriptor_cb,
	.write_descriptor_cb = gattc_write_descriptor_cb,
	.execute_write_cb = NULL,
	.read_remote_rssi_cb = NULL,
	.listen_cb = gattc_listen_cb
};

static void gatts_register_server_cb(int status, int server_if,
							bt_uuid_t *app_uuid)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_REGISTER_SERVER;

	step->callback_result.status = status;

	schedule_callback_verification(step);
}

static void gatts_connection_cb(int conn_id, int server_if, int connected,
							bt_bdaddr_t *bda)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[1];

	step->callback = CB_GATTS_CONNECTION;
	step->callback_result.conn_id = conn_id;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.connected = connected;

	/* Utilize property verification mechanism for bdaddr */
	props[0] = create_property(BT_PROPERTY_BDADDR, bda, sizeof(*bda));

	step->callback_result.num_properties = 1;
	step->callback_result.properties = repack_properties(1, props);

	g_free(props[0]->val);
	g_free(props[0]);

	schedule_callback_verification(step);
}

static void gatts_service_added_cb(int status, int server_if,
						btgatt_srvc_id_t *srvc_id,
						int srvc_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_SERVICE_ADDED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.service = g_memdup(srvc_id, sizeof(*srvc_id));
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));

	schedule_callback_verification(step);
}

static void gatts_included_service_added_cb(int status, int server_if,
							int srvc_handle,
							int inc_srvc_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_INCLUDED_SERVICE_ADDED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));
	step->callback_result.inc_srvc_handle = g_memdup(&inc_srvc_handle,
						sizeof(inc_srvc_handle));

	schedule_callback_verification(step);
}

static void gatts_characteristic_added_cb(int status, int server_if,
								bt_uuid_t *uuid,
								int srvc_handle,
								int char_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_CHARACTERISTIC_ADDED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));
	step->callback_result.uuid = g_memdup(uuid, sizeof(*uuid));
	step->callback_result.char_handle = g_memdup(&char_handle,
							sizeof(char_handle));

	schedule_callback_verification(step);
}

static void gatts_descriptor_added_cb(int status, int server_if,
								bt_uuid_t *uuid,
								int srvc_handle,
								int desc_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_DESCRIPTOR_ADDED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));
	step->callback_result.uuid = g_memdup(uuid, sizeof(*uuid));
	step->callback_result.desc_handle = g_memdup(&desc_handle,
							sizeof(desc_handle));

	schedule_callback_verification(step);
}

static void gatts_service_started_cb(int status, int server_if, int srvc_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_SERVICE_STARTED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));

	schedule_callback_verification(step);
}

static void gatts_service_stopped_cb(int status, int server_if, int srvc_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_SERVICE_STOPPED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));

	schedule_callback_verification(step);
}

static void gatts_service_deleted_cb(int status, int server_if, int srvc_handle)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_SERVICE_DELETED;

	step->callback_result.status = status;
	step->callback_result.gatt_app_id = server_if;
	step->callback_result.srvc_handle = g_memdup(&srvc_handle,
							sizeof(srvc_handle));

	schedule_callback_verification(step);
}

static void gatts_request_read_cb(int conn_id, int trans_id, bt_bdaddr_t *bda,
						int attr_handle, int offset,
						bool is_long)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[1];

	step->callback = CB_GATTS_REQUEST_READ;

	step->callback_result.conn_id = conn_id;
	step->callback_result.trans_id = trans_id;
	step->callback_result.attr_handle = g_memdup(&attr_handle,
							sizeof(attr_handle));
	step->callback_result.offset = offset;
	step->callback_result.is_long = is_long;

	/* Utilize property verification mechanism for bdaddr */
	props[0] = create_property(BT_PROPERTY_BDADDR, bda, sizeof(*bda));

	step->callback_result.num_properties = 1;
	step->callback_result.properties = repack_properties(1, props);

	g_free(props[0]->val);
	g_free(props[0]);

	schedule_callback_verification(step);
}

static void gatts_request_write_cb(int conn_id, int trans_id, bt_bdaddr_t *bda,
						int attr_handle, int offset,
						int length, bool need_rsp,
						bool is_prep, uint8_t *value)
{
	struct step *step = g_new0(struct step, 1);
	bt_property_t *props[1];

	step->callback = CB_GATTS_REQUEST_WRITE;

	step->callback_result.conn_id = conn_id;
	step->callback_result.trans_id = trans_id;
	step->callback_result.attr_handle = g_memdup(&attr_handle,
							sizeof(attr_handle));
	step->callback_result.offset = offset;
	step->callback_result.length = length;
	step->callback_result.need_rsp = need_rsp;
	step->callback_result.is_prep = is_prep;
	step->callback_result.value = g_memdup(&value, length);

	/* Utilize property verification mechanism for bdaddr */
	props[0] = create_property(BT_PROPERTY_BDADDR, bda, sizeof(*bda));

	step->callback_result.num_properties = 1;
	step->callback_result.properties = repack_properties(1, props);

	g_free(props[0]->val);
	g_free(props[0]);

	schedule_callback_verification(step);
}

static void gatts_indication_send_cb(int conn_id, int status)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_GATTS_INDICATION_SEND;

	step->callback_result.conn_id = conn_id;
	step->callback_result.status = status;

	schedule_callback_verification(step);
}

static const btgatt_server_callbacks_t btgatt_server_callbacks = {
	.register_server_cb = gatts_register_server_cb,
	.connection_cb = gatts_connection_cb,
	.service_added_cb = gatts_service_added_cb,
	.included_service_added_cb = gatts_included_service_added_cb,
	.characteristic_added_cb = gatts_characteristic_added_cb,
	.descriptor_added_cb = gatts_descriptor_added_cb,
	.service_started_cb = gatts_service_started_cb,
	.service_stopped_cb = gatts_service_stopped_cb,
	.service_deleted_cb = gatts_service_deleted_cb,
	.request_read_cb = gatts_request_read_cb,
	.request_write_cb = gatts_request_write_cb,
	.request_exec_write_cb = NULL,
	.response_confirmation_cb = NULL,
	.indication_sent_cb = gatts_indication_send_cb,
	.congestion_cb = NULL,
};

static const btgatt_callbacks_t btgatt_callbacks = {
	.size = sizeof(btgatt_callbacks),
	.client = &btgatt_client_callbacks,
	.server = &btgatt_server_callbacks
};

static void pan_control_state_cb(btpan_control_state_t state, int local_role,
					bt_status_t error, const char *ifname)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_PAN_CONTROL_STATE;
	step->callback_result.state = error;
	step->callback_result.ctrl_state = state;
	step->callback_result.local_role = local_role;

	schedule_callback_verification(step);
}

static void pan_connection_state_cb(btpan_connection_state_t state,
					bt_status_t error,
					const bt_bdaddr_t *bd_addr,
					int local_role, int remote_role)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_PAN_CONNECTION_STATE;
	step->callback_result.state = error;
	step->callback_result.conn_state = state;
	step->callback_result.local_role = local_role;
	step->callback_result.remote_role = remote_role;

	schedule_callback_verification(step);
}

static btpan_callbacks_t btpan_callbacks = {
	.size = sizeof(btpan_callbacks),
	.control_state_cb = pan_control_state_cb,
	.connection_state_cb = pan_connection_state_cb,
};

static void hdp_app_reg_state_cb(int app_id, bthl_app_reg_state_t state)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HDP_APP_REG_STATE;
	step->callback_result.app_id = app_id;
	step->callback_result.app_state = state;

	schedule_callback_verification(step);
}

static void hdp_channel_state_cb(int app_id, bt_bdaddr_t *bd_addr,
				int mdep_cfg_index, int channel_id,
				bthl_channel_state_t state, int fd)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_HDP_CHANNEL_STATE;
	step->callback_result.app_id = app_id;
	step->callback_result.channel_id = channel_id;
	step->callback_result.mdep_cfg_index = mdep_cfg_index;
	step->callback_result.channel_state = state;

	schedule_callback_verification(step);
}

static bthl_callbacks_t bthl_callbacks = {
	.size = sizeof(bthl_callbacks),
	.app_reg_state_cb = hdp_app_reg_state_cb,
	.channel_state_cb = hdp_channel_state_cb,
};

static void a2dp_connection_state_cb(btav_connection_state_t state,
							bt_bdaddr_t *bd_addr)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_A2DP_CONN_STATE;
	step->callback_result.av_conn_state = state;

	schedule_callback_verification(step);
}

static void a2dp_audio_state_cb(btav_audio_state_t state, bt_bdaddr_t *bd_addr)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_A2DP_AUDIO_STATE;
	step->callback_result.av_audio_state = state;

	schedule_callback_verification(step);
}

static btav_callbacks_t bta2dp_callbacks = {
	.size = sizeof(bta2dp_callbacks),
	.connection_state_cb = a2dp_connection_state_cb,
	.audio_state_cb = a2dp_audio_state_cb,
};

static void avrcp_get_play_status_cb(void)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_AVRCP_PLAY_STATUS_REQ;
	schedule_callback_verification(step);
}

static void avrcp_register_notification_cb(btrc_event_id_t event_id,
								uint32_t param)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_AVRCP_REG_NOTIF_REQ;
	schedule_callback_verification(step);
}

static void avrcp_get_element_attr_cb(uint8_t num_attr,
						btrc_media_attr_t *p_attrs)
{
	struct step *step = g_new0(struct step, 1);

	step->callback = CB_AVRCP_GET_ATTR_REQ;
	schedule_callback_verification(step);
}

static btrc_callbacks_t btavrcp_callbacks = {
	.size = sizeof(btavrcp_callbacks),
	.get_play_status_cb = avrcp_get_play_status_cb,
	.register_notification_cb = avrcp_register_notification_cb,
	.get_element_attr_cb = avrcp_get_element_attr_cb,
};

static btmce_mas_instance_t *copy_mas_instances(int num_instances,
						btmce_mas_instance_t *instances)
{
	int i;
	btmce_mas_instance_t *inst;

	inst = g_new0(btmce_mas_instance_t, num_instances);

	for (i = 0; i < num_instances; i++) {
		inst[i].id = instances[i].id;
		inst[i].scn = instances[i].scn;
		inst[i].msg_types = instances[i].msg_types;
		inst[i].p_name = g_memdup(instances[i].p_name,
						strlen(instances[i].p_name));
	}

	return inst;
}

static void map_client_get_remote_mas_instances_cb(bt_status_t status,
							bt_bdaddr_t *bd_addr,
							int num_instances,
							btmce_mas_instance_t
							*instances)
{
	struct step *step = g_new0(struct step, 1);

	step->callback_result.status = status;
	step->callback_result.num_mas_instances = num_instances;
	step->callback_result.mas_instances = copy_mas_instances(num_instances,
								instances);

	step->callback = CB_MAP_CLIENT_REMOTE_MAS_INSTANCES;

	schedule_callback_verification(step);
}

static btmce_callbacks_t btmap_client_callbacks = {
	.size = sizeof(btmap_client_callbacks),
	.remote_mas_instances_cb = map_client_get_remote_mas_instances_cb,
};

static bool setup_base(struct test_data *data)
{
	const hw_module_t *module;
	hw_device_t *device;
	int signal_fd[2];
	char buf[1024];
	pid_t pid;
	int len;
	int err;

	if (pipe(signal_fd))
		return false;

	pid = fork();

	if (pid < 0) {
		close(signal_fd[0]);
		close(signal_fd[1]);
		return false;
	}

	if (pid == 0) {
		if (!tester_use_debug())
			fclose(stderr);

		close(signal_fd[0]);
		emulator(signal_fd[1], data->mgmt_index);
		exit(0);
	}

	close(signal_fd[1]);
	data->bluetoothd_pid = pid;

	len = read(signal_fd[0], buf, sizeof(buf));
	if (len <= 0 || strcmp(buf, EMULATOR_SIGNAL)) {
		close(signal_fd[0]);
		return false;
	}

	close(signal_fd[0]);

	err = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID,
					AUDIO_HARDWARE_MODULE_ID_A2DP, &module);
	if (err)
		return false;

	err = audio_hw_device_open(module, &data->audio);
	if (err)
		return false;

	err = hw_get_module(BT_HARDWARE_MODULE_ID, &module);
	if (err)
		return false;

	err = module->methods->open(module, BT_HARDWARE_MODULE_ID, &device);
	if (err)
		return false;

	data->device = device;

	data->if_bluetooth = ((bluetooth_device_t *)
					device)->get_bluetooth_interface();
	if (!data->if_bluetooth)
		return false;

	if (!(data->steps = queue_new()))
		return false;

	data->pdus = queue_new();
	if (!data->pdus) {
		queue_destroy(data->steps, NULL);
		return false;
	}

	return true;
}

static void setup(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_socket(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;
	const void *sock;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	sock = data->if_bluetooth->get_profile_interface(BT_PROFILE_SOCKETS_ID);
	if (!sock) {
		tester_setup_failed();
		return;
	}

	data->if_sock = sock;

	tester_setup_complete();
}

static void setup_hidhost(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;
	const void *hid;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	hid = data->if_bluetooth->get_profile_interface(BT_PROFILE_HIDHOST_ID);
	if (!hid) {
		tester_setup_failed();
		return;
	}

	data->if_hid = hid;

	status = data->if_hid->init(&bthh_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_hid = NULL;
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_pan(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;
	const void *pan;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	pan = data->if_bluetooth->get_profile_interface(BT_PROFILE_PAN_ID);
	if (!pan) {
		tester_setup_failed();
		return;
	}

	data->if_pan = pan;

	status = data->if_pan->init(&btpan_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_pan = NULL;
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_hdp(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;
	const void *hdp;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	hdp = data->if_bluetooth->get_profile_interface(BT_PROFILE_HEALTH_ID);
	if (!hdp) {
		tester_setup_failed();
		return;
	}

	data->if_hdp = hdp;

	status = data->if_hdp->init(&bthl_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_hdp = NULL;
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_a2dp(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const bt_interface_t *if_bt;
	bt_status_t status;
	const void *a2dp;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	if_bt = data->if_bluetooth;

	status = if_bt->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	a2dp = if_bt->get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
	if (!a2dp) {
		tester_setup_failed();
		return;
	}

	data->if_a2dp = a2dp;

	status = data->if_a2dp->init(&bta2dp_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_a2dp = NULL;
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_avrcp(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const bt_interface_t *if_bt;
	bt_status_t status;
	const void *a2dp, *avrcp;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	if_bt = data->if_bluetooth;

	status = if_bt->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	a2dp = if_bt->get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
	if (!a2dp) {
		tester_setup_failed();
		return;
	}

	data->if_a2dp = a2dp;

	status = data->if_a2dp->init(&bta2dp_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_a2dp = NULL;
		tester_setup_failed();
		return;
	}

	avrcp = if_bt->get_profile_interface(BT_PROFILE_AV_RC_ID);
	if (!avrcp) {
		tester_setup_failed();
		return;
	}

	data->if_avrcp = avrcp;

	status = data->if_avrcp->init(&btavrcp_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_avrcp = NULL;
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_gatt(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;
	const void *gatt;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	gatt = data->if_bluetooth->get_profile_interface(BT_PROFILE_GATT_ID);
	if (!gatt) {
		tester_setup_failed();
		return;
	}

	data->if_gatt = gatt;

	status = data->if_gatt->init(&btgatt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_gatt = NULL;

		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_map_client(const void *test_data)
{
	struct test_data *data = tester_get_data();
	bt_status_t status;
	const void *map_client;

	if (!setup_base(data)) {
		tester_setup_failed();
		return;
	}

	status = data->if_bluetooth->init(&bt_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_bluetooth = NULL;
		tester_setup_failed();
		return;
	}

	map_client = data->if_bluetooth->get_profile_interface(
						BT_PROFILE_MAP_CLIENT_ID);
	if (!map_client) {
		tester_setup_failed();
		return;
	}

	data->if_map_client = map_client;

	status = data->if_map_client->init(&btmap_client_callbacks);
	if (status != BT_STATUS_SUCCESS) {
		data->if_map_client = NULL;

		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void teardown(const void *test_data)
{
	struct test_data *data = tester_get_data();

	queue_destroy(data->steps, NULL);
	data->steps = NULL;

	queue_destroy(data->pdus, NULL);
	data->pdus = NULL;

	/* no cleanup for map_client */
	data->if_map_client = NULL;

	if (data->if_gatt) {
		data->if_gatt->cleanup();
		data->if_gatt = NULL;
	}

	if (data->if_hid) {
		data->if_hid->cleanup();
		data->if_hid = NULL;
	}

	if (data->if_pan) {
		data->if_pan->cleanup();
		data->if_pan = NULL;
	}

	if (data->if_hdp) {
		data->if_hdp->cleanup();
		data->if_hdp = NULL;
	}

	if (data->if_stream) {
		data->audio->close_output_stream(data->audio, data->if_stream);
		data->if_stream = NULL;
	}

	if (data->if_a2dp) {
		data->if_a2dp->cleanup();
		data->if_a2dp = NULL;
	}

	if (data->if_avrcp) {
		data->if_avrcp->cleanup();
		data->if_avrcp = NULL;
	}

	if (data->if_bluetooth) {
		data->if_bluetooth->cleanup();
		data->if_bluetooth = NULL;
	}

	data->device->close(data->device);
	audio_hw_device_close(data->audio);

	/*
	 * Ssp_request_cb pointer can be set do default_ssp_req_cb.
	 * Set it back to ssp_request_cb
	 */
	bt_callbacks.ssp_request_cb = ssp_request_cb;

	if (!data->bluetoothd_pid)
		tester_teardown_complete();
}

static void emu_connectable_complete(uint16_t opcode, uint8_t status,
					const void *param, uint8_t len,
					void *user_data)
{
	struct step *step;
	struct test_data *data = user_data;

	switch (opcode) {
	case BT_HCI_CMD_WRITE_SCAN_ENABLE:
		break;
	case BT_HCI_CMD_LE_SET_ADV_ENABLE:
		/*
		 * For BREDRLE emulator we want to verify step after scan
		 * enable and not after le_set_adv_enable
		 */
		if (data->hciemu_type == HCIEMU_TYPE_BREDRLE)
			return;

		break;
	default:
		return;
	}

	step = g_new0(struct step, 1);

	if (status) {
		tester_warn("Emulated remote setup failed.");
		step->action_status = BT_STATUS_FAIL;
	} else {
		tester_debug("Emulated remote setup done.");
		step->action_status = BT_STATUS_SUCCESS;
	}

	schedule_action_verification(step);
}

void emu_setup_powered_remote_action(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost;

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_cmd_complete_cb(bthost, emu_connectable_complete, data);

	if ((data->hciemu_type == HCIEMU_TYPE_LE) ||
				(data->hciemu_type == HCIEMU_TYPE_BREDRLE)) {
		uint8_t adv[4];

		adv[0] = 0x02;	/* Field length */
		adv[1] = 0x01;	/* Flags */
		adv[2] = 0x02;	/* Flags value */
		adv[3] = 0x00;	/* Field terminator */

		bthost_set_adv_data(bthost, adv, sizeof(adv));
		bthost_set_adv_enable(bthost, 0x01);
	}

	if (data->hciemu_type != HCIEMU_TYPE_LE)
		bthost_write_scan_enable(bthost, 0x03);
}

void emu_set_pin_code_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct bthost *bthost;
	struct step *step = g_new0(struct step, 1);

	bthost = hciemu_client_get_host(data->hciemu);

	bthost_set_pin_code(bthost, action_data->pin->pin,
							action_data->pin_len);

	step->action_status = BT_STATUS_SUCCESS;

	tester_print("Setting emu pin done.");

	schedule_action_verification(step);
}

void emu_set_ssp_mode_action(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost;
	struct step *step = g_new0(struct step, 1);

	bthost = hciemu_client_get_host(data->hciemu);

	bthost_write_ssp_mode(bthost, 0x01);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

void emu_set_connect_cb_action(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct step *current_data_step = queue_peek_head(data->steps);
	void *cb = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	bthost_set_connect_cb(bthost, cb, data);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

void emu_remote_connect_hci_action(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);
	const uint8_t *master_addr;

	master_addr = hciemu_get_master_bdaddr(data->hciemu);

	tester_print("Trying to connect hci");

	if (action_data)
		bthost_hci_connect(bthost, master_addr,
						action_data->bearer_type);
	else
		bthost_hci_connect(bthost, master_addr, BDADDR_BREDR);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

void emu_remote_disconnect_hci_action(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct step *current_data_step = queue_peek_head(data->steps);
	uint16_t *handle = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	if (handle) {
		bthost_hci_disconnect(bthost, *handle, 0x13);
		step->action_status = BT_STATUS_SUCCESS;
	} else {
		step->action_status = BT_STATUS_FAIL;
	}

	schedule_action_verification(step);
}

void emu_set_io_cap(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost;
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	bthost = hciemu_client_get_host(data->hciemu);

	if (action_data)
		bthost_set_io_capability(bthost, action_data->io_cap);
	else
		bthost_set_io_capability(bthost, 0x01);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

void emu_add_l2cap_server_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct emu_set_l2cap_data *l2cap_data = current_data_step->set_data;
	struct bthost *bthost;
	struct step *step = g_new0(struct step, 1);

	if (!l2cap_data) {
		tester_warn("Invalid l2cap_data params");
		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	bthost = hciemu_client_get_host(data->hciemu);

	bthost_add_l2cap_server(bthost, l2cap_data->psm, l2cap_data->func,
							l2cap_data->user_data);

	step->action_status = BT_STATUS_SUCCESS;

done:
	schedule_action_verification(step);
}

static void print_data(const char *str, void *user_data)
{
	tester_debug("tester: %s", str);
}

static void emu_generic_cid_hook_cb(const void *data, uint16_t len,
								void *user_data)
{
	struct test_data *t_data = tester_get_data();
	struct emu_l2cap_cid_data *cid_data = user_data;
	const struct pdu_set *pdus = cid_data->pdu;
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	int i;

	for (i = 0; pdus[i].rsp.iov_base; i++) {
		if (pdus[i].req.iov_base) {
			if (pdus[i].req.iov_len != len)
				continue;

			if (memcmp(pdus[i].req.iov_base, data, len))
				continue;
		}

		if (pdus[i].rsp.iov_base) {
			util_hexdump('>', pdus[i].rsp.iov_base,
					pdus[i].rsp.iov_len, print_data, NULL);

			/* if its sdp pdu use transaction ID from request */
			if (cid_data->is_sdp) {
				struct iovec rsp[3];

				rsp[0].iov_base = pdus[i].rsp.iov_base;
				rsp[0].iov_len = 1;

				rsp[1].iov_base = ((uint8_t *) data) + 1;
				rsp[1].iov_len = 2;

				rsp[2].iov_base = pdus[i].rsp.iov_base + 3;
				rsp[2].iov_len = pdus[i].rsp.iov_len - 3;

				bthost_send_cid_v(bthost, cid_data->handle,
							cid_data->cid, rsp, 3);
			} else {
				bthost_send_cid_v(bthost, cid_data->handle,
						cid_data->cid, &pdus[i].rsp, 1);
			}

		}
	}
}

void tester_handle_l2cap_data_exchange(struct emu_l2cap_cid_data *cid_data)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);

	bthost_add_cid_hook(bthost, cid_data->handle, cid_data->cid,
					emu_generic_cid_hook_cb, cid_data);
}

void tester_generic_connect_cb(uint16_t handle, uint16_t cid, void *user_data)
{
	struct emu_l2cap_cid_data *cid_data = user_data;

	cid_data->handle = handle;
	cid_data->cid = cid;

	tester_handle_l2cap_data_exchange(cid_data);
}

static void rfcomm_connect_cb(uint16_t handle, uint16_t cid, void *user_data,
								bool status)
{
	struct step *step = g_new0(struct step, 1);

	tester_print("Connect handle %d, cid %d cb status: %d", handle, cid,
									status);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

void emu_add_rfcomm_server_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *rfcomm_data = current_data_step->set_data;
	struct bthost *bthost;
	struct step *step;

	if (!rfcomm_data) {
		tester_warn("Invalid l2cap_data params");
		return;
	}

	step = g_new0(struct step, 1);

	bthost = hciemu_client_get_host(data->hciemu);

	bthost_add_rfcomm_server(bthost, rfcomm_data->channel,
						rfcomm_connect_cb, data);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

void dummy_action(void)
{
	struct step *step = g_new0(struct step, 1);

	step->action = dummy_action;

	schedule_action_verification(step);
}

void bluetooth_enable_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->enable();

	schedule_action_verification(step);
}

void bluetooth_disable_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->disable();

	schedule_action_verification(step);
}

void bt_set_property_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step;
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_property_t *prop;

	if (!current_data_step->set_data) {
		tester_debug("BT property not set for step");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	prop = (bt_property_t *)current_data_step->set_data;

	step->action_status = data->if_bluetooth->set_adapter_property(prop);

	schedule_action_verification(step);
}

void bt_get_property_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step;
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_property_t *prop;

	if (!current_data_step->set_data) {
		tester_debug("BT property to get not defined");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	prop = (bt_property_t *)current_data_step->set_data;

	step->action_status = data->if_bluetooth->get_adapter_property(
								prop->type);

	schedule_action_verification(step);
}

void bt_start_discovery_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->start_discovery();

	schedule_action_verification(step);
}

void bt_cancel_discovery_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->cancel_discovery();

	schedule_action_verification(step);
}

void bt_get_device_props_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct step *step;

	if (!current_data_step->set_data) {
		tester_debug("bdaddr not defined");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	step->action_status =
		data->if_bluetooth->get_remote_device_properties(
						current_data_step->set_data);

	schedule_action_verification(step);
}

void bt_get_device_prop_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step;

	if (!action_data) {
		tester_warn("No arguments for 'get remote device prop' req.");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->get_remote_device_property(
							action_data->addr,
							action_data->prop_type);

	schedule_action_verification(step);
}

void bt_set_device_prop_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step;

	if (!action_data) {
		tester_warn("No arguments for 'set remote device prop' req.");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->set_remote_device_property(
							action_data->addr,
							action_data->prop);

	schedule_action_verification(step);
}

void bt_create_bond_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step;

	if (!action_data || !action_data->addr) {
		tester_warn("Bad arguments for 'create bond' req.");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	step->action_status =
			data->if_bluetooth->create_bond(action_data->addr,
						action_data->transport_type ?
						action_data->transport_type :
						BT_TRANSPORT_UNKNOWN);

	schedule_action_verification(step);
}

void bt_pin_reply_accept_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step;

	if (!action_data || !action_data->addr || !action_data->pin) {
		tester_warn("Bad arguments for 'pin reply' req.");
		tester_test_failed();
		return;
	}

	step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->pin_reply(action_data->addr,
							TRUE,
							action_data->pin_len,
							action_data->pin);

	schedule_action_verification(step);
}

void bt_ssp_reply_accept_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->ssp_reply(action_data->addr,
						action_data->ssp_variant,
						action_data->accept, 0);

	schedule_action_verification(step);
}

void bt_cancel_bond_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_bdaddr_t *addr = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->cancel_bond(addr);

	schedule_action_verification(step);
}

void bt_remove_bond_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_bdaddr_t *addr = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_bluetooth->remove_bond(addr);

	schedule_action_verification(step);
}

static void default_ssp_req_cb(bt_bdaddr_t *remote_bd_addr, bt_bdname_t *name,
				uint32_t cod, bt_ssp_variant_t pairing_variant,
				uint32_t pass_key)
{
	struct test_data *t_data = tester_get_data();

	t_data->if_bluetooth->ssp_reply(remote_bd_addr, pairing_variant, true,
								pass_key);
}

void set_default_ssp_request_handler(void)
{
	struct step *step = g_new0(struct step, 1);

	bt_callbacks.ssp_request_cb = default_ssp_req_cb;

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

static void generic_test_function(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct step *first_step;

	init_test_steps(data);

	/* first step action */
	first_step = queue_peek_head(data->steps);
	if (!first_step->action) {
		tester_print("tester: No initial action declared");
		tester_test_failed();
		return;
	}
	first_step->action();
}

#define test(data, test_setup, test, test_teardown) \
	do { \
		struct test_data *user; \
		user = g_malloc0(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = data->emu_type; \
		user->test_data = data; \
		tester_add_full(data->title, data, test_pre_setup, \
					test_setup, test, test_teardown, \
					test_post_teardown, 3, user, g_free); \
	} while (0)

static void tester_testcases_cleanup(void)
{
	remove_bluetooth_tests();
	remove_socket_tests();
	remove_hidhost_tests();
	remove_gatt_tests();
	remove_a2dp_tests();
	remove_avrcp_tests();
	remove_hdp_tests();
	remove_pan_tests();
}

static void add_bluetooth_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup, generic_test_function, teardown);
}

static void add_socket_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_socket, generic_test_function, teardown);
}

static void add_hidhost_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_hidhost, generic_test_function, teardown);
}

static void add_pan_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_pan, generic_test_function, teardown);
}

static void add_hdp_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_hdp, generic_test_function, teardown);
}

static void add_a2dp_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_a2dp, generic_test_function, teardown);
}

static void add_avrcp_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_avrcp, generic_test_function, teardown);
}

static void add_gatt_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_gatt, generic_test_function, teardown);
}

static void add_map_client_tests(void *data, void *user_data)
{
	struct test_case *tc = data;

	test(tc, setup_map_client, generic_test_function, teardown);
}

int main(int argc, char *argv[])
{
	snprintf(exec_dir, sizeof(exec_dir), "%s", dirname(argv[0]));

	tester_init(&argc, &argv);

	queue_foreach(get_bluetooth_tests(), add_bluetooth_tests, NULL);
	queue_foreach(get_socket_tests(), add_socket_tests, NULL);
	queue_foreach(get_hidhost_tests(), add_hidhost_tests, NULL);
	queue_foreach(get_pan_tests(), add_pan_tests, NULL);
	queue_foreach(get_hdp_tests(), add_hdp_tests, NULL);
	queue_foreach(get_a2dp_tests(), add_a2dp_tests, NULL);
	queue_foreach(get_avrcp_tests(), add_avrcp_tests, NULL);
	queue_foreach(get_gatt_tests(), add_gatt_tests, NULL);
	queue_foreach(get_map_client_tests(), add_map_client_tests, NULL);

	if (tester_run())
		return 1;

	tester_testcases_cleanup();

	return 0;
}
