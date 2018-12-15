/* imgssapi.c
 * This is the implementation of the GSSAPI input module.
 *
 * Note: the root gssapi code was contributed by varmojfekoj and is most often
 * maintened by him. I am just doing the plumbing around it (I event don't have a
 * test lab for gssapi yet... ). I am very grateful for this useful code
 * contribution -- rgerhards, 2008-03-05
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * Copyright 2007, 2017 Rainer Gerhards and Adiscon GmbH.
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <gssapi/gssapi.h>
#include "rsyslog.h"
#include "dirty.h"
#include "cfsysline.h"
#include "module-template.h"
#include "unicode-helper.h"
#include "net.h"
#include "srUtils.h"
#include "gss-misc.h"
#include "tcpsrv.h"
#include "tcps_sess.h"
#include "errmsg.h"
#include "netstrm.h"
#include "glbl.h"
#include "debug.h"
#include "unlimited_select.h"


MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP

/* defines */
#define ALLOWEDMETHOD_GSS 2
#define ALLOWEDMETHOD_TCP 1


/* some forward definitions - they may go away when we no longer include imtcp.c */
static rsRetVal addGSSListener(void __attribute__((unused)) *pVal, uchar *pNewVal);
static rsRetVal actGSSListener(uchar *port);
static int TCPSessGSSInit(void);
static void TCPSessGSSClose(tcps_sess_t* pSess);
static rsRetVal TCPSessGSSRecv(tcps_sess_t *pSess, void *buf, size_t buf_len, ssize_t *);
static rsRetVal onSessAccept(tcpsrv_t *pThis, tcps_sess_t *ppSess);
static rsRetVal OnSessAcceptGSS(tcpsrv_t *pThis, tcps_sess_t *ppSess);

/* static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(tcpsrv)
DEFobjCurrIf(tcps_sess)
DEFobjCurrIf(gssutil)
DEFobjCurrIf(netstrm)
DEFobjCurrIf(net)
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)

static tcpsrv_t *pOurTcpsrv = NULL;  /* our TCP server(listener) TODO: change for multiple instances */
static gss_cred_id_t gss_server_creds = GSS_C_NO_CREDENTIAL;
static uchar *srvPort;

/* our usr structure for the tcpsrv object */
typedef struct gsssrv_s {
	char allowedMethods;
} gsssrv_t;

/* our usr structure for the session object */
typedef struct gss_sess_s {
	OM_uint32 gss_flags;
	gss_ctx_id_t gss_context;
	char allowedMethods;
} gss_sess_t;


/* config variables */
struct modConfData_s {
	EMPTY_STRUCT;
};

static uchar *pszLstnPortFileName = NULL;	/* file for dynamic port */
static int iTCPSessMax = 200; /* max number of sessions */
static char *gss_listen_service_name = NULL;
static int bPermitPlainTcp = 0; /* plain tcp syslog allowed on GSSAPI port? */
static int bKeepAlive = 0; /* use SO_KEEPALIVE? */


/* methods */
/* callbacks */
static rsRetVal OnSessConstructFinalize(void *ppUsr)
{
	DEFiRet;
	gss_sess_t **ppGSess = (gss_sess_t**) ppUsr;
	gss_sess_t *pGSess;

	assert(ppGSess != NULL);

	if((pGSess = calloc(1, sizeof(gss_sess_t))) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	pGSess->gss_flags = 0;
	pGSess->gss_context = GSS_C_NO_CONTEXT;
	pGSess->allowedMethods = 0;

	*ppGSess = pGSess;

finalize_it:
	RETiRet;
}


/* Destruct the user session pointer for a GSSAPI session. Please note
 * that it *is* valid to receive a NULL user pointer. In this case, the
 * sessions is to be torn down before it was fully initialized. This
 * happens in error cases, e.g. when the host ACL did not match.
 * rgerhards, 2008-03-03
 */
static rsRetVal
OnSessDestruct(void *ppUsr)
{
	DEFiRet;
	gss_sess_t **ppGSess = (gss_sess_t**) ppUsr;

	assert(ppGSess != NULL);
	if(*ppGSess == NULL)
		FINALIZE;

	if((*ppGSess)->allowedMethods & ALLOWEDMETHOD_GSS) {
		OM_uint32 maj_stat, min_stat;
		maj_stat = gss_delete_sec_context(&min_stat, &(*ppGSess)->gss_context, GSS_C_NO_BUFFER);
		if (maj_stat != GSS_S_COMPLETE)
			gssutil.display_status((char*)"deleting context", maj_stat, min_stat);
	}

	free(*ppGSess);
	*ppGSess = NULL;

finalize_it:
	RETiRet;
}


/* Check if the host is permitted to send us messages.
 * Note: the pUsrSess may be zero if the server is running in tcp-only mode!
 */
static int
isPermittedHost(struct sockaddr *addr, char *fromHostFQDN, void *pUsrSrv, void*pUsrSess)
{
	gsssrv_t *pGSrv;
	gss_sess_t *pGSess;
	char allowedMethods = 0;

	BEGINfunc
	assert(pUsrSrv != NULL);
	pGSrv = (gsssrv_t*) pUsrSrv;
	pGSess = (gss_sess_t*) pUsrSess;

	if((pGSrv->allowedMethods & ALLOWEDMETHOD_TCP) &&
	   net.isAllowedSender2((uchar*)"TCP", addr, (char*)fromHostFQDN, 1))
		allowedMethods |= ALLOWEDMETHOD_TCP;
	if((pGSrv->allowedMethods & ALLOWEDMETHOD_GSS) &&
	   net.isAllowedSender2((uchar*)"GSS", addr, (char*)fromHostFQDN, 1))
		allowedMethods |= ALLOWEDMETHOD_GSS;
	if(allowedMethods && pGSess != NULL)
		pGSess->allowedMethods = allowedMethods;
	ENDfunc
	return allowedMethods;
}


static rsRetVal
onSessAccept(tcpsrv_t *pThis, tcps_sess_t *pSess)
{
	DEFiRet;
	gsssrv_t *pGSrv;
	
	pGSrv = (gsssrv_t*) pThis->pUsr;

	if(pGSrv->allowedMethods & ALLOWEDMETHOD_GSS) {
		iRet = OnSessAcceptGSS(pThis, pSess);
	}

	RETiRet;
}


static rsRetVal
onRegularClose(tcps_sess_t *pSess)
{
	DEFiRet;
	gss_sess_t *pGSess;

	assert(pSess != NULL);
	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	if(pGSess->allowedMethods & ALLOWEDMETHOD_GSS)
		TCPSessGSSClose(pSess);
	else {
		/* process any incomplete frames left over */
		tcps_sess.PrepareClose(pSess);
		/* Session closed */
		tcps_sess.Close(pSess);
	}
	RETiRet;
}


static rsRetVal
onErrClose(tcps_sess_t *pSess)
{
	DEFiRet;
	gss_sess_t *pGSess;

	assert(pSess != NULL);
	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	if(pGSess->allowedMethods & ALLOWEDMETHOD_GSS)
		TCPSessGSSClose(pSess);
	else
		tcps_sess.Close(pSess);

	RETiRet;
}


/* open the listen sockets */
static rsRetVal
doOpenLstnSocks(tcpsrv_t *pSrv)
{
	gsssrv_t *pGSrv;
	DEFiRet;

	ISOBJ_TYPE_assert(pSrv, tcpsrv);
	pGSrv = pSrv->pUsr;
	assert(pGSrv != NULL);

	/* first apply some config settings */
	if(pGSrv->allowedMethods) {
		if(pGSrv->allowedMethods & ALLOWEDMETHOD_GSS) {
			if(TCPSessGSSInit()) {
				LogError(0, NO_ERRCODE, "GSS-API initialization failed\n");
				pGSrv->allowedMethods &= ~(ALLOWEDMETHOD_GSS);
			}
		}
		if(pGSrv->allowedMethods) {
			/* fallback to plain TCP */
			CHKiRet(tcpsrv.create_tcp_socket(pSrv));
		} else {
			ABORT_FINALIZE(RS_RET_GSS_ERR);
		}
	}

finalize_it:
	RETiRet;
}


static rsRetVal
doRcvData(tcps_sess_t *pSess, char *buf, size_t lenBuf, ssize_t *piLenRcvd, int *const oserr)
{
	DEFiRet;
	int allowedMethods;
	gss_sess_t *pGSess;

	assert(pSess != NULL);
	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;
	assert(piLenRcvd != NULL);

	allowedMethods = pGSess->allowedMethods;
	if(allowedMethods & ALLOWEDMETHOD_GSS) {
		CHKiRet(TCPSessGSSRecv(pSess, buf, lenBuf, piLenRcvd));
	} else {
		*piLenRcvd = lenBuf;
		CHKiRet(netstrm.Rcv(pSess->pStrm, (uchar*) buf, piLenRcvd, oserr));
	}

finalize_it:
	RETiRet;
}


/* end callbacks */

static rsRetVal
addGSSListener(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	DEFiRet;

	if((ustrcmp(pNewVal, UCHAR_CONSTANT("0")) == 0 && pszLstnPortFileName == NULL)
			|| ustrcmp(pNewVal, UCHAR_CONSTANT("0")) < 0) {
		CHKmalloc(srvPort = (uchar*)strdup("514"));
	} else {
		srvPort = pNewVal;
	}

finalize_it:
	RETiRet;
}

static rsRetVal
actGSSListener(uchar *port)
{
	DEFiRet;
	gsssrv_t *pGSrv = NULL;

	if(pOurTcpsrv == NULL) {
		/* first create/init the gsssrv "object" */
		if((pGSrv = calloc(1, sizeof(gsssrv_t))) == NULL)
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

		pGSrv->allowedMethods = ALLOWEDMETHOD_GSS;
		if(bPermitPlainTcp)
			pGSrv->allowedMethods |= ALLOWEDMETHOD_TCP;
		/* gsssrv initialized */

		CHKiRet(tcpsrv.Construct(&pOurTcpsrv));
		CHKiRet(tcpsrv.SetUsrP(pOurTcpsrv, pGSrv));
		CHKiRet(tcpsrv.SetCBOnSessConstructFinalize(pOurTcpsrv, OnSessConstructFinalize));
		CHKiRet(tcpsrv.SetCBOnSessDestruct(pOurTcpsrv, OnSessDestruct));
		CHKiRet(tcpsrv.SetCBIsPermittedHost(pOurTcpsrv, isPermittedHost));
		CHKiRet(tcpsrv.SetCBRcvData(pOurTcpsrv, doRcvData));
		CHKiRet(tcpsrv.SetCBOpenLstnSocks(pOurTcpsrv, doOpenLstnSocks));
		CHKiRet(tcpsrv.SetCBOnSessAccept(pOurTcpsrv, onSessAccept));
		CHKiRet(tcpsrv.SetCBOnRegularClose(pOurTcpsrv, onRegularClose));
		CHKiRet(tcpsrv.SetCBOnErrClose(pOurTcpsrv, onErrClose));
		CHKiRet(tcpsrv.SetInputName(pOurTcpsrv, UCHAR_CONSTANT("imgssapi")));
		CHKiRet(tcpsrv.SetKeepAlive(pOurTcpsrv, bKeepAlive));
		CHKiRet(tcpsrv.SetOrigin(pOurTcpsrv, UCHAR_CONSTANT("imgssapi")));
		tcpsrv.configureTCPListen(pOurTcpsrv, port, 1, NULL);
		CHKiRet(tcpsrv.ConstructFinalize(pOurTcpsrv));
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(0, NO_ERRCODE, "error %d trying to add listener", iRet);
		if(pOurTcpsrv != NULL)
			tcpsrv.Destruct(&pOurTcpsrv);
		free(pGSrv);
	}
	RETiRet;
}


/* returns 0 if all went OK, -1 if it failed */
static int TCPSessGSSInit(void)
{
	gss_buffer_desc name_buf;
	gss_name_t server_name;
	OM_uint32 maj_stat, min_stat;

	if (gss_server_creds != GSS_C_NO_CREDENTIAL)
		return 0;

	name_buf.value = (gss_listen_service_name == NULL) ? (char*)"host" : gss_listen_service_name;
	name_buf.length = strlen(name_buf.value) + 1;
	maj_stat = gss_import_name(&min_stat, &name_buf, GSS_C_NT_HOSTBASED_SERVICE, &server_name);
	if (maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status((char*)"importing name", maj_stat, min_stat);
		return -1;
	}

	maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
				    GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
				    &gss_server_creds, NULL, NULL);
	if (maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status((char*)"acquiring credentials", maj_stat, min_stat);
		return -1;
	}

	gss_release_name(&min_stat, &server_name);
	dbgprintf("GSS-API initialized\n");
	return 0;
}


/* returns 0 if all went OK, -1 if it failed
 * tries to guess if the connection uses gssapi.
 */
static rsRetVal
OnSessAcceptGSS(tcpsrv_t *pThis, tcps_sess_t *pSess)
{
	DEFiRet;
	gss_buffer_desc send_tok, recv_tok;
	gss_name_t client;
	OM_uint32 maj_stat, min_stat, acc_sec_min_stat;
	gss_ctx_id_t *context;
	OM_uint32 *sess_flags;
	int fdSess;
	char allowedMethods;
	gsssrv_t *pGSrv;
	gss_sess_t *pGSess;
	uchar *pszPeer = NULL;
	int lenPeer = 0;
	char *buf = NULL;

	assert(pSess != NULL);

	pGSrv = (gsssrv_t*) pThis->pUsr;
	pGSess = (gss_sess_t*) pSess->pUsr;
	allowedMethods = pGSrv->allowedMethods;
	if(allowedMethods & ALLOWEDMETHOD_GSS) {
		int ret = 0;
		const size_t bufsize = glbl.GetMaxLine();
		CHKmalloc(buf = (char*) MALLOC(bufsize + 1));

		prop.GetString(pSess->fromHostIP, &pszPeer, &lenPeer);

		dbgprintf("GSS-API Trying to accept TCP session %p from %s\n", pSess, (char *)pszPeer);

		CHKiRet(netstrm.GetSock(pSess->pStrm, &fdSess)); // TODO: method access!
		if (allowedMethods & ALLOWEDMETHOD_TCP) {
			int len;
			struct timeval tv;
#ifdef USE_UNLIMITED_SELECT
			fd_set *pFds = malloc(glbl.GetFdSetSize());
#else
			fd_set fds;
			fd_set *pFds = &fds;
#endif

			do {
				FD_ZERO(pFds);
				FD_SET(fdSess, pFds);
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				ret = select(fdSess + 1, pFds, NULL, NULL, &tv);
			} while (ret < 0 && errno == EINTR);
			if (ret < 0) {
				LogError(0, RS_RET_ERR, "TCP session %p from %s will be "
						"closed, error ignored\n", pSess, (char *)pszPeer);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			} else if (ret == 0) {
				dbgprintf("GSS-API Reverting to plain TCP\n");
				pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
				ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
			}

			do {
				ret = recv(fdSess, buf, bufsize, MSG_PEEK);
			} while (ret < 0 && errno == EINTR);
			if (ret <= 0) {
				if (ret == 0) {
					dbgprintf("GSS-API Connection closed by peer %s\n", (char *)pszPeer);
				} else {
					LogError(0, RS_RET_ERR, "TCP(GSS) session %p from %s will be closed, "
					"error ignored\n", pSess, (char *)pszPeer);
				}
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			}

			if (ret < 4) {
				dbgprintf("GSS-API Reverting to plain TCP from %s\n", (char *)pszPeer);
				pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
				ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
			} else if (ret == 4) {
				/* The client might has been interupted after sending
				 * the data length (4B), give him another chance.
				 */
				srSleep(1, 0);
				do {
					ret = recv(fdSess, buf, bufsize, MSG_PEEK);
				} while (ret < 0 && errno == EINTR);
				if (ret <= 0) {
					if (ret == 0) {
						dbgprintf("GSS-API Connection closed by peer %s\n", (char *)pszPeer);
					} else {
						LogError(0, NO_ERRCODE, "TCP session %p from %s will be "
						"closed, error ignored\n", pSess, (char *)pszPeer);
					}
					ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
				}
			}

			/* TODO: how does this work together with IPv6? Does it? */
			len = ntohl((buf[0] << 24)
				    | (buf[1] << 16)
				    | (buf[2] << 8)
				    | buf[3]);
			if ((ret - 4) < len || len == 0) {
				dbgprintf("GSS-API Reverting to plain TCP from %s\n", (char *)pszPeer);
				pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
				ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
			}

			freeFdSet(pFds);
		}

		context = &pGSess->gss_context;
		*context = GSS_C_NO_CONTEXT;
		sess_flags = &pGSess->gss_flags;
		do {
			if (gssutil.recv_token(fdSess, &recv_tok) <= 0) {
				LogError(0, NO_ERRCODE, "TCP session %p from %s will be "
						"closed, error ignored\n", pSess, (char *)pszPeer);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			}
			maj_stat = gss_accept_sec_context(&acc_sec_min_stat, context, gss_server_creds,
							  &recv_tok, GSS_C_NO_CHANNEL_BINDINGS, &client,
							  NULL, &send_tok, sess_flags, NULL, NULL);
			if (recv_tok.value) {
				free(recv_tok.value);
				recv_tok.value = NULL;
			}
			if (maj_stat != GSS_S_COMPLETE && maj_stat != GSS_S_CONTINUE_NEEDED) {
				gss_release_buffer(&min_stat, &send_tok);
				if (*context != GSS_C_NO_CONTEXT)
					gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
				if ((allowedMethods & ALLOWEDMETHOD_TCP) &&
				    (GSS_ROUTINE_ERROR(maj_stat) == GSS_S_DEFECTIVE_TOKEN)) {
					dbgprintf("GSS-API Reverting to plain TCP from %s\n", (char *)pszPeer);
					dbgprintf("tcp session socket with new data: #%d\n", fdSess);
					if(tcps_sess.DataRcvd(pSess, buf, ret) != RS_RET_OK) {
						LogError(0, NO_ERRCODE, "Tearing down TCP "
							"Session %p from %s - see previous messages "
							"for reason(s)\n", pSess, (char *)pszPeer);
						ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
					}
					pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
					ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
				}
				gssutil.display_status((char*)"accepting context", maj_stat, acc_sec_min_stat);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			}
			if (send_tok.length != 0) {
				if(gssutil.send_token(fdSess, &send_tok) < 0) {
					gss_release_buffer(&min_stat, &send_tok);
					LogError(0, NO_ERRCODE, "TCP session %p from %s will be "
							"closed, error ignored\n", pSess, (char *)pszPeer);
					if (*context != GSS_C_NO_CONTEXT)
						gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
					ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
				}
				gss_release_buffer(&min_stat, &send_tok);
			}
		} while (maj_stat == GSS_S_CONTINUE_NEEDED);

		maj_stat = gss_display_name(&min_stat, client, &recv_tok, NULL);
		if (maj_stat != GSS_S_COMPLETE) {
			gssutil.display_status((char*)"displaying name", maj_stat, min_stat);
		} else {
			dbgprintf("GSS-API Accepted connection from peer %s: %s\n", (char *)pszPeer,
				(char*) recv_tok.value);
		}
		gss_release_name(&min_stat, &client);
		gss_release_buffer(&min_stat, &recv_tok);

		dbgprintf("GSS-API Provided context flags:\n");
		gssutil.display_ctx_flags(*sess_flags);
		pGSess->allowedMethods = ALLOWEDMETHOD_GSS;
	}
	
finalize_it:
	free(buf);
	RETiRet;
}


/* Replaces recv() for gssapi connections.
 */
int TCPSessGSSRecv(tcps_sess_t *pSess, void *buf, size_t buf_len, ssize_t *piLenRcvd)
{
	DEFiRet;
	gss_buffer_desc xmit_buf, msg_buf;
	gss_ctx_id_t *context;
	OM_uint32 maj_stat, min_stat;
	int fdSess;
	int     conf_state;
	int state;
	gss_sess_t *pGSess;

	assert(pSess->pUsr != NULL);
	assert(piLenRcvd != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	netstrm.GetSock(pSess->pStrm, &fdSess); // TODO: method access, CHKiRet!
	if ((state = gssutil.recv_token(fdSess, &xmit_buf)) <= 0)
		ABORT_FINALIZE(RS_RET_GSS_ERR);

	context = &pGSess->gss_context;
	maj_stat = gss_unwrap(&min_stat, *context, &xmit_buf, &msg_buf,
			      &conf_state, (gss_qop_t *) NULL);
	if(maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status((char*)"unsealing message", maj_stat, min_stat);
		if(xmit_buf.value) {
			free(xmit_buf.value);
			xmit_buf.value = 0;
		}
		ABORT_FINALIZE(RS_RET_GSS_ERR);
	}
	if (xmit_buf.value) {
		free(xmit_buf.value);
		xmit_buf.value = 0;
	}

	*piLenRcvd = msg_buf.length < buf_len ? msg_buf.length : buf_len;
	memcpy(buf, msg_buf.value, *piLenRcvd);
	gss_release_buffer(&min_stat, &msg_buf);

finalize_it:
	RETiRet;
}


/* Takes care of cleaning up gssapi stuff and then calls
 * TCPSessClose().
 */
void TCPSessGSSClose(tcps_sess_t* pSess)
{
	OM_uint32 maj_stat, min_stat;
	gss_ctx_id_t *context;
	gss_sess_t *pGSess;

	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	context = &pGSess->gss_context;
	maj_stat = gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
	if (maj_stat != GSS_S_COMPLETE)
		gssutil.display_status((char*)"deleting context", maj_stat, min_stat);
	*context = GSS_C_NO_CONTEXT;
	pGSess->gss_flags = 0;
	pGSess->allowedMethods = 0;

	tcps_sess.Close(pSess);
}


/* Counterpart of TCPSessGSSInit(). This is called to exit the GSS system
 * at all. It is a server-based session exit.
 */
static rsRetVal
TCPSessGSSDeinit(void)
{
	DEFiRet;
	OM_uint32 maj_stat, min_stat;

	if (gss_server_creds != GSS_C_NO_CREDENTIAL) {
		maj_stat = gss_release_cred(&min_stat, &gss_server_creds);
		if (maj_stat != GSS_S_COMPLETE)
			gssutil.display_status((char*)"releasing credentials", maj_stat, min_stat);
	}
	RETiRet;
}


#if 0 /* can be used to integrate into new config system */
BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
ENDbeginCnfLoad


BEGINendCnfLoad
CODESTARTendCnfLoad
ENDendCnfLoad


BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf


BEGINactivateCnf
CODESTARTactivateCnf
ENDactivateCnf


BEGINfreeCnf
CODESTARTfreeCnf
ENDfreeCnf
#endif

/* This function is called to gather input.
 */
BEGINrunInput
CODESTARTrunInput
	/* This will fail if the priviledges are dropped. Should be
	 * moved to the '*activateCnfPrePrivDrop' section eventually.
	 */
	actGSSListener(srvPort);

	iRet = tcpsrv.Run(pOurTcpsrv);
ENDrunInput


/* initialize and return if will run or not */
BEGINwillRun
CODESTARTwillRun
	if(srvPort == NULL)
		ABORT_FINALIZE(RS_RET_NO_RUN);

	net.PrintAllowedSenders(2); /* TCP */
	net.PrintAllowedSenders(3); /* GSS */
finalize_it:
ENDwillRun



BEGINmodExit
CODESTARTmodExit
	if(pOurTcpsrv != NULL)
		iRet = tcpsrv.Destruct(&pOurTcpsrv);
	TCPSessGSSDeinit();

	/* release objects we used */
	objRelease(tcps_sess, LM_TCPSRV_FILENAME);
	objRelease(tcpsrv, LM_TCPSRV_FILENAME);
	objRelease(gssutil, LM_GSSUTIL_FILENAME);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(netstrm, LM_NETSTRM_FILENAME);
	objRelease(net, LM_NET_FILENAME);
	objRelease(prop, CORE_COMPONENT);
ENDmodExit


BEGINafterRun
CODESTARTafterRun
	/* do cleanup here */
	net.clearAllowedSenders((uchar*)"TCP");
	net.clearAllowedSenders((uchar*)"GSS");
ENDafterRun


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURENonCancelInputTermination)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	if (gss_listen_service_name != NULL) {
		free(gss_listen_service_name);
		gss_listen_service_name = NULL;
	}
	if (pszLstnPortFileName != NULL) {
		free(pszLstnPortFileName);
		pszLstnPortFileName = NULL;
	}
	bPermitPlainTcp = 0;
	iTCPSessMax = 200;
	bKeepAlive = 0;
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current definition */
CODEmodInit_QueryRegCFSLineHdlr
	pOurTcpsrv = NULL;
	/* request objects we use */
	CHKiRet(objUse(tcps_sess, LM_TCPSRV_FILENAME));
	CHKiRet(objUse(tcpsrv, LM_TCPSRV_FILENAME));
	CHKiRet(objUse(gssutil, LM_GSSUTIL_FILENAME));
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(netstrm, LM_NETSTRM_FILENAME));
	CHKiRet(objUse(net, LM_NET_FILENAME));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverpermitplaintcp", 0, eCmdHdlrBinary,
				   NULL, &bPermitPlainTcp, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverrun", 0, eCmdHdlrGetWord,
				   addGSSListener, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverservicename", 0, eCmdHdlrGetWord,
				   NULL, &gss_listen_service_name, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgsslistenportfilename", 0, eCmdHdlrGetWord,
				   NULL, &pszLstnPortFileName, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssservermaxsessions", 0, eCmdHdlrInt,
				   NULL, &iTCPSessMax, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverkeepalive", 0, eCmdHdlrBinary,
				   NULL, &bKeepAlive, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vim:set ai:
 */
