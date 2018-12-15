/* ommail.c
 *
 * This is an implementation of a mail sending output module. So far, we
 * only support direct SMTP, that is talking to a SMTP server. In the long
 * term, support for using sendmail should also be implemented. Please note
 * that the SMTP protocol implementation is a very bare one. We support
 * RFC821/822 messages, without any authentication and any other nice
 * features (no MIME, no nothing). It is assumed that proper firewalling
 * and/or STMP server configuration is used together with this module.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2008-04-04 by RGerhards
 *
 * Copyright 2008-2014 Adiscon GmbH.
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
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "cfsysline.h"
#include "module-template.h"
#include "errmsg.h"
#include "datetime.h"
#include "glbl.h"
#include "parserif.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("ommail")

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(datetime)

/* we add a little support for multiple recipients. We do this via a
 * singly-linked list, enqueued from the top. -- rgerhards, 2008-08-04
 */
typedef struct toRcpt_s toRcpt_t;
struct toRcpt_s {
	uchar *pszTo;
	toRcpt_t *pNext;
};

typedef struct _instanceData {
	uchar *tplName;	/* format template to use */
	uchar *constSubject; /* if non-NULL, constant string to be used as subject */
	int8_t iMode;	/* 0 - smtp, 1 - sendmail */
	sbool bHaveSubject; /* is a subject configured? (if so, it is the second string provided by rsyslog core) */
	sbool bEnableBody; /* is a body configured? (if so, it is the second string provided by rsyslog core) */
	union {
		struct {
			uchar *pszSrv;
			uchar *pszSrvPort;
			uchar *pszFrom;
			toRcpt_t *lstRcpt;
			} smtp;
	} md;	/* mode-specific data */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
	union {
		struct {
			char RcvBuf[1024]; /* buffer for receiving server responses */
			size_t lenRcvBuf;
			size_t iRcvBuf;	/* current index into the rcvBuf (buf empty if iRcvBuf == lenRcvBuf) */
			int sock;	/* socket to this server (most important when we do multiple msgs per mail) */
			} smtp;
	} md;	/* mode-specific data */
} wrkrInstanceData_t;

typedef struct configSettings_s {
	toRcpt_t *lstRcpt;
	uchar *pszSrv;
	uchar *pszSrvPort;
	uchar *pszFrom;
	uchar *pszSubject;
	int bEnableBody; /* should a mail body be generated? (set to 0 eg for SMS gateways) */
} configSettings_t;
static configSettings_t cs;

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "server", eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "port", eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "mailfrom", eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "mailto", eCmdHdlrArray, CNFPARAM_REQUIRED },
	{ "subject.template", eCmdHdlrGetWord, 0 },
	{ "subject.text", eCmdHdlrString, 0 },
	{ "body.enable", eCmdHdlrBinary, 0 },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};



BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	cs.lstRcpt = NULL;
	cs.pszSrv = NULL;
	cs.pszSrvPort = NULL;
	cs.pszFrom = NULL;
	cs.pszSubject = NULL;
	cs.bEnableBody = 1; /* should a mail body be generated? (set to 0 eg for SMS gateways) */
ENDinitConfVars

/* forward definitions (as few as possible) */
static rsRetVal Send(int sock, const char *msg, size_t len);
static rsRetVal readResponse(wrkrInstanceData_t *pWrkrData, int *piState, int iExpected);


/* helpers for handling the recipient lists */

/* destroy a complete recipient list */
static void lstRcptDestruct(toRcpt_t *pRoot)
{
	toRcpt_t *pDel;

	while(pRoot != NULL) {
		pDel = pRoot;
		pRoot = pRoot->pNext;
		/* ready to disalloc */
		free(pDel->pszTo);
		free(pDel);
	}
}


/* This function adds a recipient to the specified list.
 * The recipient address storage is handed over -- the caller must NOT delete it.
 */
static rsRetVal
addRcpt(toRcpt_t **ppLstRcpt, uchar *newRcpt)
{
	DEFiRet;
	toRcpt_t *pNew = NULL;

	CHKmalloc(pNew = calloc(1, sizeof(toRcpt_t)));

	pNew->pszTo = newRcpt;
	pNew->pNext = *ppLstRcpt;
	*ppLstRcpt = pNew;

	DBGPRINTF("ommail::addRcpt adds recipient %s\n", newRcpt);

finalize_it:
	if(iRet != RS_RET_OK) {
		free(pNew);
		free(newRcpt); /* in any case, this is no longer needed */
	}

	RETiRet;
}

/* This function is called when a new recipient email address is to be
 * added. rgerhards, 2008-08-04
 */
static rsRetVal
legacyConfAddRcpt(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	return addRcpt(&cs.lstRcpt, pNewVal);
}


/* output the recipient list to the mail server
 * iStatusToCheck < 0 means no checking should happen
 */
static rsRetVal
WriteRcpts(wrkrInstanceData_t *pWrkrData, uchar *pszOp, size_t lenOp, int iStatusToCheck)
{
	toRcpt_t *pRcpt;
	int iState;
	DEFiRet;

	assert(lenOp != 0);

	for(pRcpt = pWrkrData->pData->md.smtp.lstRcpt ; pRcpt != NULL ; pRcpt = pRcpt->pNext) {
		DBGPRINTF("Sending '%s: <%s>'\n", pszOp, pRcpt->pszTo);
		CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)pszOp, lenOp));
		CHKiRet(Send(pWrkrData->md.smtp.sock, ":<", sizeof(":<") - 1));
		CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)pRcpt->pszTo, strlen((char*)pRcpt->pszTo)));
		CHKiRet(Send(pWrkrData->md.smtp.sock, ">\r\n", sizeof(">\r\n") - 1));
		if(iStatusToCheck >= 0)
			CHKiRet(readResponse(pWrkrData, &iState, iStatusToCheck));
	}

finalize_it:
	RETiRet;
}


/* output the recipient list in rfc2822 format
 */
static rsRetVal
WriteTos(wrkrInstanceData_t *pWrkrData, uchar *pszOp, size_t lenOp)
{
	toRcpt_t *pRcpt;
	int iTos;
	DEFiRet;

	assert(lenOp != 0);

	CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)pszOp, lenOp));
	CHKiRet(Send(pWrkrData->md.smtp.sock, ": ", sizeof(": ") - 1));

	for(pRcpt = pWrkrData->pData->md.smtp.lstRcpt, iTos = 0; pRcpt != NULL ; pRcpt = pRcpt->pNext, iTos++) {
		DBGPRINTF("Sending '%s: <%s>'\n", pszOp, pRcpt->pszTo);
		if(iTos)
			CHKiRet(Send(pWrkrData->md.smtp.sock, ", ", sizeof(", ") - 1));
		CHKiRet(Send(pWrkrData->md.smtp.sock, "<", sizeof("<") - 1));
		CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)pRcpt->pszTo, strlen((char*)pRcpt->pszTo)));
		CHKiRet(Send(pWrkrData->md.smtp.sock, ">", sizeof(">") - 1));
	}

	CHKiRet(Send(pWrkrData->md.smtp.sock, "\r\n", sizeof("\r\n") - 1));

finalize_it:
	RETiRet;
}
/* end helpers for handling the recipient lists */

BEGINcreateInstance
CODESTARTcreateInstance
	pData->constSubject = NULL;
	pData->bEnableBody = 1;
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
CODESTARTfreeInstance
	free(pData->tplName);
	if(pData->iMode == 0) {
		free(pData->md.smtp.pszSrv);
		free(pData->md.smtp.pszSrvPort);
		free(pData->md.smtp.pszFrom);
		lstRcptDestruct(pData->md.smtp.lstRcpt);
	}
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	printf("mail"); /* TODO: extend! */
ENDdbgPrintInstInfo


/* TCP support code, should probably be moved to net.c or some place else... -- rgerhards, 2008-04-04 */

/* "receive" a character from the remote server. A single character
 * is returned. Returns RS_RET_NO_MORE_DATA if the server has closed
 * the connection and RS_RET_IO_ERROR if something goes wrong. This
 * is a blocking read.
 * rgerhards, 2008-04-04
 */
static rsRetVal
getRcvChar(wrkrInstanceData_t *pWrkrData, char *pC)
{
	DEFiRet;
	ssize_t lenBuf;

	if(pWrkrData->md.smtp.iRcvBuf == pWrkrData->md.smtp.lenRcvBuf) { /* buffer empty? */
		/* yes, we need to read the next server response */
		do {
			lenBuf = recv(pWrkrData->md.smtp.sock, pWrkrData->md.smtp.RcvBuf,
			              sizeof(pWrkrData->md.smtp.RcvBuf), 0);
			if(lenBuf == 0) {
				ABORT_FINALIZE(RS_RET_NO_MORE_DATA);
			} else if(lenBuf < 0) {
				if(errno != EAGAIN) {
					ABORT_FINALIZE(RS_RET_IO_ERROR);
				}
			} else {
				/* good read */
				pWrkrData->md.smtp.iRcvBuf = 0;
				pWrkrData->md.smtp.lenRcvBuf = lenBuf;
			}

		} while(lenBuf < 1);
	}

	/* when we reach this point, we have a non-empty buffer */
	*pC = pWrkrData->md.smtp.RcvBuf[pWrkrData->md.smtp.iRcvBuf++];

finalize_it:
	RETiRet;
}


/* close the mail server connection
 * rgerhards, 2008-04-08
 */
static rsRetVal
serverDisconnect(wrkrInstanceData_t *pWrkrData)
{
	DEFiRet;
	assert(pWrkrData != NULL);

	if(pWrkrData->md.smtp.sock != -1) {
		close(pWrkrData->md.smtp.sock);
		pWrkrData->md.smtp.sock = -1;
	}

	RETiRet;
}


/* open a connection to the mail server
 * rgerhards, 2008-04-04
 */
static rsRetVal
serverConnect(wrkrInstanceData_t *pWrkrData)
{
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	const char *smtpPort;
	const char *smtpSrv;
	char errStr[1024];
	instanceData *pData;
	DEFiRet;

	pData = pWrkrData->pData;

	if(pData->md.smtp.pszSrv == NULL)
		smtpSrv = "127.0.0.1";
	else
		smtpSrv = (char*)pData->md.smtp.pszSrv;

	if(pData->md.smtp.pszSrvPort == NULL)
		smtpPort = "25";
	else
		smtpPort = (char*)pData->md.smtp.pszSrvPort;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* TODO: make configurable! */
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(smtpSrv, smtpPort, &hints, &res) != 0) {
		DBGPRINTF("error %d in getaddrinfo\n", errno);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}
	
	if((pWrkrData->md.smtp.sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		DBGPRINTF("couldn't create send socket, reason %s", rs_strerror_r(errno, errStr, sizeof(errStr)));
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

	if(connect(pWrkrData->md.smtp.sock, res->ai_addr, res->ai_addrlen) != 0) {
		DBGPRINTF("create tcp connection failed, reason %s", rs_strerror_r(errno, errStr, sizeof(errStr)));
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

finalize_it:
	if(res != NULL)
		freeaddrinfo(res);

	if(iRet != RS_RET_OK) {
		if(pWrkrData->md.smtp.sock != -1) {
			close(pWrkrData->md.smtp.sock);
			pWrkrData->md.smtp.sock = -1;
		}
	}

	RETiRet;
}


/* send text to the server, blocking send */
static rsRetVal
Send(const int sock, const char *const __restrict__ msg, const size_t len)
{
	DEFiRet;
	size_t offsBuf = 0;
	ssize_t lenSend;

	assert(msg != NULL);

	if(len == 0) /* it's valid, but does not make much sense ;) */
		FINALIZE;

	do {
		lenSend = send(sock, msg + offsBuf, len - offsBuf, 0);
		if(lenSend == -1) {
			if(errno != EAGAIN) {
				DBGPRINTF("message not (smtp/tcp)send, errno %d", errno);
				ABORT_FINALIZE(RS_RET_TCP_SEND_ERROR);
			}
		} else if(lenSend != (ssize_t) (len - offsBuf)) {
			offsBuf += lenSend; /* on to next round... */
		} else {
			FINALIZE;
		}
	} while(1);

finalize_it:
	RETiRet;
}


/* send body text to the server, blocking send
 * The body is special in that we must escape a leading dot inside a line
 */
static rsRetVal
bodySend(wrkrInstanceData_t *pWrkrData, char *msg, size_t len)
{
	DEFiRet;
	char szBuf[2048];
	size_t iSrc;
	size_t iBuf = 0;
	int bHadCR = 0;
	int bInStartOfLine = 1;

	assert(pWrkrData != NULL);
	assert(msg != NULL);

	for(iSrc = 0 ; iSrc < len ; ++iSrc) {
		if(iBuf >= sizeof(szBuf) - 1) { /* one is reserved for our extra dot */
			CHKiRet(Send(pWrkrData->md.smtp.sock, szBuf, iBuf));
			iBuf = 0;
		}
		szBuf[iBuf++] = msg[iSrc];
		switch(msg[iSrc]) {
			case '\r':
				bHadCR = 1;
				break;
			case '\n':
				if(bHadCR)
					bInStartOfLine = 1;
				bHadCR = 0;
				break;
			case '.':
				if(bInStartOfLine)
					szBuf[iBuf++] = '.'; /* space is always reserved for this! */
				/*FALLTHROUGH*/
			default:
				bInStartOfLine = 0;
				bHadCR = 0;
				break;
		}
	}

	if(iBuf > 0) { /* incomplete buffer to send (the *usual* case)? */
		CHKiRet(Send(pWrkrData->md.smtp.sock, szBuf, iBuf));
	}

finalize_it:
	RETiRet;
}


/* read response line from server
 */
static rsRetVal
readResponseLn(wrkrInstanceData_t *pWrkrData, char *pLn, size_t lenLn, size_t *const __restrict__ respLen)
{
	DEFiRet;
	size_t i = 0;
	char c;
	
	assert(pWrkrData != NULL);
	assert(pLn != NULL);
	
	do {
		CHKiRet(getRcvChar(pWrkrData, &c));
		if(c == '\n')
			break;
		if(i < (lenLn - 1)) /* if line is too long, we simply discard the rest */
			pLn[i++] = c;
	} while(1);
	DBGPRINTF("smtp server response: %s\n", pLn);
	/* do not remove, this is helpful in troubleshooting SMTP probs! */

finalize_it:
	pLn[i] = '\0';
	*respLen = i;
	RETiRet;
}


/* read numerical response code from server and compare it to requried response code.
 * If they two don't match, return RS_RET_SMTP_ERROR.
 * rgerhards, 2008-04-07
 */
static rsRetVal
readResponse(wrkrInstanceData_t *pWrkrData, int *piState, int iExpected)
{
	DEFiRet;
	int bCont;
	char buf[128];
	size_t respLen;
	
	assert(pWrkrData != NULL);
	assert(piState != NULL);
	
	bCont = 1;
	do {
		CHKiRet(readResponseLn(pWrkrData, buf, sizeof(buf), &respLen));
		if(respLen < 4) /* we treat too-short responses as error */
			ABORT_FINALIZE(RS_RET_SMTP_ERROR);
		if(buf[3] != '-') { /* last or only response line? */
			bCont = 0;
			*piState = buf[0] - '0';
			*piState = *piState * 10 + buf[1] - '0';
			*piState = *piState * 10 + buf[2] - '0';
			if(*piState != iExpected)
				ABORT_FINALIZE(RS_RET_SMTP_ERROR);
		}
	} while(bCont);
	
finalize_it:
	RETiRet;
}


/* create a timestamp suitable for use with the Date: SMTP body header
 * rgerhards, 2008-04-08
 */
static void
mkSMTPTimestamp(uchar *pszBuf, size_t lenBuf)
{
	time_t tCurr;
	struct tm tmCurr;
	static const char szDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char szMonth[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"};

	datetime.GetTime(&tCurr);
	gmtime_r(&tCurr, &tmCurr);
	snprintf((char*)pszBuf, lenBuf, "Date: %s, %2d %s %4d %02d:%02d:%02d +0000\r\n", szDay[tmCurr.tm_wday],
	tmCurr.tm_mday,
		 szMonth[tmCurr.tm_mon], 1900 + tmCurr.tm_year, tmCurr.tm_hour, tmCurr.tm_min, tmCurr.tm_sec);
}


/* send a message via SMTP
 * rgerhards, 2008-04-04
 */
static rsRetVal
sendSMTP(wrkrInstanceData_t *pWrkrData, uchar *body, uchar *subject)
{
	DEFiRet;
	int iState; /* SMTP state */
	instanceData *pData;
	uchar szDateBuf[64];
	
	pData = pWrkrData->pData;

	CHKiRet(serverConnect(pWrkrData));
	CHKiRet(readResponse(pWrkrData, &iState, 220));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "HELO ", 5));
	CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)glbl.GetLocalHostName(),
		strlen((char*)glbl.GetLocalHostName())));
	CHKiRet(Send(pWrkrData->md.smtp.sock, "\r\n", sizeof("\r\n") - 1));
	CHKiRet(readResponse(pWrkrData, &iState, 250));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "MAIL FROM:<", sizeof("MAIL FROM:<") - 1));
	CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)pData->md.smtp.pszFrom, strlen((char*)pData->md.smtp.pszFrom)));
	CHKiRet(Send(pWrkrData->md.smtp.sock, ">\r\n", sizeof(">\r\n") - 1));
	CHKiRet(readResponse(pWrkrData, &iState, 250));

	CHKiRet(WriteRcpts(pWrkrData, (uchar*)"RCPT TO", sizeof("RCPT TO") - 1, 250));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "DATA\r\n",   sizeof("DATA\r\n") - 1));
	CHKiRet(readResponse(pWrkrData, &iState, 354));

	/* now come the data part */
	/* header */
	mkSMTPTimestamp(szDateBuf, sizeof(szDateBuf));
	CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)szDateBuf, strlen((char*)szDateBuf)));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "From: <", sizeof("From: <") - 1));
	CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)pData->md.smtp.pszFrom, strlen((char*)pData->md.smtp.pszFrom)));
	CHKiRet(Send(pWrkrData->md.smtp.sock, ">\r\n", sizeof(">\r\n") - 1));

	CHKiRet(WriteTos(pWrkrData, (uchar*)"To", sizeof("To") - 1));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "Subject: ",   sizeof("Subject: ") - 1));
	CHKiRet(Send(pWrkrData->md.smtp.sock, (char*)subject, strlen((char*)subject)));
	CHKiRet(Send(pWrkrData->md.smtp.sock, "\r\n", sizeof("\r\n") - 1));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "X-Mailer: rsyslog-ommail\r\n",
		sizeof("x-mailer: rsyslog-ommail\r\n") - 1));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "\r\n",   sizeof("\r\n") - 1)); /* indicate end of header */

	/* body */
	if(pData->bEnableBody)
		CHKiRet(bodySend(pWrkrData, (char*)body, strlen((char*) body)));

	/* end of data, back to envelope transaction */
	CHKiRet(Send(pWrkrData->md.smtp.sock, "\r\n.\r\n",   sizeof("\r\n.\r\n") - 1));
	CHKiRet(readResponse(pWrkrData, &iState, 250));

	CHKiRet(Send(pWrkrData->md.smtp.sock, "QUIT\r\n",   sizeof("QUIT\r\n") - 1));
	CHKiRet(readResponse(pWrkrData, &iState, 221));

	/* we are finished, a new connection is created for each request, so let's close it now */
	CHKiRet(serverDisconnect(pWrkrData));
	
finalize_it:
	RETiRet;
}


/* in tryResume we check if we can connect to the server in question. If that is OK,
 * we close the connection without doing any actual SMTP transaction. It will be
 * reopened during the actual send process. This may not be the best way to do it if
 * there is a problem inside the SMTP transaction. However, we can't find that out without
 * actually initiating something, and that would be bad. The logic here helps us
 * correctly recover from an unreachable/down mail server, which is probably the majority
 * of problem cases. For SMTP transaction problems, we will do lots of retries, but if it
 * is a temporary problem, it will be fixed anyhow. So I consider this implementation to
 * be clean enough, especially as I think other approaches have other weaknesses.
 * rgerhards, 2008-04-08
 */
BEGINtryResume
CODESTARTtryResume
	CHKiRet(serverConnect(pWrkrData));
	CHKiRet(serverDisconnect(pWrkrData)); /* if we fail, we will never reach this line */
finalize_it:
	if(iRet == RS_RET_IO_ERROR)
		iRet = RS_RET_SUSPENDED;
ENDtryResume


BEGINdoAction
	uchar *subject;
	const instanceData *const __restrict__ pData = pWrkrData->pData;
CODESTARTdoAction
	DBGPRINTF("ommail doAction()\n");

	if(pData->constSubject != NULL)
		subject = pData->constSubject;
	else if(pData->bHaveSubject)
		subject = ppString[1];
	else
		subject = (uchar*)"message from rsyslog";

	iRet = sendSMTP(pWrkrData, ppString[0], subject);
	if(iRet != RS_RET_OK) {
		DBGPRINTF("error sending mail, suspending\n");
		iRet = RS_RET_SUSPENDED;
	}
ENDdoAction



static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->tplName = NULL;
	pData->constSubject = NULL;
}


BEGINnewActInst
	struct cnfparamvals *pvals;
	uchar *tplSubject = NULL;
	int i, j;
CODESTARTnewActInst
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "server")) {
			pData->md.smtp.pszSrv = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "port")) {
			pData->md.smtp.pszSrvPort =  (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "mailfrom")) {
			pData->md.smtp.pszFrom = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "mailto")) {
			for(j = 0 ; j < pvals[i].val.d.ar->nmemb ; ++j) {
				addRcpt(&(pData->md.smtp.lstRcpt),
					(uchar*)es_str2cstr(pvals[i].val.d.ar->arr[j], NULL));
			}
		} else if(!strcmp(actpblk.descr[i].name, "subject.template")) {
			if(pData->constSubject != NULL) {
				parser_errmsg("ommail: only one of subject.template, subject.text "
					      "can be set");
				ABORT_FINALIZE(RS_RET_DUP_PARAM);
			}
			tplSubject = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "subject.text")) {
			if(tplSubject != NULL) {
				parser_errmsg("ommail: only one of subject.template, subject.text "
					      "can be set");
				ABORT_FINALIZE(RS_RET_DUP_PARAM);
			}
			pData->constSubject = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "body.enable")) {
			pData->bEnableBody =  (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			DBGPRINTF("ommail: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

	if(tplSubject == NULL) {
		/* if no subject is configured, we need just one template string */
		CODE_STD_STRING_REQUESTparseSelectorAct(1)
	} else {
		CODE_STD_STRING_REQUESTparseSelectorAct(2)
		pData->bHaveSubject = 1;
		/* NOTE: tplSubject memory is *handed over* down here below - do NOT free() */
		CHKiRet(OMSRsetEntry(*ppOMSR, 1, tplSubject, OMSR_NO_RQD_TPL_OPTS));
	}

	if(pData->tplName == NULL) {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*) strdup("RSYSLOG_FileFormat"),
			OMSR_NO_RQD_TPL_OPTS));
	} else {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0,
			(uchar*) strdup((char*) pData->tplName),
			OMSR_NO_RQD_TPL_OPTS));
	}
CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINparseSelectorAct
CODESTARTparseSelectorAct
	if(!strncmp((char*) p, ":ommail:", sizeof(":ommail:") - 1)) {
		p += sizeof(":ommail:") - 1; /* eat indicator sequence (-1 because of '\0'!) */
	} else {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	if((iRet = createInstance(&pData)) != RS_RET_OK)
		FINALIZE;

	/* TODO: check strdup() result */

	if(cs.pszFrom == NULL) {
		LogError(0, RS_RET_MAIL_NO_FROM, "no sender address given - specify $ActionMailFrom");
		ABORT_FINALIZE(RS_RET_MAIL_NO_FROM);
	}
	if(cs.lstRcpt == NULL) {
		LogError(0, RS_RET_MAIL_NO_TO, "no recipient address given - specify $ActionMailTo");
		ABORT_FINALIZE(RS_RET_MAIL_NO_TO);
	}

	pData->md.smtp.pszFrom = (uchar*) strdup((char*)cs.pszFrom);
	pData->md.smtp.lstRcpt = cs.lstRcpt; /* we "hand over" this memory */
	cs.lstRcpt = NULL; /* note: this is different from pre-3.21.2 versions! */

	if(cs.pszSubject == NULL) {
		/* if no subject is configured, we need just one template string */
		CODE_STD_STRING_REQUESTparseSelectorAct(1)
	} else {
		CODE_STD_STRING_REQUESTparseSelectorAct(2)
		pData->bHaveSubject = 1;
		CHKiRet(OMSRsetEntry(*ppOMSR, 1, (uchar*)strdup((char*) cs.pszSubject), OMSR_NO_RQD_TPL_OPTS));
	}
	if(cs.pszSrv != NULL)
		pData->md.smtp.pszSrv = (uchar*) strdup((char*)cs.pszSrv);
	if(cs.pszSrvPort != NULL)
		pData->md.smtp.pszSrvPort = (uchar*) strdup((char*)cs.pszSrvPort);
	pData->bEnableBody = cs.bEnableBody;

	/* process template */
	iRet = cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS, (uchar*) "RSYSLOG_FileFormat");
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


/* Free string config variables and reset them to NULL (not necessarily the default!) */
static rsRetVal freeConfigVariables(void)
{
	DEFiRet;

	free(cs.pszSrv);
	cs.pszSrv = NULL;
	free(cs.pszSrvPort);
	cs.pszSrvPort = NULL;
	free(cs.pszFrom);
	cs.pszFrom = NULL;
	lstRcptDestruct(cs.lstRcpt);
	cs.lstRcpt = NULL;
	
	RETiRet;
}


BEGINmodExit
CODESTARTmodExit
	/* cleanup our allocations */
	freeConfigVariables();

	/* release what we no longer need */
	objRelease(datetime, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
ENDqueryEtryPt


/* Reset config variables for this module to default values.
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	cs.bEnableBody = 1;
	iRet = freeConfigVariables();
	RETiRet;
}


BEGINmodInit()
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	/* tell which objects we need */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));

	DBGPRINTF("ommail version %s initializing\n", VERSION);

	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"actionmailsmtpserver", 0, eCmdHdlrGetWord, NULL, &cs.pszSrv,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"actionmailsmtpport", 0, eCmdHdlrGetWord, NULL, &cs.pszSrvPort,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"actionmailfrom", 0, eCmdHdlrGetWord, NULL, &cs.pszFrom,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"actionmailto", 0, eCmdHdlrGetWord, legacyConfAddRcpt, NULL,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"actionmailsubject", 0, eCmdHdlrGetWord, NULL, &cs.pszSubject,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"actionmailenablebody", 0, eCmdHdlrBinary, NULL, &cs.bEnableBody,
	STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(	(uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
	resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit
