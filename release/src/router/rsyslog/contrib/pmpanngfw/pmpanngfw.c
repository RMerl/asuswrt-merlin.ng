/* pmpanngfw.c
 *
 * this detects logs sent by Palo Alto Networks NGFW and transforms CSV into tab-separated fields
 * for handling inside the mmnormalize
 *
 * Example: foo,"bar,""baz""",qux becomes foo<TAB>bar,"baz"<TAB>qux
 *
 * created 2010-12-13 by Luigi Mori (lmori@paloaltonetworks.com) based on pmsnare
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
#include <ctype.h>
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "msg.h"
#include "module-template.h"
#include "glbl.h"
#include "errmsg.h"
#include "parser.h"
#include "datetime.h"
#include "unicode-helper.h"
#include "typedefs.h"

MODULE_TYPE_PARSER
MODULE_TYPE_NOKEEP
PARSER_NAME("rsyslog.panngfw")

/* internal structures
 */
DEF_PMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)


/* static data */
static int bParseHOSTNAMEandTAG;    /* cache for the equally-named global param - performance enhancement */

typedef struct {
	uint64 value;
	uint64 mask;
} log_type_t;

const log_type_t log_types[] = {
	{ 0x002c544145524854ULL, 0x00FFFFFFFFFFFFFFULL }, /* THREAT, */
	{ 0x2c43494646415254ULL, 0xFFFFFFFFFFFFFFFFULL }, /* TRAFFIC, */
	{ 0x002c4d4554535953ULL, 0x00FFFFFFFFFFFFFFULL }, /* CONFIG */
	{ 0x002c4749464e4f43ULL, 0x00FFFFFFFFFFFFFFULL } /* SYSTEM */
};

#define NUM_LOG_TYPES (sizeof(log_types)/sizeof(log_type_t))


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATUREAutomaticSanitazion)
	    iRet = RS_RET_OK;
	if(eFeat == sFEATUREAutomaticPRIParsing)
	    iRet = RS_RET_OK;
ENDisCompatibleWithFeature



BEGINparse
	uchar *p2parse;
	uchar *p2target;
	uchar *msgend;
	int lenMsg, lenDelta;
	int state;
	int num_fields = 4;
	uchar *f3_commas[3];
	int cur_comma = 0;
	uint64 log_type;
	int j;
CODESTARTparse
	#define CSV_DELIMITER '\t'
	#define STATE_FIELD_START 0
	#define STATE_IN_FIELD 1
	#define STATE_IN_QUOTE 2
	#define STATE_IN_QUOTE_QUOTE 3

	state = STATE_FIELD_START;

	dbgprintf("Message will now be parsed by fix Palo Alto Networks NGFW parser.\n");
	assert(pMsg != NULL);
	assert(pMsg->pszRawMsg != NULL);

	lenMsg = pMsg->iLenRawMsg - pMsg->offAfterPRI;
	/* note: offAfterPRI is already the number of PRI chars (do not add one!) */
	p2parse = pMsg->pszRawMsg + pMsg->offAfterPRI; /* point to start of text, after PRI */
	msgend = p2parse+lenMsg;

	dbgprintf("pmpanngfw: msg to look at: [%d]'%s'\n", lenMsg, p2parse);

	/* pass the first 3 fields */
	while(p2parse < msgend) {
	    if (*p2parse == ',') {
	        f3_commas[cur_comma] = p2parse;
	        if (cur_comma == 2) {
	            break;
	        }
	        cur_comma++;
	    }
	    p2parse++;
	}

	/* check number of fields detected so far */
	if (cur_comma != 2) {
	    dbgprintf("not a PAN-OS syslog message: first 3 fields not found\n");
	    ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}

	/* check msg length */
	p2parse++;
	if ((p2parse > msgend) || ((msgend - p2parse) < sizeof(uint64))) {
	    dbgprintf("not a PAN-OS syslog message: too short\n");
	    ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}

	/* check log type */
	log_type = *((uint64 *)p2parse);
	for(j = 0; j < NUM_LOG_TYPES; j++) {
	    if ((log_type & log_types[j].mask) == log_types[j].value)
	        break;
	}
	if (j == NUM_LOG_TYPES) {
	    dbgprintf("not a PAN-OS syslog message, log type: %llx\n", log_type);
	    ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}

	/* set the delimiter */
	*f3_commas[0] = CSV_DELIMITER;
	*f3_commas[1] = CSV_DELIMITER;
	*f3_commas[2] = CSV_DELIMITER;

	p2target = p2parse;

	while(p2parse < msgend) {
	    /* dbgprintf("state: %d char: %c p2parse: %16x p2target: %16x\n", state, *p2parse, p2parse, p2target); */
	    switch(state) {
	        case STATE_FIELD_START:
	            switch(*p2parse) {
	                case '"':
	                    state = STATE_IN_QUOTE;
	                    p2parse++;
	                    break;

	                case ',':
	                    state = STATE_FIELD_START;
	                    *p2target = CSV_DELIMITER;
	                    num_fields++;
	                    p2parse++;
	                    p2target++;
	                    break;

	                default:
	                    state = STATE_IN_FIELD;
	                    *p2target = *p2parse;
	                    p2parse++;
	                    p2target++;
	            }
	            break;

	        case STATE_IN_FIELD:
	            switch(*p2parse) {
	                case ',':
	                    state = STATE_FIELD_START;
	                    *p2target = CSV_DELIMITER;
	                    num_fields++;
	                    p2parse++;
	                    p2target++;
	                    break;

	                default:
	                    *p2target = *p2parse;
	                    p2parse++;
	                    p2target++;
	            }
	            break;

	        case STATE_IN_QUOTE:
	            switch(*p2parse) {
	                case '"':
	                    state = STATE_IN_QUOTE_QUOTE;
	                    p2parse++;
	                    break;

	                default:
	                    *p2target = *p2parse;
	                    p2parse++;
	                    p2target++;
	            }
	            break;

	        case STATE_IN_QUOTE_QUOTE:
	            switch(*p2parse) {
	                case '"':
	                    state = STATE_IN_QUOTE;
	                    *p2target = *p2parse;
	                    p2parse++;
	                    p2target++;
	                    break;

	                case ',':
	                    state = STATE_FIELD_START;
	                    *p2target = CSV_DELIMITER;
	                    num_fields++;
	                    p2parse++;
	                    p2target++;
	                    break;

	                default:
	                    dbgprintf("pmpanngfw: martian char (%d) after quote in quote\n", *p2parse);
	                    ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	            }
	            break;

	        default:
	            dbgprintf("how could I have reached this state ?!?\n");
	            ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	    }
	}

	if(p2parse != p2target) {
	    lenDelta = p2parse - p2target;

	    assert(lenDelta >= 2);

	    *p2target = 0;

	    pMsg->iLenRawMsg -= lenDelta;
	    pMsg->iLenMSG -= lenDelta;
	}

	lenMsg = p2target - (pMsg->pszRawMsg + pMsg->offAfterPRI);

	DBGPRINTF("pmpanngfw: new message: [%d]'%s'\n", lenMsg, pMsg->pszRawMsg + pMsg->offAfterPRI);
	DBGPRINTF("pmpanngfw: # fields: %d\n", num_fields);

	ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);

finalize_it:
ENDparse


BEGINmodExit
CODESTARTmodExit
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(parser, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_PMOD_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(parser, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));

	DBGPRINTF("panngfw parser init called, compiled with version %s\n", VERSION);
	bParseHOSTNAMEandTAG = glbl.GetParseHOSTNAMEandTAG();
	/* cache value, is set only during rsyslogd option processing */


ENDmodInit

/* vim:set ai:
 */
