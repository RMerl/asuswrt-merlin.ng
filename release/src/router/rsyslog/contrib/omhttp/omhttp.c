/* omhttp.c
 * This is an http output module based on omelasticsearch
 *
 * NOTE: read comments in module-template.h for more specifics!
 *
 * Copyright 2011 Nathan Scott.
 * Copyright 2009-2018 Rainer Gerhards and Adiscon GmbH.
 * Copyright 2018 Christian Tramnitz
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
#include <memory.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined(__FreeBSD__)
#include <unistd.h>
#endif
#include <json.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "unicode-helper.h"
#include "obj-types.h"
#include "ratelimit.h"
#include "ruleset.h"

#ifndef O_LARGEFILE
#  define O_LARGEFILE 0
#endif

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omhttp")

/* internal structures */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(errmsg)
DEFobjCurrIf(prop)
DEFobjCurrIf(ruleset)

static prop_t *pInputName = NULL;

#define WRKR_DATA_TYPE_ES 0xBADF0001

/* REST API uses this URL:
 * https://<hostName>:<restPort>/restPath
*/
typedef struct curl_slist HEADER;
typedef struct instanceConf_s {
	int defaultPort;
	int fdErrFile;		/* error file fd or -1 if not open */
	pthread_mutex_t mutErrFile;
	uchar **serverBaseUrls;
	int numServers;
	long healthCheckTimeout;
	uchar *uid;
	uchar *pwd;
	uchar *authBuf;
	uchar *httpheaderkey;
	uchar *httpheadervalue;
	uchar *headerBuf;
	uchar *restPath;
	uchar *checkPath;
	uchar *tplName;
	uchar *errorFile;
	sbool errorOnly;
	sbool bulkmode;
	sbool interleaved;
	sbool dynRestPath;
	size_t maxbytes;
	sbool useHttps;
	sbool allowUnsignedCerts;
	uchar *caCertFile;
	uchar *myCertFile;
	uchar *myPrivKeyFile;
	sbool retryFailures;
	int ratelimitInterval;
	int ratelimitBurst;
	/* for retries */
	ratelimit_t *ratelimiter;
	uchar *retryRulesetName;
	ruleset_t *retryRuleset;
	struct instanceConf_s *next;
} instanceData;

typedef instanceConf_t instanceData;

struct modConfData_s {
	rsconf_t *pConf;		/* our overall config object */
	instanceConf_t *root, *tail;
};
static modConfData_t *loadModConf = NULL;	/* modConf ptr to use for the current load process */

typedef struct wrkrInstanceData {
	PTR_ASSERT_DEF
	instanceData *pData;
	int serverIndex;
	int replyLen;
	char *reply;
	CURL	*curlCheckConnHandle;	/* libcurl session handle for checking the server connection */
	CURL	*curlPostHandle;	/* libcurl session handle for posting data to the server */
	HEADER	*curlHeader;	/* json POST request info */
	uchar *restURL;		/* last used URL for error reporting */
	struct {
		es_str_t *data;
		int nmemb;	/* number of messages in batch (for statistics counting) */
		uchar *currTpl1;
		uchar *currTpl2;
	} batch;
} wrkrInstanceData_t;

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "server", eCmdHdlrArray, 0 },
	{ "serverport", eCmdHdlrInt, 0 },
	{ "healthchecktimeout", eCmdHdlrInt, 0 },
	{ "httpheaderkey", eCmdHdlrGetWord, 0 },
	{ "httpheadervalue", eCmdHdlrGetWord, 0 },
	{ "uid", eCmdHdlrGetWord, 0 },
	{ "pwd", eCmdHdlrGetWord, 0 },
	{ "restpath", eCmdHdlrGetWord, 0 },
	{ "dynrestpath", eCmdHdlrBinary, 0 },
	{ "bulkmode", eCmdHdlrBinary, 0 },
	{ "maxbytes", eCmdHdlrSize, 0 },
	{ "usehttps", eCmdHdlrBinary, 0 },
	{ "errorfile", eCmdHdlrGetWord, 0 },
	{ "erroronly", eCmdHdlrBinary, 0 },
	{ "interleaved", eCmdHdlrBinary, 0 },
	{ "template", eCmdHdlrGetWord, 0 },
	{ "allowunsignedcerts", eCmdHdlrBinary, 0 },
	{ "tls.cacert", eCmdHdlrString, 0 },
	{ "tls.mycert", eCmdHdlrString, 0 },
	{ "tls.myprivkey", eCmdHdlrString, 0 },
	{ "retryfailures", eCmdHdlrBinary, 0 },
	{ "ratelimit.interval", eCmdHdlrInt, 0 },
	{ "ratelimit.burst", eCmdHdlrInt, 0 },
	{ "retryruleset", eCmdHdlrString, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

static rsRetVal curlSetup(wrkrInstanceData_t *pWrkrData);

BEGINcreateInstance
CODESTARTcreateInstance
	pData->fdErrFile = -1;
	pthread_mutex_init(&pData->mutErrFile, NULL);
	pData->caCertFile = NULL;
	pData->myCertFile = NULL;
	pData->myPrivKeyFile = NULL;
	pData->ratelimiter = NULL;
	pData->retryRulesetName = NULL;
	pData->retryRuleset = NULL;
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	PTR_ASSERT_SET_TYPE(pWrkrData, WRKR_DATA_TYPE_ES);
	pWrkrData->curlHeader = NULL;
	pWrkrData->curlPostHandle = NULL;
	pWrkrData->curlCheckConnHandle = NULL;
	pWrkrData->serverIndex = 0;
	pWrkrData->restURL = NULL;
	if(pData->bulkmode) {
		pWrkrData->batch.currTpl1 = NULL;
		pWrkrData->batch.currTpl2 = NULL;
		if((pWrkrData->batch.data = es_newStr(1024)) == NULL) {
			LogError(0, RS_RET_OUT_OF_MEMORY,
				"omhttp: error creating batch string "
			        "turned off bulk mode\n");
			pData->bulkmode = 0; /* at least it works */
		}
	}
	iRet = curlSetup(pWrkrData);
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature

BEGINfreeInstance
	int i;
CODESTARTfreeInstance
	if(pData->fdErrFile != -1)
		close(pData->fdErrFile);
	pthread_mutex_destroy(&pData->mutErrFile);
	for(i = 0 ; i < pData->numServers ; ++i)
		free(pData->serverBaseUrls[i]);
	free(pData->serverBaseUrls);
	free(pData->uid);
	free(pData->httpheaderkey);
	free(pData->httpheadervalue);
	free(pData->pwd);
	if (pData->authBuf != NULL)
		free(pData->authBuf);
	if (pData->headerBuf != NULL)
	free(pData->headerBuf);
	free(pData->restPath);
	free(pData->checkPath);
	free(pData->tplName);
	free(pData->errorFile);
	free(pData->caCertFile);
	free(pData->myCertFile);
	free(pData->myPrivKeyFile);
	free(pData->retryRulesetName);
	if (pData->ratelimiter != NULL)
		ratelimitDestruct(pData->ratelimiter);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	if(pWrkrData->curlHeader != NULL) {
		curl_slist_free_all(pWrkrData->curlHeader);
		pWrkrData->curlHeader = NULL;
	}
	if(pWrkrData->curlCheckConnHandle != NULL) {
		curl_easy_cleanup(pWrkrData->curlCheckConnHandle);
		pWrkrData->curlCheckConnHandle = NULL;
	}
	if(pWrkrData->curlPostHandle != NULL) {
		curl_easy_cleanup(pWrkrData->curlPostHandle);
		pWrkrData->curlPostHandle = NULL;
	}
	if (pWrkrData->restURL != NULL) {
		free(pWrkrData->restURL);
		pWrkrData->restURL = NULL;
	}
	es_deleteStr(pWrkrData->batch.data);
ENDfreeWrkrInstance

BEGINdbgPrintInstInfo
	int i;
CODESTARTdbgPrintInstInfo
	dbgprintf("omhttp\n");
	dbgprintf("\ttemplate='%s'\n", pData->tplName);
	dbgprintf("\tnumServers=%d\n", pData->numServers);
	dbgprintf("\thealthCheckTimeout=%lu\n", pData->healthCheckTimeout);
	dbgprintf("\tserverBaseUrls=");
	for(i = 0 ; i < pData->numServers ; ++i)
		dbgprintf("%c'%s'", i == 0 ? '[' : ' ', pData->serverBaseUrls[i]);
	dbgprintf("]\n");
	dbgprintf("\tdefaultPort=%d\n", pData->defaultPort);
	dbgprintf("\tuid='%s'\n", pData->uid == NULL ? (uchar*)"(not configured)" : pData->uid);
	dbgprintf("\thttpheaderkey='%s'\n", pData->httpheaderkey == NULL ?
		(uchar*)"(not configured)" : pData->httpheaderkey);
	dbgprintf("\thttpheadervalue='%s'\n", pData->httpheadervalue == NULL ?
		(uchar*)"(not configured)" : pData->httpheadervalue);
	dbgprintf("\tpwd=(%sconfigured)\n", pData->pwd == NULL ? "not " : "");
	dbgprintf("\trest path='%s'\n", pData->restPath);
	dbgprintf("\tcheck path='%s'\n", pData->checkPath);
	dbgprintf("\tdynamic rest path=%d\n", pData->dynRestPath);
	dbgprintf("\tuse https=%d\n", pData->useHttps);
	dbgprintf("\tmaxbytes=%zu\n", pData->maxbytes);
	dbgprintf("\tallowUnsignedCerts=%d\n", pData->allowUnsignedCerts);
	dbgprintf("\terrorfile='%s'\n", pData->errorFile == NULL ?
		(uchar*)"(not configured)" : pData->errorFile);
	dbgprintf("\terroronly=%d\n", pData->errorOnly);
	dbgprintf("\tinterleaved=%d\n", pData->interleaved);
	dbgprintf("\ttls.cacert='%s'\n", pData->caCertFile);
	dbgprintf("\ttls.mycert='%s'\n", pData->myCertFile);
	dbgprintf("\ttls.myprivkey='%s'\n", pData->myPrivKeyFile);
	dbgprintf("\tretryfailures='%d'\n", pData->retryFailures);
	dbgprintf("\tratelimit.interval='%d'\n", pData->ratelimitInterval);
	dbgprintf("\tratelimit.burst='%d'\n", pData->ratelimitBurst);
ENDdbgPrintInstInfo


/* http POST result string ... useful for debugging */
static size_t
curlResult(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	char *p = (char *)ptr;
	wrkrInstanceData_t *pWrkrData = (wrkrInstanceData_t*) userdata;
	char *buf;
	size_t newlen;
	PTR_ASSERT_CHK(pWrkrData, WRKR_DATA_TYPE_ES);
	newlen = pWrkrData->replyLen + size*nmemb;
	if((buf = realloc(pWrkrData->reply, newlen + 1)) == NULL) {
		LogError(errno, RS_RET_ERR, "omhttp: realloc failed in curlResult");
		return 0; /* abort due to failure */
	}
	memcpy(buf+pWrkrData->replyLen, p, size*nmemb);
	pWrkrData->replyLen = newlen;
	pWrkrData->reply = buf;
	return size*nmemb;
}

/* Build basic URL part, which includes hostname and port as follows:
 * http://hostname:port/ based on a server param
 * Newly creates a cstr for this purpose.
 * Note: serverParam MUST NOT end in '/' (caller must strip if it exists)
 */
static rsRetVal
computeBaseUrl(const char*const serverParam,
	const int defaultPort,
	const sbool useHttps,
	uchar **baseUrl)
{
#	define SCHEME_HTTPS "https://"
#	define SCHEME_HTTP "http://"

	char portBuf[64];
	int r = 0;
	const char *host = serverParam;
	DEFiRet;

	assert(serverParam[strlen(serverParam)-1] != '/');

	es_str_t *urlBuf = es_newStr(256);
	if (urlBuf == NULL) {
		LogError(0, RS_RET_OUT_OF_MEMORY,
		"omhttp: failed to allocate es_str urlBuf in computeBaseUrl");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	/* Find where the hostname/ip of the server starts. If the scheme is not specified
	  in the uri, start the buffer with a scheme corresponding to the useHttps parameter.
	*/
	if (strcasestr(serverParam, SCHEME_HTTP))
		host = serverParam + strlen(SCHEME_HTTP);
	else if (strcasestr(serverParam, SCHEME_HTTPS))
		host = serverParam + strlen(SCHEME_HTTPS);
	else
		r = useHttps ? es_addBuf(&urlBuf, SCHEME_HTTPS, sizeof(SCHEME_HTTPS)-1) :
			es_addBuf(&urlBuf, SCHEME_HTTP, sizeof(SCHEME_HTTP)-1);

	if (r == 0) r = es_addBuf(&urlBuf, (char *)serverParam, strlen(serverParam));
	if (r == 0 && !strchr(host, ':')) {
		snprintf(portBuf, sizeof(portBuf), ":%d", defaultPort);
		r = es_addBuf(&urlBuf, portBuf, strlen(portBuf));
	}
	if (r == 0) r = es_addChar(&urlBuf, '/');
	if (r == 0) *baseUrl = (uchar*) es_str2cstr(urlBuf, NULL);

	if (r != 0 || baseUrl == NULL) {
		LogError(0, RS_RET_ERR,
			"omhttp: error occurred computing baseUrl from server %s", serverParam);
		ABORT_FINALIZE(RS_RET_ERR);
	}
finalize_it:
	if (urlBuf) {
		es_deleteStr(urlBuf);
	}
	RETiRet;
}

static inline void
incrementServerIndex(wrkrInstanceData_t *pWrkrData)
{
	pWrkrData->serverIndex = (pWrkrData->serverIndex + 1) % pWrkrData->pData->numServers;
}


/* checks if connection to ES can be established; also iterates over
 * potential servers to support high availability (HA) feature. If it
 * needs to switch server, will record new one in curl handle.
 */
static rsRetVal ATTR_NONNULL()
checkConn(wrkrInstanceData_t *const pWrkrData)
{
	CURL *curl;
	CURLcode res;
	es_str_t *urlBuf;
	char* healthUrl;
	char* serverUrl;
	char* checkPath;
	int i;
	int r;
	DEFiRet;

	pWrkrData->reply = NULL;
	pWrkrData->replyLen = 0;
	curl = pWrkrData->curlCheckConnHandle;
	urlBuf = es_newStr(256);
	if (urlBuf == NULL) {
		LogError(0, RS_RET_OUT_OF_MEMORY,
			"omhttp: unable to allocate buffer for health check uri.");
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	for(i = 0; i < pWrkrData->pData->numServers; ++i) {
		serverUrl = (char*) pWrkrData->pData->serverBaseUrls[pWrkrData->serverIndex];
		checkPath = (char*) pWrkrData->pData->checkPath;

		es_emptyStr(urlBuf);
		r = es_addBuf(&urlBuf, serverUrl, strlen(serverUrl));
		if(r == 0)
			r = es_addBuf(&urlBuf, checkPath, sizeof(checkPath)-1);
		if(r == 0)
			healthUrl = es_str2cstr(urlBuf, NULL);
		if(r != 0 || healthUrl == NULL) {
			LogError(0, RS_RET_OUT_OF_MEMORY,
				"omhttp: unable to allocate buffer for health check uri.");
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}

		curl_easy_setopt(curl, CURLOPT_URL, healthUrl);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlResult);
		res = curl_easy_perform(curl);
		free(healthUrl);

		if (res == CURLE_OK) {
			DBGPRINTF("omhttp: checkConn %s completed with success "
				"on attempt %d\n", serverUrl, i);
			ABORT_FINALIZE(RS_RET_OK);
		}

		DBGPRINTF("omhttp: checkConn %s failed on attempt %d: %s\n",
			serverUrl, i, curl_easy_strerror(res));
		incrementServerIndex(pWrkrData);
	}

	LogMsg(0, RS_RET_SUSPENDED, LOG_WARNING,
		"omhttp: checkConn failed after %d attempts.", i);
	ABORT_FINALIZE(RS_RET_SUSPENDED);

finalize_it:
	if(urlBuf != NULL)
		es_deleteStr(urlBuf);
	free(pWrkrData->reply);
	pWrkrData->reply = NULL; /* don't leave dangling pointer */
	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	DBGPRINTF("omhttp: tryResume called\n");
	iRet = checkConn(pWrkrData);
ENDtryResume


/* get the current index and type for this message */
static void ATTR_NONNULL(1)
getRestPath(const instanceData *const pData, uchar **const tpls,
		      uchar **const restPath)
{
	*restPath = pData->restPath;
	if(tpls == NULL) {
		goto done;
	}

	int iNumTpls = 1;
	if(pData->dynRestPath) {
		*restPath = tpls[iNumTpls];
		++iNumTpls;
	}

done:
	assert(restPath != NULL);
	return;
}


static rsRetVal ATTR_NONNULL(1)
setPostURL(wrkrInstanceData_t *const pWrkrData, uchar **const tpls)
{
	uchar *restPath;
	char* baseUrl;
	es_str_t *url;
	int r;
	DEFiRet;
	instanceData *const pData = pWrkrData->pData;

	baseUrl = (char*)pData->serverBaseUrls[pWrkrData->serverIndex];
	url = es_newStrFromCStr(baseUrl, strlen(baseUrl));
	if (url == NULL) {
		LogError(0, RS_RET_OUT_OF_MEMORY,
			"omhttp: error allocating new estr for POST url.");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	getRestPath(pData, tpls, &restPath);
	r = es_addBuf(&url, (char*)restPath, ustrlen(restPath));
	if(r != 0) {
		LogError(0, RS_RET_ERR, "omhttp: failure in creating restURL, "
				"error code: %d", r);
		ABORT_FINALIZE(RS_RET_ERR);
	}

	if(pWrkrData->restURL != NULL)
		free(pWrkrData->restURL);

	pWrkrData->restURL = (uchar*)es_str2cstr(url, NULL);
	curl_easy_setopt(pWrkrData->curlPostHandle, CURLOPT_URL, pWrkrData->restURL);
	DBGPRINTF("omhttp: using REST URL: '%s'\n", pWrkrData->restURL);

finalize_it:
	if (url != NULL)
		es_deleteStr(url);
	RETiRet;
}


/* this method does not directly submit but builds a batch instead. It
 * may submit, if we have dynamic restPath and the current restPath changes.
 */
static rsRetVal
buildBatch(wrkrInstanceData_t *pWrkrData, uchar *message)
{
	int length = strlen((char *)message);
	int r=0;
	DEFiRet;

	if(r == 0) r = es_addBuf(&pWrkrData->batch.data, (char*)message, length);
	if(r == 0) r = es_addBuf(&pWrkrData->batch.data, "\n", sizeof("\n")-1);
	if(r != 0) {
		LogError(0, RS_RET_ERR,
			"omhttp: growing batch failed with code %d", r);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	++pWrkrData->batch.nmemb;
	iRet = RS_RET_OK;

finalize_it:
	RETiRet;
}

/*
 * Dumps entire bulk request and response in error log
 */
static rsRetVal
getDataErrorDefault(wrkrInstanceData_t *pWrkrData,fjson_object **pReplyRoot,uchar *reqmsg,char **rendered)
{
	DEFiRet;
	fjson_object *req=NULL;
	fjson_object *errRoot=NULL;
	fjson_object *replyRoot = *pReplyRoot;

	if((req=fjson_object_new_object()) == NULL) ABORT_FINALIZE(RS_RET_ERR);
	fjson_object_object_add(req, "url", fjson_object_new_string((char*)pWrkrData->restURL));
	fjson_object_object_add(req, "postdata", fjson_object_new_string((char*)reqmsg));

	if((errRoot=fjson_object_new_object()) == NULL) ABORT_FINALIZE(RS_RET_ERR);
	fjson_object_object_add(errRoot, "request", req);
	fjson_object_object_add(errRoot, "reply", replyRoot);
	*rendered = strdup((char*)fjson_object_to_json_string(errRoot));

	req=NULL;
	fjson_object_put(errRoot);

	*pReplyRoot = NULL; /* tell caller not to delete once again! */

	finalize_it:
		fjson_object_put(req);
		RETiRet;
}

/*
 * Sets bulkRequestNextSectionStart pointer to next sections start in the buffer pointed by bulkRequest.
 * Sections are marked by { and }
 */
static rsRetVal
getSection(const char* bulkRequest, const char **bulkRequestNextSectionStart )
{
		DEFiRet;
		char* index =0;
		if( (index = strchr(bulkRequest,'\n')) != 0)/*intermediate section*/
		{
			*bulkRequestNextSectionStart = ++index;
		}
		else
		{
			*bulkRequestNextSectionStart=0;
			ABORT_FINALIZE(RS_RET_ERR);
		}

	     finalize_it:
	     	  RETiRet;
}

/*
 * Sets the new string in singleRequest for one request in bulkRequest
 * and sets lastLocation pointer to the location till which bulkrequest has been parsed.
 * (used as input to make function thread safe.)
 */
static rsRetVal
getSingleRequest(const char* bulkRequest, char** singleRequest, const char **lastLocation)
{
	DEFiRet;
	const char *req = bulkRequest;
	const char *start = bulkRequest;
	if (getSection(req,&req)!=RS_RET_OK)
		ABORT_FINALIZE(RS_RET_ERR);

	if (getSection(req,&req)!=RS_RET_OK)
			ABORT_FINALIZE(RS_RET_ERR);

	CHKmalloc(*singleRequest = (char*) calloc (req - start+ 1 + 1,1));
	/* (req - start+ 1 == length of data + 1 for terminal char)*/
	memcpy(*singleRequest,start,req - start);
	*lastLocation=req;

finalize_it:
	RETiRet;
}

/*
 * check the status of response from API
 */
static int checkReplyStatus(fjson_object* ok) {
	return (ok == NULL || !fjson_object_is_type(ok, fjson_type_int) || fjson_object_get_int(ok) < 0 ||
		fjson_object_get_int(ok) > 299);
}

/*
 * Context object for error file content creation or status check
 * response_item - the full {"create":{"_index":"idxname",.....}}
 * response_body - the inner hash of the response_item - {"_index":"idxname",...}
 * status - the "status" field from the inner hash - "status":500
 *          should be able to use fjson_object_get_int(status) to get the http result code
 */
typedef struct exeContext{
	int statusCheckOnly;
	fjson_object *errRoot;
	rsRetVal (*prepareErrorFileContent)(struct exeContext *ctx,int itemStatus,char *request,char *response,
			fjson_object *response_item, fjson_object *response_body, fjson_object *status);
	ratelimit_t *ratelimiter;
	ruleset_t *retryRuleset;
	struct json_tokener *jTokener;
} context;

/*
 * get content to be written in error file using context passed
 */
static rsRetVal
parseRequestAndResponseForContext(wrkrInstanceData_t *pWrkrData,fjson_object **pReplyRoot,uchar *reqmsg,context *ctx)
{
	DEFiRet;
	fjson_object *replyRoot = *pReplyRoot;
	int i;
	int numitems = 0;
	fjson_object *items=NULL, *jo_errors = NULL;
	int errors = 0;

	if(fjson_object_object_get_ex(replyRoot, "errors", &jo_errors)) {
		errors = fjson_object_get_boolean(jo_errors);
		if (!errors && pWrkrData->pData->retryFailures) {
			return RS_RET_OK;
		}
	}
	/*iterate over items*/
	if(!fjson_object_object_get_ex(replyRoot, "items", &items)) {
		DBGPRINTF("omhttp: no items found in response\n");
	} else {
		numitems = fjson_object_array_length(items);
	}

	if (reqmsg) {
		DBGPRINTF("omhttp: Entire request %s\n", reqmsg);
	} else {
		DBGPRINTF("omhttp: Empty request\n");
	}
	const char *lastReqRead= (char*)reqmsg;

	DBGPRINTF("omhttp: %d items in reply\n", numitems);
	for(i = 0 ; i < numitems ; ++i) {

		fjson_object *item=NULL;
		fjson_object *result=NULL;
		fjson_object *ok=NULL;
		int itemStatus=0;
		item = fjson_object_array_get_idx(items, i);
		if(item == NULL)  {
			LogError(0, RS_RET_DATAFAIL,
				"omhttp: error in http reply: "
				"cannot obtain reply array item %d", i);
			ABORT_FINALIZE(RS_RET_DATAFAIL);
		}

		fjson_object_object_get_ex(result, "status", &ok);
		if(ok == NULL) {
			DBGPRINTF("omhttp: no status response received\n");
		} else {
			itemStatus = checkReplyStatus(ok);
		}

		char *request =0;
		char *response =0;
		if(ctx->statusCheckOnly || (NULL == lastReqRead)) {
			if(itemStatus) {
				DBGPRINTF("omhttp: error in http reply: item %d, "
					"status is %d\n", i, fjson_object_get_int(ok));
				DBGPRINTF("omhttp: status check found error.\n");
				ABORT_FINALIZE(RS_RET_DATAFAIL);
			}

		} else {
			if(getSingleRequest(lastReqRead,&request,&lastReqRead) != RS_RET_OK) {
				DBGPRINTF("omhttp: Couldn't get post request\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}
			response = (char*)fjson_object_to_json_string_ext(result, FJSON_TO_STRING_PLAIN);

			if(response==NULL) {
				free(request);/*as its has been assigned.*/
				DBGPRINTF("omhttp: Error getting fjson_object_to_string_ext. Cannot "
					"continue\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}

			/*call the context*/
			rsRetVal ret = ctx->prepareErrorFileContent(ctx, itemStatus, request,
					response, item, result, ok);

			/*free memory in any case*/
			free(request);

			if(ret != RS_RET_OK) {
				DBGPRINTF("omhttp: Error in preparing errorfileContent. Cannot continue\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}
		}
	}

finalize_it:
	RETiRet;
}

/*
 * Dumps only failed requests of bulk insert
 */
static rsRetVal
getDataErrorOnly(context *ctx,int itemStatus,char *request,char *response,
		fjson_object *response_item, fjson_object *response_body, fjson_object *status)
{
	DEFiRet;
	(void)response_item; /* unused */
	(void)response_body; /* unused */
	(void)status; /* unused */
	if(itemStatus) {
		fjson_object *onlyErrorResponses =NULL;
		fjson_object *onlyErrorRequests=NULL;

		if(!fjson_object_object_get_ex(ctx->errRoot, "reply", &onlyErrorResponses)) {
			DBGPRINTF("omhttp: Failed to get reply json array. Invalid context. Cannot "
				"continue\n");
			ABORT_FINALIZE(RS_RET_ERR);
		}
		fjson_object_array_add(onlyErrorResponses, fjson_object_new_string(response));

		if(!fjson_object_object_get_ex(ctx->errRoot, "request", &onlyErrorRequests)) {
			DBGPRINTF("omhttp: Failed to get request json array. Invalid context. Cannot "
				"continue\n");
			ABORT_FINALIZE(RS_RET_ERR);
		}

		fjson_object_array_add(onlyErrorRequests, fjson_object_new_string(request));

	}

finalize_it:
	RETiRet;
}

/*
 * Dumps all requests of bulk insert interleaved with request and response
 */

static rsRetVal
getDataInterleaved(context *ctx,
	int __attribute__((unused)) itemStatus,
	char *request,
	char *response,
	fjson_object *response_item,
	fjson_object *response_body,
	fjson_object *status
)
{
	DEFiRet;
	(void)response_item; /* unused */
	(void)response_body; /* unused */
	(void)status; /* unused */
	fjson_object *interleaved =NULL;
	if(!fjson_object_object_get_ex(ctx->errRoot, "response", &interleaved)) {
		DBGPRINTF("omhttp: Failed to get response json array. Invalid context. Cannot continue\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	fjson_object *interleavedNode=NULL;
	/*create interleaved node that has req and response json data*/
	if((interleavedNode=fjson_object_new_object()) == NULL)
	{
		DBGPRINTF("omhttp: Failed to create interleaved node. Cann't continue\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	fjson_object_object_add(interleavedNode,"request", fjson_object_new_string(request));
	fjson_object_object_add(interleavedNode,"reply", fjson_object_new_string(response));

	fjson_object_array_add(interleaved, interleavedNode);



	finalize_it:
		RETiRet;
}


/*
 * Dumps only failed requests of bulk insert interleaved with request and response
 */

static rsRetVal
getDataErrorOnlyInterleaved(context *ctx,int itemStatus,char *request,char *response,
		fjson_object *response_item, fjson_object *response_body, fjson_object *status)
{
	DEFiRet;
	if (itemStatus) {
		if(getDataInterleaved(ctx, itemStatus,request,response,
				response_item, response_body, status)!= RS_RET_OK) {
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

	finalize_it:
		RETiRet;
}

/* request string looks like this:
 * "{\"create\":{\"_index\": \"rsyslog_testbench\",\"_type\":\"test-type\",
 *   \"_id\":\"FAEAFC0D17C847DA8BD6F47BC5B3800A\"}}\n
 * {\"msgnum\":\"x00000000\",\"viaq_msg_id\":\"FAEAFC0D17C847DA8BD6F47BC5B3800A\"}\n"
 * we don't want the meta header, only the data part
 * start = first \n + 1
 * end = last \n
 */
static rsRetVal
createMsgFromRequest(const char *request, context *ctx, smsg_t **msg)
{
	DEFiRet;
	fjson_object *jo_msg = NULL;
	const char *datastart, *dataend;
	size_t datalen;
	enum json_tokener_error json_error;

	*msg = NULL;
	if (!(datastart = strchr(request, '\n')) || (datastart[1] != '{')) {
		LogError(0, RS_RET_ERR,
			"omhttp: malformed original request - "
			"could not find start of original data [%s]",
			request);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	datastart++; /* advance to { */
	if (!(dataend = strchr(datastart, '\n')) || (dataend[1] != '\0')) {
		LogError(0, RS_RET_ERR,
			"omhttp: malformed original request - "
			"could not find end of original data [%s]",
			request);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	datalen = dataend - datastart;
	json_tokener_reset(ctx->jTokener);
	fjson_object *jo_request = json_tokener_parse_ex(ctx->jTokener, datastart, datalen);
	json_error = fjson_tokener_get_error(ctx->jTokener);
	if (!jo_request || (json_error != fjson_tokener_success)) {
		LogError(0, RS_RET_ERR,
			"omhttp: parse error [%s] - could not convert original "
			"request JSON back into JSON object [%s]",
			fjson_tokener_error_desc(json_error), request);
		ABORT_FINALIZE(RS_RET_ERR);
	}

	CHKiRet(msgConstruct(msg));
	MsgSetFlowControlType(*msg, eFLOWCTL_FULL_DELAY);
	MsgSetInputName(*msg, pInputName);
	if (fjson_object_object_get_ex(jo_request, "message", &jo_msg)) {
		const char *rawmsg = json_object_get_string(jo_msg);
		const size_t msgLen = (size_t)json_object_get_string_len(jo_msg);
		MsgSetRawMsg(*msg, rawmsg, msgLen);
	} else {
		MsgSetRawMsg(*msg, request, strlen(request));
	}
	MsgSetMSGoffs(*msg, 0);	/* we do not have a header... */
	CHKiRet(msgAddJSON(*msg, (uchar*)"!", jo_request, 0, 0));

	finalize_it:
		RETiRet;

}


static rsRetVal
getDataRetryFailures(context *ctx,int itemStatus,char *request,char *response,
		fjson_object *response_item, fjson_object *response_body,
		fjson_object *status __attribute__((unused)))
{
	DEFiRet;
	fjson_object *omes = NULL;
//	fjson_object *jo = NULL;
	struct json_object_iterator it = json_object_iter_begin(response_item);
	struct json_object_iterator itEnd = json_object_iter_end(response_item);
//	const char *optype = NULL;
	smsg_t *msg = NULL;

	(void)response;
	(void)itemStatus;
	CHKiRet(createMsgFromRequest(request, ctx, &msg));
	CHKmalloc(msg);
	/* add status as local variables */
	omes = json_object_new_object();

//	if (!json_object_iter_equal(&it, &itEnd))
//		optype = json_object_iter_peek_name(&it);
/*	if (optype) {
		jo = json_object_new_string(optype);
	} else {
		jo = json_object_new_string("unknown");
	}
*/

	/* add response_body fields to local var omes */
	it = json_object_iter_begin(response_body);
	itEnd = json_object_iter_end(response_body);
	while (!json_object_iter_equal(&it, &itEnd)) {
		json_object_object_add(omes, json_object_iter_peek_name(&it),
			json_object_get(json_object_iter_peek_value(&it)));
		json_object_iter_next(&it);
	}
	CHKiRet(msgAddJSON(msg, (uchar*)".omes", omes, 0, 0));
	omes = NULL;
	MsgSetRuleset(msg, ctx->retryRuleset);
	CHKiRet(ratelimitAddMsg(ctx->ratelimiter, NULL, msg));
finalize_it:
	if (omes)
		json_object_put(omes);
	RETiRet;
}

/*
 * get erroronly context
 */
static rsRetVal
initializeErrorOnlyConext(wrkrInstanceData_t *pWrkrData,context *ctx){
	DEFiRet;
	ctx->statusCheckOnly=0;
	fjson_object *errRoot=NULL;
	fjson_object *onlyErrorResponses =NULL;
	fjson_object *onlyErrorRequests=NULL;
	if((errRoot=fjson_object_new_object()) == NULL) ABORT_FINALIZE(RS_RET_ERR);

	if((onlyErrorResponses=fjson_object_new_array()) == NULL) {
		fjson_object_put(errRoot);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	if((onlyErrorRequests=fjson_object_new_array()) == NULL) {
		fjson_object_put(errRoot);
		fjson_object_put(onlyErrorResponses);
		ABORT_FINALIZE(RS_RET_ERR);
	}

	fjson_object_object_add(errRoot, "url", fjson_object_new_string((char*)pWrkrData->restURL));
	fjson_object_object_add(errRoot,"request",onlyErrorRequests);
	fjson_object_object_add(errRoot, "reply", onlyErrorResponses);
	ctx->errRoot = errRoot;
	ctx->prepareErrorFileContent= &getDataErrorOnly;
	finalize_it:
		RETiRet;
}

/*
 * get interleaved context
 */
static rsRetVal
initializeInterleavedConext(wrkrInstanceData_t *pWrkrData,context *ctx){
	DEFiRet;
	ctx->statusCheckOnly=0;
	fjson_object *errRoot=NULL;
	fjson_object *interleaved =NULL;
	if((errRoot=fjson_object_new_object()) == NULL) ABORT_FINALIZE(RS_RET_ERR);
	if((interleaved=fjson_object_new_array()) == NULL) {
		fjson_object_put(errRoot);
		ABORT_FINALIZE(RS_RET_ERR);
	}


	fjson_object_object_add(errRoot, "url", fjson_object_new_string((char*)pWrkrData->restURL));
	fjson_object_object_add(errRoot,"response",interleaved);
	ctx->errRoot = errRoot;
	ctx->prepareErrorFileContent= &getDataInterleaved;
	finalize_it:
		RETiRet;
}

/*get interleaved context*/
static rsRetVal
initializeErrorInterleavedConext(wrkrInstanceData_t *pWrkrData,context *ctx){
	DEFiRet;
	ctx->statusCheckOnly=0;
	fjson_object *errRoot=NULL;
	fjson_object *interleaved =NULL;
	if((errRoot=fjson_object_new_object()) == NULL) ABORT_FINALIZE(RS_RET_ERR);
	if((interleaved=fjson_object_new_array()) == NULL) {
		fjson_object_put(errRoot);
		ABORT_FINALIZE(RS_RET_ERR);
	}


	fjson_object_object_add(errRoot, "url", fjson_object_new_string((char*)pWrkrData->restURL));
	fjson_object_object_add(errRoot,"response",interleaved);
	ctx->errRoot = errRoot;
	ctx->prepareErrorFileContent= &getDataErrorOnlyInterleaved;
	finalize_it:
		RETiRet;
}

/*get retry failures context*/
static rsRetVal
initializeRetryFailuresContext(wrkrInstanceData_t *pWrkrData,context *ctx){
	DEFiRet;
	ctx->statusCheckOnly=0;
	fjson_object *errRoot=NULL;
	if((errRoot=fjson_object_new_object()) == NULL) ABORT_FINALIZE(RS_RET_ERR);


	fjson_object_object_add(errRoot, "url", fjson_object_new_string((char*)pWrkrData->restURL));
	ctx->errRoot = errRoot;
	ctx->prepareErrorFileContent= &getDataRetryFailures;
	CHKmalloc(ctx->jTokener = json_tokener_new());
	finalize_it:
		RETiRet;
}


/* write data error request/replies to separate error file
 * Note: we open the file but never close it before exit. If it
 * needs to be closed, HUP must be sent.
 */
static rsRetVal ATTR_NONNULL()
writeDataError(wrkrInstanceData_t *const pWrkrData,
	instanceData *const pData, fjson_object **const pReplyRoot,
	uchar *const reqmsg)
{
	char *rendered = NULL;
	size_t toWrite;
	ssize_t wrRet;
	sbool bMutLocked = 0;
	context ctx;
	ctx.errRoot=0;
	ctx.ratelimiter = pWrkrData->pData->ratelimiter;
	ctx.retryRuleset = pWrkrData->pData->retryRuleset;
	ctx.jTokener = NULL;
	DEFiRet;

	if(pData->errorFile == NULL) {
		DBGPRINTF("omhttp: no local error logger defined - "
		          "ignoring REST error information\n");
		FINALIZE;
	}

	pthread_mutex_lock(&pData->mutErrFile);
	bMutLocked = 1;

	DBGPRINTF("omhttp: error file mode: erroronly='%d' errorInterleaved='%d'\n",
		pData->errorOnly, pData->interleaved);

	if(pData->interleaved ==0 && pData->errorOnly ==0)/*default write*/
	{
		if(getDataErrorDefault(pWrkrData,pReplyRoot, reqmsg, &rendered) != RS_RET_OK) {
			ABORT_FINALIZE(RS_RET_ERR);
		}
	} else {
		/*get correct context.*/
		if(pData->interleaved && pData->errorOnly)
		{
			if(initializeErrorInterleavedConext(pWrkrData, &ctx) != RS_RET_OK) {
				DBGPRINTF("omhttp: error initializing error interleaved context.\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}

		} else if(pData->errorOnly) {
			if(initializeErrorOnlyConext(pWrkrData, &ctx) != RS_RET_OK) {

				DBGPRINTF("omhttp: error initializing error only context.\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(pData->interleaved) {
			if(initializeInterleavedConext(pWrkrData, &ctx) != RS_RET_OK) {
				DBGPRINTF("omhttp: error initializing error interleaved context.\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(pData->retryFailures) {
			if(initializeRetryFailuresContext(pWrkrData, &ctx) != RS_RET_OK) {
				DBGPRINTF("omhttp: error initializing retry failures context.\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else {
			DBGPRINTF("omhttp: None of the modes match file write. No data to write.\n");
			ABORT_FINALIZE(RS_RET_ERR);
		}

		/*execute context*/
		if(parseRequestAndResponseForContext(pWrkrData, pReplyRoot, reqmsg, &ctx)!= RS_RET_OK) {
			DBGPRINTF("omhttp: error creating file content.\n");
			ABORT_FINALIZE(RS_RET_ERR);
		}
		CHKmalloc(rendered = strdup((char*)fjson_object_to_json_string(ctx.errRoot)));
	}


	if(pData->fdErrFile == -1) {
		pData->fdErrFile = open((char*)pData->errorFile,
					O_WRONLY|O_CREAT|O_APPEND|O_LARGEFILE|O_CLOEXEC,
					S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
		if(pData->fdErrFile == -1) {
			LogError(errno, RS_RET_ERR, "omhttp: error opening error file %s",
				pData->errorFile);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

	/* we do not do real error-handling on the err file, as this finally complicates
	 * things way to much.
	 */
	DBGPRINTF("omhttp: error record: '%s'\n", rendered);
	toWrite = strlen(rendered) + 1;
	/* Note: we overwrite the '\0' terminator with '\n' -- so we avoid
	 * caling malloc() -- write() does NOT need '\0'!
	 */
	rendered[toWrite-1] = '\n'; /* NO LONGER A STRING! */
	wrRet = write(pData->fdErrFile, rendered, toWrite);
	if(wrRet != (ssize_t) toWrite) {
		LogError(errno, RS_RET_IO_ERROR,
			"omhttp: error writing error file %s, write returned %lld",
			pData->errorFile, (long long) wrRet);
	}

finalize_it:
	if(bMutLocked)
		pthread_mutex_unlock(&pData->mutErrFile);
	free(rendered);
	fjson_object_put(ctx.errRoot);
	if (ctx.jTokener)
		json_tokener_free(ctx.jTokener);
	RETiRet;
}


static rsRetVal
checkResultBulkmode(wrkrInstanceData_t *pWrkrData, fjson_object *root, uchar *reqmsg)
{
	DEFiRet;
	context ctx;
	ctx.errRoot = 0;
	ctx.ratelimiter = pWrkrData->pData->ratelimiter;
	ctx.retryRuleset = pWrkrData->pData->retryRuleset;
	ctx.statusCheckOnly=1;
	ctx.jTokener = NULL;
	if (pWrkrData->pData->retryFailures) {
		ctx.statusCheckOnly=0;
		CHKiRet(initializeRetryFailuresContext(pWrkrData, &ctx));
	}

	if(parseRequestAndResponseForContext(pWrkrData,&root,reqmsg,&ctx)!= RS_RET_OK) {
		DBGPRINTF("omhttp: error found in http reply\n");
		ABORT_FINALIZE(RS_RET_DATAFAIL);
	}

finalize_it:
	fjson_object_put(ctx.errRoot);
	if (ctx.jTokener)
		json_tokener_free(ctx.jTokener);
	RETiRet;
}


static rsRetVal
checkResult(wrkrInstanceData_t *pWrkrData, uchar *reqmsg)
{
	fjson_object *root;
	fjson_object *status;
	DEFiRet;

	root = fjson_tokener_parse(pWrkrData->reply);
	if(root == NULL) {
		LogMsg(0, RS_RET_ERR, LOG_WARNING,
			"omhttp: could not parse JSON result");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	if(pWrkrData->pData->bulkmode) {
		iRet = checkResultBulkmode(pWrkrData, root, reqmsg);
	} else {
		if(fjson_object_object_get_ex(root, "status", &status)) {
			iRet = RS_RET_DATAFAIL;
		}
	}

	/* Note: we ignore errors writing the error file, as we cannot handle
	 * these in any case.
	 */
	if(iRet == RS_RET_DATAFAIL) {
		writeDataError(pWrkrData, pWrkrData->pData, &root, reqmsg);
		iRet = RS_RET_OK; /* we have handled the problem! */
	}

finalize_it:
	if(root != NULL)
		fjson_object_put(root);
	RETiRet;
}

static void ATTR_NONNULL()
initializeBatch(wrkrInstanceData_t *pWrkrData)
{
	es_emptyStr(pWrkrData->batch.data);
	pWrkrData->batch.nmemb = 0;
}

static rsRetVal ATTR_NONNULL(1, 2)
curlPost(wrkrInstanceData_t *pWrkrData, uchar *message, int msglen, uchar **tpls,
		const int nmsgs __attribute__((unused)))
{
	CURLcode code;
	CURL *const curl = pWrkrData->curlPostHandle;
	char errbuf[CURL_ERROR_SIZE] = "";
	DEFiRet;

	PTR_ASSERT_SET_TYPE(pWrkrData, WRKR_DATA_TYPE_ES);

	pWrkrData->reply = NULL;
	pWrkrData->replyLen = 0;

	if(pWrkrData->pData->numServers > 1) {
		/* needs to be called to support ES HA feature */
		CHKiRet(checkConn(pWrkrData));
	}
	CHKiRet(setPostURL(pWrkrData, tpls));

	pWrkrData->reply = NULL;
	pWrkrData->replyLen = 0;

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (char *)message);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, msglen);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
	code = curl_easy_perform(curl);
	DBGPRINTF("curl returned %lld\n", (long long) code);
	if (code != CURLE_OK && code != CURLE_HTTP_RETURNED_ERROR) {
		LogError(0, RS_RET_SUSPENDED,
			"omhttp: we are suspending ourselfs due "
			"to server failure %lld: %s", (long long) code, errbuf);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	if(pWrkrData->reply == NULL) {
		DBGPRINTF("omhttp: pWrkrData reply==NULL, replyLen = '%d'\n",
			pWrkrData->replyLen);
	} else {
		DBGPRINTF("omhttp: pWrkrData replyLen = '%d'\n", pWrkrData->replyLen);
		if(pWrkrData->replyLen > 0) {
			pWrkrData->reply[pWrkrData->replyLen] = '\0';
			/* Append 0 Byte if replyLen is above 0 - byte has been reserved in malloc */
		}
		//TODO: replyLen++? because 0 Byte is appended
		DBGPRINTF("omhttp: pWrkrData reply: '%s'\n", pWrkrData->reply);
		CHKiRet(checkResult(pWrkrData, message));
	}

finalize_it:
	incrementServerIndex(pWrkrData);
	free(pWrkrData->reply);
	pWrkrData->reply = NULL; /* don't leave dangling pointer */
	RETiRet;
}

static rsRetVal
submitBatch(wrkrInstanceData_t *pWrkrData)
{
	char *cstr = NULL;
	DEFiRet;

	cstr = es_str2cstr(pWrkrData->batch.data, NULL);
	dbgprintf("omhttp: submitBatch, batch: '%s'\n", cstr);

	CHKiRet(curlPost(pWrkrData, (uchar*) cstr, strlen(cstr), NULL, pWrkrData->batch.nmemb));

finalize_it:
	free(cstr);
	RETiRet;
}

BEGINbeginTransaction
CODESTARTbeginTransaction
	if(!pWrkrData->pData->bulkmode) {
		FINALIZE;
	}

	initializeBatch(pWrkrData);
finalize_it:
ENDbeginTransaction

BEGINdoAction
CODESTARTdoAction

	if(pWrkrData->pData->bulkmode) {
		const size_t nBytes = ustrlen((char *)ppString[0]) + sizeof("\n")-1 ;

		/* If max bytes is set and this next message will put us over the limit,
		* submit the current buffer and reset */
		if(pWrkrData->pData->maxbytes > 0
			&& es_strlen(pWrkrData->batch.data) + nBytes > pWrkrData->pData->maxbytes ) {
			dbgprintf("omhttp: maxbytes limit reached, submitting partial "
			"batch of %d elements.\n", pWrkrData->batch.nmemb);
			CHKiRet(submitBatch(pWrkrData));
			initializeBatch(pWrkrData);
		}
		CHKiRet(buildBatch(pWrkrData, ppString[0]));

		/* If there is only one item in the batch, all previous items have been
	 	 * submitted or this is the first item for this transaction. Return previous
		 * committed so that all items leading up to the current (exclusive)
		 * are not replayed should a failure occur anywhere else in the transaction. */
		iRet = pWrkrData->batch.nmemb == 1 ? RS_RET_PREVIOUS_COMMITTED : RS_RET_DEFER_COMMIT;
	} else {
		CHKiRet(curlPost(pWrkrData, ppString[0], strlen((char*)ppString[0]),
		                 ppString, 1));
	}
finalize_it:
ENDdoAction


BEGINendTransaction
CODESTARTendTransaction
	/* End Transaction only if batch data is not empty */
	if (pWrkrData->batch.data != NULL && pWrkrData->batch.nmemb > 0) {
		CHKiRet(submitBatch(pWrkrData));
	} else {
		dbgprintf("omhttp: endTransaction, pWrkrData->batch.data is NULL, "
			"nothing to send. \n");
	}
finalize_it:
ENDendTransaction

/* Creates authentication header uid:pwd
 */
static rsRetVal
computeAuthHeader(char* uid, char* pwd, uchar** authBuf) {
	int r;
	DEFiRet;

	es_str_t* auth = es_newStr(1024);
	if (auth == NULL) {
		LogError(0, RS_RET_OUT_OF_MEMORY,
			"omhttp: failed to allocate es_str auth for auth header construction");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	r = es_addBuf(&auth, uid, strlen(uid));
	if(r == 0) r = es_addChar(&auth, ':');
	if(r == 0 && pwd != NULL) r = es_addBuf(&auth, pwd, strlen(pwd));
	if(r == 0) *authBuf = (uchar*) es_str2cstr(auth, NULL);

	if (r != 0 || *authBuf == NULL) {
		errmsg.LogError(0, RS_RET_ERR, "omhttp: failed to build auth header\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}

finalize_it:
	if (auth != NULL)
		es_deleteStr(auth);
	RETiRet;
}

static rsRetVal
computeApiHeader(char* key, char* value, uchar** headerBuf) {
	int r;
	DEFiRet;

	es_str_t* header = es_newStr(10240);
	if (header == NULL) {
		LogError(0, RS_RET_OUT_OF_MEMORY,
		"omhttp: failed to allocate es_str auth for api header construction");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	r = es_addBuf(&header, key, strlen(key));
	if(r == 0) r = es_addChar(&header, ':');
	if(r == 0) r = es_addChar(&header, ' ');
	if(r == 0 && value != NULL) r = es_addBuf(&header, value, strlen(value));
	if(r == 0) *headerBuf = (uchar*) es_str2cstr(header, NULL);

	if (r != 0 || *headerBuf == NULL) {
		errmsg.LogError(0, RS_RET_ERR, "omhttp: failed to build http header\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}

finalize_it:
	if (header != NULL)
		es_deleteStr(header);
	RETiRet;
}

static void ATTR_NONNULL()
curlSetupCommon(wrkrInstanceData_t *const pWrkrData, CURL *const handle)
{
	PTR_ASSERT_SET_TYPE(pWrkrData, WRKR_DATA_TYPE_ES);
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, pWrkrData->curlHeader);
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, TRUE);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlResult);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, pWrkrData);
	if(pWrkrData->pData->allowUnsignedCerts)
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, FALSE);
	if(pWrkrData->pData->authBuf != NULL) {
		curl_easy_setopt(handle, CURLOPT_USERPWD, pWrkrData->pData->authBuf);
		curl_easy_setopt(handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
	}
	if(pWrkrData->pData->caCertFile)
		curl_easy_setopt(handle, CURLOPT_CAINFO, pWrkrData->pData->caCertFile);
	if(pWrkrData->pData->myCertFile)
		curl_easy_setopt(handle, CURLOPT_SSLCERT, pWrkrData->pData->myCertFile);
	if(pWrkrData->pData->myPrivKeyFile)
		curl_easy_setopt(handle, CURLOPT_SSLKEY, pWrkrData->pData->myPrivKeyFile);
	/* uncomment for in-dept debuggung:
	curl_easy_setopt(handle, CURLOPT_VERBOSE, TRUE); */
}

static void ATTR_NONNULL()
curlCheckConnSetup(wrkrInstanceData_t *const pWrkrData)
{
	PTR_ASSERT_SET_TYPE(pWrkrData, WRKR_DATA_TYPE_ES);
	curlSetupCommon(pWrkrData, pWrkrData->curlCheckConnHandle);
	curl_easy_setopt(pWrkrData->curlCheckConnHandle,
		CURLOPT_TIMEOUT_MS, pWrkrData->pData->healthCheckTimeout);
}

static void ATTR_NONNULL(1)
curlPostSetup(wrkrInstanceData_t *const pWrkrData)
{
	PTR_ASSERT_SET_TYPE(pWrkrData, WRKR_DATA_TYPE_ES);
	curlSetupCommon(pWrkrData, pWrkrData->curlPostHandle);
	curl_easy_setopt(pWrkrData->curlPostHandle, CURLOPT_POST, 1);
}

#define CONTENT_JSON "Content-Type: application/json; charset=utf-8"

static rsRetVal ATTR_NONNULL()
curlSetup(wrkrInstanceData_t *const pWrkrData)
{
	struct curl_slist *slist=NULL;

	DEFiRet;
	slist = curl_slist_append(slist, CONTENT_JSON);
	if (pWrkrData->pData->headerBuf != NULL) {
		slist = curl_slist_append(slist, (char*) pWrkrData->pData->headerBuf);
	}
	pWrkrData->curlHeader = slist;
	CHKmalloc(pWrkrData->curlPostHandle = curl_easy_init());;
	curlPostSetup(pWrkrData);

	CHKmalloc(pWrkrData->curlCheckConnHandle = curl_easy_init());
	curlCheckConnSetup(pWrkrData);

finalize_it:
	if(iRet != RS_RET_OK && pWrkrData->curlPostHandle != NULL) {
		curl_easy_cleanup(pWrkrData->curlPostHandle);
		pWrkrData->curlPostHandle = NULL;
	}
	RETiRet;
}

static void ATTR_NONNULL()
setInstParamDefaults(instanceData *const pData)
{
	pData->serverBaseUrls = NULL;
	pData->defaultPort = 443;
	pData->healthCheckTimeout = 3500;
	pData->uid = NULL;
	pData->httpheaderkey = NULL;
	pData->httpheadervalue = NULL;
	pData->pwd = NULL;
	pData->authBuf = NULL;
	pData->restPath = NULL;
	pData->checkPath = NULL;
	pData->dynRestPath = 0;
	pData->bulkmode = 1;
	pData->useHttps = 1;
	pData->maxbytes = 10485760; //i.e. 10 MB Is the default max message size for AWS API Gateway
	pData->allowUnsignedCerts = 0;
	pData->tplName = NULL;
	pData->errorFile = NULL;
	pData->errorOnly=0;
	pData->interleaved=0;
	pData->caCertFile = NULL;
	pData->myCertFile = NULL;
	pData->myPrivKeyFile = NULL;
	pData->retryFailures = 0;
	pData->ratelimitBurst = 20000;
	pData->ratelimitInterval = 600;
	pData->ratelimiter = NULL;
	pData->retryRulesetName = NULL;
	pData->retryRuleset = NULL;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	char* serverParam = NULL;
	struct cnfarray* servers = NULL;
	int i;
	int iNumTpls;
	FILE *fp;
	char errStr[1024];
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
			servers = pvals[i].val.d.ar;
		} else if(!strcmp(actpblk.descr[i].name, "errorfile")) {
			pData->errorFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "erroronly")) {
			pData->errorOnly = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "interleaved")) {
			pData->interleaved = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "serverport")) {
			pData->defaultPort = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "healthchecktimeout")) {
			pData->healthCheckTimeout = (long) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "uid")) {
			pData->uid = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "httpheaderkey")) {
			pData->httpheaderkey = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "httpheadervalue")) {
			pData->httpheadervalue = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "pwd")) {
			pData->pwd = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "restpath")) {
			pData->restPath = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "checkpath")) {
			pData->checkPath = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "dynrestpath")) {
			pData->dynRestPath = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "bulkmode")) {
			pData->bulkmode = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "maxbytes")) {
			pData->maxbytes = (size_t) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "allowunsignedcerts")) {
			pData->allowUnsignedCerts = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "usehttps")) {
			pData->useHttps = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "tls.cacert")) {
			pData->caCertFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)pData->caCertFile, "r");
			if(fp == NULL) {
				rs_strerror_r(errno, errStr, sizeof(errStr));
				errmsg.LogError(0, RS_RET_NO_FILE_ACCESS,
						"error: 'tls.cacert' file %s couldn't be accessed: %s\n",
						pData->caCertFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(actpblk.descr[i].name, "tls.mycert")) {
			pData->myCertFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)pData->myCertFile, "r");
			if(fp == NULL) {
				rs_strerror_r(errno, errStr, sizeof(errStr));
				errmsg.LogError(0, RS_RET_NO_FILE_ACCESS,
						"error: 'tls.mycert' file %s couldn't be accessed: %s\n",
						pData->myCertFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(actpblk.descr[i].name, "tls.myprivkey")) {
			pData->myPrivKeyFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)pData->myPrivKeyFile, "r");
			if(fp == NULL) {
				rs_strerror_r(errno, errStr, sizeof(errStr));
				errmsg.LogError(0, RS_RET_NO_FILE_ACCESS,
						"error: 'tls.myprivkey' file %s couldn't be accessed: %s\n",
						pData->myPrivKeyFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(actpblk.descr[i].name, "retryfailures")) {
			pData->retryFailures = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "ratelimit.burst")) {
			pData->ratelimitBurst = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "ratelimit.interval")) {
			pData->ratelimitInterval = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "retryruleset")) {
			pData->retryRulesetName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			LogError(0, RS_RET_INTERNAL_ERROR, "omhttp: program error, "
				"non-handled param '%s'", actpblk.descr[i].name);
		}
	}

	if(pData->pwd != NULL && pData->uid == NULL) {
		errmsg.LogError(0, RS_RET_UID_MISSING,
			"omhttp: password is provided, but no uid "
			"- action definition invalid");
		ABORT_FINALIZE(RS_RET_UID_MISSING);
	}
	if(pData->httpheaderkey != NULL && pData->httpheadervalue == NULL) {
		errmsg.LogError(0, RS_RET_UID_MISSING,
			"omhttp: http header key is provided, but no http header value "
			"- action definition invalid");
		ABORT_FINALIZE(RS_RET_UID_MISSING);
	}
	if(pData->dynRestPath && pData->restPath == NULL) {
		errmsg.LogError(0, RS_RET_CONFIG_ERROR,
			"omhttp: requested dynamic rest path, but no name for rest "
			"path template given - action definition invalid");
		ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}

	if (pData->uid != NULL)
		CHKiRet(computeAuthHeader((char*) pData->uid, (char*) pData->pwd, &pData->authBuf));
	if (pData->httpheaderkey != NULL)
		CHKiRet(computeApiHeader((char*) pData->httpheaderkey,
				(char*) pData->httpheadervalue, &pData->headerBuf));

	iNumTpls = 1;
	if(pData->dynRestPath) ++iNumTpls;
	DBGPRINTF("omhttp: requesting %d templates\n", iNumTpls);
	CODE_STD_STRING_REQUESTnewActInst(iNumTpls)

	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((pData->tplName == NULL) ?
					    " StdJSONFmt" : (char*)pData->tplName),
		OMSR_NO_RQD_TPL_OPTS));


	/* we need to request additional templates. If we have a dynamic search index,
	 * it will always be string 1. Type may be 1 or 2, depending on whether search
	 * index is dynamic as well. Rule needs to be followed throughout the module.
	 */
	iNumTpls = 1;
	if(pData->dynRestPath) {
		CHKiRet(OMSRsetEntry(*ppOMSR, iNumTpls, ustrdup(pData->restPath),
			OMSR_NO_RQD_TPL_OPTS));
		++iNumTpls;
	}

	if (servers != NULL) {
		pData->numServers = servers->nmemb;
		pData->serverBaseUrls = malloc(servers->nmemb * sizeof(uchar*));
		if (pData->serverBaseUrls == NULL) {
			errmsg.LogError(0, RS_RET_ERR, "omhttp: unable to allocate buffer "
					"for http server configuration.");
			ABORT_FINALIZE(RS_RET_ERR);
		}

		for(i = 0 ; i < servers->nmemb ; ++i) {
			serverParam = es_str2cstr(servers->arr[i], NULL);
			if (serverParam == NULL) {
				errmsg.LogError(0, RS_RET_ERR, "omhttp: unable to allocate buffer "
					"for http server configuration.");
				ABORT_FINALIZE(RS_RET_ERR);
			}
			/* Remove a trailing slash if it exists */
			const size_t serverParamLastChar = strlen(serverParam)-1;
			if (serverParam[serverParamLastChar] == '/') {
				serverParam[serverParamLastChar] = '\0';
			}
			CHKiRet(computeBaseUrl(serverParam, pData->defaultPort, pData->useHttps,
				pData->serverBaseUrls + i));
			free(serverParam);
			serverParam = NULL;
		}
	} else {
		LogMsg(0, RS_RET_OK, LOG_WARNING,
			"omhttp: No servers specified, using localhost");
		pData->numServers = 1;
		pData->serverBaseUrls = malloc(sizeof(uchar*));
		if (pData->serverBaseUrls == NULL) {
			errmsg.LogError(0, RS_RET_ERR, "omhttp: unable to allocate buffer "
					"for http server configuration.");
			ABORT_FINALIZE(RS_RET_ERR);
		}
		CHKiRet(computeBaseUrl("localhost", pData->defaultPort, pData->useHttps, pData->serverBaseUrls));
	}

	if (pData->retryFailures) {
		CHKiRet(ratelimitNew(&pData->ratelimiter, "omhttp", NULL));
		ratelimitSetLinuxLike(pData->ratelimiter, pData->ratelimitInterval, pData->ratelimitBurst);
		ratelimitSetNoTimeCache(pData->ratelimiter);
	}

	/* node created, let's add to list of instance configs for the module */
	if(loadModConf->tail == NULL) {
		loadModConf->tail = loadModConf->root = pData;
	} else {
		loadModConf->tail->next = pData;
		loadModConf->tail = pData;
	}

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
	if (serverParam)
		free(serverParam);
ENDnewActInst


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	pModConf->root = pModConf->tail = NULL;
ENDbeginCnfLoad


BEGINendCnfLoad
CODESTARTendCnfLoad
	loadModConf = NULL; /* done loading */
ENDendCnfLoad


BEGINcheckCnf
	instanceConf_t *inst;
CODESTARTcheckCnf
	for(inst = pModConf->root ; inst != NULL ; inst = inst->next) {
		ruleset_t *pRuleset;
		rsRetVal localRet;

		if (inst->retryRulesetName) {
			localRet = ruleset.GetRuleset(pModConf->pConf, &pRuleset, inst->retryRulesetName);
			if(localRet == RS_RET_NOT_FOUND) {
				errmsg.LogError(0, localRet, "omhttp: retryruleset '%s' not found - "
						"no retry ruleset will be used", inst->retryRulesetName);
			} else {
				inst->retryRuleset = pRuleset;
			}
		}
	}
ENDcheckCnf


BEGINactivateCnf
CODESTARTactivateCnf
ENDactivateCnf


BEGINfreeCnf
CODESTARTfreeCnf
ENDfreeCnf


BEGINdoHUP
CODESTARTdoHUP
	if(pData->fdErrFile != -1) {
		close(pData->fdErrFile);
		pData->fdErrFile = -1;
	}
ENDdoHUP


BEGINmodExit
CODESTARTmodExit
	if(pInputName != NULL)
		prop.Destruct(&pInputName);
	curl_global_cleanup();
	objRelease(errmsg, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
ENDmodExit

NO_LEGACY_CONF_parseSelectorAct

BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_doHUP
CODEqueryEtryPt_TXIF_OMOD_QUERIES /* we support the transactional interface! */
CODEqueryEtryPt_STD_CONF2_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(errmsg, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
		errmsg.LogError(0, RS_RET_OBJ_CREATION_FAILED, "CURL fail. -http disabled");
		ABORT_FINALIZE(RS_RET_OBJ_CREATION_FAILED);
	}

	CHKiRet(prop.Construct(&pInputName));
	CHKiRet(prop.SetString(pInputName, UCHAR_CONSTANT("omhttp"), sizeof("omhttp") - 1));
	CHKiRet(prop.ConstructFinalize(pInputName));
ENDmodInit

/* vi:set ai:
 */
