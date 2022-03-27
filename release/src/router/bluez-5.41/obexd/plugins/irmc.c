/*
 *
 *  OBEX IrMC Sync Server
 *
 *  Copyright (C) 2010  Marcel Mol <marcel@mesa.nl>
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
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/log.h"
#include "obexd/src/obex.h"
#include "obexd/src/service.h"
#include "obexd/src/manager.h"
#include "obexd/src/mimetype.h"
#include "phonebook.h"
#include "filesystem.h"

struct aparam_header {
	uint8_t tag;
	uint8_t len;
	uint8_t val[0];
} __attribute__ ((packed));

#define DID_LEN 18

struct irmc_session {
	struct obex_session *os;
	struct apparam_field *params;
	uint16_t entries;
	GString *buffer;
	char sn[DID_LEN];
	char did[DID_LEN];
	char manu[DID_LEN];
	char model[DID_LEN];
	void *request;
};

#define IRMC_TARGET_SIZE 9

static const guint8 IRMC_TARGET[IRMC_TARGET_SIZE] = {
			0x49, 0x52, 0x4d, 0x43,  0x2d, 0x53, 0x59, 0x4e, 0x43 };

/* FIXME:
 * the IrMC specs state the first vcard should be the owner
 * vcard. As there is no simple way to collect ownerdetails
 * just create an empty vcard (which is allowed according to the
 * specs).
 */
static const char *owner_vcard =
		"BEGIN:VCARD\r\n"
		"VERSION:2.1\r\n"
		"N:\r\n"
		"TEL:\r\n"
		"X-IRMX-LUID:0\r\n"
		"END:VCARD\r\n";

static void phonebook_size_result(const char *buffer, size_t bufsize,
					int vcards, int missed,
					gboolean lastpart, void *user_data)
{
	struct irmc_session *irmc = user_data;

	DBG("vcards %d", vcards);

	irmc->params->maxlistcount = vcards;

	if (irmc->request) {
		phonebook_req_finalize(irmc->request);
		irmc->request = NULL;
	}
}

static void query_result(const char *buffer, size_t bufsize, int vcards,
				int missed, gboolean lastpart, void *user_data)
{
	struct irmc_session *irmc = user_data;
	const char *s, *t;

	DBG("bufsize %zu vcards %d missed %d", bufsize, vcards, missed);

	if (irmc->request) {
		phonebook_req_finalize(irmc->request);
		irmc->request = NULL;
	}

	/* first add a 'owner' vcard */
	if (!irmc->buffer)
		irmc->buffer = g_string_new(owner_vcard);
	else
		irmc->buffer = g_string_append(irmc->buffer, owner_vcard);

	if (buffer == NULL)
		goto done;

	/* loop around buffer and add X-IRMC-LUID attribs */
	s = buffer;
	while ((t = strstr(s, "UID:")) != NULL) {
		/* add up to UID: into buffer */
		irmc->buffer = g_string_append_len(irmc->buffer, s, t-s);
		/*
		 * add UID: line into buffer
		 * Not sure if UID is still needed if X-IRMC-LUID is there
		 */
		s = t;
		t = strstr(s, "\r\n");
		t += 2;
		irmc->buffer = g_string_append_len(irmc->buffer, s, t-s);
		/* add X-IRMC-LUID with same number as UID */
		irmc->buffer = g_string_append_len(irmc->buffer,
							"X-IRMC-LUID:", 12);
		s += 4; /* point to uid number */
		irmc->buffer = g_string_append_len(irmc->buffer, s, t-s);
		s = t;
	}
	/* add remaining bit of buffer */
	irmc->buffer = g_string_append(irmc->buffer, s);

done:
	obex_object_set_io_flags(irmc, G_IO_IN, 0);
}

static void *irmc_connect(struct obex_session *os, int *err)
{
	struct irmc_session *irmc;
	struct apparam_field *param;
	int ret;

	DBG("");

	manager_register_session(os);

	irmc = g_new0(struct irmc_session, 1);
	irmc->os = os;

	/* FIXME:
	 * Ideally get capabilities info here and use that to define
	 * IrMC DID and SN etc parameters.
	 * For now lets used hostname and some 'random' value
	 */
	gethostname(irmc->did, DID_LEN);
	strncpy(irmc->sn, "12345", sizeof(irmc->sn) - 1);
	strncpy(irmc->manu, "obex", sizeof(irmc->manu) - 1);
	strncpy(irmc->model, "mymodel", sizeof(irmc->model) - 1);

	/* We need to know the number of contact/cal/nt entries
	 * somewhere so why not do it now.
	 */
	param = g_new0(struct apparam_field, 1);
	param->maxlistcount = 0; /* to count the number of vcards... */
	param->filter = 0x200085; /* UID TEL N VERSION */
	irmc->params = param;
	irmc->request = phonebook_pull(PB_CONTACTS, irmc->params,
					phonebook_size_result, irmc, err);
	ret = phonebook_pull_read(irmc->request);
	if (err)
		*err = ret;

	return irmc;
}

static int irmc_get(struct obex_session *os, void *user_data)
{
	struct irmc_session *irmc = user_data;
	const char *type = obex_get_type(os);
	const char *name = obex_get_name(os);
	char *path;
	int ret;

	DBG("name %s type %s irmc %p", name, type ? type : "NA", irmc);

	path = g_strdup(name);

	ret = obex_get_stream_start(os, path);

	g_free(path);

	return ret;
}

static void irmc_disconnect(struct obex_session *os, void *user_data)
{
	struct irmc_session *irmc = user_data;

	DBG("");

	manager_unregister_session(os);

	if (irmc->params) {
		if (irmc->params->searchval)
			g_free(irmc->params->searchval);
		g_free(irmc->params);
	}

	if (irmc->buffer)
		g_string_free(irmc->buffer, TRUE);

	g_free(irmc);
}

static int irmc_chkput(struct obex_session *os, void *user_data)
{
	DBG("");
	/* Reject all PUTs */
	return -EBADR;
}

static int irmc_open_devinfo(struct irmc_session *irmc)
{
	if (!irmc->buffer)
		irmc->buffer = g_string_new("");

	g_string_append_printf(irmc->buffer,
				"MANU:%s\r\n"
				"MOD:%s\r\n"
				"SN:%s\r\n"
				"IRMC-VERSION:1.1\r\n"
				"PB-TYPE-TX:VCARD2.1\r\n"
				"PB-TYPE-RX:NONE\r\n"
				"CAL-TYPE-TX:NONE\r\n"
				"CAL-TYPE-RX:NONE\r\n"
				"MSG-TYPE-TX:NONE\r\n"
				"MSG-TYPE-RX:NONE\r\n"
				"NOTE-TYPE-TX:NONE\r\n"
				"NOTE-TYPE-RX:NONE\r\n",
				irmc->manu, irmc->model, irmc->sn);

	return 0;
}

static int irmc_open_pb(struct irmc_session *irmc)
{
	int ret;

	/* how can we tell if the vcard count call already finished? */
	irmc->request = phonebook_pull(PB_CONTACTS, irmc->params,
						query_result, irmc, &ret);
	if (ret < 0) {
		DBG("phonebook_pull failed...");
		return ret;
	}

	ret = phonebook_pull_read(irmc->request);
	if (ret < 0) {
		DBG("phonebook_pull_read failed...");
		return ret;
	}

	return 0;
}

static int irmc_open_info(struct irmc_session *irmc)
{
	if (irmc->buffer == NULL)
		irmc->buffer = g_string_new("");

	g_string_printf(irmc->buffer, "Total-Records:%d\r\n"
				"Maximum-Records:%d\r\n"
				"IEL:2\r\n"
				"DID:%s\r\n",
				irmc->params->maxlistcount,
				irmc->params->maxlistcount, irmc->did);

	return 0;
}

static int irmc_open_cc(struct irmc_session *irmc)
{
	if (irmc->buffer == NULL)
		irmc->buffer = g_string_new("");

	g_string_printf(irmc->buffer, "%d\r\n", irmc->params->maxlistcount);

	return 0;
}

static int irmc_open_cal(struct irmc_session *irmc)
{
	/* no suport yet. Just return an empty buffer. cal.vcs */
	DBG("unsupported, returning empty buffer");

	if (!irmc->buffer)
		irmc->buffer = g_string_new("");

	return 0;
}

static int irmc_open_nt(struct irmc_session *irmc)
{
	/* no suport yet. Just return an empty buffer. nt.vnt */
	DBG("unsupported, returning empty buffer");

	if (!irmc->buffer)
		irmc->buffer = g_string_new("");

	return 0;
}

static int irmc_open_luid(struct irmc_session *irmc)
{
	if (irmc->buffer == NULL)
		irmc->buffer = g_string_new("");

	DBG("changelog request, force whole book");
	g_string_printf(irmc->buffer, "SN:%s\r\n"
					"DID:%s\r\n"
					"Total-Records:%d\r\n"
					"Maximum-Records:%d\r\n"
					"*\r\n",
					irmc->sn, irmc->did,
					irmc->params->maxlistcount,
					irmc->params->maxlistcount);

	return 0;
}

static void *irmc_open(const char *name, int oflag, mode_t mode, void *context,
							size_t *size, int *err)
{
	struct irmc_session *irmc = context;
	int ret = 0;
	char *path;

	DBG("name %s context %p", name, context);

	if (oflag != O_RDONLY) {
		ret = -EPERM;
		goto fail;
	}

	if (name == NULL) {
		ret = -EBADR;
		goto fail;
	}

	/* Always contains the absolute path */
	if (g_path_is_absolute(name))
		path = g_strdup(name);
	else
		path = g_build_filename("/", name, NULL);

	if (g_str_equal(path, PB_DEVINFO))
		ret = irmc_open_devinfo(irmc);
	else if (g_str_equal(path, PB_CONTACTS))
		ret = irmc_open_pb(irmc);
	else if (g_str_equal(path, PB_INFO_LOG))
		ret = irmc_open_info(irmc);
	else if (g_str_equal(path, PB_CC_LOG))
		ret = irmc_open_cc(irmc);
	else if (g_str_has_prefix(path, PB_CALENDAR_FOLDER))
		ret = irmc_open_cal(irmc);
	else if (g_str_has_prefix(path, PB_NOTES_FOLDER))
		ret = irmc_open_nt(irmc);
	else if (g_str_has_prefix(path, PB_LUID_FOLDER))
		ret = irmc_open_luid(irmc);
	else
		ret = -EBADR;

	g_free(path);

	if (ret == 0)
		return irmc;

fail:
	if (err)
		*err = ret;

	return NULL;
}

static int irmc_close(void *object)
{
	struct irmc_session *irmc = object;

	DBG("");

	if (irmc->buffer) {
		g_string_free(irmc->buffer, TRUE);
		irmc->buffer = NULL;
	}

	if (irmc->request) {
		phonebook_req_finalize(irmc->request);
		irmc->request = NULL;
	}

	return 0;
}

static ssize_t irmc_read(void *object, void *buf, size_t count)
{
	struct irmc_session *irmc = object;
	int len;

	DBG("buffer %p count %zu", irmc->buffer, count);
	if (!irmc->buffer)
                return -EAGAIN;

	len = string_read(irmc->buffer, buf, count);
	DBG("returning %d bytes", len);
	return len;
}

static struct obex_mime_type_driver irmc_driver = {
	.target = IRMC_TARGET,
	.target_size = IRMC_TARGET_SIZE,
	.open = irmc_open,
	.close = irmc_close,
	.read = irmc_read,
};

static struct obex_service_driver irmc = {
	.name = "IRMC Sync server",
	.service = OBEX_IRMC,
	.target = IRMC_TARGET,
	.target_size = IRMC_TARGET_SIZE,
	.connect = irmc_connect,
	.get = irmc_get,
	.disconnect = irmc_disconnect,
	.chkput = irmc_chkput
};

static int irmc_init(void)
{
	int err;

	DBG("");
	err = phonebook_init();
	if (err < 0)
		return err;

	err = obex_mime_type_driver_register(&irmc_driver);
	if (err < 0)
		goto fail_mime_irmc;

	err = obex_service_driver_register(&irmc);
	if (err < 0)
		goto fail_irmc_reg;

	return 0;

fail_irmc_reg:
	obex_mime_type_driver_unregister(&irmc_driver);
fail_mime_irmc:
	phonebook_exit();

	return err;
}

static void irmc_exit(void)
{
	DBG("");
	obex_service_driver_unregister(&irmc);
	obex_mime_type_driver_unregister(&irmc_driver);
	phonebook_exit();
}

OBEX_PLUGIN_DEFINE(irmc, irmc_init, irmc_exit)
