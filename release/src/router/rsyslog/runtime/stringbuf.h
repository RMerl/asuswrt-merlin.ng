/* stringbuf.h
 * The counted string object
 *
 * \author  Rainer Gerhards <rgerhards@adiscon.com>
 * \date    2005-09-07
 *          Initial version  begun.
 *
 * Copyright 2005-2016 Adiscon GmbH. All Rights Reserved.
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
#ifndef _STRINGBUF_H_INCLUDED__
#define _STRINGBUF_H_INCLUDED__ 1

#include <assert.h>
#include <libestr.h>

/**
 * The dynamic string buffer object.
 */
typedef struct cstr_s
{
#ifndef	NDEBUG
	rsObjID OID;		/**< object ID */
	sbool isFinalized;
#endif
	uchar *pBuf;		/**< pointer to the string buffer, may be NULL if string is empty */
	size_t iBufSize;	/**< current maximum size of the string buffer */
	size_t iStrLen;		/**< length of the string in characters. */
} cstr_t;


/**
 * Construct a rsCStr object.
 */
rsRetVal cstrConstruct(cstr_t **ppThis);
#define rsCStrConstruct(x) cstrConstruct((x))
rsRetVal cstrConstructFromESStr(cstr_t **ppThis, es_str_t *str);
rsRetVal rsCStrConstructFromszStr(cstr_t **ppThis, const uchar *sz);
rsRetVal rsCStrConstructFromCStr(cstr_t **ppThis, const cstr_t *pFrom);
rsRetVal rsCStrConstructFromszStrf(cstr_t **ppThis, const char *fmt, ...) __attribute__((format(printf,2, 3)));

/**
 * Destruct the string buffer object.
 */
void rsCStrDestruct(cstr_t **ppThis);
#define cstrDestruct(x) rsCStrDestruct((x))


/* Append a character to the current string object. This may only be done until
 * cstrFinalize() is called.
 * rgerhards, 2009-06-16
 */
rsRetVal cstrAppendChar(cstr_t *pThis, const uchar c);

/* Finalize the string object. This must be called after all data is added to it
 * but before that data is used.
 * rgerhards, 2009-06-16
 */
#ifdef NDEBUG
#define cstrFinalize(pThis) { \
	if((pThis)->iStrLen > 0) \
		(pThis)->pBuf[(pThis)->iStrLen] = '\0'; /* space is always reserved for this */ \
}
#else
#define cstrFinalize(pThis) { \
	if((pThis)->iStrLen > 0) \
		(pThis)->pBuf[(pThis)->iStrLen] = '\0'; /* space is always reserved for this */ \
	(pThis)->isFinalized = 1; \
}
#endif


/**
 * Truncate "n" number of characters from the end of the
 * string. The buffer remains unchanged, just the
 * string length is manipulated. This is for performance
 * reasons.
 */
rsRetVal rsCStrTruncate(cstr_t *pThis, size_t nTrunc);

void cstrTrimTrailingWhiteSpace(cstr_t *pThis);

/**
 * Append a string to the buffer. For performance reasons,
 * use rsCStrAppenStrWithLen() if you know the length.
 *
 * \param psz pointer to string to be appended. Must not be NULL.
 */
rsRetVal rsCStrAppendStr(cstr_t *pThis, const uchar* psz);

/**
 * Append a string to the buffer.
 *
 * \param psz pointer to string to be appended. Must not be NULL.
 * \param iStrLen the length of the string pointed to by psz
 */
rsRetVal rsCStrAppendStrWithLen(cstr_t *pThis, const uchar* psz, size_t iStrLen);

/**
 * Append a printf-style formated string to the buffer.
 *
 * \param fmt pointer to the format string (see man 3 printf for details). Must not be NULL.
 */
rsRetVal rsCStrAppendStrf(cstr_t *pThis, const char *fmt, ...) __attribute__((format(printf,2, 3)));

/**
 * Append an integer to the string. No special formatting is
 * done.
 */
rsRetVal rsCStrAppendInt(cstr_t *pThis, long i);


rsRetVal strExit(void);
uchar*  cstrGetSzStrNoNULL(cstr_t *pThis);
#define rsCStrGetSzStrNoNULL(x) cstrGetSzStrNoNULL(x)
rsRetVal rsCStrSetSzStr(cstr_t *pThis, uchar *pszNew);
int rsCStrCStrCmp(cstr_t *pCS1, cstr_t *pCS2);
int rsCStrSzStrCmp(cstr_t *pCS1, uchar *psz, size_t iLenSz);
int rsCStrOffsetSzStrCmp(cstr_t *pCS1, size_t iOffset, uchar *psz, size_t iLenSz);
int rsCStrLocateSzStr(cstr_t *pCStr, uchar *sz);
int rsCStrLocateInSzStr(cstr_t *pThis, uchar *sz);
int rsCStrSzStrStartsWithCStr(cstr_t *pCS1, uchar *psz, size_t iLenSz);
rsRetVal rsCStrSzStrMatchRegex(cstr_t *pCS1, uchar *psz, int iType, void *cache);
void rsCStrRegexDestruct(void *rc);

/* new calling interface */
rsRetVal cstrConvSzStrAndDestruct(cstr_t **pThis, uchar **ppSz, int bRetNULL);
rsRetVal cstrAppendCStr(cstr_t *pThis, cstr_t *pstrAppend);

/* now come inline-like functions */
#ifdef NDEBUG
#	define cstrLen(x) ((int)((x)->iStrLen))
#else
	int cstrLen(cstr_t *pThis);
#endif
#define rsCStrLen(s) cstrLen((s))

#define rsCStrGetBufBeg(x) ((x)->pBuf)

rsRetVal strInit(void);

#endif /* single include */
