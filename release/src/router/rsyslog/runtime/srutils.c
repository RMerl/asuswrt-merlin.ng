/**\file srUtils.c
 * \brief General utilties that fit nowhere else.
 *
 * The namespace for this file is "srUtil".
 *
 * \author  Rainer Gerhards <rgerhards@adiscon.com>
 * \date    2003-09-09
 *          Coding begun.
 *
 * Copyright 2003-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include <ctype.h>
#include <inttypes.h>
#include <fcntl.h>

#include "rsyslog.h"
#include "srUtils.h"
#include "obj.h"
#include "errmsg.h"

#if _POSIX_TIMERS <= 0
#include <sys/time.h>
#endif

/* here we host some syslog specific names. There currently is no better place
 * to do it, but over here is also not ideal... -- rgerhards, 2008-02-14
 * rgerhards, 2008-04-16: note in LGPL move: the code tables below exist in
 * the same way in BSD, so it is not a problem to move them from GPLv3 to LGPL.
 * And nobody modified them since it was under LGPL, so we can also move it
 * to ASL 2.0.
 */
syslogName_t	syslogPriNames[] = {
	{"alert",	LOG_ALERT},
	{"crit",	LOG_CRIT},
	{"debug",	LOG_DEBUG},
	{"emerg",	LOG_EMERG},
	{"err",		LOG_ERR},
	{"error",	LOG_ERR},		/* DEPRECATED */
	{"info",	LOG_INFO},
	{"none",	INTERNAL_NOPRI},	/* INTERNAL */
	{"notice",	LOG_NOTICE},
	{"panic",	LOG_EMERG},		/* DEPRECATED */
	{"warn",	LOG_WARNING},		/* DEPRECATED */
	{"warning",	LOG_WARNING},
	{"*",		TABLE_ALLPRI},
	{NULL,		-1}
};

#ifndef LOG_AUTHPRIV
#	define LOG_AUTHPRIV LOG_AUTH
#endif
syslogName_t	syslogFacNames[] = {
	{"auth",         LOG_AUTH},
	{"authpriv",     LOG_AUTHPRIV},
	{"cron",         LOG_CRON},
	{"daemon",       LOG_DAEMON},
	{"kern",         LOG_KERN},
	{"lpr",          LOG_LPR},
	{"mail",         LOG_MAIL},
	{"mark",         LOG_MARK},		/* INTERNAL */
	{"news",         LOG_NEWS},
	{"ntp",          (12<<3) },             /* NTP, perhaps BSD-specific? */
	{"security",     LOG_AUTH},		/* DEPRECATED */
	{"bsd_security", (13<<3) },		/* BSD-specific, unfortunatly with duplicate name... */
	{"syslog",       LOG_SYSLOG},
	{"user",         LOG_USER},
	{"uucp",         LOG_UUCP},
#if defined(LOG_FTP)
	{"ftp",          LOG_FTP},
#endif
#if defined(LOG_AUDIT)
	{"audit",        LOG_AUDIT},
#endif
	{"console",	 (14 << 3)},		/* BSD-specific priority */
	{"local0",       LOG_LOCAL0},
	{"local1",       LOG_LOCAL1},
	{"local2",       LOG_LOCAL2},
	{"local3",       LOG_LOCAL3},
	{"local4",       LOG_LOCAL4},
	{"local5",       LOG_LOCAL5},
	{"local6",       LOG_LOCAL6},
	{"local7",       LOG_LOCAL7},
	{"invld",        LOG_INVLD},
	{NULL,           -1},
};

/* ################################################################# *
 * private members                                                   *
 * ################################################################# */

/* As this is not a "real" object, there won't be any private
 * members in this file.
 */

/* ################################################################# *
 * public members                                                    *
 * ################################################################# */

rsRetVal srUtilItoA(char *pBuf, int iLenBuf, number_t iToConv)
{
	int i;
	int bIsNegative;
	char szBuf[64];	/* sufficiently large for my lifespan and those of my children... ;) */

	assert(pBuf != NULL);
	assert(iLenBuf > 1);	/* This is actually an app error and as thus checked for... */

	if(iToConv < 0)
	{
		bIsNegative = RSTRUE;
		iToConv *= -1;
	}
	else
		bIsNegative = RSFALSE;

	/* first generate a string with the digits in the reverse direction */
	i = 0;
	do
	{
		szBuf[i++] = iToConv % 10 + '0';
		iToConv /= 10;
	} while(iToConv > 0);	/* warning: do...while()! */
	--i; /* undo last increment - we were pointing at NEXT location */

	/* make sure we are within bounds... */
	if(i + 2 > iLenBuf)	/* +2 because: a) i starts at zero! b) the \0 byte */
		return RS_RET_PROVIDED_BUFFER_TOO_SMALL;

	/* then move it to the right direction... */
	if(bIsNegative == RSTRUE)
		*pBuf++ = '-';
	while(i >= 0)
		*pBuf++ = szBuf[i--];
	*pBuf = '\0';	/* terminate it!!! */

	return RS_RET_OK;
}

uchar *srUtilStrDup(uchar *pOld, size_t len)
{
	uchar *pNew;

	assert(pOld != NULL);
	
	if((pNew = MALLOC(len + 1)) != NULL)
		memcpy(pNew, pOld, len + 1);

	return pNew;
}


/* creates a path recursively
 * Return 0 on success, -1 otherwise. On failure, errno * hold the last OS error.
 * Param "mode" holds the mode that all non-existing directories are to be
 * created with.
 * Note that we have a potential race inside that code, a race that even exists
 * outside of the rsyslog process (if multiple instances run, or other programs
 * generate directories): If the directory does not exist, a context switch happens,
 * at that moment another process creates it, then our creation on the context
 * switch back fails. This actually happened in practice, and depending on the
 * configuration it is even likely to happen. We can not solve this situation
 * with a mutex, as that works only within out process space. So the solution
 * is that we take the optimistic approach, try the creation, and if it fails
 * with "already exists" we go back and do one retry of the check/create
 * sequence. That should then succeed. If the directory is still not found but
 * the creation fails in the similar way, we return an error on that second
 * try because otherwise we would potentially run into an endless loop.
 * loop. -- rgerhards, 2010-03-25
 * The likeliest scenario for a prolonged contest of creating the parent directiories
 * is within our process space. This can happen with a high probability when two
 * threads, that want to start logging to files within same directory tree, are
 * started close to each other. We should fix what we can. -- nipakoo, 2017-11-25
 */
static int real_makeFileParentDirs(const uchar *const szFile, const size_t lenFile, const mode_t mode,
	const uid_t uid, const gid_t gid, const int bFailOnChownFail)
{
	uchar *p;
	uchar *pszWork;
	size_t len;

	assert(szFile != NULL);
	assert(lenFile > 0);

	len = lenFile + 1; /* add one for '\0'-byte */
	if((pszWork = MALLOC(len)) == NULL)
		return -1;
	memcpy(pszWork, szFile, len);
	for(p = pszWork+1 ; *p ; p++)
		if(*p == '/') {
			/* temporarily terminate string, create dir and go on */
			*p = '\0';
			int bErr = 0;
			if(mkdir((char*)pszWork, mode) == 0) {
				if(uid != (uid_t) -1 || gid != (gid_t) -1) {
					/* we need to set owner/group */
					if(chown((char*)pszWork, uid, gid) != 0) {
						LogError(errno, RS_RET_DIR_CHOWN_ERROR,
							"chown for directory '%s' failed", pszWork);
						if(bFailOnChownFail) {
							/* ignore if configured to do so */
							bErr = 1;
						}
					}
				}
			} else if(errno != EEXIST) {
				/* EEXIST is ok, means this component exists */
				bErr = 1;
			}

			if(bErr) {
				int eSave = errno;
				free(pszWork);
				errno = eSave;
				return -1;
			}
			*p = '/';
		}
	free(pszWork);
	return 0;
}
/* note: this small function is the stub for the brain-dead POSIX cancel handling */
int makeFileParentDirs(const uchar *const szFile, const size_t lenFile, const mode_t mode,
		       const uid_t uid, const gid_t gid, const int bFailOnChownFail)
{
	static pthread_mutex_t mutParentDir = PTHREAD_MUTEX_INITIALIZER;
	int r;	/* needs to be declared OUTSIDE of pthread_cleanup... macros! */
	pthread_mutex_lock(&mutParentDir);
	pthread_cleanup_push(mutexCancelCleanup, &mutParentDir);

	r = real_makeFileParentDirs(szFile, lenFile, mode, uid, gid, bFailOnChownFail);

	pthread_mutex_unlock(&mutParentDir);
	pthread_cleanup_pop(0);
	return r;
}


/* execute a program with a single argument
 * returns child pid if everything ok, 0 on failure. if
 * it fails, errno is set. if it fails after the fork(), the caller
 * can not be notfied for obvious reasons. if bwait is set to 1,
 * the code waits until the child terminates - that potentially takes
 * a lot of time.
 * implemented 2007-07-20 rgerhards
 */
int execProg(uchar *program, int bWait, uchar *arg)
{
	int pid;
	int sig;
	struct sigaction sigAct;

	dbgprintf("exec program '%s' with param '%s'\n", program, arg);
	pid = fork();
	if (pid < 0) {
		return 0;
	}

	if(pid) {       /* Parent */
		if(bWait)
			if(waitpid(pid, NULL, 0) == -1)
				if(errno != ECHILD) {
					/* we do not use logerror(), because
					 * that might bring us into an endless
					 * loop. At some time, we may
					 * reconsider this behaviour.
					 */
					dbgprintf("could not wait on child after executing '%s'",
					        (char*)program);
				}
		return pid;
	}
	/* Child */
	alarm(0); /* create a clean environment before we exec the real child */

	memset(&sigAct, 0, sizeof(sigAct));
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = SIG_DFL;

	for(sig = 1 ; sig < NSIG ; ++sig)
		sigaction(sig, &sigAct, NULL);

	execlp((char*)program, (char*) program, (char*)arg, NULL);
	/* In the long term, it's a good idea to implement some enhanced error
	 * checking here. However, it can not easily be done. For starters, we
	 * may run into endless loops if we log to syslog. The next problem is
	 * that output is typically not seen by the user. For the time being,
	 * we use no error reporting, which is quite consitent with the old
	 * system() way of doing things. rgerhards, 2007-07-20
	 */
	perror("exec");
	exit(1); /* not much we can do in this case */
}


/* skip over whitespace in a standard C string. The
 * provided pointer is advanced to the first non-whitespace
 * charater or the \0 byte, if there is none. It is never
 * moved past the \0.
 */
void skipWhiteSpace(uchar **pp)
{
	register uchar *p;

	assert(pp != NULL);
	assert(*pp != NULL);

	p = *pp;
	while(*p && isspace((int) *p))
		++p;
	*pp = p;
}


/* generate a file name from four parts:
 * <directory name>/<name>.<number>
 * If number is negative, it is not used. If any of the strings is
 * NULL, an empty string is used instead. Length must be provided.
 * lNumDigits is the minimum number of digits that lNum should have. This
 * is to pretty-print the file name, e.g. lNum = 3, lNumDigits= 4 will
 * result in "0003" being used inside the file name. Set lNumDigits to 0
 * to use as few space as possible.
 * rgerhards, 2008-01-03
 */
#if !defined(_AIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
rsRetVal genFileName(uchar **ppName, uchar *pDirName, size_t lenDirName, uchar *pFName,
		     size_t lenFName, int64_t lNum, int lNumDigits)
{
	DEFiRet;
	uchar *pName;
	uchar *pNameWork;
	size_t lenName;
	uchar szBuf[128];	/* buffer for number */
	char szFmtBuf[32];	/* buffer for snprintf format */
	size_t lenBuf;

	if(lNum < 0) {
		szBuf[0] = '\0';
		lenBuf = 0;
	} else {
		if(lNumDigits > 0) {
			snprintf(szFmtBuf, sizeof(szFmtBuf), ".%%0%d" PRId64, lNumDigits);
			lenBuf = snprintf((char*)szBuf, sizeof(szBuf), szFmtBuf, lNum);
		} else
			lenBuf = snprintf((char*)szBuf, sizeof(szBuf), ".%" PRId64, lNum);
	}

	lenName = lenDirName + 1 + lenFName + lenBuf + 1; /* last +1 for \0 char! */
	if((pName = MALLOC(lenName)) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	
	/* got memory, now construct string */
	memcpy(pName, pDirName, lenDirName);
	pNameWork = pName + lenDirName;
	*pNameWork++ = '/';
	memcpy(pNameWork, pFName, lenFName);
	pNameWork += lenFName;
	if(lenBuf > 0) {
		memcpy(pNameWork, szBuf, lenBuf);
		pNameWork += lenBuf;
	}
	*pNameWork = '\0';

	*ppName = pName;

finalize_it:
	RETiRet;
}
#if !defined(_AIX)
#pragma GCC diagnostic pop
#endif

/* get the number of digits required to represent a given number. We use an
 * iterative approach as we do not like to draw in the floating point
 * library just for log(). -- rgerhards, 2008-01-10
 */
int getNumberDigits(long lNum)
{
	int iDig;

	if(lNum == 0)
		iDig = 1;
	else
		for(iDig = 0 ; lNum != 0 ; ++iDig)
			lNum /= 10;

	return iDig;
}


/* compute an absolute time timeout suitable for calls to pthread_cond_timedwait()
 * iTimeout is in milliseconds
 * rgerhards, 2008-01-14
 */
rsRetVal
timeoutComp(struct timespec *pt, long iTimeout)
{
#	if _POSIX_TIMERS <= 0
	struct timeval tv;
#	endif

	BEGINfunc
	assert(pt != NULL);
	/* compute timeout */

#	if _POSIX_TIMERS > 0
	/* this is the "regular" code */
	clock_gettime(CLOCK_REALTIME, pt);
#	else
	gettimeofday(&tv, NULL);
	pt->tv_sec = tv.tv_sec;
	pt->tv_nsec = tv.tv_usec * 1000;
#	endif
	pt->tv_sec += iTimeout / 1000;
	pt->tv_nsec += (iTimeout % 1000) * 1000000; /* think INTEGER arithmetic! */
	if(pt->tv_nsec > 999999999) { /* overrun? */
		pt->tv_nsec -= 1000000000;
		++pt->tv_sec;
	}
	ENDfunc
	return RS_RET_OK; /* so far, this is static... */
}

long long
currentTimeMills(void)
{
	struct timespec tm;
#	if _POSIX_TIMERS <= 0
	struct timeval tv;
#	endif

#	if _POSIX_TIMERS > 0
	clock_gettime(CLOCK_REALTIME, &tm);
#	else
	gettimeofday(&tv, NULL);
	tm.tv_sec = tv.tv_sec;
	tm.tv_nsec = tv.tv_usec * 1000;
#	endif

	return ((long long) tm.tv_sec) * 1000 + (tm.tv_nsec / 1000000);
}


/* This function is kind of the reverse of timeoutComp() - it takes an absolute
 * timeout value and computes how far this is in the future. If the value is already
 * in the past, 0 is returned. The return value is in ms.
 * rgerhards, 2008-01-25
 */
long
timeoutVal(struct timespec *pt)
{
	struct timespec t;
	long iTimeout;
#	if _POSIX_TIMERS <= 0
	struct timeval tv;
#	endif

	BEGINfunc
	assert(pt != NULL);
	/* compute timeout */
#	if _POSIX_TIMERS > 0
	/* this is the "regular" code */
	clock_gettime(CLOCK_REALTIME, &t);
#	else
	gettimeofday(&tv, NULL);
	t.tv_sec = tv.tv_sec;
	t.tv_nsec = tv.tv_usec * 1000;
#	endif
	iTimeout = (pt->tv_nsec - t.tv_nsec) / 1000000;
	iTimeout += (pt->tv_sec - t.tv_sec) * 1000;

	if(iTimeout < 0)
		iTimeout = 0;

	ENDfunc
	return iTimeout;
}


/* cancellation cleanup handler - frees provided mutex
 * rgerhards, 2008-01-14
 */
void
mutexCancelCleanup(void *arg)
{
	BEGINfunc
	assert(arg != NULL);
	d_pthread_mutex_unlock((pthread_mutex_t*) arg);
	ENDfunc
}


/* rsSleep() - a fairly portable way to to sleep. It
 * will wake up when
 * a) the wake-time is over
 * rgerhards, 2008-01-28
 */
void
srSleep(int iSeconds, int iuSeconds)
{
	struct timeval tvSelectTimeout;

	BEGINfunc
	tvSelectTimeout.tv_sec = iSeconds;
	tvSelectTimeout.tv_usec = iuSeconds; /* micro seconds */
	select(0, NULL, NULL, NULL, &tvSelectTimeout);
	ENDfunc
}


/* From varmojfekoj's mail on why he provided rs_strerror_r():
 * There are two problems with strerror_r():
 * I see you've rewritten some of the code which calls it to use only
 * the supplied buffer; unfortunately the GNU implementation sometimes
 * doesn't use the buffer at all and returns a pointer to some
 * immutable string instead, as noted in the man page.
 *
 * The other problem is that on some systems strerror_r() has a return
 * type of int.
 *
 * So I've written a wrapper function rs_strerror_r(), which should
 * take care of all this and be used instead.
 *
 * Added 2008-01-30
 */
char *rs_strerror_r(int errnum, char *buf, size_t buflen) {
#ifndef HAVE_STRERROR_R
	char *pszErr;
	pszErr = strerror(errnum);
	snprintf(buf, buflen, "%s", pszErr);
#else
#	ifdef STRERROR_R_CHAR_P
		char *p = strerror_r(errnum, buf, buflen);
		if (p != buf) {
			strncpy(buf, p, buflen);
			buf[buflen - 1] = '\0';
		}
#	else
		strerror_r(errnum, buf, buflen);
#	endif
#endif /* #ifdef __hpux */
	return buf;
}


/*  Decode a symbolic name to a numeric value */
int decodeSyslogName(uchar *name, syslogName_t *codetab)
{
	register syslogName_t *c;
	register uchar *p;
	uchar buf[80];

	ASSERT(name != NULL);
	ASSERT(codetab != NULL);

	DBGPRINTF("symbolic name: %s", name);
	if(isdigit((int) *name)) {
		DBGPRINTF("\n");
		return (atoi((char*) name));
	}
	strncpy((char*) buf, (char*) name, 79);
	for(p = buf; *p; p++) {
		if (isupper((int) *p))
			*p = tolower((int) *p);
	}
	for(c = codetab; c->c_name; c++) {
		if(!strcmp((char*) buf, (char*) c->c_name)) {
			DBGPRINTF(" ==> %d\n", c->c_val);
			return (c->c_val);
		}
	}
	DBGPRINTF("\n");
	return (-1);
}


/**
 * getSubString
 *
 * Copy a string byte by byte until the occurrence
 * of a given separator.
 *
 * \param ppSrc		Pointer to a pointer of the source array of characters. If a
			separator detected the Pointer points to the next char after the
			separator. Except if the end of the string is dedected ('\n').
			Then it points to the terminator char.
 * \param pDst		Pointer to the destination array of characters. Here the substing
			will be stored.
 * \param DstSize	Maximum numbers of characters to store.
 * \param cSep		Separator char.
 * \ret int		Returns 0 if no error occured.
 *
 * rgerhards, 2008-02-12: some notes are due... I will once again fix this function, this time
 * so that it treats ' ' as a request for whitespace. But in general, the function and its callers
 * should be changed over time, this is not really very good code...
 */
int getSubString(uchar **ppSrc,  char *pDst, size_t DstSize, char cSep)
{
	uchar *pSrc = *ppSrc;
	int iErr = 0; /* 0 = no error, >0 = error */
	while((cSep == ' ' ? !isspace(*pSrc) : *pSrc != cSep) && *pSrc != '\n' && *pSrc != '\0' && DstSize>1) {
		*pDst++ = *(pSrc)++;
		DstSize--;
	}
	/* check if the Dst buffer was to small */
	if ((cSep == ' ' ? !isspace(*pSrc) : *pSrc != cSep) && *pSrc != '\n' && *pSrc != '\0') {
		dbgprintf("in getSubString, error Src buffer > Dst buffer\n");
		iErr = 1;
	}
	if (*pSrc == '\0' || *pSrc == '\n')
		/* this line was missing, causing ppSrc to be invalid when it
		 * was returned in case of end-of-string. rgerhards 2005-07-29
		 */
		*ppSrc = pSrc;
	else
		*ppSrc = pSrc+1;
	*pDst = '\0';
	return iErr;
}


/* get the size of a file or return appropriate error code. If an error is returned,
 * *pSize content is undefined.
 * rgerhards, 2009-06-12
 */
rsRetVal
getFileSize(uchar *pszName, off_t *pSize)
{
	int ret;
	struct stat statBuf;
	DEFiRet;

	ret = stat((char*) pszName, &statBuf);
	if(ret == -1) {
		switch(errno) {
			case EACCES: ABORT_FINALIZE(RS_RET_NO_FILE_ACCESS);
			case ENOTDIR:
			case ENOENT:  ABORT_FINALIZE(RS_RET_FILE_NOT_FOUND);
			default:      ABORT_FINALIZE(RS_RET_FILE_NO_STAT);
		}
	}

	*pSize = statBuf.st_size;

finalize_it:
	RETiRet;
}

/* Returns 1 if the given string contains a non-escaped glob(3)
 * wildcard character and 0 otherwise (or if the string is empty).
 */
int
containsGlobWildcard(char *str)
{
	char *p;
	if(!str) {
		return 0;
	}
	/* From Linux Programmer's Guide:
	 * "A string is a wildcard pattern if it contains one of the characters '?', '*', '{' or '['"
	 * "One can remove the special meaning of '?', '*', '{' and '[' by preceding them by a backslash"
	 */
	for(p = str; *p != '\0'; p++) {
		if((*p == '?' || *p == '*' || *p == '[' || *p == '{') &&
				(p == str || *(p-1) != '\\')) {
			return 1;
		}
	}
	return 0;
}

static void seedRandomInsecureNumber(void)
{
	struct timespec t;
	timeoutComp(&t, 0);
	long long x = t.tv_sec * 3 + t.tv_nsec * 2;
	srandom((unsigned int) x);
}

static long int randomInsecureNumber(void)
{
	return random();
}

#ifdef OS_LINUX
static int fdURandom = -1;
void seedRandomNumber(void)
{
	fdURandom = open("/dev/urandom", O_RDONLY);
	if(fdURandom == -1) {
		LogError(errno, RS_RET_IO_ERROR, "failed to seed random number generation,"
			" will use fallback (open urandom failed)");
		seedRandomInsecureNumber();
	}
}

long int randomNumber(void)
{
	long int ret;
	if(fdURandom >= 0) {
		if(read(fdURandom, &ret, sizeof(long int)) == -1) {
			LogError(errno, RS_RET_IO_ERROR, "failed to generate random number, will"
				" use fallback (read urandom failed)");
			ret = randomInsecureNumber();
		}
	} else {
		ret = randomInsecureNumber();
	}
	return ret;
}
#else
void seedRandomNumber(void)
{
	seedRandomInsecureNumber();
}

long int randomNumber(void)
{
	return randomInsecureNumber();
}
#endif


/* process "binary" parameters where this is needed to execute
 * programs (namely mmexternal and omprog).
 * Most importantly, split them into argv[] and get the binary name
 */
rsRetVal ATTR_NONNULL()
split_binary_parameters(uchar **const szBinary, char ***const __restrict__ aParams,
	int *const iParams, es_str_t *const param_binary)
{
	es_size_t iCnt;
	es_size_t iStr;
	int iPrm;
	es_str_t *estrParams = NULL;
	es_str_t *estrBinary = param_binary;
	es_str_t *estrTmp = NULL;
	uchar *c;
	int bInQuotes;
	DEFiRet;
	assert(iParams != NULL);
	assert(param_binary != NULL);

	/* Search for end of binary name */
	c = es_getBufAddr(param_binary);
	iCnt = 0;
	while(iCnt < es_strlen(param_binary) ) {
		if (c[iCnt] == ' ') {
			/* Split binary name from parameters */
			estrBinary = es_newStrFromSubStr( param_binary, 0, iCnt);
			estrParams = es_newStrFromSubStr( param_binary, iCnt+1,
					es_strlen(param_binary));
			break;
		}
		iCnt++;
	}
	*szBinary = (uchar*)es_str2cstr(estrBinary, NULL);
	DBGPRINTF("szBinary = '%s'\n", *szBinary);

	*iParams = 1; /* we always have argv[0] */
	/* count size of argv[] */
	if (estrParams != NULL) {
		 (*iParams)++; /* last parameter is not counted in loop below! */
		if(Debug) {
			char *params = es_str2cstr(estrParams, NULL);
			dbgprintf("szParams = '%s'\n", params);
			free(params);
		}
		c = es_getBufAddr(estrParams);
		for(iCnt = 0 ; iCnt < es_strlen(estrParams) ; ++iCnt) {
			if (c[iCnt] == ' ' && c[iCnt-1] != '\\')
				 (*iParams)++;
		}
	}
	DBGPRINTF("iParams %d (+1 for NULL terminator)\n", *iParams);

	/* create argv[] */
	CHKmalloc(*aParams = malloc((*iParams + 1) * sizeof(char*)));
	iPrm = 0;
	bInQuotes = FALSE;
	/* Set first parameter to binary */
	(*aParams)[iPrm] = strdup((char*)*szBinary);
	iPrm++;
	if (estrParams != NULL) {
		iCnt = iStr = 0;
		c = es_getBufAddr(estrParams); /* Reset to beginning */
		while(iCnt < es_strlen(estrParams) ) {
			if ( c[iCnt] == ' ' && !bInQuotes ) {
				estrTmp = es_newStrFromSubStr( estrParams, iStr, iCnt-iStr);
			} else if ( iCnt+1 >= es_strlen(estrParams) ) {
				estrTmp = es_newStrFromSubStr( estrParams, iStr, iCnt-iStr+1);
			} else if (c[iCnt] == '"') {
				bInQuotes = !bInQuotes;
			}

			if ( estrTmp != NULL ) {
				(*aParams)[iPrm] = es_str2cstr(estrTmp, NULL);
				iStr = iCnt+1; /* Set new start */
				DBGPRINTF("Param (%d): '%s'\n", iPrm, (*aParams)[iPrm]);
				es_deleteStr( estrTmp );
				estrTmp = NULL;
				iPrm++;
			}
			iCnt++;
		}
	}
	(*aParams)[iPrm] = NULL; /* NULL per argv[] convention */

finalize_it:
	if(estrBinary != param_binary) {
		es_deleteStr(estrBinary);
	}
	if(estrParams != NULL) {
		es_deleteStr(estrParams);
	}
	RETiRet;
}
