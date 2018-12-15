/**
 * @file string.c
 * Implements string handling
 *
 * @note: for efficiency reasons, later builds may spread the
 * individual functions across different source modules. I was a 
 * bit lazy to do this right now and I am totally unsure if it
 * really is worth the effort.
 *//*
 * libestr - some essentials for string handling (and a bit more)
 * Copyright 2010 by Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of libestr.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * A copy of the LGPL v2.1 can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include "libestr.h"

#define ERR_ABORT {r = 1; goto done; }

#if 1 /* !defined(NDEBUG) TODO: decide if we want this or not! */
#	define CHECK_STR 
#	define ASSERT_STR(s)
#else
#	define CHECK_STR
		if(s->objID != ES_STRING_OID) { \
			r = -1; \
			goto done; \
		}
#	define ASSERT_STR(s) assert((s)->objID == ES_STRING_OID)
#endif /* #if !defined(NDEBUG) */


/* ------------------------------ HELPERS ------------------------------ */

/**
 * Extend string buffer.
 * This is called if the size is insufficient. Note that the string
 * pointer will be changed.
 * @param[in/out] ps pointer to (pointo to) string to be extened
 * @param[in] minNeeded minimum number of additional bytes needed
 * @returns 0 on success, something else otherwise
 */
int
es_extendBuf(es_str_t **ps, es_size_t minNeeded)
{
	int r = 0;
	es_str_t *s = *ps;
	es_size_t newSize;
	es_size_t newAlloc;

	ASSERT_STR(s);
	/* first compute the new size needed */
	if(minNeeded > s->lenBuf) {
		newSize = s->lenBuf + minNeeded;
	} else {
		newSize = 2 * s->lenBuf;
	}
	if(newSize < minNeeded) { /* overflow? */
		r = ENOMEM;
		goto done;
	}

	newAlloc = newSize + sizeof(es_str_t);
	if(newAlloc < newSize) { /* overflow? */
		r = ENOMEM;
		goto done;
	}

	if((s = (es_str_t*) realloc(s, newAlloc)) == NULL) {
		r = errno;
		goto done;
	}
	s->lenBuf = newSize;
	*ps = s;

done:
	return r;
}


/* ------------------------------ END HELPERS ------------------------------ */

es_str_t *
es_newStr(es_size_t lenhint)
{
	es_str_t *s;
	/* we round length to a multiple of 8 in the hope to reduce
	 * memory fragmentation.
	 */
	if(lenhint & 0x07)
		lenhint = lenhint - (lenhint & 0x07) + 8;

	if(sizeof(es_str_t) + lenhint < lenhint) { /* overflow? */
		s = NULL;
		goto done;
	}
	if((s = malloc(sizeof(es_str_t) + lenhint)) == NULL)
		goto done;

#	ifndef NDEBUG
	/*s->objID = ES_STRING_OID;*/
#	endif
	s->lenBuf = lenhint;
	s->lenStr = 0;

done:
	return s;
}


es_str_t*
es_newStrFromCStr(const char *cstr, es_size_t len)
{
	es_str_t *s;
	
	if((s = es_newStr(len)) == NULL) goto done;
	memcpy(es_getBufAddr(s), cstr, len);
	s->lenStr = len;

done:
	return s;
}


es_str_t*
es_newStrFromBuf(char *buf, es_size_t len)
{
	es_str_t *s;
	
	if((s = es_newStr(len)) == NULL) goto done;

	memcpy(es_getBufAddr(s), buf, len);
	s->lenStr = len;

done:
	return s;
}


es_str_t*
es_newStrFromNumber(long long num)
{
	char numbuf[20];	/* 2^64 has 20 digits ;) */
	int i,j;
	char minus = '\0';
	es_str_t *s;
	long long upperBorder = -9223372036854775807LL;
	--upperBorder; /* handle number in C90 and newer modes */
	
	/* handle border case */
	if(num == upperBorder) {
		s = es_newStrFromCStr("-9223372036854775808", 20);
		goto done;
	}

	if (num < 0) {
	    minus = '-';
	    num = -num;
	}
	
	/* generate string (reversed) */
	for(i = 0 ; num != 0 ; ++i) {
		numbuf[i] = num % 10 + '0';
		num /= 10;
	}
	if(i == 0)
		numbuf [i++] = '0';
	if (minus != '\0')
		numbuf[i++] = minus;

	/* now create the actual string */
	if((s = es_newStr(i)) == NULL) goto done;
	s->lenStr = i;
	for(j = 0 ; --i >= 0 ; ++j) {
		es_getBufAddr(s)[j] = numbuf[i];
	}

done:
	return s;
}


es_str_t*
es_newStrFromSubStr(es_str_t *str, es_size_t start, es_size_t len)
{
	es_str_t *s;
	
	if(start+len < start) {
		s = NULL;
		goto done;
	}
	if((s = es_newStr(len)) == NULL) goto done;

	if(start > es_strlen(str))
		goto done;
	else if(start + len > es_strlen(str) - 1)
		len = es_strlen(str) - start;

	memcpy(es_getBufAddr(s), es_getBufAddr(str)+start, len);
	s->lenStr = len;

done:
	return s;
}

void
es_deleteStr(es_str_t *s)
{
	ASSERT_STR(s);
#	if 0 /*!defined(NDEBUG)*/
	s->objID = ES_STRING_FREED;
#	endif
	free(s);
}


int
es_strbufcmp(es_str_t *s, const unsigned char *buf, es_size_t lenBuf)
{
	int r;
	es_size_t i;
	unsigned char *c;

	ASSERT_STR(s);
	assert(buf != NULL);
	c = es_getBufAddr(s);
	r = 0;	/* assume: strings equal, will be reset if not */
	for(i = 0 ; i < s->lenStr ; ++i) {
		if(i == lenBuf) {
			r = 1; /* strings are so far equal, but second string is smaller */
			break;
		}
		if(c[i] != buf[i]) {
			r = c[i] - buf[i];
			break;
		}
	}
	if(r == 0 && s->lenStr < lenBuf)
		r = -1; /* strings are so far equal, but first string is smaller */
	return r;
}


/* The following is the case-insensitive version of es_strbufcmp. It is
 * a separate function for speed puprposes. However, the code is almost
 * identical to es_strbufcmp, so when that one is updated, changes should
 * be copied over to here as well. The only difference is the tolower()
 * call, so change propagation is easy ;)
 */
int
es_strcasebufcmp(es_str_t *s, const unsigned char *buf, es_size_t lenBuf)
{
	int r;
	es_size_t i;
	unsigned char *c;

	ASSERT_STR(s);
	assert(buf != NULL);
	c = es_getBufAddr(s);
	r = 0;	/* assume: strings equal, will be reset if not */
	for(i = 0 ; i < s->lenStr ; ++i) {
		if(i == lenBuf) {
			r = 1;
			break;
		}
		if(tolower(c[i]) != tolower(buf[i])) {
			r = tolower(c[i]) - tolower(buf[i]);
			break;
		}
	}
	if(r == 0 && s->lenStr < lenBuf)
		r = -1;
	return r;
}
int
es_strncmp(es_str_t *s1, es_str_t *s2, es_size_t len)
{
	int r;
	es_size_t i;
	unsigned char *c1, *c2;

	ASSERT_STR(s1);
	ASSERT_STR(s2);
	c1 = es_getBufAddr(s1);
	c2 = es_getBufAddr(s2);
	r = 0;	/* assume: strings equal, will be reset if not */
	for(i = 0 ; i < len ; ++i) {
		if(i >= s1->lenStr) {
			if(i >= s2->lenStr) {
				break; /* we are done, match ready */
			} else {
				r = -1; /* first string smaller --> less */
				break;
			}
		} else {
			if(i >= s2->lenStr) {
				r = 1; /* first string smaller --> greater */
				break;
			} else {
				if(c1[i] != c2[i]) {
					r = c1[i] - c2[i];
					break;
				}
			}
		}
	}
	return r;
}


/* The following is the case-insensitive version of es_strContains. It is
 * a separate function for speed puprposes. However, the code is almost
 * identical to es_strContains, so when that one is updated, changes should
 * be copied over to here as well. The only difference is the tolower()
 * call, so change propagation is easy ;)
 */
int
es_strncasecmp(es_str_t *s1, es_str_t *s2, es_size_t len)
{
	int r;
	es_size_t i;
	unsigned char *c1, *c2;

	ASSERT_STR(s1);
	ASSERT_STR(s2);
	c1 = es_getBufAddr(s1);
	c2 = es_getBufAddr(s2);
	r = 0;	/* assume: strings equal, will be reset if not */
	for(i = 0 ; i < len ; ++i) {
		if(i >= s1->lenStr) {
			if(i >= s1->lenStr) {
				break; /* we are done, match ready */
			} else {
				r = -1; /* first string smaller --> less */
				break;
			}
		} else {
			if(i >= s1->lenStr) {
				r = 1; /* first string smaller --> greater */
				break;
			} else {
				if(tolower(c1[i]) != tolower(c2[i])) {
					r = tolower(c1[i]) - tolower(c2[i]);
					break;
				}
			}
		}
	}
	return r;
}


int
es_strContains(es_str_t *s1, es_str_t *s2)
{
	es_size_t i, j;
	es_size_t max;
	unsigned char *c1, *c2;
	int r;
	
	r = -1;
	if(s2->lenStr > s1->lenStr) {
		/* can not be contained ;) */
		goto done;
	}

	c1 = es_getBufAddr(s1);
	c2 = es_getBufAddr(s2);
	max = s1->lenStr - s2->lenStr + 1;
	for(i = 0 ; i < max ; ++i) {
		for(j = 0 ; j < s2->lenStr; ++j) {
			if(c1[i+j] != c2[j])
				break;
		}
		if(j == s2->lenStr) {
			r = i;
			break;
		}
	}

done:	return r;
}


/* The following is the case-insensitive version of es_strContains. It is
 * a separate function for speed puprposes. However, the code is almost
 * identical to es_strContains, so when that one is updated, changes should
 * be copied over to here as well. The only difference is the tolower()
 * call, so change propagation is easy ;)
 */
int
es_strCaseContains(es_str_t *s1, es_str_t *s2)
{
	es_size_t i, j;
	es_size_t max;
	unsigned char *c1, *c2;
	int r;
	
	r = -1;
	if(s2->lenStr > s1->lenStr) {
		/* can not be contained ;) */
		goto done;
	}

	c1 = es_getBufAddr(s1);
	c2 = es_getBufAddr(s2);
	max = s1->lenStr - s2->lenStr + 1;
	for(i = 0 ; i < max ; ++i) {
		for(j = 0 ; j < s2->lenStr; ++j) {
			if(tolower(c1[i+j]) != tolower(c2[j]))
				break;
		}
		if(j == s2->lenStr) {
			r = i;
			break;
		}
	}

done:	return r;
}


int
es_addChar(es_str_t **ps, const unsigned char c)
{
	int r = 0;

	if((*ps)->lenStr >= (*ps)->lenBuf) {  
		if((r = es_extendBuf(ps, 1)) != 0) goto done;
	}

	/* ok, when we reach this, we have sufficient memory */
	*(es_getBufAddr(*ps) + (*ps)->lenStr++) = c;

done:
	return r;
}


int
es_addBuf(es_str_t **ps1, const char *buf, es_size_t lenBuf)
{
	int r;
	es_size_t newlen;
	es_str_t *s1 = *ps1;

	ASSERT_STR(s1);
	if(lenBuf == 0) {
		r = 0;
		goto done;
	}

	newlen = s1->lenStr + lenBuf;
	if(newlen != (size_t) s1->lenStr + (size_t) lenBuf) {
		r = ENOMEM;
		goto done;
	}
	if(s1->lenBuf < newlen) {
		/* we need to extend */
		if((r = es_extendBuf(ps1, newlen - s1->lenBuf)) != 0) goto done;
		s1 = *ps1;
	}
	
	/* do the actual copy, we now *have* the space required */
	memcpy(es_getBufAddr(s1)+s1->lenStr, buf, lenBuf);
	s1->lenStr = newlen;
	r = 0; /* all well */

done:
	return r;
}


char *
es_str2cstr(es_str_t *s, const char *nulEsc)
{
	char *cstr;
	es_size_t lenEsc;
	int nbrNUL;
	es_size_t i;
	size_t iDst;
	unsigned char *c;

	/* detect number of NULs inside string */
	c = es_getBufAddr(s);
	nbrNUL = 0;
	for(i = 0 ; i < s->lenStr ; ++i) {
		if(c[i] == 0x00)
			++nbrNUL;
	}

	if(nbrNUL == 0) {
		/* no special handling needed */
		if((cstr = malloc(s->lenStr + 1)) == NULL) goto done;
		if(s->lenStr > 0)
			memcpy(cstr, c, s->lenStr);
		cstr[s->lenStr] = '\0';
	} else {
		/* we have NUL bytes present and need to process them
		 * during creation of the C string.
		 */
		lenEsc = (nulEsc == NULL) ? 0 : strlen(nulEsc);
		if((cstr = malloc(s->lenStr + nbrNUL * (lenEsc - 1) + 1)) == NULL)
			goto done;
		for(i = iDst = 0 ; i < s->lenStr ; ++i) {
			if(c[i] == 0x00) {
				if(lenEsc == 1) {
					cstr[iDst++] = *nulEsc;
				} else if(lenEsc > 1) {
					memcpy(cstr + iDst, nulEsc, lenEsc);
					iDst += lenEsc;
				}
			} else {
				cstr[iDst++] = c[i];
			}
		}
		cstr[iDst] = '\0';
	}

done:
	return cstr;
}

/*helpers to es_str2num */
/* startindex is provided for decimal to cover '-' */
static inline long long
es_str2num_dec(es_str_t *s, unsigned i, int *bSuccess)
{
	long long num;
	unsigned char *c;

	num = 0;
	c = es_getBufAddr(s);
	while(i < s->lenStr && isdigit(c[i])) {
		num = num * 10 + c[i] - '0';
		++i;
	}
	if(bSuccess != NULL)
		*bSuccess = (i == s->lenStr) ? 1 : 0;
	return num;
}
static inline long long
es_str2num_oct(es_str_t *s, int *bSuccess)
{
	long long num;
	unsigned char *c;
	unsigned i;

	i = 0;
	num = 0;
	c = es_getBufAddr(s);
	while(i < s->lenStr && (c[i] >= '0' && c[i] <= '7')) {
		num = num * 8 + c[i] - '0';
		++i;
	}
	if(bSuccess != NULL)
		*bSuccess = (i == s->lenStr) ? 1 : 0;
	return num;
}
static inline long long
es_str2num_hex(es_str_t *s, int *bSuccess)
{
	long long num;
	unsigned char *c;
	unsigned i;

	i = 0;
	num = 0;
	c = es_getBufAddr(s) + 2;
	while(i < s->lenStr && isxdigit(c[i])) {
		if(isdigit(c[i]))
			num = num * 16 + c[i] - '0';
		else
			num = num * 16 + tolower(c[i]) - 'a';
		++i;
	}
	if(bSuccess != NULL)
		*bSuccess = (i == s->lenStr) ? 1 : 0;
	return num;
}
/*end helpers to es_str2num */

long long
es_str2num(es_str_t *s, int *bSuccess)
{
	long long num;
	unsigned char *c;

	if(s->lenStr == 0) {
		num = 0;
		*bSuccess = 0;
		goto done;
	}

	c = es_getBufAddr(s);
	if(c[0] == '-') {
		num = -es_str2num_dec(s, 1, bSuccess);
	} else if(c[0] == '0') {
		if(s->lenStr > 1 && c[1] == 'x') {
			num = es_str2num_hex(s, bSuccess);
		} else { 
			num = es_str2num_oct(s, bSuccess);
		}
	} else { /* decimal */
		num = es_str2num_dec(s, 0, bSuccess);
	}

done:	return num;
}


/**
 * Get numerical value of a hex digit. This is a helper function.
 * @param[in] c a character containing 0..9, A..Z, a..z anything else
 * is an (undetected) error.
 */
static inline int hexDigitVal(char c)
{
	int r;
	if(c < 'A')
		r = c - '0';
	else if(c < 'a')
		r = c - 'A' + 10;
	else
		r = c - 'a' + 10;
	return r;
}

/* Handle the actual unescaping.
 * a helper to es_unescapeStr(), to help make the function easier to read.
 */
static inline void
doUnescape(unsigned char *c, es_size_t lenStr, es_size_t *iSrc, es_size_t iDst)
{
	if(c[*iSrc] == '\\') {
		if(++(*iSrc) == lenStr) {
			/* error, incomplete escape, treat as single char */
			c[iDst] = '\\';
		}
		/* regular case, unescape */
		switch(c[*iSrc]) {
		case '0':
			c[iDst] = '\0';
			break;
		case 'a':
			c[iDst] = '\007';
			break;
		case 'b':
			c[iDst] = '\b';
			break;
		case 'f':
			c[iDst] = '\014';
			break;
		case 'n':
			c[iDst] = '\n';
			break;
		case 'r':
			c[iDst] = '\r';
			break;
		case 't':
			c[iDst] = '\t';
			break;
		case '\'':
			c[iDst] = '\'';
			break;
		case '"':
			c[iDst] = '"';
			break;
		case '?':
			c[iDst] = '?';
			break;
		case '\\':
			c[iDst] = '\\';
			break;
		case 'x':
			if((*iSrc)+1 == lenStr) {
				/* just end run, leave as is */
				*iSrc += 1;
				goto done;
			}
			if(    (*iSrc)+2 == lenStr
			   || !isxdigit(c[(*iSrc)+1])
			   || !isxdigit(c[(*iSrc)+2])) {
				/* error, incomplete escape, use as is */
				c[iDst] = '\\';
				--(*iSrc);
			}
			c[iDst] = (hexDigitVal(c[(*iSrc)+1]) << 4) +
				  hexDigitVal(c[(*iSrc)+2]);
			*iSrc += 2;
			break;
		default:
			/* error, incomplete escape, use as is.  Ideally we
			   should reject it instead, to allow for future
			   enhancements, but that would break ABI of
			   es_unescapeStr. */
			c[iDst] = '\\';
			--(*iSrc);
			break;
		}
	} else {
		/* regular character */
		c[iDst] = c[*iSrc];
	}
done:	return;
}

void
es_unescapeStr(es_str_t *s)
{
	es_size_t iSrc, iDst;
	unsigned char *c;
	assert(s != NULL);

	c = es_getBufAddr(s);
	/* scan for first escape sequence (if we are luky, there is none!) */
	iSrc = 0;
	while(iSrc < s->lenStr && c[iSrc] != '\\')
		++iSrc;
	/* now we have a sequence or end of string. In any case, we process
	 * all remaining characters (maybe 0!) and unescape.
	 */
	if(iSrc != s->lenStr) {
		iDst = iSrc;
		while(iSrc < s->lenStr) {
			doUnescape(c, s->lenStr, &iSrc, iDst);
			++iSrc;
			++iDst;
		}
		s->lenStr = iDst;
	}
}

void
es_tolower(es_str_t *s)
{
	es_size_t i;

	for(i = 0 ; i < s->lenStr ; ++i)
		es_getBufAddr(s)[i] = tolower(es_getBufAddr(s)[i]);
}
