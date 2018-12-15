/* mmgrok.c
 * Grok the message is parsed into a structured json data inside JSON.
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
#include <ctype.h>
#include <json.h>
#include <grok.h>
#include <glib.h>
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "dirty.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmgrok");

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

DEF_OMOD_STATIC_DATA

typedef struct result_s{
	char *key;
	int key_len;
	const char *value;
	int value_len;
	char *type;
}result_t;

/* config variables */
typedef struct _instanceData
{
	char *pszPatternDir;
	char *pszMatch;
	char *pszSource;
	char *pszTarget;/* as a json root for store parse json data */
	smsg_t *pmsg;    /* store  origin messages*/
}instanceData;

typedef struct wrkrInstanceData{
	instanceData *pData;
}wrkrInstanceData_t;

struct modConfData_s{
	rsconf_t   *pConf;/* our overall config object */
};

static  modConfData_t   *loadModConf = NULL;
static  modConfData_t   *runModConf   = NULL;

/* action (instance) paramters */
static struct cnfparamdescr actpdescr[]={
	{"patterndir",eCmdHdlrString,0},
	{"match",eCmdHdlrString,0},
	{"source",eCmdHdlrString,0},
	{"target",eCmdHdlrString,0},
};

static struct cnfparamblk actpblk =
{
	CNFPARAMBLK_VERSION,
	sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	actpdescr
};

BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
ENDbeginCnfLoad

BEGINendCnfLoad
CODESTARTendCnfLoad
ENDendCnfLoad

BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf

BEGINactivateCnf
CODESTARTactivateCnf
	runModConf = pModConf;
ENDactivateCnf

BEGINfreeCnf
CODESTARTfreeCnf
ENDfreeCnf

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature

BEGINfreeInstance
CODESTARTfreeInstance
	 free(pData->pszPatternDir);
	 free(pData->pszMatch);
	 free(pData->pszSource);
	 free(pData->pszTarget);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


static inline void setInstParamDefaults(instanceData *pData)
{
	pData->pszPatternDir= NULL;
	pData->pszMatch = NULL;
	pData->pszSource = NULL;
	pData->pszTarget = NULL;
	pData->pmsg = NULL;
}


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmgrok)\n");
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CODE_STD_STRING_REQUESTnewActInst(1)
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, NULL, OMSR_TPL_AS_MSG));
	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "patterndir")) {
			pData->pszPatternDir= es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		else if(!strcmp(actpblk.descr[i].name, "match")) {
			pData->pszMatch = es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		else if(!strcmp(actpblk.descr[i].name, "source")) {
			pData->pszSource= es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		else  if(!strcmp(actpblk.descr[i].name,"target"))
	            {
	                    pData->pszTarget=es_str2cstr(pvals[i].val.d.estr,NULL);
	                    continue;
	            }
		else{
	         	DBGPRINTF("mmgrok: program error, non-handled "
	               	"param '%s'\n", actpblk.descr[i].name);
		}
	}
	if(pData->pszTarget == NULL) {
		CHKmalloc(pData->pszTarget = strdup("!"));
	}
CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	DBGPRINTF("mmgrok\n");
ENDdbgPrintInstInfo

BEGINtryResume
CODESTARTtryResume
ENDtryResume

static inline grok_t *CreateGrok(void)
{
	grok_t  *grok = grok_new();
	if(grok == NULL){
	    DBGPRINTF("mmgrok: create a grok faile!");
	    exit(1);
	}
	grok_init(grok);
	return grok;
}

/* the parseing is complate message into json */
static rsRetVal
smsg_to_json(GList *list,instanceData *pData)
{
	GList *it= list;

	struct json_object *json;
	struct json_object *jval;

	DEFiRet;

	json = json_object_new_object();
	if(json == NULL)
	{
	        ABORT_FINALIZE(RS_RET_ERR);
	}
	for(;it;it= it->next)
	{
	int key_len = ((result_t *)it->data)->key_len;
	    char *key = (char *)malloc(key_len+1);
	    snprintf(key,key_len+1,"%.*s",key_len,((result_t *)it->data)->key);
	int value_len = ((result_t *)it->data)->value_len;
	char *value = (char *)malloc(value_len+1);
	    snprintf(value,value_len+1,"%.*s",value_len,((result_t*)it->data)->value);
	jval = json_object_new_string(value);
	json_object_object_add(json,key,jval);
	free(key);
	free(value);
	}
	msgAddJSON(pData->pmsg,(uchar*)pData->pszTarget,json,0,0);
finalize_it:
	RETiRet;
}

/* store parse result ,use list in glib*/
static rsRetVal
parse_result_store(const grok_match_t gm,instanceData *pData)
{
	    GList *re_list = NULL;
	    char  *pname;
	    const char  *pdata;
	    int    pname_len,pdata_len;

	    char *key;
	    char *type;
	    DEFiRet;

	    grok_match_walk_init(&gm); //grok API

	    while(grok_match_walk_next(&gm,&pname,&pname_len,&pdata,&pdata_len) == 0)
	    {
	        /* parse key and value type from patterns */
	        key = strchr(pname,':');
	
	        if(key!=NULL)
	        {
	            int key_len;
	            result_t *result = g_new0(result_t,1);
	            key_len = pname_len - ((key+1) - pname);
	            key = key+1;
	            pname_len = key_len;
	            type = strchr(key,':');
	            int type_len;
	            if(type!=NULL)
	            {
	                key_len = (type - key);
	                type = type+1;
	                type_len = pname_len - key_len -1;
	                sprintf(type,"%.*s",type_len,type);
	            }
	            else{type = (char*)"null";}
	            /* store parse result into list */
	            result->key = key;
	            result->key_len = key_len;
	            result->value = pdata;
	            result->value_len = pdata_len;
	            result->type = type;
	            /* the value of merger the same key*/
	            re_list = g_list_append(re_list,result);
	        }
	    }
	    smsg_to_json(re_list,pData);
	    g_list_free(re_list);
	    grok_match_walk_end(&gm);
	RETiRet;
}

/* motify message for per line */
static rsRetVal
MotifyLine(char *line,grok_t *grok,instanceData *pData)
{
	grok_match_t  gm;
	DEFiRet;
	grok_patterns_import_from_file(grok,pData->pszPatternDir);
	int compile = grok_compile(grok,pData->pszMatch);
	if(compile!=GROK_OK)
	{
	    DBGPRINTF("mmgrok: grok_compile faile!exit code: %d\n",compile);
	ABORT_FINALIZE(RS_RET_ERR);
	}
	int exe = grok_exec(grok,line,&gm);
	if(exe!=GROK_OK)
	{
	    DBGPRINTF("mmgrok: grok_exec faile!exit code: %d\n",exe);
	ABORT_FINALIZE(RS_RET_ERR);
	}
	parse_result_store(gm,pData);
finalize_it:
	RETiRet;
}

/* motify rsyslog messages */
static rsRetVal
MotifyMessage(instanceData *pData)
{
	char *saveptr = NULL;
	DEFiRet;
	grok_t  *grok = CreateGrok();
	char     *msg = strdup(pData->pszSource);
	char     *line = NULL;
	line = strtok_r(msg, "\n", &saveptr);
	while(line!=NULL) {
		MotifyLine(line,grok,pData);
		line = strtok_r(NULL, "\n", &saveptr);
	}
	free(msg);msg=NULL;
	RETiRet;
}


BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	uchar *buf;
	    instanceData *pData;
	
CODESTARTdoAction
	    pData = pWrkrData->pData;
	buf = getMSG(pMsg);
	    pData->pmsg = pMsg;
	while(*buf && isspace(*buf)) {
		++buf;
	}

	if(*buf == '\0' ) {
		DBGPRINTF("mmgrok:  not msg for mmgrok!");
		ABORT_FINALIZE(RS_RET_NO_CEE_MSG);
	}
	pData->pszSource = (char *)buf;
	CHKiRet(MotifyMessage(pData));

finalize_it:
ENDdoAction

BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	if(strncmp((char*) p, ":mmgrok:", sizeof(":mmgrok:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	p += sizeof(":mmgrok:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	if(*(p-1) == ';')
		--p;
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_TPL_AS_MSG, (uchar*) "RSYSLOG_FileFormat"));
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct

BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
ENDqueryEtryPt

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	RETiRet;
}

BEGINmodInit()
	rsRetVal localRet;
	rsRetVal (*pomsrGetSupportedTplOpts)(unsigned long *pOpts);
	unsigned long opts;
	int bMsgPassingSupported;
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	DBGPRINTF("mmgrok: module compiled with rsyslog version %s.\n", VERSION);
	bMsgPassingSupported = 0;
	localRet = pHostQueryEtryPt((uchar*)"OMSRgetSupportedTplOpts",
			&pomsrGetSupportedTplOpts);
	if(localRet == RS_RET_OK) {
		CHKiRet((*pomsrGetSupportedTplOpts)(&opts));
		if(opts & OMSR_TPL_AS_MSG)
			bMsgPassingSupported = 1;
	} else if(localRet != RS_RET_ENTRY_POINT_NOT_FOUND) {
		ABORT_FINALIZE(localRet); /* Something else went wrong, not acceptable */
	}
	
	if(!bMsgPassingSupported) {
		DBGPRINTF("mmgrok: msg-passing is not supported by rsyslog core, "
			  "can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_MSG_PASSING);
	}

	
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
				    resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit
