/* omrabbitmq.c
 *
 * This output plugin enables rsyslog to send messages to the RabbitMQ.
 *
 * Copyright 2012-2013 Vaclav Tomec
 * Copyright 2014 Rainer Gerhards
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Author: Vaclav Tomec
 * <vaclav.tomec@gmail.com>
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
#include <time.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"

#include <amqp.h>
#include <amqp_tcp_socket.h>

#define RABBITMQ_CHANNEL 1

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omrabbitmq")


/*
 * internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	/* here you need to define all action-specific data. A record of type
	 * instanceData will be handed over to each instance of the action. Keep
	 * in mind that there may be several invocations of the same type of action
	 * inside rsyslog.conf, and this is what keeps them apart. Do NOT use
	 * static data for this!
	 */
	pthread_mutex_t mutDoAct;
	amqp_connection_state_t conn;
	amqp_basic_properties_t props;
	uchar *host;
	int port;
	uchar *vhost;
	uchar *user;
	uchar *password;
	char *exchange;
	uchar *routing_key;
	uchar *tplName;
	char *exchange_type;
	int durable;
	int auto_delete;
	int delivery_mode;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "host", eCmdHdlrGetWord, 0 },
	{ "port", eCmdHdlrInt, 0 },
	{ "virtual_host", eCmdHdlrGetWord, 0 },
	{ "user", eCmdHdlrGetWord, 0 },
	{ "password", eCmdHdlrGetWord, 0 },
	{ "exchange", eCmdHdlrGetWord, 0 },
	{ "routing_key", eCmdHdlrGetWord, 0 },
	{ "template", eCmdHdlrGetWord, 0 },
	{ "exchange_type", eCmdHdlrGetWord, 0},
	{ "durable", eCmdHdlrNonNegInt, 0},
	{ "auto_delete", eCmdHdlrNonNegInt, 0},
	{ "delivery_mode", eCmdHdlrNonNegInt, 0}
};
static struct cnfparamblk actpblk =
	{
		CNFPARAMBLK_VERSION,
		sizeof(actpdescr)/sizeof(struct cnfparamdescr),
		actpdescr
	};


/*
 * Report general error
 */
static int
die_on_error(int x, char const *context)
{
	int retVal = 0; // false

	if (x < 0) {
		const char *errstr = amqp_error_string2(-x);
		LogError(0, RS_RET_ERR, "omrabbitmq: %s: %s", context, errstr);

		retVal = 1; // true
	}

	return retVal;
}


/*
 * Report AMQP specific error
 */
static int
die_on_amqp_error(amqp_rpc_reply_t x, char const *context)
{
	int retVal = 1; // true

	switch (x.reply_type) {
	case AMQP_RESPONSE_NORMAL:
		retVal = 0; // false
		break;

	case AMQP_RESPONSE_NONE:
		LogError(0, RS_RET_ERR, "omrabbitmq: %s: missing RPC reply type!", context);
		break;

	case AMQP_RESPONSE_LIBRARY_EXCEPTION:
		LogError(0, RS_RET_ERR, "omrabbitmq: %s: %s", context, amqp_error_string2(x.library_error));
		break;

	case AMQP_RESPONSE_SERVER_EXCEPTION:
		switch (x.reply.id) {
		case AMQP_CONNECTION_CLOSE_METHOD: {
			amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
			LogError(0, RS_RET_ERR, "omrabbitmq: %s: server connection error %d, message: %.*s",
				context,
				m->reply_code,
				(int) m->reply_text.len, (char *) m->reply_text.bytes);
			break;
			}
		case AMQP_CHANNEL_CLOSE_METHOD: {
			amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
			LogError(0, RS_RET_ERR, "omrabbitmq: %s: server channel error %d, message: %.*s",
				context,
				m->reply_code,
				(int) m->reply_text.len, (char *) m->reply_text.bytes);
			break;
			}
		default:
			LogError(0, RS_RET_ERR, "omrabbitmq: %s: unknown server error, method "
					"id 0x%08X\n", context, x.reply.id);
			break;
		}
		break;

	}

	return retVal;
}


static amqp_bytes_t
cstring_bytes(const char *str)
{
	return str ? amqp_cstring_bytes(str) : amqp_empty_bytes;
}


static void
closeAMQPConnection(instanceData *pData)
{
	if (pData->conn != NULL) {
		die_on_amqp_error(amqp_channel_close(pData->conn, 1, AMQP_REPLY_SUCCESS), "amqp_channel_close");
		die_on_amqp_error(amqp_connection_close(pData->conn, AMQP_REPLY_SUCCESS), "amqp_connection_close");
		die_on_error(amqp_destroy_connection(pData->conn), "amqp_destroy_connection");

		pData->conn = NULL;
	}
}


/*
 * Initialize RabbitMQ connection
 */
static rsRetVal
initRabbitMQ(instanceData *pData)
{
	amqp_socket_t *asocket;
	amqp_exchange_declare_t edReq;
	DEFiRet;

	DBGPRINTF("omrabbitmq: trying connect to '%s' at port %d\n", pData->host, pData->port);

	pData->conn = amqp_new_connection();

	asocket = amqp_tcp_socket_new(pData->conn);
	if (!asocket) {
		LogError(0, RS_RET_ERR, "omrabbitmq: Error allocating tcp socket");

		pData->conn = NULL;
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	if (die_on_error(amqp_socket_open(asocket, (char*) pData->host, pData->port), "Opening socket")) {
		pData->conn = NULL;
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	if (die_on_amqp_error(amqp_login(pData->conn, (char*) pData->vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
	pData->user, pData->password),
		"Logging in")) {
		pData->conn = NULL;
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	amqp_channel_open(pData->conn, RABBITMQ_CHANNEL);
	if (die_on_amqp_error(amqp_get_rpc_reply(pData->conn), "Opening channel")) {
		pData->conn = NULL;
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	if(pData->exchange_type != NULL) {
		edReq.ticket = 0;
		edReq.exchange = amqp_cstring_bytes(pData->exchange);
		edReq.type = amqp_cstring_bytes(pData->exchange_type);
		edReq.passive = 0;
		edReq.durable = pData->durable;
		edReq.auto_delete = pData->auto_delete;
		edReq.internal = 0;
		edReq.nowait = 0;
		edReq.arguments = amqp_empty_table;

		amqp_simple_rpc_decoded(pData->conn, RABBITMQ_CHANNEL, AMQP_EXCHANGE_DECLARE_METHOD,
		AMQP_EXCHANGE_DECLARE_OK_METHOD, &edReq);
		if(die_on_amqp_error(amqp_get_rpc_reply(pData->conn), "Declaring exchange")) {
			pData->conn = NULL;
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}
	}

finalize_it:
	RETiRet;
}


BEGINcreateInstance
CODESTARTcreateInstance
	pthread_mutex_init(&pData->mutDoAct, NULL);
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


BEGINfreeInstance
CODESTARTfreeInstance
	/* this is a cleanup callback. All dynamically-allocated resources
	 * in instance data must be cleaned up here. Prime examples are
	 * malloc()ed memory, file & database handles and the like.
	 */
	closeAMQPConnection(pData);
	free(pData->host);
	free(pData->vhost);
	free(pData->user);
	free(pData->password);
	free(pData->exchange);
	free(pData->routing_key);
	free(pData->tplName);
	free(pData->exchange_type);
	pthread_mutex_destroy(&pData->mutDoAct);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance

BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	/* permits to spit out some debug info */
	dbgprintf("omrabbitmq\n");
	dbgprintf("\thost='%s'\n", pData->host);
	dbgprintf("\tport=%d\n", pData->port);
	dbgprintf("\tvirtual_host='%s'\n", pData->vhost);
	dbgprintf("\tuser='%s'\n", pData->user == NULL ? (uchar*)"(not configured)" : pData->user);
	dbgprintf("\tpassword=(%sconfigured)\n", pData->password == NULL ? "not " : "");
	dbgprintf("\texchange='%s'\n", pData->exchange);
	dbgprintf("\trouting_key='%s'\n", pData->routing_key);
	dbgprintf("\ttemplate='%s'\n", pData->tplName);
	dbgprintf("\texchange_type='%s'\n", pData->exchange_type);
	dbgprintf("\tauto_delete=%d\n", pData->auto_delete);
	dbgprintf("\tdurable=%d\n", pData->durable);
	dbgprintf("\tdelivery_mode=%d\n", pData->delivery_mode);
ENDdbgPrintInstInfo


BEGINtryResume
	instanceData *pData = pWrkrData->pData;
CODESTARTtryResume
	/* this is called when an action has been suspended and the
	 * rsyslog core tries to resume it. The action must then
	 * retry (if possible) and report RS_RET_OK if it succeeded
	 * or RS_RET_SUSPENDED otherwise.
	 * Note that no data can be written in this callback, as it is
	 * not present. Prime examples of what can be retried are
	 * reconnects to remote hosts, reconnects to database,
	 * opening of files and the like.
	 * If there is no retry-type of operation, the action may
	 * return RS_RET_OK, so that it will get called on its doAction
	 * entry point (where it receives data), retries there, and
	 * immediately returns RS_RET_SUSPENDED if that does not work
	 * out. This disables some optimizations in the core's retry logic,
	 * but is a valid and expected behaviour. Note that it is also OK
	 * for the retry entry point to return OK but the immediately following
	 * doAction call to fail. In real life, for example, a buggy com line
	 * may cause such behaviour.
	 * Note that there is no guarantee that the core will very quickly
	 * call doAction after the retry succeeded. Today, it does, but that may
	 * not always be the case.
	 */

	pthread_mutex_lock(&pData->mutDoAct);
	if (pData->conn == NULL) {
		iRet = initRabbitMQ(pData);
	}
	pthread_mutex_unlock(&pData->mutDoAct);

ENDtryResume


BEGINdoAction
	instanceData *pData = pWrkrData->pData;
CODESTARTdoAction
	/* this is where you receive the message and need to carry out the
	 * action. Data is provided in ppString[i] where 0 <= i <= num of strings
	 * requested.
	 * Return RS_RET_OK if all goes well, RS_RET_SUSPENDED if the action can
	 * currently not complete, or an error code or RS_RET_DISABLED. The later
	 * two should only be returned if there is no hope that the action can be
	 * restored unless an rsyslog restart (prime example is an invalid config).
	 * Error code or RS_RET_DISABLED permanently disables the action, up to
	 * the next restart.
	 */

	amqp_bytes_t body_bytes;

	pthread_mutex_lock(&pData->mutDoAct);
	if (pData->conn == NULL) {
		CHKiRet(initRabbitMQ(pData));
	}

	body_bytes = amqp_cstring_bytes((char *)ppString[0]);

	if (die_on_error(amqp_basic_publish(pData->conn, RABBITMQ_CHANNEL,
			cstring_bytes((char *) pData->exchange),
			cstring_bytes((char *) pData->routing_key),
			0, 0, &pData->props, body_bytes), "amqp_basic_publish")) {
		closeAMQPConnection(pData);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

finalize_it:
	pthread_mutex_unlock(&pData->mutDoAct);
ENDdoAction


static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->host = NULL;
	pData->port = 5672;
	pData->vhost = NULL;
	pData->user = NULL;
	pData->password = NULL;
	pData->exchange = NULL;
	pData->routing_key = NULL;
	pData->tplName = NULL;
	pData->exchange_type = NULL;
	pData->auto_delete = 0;
	pData->durable = 0;
	pData->delivery_mode = 2;
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

	CODE_STD_STRING_REQUESTparseSelectorAct(1)

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if (!pvals[i].bUsed)
			continue;
		if (!strcmp(actpblk.descr[i].name, "host")) {
			pData->host = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "port")) {
			pData->port = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "virtual_host")) {
			pData->vhost = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "user")) {
			pData->user = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "password")) {
			pData->password = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "exchange")) {
			pData->exchange = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "routing_key")) {
			pData->routing_key = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "exchange_type")) {
			pData->exchange_type = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "auto_delete")) {
			pData->auto_delete = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "durable")) {
			pData->durable = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "delivery_mode")) {
			pData->delivery_mode = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("omrabbitmq: program error, non-handled param '%s'\n", actpblk.descr[i].name);
		}
	}

	if (pData->host == NULL) {
		LogError(0, RS_RET_INVALID_PARAMS, "omrabbitmq module disabled: parameter host must be "
			"specified");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	if (pData->vhost == NULL) {
		LogError(0, RS_RET_INVALID_PARAMS, "omrabbitmq module disabled: parameter "
					"virtual_host must be specified");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	if (pData->user == NULL) {
		LogError(0, RS_RET_INVALID_PARAMS, "omrabbitmq module disabled: parameter user "
			"must be specified");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	if (pData->password == NULL) {
		LogError(0, RS_RET_INVALID_PARAMS, "omrabbitmq module disabled: parameter password "
				"must be specified");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	if (pData->exchange == NULL) {
		LogError(0, RS_RET_INVALID_PARAMS, "omrabbitmq module disabled: parameter exchange "
				"must be specified");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	if (pData->routing_key == NULL) {
		LogError(0, RS_RET_INVALID_PARAMS, "omrabbitmq module disabled: parameter "
					"routing_key must be specified");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	// RabbitMQ properties initialization
	memset(&pData->props, 0, sizeof pData->props);
	pData->props._flags = AMQP_BASIC_DELIVERY_MODE_FLAG;
	pData->props.delivery_mode = pData->delivery_mode;
	pData->props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
	pData->props.content_type = amqp_cstring_bytes("application/json");

	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((pData->tplName == NULL) ?
					    " StdJSONFmt" : (char*)pData->tplName),
		OMSR_NO_RQD_TPL_OPTS));

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


NO_LEGACY_CONF_parseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
	CODEqueryEtryPt_STD_OMOD_QUERIES
	CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
	CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
ENDmodInit
