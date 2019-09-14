/* parsing routines for the counted string class. These
 * routines provide generic parsing aid as well some fairly
 * complex routines targeted toward specific needs.
 *
 * General information - read this:
 * All routines work on a single CStr object, which must be supplied
 * during construction. The parse class keeps an internal pointer of
 * where the next parse operation is to start (you could also say
 * this is where the last parse operation stopped).
 *
 * Each parse operation carried out by this package starts from the
 * parse pointer, parses the caller-requested element (e.g. an
 * integer or delemited string) and the update the parse pointer. If
 * the caller tries to parse beyond the end of the original string,
 * an error is returned. In general, all functions return a parsRet
 * error code and all require the parseObj to be the first parameter.
 * The to-be-parsed string provided to the parse object MUST NOT be
 * freed or modified by the caller during the lifetime of the parse
 * object. However, the caller must free it when it is no longer needed.
 * Optinally, the parse object can be instructed to do that. All objects
 * returned by the parse routines must be freed by the caller. For
 * simpler data types (like integers), the caller must provide the
 * necessary buffer space.
 *
 * begun 2005-09-09 rgerhards
 *
 * Copyright (C) 2005-2012 Adiscon GmbH
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
#ifndef _PARSE_H_INCLUDED__
#define _PARSE_H_INCLUDED__ 1

#include "stringbuf.h"

/**
 * The parse object
 */
struct rsParsObject
{
#ifndef NDEBUG
	rsObjID OID;			/**< object ID */
#endif
	cstr_t *pCStr;		/**< pointer to the string object we are parsing */
	int iCurrPos;			/**< current parsing position (char offset) */
};
typedef struct rsParsObject rsParsObj;


/* BEGIN "inline"-like functions */
/* END "inline"-like functions */

int rsParsGetParsePointer(rsParsObj *pThis);

/**
 * Construct a rsPars object.
 */
rsRetVal rsParsConstruct(rsParsObj **ppThis);
rsRetVal rsParsAssignString(rsParsObj *pThis, cstr_t *pCStr);

/* parse an integer. The parse pointer is advanced */
rsRetVal parsInt(rsParsObj *pThis, int* pInt);

/* Skip whitespace. Often used to trim parsable entries. */
rsRetVal parsSkipWhitespace(rsParsObj *pThis);

/* Parse string up to a delimiter.
 *
 * Input:
 * cDelim - the delimiter
 *   The following two are for whitespace stripping,
 *   0 means "no", 1 "yes"
 *   - bTrimLeading
 *   - bTrimTrailing
 *
 * Output:
 * ppCStr Pointer to the parsed string
 */
rsRetVal parsDelimCStr(rsParsObj *pThis, cstr_t **ppCStr, char cDelim, int bTrimLeading,
int bTrimTrailing, int bConvLower);

rsRetVal parsSkipAfterChar(rsParsObj *pThis, char c);
rsRetVal parsQuotedCStr(rsParsObj *pThis, cstr_t **ppCStr);
rsRetVal rsParsConstructFromSz(rsParsObj **ppThis, unsigned char *psz);
rsRetVal rsParsDestruct(rsParsObj *pThis);
int parsIsAtEndOfParseString(rsParsObj *pThis);
int parsGetCurrentPosition(rsParsObj *pThis);
char parsPeekAtCharAtParsPtr(rsParsObj *pThis);
#ifdef SYSLOG_INET
rsRetVal parsAddrWithBits(rsParsObj *pThis, netAddr_t **pIP, int *pBits);
#endif

#endif
/* vim:set ai:
 */
