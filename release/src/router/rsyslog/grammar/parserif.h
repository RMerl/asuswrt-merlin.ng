/* rsyslog parser interface.
 *
 * Copyright 2011-2016 Rainer Gerhards
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
#ifndef PARSERIF_H_DEFINED
#define PARSERIF_H_DEFINED
#include "rainerscript.h"
int cnfSetLexFile(char*);
void parser_errmsg(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void parser_warnmsg(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void tellLexEndParsing(void);
#ifndef IN_GRAMMAR_Y
int yyparse(void);
extern int yydebug;
extern int yylineno;
extern char *cnfcurrfn;
#endif

/* entry points to be called after the parser has processed the
 * element in question. Actual processing must than be done inside
 * these functions.
 */
void cnfDoObj(struct cnfobj *o);
void cnfDoScript(struct cnfstmt *script);
void cnfDoCfsysline(char *ln);
void cnfDoBSDTag(char *ln);
void cnfDoBSDHost(char *ln);
int cnfAddConfigBuffer(es_str_t *const str, const char *const cnfobj_name);
#endif
