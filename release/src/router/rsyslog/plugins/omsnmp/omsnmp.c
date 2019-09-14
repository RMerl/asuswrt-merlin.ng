/* omsnmp.c
 *
 * This module sends an snmp trap.
 *
 * Copyright 2007-2018 Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <assert.h>
#include "conf.h"
#include "syslogd-types.h"
#include "cfsysline.h"
#include "module-template.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "omsnmp.h"
#include "errmsg.h"
#include "parserif.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omsnmp")

/* internal structures
 */
DEF_OMOD_STATIC_DATA

/* Default static snmp OID's */
/*unused
static oid             objid_enterprise[] = { 1, 3, 6, 1, 4, 1, 3, 1, 1 };
static oid             objid_sysdescr[] = { 1, 3, 6, 1, 2, 1, 1, 1, 0 };
*/
static oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
static oid             objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };


typedef struct _instanceData {
	uchar	*szTransport;	/* Transport - Can be udp, tcp, udp6, tcp6 and other types supported by NET-SNMP */
	uchar	*szTarget;	/* IP/hostname of Snmp Target*/
	uchar	*szCommunity;	/* Snmp Community */
	uchar	*szEnterpriseOID;/* Snmp Enterprise OID - default is (1.3.6.1.4.1.3.1.1 = enterprises.cmu.1.1) */
	uchar	*szSnmpTrapOID;	/* Snmp Trap OID - default is (1.3.6.1.4.1.19406.1.2.1 =
ADISCON-MONITORWARE-MIB::syslogtrap) */
	uchar	*szSyslogMessageOID;	/* Snmp OID used for the Syslog Message:
	        * default is 1.3.6.1.4.1.19406.1.1.2.1 - ADISCON-MONITORWARE-MIB::syslogMsg
		* You will need the ADISCON-MONITORWARE-MIB and ADISCON-MIB mibs installed on the receiver
		* side in order to decode this mib.
		* Downloads of these mib files can be found here:
		*	http://www.adiscon.org/download/ADISCON-MONITORWARE-MIB.txt
		*	http://www.adiscon.org/download/ADISCON-MIB.txt
		*/
	int iPort;			/* Target Port */
	int iSNMPVersion;		/* SNMP Version to use */
	int iTrapType;			/* Snmp TrapType or GenericType */
	int iSpecificType;		/* Snmp Specific Type */

	uchar	*tplName;       	/* format template to use */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
	netsnmp_session *snmpsession;	/* Holds to SNMP Session, NULL if not initialized */
} wrkrInstanceData_t;

typedef struct configSettings_s {
	uchar* pszTransport; /* default transport */
	uchar* pszTarget;
	/* note using an unsigned for a port number is not a good idea from an IPv6 point of view */
	int iPort;
	int iSNMPVersion;	/* 0 Means SNMPv1, 1 Means SNMPv2c */
	uchar* pszCommunity;
	uchar* pszEnterpriseOID;
	uchar* pszSnmpTrapOID;
	uchar* pszSyslogMessageOID;
	int iSpecificType;
	int iTrapType;		/*Default is SNMP_TRAP_ENTERPRISESPECIFIC */
	/*
				Possible Values
		SNMP_TRAP_COLDSTART		(0)
		SNMP_TRAP_WARMSTART		(1)
		SNMP_TRAP_LINKDOWN		(2)
		SNMP_TRAP_LINKUP		(3)
		SNMP_TRAP_AUTHFAIL		(4)
		SNMP_TRAP_EGPNEIGHBORLOSS	(5)
		SNMP_TRAP_ENTERPRISESPECIFIC	(6)
	*/
} configSettings_t;
static configSettings_t cs;

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "server", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "port", eCmdHdlrInt, 0 },
	{ "transport", eCmdHdlrString, 0 },
	{ "version", eCmdHdlrInt, 0 },
	{ "community", eCmdHdlrString, 0 },
	{ "enterpriseoid", eCmdHdlrString, 0 },
	{ "trapoid", eCmdHdlrString, 0 },
	{ "messageoid", eCmdHdlrString, 0 },
	{ "traptype", eCmdHdlrInt, 0 },
	{ "specifictype", eCmdHdlrInt, 0 },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	cs.pszTransport = NULL;
	cs.pszTarget = NULL;
	cs.iPort = 0;
	cs.iSNMPVersion = 1;
	cs.pszCommunity = NULL;
	cs.pszEnterpriseOID = NULL;
	cs.pszSnmpTrapOID = NULL;
	cs.pszSyslogMessageOID = NULL;
	cs.iSpecificType = 0;
	cs.iTrapType = SNMP_TRAP_ENTERPRISESPECIFIC;
ENDinitConfVars

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	pWrkrData->snmpsession = NULL;
ENDcreateWrkrInstance

BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("SNMPTransport: %s\n", pData->szTransport);
	dbgprintf("SNMPTarget: %s\n", pData->szTarget);
	dbgprintf("SNMPPort: %d\n", pData->iPort);
	dbgprintf("SNMPVersion (0=v1, 1=v2c): %d\n", pData->iSNMPVersion);
	dbgprintf("Community: %s\n", pData->szCommunity);
	dbgprintf("EnterpriseOID: %s\n", pData->szEnterpriseOID);
	dbgprintf("SnmpTrapOID: %s\n", pData->szSnmpTrapOID);
	dbgprintf("SyslogMessageOID: %s\n", pData->szSyslogMessageOID);
	dbgprintf("TrapType: %d\n", pData->iTrapType);
	dbgprintf("SpecificType: %d\n", pData->iSpecificType);
ENDdbgPrintInstInfo


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	/* we are not compatible with repeated msg reduction feature, so do not allow it */
ENDisCompatibleWithFeature

/* Exit SNMP Session
 * alorbach, 2008-02-12
 */
static rsRetVal
omsnmp_exitSession(wrkrInstanceData_t *pWrkrData)
{
	DEFiRet;

	if(pWrkrData->snmpsession != NULL) {
		DBGPRINTF("omsnmp_exitSession: Clearing Session to '%s' on Port = '%d'\n",
			  pWrkrData->pData->szTarget, pWrkrData->pData->iPort);
		snmp_close(pWrkrData->snmpsession);
		pWrkrData->snmpsession = NULL;
	}

	RETiRet;
}

/* Init SNMP Session
 * alorbach, 2008-02-12
 */
static rsRetVal
omsnmp_initSession(wrkrInstanceData_t *pWrkrData)
{
	netsnmp_session session;
	instanceData *pData;
	char szTargetAndPort[MAXHOSTNAMELEN+128]; /* work buffer for specifying a full target and port string */
	DEFiRet;
	
	/* should not happen, but if session is not cleared yet - we do it now! */
	if (pWrkrData->snmpsession != NULL)
		omsnmp_exitSession(pWrkrData);

	pData = pWrkrData->pData;

	snprintf((char*)szTargetAndPort, sizeof(szTargetAndPort), "%s:%s:%d",
			(pData->szTransport == NULL) ? "udp" : (char*)pData->szTransport,
			pData->szTarget, pData->iPort == 0 ? 162 : pData->iPort);

	dbgprintf( "omsnmp_initSession: ENTER - Target = '%s' on Port = '%d'\n", pData->szTarget, pData->iPort);

	if (setenv("POSIXLY_CORRECT", "1", 1) == -1)
		ABORT_FINALIZE(RS_RET_ERR);
	
	snmp_sess_init(&session);
	session.version = pData->iSNMPVersion;
	session.callback = NULL; /* NOT NEEDED */
	session.callback_magic = NULL;
	session.peername = (char*) szTargetAndPort;
	
	/* Set SNMP Community */
	if (session.version == SNMP_VERSION_1 || session.version == SNMP_VERSION_2c) {
		session.community = (unsigned char *) pData->szCommunity
			== NULL ? (uchar*)"public" : pData->szCommunity;
		session.community_len = strlen((char*) session.community);
	}

	pWrkrData->snmpsession = snmp_open(&session);
	if (pWrkrData->snmpsession == NULL) {
		LogError(0, RS_RET_SUSPENDED, "omsnmp_initSession: snmp_open to host '%s' on Port '%d' "
		"failed\n", pData->szTarget, pData->iPort);
		/* Stay suspended */
		iRet = RS_RET_SUSPENDED;
	}

finalize_it:
	RETiRet;
}

static rsRetVal omsnmp_sendsnmp(wrkrInstanceData_t *pWrkrData, uchar *psz)
{
	DEFiRet;

	netsnmp_pdu    *pdu = NULL;
	oid             enterpriseoid[MAX_OID_LEN];
	size_t          enterpriseoidlen = MAX_OID_LEN;
	oid				oidSyslogMessage[MAX_OID_LEN];
	size_t			oLen = MAX_OID_LEN;
	int             status;
	char            *trap = NULL;
	const char		*strErr = NULL;
	instanceData *pData;

	pData = pWrkrData->pData;
	/* Init SNMP Session if necessary */
	if (pWrkrData->snmpsession == NULL) {
		CHKiRet(omsnmp_initSession(pWrkrData));
	}
	
	/* String should not be NULL */
	ASSERT(psz != NULL);
	dbgprintf( "omsnmp_sendsnmp: ENTER - Syslogmessage = '%s'\n", (char*)psz);

	/* If SNMP Version1 is configured !*/
	if(pWrkrData->snmpsession->version == SNMP_VERSION_1) {
		pdu = snmp_pdu_create(SNMP_MSG_TRAP);

		/* Set enterprise */
		if(!snmp_parse_oid(pData->szEnterpriseOID == NULL ? "1.3.6.1.4.1.3.1.1" :
			(char*)pData->szEnterpriseOID, enterpriseoid, &enterpriseoidlen )) {
			strErr = snmp_api_errstring(snmp_errno);
			LogError(0, RS_RET_DISABLE_ACTION, "omsnmp_sendsnmp: Parsing EnterpriseOID "
					"failed '%s' with error '%s' \n", pData->szSyslogMessageOID, strErr);
			ABORT_FINALIZE(RS_RET_DISABLE_ACTION);
		}
		CHKmalloc(pdu->enterprise = (oid *) MALLOC(enterpriseoidlen * sizeof(oid)));
		memcpy(pdu->enterprise, enterpriseoid, enterpriseoidlen * sizeof(oid));
		pdu->enterprise_length = enterpriseoidlen;

		/* Set Traptype */
		pdu->trap_type = pData->iTrapType;

		/* Set SpecificType */
		pdu->specific_type = pData->iSpecificType;

		/* Set Updtime */
		pdu->time = get_uptime();
	}
	/* If SNMP Version2c is configured !*/
	else if (pWrkrData->snmpsession->version == SNMP_VERSION_2c)
	{
		long sysuptime;
		char csysuptime[20];

		/* Create PDU */
		pdu = snmp_pdu_create(SNMP_MSG_TRAP2);

		/* Set uptime */
		sysuptime = get_uptime();
		snprintf( csysuptime, sizeof(csysuptime) , "%ld", sysuptime);
		trap = csysuptime;
		snmp_add_var(pdu, objid_sysuptime, sizeof(objid_sysuptime) / sizeof(oid), 't', trap);

		/* Now set the SyslogMessage Trap OID */
		if ( snmp_add_var(pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid), 'o',
			pData->szSnmpTrapOID == NULL ?  "1.3.6.1.4.1.19406.1.2.1" : (char*) pData->szSnmpTrapOID
			) != 0) {
			strErr = snmp_api_errstring(snmp_errno);
			LogError(0, RS_RET_DISABLE_ACTION, "omsnmp_sendsnmp: Adding trap OID failed '%s' "
			"with error '%s' \n", pData->szSnmpTrapOID, strErr);
			ABORT_FINALIZE(RS_RET_DISABLE_ACTION);
		}
	}

	/* SET TRAP PARAMETER for SyslogMessage! */
/*	dbgprintf( "omsnmp_sendsnmp: SyslogMessage '%s'\n", psz );*/

	/* First create new OID object */
	if (snmp_parse_oid(pData->szSyslogMessageOID == NULL ?
			    "1.3.6.1.4.1.19406.1.1.2.1" : (char*)pData->szSyslogMessageOID,
				oidSyslogMessage, &oLen)) {
		int iErrCode = snmp_add_var(pdu, oidSyslogMessage, oLen, 's', (char*) psz);
		if (iErrCode) {
			const char *str = snmp_api_errstring(iErrCode);
			LogError(0, RS_RET_DISABLE_ACTION,  "omsnmp_sendsnmp: Invalid SyslogMessage OID, "
			"error code '%d' - '%s'\n", iErrCode, str );
			ABORT_FINALIZE(RS_RET_DISABLE_ACTION);
		}
	} else {
		strErr = snmp_api_errstring(snmp_errno);
		LogError(0, RS_RET_DISABLE_ACTION, "omsnmp_sendsnmp: Parsing SyslogMessageOID failed '%s' "
		"with error '%s' \n", pData->szSyslogMessageOID, strErr);

		ABORT_FINALIZE(RS_RET_DISABLE_ACTION);
	}

	/* Send the TRAP */
	status = snmp_send(pWrkrData->snmpsession, pdu) == 0;
	if (status)
	{
		/* Debug Output! */
		int iErrorCode = pWrkrData->snmpsession->s_snmp_errno;
		LogError(0, RS_RET_SUSPENDED,  "omsnmp_sendsnmp: snmp_send failed error '%d', "
		"Description='%s'\n", iErrorCode*(-1), api_errors[iErrorCode*(-1)]);

		/* Clear Session */
		omsnmp_exitSession(pWrkrData);

		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pdu != NULL) {
			snmp_free_pdu(pdu);
		}
	}

	dbgprintf( "omsnmp_sendsnmp: LEAVE\n");
	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	iRet = omsnmp_initSession(pWrkrData);
ENDtryResume

BEGINdoAction
CODESTARTdoAction
	/* Abort if the STRING is not set, should never happen */
	if (ppString[0] == NULL) {
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}
	
	/* This will generate and send the SNMP Trap */
	iRet = omsnmp_sendsnmp(pWrkrData, ppString[0]);
finalize_it:
ENDdoAction

BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->tplName);
	free(pData->szTarget);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	omsnmp_exitSession(pWrkrData);
ENDfreeWrkrInstance

static void
setInstParamDefaults(instanceData *pData)
{
	pData->tplName = NULL;
	pData->szCommunity = NULL;
	pData->szEnterpriseOID = NULL;
	pData->szSnmpTrapOID = NULL;
	pData->szSyslogMessageOID = NULL;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	CODE_STD_STRING_REQUESTnewActInst(1)
	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "server")) {
			pData->szTarget = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "port")) {
			pData->iPort = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "transport")) {
			pData->szTransport = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "version")) {
			pData->iSNMPVersion = pvals[i].val.d.n;
			if(pData->iSNMPVersion < 0 || cs.iSNMPVersion > 1)
				pData->iSNMPVersion = 1;
		} else if(!strcmp(actpblk.descr[i].name, "community")) {
			pData->szCommunity = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "enterpriseoid")) {
			pData->szEnterpriseOID = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "trapoid")) {
			pData->szSnmpTrapOID = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "messageoid")) {
			pData->szSyslogMessageOID = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "traptype")) {
			pData->iTrapType = pvals[i].val.d.n;
			if(cs.iTrapType < 0 || cs.iTrapType >= 6) {
				parser_errmsg("omsnmp: traptype invalid, setting to ENTERPRISESPECIFIC");
				pData->iTrapType = SNMP_TRAP_ENTERPRISESPECIFIC;
			}
		} else if(!strcmp(actpblk.descr[i].name, "specifictype")) {
			pData->iSpecificType = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("ompipe: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

	/* Init NetSNMP library and read in MIB database */
	init_snmp("rsyslog");

	/* Set some defaults in the NetSNMP library */
	netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DEFAULT_PORT, pData->iPort );

	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((pData->tplName == NULL) ?
						"RSYSLOG_FileFormat" : (char*)pData->tplName),
						OMSR_NO_RQD_TPL_OPTS));

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	if(!strncmp((char*) p, ":omsnmp:", sizeof(":omsnmp:") - 1)) {
		p += sizeof(":omsnmp:") - 1; /* eat indicator sequence (-1 because of '\0'!) */
	} else {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	if((iRet = createInstance(&pData)) != RS_RET_OK)
		FINALIZE;

	/* Check Target */
	if(cs.pszTarget == NULL) {
		ABORT_FINALIZE( RS_RET_PARAM_ERROR );
	} else {
		CHKmalloc(pData->szTarget = (uchar*) strdup((char*)cs.pszTarget));
	}

	/* copy config params */
	pData->szTransport = (uchar*) ((cs.pszTransport == NULL) ? NULL : strdup((char*)cs.pszTransport));
	pData->szCommunity = (uchar*) ((cs.pszCommunity == NULL) ? NULL : strdup((char*)cs.pszCommunity));
	pData->szEnterpriseOID = (uchar*) ((cs.pszEnterpriseOID == NULL) ? NULL : strdup((char*)cs.pszEnterpriseOID));
	pData->szSnmpTrapOID = (uchar*) ((cs.pszSnmpTrapOID == NULL) ? NULL : strdup((char*)cs.pszSnmpTrapOID));
	pData->szSyslogMessageOID = (uchar*) ((cs.pszSyslogMessageOID == NULL)
	? NULL : strdup((char*)cs.pszSyslogMessageOID));
	pData->iPort = cs.iPort;
	pData->iSpecificType = cs.iSpecificType;
	
	/* Set SNMPVersion */
	if ( cs.iSNMPVersion < 0 || cs.iSNMPVersion > 1)		/* Set default to 1 if out of range */
		pData->iSNMPVersion = 1;
	else
		pData->iSNMPVersion = cs.iSNMPVersion;

	/* Copy TrapType */
	if ( cs.iTrapType < 0 || cs.iTrapType >= 6)		/* Only allow values from 0 to 6 !*/
		pData->iTrapType = SNMP_TRAP_ENTERPRISESPECIFIC;
	else
		pData->iTrapType = cs.iTrapType;

	/* Print Debug info */
	dbgprintf("SNMPTransport: %s\n", pData->szTransport);
	dbgprintf("SNMPTarget: %s\n", pData->szTarget);
	dbgprintf("SNMPPort: %d\n", pData->iPort);
	dbgprintf("SNMPVersion (0=v1, 1=v2c): %d\n", pData->iSNMPVersion);
	dbgprintf("Community: %s\n", pData->szCommunity);
	dbgprintf("EnterpriseOID: %s\n", pData->szEnterpriseOID);
	dbgprintf("SnmpTrapOID: %s\n", pData->szSnmpTrapOID);
	dbgprintf("SyslogMessageOID: %s\n", pData->szSyslogMessageOID);
	dbgprintf("TrapType: %d\n", pData->iTrapType);
	dbgprintf("SpecificType: %d\n", pData->iSpecificType);

	/* process template */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS,
		(uchar*) "RSYSLOG_TraditionalForwardFormat"));

	/* Init NetSNMP library and read in MIB database */
	init_snmp("rsyslog");

	/* Set some defaults in the NetSNMP library */
	netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DEFAULT_PORT, pData->iPort );
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


/* Reset config variables for this module to default values.
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	free(cs.pszTarget);
	cs.pszTarget = NULL;
	free(cs.pszCommunity);
	cs.pszCommunity = NULL;
	free(cs.pszEnterpriseOID);
	cs.pszEnterpriseOID = NULL;
	free(cs.pszSnmpTrapOID);
	cs.pszSnmpTrapOID = NULL;
	free(cs.pszSyslogMessageOID);
	cs.pszSyslogMessageOID = NULL;
	cs.iPort = 0;
	cs.iSNMPVersion = 1;
	cs.iSpecificType = 0;
	cs.iTrapType = SNMP_TRAP_ENTERPRISESPECIFIC;
	RETiRet;
}


BEGINmodExit
CODESTARTmodExit
	free(cs.pszTarget);
	free(cs.pszCommunity);
	free(cs.pszEnterpriseOID);
	free(cs.pszSnmpTrapOID);
	free(cs.pszSyslogMessageOID);

	/* release what we no longer need */
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	initConfVars();

	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmptransport", 0, eCmdHdlrGetWord, NULL, &cs.pszTransport,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmptarget", 0, eCmdHdlrGetWord, NULL, &cs.pszTarget,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmptargetport", 0, eCmdHdlrInt, NULL, &cs.iPort,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmpversion", 0, eCmdHdlrInt, NULL, &cs.iSNMPVersion,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmpcommunity", 0, eCmdHdlrGetWord, NULL, &cs.pszCommunity,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmpenterpriseoid", 0, eCmdHdlrGetWord, NULL,
	&cs.pszEnterpriseOID, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmptrapoid", 0, eCmdHdlrGetWord, NULL, &cs.pszSnmpTrapOID,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmpsyslogmessageoid", 0, eCmdHdlrGetWord, NULL,
	&cs.pszSyslogMessageOID, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmpspecifictype", 0, eCmdHdlrInt, NULL, &cs.iSpecificType,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionsnmptraptype", 0, eCmdHdlrInt, NULL, &cs.iTrapType,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler, resetConfigVariables,
	NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit
/*
 * vi:set ai:
 */
