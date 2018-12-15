/* tcpclt.c
 *
 * This is the implementation of TCP-based syslog clients (the counterpart
 * of the tcpsrv class).
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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "rsyslog.h"
#include "dirty.h"
#include "syslogd-types.h"
#include "net.h"
#include "tcpclt.h"
#include "module-template.h"
#include "srUtils.h"

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* static data */
DEFobjStaticHelpers

/* Initialize TCP sockets (for sender)
 */
static int
CreateSocket(struct addrinfo *addrDest)
{
	int fd;
	struct addrinfo *r;
	
	r = addrDest;

	while(r != NULL) {
		fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
		if (fd != -1) {
			/* We can not allow the TCP sender to block syslogd, at least
			 * not in a single-threaded design. That would cause rsyslogd to
			 * loose input messages - which obviously also would affect
			 * other selector lines, too. So we do set it to non-blocking and
			 * handle the situation ourselfs (by discarding messages). IF we run
			 * dual-threaded, however, the situation is different: in this case,
			 * the receivers and the selector line processing are only loosely
			 * coupled via a memory buffer. Now, I think, we can afford the extra
			 * wait time. Thus, we enable blocking mode for TCP if we compile with
			 * pthreads. -- rgerhards, 2005-10-25
			 * And now, we always run on multiple threads... -- rgerhards, 2007-12-20
			 */
			if (connect (fd, r->ai_addr, r->ai_addrlen) != 0) {
				if(errno == EINPROGRESS) {
					/* this is normal - will complete later select */
					return fd;
				} else {
					char errStr[1024];
					dbgprintf("create tcp connection failed, reason %s",
						rs_strerror_r(errno, errStr, sizeof(errStr)));
				}

			}
			else {
				return fd;
			}
			close(fd);
		}
		else {
			char errStr[1024];
			dbgprintf("couldn't create send socket, reason %s", rs_strerror_r(errno, errStr,
				sizeof(errStr)));
		}
		r = r->ai_next;
	}

	dbgprintf("no working socket could be obtained");

	return -1;
}



/* Build frame based on selected framing
 * This function was created by pulling code from TCPSend()
 * on 2007-12-27 by rgerhards. Older comments are still relevant.
 *
 * In order to support compressed messages via TCP, we must support an
 * octet-counting based framing (LF may be part of the compressed message).
 * We are now supporting the same mode that is available in IETF I-D
 * syslog-transport-tls-05 (current at the time of this writing). This also
 * eases things when we go ahead and implement that framing. I have now made
 * available two cases where this framing is used: either by explitely
 * specifying it in the config file or implicitely when sending a compressed
 * message. In the later case, compressed and uncompressed messages within
 * the same session have different framings. If it is explicitely set to
 * octet-counting, only this framing mode is used within the session.
 * rgerhards, 2006-12-07
 */
static rsRetVal
TCPSendBldFrame(tcpclt_t *pThis, char **pmsg, size_t *plen, int *pbMustBeFreed)
{
	DEFiRet;
	TCPFRAMINGMODE framingToUse;
	int bIsCompressed;
	size_t len;
	char *msg;
	char *buf = NULL;	/* if this is non-NULL, it MUST be freed before return! */

	assert(plen != NULL);
	assert(pbMustBeFreed != NULL);
	assert(pmsg != NULL);

	msg = *pmsg;
	len = *plen;
	bIsCompressed = *msg == 'z';	/* cache this, so that we can modify the message buffer */
	/* select framing for this record. If we have a compressed record, we always need to
	 * use octet counting because the data potentially contains all control characters
	 * including LF.
	 */
	framingToUse = bIsCompressed ? TCP_FRAMING_OCTET_COUNTING : pThis->tcp_framing;

	/* now check if we need to add a line terminator. We need to
	 * copy the string in memory in this case, this is probably
	 * quicker than using writev and definitely quicker than doing
	 * two socket calls.
	 * rgerhards 2005-07-22
	 *
	 * Some messages already contain a \n character at the end
	 * of the message. We append one only if we there is not
	 * already one. This seems the best fit, though this also
	 * means the message does not arrive unaltered at the final
	 * destination. But in the spirit of legacy syslog, this is
	 * probably the best to do...
	 * rgerhards 2005-07-20
	 */

	/* Build frame based on selected framing */
	if(framingToUse == TCP_FRAMING_OCTET_STUFFING) {
		if((*(msg+len-1) != pThis->tcp_framingDelimiter)) {
			/* in the malloc below, we need to add 2 to the length. The
			 * reason is that we a) add one character and b) len does
			 * not take care of the '\0' byte. Up until today, it was just
			 * +1 , which caused rsyslogd to sometimes dump core.
			 * I have added this comment so that the logic is not accidently
			 * changed again. rgerhards, 2005-10-25
			 */
			if((buf = MALLOC(len + 2)) == NULL) {
				/* extreme mem shortage, try to solve
				 * as good as we can. No point in calling
				 * any alarms, they might as well run out
				 * of memory (the risk is very high, so we
				 * do NOT risk that). If we have a message of
				 * more than 1 byte (what I guess), we simply
				 * overwrite the last character.
				 * rgerhards 2005-07-22
				 */
				if(len > 1) {
					*(msg+len-1) = pThis->tcp_framingDelimiter;
				} else {
					/* we simply can not do anything in
					 * this case (its an error anyhow...).
					 */
				}
			} else {
				/* we got memory, so we can copy the message */
				memcpy(buf, msg, len); /* do not copy '\0' */
				*(buf+len) = pThis->tcp_framingDelimiter;
				*(buf+len+1) = '\0';
				msg = buf; /* use new one */
				++len; /* care for the \n */
			}
		}
	} else {
		/* Octect-Counting
		 * In this case, we need to always allocate a buffer. This is because
		 * we need to put a header in front of the message text
		 */
		char szLenBuf[16];
		int iLenBuf;

		/* important: the printf-mask is "%d<sp>" because there must be a
		 * space after the len!
		 *//* The chairs of the IETF syslog-sec WG have announced that it is
		 * consensus to do the octet count on the SYSLOG-MSG part only. I am
		 * now changing the code to reflect this. Hopefully, it will not change
		 * once again (there can no compatibility layer programmed for this).
		 * To be on the save side, I just comment the code out. I mark these
		 * comments with "IETF20061218".
		 * rgerhards, 2006-12-19
		 */
		iLenBuf = snprintf(szLenBuf, sizeof(szLenBuf), "%d ", (int) len);
		/* IETF20061218 iLenBuf =
		  snprintf(szLenBuf, sizeof(szLenBuf), "%d ", len + iLenBuf);*/

		if((buf = MALLOC(len + iLenBuf)) == NULL) {
			/* we are out of memory. This is an extreme situation. We do not
			 * call any alarm handlers because they most likely run out of mem,
			 * too. We are brave enough to call debug output, though. Other than
			 * that, there is nothing left to do. We can not sent the message (as
			 * in case of the other framing, because the message is incomplete.
			 * We could, however, send two chunks (header and text separate), but
			 * that would cause a lot of complexity in the code. So we think it
			 * is appropriate enough to just make sure we do not crash in this
			 * very unlikely case. For this, it is justified just to loose
			 * the message. Rgerhards, 2006-12-07
			 */
			 dbgprintf("Error: out of memory when building TCP octet-counted "
				 "frame. Message is lost, trying to continue.\n");
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
		}

		 memcpy(buf, szLenBuf, iLenBuf); /* header */
		 memcpy(buf + iLenBuf, msg, len); /* message */
		 len += iLenBuf;	/* new message size */
		 msg = buf;	/* set message buffer */
	}

	/* frame building complete, on to actual sending */

	*plen = len;
	if(buf == NULL) {
		/* msg not modified */
		*pbMustBeFreed = 0;
	} else {
		*pmsg = msg;
		*pbMustBeFreed = 1;
	}

finalize_it:
	RETiRet;
}


/* Sends a TCP message. It is first checked if the
 * session is open and, if not, it is opened. Then the send
 * is tried. If it fails, one silent re-try is made. If the send
 * fails again, an error status (-1) is returned. If all goes well,
 * 0 is returned. The TCP session is NOT torn down.
 * For now, EAGAIN is ignored (causing message loss) - but it is
 * hard to do something intelligent in this case. With this
 * implementation here, we can not block and/or defer. Things are
 * probably a bit better when we move to liblogging. The alternative
 * would be to enhance the current select server with buffering and
 * write descriptors. This seems not justified, given the expected
 * short life span of this code (and the unlikeliness of this event).
 * rgerhards 2005-07-06
 * This function is now expected to stay. Libloging won't be used for
 * that purpose. I have added the param "len", because it is known by the
 * caller and so saves us some time. Also, it MUST be given because there
 * may be NULs inside msg so that we can not rely on strlen(). Please note
 * that the restrictions outlined above do not existin in multi-threaded
 * mode, which we assume will now be most often used. So there is no
 * real issue with the potential message loss in single-threaded builds.
 * rgerhards, 2006-11-30
 * I greatly restructured the function to be more generic and work
 * with function pointers. So it now can be used with any type of transport,
 * as long as it follows stream semantics. This was initially done to
 * support plain TCP and GSS via common code.
 */
static int
Send(tcpclt_t *pThis, void *pData, char *msg, size_t len)
{
	DEFiRet;
	int bDone = 0;
	int retry = 0;
	int bMsgMustBeFreed = 0;/* must msg be freed at end of function? 0 - no, 1 - yes */

	ISOBJ_TYPE_assert(pThis, tcpclt);
	assert(pData != NULL);
	assert(msg != NULL);
	assert(len > 0);

	CHKiRet(TCPSendBldFrame(pThis, &msg, &len, &bMsgMustBeFreed));

	if(pThis->iRebindInterval > 0  && ++pThis->iNumMsgs == pThis->iRebindInterval) {
		/* we need to rebind, and use the retry logic for this*/
		CHKiRet(pThis->prepRetryFunc(pData)); /* try to recover */
		pThis->iNumMsgs = 0;
	}

	while(!bDone) { /* loop is broken when send succeeds or error occurs */
		CHKiRet(pThis->initFunc(pData));
		iRet = pThis->sendFunc(pData, msg, len);

		if(iRet == RS_RET_OK || iRet == RS_RET_DEFER_COMMIT || iRet == RS_RET_PREVIOUS_COMMITTED) {
			/* we are done, we also use this as indication that the previous
			 * message was succesfully received (it's not always the case, but its at
			 * least our best shot at it -- rgerhards, 2008-03-12
			 * As of 2008-06-09, we have implemented an algorithm which detects connection
			 * loss quite good in some (common) scenarios. Thus, the probability of
			 * message duplication due to the code below has increased. We so now have
			 * a config setting, default off, that enables the user to request retransmits.
			 * However, if not requested, we do NOT need to do all the stuff needed for it.
			 */
			if(pThis->bResendLastOnRecon == 1) {
				if(pThis->prevMsg != NULL)
					free(pThis->prevMsg);
				/* if we can not alloc a new buffer, we silently ignore it. The worst that
				 * happens is that we lose our message recovery buffer - anything else would
				 * be worse, so don't try anything ;) -- rgerhards, 2008-03-12
				 */
				if((pThis->prevMsg = MALLOC(len)) != NULL) {
					memcpy(pThis->prevMsg, msg, len);
					pThis->lenPrevMsg = len;
				}
			}

			/* we are done with this record */
			bDone = 1;
		} else {
			if(retry == 0) { /* OK, one retry */
				++retry;
				CHKiRet(pThis->prepRetryFunc(pData)); /* try to recover */
				/* now try to send our stored previous message (which most probably
				 * didn't make it. Note that if bResendLastOnRecon is 0, prevMsg will
				 * never become non-NULL, so the check below covers all cases.
				 */
				if(pThis->prevMsg != NULL) {
					CHKiRet(pThis->initFunc(pData));
					CHKiRet(pThis->sendFunc(pData, pThis->prevMsg, pThis->lenPrevMsg));
				}
			} else {
				/* OK, max number of retries reached, nothing we can do */
				bDone = 1;
			}
		}
	}

finalize_it:
	if(bMsgMustBeFreed)
		free(msg);
	RETiRet;
}


/* set functions */
static rsRetVal
SetResendLastOnRecon(tcpclt_t *pThis, int bResendLastOnRecon)
{
	DEFiRet;
	pThis->bResendLastOnRecon = (short) bResendLastOnRecon;
	RETiRet;
}
static rsRetVal
SetSendInit(tcpclt_t *pThis, rsRetVal (*pCB)(void*))
{
	DEFiRet;
	pThis->initFunc = pCB;
	RETiRet;
}
static rsRetVal
SetSendPrepRetry(tcpclt_t *pThis, rsRetVal (*pCB)(void*))
{
	DEFiRet;
	pThis->prepRetryFunc = pCB;
	RETiRet;
}
static rsRetVal
SetSendFrame(tcpclt_t *pThis, rsRetVal (*pCB)(void*, char*, size_t))
{
	DEFiRet;
	pThis->sendFunc = pCB;
	RETiRet;
}
static rsRetVal
SetFraming(tcpclt_t *pThis, TCPFRAMINGMODE framing)
{
	DEFiRet;
	pThis->tcp_framing = framing;
	RETiRet;
}
static rsRetVal
SetFramingDelimiter(tcpclt_t *pThis, uchar tcp_framingDelimiter)
{
	DEFiRet;
	pThis->tcp_framingDelimiter = tcp_framingDelimiter;
	RETiRet;
}
static rsRetVal
SetRebindInterval(tcpclt_t *pThis, int iRebindInterval)
{
	DEFiRet;
	pThis->iRebindInterval = iRebindInterval;
	RETiRet;
}


/* Standard-Constructor
 */
BEGINobjConstruct(tcpclt) /* be sure to specify the object type also in END macro! */
	pThis->tcp_framingDelimiter = '\n';
ENDobjConstruct(tcpclt)


/* ConstructionFinalizer
 */
static rsRetVal
tcpcltConstructFinalize(tcpclt_t __attribute__((unused)) *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcpclt);

	RETiRet;
}


/* destructor for the tcpclt object */
BEGINobjDestruct(tcpclt) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(tcpclt)
	if(pThis->prevMsg != NULL)
		free(pThis->prevMsg);
ENDobjDestruct(tcpclt)


/* ------------------------------ handling the interface plumbing ------------------------------ */

/* queryInterface function
 * rgerhards, 2008-03-12
 */
BEGINobjQueryInterface(tcpclt)
CODESTARTobjQueryInterface(tcpclt)
	if(pIf->ifVersion != tcpcltCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = tcpcltConstruct;
	pIf->ConstructFinalize = tcpcltConstructFinalize;
	pIf->Destruct = tcpcltDestruct;

	pIf->CreateSocket = CreateSocket;
	pIf->Send = Send;

	/* set functions */
	pIf->SetResendLastOnRecon = SetResendLastOnRecon;
	pIf->SetSendInit = SetSendInit;
	pIf->SetSendFrame = SetSendFrame;
	pIf->SetSendPrepRetry = SetSendPrepRetry;
	pIf->SetFraming = SetFraming;
	pIf->SetFramingDelimiter = SetFramingDelimiter;
	pIf->SetRebindInterval = SetRebindInterval;

finalize_it:
ENDobjQueryInterface(tcpclt)


/* exit our class
 * rgerhards, 2008-03-10
 */
BEGINObjClassExit(tcpclt, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(tcpclt)
	/* release objects we no longer need */
ENDObjClassExit(tcpclt)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-29
 */
BEGINObjClassInit(tcpclt, 1, OBJ_IS_LOADABLE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	/* request objects we use */

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, tcpcltConstructFinalize);
ENDObjClassInit(tcpclt)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
	/* de-init in reverse order! */
	tcpcltClassExit();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(tcpcltClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
ENDmodInit


/*
 * vi:set ai:
 */
