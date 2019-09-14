/* ommongodb.c
 * Output module for mongodb.
 *
 * Copyright 2007-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * Copyright 2017 Jeremie Jourdin and Hugo Soszynski and aDvens
 * Remove deprecated libmongo-client and use libmongoc (mongo-c-driver)
 * This new library handle TLS and replicaset
 *
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <json.h>
/* we need this to avoid issues with older versions of libbson */
#ifndef AIX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#endif
#include <mongoc.h>
#include <bson.h>
#ifndef AIX
#pragma GCC diagnostic pop
#endif

#include "rsyslog.h"
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "datetime.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "parserif.h"
#include "unicode-helper.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("ommongodb")
/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(datetime)

typedef struct _instanceData {
	struct json_tokener *json_tokener; /* only if (tplName != NULL) */
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	char *server;
	char *port;
	char *uristr;
	char *ssl_ca;
	char *ssl_cert;
	char *uid;
	char *pwd;
	uint32_t allowed_error_codes[256];
	int allowed_error_codes_nbr;
	char *db;
	char *collection_name;
	char *tplName;
	int bErrMsgPermitted;	/* only one errmsg permitted per connection */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "server", eCmdHdlrGetWord, 0 },
	{ "serverport", eCmdHdlrGetWord, 0 },
	{ "uristr", eCmdHdlrGetWord, 0 },
	{ "ssl_ca", eCmdHdlrGetWord, 0 },
	{ "ssl_cert", eCmdHdlrGetWord, 0 },
	{ "uid", eCmdHdlrGetWord, 0 },
	{ "pwd", eCmdHdlrGetWord, 0 },
	{ "db", eCmdHdlrGetWord, 0 },
	{ "collection", eCmdHdlrGetWord, 0 },
	{ "template", eCmdHdlrGetWord, 0 },
	{ "allowed_error_codes", eCmdHdlrArray, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

static pthread_mutex_t mutDoAct = PTHREAD_MUTEX_INITIALIZER;

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	/* use this to specify if select features are supported by this
	 * plugin. If not, the framework will handle that. Currently, only
	 * RepeatedMsgReduction ("last message repeated n times") is optional.
	 */
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature

static void closeMongoDB(instanceData *pData)
{
	if(pData->client != NULL) {
		if (pData->collection != NULL) {
			mongoc_collection_destroy (pData->collection);
		}

		mongoc_client_destroy (pData->client);
		mongoc_cleanup ();
	}
}


BEGINfreeInstance
CODESTARTfreeInstance
	closeMongoDB(pData);

	if (pData->json_tokener != NULL)
		json_tokener_free(pData->json_tokener);
	free(pData->server);
	free(pData->port);
	free(pData->ssl_ca);
	free(pData->ssl_cert);
	free(pData->uristr);
	free(pData->uid);
	free(pData->pwd);
	free(pData->db);
	free(pData->collection_name);
	free(pData->tplName);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	/* nothing special here */
	(void)pData;
ENDdbgPrintInstInfo


/* report error that occured during *last* operation
 */
static void
reportMongoError(instanceData *pData)
{
	if(pData->bErrMsgPermitted) {
		LogError(0, RS_RET_ERR, "ommongodb: error: %s", pData->error.message);
		pData->bErrMsgPermitted = 0;
	}
}


/* The following function is responsible for initializing a
 * MongoDB connection.
 * Initially added 2004-10-28 mmeckelein
 */
static rsRetVal initMongoDB(instanceData *pData, int bSilent)
{
	DEFiRet;

	DBGPRINTF("ommongodb: uristr is '%s'", pData->uristr);
	mongoc_init ();
	pData->client = mongoc_client_new (pData->uristr);
	if (pData->ssl_cert && pData->ssl_ca) {
#ifdef HAVE_MONGOC_CLIENT_SET_SSL_OPTS
		mongoc_ssl_opt_t ssl_opts;
		memset(&ssl_opts, 0, sizeof(mongoc_ssl_opt_t));
		ssl_opts.pem_file = pData->ssl_cert;
		ssl_opts.ca_file = pData->ssl_ca;
		mongoc_client_set_ssl_opts (pData->client, &ssl_opts);
#else
		dbgprintf("ommongodb: mongo-c-driver was not built with SSL options, ssl directives will not be used.");
#endif
	}
	if(pData->client == NULL) {
		if(!bSilent) {
			reportMongoError(pData);
			dbgprintf("ommongodb: can not initialize MongoDB handle");
		}
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}
	pData->collection = mongoc_client_get_collection (pData->client, pData->db, pData->collection_name);

finalize_it:
	RETiRet;
}


/* map syslog severity to lumberjack level
 * TODO: consider moving this to msg.c - make some dirty "friend" references...
 * rgerhards, 2012-03-19
 */
static const char *
getLumberjackLevel(short severity)
{
	switch(severity) {
		case 0: return "FATAL";
		case 1:
		case 2:
		case 3: return "ERROR";
		case 4: return "WARN";
		case 5:
		case 6: return "INFO";
		case 7: return "DEBUG";
		default:DBGPRINTF("ommongodb: invalid syslog severity %u\n", severity);
			return "INVLD";
	}
}


/* small helper: get integer power of 10 */
static int
i10pow(int exp)
{
	int r = 1;
	while(exp > 0) {
		r *= 10;
		exp--;
	}
	return r;
}
/* Return a BSON document when an user hasn't specified a template.
 * In this mode, we use the standard document format, which is somewhat
 * aligned to cee (as described in project lumberjack). Note that this is
 * a moving target, so we may run out of sync (and stay so to retain
 * backward compatibility, which we consider pretty important).
 */
static bson_t *getDefaultBSON(smsg_t *pMsg)
{
	bson_t *doc = NULL;
	char *procid; short unsigned procid_free; rs_size_t procid_len;
	char *tag; short unsigned tag_free; rs_size_t tag_len;
	char *pid; short unsigned pid_free; rs_size_t pid_len;
	char *sys; short unsigned sys_free; rs_size_t sys_len;
	char *msg; short unsigned msg_free; rs_size_t msg_len;
	int severity, facil;
	int64 ts_gen, ts_rcv; /* timestamps: generated, received */
	int secfrac;
	msgPropDescr_t cProp; /* we use internal implementation knowledge... */

	cProp.id = PROP_PROGRAMNAME;
	procid = (char*)MsgGetProp(pMsg, NULL, &cProp, &procid_len, &procid_free, NULL);
	cProp.id = PROP_SYSLOGTAG;
	tag = (char*)MsgGetProp(pMsg, NULL, &cProp, &tag_len, &tag_free, NULL);
	cProp.id = PROP_PROCID;
	pid = (char*)MsgGetProp(pMsg, NULL, &cProp, &pid_len, &pid_free, NULL);
	cProp.id = PROP_HOSTNAME;
	sys = (char*)MsgGetProp(pMsg, NULL, &cProp, &sys_len, &sys_free, NULL);
	cProp.id = PROP_MSG;
	msg = (char*)MsgGetProp(pMsg, NULL, &cProp, &msg_len, &msg_free, NULL);

	/* TODO: move to datetime? Refactor in any case! rgerhards, 2012-03-30 */
	ts_gen = (int64) datetime.syslogTime2time_t(&pMsg->tTIMESTAMP) * 1000; /* ms! */
	DBGPRINTF("ommongodb: ts_gen is %lld\n", (long long) ts_gen);
	DBGPRINTF("ommongodb: secfrac is %d, precision %d\n",
			pMsg->tTIMESTAMP.secfrac,
			pMsg->tTIMESTAMP.secfracPrecision);
	if(pMsg->tTIMESTAMP.secfracPrecision > 3) {
		secfrac = pMsg->tTIMESTAMP.secfrac / i10pow(pMsg->tTIMESTAMP.secfracPrecision - 3);
	} else if(pMsg->tTIMESTAMP.secfracPrecision < 3) {
		secfrac = pMsg->tTIMESTAMP.secfrac * i10pow(3 - pMsg->tTIMESTAMP.secfracPrecision);
	} else {
		secfrac = pMsg->tTIMESTAMP.secfrac;
	}
	ts_gen += secfrac;
	ts_rcv = (int64) datetime.syslogTime2time_t(&pMsg->tRcvdAt) * 1000; /* ms! */
	if(pMsg->tRcvdAt.secfracPrecision > 3) {
		secfrac = pMsg->tRcvdAt.secfrac / i10pow(pMsg->tRcvdAt.secfracPrecision - 3);
	} else if(pMsg->tRcvdAt.secfracPrecision < 3) {
		secfrac = pMsg->tRcvdAt.secfrac * i10pow(3 - pMsg->tRcvdAt.secfracPrecision);
	} else {
		secfrac = pMsg->tRcvdAt.secfrac;
	}
	ts_rcv += secfrac;

	/* the following need to be int, but are short, so we need to xlat */
	severity = pMsg->iSeverity;
	facil = pMsg->iFacility;

	doc = bson_new ();
	bson_oid_t oid;
	bson_oid_init (&oid, NULL);
	BSON_APPEND_OID (doc, "_id", &oid);
	BSON_APPEND_UTF8 (doc, "sys", sys);
	BSON_APPEND_DATE_TIME (doc, "time", ts_gen);
	BSON_APPEND_DATE_TIME (doc, "time_rcvd", ts_rcv);
	BSON_APPEND_UTF8 (doc, "msg", msg);
	BSON_APPEND_INT32 (doc, "syslog_fac", facil);
	BSON_APPEND_INT32 (doc, "syslog_sever", severity);
	BSON_APPEND_UTF8 (doc, "syslog_tag", tag);
	BSON_APPEND_UTF8 (doc, "procid", procid);
	BSON_APPEND_UTF8 (doc, "pid", pid);
	BSON_APPEND_UTF8 (doc, "level", getLumberjackLevel(pMsg->iSeverity));

	if(procid_free) free(procid);
	if(tag_free) free(tag);
	if(pid_free) free(pid);
	if(sys_free) free(sys);
	if(msg_free) free(msg);

	return doc;
}

static bson_t *BSONFromJSONArray(struct json_object *json);
static bson_t *BSONFromJSONObject(struct json_object *json);
static int BSONAppendExtendedJSON(bson_t *doc, const char *name, struct json_object *json);

/* Append a BSON variant of json to doc using name.  Return TRUE on success */
static int
BSONAppendJSONObject(bson_t *doc, const char *name, struct json_object *json)
{

	switch(json != NULL ? json_object_get_type(json) : json_type_null) {

	case json_type_null:
		return BSON_APPEND_NULL(doc, name);
	case json_type_boolean:
		return BSON_APPEND_BOOL(doc, name, json_object_get_boolean(json));
	case json_type_double:
		return BSON_APPEND_DOUBLE(doc, name, json_object_get_double(json));
	case json_type_int: {
		int64_t i;

		i = json_object_get_int64(json);
		if (i >= INT32_MIN && i <= INT32_MAX)
			return BSON_APPEND_INT32(doc, name, i);
		else
			return BSON_APPEND_INT64(doc, name, i);
	}
	case json_type_object: {

		if (BSONAppendExtendedJSON(doc, name, json) == TRUE)
		    return TRUE;

		bson_t *sub;
		int ok;

		sub = BSONFromJSONObject(json);
		if (sub == NULL)
			return FALSE;
		ok = BSON_APPEND_DOCUMENT(doc, name, sub);
		bson_destroy(sub);
		return ok;
	}
	case json_type_array: {
		bson_t *sub;
		int ok;

		sub = BSONFromJSONArray(json);
		if (sub == NULL)
			return FALSE;
		ok = BSON_APPEND_DOCUMENT(doc, name, sub);
		bson_destroy(sub);
		return ok;
	}
	case json_type_string: {
		/* Convert text to ISODATE when needed */
		if (strncmp(name, "date", 5) == 0 || strncmp(name, "time", 5) == 0 ) {
			struct tm tm;
			if (strptime(json_object_get_string(json), "%Y-%m-%dT%H:%M:%S:%Z", &tm) != NULL ) {
				time_t epoch;
				int64 ts;
				epoch = mktime(&tm) ;
				ts = 1000 * (int64) epoch;
				return BSON_APPEND_DATE_TIME (doc, name, ts);
			}
		}
		else {
			return BSON_APPEND_UTF8(doc, name, json_object_get_string(json));
		}
	}
	default:
		return FALSE;
	}
}

/* Note: this function assumes that at max a single sub-object exists. This
 * may need to be extended to cover cases where multiple objects are contained.
 * However, I am not sure about the original intent of this contribution and
 * just came across it when refactoring the json calls. As everything seems
 * to work since quite a while, I do not make any changes now.
 * rgerhards, 2016-04-09
 */
static int
BSONAppendExtendedJSON(bson_t *doc, const char *name, struct json_object *json)
{
	struct json_object_iterator itEnd = json_object_iter_end(json);
	struct json_object_iterator it = json_object_iter_begin(json);

	if (!json_object_iter_equal(&it, &itEnd)) {
		const char *const key = json_object_iter_peek_name(&it);
		if (strcmp(key, "$date") == 0) {
			struct tm tm;
			int64 ts;
			struct json_object *val;

			val = json_object_iter_peek_value(&it);
			DBGPRINTF("ommongodb: extended json date detected %s", json_object_get_string(val));
			tm.tm_isdst = -1;
			strptime(json_object_get_string(val), "%Y-%m-%dT%H:%M:%S%z", &tm);
			ts = 1000 * (int64) mktime(&tm);
			return BSON_APPEND_DATE_TIME (doc, name, ts);
		}
	}
	return FALSE;
}

/* Return a BSON variant of json, which must be a json_type_array */
static bson_t *BSONFromJSONArray(struct json_object *json)
{
	/* Way more than necessary */
	bson_t *doc = NULL;
	size_t i, array_len;

	doc = bson_new();
	if(doc == NULL)
		goto error;

	array_len = json_object_array_length(json);
	for (i = 0; i < array_len; i++) {
		char buf[sizeof(size_t) * CHAR_BIT + 1];

		if ((size_t)snprintf(buf, sizeof(buf), "%zu", i) >= sizeof(buf))
			goto error;
		if (BSONAppendJSONObject(doc, buf, json_object_array_get_idx(json, i)) == FALSE)
			goto error;
	}

	return doc;

error:
	if(doc != NULL)
		bson_destroy(doc);
	return NULL;
}

/* Return a BSON variant of json, which must be a json_type_object */
static bson_t *BSONFromJSONObject(struct json_object *json)
{
	bson_t *doc = NULL;

	doc = bson_new();
	if(doc == NULL)
		return NULL;

	struct json_object_iterator it = json_object_iter_begin(json);
	struct json_object_iterator itEnd = json_object_iter_end(json);
	while (!json_object_iter_equal(&it, &itEnd)) {
		if (BSONAppendJSONObject(doc, json_object_iter_peek_name(&it),
			json_object_iter_peek_value(&it)) == FALSE)
			goto error;
		json_object_iter_next(&it);
	}

	return doc;

error:
	if(doc != NULL)
		bson_destroy(doc);
	return NULL;

}

BEGINtryResume
CODESTARTtryResume
	if(pWrkrData->pData->client == NULL) {
		iRet = initMongoDB(pWrkrData->pData, 1);
	}
ENDtryResume

/*
 * Check if `code` is in the allowed error codes.
 * Return 1 if so, 0 otherwise.
 */
static int is_allowed_error_code(instanceData const* pData, uint32_t code)
{
	int i;

	i = 0;
	while (i < pData->allowed_error_codes_nbr) {
		if (code == pData->allowed_error_codes[i])
			return 1;
		++i;
	}
	return 0;
}

BEGINdoAction_NoStrings
	bson_t *doc = NULL;
	instanceData *pData;
CODESTARTdoAction
	pthread_mutex_lock(&mutDoAct);
	pData = pWrkrData->pData;
	/* see if we are ready to proceed */
	if(pData->client == NULL) {
		CHKiRet(initMongoDB(pData, 0));
	}

	if(pData->tplName == NULL) {
		doc = getDefaultBSON(*(smsg_t**)pMsgData);
	} else {
		doc = BSONFromJSONObject(*(struct json_object **)pMsgData);
	}
	if(doc == NULL) {
		dbgprintf("ommongodb: error creating BSON doc\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	if (mongoc_collection_insert (pData->collection, MONGOC_INSERT_NONE, doc, NULL, &(pData->error) ) ) {
		pData->bErrMsgPermitted = 1;
	} else if (is_allowed_error_code(pData, pData->error.code)) {
		dbgprintf("ommongodb: insert error: allowing error code\n");
	} else {
		dbgprintf("ommongodb: insert error\n");
		reportMongoError(pData);
		/* close on insert error to permit resume */
		closeMongoDB(pData);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

finalize_it:
	pthread_mutex_unlock(&mutDoAct);
	if(doc != NULL)
		bson_destroy(doc);
ENDdoAction


static void setInstParamDefaults(instanceData *pData)
{
	pData->server = NULL;
	pData->port = NULL;
	pData->uristr = NULL;
	pData->ssl_ca = NULL;
	pData->ssl_cert = NULL;
	pData->uid = NULL;
	pData->pwd = NULL;
	pData->db = NULL;
	pData->collection = NULL;
	pData->tplName = NULL;
	memset (pData->allowed_error_codes, 0, 256 * sizeof(uint32_t));
	pData->allowed_error_codes_nbr = 0;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	dbgprintf("ommongodb: Getting configuration.\n");
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	dbgprintf("ommongodb: Parsing configuration directives.\n");
	CODE_STD_STRING_REQUESTnewActInst(1)
	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "uristr")) {
			pData->uristr = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "server")) {
			pData->server = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "serverport")) {
			pData->port = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "db")) {
			pData->db = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "collection")) {
			pData->collection_name = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "ssl_ca")) {
			pData->ssl_ca = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "ssl_cert")) {
			pData->ssl_cert = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "uid")) {
			pData->uid = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "pwd")) {
			pData->pwd = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "allowed_error_codes")) {
			const int maxerrcodes = sizeof(pData->allowed_error_codes) / sizeof(uint32_t);
			pData->allowed_error_codes_nbr = pvals[i].val.d.ar->nmemb;
			if(pData->allowed_error_codes_nbr > maxerrcodes) {
				parser_errmsg("ommongodb: %d allowed_error_codes given, but max "
					"supported number is %d. Only the first %d error codes will "
					"be accepted", pData->allowed_error_codes_nbr, maxerrcodes, maxerrcodes);
				pData->allowed_error_codes_nbr = maxerrcodes;
			}
			for(int j = 0 ; j <  pData->allowed_error_codes_nbr ; ++j) {
				const char *const str = es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
				assert(str != NULL);
				pData->allowed_error_codes[j] = (unsigned)atoi(str);
				free((void*)str);
			}
		} else {
			dbgprintf("ommongodb: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

	if(pData->tplName == NULL) {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, NULL, OMSR_TPL_AS_MSG));
	} else {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, ustrdup(pData->tplName),
				     OMSR_TPL_AS_JSON));
		CHKmalloc(pData->json_tokener = json_tokener_new());
	}

	if(pData->db == NULL)
		CHKmalloc(pData->db = (char*)strdup("syslog"));
	if(pData->collection_name == NULL)
		CHKmalloc(pData->collection_name = (char*)strdup("log"));

	/*
	 * If we don't have a uristr, we need to build it
	 */
	dbgprintf("ommongodb: Checking the uristr.\n");
	if(pData->uristr == NULL){
		dbgprintf("ommongodb: No uristr, building one.\n");
		char* tmp = NULL;
		if(pData->server == NULL)
			CHKmalloc(pData->server = (char*)strdup("127.0.0.1"));
		if(pData->port == NULL)
			CHKmalloc(pData->port = (char*)strdup("27017"));

		/* We need to calculate the total length of the connection uri.
		 * We will let it readable and let gcc do the optimisation for us.
		 */
		size_t server = strlen(pData->server);
		size_t port = strlen(pData->port);
		size_t uid = 0;
		size_t pwd = 0;
		size_t uri_len = strlen("mongodb://") + server + port + 2;
		if(pData->uid && pData->pwd){
			uid = strlen(pData->uid);
			pwd = strlen(pData->pwd);
			uri_len += uid + pwd + 2;
		}
		if(pData->ssl_ca && pData->ssl_cert)
			uri_len += strlen("?ssl=true"); /* "?ssl=true" & "&ssl=true" are the same size */

		/*
		 * Formatting string "by hand" is a lot faster on execution than a snprintf for example.
		 */
		CHKmalloc(pData->uristr = malloc(uri_len + 1));
		tmp = stpncpy(pData->uristr, "mongodb://", 10);
		if(pData->uid && pData->pwd){
			dbgprintf("ommongodb: Adding uid & pwd to uristr.\n");
			tmp = stpncpy(tmp, pData->uid, uid);
			*tmp = ':';
			++tmp;
			tmp = stpncpy(tmp, pData->pwd, pwd);
			*tmp = '@';
			++tmp;
		}
		dbgprintf("ommongodb: Adding server & port to uristr.\n");
		tmp = stpncpy(tmp, pData->server, server);
		*tmp = ':';
		++tmp;
		tmp = stpncpy(tmp, pData->port, port);
		*tmp = '/';
		++tmp;
		if(pData->ssl_ca && pData->ssl_cert){
			dbgprintf("ommongodb: Adding ssl to uristr.\n");
			if(pData->uid && pData->pwd)
				tmp = stpncpy(tmp, "&ssl=true", 9);
			else
				tmp = stpncpy(tmp, "?ssl=true", 9);
		}
		*tmp = '\0';
	}
	dbgprintf("ommongodb: The uristr: %s\n", pData->uristr);
	dbgprintf("ommongodb: End of the configuration.\n");

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


NO_LEGACY_CONF_parseSelectorAct


BEGINmodExit
CODESTARTmodExit
	objRelease(datetime, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
ENDqueryEtryPt

BEGINmodInit()
	rsRetVal localRet;
	rsRetVal (*pomsrGetSupportedTplOpts)(unsigned long *pOpts);
	unsigned long opts;
	int bJSONPassingSupported;
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	INITChkCoreFeature(bCoreSupportsBatching, CORE_FEATURE_BATCHING);
	DBGPRINTF("ommongodb: module compiled with rsyslog version %s.\n", VERSION);

	/* check if the rsyslog core supports parameter passing code */
	bJSONPassingSupported = 0;
	localRet = pHostQueryEtryPt((uchar*)"OMSRgetSupportedTplOpts",
				    &pomsrGetSupportedTplOpts);
	if(localRet == RS_RET_OK) {
		/* found entry point, so let's see if core supports msg passing */
		CHKiRet((*pomsrGetSupportedTplOpts)(&opts));
		if(opts & OMSR_TPL_AS_JSON)
			bJSONPassingSupported = 1;
	} else if(localRet != RS_RET_ENTRY_POINT_NOT_FOUND) {
		ABORT_FINALIZE(localRet); /* Something else went wrong, not acceptable */
	}
	if(!bJSONPassingSupported) {
		DBGPRINTF("ommongodb: JSON-passing is not supported by rsyslog core, "
			  "can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_JSON_PASSING);
	}
ENDmodInit
