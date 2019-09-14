/* gss-misc.c
 * This is a miscellaneous helper class for gss-api features.
 *
 * Copyright 2007-2017 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fnmatch.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <gssapi/gssapi.h>
#include "dirty.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "net.h"
#include "template.h"
#include "msg.h"
#include "module-template.h"
#include "obj.h"
#include "errmsg.h"
#include "gss-misc.h"
#include "debug.h"
#include "glbl.h"
#include "unlimited_select.h"

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)

static void display_status_(char *m, OM_uint32 code, int type)
{
	OM_uint32 maj_stat, min_stat, msg_ctx = 0;
	gss_buffer_desc msg;

	do {
		maj_stat = gss_display_status(&min_stat, code, type, GSS_C_NO_OID, &msg_ctx, &msg);
		if (maj_stat != GSS_S_COMPLETE) {
			LogError(0, NO_ERRCODE, "GSS-API error in gss_display_status called from <%s>\n", m);
			break;
		} else {
			char buf[1024];
			snprintf(buf, sizeof(buf), "GSS-API error %s: %s\n", m, (char *) msg.value);
			buf[sizeof(buf) - 1] = '\0';
			LogError(0, NO_ERRCODE, "%s", buf);
		}
		if (msg.length != 0)
			gss_release_buffer(&min_stat, &msg);
	} while (msg_ctx);
}


static void display_status(char *m, OM_uint32 maj_stat, OM_uint32 min_stat)
{
	display_status_(m, maj_stat, GSS_C_GSS_CODE);
	display_status_(m, min_stat, GSS_C_MECH_CODE);
}


static void display_ctx_flags(OM_uint32 flags)
{
	if (flags & GSS_C_DELEG_FLAG)
		dbgprintf("GSS_C_DELEG_FLAG\n");
	if (flags & GSS_C_MUTUAL_FLAG)
		dbgprintf("GSS_C_MUTUAL_FLAG\n");
	if (flags & GSS_C_REPLAY_FLAG)
		dbgprintf("GSS_C_REPLAY_FLAG\n");
	if (flags & GSS_C_SEQUENCE_FLAG)
		dbgprintf("GSS_C_SEQUENCE_FLAG\n");
	if (flags & GSS_C_CONF_FLAG)
		dbgprintf("GSS_C_CONF_FLAG\n");
	if (flags & GSS_C_INTEG_FLAG)
		dbgprintf("GSS_C_INTEG_FLAG\n");
}


static int read_all(int fd, char *buf, unsigned int nbyte)
{
	int ret;
	char *ptr;
	struct timeval tv;
#ifdef USE_UNLIMITED_SELECT
	fd_set *pRfds = malloc(glbl.GetFdSetSize());

	if (pRfds == NULL)
		return -1;
#else
	fd_set rfds;
	fd_set *pRfds = &rfds;
#endif

	for (ptr = buf; nbyte; ptr += ret, nbyte -= ret) {
		FD_ZERO(pRfds);
		FD_SET(fd, pRfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		if ((ret = select(FD_SETSIZE, pRfds, NULL, NULL, &tv)) <= 0
						|| !FD_ISSET(fd, pRfds)) {
			freeFdSet(pRfds);
			return ret;
		}
		ret = recv(fd, ptr, nbyte, 0);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			freeFdSet(pRfds);
			return (ret);
		} else if (ret == 0) {
			freeFdSet(pRfds);
			return (ptr - buf);
		}
	}

	freeFdSet(pRfds);
	return (ptr - buf);
}


static int write_all(int fd, char *buf, unsigned int nbyte)
{
	int     ret;
	char   *ptr;

	for (ptr = buf; nbyte; ptr += ret, nbyte -= ret) {
		ret = send(fd, ptr, nbyte, 0);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return (ret);
		} else if (ret == 0) {
			return (ptr - buf);
		}
	}

	return (ptr - buf);
}


static int recv_token(int s, gss_buffer_t tok)
{
	int ret;
	unsigned char lenbuf[4] = "xxx"; // initialized to make clang static analyzer happy
	unsigned int len;

	ret = read_all(s, (char *) lenbuf, 4);
	if (ret < 0) {
		LogError(0, NO_ERRCODE, "GSS-API error reading token length");
		return -1;
	} else if (!ret) {
		return 0;
	} else if (ret != 4) {
		LogError(0, NO_ERRCODE, "GSS-API error reading token length");
		return -1;
	}

	len = ((lenbuf[0] << 24)
	       | (lenbuf[1] << 16)
	       | (lenbuf[2] << 8)
	       | lenbuf[3]);
	tok->length = ntohl(len);

	tok->value = (char *) MALLOC(tok->length ? tok->length : 1);
	if (tok->length && tok->value == NULL) {
		LogError(0, NO_ERRCODE, "Out of memory allocating token data\n");
		return -1;
	}

	ret = read_all(s, (char *) tok->value, tok->length);
	if (ret < 0) {
		LogError(0, NO_ERRCODE, "GSS-API error reading token data");
		free(tok->value);
		return -1;
	} else if (ret != (int) tok->length) {
		LogError(0, NO_ERRCODE, "GSS-API error reading token data");
		free(tok->value);
		return -1;
	}

	return 1;
}


static int send_token(int s, gss_buffer_t tok)
{
	int     ret;
	unsigned char lenbuf[4];
	unsigned int len;

	if (tok->length > 0xffffffffUL)
		abort();  /* TODO: we need to reconsider this, abort() is not really
				a solution - degrade, but keep running */
	len = htonl(tok->length);
	lenbuf[0] = (len >> 24) & 0xff;
	lenbuf[1] = (len >> 16) & 0xff;
	lenbuf[2] = (len >> 8) & 0xff;
	lenbuf[3] = len & 0xff;

	ret = write_all(s, (char *) lenbuf, 4);
	if (ret < 0) {
		LogError(0, NO_ERRCODE, "GSS-API error sending token length");
		return -1;
	} else if (ret != 4) {
		LogError(0, NO_ERRCODE, "GSS-API error sending token length");
		return -1;
	}

	ret = write_all(s, tok->value, tok->length);
	if (ret < 0) {
		LogError(0, NO_ERRCODE, "GSS-API error sending token data");
		return -1;
	} else if (ret != (int) tok->length) {
		LogError(0, NO_ERRCODE, "GSS-API error sending token data");
		return -1;
	}

	return 0;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(gssutil)
CODESTARTobjQueryInterface(gssutil)
	if(pIf->ifVersion != gssutilCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->recv_token = recv_token;
	pIf->send_token = send_token;
	pIf->display_status = display_status;
	pIf->display_ctx_flags = display_ctx_flags;

finalize_it:
ENDobjQueryInterface(gssutil)


/* exit our class
 * rgerhards, 2008-03-10
 */
BEGINObjClassExit(gssutil, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(gssutil)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
ENDObjClassExit(gssutil)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-29
 */
BEGINAbstractObjClassInit(gssutil, 1, OBJ_IS_LOADABLE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
ENDObjClassInit(gssutil)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
	gssutilClassExit();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(gssutilClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
ENDmodInit
