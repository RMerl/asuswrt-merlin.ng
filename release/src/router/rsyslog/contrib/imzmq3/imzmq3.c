/* imzmq3.c
 *
 * This input plugin enables rsyslog to read messages from a ZeroMQ
 * queue.
 *
 * Copyright 2012 Talksum, Inc.
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
 * Authors:
 * David Kelly <davidk@talksum.com>
 * Hongfei Cheng <hongfeic@talksum.com>
 */


#include "config.h"
#include "rsyslog.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cfsysline.h"
#include "dirty.h"
#include "errmsg.h"
#include "glbl.h"
#include "module-template.h"
#include "msg.h"
#include "net.h"
#include "parser.h"
#include "prop.h"
#include "ruleset.h"
#include "srUtils.h"
#include "unicode-helper.h"

#include <czmq.h>

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imzmq3");

/* convienent symbols to denote a socket we want to bind
 * vs one we want to just connect to
 */
#define ACTION_CONNECT 1
#define ACTION_BIND    2

/* Module static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)
DEFobjCurrIf(ruleset)


/* ----------------------------------------------------------------------------
 * structs to describe sockets
 */
typedef struct _socket_type {
	const char* name;
	int type;
} socket_type;

/* more overkill, but seems nice to be consistent.*/
typedef struct _socket_action {
	const char* name;
	int action;
} socket_action;

typedef struct _poller_data {
	ruleset_t* ruleset;
	thrdInfo_t* thread;
} poller_data;


/* a linked-list of subscription topics */
typedef struct sublist_t {
	char* subscribe;
	struct sublist_t* next;
} sublist;

struct instanceConf_s {
	int type;
	int action;
	char* description;
	int sndHWM; /* if you want more than 2^32 messages, */
	int rcvHWM; /* then pass in 0 (the default). */
	char* identity;
	sublist* subscriptions;
	int sndBuf;
	int rcvBuf;
	int linger;
	int backlog;
	int sndTimeout;
	int rcvTimeout;
	int maxMsgSize;
	int rate;
	int recoveryIVL;
	int multicastHops;
	int reconnectIVL;
	int reconnectIVLMax;
	int ipv4Only;
	int affinity;
	uchar* pszBindRuleset;
	ruleset_t* pBindRuleset;
	struct instanceConf_s* next;

};

struct modConfData_s {
	rsconf_t* pConf;
	instanceConf_t* root;
	instanceConf_t* tail;
	int io_threads;
};
struct lstn_s {
	struct lstn_s* next;
	void* sock;
	ruleset_t* pRuleset;
};

/* ----------------------------------------------------------------------------
 *  Static definitions/initializations.
 */
static modConfData_t* runModConf = NULL;
static struct lstn_s* lcnfRoot = NULL;
static struct lstn_s* lcnfLast = NULL;
static prop_t* s_namep = NULL;
static zloop_t* s_zloop = NULL;
static zctx_t* s_context = NULL;
static socket_type socketTypes[] = {
	{"SUB", ZMQ_SUB },
	{"PULL", ZMQ_PULL },
	{"ROUTER", ZMQ_ROUTER },
	{"XSUB", ZMQ_XSUB }
};

static socket_action socketActions[] = {
	{"BIND", ACTION_BIND},
	{"CONNECT", ACTION_CONNECT},
};

static struct cnfparamdescr modpdescr[] = {
	{ "ioThreads", eCmdHdlrInt, 0 },
};

static struct cnfparamblk modpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	modpdescr
};

static struct cnfparamdescr inppdescr[] = {
	{ "description",         eCmdHdlrGetWord, 0 },
	{ "sockType",            eCmdHdlrGetWord, 0 },
	{ "subscribe",           eCmdHdlrGetWord, 0 },
	{ "ruleset",             eCmdHdlrGetWord, 0 },
	{ "action",              eCmdHdlrGetWord, 0 },
	{ "sndHWM",              eCmdHdlrInt,     0 },
	{ "rcvHWM",              eCmdHdlrInt,     0 },
	{ "identity",            eCmdHdlrGetWord, 0 },
	{ "sndBuf",              eCmdHdlrInt,     0 },
	{ "rcvBuf",              eCmdHdlrInt,     0 },
	{ "linger",              eCmdHdlrInt,     0 },
	{ "backlog",             eCmdHdlrInt,     0 },
	{ "sndTimeout",          eCmdHdlrInt,     0 },
	{ "rcvTimeout",          eCmdHdlrInt,     0 },
	{ "maxMsgSize",          eCmdHdlrInt,     0 },
	{ "rate",                eCmdHdlrInt,     0 },
	{ "recoveryIVL",         eCmdHdlrInt,     0 },
	{ "multicastHops",       eCmdHdlrInt,     0 },
	{ "reconnectIVL",        eCmdHdlrInt,     0 },
	{ "reconnectIVLMax",     eCmdHdlrInt,     0 },
	{ "ipv4Only",            eCmdHdlrInt,     0 },
	{ "affinity",            eCmdHdlrInt,     0 }
};

static struct cnfparamblk inppblk = {
	CNFPARAMBLK_VERSION,
	sizeof(inppdescr)/sizeof(struct cnfparamdescr),
	inppdescr
};

#include "im-helper.h" /* must be included AFTER the type definitions! */

/* ----------------------------------------------------------------------------
 * Helper functions
 */

/* get the name of a socket type, return the ZMQ_XXX type
 * or -1 if not a supported type (see above)
 */
static int getSocketType(char* name) {
	int type = -1;
	uint i;

	/* match name with known socket type */
	for(i=0; i<sizeof(socketTypes)/sizeof(socket_type); ++i) {
	    if( !strcmp(socketTypes[i].name, name) ) {
	        type = socketTypes[i].type;
	        break;
	    }
	}

	/* whine if no match was found. */
	if (type == -1)
	    LogError(0, NO_ERRCODE, "unknown type %s",name);

	return type;
}


static int getSocketAction(char* name) {
	int action = -1;
	uint i;

	/* match name with known socket action */
	for(i=0; i < sizeof(socketActions)/sizeof(socket_action); ++i) {
	    if(!strcmp(socketActions[i].name, name)) {
	        action = socketActions[i].action;
	        break;
	    }
	}

	/* whine if no matching action was found */
	if (action == -1)
	    LogError(0, NO_ERRCODE, "unknown action %s",name);

	return action;
}


static void setDefaults(instanceConf_t* info) {
	info->type            = -1;
	info->action          = -1;
	info->description     = NULL;
	info->sndHWM          = -1;
	info->rcvHWM          = -1;
	info->identity        = NULL;
	info->subscriptions   = NULL;
	info->pszBindRuleset  = NULL;
	info->pBindRuleset    = NULL;
	info->sndBuf          = -1;
	info->rcvBuf          = -1;
	info->linger          = -1;
	info->backlog         = -1;
	info->sndTimeout      = -1;
	info->rcvTimeout      = -1;
	info->maxMsgSize      = -1;
	info->rate            = -1;
	info->recoveryIVL     = -1;
	info->multicastHops   = -1;
	info->reconnectIVL    = -1;
	info->reconnectIVLMax = -1;
	info->ipv4Only        = -1;
	info->affinity        = -1;
	info->next            = NULL;
};

/* given a comma separated list of subscriptions, create a char* array of them
 * to set later
 */
static rsRetVal parseSubscriptions(char* subscribes, sublist** subList){
	char* tok = strtok(subscribes, ",");
	sublist* currentSub;
	sublist* head;
	DEFiRet;

	/* create empty list */
	CHKmalloc(*subList = (sublist*)MALLOC(sizeof(sublist)));
	head = *subList;
	head->next = NULL;
	head->subscribe=NULL;
	currentSub=head;

	if(tok) {
	    head->subscribe=strdup(tok);
	    for(tok=strtok(NULL, ","); tok!=NULL;tok=strtok(NULL, ",")) {
	        CHKmalloc(currentSub->next = (sublist*)MALLOC(sizeof(sublist)));
	        currentSub=currentSub->next;
	        currentSub->subscribe=strdup(tok);
	        currentSub->next=NULL;
	    }
	} else {
	    /* make empty subscription ie subscribe="" */
	    head->subscribe=strdup("");
	}
	/* TODO: temporary logging */
	currentSub = head;
	DBGPRINTF("imzmq3: Subscriptions:");
	for(currentSub = head; currentSub != NULL; currentSub=currentSub->next) {
	    DBGPRINTF("'%s'", currentSub->subscribe);
	}
	DBGPRINTF("\n");

finalize_it:
	RETiRet;
}

static rsRetVal validateConfig(instanceConf_t* info) {

	if (info->type == -1) {
	    LogError(0, RS_RET_INVALID_PARAMS,
	                    "you entered an invalid type");
	    return RS_RET_INVALID_PARAMS;
	}
	if (info->action == -1) {
	    LogError(0, RS_RET_INVALID_PARAMS,
	                    "you entered an invalid action");
	    return RS_RET_INVALID_PARAMS;
	}
	if (info->description == NULL) {
	    LogError(0, RS_RET_INVALID_PARAMS,
	                    "you didn't enter a description");
	    return RS_RET_INVALID_PARAMS;
	}
	if(info->type == ZMQ_SUB && info->subscriptions == NULL) {
	    LogError(0, RS_RET_INVALID_PARAMS,
	                    "SUB sockets need a subscription");
	    return RS_RET_INVALID_PARAMS;
	}
	if(info->type != ZMQ_SUB && info->subscriptions != NULL) {
	    LogError(0, RS_RET_INVALID_PARAMS,
	                    "only SUB sockets can have subscriptions");
	    return RS_RET_INVALID_PARAMS;
	}
	return RS_RET_OK;
}

static rsRetVal createContext(void) {
	if (s_context == NULL) {
	    DBGPRINTF("imzmq3: creating zctx...");
	    zsys_handler_set(NULL);
	    s_context = zctx_new();

	    if (s_context == NULL) {
	        LogError(0, RS_RET_INVALID_PARAMS,
	                        "zctx_new failed: %s",
	                        zmq_strerror(errno));
	        /* DK: really should do better than invalid params...*/
	        return RS_RET_INVALID_PARAMS;
	    }
	    DBGPRINTF("success!\n");
	    if (runModConf->io_threads > 1) {
	        DBGPRINTF("setting io worker threads to %d\n", runModConf->io_threads);
	        zctx_set_iothreads(s_context, runModConf->io_threads);
	    }
	}
	return RS_RET_OK;
}

static rsRetVal createSocket(instanceConf_t* info, void** sock) {
	int rv;
	sublist* sub;

	*sock = zsocket_new(s_context, info->type);
	if (!sock) {
	    LogError(0,
	                    RS_RET_INVALID_PARAMS,
	                    "zsocket_new failed: %s, for type %d",
	                    zmq_strerror(errno),info->type);
	    /* DK: invalid params seems right here */
	    return RS_RET_INVALID_PARAMS;
	}
	DBGPRINTF("imzmq3: socket of type %d created successfully\n", info->type)
	/* Set options *before* the connect/bind. */
	if (info->identity)             zsocket_set_identity(*sock, info->identity);
	if (info->sndBuf > -1)          zsocket_set_sndbuf(*sock, info->sndBuf);
	if (info->rcvBuf > -1)          zsocket_set_rcvbuf(*sock, info->rcvBuf);
	if (info->linger > -1)          zsocket_set_linger(*sock, info->linger);
	if (info->backlog > -1)         zsocket_set_backlog(*sock, info->backlog);
	if (info->sndTimeout > -1)      zsocket_set_sndtimeo(*sock, info->sndTimeout);
	if (info->rcvTimeout > -1)      zsocket_set_rcvtimeo(*sock, info->rcvTimeout);
	if (info->maxMsgSize > -1)      zsocket_set_maxmsgsize(*sock, info->maxMsgSize);
	if (info->rate > -1)            zsocket_set_rate(*sock, info->rate);
	if (info->recoveryIVL > -1)     zsocket_set_recovery_ivl(*sock, info->recoveryIVL);
	if (info->multicastHops > -1)   zsocket_set_multicast_hops(*sock, info->multicastHops);
	if (info->reconnectIVL > -1)    zsocket_set_reconnect_ivl(*sock, info->reconnectIVL);
	if (info->reconnectIVLMax > -1) zsocket_set_reconnect_ivl_max(*sock, info->reconnectIVLMax);
	if (info->ipv4Only > -1)        zsocket_set_ipv4only(*sock, info->ipv4Only);
	if (info->affinity > -1)        zsocket_set_affinity(*sock, info->affinity);
	if (info->sndHWM > -1 )         zsocket_set_sndhwm(*sock, info->sndHWM);
	if (info->rcvHWM > -1 )         zsocket_set_rcvhwm(*sock, info->rcvHWM);
	/* Set subscriptions.*/
	if (info->type == ZMQ_SUB) {
	    for(sub = info->subscriptions; sub!=NULL; sub=sub->next) {
	        zsocket_set_subscribe(*sock, sub->subscribe);
	    }
	}

	/* Do the bind/connect... */
	if (info->action==ACTION_CONNECT) {
	    rv = zsocket_connect(*sock, "%s", info->description);
	    if (rv == -1) {
	        LogError(0,
	                        RS_RET_INVALID_PARAMS,
	                        "zmq_connect using %s failed: %s",
	                        info->description, zmq_strerror(errno));
	        return RS_RET_INVALID_PARAMS;
	    }
	    DBGPRINTF("imzmq3: connect for %s successful\n",info->description);
	} else {
	    rv = zsocket_bind(*sock, "%s", info->description);
	    if (rv == -1) {
	        LogError(0,
	                        RS_RET_INVALID_PARAMS,
	                        "zmq_bind using %s failed: %s",
	                        info->description, zmq_strerror(errno));
	        return RS_RET_INVALID_PARAMS;
	    }
	    DBGPRINTF("imzmq3: bind for %s successful\n",info->description);
	}
	return RS_RET_OK;
}

/* ----------------------------------------------------------------------------
 * Module endpoints
 */


/* add an actual endpoint
 */
static rsRetVal createInstance(instanceConf_t** pinst) {
	DEFiRet;
	instanceConf_t* inst;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));

	/* set defaults into new instance config struct */
	setDefaults(inst);

	/* add this to the config */
	if (runModConf->root == NULL || runModConf->tail == NULL) {
	    runModConf->tail = runModConf->root = inst;
	} else {
	    runModConf->tail->next = inst;
	    runModConf->tail       = inst;
	}
	*pinst = inst;
finalize_it:
	RETiRet;
}

static rsRetVal createListener(struct cnfparamvals* pvals) {
	instanceConf_t* inst;
	int i;
	DEFiRet;

	CHKiRet(createInstance(&inst));
	for(i = 0 ; i < inppblk.nParams ; ++i) {
	    if(!pvals[i].bUsed)
	        continue;
	    if(!strcmp(inppblk.descr[i].name, "ruleset")) {
	        inst->pszBindRuleset = (uchar *)es_str2cstr(pvals[i].val.d.estr, NULL);
	    } else if(!strcmp(inppblk.descr[i].name, "description")) {
	        inst->description = es_str2cstr(pvals[i].val.d.estr, NULL);
	    } else if(!strcmp(inppblk.descr[i].name, "sockType")){
	        inst->type = getSocketType(es_str2cstr(pvals[i].val.d.estr, NULL));
	    } else if(!strcmp(inppblk.descr[i].name, "action")){
	        inst->action = getSocketAction(es_str2cstr(pvals[i].val.d.estr, NULL));
	    } else if(!strcmp(inppblk.descr[i].name, "sndHWM")) {
	        inst->sndHWM = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "rcvHWM")) {
	        inst->rcvHWM = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "subscribe")) {
	        char *subscribes = es_str2cstr(pvals[i].val.d.estr, NULL);
	        rsRetVal ret = parseSubscriptions(subscribes, &inst->subscriptions);
	        free(subscribes);
	        CHKiRet(ret);
	    } else if(!strcmp(inppblk.descr[i].name, "identity")){
	        inst->identity = es_str2cstr(pvals[i].val.d.estr, NULL);
	    } else if(!strcmp(inppblk.descr[i].name, "sndBuf")) {
	        inst->sndBuf = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "rcvBuf")) {
	        inst->rcvBuf = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "linger")) {
	        inst->linger = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "backlog")) {
	        inst->backlog = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "sndTimeout")) {
	        inst->sndTimeout = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "rcvTimeout")) {
	        inst->rcvTimeout = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "maxMsgSize")) {
	        inst->maxMsgSize = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "rate")) {
	        inst->rate = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "recoveryIVL")) {
	        inst->recoveryIVL = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "multicastHops")) {
	        inst->multicastHops = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "reconnectIVL")) {
	        inst->reconnectIVL = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "reconnectIVLMax")) {
	        inst->reconnectIVLMax = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "ipv4Only")) {
	        inst->ipv4Only = (int) pvals[i].val.d.n;
	    } else if(!strcmp(inppblk.descr[i].name, "affinity")) {
	        inst->affinity = (int) pvals[i].val.d.n;
	    } else {
	        LogError(0, NO_ERRCODE, "imzmq3: program error, non-handled "
	                        "param '%s'\n", inppblk.descr[i].name);
	    }

	}
finalize_it:
	RETiRet;
}

static rsRetVal addListener(instanceConf_t* inst){
	/* create the socket */
	void* sock;
	struct lstn_s* newcnfinfo;
	DEFiRet;

	CHKiRet(createSocket(inst, &sock));

	/* now create new lstn_s struct */
	CHKmalloc(newcnfinfo=(struct lstn_s*)MALLOC(sizeof(struct lstn_s)));
	newcnfinfo->next = NULL;
	newcnfinfo->sock = sock;
	newcnfinfo->pRuleset = inst->pBindRuleset;

	/* add this struct to the global */
	if(lcnfRoot == NULL) {
	    lcnfRoot = newcnfinfo;
	}
	if(lcnfLast == NULL) {
	    lcnfLast = newcnfinfo;
	} else {
	    lcnfLast->next = newcnfinfo;
	    lcnfLast = newcnfinfo;
	}

finalize_it:
	RETiRet;
}

static int handlePoll(zloop_t __attribute__((unused)) * loop, zmq_pollitem_t *poller, void* pd) {
	smsg_t* pMsg;
	poller_data* pollerData = (poller_data*)pd;

	char* buf = zstr_recv(poller->socket);
	if (msgConstruct(&pMsg) == RS_RET_OK) {
	    MsgSetRawMsg(pMsg, buf, strlen(buf));
	    MsgSetInputName(pMsg, s_namep);
	    MsgSetHOSTNAME(pMsg, glbl.GetLocalHostName(), ustrlen(glbl.GetLocalHostName()));
	    MsgSetRcvFrom(pMsg, glbl.GetLocalHostNameProp());
	    MsgSetRcvFromIP(pMsg, glbl.GetLocalHostIP());
	    MsgSetMSGoffs(pMsg, 0);
	    MsgSetFlowControlType(pMsg, eFLOWCTL_NO_DELAY);
	    MsgSetRuleset(pMsg, pollerData->ruleset);
	    pMsg->msgFlags = NEEDS_PARSING | PARSE_HOSTNAME;
	    submitMsg2(pMsg);
	}

	/* gotta free the string returned from zstr_recv() */
	free(buf);

	if( pollerData->thread->bShallStop == TRUE) {
	    /* a handler that returns -1 will terminate the
	       czmq reactor loop
	    */
	    return -1;
	}

	return 0;
}

/* called when runInput is called by rsyslog
 */
static rsRetVal rcv_loop(thrdInfo_t* pThrd){
	size_t          n_items = 0;
	size_t          i;
	int             rv;
	zmq_pollitem_t* items = NULL;
	poller_data*    pollerData = NULL;
	struct lstn_s*  current;
	instanceConf_t* inst;
	DEFiRet;

	/* now add listeners. This actually creates the sockets, etc... */
	for (inst = runModConf->root; inst != NULL; inst=inst->next) {
	    addListener(inst);
	}
	if (lcnfRoot == NULL) {
	    LogError(0, NO_ERRCODE, "imzmq3: no listeners were "
	                    "started, input not activated.\n");
	    ABORT_FINALIZE(RS_RET_NO_RUN);
	}

	/* count the # of items first */
	for(current=lcnfRoot;current!=NULL;current=current->next)
	    n_items++;

	/* make arrays of pollitems, pollerdata so they are easy to delete later */

	/* create the poll items*/
	CHKmalloc(items = (zmq_pollitem_t*)MALLOC(sizeof(zmq_pollitem_t)*n_items));

	/* create poller data (stuff to pass into the zmq closure called when we get a message)*/
	CHKmalloc(pollerData = (poller_data*)MALLOC(sizeof(poller_data)*n_items));

	/* loop through and initialize the poll items and poller_data arrays...*/
	for(i=0, current = lcnfRoot; current != NULL; current = current->next, i++) {
	    /* create the socket, update items.*/
	    items[i].socket=current->sock;
	    items[i].events = ZMQ_POLLIN;

	    /* now update the poller_data for this item */
	    pollerData[i].thread  = pThrd;
	    pollerData[i].ruleset = current->pRuleset;
	}

	s_zloop = zloop_new();
	for(i=0; i<n_items; ++i) {

	    rv = zloop_poller(s_zloop, &items[i], handlePoll, &pollerData[i]);
	    if (rv) {
	        LogError(0, NO_ERRCODE, "imzmq3: zloop_poller failed for item %zu: %s", i, zmq_strerror(errno));
	    }
	}
	DBGPRINTF("imzmq3: zloop_poller starting...");
	zloop_start(s_zloop);
	zloop_destroy(&s_zloop);
	DBGPRINTF("imzmq3: zloop_poller stopped.");
finalize_it:
	zctx_destroy(&s_context);

	free(items);
	free(pollerData);
	RETiRet;
}

/* ----------------------------------------------------------------------------
 * input module functions
 */

BEGINrunInput
CODESTARTrunInput
	CHKiRet(rcv_loop(pThrd));
finalize_it:
	RETiRet;
ENDrunInput


/* initialize and return if will run or not */
BEGINwillRun
CODESTARTwillRun
	/* we need to create the inputName property (only once during our
	   lifetime) */
	CHKiRet(prop.Construct(&s_namep));
	CHKiRet(prop.SetString(s_namep,
	                       UCHAR_CONSTANT("imzmq3"),
	                       sizeof("imzmq3") - 1));
	CHKiRet(prop.ConstructFinalize(s_namep));

finalize_it:
ENDwillRun


BEGINafterRun
CODESTARTafterRun
	/* do cleanup here */
	if (s_namep != NULL)
	    prop.Destruct(&s_namep);
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
ENDmodExit


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if (eFeat == sFEATURENonCancelInputTermination)
	    iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	/* After endCnfLoad() (BEGINendCnfLoad...ENDendCnfLoad) is called,
	 * the pModConf pointer must not be used to change the in-memory
	 * config object. It's safe to use the same pointer for accessing
	 * the config object until freeCnf() (BEGINfreeCnf...ENDfreeCnf). */
	runModConf = pModConf;
	runModConf->pConf = pConf;
	/* init module config */
	runModConf->io_threads = 0; /* 0 means don't set it */
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals* pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if (NULL == pvals) {
	     LogError(0, RS_RET_MISSING_CNFPARAMS, "imzmq3: error processing module "
	                     " config parameters ['module(...)']");
	     ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	for (i=0; i < modpblk.nParams; ++i) {
	    if (!pvals[i].bUsed)
	        continue;
	    if (!strcmp(modpblk.descr[i].name, "ioThreads")) {
	        runModConf->io_threads = (int)pvals[i].val.d.n;
	    } else {
	        LogError(0, RS_RET_INVALID_PARAMS,
	                       "imzmq3: config error, unknown "
	                       "param %s in setModCnf\n",
	                       modpblk.descr[i].name);
	    }
	}

finalize_it:
	if (pvals != NULL)
	    cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf


BEGINendCnfLoad
CODESTARTendCnfLoad
	/* Last chance to make changes to the in-memory config object for this
	 * input module. After this call, the config object must no longer be
	 * changed. */
	if (pModConf != runModConf) {
	    LogError(0, NO_ERRCODE, "imzmq3: pointer of in-memory config object has "
	                    "changed - pModConf=%p, runModConf=%p", pModConf, runModConf);
	}
	assert(pModConf == runModConf);
ENDendCnfLoad


/* function to generate error message if framework does not find requested ruleset */
static inline void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE, "imzmq3: ruleset '%s' for socket %s not found - "
	                "using default ruleset instead", inst->pszBindRuleset,
	                inst->description);
}


BEGINcheckCnf
instanceConf_t* inst;
CODESTARTcheckCnf
	for(inst = pModConf->root; inst!=NULL; inst=inst->next) {
	    std_checkRuleset(pModConf, inst);
	    /* now, validate the instanceConf */
	    CHKiRet(validateConfig(inst));
	}
finalize_it:
	RETiRet;
ENDcheckCnf


BEGINactivateCnfPrePrivDrop
CODESTARTactivateCnfPrePrivDrop
	if (pModConf != runModConf) {
	    LogError(0, NO_ERRCODE, "imzmq3: pointer of in-memory config object has "
	                    "changed - pModConf=%p, runModConf=%p", pModConf, runModConf);
	}
	assert(pModConf == runModConf);

	/* first create the context */
	createContext();

	/* could setup context here, and set the global worker threads
	   and so on... */
ENDactivateCnfPrePrivDrop


BEGINactivateCnf
CODESTARTactivateCnf
	if (pModConf != runModConf) {
	    LogError(0, NO_ERRCODE, "imzmq3: pointer of in-memory config object has "
	                    "changed - pModConf=%p, runModConf=%p", pModConf, runModConf);
	}
	assert(pModConf == runModConf);
ENDactivateCnf


BEGINfreeCnf
	struct lstn_s *lstn, *lstn_r;
	instanceConf_t *inst, *inst_r;
	sublist *sub, *sub_r;
CODESTARTfreeCnf
	DBGPRINTF("imzmq3: BEGINfreeCnf ...\n");
	if (pModConf != runModConf) {
	    LogError(0, NO_ERRCODE, "imzmq3: pointer of in-memory config object has "
	                    "changed - pModConf=%p, runModConf=%p", pModConf, runModConf);
	}
	for (lstn = lcnfRoot; lstn != NULL; ) {
	    lstn_r = lstn;
	    lstn = lstn_r->next;
	    free(lstn_r);
	}
	for (inst = pModConf->root ; inst != NULL ; ) {
	    for (sub = inst->subscriptions; sub != NULL; ) {
	        free(sub->subscribe);
	        sub_r = sub;
	        sub = sub_r->next;
	        free(sub_r);
	    }
	    free(inst->pszBindRuleset);
	    inst_r = inst;
	    inst = inst->next;
	    free(inst_r);
	}
ENDfreeCnf


BEGINnewInpInst
	struct cnfparamvals* pvals;
CODESTARTnewInpInst

	DBGPRINTF("newInpInst (imzmq3)\n");
	pvals = nvlstGetParams(lst, &inppblk, NULL);
	if(NULL==pvals) {
	    LogError(0, RS_RET_MISSING_CNFPARAMS,
	                    "imzmq3: required parameters are missing\n");
	    ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	DBGPRINTF("imzmq3: input param blk:\n");
	cnfparamsPrint(&inppblk, pvals);

	/* now, parse the config params and so on... */
	CHKiRet(createListener(pvals));

finalize_it:
CODE_STD_FINALIZERnewInpInst
	cnfparamvalsDestruct(pvals, &inppblk);
ENDnewInpInst


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_STD_CONF2_PREPRIVDROP_QUERIES
CODEqueryEtryPt_STD_CONF2_IMOD_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	/* we only support the current interface specification */
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));
	LogError(0, RS_RET_DEPRECATED, "note: imzmq3 module is deprecated and will "
	"be removed soon. Do no longer use it, switch to imczmq. See "
	"https://github.com/rsyslog/rsyslog/issues/2103 for details.");
ENDmodInit


