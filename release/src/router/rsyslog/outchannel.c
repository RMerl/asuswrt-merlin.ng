/* This is the output channel processing code of rsyslog.
 * Output channels - in the long term - will define how
 * messages will be sent to whatever file or other medium.
 * Currently, they mainly provide a way to store some file-related
 * information (most importantly the maximum file size allowed).
 * Please see syslogd.c for license information.
 * begun 2005-06-21 rgerhards
 *
 * Copyright (C) 2005-2016 Adiscon GmbH
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
#include "config.h"

#include "rsyslog.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "stringbuf.h"
#include "outchannel.h"
#include "rsconf.h"
#include "debug.h"

/* Constructs a outchannel list object. Returns pointer to it
 * or NULL (if it fails).
 */
struct outchannel* ochConstruct(void)
{
	struct outchannel *pOch;
	if((pOch = calloc(1, sizeof(struct outchannel))) == NULL)
		return NULL;
	
	/* basic initialisaion is done via calloc() - need to
	 * initialize only values != 0. */

	if(loadConf->och.ochLast == NULL)
	{ /* we are the first element! */
		loadConf->och.ochRoot = loadConf->och.ochLast = pOch;
	}
	else
	{
		loadConf->och.ochLast->pNext = pOch;
		loadConf->och.ochLast = pOch;
	}

	return(pOch);
}


/* skips the next comma and any whitespace
 * in front and after it.
 */
static void skip_Comma(char **pp)
{
	register char *p;

	assert(pp != NULL);
	assert(*pp != NULL);

	p = *pp;
	while(isspace((int)*p))
		++p;
	if(*p == ',')
		++p;
	while(isspace((int)*p))
		++p;
	*pp = p;
}

/* helper to ochAddLine. Parses a comma-delimited field
 * The field is delimited by SP or comma. Leading whitespace
 * is "eaten" and does not become part of the field content.
 */
static rsRetVal get_Field(uchar **pp, uchar **pField)
{
	DEFiRet;
	register uchar *p;
	cstr_t *pStrB = NULL;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pField != NULL);

	skip_Comma((char**)pp);
	p = *pp;

	CHKiRet(cstrConstruct(&pStrB));

	/* copy the field */
	while(*p && *p != ' ' && *p != ',') {
		CHKiRet(cstrAppendChar(pStrB, *p++));
	}

	*pp = p;
	cstrFinalize(pStrB);
	CHKiRet(cstrConvSzStrAndDestruct(&pStrB, pField, 0));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pStrB != NULL)
			cstrDestruct(&pStrB);
	}

	RETiRet;
}


/* helper to ochAddLine. Parses a off_t type from the
 * input line.
 * returns: 0 - ok, 1 - failure
 */
static int get_off_t(uchar **pp, off_t *pOff_t)
{
	register uchar *p;
	off_t val;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pOff_t != NULL);

	skip_Comma((char**)pp);
	p = *pp;

	val = 0;
	while(*p && isdigit((int)*p)) {
		val = val * 10 + (*p - '0');
		++p;
	}

	*pp = p;
	*pOff_t = val;

	return 0;
}


/* helper to ochAddLine. Parses everything from the
 * current position to the end of line and returns it
 * to the caller. Leading white space is removed, but
 * not trailing.
 */
static rsRetVal get_restOfLine(uchar **pp, uchar **pBuf)
{
	DEFiRet;
	register uchar *p;
	cstr_t *pStrB = NULL;

	assert(pp != NULL);
	assert(*pp != NULL);
	assert(pBuf != NULL);

	skip_Comma((char**)pp);
	p = *pp;

	CHKiRet(cstrConstruct(&pStrB));

	/* copy the field */
	while(*p) {
		CHKiRet(cstrAppendChar(pStrB, *p++));
	}

	*pp = p;
	cstrFinalize(pStrB);
	CHKiRet(cstrConvSzStrAndDestruct(&pStrB, pBuf, 0));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pStrB != NULL)
			cstrDestruct(&pStrB);
	}

	RETiRet;
}


/* Add a new outchannel line
 * returns pointer to new object if it succeeds, NULL otherwise.
 * An outchannel line is primarily a set of fields delemited by commas.
 * There might be some whitespace between the field (but not within)
 * and the commas. This can be removed.
 */
struct outchannel *ochAddLine(char* pName, uchar** ppRestOfConfLine)
{
	struct outchannel *pOch;
	uchar *p;

	assert(pName != NULL);
	assert(ppRestOfConfLine != NULL);

	if((pOch = ochConstruct()) == NULL)
		return NULL;
	
	pOch->iLenName = strlen(pName);
	pOch->pszName = (char*) MALLOC(pOch->iLenName + 1);
	if(pOch->pszName == NULL) {
		dbgprintf("ochAddLine could not alloc memory for outchannel name!");
		pOch->iLenName = 0;
		return NULL;
		/* I know - we create a memory leak here - but I deem
		 * it acceptable as it is a) a very small leak b) very
		 * unlikely to happen. rgerhards 2004-11-17
		 */
	}
	memcpy(pOch->pszName, pName, pOch->iLenName + 1);

	/* now actually parse the line */
	p = *ppRestOfConfLine;
	assert(p != NULL);

	/* get params */
	get_Field(&p, &pOch->pszFileTemplate);
	if(*p) get_off_t(&p, &pOch->uSizeLimit);
	if(*p) get_restOfLine(&p, &pOch->cmdOnSizeLimit);

	*ppRestOfConfLine = p;
	return(pOch);
}


/* Find a outchannel object based on name. Search
 * currently is case-sensitive (should we change?).
 * returns pointer to outchannel object if found and
 * NULL otherwise.
 * rgerhards 2004-11-17
 */
struct outchannel *ochFind(char *pName, int iLenName)
{
	struct outchannel *pOch;

	assert(pName != NULL);

	pOch = loadConf->och.ochRoot;
	while(pOch != NULL &&
	      !(pOch->iLenName == iLenName &&
	        !strcmp(pOch->pszName, pName)
	        ))
		{
			pOch = pOch->pNext;
		}
	return(pOch);
}

/* Destroy the outchannel structure. This is for de-initialization
 * at program end. Everything is deleted.
 * rgerhards 2005-02-22
 */
void ochDeleteAll(void)
{
	struct outchannel *pOch, *pOchDel;

	pOch = loadConf->och.ochRoot;
	while(pOch != NULL) {
		dbgprintf("Delete Outchannel: Name='%s'\n ", pOch->pszName == NULL? "NULL" : pOch->pszName);
		pOchDel = pOch;
		pOch = pOch->pNext;
		if(pOchDel->pszName != NULL)
			free(pOchDel->pszName);
		free(pOchDel);
	}
}


/* Print the outchannel structure. This is more or less a
 * debug or test aid, but anyhow I think it's worth it...
 */
void ochPrintList(void)
{
	struct outchannel *pOch;

	pOch = loadConf->och.ochRoot;
	while(pOch != NULL) {
		dbgprintf("Outchannel: Name='%s'\n", pOch->pszName == NULL? "NULL" : pOch->pszName);
		dbgprintf("\tFile Template: '%s'\n", pOch->pszFileTemplate == NULL ? "NULL" :
			(char*) pOch->pszFileTemplate);
		dbgprintf("\tMax Size.....: %lu\n", (long unsigned) pOch->uSizeLimit);
		dbgprintf("\tOnSizeLimtCmd: '%s'\n", pOch->cmdOnSizeLimit == NULL ? "NULL" :
			(char*) pOch->cmdOnSizeLimit);
		pOch = pOch->pNext; /* done, go next */
	}
}
/* vi:set ai:
 */
