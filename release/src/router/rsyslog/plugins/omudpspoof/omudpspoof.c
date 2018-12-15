/* omudpspoof.c
 *
 * This is a udp-based output module that support spoofing.
 *
 * This file builds on UDP spoofing code contributed by
 * David Lang <david@lang.hm>. I then created a "real" rsyslog module
 * out of that code and omfwd. I decided to make it a separate module because
 * omfwd already mixes up too many things (TCP & UDP & a different modes,
 * this has historic reasons), it would not be a good idea to also add
 * spoofing to it. And, looking at the requirements, there is little in
 * common between omfwd and this module.
 *
 * Note: I have briefly checked libnet source code and I somewhat have the feeling
 * that under some circumstances we may get into trouble with the lib. For
 * example, it registers an atexit() handler, which should not play nicely
 * with our dynamically loaded modules. Anyhow, I refrain from looking deeper
 * at libnet code, especially as testing does not show any real issues. If some
 * occur, it may be easier to modify libnet for dynamic load environments than
 * using a work-around (as a side not, libnet looks somewhat unmaintained, the CVS
 * I can see on sourceforge dates has no updates done less than 7 years ago).
 * On the other hand, it looks like libnet is thread safe (at least is appropriately
 * compiled, which I hope the standard packages are). So I do not guard calls to
 * it with my own mutex calls.
 * rgerhards, 2009-07-10
 *
 * Copyright 2009 David Lang (spoofing code)
 * Copyright 2009-2016 Rainer Gerhards and Adiscon GmbH.
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
#include <fnmatch.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "net.h"
#include "template.h"
#include "msg.h"
#include "cfsysline.h"
#include "module-template.h"
#include "glbl.h"
#include "errmsg.h"
#include "dirty.h"
#include "unicode-helper.h"
#include "debug.h"


#include <libnet.h>
#define _BSD_SOURCE 1
#define __BSD_SOURCE 1
#define __FAVOR_BSD 1


MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omudpspoof")

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(net)

typedef struct _instanceData {
	uchar 	*tplName;	/* name of assigned template */
	uchar	*host;
	uchar	*port;
	uchar	*sourceTpl;
	int	mtu;
	u_short sourcePortStart;	/* for sorce port iteration */
	u_short sourcePortEnd;
	int	bReportLibnetInitErr; /* help prevent multiple error messages on init err */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
	libnet_t *libnet_handle;
	u_short sourcePort;
	int	*pSockArray;		/* sockets to use for UDP */
	struct addrinfo *f_addr;
	char errbuf[LIBNET_ERRBUF_SIZE];
} wrkrInstanceData_t;

#define DFLT_SOURCE_PORT_START 32000
#define DFLT_SOURCE_PORT_END   42000

typedef struct configSettings_s {
	uchar *tplName; /* name of the default template to use */
	uchar *pszSourceNameTemplate; /* name of the template containing the spoofing address */
	uchar *pszTargetHost;
	uchar *pszTargetPort;
	int iSourcePortStart;
	int iSourcePortEnd;
} configSettings_t;
static configSettings_t cs;

/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "target", eCmdHdlrGetWord, 1 },
	{ "port", eCmdHdlrGetWord, 0 },
	{ "sourcetemplate", eCmdHdlrGetWord, 0 },
	{ "sourceport.start", eCmdHdlrInt, 0 },
	{ "sourceport.end", eCmdHdlrInt, 0 },
	{ "mtu", eCmdHdlrInt, 0 },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "template", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
	uchar 	*tplName;	/* default template */
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */



BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	cs.tplName = NULL;
	cs.pszSourceNameTemplate = NULL;
	cs.pszTargetHost = NULL;
	cs.pszTargetPort = NULL;
	cs.iSourcePortStart = DFLT_SOURCE_PORT_START;
	cs.iSourcePortEnd = DFLT_SOURCE_PORT_END;
ENDinitConfVars


/* add some variables needed for libnet */
pthread_mutex_t mutLibnet;

/* forward definitions */
static rsRetVal doTryResume(wrkrInstanceData_t *pWrkrData);


/* this function gets the default template. It coordinates action between
 * old-style and new-style configuration parts.
 */
static uchar*
getDfltTpl(void)
{
	if(loadModConf != NULL && loadModConf->tplName != NULL)
		return loadModConf->tplName;
	else if(cs.tplName == NULL)
		return (uchar*)"RSYSLOG_TraditionalForwardFormat";
	else
		return cs.tplName;
}


/* set the default template to be used
 * This is a module-global parameter, and as such needs special handling. It needs to
 * be coordinated with values set via the v2 config system (rsyslog v6+). What we do
 * is we do not permit this directive after the v2 config system has been used to set
 * the parameter.
 */
static rsRetVal
setLegacyDfltTpl(void __attribute__((unused)) *pVal, uchar* newVal)
{
	DEFiRet;

	if(loadModConf != NULL && loadModConf->tplName != NULL) {
		free(newVal);
		LogError(0, RS_RET_ERR, "omudpspoof default template already set via module "
			"global parameter - can no longer be changed");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	free(cs.tplName);
	cs.tplName = newVal;
finalize_it:
	RETiRet;
}

/* Close the UDP sockets.
 * rgerhards, 2009-05-29
 */
static rsRetVal
closeUDPSockets(wrkrInstanceData_t *pWrkrData)
{
	DEFiRet;
	if(pWrkrData->pSockArray != NULL) {
		net.closeUDPListenSockets(pWrkrData->pSockArray);
		pWrkrData->pSockArray = NULL;
		freeaddrinfo(pWrkrData->f_addr);
		pWrkrData->f_addr = NULL;
	}
	RETiRet;
}


/* get the syslog forward port
 * We may change the implementation to try to lookup the port
 * if it is unspecified. So far, we use the IANA default auf 514.
 * rgerhards, 2007-06-28
 */
static inline uchar *getFwdPt(instanceData *pData)
{
	return (pData->port == NULL) ? UCHAR_CONSTANT("514") : pData->port;
}


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	pModConf->tplName = NULL;
ENDbeginCnfLoad

BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "error processing module "
				"config parameters [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("module (global) param blk for omudpspoof:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "template")) {
			loadModConf->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			if(cs.tplName != NULL) {
				LogError(0, RS_RET_DUP_PARAM, "omudpspoof: warning: default template "
						"was already set via legacy directive - may lead to inconsistent "
						"results.");
			}
		} else {
			dbgprintf("omudpspoof: program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}
finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf

BEGINendCnfLoad
CODESTARTendCnfLoad
	loadModConf = NULL; /* done loading */
	/* free legacy config vars */
	free(cs.tplName);
	cs.tplName = NULL;
ENDendCnfLoad

BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf

BEGINactivateCnf
CODESTARTactivateCnf
	runModConf = pModConf;
ENDactivateCnf

BEGINfreeCnf
CODESTARTfreeCnf
	free(pModConf->tplName);
ENDfreeCnf


BEGINcreateInstance
CODESTARTcreateInstance
	pData->mtu = 1500;
	pData->bReportLibnetInitErr = 1;
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	pWrkrData->libnet_handle = NULL;
	pWrkrData->sourcePort = pData->sourcePortStart;
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	/* final cleanup */
	free(pData->tplName);
	free(pData->port);
	free(pData->host);
	free(pData->sourceTpl);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	closeUDPSockets(pWrkrData);
	if(pWrkrData->libnet_handle != NULL)
		libnet_destroy(pWrkrData->libnet_handle);
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	DBGPRINTF("%s", pData->host);
ENDdbgPrintInstInfo


/* Send a message via UDP
 * Note: libnet is not thread-safe, so we need to ensure that only one
 * instance ever is calling libnet code.
 * rgehards, 2007-12-20
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
static rsRetVal
UDPSend(wrkrInstanceData_t *pWrkrData, uchar *pszSourcename, char *msg, size_t len)
{
	struct addrinfo *r;
	int lsent = 0;
	int bSendSuccess;
	instanceData *pData;
	struct sockaddr_in *tempaddr,source_ip;
	libnet_ptag_t ip, ipo;
	libnet_ptag_t udp;
	sbool bNeedUnlock = 0;
	/* hdrOffs = fragmentation flags + offset (in bytes)
	* divided by 8 */
	unsigned msgOffs, hdrOffs;
	unsigned maxPktLen, pktLen;
	DEFiRet;

	if(pWrkrData->pSockArray == NULL) {
		CHKiRet(doTryResume(pWrkrData));
	}
	pData = pWrkrData->pData;

	if(len > 65528) {
		DBGPRINTF("omudpspoof: msg with length %d truncated to 64k: '%.768s'\n",
			  (int) len, msg);
		len = 65528;
	}

	ip = ipo = udp = 0;
	if(pWrkrData->sourcePort++ >= pData->sourcePortEnd){
		pWrkrData->sourcePort = pData->sourcePortStart;
	}

	inet_pton(AF_INET, (char*)pszSourcename, &(source_ip.sin_addr));

	bSendSuccess = RSFALSE;
	d_pthread_mutex_lock(&mutLibnet);
	bNeedUnlock = 1;
	for (r = pWrkrData->f_addr; r && bSendSuccess == RSFALSE ; r = r->ai_next) {
		tempaddr = (struct sockaddr_in *)r->ai_addr;
		/* Getting max payload size (must be multiple of 8) */
		maxPktLen = (pData->mtu - LIBNET_IPV4_H) & ~0x07;
		msgOffs = 0;
		/* We're doing (payload size - UDP header size) and not
		* checking if it's a multiple of 8 because we know the
		* header is 8 bytes long */
		if(len > (maxPktLen - LIBNET_UDP_H) ) {
			hdrOffs = IP_MF;
			pktLen = maxPktLen - LIBNET_UDP_H;
		} else {
			hdrOffs = 0;
			pktLen = len;
		}
		DBGPRINTF("omudpspoof: stage 1: MF:%d, hdrOffs %d, pktLen %d\n",
			  (hdrOffs & IP_MF) >> 13, (hdrOffs & 0x1FFF) << 3, pktLen);
		libnet_clear_packet(pWrkrData->libnet_handle);
		/* note: libnet does need ports in host order NOT in network byte order! -- rgerhards, 2009-11-12 */
		udp = libnet_build_udp(
			pWrkrData->sourcePort,	/* source port */
			ntohs(tempaddr->sin_port),/* destination port */
			pktLen+LIBNET_UDP_H,	/* packet length */
			0,			/* checksum */
			(u_char*)msg,		/* payload */
			pktLen,	                /* payload size */
			pWrkrData->libnet_handle,	/* libnet handle */
			udp);			/* libnet id */
		if (udp == -1) {
			DBGPRINTF("omudpspoof: can't build UDP header: %s\n",
				libnet_geterror(pWrkrData->libnet_handle));
		}

		ip = libnet_build_ipv4(
			LIBNET_IPV4_H+LIBNET_UDP_H+pktLen, /* length */
			0,				/* TOS */
			242,				/* IP ID */
			hdrOffs,			/* IP Frag */
			64,				/* TTL */
			IPPROTO_UDP,			/* protocol */
			0,				/* checksum */
			source_ip.sin_addr.s_addr,
			tempaddr->sin_addr.s_addr,
			NULL,				/* payload */
			0,				/* payload size */
			pWrkrData->libnet_handle,		/* libnet handle */
			ip);				/* libnet id */
		if (ip == -1) {
			DBGPRINTF("omudpspoof: can't build IP header: %s\n",
				libnet_geterror(pWrkrData->libnet_handle));
		}

		/* Write it to the wire. */
		lsent = libnet_write(pWrkrData->libnet_handle);
		if(lsent != (int) (LIBNET_IPV4_H+LIBNET_UDP_H+pktLen)) {
			/* note: access to fd is a libnet internal. If a newer version of libnet does
			 * not expose that member, we should simply remove it. However, while it is there
			 * it is useful for consolidating with strace output.
			 */
			DBGPRINTF("omudpspoof: write error (total len %d): pktLen %d, sent %d, fd %d: %s\n",
				  (int) len, LIBNET_IPV4_H+LIBNET_UDP_H+pktLen, lsent, pWrkrData->libnet_handle->fd,
				  libnet_geterror(pWrkrData->libnet_handle));
			if(lsent != -1) {
				bSendSuccess = RSTRUE;
			}
		} else {
			bSendSuccess = RSTRUE;
		}
		msgOffs += pktLen;

		/* We need to get rid of the UDP header to build the other fragments */
		libnet_clear_packet(pWrkrData->libnet_handle);
		ip = LIBNET_PTAG_INITIALIZER;
		while(len > msgOffs ) { /* loop until all payload is sent */
			/* check if there will be more fragments */
			if((len - msgOffs) > maxPktLen) {
				/* In IP's eyes, the UDP header in the first packet
				* needs to be in the offset, so we add its size to
				* the payload offset here */
				hdrOffs = IP_MF + (msgOffs + LIBNET_UDP_H)/8;
				pktLen = maxPktLen;
			} else {
				/* See above */
				hdrOffs = (msgOffs + LIBNET_UDP_H)/8;
				pktLen = len - msgOffs;
			}
			DBGPRINTF("omudpspoof: stage 2: MF:%d, hdrOffs %d, pktLen %d\n",
				  (hdrOffs & IP_MF) >> 13, (hdrOffs & 0x1FFF) << 3, pktLen);
			ip = libnet_build_ipv4(
				LIBNET_IPV4_H + pktLen,         /* length */
				0,				/* TOS */
				242,				/* IP ID */
				hdrOffs,			/* IP Frag */
				64,				/* TTL */
				IPPROTO_UDP,			/* protocol */
				0,				/* checksum */
				source_ip.sin_addr.s_addr,
				tempaddr->sin_addr.s_addr,
				(uint8_t*)(msg+msgOffs),	/* payload */
				pktLen, 			/* payload size */
				pWrkrData->libnet_handle,		/* libnet handle */
				ip);				/* libnet id */
			if (ip == -1) {
				DBGPRINTF("omudpspoof: can't build IP fragment header: %s\n",
					libnet_geterror(pWrkrData->libnet_handle));
			}
			/* Write it to the wire. */
			lsent = libnet_write(pWrkrData->libnet_handle);
			if(lsent != (int) (LIBNET_IPV4_H+pktLen)) {
				DBGPRINTF("omudpspoof: fragment write error len %d, sent %d: %s\n",
					  (int) (LIBNET_IPV4_H+LIBNET_UDP_H+len), lsent,
						libnet_geterror(pWrkrData->libnet_handle));
				bSendSuccess = RSFALSE;
				continue;
			}
			msgOffs += pktLen;
		}
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pWrkrData->libnet_handle != NULL) {
			libnet_destroy(pWrkrData->libnet_handle);
			pWrkrData->libnet_handle = NULL;
		}
	}
	if(bNeedUnlock) {
		d_pthread_mutex_unlock(&mutLibnet);
	}
	RETiRet;
}
#pragma GCC diagnostic pop


/* try to resume connection if it is not ready
 * rgerhards, 2007-08-02
 */
static rsRetVal doTryResume(wrkrInstanceData_t *pWrkrData)
{
	int iErr;
	struct addrinfo *res;
	struct addrinfo hints;
	instanceData *pData;
	DEFiRet;

	if(pWrkrData->pSockArray != NULL)
		FINALIZE;
	pData = pWrkrData->pData;

	if(pWrkrData->libnet_handle == NULL) {
		/* Initialize the libnet library.  Root priviledges are required.
		 * this initializes a IPv4 socket to use for forging UDP packets.
		 */
		pWrkrData->libnet_handle = libnet_init(
		    LIBNET_RAW4,                            /* injection type */
		    NULL,                                   /* network interface */
		    pWrkrData->errbuf);                     /* errbuf */

		if(pWrkrData->libnet_handle == NULL) {
			if(pData->bReportLibnetInitErr) {
				LogError(0, RS_RET_ERR_LIBNET_INIT, "omudpsoof: error "
				                "initializing libnet - are you running as root?");
				pData->bReportLibnetInitErr = 0;
			}
			ABORT_FINALIZE(RS_RET_ERR_LIBNET_INIT);
		}
	}
	DBGPRINTF("omudpspoof: libnit_init() ok\n");
	pData->bReportLibnetInitErr = 1;

	/* The remote address is not yet known and needs to be obtained */
	DBGPRINTF("omudpspoof trying resume for '%s'\n", pData->host);
	memset(&hints, 0, sizeof(hints));
	/* port must be numeric, because config file syntax requires this */
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_family = glbl.GetDefPFFamily();
	hints.ai_socktype = SOCK_DGRAM;
	if((iErr = (getaddrinfo((char*)pData->host, (char*)getFwdPt(pData), &hints, &res))) != 0) {
		DBGPRINTF("could not get addrinfo for hostname '%s':'%s': %d%s\n",
			  pData->host, getFwdPt(pData), iErr, gai_strerror(iErr));
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}
	DBGPRINTF("%s found, resuming.\n", pData->host);
	pWrkrData->f_addr = res;
	pWrkrData->pSockArray = net.create_udp_socket((uchar*)pData->host, NULL, 0, 0, 0, 0, NULL);

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pWrkrData->f_addr != NULL) {
			freeaddrinfo(pWrkrData->f_addr);
			pWrkrData->f_addr = NULL;
		}
		iRet = RS_RET_SUSPENDED;
	}

	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	iRet = doTryResume(pWrkrData);
ENDtryResume

BEGINdoAction
	char *psz; /* temporary buffering */
	unsigned l;
	int iMaxLine;
CODESTARTdoAction
	CHKiRet(doTryResume(pWrkrData));

	DBGPRINTF(" %s:%s/omudpspoof, src '%s', msg strt '%.256s'\n", pWrkrData->pData->host,
		  getFwdPt(pWrkrData->pData), ppString[1], ppString[0]);

	iMaxLine = glbl.GetMaxLine();
	psz = (char*) ppString[0];
	l = strlen((char*) psz);
	if((int) l > iMaxLine)
		l = iMaxLine;

	CHKiRet(UDPSend(pWrkrData, ppString[1], psz, l));

finalize_it:
ENDdoAction


static void
setInstParamDefaults(instanceData *pData)
{
	pData->tplName = NULL;
	pData->sourcePortStart = DFLT_SOURCE_PORT_START;
	pData->sourcePortEnd = DFLT_SOURCE_PORT_END;
	pData->host = NULL;
	pData->port = NULL;
	pData->sourceTpl = (uchar*) strdup("RSYSLOG_omudpspoofDfltSourceTpl");
	pData->mtu = 1500;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	uchar *tplToUse;
	int i;
CODESTARTnewActInst
	DBGPRINTF("newActInst (omudpspoof)\n");

	pvals = nvlstGetParams(lst, &actpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "omudpspoof: mandatory "
		                "parameters missing");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("action param blk in omudpspoof:\n");
		cnfparamsPrint(&actpblk, pvals);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "target")) {
			pData->host = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "port")) {
			pData->port = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "sourcetemplate")) {
			free(pData->sourceTpl);
			pData->sourceTpl = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "sourceport.start")) {
			pData->sourcePortStart = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "sourceport.end")) {
			pData->sourcePortEnd = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "mtu")) {
			pData->mtu = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			DBGPRINTF("omudpspoof: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}
	CODE_STD_STRING_REQUESTnewActInst(2)

	tplToUse = ustrdup((pData->tplName == NULL) ? getDfltTpl() : pData->tplName);
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, tplToUse, OMSR_NO_RQD_TPL_OPTS));
	CHKiRet(OMSRsetEntry(*ppOMSR, 1, ustrdup(pData->sourceTpl), OMSR_NO_RQD_TPL_OPTS));

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINparseSelectorAct
	uchar *sourceTpl;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(2)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":omudpspoof:", sizeof(":omudpspoof:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":omudpspoof:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	sourceTpl = (cs.pszSourceNameTemplate == NULL) ? UCHAR_CONSTANT("RSYSLOG_omudpspoofDfltSourceTpl")
						    : cs.pszSourceNameTemplate;

	if(cs.pszTargetHost == NULL) {
		LogError(0, NO_ERRCODE, "No $ActionOMUDPSpoofTargetHost given, can not continue "
			"with this action.");
		ABORT_FINALIZE(RS_RET_HOST_NOT_SPECIFIED);
	}

	/* fill instance properties */
	CHKmalloc(pData->host = ustrdup(cs.pszTargetHost));
	if(cs.pszTargetPort == NULL)
		pData->port = NULL;
	else
		CHKmalloc(pData->port = ustrdup(cs.pszTargetPort));
	CHKiRet(OMSRsetEntry(*ppOMSR, 1, ustrdup(sourceTpl), OMSR_NO_RQD_TPL_OPTS));
	pData->sourcePortStart = cs.iSourcePortStart;
	pData->sourcePortEnd = cs.iSourcePortEnd;

	/* process template */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS,
		(cs.tplName == NULL) ? (uchar*)"RSYSLOG_TraditionalForwardFormat" : cs.tplName));

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


/* a common function to free our configuration variables - used both on exit
 * and on $ResetConfig processing. -- rgerhards, 2008-05-16
 */
static void
freeConfigVars(void)
{
	free(cs.tplName);
	cs.tplName = NULL;
	free(cs.pszTargetHost);
	cs.pszTargetHost = NULL;
	free(cs.pszTargetPort);
	cs.pszTargetPort = NULL;
}


BEGINmodExit
CODESTARTmodExit
	/* destroy the libnet state needed for forged UDP sources */
	pthread_mutex_destroy(&mutLibnet);
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(net, LM_NET_FILENAME);
	freeConfigVars();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
ENDqueryEtryPt


/* Reset config variables for this module to default values.
 * rgerhards, 2008-03-28
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	freeConfigVars();
	/* we now must reset all non-string values */
	cs.iSourcePortStart = DFLT_SOURCE_PORT_START;
	cs.iSourcePortEnd = DFLT_SOURCE_PORT_END;
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(net,LM_NET_FILENAME));

	pthread_mutex_init(&mutLibnet, NULL);

	CHKiRet(regCfSysLineHdlr((uchar *)"actionomudpspoofdefaulttemplate", 0, eCmdHdlrGetWord,
	setLegacyDfltTpl, NULL, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionomudpspoofsourcenametemplate", 0, eCmdHdlrGetWord, NULL,
	&cs.pszSourceNameTemplate, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionomudpspooftargethost", 0, eCmdHdlrGetWord, NULL,
	&cs.pszTargetHost, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionomudpspooftargetport", 0, eCmdHdlrGetWord, NULL,
	&cs.pszTargetPort, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionomudpspoofsourceportstart", 0, eCmdHdlrInt, NULL,
	&cs.iSourcePortStart, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionomudpspoofsourceportend", 0, eCmdHdlrInt, NULL,
	&cs.iSourcePortEnd, NULL));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler, resetConfigVariables,
	NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vim:set ai:
 */
