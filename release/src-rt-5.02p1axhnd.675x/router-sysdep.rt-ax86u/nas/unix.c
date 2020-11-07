/*
 * Unix support routines
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: unix.c 241182 2011-02-17 21:50:03Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <string.h>

void initopt(void);
int getopt(int argc, char **argv, char *ostr);

/* ****************************************** */
/* Some support functionality to match unix */

/* get option letter from argument vector  */
int opterr = 1;
int optind = 1;
int optopt;                     /* character checked for validity */
char *optarg;                /* argument associated with option */

#define EMSG	""
#define BADCH ('?')

static char *place = EMSG;        /* option letter processing */

void initopt(void)
{
	opterr = 1;
	optind = 1;
	place = EMSG;        /* option letter processing */
}

int getopt(int argc, char **argv, char *ostr)
{
	register char *oli;                        /* option letter list index */

	if (!*place) {
		/* update scanning pointer */
		if (optind >= argc || *(place = argv[optind]) != '-' || !*++place) {
			return EOF;
		}
		if (*place == '-') {
			/* found "--" */
			++optind;
			return EOF;
		}
	}

	/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, optopt))) {
		if (!*place) {
			++optind;
		}
		printf("illegal option");
		return BADCH;
	}
	if (*++oli != ':') {
		/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	} else {
		/* need an argument */
		if (*place) {
			optarg = place;                /* no white space */
		} else  if (argc <= ++optind) {
			/* no arg */
			place = EMSG;
			printf("option requires an argument");
			return BADCH;
		} else {
			optarg = argv[optind];                /* white space */
		}
		place = EMSG;
		++optind;
	}
	return optopt; /* return option letter */
}
