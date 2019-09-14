/* rsyslog rainerscript definitions
 *
 * Copyright 2011-2018 Rainer Gerhards
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
#ifndef INC_UTILS_H
#define INC_UTILS_H
#include <stdio.h>
#include <libestr.h>
#include <typedefs.h>
#include <sys/types.h>
#include <json.h>
#include <regex.h>
#include "typedefs.h"

#define LOG_NFACILITIES 24+1 /* we copy&paste this as including rsyslog.h gets us in off64_t trouble... :-( */
#define CNFFUNC_MAX_ARGS 32
	/**< maximum number of arguments that any function can have (among
	 *   others, this is used to size data structures).
	 */

enum cnfobjType {
	CNFOBJ_ACTION,
	CNFOBJ_RULESET,
	CNFOBJ_GLOBAL,
	CNFOBJ_INPUT,
	CNFOBJ_MODULE,
	CNFOBJ_TPL,
	CNFOBJ_PROPERTY,
	CNFOBJ_CONSTANT,
	CNFOBJ_MAINQ,
	CNFOBJ_LOOKUP_TABLE,
	CNFOBJ_PARSER,
	CNFOBJ_TIMEZONE,
	CNFOBJ_DYN_STATS,
	CNFOBJ_INVALID = 0
};

const char* cnfobjType2str(enum cnfobjType ot);

/* a variant type, for example used for expression evaluation
 * 2011-07-15/rger: note that there exists a "legacy" object var,
 * which implements the same idea, but in a suboptimal manner. I have
 * stipped this down as much as possible, but will keep it for a while
 * to avoid unnecessary complexity during development. TODO: in the long
 * term, var shall be replaced by struct svar.
 */
struct svar{
	union {
		es_str_t *estr;
		struct cnfarray *ar;
		long long n;
		struct json_object *json;
	} d;
	char datatype; /* 'N' number, 'S' string, 'J' JSON, 'A' array
			* Note: 'A' is only supported during config phase
			*/
};

struct cnfobj {
	enum cnfobjType objType;
	struct nvlst *nvlst;
	struct objlst *subobjs;
	struct cnfstmt *script;
};

struct objlst {
	struct objlst *next;
	struct cnfobj *obj;
};

struct nvlst {
	struct nvlst *next;
	es_str_t *name;
	struct svar val;
	unsigned char bUsed;
	/**< was this node used during config processing? If not, this
	 *   indicates an error. After all, the user specified a setting
	 *   that the software does not know.
	 */
};

/* the following structures support expressions, and may (very much later
 * be the sole foundation for the AST.
 *
 * nodetypes (list not yet complete)
 * F - function
 * N - number
 * P - fparamlst
 * R - rule
 * S - string
 * V - var
 * A - (string) array
 * ... plus the S_* #define's below:
 */
#define S_STOP 4000
#define S_PRIFILT 4001
#define S_PROPFILT 4002
#define S_IF 4003
#define S_ACT 4004
#define S_NOP 4005	/* usually used to disable some statement */
#define S_SET 4006
#define S_UNSET 4007
#define S_CALL 4008
#define S_FOREACH 4009
#define S_RELOAD_LOOKUP_TABLE 4010
#define S_CALL_INDIRECT 4011

enum cnfFiltType { CNFFILT_NONE, CNFFILT_PRI, CNFFILT_PROP, CNFFILT_SCRIPT };
const char* cnfFiltType2str(const enum cnfFiltType filttype);


struct cnfstmt {
	unsigned nodetype;
	struct cnfstmt *next;
	uchar *printable; /* printable text for debugging */
	union {
		struct {
			struct cnfexpr *expr;
			struct cnfstmt *t_then;
			struct cnfstmt *t_else;
		} s_if;
		struct {
			uchar *varname;
			struct cnfexpr *expr;
			int force_reset;
		} s_set;
		struct {
			uchar *varname;
		} s_unset;
		struct {
			es_str_t *name;
			struct cnfstmt *stmt;
			ruleset_t *ruleset;	/* non-NULL if the ruleset has a queue assigned */
		} s_call;
		struct {
			struct cnfexpr *expr;
		} s_call_ind;
		struct {
			uchar pmask[LOG_NFACILITIES+1];	/* priority mask */
			struct cnfstmt *t_then;
			struct cnfstmt *t_else;
		} s_prifilt;
		struct {
			fiop_t operation;
			regex_t *regex_cache;/* cache for compiled REs, if used */
			struct cstr_s *pCSCompValue;/* value to "compare" against */
			sbool isNegated;
			msgPropDescr_t prop; /* requested property */
			struct cnfstmt *t_then;
			struct cnfstmt *t_else;
		} s_propfilt;
		struct action_s *act;
	struct {
			struct cnfitr *iter;
			struct cnfstmt *body;
		} s_foreach;
	struct {
			lookup_ref_t *table;
			uchar *table_name;
			uchar *stub_value;
		} s_reload_lookup_table;
	} d;
};

struct cnfexpr {
	unsigned nodetype;
	struct cnfexpr *l;
	struct cnfexpr *r;
} __attribute__((aligned (8)));

struct cnfitr {
	char* var;
	struct cnfexpr* collection;
} __attribute__((aligned (8)));

struct cnfnumval {
	unsigned nodetype;
	long long val;
} __attribute__((aligned (8)));

struct cnfstringval {
	unsigned nodetype;
	es_str_t *estr;
} __attribute__((aligned (8)));

struct cnfvar {
	unsigned nodetype;
	char *name;
	msgPropDescr_t prop;
} __attribute__((aligned (8)));

struct cnfarray {
	unsigned nodetype;
	int nmemb;
	es_str_t **arr;
} __attribute__((aligned (8)));

struct cnffparamlst {
	unsigned nodetype; /* P */
	struct cnffparamlst *next;
	struct cnfexpr *expr;
} __attribute__((aligned (8)));

enum cnffuncid {
	CNFFUNC_INVALID = 0, /**< defunct entry, do not use (should normally not be present) */
	CNFFUNC_NAME = 1,   /**< use name to call function (for future use) */
	CNFFUNC_STRLEN,
	CNFFUNC_SUBSTRING,
	CNFFUNC_GETENV,
	CNFFUNC_TOLOWER,
	CNFFUNC_CSTR,
	CNFFUNC_CNUM,
	CNFFUNC_RE_MATCH,
	CNFFUNC_RE_EXTRACT,
	CNFFUNC_FIELD,
	CNFFUNC_PRIFILT,
	CNFFUNC_LOOKUP,
	CNFFUNC_EXEC_TEMPLATE,
	CNFFUNC_REPLACE,
	CNFFUNC_WRAP,
	CNFFUNC_RANDOM,
	CNFFUNC_DYN_INC,
	CNFFUNC_IPV42NUM,
	CNFFUNC_NUM2IPV4,
	CNFFUNC_INT2HEX,
	CNFFUNC_LTRIM,
	CNFFUNC_RTRIM,
	CNFFUNC_FORMAT_TIME,
	CNFFUNC_PARSE_TIME,
	CNFFUNC_PARSE_JSON,
	CNFFUNC_PREVIOUS_ACTION_SUSPENDED,
	CNFFUNC_SCRIPT_ERROR,
	CNFFUNC_HTTP_REQUEST,
	CNFFUNC_IS_TIME
};

typedef struct cnffunc cnffunc_t;
typedef void (*rscriptFuncPtr) (cnffunc_t *, struct svar *, void *, wti_t *);

struct cnffunc {
	unsigned nodetype;
	es_str_t *fname;
	unsigned short nParams;
	rscriptFuncPtr fPtr;
	void *funcdata;	/* global data for function-specific use (e.g. compiled regex) */
	uint8_t destructable_funcdata;
	struct cnfexpr *expr[];
} __attribute__((aligned (8)));


struct scriptFunct {
	const char *fname;
	unsigned short minParams;
	unsigned short maxParams;
	rscriptFuncPtr fPtr;
	rsRetVal (*initFunc) (struct cnffunc *);
	void (*destruct) (struct cnffunc *);
	/* currently no optimizer entrypoint, may be added later.
	 * Since the optimizer needs metadata about functions, it does
	 * not seem practical to add such a function at the current state
	 */
};


/* future extensions
struct x {
	int nodetype;
};
*/


/* the following defines describe the parameter block for puling
 * config parameters. Note that the focus is on ease and saveness of
 * use, not performance. For example, we address parameters by name
 * instead of index, because the former is less error-prone. The (severe)
 * performance hit does not matter, as it is a one-time hit during config
 * load but never during actual processing. So there is really no reason
 * to care.
 */
struct cnfparamdescr { /* first the param description */
	const char *name;/**< not a es_str_t to ease definition in code */
	ecslCmdHdrlType type;
	unsigned flags;
};
/* flags for cnfparamdescr: */
#define CNFPARAM_REQUIRED	0x0001
#define CNFPARAM_DEPRECATED	0x0002

struct cnfparamblk { /* now the actual param block use in API calls */
	unsigned short version;
	unsigned short nParams;
	struct cnfparamdescr *descr;
};
#define CNFPARAMBLK_VERSION 1
	/**< caller must have same version as engine -- else things may
	 * be messed up. But note that we may support multiple versions
	 * inside the engine, if at some later stage we want to do
	 * that. -- rgerhards, 2011-07-15
	 */
struct cnfparamvals { /* the values we obtained for param descr. */
	struct svar val;
	unsigned char bUsed;
};

struct funcData_prifilt {
	uchar pmask[LOG_NFACILITIES+1];	/* priority mask */
};

/* script errno-like interface error codes: */
#define RS_SCRIPT_EOK		0
#define RS_SCRIPT_EINVAL	1

void varFreeMembers(const struct svar *r);
rsRetVal addMod2List(const int version, struct scriptFunct *functArray);
int cnfParseBuffer(char *buf, unsigned lenBuf);
void readConfFile(FILE *fp, es_str_t **str);
struct objlst* objlstNew(struct cnfobj *obj);
void objlstDestruct(struct objlst *lst);
void objlstPrint(struct objlst *lst);
struct nvlst* nvlstNewArray(struct cnfarray *ar);
struct nvlst* nvlstNewStr(es_str_t *value);
struct nvlst* nvlstNewStrBackticks(es_str_t *const value);
struct nvlst* nvlstSetName(struct nvlst *lst, es_str_t *name);
void nvlstDestruct(struct nvlst *lst);
void nvlstPrint(struct nvlst *lst);
void nvlstChkUnused(struct nvlst *lst);
struct nvlst* nvlstFindName(struct nvlst *lst, es_str_t *name);
struct cnfobj* cnfobjNew(enum cnfobjType objType, struct nvlst *lst);
void cnfobjDestruct(struct cnfobj *o);
void cnfobjPrint(struct cnfobj *o);
struct cnfexpr* cnfexprNew(unsigned nodetype, struct cnfexpr *l, struct cnfexpr *r);
void cnfexprPrint(struct cnfexpr *expr, int indent);
void cnfexprEval(const struct cnfexpr *const expr, struct svar *ret, void *pusr, wti_t *pWti);
int cnfexprEvalBool(struct cnfexpr *expr, void *usrptr, wti_t *pWti);
struct json_object* cnfexprEvalCollection(struct cnfexpr * const expr, void * const usrptr, wti_t *pWti);
void cnfexprDestruct(struct cnfexpr *expr);
struct cnfnumval* cnfnumvalNew(long long val);
struct cnfstringval* cnfstringvalNew(es_str_t *estr);
struct cnfvar* cnfvarNew(char *name);
struct cnffunc * cnffuncNew(es_str_t *fname, struct cnffparamlst* paramlst);
struct cnffparamlst * cnffparamlstNew(struct cnfexpr *expr, struct cnffparamlst *next);
int cnfDoInclude(const char *name, const int optional);
int cnfparamGetIdx(struct cnfparamblk *params, const char *name);
struct cnfparamvals* nvlstGetParams(struct nvlst *lst, struct cnfparamblk *params,
	       struct cnfparamvals *vals);
void cnfparamsPrint(const struct cnfparamblk *params, const struct cnfparamvals *vals);
int cnfparamvalsIsSet(struct cnfparamblk *params, struct cnfparamvals *vals);
void varDelete(const struct svar *v);
void cnfparamvalsDestruct(const struct cnfparamvals *paramvals, const struct cnfparamblk *blk);
struct cnfstmt * cnfstmtNew(unsigned s_type);
struct cnfitr * cnfNewIterator(char *var, struct cnfexpr *collection);
void cnfstmtPrintOnly(struct cnfstmt *stmt, int indent, sbool subtree);
void cnfstmtPrint(struct cnfstmt *stmt, int indent);
struct cnfstmt* scriptAddStmt(struct cnfstmt *root, struct cnfstmt *s);
struct objlst* objlstAdd(struct objlst *root, struct cnfobj *o);
char *rmLeadingSpace(char *s);
struct cnfstmt * cnfstmtNewPRIFILT(char *prifilt, struct cnfstmt *t_then);
struct cnfstmt * cnfstmtNewPROPFILT(char *propfilt, struct cnfstmt *t_then);
struct cnfstmt * cnfstmtNewAct(struct nvlst *lst);
struct cnfstmt * cnfstmtNewLegaAct(char *actline);
struct cnfstmt * cnfstmtNewSet(char *var, struct cnfexpr *expr, int force_reset);
struct cnfstmt * cnfstmtNewUnset(char *var);
struct cnfstmt * cnfstmtNewCall(es_str_t *name);
struct cnfstmt * cnfstmtNewContinue(void);
struct cnfstmt * cnfstmtNewReloadLookupTable(struct cnffparamlst *fparams);
void cnfstmtDestructLst(struct cnfstmt *root);
struct cnfstmt *cnfstmtOptimize(struct cnfstmt *root);
struct cnfarray* cnfarrayNew(es_str_t *val);
struct cnfarray* cnfarrayDup(struct cnfarray *old);
struct cnfarray* cnfarrayAdd(struct cnfarray *ar, es_str_t *val);
void cnfarrayContentDestruct(struct cnfarray *ar);
const char* getFIOPName(unsigned iFIOP);
rsRetVal initRainerscript(void);
void unescapeStr(uchar *s, int len);
const char * tokenval2str(int tok);
uchar* var2CString(struct svar *__restrict__ const r, int *__restrict__ const bMustFree);
long long var2Number(struct svar *r, int *bSuccess);
void includeProcessCnf(struct nvlst *const lst);

/* debug helper */
void cstrPrint(const char *text, es_str_t *estr);

#endif
