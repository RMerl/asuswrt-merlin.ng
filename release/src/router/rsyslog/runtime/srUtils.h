/*! \file srUtils.h
 *  \brief General, small utilities that fit nowhere else.
 *
 * \author  Rainer Gerhards <rgerhards@adiscon.com>
 * \date    2003-09-09
 *          Coding begun.
 *
 * Copyright 2003-2016 Rainer Gerhards and Adiscon GmbH.
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
#ifndef __SRUTILS_H_INCLUDED__
#define __SRUTILS_H_INCLUDED__ 1

#include <stdlib.h>
#include <stdint.h>

/* syslog names */
#ifndef LOG_MAKEPRI
#	define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))
#endif
#define INTERNAL_NOPRI	0x10	/* the "no priority" priority */
#define TABLE_NOPRI	0	/* Value to indicate no priority in f_pmask */
#define TABLE_ALLPRI    0xFF    /* Value to indicate all priorities in f_pmask */
#define	LOG_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0) /* mark "facility" */

typedef struct syslogName_s {
	const char	*c_name;
	int	c_val;
} syslogName_t;

extern syslogName_t syslogPriNames[];
extern syslogName_t syslogFacNames[];

/**
 * A reimplementation of itoa(), as this is not available
 * on all platforms. We used the chance to make an interface
 * that fits us well, so it is no longer plain itoa().
 *
 * This method works with the US-ASCII alphabet. If you port this
 * to e.g. EBCDIC, you need to make a small adjustment. Keep in mind,
 * that on the wire it MUST be US-ASCII, so basically all you need
 * to do is replace the constant '0' with 0x30 ;).
 *
 * \param pBuf Caller-provided buffer that will receive the
 *              generated ASCII string.
 *
 * \param iLenBuf Length of the caller-provided buffer.
 *
 * \param iToConv The integer to be converted.
 */
rsRetVal srUtilItoA(char *pBuf, int iLenBuf, number_t iToConv);

/**
 * A method to duplicate a string for which the length is known.
 * Len must be the length in characters WITHOUT the trailing
 * '\0' byte.
 * rgerhards, 2007-07-10
 */
unsigned char *srUtilStrDup(unsigned char *pOld, size_t len);
/**
 * A method to create a directory and all its missing parents for
 * a given file name. Please not that the rightmost element is
 * considered to be a file name and thus NO directory is being created
 * for it.
 * added 2007-07-17 by rgerhards
 */
int makeFileParentDirs(const uchar *const szFile, const size_t lenFile, const mode_t mode,
	const uid_t uid, const gid_t gid, const int bFailOnChown);
int execProg(uchar *program, int bWait, uchar *arg);
void skipWhiteSpace(uchar **pp);
rsRetVal genFileName(uchar **ppName, uchar *pDirName, size_t lenDirName, uchar *pFName,
		     size_t lenFName, int64_t lNum, int lNumDigits);
int getNumberDigits(long lNum);
rsRetVal timeoutComp(struct timespec *pt, long iTimeout);
long timeoutVal(struct timespec *pt);
void mutexCancelCleanup(void *arg);
void srSleep(int iSeconds, int iuSeconds);
char *rs_strerror_r(int errnum, char *buf, size_t buflen);
int decodeSyslogName(uchar *name, syslogName_t *codetab);
int getSubString(uchar **ppSrc,  char *pDst, size_t DstSize, char cSep);
rsRetVal getFileSize(uchar *pszName, off_t *pSize);
int containsGlobWildcard(char *str);
void seedRandomNumber(void);
#define MAX_RANDOM_NUMBER RAND_MAX
long int randomNumber(void);
long long currentTimeMills(void);
rsRetVal ATTR_NONNULL() split_binary_parameters(uchar **const szBinary,
	char ***const aParams, int *const iParams, es_str_t *const param_binary);

/* mutex operations */
/* some useful constants */
#define DEFVARS_mutexProtection\
	int bLockedOpIsLocked=0
#define BEGIN_MTX_PROTECTED_OPERATIONS(mut, bMustLock) \
	if(bMustLock == LOCK_MUTEX) { \
		d_pthread_mutex_lock(mut); \
		assert(bLockedOpIsLocked == 0); \
		bLockedOpIsLocked = 1; \
	}
#define END_MTX_PROTECTED_OPERATIONS(mut) \
	if(bLockedOpIsLocked) { \
		d_pthread_mutex_unlock(mut); \
		bLockedOpIsLocked = 0; \
	}

#endif
