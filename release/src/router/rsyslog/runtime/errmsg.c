/* The errmsg object.
 *
 * Module begun 2008-03-05 by Rainer Gerhards, based on some code
 * from syslogd.c. I converted this module to lgpl and have checked that
 * all contributors agreed to that step.
 * Now moving to ASL 2.0, and contributor checks tell that there is no need
 * to take further case, as the code now boils to be either my own or, a few lines,
 * of the original BSD-licenses sysklogd code. rgerhards, 2012-01-16
 *
 * Copyright 2008-2018 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
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
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "rsyslog.h"
#include "obj.h"
#include "msg.h"
#include "errmsg.h"
#include "srUtils.h"
#include "stringbuf.h"

/* static data */
DEFobjStaticHelpers

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

static int bHadErrMsgs; /* indicates if we had error messages since reset of this flag
			 * This is used to abort a run if the config is unclean.
			 */

static int fdOversizeMsgLog = -1;
static pthread_mutex_t oversizeMsgLogMut = PTHREAD_MUTEX_INITIALIZER;

/* ------------------------------ methods ------------------------------ */

/* Resets the error message flag. Must be done before processing config
 * files.
 */
void
resetErrMsgsFlag(void)
{
	bHadErrMsgs = 0;
}

int
hadErrMsgs(void)
{
	return bHadErrMsgs;
}

/* We now receive three parameters: one is the internal error code
 * which will also become the error message number, the second is
 * errno - if it is non-zero, the corresponding error message is included
 * in the text and finally the message text itself. Note that it is not
 * 100% clean to use the internal errcode, as it may be reached from
 * multiple actual error causes. However, it is much better than having
 * no error code at all (and in most cases, a single internal error code
 * maps to a specific error event).
 * rgerhards, 2008-06-27
 */
static void
doLogMsg(const int iErrno, const int iErrCode,  const int severity, const char *msg)
{
	char buf[2048];
	char errStr[1024];
	
	dbgprintf("Called LogMsg, msg: %s\n", msg);

	if(iErrno != 0) {
		rs_strerror_r(iErrno, errStr, sizeof(errStr));
		if(iErrCode == NO_ERRCODE || iErrCode == RS_RET_ERR) {
			snprintf(buf, sizeof(buf), "%s: %s [v%s]", msg, errStr, VERSION);
		} else {
			snprintf(buf, sizeof(buf), "%s: %s [v%s try http://www.rsyslog.com/e/%d ]",
				msg, errStr, VERSION, iErrCode * -1);
		}
	} else {
		if(iErrCode == NO_ERRCODE || iErrCode == RS_RET_ERR) {
			snprintf(buf, sizeof(buf), "%s [v%s]", msg, VERSION);
		} else {
			snprintf(buf, sizeof(buf), "%s [v%s try http://www.rsyslog.com/e/%d ]", msg,
				VERSION, iErrCode * -1);
		}
	}
	buf[sizeof(buf) - 1] = '\0'; /* just to be on the safe side... */
	errno = 0;
	
	const int msglen = (int) strlen(buf);
	if(msglen > glblGetMaxLine()) {
		/* in extreme cases, our error messages may be longer than the configured
		 * max message size. If so, we just truncate without further indication, as
		 * anything else would probably lead to a death loop on error messages.
		 * Note that we do not split, as we really do not anticipate there is
		 * much value in supporting extremely short max message sizes - we assume
		 * it's just a testbench thing. -- rgerhards, 2018-05-11
		 */
		 buf[glblGetMaxLine()] = '\0'; /* space must be available! */
	}

	glblErrLogger(severity, iErrCode, (uchar*)buf);

	if(severity == LOG_ERR)
		bHadErrMsgs = 1;
}

/* We now receive three parameters: one is the internal error code
 * which will also become the error message number, the second is
 * errno - if it is non-zero, the corresponding error message is included
 * in the text and finally the message text itself. Note that it is not
 * 100% clean to use the internal errcode, as it may be reached from
 * multiple actual error causes. However, it is much better than having
 * no error code at all (and in most cases, a single internal error code
 * maps to a specific error event).
 * rgerhards, 2008-06-27
 */
void __attribute__((format(printf, 3, 4)))
LogError(const int iErrno, const int iErrCode, const char *fmt, ... )
{
	va_list ap;
	char buf[2048];
	size_t lenBuf;
	
	va_start(ap, fmt);
	lenBuf = vsnprintf(buf, sizeof(buf), fmt, ap);
	if(lenBuf >= sizeof(buf)) {
		/* if our buffer was too small, we simply truncate. */
		lenBuf--;
	}
	va_end(ap);
	buf[sizeof(buf) - 1] = '\0'; /* just to be on the safe side... */
	
	doLogMsg(iErrno, iErrCode, LOG_ERR, buf);
}

/* We now receive three parameters: one is the internal error code
 * which will also become the error message number, the second is
 * errno - if it is non-zero, the corresponding error message is included
 * in the text and finally the message text itself. Note that it is not
 * 100% clean to use the internal errcode, as it may be reached from
 * multiple actual error causes. However, it is much better than having
 * no error code at all (and in most cases, a single internal error code
 * maps to a specific error event).
 * rgerhards, 2008-06-27
 */
void __attribute__((format(printf, 4, 5)))
LogMsg(const int iErrno, const int iErrCode, const int severity, const char *fmt, ... )
{
	va_list ap;
	char buf[2048];
	size_t lenBuf;
	
	va_start(ap, fmt);
	lenBuf = vsnprintf(buf, sizeof(buf), fmt, ap);
	if(lenBuf >= sizeof(buf)) {
		/* if our buffer was too small, we simply truncate. */
		lenBuf--;
	}
	va_end(ap);
	buf[sizeof(buf) - 1] = '\0'; /* just to be on the safe side... */
	
	doLogMsg(iErrno, iErrCode, severity, buf);
}


/* Write an oversize message to the oversize message error log.
 * We do NOT handle errors during writing that log other than emitting
 * yet another error message. The reason is that there really is nothing
 * else that we could do in that case.
 * rgerhards, 2018-05-03
 */
rsRetVal ATTR_NONNULL()
writeOversizeMessageLog(const smsg_t *const pMsg)
{
	struct json_object *json = NULL;
	char *rendered = NULL;
	struct json_object *jval;
	uchar *buf;
	size_t toWrite;
	ssize_t wrRet;
	int dummy;
	int mutexLocked = 0;
	DEFiRet;
	ISOBJ_TYPE_assert(pMsg, msg);

	if(glblGetOversizeMsgErrorFile() == NULL) {
		FINALIZE;
	}

	pthread_mutex_lock(&oversizeMsgLogMut);
	mutexLocked = 1;

	if(fdOversizeMsgLog == -1) {
		fdOversizeMsgLog = open((char*)glblGetOversizeMsgErrorFile(),
					O_WRONLY|O_CREAT|O_APPEND|O_LARGEFILE|O_CLOEXEC,
					S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
		if(fdOversizeMsgLog == -1) {
			LogError(errno, RS_RET_ERR, "error opening oversize message log file %s",
				glblGetOversizeMsgErrorFile());
			FINALIZE;
		}
	}

	assert(fdOversizeMsgLog != -1);
	json = json_object_new_object();
	if(json == NULL) {
		FINALIZE;
	}

	getRawMsg(pMsg, &buf, &dummy);
	jval = json_object_new_string((char*)buf);
	json_object_object_add(json, "rawmsg", jval);

	getInputName(pMsg, &buf, &dummy);
	jval = json_object_new_string((char*)buf);
	json_object_object_add(json, "input", jval);

	CHKmalloc(rendered = strdup((char*)fjson_object_to_json_string(json)));

	toWrite = strlen(rendered) + 1;
	/* Note: we overwrite the '\0' terminator with '\n' -- so we avoid
	 * calling malloc() -- write() does NOT need '\0'!
	 */
	rendered[toWrite-1] = '\n'; /* NO LONGER A STRING! */
	wrRet = write(fdOversizeMsgLog, rendered, toWrite);
	if(wrRet != (ssize_t) toWrite) {
		LogError(errno, RS_RET_IO_ERROR,
			"error writing oversize message log file %s, write returned %lld",
			glblGetOversizeMsgErrorFile(), (long long) wrRet);
	}

finalize_it:
	free(rendered);
	if(mutexLocked) {
		pthread_mutex_unlock(&oversizeMsgLogMut);
	}
	if(json != NULL) {
		fjson_object_put(json);
	}
	RETiRet;
}


void
errmsgDoHUP(void)
{
	pthread_mutex_lock(&oversizeMsgLogMut);
	close(fdOversizeMsgLog);
	fdOversizeMsgLog = -1;
	pthread_mutex_unlock(&oversizeMsgLogMut);
}


/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(errmsg)
CODESTARTobjQueryInterface(errmsg)
	if(pIf->ifVersion != errmsgCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->LogError = LogError;
	pIf->LogMsg = LogMsg;
finalize_it:
ENDobjQueryInterface(errmsg)


/* Initialize the errmsg class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINAbstractObjClassInit(errmsg, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */

	/* set our own handlers */
ENDObjClassInit(errmsg)

/* Exit the class.
 * rgerhards, 2008-04-17
 */
BEGINObjClassExit(errmsg, OBJ_IS_CORE_MODULE) /* class, version */
	/* release objects we no longer need */
	if(fdOversizeMsgLog != -1) {
		close(fdOversizeMsgLog);
	}
ENDObjClassExit(errmsg)
