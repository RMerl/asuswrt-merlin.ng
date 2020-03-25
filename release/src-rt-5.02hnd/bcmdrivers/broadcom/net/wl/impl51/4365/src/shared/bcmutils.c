/*
 * Driver O/S-independent utility routines
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * $Id: bcmutils.c 668198 2016-11-02 07:31:15Z $
 */

#ifndef __FreeBSD__
#include <bcm_cfg.h>
#endif
#include <typedefs.h>
#include <bcmdefs.h>
#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <machine/stdarg.h>
#include <stdbool.h>
#else
#include <stdarg.h>
#endif
#ifdef BCMDRIVER

#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmnvram.h>

#else /* !BCMDRIVER */

#include <stdio.h>
#include <string.h>
#include <bcmutils.h>

#if defined(BCMEXTSUP)
#include <bcm_osl.h>
#endif

#ifndef ASSERT
#define ASSERT(exp)
#endif

#endif /* !BCMDRIVER */

#if defined(_WIN32) || defined(NDIS) || defined(_CFE_)
/* Debatable */
#include <bcmstdlib.h>
#endif
#include <bcmendian.h>
#include <bcmdevs.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmip.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#ifdef BCMPERFSTATS
#include <bcmperf.h>
#endif


void *_bcmutils_dummy_fn = NULL;

#if defined(BCMDRIVER)
/* Forward declarations */
static char * getvar_internal(char *vars, const char *name);
static int getintvar_internal(char *vars, const char *name);
static int getintvararray_internal(char *vars, const char *name, int index);
static int getintvararraysize_internal(char *vars, const char *name);
#endif 

#ifdef CUSTOM_DSCP_TO_PRIO_MAPPING
#define CUST_IPV4_TOS_PREC_MASK 0x3F
#define DCSP_MAX_VALUE 64
/* 0:BE,1:BK,2:RESV(BK):,3:EE,:4:CL,5:VI,6:VO,7:NC */
int dscp2priomap[DCSP_MAX_VALUE]=
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, /* BK->BE */
	2, 0, 0, 0, 0, 0, 0, 0,
	3, 0, 0, 0, 0, 0, 0, 0,
	4, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0,
	6, 0, 0, 0, 0, 0, 0, 0,
	7, 0, 0, 0, 0, 0, 0, 0
};
#endif /* CUSTOM_DSCP_TO_PRIO_MAPPING */


#ifdef BCMDRIVER

#ifdef WLC_LOW
/* nvram vars cache */
static char *nvram_vars = NULL;
static int vars_len = -1;
#endif /* WLC_LOW */

/* rxcpl list management */
bcm_rxcplid_list_t *g_rxcplid_list = NULL;

static const char BCMATTACHDATA(rstr_allocating_rxcplid_buffers)[]   =
	"allocating a max of %d rxcplid buffers\n";
static const char BCMATTACHDATA(rstr_rxcplid_list_already_inited)[]  =
	"ERROR: rxcplid list already inited\n";
static const char BCMATTACHDATA(rstr_rxcplid_list_allocation_fail)[] =
	"ERROR: rxcplid list allocation fail, size %d, items %d\n";
bool
BCMATTACHFN(bcm_alloc_rxcplid_list)(osl_t *osh, uint32 rxcpl_max)
{
	uint32 size;
	rxcpl_info_t *ptr;
	uint32 i;

	printf("allocating a max of %d rxcplid buffers\n", rxcpl_max);

	if (g_rxcplid_list != NULL) {
		printf(rstr_rxcplid_list_already_inited);
		return FALSE;
	}
	size = sizeof(bcm_rxcplid_list_t) + (rxcpl_max * sizeof(rxcpl_info_t));
	g_rxcplid_list = (bcm_rxcplid_list_t *)MALLOC(osh, size);
	if (g_rxcplid_list == NULL) {
		printf("ERROR: rxcplid list allocation fail, size %d, items %d\n", size, rxcpl_max);
		return FALSE;
	}
	bzero(g_rxcplid_list, size);
	g_rxcplid_list->max = rxcpl_max - 1;
	g_rxcplid_list->rxcpl_ptr = (rxcpl_info_t *)(g_rxcplid_list + 1);
	g_rxcplid_list->free_list = g_rxcplid_list->rxcpl_ptr + 1;
	ptr = g_rxcplid_list->free_list;
	for (i = 1; i <= g_rxcplid_list->max; i++) {
		ptr->rxcpl_id.idx = i;
		if (i != g_rxcplid_list->max) {
			ptr->free_next = ptr + 1;
			ptr = ptr->free_next;
		}
		else
			ptr->free_next = NULL;
	}
	g_rxcplid_list->avail = g_rxcplid_list->max;
	return TRUE;
}

rxcpl_info_t *
bcm_alloc_rxcplinfo()
{
	rxcpl_info_t *ptr;
	if (g_rxcplid_list == NULL)
		return NULL;
	if (g_rxcplid_list->avail == 0)
		return NULL;
	if (g_rxcplid_list->free_list == NULL)
		return NULL;

	ptr = g_rxcplid_list->free_list;
	g_rxcplid_list->free_list = ptr->free_next;

	g_rxcplid_list->avail--;

	if ((g_rxcplid_list->free_list == NULL) && (g_rxcplid_list->avail != 0)) {
		printf("ERROR :something is really wrong here, idx is %d, avail %d\n",
			ptr->rxcpl_id.idx, g_rxcplid_list->avail);
		ASSERT(0);
	}

	ptr->rxcpl_id.next_idx = 0;
	ptr->rxcpl_id.flags = 0;
	ptr->free_next = NULL;

	if (ptr->rxcpl_id.idx == 0) {
		printf("ERROR: allocating rxpl_info id %d\n", ptr->rxcpl_id.idx);
		ASSERT(0);
	}
	BCM_RXCPL_SET_IN_TRANSIT(ptr);

	return ptr;
}

void
bcm_free_rxcplinfo(rxcpl_info_t *ptr)
{
	if (g_rxcplid_list == NULL)
		return;

	if (g_rxcplid_list->avail == g_rxcplid_list->max) {
		printf("ERROR: fail to free the rxcpl entry %d, avail %d\n",
			ptr->rxcpl_id.idx, g_rxcplid_list->avail);
		ASSERT(0);
		return;
	}
	if (ptr->rxcpl_id.idx == 0) {
		printf("ERROR: freeing rxpl_info id %d\n", ptr->rxcpl_id.idx);
		ASSERT(0);
	}

	ptr->free_next = g_rxcplid_list->free_list;
	g_rxcplid_list->free_list = ptr;
	g_rxcplid_list->avail++;
	ptr->rxcpl_id.next_idx = 0;
	ptr->rxcpl_id.flags = 0;

}

void
bcm_chain_rxcplid(uint16 first,  uint16 next)
{
	rxcpl_info_t *ptr;
	uint32 count = 0;

	if (g_rxcplid_list == NULL)
		return;
	if ((first == 0) || (next == 0) || (first == next)) {
		printf("ERROR: chaining  going bad first %d, next %d\n", first, next);
		ASSERT(0);
		return;

	}

	if ((first > g_rxcplid_list->max) || (next > g_rxcplid_list->max))
		return;
	ptr = bcm_id2rxcplinfo(first);
#ifdef DEBUG
	if (bcm_rxcpllist_end(ptr, &count) != ptr) {
		printf("chaining with chain at front\n");
	}
#endif
	ptr = bcm_rxcpllist_end(ptr, &count);
	ptr->rxcpl_id.next_idx = next;
}

rxcpl_info_t *
bcm_id2rxcplinfo(uint16 id)
{
	if (id == 0) {
		return NULL;
	}
	if (id > g_rxcplid_list->max) {
		ASSERT(0);
		return NULL;
	}
	return (&g_rxcplid_list->rxcpl_ptr[id]);

}

uint16
bcm_rxcplinfo2id(rxcpl_info_t *ptr)
{
	return (uint16)(ptr->rxcpl_id.idx);
}

rxcpl_info_t *
bcm_rxcpllist_end(rxcpl_info_t *ptr, uint32 *count)
{
	uint32 cnt = 1;
	while (ptr->rxcpl_id.next_idx != 0) {
		ptr = bcm_id2rxcplinfo((uint16)(ptr->rxcpl_id.next_idx));
		cnt++;
	}
	*count = cnt;
	return ptr;
}

/* copy a pkt buffer chain into a buffer */
uint
pktcopy(osl_t *osh, void *p, uint offset, int len, uchar *buf)
{
	uint n, ret = 0;

	if (len < 0)
		len = 4096;	/* "infinite" */

	/* skip 'offset' bytes */
	for (; p && offset; p = PKTNEXT(osh, p)) {
		if (offset < (uint)PKTLEN(osh, p))
			break;
		offset -= PKTLEN(osh, p);
	}

	if (!p)
		return 0;

	/* copy the data */
	for (; p && len; p = PKTNEXT(osh, p)) {
		n = MIN((uint)PKTLEN(osh, p) - offset, (uint)len);
		bcopy(PKTDATA(osh, p) + offset, buf, n);
		buf += n;
		len -= n;
		ret += n;
		offset = 0;
	}

	return ret;
}

/* copy a buffer into a pkt buffer chain */
uint
pktfrombuf(osl_t *osh, void *p, uint offset, int len, uchar *buf)
{
	uint n, ret = 0;


	/* skip 'offset' bytes */
	for (; p && offset; p = PKTNEXT(osh, p)) {
		if (offset < (uint)PKTLEN(osh, p))
			break;
		offset -= PKTLEN(osh, p);
	}

	if (!p)
		return 0;

	/* copy the data */
	for (; p && len; p = PKTNEXT(osh, p)) {
		n = MIN((uint)PKTLEN(osh, p) - offset, (uint)len);
		bcopy(buf, PKTDATA(osh, p) + offset, n);
		buf += n;
		len -= n;
		ret += n;
		offset = 0;
	}

	return ret;
}

#ifdef NOTYET
/* copy data from one pkt buffer (chain) to another */
uint
pkt2pktcopy(osl_t *osh, void *p1, uint offs1, void *p2, uint offs2, int maxlen)
{
	uint8 *dp1, *dp2;
	uint len1, len2, copylen, totallen;

	for (; p1 && offs; p1 = PKTNEXT(osh, p1)) {
		if (offs1 < (uint)PKTLEN(osh, p1))
			break;
		offs1 -= PKTLEN(osh, p1);
	}
	for (; p2 && offs; p2 = PKTNEXT(osh, p2)) {
		if (offs2 < (uint)PKTLEN(osh, p2))
			break;
		offs2 -= PKTLEN(osh, p2);
	}

	/* Heck w/it, only need the above for now */
}
#endif /* NOTYET */


/* return total length of buffer chain */
uint BCMFASTPATH
pkttotlen(osl_t *osh, void *p)
{
	uint total;
	int len;

	total = 0;
	for (; p; p = PKTNEXT(osh, p)) {
		len = PKTLEN(osh, p);
#ifdef MACOSX
		if (len < 0) {
			/* Bad packet length, just drop and exit */
#ifdef BCMDBG
			printf("wl: pkttotlen bad (%p,%d)\n", p, len);
#endif
			break;
		}
#endif /* MACOSX */
		total += len;
#ifdef BCMLFRAG
		if (BCMLFRAG_ENAB()) {
			if (PKTISFRAG(osh, p)) {
				total += PKTFRAGTOTLEN(osh, p);
			}
		}
#endif
	}

	return (total);
}

/* return the last buffer of chained pkt */
void *
pktlast(osl_t *osh, void *p)
{
	for (; PKTNEXT(osh, p); p = PKTNEXT(osh, p))
		;

	return (p);
}

/* count segments of a chained packet */
uint BCMFASTPATH
pktsegcnt(osl_t *osh, void *p)
{
	uint cnt;

	for (cnt = 0; p; p = PKTNEXT(osh, p)) {
		cnt++;
#ifdef BCMLFRAG
		if (BCMLFRAG_ENAB()) {
			if (PKTISFRAG(osh, p)) {
				cnt += PKTFRAGTOTNUM(osh, p);
			}
		}
#endif
	}

	return cnt;
}


/* count segments of a chained packet */
uint BCMFASTPATH
pktsegcnt_war(osl_t *osh, void *p)
{
	uint cnt;
	uint8 *pktdata;
	uint len, remain, align64;

	for (cnt = 0; p; p = PKTNEXT(osh, p)) {
		cnt++;
		len = PKTLEN(osh, p);
		if (len > 128) {
			pktdata = (uint8 *)PKTDATA(osh, p);	/* starting address of data */
			/* Check for page boundary straddle (2048B) */
			if (((uintptr)pktdata & ~0x7ff) != ((uintptr)(pktdata+len) & ~0x7ff))
				cnt++;

			align64 = (uint)((uintptr)pktdata & 0x3f);	/* aligned to 64B */
			align64 = (64 - align64) & 0x3f;
			len -= align64;		/* bytes from aligned 64B to end */
			/* if aligned to 128B, check for MOD 128 between 1 to 4B */
			remain = len % 128;
			if (remain > 0 && remain <= 4)
				cnt++;		/* add extra seg */
		}
	}

	return cnt;
}

uint8 * BCMFASTPATH
pktdataoffset(osl_t *osh, void *p,  uint offset)
{
	uint total = pkttotlen(osh, p);
	uint pkt_off = 0, len = 0;
	uint8 *pdata = (uint8 *) PKTDATA(osh, p);

	if (offset > total)
		return NULL;

	for (; p; p = PKTNEXT(osh, p)) {
		pdata = (uint8 *) PKTDATA(osh, p);
		pkt_off = offset - len;
		len += PKTLEN(osh, p);
		if (len > offset)
			break;
	}
	return (uint8*) (pdata+pkt_off);
}


/* given a offset in pdata, find the pkt seg hdr */
void *
pktoffset(osl_t *osh, void *p,  uint offset)
{
	uint total = pkttotlen(osh, p);
	uint len = 0;

	if (offset > total)
		return NULL;

	for (; p; p = PKTNEXT(osh, p)) {
		len += PKTLEN(osh, p);
		if (len > offset)
			break;
	}
	return p;
}

#endif /* BCMDRIVER */

#if !defined(BCMROMOFFLOAD_EXCLUDE_BCMUTILS_FUNCS)
const unsigned char bcm_ctype[] = {

	_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,			/* 0-7 */
	_BCM_C, _BCM_C|_BCM_S, _BCM_C|_BCM_S, _BCM_C|_BCM_S, _BCM_C|_BCM_S, _BCM_C|_BCM_S, _BCM_C,
	_BCM_C,	/* 8-15 */
	_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,			/* 16-23 */
	_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,			/* 24-31 */
	_BCM_S|_BCM_SP,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,		/* 32-39 */
	_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 40-47 */
	_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,			/* 48-55 */
	_BCM_D,_BCM_D,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 56-63 */
	_BCM_P, _BCM_U|_BCM_X, _BCM_U|_BCM_X, _BCM_U|_BCM_X, _BCM_U|_BCM_X, _BCM_U|_BCM_X,
	_BCM_U|_BCM_X, _BCM_U, /* 64-71 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,			/* 72-79 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,			/* 80-87 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 88-95 */
	_BCM_P, _BCM_L|_BCM_X, _BCM_L|_BCM_X, _BCM_L|_BCM_X, _BCM_L|_BCM_X, _BCM_L|_BCM_X,
	_BCM_L|_BCM_X, _BCM_L, /* 96-103 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L, /* 104-111 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L, /* 112-119 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_C, /* 120-127 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 128-143 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 144-159 */
	_BCM_S|_BCM_SP, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P,
	_BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P,	/* 160-175 */
	_BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P,
	_BCM_P, _BCM_P, _BCM_P, _BCM_P, _BCM_P,	/* 176-191 */
	_BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U,
	_BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U,	/* 192-207 */
	_BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_P, _BCM_U, _BCM_U, _BCM_U,
	_BCM_U, _BCM_U, _BCM_U, _BCM_U, _BCM_L,	/* 208-223 */
	_BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L,
	_BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L,	/* 224-239 */
	_BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_P, _BCM_L, _BCM_L, _BCM_L,
	_BCM_L, _BCM_L, _BCM_L, _BCM_L, _BCM_L /* 240-255 */
};

ulong
bcm_strtoul(const char *cp, char **endp, uint base)
{
	ulong result, last_result = 0, value;
	bool minus;

	minus = FALSE;

	while (bcm_isspace(*cp))
		cp++;

	if (cp[0] == '+')
		cp++;
	else if (cp[0] == '-') {
		minus = TRUE;
		cp++;
	}

	if (base == 0) {
		if (cp[0] == '0') {
			if ((cp[1] == 'x') || (cp[1] == 'X')) {
				base = 16;
				cp = &cp[2];
			} else {
				base = 8;
				cp = &cp[1];
			}
		} else
			base = 10;
	} else if (base == 16 && (cp[0] == '0') && ((cp[1] == 'x') || (cp[1] == 'X'))) {
		cp = &cp[2];
	}

	result = 0;

	while (bcm_isxdigit(*cp) &&
	       (value = bcm_isdigit(*cp) ? *cp-'0' : bcm_toupper(*cp)-'A'+10) < base) {
		result = result*base + value;
		/* Detected overflow */
		if (result < last_result && !minus) {
			if (endp) {
				/* Go to the end of current number */
				while (bcm_isxdigit(*cp)) {
					cp++;
				}
				*endp = DISCARD_QUAL(cp, char);
			}
			return (ulong)-1;
		}
		last_result = result;
		cp++;
	}

	if (minus)
		result = (ulong)(-(long)result);

	if (endp)
		*endp = DISCARD_QUAL(cp, char);

	return (result);
}

int
bcm_atoi(const char *s)
{
	return (int)bcm_strtoul(s, NULL, 10);
}

/* return pointer to location of substring 'needle' in 'haystack' */
char *
bcmstrstr(const char *haystack, const char *needle)
{
	int len, nlen;
	int i;

	if ((haystack == NULL) || (needle == NULL))
		return DISCARD_QUAL(haystack, char);

	nlen = (int)strlen(needle);
	len = (int)strlen(haystack) - nlen + 1;

	for (i = 0; i < len; i++)
		if (memcmp(needle, &haystack[i], nlen) == 0)
			return DISCARD_QUAL(&haystack[i], char);
	return (NULL);
}

char *
bcmstrnstr(const char *s, uint s_len, const char *substr, uint substr_len)
{
	for (; s_len >= substr_len; s++, s_len--)
		if (strncmp(s, substr, substr_len) == 0)
			return DISCARD_QUAL(s, char);

	return NULL;
}

char *
bcmstrcat(char *dest, const char *src)
{
	char *p;

	p = dest + strlen(dest);

	while ((*p++ = *src++) != '\0')
		;

	return (dest);
}

char *
bcmstrncat(char *dest, const char *src, uint size)
{
	char *endp;
	char *p;

	p = dest + strlen(dest);
	endp = p + size;

	while (p != endp && (*p++ = *src++) != '\0')
		;

	return (dest);
}


/****************************************************************************
* Function:   bcmstrtok
*
* Purpose:
*  Tokenizes a string. This function is conceptually similiar to ANSI C strtok(),
*  but allows strToken() to be used by different strings or callers at the same
*  time. Each call modifies '*string' by substituting a NULL character for the
*  first delimiter that is encountered, and updates 'string' to point to the char
*  after the delimiter. Leading delimiters are skipped.
*
* Parameters:
*  string      (mod) Ptr to string ptr, updated by token.
*  delimiters  (in)  Set of delimiter characters.
*  tokdelim    (out) Character that delimits the returned token. (May
*                    be set to NULL if token delimiter is not required).
*
* Returns:  Pointer to the next token found. NULL when no more tokens are found.
*****************************************************************************
*/
char *
bcmstrtok(char **string, const char *delimiters, char *tokdelim)
{
	unsigned char *str;
	unsigned long map[8];
	int count;
	char *nextoken;

	if (tokdelim != NULL) {
		/* Prime the token delimiter */
		*tokdelim = '\0';
	}

	/* Clear control map */
	for (count = 0; count < 8; count++) {
		map[count] = 0;
	}

	/* Set bits in delimiter table */
	do {
		map[*delimiters >> 5] |= (1 << (*delimiters & 31));
	}
	while (*delimiters++);

	str = (unsigned char*)*string;

	/* Find beginning of token (skip over leading delimiters). Note that
	 * there is no token iff this loop sets str to point to the terminal
	 * null (*str == '\0')
	 */
	while (((map[*str >> 5] & (1 << (*str & 31))) && *str) || (*str == ' ')) {
		str++;
	}

	nextoken = (char*)str;

	/* Find the end of the token. If it is not the end of the string,
	 * put a null there.
	 */
	for (; *str; str++) {
		if (map[*str >> 5] & (1 << (*str & 31))) {
			if (tokdelim != NULL) {
				*tokdelim = *str;
			}

			*str++ = '\0';
			break;
		}
	}

	*string = (char*)str;

	/* Determine if a token has been found. */
	if (nextoken == (char *) str) {
		return NULL;
	}
	else {
		return nextoken;
	}
}


#define xToLower(C) \
	((C >= 'A' && C <= 'Z') ? (char)((int)C - (int)'A' + (int)'a') : C)


/****************************************************************************
* Function:   bcmstricmp
*
* Purpose:    Compare to strings case insensitively.
*
* Parameters: s1 (in) First string to compare.
*             s2 (in) Second string to compare.
*
* Returns:    Return 0 if the two strings are equal, -1 if t1 < t2 and 1 if
*             t1 > t2, when ignoring case sensitivity.
*****************************************************************************
*/
int
bcmstricmp(const char *s1, const char *s2)
{
	char dc, sc;

	while (*s2 && *s1) {
		dc = xToLower(*s1);
		sc = xToLower(*s2);
		if (dc < sc) return -1;
		if (dc > sc) return 1;
		s1++;
		s2++;
	}

	if (*s1 && !*s2) return 1;
	if (!*s1 && *s2) return -1;
	return 0;
}


/****************************************************************************
* Function:   bcmstrnicmp
*
* Purpose:    Compare to strings case insensitively, upto a max of 'cnt'
*             characters.
*
* Parameters: s1  (in) First string to compare.
*             s2  (in) Second string to compare.
*             cnt (in) Max characters to compare.
*
* Returns:    Return 0 if the two strings are equal, -1 if t1 < t2 and 1 if
*             t1 > t2, when ignoring case sensitivity.
*****************************************************************************
*/
int
bcmstrnicmp(const char* s1, const char* s2, int cnt)
{
	char dc, sc;

	while (*s2 && *s1 && cnt) {
		dc = xToLower(*s1);
		sc = xToLower(*s2);
		if (dc < sc) return -1;
		if (dc > sc) return 1;
		s1++;
		s2++;
		cnt--;
	}

	if (!cnt) return 0;
	if (*s1 && !*s2) return 1;
	if (!*s1 && *s2) return -1;
	return 0;
}

/* parse a xx:xx:xx:xx:xx:xx format ethernet address */
int
bcm_ether_atoe(const char *p, struct ether_addr *ea)
{
	int i = 0;
	char *ep;

	for (;;) {
		ea->octet[i++] = (char) bcm_strtoul(p, &ep, 16);
		p = ep;
		if (!*p++ || i == 6)
			break;
	}

	return (i == 6);
}

int
bcm_atoipv4(const char *p, struct ipv4_addr *ip)
{

	int i = 0;
	char *c;
	for (;;) {
		ip->addr[i++] = (uint8)bcm_strtoul(p, &c, 0);
		if (*c++ != '.' || i == IPV4_ADDR_LEN)
			break;
		p = c;
	}
	return (i == IPV4_ADDR_LEN);
}
#endif	/* !BCMROMOFFLOAD_EXCLUDE_BCMUTILS_FUNCS */

#ifdef DONGLEBUILD

const struct ether_addr ether_bcast = {{255, 255, 255, 255, 255, 255}};
const struct ether_addr ether_null = {{0, 0, 0, 0, 0, 0}};
const struct ether_addr ether_ipv6_mcast = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x01}};

int
ether_isbcast(const void *ea)
{
	return (memcmp(ea, &ether_bcast, sizeof(struct ether_addr)) == 0);
}

int
ether_isnulladdr(const void *ea)
{
	return (memcmp(ea, &ether_null, sizeof(struct ether_addr)) == 0);
}

#endif /* _RTE_ */

#if defined(CONFIG_USBRNDIS_RETAIL) || defined(NDIS_MINIPORT_DRIVER)
/* registry routine buffer preparation utility functions:
 * parameter order is like strncpy, but returns count
 * of bytes copied. Minimum bytes copied is null char(1)/wchar(2)
 */
ulong
wchar2ascii(char *abuf, ushort *wbuf, ushort wbuflen, ulong abuflen)
{
	ulong copyct = 1;
	ushort i;

	if (abuflen == 0)
		return 0;

	/* wbuflen is in bytes */
	wbuflen /= sizeof(ushort);

	for (i = 0; i < wbuflen; ++i) {
		if (--abuflen == 0)
			break;
		*abuf++ = (char) *wbuf++;
		++copyct;
	}
	*abuf = '\0';

	return copyct;
}
#endif /* CONFIG_USBRNDIS_RETAIL || NDIS_MINIPORT_DRIVER */

char *
bcm_ether_ntoa(const struct ether_addr *ea, char *buf)
{
	static const char hex[] =
	  {
		  '0', '1', '2', '3', '4', '5', '6', '7',
		  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	  };
	const uint8 *octet = ea->octet;
	char *p = buf;
	int i;

	for (i = 0; i < 6; i++, octet++) {
		*p++ = hex[(*octet >> 4) & 0xf];
		*p++ = hex[*octet & 0xf];
		*p++ = ':';
	}

	*(p-1) = '\0';

	return (buf);
}

char *
bcm_ip_ntoa(struct ipv4_addr *ia, char *buf)
{
	snprintf(buf, 16, "%d.%d.%d.%d",
	         ia->addr[0], ia->addr[1], ia->addr[2], ia->addr[3]);
	return (buf);
}

char *
bcm_ipv6_ntoa(void *ipv6, char *buf)
{
	/* Implementing RFC 5952 Sections 4 + 5 */
	/* Not thoroughly tested */
	uint16 tmp[8];
	uint16 *a = &tmp[0];
	char *p = buf;
	int i, i_max = -1, cnt = 0, cnt_max = 1;
	uint8 *a4 = NULL;
	memcpy((uint8 *)&tmp[0], (uint8 *)ipv6, IPV6_ADDR_LEN);

	for (i = 0; i < IPV6_ADDR_LEN/2; i++) {
		if (a[i]) {
			if (cnt > cnt_max) {
				cnt_max = cnt;
				i_max = i - cnt;
			}
			cnt = 0;
		} else
			cnt++;
	}
	if (cnt > cnt_max) {
		cnt_max = cnt;
		i_max = i - cnt;
	}
	if (i_max == 0 &&
		/* IPv4-translated: ::ffff:0:a.b.c.d */
		((cnt_max == 4 && a[4] == 0xffff && a[5] == 0) ||
		/* IPv4-mapped: ::ffff:a.b.c.d */
		(cnt_max == 5 && a[5] == 0xffff)))
		a4 = (uint8*) (a + 6);

	for (i = 0; i < IPV6_ADDR_LEN/2; i++) {
		if ((uint8*) (a + i) == a4) {
			snprintf(p, 16, ":%u.%u.%u.%u", a4[0], a4[1], a4[2], a4[3]);
			break;
		} else if (i == i_max) {
			*p++ = ':';
			i += cnt_max - 1;
			p[0] = ':';
			p[1] = '\0';
		} else {
			if (i)
				*p++ = ':';
			p += snprintf(p, 8, "%x", ntoh16(a[i]));
		}
	}

	return buf;
}
#ifdef BCMDRIVER

void
bcm_mdelay(uint ms)
{
	uint i;

	for (i = 0; i < ms; i++) {
		OSL_DELAY(1000);
	}
}

/*
 * Search the name=value vars for a specific one and return its value.
 * Returns NULL if not found.
 */
char *
getvar(char *vars, const char *name)
{
	NVRAM_RECLAIM_CHECK(name);
	return getvar_internal(vars, name);
}

static char *
#if defined(BCMROMBUILD) || defined(WLTEST) || defined(BCMDBG_DUMP) || \
	!defined(WLC_HIGH) || defined(ATE_BUILD)
getvar_internal(char *vars, const char *name)
#else
BCMATTACHFN(getvar_internal)(char *vars, const char *name)
#endif 
{
#ifdef	_MINOSL_
	return NULL;
#else
	char *s;
	int len;

	if (!name)
		return NULL;

	len = strlen(name);
	if (len == 0)
		return NULL;

	/* first look in vars[] */
	for (s = vars; s && *s;) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}

	/* then query nvram */
	return (nvram_get(name));
#endif	/* defined(_MINOSL_) */
}

/*
 * Search the vars for a specific one and return its value as
 * an integer. Returns 0 if not found.
 */
int
getintvar(char *vars, const char *name)
{
	NVRAM_RECLAIM_CHECK(name);
	return getintvar_internal(vars, name);
}

static int
getintvar_internal(char *vars, const char *name)
{
#ifdef	_MINOSL_
	return 0;
#else
	char *val;

	if ((val = getvar_internal(vars, name)) == NULL)
		return (0);

	return (bcm_strtoul(val, NULL, 0));
#endif	/* _MINOSL_ */
}

int
getintvararray(char *vars, const char *name, int index)
{
	NVRAM_RECLAIM_CHECK(name);
	return getintvararray_internal(vars, name, index);
}

static int
getintvararray_internal(char *vars, const char *name, int index)
{
#ifdef	_MINOSL_
	return 0;
#else
	char *buf, *endp;
	int i = 0;
	int val = 0;

	if ((buf = getvar_internal(vars, name)) == NULL) {
		return (0);
	}

	/* table values are always separated by "," or " " */
	while (*buf != '\0') {
		val = bcm_strtoul(buf, &endp, 0);
		if (i == index) {
			return val;
		}
		buf = endp;
		/* delimiter is ',' */
		if (*buf == ',')
			buf++;
		i++;
	}
	return (0);
#endif	/* _MINOSL_ */
}

int
getintvararraysize(char *vars, const char *name)
{
	NVRAM_RECLAIM_CHECK(name);
	return getintvararraysize_internal(vars, name);
}

static int
getintvararraysize_internal(char *vars, const char *name)
{
#ifdef	_MINOSL_
	return 0;
#else
	char *buf, *endp;
	int count = 0;
	int val = 0;

	if ((buf = getvar_internal(vars, name)) == NULL) {
		return (0);
	}

	/* table values are always separated by "," or " " */
	while (*buf != '\0') {
		val = bcm_strtoul(buf, &endp, 0);
		buf = endp;
		/* delimiter is ',' */
		if (*buf == ',')
			buf++;
		count++;
	}
	BCM_REFERENCE(val);
	return count;
#endif	/* _MINOSL_ */
}

/* Search for token in comma separated token-string */
static int
findmatch(const char *string, const char *name)
{
	uint len;
	char *c;

	len = strlen(name);
	while ((c = strchr(string, ',')) != NULL) {
		if (len == (uint)(c - string) && !strncmp(string, name, len))
			return 1;
		string = c + 1;
	}

	return (!strcmp(string, name));
}

/* Return gpio pin number assigned to the named pin
 *
 * Variable should be in format:
 *
 *	gpio<N>=pin_name,pin_name
 *
 * This format allows multiple features to share the gpio with mutual
 * understanding.
 *
 * 'def_pin' is returned if a specific gpio is not defined for the requested functionality
 * and if def_pin is not used by others.
 */
uint
getgpiopin(char *vars, char *pin_name, uint def_pin)
{
	char name[] = "gpioXXXX";
	char *val;
	uint pin;

	/* Go thru all possibilities till a match in pin name */
	for (pin = 0; pin < GPIO_NUMPINS; pin ++) {
		snprintf(name, sizeof(name), "gpio%d", pin);
		val = getvar(vars, name);
		if (val && findmatch(val, pin_name))
			return pin;
	}

	if (def_pin != GPIO_PIN_NOTDEFINED) {
		/* make sure the default pin is not used by someone else */
		snprintf(name, sizeof(name), "gpio%d", def_pin);
		if (getvar(vars, name)) {
			def_pin =  GPIO_PIN_NOTDEFINED;
		}
	}
	return def_pin;
}

#if defined(BCMPERFSTATS) || defined(BCMTSTAMPEDLOGS)

#if defined(__ARM_ARCH_7R__)
#define BCMLOG_CYCLE_OVERHEAD	54	/* Number of CPU cycle overhead due to bcmlog().
					 * This is to compensate CPU cycle incurred by
					 * added bcmlog() function call for profiling.
					 */
#else
#define BCMLOG_CYCLE_OVERHEAD	0
#endif

#define	LOGSIZE	256			/* should be power of 2 to avoid div below */
static struct {
	uint	cycles;
	const char	*fmt;
	uint	a1;
	uint	a2;
	uchar   indent;		/* track indent level for nice printing */
} logtab[LOGSIZE];

/* last entry logged  */
static uint logi = 0;
/* next entry to read */
static uint volatile readi = 0;
#endif	/* defined(BCMPERFSTATS) || defined(BCMTSTAMPEDLOGS) */

#ifdef BCMPERFSTATS
/* TODO: make the utility configurable (choose between icache, dcache, hits, misses ...)  */
void
bcm_perf_enable()
{
	BCMPERF_ENABLE_INSTRCOUNT();
	BCMPERF_ENABLE_ICACHE_MISS();
	BCMPERF_ENABLE_ICACHE_HIT();
}

/* WARNING:  This routine uses OSL_GETCYCLES(), which can give unexpected results on
 * modern speed stepping CPUs.  Use bcmtslog() instead in combination with TSF counter.
 */
void
bcmlog(char *fmt, uint a1, uint a2)
{
	static uint last = 0;
	uint cycles, i, elapsed;
	OSL_GETCYCLES(cycles);

	i = logi;

	elapsed = cycles - last;
	if (elapsed > BCMLOG_CYCLE_OVERHEAD)
		logtab[i].cycles = elapsed - BCMLOG_CYCLE_OVERHEAD;
	else
		logtab[i].cycles = 0;
	logtab[i].fmt = fmt;
	logtab[i].a1 = a1;
	logtab[i].a2 = a2;

	logi = (i + 1) % LOGSIZE;
	last = cycles;

	/* if log buffer is overflowing, readi should be advanced.
	 * Otherwise logi and readi will become out of sync.
	 */
	if (logi == readi) {
		readi = (readi + 1) % LOGSIZE;
	} else {
		/* This redundant else is to make CPU cycles of bcmlog() function to be uniform,
		 * so that the cycle compensation with BCMLOG_CYCLE_OVERHEAD is more accurate.
		 */
		readi = readi % LOGSIZE;
	}
}

/* Same as bcmlog but specializes the use of a1 and a2 to
 * store icache misses and instruction count.
 *  TODO : make this use a configuration array to decide what counter to read.
 * We are limited to 2 numbers but it seems it is the most we can get anyway
 * since dcache and icache cannot be enabled at the same time. Recording
 * both the hits and misses at the same time for a given cache is not that useful either.
*/

void
bcmstats(char *fmt)
{
	static uint last = 0;
	static uint32 ic_miss = 0;
	static uint32 instr_count = 0;
	uint32 ic_miss_cur;
	uint32 instr_count_cur;
	uint cycles, i;

	OSL_GETCYCLES(cycles);
	BCMPERF_GETICACHE_MISS(ic_miss_cur);
	BCMPERF_GETINSTRCOUNT(instr_count_cur);

	i = logi;

	logtab[i].cycles = cycles - last;
	logtab[i].a1 = ic_miss_cur - ic_miss;
	logtab[i].a2 = instr_count_cur - instr_count;
	logtab[i].fmt = fmt;

	logi = (i + 1) % LOGSIZE;

	last = cycles;
	instr_count = instr_count_cur;
	ic_miss = ic_miss_cur;

	/* if log buffer is overflowing, readi should be advanced.
	 * Otherwise logi and readi will become out of sync.
	 */
	if (logi == readi) {
		readi = (readi + 1) % LOGSIZE;
	} else {
		/* This redundant else is to make CPU cycles of bcmstats() function to be uniform
		 */
		readi = readi % LOGSIZE;
	}
}

/*
 * TODO (linux version): a "proc" version where the log would be dumped
 * on the proc file directly.
 */

void
bcmdumplog(char *buf, int size)
{
	char *limit;
	int j = 0;
	int num;

	limit = buf + size - 80;
	*buf = '\0';

	num = logi - readi;

	if (num < 0)
		num += LOGSIZE;

	/* print in chronological order */

	for (j = 0; j < num && (buf < limit); readi = (readi + 1) % LOGSIZE, j++) {
		if (logtab[readi].fmt == NULL)
		    continue;
		buf += snprintf(buf, (limit - buf), "%d\t", logtab[readi].cycles);
		buf += snprintf(buf, (limit - buf), logtab[readi].fmt, logtab[readi].a1,
		                logtab[readi].a2);
		buf += snprintf(buf, (limit - buf), "\n");
	}

}


/*
 * Dump one log entry at a time.
 * Return index of next entry or -1 when no more .
 */
int
bcmdumplogent(char *buf, uint i)
{
	bool hit;

	/*
	 * If buf is NULL, return the starting index,
	 * interpreting i as the indicator of last 'i' entries to dump.
	 */
	if (buf == NULL) {
		i = ((i > 0) && (i < (LOGSIZE - 1))) ? i : (LOGSIZE - 1);
		return ((logi - i) % LOGSIZE);
	}

	*buf = '\0';

	ASSERT(i < LOGSIZE);

	if (i == logi)
		return (-1);

	hit = FALSE;
	for (; (i != logi) && !hit; i = (i + 1) % LOGSIZE) {
		if (logtab[i].fmt == NULL)
			continue;
		buf += sprintf(buf, "%d: %d\t", i, logtab[i].cycles);
		buf += sprintf(buf, logtab[i].fmt, logtab[i].a1, logtab[i].a2);
		buf += sprintf(buf, "\n");
		hit = TRUE;
	}

	return (i);
}

#endif	/* BCMPERFSTATS */

#if defined(BCMTSTAMPEDLOGS)
/* Store a TSF timestamp and a log line in the log buffer */
/*
	a1 is used to signify entering/exiting a routine.  When entering
	the indent level is increased.  When exiting, the delta since entering
	is printed and the indent level is bumped back out.
	Nesting can go up to level MAX_TS_INDENTS deep.
*/
#define MAX_TS_INDENTS 20
void
bcmtslog(uint32 tstamp, const char *fmt, uint a1, uint a2)
{
	uint i = logi;
	bool use_delta = TRUE;
	static uint32 last = 0;	/* used only when use_delta is true */
	static uchar indent = 0;
	static uint32 indents[MAX_TS_INDENTS];

	logtab[i].cycles = tstamp;
	if (use_delta)
		logtab[i].cycles -= last;

	logtab[i].a2 = a2;

	if (a1 == TS_EXIT && indent) {
		indent--;
		logtab[i].a2 = tstamp - indents[indent];
	}

	logtab[i].fmt = fmt;
	logtab[i].a1 = a1;
	logtab[i].indent = indent;

	if (a1 == TS_ENTER) {
		indents[indent] = tstamp;
		if (indent < MAX_TS_INDENTS - 1)
			indent++;
	}

	if (use_delta)
		last = tstamp;
	logi = (i + 1) % LOGSIZE;
}

/* Print out a microsecond timestamp as "sec.ms.us " */
void
bcmprinttstamp(uint32 ticks)
{
	uint us, ms, sec;

	us = (ticks % TSF_TICKS_PER_MS) * 1000 / TSF_TICKS_PER_MS;
	ms = ticks / TSF_TICKS_PER_MS;
	sec = ms / 1000;
	ms -= sec * 1000;
	printf("%04u.%03u.%03u ", sec, ms, us);
}

/* Print out the log buffer with timestamps */
void
bcmprinttslogs(void)
{
	int j = 0;
	int num;

	num = logi - readi;
	if (num < 0)
		num += LOGSIZE;

	/* Format and print the log entries directly in chronological order */
	for (j = 0; j < num; readi = (readi + 1) % LOGSIZE, j++) {
		if (logtab[readi].fmt == NULL)
		    continue;
		bcmprinttstamp(logtab[readi].cycles);
		printf(logtab[readi].fmt, logtab[readi].a1, logtab[readi].a2);
		printf("\n");
	}
}

/*
	Identical to bcmdumplog, but output is based on tsf instead of cycles.

	a1 is used to signify entering/exiting a routine.  When entering
	the indent level is increased.  When exiting, the delta since entering
	is printed and the indent level is bumped back out.
*/
void
bcmdumptslog(char *buf, int size)
{
	char *limit;
	int j = 0;
	int num;
	uint us, ms, sec;
	int skip;
	char *lines = "| | | | | | | | | | | | | | | | | | | |";

	limit = buf + size - 80;
	*buf = '\0';

	num = logi - readi;

	if (num < 0)
		num += LOGSIZE;

	/* print in chronological order */
	for (j = 0; j < num && (buf < limit); readi = (readi + 1) % LOGSIZE, j++) {
		char *last_buf = buf;
		if (logtab[readi].fmt == NULL)
			continue;

		us = (logtab[readi].cycles % TSF_TICKS_PER_MS) * 1000 / TSF_TICKS_PER_MS;
		ms = logtab[readi].cycles / TSF_TICKS_PER_MS;
		sec = ms / 1000;
		ms -= sec * 1000;

		buf += snprintf(buf, (limit - buf), "%04u.%03u.%03u ", sec, ms, us);

		/* 2 spaces for each indent level */
		buf += snprintf(buf, (limit - buf), "%.*s", logtab[readi].indent * 2, lines);

		buf += snprintf(buf, (limit - buf), logtab[readi].fmt);

		/* If a1 is ENTER or EXIT, print the + or - */
		skip = 0;
		if (logtab[readi].a1 == TS_ENTER) {
			buf += snprintf(buf, (limit - buf), " +");
			skip++;
		}
		if (logtab[readi].a1 == TS_EXIT) {
			buf += snprintf(buf, (limit - buf), " -");
			skip++;
		}

		/* else print the real a1 */
		if (logtab[readi].a1 && !skip)
			buf += snprintf(buf, (limit - buf), "   %d", logtab[readi].a1);

		/*
		   If exiting routine, print a nicely formatted delta since entering.
		   Otherwise, just print a2 normally.
		*/
		if (logtab[readi].a2) {
			if (logtab[readi].a1 == TS_EXIT) {
				int num_space = 75 - (buf - last_buf);
				buf += snprintf(buf, (limit - buf), "%*.s", num_space, "");
				buf += snprintf(buf, (limit - buf), "%5d usecs", logtab[readi].a2);
			} else
				buf += snprintf(buf, (limit - buf), "  %d", logtab[readi].a2);
		}
		buf += snprintf(buf, (limit - buf), "\n");
		last_buf = buf;
	}
}

#endif	/* BCMTSTAMPEDLOGS */

#if defined(BCMDBG) || defined(DHD_DEBUG)
/* pretty hex print a pkt buffer chain */
void
prpkt(const char *msg, osl_t *osh, void *p0)
{
	void *p;

	if (msg && (msg[0] != '\0'))
		printf("%s:\n", msg);

	for (p = p0; p; p = PKTNEXT(osh, p))
		prhex(NULL, PKTDATA(osh, p), PKTLEN(osh, p));
}
#endif	/* BCMDBG || DHD_DEBUG */

/* Takes an Ethernet frame and sets out-of-bound PKTPRIO.
 * Also updates the inplace vlan tag if requested.
 * For debugging, it returns an indication of what it did.
 */
uint BCMFASTPATH
pktsetprio(void *pkt, bool update_vtag)
{
	struct ether_header *eh;
	struct ethervlan_header *evh;
	uint8 *pktdata;
	int priority = 0;
	int rc = 0;

	pktdata = (uint8 *)PKTDATA(OSH_NULL, pkt);
	ASSERT(ISALIGNED((uintptr)pktdata, sizeof(uint16)));

	eh = (struct ether_header *) pktdata;

	if (eh->ether_type == hton16(ETHER_TYPE_8021Q)) {
		uint16 vlan_tag;
		int vlan_prio, dscp_prio = 0;

		evh = (struct ethervlan_header *)eh;

		vlan_tag = ntoh16(evh->vlan_tag);
		vlan_prio = (int) (vlan_tag >> VLAN_PRI_SHIFT) & VLAN_PRI_MASK;

		if ((evh->ether_type == hton16(ETHER_TYPE_IP)) ||
			(evh->ether_type == hton16(ETHER_TYPE_IPV6))) {
			uint8 *ip_body = pktdata + sizeof(struct ethervlan_header);
			uint8 tos_tc = IP_TOS46(ip_body);
			dscp_prio = (int)(tos_tc >> IPV4_TOS_PREC_SHIFT);
		}

		/* DSCP priority gets precedence over 802.1P (vlan tag) */
		if (dscp_prio != 0) {
			priority = dscp_prio;
			rc |= PKTPRIO_VDSCP;
		} else {
			priority = vlan_prio;
			rc |= PKTPRIO_VLAN;
		}
		/*
		 * If the DSCP priority is not the same as the VLAN priority,
		 * then overwrite the priority field in the vlan tag, with the
		 * DSCP priority value. This is required for Linux APs because
		 * the VLAN driver on Linux, overwrites the skb->priority field
		 * with the priority value in the vlan tag
		 */
		if (update_vtag && (priority != vlan_prio)) {
			vlan_tag &= ~(VLAN_PRI_MASK << VLAN_PRI_SHIFT);
			vlan_tag |= (uint16)priority << VLAN_PRI_SHIFT;
			evh->vlan_tag = hton16(vlan_tag);
			rc |= PKTPRIO_UPD;
		}
	} else if ((eh->ether_type == hton16(ETHER_TYPE_IP)) ||
		(eh->ether_type == hton16(ETHER_TYPE_IPV6))) {
		uint8 *ip_body = pktdata + sizeof(struct ether_header);
		uint8 tos_tc = IP_TOS46(ip_body);
		uint8 dscp = tos_tc >> IPV4_TOS_DSCP_SHIFT;
		switch (dscp) {
		case DSCP_EF:
			priority = PRIO_8021D_VO;
			break;
		case DSCP_AF31:
		case DSCP_AF32:
		case DSCP_AF33:
			priority = PRIO_8021D_CL;
			break;
		case DSCP_AF21:
		case DSCP_AF22:
		case DSCP_AF23:
		case DSCP_AF11:
		case DSCP_AF12:
		case DSCP_AF13:
			priority = PRIO_8021D_EE;
			break;
		default:
#ifndef CUSTOM_DSCP_TO_PRIO_MAPPING
			priority = (int)(tos_tc >> IPV4_TOS_PREC_SHIFT);
#else
			priority = (int)dscp2priomap[((tos_tc >> IPV4_TOS_DSCP_SHIFT)
				& CUST_IPV4_TOS_PREC_MASK)];
#endif
			break;
		}

		rc |= PKTPRIO_DSCP;
	}

	ASSERT(priority >= 0 && priority <= MAXPRIO);
	PKTSETPRIO(pkt, priority);
	return (rc | priority);
}

/* Returns TRUE and DSCP if IP header found, FALSE otherwise.
 */
bool BCMFASTPATH
pktgetdscp(uint8 *pktdata, uint pktlen, uint8 *dscp)
{
	struct ether_header *eh;
	struct ethervlan_header *evh;
	uint8 *ip_body;
	bool rc = FALSE;

	/* minimum length is ether header and IP header */
	if (pktlen < sizeof(struct ether_header) + IPV4_MIN_HEADER_LEN)
		return FALSE;

	eh = (struct ether_header *) pktdata;

	if (eh->ether_type == HTON16(ETHER_TYPE_IP)) {
		ip_body = pktdata + sizeof(struct ether_header);
		*dscp = IP_DSCP46(ip_body);
		rc = TRUE;
	}
	else if (eh->ether_type == HTON16(ETHER_TYPE_8021Q)) {
		evh = (struct ethervlan_header *)eh;

		/* minimum length is ethervlan header and IP header */
		if (pktlen >= sizeof(struct ethervlan_header) + IPV4_MIN_HEADER_LEN &&
			evh->ether_type == HTON16(ETHER_TYPE_IP)) {
			ip_body = pktdata + sizeof(struct ethervlan_header);
			*dscp = IP_DSCP46(ip_body);
			rc = TRUE;
		}
	}

	return rc;
}

#ifndef BCM_BOOTLOADER
/* The 0.5KB string table is not removed by compiler even though it's unused */

static char bcm_undeferrstr[32];
static const char *bcmerrorstrtable[] = BCMERRSTRINGTABLE;

/* Convert the error codes into related error strings  */
const char *
bcmerrorstr(int bcmerror)
{
	/* check if someone added a bcmerror code but forgot to add errorstring */
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(bcmerrorstrtable) - 1));

	if (bcmerror > 0 || bcmerror < BCME_LAST) {
		snprintf(bcm_undeferrstr, sizeof(bcm_undeferrstr), "Undefined error %d", bcmerror);
		return bcm_undeferrstr;
	}

	ASSERT(strlen(bcmerrorstrtable[-bcmerror]) < BCME_STRLEN);

	return bcmerrorstrtable[-bcmerror];
}

#endif /* !BCM_BOOTLOADER */

#ifdef WLC_LOW
static void
BCMATTACHFN(bcm_nvram_refresh)(char *flash)
{
	int i;
	int ret = 0;

	ASSERT(flash != NULL);

	/* default "empty" vars cache */
	bzero(flash, 2);

	if ((ret = nvram_getall(flash, MAX_NVRAM_SPACE)))
		return;

	/* determine nvram length */
	for (i = 0; i < MAX_NVRAM_SPACE; i++) {
		if (flash[i] == '\0' && flash[i+1] == '\0')
			break;
	}

	if (i > 1)
		vars_len = i + 2;
	else
		vars_len = 0;
}

char *
BCMATTACHFN(bcm_nvram_vars)(uint *length)
{
#ifndef BCMNVRAMR
	/* cache may be stale if nvram is read/write */
	if (nvram_vars) {
		ASSERT(!bcmreclaimed);
		bcm_nvram_refresh(nvram_vars);
	}
#endif
	if (length)
		*length = vars_len;
	return nvram_vars;
}

/* copy nvram vars into locally-allocated multi-string array */
int
BCMATTACHFN(bcm_nvram_cache)(void *sih)
{
	int ret = 0;
	void *osh;
	char *flash = NULL;

	if (vars_len >= 0) {
#ifndef BCMNVRAMR
		bcm_nvram_refresh(nvram_vars);
#endif
		return 0;
	}

	osh = si_osh((si_t *)sih);

	/* allocate memory and read in flash */
	if (!(flash = MALLOC(osh, MAX_NVRAM_SPACE))) {
		ret = BCME_NOMEM;
		goto exit;
	}

	bcm_nvram_refresh(flash);

#ifdef BCMNVRAMR
	if (vars_len > 3) {
		/* copy into a properly-sized buffer */
		if (!(nvram_vars = MALLOC(osh, vars_len))) {
			ret = BCME_NOMEM;
		} else
			bcopy(flash, nvram_vars, vars_len);
	}
	MFREE(osh, flash, MAX_NVRAM_SPACE);
#else
	/* cache must be full size of nvram if read/write */
	nvram_vars = flash;
#endif	/* BCMNVRAMR */

exit:
	return ret;
}
#endif /* WLC_LOW */

#ifdef BCMDBG_PKT   /* pkt logging for debugging */
/* Add a packet to the pktlist */
static void
_pktlist_add(pktlist_info_t *pktlist, void *pkt, int line, char *file)
{
	uint16 i;
	char *basename;
#ifdef BCMDBG_PTRACE
	uint16 *idx = PKTLIST_IDX(pkt);
#endif /* BCMDBG_PTRACE */

	ASSERT(pktlist->count < PKTLIST_SIZE);

	/* Verify the packet is not already part of the list */
	for (i = 0; i < pktlist->count; i++) {
		if (pktlist->list[i].pkt == pkt)
			ASSERT(0);
	}
	pktlist->list[pktlist->count].pkt = pkt;
	pktlist->list[pktlist->count].line = line;

	basename = strrchr(file, '/');
	if (basename)
		basename++;
	else
		basename = file;
	pktlist->list[pktlist->count].file = basename;
#ifdef BCMDBG_PTRACE
	*idx = pktlist->count;
	bzero(pktlist->list[pktlist->count].pkt_trace, PKTTRACE_MAX_BYTES);
#endif /* BCMDBG_PTRACE */
	pktlist->count++;

	return;
}

void
pktlist_add(pktlist_info_t *pktlist, void *pkt, int line, char *file)
{
	void *p;
	for (p = pkt; p != NULL; p = PKTCLINK(p))
		_pktlist_add(pktlist, p, line, file);
}

/* Remove a packet from the pktlist */
static void
_pktlist_remove(pktlist_info_t *pktlist, void *pkt)
{
	uint16 i;
	uint16 num = pktlist->count;
#ifdef BCMDBG_PTRACE
	uint16 *idx = PKTLIST_IDX(pkt);

	ASSERT((*idx) < pktlist->count);
#endif /* BCMDBG_PTRACE */

	/* find the index where pkt exists */
	for (i = 0; i < num; i++) {
		/* check for the existence of pkt in the list */
		if (pktlist->list[i].pkt == pkt) {
#ifdef BCMDBG_PTRACE
			ASSERT((*idx) == i);
#endif /* BCMDBG_PTRACE */
			/* replace with the last element */
			pktlist->list[i].pkt = pktlist->list[num-1].pkt;
			pktlist->list[i].line = pktlist->list[num-1].line;
			pktlist->list[i].file = pktlist->list[num-1].file;
#ifdef BCMDBG_PTRACE
			memcpy(pktlist->list[i].pkt_trace, pktlist->list[num-1].pkt_trace,
				PKTTRACE_MAX_BYTES);
			idx = PKTLIST_IDX(pktlist->list[i].pkt);
			*idx = i;
#endif /* BCMDBG_PTRACE */
			pktlist->count--;
			return;
		}
	}
	ASSERT(0);
}

void
pktlist_remove(pktlist_info_t *pktlist, void *pkt)
{
	void *p;
	for (p = pkt; p != NULL; p = PKTCLINK(p))
		_pktlist_remove(pktlist, p);
}

#ifdef BCMDBG_PTRACE
static void
_pktlist_trace(pktlist_info_t *pktlist, void *pkt, uint16 bit)
{
	uint16 *idx = PKTLIST_IDX(pkt);

	ASSERT(((*idx) < pktlist->count) && (bit < PKTTRACE_MAX_BITS));
	ASSERT(pktlist->list[(*idx)].pkt == pkt);

	pktlist->list[(*idx)].pkt_trace[bit/NBBY] |= (1 << ((bit)%NBBY));

}
void
pktlist_trace(pktlist_info_t *pktlist, void *pkt, uint16 bit)
{
	void *p;
	for (p = pkt; p != NULL; p = PKTCLINK(p))
		_pktlist_trace(pktlist, p, bit);
}
#endif /* BCMDBG_PTRACE */

/* Dump the pktlist (and the contents of each packet if 'data'
 * is set). 'buf' should be large enough
 */

char *
pktlist_dump(pktlist_info_t *pktlist, char *buf)
{
	char *obuf = buf;
	uint16 i;

	if (buf != NULL)
		buf += sprintf(buf, "Packet list dump:\n");
	else
		printf("Packet list dump:\n");

	for (i = 0; i < (pktlist->count); i++) {
		if (buf != NULL)
			buf += sprintf(buf, "Pkt_addr: 0x%p Line: %d File: %s\t",
				pktlist->list[i].pkt, pktlist->list[i].line,
				pktlist->list[i].file);
		else
			printf("Pkt_addr: 0x%p Line: %d File: %s\t", pktlist->list[i].pkt,
				pktlist->list[i].line, pktlist->list[i].file);

/* #ifdef NOTDEF  Remove this ifdef to print pkttag and pktdata */
		if (buf != NULL) {
			if (PKTTAG(pktlist->list[i].pkt)) {
				/* Print pkttag */
				buf += sprintf(buf, "Pkttag(in hex): ");
				buf += bcm_format_hex(buf, PKTTAG(pktlist->list[i].pkt),
					OSL_PKTTAG_SZ);
			}
			buf += sprintf(buf, "Pktdata(in hex): ");
			buf += bcm_format_hex(buf, PKTDATA(OSH_NULL, pktlist->list[i].pkt),
			                      PKTLEN(OSH_NULL, pktlist->list[i].pkt));
		} else {
			void *pkt = pktlist->list[i].pkt, *npkt;

			printf("Pkt[%d] Dump:\n", i);
			while (pkt) {
				int hroom, pktlen;
				uchar *src;
#ifdef BCMDBG_PTRACE
				uint16 *idx = PKTLIST_IDX(pkt);

				ASSERT((*idx) < pktlist->count);
				prhex("Pkt Trace (in hex):", pktlist->list[(*idx)].pkt_trace,
					PKTTRACE_MAX_BYTES);
#endif /* BCMDBG_PTRACE */
				npkt = (void *)PKTNEXT(OSH_NULL, pkt);
				PKTSETNEXT(OSH_NULL, pkt, NULL);

				src = (uchar *)(PKTTAG(pkt));
				pktlen = PKTLEN(OSH_NULL, pkt);
				hroom = PKTHEADROOM(OSH_NULL, pkt);

				printf("Pkttag_addr: %p\n", src);
				if (src)
					prhex("Pkttag(in hex): ", src, OSL_PKTTAG_SZ);
				src = (uchar *) (PKTDATA(OSH_NULL, pkt));
				printf("Pkthead_addr: %p len: %d\n", src - hroom, hroom);
				prhex("Pkt headroom content(in hex): ", src - hroom, hroom);
				printf("Pktdata_addr: %p len: %d\n", src, pktlen);
				prhex("Pktdata(in hex): ", src, pktlen);

				pkt = npkt;
			}
		}
/* #endif  NOTDEF */

		if (buf != NULL)
			buf += sprintf(buf, "\n");
		else
			printf("\n");
	}
	return obuf;
}
#endif  /* BCMDBG_PKT */

/* iovar table lookup */
/* could mandate sorted tables and do a binary search */
const bcm_iovar_t*
bcm_iovar_lookup(const bcm_iovar_t *table, const char *name)
{
	const bcm_iovar_t *vi;
	const char *lookup_name;

	/* skip any ':' delimited option prefixes */
	lookup_name = strrchr(name, ':');
	if (lookup_name != NULL)
		lookup_name++;
	else
		lookup_name = name;

	ASSERT(table != NULL);

	for (vi = table; vi->name; vi++) {
		if (!strcmp(vi->name, lookup_name))
			return vi;
	}
	/* ran to end of table */

	return NULL; /* var name not found */
}

int
bcm_iovar_lencheck(const bcm_iovar_t *vi, void *arg, int len, bool set)
{
	int bcmerror = 0;

	/* length check on io buf */
	switch (vi->type) {
	case IOVT_BOOL:
	case IOVT_INT8:
	case IOVT_INT16:
	case IOVT_INT32:
	case IOVT_UINT8:
	case IOVT_UINT16:
	case IOVT_UINT32:
		/* all integers are int32 sized args at the ioctl interface */
		if (len < (int)sizeof(int)) {
			bcmerror = BCME_BUFTOOSHORT;
		}
		break;

	case IOVT_BUFFER:
		/* buffer must meet minimum length requirement */
		if (len < vi->minlen) {
			bcmerror = BCME_BUFTOOSHORT;
		}
		break;

	case IOVT_VOID:
		if (!set) {
			/* Cannot return nil... */
			bcmerror = BCME_UNSUPPORTED;
		} else if (len) {
			/* Set is an action w/o parameters */
			bcmerror = BCME_BUFTOOLONG;
		}
		break;

	default:
		/* unknown type for length check in iovar info */
		ASSERT(0);
		bcmerror = BCME_UNSUPPORTED;
	}

	return bcmerror;
}

#endif	/* BCMDRIVER */


uint8 *
bcm_write_tlv(int type, const void *data, int datalen, uint8 *dst)
{
	uint8 *new_dst = dst;
	bcm_tlv_t *dst_tlv = (bcm_tlv_t *)dst;

	/* dst buffer should always be valid */
	ASSERT(dst);

	/* data len must be within valid range */
	ASSERT((datalen >= 0) && (datalen <= BCM_TLV_MAX_DATA_SIZE));

	/* source data buffer pointer should be valid, unless datalen is 0
	 * meaning no data with this TLV
	 */
	ASSERT((data != NULL) || (datalen == 0));

	/* only do work if the inputs are valid
	 * - must have a dst to write to AND
	 * - datalen must be within range AND
	 * - the source data pointer must be non-NULL if datalen is non-zero
	 * (this last condition detects datalen > 0 with a NULL data pointer)
	 */
	if ((dst != NULL) &&
	    ((datalen >= 0) && (datalen <= BCM_TLV_MAX_DATA_SIZE)) &&
	    ((data != NULL) || (datalen == 0))) {

	        /* write type, len fields */
		dst_tlv->id = (uint8)type;
	        dst_tlv->len = (uint8)datalen;

		/* if data is present, copy to the output buffer and update
		 * pointer to output buffer
		 */
		if (datalen > 0) {

			memcpy(dst_tlv->data, data, datalen);
		}

		/* update the output destination poitner to point past
		 * the TLV written
		 */
		new_dst = dst + BCM_TLV_HDR_SIZE + datalen;
	}

	return (new_dst);
}

uint8 *
bcm_write_tlv_safe(int type, const void *data, int datalen, uint8 *dst, int dst_maxlen)
{
	uint8 *new_dst = dst;

	if ((datalen >= 0) && (datalen <= BCM_TLV_MAX_DATA_SIZE)) {

		/* if len + tlv hdr len is more than destlen, don't do anything
		 * just return the buffer untouched
		 */
		if ((int)(datalen + BCM_TLV_HDR_SIZE) <= dst_maxlen) {

			new_dst = bcm_write_tlv(type, data, datalen, dst);
		}
	}

	return (new_dst);
}

uint8 *
bcm_copy_tlv(const void *src, uint8 *dst)
{
	uint8 *new_dst = dst;
	const bcm_tlv_t *src_tlv = (const bcm_tlv_t *)src;
	uint totlen;

	ASSERT(dst && src);
	if (dst && src) {

		totlen = BCM_TLV_HDR_SIZE + src_tlv->len;
		memcpy(dst, src_tlv, totlen);
		new_dst = dst + totlen;
	}

	return (new_dst);
}


uint8 *bcm_copy_tlv_safe(const void *src, uint8 *dst, int dst_maxlen)
{
	uint8 *new_dst = dst;
	const bcm_tlv_t *src_tlv = (const bcm_tlv_t *)src;

	ASSERT(src);
	if (src) {
		if (bcm_valid_tlv(src_tlv, dst_maxlen)) {
			new_dst = bcm_copy_tlv(src, dst);
		}
	}

	return (new_dst);
}


#if !defined(BCMROMOFFLOAD_EXCLUDE_BCMUTILS_FUNCS)
/*******************************************************************************
 * crc8
 *
 * Computes a crc8 over the input data using the polynomial:
 *
 *       x^8 + x^7 +x^6 + x^4 + x^2 + 1
 *
 * The caller provides the initial value (either CRC8_INIT_VALUE
 * or the previous returned value) to allow for processing of
 * discontiguous blocks of data.  When generating the CRC the
 * caller is responsible for complementing the final return value
 * and inserting it into the byte stream.  When checking, a final
 * return value of CRC8_GOOD_VALUE indicates a valid CRC.
 *
 * Reference: Dallas Semiconductor Application Note 27
 *   Williams, Ross N., "A Painless Guide to CRC Error Detection Algorithms",
 *     ver 3, Aug 1993, ross@guest.adelaide.edu.au, Rocksoft Pty Ltd.,
 *     ftp://ftp.rocksoft.com/clients/rocksoft/papers/crc_v3.txt
 *
 * ****************************************************************************
 */

static const uint8 crc8_table[256] = {
    0x00, 0xF7, 0xB9, 0x4E, 0x25, 0xD2, 0x9C, 0x6B,
    0x4A, 0xBD, 0xF3, 0x04, 0x6F, 0x98, 0xD6, 0x21,
    0x94, 0x63, 0x2D, 0xDA, 0xB1, 0x46, 0x08, 0xFF,
    0xDE, 0x29, 0x67, 0x90, 0xFB, 0x0C, 0x42, 0xB5,
    0x7F, 0x88, 0xC6, 0x31, 0x5A, 0xAD, 0xE3, 0x14,
    0x35, 0xC2, 0x8C, 0x7B, 0x10, 0xE7, 0xA9, 0x5E,
    0xEB, 0x1C, 0x52, 0xA5, 0xCE, 0x39, 0x77, 0x80,
    0xA1, 0x56, 0x18, 0xEF, 0x84, 0x73, 0x3D, 0xCA,
    0xFE, 0x09, 0x47, 0xB0, 0xDB, 0x2C, 0x62, 0x95,
    0xB4, 0x43, 0x0D, 0xFA, 0x91, 0x66, 0x28, 0xDF,
    0x6A, 0x9D, 0xD3, 0x24, 0x4F, 0xB8, 0xF6, 0x01,
    0x20, 0xD7, 0x99, 0x6E, 0x05, 0xF2, 0xBC, 0x4B,
    0x81, 0x76, 0x38, 0xCF, 0xA4, 0x53, 0x1D, 0xEA,
    0xCB, 0x3C, 0x72, 0x85, 0xEE, 0x19, 0x57, 0xA0,
    0x15, 0xE2, 0xAC, 0x5B, 0x30, 0xC7, 0x89, 0x7E,
    0x5F, 0xA8, 0xE6, 0x11, 0x7A, 0x8D, 0xC3, 0x34,
    0xAB, 0x5C, 0x12, 0xE5, 0x8E, 0x79, 0x37, 0xC0,
    0xE1, 0x16, 0x58, 0xAF, 0xC4, 0x33, 0x7D, 0x8A,
    0x3F, 0xC8, 0x86, 0x71, 0x1A, 0xED, 0xA3, 0x54,
    0x75, 0x82, 0xCC, 0x3B, 0x50, 0xA7, 0xE9, 0x1E,
    0xD4, 0x23, 0x6D, 0x9A, 0xF1, 0x06, 0x48, 0xBF,
    0x9E, 0x69, 0x27, 0xD0, 0xBB, 0x4C, 0x02, 0xF5,
    0x40, 0xB7, 0xF9, 0x0E, 0x65, 0x92, 0xDC, 0x2B,
    0x0A, 0xFD, 0xB3, 0x44, 0x2F, 0xD8, 0x96, 0x61,
    0x55, 0xA2, 0xEC, 0x1B, 0x70, 0x87, 0xC9, 0x3E,
    0x1F, 0xE8, 0xA6, 0x51, 0x3A, 0xCD, 0x83, 0x74,
    0xC1, 0x36, 0x78, 0x8F, 0xE4, 0x13, 0x5D, 0xAA,
    0x8B, 0x7C, 0x32, 0xC5, 0xAE, 0x59, 0x17, 0xE0,
    0x2A, 0xDD, 0x93, 0x64, 0x0F, 0xF8, 0xB6, 0x41,
    0x60, 0x97, 0xD9, 0x2E, 0x45, 0xB2, 0xFC, 0x0B,
    0xBE, 0x49, 0x07, 0xF0, 0x9B, 0x6C, 0x22, 0xD5,
    0xF4, 0x03, 0x4D, 0xBA, 0xD1, 0x26, 0x68, 0x9F
};

#define CRC_INNER_LOOP(n, c, x) \
	(c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]

uint8
hndcrc8(
	uint8 *pdata,	/* pointer to array of data to process */
	uint  nbytes,	/* number of input data bytes to process */
	uint8 crc	/* either CRC8_INIT_VALUE or previous return value */
)
{
	/* hard code the crc loop instead of using CRC_INNER_LOOP macro
	 * to avoid the undefined and unnecessary (uint8 >> 8) operation.
	 */
	while (nbytes-- > 0)
		crc = crc8_table[(crc ^ *pdata++) & 0xff];

	return crc;
}

/*******************************************************************************
 * crc16
 *
 * Computes a crc16 over the input data using the polynomial:
 *
 *       x^16 + x^12 +x^5 + 1
 *
 * The caller provides the initial value (either CRC16_INIT_VALUE
 * or the previous returned value) to allow for processing of
 * discontiguous blocks of data.  When generating the CRC the
 * caller is responsible for complementing the final return value
 * and inserting it into the byte stream.  When checking, a final
 * return value of CRC16_GOOD_VALUE indicates a valid CRC.
 *
 * Reference: Dallas Semiconductor Application Note 27
 *   Williams, Ross N., "A Painless Guide to CRC Error Detection Algorithms",
 *     ver 3, Aug 1993, ross@guest.adelaide.edu.au, Rocksoft Pty Ltd.,
 *     ftp://ftp.rocksoft.com/clients/rocksoft/papers/crc_v3.txt
 *
 * ****************************************************************************
 */

static const uint16 crc16_table[256] = {
    0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
    0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
    0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
    0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
    0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
    0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
    0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
    0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
    0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
    0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
    0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
    0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
    0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
    0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
    0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
    0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
    0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
    0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
    0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
    0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
    0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
    0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
    0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
    0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
    0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
    0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
    0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
    0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
    0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
    0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

uint16
hndcrc16(
    uint8 *pdata,  /* pointer to array of data to process */
    uint nbytes, /* number of input data bytes to process */
    uint16 crc     /* either CRC16_INIT_VALUE or previous return value */
)
{
	while (nbytes-- > 0)
		CRC_INNER_LOOP(16, crc, *pdata++);
	return crc;
}

static const uint32 crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/*
 * crc input is CRC32_INIT_VALUE for a fresh start, or previous return value if
 * accumulating over multiple pieces.
 */
uint32
hndcrc32(uint8 *pdata, uint nbytes, uint32 crc)
{
	uint8 *pend;
#ifdef __mips__
	uint8 tmp[4];
	ulong *tptr = (ulong *)tmp;

	if (nbytes > 3) {
		/* in case the beginning of the buffer isn't aligned */
		pend = (uint8 *)((uint)(pdata + 3) & ~0x3);
		nbytes -= (pend - pdata);
		while (pdata < pend)
			CRC_INNER_LOOP(32, crc, *pdata++);
	}

	if (nbytes > 3) {
		/* handle bulk of data as 32-bit words */
		pend = pdata + (nbytes & ~0x3);
		while (pdata < pend) {
			*tptr = *(ulong *)pdata;
			pdata += sizeof(ulong *);
			CRC_INNER_LOOP(32, crc, tmp[0]);
			CRC_INNER_LOOP(32, crc, tmp[1]);
			CRC_INNER_LOOP(32, crc, tmp[2]);
			CRC_INNER_LOOP(32, crc, tmp[3]);
		}
	}

	/* 1-3 bytes at end of buffer */
	pend = pdata + (nbytes & 0x03);
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);
#else
	pend = pdata + nbytes;
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);
#endif /* __mips__ */

	return crc;
}

#ifdef notdef
#define CLEN 	1499 	/*  CRC Length */
#define CBUFSIZ 	(CLEN+4)
#define CNBUFS		5 /* # of bufs */

void
testcrc32(void)
{
	uint j, k, l;
	uint8 *buf;
	uint len[CNBUFS];
	uint32 crcr;
	uint32 crc32tv[CNBUFS] =
		{0xd2cb1faa, 0xd385c8fa, 0xf5b4f3f3, 0x55789e20, 0x00343110};

	ASSERT((buf = MALLOC(CBUFSIZ*CNBUFS)) != NULL);

	/* step through all possible alignments */
	for (l = 0; l <= 4; l++) {
		for (j = 0; j < CNBUFS; j++) {
			len[j] = CLEN;
			for (k = 0; k < len[j]; k++)
				*(buf + j*CBUFSIZ + (k+l)) = (j+k) & 0xff;
		}

		for (j = 0; j < CNBUFS; j++) {
			crcr = crc32(buf + j*CBUFSIZ + l, len[j], CRC32_INIT_VALUE);
			ASSERT(crcr == crc32tv[j]);
		}
	}

	MFREE(buf, CBUFSIZ*CNBUFS);
	return;
}
#endif /* notdef */

/*
 * Advance from the current 1-byte tag/1-byte length/variable-length value
 * triple, to the next, returning a pointer to the next.
 * If the current or next TLV is invalid (does not fit in given buffer length),
 * NULL is returned.
 * *buflen is not modified if the TLV elt parameter is invalid, or is decremented
 * by the TLV parameter's length if it is valid.
 */
bcm_tlv_t *
bcm_next_tlv(bcm_tlv_t *elt, int *buflen)
{
	int len;

	/* validate current elt */
	if (!bcm_valid_tlv(elt, *buflen)) {
		return NULL;
	}

	/* advance to next elt */
	len = elt->len;
	elt = (bcm_tlv_t*)(elt->data + len);
	*buflen -= (TLV_HDR_LEN + len);

	/* validate next elt */
	if (!bcm_valid_tlv(elt, *buflen)) {
		return NULL;
	}

	return elt;
}

/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 */
bcm_tlv_t *
bcm_parse_tlvs(void *buf, int buflen, uint key)
{
	bcm_tlv_t *elt;
	int totlen;

	elt = (bcm_tlv_t*)buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= TLV_HDR_LEN) {
		int len = elt->len;

		/* validate remaining totlen */
		if ((elt->id == key) && (totlen >= (int)(len + TLV_HDR_LEN))) {

			return (elt);
		}

		elt = (bcm_tlv_t*)((uint8*)elt + (len + TLV_HDR_LEN));
		totlen -= (len + TLV_HDR_LEN);
	}

	return NULL;
}

/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 * return NULL if not found or length field < min_varlen
 */
bcm_tlv_t *
bcm_parse_tlvs_min_bodylen(void *buf, int buflen, uint key, int min_bodylen)
{
	bcm_tlv_t * ret = bcm_parse_tlvs(buf, buflen, key);
	if (ret == NULL || ret->len < min_bodylen) {
		return NULL;
	}
	return ret;
}

/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag.  Stop parsing when we see an element whose ID is greater
 * than the target key.
 */
bcm_tlv_t *
bcm_parse_ordered_tlvs(void *buf, int buflen, uint key)
{
	bcm_tlv_t *elt;
	int totlen;

	elt = (bcm_tlv_t*)buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= TLV_HDR_LEN) {
		uint id = elt->id;
		int len = elt->len;

		/* Punt if we start seeing IDs > than target key */
		if (id > key) {
			return (NULL);
		}

		/* validate remaining totlen */
		if ((id == key) && (totlen >= (int)(len + TLV_HDR_LEN))) {
			return (elt);
		}

		elt = (bcm_tlv_t*)((uint8*)elt + (len + TLV_HDR_LEN));
		totlen -= (len + TLV_HDR_LEN);
	}
	return NULL;
}
#endif	/* !BCMROMOFFLOAD_EXCLUDE_BCMUTILS_FUNCS */

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_PRHDRS) || \
	defined(WLMSG_PRPKT) || defined(WLMSG_ASSOC) || defined(BCMDBG_DUMP) || \
	defined(DHD_DEBUG)
int
bcm_format_field(const bcm_bit_desc_ex_t *bd, uint32 flags, char* buf, int len)
{
	int i, slen = 0;
	uint32 bit, mask;
	const char *name;
	mask = bd->mask;
	if (len < 2 || !buf)
		return 0;

	buf[0] = '\0';

	for (i = 0;  (name = bd->bitfield[i].name) != NULL; i++) {
		bit = bd->bitfield[i].bit;
		if ((flags & mask) == bit) {
			if (len > (int)strlen(name)) {
				slen = strlen(name);
				strncpy(buf, name, slen+1);
			}
			break;
		}
	}
	return slen;
}

int
bcm_format_flags(const bcm_bit_desc_t *bd, uint32 flags, char* buf, int len)
{
	int i;
	char* p = buf;
	char hexstr[16];
	int slen = 0, nlen = 0;
	uint32 bit;
	const char* name;

	if (len < 2 || !buf)
		return 0;

	buf[0] = '\0';

	for (i = 0; flags != 0; i++) {
		bit = bd[i].bit;
		name = bd[i].name;
		if (bit == 0 && flags != 0) {
			/* print any unnamed bits */
			snprintf(hexstr, 16, "0x%X", flags);
			name = hexstr;
			flags = 0;	/* exit loop */
		} else if ((flags & bit) == 0)
			continue;
		flags &= ~bit;
		nlen = strlen(name);
		slen += nlen;
		/* count btwn flag space */
		if (flags != 0)
			slen += 1;
		/* need NULL char as well */
		if (len <= slen)
			break;
		/* copy NULL char but don't count it */
		strncpy(p, name, nlen + 1);
		p += nlen;
		/* copy btwn flag space and NULL char */
		if (flags != 0)
			p += snprintf(p, 2, " ");
	}

	/* indicate the str was too short */
	if (flags != 0) {
		if (len < 2)
			p -= 2 - len;	/* overwrite last char */
		p += snprintf(p, 2, ">");
	}

	return (int)(p - buf);
}
#endif /* BCMDBG || WLMSG_PRHDRS || WLMSG_PRPKT || WLMSG_ASSOC || BCMDBG_DUMP || DHD_DEBUG */

/* print bytes formatted as hex to a string. return the resulting string length */
int
bcm_format_hex(char *str, const void *bytes, int len)
{
	int i;
	char *p = str;
	const uint8 *src = (const uint8*)bytes;

	for (i = 0; i < len; i++) {
		p += snprintf(p, 3, "%02X", *src);
		src++;
	}
	return (int)(p - str);
}

/* pretty hex print a contiguous buffer */
void
prhex(const char *msg, uchar *buf, uint nbytes)
{
	char line[128], *p;
	int len = sizeof(line);
	int nchar;
	uint i;

	if (msg && (msg[0] != '\0'))
		printf("%s:\n", msg);

	p = line;
	for (i = 0; i < nbytes; i++) {
		if (i % 16 == 0) {
			nchar = snprintf(p, len, "  %04x: ", i);	/* line prefix */
			p += nchar;
			len -= nchar;
		}
		if (len > 0) {
			nchar = snprintf(p, len, "%02x ", buf[i]);
			p += nchar;
			len -= nchar;
		}

		if (i % 16 == 15) {
			printf("%s\n", line);		/* flush line */
			p = line;
			len = sizeof(line);
		}
	}

	/* flush last partial line */
	if (p != line)
		printf("%s\n", line);
}

static const char *crypto_algo_names[] = {
	"NONE",
	"WEP1",
	"TKIP",
	"WEP128",
	"AES_CCM",
	"AES_OCB_MSDU",
	"AES_OCB_MPDU",
#ifdef BCMCCX
	"CKIP",
	"CKIP_MMH",
	"WEP_MMH",
	"NALG",
#else
	"NALG",
	"UNDEF",
	"UNDEF",
	"UNDEF",
#endif /* BCMCCX */
	"WAPI",
	"PMK",
	"BIP",
	"AES_GCM",
	"AES_CCM256",
	"AES_GCM256",
	"BIP_CMAC256",
	"BIP_GMAC",
	"BIP_GMAC256",
	"UNDEF"
};

const char *
bcm_crypto_algo_name(uint algo)
{
	return (algo < ARRAYSIZE(crypto_algo_names)) ? crypto_algo_names[algo] : "ERR";
}

#ifdef BCMDBG
void
deadbeef(void *p, uint len)
{
	static uint8 meat[] = { 0xde, 0xad, 0xbe, 0xef };

	while (len-- > 0) {
		*(uint8*)p = meat[((uintptr)p) & 3];
		p = (uint8*)p + 1;
	}
}
#endif /* BCMDBG */

char *
bcm_chipname(uint chipid, char *buf, uint len)
{
	const char *fmt;

	fmt = ((chipid > 0xa000) || (chipid < 0x4000)) ? "%d" : "%x";
	snprintf(buf, len, fmt, chipid);
	return buf;
}

/* Produce a human-readable string for boardrev */
char *
bcm_brev_str(uint32 brev, char *buf)
{
	if (brev < 0x100)
		snprintf(buf, 8, "%d.%d", (brev & 0xf0) >> 4, brev & 0xf);
	else
		snprintf(buf, 8, "%c%03x", ((brev & 0xf000) == 0x1000) ? 'P' : 'A', brev & 0xfff);

	return (buf);
}

#define BUFSIZE_TODUMP_ATONCE 512 /* Buffer size */

/* dump large strings to console */
void
printbig(char *buf)
{
	uint len, max_len;
	char c;

	len = (uint)strlen(buf);

	max_len = BUFSIZE_TODUMP_ATONCE;

	while (len > max_len) {
		c = buf[max_len];
		buf[max_len] = '\0';
		printf("%s", buf);
		buf[max_len] = c;

		buf += max_len;
		len -= max_len;
	}
	/* print the remaining string */
	printf("%s\n", buf);
	return;
}

/* routine to dump fields in a fileddesc structure */
uint
bcmdumpfields(bcmutl_rdreg_rtn read_rtn, void *arg0, uint arg1, struct fielddesc *fielddesc_array,
	char *buf, uint32 bufsize)
{
	uint  filled_len;
	int len;
	struct fielddesc *cur_ptr;

	filled_len = 0;
	cur_ptr = fielddesc_array;

	while (bufsize > 1) {
		if (cur_ptr->nameandfmt == NULL)
			break;
		len = snprintf(buf, bufsize, cur_ptr->nameandfmt,
		               read_rtn(arg0, arg1, cur_ptr->offset));
		/* check for snprintf overflow or error */
		if (len < 0 || (uint32)len >= bufsize)
			len = bufsize - 1;
		buf += len;
		bufsize -= len;
		filled_len += len;
		cur_ptr++;
	}
	return filled_len;
}

uint
bcm_mkiovar(const char *name, char *data, uint datalen, char *buf, uint buflen)
{
	uint len;

	len = (uint)strlen(name) + 1;

	if ((len + datalen) > buflen)
		return 0;

	strncpy(buf, name, buflen);

	/* append data onto the end of the name string */
	memcpy(&buf[len], data, datalen);
	len += datalen;

	return len;
}

/* Quarter dBm units to mW
 * Table starts at QDBM_OFFSET, so the first entry is mW for qdBm=153
 * Table is offset so the last entry is largest mW value that fits in
 * a uint16.
 */

#define QDBM_OFFSET 153		/* Offset for first entry */
#define QDBM_TABLE_LEN 40	/* Table size */

/* Smallest mW value that will round up to the first table entry, QDBM_OFFSET.
 * Value is ( mW(QDBM_OFFSET - 1) + mW(QDBM_OFFSET) ) / 2
 */
#define QDBM_TABLE_LOW_BOUND 6493 /* Low bound */

/* Largest mW value that will round down to the last table entry,
 * QDBM_OFFSET + QDBM_TABLE_LEN-1.
 * Value is ( mW(QDBM_OFFSET + QDBM_TABLE_LEN - 1) + mW(QDBM_OFFSET + QDBM_TABLE_LEN) ) / 2.
 */
#define QDBM_TABLE_HIGH_BOUND 64938 /* High bound */

static const uint16 nqdBm_to_mW_map[QDBM_TABLE_LEN] = {
/* qdBm: 	+0 	+1 	+2 	+3 	+4 	+5 	+6 	+7 */
/* 153: */      6683,	7079,	7499,	7943,	8414,	8913,	9441,	10000,
/* 161: */      10593,	11220,	11885,	12589,	13335,	14125,	14962,	15849,
/* 169: */      16788,	17783,	18836,	19953,	21135,	22387,	23714,	25119,
/* 177: */      26607,	28184,	29854,	31623,	33497,	35481,	37584,	39811,
/* 185: */      42170,	44668,	47315,	50119,	53088,	56234,	59566,	63096
};

uint16
bcm_qdbm_to_mw(uint8 qdbm)
{
	uint factor = 1;
	int idx = qdbm - QDBM_OFFSET;

	if (idx >= QDBM_TABLE_LEN) {
		/* clamp to max uint16 mW value */
		return 0xFFFF;
	}

	/* scale the qdBm index up to the range of the table 0-40
	 * where an offset of 40 qdBm equals a factor of 10 mW.
	 */
	while (idx < 0) {
		idx += 40;
		factor *= 10;
	}

	/* return the mW value scaled down to the correct factor of 10,
	 * adding in factor/2 to get proper rounding.
	 */
	return ((nqdBm_to_mW_map[idx] + factor/2) / factor);
}

uint8
bcm_mw_to_qdbm(uint16 mw)
{
	uint8 qdbm;
	int offset;
	uint mw_uint = mw;
	uint boundary;

	/* handle boundary case */
	if (mw_uint <= 1)
		return 0;

	offset = QDBM_OFFSET;

	/* move mw into the range of the table */
	while (mw_uint < QDBM_TABLE_LOW_BOUND) {
		mw_uint *= 10;
		offset -= 40;
	}

	for (qdbm = 0; qdbm < QDBM_TABLE_LEN-1; qdbm++) {
		boundary = nqdBm_to_mW_map[qdbm] + (nqdBm_to_mW_map[qdbm+1] -
		                                    nqdBm_to_mW_map[qdbm])/2;
		if (mw_uint < boundary) break;
	}

	qdbm += (uint8)offset;

	return (qdbm);
}


uint
bcm_bitcount(uint8 *bitmap, uint length)
{
	uint bitcount = 0, i;
	uint8 tmp;
	for (i = 0; i < length; i++) {
		tmp = bitmap[i];
		while (tmp) {
			bitcount++;
			tmp &= (tmp - 1);
		}
	}
	return bitcount;
}

#ifdef BCMDRIVER

/* Initialization of bcmstrbuf structure */
void
bcm_binit(struct bcmstrbuf *b, char *buf, uint size)
{
	b->origsize = b->size = size;
	b->origbuf = b->buf = buf;
}

/* Buffer sprintf wrapper to guard against buffer overflow */
int
bcm_bprintf(struct bcmstrbuf *b, const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);

	r = vsnprintf(b->buf, b->size, fmt, ap);

	/* Non Ansi C99 compliant returns -1,
	 * Ansi compliant return r >= b->size,
	 * bcmstdlib returns 0, handle all
	 */
	/* r == 0 is also the case when strlen(fmt) is zero.
	 * typically the case when "" is passed as argument.
	 */
	if ((r == -1) || (r >= (int)b->size)) {
		b->size = 0;
	} else {
		b->size -= r;
		b->buf += r;
	}

	va_end(ap);

	return r;
}

void
bcm_bprhex(struct bcmstrbuf *b, const char *msg, bool newline, const uint8 *buf, int len)
{
	int i;

	if (msg != NULL && msg[0] != '\0')
		bcm_bprintf(b, "%s", msg);
	for (i = 0; i < len; i ++)
		bcm_bprintf(b, "%02X", buf[i]);
	if (newline)
		bcm_bprintf(b, "\n");
}

void
bcm_inc_bytes(uchar *num, int num_bytes, uint8 amount)
{
	int i;

	for (i = 0; i < num_bytes; i++) {
		num[i] += amount;
		if (num[i] >= amount)
			break;
		amount = 1;
	}
}

int
bcm_cmp_bytes(const uchar *arg1, const uchar *arg2, uint8 nbytes)
{
	int i;

	for (i = nbytes - 1; i >= 0; i--) {
		if (arg1[i] != arg2[i])
			return (arg1[i] - arg2[i]);
	}
	return 0;
}

void
bcm_print_bytes(const char *name, const uchar *data, int len)
{
	int i;
	int per_line = 0;

	printf("%s: %d \n", name ? name : "", len);
	for (i = 0; i < len; i++) {
		printf("%02x ", *data++);
		per_line++;
		if (per_line == 16) {
			per_line = 0;
			printf("\n");
		}
	}
	printf("\n");
}

/* Look for vendor-specific IE with specified OUI and optional type */
bcm_tlv_t *
bcm_find_vendor_ie(void *tlvs, int tlvs_len, const char *voui, uint8 *type, int type_len)
{
	bcm_tlv_t *ie;
	uint8 ie_len;

	ie = (bcm_tlv_t*)tlvs;

	/* make sure we are looking at a valid IE */
	if (ie == NULL || !bcm_valid_tlv(ie, tlvs_len)) {
		return NULL;
	}

	/* Walk through the IEs looking for an OUI match */
	do {
		ie_len = ie->len;
		if ((ie->id == DOT11_MNG_PROPR_ID) &&
		    (ie_len >= (DOT11_OUI_LEN + type_len)) &&
		    !bcmp(ie->data, voui, DOT11_OUI_LEN))
		{
			/* compare optional type */
			if (type_len == 0 ||
			    !bcmp(&ie->data[DOT11_OUI_LEN], type, type_len)) {
				return (ie);		/* a match */
			}
		}
	} while ((ie = bcm_next_tlv(ie, &tlvs_len)) != NULL);

	return NULL;
}

#if defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_INFORM) || \
	defined(WLMSG_ASSOC) || defined(WLMSG_PRPKT) || defined(WLMSG_WSEC)
#define SSID_FMT_BUF_LEN	((4 * DOT11_MAX_SSID_LEN) + 1)

int
bcm_format_ssid(char* buf, const uchar ssid[], uint ssid_len)
{
	uint i, c;
	char *p = buf;
	char *endp = buf + SSID_FMT_BUF_LEN;

	if (ssid_len > DOT11_MAX_SSID_LEN) ssid_len = DOT11_MAX_SSID_LEN;

	for (i = 0; i < ssid_len; i++) {
		c = (uint)ssid[i];
		if (c == '\\') {
			*p++ = '\\';
			*p++ = '\\';
		} else if (bcm_isprint((uchar)c)) {
			*p++ = (char)c;
		} else {
			p += snprintf(p, (endp - p), "\\x%02X", c);
		}
	}
	*p = '\0';
	ASSERT(p < endp);

	return (int)(p - buf);
}
#endif /* WLTINYDUMP || BCMDBG || WLMSG_INFORM || WLMSG_ASSOC || WLMSG_PRPKT */

#endif /* BCMDRIVER */

/*
 * ProcessVars:Takes a buffer of "<var>=<value>\n" lines read from a file and ending in a NUL.
 * also accepts nvram files which are already in the format of <var1>=<value>\0\<var2>=<value2>\0
 * Removes carriage returns, empty lines, comment lines, and converts newlines to NULs.
 * Shortens buffer as needed and pads with NULs.  End of buffer is marked by two NULs.
*/

unsigned int
process_nvram_vars(char *varbuf, unsigned int len)
{
	char *dp;
	bool findNewline;
	int column;
	unsigned int buf_len, n;
	unsigned int pad = 0;

	dp = varbuf;

	findNewline = FALSE;
	column = 0;

	for (n = 0; n < len; n++) {
		if (varbuf[n] == '\r')
			continue;
		if (findNewline && varbuf[n] != '\n')
			continue;
		findNewline = FALSE;
		if (varbuf[n] == '#') {
			findNewline = TRUE;
			continue;
		}
		if (varbuf[n] == '\n') {
			if (column == 0)
				continue;
			*dp++ = 0;
			column = 0;
			continue;
		}
		*dp++ = varbuf[n];
		column++;
	}
	buf_len = (unsigned int)(dp - varbuf);
	if (buf_len % 4) {
		pad = 4 - buf_len % 4;
		if (pad && (buf_len + pad <= len)) {
			buf_len += pad;
		}
	}

	while (dp < varbuf + n)
		*dp++ = 0;

	return buf_len;
}

/* calculate a * b + c */
void
bcm_uint64_multiple_add(uint32* r_high, uint32* r_low, uint32 a, uint32 b, uint32 c)
{
#define FORMALIZE(var) {cc += (var & 0x80000000) ? 1 : 0; var &= 0x7fffffff;}
	uint32 r1, r0;
	uint32 a1, a0, b1, b0, t, cc = 0;

	a1 = a >> 16;
	a0 = a & 0xffff;
	b1 = b >> 16;
	b0 = b & 0xffff;

	r0 = a0 * b0;
	FORMALIZE(r0);

	t = (a1 * b0) << 16;
	FORMALIZE(t);

	r0 += t;
	FORMALIZE(r0);

	t = (a0 * b1) << 16;
	FORMALIZE(t);

	r0 += t;
	FORMALIZE(r0);

	FORMALIZE(c);

	r0 += c;
	FORMALIZE(r0);

	r0 |= (cc % 2) ? 0x80000000 : 0;
	r1 = a1 * b1 + ((a1 * b0) >> 16) + ((b1 * a0) >> 16) + (cc / 2);

	*r_high = r1;
	*r_low = r0;
}

/* calculate a / b */
void
bcm_uint64_divide(uint32* r, uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low, r0 = 0;

	if (b < 2)
		return;

	while (a1 != 0) {
		r0 += (0xffffffff / b) * a1;
		bcm_uint64_multiple_add(&a1, &a0, ((0xffffffff % b) + 1) % b, a1, a0);
	}

	r0 += a0 / b;
	*r = r0;
}

#ifndef setbit /* As in the header file */
#ifdef BCMUTILS_BIT_MACROS_USE_FUNCS
/* Set bit in byte array. */
void
setbit(void *array, uint bit)
{
	((uint8 *)array)[bit / NBBY] |= 1 << (bit % NBBY);
}

/* Clear bit in byte array. */
void
clrbit(void *array, uint bit)
{
	((uint8 *)array)[bit / NBBY] &= ~(1 << (bit % NBBY));
}

/* Test if bit is set in byte array. */
bool
isset(const void *array, uint bit)
{
	return (((const uint8 *)array)[bit / NBBY] & (1 << (bit % NBBY)));
}

/* Test if bit is clear in byte array. */
bool
isclr(const void *array, uint bit)
{
	return ((((const uint8 *)array)[bit / NBBY] & (1 << (bit % NBBY))) == 0);
}
#endif /* BCMUTILS_BIT_MACROS_USE_FUNCS */
#endif /* setbit */

void
set_bitrange(void *array, uint start, uint end, uint maxbit)
{
	uint startbyte = start/NBBY;
	uint endbyte = end/NBBY;
	uint i, startbytelastbit, endbytestartbit;

	if (end >= start) {
		if (endbyte - startbyte > 1)
		{
			startbytelastbit = (startbyte+1)*NBBY - 1;
			endbytestartbit = endbyte*NBBY;
			for (i = startbyte+1; i < endbyte; i++)
				((uint8 *)array)[i] = 0xFF;
			for (i = start; i <= startbytelastbit; i++)
				setbit(array, i);
			for (i = endbytestartbit; i <= end; i++)
				setbit(array, i);
		} else {
			for (i = start; i <= end; i++)
				setbit(array, i);
		}
	}
	else {
		set_bitrange(array, start, maxbit, maxbit);
		set_bitrange(array, 0, end, maxbit);
	}
}

/* Number of bits in a uint32 */
#define NUM_BITS_U32                (NBBY * sizeof(uint32))
/* Number of uint32 required for a bitmap */
#define BMAP_NUM_U32(sz)            ((sz) / NUM_BITS_U32)
/* Given a bitmap position [0..maxbits), get the index iinto uint32 array */
#define BMAP_B2U32IDX(pos)          ((pos) / NUM_BITS_U32)

#define __WRAP_INC(ix, wrap)        (((ix) + 1) % (wrap))

/* Helper routines for counting sequence of 0's and setting a sequence of 1's */
static INLINE uint32 /* within a single uint32 */
_bcm_count_zeros_sequence_u32(uint32 u32, uint32 pos32, const uint32 numbits);
static INLINE uint32 /* across 'm'ultiple uint32 */
_bcm_count_zeros_sequence_m32(const uint32 *bmap, uint32 s32, const uint32 e32,
	const uint32 spos32, const uint32 epos, const uint32 numbits,
	const uint32 maxbits);

/**
 * +---------------------------------------------------------------------------+
 * count number of contiguous zeros from start pos in a uint32
 * bounded to a range of numbits
 * +---------------------------------------------------------------------------+
 */
static INLINE uint32
_bcm_count_zeros_sequence_u32(uint32 u32, uint32 pos32, const uint32 numbits)
{
	uint32 mask; /* clearing mask */

	/* Zero out all bits trailing from bit pos32 in a uint32 bit word */
	mask = ((~0U) << pos32);
	u32 &= mask;

	/* Zero out all bits leading till bit pos32 + numbits in a uint32 bit word */
	mask = ((~0U) >> (NUM_BITS_U32 - (pos32 + numbits)));
	u32 &= mask;

	if (u32 != 0U) { /* at least one set bit after masking trailing bits */
		uint32 lz, tz; /* leading zeros and trailing zeros */
		u32 ^= (u32-1); /* clear all set bits except the trailing set bit */

		/* Count leading zeros: Use ARMv7 clz instruction */
		lz = bcm_count_leading_zeros(u32);
		tz = 31 - lz; /* number of trailing zero bits */
		return (tz - pos32); /* number of trailing zero bits from position */
	} else {
		return numbits; /* return range, as all bits until end of word are zero */
	}
}

/**
 * +---------------------------------------------------------------------------+
 *
 * _bcm_count_zeros_sequence_m32
 *
 * count number of contiguous zeros from start pos in a multi-uint32.
 *
 *    bmap   : pointer to the start of a multiword bitmap
 *    s32    : uint32 array index in which the start position occurs
 *    e32    : uint32 array index in which the end position occurs
 *    spos32 : postion in the uint32 word identified by s32
 *    epos   : position of the end bit in the multiword bitmap
 *    numbits: number of bits in the range from [spos .. epos], inclusive
 *    maxbits: total bits in the bmap
 *
 * +---------------------------------------------------------------------------+
 */
static INLINE uint32
_bcm_count_zeros_sequence_m32(const uint32 *bmap, uint32 s32, const uint32 e32,
	const uint32 spos32, const uint32 epos, const uint32 numbits,
	const uint32 maxbits)
{
	uint32 w32, count, find, found;

	/* Search first word in multi word bitmap */
	find = NUM_BITS_U32 - spos32;
	count = _bcm_count_zeros_sequence_u32(bmap[s32], spos32, find);

	if (count < find) {
		return count;
	}

	/* Loop over each uint32 with wrapover, until end uint32 (not inclusive) */
	w32 = (maxbits / NUM_BITS_U32);
	for (s32 = __WRAP_INC(s32, w32); s32 != e32; s32 = __WRAP_INC(s32, w32)) {
		if (bmap[s32] != 0) {
			/* range is entire word */
			found = _bcm_count_zeros_sequence_u32(bmap[s32], 0, NUM_BITS_U32);
			return count + found;
		}
		count += NUM_BITS_U32;
	}

	/* Now search in the end uint32 word */
	find = (epos % NUM_BITS_U32) + 1; /* range is from 0 to end position */
	found = _bcm_count_zeros_sequence_u32(bmap[e32], 0, find);
	return count + found;
}

/**
 * +---------------------------------------------------------------------------+
 *
 * Count the number of continous zeroes starting from bit position "pos",
 * upto "numbits" bits, in a bitmap carrying "maxbits" number of bits.
 *
 *      pos : starting bit position.
 *            pos must be in the positional range [0 .. maxbits)
 *            bit at pos is tested for 0.
 *
 *      numbits : number of bits in the positional range [pos .. pos+numbits)
 *            to be tested for 0. bit at position pos+numbits is not tested.
 *            The positional range may wrap around the bmap.
 *
 *      maxbits : total number of bits in bmap
 *
 * +---------------------------------------------------------------------------+
 *
 * Example: Input bitmap with "maxbits" = 128 total bits:
 *
 *      3        2        1        0      0
 *      1        3        5        7      0      "bmap"
 *
 *      10000000 00000000 00000000 10000001      0x80000081
 *      00000000 10000000 00000000 00000000      0x00800000
 *      00000000 00000000 00000000 00000000      0x00010000
 *      00000001 00000000 00000000 00000000      0x01000000
 *
 *      --------------------------
 *      | "pos" | "num" | return |
 *      --------------------------
 *      |  ANY  |    0  | ASSERT |  numbits cannot be zero !
 *      |    0  |    1  |    0   |
 *      |   30  |   10  |    1   |
 *      |   32  |   32  |   23   |
 *      |   60  |   64  |   60   |
 *      |  127  |    2  |    1   |
 *      --------------------------
 *
 * +---------------------------------------------------------------------------+
 */
uint32
bcm_count_zeros_sequence(const uint8 *bmap,
	const uint32 pos, const uint32 numbits, const uint32 maxbits)
{
	uint32 epos; /* end bit position to test */
	uint32 s32;  /* index into uint32 array for pos bit */
	uint32 e32;  /* index into uint32 array for epos bit */
	uint32 count;

	ASSERT(pos < maxbits);
	ASSERT(numbits != 0U);
	ASSERT(numbits <= maxbits);
	ASSERT(maxbits > NUM_BITS_U32);

	if (numbits == 0)
		return 0;

	s32 = BMAP_B2U32IDX(pos); /* index into uint32 array of pos */

	if (numbits == 1) { /* test a single bit */
		count = ((((const uint32 *)bmap)[s32] & (1 << (pos % NUM_BITS_U32))) == 0);
		return count;
	}

	epos = (pos + (numbits - 1)) % maxbits; /* end bit position for range */
	e32 = BMAP_B2U32IDX(epos); /* index into uint32 array of epos */

	if ((e32 == s32) && (numbits <= NUM_BITS_U32)) {
		/* both pos and epos fall in the same uint32 word */
		count = _bcm_count_zeros_sequence_u32(((const uint32 *)bmap)[s32],
			pos % NUM_BITS_U32, numbits);
	} else { /* pos and epos span multiple uint32 */
		count = _bcm_count_zeros_sequence_m32((const uint32 *)bmap, s32, e32,
			pos % NUM_BITS_U32, epos, numbits, maxbits);
	}

	return count;
}


void
bcm_bitprint32(const uint32 u32arg)
{
	int i;
	for (i = NBITS(uint32) - 1; i >= 0; i--) {
		isbitset(u32arg, i) ? printf("1") : printf("0");
		if ((i % NBBY) == 0) printf(" ");
	}
	printf("\n");
}

/* calculate checksum for ip header, tcp / udp header / data */
uint16
bcm_ip_cksum(uint8 *buf, uint32 len, uint32 sum)
{
	while (len > 1) {
		sum += (buf[0] << 8) | buf[1];
		buf += 2;
		len -= 2;
	}

	if (len > 0) {
		sum += (*buf) << 8;
	}

	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}

	return ((uint16)~sum);
}

#ifdef BCMDRIVER
/*
 * Hierarchical Multiword bitmap based small id allocator.
 *
 * Multilevel hierarchy bitmap. (maximum 2 levels)
 * First hierarchy uses a multiword bitmap to identify 32bit words in the
 * second hierarchy that have at least a single bit set. Each bit in a word of
 * the second hierarchy represents a unique ID that may be allocated.
 *
 * BCM_MWBMAP_ITEMS_MAX: Maximum number of IDs managed.
 * BCM_MWBMAP_BITS_WORD: Number of bits in a bitmap word word
 * BCM_MWBMAP_WORDS_MAX: Maximum number of bitmap words needed for free IDs.
 * BCM_MWBMAP_WDMAP_MAX: Maximum number of bitmap wordss identifying first non
 *                       non-zero bitmap word carrying at least one free ID.
 * BCM_MWBMAP_SHIFT_OP:  Used in MOD, DIV and MUL operations.
 * BCM_MWBMAP_INVALID_IDX: Value ~0U is treated as an invalid ID
 *
 * Design Notes:
 * BCM_MWBMAP_USE_CNTSETBITS trades CPU for memory. A runtime count of how many
 * bits are computed each time on allocation and deallocation, requiring 4
 * array indexed access and 3 arithmetic operations. When not defined, a runtime
 * count of set bits state is maintained. Upto 32 Bytes per 1024 IDs is needed.
 * In a 4K max ID allocator, up to 128Bytes are hence used per instantiation.
 * In a memory limited system e.g. dongle builds, a CPU for memory tradeoff may
 * be used by defining BCM_MWBMAP_USE_CNTSETBITS.
 *
 * Note: wd_bitmap[] is statically declared and is not ROM friendly ... array
 * size is fixed. No intention to support larger than 4K indice allocation. ID
 * allocators for ranges smaller than 4K will have a wastage of only 12Bytes
 * with savings in not having to use an indirect access, had it been dynamically
 * allocated.
 */
#if defined(DONGLEBUILD)
#define BCM_MWBMAP_USE_CNTSETBITS	        /* runtime count set bits */
#define BCM_MWBMAP_ITEMS_MAX	(4 * 1024)
#else  /* ! DONGLEBUILD */
#define BCM_MWBMAP_ITEMS_MAX    (4 * 1024)  /* May increase to 16K */
#endif /*   DONGLEBUILD */

#define BCM_MWBMAP_BITS_WORD    (NBITS(uint32))
#define BCM_MWBMAP_WORDS_MAX    (BCM_MWBMAP_ITEMS_MAX / BCM_MWBMAP_BITS_WORD)
#define BCM_MWBMAP_WDMAP_MAX    (BCM_MWBMAP_WORDS_MAX / BCM_MWBMAP_BITS_WORD)
#define BCM_MWBMAP_SHIFT_OP     (5)
#define BCM_MWBMAP_MODOP(ix)    ((ix) & (BCM_MWBMAP_BITS_WORD - 1))
#define BCM_MWBMAP_DIVOP(ix)    ((ix) >> BCM_MWBMAP_SHIFT_OP)
#define BCM_MWBMAP_MULOP(ix)    ((ix) << BCM_MWBMAP_SHIFT_OP)

/* Redefine PTR() and/or HDL() conversion to invoke audit for debugging */
#define BCM_MWBMAP_PTR(hdl)		((struct bcm_mwbmap *)(hdl))
#define BCM_MWBMAP_HDL(ptr)		((void *)(ptr))

#if defined(BCM_MWBMAP_DEBUG)
#define BCM_MWBMAP_AUDIT(mwb) \
	do { \
		ASSERT((mwb != NULL) && \
		       (((struct bcm_mwbmap *)(mwb))->magic == (void *)(mwb))); \
		bcm_mwbmap_audit(mwb); \
	} while (0)
#define MWBMAP_ASSERT(exp)		ASSERT(exp)
#define MWBMAP_DBG(x)           printf x
#else   /* !BCM_MWBMAP_DEBUG */
#define BCM_MWBMAP_AUDIT(mwb)   do {} while (0)
#define MWBMAP_ASSERT(exp)		do {} while (0)
#define MWBMAP_DBG(x)
#endif  /* !BCM_MWBMAP_DEBUG */


typedef struct bcm_mwbmap {     /* Hierarchical multiword bitmap allocator    */
	uint16 wmaps;               /* Total number of words in free wd bitmap    */
	uint16 imaps;               /* Total number of words in free id bitmap    */
	int16  ifree;               /* Count of free indices. Used only in audits */
	uint16 total;               /* Total indices managed by multiword bitmap  */

	void * magic;               /* Audit handle parameter from user           */

	uint32 wd_bitmap[BCM_MWBMAP_WDMAP_MAX]; /* 1st level bitmap of            */
#if !defined(BCM_MWBMAP_USE_CNTSETBITS)
	int8   wd_count[BCM_MWBMAP_WORDS_MAX];  /* free id running count, 1st lvl */
#endif /*  ! BCM_MWBMAP_USE_CNTSETBITS */

	uint32 id_bitmap[0];        /* Second level bitmap                        */
} bcm_mwbmap_t;

/* Incarnate a hierarchical multiword bitmap based small index allocator. */
struct bcm_mwbmap *
BCMATTACHFN(bcm_mwbmap_init)(osl_t *osh, uint32 items_max)
{
	struct bcm_mwbmap * mwbmap_p;
	uint32 wordix, size, words, extra;

	/* Implementation Constraint: Uses 32bit word bitmap */
	MWBMAP_ASSERT(BCM_MWBMAP_BITS_WORD == 32U);
	MWBMAP_ASSERT(BCM_MWBMAP_SHIFT_OP == 5U);
	MWBMAP_ASSERT(ISPOWEROF2(BCM_MWBMAP_ITEMS_MAX));
	MWBMAP_ASSERT((BCM_MWBMAP_ITEMS_MAX % BCM_MWBMAP_BITS_WORD) == 0U);

	ASSERT(items_max <= BCM_MWBMAP_ITEMS_MAX);

	/* Determine the number of words needed in the multiword bitmap */
	extra = BCM_MWBMAP_MODOP(items_max);
	words = BCM_MWBMAP_DIVOP(items_max) + ((extra != 0U) ? 1U : 0U);

	/* Allocate runtime state of multiword bitmap */
	/* Note: wd_count[] or wd_bitmap[] are not dynamically allocated */
	size = sizeof(bcm_mwbmap_t) + (sizeof(uint32) * words);
	mwbmap_p = (bcm_mwbmap_t *)MALLOC(osh, size);
	if (mwbmap_p == (bcm_mwbmap_t *)NULL) {
		ASSERT(0);
		goto error1;
	}
	memset(mwbmap_p, 0, size);

	/* Initialize runtime multiword bitmap state */
	mwbmap_p->imaps = (uint16)words;
	mwbmap_p->ifree = (int16)items_max;
	mwbmap_p->total = (uint16)items_max;

	/* Setup magic, for use in audit of handle */
	mwbmap_p->magic = BCM_MWBMAP_HDL(mwbmap_p);

	/* Setup the second level bitmap of free indices */
	/* Mark all indices as available */
	for (wordix = 0U; wordix < mwbmap_p->imaps; wordix++) {
		mwbmap_p->id_bitmap[wordix] = (uint32)(~0U);
#if !defined(BCM_MWBMAP_USE_CNTSETBITS)
		mwbmap_p->wd_count[wordix] = BCM_MWBMAP_BITS_WORD;
#endif /*  ! BCM_MWBMAP_USE_CNTSETBITS */
	}

	/* Ensure that extra indices are tagged as un-available */
	if (extra) { /* fixup the free ids in last bitmap and wd_count */
		uint32 * bmap_p = &mwbmap_p->id_bitmap[mwbmap_p->imaps - 1];
		*bmap_p ^= (uint32)(~0U << extra); /* fixup bitmap */
#if !defined(BCM_MWBMAP_USE_CNTSETBITS)
		mwbmap_p->wd_count[mwbmap_p->imaps - 1] = (int8)extra; /* fixup count */
#endif /*  ! BCM_MWBMAP_USE_CNTSETBITS */
	}

	/* Setup the first level bitmap hierarchy */
	extra = BCM_MWBMAP_MODOP(mwbmap_p->imaps);
	words = BCM_MWBMAP_DIVOP(mwbmap_p->imaps) + ((extra != 0U) ? 1U : 0U);

	mwbmap_p->wmaps = (uint16)words;

	for (wordix = 0U; wordix < mwbmap_p->wmaps; wordix++)
		mwbmap_p->wd_bitmap[wordix] = (uint32)(~0U);
	if (extra) {
		uint32 * bmap_p = &mwbmap_p->wd_bitmap[mwbmap_p->wmaps - 1];
		*bmap_p ^= (uint32)(~0U << extra); /* fixup bitmap */
	}

	return mwbmap_p;

error1:
	return BCM_MWBMAP_INVALID_HDL;
}

/* Release resources used by multiword bitmap based small index allocator. */
void
BCMATTACHFN(bcm_mwbmap_fini)(osl_t * osh, struct bcm_mwbmap * mwbmap_hdl)
{
	bcm_mwbmap_t * mwbmap_p;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	MFREE(osh, mwbmap_p, sizeof(struct bcm_mwbmap)
	                     + (sizeof(uint32) * mwbmap_p->imaps));
	return;
}

/* Allocate a unique small index using a multiword bitmap index allocator.    */
uint32 BCMFASTPATH
bcm_mwbmap_alloc(struct bcm_mwbmap * mwbmap_hdl)
{
	bcm_mwbmap_t * mwbmap_p;
	uint32 wordix, bitmap;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	/* Start with the first hierarchy */
	for (wordix = 0; wordix < mwbmap_p->wmaps; ++wordix) {

		bitmap = mwbmap_p->wd_bitmap[wordix]; /* get the word bitmap */

		if (bitmap != 0U) {

			uint32 count, bitix, *bitmap_p;

			bitmap_p = &mwbmap_p->wd_bitmap[wordix];

			/* clear all except trailing 1 */
			bitmap   = (uint32)(((int)(bitmap)) & (-((int)(bitmap))));
			MWBMAP_ASSERT(C_bcm_count_leading_zeros(bitmap) ==
			              bcm_count_leading_zeros(bitmap));
			bitix    = (BCM_MWBMAP_BITS_WORD - 1)
			         - bcm_count_leading_zeros(bitmap); /* use asm clz */
			wordix   = BCM_MWBMAP_MULOP(wordix) + bitix;

			/* Clear bit if wd count is 0, without conditional branch */
#if defined(BCM_MWBMAP_USE_CNTSETBITS)
			count = bcm_cntsetbits(mwbmap_p->id_bitmap[wordix]) - 1;
#else  /* ! BCM_MWBMAP_USE_CNTSETBITS */
			mwbmap_p->wd_count[wordix]--;
			count = mwbmap_p->wd_count[wordix];
			MWBMAP_ASSERT(count ==
			              (bcm_cntsetbits(mwbmap_p->id_bitmap[wordix]) - 1));
#endif /* ! BCM_MWBMAP_USE_CNTSETBITS */
			MWBMAP_ASSERT(count >= 0);

			/* clear wd_bitmap bit if id_map count is 0 */
			bitmap = (count == 0) << bitix;

			MWBMAP_DBG((
			    "Lvl1: bitix<%02u> wordix<%02u>: %08x ^ %08x = %08x wfree %d",
			    bitix, wordix, *bitmap_p, bitmap, (*bitmap_p) ^ bitmap, count));

			*bitmap_p ^= bitmap;

			/* Use bitix in the second hierarchy */
			bitmap_p = &mwbmap_p->id_bitmap[wordix];

			bitmap = mwbmap_p->id_bitmap[wordix]; /* get the id bitmap */
			MWBMAP_ASSERT(bitmap != 0U);

			/* clear all except trailing 1 */
			bitmap   = (uint32)(((int)(bitmap)) & (-((int)(bitmap))));
			MWBMAP_ASSERT(C_bcm_count_leading_zeros(bitmap) ==
			              bcm_count_leading_zeros(bitmap));
			bitix    = BCM_MWBMAP_MULOP(wordix)
			         + (BCM_MWBMAP_BITS_WORD - 1)
			         - bcm_count_leading_zeros(bitmap); /* use asm clz */

			mwbmap_p->ifree--; /* decrement system wide free count */
			MWBMAP_ASSERT(mwbmap_p->ifree >= 0);

			MWBMAP_DBG((
			    "Lvl2: bitix<%02u> wordix<%02u>: %08x ^ %08x = %08x ifree %d",
			    bitix, wordix, *bitmap_p, bitmap, (*bitmap_p) ^ bitmap,
			    mwbmap_p->ifree));

			*bitmap_p ^= bitmap; /* mark as allocated = 1b0 */

			return bitix;
		}
	}

	ASSERT(mwbmap_p->ifree == 0);

	return BCM_MWBMAP_INVALID_IDX;
}

/* Force an index at a specified position to be in use */
void
bcm_mwbmap_force(struct bcm_mwbmap * mwbmap_hdl, uint32 bitix)
{
	bcm_mwbmap_t * mwbmap_p;
	uint32 count, wordix, bitmap, *bitmap_p;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	ASSERT(bitix < mwbmap_p->total);

	/* Start with second hierarchy */
	wordix   = BCM_MWBMAP_DIVOP(bitix);
	bitmap   = (uint32)(1U << BCM_MWBMAP_MODOP(bitix));
	bitmap_p = &mwbmap_p->id_bitmap[wordix];

	ASSERT((*bitmap_p & bitmap) == bitmap);

	mwbmap_p->ifree--; /* update free count */
	ASSERT(mwbmap_p->ifree >= 0);

	MWBMAP_DBG(("Lvl2: bitix<%u> wordix<%u>: %08x ^ %08x = %08x ifree %d",
	           bitix, wordix, *bitmap_p, bitmap, (*bitmap_p) ^ bitmap,
	           mwbmap_p->ifree));

	*bitmap_p ^= bitmap; /* mark as in use */

	/* Update first hierarchy */
	bitix    = wordix;

	wordix   = BCM_MWBMAP_DIVOP(bitix);
	bitmap_p = &mwbmap_p->wd_bitmap[wordix];

#if defined(BCM_MWBMAP_USE_CNTSETBITS)
	count = bcm_cntsetbits(mwbmap_p->id_bitmap[bitix]);
#else  /* ! BCM_MWBMAP_USE_CNTSETBITS */
	mwbmap_p->wd_count[bitix]--;
	count = mwbmap_p->wd_count[bitix];
	MWBMAP_ASSERT(count == bcm_cntsetbits(mwbmap_p->id_bitmap[bitix]));
#endif /* ! BCM_MWBMAP_USE_CNTSETBITS */
	MWBMAP_ASSERT(count >= 0);

	bitmap   = (count == 0) << BCM_MWBMAP_MODOP(bitix);

	MWBMAP_DBG(("Lvl1: bitix<%02lu> wordix<%02u>: %08x ^ %08x = %08x wfree %d",
	           BCM_MWBMAP_MODOP(bitix), wordix, *bitmap_p, bitmap,
	           (*bitmap_p) ^ bitmap, count));

	*bitmap_p ^= bitmap; /* mark as in use */

	return;
}

/* Free a previously allocated index back into the multiword bitmap allocator */
void BCMFASTPATH
bcm_mwbmap_free(struct bcm_mwbmap * mwbmap_hdl, uint32 bitix)
{
	bcm_mwbmap_t * mwbmap_p;
	uint32 wordix, bitmap, *bitmap_p;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	ASSERT(bitix < mwbmap_p->total);

	/* Start with second level hierarchy */
	wordix   = BCM_MWBMAP_DIVOP(bitix);
	bitmap   = (1U << BCM_MWBMAP_MODOP(bitix));
	bitmap_p = &mwbmap_p->id_bitmap[wordix];

	ASSERT((*bitmap_p & bitmap) == 0U);	/* ASSERT not a double free */

	mwbmap_p->ifree++; /* update free count */
	ASSERT(mwbmap_p->ifree <= mwbmap_p->total);

	MWBMAP_DBG(("Lvl2: bitix<%02u> wordix<%02u>: %08x | %08x = %08x ifree %d",
	           bitix, wordix, *bitmap_p, bitmap, (*bitmap_p) | bitmap,
	           mwbmap_p->ifree));

	*bitmap_p |= bitmap; /* mark as available */

	/* Now update first level hierarchy */

	bitix    = wordix;

	wordix   = BCM_MWBMAP_DIVOP(bitix); /* first level's word index */
	bitmap   = (1U << BCM_MWBMAP_MODOP(bitix));
	bitmap_p = &mwbmap_p->wd_bitmap[wordix];

#if !defined(BCM_MWBMAP_USE_CNTSETBITS)
	mwbmap_p->wd_count[bitix]++;
#endif

#if defined(BCM_MWBMAP_DEBUG)
	{
		uint32 count;
#if defined(BCM_MWBMAP_USE_CNTSETBITS)
		count = bcm_cntsetbits(mwbmap_p->id_bitmap[bitix]);
#else  /*  ! BCM_MWBMAP_USE_CNTSETBITS */
		count = mwbmap_p->wd_count[bitix];
		MWBMAP_ASSERT(count == bcm_cntsetbits(mwbmap_p->id_bitmap[bitix]));
#endif /*  ! BCM_MWBMAP_USE_CNTSETBITS */

		MWBMAP_ASSERT(count <= BCM_MWBMAP_BITS_WORD);

		MWBMAP_DBG(("Lvl1: bitix<%02u> wordix<%02u>: %08x | %08x = %08x wfree %d",
		            bitix, wordix, *bitmap_p, bitmap, (*bitmap_p) | bitmap, count));
	}
#endif /* BCM_MWBMAP_DEBUG */

	*bitmap_p |= bitmap;

	return;
}

/* Fetch the toal number of free indices in the multiword bitmap allocator */
uint32
bcm_mwbmap_free_cnt(struct bcm_mwbmap * mwbmap_hdl)
{
	bcm_mwbmap_t * mwbmap_p;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	ASSERT(mwbmap_p->ifree >= 0);

	return mwbmap_p->ifree;
}

/* Determine whether an index is inuse or free */
bool
bcm_mwbmap_isfree(struct bcm_mwbmap * mwbmap_hdl, uint32 bitix)
{
	bcm_mwbmap_t * mwbmap_p;
	uint32 wordix, bitmap;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	ASSERT(bitix < mwbmap_p->total);

	wordix   = BCM_MWBMAP_DIVOP(bitix);
	bitmap   = (1U << BCM_MWBMAP_MODOP(bitix));

	return ((mwbmap_p->id_bitmap[wordix] & bitmap) != 0U);
}

/* Debug dump a multiword bitmap allocator */
void
bcm_mwbmap_show(struct bcm_mwbmap * mwbmap_hdl)
{
	uint32 ix, count;
	bcm_mwbmap_t * mwbmap_p;

	BCM_MWBMAP_AUDIT(mwbmap_hdl);
	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	printf("mwbmap_p %p wmaps %u imaps %u ifree %d total %u\n", mwbmap_p,
	       mwbmap_p->wmaps, mwbmap_p->imaps, mwbmap_p->ifree, mwbmap_p->total);
	for (ix = 0U; ix < mwbmap_p->wmaps; ix++) {
		printf("\tWDMAP:%2u. 0x%08x\t", ix, mwbmap_p->wd_bitmap[ix]);
		bcm_bitprint32(mwbmap_p->wd_bitmap[ix]);
		printf("\n");
	}
	for (ix = 0U; ix < mwbmap_p->imaps; ix++) {
#if defined(BCM_MWBMAP_USE_CNTSETBITS)
		count = bcm_cntsetbits(mwbmap_p->id_bitmap[ix]);
#else  /* ! BCM_MWBMAP_USE_CNTSETBITS */
		count = mwbmap_p->wd_count[ix];
		MWBMAP_ASSERT(count == bcm_cntsetbits(mwbmap_p->id_bitmap[ix]));
#endif /* ! BCM_MWBMAP_USE_CNTSETBITS */
		printf("\tIDMAP:%2u. 0x%08x %02u\t", ix, mwbmap_p->id_bitmap[ix], count);
		bcm_bitprint32(mwbmap_p->id_bitmap[ix]);
		printf("\n");
	}

	return;
}

/* Audit a hierarchical multiword bitmap */
void
bcm_mwbmap_audit(struct bcm_mwbmap * mwbmap_hdl)
{
	bcm_mwbmap_t * mwbmap_p;
	uint32 count, free_cnt = 0U, wordix, idmap_ix, bitix, *bitmap_p;

	mwbmap_p = BCM_MWBMAP_PTR(mwbmap_hdl);

	for (wordix = 0U; wordix < mwbmap_p->wmaps; ++wordix) {

		bitmap_p = &mwbmap_p->wd_bitmap[wordix];

		for (bitix = 0U; bitix < BCM_MWBMAP_BITS_WORD; bitix++) {
			if ((*bitmap_p) & (1 << bitix)) {
				idmap_ix = BCM_MWBMAP_MULOP(wordix) + bitix;
#if defined(BCM_MWBMAP_USE_CNTSETBITS)
				count = bcm_cntsetbits(mwbmap_p->id_bitmap[idmap_ix]);
#else  /* ! BCM_MWBMAP_USE_CNTSETBITS */
				count = mwbmap_p->wd_count[idmap_ix];
				ASSERT(count == bcm_cntsetbits(mwbmap_p->id_bitmap[idmap_ix]));
#endif /* ! BCM_MWBMAP_USE_CNTSETBITS */
				ASSERT(count != 0U);
				free_cnt += count;
			}
		}
	}

	ASSERT((int)free_cnt == mwbmap_p->ifree);
}
/* END : Multiword bitmap based 64bit to Unique 32bit Id allocator. */

/* Simple 16bit Id allocator using a stack implementation. */
typedef struct id16_map {
	uint16  total;     /* total number of ids managed by allocator */
	uint16  start;     /* start value of 16bit ids to be managed */
	uint32  failures;  /* count of failures */
	void    *dbg;      /* debug placeholder */
	int     stack_idx; /* index into stack of available ids */
	uint16  stack[0];  /* stack of 16 bit ids */
} id16_map_t;

#define ID16_MAP_SZ(items)      (sizeof(id16_map_t) + \
	                             (sizeof(uint16) * (items)))

#if defined(BCM_DBG)

/* Uncomment BCM_DBG_ID16 to debug double free */
/* #define BCM_DBG_ID16 */

typedef struct id16_map_dbg {
	uint16  total;
	bool    avail[0];
} id16_map_dbg_t;
#define ID16_MAP_DBG_SZ(items)  (sizeof(id16_map_dbg_t) + \
	                             (sizeof(bool) * (items)))
#define ID16_MAP_MSG(x)         print x
#else
#define ID16_MAP_MSG(x)
#endif /* BCM_DBG */

void * /* Construct an id16 allocator: [start_val16 .. start_val16+total_ids) */
id16_map_init(osl_t *osh, uint16 total_ids, uint16 start_val16)
{
	uint16 idx, val16;
	id16_map_t * id16_map;

	ASSERT(total_ids > 0);
	ASSERT((start_val16 + total_ids) < ID16_INVALID);

	id16_map = (id16_map_t *) MALLOC(osh, ID16_MAP_SZ(total_ids));
	if (id16_map == NULL) {
		return NULL;
	}

	id16_map->total = total_ids;
	id16_map->start = start_val16;
	id16_map->failures = 0;
	id16_map->dbg = NULL;

	/* Populate stack with 16bit id values, commencing with start_val16 */
	id16_map->stack_idx = 0;
	val16 = start_val16;

	for (idx = 0; idx < total_ids; idx++, val16++) {
		id16_map->stack_idx = idx;
		id16_map->stack[id16_map->stack_idx] = val16;
	}

#if defined(BCM_DBG) && defined(BCM_DBG_ID16)
	id16_map->dbg = MALLOC(osh, ID16_MAP_DBG_SZ(total_ids));

	if (id16_map->dbg) {
		id16_map_dbg_t *id16_map_dbg = (id16_map_dbg_t *)id16_map->dbg;

		id16_map_dbg->total = total_ids;
		for (idx = 0; idx < total_ids; idx++) {
			id16_map_dbg->avail[idx] = TRUE;
		}
	}
#endif /* BCM_DBG && BCM_DBG_ID16 */

	return (void *)id16_map;
}

void * /* Destruct an id16 allocator instance */
id16_map_fini(osl_t *osh, void * id16_map_hndl)
{
	uint16 total_ids;
	id16_map_t * id16_map;

	if (id16_map_hndl == NULL)
		return NULL;

	id16_map = (id16_map_t *)id16_map_hndl;

	total_ids = id16_map->total;
	ASSERT(total_ids > 0);

#if defined(BCM_DBG) && defined(BCM_DBG_ID16)
	if (id16_map->dbg) {
		MFREE(osh, id16_map->dbg, ID16_MAP_DBG_SZ(total_ids));
		id16_map->dbg = NULL;
	}
#endif /* BCM_DBG && BCM_DBG_ID16 */

	id16_map->total = 0;
	MFREE(osh, id16_map, ID16_MAP_SZ(total_ids));

	return NULL;
}

uint16 BCMFASTPATH /* Allocate a unique 16bit id */
id16_map_alloc(void * id16_map_hndl)
{
	uint16 val16;
	id16_map_t * id16_map;

	ASSERT(id16_map_hndl != NULL);

	id16_map = (id16_map_t *)id16_map_hndl;

	ASSERT(id16_map->total > 0);

	if (id16_map->stack_idx < 0) {
		id16_map->failures++;
		return ID16_INVALID;
	}

	val16 = id16_map->stack[id16_map->stack_idx];
	id16_map->stack_idx--;

#if defined(BCM_DBG) && defined(BCM_DBG_ID16)

	ASSERT(val16 < (id16_map->start + id16_map->total));

	if (id16_map->dbg) { /* Validate val16 */
		id16_map_dbg_t *id16_map_dbg = (id16_map_dbg_t *)id16_map->dbg;

		ASSERT(id16_map_dbg->avail[val16 - id16_map->start] == TRUE);
		id16_map_dbg->avail[val16 - id16_map->start] = FALSE;
	}
#endif /* BCM_DBG && BCM_DBG_ID16 */

	return val16;
}


void BCMFASTPATH /* Free a 16bit id value into the id16 allocator */
id16_map_free(void * id16_map_hndl, uint16 val16)
{
	id16_map_t * id16_map;

	ASSERT(id16_map_hndl != NULL);

	id16_map = (id16_map_t *)id16_map_hndl;

#if defined(BCM_DBG) && defined(BCM_DBG_ID16)

	ASSERT(val16 < (id16_map->start + id16_map->total));

	if (id16_map->dbg) { /* Validate val16 */
		id16_map_dbg_t *id16_map_dbg = (id16_map_dbg_t *)id16_map->dbg;

		ASSERT(id16_map_dbg->avail[val16 - id16_map->start] == FALSE);
		id16_map_dbg->avail[val16 - id16_map->start] = TRUE;
	}
#endif /* BCM_DBG && BCM_DBG_ID16 */

	id16_map->stack_idx++;
	id16_map->stack[id16_map->stack_idx] = val16;
}

uint32 /* Returns number of failures to allocate an unique id16 */
id16_map_failures(void * id16_map_hndl)
{
	ASSERT(id16_map_hndl != NULL);
	return ((id16_map_t *)id16_map_hndl)->failures;
}

bool
id16_map_audit(void * id16_map_hndl)
{
	int idx;
	int insane = 0;
	id16_map_t * id16_map;

	ASSERT(id16_map_hndl != NULL);

	id16_map = (id16_map_t *)id16_map_hndl;

	ASSERT((id16_map->stack_idx > 0) && (id16_map->stack_idx < id16_map->total));
	for (idx = 0; idx <= id16_map->stack_idx; idx++) {
		ASSERT(id16_map->stack[idx] >= id16_map->start);
		ASSERT(id16_map->stack[idx] < (id16_map->start + id16_map->total));

#if defined(BCM_DBG) && defined(BCM_DBG_ID16)
		if (id16_map->dbg) {
			uint16 val16 = id16_map->stack[idx];
			if (((id16_map_dbg_t *)(id16_map->dbg))->avail[val16] != TRUE) {
				insane |= 1;
				ID16_MAP_MSG(("id16_map<%p>: stack_idx %u invalid val16 %u\n",
				              id16_map_hndl, idx, val16));
			}
		}
#endif /* BCM_DBG && BCM_DBG_ID16 */
	}

#if defined(BCM_DBG) && defined(BCM_DBG_ID16)
	if (id16_map->dbg) {
		uint16 avail = 0; /* Audit available ids counts */
		for (idx = 0; idx < id16_map_dbg->total; idx++) {
			if (((id16_map_dbg_t *)(id16_map->dbg))->avail[idx16] == TRUE)
				avail++;
		}
		if (avail && (avail != (id16_map->stack_idx + 1))) {
			insane |= 1;
			ID16_MAP_MSG(("id16_map<%p>: avail %u stack_idx %u\n",
			              id16_map_hndl, avail, id16_map->stack_idx));
		}
	}
#endif /* BCM_DBG && BCM_DBG_ID16 */

	return (!!insane);
}
/* END: Simple id16 allocator */


#endif /* BCMDRIVER */

/* calculate a >> b; and returns only lower 32 bits */
void
bcm_uint64_right_shift(uint32* r, uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low, r0 = 0;

	if (b == 0) {
		r0 = a_low;
		*r = r0;
		return;
	}

	if (b < 32) {
		a0 = a0 >> b;
		a1 = a1 & ((1 << b) - 1);
		a1 = a1 << (32 - b);
		r0 = a0 | a1;
		*r = r0;
		return;
	} else {
		r0 = a1 >> (b - 32);
		*r = r0;
		return;
	}

}

/* calculate a + b where a is a 64 bit number and b is a 32 bit number */
void
bcm_add_64(uint32* r_hi, uint32* r_lo, uint32 offset)
{
	uint32 r1_lo = *r_lo;
	(*r_lo) += offset;
	if (*r_lo < r1_lo)
		(*r_hi) ++;
}

/* calculate a - b where a is a 64 bit number and b is a 32 bit number */
void
bcm_sub_64(uint32* r_hi, uint32* r_lo, uint32 offset)
{
	uint32 r1_lo = *r_lo;
	(*r_lo) -= offset;
	if (*r_lo > r1_lo)
		(*r_hi) --;
}

#ifdef DEBUG_COUNTER
#if (OSL_SYSUPTIME_SUPPORT == TRUE)
void counter_printlog(counter_tbl_t *ctr_tbl)
{
	uint32 now;

	if (!ctr_tbl->enabled)
		return;

	now = OSL_SYSUPTIME();

	if (now - ctr_tbl->prev_log_print > ctr_tbl->log_print_interval) {
		uint8 i = 0;
		printf("counter_print(%s %d):", ctr_tbl->name, now - ctr_tbl->prev_log_print);

		for (i = 0; i < ctr_tbl->needed_cnt; i++) {
			printf(" %u", ctr_tbl->cnt[i]);
		}
		printf("\n");

		ctr_tbl->prev_log_print = now;
		bzero(ctr_tbl->cnt, CNTR_TBL_MAX * sizeof(uint));
	}
}
#else
/* OSL_SYSUPTIME is not supported so no way to get time */
#define counter_printlog(a) do {} while (0)
#endif /* OSL_SYSUPTIME_SUPPORT == TRUE */
#endif /* DEBUG_COUNTER */

#ifdef BCMDRIVER
void
BCMATTACHFN(dll_pool_detach)(void * osh, dll_pool_t * pool, uint16 elems_max, uint16 elem_size)
{
	uint32 mem_size;
	mem_size = sizeof(dll_pool_t) + (elems_max * elem_size);
	if (pool)
		MFREE(osh, pool, mem_size);
}
dll_pool_t *
BCMATTACHFN(dll_pool_init)(void * osh, uint16 elems_max, uint16 elem_size)
{
	uint32 mem_size, i;
	dll_pool_t * dll_pool_p;
	dll_t * elem_p;

	ASSERT(elem_size > sizeof(dll_t));

	mem_size = sizeof(dll_pool_t) + (elems_max * elem_size);

	if ((dll_pool_p = (dll_pool_t *)MALLOCZ(osh, mem_size)) == NULL) {
		printf("dll_pool_init: elems_max<%u> elem_size<%u> malloc failure\n",
			elems_max, elem_size);
		ASSERT(0);
		return dll_pool_p;
	}

	dll_init(&dll_pool_p->free_list);
	dll_pool_p->elems_max = elems_max;
	dll_pool_p->elem_size = elem_size;

	elem_p = dll_pool_p->elements;
	for (i = 0; i < elems_max; i++) {
		dll_append(&dll_pool_p->free_list, elem_p);
		elem_p = (dll_t *)((uintptr)elem_p + elem_size);
	}

	dll_pool_p->free_count = elems_max;

	return dll_pool_p;
}


void *
dll_pool_alloc(dll_pool_t * dll_pool_p)
{
	dll_t * elem_p;

	if (dll_pool_p->free_count == 0) {
		ASSERT(dll_empty(&dll_pool_p->free_list));
		return NULL;
	}

	elem_p = dll_head_p(&dll_pool_p->free_list);
	dll_delete(elem_p);
	dll_pool_p->free_count -= 1;

	return (void *)elem_p;
}

void
dll_pool_free(dll_pool_t * dll_pool_p, void * elem_p)
{
	dll_t * node_p = (dll_t *)elem_p;
	dll_prepend(&dll_pool_p->free_list, node_p);
	dll_pool_p->free_count += 1;
}


void
dll_pool_free_tail(dll_pool_t * dll_pool_p, void * elem_p)
{
	dll_t * node_p = (dll_t *)elem_p;
	dll_append(&dll_pool_p->free_list, node_p);
	dll_pool_p->free_count += 1;
}

#ifdef BCMDBG
void
dll_pool_dump(dll_pool_t * dll_pool_p, dll_elem_dump elem_dump)
{
	dll_t * elem_p;
	dll_t * next_p;
	printf("dll_pool<%p> free_count<%u> elems_max<%u> elem_size<%u>\n",
		dll_pool_p, dll_pool_p->free_count,
		dll_pool_p->elems_max, dll_pool_p->elem_size);

	for (elem_p = dll_head_p(&dll_pool_p->free_list);
		 !dll_end(&dll_pool_p->free_list, elem_p); elem_p = next_p) {

		next_p = dll_next_p(elem_p);
		printf("\telem<%p>\n", elem_p);
		if (elem_dump != NULL)
			elem_dump((void *)elem_p);
	}
}
#endif /* BCMDBG */
#endif /* BCMDRIVER */
