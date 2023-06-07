// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Codecoup
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jlink.h"

#define RTT_CONTROL_START		0
#define RTT_CONTROL_STOP		1
#define RTT_CONTROL_GET_DESC		2
#define RTT_CONTROL_GET_NUM_BUF		3
#define RTT_CONTROL_GET_STAT		4

#define RTT_DIRECTION_UP		0
#define RTT_DIRECTION_DOWN		1

static const char * const jlink_so_name[] = {
	"/usr/lib/libjlinkarm.so",
	"/usr/lib/libjlinkarm.so.6",
	"/opt/SEGGER/JLink/libjlinkarm.so",
	"/opt/SEGGER/JLink/libjlinkarm.so.6",
};

struct rtt_desc {
	uint32_t index;
	uint32_t direction;
	char name[32];
	uint32_t size;
	uint32_t flags;
};

static struct rtt_desc rtt_desc;

typedef int (*jlink_emu_selectbyusbsn_func) (unsigned int sn);
typedef int (*jlink_open_func) (void);
typedef int (*jlink_execcommand_func) (char *in, char *out, int size);
typedef int (*jlink_tif_select_func) (int);
typedef void (*jlink_setspeed_func) (long int speed);
typedef int (*jlink_connect_func) (void);
typedef unsigned int (*jlink_getsn_func) (void);
typedef void (*jlink_emu_getproductname_func) (char *out, int size);
typedef int (*jlink_rtterminal_control_func) (int cmd, void *data);
typedef int (*jlink_rtterminal_read_func) (int cmd, char *buf, int size);

struct jlink {
	jlink_emu_selectbyusbsn_func emu_selectbyusbsn;
	jlink_open_func open;
	jlink_execcommand_func execcommand;
	jlink_tif_select_func tif_select;
	jlink_setspeed_func setspeed;
	jlink_connect_func connect;
	jlink_getsn_func getsn;
	jlink_emu_getproductname_func emu_getproductname;
	jlink_rtterminal_control_func rtterminal_control;
	jlink_rtterminal_read_func rtterminal_read;
};

static struct jlink jlink;

#ifndef NELEM
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#endif

int jlink_init(void)
{
	void *so;
	unsigned int i;

	for (i = 0; i < NELEM(jlink_so_name); i++) {
		so = dlopen(jlink_so_name[i], RTLD_LAZY);
		if (so)
			break;
	}

	if (!so)
		return -EIO;

	jlink.emu_selectbyusbsn = dlsym(so, "JLINK_EMU_SelectByUSBSN");
	jlink.open = dlsym(so, "JLINK_Open");
	jlink.execcommand = dlsym(so, "JLINK_ExecCommand");
	jlink.tif_select = dlsym(so, "JLINK_TIF_Select");
	jlink.setspeed = dlsym(so, "JLINK_SetSpeed");
	jlink.connect = dlsym(so, "JLINK_Connect");
	jlink.getsn = dlsym(so, "JLINK_GetSN");
	jlink.emu_getproductname = dlsym(so, "JLINK_EMU_GetProductName");
	jlink.rtterminal_control = dlsym(so, "JLINK_RTTERMINAL_Control");
	jlink.rtterminal_read = dlsym(so, "JLINK_RTTERMINAL_Read");

	if (!jlink.emu_selectbyusbsn || !jlink.open || !jlink.execcommand ||
			!jlink.tif_select || !jlink.setspeed ||
			!jlink.connect || !jlink.getsn ||
			!jlink.emu_getproductname ||
			!jlink.rtterminal_control || !jlink.rtterminal_read)
		return -EIO;

	return 0;
}

int jlink_connect(char *cfg)
{
	const char *device = NULL;
	int tif = 1;
	unsigned int speed = 1000;
	unsigned int serial_no = 0;
	char *tok;
	char buf[64];

	tok = strtok(cfg, ",");
	device = tok;

	tok = strtok(NULL, ",");
	if (!tok)
		goto connect;
	if (strlen(tok))
		serial_no = atoi(tok);

	tok = strtok(NULL, ",");
	if (!tok)
		goto connect;
	if (strlen(tok)) {
		if (!strcasecmp("swd", tok))
			tif = 1;
		else
			return -EINVAL;
	}

	tok = strtok(NULL, ",");
	if (!tok)
		goto connect;
	if (strlen(tok))
		speed = atoi(tok);

connect:
	if (serial_no)
		if (jlink.emu_selectbyusbsn(serial_no) < 0) {
			fprintf(stderr, "Failed to select emu by SN\n");
			return -ENODEV;
		}

	if (jlink.open() < 0) {
		fprintf(stderr, "Failed to open J-Link\n");
		return -ENODEV;
	}

	snprintf(buf, sizeof(buf), "device=%s", device);
	if (jlink.execcommand(buf, NULL, 0) < 0) {
		fprintf(stderr, "Failed to select target device\n");
		return -ENODEV;
	}

	if (jlink.tif_select(tif) < 0) {
		fprintf(stderr, "Failed to select target interface\n");
		return -ENODEV;
	}

	jlink.setspeed(speed);

	if (jlink.connect() < 0) {
		fprintf(stderr, "Failed to open target\n");
		return -EIO;
	}

	serial_no = jlink.getsn();
	jlink.emu_getproductname(buf, sizeof(buf));

	printf("Connected to %s (S/N: %u)\n", buf, serial_no);

	return 0;
}

int jlink_start_rtt(char *cfg)
{
	unsigned int address = 0;
	unsigned int area_size = 0;
	const char *buffer = "btmonitor";
	char *tok;
	char cmd[64];
	int rtt_dir;
	int count;
	int i;

	if (!cfg)
		goto find_rttcb;

	tok = strtok(cfg, ",");
	if (strlen(tok)) {
		address = strtol(tok, NULL, 0);
		area_size = 0x1000;
	}

	tok = strtok(NULL, ",");
	if (!tok)
		goto find_rttcb;
	if (strlen(tok))
		area_size = strtol(tok, NULL, 0);

	tok = strtok(NULL, ",");
	if (!tok)
		goto find_rttcb;
	if (strlen(tok))
		buffer = tok;

find_rttcb:
	if (address || area_size) {
		if (!area_size)
			snprintf(cmd, sizeof(cmd), "SetRTTAddr 0x%x", address);
		else
			snprintf(cmd, sizeof(cmd),
						"SetRTTSearchRanges 0x%x 0x%x",
						address, area_size);

		if (jlink.execcommand(cmd, NULL, 0) < 0)
			return -EIO;
	}

	if (jlink.rtterminal_control(RTT_CONTROL_START, NULL) < 0) {
		fprintf(stderr, "Failed to initialize RTT\n");
		return -1;
	}

	/* RTT may need some time to find control block so we need to wait */
	do {
		usleep(100);
		rtt_dir = RTT_DIRECTION_UP;
		count = jlink.rtterminal_control(RTT_CONTROL_GET_NUM_BUF,
								&rtt_dir);
	} while (count < 0);

	for (i = 0; i < count; i++) {
		memset(&rtt_desc, 0, sizeof(rtt_desc));
		rtt_desc.index = i;
		rtt_desc.direction = RTT_DIRECTION_UP;

		if (jlink.rtterminal_control(RTT_CONTROL_GET_DESC,
								&rtt_desc) < 0)
			continue;

		if (rtt_desc.size > 0 && !strcmp(buffer, rtt_desc.name))
			break;
	}

	if (i == count)
		return -ENODEV;

	printf("Using RTT up buffer #%d (size: %d)\n", i, rtt_desc.size);

	return 0;
}

int jlink_rtt_read(void *buf, size_t size)
{
	return jlink.rtterminal_read(rtt_desc.index, buf, size);
}
