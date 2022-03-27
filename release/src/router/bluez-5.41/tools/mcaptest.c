/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014 Intel Corporation
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "btio/btio.h"
#include "lib/l2cap.h"
#include "profiles/health/mcap.h"

enum {
	MODE_NONE,
	MODE_CONNECT,
	MODE_LISTEN,
};

static GMainLoop *mloop;

static int ccpsm = 0x1003, dcpsm = 0x1005;

static struct mcap_instance *mcap = NULL;
static struct mcap_mdl *mdl = NULL;
static uint16_t mdlid;

static int control_mode = MODE_LISTEN;
static int data_mode = MODE_LISTEN;

static int mdl_conn_req_result = MCAP_SUCCESS;

static gboolean send_synccap_req = FALSE;
static gboolean mcl_disconnect = FALSE;
static gboolean mdl_disconnect = FALSE;
static int mcl_disconnect_timeout = -1;
static int mdl_disconnect_timeout = -1;

static struct mcap_mcl *mcl = NULL;

static gboolean no_close = FALSE;

#define REQ_CLOCK_ACC 0x1400

static void mdl_close(struct mcap_mdl *mdl)
{
	int fd = -1;

	printf("%s\n", __func__);

	if (mdl_disconnect_timeout >= 0)
		sleep(mdl_disconnect_timeout);

	fd = mcap_mdl_get_fd(mdl);

	if (fd > 0)
		close(fd);
}

static void mdl_connected_cb(struct mcap_mdl *mdl, void *data)
{
	printf("%s\n", __func__);

	if (mdl_disconnect)
		mdl_close(mdl);
}

static void mdl_closed_cb(struct mcap_mdl *mdl, void *data)
{
	printf("%s\n", __func__);

	if (mcl_disconnect && mcl_disconnect_timeout >= 0) {
		sleep(mcl_disconnect_timeout);

		printf("Closing MCAP communication link\n");
		mcap_close_mcl(mcl, TRUE);

		if (no_close)
			return;

		g_main_loop_quit(mloop);
	}
}

static void mdl_deleted_cb(struct mcap_mdl *mdl, void *data)
{
	/* TODO */
	printf("%s\n", __func__);

	/* Disconnecting MDL latency timeout */
	if (mdl_disconnect_timeout >= 0)
		sleep(mdl_disconnect_timeout);
}

static void mdl_aborted_cb(struct mcap_mdl *mdl, void *data)
{
	/* TODO */
	printf("%s\n", __func__);
}

static uint8_t mdl_conn_req_cb(struct mcap_mcl *mcl, uint8_t mdepid,
				uint16_t mdlid, uint8_t *conf, void *data)
{
	int ret;

	printf("%s\n", __func__);

	ret = mdl_conn_req_result;

	mdl_conn_req_result = MCAP_SUCCESS;

	return ret;
}

static uint8_t mdl_reconn_req_cb(struct mcap_mdl *mdl, void *data)
{
	printf("%s\n", __func__);

	return MCAP_SUCCESS;
}

static void create_mdl_cb(struct mcap_mdl *mcap_mdl, uint8_t type, GError *gerr,
								gpointer data);

static void mcl_reconnected(struct mcap_mcl *mcl, gpointer data)
{
	GError *gerr = NULL;

	printf("%s\n", __func__);

	if (data_mode == MODE_CONNECT) {
		mcap_create_mdl(mcl, 1, 0, create_mdl_cb, NULL, NULL, &gerr);
		if (gerr) {
			printf("Could not connect MDL: %s\n", gerr->message);
			g_error_free(gerr);
		}
	}
}

static void mcl_disconnected(struct mcap_mcl *mcl, gpointer data)
{
	/* TODO */
	printf("%s\n", __func__);

	if (no_close)
		return;

	g_main_loop_quit(mloop);
}

static void mcl_uncached(struct mcap_mcl *mcl, gpointer data)
{
	/* TODO */
	printf("%s\n", __func__);
}

static void connect_mdl_cb(struct mcap_mdl *mdl, GError *gerr, gpointer data)
{
	mdlid = mcap_mdl_get_mdlid(mdl);

	printf("%s\n", __func__);

	if (mdlid == MCAP_MDLID_RESERVED)
		printf("MCAP mdlid is reserved");
	else
		printf("MDL %d connected\n", mdlid);
}

static void create_mdl_cb(struct mcap_mdl *mcap_mdl, uint8_t type, GError *gerr,
								gpointer data)
{
	GError *err = NULL;

	printf("%s\n", __func__);

	if (gerr) {
		printf("MDL error: %s\n", gerr->message);

		if (!no_close)
			g_main_loop_quit(mloop);

		return;
	}

	if (mdl)
		mcap_mdl_unref(mdl);

	mdl = mcap_mdl_ref(mcap_mdl);

	if (!mcap_connect_mdl(mdl, L2CAP_MODE_ERTM, dcpsm, connect_mdl_cb, NULL,
								NULL, &err)) {
		printf("Error connecting to mdl: %s\n", err->message);
		g_error_free(err);

		if (no_close)
			return;

		g_main_loop_quit(mloop);
	}
}

static void sync_cap_cb(struct mcap_mcl *mcl, uint8_t mcap_err,
			uint8_t btclockres, uint16_t synclead,
			uint16_t tmstampres, uint16_t tmstampacc, GError *err,
			gpointer data)
{
	/* TODO */
	printf("%s\n", __func__);
}

static void trigger_mdl_action(int mode)
{
	GError *gerr = NULL;
	gboolean ret;

	ret = mcap_mcl_set_cb(mcl, NULL, &gerr,
		MCAP_MDL_CB_CONNECTED, mdl_connected_cb,
		MCAP_MDL_CB_CLOSED, mdl_closed_cb,
		MCAP_MDL_CB_DELETED, mdl_deleted_cb,
		MCAP_MDL_CB_ABORTED, mdl_aborted_cb,
		MCAP_MDL_CB_REMOTE_CONN_REQ, mdl_conn_req_cb,
		MCAP_MDL_CB_REMOTE_RECONN_REQ, mdl_reconn_req_cb,
		MCAP_MDL_CB_INVALID);

	if (!ret && gerr) {
		printf("MCL cannot handle connection %s\n",
							gerr->message);
		g_error_free(gerr);
	}

	if (mode == MODE_CONNECT) {
		printf("Creating MCAP Data End Point\n");
		mcap_create_mdl(mcl, 1, 0, create_mdl_cb, NULL, NULL, &gerr);
		if (gerr) {
			printf("Could not connect MDL: %s\n", gerr->message);
			g_error_free(gerr);
		}
	}

	if (send_synccap_req && mcap->csp_enabled) {
		mcap_sync_init(mcl);

		mcap_sync_cap_req(mcl, REQ_CLOCK_ACC, sync_cap_cb, NULL, &gerr);
		if (gerr) {
			printf("MCAP Sync req error: %s\n", gerr->message);
			g_error_free(gerr);
		}
	}
}

static void mcl_connected(struct mcap_mcl *mcap_mcl, gpointer data)
{
	printf("%s\n", __func__);

	if (mcl) {
		mcap_sync_stop(mcl);
		mcap_mcl_unref(mcl);
	}

	mcl = mcap_mcl_ref(mcap_mcl);
	trigger_mdl_action(data_mode);
}

static void create_mcl_cb(struct mcap_mcl *mcap_mcl, GError *err, gpointer data)
{
	printf("%s\n", __func__);

	if (err) {
		printf("Could not connect MCL: %s\n", err->message);

		if (!no_close)
			g_main_loop_quit(mloop);

		return;
	}

	if (mcl) {
		mcap_sync_stop(mcl);
		mcap_mcl_unref(mcl);
	}

	mcl = mcap_mcl_ref(mcap_mcl);
	trigger_mdl_action(data_mode);
}

static void usage(void)
{
	printf("mcaptest - MCAP testing ver %s\n", VERSION);
	printf("Usage:\n"
		"\tmcaptest <control_mode> <data_mode> [options]\n");
	printf("Control Link Mode:\n"
		"\t-c connect <dst_addr>\n"
		"\t-b close control link after closing data link\n"
		"\t-e <timeout> disconnect MCL and quit after MDL is closed\n"
		"\t-g send clock sync capability request if MCL connected\n");
	printf("Data Link Mode:\n"
		"\t-d connect\n"
		"\t-a close data link immediately after being connected"
		"\t-f <timeout> disconnect MDL after it's connected\n"
		"\t-u send \'Unavailable\' on first MDL connection request\n");
	printf("Options:\n"
		"\t-n don't exit after mcl disconnect/err receive\n"
		"\t-i <hcidev>        HCI device\n"
		"\t-C <control_ch>    Control channel PSM\n"
		"\t-D <data_ch>       Data channel PSM\n");
}

static struct option main_options[] = {
	{ "help",		0, 0, 'h' },
	{ "device",		1, 0, 'i' },
	{ "connect_cl",		1, 0, 'c' },
	{ "disconnect_cl",	1, 0, 'e' },
	{ "synccap_req",	0, 0, 'g' },
	{ "connect_dl",		0, 0, 'd' },
	{ "disconnect_da",	0, 0, 'a' },
	{ "disconnect_ca",	0, 0, 'b' },
	{ "disconnect_dl",	1, 0, 'f' },
	{ "unavailable_dl",	0, 0, 'u' },
	{ "no exit mcl dis/err",0, 0, 'n' },
	{ "control_ch",		1, 0, 'C' },
	{ "data_ch",		1, 0, 'D' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	GError *err = NULL;
	bdaddr_t src, dst;
	int opt;
	char bdastr[18];

	hci_devba(0, &src);
	bacpy(&dst, BDADDR_ANY);

	mloop = g_main_loop_new(NULL, FALSE);
	if (!mloop) {
		printf("Cannot create main loop\n");

		exit(1);
	}

	while ((opt = getopt_long(argc, argv, "+i:c:C:D:e:f:dghunab",
						main_options, NULL)) != EOF) {
		switch (opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &src);
			else
				str2ba(optarg, &src);

			break;

		case 'c':
			control_mode = MODE_CONNECT;
			str2ba(optarg, &dst);

			break;

		case 'd':
			data_mode = MODE_CONNECT;

			break;

		case 'a':
			mdl_disconnect = TRUE;

			break;

		case 'b':
			mcl_disconnect = TRUE;

			break;

		case 'e':
			mcl_disconnect_timeout = atoi(optarg);

			break;

		case 'f':
			mdl_disconnect_timeout = atoi(optarg);

			break;

		case 'g':
			send_synccap_req = TRUE;

			break;

		case 'u':
			mdl_conn_req_result = MCAP_RESOURCE_UNAVAILABLE;

			break;

		case 'n':
			no_close = TRUE;

			break;

		case 'C':
			ccpsm = atoi(optarg);

			break;

		case 'D':
			dcpsm = atoi(optarg);

			break;

		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	mcap = mcap_create_instance(&src, BT_IO_SEC_MEDIUM, ccpsm, dcpsm,
					mcl_connected, mcl_reconnected,
					mcl_disconnected, mcl_uncached,
					NULL, /* CSP is not used right now */
					NULL, &err);

	if (!mcap) {
		printf("MCAP instance creation failed %s\n", err->message);
		g_error_free(err);

		exit(1);
	}

	mcap_enable_csp(mcap);

	switch (control_mode) {
	case MODE_CONNECT:
		ba2str(&dst, bdastr);
		printf("Connecting to %s\n", bdastr);

		mcap_create_mcl(mcap, &dst, ccpsm, create_mcl_cb, NULL, NULL,
									&err);

		if (err) {
			printf("MCAP create error %s\n", err->message);
			g_error_free(err);

			exit(1);
		}

		break;
	case MODE_LISTEN:
		printf("Listening for control channel connection\n");

		break;
	case MODE_NONE:
	default:
		goto done;
	}

	g_main_loop_run(mloop);

done:
	printf("Done\n");

	if (mcap)
		mcap_instance_unref(mcap);

	g_main_loop_unref(mloop);

	return 0;
}
