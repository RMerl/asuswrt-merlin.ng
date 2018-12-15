/* syslogd-type.h
 * This file contains type defintions used by syslogd and its modules.
 * It is a required input for any module.
 *
 * File begun on 2007-07-13 by RGerhards (extracted from syslogd.c)
 *
 * Copyright 2007-2018 Adiscon GmbH.
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
#ifndef	SYSLOGD_TYPES_INCLUDED
#define	SYSLOGD_TYPES_INCLUDED 1

#include "stringbuf.h"
#include <sys/param.h>

/* we use RSTRUE/FALSE to prevent name claches with other packages */
#define RSFALSE 0
#define RSTRUE 1

#define MAXFNAME	4096	/* max file pathname length */

#define	_DB_MAXDBLEN	128	/* maximum number of db */
#define _DB_MAXUNAMELEN	128	/* maximum number of user name */
#define	_DB_MAXPWDLEN	128 	/* maximum number of user's pass */
#define _DB_DELAYTIMEONERROR	20	/* If an error occur we stop logging until
					   a delayed time is over */


/* we define features of the syslog code. This features can be used
 * to check if modules are compatible with them - and possible other
 * applications I do not yet envision. -- rgerhards, 2007-07-24
 */
typedef enum _syslogFeature {
	sFEATURERepeatedMsgReduction = 1,	/* for output modules */
	sFEATURENonCancelInputTermination = 2,	/* for input modules */
	sFEATUREAutomaticSanitazion = 3,	/* for parser modules */
	sFEATUREAutomaticPRIParsing = 4		/* for parser modules */
} syslogFeature;

/* we define our own facility and severities */
/* facility and severity codes */
typedef struct _syslogCode {
	char    *c_name;
	int     c_val;
} syslogCODE;

/* values for host comparisons specified with host selector blocks
 * (+host, -host). rgerhards 2005-10-18.
 */
enum _EHostnameCmpMode {
	HN_NO_COMP = 0, /* do not compare hostname */
	HN_COMP_MATCH = 1, /* hostname must match */
	HN_COMP_NOMATCH = 2 /* hostname must NOT match */
};
typedef enum _EHostnameCmpMode EHostnameCmpMode;

/* time type numerical values for structure below */
#define TIME_TYPE_UNINIT	0
#define TIME_TYPE_RFC3164	1
#define TIME_TYPE_RFC5424	2
/* rgerhards 2004-11-11: the following structure represents
 * a time as it is used in syslog.
 * rgerhards, 2009-06-23: packed structure for better cache performance
 * (but left ultimate decision about packing to compiler)
 */
struct syslogTime {
	intTiny timeType;	/* 0 - unitinialized , 1 - RFC 3164, 2 - syslog-protocol */
	intTiny month;
	intTiny day;
	intTiny hour; /* 24 hour clock */
	intTiny minute;
	intTiny second;
	intTiny secfracPrecision;
	intTiny OffsetMinute;	/* UTC offset in minutes */
	intTiny OffsetHour;	/* UTC offset in hours
				 * full UTC offset minutes = OffsetHours*60 + OffsetMinute. Then use
				 * OffsetMode to know the direction.
				 */
	char OffsetMode;	/* UTC offset + or - */
	short year;
	int secfrac;	/* fractional seconds (must be 32 bit!) */
	intTiny inUTC;	/* forced UTC? */
};
typedef struct syslogTime syslogTime_t;

struct tzinfo {
	char *id;
	char offsMode;
	int8_t offsHour;
	int8_t offsMin;
};
typedef struct tzinfo tzinfo_t;

typedef enum 	{ ACT_STRING_PASSING = 0, ACT_ARRAY_PASSING = 1, ACT_MSG_PASSING = 2,
	  ACT_JSON_PASSING = 3} paramPassing_t;

#endif /* #ifndef SYSLOGD_TYPES_INCLUDED */
/* vi:set ai:
 */
