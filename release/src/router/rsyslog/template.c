/* This is the template processing code of rsyslog.
 * begun 2004-11-17 rgerhards
 *
 * Copyright 2004-2018 Rainer Gerhards and Adiscon
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
 *
 * Note: there is a tiny bit of code left where I could not get any response
 * from the author if this code can be placed under ASL2.0. I have guarded this
 * with #ifdef STRICT_GPLV3. Only if that macro is defined, the code will be
 * compiled. Otherwise this feature is not present. The plan is to do a
 * different implementation in the future to get rid of this problem.
 * rgerhards, 2012-08-25
 */
#include "config.h"

#include "rsyslog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <json.h>
#include "stringbuf.h"
#include "syslogd-types.h"
#include "template.h"
#include "msg.h"
#include "dirty.h"
#include "obj.h"
#include "errmsg.h"
#include "strgen.h"
#include "rsconf.h"
#include "msg.h"
#include "parserif.h"
#include "unicode-helper.h"

#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif
/* static data */
DEFobjCurrIf(obj)
DEFobjCurrIf(strgen)

/* tables for interfacing with the v6 config system */
static struct cnfparamdescr cnfparamdescr[] = {
	{ "name", eCmdHdlrString, 1 },
	{ "type", eCmdHdlrString, 1 },
	{ "string", eCmdHdlrString, 0 },
	{ "plugin", eCmdHdlrString, 0 },
	{ "subtree", eCmdHdlrString, 0 },
	{ "option.stdsql", eCmdHdlrBinary, 0 },
	{ "option.sql", eCmdHdlrBinary, 0 },
	{ "option.json", eCmdHdlrBinary, 0 },
	{ "option.jsonf", eCmdHdlrBinary, 0 },
	{ "option.casesensitive", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk pblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(cnfparamdescr)/sizeof(struct cnfparamdescr),
	  cnfparamdescr
	};

static struct cnfparamdescr cnfparamdescrProperty[] = {
	{ "name", eCmdHdlrString, 1 },
	{ "outname", eCmdHdlrString, 0 },
	{ "dateformat", eCmdHdlrString, 0 },
	{ "date.inutc", eCmdHdlrBinary, 0 },
	{ "compressspace", eCmdHdlrBinary, 0 },
	{ "caseconversion", eCmdHdlrString, 0 },
	{ "controlcharacters", eCmdHdlrString, 0 },
	{ "securepath", eCmdHdlrString, 0 },
	{ "format", eCmdHdlrString, 0 },
	{ "position.from", eCmdHdlrInt, 0 },
	{ "position.to", eCmdHdlrInt, 0 },
	{ "position.relativetoend", eCmdHdlrBinary, 0 },
	{ "field.number", eCmdHdlrInt, 0 },
	{ "field.delimiter", eCmdHdlrInt, 0 },
	{ "regex.expression", eCmdHdlrString, 0 },
	{ "regex.type", eCmdHdlrString, 0 },
	{ "regex.nomatchmode", eCmdHdlrString, 0 },
	{ "regex.match", eCmdHdlrInt, 0 },
	{ "regex.submatch", eCmdHdlrInt, 0 },
	{ "droplastlf", eCmdHdlrBinary, 0 },
	{ "fixedwidth", eCmdHdlrBinary, 0 },
	{ "mandatory", eCmdHdlrBinary, 0 },
	{ "spifno1stsp", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk pblkProperty =
	{ CNFPARAMBLK_VERSION,
	  sizeof(cnfparamdescrProperty)/sizeof(struct cnfparamdescr),
	  cnfparamdescrProperty
	};

static struct cnfparamdescr cnfparamdescrConstant[] = {
	{ "value", eCmdHdlrString, 1 },
	{ "format", eCmdHdlrString, 0 },
	{ "outname", eCmdHdlrString, 0 }
};
static struct cnfparamblk pblkConstant =
	{ CNFPARAMBLK_VERSION,
	  sizeof(cnfparamdescrConstant)/sizeof(struct cnfparamdescr),
	  cnfparamdescrConstant
	};


#ifdef FEATURE_REGEXP
DEFobjCurrIf(regexp)
static int bFirstRegexpErrmsg = 1; /**< did we already do a "can't load regexp" error message? */
#endif

/* helper to tplToString and strgen's, extends buffer */
#define ALLOC_INC 128
rsRetVal
ExtendBuf(actWrkrIParams_t *__restrict__ const iparam, const size_t iMinSize)
{
	uchar *pNewBuf;
	size_t iNewSize;
	DEFiRet;

	iNewSize = (iMinSize / ALLOC_INC + 1) * ALLOC_INC;
	CHKmalloc(pNewBuf = (uchar*) realloc(iparam->param, iNewSize));
	iparam->param = pNewBuf;
	iparam->lenBuf = iNewSize;

finalize_it:
	RETiRet;
}


/* This functions converts a template into a string.
 *
 * The function takes a pointer to a template and a pointer to a msg object
 * as well as a pointer to an output buffer and its size. Note that the output
 * buffer pointer may be NULL, size 0, in which case a new one is allocated.
 * The output buffer is grown as required. It is the caller's duty to free the
 * buffer when it is done. Note that it is advisable to reuse memory, as this
 * offers big performance improvements.
 * rewritten 2009-06-19 rgerhards
 */
rsRetVal
tplToString(struct template *__restrict__ const pTpl,
	    smsg_t *__restrict__ const pMsg,
	    actWrkrIParams_t *__restrict__ const iparam,
	    struct syslogTime *const ttNow)
{
	DEFiRet;
	struct templateEntry *__restrict__ pTpe;
	size_t iBuf;
	unsigned short bMustBeFreed = 0;
	uchar *pVal;
	rs_size_t iLenVal = 0;

	if(pTpl->pStrgen != NULL) {
		CHKiRet(pTpl->pStrgen(pMsg, iparam));
		FINALIZE;
	}

	if(pTpl->bHaveSubtree) {
		/* only a single CEE subtree must be provided */
		/* note: we could optimize the code below, however, this is
		 * not worth the effort, as this passing mode is not expected
		 * in subtree mode and so most probably only used for debug & test.
		 */
		getJSONPropVal(pMsg, &pTpl->subtree, &pVal, &iLenVal, &bMustBeFreed);
		if(iLenVal >= (rs_size_t)iparam->lenBuf) /* we reserve one char for the final \0! */
			CHKiRet(ExtendBuf(iparam, iLenVal + 1));
		memcpy(iparam->param, pVal, iLenVal+1);
		FINALIZE;
	}
	
	/* we have a "regular" template with template entries */

	/* loop through the template. We obtain one value
	 * and copy it over to our dynamic string buffer. Then, we
	 * free the obtained value (if requested). We continue this
	 * loop until we got hold of all values.
	 */
	pTpe = pTpl->pEntryRoot;
	iBuf = 0;
	const int extra_space = (pTpl->optFormatEscape == JSONF) ? 1 : 3;
	if(pTpl->optFormatEscape == JSONF) {
		if(iparam->lenBuf < 2) /* we reserve one char for the final \0! */
			CHKiRet(ExtendBuf(iparam, 2));
		iBuf = 1;
		*iparam->param = '{';
	}
	while(pTpe != NULL) {
		if(pTpe->eEntryType == CONSTANT) {
			pVal = (uchar*) pTpe->data.constant.pConstant;
			iLenVal = pTpe->data.constant.iLenConstant;
			bMustBeFreed = 0;
		} else 	if(pTpe->eEntryType == FIELD) {
			pVal = (uchar*) MsgGetProp(pMsg, pTpe, &pTpe->data.field.msgProp,
						   &iLenVal, &bMustBeFreed, ttNow);
			/* we now need to check if we should use SQL option. In this case,
			 * we must go over the generated string and escape '\'' characters.
			 * rgerhards, 2005-09-22: the option values below look somewhat misplaced,
			 * but they are handled in this way because of legacy (don't break any
			 * existing thing).
			 */
			if(pTpl->optFormatEscape == SQL_ESCAPE)
				doEscape(&pVal, &iLenVal, &bMustBeFreed, SQL_ESCAPE);
			else if(pTpl->optFormatEscape == JSON_ESCAPE)
				doEscape(&pVal, &iLenVal, &bMustBeFreed, JSON_ESCAPE);
			else if(pTpl->optFormatEscape == STDSQL_ESCAPE)
				doEscape(&pVal, &iLenVal, &bMustBeFreed, STDSQL_ESCAPE);
		} else {
			DBGPRINTF("TplToString: invalid entry type %d\n", pTpe->eEntryType);
			pVal = (uchar*) "*LOGIC ERROR*";
			iLenVal = sizeof("*LOGIC ERROR*") - 1;
			bMustBeFreed = 0;
		}
		/* got source, now copy over */
		if(iLenVal > 0) { /* may be zero depending on property */
			/* first, make sure buffer fits */
			if(iBuf + iLenVal + extra_space >= iparam->lenBuf) /* we reserve one char for the final \0! */
				CHKiRet(ExtendBuf(iparam, iBuf + iLenVal + 1));

			memcpy(iparam->param + iBuf, pVal, iLenVal);
			iBuf += iLenVal;
			if(pTpl->optFormatEscape == JSONF) {
				memcpy(iparam->param + iBuf,
					(pTpe->pNext == NULL) ? "}\n" : ", ", 2);
				iBuf += 2;
			}
		}

		if(bMustBeFreed) {
			free(pVal);
			bMustBeFreed = 0;
		}

		pTpe = pTpe->pNext;
	}

	if(iBuf == iparam->lenBuf) {
		/* in the weired case of an *empty* template, this can happen.
		 * it is debatable if we should really fix it here or simply
		 * forbid that case. However, performance toll is minimal, so
		 * I tend to permit it. -- 2010-11-05 rgerhards
		 */
		CHKiRet(ExtendBuf(iparam, iBuf + 1));
	}
	iparam->param[iBuf] = '\0';
	iparam->lenStr = iBuf;
	
finalize_it:
	if(bMustBeFreed) {
		free(pVal);
		bMustBeFreed = 0;
	}

	RETiRet;
}


/* This functions converts a template into an array of strings.
 * For further general details, see the very similar funtion
 * tpltoString().
 * Instead of a string, an array of string pointers is returned by
 * thus function. The caller is repsonsible for destroying that array as
 * well as all of its elements. The array is of fixed size. It's end
 * is indicated by a NULL pointer.
 * rgerhards, 2009-04-03
 */
rsRetVal
tplToArray(struct template *pTpl, smsg_t *pMsg, uchar*** ppArr, struct syslogTime *ttNow)
{
	DEFiRet;
	struct templateEntry *pTpe;
	uchar **pArr;
	int iArr;
	rs_size_t propLen;
	unsigned short bMustBeFreed;
	uchar *pVal;

	assert(pTpl != NULL);
	assert(pMsg != NULL);
	assert(ppArr != NULL);

	if(pTpl->bHaveSubtree) {
		/* Note: this mode is untested, as there is no official plugin
		 *       using array passing, so I simply could not test it.
		 */
		CHKmalloc(pArr = calloc(2, sizeof(uchar*)));
		getJSONPropVal(pMsg, &pTpl->subtree, &pVal, &propLen, &bMustBeFreed);
		if(bMustBeFreed) { /* if it must be freed, it is our own private copy... */
			pArr[0] = pVal; /* ... so we can use it! */
		} else {
			CHKmalloc(pArr[0] = (uchar*)strdup((char*) pVal));
		}
		FINALIZE;
	}

	/* loop through the template. We obtain one value, create a
	 * private copy (if necessary), add it to the string array
	 * and then on to the next until we have processed everything.
	 */
	CHKmalloc(pArr = calloc(pTpl->tpenElements + 1, sizeof(uchar*)));
	iArr = 0;

	pTpe = pTpl->pEntryRoot;
	while(pTpe != NULL) {
		if(pTpe->eEntryType == CONSTANT) {
			CHKmalloc(pArr[iArr] = (uchar*)strdup((char*) pTpe->data.constant.pConstant));
		} else 	if(pTpe->eEntryType == FIELD) {
			pVal = (uchar*) MsgGetProp(pMsg, pTpe, &pTpe->data.field.msgProp,
						   &propLen, &bMustBeFreed, ttNow);
			if(bMustBeFreed) { /* if it must be freed, it is our own private copy... */
				pArr[iArr] = pVal; /* ... so we can use it! */
			} else {
				CHKmalloc(pArr[iArr] = (uchar*)strdup((char*) pVal));
			}
		}
		iArr++;
		pTpe = pTpe->pNext;
	}

finalize_it:
	*ppArr = (iRet == RS_RET_OK) ? pArr : NULL;
	if(iRet == RS_RET_OK) {
		*ppArr = pArr;
	} else {
		*ppArr = NULL;
		free(pArr);
	}

	RETiRet;
}


/* This functions converts a template into a json object.
 * For further general details, see the very similar funtion
 * tpltoString().
 * rgerhards, 2012-08-29
 */
rsRetVal
tplToJSON(struct template *pTpl, smsg_t *pMsg, struct json_object **pjson, struct syslogTime *ttNow)
{
	struct templateEntry *pTpe;
	rs_size_t propLen;
	unsigned short bMustBeFreed;
	uchar *pVal;
	struct json_object *json, *jsonf;
	rsRetVal localRet;
	DEFiRet;

	if(pTpl->bHaveSubtree){
		if(jsonFind(pMsg->json, &pTpl->subtree, pjson) != RS_RET_OK)
			*pjson = NULL;
		if(*pjson == NULL) {
			/* we need to have a root object! */
			*pjson = json_object_new_object();
		} else {
			json_object_get(*pjson); /* inc refcount */
		}
		FINALIZE;
	}

	json = json_object_new_object();
	for(pTpe = pTpl->pEntryRoot ; pTpe != NULL ; pTpe = pTpe->pNext) {
		if(pTpe->eEntryType == CONSTANT) {
			if(pTpe->fieldName == NULL)
				continue;
			jsonf = json_object_new_string((char*) pTpe->data.constant.pConstant);
			json_object_object_add(json, (char*)pTpe->fieldName, jsonf);
		} else 	if(pTpe->eEntryType == FIELD) {
			if(pTpe->data.field.msgProp.id == PROP_CEE        ||
			   pTpe->data.field.msgProp.id == PROP_LOCAL_VAR  ||
			   pTpe->data.field.msgProp.id == PROP_GLOBAL_VAR   ) {
				localRet = msgGetJSONPropJSON(pMsg, &pTpe->data.field.msgProp, &jsonf);
				if(localRet == RS_RET_OK) {
					json_object_object_add(json, (char*)pTpe->fieldName, json_object_get(jsonf));
				} else {
					DBGPRINTF("tplToJSON: error %d looking up property %s\n",
						  localRet, pTpe->fieldName);
					if(pTpe->data.field.options.bMandatory) {
						json_object_object_add(json, (char*)pTpe->fieldName, NULL);
					}
				}
			} else  {
				pVal = (uchar*) MsgGetProp(pMsg, pTpe, &pTpe->data.field.msgProp,
							   &propLen, &bMustBeFreed, ttNow);
				if(pTpe->data.field.options.bMandatory || propLen > 0) {
					jsonf = json_object_new_string_len((char*)pVal, propLen+1);
					json_object_object_add(json, (char*)pTpe->fieldName, jsonf);
				}
				if(bMustBeFreed) { /* json-c makes its own private copy! */
					free(pVal);
				}
			}
		}
	}
	assert(iRet == RS_RET_OK);
	*pjson = json;

finalize_it:
	RETiRet;
}


/* Helper to doEscape. This is called if doEscape
 * runs out of memory allocating the escaped string.
 * Then we are in trouble. We can
 * NOT simply return the unmodified string because this
 * may cause SQL injection. But we also can not simply
 * abort the run, this would be a DoS. I think an appropriate
 * measure is to remove the dangerous \' characters (SQL). We
 * replace them by \", which will break the message and
 * signatures eventually present - but this is the
 * best thing we can do now (or does anybody
 * have a better idea?). rgerhards 2004-11-23
 * added support for escape mode (see doEscape for details).
 * if mode = SQL_ESCAPE, then backslashes are changed to slashes.
 * rgerhards 2005-09-22
 */
static void doEmergencyEscape(register uchar *p, int mode)
{
	while(*p) {
		if((mode == SQL_ESCAPE||mode == STDSQL_ESCAPE) && *p == '\'') {
			*p = '"';
		} else if(mode == JSON_ESCAPE) {
			if(*p == '"') {
				*p = '\'';
			} else if(*p == '\\' ) {
				*p = '/';
			}
		} else if((mode == SQL_ESCAPE) && *p == '\\') {
			*p = '/';
		}
		++p;
	}
}


/* SQL-Escape a string. Single quotes are found and
 * replaced by two of them. A new buffer is allocated
 * for the provided string and the provided buffer is
 * freed. The length is updated. Parameter pbMustBeFreed
 * is set to 1 if a new buffer is allocated. Otherwise,
 * it is left untouched.
 * --
 * We just discovered a security issue. MySQL is so
 * "smart" to not only support the standard SQL mechanism
 * for escaping quotes, but to also provide its own (using
 * c-type syntax with backslashes). As such, it is actually
 * possible to do sql injection via rsyslogd. The cure is now
 * to escape backslashes, too. As we have found on the web, some
 * other databases seem to be similar "smart" (why do we have standards
 * at all if they are violated without any need???). Even better, MySQL's
 * smartness depends on config settings. So we add a new option to this
 * function that allows the caller to select if they want to standard or
 * "smart" encoding ;)
 * --
 * Parameter "mode" is STDSQL_ESCAPE, SQL_ESCAPE "smart" SQL engines, or
 * JSON_ESCAPE for everyone requiring escaped JSON (e.g. ElasticSearch).
 * 2005-09-22 rgerhards
 */
rsRetVal
doEscape(uchar **pp, rs_size_t *pLen, unsigned short *pbMustBeFreed, int mode)
{
	DEFiRet;
	uchar *p = NULL;
	int iLen;
	cstr_t *pStrB = NULL;
	uchar *pszGenerated;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pLen != NULL);
	assert(pbMustBeFreed != NULL);

	/* first check if we need to do anything at all... */
	if(mode == STDSQL_ESCAPE)
		for(p = *pp ; *p && *p != '\'' ; ++p)
			;
	else if(mode == SQL_ESCAPE)
		for(p = *pp ; *p && *p != '\'' && *p != '\\' ; ++p)
			;
	else if(mode == JSON_ESCAPE)
		for(p = *pp ; *p &&  (*p == '"' || *p == '\\' ) ; ++p)
			;
	/* when we get out of the loop, we are either at the
	 * string terminator or the first character to escape */
	if(p && *p == '\0')
		FINALIZE; /* nothing to do in this case! */

	p = *pp;
	iLen = *pLen;
	CHKiRet(cstrConstruct(&pStrB));

	while(*p) {
		if((mode == SQL_ESCAPE || mode == STDSQL_ESCAPE) && *p == '\'') {
			CHKiRet(cstrAppendChar(pStrB, (mode == STDSQL_ESCAPE) ? '\'' : '\\'));
			iLen++;	/* reflect the extra character */
		} else if((mode == SQL_ESCAPE) && *p == '\\') {
			CHKiRet(cstrAppendChar(pStrB, '\\'));
			iLen++;	/* reflect the extra character */
		} else if((mode == JSON_ESCAPE) &&  (*p == '"' || *p == '\\' )) {
			CHKiRet(cstrAppendChar(pStrB, '\\'));
			iLen++;	/* reflect the extra character */
		}
		CHKiRet(cstrAppendChar(pStrB, *p));
		++p;
	}
	cstrFinalize(pStrB);
	CHKiRet(cstrConvSzStrAndDestruct(&pStrB, &pszGenerated, 0));

	if(*pbMustBeFreed)
		free(*pp); /* discard previous value */

	*pp = pszGenerated;
	*pLen = iLen;
	*pbMustBeFreed = 1;

finalize_it:
	if(iRet != RS_RET_OK) {
		doEmergencyEscape(*pp, mode);
		if(pStrB != NULL)
			cstrDestruct(&pStrB);
	}

	RETiRet;
}


/* Constructs a template entry object. Returns pointer to it
 * or NULL (if it fails). Pointer to associated template list entry
 * must be provided.
 */
static struct templateEntry* tpeConstruct(struct template *pTpl)
{
	struct templateEntry *pTpe;

	assert(pTpl != NULL);

	if((pTpe = calloc(1, sizeof(struct templateEntry))) == NULL)
		return NULL;
	
	/* basic initialization is done via calloc() - need to
	 * initialize only values != 0. */

	if(pTpl->pEntryLast == NULL){
		/* we are the first element! */
		pTpl->pEntryRoot = pTpl->pEntryLast  = pTpe;
	} else {
		pTpl->pEntryLast->pNext = pTpe;
		pTpl->pEntryLast  = pTpe;
	}
	pTpl->tpenElements++;

	return(pTpe);
}


/* Helper function to apply case-sensitivity to templates.
 */
static void
apply_case_sensitivity(struct template *pTpl)
{
	if(pTpl->optCaseSensitive) return;

	struct templateEntry *pTpe;

	for(pTpe = pTpl->pEntryRoot ; pTpe != NULL ; pTpe = pTpe->pNext) {
		if(pTpe->eEntryType == FIELD) {
			if(pTpe->data.field.msgProp.id == PROP_CEE        ||
			   pTpe->data.field.msgProp.id == PROP_LOCAL_VAR  ||
			   pTpe->data.field.msgProp.id == PROP_GLOBAL_VAR   ) {
				uchar* p;
				p = pTpe->fieldName;
				for ( ; *p; ++p) *p = tolower(*p);
				p = pTpe->data.field.msgProp.name;
				for ( ; *p; ++p) *p = tolower(*p);
			}
		}
	}
}


/* Constructs a template list object. Returns pointer to it
 * or NULL (if it fails).
 */
static struct template*
tplConstruct(rsconf_t *conf)
{
	struct template *pTpl;
	if((pTpl = calloc(1, sizeof(struct template))) == NULL)
		return NULL;
	
	/* basic initialisation is done via calloc() - need to
	 * initialize only values != 0. */

	if(conf->templates.last == NULL)	{
		/* we are the first element! */
		conf->templates.root = conf->templates.last = pTpl;
	} else {
		conf->templates.last->pNext = pTpl;
		conf->templates.last = pTpl;
	}

	return(pTpl);
}


/* helper to tplAddLine. Parses a constant and generates
 * the necessary structure.
 * Paramter "bDoEscapes" is to support legacy vs. v6+ config system. In
 * legacy, we must do escapes ourselves, whereas v6+ passes in already
 * escaped strings (which we are NOT permitted to further escape, this would
 * cause invalid result strings!). Note: if escapes are not permitted,
 * quotes (") are just a regular character and do NOT terminate the constant!
 */
static rsRetVal
do_Constant(unsigned char **pp, struct template *pTpl, int bDoEscapes)
{
	register unsigned char *p;
	cstr_t *pStrB;
	struct templateEntry *pTpe;
	int i;
	DEFiRet;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pTpl != NULL);

	p = *pp;

	CHKiRet(cstrConstruct(&pStrB));
	/* process the message and expand escapes
	 * (additional escapes can be added here if needed)
	 */
	while(*p && *p != '%' && !(bDoEscapes && *p == '\"')) {
		if(bDoEscapes && *p == '\\') {
			switch(*++p) {
				case '\0':
					/* the best we can do - it's invalid anyhow... */
					cstrAppendChar(pStrB, *p);
					break;
				case 'n':
					cstrAppendChar(pStrB, '\n');
					++p;
					break;
				case 'r':
					cstrAppendChar(pStrB, '\r');
					++p;
					break;
				case '\\':
					cstrAppendChar(pStrB, '\\');
					++p;
					break;
				case '%':
					cstrAppendChar(pStrB, '%');
					++p;
					break;
				case '0': /* numerical escape sequence */
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					i = 0;
					while(*p && isdigit((int)*p)) {
						i = i * 10 + *p++ - '0';
					}
					cstrAppendChar(pStrB, i);
					break;
				default:
					cstrAppendChar(pStrB, *p++);
					break;
			}
		}
		else
			cstrAppendChar(pStrB, *p++);
	}

	if((pTpe = tpeConstruct(pTpl)) == NULL) {
		rsCStrDestruct(&pStrB);
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	pTpe->eEntryType = CONSTANT;
	cstrFinalize(pStrB);
	/* We obtain the length from the counted string object
	 * (before we delete it). Later we might take additional
	 * benefit from the counted string object.
	 * 2005-09-09 rgerhards
	 */
	pTpe->data.constant.iLenConstant = rsCStrLen(pStrB);
	CHKiRet(cstrConvSzStrAndDestruct(&pStrB, &pTpe->data.constant.pConstant, 0));

	*pp = p;

finalize_it:
	RETiRet;
}

/* Helper that checks to see if a property already has a format
 * type defined
 */
static int hasFormat(struct templateEntry *pTpe) {
	return (
		pTpe->data.field.options.bCSV ||
		pTpe->data.field.options.bJSON ||
		pTpe->data.field.options.bJSONf ||
		pTpe->data.field.options.bJSONr
	);
}

/* Helper to do_Parameter(). This parses the formatting options
 * specified in a template variable. It returns the passed-in pointer
 * updated to the next processed character.
 */
static void doOptions(unsigned char **pp, struct templateEntry *pTpe)
{
	register unsigned char *p;
	unsigned char Buf[64];
	size_t i;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pTpe != NULL);

	p = *pp;

	while(*p && *p != '%' && *p != ':') {
		/* outer loop - until end of options */
		i = 0;
		while((i < sizeof(Buf)-1) &&
		      *p && *p != '%' && *p != ':' && *p != ',') {
			/* inner loop - until end of ONE option */
			Buf[i++] = tolower((int)*p);
			++p;
		}
		Buf[i] = '\0'; /* terminate */
		/* check if we need to skip oversize option */
		while(*p && *p != '%' && *p != ':' && *p != ',')
			++p;	/* just skip */
		if(*p == ',')
			++p; /* eat ',' */
		/* OK, we got the option, so now lets look what
		 * it tells us...
		 */
		 if(!strcmp((char*)Buf, "date-mysql")) {
			pTpe->data.field.eDateFormat = tplFmtMySQLDate;
		} else if(!strcmp((char*)Buf, "date-pgsql")) {
			pTpe->data.field.eDateFormat = tplFmtPgSQLDate;
		 } else if(!strcmp((char*)Buf, "date-rfc3164")) {
			pTpe->data.field.eDateFormat = tplFmtRFC3164Date;
		 } else if(!strcmp((char*)Buf, "date-rfc3164-buggyday")) {
			pTpe->data.field.eDateFormat = tplFmtRFC3164BuggyDate;
		 } else if(!strcmp((char*)Buf, "date-rfc3339")) {
			pTpe->data.field.eDateFormat = tplFmtRFC3339Date;
		 } else if(!strcmp((char*)Buf, "date-unixtimestamp")) {
			pTpe->data.field.eDateFormat = tplFmtUnixDate;
		 } else if(!strcmp((char*)Buf, "date-subseconds")) {
			pTpe->data.field.eDateFormat = tplFmtSecFrac;
		 } else if(!strcmp((char*)Buf, "date-wdayname")) {
			pTpe->data.field.eDateFormat = tplFmtWDayName;
		 } else if(!strcmp((char*)Buf, "date-wday")) {
			pTpe->data.field.eDateFormat = tplFmtWDay;
		 } else if(!strcmp((char*)Buf, "date-year")) {
			pTpe->data.field.eDateFormat = tplFmtYear;
		 } else if(!strcmp((char*)Buf, "date-month")) {
			pTpe->data.field.eDateFormat = tplFmtMonth;
		 } else if(!strcmp((char*)Buf, "date-day")) {
			pTpe->data.field.eDateFormat = tplFmtDay;
		 } else if(!strcmp((char*)Buf, "date-hour")) {
			pTpe->data.field.eDateFormat = tplFmtHour;
		 } else if(!strcmp((char*)Buf, "date-minute")) {
			pTpe->data.field.eDateFormat = tplFmtMinute;
		 } else if(!strcmp((char*)Buf, "date-second")) {
			pTpe->data.field.eDateFormat = tplFmtSecond;
		 } else if(!strcmp((char*)Buf, "date-tzoffshour")) {
			pTpe->data.field.eDateFormat = tplFmtTZOffsHour;
		 } else if(!strcmp((char*)Buf, "date-tzoffsmin")) {
			pTpe->data.field.eDateFormat = tplFmtTZOffsMin;
		 } else if(!strcmp((char*)Buf, "date-tzoffsdirection")) {
			pTpe->data.field.eDateFormat = tplFmtTZOffsDirection;
		 } else if (!strcmp((char*)Buf, "date-ordinal")) {
			pTpe->data.field.eDateFormat = tplFmtOrdinal;
		 } else if (!strcmp((char*)Buf, "date-week")) {
			pTpe->data.field.eDateFormat = tplFmtWeek;
		 } else if(!strcmp((char*)Buf, "date-utc")) {
			pTpe->data.field.options.bDateInUTC = 1;
		 } else if(!strcmp((char*)Buf, "lowercase")) {
			pTpe->data.field.eCaseConv = tplCaseConvLower;
		 } else if(!strcmp((char*)Buf, "uppercase")) {
			pTpe->data.field.eCaseConv = tplCaseConvUpper;
		 } else if(!strcmp((char*)Buf, "sp-if-no-1st-sp")) {
			pTpe->data.field.options.bSPIffNo1stSP = 1;
		 } else if(!strcmp((char*)Buf, "compressspace")) {
			pTpe->data.field.options.bCompressSP = 1;
		 } else if(!strcmp((char*)Buf, "escape-cc")) {
			pTpe->data.field.options.bEscapeCC = 1;
		 } else if(!strcmp((char*)Buf, "drop-cc")) {
			pTpe->data.field.options.bDropCC = 1;
		 } else if(!strcmp((char*)Buf, "space-cc")) {
			pTpe->data.field.options.bSpaceCC = 1;
		 } else if(!strcmp((char*)Buf, "drop-last-lf")) {
			pTpe->data.field.options.bDropLastLF = 1;
		 } else if(!strcmp((char*)Buf, "secpath-drop")) {
			pTpe->data.field.options.bSecPathDrop = 1;
		 } else if(!strcmp((char*)Buf, "secpath-replace")) {
			pTpe->data.field.options.bSecPathReplace = 1;
		 } else if(!strcmp((char*)Buf, "pos-end-relative")) {
			pTpe->data.field.options.bFromPosEndRelative = 1;
		 } else if(!strcmp((char*)Buf, "fixed-width")) {
			pTpe->data.field.options.bFixedWidth = 1;
		 } else if(!strcmp((char*)Buf, "csv")) {
			if(hasFormat(pTpe)) {
				LogError(0, NO_ERRCODE, "error: can only specify "
					"one option out of (json, jsonf, jsonr, jsonfr, csv) - csv ignored");
			} else {
				pTpe->data.field.options.bCSV = 1;
			}
		 } else if(!strcmp((char*)Buf, "json")) {
			if(hasFormat(pTpe)) {
				LogError(0, NO_ERRCODE, "error: can only specify "
					"one option out of (json, jsonf, jsonr, jsonfr, csv) - json ignored");
			} else {
				pTpe->data.field.options.bJSON = 1;
			}
		 } else if(!strcmp((char*)Buf, "jsonf")) {
			if(hasFormat(pTpe)) {
				LogError(0, NO_ERRCODE, "error: can only specify "
					"one option out of (json, jsonf, jsonr, jsonfr, csv) - jsonf ignored");
			} else {
				pTpe->data.field.options.bJSONf = 1;
			}
		 } else if(!strcmp((char*)Buf, "jsonr")) {
			if(hasFormat(pTpe)) {
				LogError(0, NO_ERRCODE, "error: can only specify "
					"one option out of (json, jsonf, jsonr, jsonfr, csv) - jsonr ignored");
			} else {
				pTpe->data.field.options.bJSONr = 1;
			}
		 } else if(!strcmp((char*)Buf, "jsonfr")) {
			if(hasFormat(pTpe)) {
				LogError(0, NO_ERRCODE, "error: can only specify "
					"one option out of (json, jsonf, jsonr, jsonfr, csv) - jsonfr ignored");
			} else {
				pTpe->data.field.options.bJSONfr = 1;
			}
		 } else if(!strcmp((char*)Buf, "mandatory-field")) {
			 pTpe->data.field.options.bMandatory = 1;
		 } else {
			LogError(0, NO_ERRCODE, "template error: invalid field option '%s' "
				"specified - ignored", Buf);
		 }
	}

	*pp = p;
}

/* helper to tplAddLine. Parses a parameter and generates
 * the necessary structure.
 */
static rsRetVal
do_Parameter(uchar **pp, struct template *pTpl)
{
	uchar *p;
	cstr_t *pStrProp = NULL;
	cstr_t *pStrField = NULL;
	struct templateEntry *pTpe;
	int iNum;	/* to compute numbers */
#ifdef FEATURE_REGEXP
	/* APR: variables for regex */
	rsRetVal iRetLocal;
	int longitud;
	unsigned char *regex_char;
	unsigned char *regex_end;
#endif
	DEFiRet;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pTpl != NULL);

	p = (uchar*) *pp;
	CHKiRet(cstrConstruct(&pStrProp));
	CHKmalloc(pTpe = tpeConstruct(pTpl));
	pTpe->eEntryType = FIELD;

	while(*p && *p != '%' && *p != ':') {
		cstrAppendChar(pStrProp, *p);
		++p;
	}

	/* got the name */
	cstrFinalize(pStrProp);

	CHKiRet(msgPropDescrFill(&pTpe->data.field.msgProp, cstrGetSzStrNoNULL(pStrProp),
		cstrLen(pStrProp)));

	/* Check frompos, if it has an R, then topos should be a regex */
	if(*p == ':') {
		pTpe->bComplexProcessing = 1;
		++p; /* eat ':' */
#ifdef FEATURE_REGEXP
		if(*p == 'R') {
			/* APR: R found! regex alarm ! :) */
			++p;	/* eat ':' */

			/* first come the regex type */
			if(*p == ',') {
				++p; /* eat ',' */
				if(p[0] == 'B' && p[1] == 'R' && p[2] == 'E' && (p[3] == ',' || p[3] == ':')) {
					pTpe->data.field.typeRegex = TPL_REGEX_BRE;
					p += 3; /* eat indicator sequence */
				} else if(p[0] == 'E' && p[1] == 'R' && p[2] == 'E' && (p[3] == ',' || p[3] == ':')) {
					pTpe->data.field.typeRegex = TPL_REGEX_ERE;
					p += 3; /* eat indicator sequence */
				} else {
					LogError(0, NO_ERRCODE, "error: invalid regular expression "
							"type, rest of line %s", (char*) p);
				}
			}

			/* now check for submatch ID */
			pTpe->data.field.iSubMatchToUse = 0;
			if(*p == ',') {
				/* in this case a number follows, which indicates which match
				 * shall be used. This must be a single digit.
				 */
				++p; /* eat ',' */
				if(isdigit((int) *p)) {
					pTpe->data.field.iSubMatchToUse = *p - '0';
					++p; /* eat digit */
				}
			}

			/* now pull what to do if we do not find a match */
			if(*p == ',') {
				++p; /* eat ',' */
				if(p[0] == 'D' && p[1] == 'F' && p[2] == 'L' && p[3] == 'T'
				   && (p[4] == ',' || p[4] == ':')) {
					pTpe->data.field.nomatchAction = TPL_REGEX_NOMATCH_USE_DFLTSTR;
					p += 4; /* eat indicator sequence */
				} else if(p[0] == 'B' && p[1] == 'L' && p[2] == 'A' && p[3] == 'N' && p[4] == 'K'
				          && (p[5] == ',' || p[5] == ':')) {
					pTpe->data.field.nomatchAction = TPL_REGEX_NOMATCH_USE_BLANK;
					p += 5; /* eat indicator sequence */
				} else if(p[0] == 'F' && p[1] == 'I' && p[2] == 'E' && p[3] == 'L' && p[4] == 'D'
				          && (p[5] == ',' || p[5] == ':')) {
					pTpe->data.field.nomatchAction = TPL_REGEX_NOMATCH_USE_WHOLE_FIELD;
					p += 5; /* eat indicator sequence */
				} else if(p[0] == 'Z' && p[1] == 'E' && p[2] == 'R' && p[3] == 'O'
				          && (p[4] == ',' || p[4] == ':')) {
					pTpe->data.field.nomatchAction = TPL_REGEX_NOMATCH_USE_ZERO;
					p += 4; /* eat indicator sequence */
				} else if(p[0] == ',') { /* empty, use default */
					pTpe->data.field.nomatchAction = TPL_REGEX_NOMATCH_USE_DFLTSTR;
					 /* do NOT eat indicator sequence, as this was already eaten - the
					  * comma itself is already part of the next field.
					  */
				} else {
					LogError(0, NO_ERRCODE, "template %s error: invalid regular "
						"expression type, rest of line %s",
						pTpl->pszName, (char*) p);
				}
			}

			/* now check for match ID */
			pTpe->data.field.iMatchToUse = 0;
			if(*p == ',') {
				/* in this case a number follows, which indicates which match
				 * shall be used. This must be a single digit.
				 */
				++p; /* eat ',' */
				if(isdigit((int) *p)) {
					pTpe->data.field.iMatchToUse = *p - '0';
					++p; /* eat digit */
				}
			}

			if(*p != ':') {
				/* There is something more than an R , this is invalid ! */
				/* Complain on extra characters */
				LogError(0, NO_ERRCODE, "error: invalid character in frompos "
						"after \"R\", property: '%%%s'", (char*) *pp);
			} else {
				pTpe->data.field.has_regex = 1;
				dbgprintf("we have a regexp and use match #%d, submatch #%d\n",
					  pTpe->data.field.iMatchToUse, pTpe->data.field.iSubMatchToUse);
			}
		} else {
			/* now we fall through the "regular" FromPos code */
#endif /* #ifdef FEATURE_REGEXP */
			if(*p == 'F') {
#ifdef STRICT_GPLV3
				pTpe->data.field.field_expand = 0;
#endif
				/* we have a field counter, so indicate it in the template */
				++p; /* eat 'F' */
				if (*p == ':') {
					/* no delimiter specified, so use the default (HT) */
					pTpe->data.field.has_fields = 1;
					pTpe->data.field.field_delim = 9;
				} else if (*p == ',') {
					++p; /* eat ',' */
					/* configured delimiter follows, so we need to obtain
					 * it. Important: the following number must be the
					 * **DECIMAL** ASCII value of the delimiter character.
					 */
					pTpe->data.field.has_fields = 1;
					if(!isdigit((int)*p)) {
						/* complain and use default */
						LogError(0, NO_ERRCODE, "error: invalid character in "
"frompos after \"F,\", property: '%%%s' - using 9 (HT) as field delimiter",
						    (char*) *pp);
						pTpe->data.field.field_delim = 9;
					} else {
						iNum = 0;
						while(isdigit((int)*p))
							iNum = iNum * 10 + *p++ - '0';
						if(iNum < 0 || iNum > 255) {
							LogError(0, NO_ERRCODE, "error: non-USASCII delimiter "
"character value %d in template - using 9 (HT) as substitute", iNum);
							pTpe->data.field.field_delim = 9;
						} else {
							pTpe->data.field.field_delim = iNum;
#							ifdef STRICT_GPLV3
							if (*p == '+') {
								pTpe->data.field.field_expand = 1;
								p ++;
							}
#							endif
							if(*p == ',') { /* real fromPos? */
								++p;
								iNum = 0;
								while(isdigit((int)*p))
									iNum = iNum * 10 + *p++ - '0';
								pTpe->data.field.iFromPos = iNum;
							} else if(*p != ':') {
								parser_errmsg("error: invalid character "
								"'%c' in frompos after \"F,\", property: '%s' "
								"be sure to use DECIMAL character "
								"codes!", *p, (char*) *pp);
								ABORT_FINALIZE(RS_RET_SYNTAX_ERROR);
							}
						}
					}
				} else {
					/* invalid character after F, so we need to reject
					 * this.
					 */
					LogError(0, NO_ERRCODE, "error: invalid character in frompos "
						"after \"F\", property: '%%%s'", (char*) *pp);
				}
			} else {
				/* we now have a simple offset in frompos (the previously "normal" case) */
				iNum = 0;
				while(isdigit((int)*p))
					iNum = iNum * 10 + *p++ - '0';
				pTpe->data.field.iFromPos = iNum;
				/* skip to next known good */
				while(*p && *p != '%' && *p != ':') {
					/* TODO: complain on extra characters */
					dbgprintf("error: extra character in frompos: '%s'\n", p);
					++p;
				}
			}
#ifdef FEATURE_REGEXP
		}
#endif /* #ifdef FEATURE_REGEXP */
	}
	/* check topos  (holds an regex if FromPos is "R"*/
	if(*p == ':') {
		++p; /* eat ':' */

#ifdef FEATURE_REGEXP
		if (pTpe->data.field.has_regex) {
			dbgprintf("debug: has regex \n");
			/* APR 2005-09 I need the string that represent the regex */
			/* The regex end is: "--end" */
			/* TODO : this is hardcoded and cant be escaped, please change */
			regex_end = (unsigned char*) strstr((char*)p, "--end");
			if (regex_end == NULL) {
				dbgprintf("error: can not find regex end in: '%s'\n", p);
				pTpe->data.field.has_regex = 0;
			} else {
				/* We get here ONLY if the regex end was found */
				longitud = regex_end - p;
				/* Malloc for the regex string */
				regex_char = (unsigned char *) MALLOC(longitud + 1);
				if(regex_char == NULL) {
					dbgprintf("Could not allocate memory for template parameter!\n");
					pTpe->data.field.has_regex = 0;
					ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
				}

				/* Get the regex string for compiling later */
				memcpy(regex_char, p, longitud);
				regex_char[longitud] = '\0';
				dbgprintf("debug: regex detected: '%s'\n", regex_char);
				/* Now i compile the regex */
				/* Remember that the re is an attribute of the Template entry */
				if((iRetLocal = objUse(regexp, LM_REGEXP_FILENAME)) == RS_RET_OK) {
					int iOptions;
					iOptions = (pTpe->data.field.typeRegex == TPL_REGEX_ERE) ? REG_EXTENDED : 0;
					int errcode;
					if((errcode = regexp.regcomp(&(pTpe->data.field.re),
						(char*) regex_char, iOptions) != 0)) {
						char errbuff[512];
						regexp.regerror(errcode, &(pTpe->data.field.re),
							errbuff, sizeof(errbuff));
						DBGPRINTF("Template.c: Error in regular expression: %s\n", errbuff);
						pTpe->data.field.has_regex = 2;
					}
				} else {
					/* regexp object could not be loaded */
					dbgprintf("error %d trying to load regexp library - this may be desired "
					"and thus OK", iRetLocal);
					if(bFirstRegexpErrmsg) {
					/* prevent flood of messages, maybe even an endless loop! */
						bFirstRegexpErrmsg = 0;
						LogError(0, NO_ERRCODE, "regexp library could not be loaded "
							"(error %d), regexp ignored", iRetLocal);
					}
					pTpe->data.field.has_regex = 2;
				}

				/* Finally we move the pointer to the end of the regex
				 * so it aint parsed twice or something weird */
				p = regex_end + 5/*strlen("--end")*/;
				free(regex_char);
			}
		} else if(*p == '$') {
			/* shortcut for "end of message */
			p++; /* eat '$' */
			/* in this case, we do a quick, somewhat dirty but totally
			 * legitimate trick: we simply use a topos that is higher than
			 * potentially ever can happen. The code below checks that no copy
			 * will occur after the end of string, so this is perfectly legal.
			 * rgerhards, 2006-10-17
			 */
			pTpe->data.field.iToPos = 9999999;
		} else {
			/* fallthrough to "regular" ToPos code */
#endif /* #ifdef FEATURE_REGEXP */

			if(pTpe->data.field.has_fields == 1) {
				iNum = 0;
				while(isdigit((int)*p))
					iNum = iNum * 10 + *p++ - '0';
				pTpe->data.field.iFieldNr = iNum;
				if(*p == ',') { /* get real toPos? */
					++p;
					iNum = 0;
					while(isdigit((int)*p))
						iNum = iNum * 10 + *p++ - '0';
					pTpe->data.field.iToPos = iNum;
				}
			} else {
				iNum = 0;
				while(isdigit((int)*p))
					iNum = iNum * 10 + *p++ - '0';
				pTpe->data.field.iToPos = iNum;
			}
			/* skip to next known good */
			while(*p && *p != '%' && *p != ':') {
				/* TODO: complain on extra characters */
				dbgprintf("error: extra character in frompos: '%s'\n", p);
				++p;
			}
#ifdef FEATURE_REGEXP
		}
#endif /* #ifdef FEATURE_REGEXP */
	}

	/* check options */
	if(*p == ':') {
		++p; /* eat ':' */
		doOptions(&p, pTpe);
	}

	if(pTpe->data.field.options.bFromPosEndRelative) {
		if(pTpe->data.field.iToPos > pTpe->data.field.iFromPos) {
			iNum = pTpe->data.field.iToPos;
			pTpe->data.field.iToPos = pTpe->data.field.iFromPos;
			pTpe->data.field.iFromPos = iNum;
		}
	} else {
		if(pTpe->data.field.iToPos < pTpe->data.field.iFromPos) {
			iNum = pTpe->data.field.iToPos;
			pTpe->data.field.iToPos = pTpe->data.field.iFromPos;
			pTpe->data.field.iFromPos = iNum;
		}
	}


	/* check field name */
	if(*p == ':') {
		++p; /* eat ':' */
		CHKiRet(cstrConstruct(&pStrField));
		while(*p != ':' && *p != '%' && *p != '\0') {
			cstrAppendChar(pStrField, *p);
			++p;
		}
		cstrFinalize(pStrField);
	}

	/* save field name - if none was given, use the property name instead */
	if(pStrField == NULL) {
		if(pTpe->data.field.msgProp.id == PROP_CEE || pTpe->data.field.msgProp.id == PROP_LOCAL_VAR) {
			/* in CEE case, we remove "$!"/"$." from the fieldname - it's just our indicator */
			pTpe->fieldName = ustrdup(cstrGetSzStrNoNULL(pStrProp)+2);
			pTpe->lenFieldName = cstrLen(pStrProp)-2;
		} else {
			pTpe->fieldName = ustrdup(cstrGetSzStrNoNULL(pStrProp));
			pTpe->lenFieldName = cstrLen(pStrProp);
		}
	} else {
		pTpe->fieldName = ustrdup(cstrGetSzStrNoNULL(pStrField));
		pTpe->lenFieldName = ustrlen(pTpe->fieldName);
		cstrDestruct(&pStrField);
	}
	if(pTpe->fieldName == NULL) {
		DBGPRINTF("template/do_Parameter: fieldName is NULL!\n");
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	if(*p) ++p; /* eat '%' */
	*pp = p;
finalize_it:
	if(pStrProp != NULL)
		cstrDestruct(&pStrProp);
	RETiRet;
}


/* Add a new entry for a template module.
 * returns pointer to new object if it succeeds, NULL otherwise.
 * rgerhards, 2010-05-31
 */
static rsRetVal
tplAddTplMod(struct template *pTpl, uchar** ppRestOfConfLine)
{
	uchar *pSrc;
	uchar szMod[2048];
	unsigned lenMod;
	strgen_t *pStrgen;
	DEFiRet;

	pSrc = *ppRestOfConfLine;
	lenMod = 0;
	while(*pSrc && !isspace(*pSrc) && lenMod < sizeof(szMod) - 1) {
		szMod[lenMod] = *pSrc++;
		lenMod++;

	}
	szMod[lenMod] = '\0';
	*ppRestOfConfLine = pSrc;
	CHKiRet(strgen.FindStrgen(&pStrgen, szMod));
	pTpl->pStrgen = pStrgen->pModule->mod.sm.strgen;
	DBGPRINTF("template bound to strgen '%s'\n", szMod);
	/* check if the name potentially contains some well-known options
	 * Note: we have opted to let the name contain all options. This sounds
	 * useful, because the strgen MUST actually implement a specific set
	 * of options. Doing this via the name looks to the enduser as if the
	 * regular syntax were used, and it make sure the strgen postively
	 * acknowledged implementing the option. -- rgerhards, 2011-03-21
	 */
	if(lenMod > 6 && !strcasecmp((char*) szMod + lenMod - 7, ",stdsql")) {
		pTpl->optFormatEscape = STDSQL_ESCAPE;
		DBGPRINTF("strgen supports the stdsql option\n");
	} else if(lenMod > 3 && !strcasecmp((char*) szMod+ lenMod - 4, ",sql")) {
		pTpl->optFormatEscape = SQL_ESCAPE;
		DBGPRINTF("strgen supports the sql option\n");
	} else if(lenMod > 4 && !strcasecmp((char*) szMod+ lenMod - 4, ",json")) {
		pTpl->optFormatEscape = JSON_ESCAPE;
		DBGPRINTF("strgen supports the json option\n");
	}

finalize_it:
	RETiRet;
}


/* Add a new template line
 * returns pointer to new object if it succeeds, NULL otherwise.
 */
struct template *tplAddLine(rsconf_t *conf, const char* pName, uchar** ppRestOfConfLine)
{
	struct template *pTpl;
	unsigned char *p;
	int bDone;
	size_t i;
	rsRetVal localRet;

	assert(pName != NULL);
	assert(ppRestOfConfLine != NULL);
	if((pTpl = tplConstruct(conf)) == NULL)
		return NULL;
	
	DBGPRINTF("tplAddLine processing template '%s'\n", pName);
	pTpl->iLenName = strlen(pName);
	pTpl->pszName = (char*) MALLOC(pTpl->iLenName + 1);
	if(pTpl->pszName == NULL) {
		dbgprintf("tplAddLine could not alloc memory for template name!");
		pTpl->iLenName = 0;
		return NULL;
		/* I know - we create a memory leak here - but I deem
		 * it acceptable as it is a) a very small leak b) very
		 * unlikely to happen. rgerhards 2004-11-17
		 */
	}
	memcpy(pTpl->pszName, pName, pTpl->iLenName + 1);

	/* now actually parse the line */
	p = *ppRestOfConfLine;
	assert(p != NULL);

	while(isspace((int)*p))/* skip whitespace */
		++p;
	
	switch(*p) {
	case '"': /* just continue */
		break;
	case '=':
		*ppRestOfConfLine = p + 1;
		localRet = tplAddTplMod(pTpl, ppRestOfConfLine);
		if(localRet != RS_RET_OK) {
			LogError(0, localRet, "Template '%s': error %d defining template via strgen module",
					pTpl->pszName, localRet);
			/* we simply make the template defunct in this case by setting
			 * its name to a zero-string. We do not free it, as this would
			 * require additional code and causes only a very small memory
			 * consumption. Memory is freed, however, in normal operation
			 * and most importantly by HUPing syslogd.
			 */
			*pTpl->pszName = '\0';
		}
		return NULL;
	default:
		dbgprintf("Template '%s' invalid, does not start with '\"'!\n", pTpl->pszName);
		/* we simply make the template defunct in this case by setting
		 * its name to a zero-string. We do not free it, as this would
		 * require additional code and causes only a very small memory
		 * consumption.
		 */
		*pTpl->pszName = '\0';
		return NULL;
	}
	++p;

	/* we finally go to the actual template string - so let's have some fun... */
	bDone = *p ? 0 : 1;
	while(!bDone) {
		switch(*p) {
			case '\0':
				bDone = 1;
				break;
			case '%': /* parameter */
				++p; /* eat '%' */
				if(do_Parameter(&p, pTpl) != RS_RET_OK) {
					dbgprintf("tplAddLine error: parameter invalid");
					return NULL;
				};
				break;
			default: /* constant */
				do_Constant(&p, pTpl, 1);
				break;
		}
		if(*p == '"') {/* end of template string? */
			++p;	/* eat it! */
			bDone = 1;
		}
	}
	
	/* we now have the template - let's look at the options (if any)
	 * we process options until we reach the end of the string or
	 * an error occurs - whichever is first.
	 */
	while(*p) {
		while(isspace((int)*p))/* skip whitespace */
			++p;

		if(*p != ',')
			break;
		++p; /* eat ',' */

		while(isspace((int)*p))/* skip whitespace */
			++p;

		/* read option word */
		char optBuf[128] = { '\0' }; /* buffer for options - should be more than enough... */
		i = 0;
		while((i < (sizeof(optBuf) - 1))
		      && *p && *p != '=' && *p !=',' && *p != '\n') {
			optBuf[i++] = tolower((int)*p);
			++p;
		}
		optBuf[i] = '\0';

		if(*p == '\n')
			++p;

		/* as of now, the no form is nonsense... but I do include
		 * it anyhow... ;) rgerhards 2004-11-22
		 */
		if(!strcmp(optBuf, "stdsql")) {
			pTpl->optFormatEscape = STDSQL_ESCAPE;
		} else if(!strcmp(optBuf, "json")) {
			pTpl->optFormatEscape = JSON_ESCAPE;
		} else if(!strcmp(optBuf, "sql")) {
			pTpl->optFormatEscape = SQL_ESCAPE;
		} else if(!strcmp(optBuf, "nosql")) {
			pTpl->optFormatEscape = NO_ESCAPE;
		} else if(!strcmp(optBuf, "casesensitive")) {
			pTpl->optCaseSensitive = 1;
		} else {
			dbgprintf("Invalid option '%s' ignored.\n", optBuf);
		}
	}

	*ppRestOfConfLine = p;
	apply_case_sensitivity(pTpl);

	return(pTpl);
}

static rsRetVal
createConstantTpe(struct template *pTpl, struct cnfobj *o)
{
	struct templateEntry *pTpe;
	es_str_t *value = NULL; /* init just to keep compiler happy - mandatory parameter */
	int i;
	int is_jsonf = 0;
	struct cnfparamvals *pvals = NULL;
	struct json_object *json = NULL;
	struct json_object *jval = NULL;
	uchar *outname = NULL;
	DEFiRet;

	/* pull params */
	pvals = nvlstGetParams(o->nvlst, &pblkConstant, NULL);
	if(pvals == NULL) {
		parser_errmsg("error processing template parameters");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	cnfparamsPrint(&pblkConstant, pvals);
	
	for(i = 0 ; i < pblkConstant.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(pblkConstant.descr[i].name, "value")) {
			value = pvals[i].val.d.estr;
		} else if(!strcmp(pblkConstant.descr[i].name, "outname")) {
			outname = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblkConstant.descr[i].name, "format")) {
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"jsonf", sizeof("jsonf")-1)) {
				is_jsonf = 1;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid format type '%s' for constant",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else {
			LogError(0, RS_RET_INTERNAL_ERROR,
				"template:constantTpe: program error, non-handled "
				"param '%s'\n", pblkConstant.descr[i].name);
		}
	}

	if(is_jsonf && outname == NULL) {
		parser_errmsg("constant set to format jsonf, but outname not specified - aborting");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	/* just double-check */
	assert(value != NULL);

	/* apply */
	CHKmalloc(pTpe = tpeConstruct(pTpl));
	es_unescapeStr(value);
	pTpe->eEntryType = CONSTANT;
	pTpe->fieldName = outname;
	if(outname != NULL)
		pTpe->lenFieldName = ustrlen(outname);
	if(is_jsonf) {
		CHKmalloc(json = json_object_new_object());
		const char *sz = es_str2cstr(value, NULL);
		CHKmalloc(sz);
		CHKmalloc(jval = json_object_new_string(sz));
		free((void*)sz);
		json_object_object_add(json, (char*)outname, jval);
		CHKmalloc(sz = json_object_get_string(json));
		const size_t len_json = strlen(sz) - 4;
		CHKmalloc(pTpe->data.constant.pConstant = (uchar*) strndup(sz+2, len_json));
		pTpe->data.constant.iLenConstant = ustrlen(pTpe->data.constant.pConstant);
		json_object_put(json);
	} else {
		pTpe->data.constant.iLenConstant = es_strlen(value);
		pTpe->data.constant.pConstant = (uchar*)es_str2cstr(value, NULL);
	}

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &pblkConstant);
	RETiRet;
}

static rsRetVal
createPropertyTpe(struct template *pTpl, struct cnfobj *o)
{
	struct templateEntry *pTpe;
	uchar *name = NULL;
	uchar *outname = NULL;
	int i;
	int droplastlf = 0;
	int spifno1stsp = 0;
	int mandatory = 0;
	int frompos = -1;
	int topos = -1;
	int fieldnum = -1;
	int fielddelim = 9; /* default is HT (USACSII 9) */
	int fixedwidth = 0;
	int re_matchToUse = 0;
	int re_submatchToUse = 0;
	int bComplexProcessing = 0;
	int bPosRelativeToEnd = 0;
	int bDateInUTC = 0;
	int bCompressSP = 0;
	char *re_expr = NULL;
	struct cnfparamvals *pvals = NULL;
	enum {F_NONE, F_CSV, F_JSON, F_JSONF, F_JSONR, F_JSONFR} formatType = F_NONE;
	enum {CC_NONE, CC_ESCAPE, CC_SPACE, CC_DROP} controlchr = CC_NONE;
	enum {SP_NONE, SP_DROP, SP_REPLACE} secpath = SP_NONE;
	enum tplFormatCaseConvTypes caseconv = tplCaseConvNo;
	enum tplFormatTypes datefmt = tplFmtDefault;
	enum tplRegexType re_type = TPL_REGEX_BRE;
	enum tlpRegexNoMatchType re_nomatchType = TPL_REGEX_NOMATCH_USE_DFLTSTR;
	DEFiRet;

	/* pull params */
	pvals = nvlstGetParams(o->nvlst, &pblkProperty, NULL);
	if(pvals == NULL) {
		parser_errmsg("error processing template entry config parameters");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	cnfparamsPrint(&pblkProperty, pvals);
	
	for(i = 0 ; i < pblkProperty.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(pblkProperty.descr[i].name, "name")) {
			name = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblkProperty.descr[i].name, "droplastlf")) {
			droplastlf = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "fixedwidth")) {
			fixedwidth = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "mandatory")) {
			mandatory = pvals[i].val.d.n;
		} else if(!strcmp(pblkProperty.descr[i].name, "spifno1stsp")) {
			spifno1stsp = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "outname")) {
			outname = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblkProperty.descr[i].name, "position.from")) {
			frompos = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "position.to")) {
			topos = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "position.relativetoend")) {
			bPosRelativeToEnd = pvals[i].val.d.n;
		} else if(!strcmp(pblkProperty.descr[i].name, "field.number")) {
			fieldnum = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "field.delimiter")) {
			fielddelim = pvals[i].val.d.n;
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "regex.expression")) {
			re_expr = es_str2cstr(pvals[i].val.d.estr, NULL);
			bComplexProcessing = 1;
		} else if(!strcmp(pblkProperty.descr[i].name, "regex.type")) {
			bComplexProcessing = 1;
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"BRE", sizeof("BRE")-1)) {
				re_type = TPL_REGEX_BRE;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"ERE", sizeof("ERE")-1)) {
				re_type = TPL_REGEX_ERE;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid regex.type '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblkProperty.descr[i].name, "regex.nomatchmode")) {
			bComplexProcessing = 1;
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"DFLT", sizeof("DFLT")-1)) {
				re_nomatchType = TPL_REGEX_NOMATCH_USE_DFLTSTR;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"BLANK", sizeof("BLANK")-1)) {
				re_nomatchType = TPL_REGEX_NOMATCH_USE_BLANK;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"FIELD", sizeof("FIELD")-1)) {
				re_nomatchType = TPL_REGEX_NOMATCH_USE_WHOLE_FIELD;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"ZERO", sizeof("ZERO")-1)) {
				re_nomatchType = TPL_REGEX_NOMATCH_USE_ZERO;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid format type '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblkProperty.descr[i].name, "regex.match")) {
			bComplexProcessing = 1;
			re_matchToUse = pvals[i].val.d.n;
		} else if(!strcmp(pblkProperty.descr[i].name, "regex.submatch")) {
			bComplexProcessing = 1;
			re_submatchToUse = pvals[i].val.d.n;
		} else if(!strcmp(pblkProperty.descr[i].name, "format")) {
			bComplexProcessing = 1;
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"csv", sizeof("csv")-1)) {
				formatType = F_CSV;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"json", sizeof("json")-1)) {
				formatType = F_JSON;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"jsonf", sizeof("jsonf")-1)) {
				formatType = F_JSONF;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"jsonr", sizeof("jsonr")-1)) {
				formatType = F_JSONR;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"jsonfr", sizeof("jsonfr")-1)) {
				formatType = F_JSONFR;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid format type '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblkProperty.descr[i].name, "controlcharacters")) {
			bComplexProcessing = 1;
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"escape", sizeof("escape")-1)) {
				controlchr = CC_ESCAPE;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"space", sizeof("space")-1)) {
				controlchr = CC_SPACE;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"drop", sizeof("drop")-1)) {
				controlchr = CC_DROP;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid controlcharacter mode '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblkProperty.descr[i].name, "securepath")) {
			bComplexProcessing = 1;
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"drop", sizeof("drop")-1)) {
				secpath = SP_DROP;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"replace", sizeof("replace")-1)) {
				secpath = SP_REPLACE;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid securepath mode '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblkProperty.descr[i].name, "caseconversion")) {
			bComplexProcessing = 1;
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"lower", sizeof("lower")-1)) {
				caseconv = tplCaseConvLower;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"upper", sizeof("upper")-1)) {
				caseconv = tplCaseConvUpper;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid caseconversion type '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblkProperty.descr[i].name, "compressspace")) {
			bComplexProcessing = 1;
			bCompressSP = pvals[i].val.d.n;
		} else if(!strcmp(pblkProperty.descr[i].name, "date.inutc")) {
			bDateInUTC = pvals[i].val.d.n;
		} else if(!strcmp(pblkProperty.descr[i].name, "dateformat")) {
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"mysql", sizeof("mysql")-1)) {
				datefmt = tplFmtMySQLDate;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"pgsql", sizeof("pgsql")-1)) {
				datefmt = tplFmtPgSQLDate;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"rfc3164", sizeof("rfc3164")-1)) {
				datefmt = tplFmtRFC3164Date;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"rfc3164-buggyday",
				sizeof("rfc3164-buggyday")-1)) {
				datefmt = tplFmtRFC3164BuggyDate;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"rfc3339", sizeof("rfc3339")-1)) {
				datefmt = tplFmtRFC3339Date;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"unixtimestamp",
			sizeof("unixtimestamp")-1)) {
				datefmt = tplFmtUnixDate;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"subseconds", sizeof("subseconds")-1)) {
				datefmt = tplFmtSecFrac;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"wdayname", sizeof("wdayname")-1)) {
				datefmt = tplFmtWDayName;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"wday", sizeof("wday")-1)) {
				datefmt = tplFmtWDay;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"year", sizeof("year")-1)) {
				datefmt = tplFmtYear;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"month", sizeof("month")-1)) {
				datefmt = tplFmtMonth;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"day", sizeof("day")-1)) {
				datefmt = tplFmtDay;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"hour", sizeof("hour")-1)) {
				datefmt = tplFmtHour;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"minute", sizeof("minute")-1)) {
				datefmt = tplFmtMinute;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"second", sizeof("second")-1)) {
				datefmt = tplFmtSecond;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"tzoffshour", sizeof("tzoffshour")-1)) {
				datefmt = tplFmtTZOffsHour;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"tzoffsmin", sizeof("tzoffsmin")-1)) {
				datefmt = tplFmtTZOffsMin;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"tzoffsdirection",
				sizeof("tzoffsdirection")-1)) {
				datefmt = tplFmtTZOffsDirection;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"ordinal", sizeof("ordinal")-1)) {
				datefmt = tplFmtOrdinal;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"week", sizeof("week")-1)) {
				datefmt = tplFmtWeek;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid date format '%s' for property",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else {
			dbgprintf("template:propertyTpe: program error, non-handled "
			  "param '%s'\n", pblkProperty.descr[i].name);
		}
	}
	if (name == NULL) {
		CHKmalloc(name = (uchar*)strdup(""));
	}
	if(outname == NULL) {
		/* we need to drop "$!" prefix, if present */
		if(ustrlen(name) >= 2 && !strncmp((char*)name, "$!", 2))
			outname = ustrdup(name + 2);
		else
			outname = ustrdup(name);
	}

	/* sanity check */
	if(topos == -1 && frompos != -1)
		topos = 2000000000; /* large enough ;) */
	if(frompos == -1 && topos != -1)
		frompos = 0;
	if(bPosRelativeToEnd) {
		if(topos > frompos) {
			LogError(0, RS_RET_ERR, "position.to=%d is higher than postion.from=%d "
					"in 'relativeToEnd' mode\n", topos, frompos);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	} else {
		if(topos < frompos) {
			LogError(0, RS_RET_ERR, "position.to=%d is lower than postion.from=%d\n",
				topos, frompos);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
	if(fieldnum != -1 && re_expr != NULL) {
		LogError(0, RS_RET_ERR, "both field extraction and regex extraction "
				"specified - this is not possible, remove one");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	/* apply */
	CHKmalloc(pTpe = tpeConstruct(pTpl));
	pTpe->eEntryType = FIELD;
	CHKiRet(msgPropDescrFill(&pTpe->data.field.msgProp, name, strlen((char*)name)));
	pTpe->data.field.options.bDropLastLF = droplastlf;
	pTpe->data.field.options.bSPIffNo1stSP = spifno1stsp;
	pTpe->data.field.options.bMandatory = mandatory;
	pTpe->data.field.options.bFixedWidth = fixedwidth;
	pTpe->data.field.eCaseConv = caseconv;
	switch(formatType) {
	case F_NONE:
		/* all set ;) */
		break;
	case F_CSV:
		pTpe->data.field.options.bCSV = 1;
		break;
	case F_JSON:
		pTpe->data.field.options.bJSON = 1;
		break;
	case F_JSONF:
		pTpe->data.field.options.bJSONf = 1;
		break;
	case F_JSONR:
		pTpe->data.field.options.bJSONr = 1;
		break;
	case F_JSONFR:
		pTpe->data.field.options.bJSONfr = 1;
		break;
	}
	switch(controlchr) {
	case CC_NONE:
		/* all set ;) */
		break;
	case CC_ESCAPE:
		pTpe->data.field.options.bEscapeCC = 1;
		break;
	case CC_SPACE:
		pTpe->data.field.options.bSpaceCC = 1;
		break;
	case CC_DROP:
		pTpe->data.field.options.bDropCC = 1;
		break;
	}
	switch(secpath) {
	case SP_NONE:
		/* all set ;) */
		break;
	case SP_DROP:
		pTpe->data.field.options.bSecPathDrop = 1;
		break;
	case SP_REPLACE:
		pTpe->data.field.options.bSecPathReplace = 1;
		break;
	}
	pTpe->fieldName = outname;
	if(outname != NULL)
		pTpe->lenFieldName = ustrlen(outname);
	outname = NULL;
	pTpe->bComplexProcessing = bComplexProcessing;
	pTpe->data.field.eDateFormat = datefmt;
	pTpe->data.field.options.bDateInUTC = bDateInUTC;
	pTpe->data.field.options.bCompressSP = bCompressSP;
	if(fieldnum != -1) {
		pTpe->data.field.has_fields = 1;
		pTpe->data.field.iFieldNr = fieldnum;
		pTpe->data.field.field_delim = fielddelim;
	}
	if(frompos != -1) {
		pTpe->data.field.iFromPos = frompos;
		pTpe->data.field.iToPos = topos;
		pTpe->data.field.options.bFromPosEndRelative = bPosRelativeToEnd;
	}
	if(re_expr != NULL) {
		rsRetVal iRetLocal;
		pTpe->data.field.typeRegex = re_type;
		pTpe->data.field.nomatchAction = re_nomatchType;
		pTpe->data.field.iMatchToUse = re_matchToUse;
		pTpe->data.field.iSubMatchToUse = re_submatchToUse;
		pTpe->data.field.has_regex = 1;
		if((iRetLocal = objUse(regexp, LM_REGEXP_FILENAME)) == RS_RET_OK) {
			int iOptions;
			iOptions = (pTpe->data.field.typeRegex == TPL_REGEX_ERE) ? REG_EXTENDED : 0;
			if(regexp.regcomp(&(pTpe->data.field.re), (char*) re_expr, iOptions) != 0) {
				LogError(0, NO_ERRCODE, "error compiling regex '%s'", re_expr);
				pTpe->data.field.has_regex = 2;
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else {
			/* regexp object could not be loaded */
			if(bFirstRegexpErrmsg) { /* prevent flood of messages, maybe even an endless loop! */
				bFirstRegexpErrmsg = 0;
				LogError(0, NO_ERRCODE, "regexp library could not be loaded (error %d), "
						"regexp ignored", iRetLocal);
			}
			pTpe->data.field.has_regex = 2;
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &pblkProperty);
	free(name);
	free(outname);
	RETiRet;
}

/* create a template in list mode, is build from sub-objects */
static rsRetVal
createListTpl(struct template *pTpl, struct cnfobj *o)
{
	struct objlst *lst;
	DEFiRet;

	dbgprintf("create template from subobjs\n");
	objlstPrint(o->subobjs);

	for(lst = o->subobjs ; lst != NULL ; lst = lst->next) {
		switch(lst->obj->objType) {
		case CNFOBJ_PROPERTY:
			CHKiRet(createPropertyTpe(pTpl, lst->obj));
			break;
		case CNFOBJ_CONSTANT:
			CHKiRet(createConstantTpe(pTpl, lst->obj));
			break;
		default:dbgprintf("program error: invalid object type %d "
				  "in createLstTpl\n", lst->obj->objType);
			break;
		}
		nvlstChkUnused(lst->obj->nvlst);
	}
finalize_it:
	RETiRet;
}

/* Add a new template via the v6 config system.  */
rsRetVal ATTR_NONNULL()
tplProcessCnf(struct cnfobj *o)
{
	struct template *pTpl = NULL;
	struct cnfparamvals *pvals = NULL;
	int lenName = 0; /* init just to keep compiler happy: mandatory parameter */
	char *name = NULL;
	uchar *tplStr = NULL;
	uchar *plugin = NULL;
	uchar *p;
	msgPropDescr_t subtree;
	sbool bHaveSubtree = 0;
	enum { T_STRING, T_PLUGIN, T_LIST, T_SUBTREE }
		tplType = T_STRING; /* init just to keep compiler happy: mandatory parameter */
	int i;
	int o_sql=0, o_stdsql=0, o_jsonf=0, o_json=0, o_casesensitive=0; /* options */
	int numopts;
	rsRetVal localRet;
	DEFiRet;

	pvals = nvlstGetParams(o->nvlst, &pblk, NULL);
	if(pvals == NULL) {
		parser_errmsg("error processing template config parameters");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	cnfparamsPrint(&pblk, pvals);
	
	for(i = 0 ; i < pblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(pblk.descr[i].name, "name")) {
			lenName = es_strlen(pvals[i].val.d.estr);
			name = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblk.descr[i].name, "type")) {
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"string", sizeof("string")-1)) {
				tplType = T_STRING;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"plugin", sizeof("plugin")-1)) {
				tplType = T_PLUGIN;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"list", sizeof("list")-1)) {
				tplType = T_LIST;
			} else if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"subtree", sizeof("subtree")-1)) {
				tplType = T_SUBTREE;
			} else {
				uchar *typeStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid template type '%s'",
					typeStr);
				free(typeStr);
				ABORT_FINALIZE(RS_RET_ERR);
			}
		} else if(!strcmp(pblk.descr[i].name, "string")) {
			tplStr = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblk.descr[i].name, "subtree")) {
			uchar *st_str = es_getBufAddr(pvals[i].val.d.estr);
			if(st_str[0] != '$' || st_str[1] != '!') {
				char *cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_ERR, "invalid subtree "
					"parameter, variable must start with '$!' but "
					"var name is '%s'", cstr);
				free(cstr);
				free(name); /* overall assigned */
				ABORT_FINALIZE(RS_RET_ERR);
			} else {
				uchar *cstr;
				cstr  = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
				CHKiRet(msgPropDescrFill(&subtree, cstr, ustrlen(cstr)));
				free(cstr);
				bHaveSubtree = 1;
			}
		} else if(!strcmp(pblk.descr[i].name, "plugin")) {
			plugin = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblk.descr[i].name, "option.stdsql")) {
			o_stdsql = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "option.sql")) {
			o_sql = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "option.json")) {
			o_json = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "option.jsonf")) {
			o_jsonf = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "option.casesensitive")) {
			o_casesensitive = pvals[i].val.d.n;
		} else {
			dbgprintf("template: program error, non-handled "
			  "param '%s'\n", pblk.descr[i].name);
		}
	}

	/* the following check is just for clang static anaylzer: this condition
	 * cannot occur if all is setup well, because "name" is a required parameter
	 * inside the param block and so the code should err out above.
	 */
	if(name == NULL) {
		DBGPRINTF("template/tplProcessConf: logic error name == NULL - pblk wrong?\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	/* do config sanity checks */
	if(tplStr  == NULL) {
		if(tplType == T_STRING) {
			LogError(0, RS_RET_ERR, "template '%s' of type string needs "
				"string parameter", name);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	} else {
		if(tplType != T_STRING) {
			LogError(0, RS_RET_ERR, "template '%s' is not a string "
				"template but has a string specified - ignored", name);
		}
	}

	if(plugin  == NULL) {
		if(tplType == T_PLUGIN) {
			LogError(0, RS_RET_ERR, "template '%s' of type plugin needs "
				"plugin parameter", name);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	} else {
		if(tplType != T_PLUGIN) {
			LogError(0, RS_RET_ERR, "template '%s' is not a plugin "
				"template but has a plugin specified - ignored", name);
		}
	}

	if(!bHaveSubtree) {
		if(tplType == T_SUBTREE) {
			LogError(0, RS_RET_ERR, "template '%s' of type subtree needs "
				"subtree parameter", name);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	} else {
		if(tplType != T_SUBTREE) {
			LogError(0, RS_RET_ERR, "template '%s' is not a subtree "
				"template but has a subtree specified - ignored", name);
		}
	}

	if(o->subobjs  == NULL) {
		if(tplType == T_LIST) {
			LogError(0, RS_RET_ERR, "template '%s' of type list has "
				"has no parameters specified", name);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	} else {
		if(tplType != T_LIST) {
			LogError(0, RS_RET_ERR, "template '%s' is not a list "
				"template but has parameters specified - ignored", name);
		}
	}

	numopts = 0;
	if(o_sql) ++numopts;
	if(o_stdsql) ++numopts;
	if(o_json) ++numopts;
	if(o_jsonf) ++numopts;
	if(numopts > 1) {
		LogError(0, RS_RET_ERR, "template '%s' has multiple incompatible "
			"options of sql, stdsql or json specified", name);
		ABORT_FINALIZE(RS_RET_ERR);
	}

	/* config ok */
	if((pTpl = tplConstruct(loadConf)) == NULL) {
		DBGPRINTF("template.c: tplConstruct failed!\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	pTpl->pszName = name;
	pTpl->iLenName = lenName;
	
	switch(tplType) {
	case T_STRING:	p = tplStr;
			while(*p) {
				switch(*p) {
					case '%': /* parameter */
						++p; /* eat '%' */
						CHKiRet(do_Parameter(&p, pTpl));
						break;
					default: /* constant */
						do_Constant(&p, pTpl, 0);
						break;
				}
			}
			break;
	case T_PLUGIN:	p = plugin;
			/* TODO: the use of tplAddTplMod() can be improved! */
			localRet = tplAddTplMod(pTpl, &p);
			if(localRet != RS_RET_OK) {
				LogError(0, localRet, "template '%s': error %d "
						"defining template via plugin (strgen) module",
						pTpl->pszName, localRet);
				ABORT_FINALIZE(localRet);
			}
			break;
	case T_LIST:	createListTpl(pTpl, o);
			break;
	case T_SUBTREE:	memcpy(&pTpl->subtree, &subtree, sizeof(msgPropDescr_t));
			pTpl->bHaveSubtree = 1;
			break;
	}
	
	pTpl->optFormatEscape = NO_ESCAPE;
	if(o_stdsql)
		pTpl->optFormatEscape = STDSQL_ESCAPE;
	else if(o_sql)
		pTpl->optFormatEscape = SQL_ESCAPE;
	else if(o_json)
		pTpl->optFormatEscape = JSON_ESCAPE;
	else if(o_jsonf)
		pTpl->optFormatEscape = JSONF;

	if(o_casesensitive)
		pTpl->optCaseSensitive = 1;
	apply_case_sensitivity(pTpl);
finalize_it:
	free(tplStr);
	free(plugin);
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &pblk);
	if(iRet != RS_RET_OK) {
		if(pTpl != NULL) {
			/* we simply make the template defunct in this case by setting
			 * its name to a zero-string. We do not free it, as this would
			 * require additional code and causes only a very small memory
			 * consumption. TODO: maybe in next iteration...
			 */
			*pTpl->pszName = '\0';
		}
	}

	RETiRet;
}


/* Find a template object based on name. Search
 * currently is case-sensitive (should we change?).
 * returns pointer to template object if found and
 * NULL otherwise.
 * rgerhards 2004-11-17
 */
struct template *tplFind(rsconf_t *conf, char *pName, int iLenName)
{
	struct template *pTpl;

	assert(pName != NULL);

	pTpl = conf->templates.root;
	while(pTpl != NULL &&
	      !(pTpl->iLenName == iLenName &&
	        !strcmp(pTpl->pszName, pName)
	        ))
		{
			pTpl = pTpl->pNext;
		}
	return(pTpl);
}

/* Destroy the template structure. This is for de-initialization
 * at program end. Everything is deleted.
 * rgerhards 2005-02-22
 * I have commented out dbgprintfs, because they are not needed for
 * "normal" debugging. Uncomment them, if they are needed.
 * rgerhards, 2007-07-05
 */
void tplDeleteAll(rsconf_t *conf)
{
	struct template *pTpl, *pTplDel;
	struct templateEntry *pTpe, *pTpeDel;
	BEGINfunc

	pTpl = conf->templates.root;
	while(pTpl != NULL) {
		/* dbgprintf("Delete Template: Name='%s'\n ", pTpl->pszName == NULL? "NULL" : pTpl->pszName);*/
		pTpe = pTpl->pEntryRoot;
		while(pTpe != NULL) {
			pTpeDel = pTpe;
			pTpe = pTpe->pNext;
			/*dbgprintf("\tDelete Entry(%x): type %d, ", (unsigned) pTpeDel, pTpeDel->eEntryType);*/
			switch(pTpeDel->eEntryType) {
			case UNDEFINED:
				/*dbgprintf("(UNDEFINED)");*/
				break;
			case CONSTANT:
				/*dbgprintf("(CONSTANT), value: '%s'",
					pTpeDel->data.constant.pConstant);*/
				free(pTpeDel->data.constant.pConstant);
				break;
			case FIELD:
				/* check if we have a regexp and, if so, delete it */
#ifdef FEATURE_REGEXP
				if(pTpeDel->data.field.has_regex != 0) {
					if(objUse(regexp, LM_REGEXP_FILENAME) == RS_RET_OK) {
						regexp.regfree(&(pTpeDel->data.field.re));
					}
				}
#endif
				msgPropDescrDestruct(&pTpeDel->data.field.msgProp);
				break;
			}
			free(pTpeDel->fieldName);
			/*dbgprintf("\n");*/
			free(pTpeDel);
		}
		pTplDel = pTpl;
		pTpl = pTpl->pNext;
		free(pTplDel->pszName);
		if(pTplDel->bHaveSubtree)
			msgPropDescrDestruct(&pTplDel->subtree);
		free(pTplDel);
	}
	ENDfunc
}


/* Destroy all templates obtained from conf file
 * preserving hardcoded ones. This is called from init().
 */
void tplDeleteNew(rsconf_t *conf)
{
	struct template *pTpl, *pTplDel;
	struct templateEntry *pTpe, *pTpeDel;

	BEGINfunc

	if(conf->templates.root == NULL || conf->templates.lastStatic == NULL)
		return;

	pTpl = conf->templates.lastStatic->pNext;
	conf->templates.lastStatic->pNext = NULL;
	conf->templates.last = conf->templates.lastStatic;
	while(pTpl != NULL) {
		/* dbgprintf("Delete Template: Name='%s'\n ", pTpl->pszName == NULL? "NULL" : pTpl->pszName);*/
		pTpe = pTpl->pEntryRoot;
		while(pTpe != NULL) {
			pTpeDel = pTpe;
			pTpe = pTpe->pNext;
			/*dbgprintf("\tDelete Entry(%x): type %d, ", (unsigned) pTpeDel, pTpeDel->eEntryType);*/
			switch(pTpeDel->eEntryType) {
			case UNDEFINED:
				/*dbgprintf("(UNDEFINED)");*/
				break;
			case CONSTANT:
				/*dbgprintf("(CONSTANT), value: '%s'",
					pTpeDel->data.constant.pConstant);*/
				free(pTpeDel->data.constant.pConstant);
				break;
			case FIELD:
#ifdef FEATURE_REGEXP
				/* check if we have a regexp and, if so, delete it */
				if(pTpeDel->data.field.has_regex != 0) {
					if(objUse(regexp, LM_REGEXP_FILENAME) == RS_RET_OK) {
						regexp.regfree(&(pTpeDel->data.field.re));
					}
				}
#endif
				msgPropDescrDestruct(&pTpeDel->data.field.msgProp);
				break;
			}
			/*dbgprintf("\n");*/
			free(pTpeDel);
		}
		pTplDel = pTpl;
		pTpl = pTpl->pNext;
		free(pTplDel->pszName);
		if(pTplDel->bHaveSubtree)
			msgPropDescrDestruct(&pTplDel->subtree);
		free(pTplDel);
	}
	ENDfunc
}

/* Store the pointer to the last hardcoded teplate */
void tplLastStaticInit(rsconf_t *conf, struct template *tpl)
{
	conf->templates.lastStatic = tpl;
}

/* Print the template structure. This is more or less a
 * debug or test aid, but anyhow I think it's worth it...
 */
void tplPrintList(rsconf_t *conf)
{
	struct template *pTpl;
	struct templateEntry *pTpe;

	pTpl = conf->templates.root;
	while(pTpl != NULL) {
		dbgprintf("Template: Name='%s' ", pTpl->pszName == NULL? "NULL" : pTpl->pszName);
		if(pTpl->optFormatEscape == SQL_ESCAPE)
			dbgprintf("[SQL-Format (MySQL)] ");
		else if(pTpl->optFormatEscape == JSON_ESCAPE)
			dbgprintf("[JSON-Escaped Format] ");
		else if(pTpl->optFormatEscape == STDSQL_ESCAPE)
			dbgprintf("[SQL-Format (standard SQL)] ");
		else if(pTpl->optFormatEscape == JSONF)
			dbgprintf("[JSON fields] ");
		if(pTpl->optCaseSensitive)
			dbgprintf("[Case Sensitive Vars] ");
		dbgprintf("\n");
		pTpe = pTpl->pEntryRoot;
		while(pTpe != NULL) {
			dbgprintf("\tEntry(%lx): type %d, ", (unsigned long) pTpe, pTpe->eEntryType);
			switch(pTpe->eEntryType) {
			case UNDEFINED:
				dbgprintf("(UNDEFINED)");
				break;
			case CONSTANT:
				dbgprintf("(CONSTANT), value: '%s'",
					pTpe->data.constant.pConstant);
				break;
			case FIELD:
				dbgprintf("(FIELD), value: '%d' ", pTpe->data.field.msgProp.id);
				if(pTpe->data.field.msgProp.id == PROP_CEE) {
					dbgprintf("[EE-Property: '%s'] ", pTpe->data.field.msgProp.name);
				} else if(pTpe->data.field.msgProp.id == PROP_LOCAL_VAR) {
					dbgprintf("[Local Var: '%s'] ", pTpe->data.field.msgProp.name);
				//} else if(pTpe->data.field.propid == PROP_GLOBAL_VAR) {
				//	dbgprintf("[Global Var: '%s'] ", pTpe->data.field.propName);
				}
				switch(pTpe->data.field.eDateFormat) {
				case tplFmtDefault:
					break;
				case tplFmtMySQLDate:
					dbgprintf("[Format as MySQL-Date] ");
					break;
				case tplFmtPgSQLDate:
					dbgprintf("[Format as PgSQL-Date] ");
					break;
				case tplFmtRFC3164Date:
					dbgprintf("[Format as RFC3164-Date] ");
					break;
				case tplFmtRFC3339Date:
					dbgprintf("[Format as RFC3339-Date] ");
					break;
				case tplFmtUnixDate:
					dbgprintf("[Format as Unix timestamp] ");
					break;
				case tplFmtSecFrac:
					dbgprintf("[fractional seconds, only] ");
					break;
				case tplFmtRFC3164BuggyDate:
					dbgprintf("[Format as buggy RFC3164-Date] ");
					break;
				default:
					dbgprintf("[UNKNOWN eDateFormat %d] ", pTpe->data.field.eDateFormat);
				}
				switch(pTpe->data.field.eCaseConv) {
				case tplCaseConvNo:
					break;
				case tplCaseConvLower:
					dbgprintf("[Converted to Lower Case] ");
					break;
				case tplCaseConvUpper:
					dbgprintf("[Converted to Upper Case] ");
					break;
				}
				if(pTpe->data.field.options.bEscapeCC) {
				  	dbgprintf("[escape control-characters] ");
				}
				if(pTpe->data.field.options.bDropCC) {
				  	dbgprintf("[drop control-characters] ");
				}
				if(pTpe->data.field.options.bSpaceCC) {
				  	dbgprintf("[replace control-characters with space] ");
				}
				if(pTpe->data.field.options.bSecPathDrop) {
				  	dbgprintf("[slashes are dropped] ");
				}
				if(pTpe->data.field.options.bSecPathReplace) {
				  	dbgprintf("[slashes are replaced by '_'] ");
				}
				if(pTpe->data.field.options.bSPIffNo1stSP) {
				  	dbgprintf("[SP iff no first SP] ");
				}
				if(pTpe->data.field.options.bCSV) {
				  	dbgprintf("[format as CSV (RFC4180)]");
				}
				if(pTpe->data.field.options.bJSON) {
					dbgprintf("[format as JSON] ");
				}
				if(pTpe->data.field.options.bJSONf) {
					dbgprintf("[format as JSON field] ");
				}
				if(pTpe->data.field.options.bJSONr) {
					dbgprintf("[format as JSON without re-escaping] ");
				}
				if(pTpe->data.field.options.bJSONfr) {
					dbgprintf("[format as JSON field without re-escaping] ");
				}
				if(pTpe->data.field.options.bMandatory) {
					dbgprintf("[mandatory field] ");
				}
				if(pTpe->data.field.options.bDropLastLF) {
				  	dbgprintf("[drop last LF in msg] ");
				}
				if(pTpe->data.field.has_fields == 1) {
				  	dbgprintf("[substring, field #%d only (delemiter %d)] ",
						pTpe->data.field.iFieldNr, pTpe->data.field.field_delim);
				}
				if(pTpe->data.field.iFromPos != 0 || pTpe->data.field.iToPos != 0) {
				  	dbgprintf("[substring, from character %d to %d] ",
						pTpe->data.field.iFromPos,
						pTpe->data.field.iToPos);
				}
				break;
			}
			if(pTpe->bComplexProcessing)
				dbgprintf("[COMPLEX]");
			dbgprintf("\n");
			pTpe = pTpe->pNext;
		}
		pTpl = pTpl->pNext; /* done, go next */
	}
}

int tplGetEntryCount(struct template *pTpl)
{
	assert(pTpl != NULL);
	return(pTpl->tpenElements);
}

rsRetVal templateInit(void)
{
	DEFiRet;
	CHKiRet(objGetObjInterface(&obj));
	CHKiRet(objUse(strgen, CORE_COMPONENT));

finalize_it:
	RETiRet;
}
