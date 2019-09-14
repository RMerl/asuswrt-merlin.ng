/* imsolaris.c
 * This input module is used to gather local log data under Solaris. This
 * includes messages from local applications AS WELL AS the kernel log.
 * I first considered to make all of this available via imklog, but that
 * did not lock appropriately on second thought. So I created this module
 * that does anything for local message recption.
 *
 * This module is not meant to be used on plaforms other than Solaris. As
 * such, trying to compile it elswhere will probably fail with all sorts
 * of errors.
 *
 * Some notes on the Solaris syslog mechanism:
 * Both system (kernel) and application log messages are provided via
 * a single message stream.
 *
 * Solaris checks if the syslogd is running. If so, syslog() emits messages
 * to the log socket, only. Otherwise, it emits messages to the console.
 * It is possible to gather these console messages as well. However, then
 * we clutter the console.
 * Solaris does this "syslogd alive check" in a somewhat unexpected way
 * (at least unexpected for me): it uses the so-called "door" mechanism, a
 * fast RPC facility. I first thought that the door API was used to submit
 * the actual syslog messages. But this is not the case. Instead, a door
 * call is done, and the server process inside rsyslog simply does NOTHING
 * but return. All that Solaris sylsogd() is interested in is if the door
 * server (we) responds and thus can be considered alive. The actual message
 * is then submitted via the usual stream. I have to admit I do not
 * understand why the message itself is not passed via this high-performance
 * API. But anyhow, that's nothing I can change, so the most important thing
 * is to note how Solaris does this thing ;)
 * The syslog() library call checks syslogd state for *each* call (what a
 * waste of time...) and decides each time if the message should go to the
 * console or not.  According to OpenSolaris sources, it looks like there is
 * message loss potential when the door file is created before all data has
 * been pulled from the stream. While I have to admit that I do not fully
 * understand that problem, I will follow the original code advise and do
 * one complete pull cycle on the log socket (until it has no further data
 * available) and only thereafter create the door file and start the "regular"
 * pull cycle. As of my understanding, there is a minimal race between the
 * point where the intial pull cycle has ended and the door file is created,
 * but that race is also present in OpenSolaris syslogd code, so it should
 * not matter that much (plus, I do not know how to avoid it...)
 *
 * File begun on 2010-04-15 by RGerhards
 *
 * Copyright 2010 Rainer Gerhards and Adiscon GmbH.
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stropts.h>
#include <sys/strlog.h>
#include <errno.h>
#include "dirty.h"
#include "cfsysline.h"
#include "unicode-helper.h"
#include "module-template.h"
#include "srUtils.h"
#include "errmsg.h"
#include "net.h"
#include "glbl.h"
#include "msg.h"
#include "prop.h"
#include "sun_cddl.h"

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imsolaris")

/* defines */
#define PATH_LOG	"/dev/log"


/* Module static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)


/* config settings */
struct modConfData_s {
	EMPTY_STRUCT;
};

static prop_t *pInputName = NULL;	/* our inputName currently is always "imuxsock", and this will hold it */
static char *LogName = NULL;	/* the log socket name TODO: make configurable! */


/* a function to replace the sun logerror() function.
 * It generates an error message from the supplied string. The main
 * reason for not calling logError directly is that sun_cddl.c does not
 * know or has acces to rsyslog objects (namely errmsg) -- and we do not
 * want to do this effort. -- rgerhards, 2010-04-19
 */
void
imsolaris_logerror(int err, char *errStr)
{
	LogError(err, RS_RET_ERR_DOOR, "%s", errStr);
}


/* we try to recover a failed file by closing and re-opening
 * it. We loop until the re-open works, but wait between each
 * failure. If the open succeeds, we assume all is well. If it is
 * not, we will run into the retry process with the next
 * iteration.
 * rgerhards, 2010-04-19
 */
static void
tryRecover(void)
{
	int tryNum = 1;
	int waitsecs;
	int waitusecs;
	rsRetVal iRet;

	close(sun_Pfd.fd);
	sun_Pfd.fd = -1;

	while(1) { /* loop broken inside */
		iRet = sun_openklog((LogName == NULL) ? PATH_LOG : LogName);
		if(iRet == RS_RET_OK) {
			if(tryNum > 1) {
				LogError(0, iRet, "failure on system log socket recovered.");
			}
			break;
		}
		/* failure, so sleep a bit. We wait try*10 ms, with a max of 15 seconds */
		if(tryNum == 1) {
			LogError(0, iRet, "failure on system log socket, trying to recover...");
		}
		waitusecs = tryNum * 10000;
		waitsecs = waitusecs / 1000000;
		DBGPRINTF("imsolaris: try %d to recover system log socket in %d.%d seconds\n",
			  tryNum, waitsecs, waitusecs);
		if(waitsecs > 15) {
			waitsecs = 15;
			waitusecs = 0;
		} else  {
			waitusecs = waitusecs % 1000000;
		}
		srSleep(waitsecs, waitusecs);
		++tryNum;
	}
}


/* This function receives data from a socket indicated to be ready
 * to receive and submits the message received for processing.
 * rgerhards, 2007-12-20
 * Interface changed so that this function is passed the array index
 * of the socket which is to be processed. This eases access to the
 * growing number of properties. -- rgerhards, 2008-08-01
 */
static rsRetVal
readLog(int fd, uchar *pRcv, int iMaxLine)
{
	DEFiRet;
	struct strbuf data;
	struct strbuf ctl;
	struct log_ctl hdr;
	int flags;
	smsg_t *pMsg;
	int ret;
	char errStr[1024];

	data.buf = (char*)pRcv;
	data.maxlen = iMaxLine;
	ctl.maxlen = sizeof (struct log_ctl);
	ctl.buf = (caddr_t)&hdr;
	flags = 0;
	ret = getmsg(fd, &ctl, &data, &flags);
	if(ret < 0) {
		if(errno == EINTR) {
			FINALIZE;
		} else 	{
			int en = errno;
			rs_strerror_r(errno, errStr, sizeof(errStr));
			DBGPRINTF("imsolaris: stream input error on fd %d: %s.\n", fd, errStr);
			LogError(en, NO_ERRCODE, "imsolaris: stream input error: %s", errStr);
			tryRecover();
		}
	} else {
		DBGPRINTF("imsolaris: message from log stream %d: %s\n", fd, pRcv);
		pRcv[data.len] = '\0'; /* make sure it is a valid C-String */
		CHKiRet(msgConstruct(&pMsg));
		MsgSetInputName(pMsg, pInputName);
		MsgSetRawMsg(pMsg, (char*)pRcv, strlen((char*)pRcv));
		MsgSetHOSTNAME(pMsg, glbl.GetLocalHostName(), ustrlen(glbl.GetLocalHostName()));
		msgSetPRI(pMsg, hdr.pri);
		pMsg->msgFlags = NEEDS_PARSING | NO_PRI_IN_RAW;
		CHKiRet(submitMsg(pMsg));
	}

finalize_it:
	RETiRet;
}


/* once the system is fully initialized, we wait for new messages.
 * We may think about replacing this with a read-loop, thus saving
 * us the overhead of the poll.
 * The timeout variable is the timeout to use for poll. During startup,
 * it should be set to 0 (non-blocking) and later to -1 (infinit, blocking).
 * This mimics the (strange) behaviour of the original syslogd.
 * rgerhards, 2010-04-19
 */
static rsRetVal
getMsgs(thrdInfo_t *pThrd, int timeout)
{
	DEFiRet;
	int nfds;
	int iMaxLine;
	uchar *pRcv = NULL; /* receive buffer */
	uchar bufRcv[4096+1];
	char errStr[1024];

	iMaxLine = glbl.GetMaxLine();

	/* we optimize performance: if iMaxLine is below 4K (which it is in almost all
	 * cases, we use a fixed buffer on the stack. Only if it is higher, heap memory
	 * is used. We could use alloca() to achive a similar aspect, but there are so
	 * many issues with alloca() that I do not want to take that route.
	 * rgerhards, 2008-09-02
	 */
	if((size_t) iMaxLine < sizeof(bufRcv) - 1) {
		pRcv = bufRcv;
	} else {
		CHKmalloc(pRcv = (uchar*) malloc(iMaxLine + 1));
	}

	 while(pThrd->bShallStop != RSTRUE) {
		DBGPRINTF("imsolaris: waiting for next message (timeout %d)...\n", timeout);
		if(timeout == 0) {
			nfds = poll(&sun_Pfd, 1, timeout); /* wait without timeout */

			if(pThrd->bShallStop == RSTRUE) {
				break;
			}

			if(nfds == 0) {
				if(timeout == 0) {
					DBGPRINTF("imsolaris: no more messages, getMsgs() terminates\n");
					FINALIZE;
				} else {
					continue;
				}
			}

			if(nfds < 0) {
				if(errno != EINTR) {
					int en = errno;
					rs_strerror_r(en, errStr, sizeof(errStr));
					DBGPRINTF("imsolaris: poll error: %d = %s.\n", errno, errStr);
					LogError(en, NO_ERRCODE, "imsolaris: poll error: %s",
							errStr);
				}
				continue;
			}
			if(sun_Pfd.revents & POLLIN) {
				readLog(sun_Pfd.fd, pRcv, iMaxLine);
			} else if(sun_Pfd.revents & (POLLNVAL|POLLHUP|POLLERR)) {
				tryRecover();
			}
		} else {
			/* if we have an infinite wait, we do not use poll at all
			 * I'd consider this a waste of time. However, I do not totally
			 * remove the code, as it may be useful if we decide at some
			 * point to provide a capability to support multiple input streams
			 * at once (this may be useful for a jail). In that case, the poll()
			 * loop would be needed, and so it doesn't make much sense to change
			 * the code to not support it. -- rgerhards, 2010-04-20
			 */
			readLog(sun_Pfd.fd, pRcv, iMaxLine);
		}

	}

finalize_it:
	if(pRcv != NULL && (size_t) iMaxLine >= sizeof(bufRcv) - 1)
		free(pRcv);

	RETiRet;
}


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


/* This function is called to gather input. */
BEGINrunInput
CODESTARTrunInput
	/* this is an endless loop - it is terminated when the thread is
	 * signalled to do so. This, however, is handled by the framework,
	 * right into the sleep below.
	 */

	DBGPRINTF("imsolaris: doing startup poll before openeing door()\n");
	CHKiRet(getMsgs(pThrd, 0));

	/* note: sun's syslogd code claims that the door should only
	 * be opened when the log stream has been polled. So file header
	 * comment of this file for more details.
	 */
	sun_open_door();
	DBGPRINTF("imsolaris: starting regular poll loop\n");
	iRet = getMsgs(pThrd, -1); /* this is the primary poll loop, infinite timeout */

	DBGPRINTF("imsolaris: terminating (bShallStop=%d)\n", pThrd->bShallStop);
finalize_it:
	RETiRet;
ENDrunInput


BEGINwillRun
CODESTARTwillRun
	/* we need to create the inputName property (only once during our lifetime) */
	CHKiRet(prop.Construct(&pInputName));
	CHKiRet(prop.SetString(pInputName, UCHAR_CONSTANT("imsolaris"), sizeof("imsolaris") - 1));
	CHKiRet(prop.ConstructFinalize(pInputName));

	iRet = sun_openklog((LogName == NULL) ? PATH_LOG : LogName);
	if(iRet != RS_RET_OK) {
		LogError(0, iRet, "error opening system log socket");
	}
finalize_it:
ENDwillRun


BEGINafterRun
CODESTARTafterRun
	/* do cleanup here */
	if(pInputName != NULL)
		prop.Destruct(&pInputName);
	free(LogName);
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	sun_delete_doorfiles();
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
ENDmodExit


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

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp,
				     void __attribute__((unused)) *pVal)
{
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	DBGPRINTF("imsolaris version %s initializing\n", PACKAGE_VERSION);

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"imsolarislogsocketname", 0, eCmdHdlrGetWord,
		NULL, &LogName, STD_LOADABLE_MODULE_ID));
ENDmodInit
/* vim:set ai:
 */
