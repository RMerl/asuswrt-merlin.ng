/* mmrfc5424addhmac.c
 * custom module: add hmac to RFC5424 messages
 *
 * Note on important design decision: This module is fully self-contained.
 * Most importantly, it does not rely on mmpstrucdata to populate the
 * structured data portion of the messages JSON. There are two reasons
 * for this:
 * 1. robustness
 *    - this guard against misconfiguration
 *    - it permits us to be more liberal in regard to malformed
 *      structured data
 *    - it permits us to handle border-cases (like duplicate
 *      SD-IDs) with much less complexity
 * 2. performance
 *    With being "on the spot" of what we need we can reduce memory
 *    reads and writes. This is a considerable save if the JSON representation
 *    is not otherwise needed.
 *
 * Note that the recommended calling sequence if both of these modules
 * are used is
 *
 * 1. mmrfc5424addhmac
 * 2. mmpstrucdata
 *
 * This sequence permits mmpstrucdata to pick up the modifications we
 * made in this module here.
 *
 * Copyright 2013 Adiscon GmbH.
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
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <openssl/hmac.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmrfc5424addhmac")


DEF_OMOD_STATIC_DATA

/* config variables */

typedef struct _instanceData {
	uchar *key;
	int16_t keylen;	/* cached length of key, to avoid recomputation */
	uchar *sdid;	/* SD-ID to be used to persist the hmac */
	int16_t sdidLen;
	const EVP_MD *algo;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
};
static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "key", eCmdHdlrString, 1 },
	{ "hashfunction", eCmdHdlrString, 1 },
	{ "sd_id", eCmdHdlrGetWord, 1 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
ENDbeginCnfLoad

BEGINendCnfLoad
CODESTARTendCnfLoad
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
ENDfreeCnf


BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->key = NULL;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	char *ciphername;
	int i;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmrfc5424addhmac)\n");
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CODE_STD_STRING_REQUESTnewActInst(1)
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, NULL, OMSR_TPL_AS_MSG));
	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "key")) {
			pData->key = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			pData->keylen = es_strlen(pvals[i].val.d.estr);
		} else if(!strcmp(actpblk.descr[i].name, "hashfunction")) {
			ciphername = es_str2cstr(pvals[i].val.d.estr, NULL);
			pData->algo = EVP_get_digestbyname(ciphername);
			if(pData->algo == NULL) {
				LogError(0, RS_RET_CRY_INVLD_ALGO,
					"hashFunction '%s' unknown to openssl - "
					"cannot continue", ciphername);
				free(ciphername);
				ABORT_FINALIZE(RS_RET_CRY_INVLD_ALGO);
			}
			free(ciphername);
		} else if(!strcmp(actpblk.descr[i].name, "sd_id")) {
			pData->sdid = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			pData->sdidLen = es_strlen(pvals[i].val.d.estr);
		} else {
			dbgprintf("mmrfc5424addhmac: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume


/* turn the binary data in bin of length len into a
 * printable hex string. "print" must be 2*len+1 (for \0)
 */
static void
hexify(uchar *bin, int len, uchar *print)
{
	static const char hexchars[16] =
	   {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	int iSrc, iDst;

	for(iSrc = iDst = 0 ; iSrc < len ; ++iSrc) {
		print[iDst++] = hexchars[bin[iSrc]>>4];
		print[iDst++] = hexchars[bin[iSrc]&0x0f];
	}
	print[iDst] = '\0';
}


/* skip to end of current SD-ID. This function can be improved
 * in regard to fully parsing based on RFC5424, HOWEVER, this would
 * also reduce performance. So we consider the current implementation
 * to be superior.
 */
static void
skipSDID(uchar *sdbuf, int sdlen, int *rootIdx)
{
	int i;
	i = *rootIdx;
	while(i < sdlen) {
		if(sdbuf[i] == ']') {
			if(i > *rootIdx && sdbuf[i-1] == '\\') {
				; /* escaped, nothing to do! */
			} else {
				++i; /* eat ']' */
				break;
			}
		}
		++i;
	}
	*rootIdx = i;
}

static void
getSDID(uchar *sdbuf, int sdlen, int *rootIdx, uchar *sdid)
{
	int i, j;
	i = *rootIdx;
	j = 0;

	if(sdbuf[i] != '[') {
		++i;
		goto done;
	}
	
	++i;
	while(i < sdlen && sdbuf[i] != '=' && sdbuf[i] != ' '
	                && sdbuf[i] != ']' && sdbuf[i] != '"') {
		sdid[j++] = sdbuf[i++];
	}
done:
	sdid[j] = '\0';
	*rootIdx = i;
}

/* check if "our" hmac is already present */
static sbool
isHmacPresent(instanceData *pData, smsg_t *pMsg)
{
	uchar *sdbuf;
	rs_size_t sdlen;
	sbool found;
	int i;
	uchar sdid[33]; /* RFC-based size limit */

	MsgGetStructuredData(pMsg, &sdbuf, &sdlen);
	found = 0;

	if(sdbuf[0] == '-') /* RFC: struc data is empty! */
		goto done;

	i = 0;
	while(i < sdlen && !found) {
		getSDID(sdbuf, sdlen, &i, sdid);
		if(!strcmp((char*)pData->sdid, (char*)sdid)) {
			found = 1;
			break;
		}
		skipSDID(sdbuf, sdlen, &i);
	}

done:
	return found;
}

static rsRetVal
hashMsg(instanceData *pData, smsg_t *pMsg)
{
	uchar *pRawMsg;
	int lenRawMsg;
	uchar *sdbuf;
	rs_size_t sdlen;
	unsigned int hashlen;
	uchar hash[EVP_MAX_MD_SIZE];
	uchar hashPrintable[2*EVP_MAX_MD_SIZE+1];
	uchar newsd[64*1024];	/* we assume this is sufficient... */
	int lenNewsd;
	DEFiRet;

	MsgGetStructuredData(pMsg, &sdbuf, &sdlen);
	getRawMsg(pMsg, &pRawMsg, &lenRawMsg);
	HMAC(pData->algo, pData->key, pData->keylen,
	     pRawMsg, lenRawMsg, hash, &hashlen);
	hexify(hash, hashlen, hashPrintable);
	lenNewsd = snprintf((char*)newsd, sizeof(newsd), "[%s hash=\"%s\"]",
		            (char*)pData->sdid, (char*)hashPrintable);
	MsgAddToStructuredData(pMsg, newsd, lenNewsd);
	RETiRet;
}


BEGINdoAction
	instanceData *pData = pWrkrData->pData;
	smsg_t *pMsg;
CODESTARTdoAction
	pMsg = (smsg_t*) ppString[0];
	if(   msgGetProtocolVersion(pMsg) == MSG_RFC5424_PROTOCOL
	   && !isHmacPresent(pData, pMsg)) {
		hashMsg(pData, pMsg);
	} else {
		if(Debug) {
			uchar *pRawMsg;
			int lenRawMsg;
			getRawMsg(pMsg, &pRawMsg, &lenRawMsg);
			dbgprintf("mmrfc5424addhmac: non-rfc5424 or HMAC already "
			          "present: %.256s\n", pRawMsg);
		}
	}
ENDdoAction


NO_LEGACY_CONF_parseSelectorAct


BEGINmodExit
CODESTARTmodExit
	EVP_cleanup();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
ENDqueryEtryPt



BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	DBGPRINTF("mmrfc5424addhmac: module compiled with rsyslog version %s.\n", VERSION);
	OpenSSL_add_all_digests();
ENDmodInit
