/* This is the header for template processing code of rsyslog.
 * begun 2004-11-17 rgerhards
 *
 * Copyright (C) 2004-2013 by Rainer Gerhards and Adiscon GmbH
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

#ifndef	TEMPLATE_H_INCLUDED
#define	TEMPLATE_H_INCLUDED 1

#include <json.h>
#include <libestr.h>
#include "regexp.h"
#include "stringbuf.h"

struct template {
	struct template *pNext;
	char *pszName;
	int iLenName;
	rsRetVal (*pStrgen)(const smsg_t*const, actWrkrIParams_t *const iparam);
	sbool bHaveSubtree;
	msgPropDescr_t subtree;	/* subtree property name for subtree-type templates */
	int tpenElements; /* number of elements in templateEntry list */
	struct templateEntry *pEntryRoot;
	struct templateEntry *pEntryLast;
	char optFormatEscape;	/* in text fields, */
#	define NO_ESCAPE 0	/* 0 - do not escape, */
#	define SQL_ESCAPE 1	/* 1 - escape "the MySQL way"  */
#	define STDSQL_ESCAPE 2  /* 2 - escape quotes by double quotes, */
#	define JSON_ESCAPE 3	/* 3 - escape double quotes for JSON.  */
#	define JSONF 4		/* 4 - not a real escape - template contains json fields only */
	/* following are options. All are 0/1 defined (either on or off).
	 * we use chars because they are faster than bit fields and smaller
	 * than short...
	 */
	char optCaseSensitive;  /* case-sensitive variable property references, default False, 0 */
};

enum EntryTypes { UNDEFINED = 0, CONSTANT = 1, FIELD = 2 };
enum tplFormatTypes { tplFmtDefault = 0, tplFmtMySQLDate = 1,
			tplFmtRFC3164Date = 2, tplFmtRFC3339Date = 3, tplFmtPgSQLDate = 4,
			tplFmtSecFrac = 5, tplFmtRFC3164BuggyDate = 6, tplFmtUnixDate = 7,
			tplFmtWDayName = 8, tplFmtYear = 9, tplFmtMonth = 10, tplFmtDay = 11,
			tplFmtHour = 12, tplFmtMinute = 13, tplFmtSecond = 14,
			tplFmtTZOffsHour = 15, tplFmtTZOffsMin = 16, tplFmtTZOffsDirection = 17,
			tplFmtWDay = 18, tplFmtOrdinal = 19, tplFmtWeek = 20};
enum tplFormatCaseConvTypes { tplCaseConvNo = 0, tplCaseConvUpper = 1, tplCaseConvLower = 2 };
enum tplRegexType { TPL_REGEX_BRE = 0, /* posix BRE */
		    TPL_REGEX_ERE = 1  /* posix ERE */
		  };

#include "msg.h"

/* a specific parse entry */
struct templateEntry {
	struct templateEntry *pNext;
	enum EntryTypes eEntryType;
	uchar *fieldName;	/**< field name to be used for structured output */
	int lenFieldName;
	sbool bComplexProcessing; /**< set if complex processing (options, etc) is required */
	union {
		struct {
			uchar *pConstant;	/* pointer to constant value */
			int iLenConstant;	/* its length */
		} constant;
		struct {
			msgPropDescr_t msgProp;	/* property to be used */
			unsigned iFromPos;	/* for partial strings only chars from this position ... */
			unsigned iToPos;	/* up to that one... */
			unsigned iFieldNr;	/* for field extraction: field to extract */
#ifdef FEATURE_REGEXP
			regex_t re;	/* APR: this is the regular expression */
			short has_regex;
			short iMatchToUse;/* which match should be obtained (10 max) */
			short iSubMatchToUse;/* which submatch should be obtained (10 max) */
			enum tplRegexType typeRegex;
			enum tlpRegexNoMatchType {
				TPL_REGEX_NOMATCH_USE_DFLTSTR = 0,
				/* use the (old style) default "**NO MATCH**" string */
				TPL_REGEX_NOMATCH_USE_BLANK = 1, /* use a blank string */
				TPL_REGEX_NOMATCH_USE_WHOLE_FIELD = 2, /* use the full field contents
									that we were searching in*/
				TPL_REGEX_NOMATCH_USE_ZERO = 3 /* use  0 (useful for numerical values) */
			}  nomatchAction;	/**< what to do if we do not have a match? */

#endif
			unsigned has_fields; /* support for field-counting: field to extract */
			unsigned char field_delim; /* support for field-counting: field delemiter char */
#ifdef STRICT_GPLV3
			int field_expand;	/* use multiple instances of the field delimiter as a single one? */
#endif


			enum tplFormatTypes eDateFormat;
			enum tplFormatCaseConvTypes eCaseConv;
			struct { 		/* bit fields! */
				unsigned bDropCC: 1;		/* drop control characters? */
				unsigned bSpaceCC: 1;		/* change control characters to spaceescape? */
				unsigned bEscapeCC: 1;		/* escape control characters? */
				unsigned bCompressSP: 1;	/* compress multiple spaces to a single one? */
				unsigned bDropLastLF: 1;	/* drop last LF char in msg (PIX!) */
				unsigned bSecPathDrop: 1;	/* drop slashes, replace dots, empty string */
				unsigned bSecPathReplace: 1;	/* replace slashes, replace dots, empty string */
				unsigned bSPIffNo1stSP: 1;	/* be a space if 1st pos if field is no space*/
				unsigned bCSV: 1;		/* format field in CSV (RFC 4180) format */
				unsigned bJSON: 1;		/* format field JSON escaped */
				unsigned bJSONf: 1;		/* format field JSON *field* (n/v pair) */
				unsigned bJSONr: 1;		/* format field JSON non escaped */
				unsigned bJSONfr: 1;		/* format field JSON *field* non escaped (n/v pair) */
				unsigned bMandatory: 1;		/* mandatory field - emit even if empty */
				unsigned bFromPosEndRelative: 1;/* is From/To-Pos relative to end of string? */
				unsigned bFixedWidth: 1;	/* space pad to toChar if string is shorter */
				unsigned bDateInUTC: 1;		/* should date be expressed in UTC? */
			} options;		/* options as bit fields */
		} field;
	} data;
};


/* interfaces */
BEGINinterface(tpl) /* name must also be changed in ENDinterface macro! */
ENDinterface(tpl)
#define tplCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */

/* prototypes */
PROTOTYPEObj(tpl);


//struct template* tplConstruct(void);
struct template *tplAddLine(rsconf_t *conf, const char* pName, unsigned char** pRestOfConfLine);
struct template *tplFind(rsconf_t *conf, char *pName, int iLenName);
int tplGetEntryCount(struct template *pTpl);
void tplDeleteAll(rsconf_t *conf);
void tplDeleteNew(rsconf_t *conf);
void tplPrintList(rsconf_t *conf);
void tplLastStaticInit(rsconf_t *conf, struct template *tpl);
rsRetVal ExtendBuf(actWrkrIParams_t *const iparam, const size_t iMinSize);
int tplRequiresDateCall(struct template *pTpl);
/* note: if a compiler warning for undefined type tells you to look at this
 * code line below, the actual cause is that you currently MUST include template.h
 * BEFORE msg.h, even if your code file does not actually need it.
 * rgerhards, 2007-08-06
 */
rsRetVal tplToArray(struct template *pTpl, smsg_t *pMsg, uchar*** ppArr, struct syslogTime *ttNow);
rsRetVal tplToJSON(struct template *pTpl, smsg_t *pMsg, struct json_object **, struct syslogTime *ttNow);
rsRetVal doEscape(uchar **pp, rs_size_t *pLen, unsigned short *pbMustBeFreed, int escapeMode);
rsRetVal
tplToString(struct template *__restrict__ const pTpl,
	    smsg_t *__restrict__ const pMsg,
	    actWrkrIParams_t *__restrict__ const iparam,
	    struct syslogTime *const ttNow);

rsRetVal templateInit(void);
rsRetVal tplProcessCnf(struct cnfobj *o);

#endif /* #ifndef TEMPLATE_H_INCLUDED */
/* vim:set ai:
 */
