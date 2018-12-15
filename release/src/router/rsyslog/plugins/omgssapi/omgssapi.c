/* omgssapi.c
 * This is the implementation of the build-in forwarding output module.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
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
#ifdef USE_GSSAPI
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fnmatch.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <zlib.h>
#include <pthread.h>
#include <gssapi/gssapi.h>
#include "dirty.h"
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "net.h"
#include "template.h"
#include "msg.h"
#include "cfsysline.h"
#include "module-template.h"
#include "gss-misc.h"
#include "tcpclt.h"
#include "glbl.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP


static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(gssutil)
DEFobjCurrIf(tcpclt)

typedef struct _instanceData {
	char	*f_hname;
	short	sock;			/* file descriptor */
	enum { /* TODO: we shoud revisit these definitions */
		eDestFORW,
		eDestFORW_SUSP,
		eDestFORW_UNKN
	} eDestState;
	struct addrinfo *f_addr;
	int compressionLevel; /* 0 - no compression, else level for zlib */
	char *port;
	tcpclt_t *pTCPClt;		/* our tcpclt object */
	gss_ctx_id_t gss_context;
	OM_uint32 gss_flags;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

/* config data */

typedef enum gss_mode_e {
	GSSMODE_MIC,
	GSSMODE_ENC
} gss_mode_t;

static struct configSettings_s {
	uchar	*pszTplName; /* name of the default template to use */
	char *gss_base_service_name;
	gss_mode_t gss_mode;
} cs;

static pthread_mutex_t mutDoAct = PTHREAD_MUTEX_INITIALIZER;

/* get the syslog forward port from selector_t. The passed in
 * struct must be one that is setup for forwarding.
 * rgerhards, 2007-06-28
 * We may change the implementation to try to lookup the port
 * if it is unspecified. So far, we use the IANA default auf 514.
 */
static const char *
getFwdSyslogPt(instanceData *pData)
{
	assert(pData != NULL);
	if(pData->port == NULL)
		return("514");
	else
		return(pData->port);
}

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINfreeInstance
OM_uint32 maj_stat, min_stat;
CODESTARTfreeInstance
	switch (pData->eDestState) {
		case eDestFORW:
		case eDestFORW_SUSP:
			freeaddrinfo(pData->f_addr);
			/* fall through */
		case eDestFORW_UNKN:
			if(pData->port != NULL)
				free(pData->port);
			break;
	}

	if (pData->gss_context != GSS_C_NO_CONTEXT) {
		maj_stat = gss_delete_sec_context(&min_stat, &pData->gss_context, GSS_C_NO_BUFFER);
		if (maj_stat != GSS_S_COMPLETE)
			gssutil.display_status((char*)"deleting context", maj_stat, min_stat);
	}
	/* this is meant to be done when module is unloaded,
	   but since this module is static...
	*/
	free(cs.gss_base_service_name);
	cs.gss_base_service_name = NULL;

	/* final cleanup */
	tcpclt.Destruct(&pData->pTCPClt);
	if(pData->sock >= 0)
		close(pData->sock);

	if(pData->f_hname != NULL)
		free(pData->f_hname);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance

BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	printf("%s", pData->f_hname);
ENDdbgPrintInstInfo


/* This function is called immediately before a send retry is attempted.
 * It shall clean up whatever makes sense.
 * rgerhards, 2007-12-28
 */
static rsRetVal TCPSendGSSPrepRetry(void __attribute__((unused)) *pData)
{
	/* in case of TCP/GSS, there is nothing to do */
	return RS_RET_OK;
}


static rsRetVal TCPSendGSSInit(void *pvData)
{
	DEFiRet;
	int s = -1;
	const char *base;
	OM_uint32 maj_stat, min_stat, init_sec_min_stat, *sess_flags, ret_flags;
	gss_buffer_desc out_tok, in_tok;
	gss_buffer_t tok_ptr;
	gss_name_t target_name;
	gss_ctx_id_t *context;
	instanceData *pData = (instanceData *) pvData;

	assert(pData != NULL);

	/* if the socket is already initialized, we are done */
	if(pData->sock > 0)
		ABORT_FINALIZE(RS_RET_OK);

	base = (cs.gss_base_service_name == NULL) ? "host" : cs.gss_base_service_name;
	out_tok.length = strlen(pData->f_hname) + strlen(base) + 2;
	CHKmalloc(out_tok.value = MALLOC(out_tok.length));
	strcpy(out_tok.value, base);
	strcat(out_tok.value, "@");
	strcat(out_tok.value, pData->f_hname);
	dbgprintf("GSS-API service name: %s\n", (char*) out_tok.value);

	tok_ptr = GSS_C_NO_BUFFER;
	context = &pData->gss_context;
	*context = GSS_C_NO_CONTEXT;

	maj_stat = gss_import_name(&min_stat, &out_tok, GSS_C_NT_HOSTBASED_SERVICE, &target_name);
	free(out_tok.value);
	out_tok.value = NULL;
	out_tok.length = 0;

	if (maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status((char*)"parsing name", maj_stat, min_stat);
		goto fail;
	}

	sess_flags = &pData->gss_flags;
	*sess_flags = GSS_C_MUTUAL_FLAG;
	if (cs.gss_mode == GSSMODE_MIC) {
		*sess_flags |= GSS_C_INTEG_FLAG;
	}
	if (cs.gss_mode == GSSMODE_ENC) {
		*sess_flags |= GSS_C_CONF_FLAG;
	}
	dbgprintf("GSS-API requested context flags:\n");
	gssutil.display_ctx_flags(*sess_flags);

	do {
		maj_stat = gss_init_sec_context(&init_sec_min_stat, GSS_C_NO_CREDENTIAL, context,
						target_name, GSS_C_NO_OID, *sess_flags, 0, NULL,
						tok_ptr, NULL, &out_tok, &ret_flags, NULL);
		if (tok_ptr != GSS_C_NO_BUFFER)
			free(in_tok.value);

		if (maj_stat != GSS_S_COMPLETE
		    && maj_stat != GSS_S_CONTINUE_NEEDED) {
			gssutil.display_status((char*)"initializing context", maj_stat, init_sec_min_stat);
			goto fail;
		}

		if (s == -1)
			if ((s = pData->sock = tcpclt.CreateSocket(pData->f_addr)) == -1)
				goto fail;

		if (out_tok.length != 0) {
			dbgprintf("GSS-API Sending init_sec_context token (length: %ld)\n", (long) out_tok.length);
			if (gssutil.send_token(s, &out_tok) < 0) {
				goto fail;
			}
		}
		gss_release_buffer(&min_stat, &out_tok);

		if (maj_stat == GSS_S_CONTINUE_NEEDED) {
			dbgprintf("GSS-API Continue needed...\n");
			if (gssutil.recv_token(s, &in_tok) <= 0) {
				goto fail;
			}
			tok_ptr = &in_tok;
		}
	} while (maj_stat == GSS_S_CONTINUE_NEEDED);

	dbgprintf("GSS-API Provided context flags:\n");
	*sess_flags = ret_flags;
	gssutil.display_ctx_flags(*sess_flags);

	dbgprintf("GSS-API Context initialized\n");
	gss_release_name(&min_stat, &target_name);

finalize_it:
	RETiRet;

fail:
	LogError(0, RS_RET_GSS_SENDINIT_ERROR, "GSS-API Context initialization failed\n");
	gss_release_name(&min_stat, &target_name);
	gss_release_buffer(&min_stat, &out_tok);
	if (*context != GSS_C_NO_CONTEXT) {
		gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
		*context = GSS_C_NO_CONTEXT;
	}
	if (s != -1)
		close(s);
	pData->sock = -1;
	ABORT_FINALIZE(RS_RET_GSS_SENDINIT_ERROR);
}


static rsRetVal TCPSendGSSSend(void *pvData, char *msg, size_t len)
{
	int s;
	gss_ctx_id_t *context;
	OM_uint32 maj_stat, min_stat;
	gss_buffer_desc in_buf, out_buf;
	instanceData *pData = (instanceData *) pvData;

	assert(pData != NULL);
	assert(msg != NULL);
	assert(len > 0);

	s = pData->sock;
	context = &pData->gss_context;
	in_buf.value = msg;
	in_buf.length = len;
	maj_stat = gss_wrap(&min_stat, *context, (cs.gss_mode == GSSMODE_ENC) ? 1 : 0, GSS_C_QOP_DEFAULT,
			    &in_buf, NULL, &out_buf);
	if (maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status((char*)"wrapping message", maj_stat, min_stat);
		goto fail;
	}
	
	if (gssutil.send_token(s, &out_buf) < 0) {
		goto fail;
	}
	gss_release_buffer(&min_stat, &out_buf);

	return RS_RET_OK;

fail:
	close(s);
	pData->sock = -1;
	gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
	*context = GSS_C_NO_CONTEXT;
	gss_release_buffer(&min_stat, &out_buf);
	dbgprintf("message not (GSS/tcp)send");
	return RS_RET_GSS_SEND_ERROR;
}


/* try to resume connection if it is not ready
 * rgerhards, 2007-08-02
 */
static rsRetVal doTryResume(instanceData *pData)
{
	DEFiRet;
	struct addrinfo *res;
	struct addrinfo hints;
	unsigned e;

	switch (pData->eDestState) {
	case eDestFORW_SUSP:
		iRet = RS_RET_OK; /* the actual check happens during doAction() only */
		pData->eDestState = eDestFORW;
		break;

	case eDestFORW_UNKN:
		/* The remote address is not yet known and needs to be obtained */
		dbgprintf(" %s\n", pData->f_hname);
		memset(&hints, 0, sizeof(hints));
		/* port must be numeric, because config file syntax requests this */
		/* TODO: this code is a duplicate from cfline() - we should later create
		 * a common function.
		 */
		hints.ai_flags = AI_NUMERICSERV;
		hints.ai_family = glbl.GetDefPFFamily();
		hints.ai_socktype = SOCK_STREAM;
		if((e = getaddrinfo(pData->f_hname,
				    getFwdSyslogPt(pData), &hints, &res)) == 0) {
			dbgprintf("%s found, resuming.\n", pData->f_hname);
			pData->f_addr = res;
			pData->eDestState = eDestFORW;
		} else {
			iRet = RS_RET_SUSPENDED;
		}
		break;
	case eDestFORW:
		/* NOOP */
		break;
	}

	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	pthread_mutex_lock(&mutDoAct);
	iRet = doTryResume(pWrkrData->pData);
	pthread_mutex_unlock(&mutDoAct);
ENDtryResume

BEGINdoAction
	char *psz = NULL; /* temporary buffering */
	register unsigned l;
	int iMaxLine;
	instanceData *pData;
CODESTARTdoAction
	pthread_mutex_lock(&mutDoAct);
	pData = pWrkrData->pData;
	switch (pData->eDestState) {
	case eDestFORW_SUSP:
		dbgprintf("internal error in omgssapi.c, eDestFORW_SUSP in doAction()!\n");
		iRet = RS_RET_SUSPENDED;
		break;

	case eDestFORW_UNKN:
		dbgprintf("doAction eDestFORW_UNKN\n");
		iRet = doTryResume(pData);
		break;

	case eDestFORW:
		dbgprintf(" %s:%s/%s\n", pData->f_hname, getFwdSyslogPt(pData), "tcp-gssapi");
		iMaxLine = glbl.GetMaxLine();
		psz = (char*) ppString[0];
		l = strlen((char*) psz);
		if((int) l > iMaxLine)
			l = iMaxLine;

		/* Check if we should compress and, if so, do it. We also
		 * check if the message is large enough to justify compression.
		 * The smaller the message, the less likely is a gain in compression.
		 * To save CPU cycles, we do not try to compress very small messages.
		 * What "very small" means needs to be configured. Currently, it is
		 * hard-coded but this may be changed to a config parameter.
		 * rgerhards, 2006-11-30
		 */
		if(pData->compressionLevel && (l > CONF_MIN_SIZE_FOR_COMPRESS)) {
			Bytef *out;
			uLongf destLen = iMaxLine + iMaxLine/100 +12; /* recommended value from zlib doc */
			uLong srcLen = l;
			int ret;
			/* TODO: optimize malloc sequence? -- rgerhards, 2008-09-02 */
			CHKmalloc(out = (Bytef*) MALLOC(iMaxLine + iMaxLine/100 + 12));
			out[0] = 'z';
			out[1] = '\0';
			ret = compress2((Bytef*) out+1, &destLen, (Bytef*) psz,
					srcLen, pData->compressionLevel);
			dbgprintf("Compressing message, length was %d now %d, return state  %d.\n",
				l, (int) destLen, ret);
			if(ret != Z_OK) {
				/* if we fail, we complain, but only in debug mode
				 * Otherwise, we are silent. In any case, we ignore the
				 * failed compression and just sent the uncompressed
				 * data, which is still valid. So this is probably the
				 * best course of action.
				 * rgerhards, 2006-11-30
				 */
				dbgprintf("Compression failed, sending uncompressed message\n");
				free(out);
			} else if(destLen+1 < l) {
				/* only use compression if there is a gain in using it! */
				dbgprintf("there is gain in compression, so we do it\n");
				psz = (char*) out;
				l = destLen + 1; /* take care for the "z" at message start! */
			} else {
				free(out);
			}
			++destLen;
		}

		CHKiRet_Hdlr(tcpclt.Send(pData->pTCPClt, pData, psz, l)) {
			/* error! */
			dbgprintf("error forwarding via tcp, suspending\n");
			pData->eDestState = eDestFORW_SUSP;
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}
		break;
	}
finalize_it:
	if((psz != NULL) && (psz != (char*) ppString[0]))  {
		/* we need to free temporary buffer, alloced above - Naoya Nakazawa, 2010-01-11 */
		free(psz);
	}
	pthread_mutex_unlock(&mutDoAct);
ENDdoAction


BEGINparseSelectorAct
	uchar *q;
	int i;
	int error;
	int bErr;
	struct addrinfo hints, *res;
	TCPFRAMINGMODE tcp_framing = TCP_FRAMING_OCTET_STUFFING;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us
	 * The first test [*p == '>'] can be skipped if a module shall only
	 * support the newer slection syntax [:modname:]. This is in fact
	 * recommended for new modules. Please note that over time this part
	 * will be handled by rsyslogd itself, but for the time being it is
	 * a good compromise to do it at the module level.
	 * rgerhards, 2007-10-15
	 */

	if(!strncmp((char*) p, ":omgssapi:", sizeof(":omgssapi:") - 1)) {
		p += sizeof(":omgssapi:") - 1; /* eat indicator sequence (-1 because of '\0'!) */
	} else {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	if((iRet = createInstance(&pData)) != RS_RET_OK)
		goto finalize_it;

	/* we are now after the protocol indicator. Now check if we should
	 * use compression. We begin to use a new option format for this:
	 * @(option,option)host:port
	 * The first option defined is "z[0..9]" where the digit indicates
	 * the compression level. If it is not given, 9 (best compression) is
	 * assumed. An example action statement might be:
	 * @@(z5,o)127.0.0.1:1400
	 * Which means send via TCP with medium (5) compresion (z) to the local
	 * host on port 1400. The '0' option means that octet-couting (as in
	 * IETF I-D syslog-transport-tls) is to be used for framing (this option
	 * applies to TCP-based syslog only and is ignored when specified with UDP).
	 * That is not yet implemented.
	 * rgerhards, 2006-12-07
	 */
	if(*p == '(') {
		/* at this position, it *must* be an option indicator */
		do {
			++p; /* eat '(' or ',' (depending on when called) */
			/* check options */
			if(*p == 'z') { /* compression */
				++p; /* eat */
				if(isdigit((int) *p)) {
					int iLevel;
					iLevel = *p - '0';
					++p; /* eat */
					pData->compressionLevel = iLevel;
				} else {
					LogError(0, NO_ERRCODE, "Invalid compression level '%c' specified in "
						 "forwardig action - NOT turning on compression.",
						 *p);
				}
			} else if(*p == 'o') { /* octet-couting based TCP framing? */
				++p; /* eat */
				/* no further options settable */
				tcp_framing = TCP_FRAMING_OCTET_COUNTING;
			} else { /* invalid option! Just skip it... */
				LogError(0, NO_ERRCODE, "Invalid option %c in forwarding action - "
					"ignoring.", *p);
				++p; /* eat invalid option */
			}
			/* the option processing is done. We now do a generic skip
			 * to either the next option or the end of the option
			 * block.
			 */
			while(*p && *p != ')' && *p != ',')
				++p;	/* just skip it */
		} while(*p && *p == ','); /* Attention: do.. while() */
		if(*p == ')')
			++p; /* eat terminator, on to next */
		else
			/* we probably have end of string - leave it for the rest
			 * of the code to handle it (but warn the user)
			 */
			LogError(0, NO_ERRCODE, "Option block not terminated in gssapi forward action.");
	}
	/* extract the host first (we do a trick - we replace the ';' or ':' with a '\0')
	 * now skip to port and then template name. rgerhards 2005-07-06
	 */
	for(q = p ; *p && *p != ';' && *p != ':' && *p != '#' ; ++p)
		/* JUST SKIP */;

	pData->port = NULL;
	if(*p == ':') { /* process port */
		uchar * tmp;

		*p = '\0'; /* trick to obtain hostname (later)! */
		tmp = ++p;
		for(i=0 ; *p && isdigit((int) *p) ; ++p, ++i)
			/* SKIP AND COUNT */;
		pData->port = MALLOC(i + 1);
		if(pData->port == NULL) {
			LogError(0, NO_ERRCODE, "Could not get memory to store syslog forwarding port, "
				 "using default port, results may not be what you intend\n");
			/* we leave f_forw.port set to NULL, this is then handled by
			 * getFwdSyslogPt().
			 */
		} else {
			memcpy(pData->port, tmp, i);
			*(pData->port + i) = '\0';
		}
	}
	

	/* now skip to template */
	bErr = 0;
	while(*p && *p != ';') {
		if(*p && *p != ';' && !isspace((int) *p)) {
			if(bErr == 0) { /* only 1 error msg! */
				bErr = 1;
				errno = 0;
				LogError(0, NO_ERRCODE, "invalid selector line (port), probably not doing "
					 "what was intended");
			}
		}
		++p;
	}

	/* TODO: make this if go away! */
	if(*p == ';' || *p == '#' || isspace(*p)) {
		uchar cTmp = *p;
		*p = '\0'; /* trick to obtain hostname (later)! */
		CHKmalloc(pData->f_hname = strdup((char*) q));
		*p = cTmp;
	} else {
		CHKmalloc(pData->f_hname = strdup((char*) q));
	}

	/* process template */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS,
			(cs.pszTplName == NULL) ? (uchar*)"RSYSLOG_TraditionalForwardFormat" : cs.pszTplName));

	/* first set the pData->eDestState */
	memset(&hints, 0, sizeof(hints));
	/* port must be numeric, because config file syntax requests this */
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_family = glbl.GetDefPFFamily();
	hints.ai_socktype = SOCK_STREAM;
	if( (error = getaddrinfo(pData->f_hname, getFwdSyslogPt(pData), &hints, &res)) != 0) {
		pData->eDestState = eDestFORW_UNKN;
	} else {
		pData->eDestState = eDestFORW;
		pData->f_addr = res;
	}

	/* now create our tcpclt */
	CHKiRet(tcpclt.Construct(&pData->pTCPClt));
	/* and set callbacks */
	CHKiRet(tcpclt.SetSendInit(pData->pTCPClt, TCPSendGSSInit));
	CHKiRet(tcpclt.SetSendFrame(pData->pTCPClt, TCPSendGSSSend));
	CHKiRet(tcpclt.SetSendPrepRetry(pData->pTCPClt, TCPSendGSSPrepRetry));
	CHKiRet(tcpclt.SetFraming(pData->pTCPClt, tcp_framing));

	/* TODO: do we need to call freeInstance if we failed - this is a general question for
	 * all output modules. I'll address it lates as the interface evolves. rgerhards, 2007-07-25
	 */
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
	objRelease(glbl, CORE_COMPONENT);
	objRelease(gssutil, LM_GSSUTIL_FILENAME);
	objRelease(tcpclt, LM_TCPCLT_FILENAME);

	if(cs.pszTplName != NULL) {
		free(cs.pszTplName);
		cs.pszTplName = NULL;
	}
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt


/* set a new GSSMODE based on config directive */
static rsRetVal setGSSMode(void __attribute__((unused)) *pVal, uchar *mode)
{
	DEFiRet;

	if (!strcmp((char *) mode, "integrity")) {
		cs.gss_mode = GSSMODE_MIC;
		dbgprintf("GSS-API gssmode set to GSSMODE_MIC\n");
	} else if (!strcmp((char *) mode, "encryption")) {
		cs.gss_mode = GSSMODE_ENC;
		dbgprintf("GSS-API gssmode set to GSSMODE_ENC\n");
	} else {
		LogError(0, RS_RET_INVALID_PARAMS, "unknown gssmode parameter: %s", (char *) mode);
		iRet = RS_RET_INVALID_PARAMS;
	}
	free(mode);

	RETiRet;
}


static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	cs.gss_mode = GSSMODE_ENC;
	free(cs.gss_base_service_name);
	cs.gss_base_service_name = NULL;
	free(cs.pszTplName);
	cs.pszTplName = NULL;
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(gssutil, LM_GSSUTIL_FILENAME));
	CHKiRet(objUse(tcpclt, LM_TCPCLT_FILENAME));

	CHKiRet(omsdRegCFSLineHdlr((uchar *)"gssforwardservicename", 0, eCmdHdlrGetWord, NULL,
	&cs.gss_base_service_name, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"gssmode", 0, eCmdHdlrGetWord, setGSSMode, &cs.gss_mode,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actiongssforwarddefaulttemplate", 0, eCmdHdlrGetWord, NULL,
	&cs.pszTplName, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler, resetConfigVariables,
	NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

#endif /* #ifdef USE_GSSAPI */
/* vi:set ai:
 */
