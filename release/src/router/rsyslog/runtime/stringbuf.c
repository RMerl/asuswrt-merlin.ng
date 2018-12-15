/* This is the byte-counted string class for rsyslog.
 * This object has a lot of legacy. Among others, it was started to
 * support embedded \0 bytes, which looked like they were needed to
 * be supported by RFC developments at that time. Later, this was
 * no longer a requirement, and we refactored the class in 2016
 * to some simpler internals which make use of the fact that no
 * NUL can ever occur in rsyslog strings (they are escaped at the
 * input side of rsyslog).
 * It now serves primarily to a) dynamic string creation, b) keep
 * old interfaces supported, and c) some special functionality,
 * e.g. search. Further refactoring and simplificytin may make
 * sense.
 *
 * Copyright (C) 2005-2018 Adiscon GmbH
 *
 * This file is part of the rsyslog runtime library.
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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <libestr.h>
#include "rsyslog.h"
#include "stringbuf.h"
#include "srUtils.h"
#include "regexp.h"
#include "errmsg.h"
#include "unicode-helper.h"


/* ################################################################# *
 * private members                                                   *
 * ################################################################# */

/* static data */
DEFobjCurrIf(obj)
DEFobjCurrIf(regexp)

/* ################################################################# *
 * public members                                                    *
 * ################################################################# */


rsRetVal
cstrConstruct(cstr_t **const ppThis)
{
	DEFiRet;
	cstr_t *pThis;

	CHKmalloc(pThis = (cstr_t*) malloc(sizeof(cstr_t)));
	rsSETOBJTYPE(pThis, OIDrsCStr);
	#ifndef NDEBUG
	pThis->isFinalized = 0;
	#endif
	pThis->pBuf = NULL;
	pThis->iBufSize = 0;
	pThis->iStrLen = 0;
	*ppThis = pThis;

finalize_it:
	RETiRet;
}


/* construct from sz string
 * rgerhards 2005-09-15
 */
rsRetVal
rsCStrConstructFromszStr(cstr_t **const ppThis, const uchar *const sz)
{
	DEFiRet;
	cstr_t *pThis;

	CHKiRet(rsCStrConstruct(&pThis));

	pThis->iStrLen = strlen((char *) sz);
	pThis->iBufSize = strlen((char *) sz) + 1;
	if((pThis->pBuf = (uchar*) MALLOC(pThis->iBufSize)) == NULL) {
		RSFREEOBJ(pThis);
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	/* we do NOT need to copy the \0! */
	memcpy(pThis->pBuf, sz, pThis->iStrLen);

	*ppThis = pThis;

finalize_it:
	RETiRet;
}


/* a helper function for rsCStr*Strf()
 */
static rsRetVal rsCStrConstructFromszStrv(cstr_t **ppThis, const char *fmt,
va_list ap) __attribute__((format(printf,2, 0)));
static rsRetVal
rsCStrConstructFromszStrv(cstr_t **const ppThis, const char *const fmt, va_list ap)
{
	DEFiRet;
	cstr_t *pThis;
	va_list ap2;
	int len;

	va_copy(ap2, ap);
	len = vsnprintf(NULL, 0, (char*)fmt, ap2);
	va_end(ap2);

	if(len < 0)
		ABORT_FINALIZE(RS_RET_ERR);

	CHKiRet(rsCStrConstruct(&pThis));

	pThis->iStrLen = len;
	pThis->iBufSize = len + 1;
	len++; /* account for the \0 written by vsnprintf */
	if((pThis->pBuf = (uchar*) MALLOC(pThis->iBufSize)) == NULL) {
		RSFREEOBJ(pThis);
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	vsnprintf((char*)pThis->pBuf, len, (char*)fmt, ap);
	*ppThis = pThis;
finalize_it:
	RETiRet;
}


/* construct from a printf-style formated string
 */
rsRetVal
rsCStrConstructFromszStrf(cstr_t **ppThis, const char *fmt, ...)
{
	DEFiRet;
	va_list ap;

	va_start(ap, fmt);
	iRet = rsCStrConstructFromszStrv(ppThis, fmt, ap);
	va_end(ap);

	RETiRet;
}


/* construct from es_str_t string
 * rgerhards 2010-12-03
 */
rsRetVal
cstrConstructFromESStr(cstr_t **const ppThis, es_str_t *const str)
{
	DEFiRet;
	cstr_t *pThis;

	CHKiRet(rsCStrConstruct(&pThis));

	pThis->iStrLen = es_strlen(str);
	pThis->iBufSize = pThis->iStrLen + 1;
	if((pThis->pBuf = (uchar*) MALLOC(pThis->iBufSize)) == NULL) {
		RSFREEOBJ(pThis);
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	/* we do NOT need to copy the \0! */
	memcpy(pThis->pBuf, es_getBufAddr(str), pThis->iStrLen);

	*ppThis = pThis;

finalize_it:
	RETiRet;
}

/* construct from CStr object.
 * rgerhards 2005-10-18
 */
rsRetVal ATTR_NONNULL()
rsCStrConstructFromCStr(cstr_t **const ppThis, const cstr_t *const pFrom)
{
	DEFiRet;
	cstr_t *pThis;

	rsCHECKVALIDOBJECT(pFrom, OIDrsCStr);

	CHKiRet(rsCStrConstruct(&pThis));
	if(pFrom->iStrLen > 0) {
		pThis->iStrLen = pFrom->iStrLen;
		pThis->iBufSize = pFrom->iStrLen + 1;
		if((pThis->pBuf = (uchar*) MALLOC(pThis->iBufSize)) == NULL) {
			RSFREEOBJ(pThis);
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
		}
		memcpy(pThis->pBuf, pFrom->pBuf, pThis->iStrLen);
	}

	*ppThis = pThis;
finalize_it:
	RETiRet;
}


void rsCStrDestruct(cstr_t **const ppThis)
{
	free((*ppThis)->pBuf);
	RSFREEOBJ(*ppThis);
	*ppThis = NULL;
}


/* extend the string buffer if its size is insufficient.
 * Param iMinNeeded is the minumum free space needed. If it is larger
 * than the default alloc increment, space for at least this amount is
 * allocated. In practice, a bit more is allocated because we envision that
 * some more characters may be added after these.
 * rgerhards, 2008-01-07
 * changed to utilized realloc() -- rgerhards, 2009-06-16
 */
static rsRetVal
rsCStrExtendBuf(cstr_t *const __restrict__ pThis, const size_t iMinNeeded)
{
	uchar *pNewBuf;
	size_t iNewSize;
	DEFiRet;

	/* first compute the new size needed */
	if(iMinNeeded > RS_STRINGBUF_ALLOC_INCREMENT) {
		/* we allocate "n" ALLOC_INCREMENTs. Usually, that should
		 * leave some room after the absolutely needed one. It also
		 * reduces memory fragmentation. Note that all of this are
		 * integer operations (very important to understand what is
		 * going on)! Parenthesis are for better readibility.
		 */
		iNewSize = (iMinNeeded / RS_STRINGBUF_ALLOC_INCREMENT + 1) * RS_STRINGBUF_ALLOC_INCREMENT;
	} else {
		iNewSize = pThis->iBufSize + RS_STRINGBUF_ALLOC_INCREMENT;
	}
	iNewSize += pThis->iBufSize; /* add current size */

	/* DEV debugging only: dbgprintf("extending string buffer, old %d, new %d\n", pThis->iBufSize, iNewSize); */
	CHKmalloc(pNewBuf = (uchar*) realloc(pThis->pBuf, iNewSize));
	pThis->iBufSize = iNewSize;
	pThis->pBuf = pNewBuf;

finalize_it:
	RETiRet;
}

/* Append a character to the current string object. This may only be done until
 * cstrFinalize() is called.
 * rgerhards, 2009-06-16
 */
rsRetVal cstrAppendChar(cstr_t *const __restrict__ pThis, const uchar c)
{
	rsRetVal iRet = RS_RET_OK;

	if(pThis->iStrLen+1 >= pThis->iBufSize) {
		CHKiRet(rsCStrExtendBuf(pThis, 1)); /* need more memory! */
	}

	/* ok, when we reach this, we have sufficient memory */
	*(pThis->pBuf + pThis->iStrLen++) = c;

finalize_it:
	return iRet;
}

/* append a string of known length. In this case, we make sure we do at most
 * one additional memory allocation.
 */
rsRetVal rsCStrAppendStrWithLen(cstr_t *const pThis, const uchar*const  psz, const size_t iStrLen)
{
	DEFiRet;

	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);
	assert(psz != NULL);

	/* does the string fit? */
	if(pThis->iStrLen + iStrLen >= pThis->iBufSize) {
		CHKiRet(rsCStrExtendBuf(pThis, iStrLen)); /* need more memory! */
	}

	/* ok, now we always have sufficient continues memory to do a memcpy() */
	memcpy(pThis->pBuf + pThis->iStrLen, psz, iStrLen);
	pThis->iStrLen += iStrLen;

finalize_it:
	RETiRet;
}


/* changed to be a wrapper to rsCStrAppendStrWithLen() so that
 * we can save some time when we have the length but do not
 * need to change existing code.
 * rgerhards, 2007-07-03
 */
rsRetVal rsCStrAppendStr(cstr_t *const pThis, const uchar*const  psz)
{
	return rsCStrAppendStrWithLen(pThis, psz, strlen((char*) psz));
}


/* append the contents of one cstr_t object to another
 * rgerhards, 2008-02-25
 */
rsRetVal cstrAppendCStr(cstr_t *pThis, cstr_t *pstrAppend)
{
	return rsCStrAppendStrWithLen(pThis, pstrAppend->pBuf, pstrAppend->iStrLen);
}


/* append a printf-style formated string
 */
rsRetVal rsCStrAppendStrf(cstr_t *pThis, const char *fmt, ...)
{
	DEFiRet;
	va_list ap;
	cstr_t *pStr = NULL;

	va_start(ap, fmt);
	iRet = rsCStrConstructFromszStrv(&pStr, (char*)fmt, ap);
	va_end(ap);

	if(iRet != RS_RET_OK)
		goto finalize_it;

	iRet = cstrAppendCStr(pThis, pStr);
	rsCStrDestruct(&pStr);
finalize_it:
	RETiRet;
}


rsRetVal rsCStrAppendInt(cstr_t *pThis, long i)
{
	DEFiRet;
	uchar szBuf[32];

	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);

	CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), i));

	iRet = rsCStrAppendStr(pThis, szBuf);
finalize_it:
	RETiRet;
}


/* Sets the string object to the classigal sz-string provided.
 * Any previously stored vlaue is discarded. If a NULL pointer
 * the the new value (pszNew) is provided, an empty string is
 * created (this is NOT an error!).
 * rgerhards, 2005-10-18
 */
rsRetVal rsCStrSetSzStr(cstr_t *const __restrict__ pThis,
	uchar *const __restrict__ pszNew)
{
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);

	if(pszNew == NULL) {
		free(pThis->pBuf);
		pThis->pBuf = NULL;
		pThis->iStrLen = 0;
		pThis->iBufSize = 0;
	} else {
		const size_t newlen = strlen((char*)pszNew);
		if(newlen > pThis->iBufSize) {
			uchar *const newbuf = (uchar*) realloc(pThis->pBuf, newlen + 1);
			if(newbuf == NULL) {
				/* we keep the old value, best we can do */
				return RS_RET_OUT_OF_MEMORY;
			}
			pThis->pBuf = newbuf;
			pThis->iBufSize = newlen + 1;
		}
		pThis->iStrLen = newlen;
		memcpy(pThis->pBuf, pszNew, pThis->iStrLen);
	}

	return RS_RET_OK;
}

/* Converts the CStr object to a classical zero-terminated C string
 * and returns that string. The caller must not free it and must not
 * destroy the CStr object as long as the ascii string is used.
 */
uchar*
cstrGetSzStrNoNULL(cstr_t *const __restrict__ pThis)
{
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);
	assert(pThis->isFinalized);
	return (pThis->pBuf == NULL) ? (uchar*) "" : pThis->pBuf;
}


/* Converts the CStr object to a classical zero-terminated C string,
 * returns that string and destroys the CStr object. The returned string
 * MUST be freed by the caller. The function might return NULL if
 * no memory can be allocated.
 *
 * This is the NEW replacement for rsCStrConvSzStrAndDestruct which does
 * no longer utilize a special buffer but soley works on pBuf (and also
 * assumes that cstrFinalize had been called).
 *
 * Parameters are as follows:
 * pointer to the object, pointer to string-pointer to receive string and
 * bRetNULL: 0 - must not return NULL on empty string, return "" in that
 * case, 1 - return NULL instead of an empty string.
 * PLEASE NOTE: the caller must free the memory returned in ppSz in any case
 * (except, of course, if it is NULL).
 */
rsRetVal cstrConvSzStrAndDestruct(cstr_t **ppThis, uchar **ppSz, int bRetNULL)
{
	DEFiRet;
	uchar* pRetBuf;
	cstr_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis->isFinalized);
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);
	assert(ppSz != NULL);
	assert(bRetNULL == 0 || bRetNULL == 1);

	if(pThis->pBuf == NULL) {
		if(bRetNULL == 0) {
			CHKmalloc(pRetBuf = MALLOC(1));
			*pRetBuf = '\0';
		} else {
			pRetBuf = NULL;
		}
	} else {
		pThis->pBuf[pThis->iStrLen] = '\0'; /* space for this is reserved */
		pRetBuf = pThis->pBuf;
	}

	*ppSz = pRetBuf;

finalize_it:
	/* We got it, now free the object ourselfs. Please note
	 * that we can NOT use the rsCStrDestruct function as it would
	 * also free the sz String buffer, which we pass on to the user.
	 */
	RSFREEOBJ(pThis);
	*ppThis = NULL;

	RETiRet;
}


/* return the length of the current string
 * 2005-09-09 rgerhards
 * Please note: this is only a function in a debug build.
 * For release builds, it is a macro defined in stringbuf.h.
 * This is due to performance reasons.
 */
#ifndef NDEBUG
int cstrLen(cstr_t *pThis)
{
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);
	return(pThis->iStrLen);
}
#endif

/* Truncate characters from the end of the string.
 * rgerhards 2005-09-15
 */
rsRetVal rsCStrTruncate(cstr_t *pThis, size_t nTrunc)
{
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);

	if(pThis->iStrLen < nTrunc)
		return RS_TRUNCAT_TOO_LARGE;
	
	pThis->iStrLen -= nTrunc;

	return RS_RET_OK;
}

/* Trim trailing whitespace from a given string
 */
void
cstrTrimTrailingWhiteSpace(cstr_t *const __restrict__ pThis)
{
	register int i;
	register uchar *pC;
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);

	if(pThis->iStrLen == 0)
		goto done; /* empty string -> nothing to trim ;) */
	i = pThis->iStrLen;
	pC = pThis->pBuf + i - 1;
	while(i > 0 && isspace((int)*pC)) {
		--pC;
		--i;
	}
	/* i now is the new string length! */
	if(i != (int) pThis->iStrLen) {
		pThis->iStrLen = i;
		pThis->pBuf[pThis->iStrLen] = '\0'; /* we always have this space */ //TODO: can we remove this?
	}

done:	return;
}

/* compare two string objects - works like strcmp(), but operates
 * on CStr objects. Please note that this version here is
 * faster in the majority of cases, simply because it can
 * rely on StrLen.
 * rgerhards 2005-09-19
 * fixed bug, in which only the last byte was actually compared
 * in equal-size strings.
 * rgerhards, 2005-09-26
 */
int
rsCStrCStrCmp(cstr_t *const __restrict__ pCS1, cstr_t *const __restrict__ pCS2)
{
	rsCHECKVALIDOBJECT(pCS1, OIDrsCStr);
	rsCHECKVALIDOBJECT(pCS2, OIDrsCStr);
	if(pCS1->iStrLen == pCS2->iStrLen)
		if(pCS1->iStrLen == 0)
			return 0; /* zero-sized string are equal ;) */
		else
			return memcmp(pCS1->pBuf, pCS2->pBuf, pCS1->iStrLen);
	else
		return pCS1->iStrLen - pCS2->iStrLen;
}


/* check if a sz-type string starts with a CStr object. This function
 * is initially written to support the "startswith" property-filter
 * comparison operation. Maybe it also has other needs.
 * This functions is modelled after the strcmp() series, thus a
 * return value of 0 indicates that the string starts with the
 * sequence while -1 indicates it does not!
 * rgerhards 2005-10-19
 */
int
rsCStrSzStrStartsWithCStr(cstr_t *const __restrict__ pCS1,
	uchar *const __restrict__ psz,
	const size_t iLenSz)
{
	rsCHECKVALIDOBJECT(pCS1, OIDrsCStr);
	assert(psz != NULL);
	assert(iLenSz == strlen((char*)psz)); /* just make sure during debugging! */
	if(iLenSz >= pCS1->iStrLen) {
		if(pCS1->iStrLen == 0)
			return 0; /* yes, it starts with a zero-sized string ;) */
		else
			return memcmp(psz, pCS1->pBuf, pCS1->iStrLen);
	} else {
		return -1; /* pCS1 is less then psz */
	}
}


/* check if a CStr object matches a regex.
 * msamia@redhat.com 2007-07-12
 * @return returns 0 if matched
 * bug: doesn't work for CStr containing \0
 * rgerhards, 2007-07-16: bug is no real bug, because rsyslogd ensures there
 * never is a \0 *inside* a property string.
 * Note that the function returns -1 if regexp functionality is not available.
 * rgerhards: 2009-03-04: ERE support added, via parameter iType: 0 - BRE, 1 - ERE
 * Arnaud Cornet/rgerhards: 2009-04-02: performance improvement by caching compiled regex
 * If a caller does not need the cached version, it must still provide memory for it
 * and must call rsCStrRegexDestruct() afterwards.
 */
rsRetVal rsCStrSzStrMatchRegex(cstr_t *pCS1, uchar *psz, int iType, void *rc)
{
	regex_t **cache = (regex_t**) rc;
	int ret;
	DEFiRet;

	assert(pCS1 != NULL);
	assert(psz != NULL);
	assert(cache != NULL);

	if(objUse(regexp, LM_REGEXP_FILENAME) == RS_RET_OK) {
		if (*cache == NULL) {
			*cache = calloc(sizeof(regex_t), 1);
			int errcode;
			if((errcode = regexp.regcomp(*cache, (char*) rsCStrGetSzStrNoNULL(pCS1),
				(iType == 1 ? REG_EXTENDED : 0) | REG_NOSUB))) {
				char errbuff[512];
				regexp.regerror(errcode, *cache, errbuff, sizeof(errbuff));
				LogError(0, NO_ERRCODE, "Error in regular expression: %s\n", errbuff);
				ABORT_FINALIZE(RS_RET_NOT_FOUND);
			}
		}
		ret = regexp.regexec(*cache, (char*) psz, 0, NULL, 0);
		if(ret != 0)
			ABORT_FINALIZE(RS_RET_NOT_FOUND);
	} else {
		ABORT_FINALIZE(RS_RET_NOT_FOUND);
	}

finalize_it:
	RETiRet;
}


/* free a cached compiled regex
 * Caller must provide a pointer to a buffer that was created by
 * rsCStrSzStrMatchRegexCache()
 */
void rsCStrRegexDestruct(void *rc)
{
	regex_t **cache = rc;
	
	assert(cache != NULL);
	assert(*cache != NULL);

	if(objUse(regexp, LM_REGEXP_FILENAME) == RS_RET_OK) {
		regexp.regfree(*cache);
		free(*cache);
		*cache = NULL;
	}
}


/* compare a rsCStr object with a classical sz string.  This function
 * is almost identical to rsCStrZsStrCmp(), but it also takes an offset
 * to the CStr object from where the comparison is to start.
 * I have thought quite a while if it really makes sense to more or
 * less duplicate the code. After all, if you call it with an offset of
 * zero, the functionality is exactly the same. So it looks natural to
 * just have a single function. However, supporting the offset requires
 * some (few) additional integer operations. While they are few, they
 * happen at places in the code that is run very frequently. All in all,
 * I have opted for performance and thus duplicated the code. I hope
 * this is a good, or at least acceptable, compromise.
 * rgerhards, 2005-09-26
 * This function also has an offset-pointer which allows to
 * specify *where* the compare operation should begin in
 * the CStr. If everything is to be compared, it must be set
 * to 0. If some leading bytes are to be skipped, it must be set
 * to the first index that is to be compared. It must not be
 * set higher than the string length (this is considered a
 * program bug and will lead to unpredictable results and program aborts).
 * rgerhards 2005-09-26
 */
int rsCStrOffsetSzStrCmp(cstr_t *pCS1, size_t iOffset, uchar *psz, size_t iLenSz)
{
	rsCHECKVALIDOBJECT(pCS1, OIDrsCStr);
	assert(iOffset < pCS1->iStrLen);
	assert(iLenSz == strlen((char*)psz)); /* just make sure during debugging! */
	if((pCS1->iStrLen - iOffset) == iLenSz) {
		/* we are using iLenSz below, because the lengths
		 * are equal and iLenSz is faster to access
		 */
		if(iLenSz == 0) {
			return 0; /* zero-sized strings are equal ;) */
		} else {  /* we now have two non-empty strings of equal
			 * length, so we need to actually check if they
			 * are equal.
			 */
			return memcmp(pCS1->pBuf+iOffset, psz, iLenSz);
		}
	}
	else {
		return pCS1->iStrLen - iOffset - iLenSz;
	}
}


/* compare a rsCStr object with a classical sz string.
 * Just like rsCStrCStrCmp, just for a different data type.
 * There must not only the sz string but also its length be
 * provided. If the caller does not know the length he can
 * call with
 * rsCstrSzStrCmp(pCS, psz, strlen((char*)psz));
 * we are not doing the strlen((char*)) ourselfs as the caller might
 * already know the length and in such cases we can save the
 * overhead of doing it one more time (strelen() is costly!).
 * The bottom line is that the provided length MUST be correct!
 * The to sz string pointer must not be NULL!
 * rgerhards 2005-09-26
 */
int rsCStrSzStrCmp(cstr_t *pCS1, uchar *psz, size_t iLenSz)
{
	rsCHECKVALIDOBJECT(pCS1, OIDrsCStr);
	assert(psz != NULL);
	assert(iLenSz == strlen((char*)psz)); /* just make sure during debugging! */
	if(pCS1->iStrLen == iLenSz)
		if(iLenSz == 0)
			return 0; /* zero-sized strings are equal ;) */
		else
			return strncmp((char*)pCS1->pBuf, (char*)psz, iLenSz);
	else
		return (ssize_t) pCS1->iStrLen - (ssize_t) iLenSz;
}


/* Locate the first occurence of this rsCStr object inside a standard sz string.
 * Returns the offset (0-bound) of this first occurrence. If not found, -1 is
 * returned. Both parameters MUST be given (NULL is not allowed).
 * rgerhards 2005-09-19
 */
int ATTR_NONNULL(1, 2)
rsCStrLocateInSzStr(cstr_t *const pThis, uchar *const sz)
{
	size_t i;
	size_t iMax;
	size_t len_sz = ustrlen(sz);
	int bFound;
	rsCHECKVALIDOBJECT(pThis, OIDrsCStr);
	assert(sz != NULL);
	
	if(pThis->iStrLen == 0)
		return 0;
	
	/* compute the largest index where a match could occur - after all,
	 * the to-be-located string must be able to be present in the
	 * searched string (it needs its size ;)).
	 */
	iMax = (pThis->iStrLen >= len_sz) ? 0 : len_sz - pThis->iStrLen;

	bFound = 0;
	i = 0;
	while(i  <= iMax && !bFound) {
		size_t iCheck;
		uchar *pComp = sz + i;
		for(iCheck = 0 ; iCheck < pThis->iStrLen ; ++iCheck)
			if(*(pComp + iCheck) != *(pThis->pBuf + iCheck))
				break;
		if(iCheck == pThis->iStrLen)
			bFound = 1; /* found! - else it wouldn't be equal */
		else
			++i; /* on to the next try */
	}

	return(bFound ? (int) i : -1);
}


/* our exit function. TODO: remove once converted to a class
 * rgerhards, 2008-03-11
 */
rsRetVal strExit(void)
{
	DEFiRet;
	objRelease(regexp, LM_REGEXP_FILENAME);
	RETiRet;
}


/* our init function. TODO: remove once converted to a class
 */
rsRetVal
strInit(void)
{
	DEFiRet;
	CHKiRet(objGetObjInterface(&obj));

finalize_it:
	RETiRet;
}
