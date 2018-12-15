/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *
 * omamqp1.c
 *
 * This output plugin enables rsyslog to send messages to an AMQP 1.0 protocol
 * compliant message bus.
 *
 * AMQP glue code Copyright (C) 2015-2016 Kenneth A. Giusti
 * <kgiusti@gmail.com>
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
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"

#include <pthread.h>
#include <time.h>
#include <proton/reactor.h>
#include <proton/handlers.h>
#include <proton/event.h>
#include <proton/connection.h>
#include <proton/session.h>
#include <proton/link.h>
#include <proton/delivery.h>
#include <proton/message.h>
#include <proton/transport.h>
#include <proton/sasl.h>
#include <proton/url.h>
#include <proton/version.h>


/* work-around issues in this contributed module */
#pragma GCC diagnostic ignored "-Wswitch-enum"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omamqp1")


/* internal structures
 */
DEF_OMOD_STATIC_DATA


/* Settings for the action */
typedef struct _configSettings {
	pn_url_t *url;      /* address of message bus */
	uchar *username;    /* authentication credentials */
	uchar *password;
	uchar *target;      /* endpoint for sent log messages */
	uchar *templateName;
	int bDisableSASL;   /* do not enable SASL? 0-enable 1-disable */
	int idleTimeout;    /* disconnect idle connection (seconds) */
	int reconnectDelay; /* pause before re-connecting (seconds) */
	int maxRetries;   /* drop unrouteable messages after maxRetries attempts */
} configSettings_t;


/* Control for communicating with the protocol engine thread */

typedef enum {          // commands sent to protocol thread
	COMMAND_DONE,       // marks command complete
	COMMAND_SEND,       // send a message to the message bus
	COMMAND_IS_READY,   // is the connection to the message bus active?
	COMMAND_SHUTDOWN    // cleanup and terminate protocol thread.
} commands_t;


typedef struct _threadIPC {
	pthread_mutex_t lock;
	pthread_cond_t condition;
	commands_t command;
	rsRetVal result;    // of command
	pn_message_t *message;
	uint64_t    tag;    // per message id
} threadIPC_t;


/* per-instance data */

typedef struct _instanceData {
	configSettings_t config;
	threadIPC_t ipc;
	int bThreadRunning;
	pthread_t thread_id;
	pn_reactor_t *reactor;
	pn_handler_t *handler;
	pn_message_t *message;
	int log_count;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;


/* glue code */

typedef void dispatch_t(pn_handler_t *, pn_event_t *, pn_event_type_t);

static void _init_thread_ipc(threadIPC_t *pIPC);
static void _clean_thread_ipc(threadIPC_t *ipc);
static void _init_config_settings(configSettings_t *pConfig);
static void _clean_config_settings(configSettings_t *pConfig);
static rsRetVal _shutdown_thread(instanceData *pData);
static rsRetVal _new_handler(pn_handler_t **handler,
	                         pn_reactor_t *reactor,
	                         dispatch_t *dispatcher,
	                         configSettings_t *config,
	                         threadIPC_t *ipc);
static void _del_handler(pn_handler_t *handler);
static rsRetVal _launch_protocol_thread(instanceData *pData);
static rsRetVal _shutdown_thread(instanceData *pData);
static rsRetVal _issue_command(threadIPC_t *ipc,
	                           pn_reactor_t *reactor,
	                           commands_t command,
	                           pn_message_t *message);
static void dispatcher(pn_handler_t *handler,
	                   pn_event_t *event,
	                   pn_event_type_t type);


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "host", eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "target", eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "username", eCmdHdlrGetWord, 0 },
	{ "password", eCmdHdlrGetWord, 0 },
	{ "template", eCmdHdlrGetWord, 0 },
	{ "idleTimeout", eCmdHdlrNonNegInt, 0 },
	{ "reconnectDelay", eCmdHdlrPositiveInt, 0 },
	{ "maxRetries", eCmdHdlrNonNegInt, 0 },
	{ "disableSASL", eCmdHdlrInt, 0 }
};
static struct cnfparamblk actpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	actpdescr
};


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
{
	if (eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
}
ENDisCompatibleWithFeature


BEGINcreateInstance
CODESTARTcreateInstance
{
	memset(pData, 0, sizeof(instanceData));
	_init_config_settings(&pData->config);
	_init_thread_ipc(&pData->ipc);
}
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINfreeInstance
CODESTARTfreeInstance
{
	_shutdown_thread(pData);
	_clean_config_settings(&pData->config);
	_clean_thread_ipc(&pData->ipc);
	if (pData->reactor) pn_decref(pData->reactor);
	if (pData->handler) pn_decref(pData->handler);
	if (pData->message) pn_decref(pData->message);
}
ENDfreeInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
{
	configSettings_t *cfg = &pData->config;
	dbgprintf("omamqp1:\n");
	dbgprintf("  host=%s\n", pn_url_str(cfg->url));
	dbgprintf("  username=%s\n", cfg->username);
	//dbgprintf("  password=%s\n", pData->password);
	dbgprintf("  target=%s\n", cfg->target);
	dbgprintf("  template=%s\n", cfg->templateName);
	dbgprintf("  disableSASL=%d\n", cfg->bDisableSASL);
	dbgprintf("  idleTimeout=%d\n", cfg->idleTimeout);
	dbgprintf("  reconnectDelay=%d\n", cfg->reconnectDelay);
	dbgprintf("  maxRetries=%d\n", cfg->maxRetries);
	dbgprintf("  running=%d\n", pData->bThreadRunning);
}
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
{
	// is the link active?
	instanceData *pData = pWrkrData->pData;
	iRet = _issue_command(&pData->ipc, pData->reactor, COMMAND_IS_READY, NULL);
}
ENDtryResume


BEGINbeginTransaction
CODESTARTbeginTransaction
{
	DBGPRINTF("omamqp1: beginTransaction\n");
	instanceData *pData = pWrkrData->pData;
	pData->log_count = 0;
	if (pData->message) pn_decref(pData->message);
	pData->message = pn_message();
	CHKmalloc(pData->message);
	pn_data_t *body = pn_message_body(pData->message);
	pn_data_put_list(body);
	pn_data_enter(body);
}
finalize_it:
ENDbeginTransaction


BEGINdoAction
CODESTARTdoAction
{
	DBGPRINTF("omamqp1: doAction\n");
	instanceData *pData = pWrkrData->pData;
	if (!pData->message) ABORT_FINALIZE(RS_RET_OK);
	pn_bytes_t msg = pn_bytes(strlen((const char *)ppString[0]),
		(const char *)ppString[0]);
	pn_data_t *body = pn_message_body(pData->message);
	pn_data_put_string(body, msg);
	pData->log_count++;
	iRet = RS_RET_DEFER_COMMIT;
}
finalize_it:
ENDdoAction


BEGINendTransaction
CODESTARTendTransaction
{
	DBGPRINTF("omamqp1: endTransaction\n");
	instanceData *pData = pWrkrData->pData;
	if (!pData->message) ABORT_FINALIZE(RS_RET_OK);
	pn_data_t *body = pn_message_body(pData->message);
	pn_data_exit(body);
	pn_message_t *message = pData->message;
	pData->message = NULL;
	if (pData->log_count > 0) {
		CHKiRet(_issue_command(&pData->ipc, pData->reactor, COMMAND_SEND, message));
	} else {
		DBGPRINTF("omamqp1: no log messages to send\n");
		pn_decref(message);
	}
}
finalize_it:
ENDendTransaction


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	configSettings_t *cs;
CODESTARTnewActInst
{
	if ((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	cs = &pData->config;

	CODE_STD_STRING_REQUESTnewActInst(1);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if (!pvals[i].bUsed)
			continue;
		if (!strcmp(actpblk.descr[i].name, "host")) {
			char *u = es_str2cstr(pvals[i].val.d.estr, NULL);
			cs->url = pn_url_parse(u);
			if (!cs->url) {
				LogError(0, RS_RET_CONF_PARSE_ERROR, "omamqp1: Invalid host URL configured: '%s'", u);
				free(u);
				ABORT_FINALIZE(RS_RET_CONF_PARSE_ERROR);
			}
			free(u);
		} else if (!strcmp(actpblk.descr[i].name, "template")) {
			cs->templateName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "target")) {
			cs->target = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "username")) {
			cs->username = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "password")) {
			cs->password = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "reconnectDelay")) {
			cs->reconnectDelay = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "idleTimeout")) {
			cs->idleTimeout = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "maxRetries")) {
			cs->maxRetries = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "disableSASL")) {
			cs->bDisableSASL = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("omamqp1: program error, unrecognized param '%s', ignored.\n",
			actpblk.descr[i].name);
		}
	}

	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((cs->templateName == NULL)
		? "RSYSLOG_FileFormat" : (char*)cs->templateName), OMSR_NO_RQD_TPL_OPTS));

	// once configuration is known, start the protocol engine thread
	pData->reactor = pn_reactor();
	CHKmalloc(pData->reactor);
	CHKiRet(_new_handler(&pData->handler, pData->reactor, dispatcher, &pData->config, &pData->ipc));
	CHKiRet(_launch_protocol_thread(pData));
}
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
	CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
	CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
	CODEqueryEtryPt_TXIF_OMOD_QUERIES   /* use transaction interface */
	CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
{
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current
						interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	INITChkCoreFeature(bCoreSupportsBatching, CORE_FEATURE_BATCHING);
	DBGPRINTF("omamqp1: module compiled with rsyslog version %s.\n", VERSION);
	DBGPRINTF("omamqp1: %susing transactional output interface.\n", bCoreSupportsBatching ? "" : "not ");
}
ENDmodInit


///////////////////////////////////////
// All the Proton-specific glue code //
///////////////////////////////////////


/* state maintained by the protocol thread */

typedef struct {
	const configSettings_t *config;
	threadIPC_t   *ipc;
	pn_reactor_t *reactor;  // AMQP 1.0 protocol engine
	pn_connection_t *conn;
	pn_link_t *sender;
	pn_delivery_t *delivery;
	char *encode_buffer;
	size_t buffer_size;
	uint64_t tag;
	int msgs_sent;
	int msgs_settled;
	int retries;
	sbool stopped;
} protocolState_t;

// protocolState_t is embedded in the engine handler
#define PROTOCOL_STATE(eh) ((protocolState_t *) pn_handler_mem(eh))


static void _init_config_settings(configSettings_t *pConfig)
{
	memset(pConfig, 0, sizeof(configSettings_t));
	pConfig->reconnectDelay = 5;
	pConfig->maxRetries = 10;
}


static void _clean_config_settings(configSettings_t *pConfig)
{
	if (pConfig->url) pn_url_free(pConfig->url);
	if (pConfig->username) free(pConfig->username);
	if (pConfig->password) free(pConfig->password);
	if (pConfig->target) free(pConfig->target);
	if (pConfig->templateName) free(pConfig->templateName);
	memset(pConfig, 0, sizeof(configSettings_t));
}


static void _init_thread_ipc(threadIPC_t *pIPC)
{
	memset(pIPC, 0, sizeof(threadIPC_t));
	pthread_mutex_init(&pIPC->lock, NULL);
	pthread_cond_init(&pIPC->condition, NULL);
	pIPC->command = COMMAND_DONE;
	pIPC->result = RS_RET_OK;
}

static void _clean_thread_ipc(threadIPC_t *ipc)
{
	pthread_cond_destroy(&ipc->condition);
	pthread_mutex_destroy(&ipc->lock);
}


// create a new handler for the engine and set up the protocolState
static rsRetVal _new_handler(pn_handler_t **handler,
				pn_reactor_t *reactor,
				dispatch_t *dispatch,
				configSettings_t *config,
				threadIPC_t *ipc)
{
	DEFiRet;
	*handler = pn_handler_new(dispatch, sizeof(protocolState_t), _del_handler);
	CHKmalloc(*handler);
	pn_handler_add(*handler, pn_handshaker());
	protocolState_t *pState = PROTOCOL_STATE(*handler);
	memset(pState, 0, sizeof(protocolState_t));
	pState->buffer_size = 64;  // will grow if not enough
	pState->encode_buffer = (char *)malloc(pState->buffer_size);
	CHKmalloc(pState->encode_buffer);
	pState->reactor = reactor;
	pState->stopped = false;
	// these are _references_, don't free them:
	pState->config = config;
	pState->ipc = ipc;

finalize_it:
	RETiRet;
}


// in case existing buffer too small
static rsRetVal _grow_buffer(protocolState_t *pState)
{
	DEFiRet;
	pState->buffer_size *= 2;
	free(pState->encode_buffer);
	pState->encode_buffer = (char *)malloc(pState->buffer_size);
	CHKmalloc(pState->encode_buffer);

finalize_it:
	RETiRet;
}


/* release the pn_handler_t instance. Do not call this directly,
 * it will be called by the reactor when all references to the
 * handler have been released.
 */
static void _del_handler(pn_handler_t *handler)
{
	protocolState_t *pState = PROTOCOL_STATE(handler);
	if (pState->encode_buffer) free(pState->encode_buffer);
}


// Close the sender and its parent session and connection
static void _close_connection(protocolState_t *ps)
{
	if (ps->sender) {
		pn_link_close(ps->sender);
		pn_session_close(pn_link_session(ps->sender));
	}
	if (ps->conn) pn_connection_close(ps->conn);
}

static void _abort_command(protocolState_t *ps)
{
	threadIPC_t *ipc = ps->ipc;

	pthread_mutex_lock(&ipc->lock);
	switch (ipc->command) {
	case COMMAND_SEND:
	  dbgprintf("omamqp1: aborted the message send in progress\n");
	  CASE_FALLTHROUGH
	case COMMAND_IS_READY:
	  ipc->result = RS_RET_SUSPENDED;
	  ipc->command = COMMAND_DONE;
	  pthread_cond_signal(&ipc->condition);
	  break;
	case COMMAND_SHUTDOWN: // cannot be aborted
	case COMMAND_DONE:
	  break;
	}
	pthread_mutex_unlock(&ipc->lock);
}


// log a protocol error received from the message bus
static void _log_error(const char *message, pn_condition_t *cond)
{
	const char *name = pn_condition_get_name(cond);
	const char *desc = pn_condition_get_description(cond);
	dbgprintf("omamqp1: %s %s:%s\n",
	          message,
	          (name) ? name : "<no name>",
	          (desc) ? desc : "<no description>");
}


/* is the link ready to send messages? */
static sbool _is_ready(pn_link_t *link)
{
	return (link
	        && pn_link_state(link) == (PN_LOCAL_ACTIVE | PN_REMOTE_ACTIVE)
	        && pn_link_credit(link) > 0);
}


/* Process each event emitted by the protocol engine */
static void dispatcher(pn_handler_t *handler, pn_event_t *event, pn_event_type_t type)
{
	protocolState_t *ps = PROTOCOL_STATE(handler);
	const configSettings_t *cfg = ps->config;

	//DBGPRINTF("omamqp1: Event received: %s\n", pn_event_type_name(type));

	switch (type) {

	case PN_LINK_REMOTE_OPEN:
	    DBGPRINTF("omamqp1: Message bus opened link.\n");
	    break;

	case PN_DELIVERY:
	    // has the message been delivered to the message bus?
	    if (ps->delivery) {
	        assert(ps->delivery == pn_event_delivery(event));
	        if (pn_delivery_updated(ps->delivery)) {
	            rsRetVal result = RS_RET_IDLE;
	            uint64_t rs = pn_delivery_remote_state(ps->delivery);
	            switch (rs) {
	            case PN_ACCEPTED:
	                DBGPRINTF("omamqp1: Message ACCEPTED by message bus\n");
	                result = RS_RET_OK;
	                break;
	            case PN_REJECTED:
	              dbgprintf("omamqp1: message bus rejected log message: invalid message - dropping\n");
	                // message bus considers this a 'bad message'. Cannot be redelivered.
	                // Likely a configuration error. Drop the message by returning OK
	                result = RS_RET_OK;
	                break;
	            case PN_RELEASED:
	            case PN_MODIFIED:
		// the message bus cannot accept the message.  This may be temporary - retry
		// up to maxRetries before dropping
	                if (++ps->retries >= cfg->maxRetries) {
	                  dbgprintf("omamqp1: message bus failed to accept message - dropping\n");
	                  result = RS_RET_OK;
	                } else {
	                  dbgprintf("omamqp1: message bus cannot accept message, retrying\n");
	                  result = RS_RET_SUSPENDED;
	                }
	                break;
	            case PN_RECEIVED:
	                // not finished yet, wait for next delivery update
	                break;
	            default:
	                // no other terminal states defined, so ignore anything else
	                dbgprintf("omamqp1: unknown delivery state=0x%lX, assuming message accepted\n",
	                          (unsigned long) pn_delivery_remote_state(ps->delivery));
	                result = RS_RET_OK;
	                break;
	            }

	            if (result != RS_RET_IDLE) {
	                // the command is complete
	                threadIPC_t *ipc = ps->ipc;
	                pthread_mutex_lock(&ipc->lock);
	                assert(ipc->command == COMMAND_SEND);
	                ipc->result = result;
	                ipc->command = COMMAND_DONE;
	                pthread_cond_signal(&ipc->condition);
	                pthread_mutex_unlock(&ipc->lock);
	                pn_delivery_settle(ps->delivery);
	                ps->delivery = NULL;
	                if (result == RS_RET_OK) {
	                  ps->retries = 0;
	                }
	            }
	        }
	    }
	    break;


	case PN_CONNECTION_BOUND:
	    if (!cfg->bDisableSASL) {
	        // force use of SASL, even allowing PLAIN authentication
	        pn_sasl_t *sasl = pn_sasl(pn_event_transport(event));
#if PN_VERSION_MAJOR == 0 && PN_VERSION_MINOR >= 10
	        pn_sasl_set_allow_insecure_mechs(sasl, true);
#else
	        // proton version <= 0.9 only supports PLAIN authentication
	        const char *user = cfg->username
	            ? (char *)cfg->username
	            : pn_url_get_username(cfg->url);
	        if (user) {
	            pn_sasl_plain(sasl, user, (cfg->password
	                                       ? (char *) cfg->password
	                                       : pn_url_get_password(cfg->url)));
	        }
#endif
	    }
	    if (cfg->idleTimeout) {
	        // configured as seconds, set as milliseconds
	        pn_transport_set_idle_timeout(pn_event_transport(event),
	                                      cfg->idleTimeout * 1000);
	    }
	    break;

	case PN_CONNECTION_UNBOUND:
	    DBGPRINTF("omamqp1: cleaning up connection resources\n");
	    pn_connection_release(pn_event_connection(event));
	    ps->conn = NULL;
	    ps->sender = NULL;
	    ps->delivery = NULL;
	    break;


	case PN_TRANSPORT_ERROR:
	    {
	        // TODO: if auth failure, does it make sense to retry???
	        pn_transport_t *tport = pn_event_transport(event);
	        pn_condition_t *cond = pn_transport_condition(tport);
	        if (pn_condition_is_set(cond)) {
	            _log_error("transport failure", cond);
	        }
	        dbgprintf("omamqp1: network transport failed, reconnecting...\n");
	        // the protocol thread will attempt to reconnect if it is not
	        // being shut down
	    }
	    break;

	default:
	    break;
	}
}


// Send a command to the protocol thread and
// wait for the command to complete
static rsRetVal _issue_command(threadIPC_t *ipc,
	                           pn_reactor_t *reactor,
	                           commands_t command,
	                           pn_message_t *message)
{
	DEFiRet;

	DBGPRINTF("omamqp1: Sending command %d to protocol thread\n", command);

	pthread_mutex_lock(&ipc->lock);

	if (message) {
	    assert(ipc->message == NULL);
	    ipc->message = message;
	}
	assert(ipc->command == COMMAND_DONE);
	ipc->command = command;
	pn_reactor_wakeup(reactor);
	while (ipc->command != COMMAND_DONE) {
	    pthread_cond_wait(&ipc->condition, &ipc->lock);
	}
	iRet = ipc->result;
	if (ipc->message) {
	    pn_decref(ipc->message);
	    ipc->message = NULL;
	}

	pthread_mutex_unlock(&ipc->lock);

	DBGPRINTF("omamqp1: Command %d completed, status=%d\n", command, iRet);
	RETiRet;
}


// check if a command needs processing
static void _poll_command(protocolState_t *ps)
{
	if (ps->stopped) return;

	threadIPC_t *ipc = ps->ipc;

	pthread_mutex_lock(&ipc->lock);

	switch (ipc->command) {

	case COMMAND_SHUTDOWN:
	    DBGPRINTF("omamqp1: Protocol thread processing shutdown command\n");
	    ps->stopped = true;
	    _close_connection(ps);
	    // wait for the shutdown to complete before ack'ing this command
	    break;

	case COMMAND_IS_READY:
	    DBGPRINTF("omamqp1: Protocol thread processing ready query command\n");
	    ipc->result = _is_ready(ps->sender)
	                  ? RS_RET_OK
	                  : RS_RET_SUSPENDED;
	    ipc->command = COMMAND_DONE;
	    pthread_cond_signal(&ipc->condition);
	    break;

	case COMMAND_SEND:
	    if (ps->delivery) break;  // currently processing this command
	    DBGPRINTF("omamqp1: Protocol thread processing send message command\n");
	    if (!_is_ready(ps->sender)) {
	        ipc->result = RS_RET_SUSPENDED;
	        ipc->command = COMMAND_DONE;
	        pthread_cond_signal(&ipc->condition);
	        break;
	    }

	    // send the message
	    ++ps->tag;
	    ps->delivery = pn_delivery(ps->sender,
	                               pn_dtag((const char *)&ps->tag, sizeof(ps->tag)));
	    pn_message_t *message = ipc->message;
	    assert(message);

	    int rc = 0;
	    size_t len = ps->buffer_size;
	    do {
	        rc = pn_message_encode(message, ps->encode_buffer, &len);
	        if (rc == PN_OVERFLOW) {
	            _grow_buffer(ps);
	            len = ps->buffer_size;
	        }
	    } while (rc == PN_OVERFLOW);

	    pn_link_send(ps->sender, ps->encode_buffer, len);
	    pn_link_advance(ps->sender);
	    ++ps->msgs_sent;
	    // command completes when remote updates the delivery (see PN_DELIVERY)
	    break;

	case COMMAND_DONE:
	    break;
	}

	pthread_mutex_unlock(&ipc->lock);
}

/* runs the protocol engine, allowing it to handle TCP socket I/O and timer
 * events in the background.
*/
static void *amqp1_thread(void *arg)
{
	DBGPRINTF("omamqp1: Protocol thread started\n");

	pn_handler_t *handler = (pn_handler_t *)arg;
	protocolState_t *ps = PROTOCOL_STATE(handler);
	const configSettings_t *cfg = ps->config;

	// have pn_reactor_process() exit after 5 sec to poll for commands
	pn_reactor_set_timeout(ps->reactor, 5000);
	pn_reactor_start(ps->reactor);

	while (!ps->stopped) {
	    // setup a connection:
	    const char *host = pn_url_get_host(cfg->url);
	    const char *port = pn_url_get_port(cfg->url);
	    if (!port) port = "5672";

#if PN_VERSION_MAJOR == 0 && PN_VERSION_MINOR >= 13
	    ps->conn = pn_reactor_connection_to_host(ps->reactor,
	                                             host,
	                                             port,
	                                             handler);
	    pn_connection_set_hostname(ps->conn, host);
#else
	    {
	        char host_addr[300];
	        ps->conn = pn_reactor_connection(ps->reactor, handler);
	        snprintf(host_addr, sizeof(host_addr), "%s:%s", host, port);
	        pn_connection_set_hostname(ps->conn, host_addr);
	    }
#endif
	    pn_connection_set_container(ps->conn, "rsyslogd-omamqp1");

#if PN_VERSION_MAJOR == 0 && PN_VERSION_MINOR >= 10
	    // proton version <= 0.9 did not support Cyrus SASL
	    const char *user = cfg->username
	        ? (char *)cfg->username
	        : pn_url_get_username(cfg->url);
	    if (user)
	        pn_connection_set_user(ps->conn, user);

	    const char *pword = cfg->password
	        ? (char *) cfg->password
	        : pn_url_get_password(cfg->url);
	    if (pword)
	        pn_connection_set_password(ps->conn, pword);
#endif
	    pn_connection_open(ps->conn);
	    pn_session_t *ssn = pn_session(ps->conn);
	    pn_session_open(ssn);
	    ps->sender = pn_sender(ssn, (char *)cfg->target);
	    pn_link_set_snd_settle_mode(ps->sender, PN_SND_UNSETTLED);
	    char *addr = (char *)ps->config->target;
	    pn_terminus_set_address(pn_link_target(ps->sender), addr);
	    pn_terminus_set_address(pn_link_source(ps->sender), addr);
	    pn_link_open(ps->sender);

	    // run the protocol engine until the connection closes or thread is shut down
	    sbool engine_running = true;
	    while (engine_running) {
	        engine_running = pn_reactor_process(ps->reactor);
	        _poll_command(ps);
	    }

	    DBGPRINTF("omamqp1: reactor finished\n");

	    _abort_command(ps);   // unblock main thread if necessary

	    // delay reconnectDelay seconds before re-connecting:
	    int delay = ps->config->reconnectDelay;
	    while (delay-- > 0 && !ps->stopped) {
	        srSleep(1, 0);
	        _poll_command(ps);
	    }
	}
	pn_reactor_stop(ps->reactor);

	// stop command is now done:
	threadIPC_t *ipc = ps->ipc;
	pthread_mutex_lock(&ipc->lock);
	ipc->result = RS_RET_OK;
	ipc->command = COMMAND_DONE;
	pthread_cond_signal(&ipc->condition);
	pthread_mutex_unlock(&ipc->lock);

	DBGPRINTF("omamqp1: Protocol thread stopped\n");

	return 0;
}


static rsRetVal _launch_protocol_thread(instanceData *pData)
{
	int rc;
	DBGPRINTF("omamqp1: Starting protocol thread\n");
	do {
	    rc = pthread_create(&pData->thread_id, NULL, amqp1_thread, pData->handler);
	    if (!rc) {
	        pData->bThreadRunning = true;
	        return RS_RET_OK;
	    }
	} while (rc == EAGAIN);
	LogError(0, RS_RET_SYS_ERR, "omamqp1: thread create failed: %d", rc);
	return RS_RET_SYS_ERR;
}

static rsRetVal _shutdown_thread(instanceData *pData)
{
	DEFiRet;

	if (pData->bThreadRunning) {
	    DBGPRINTF("omamqp1: shutting down thread...\n");
	    CHKiRet(_issue_command(&pData->ipc, pData->reactor, COMMAND_SHUTDOWN, NULL));
	    pthread_join(pData->thread_id, NULL);
	    pData->bThreadRunning = false;
	    DBGPRINTF("omamqp1: thread shutdown complete\n");
	}

finalize_it:
	RETiRet;
}



/* vi:set ai:
 */

