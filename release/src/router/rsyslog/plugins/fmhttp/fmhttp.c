/* fmhttp.c
 * This is a function module for http functions.
 *
 * File begun on 2018-03-16 by JGerhards
 *
 * Copyright 2010-2018 Rainer Gerhards, Jan Gerhards and Adiscon GmbH.
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <curl/curl.h>
#include "errmsg.h"
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "msg.h"
#include "parserif.h"
#include "module-template.h"
#include "rainerscript.h"
#include "unicode-helper.h"

MODULE_TYPE_FUNCTION
MODULE_TYPE_NOKEEP
DEF_FMOD_STATIC_DATA

struct curl_funcData {
	const char *reply;
	size_t replyLen;
};

/* curl callback for doFunc_http_request */
static size_t
curlResult(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	char *buf;
	size_t newlen;
	struct cnffunc *const func = (struct cnffunc *) userdata;
	assert(func != NULL);
	struct curl_funcData *const curlData = (struct curl_funcData*) func->funcdata;
	assert(curlData != NULL);

	if(ptr == NULL) {
		LogError(0, RS_RET_ERR, "internal error: libcurl provided ptr=NULL");
		return 0;
	}

	newlen = curlData->replyLen + size*nmemb;
	if((buf = realloc((void*)curlData->reply, newlen + 1)) == NULL) {
		LogError(errno, RS_RET_ERR, "rainerscript: realloc failed in curlResult");
		return 0; /* abort due to failure */
	}
	memcpy(buf+curlData->replyLen, (char*)ptr, size*nmemb);
	curlData->replyLen = newlen;
	curlData->reply = buf;
	return size*nmemb;
}

static void ATTR_NONNULL()
doFunc_http_request(struct cnffunc *__restrict__ const func,
	struct svar *__restrict__ const ret,
	void *__restrict__ const usrptr,
	wti_t *__restrict__ const pWti)
{
	struct svar srcVal;
	int bMustFree;
	cnfexprEval(func->expr[0], &srcVal, usrptr, pWti);
	char *url = (char*) var2CString(&srcVal, &bMustFree);

	int resultSet = 0;
	CURL *handle = NULL;
	CURLcode res;
	assert(func != NULL);
	struct curl_funcData *const curlData = (struct curl_funcData*) func->funcdata;
	assert(curlData != NULL);
	rsRetVal iRet __attribute__((unused)) = RS_RET_OK;

	CHKmalloc(handle = curl_easy_init());
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, TRUE);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlResult);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, func);

	curl_easy_setopt(handle, CURLOPT_URL, url);
	res = curl_easy_perform(handle);
	if(res != CURLE_OK) {
		LogError(0, RS_RET_IO_ERROR,
			"rainerscript: http_request to failed, URL: '%s', error %s",
			url, curl_easy_strerror(res));
		ABORT_FINALIZE(RS_RET_OK);
	}


	CHKmalloc(ret->d.estr = es_newStrFromCStr(curlData->reply, curlData->replyLen));
	ret->datatype = 'S';
	resultSet = 1;

finalize_it:
	free((void*)curlData->reply);
	curlData->reply = NULL;
	curlData->replyLen = 0;

	if(handle != NULL) {
		curl_easy_cleanup(handle);
	}
	if(!resultSet) {
		/* provide dummy value */
		ret->d.n = 0;
		ret->datatype = 'N';
	}
	if(bMustFree) {
		free(url);
	}
	varFreeMembers(&srcVal);
}

static rsRetVal ATTR_NONNULL(1)
initFunc_http_request(struct cnffunc *const func)
{
	DEFiRet;

	func->destructable_funcdata = 1;
	CHKmalloc(func->funcdata = calloc(1, sizeof(struct curl_funcData)));
	if(func->nParams != 1) {
		parser_errmsg("rsyslog logic error in line %d of file %s\n",
			__LINE__, __FILE__);
		FINALIZE;
	}

finalize_it:
	RETiRet;
}

static void ATTR_NONNULL(1)
destructFunc_http_request(struct cnffunc *const func)
{
	if(func->funcdata != NULL) {
		free((void*) ((struct curl_funcData*)func->funcdata)->reply);
	}
}

static struct scriptFunct functions[] = {
	{"http_request", 1, 1, doFunc_http_request, initFunc_http_request, destructFunc_http_request},
	{NULL, 0, 0, NULL, NULL, NULL} //last element to check end of array
};

BEGINgetFunctArray
CODESTARTgetFunctArray
	*version = 1;
	*functArray = functions;
ENDgetFunctArray


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_FMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	dbgprintf("rsyslog fmhttp init called, compiled with version %s\n", VERSION);
ENDmodInit
