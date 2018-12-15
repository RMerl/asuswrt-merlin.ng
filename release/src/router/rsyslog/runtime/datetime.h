/* The datetime object. Contains time-related functions.
 *
 * Copyright 2008-2015 Adiscon GmbH.
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
#ifndef INCLUDED_DATETIME_H
#define INCLUDED_DATETIME_H

/* TODO: define error codes */
#define NO_ERRCODE -1

/* the datetime object */
typedef struct datetime_s {
	char dummy;
} datetime_t;

typedef enum {
	DATE_INVALID = -1,
	DATE_RFC3164 =  0,
	DATE_RFC3339 =  1,
	DATE_UNIX    =  2,
} dateTimeFormat_t;

/* interfaces */
BEGINinterface(datetime) /* name must also be changed in ENDinterface macro! */
	void (*getCurrTime)(struct syslogTime *t, time_t *ttSeconds, const int inUTC);
	rsRetVal (*ParseTIMESTAMP3339)(struct syslogTime *pTime, uchar** ppszTS, int*);
	rsRetVal (*ParseTIMESTAMP3164)(struct syslogTime *pTime, uchar** pszTS, int*, const int bParseTZ,
const int bDetectYearAfterTime);
	int (*formatTimestampToMySQL)(struct syslogTime *ts, char* pDst);
	int (*formatTimestampToPgSQL)(struct syslogTime *ts, char *pDst);
	int (*formatTimestamp3339)(struct syslogTime *ts, char* pBuf);
	int (*formatTimestamp3164)(struct syslogTime *ts, char* pBuf, int);
	int (*formatTimestampSecFrac)(struct syslogTime *ts, char* pBuf);
	/* v3, 2009-11-12 */
	time_t (*GetTime)(time_t *ttSeconds);
	/* v6, 2011-06-20 , v10, 2016-01-12*/
	void (*timeval2syslogTime)(struct timeval *tp, struct syslogTime *t, const int inUTC);
	/* v7, 2012-03-29 */
	int (*formatTimestampUnix)(struct syslogTime *ts, char*pBuf);
	time_t (*syslogTime2time_t)(const struct syslogTime *ts);
	/* v11, 2017-10-05 */
	int (*formatUnixTimeFromTime_t)(time_t time, const char *format, char *pBuf, uint pBufMax);
ENDinterface(datetime)
#define datetimeCURR_IF_VERSION 11 /* increment whenever you change the interface structure! */
/* interface changes:
 * 1 - initial version
 * 2 - not compatible to 1 - bugfix required ParseTIMESTAMP3164 to accept char ** as
 *     last parameter. Did not try to remain compatible as this is not something any
 *     third-party module should call. -- rgerhards, 2008.-09-12
 * 3 - taken by v5 branch!
 * 4 - formatTimestamp3164 takes a third int parameter
 * 5 - merge of versions 3 + 4 (2010-03-09)
 * 6 - see above
 * 8 - ParseTIMESTAMP3164 has addtl parameter to permit TZ string parsing
 * 9 - ParseTIMESTAMP3164 has addtl parameter to permit year parsing
 * 10 - functions having addtl paramater inUTC to emit time in UTC:
 *      timeval2syslogTime, getCurrtime
 * 11 - Add formatUnixTimeFromTime_t
 */

#define PARSE3164_TZSTRING 1
#define NO_PARSE3164_TZSTRING 0

#define PERMIT_YEAR_AFTER_TIME 1
#define NO_PERMIT_YEAR_AFTER_TIME 0

/* two defines for functions that create timestamps either in local
 * time or UTC.
 */
#define TIME_IN_UTC 1
#define TIME_IN_LOCALTIME 0

/* prototypes */
PROTOTYPEObj(datetime);
void applyDfltTZ(struct syslogTime *pTime, char *tz);
int getWeekdayNbr(struct syslogTime *ts);
int getOrdinal(struct syslogTime *ts);
int getWeek(struct syslogTime *ts);
void timeConvertToUTC(const struct syslogTime *const __restrict__ local, struct syslogTime *const __restrict__ utc);
time_t getTime(time_t *ttSeconds);
dateTimeFormat_t getDateTimeFormatFromStr(const char * const __restrict__ s);

#endif /* #ifndef INCLUDED_DATETIME_H */
