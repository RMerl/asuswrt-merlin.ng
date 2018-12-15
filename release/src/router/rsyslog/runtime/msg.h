/* msg.h
 * Header file for all msg-related functions.
 *
 * File begun on 2007-07-13 by RGerhards (extracted from syslogd.c)
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "template.h" /* this is a quirk, but these two are too interdependant... */

#ifndef	MSG_H_INCLUDED
#define	MSG_H_INCLUDED 1

#include <pthread.h>
#include <libestr.h>
#include <stdint.h>
#include <json.h>
#include "obj.h"
#include "syslogd-types.h"
#include "template.h"
#include "atomic.h"

/* rgerhards 2004-11-08: The following structure represents a
 * syslog message.
 *
 * Important Note:
 * The message object is used for multiple purposes (once it
 * has been created). Once created, it actully is a read-only
 * object (though we do not specifically express this). In order
 * to avoid multiple copies of the same object, we use a
 * reference counter. This counter is set to 1 by the constructer
 * and increased by 1 with a call to MsgAddRef(). The destructor
 * checks the reference count. If it is more than 1, only the counter
 * will be decremented. If it is 1, however, the object is actually
 * destroyed. To make this work, it is vital that MsgAddRef() is
 * called each time a "copy" is stored somewhere.
 *
 * WARNING: this structure is not calloc()ed, so be careful when
 * adding new fields. You need to initialize them in
 * msgBaseConstruct(). That function header comment also describes
 * why this is the case.
 */
struct msg {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	flowControl_t flowCtlType;
	/**< type of flow control we can apply, for enqueueing, needs not to be persisted because
				        once data has entered the queue, this property is no longer needed. */
	pthread_mutex_t mut;
	int	iRefCount;	/* reference counter (0 = unused) */
	sbool	bParseSuccess;	/* set to reflect state of last executed higher level parser */
	unsigned short	iSeverity;/* the severity  */
	unsigned short	iFacility;/* Facility code */
	short	offAfterPRI;	/* offset, at which raw message WITHOUT PRI part starts in pszRawMsg */
	short	offMSG;		/* offset at which the MSG part starts in pszRawMsg */
	short	iProtocolVersion;/* protocol version of message received 0 - legacy, 1 syslog-protocol) */
	int	msgFlags;	/* flags associated with this message */
	int	iLenRawMsg;	/* length of raw message */
	int	iLenMSG;	/* Length of the MSG part */
	int	iLenTAG;	/* Length of the TAG part */
	int	iLenHOSTNAME;	/* Length of HOSTNAME */
	int	iLenPROGNAME;	/* Length of PROGNAME (-1 = not yet set) */
	uchar	*pszRawMsg;	/* message as it was received on the wire. This is important in case we
				 * need to preserve cryptographic verifiers.  */
	uchar	*pszHOSTNAME;	/* HOSTNAME from syslog message */
	char *pszRcvdAt3164;	/* time as RFC3164 formatted string (always 15 charcters) */
	char *pszRcvdAt3339;	/* time as RFC3164 formatted string (32 charcters at most) */
	char *pszRcvdAt_MySQL;	/* rcvdAt as MySQL formatted string (always 14 charcters) */
	char *pszRcvdAt_PgSQL;  /* rcvdAt as PgSQL formatted string (always 21 characters) */
	char *pszTIMESTAMP3164;	/* TIMESTAMP as RFC3164 formatted string (always 15 charcters) */
	char *pszTIMESTAMP3339;	/* TIMESTAMP as RFC3339 formatted string (32 charcters at most) */
	char *pszTIMESTAMP_MySQL;/* TIMESTAMP as MySQL formatted string (always 14 charcters) */
	char *pszTIMESTAMP_PgSQL;/* TIMESTAMP as PgSQL formatted string (always 21 characters) */
	uchar *pszStrucData;    /* STRUCTURED-DATA */
	uint16_t lenStrucData;	/* (cached) length of STRUCTURED-DATA */
	cstr_t *pCSAPPNAME;	/* APP-NAME */
	cstr_t *pCSPROCID;	/* PROCID */
	cstr_t *pCSMSGID;	/* MSGID */
	prop_t *pInputName;	/* input name property */
	prop_t *pRcvFromIP;	/* IP of system message was received from */
	union {
		prop_t *pRcvFrom;/* name of system message was received from */
		struct sockaddr_storage *pfrominet; /* unresolved name */
	} rcvFrom;

	ruleset_t *pRuleset;	/* ruleset to be used for processing this message */
	time_t ttGenTime;	/* time msg object was generated, same as tRcvdAt, but a Unix timestamp.
				   While this field looks redundant, it is required because a Unix timestamp
				   is used at later processing stages (namely in the output arena). Thanks to
				   the subleties of how time is defined, there is no reliable way to reconstruct
				   the Unix timestamp from the syslogTime fields (in practice, we may be close
				   enough to reliable, but I prefer to leave the subtle things to the OS, where
				   it obviously is solved in way or another...). */
	struct syslogTime tRcvdAt;/* time the message entered this program */
	struct syslogTime tTIMESTAMP;/* (parsed) value of the timestamp */
	struct json_object *json;
	struct json_object *localvars;
	/* some fixed-size buffers to save malloc()/free() for frequently used fields (from the default templates) */
	uchar szRawMsg[CONF_RAWMSG_BUFSIZE];
	/* most messages are small, and these are stored here (without malloc/free!) */
	uchar szHOSTNAME[CONF_HOSTNAME_BUFSIZE];
	union {
		uchar	*ptr;	/* pointer to progname value */
		uchar	szBuf[CONF_PROGNAME_BUFSIZE];
	} PROGNAME;
	union {
		uchar	*pszTAG;	/* pointer to tag value */
		uchar	szBuf[CONF_TAG_BUFSIZE];
	} TAG;
	char pszTimestamp3164[CONST_LEN_TIMESTAMP_3164 + 1];
	char pszTimestamp3339[CONST_LEN_TIMESTAMP_3339 + 1];
	char pszTIMESTAMP_SecFrac[7];
	/* Note: a pointer is 64 bits/8 char, so this is actually fewer than a pointer! */
	char pszRcvdAt_SecFrac[7];
	/* same as above. Both are fractional seconds for their respective timestamp */
	char pszTIMESTAMP_Unix[12]; /* almost as small as a pointer! */
	char pszRcvdAt_Unix[12];
	char dfltTZ[8];	    /* 7 chars max, less overhead than ptr! */
	uchar *pszUUID; /* The message's UUID */
};


/* message flags (msgFlags), not an enum for historical reasons */
#define NOFLAG		0x000
/* no flag is set (to be used when a flag must be specified and none is required) */
#define INTERNAL_MSG	0x001
/* msg generated by logmsgInternal() --> special handling */
/* 0x002 not used because it was previously a known value - rgerhards, 2008-10-09 */
#define IGNDATE		0x004
/* ignore, if given, date in message and use date of reception as msg date */
#define MARK		0x008
/* this message is a mark */
#define NEEDS_PARSING	0x010
/* raw message, must be parsed before processing can be done */
#define PARSE_HOSTNAME	0x020
/* parse the hostname during message parsing */
#define NEEDS_DNSRESOL	0x040
/* fromhost address is unresolved and must be locked up via DNS reverse lookup first */
#define NEEDS_ACLCHK_U	0x080
/* check UDP ACLs after DNS resolution has been done in main queue consumer */
#define NO_PRI_IN_RAW	0x100
/* rawmsg does not include a PRI (Solaris!), but PRI is already set correctly in the msg object */
#define PRESERVE_CASE	0x200
/* preserve case in fromhost */

/* (syslog) protocol types */
#define MSG_LEGACY_PROTOCOL 0
#define MSG_RFC5424_PROTOCOL 1

#define MAX_VARIABLE_NAME_LEN 1024

/* function prototypes
 */
PROTOTYPEObjClassInit(msg);
rsRetVal msgConstruct(smsg_t **ppThis);
rsRetVal msgConstructWithTime(smsg_t **ppThis, const struct syslogTime *stTime, const time_t ttGenTime);
rsRetVal msgConstructForDeserializer(smsg_t **ppThis);
rsRetVal msgConstructFinalizer(smsg_t *pThis);
rsRetVal msgDestruct(smsg_t **ppM);
smsg_t * MsgDup(smsg_t * pOld);
smsg_t *MsgAddRef(smsg_t *pM);
void setProtocolVersion(smsg_t *pM, int iNewVersion);
void MsgSetInputName(smsg_t *pMsg, prop_t*);
void MsgSetDfltTZ(smsg_t *pThis, char *tz);
rsRetVal MsgSetAPPNAME(smsg_t *pMsg, const char* pszAPPNAME);
rsRetVal MsgSetPROCID(smsg_t *pMsg, const char* pszPROCID);
rsRetVal MsgSetMSGID(smsg_t *pMsg, const char* pszMSGID);
void MsgSetParseSuccess(smsg_t *pMsg, int bSuccess);
void MsgSetTAG(smsg_t *pMsg, const uchar* pszBuf, const size_t lenBuf);
void MsgSetRuleset(smsg_t *pMsg, ruleset_t*);
rsRetVal MsgSetFlowControlType(smsg_t *pMsg, flowControl_t eFlowCtl);
rsRetVal MsgSetStructuredData(smsg_t *const pMsg, const char* pszStrucData);
rsRetVal MsgAddToStructuredData(smsg_t *pMsg, uchar *toadd, rs_size_t len);
void MsgGetStructuredData(smsg_t *pM, uchar **pBuf, rs_size_t *len);
rsRetVal msgSetFromSockinfo(smsg_t *pThis, struct sockaddr_storage *sa);
void MsgSetRcvFrom(smsg_t *pMsg, prop_t*);
void MsgSetRcvFromStr(smsg_t *const pMsg, const uchar* pszRcvFrom, const int, prop_t **);
rsRetVal MsgSetRcvFromIP(smsg_t *pMsg, prop_t*);
rsRetVal MsgSetRcvFromIPStr(smsg_t *const pThis, const uchar *psz, const int len, prop_t **ppProp);
void MsgSetHOSTNAME(smsg_t *pMsg, const uchar* pszHOSTNAME, const int lenHOSTNAME);
rsRetVal MsgSetAfterPRIOffs(smsg_t *pMsg, short offs);
void MsgSetMSGoffs(smsg_t *pMsg, short offs);
void MsgSetRawMsgWOSize(smsg_t *pMsg, char* pszRawMsg);
void ATTR_NONNULL() MsgSetRawMsg(smsg_t *const pThis, const char*const pszRawMsg, const size_t lenMsg);
rsRetVal MsgReplaceMSG(smsg_t *pThis, const uchar* pszMSG, int lenMSG);
uchar *MsgGetProp(smsg_t *pMsg, struct templateEntry *pTpe, msgPropDescr_t *pProp,
		  rs_size_t *pPropLen, unsigned short *pbMustBeFreed, struct syslogTime *ttNow);
uchar *getRcvFrom(smsg_t *pM);
void getTAG(smsg_t *pM, uchar **ppBuf, int *piLen);
const char *getTimeReported(smsg_t *pM, enum tplFormatTypes eFmt);
const char *getPRI(smsg_t *pMsg);
int getPRIi(const smsg_t * const pM);
int ATTR_NONNULL() getRawMsgLen(const smsg_t *const pMsg);
void getRawMsg(const smsg_t *pM, uchar **pBuf, int *piLen);
void ATTR_NONNULL() MsgTruncateToMaxSize(smsg_t *const pThis);
rsRetVal msgAddJSON(smsg_t *pM, uchar *name, struct json_object *json, int force_reset, int sharedReference);
rsRetVal msgAddMetadata(smsg_t *msg, uchar *metaname, uchar *metaval);
rsRetVal msgAddMultiMetadata(smsg_t *msg, const uchar **metaname, const uchar **metaval, const int count);
rsRetVal MsgGetSeverity(smsg_t *pThis, int *piSeverity);
rsRetVal MsgDeserialize(smsg_t *pMsg, strm_t *pStrm);
rsRetVal MsgSetPropsViaJSON(smsg_t *__restrict__ const pMsg, const uchar *__restrict__ const json);
rsRetVal MsgSetPropsViaJSON_Object(smsg_t *__restrict__ const pMsg, struct json_object *json);
const uchar* msgGetJSONMESG(smsg_t *__restrict__ const pMsg);

uchar *getMSG(smsg_t *pM);
const char *getHOSTNAME(smsg_t *pM);
char *getPROCID(smsg_t *pM, sbool bLockMutex);
char *getAPPNAME(smsg_t *pM, sbool bLockMutex);
void setMSGLen(smsg_t *pM, int lenMsg);
int getMSGLen(smsg_t *pM);
void getInputName(const smsg_t * const pM, uchar **ppsz, int *const plen);

int getHOSTNAMELen(smsg_t *pM);
uchar *getProgramName(smsg_t *pM, sbool bLockMutex);
uchar *getRcvFrom(smsg_t *pM);
rsRetVal propNameToID(uchar *pName, propid_t *pPropID);
uchar *propIDToName(propid_t propID);
rsRetVal msgGetJSONPropJSON(smsg_t *pMsg, msgPropDescr_t *pProp, struct json_object **pjson);
rsRetVal msgGetJSONPropJSONorString(smsg_t * const pMsg, msgPropDescr_t *pProp, struct json_object **pjson,
uchar **pcstr);
rsRetVal getJSONPropVal(smsg_t *pMsg, msgPropDescr_t *pProp, uchar **pRes, rs_size_t *buflen,
unsigned short *pbMustBeFreed);
rsRetVal msgSetJSONFromVar(smsg_t *pMsg, uchar *varname, struct svar *var, int force_reset);
rsRetVal msgDelJSON(smsg_t *pMsg, uchar *varname);
rsRetVal jsonFind(struct json_object *jroot, msgPropDescr_t *pProp, struct json_object **jsonres);

rsRetVal msgPropDescrFill(msgPropDescr_t *pProp, uchar *name, int nameLen);
void msgPropDescrDestruct(msgPropDescr_t *pProp);
void msgSetPRI(smsg_t *const __restrict__ pMsg, syslog_pri_t pri);

#define msgGetProtocolVersion(pM) ((pM)->iProtocolVersion)

/* returns non-zero if the message has structured data, 0 otherwise */
#define MsgHasStructuredData(pM) (((pM)->pszStrucData == NULL) ? 0 : 1)

/* ------------------------------ some inline functions ------------------------------ */

/* add Metadata to the message. This is stored in a special JSON
 * container. Note that only string types are currently supported,
 * what should pose absolutely no problem with the string-ish nature
 * of rsyslog metadata.
 * added 2015-01-09 rgerhards
 */
/* set raw message size. This is needed in some cases where a trunctation is necessary
 * but the raw message must not be newly set. The most important (and currently only)
 * use case is if we remove trailing LF or NUL characters. Note that the size can NOT
 * be extended, only shrunk!
 * rgerhards, 2009-08-26
 */
static inline void __attribute__((unused))
MsgSetRawMsgSize(smsg_t *const __restrict__ pMsg, const size_t newLen)
{
	assert(newLen <= (size_t) pMsg->iLenRawMsg);
	pMsg->iLenRawMsg = newLen;
	pMsg->pszRawMsg[newLen] = '\0';
}

/* get the ruleset that is associated with the ruleset.
 * May be NULL. -- rgerhards, 2009-10-27
 */
#define MsgGetRuleset(pMsg) ((pMsg)->pRuleset)

#endif /* #ifndef MSG_H_INCLUDED */
