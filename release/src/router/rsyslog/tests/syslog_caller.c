/* A testing tool that just emits a number of
 * messages to the system log socket.
 *
 * Options
 *
 * -s severity (0..7 accoding to syslog spec, r "rolling", default 6)
 * -m number of messages to generate (default 500)
 * -C liblognorm-stdlog channel description
 * -f message format to use
 *
 * Part of the testbench for rsyslog.
 *
 * Copyright 2010-2018 Rainer Gerhards and Adiscon GmbH.
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
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_AIX)
	#include  <unistd.h>
#else
	#include <getopt.h>
#endif
#include <errno.h>
#include <string.h>
#include <syslog.h>
#ifdef HAVE_LIBLOGGING_STDLOG
#include <liblogging/stdlog.h>
#endif

static enum { FMT_NATIVE, FMT_SYSLOG_INJECT_L, FMT_SYSLOG_INJECT_C
	} fmt = FMT_NATIVE;

static void usage(void)
{
	fprintf(stderr, "usage: syslog_caller num-messages\n");
	exit(1);
}


#ifdef HAVE_LIBLOGGING_STDLOG
/* buffer must be large "enough" [4K?] */
static void
genMsg(char *buf, const int sev, const int iRun)
{
	switch(fmt) {
	case FMT_NATIVE:
		sprintf(buf, "test message nbr %d, severity=%d", iRun, sev);
		break;
	case FMT_SYSLOG_INJECT_L:
		sprintf(buf, "test\n");
		break;
	case FMT_SYSLOG_INJECT_C:
		sprintf(buf, "test 1\t2");
		break;
	}
}
#endif

int main(int argc, char *argv[])
{
	int i;
	int opt;
	int bRollingSev = 0;
	int sev = 6;
	int msgs = 500;
#ifdef HAVE_LIBLOGGING_STDLOG
	stdlog_channel_t logchan = NULL;
	const char *chandesc = "syslog:";
	char msgbuf[4096];
#endif

#ifdef HAVE_LIBLOGGING_STDLOG
	stdlog_init(STDLOG_USE_DFLT_OPTS);
	while((opt = getopt(argc, argv, "m:s:C:f:")) != -1) {
#else
	while((opt = getopt(argc, argv, "m:s:")) != -1) {
#endif
		switch (opt) {
		case 's':	if(*optarg == 'r') {
					bRollingSev = 1;
					sev = 0;
				} else
#ifdef HAVE_LIBLOGGING_STDLOG
					sev = atoi(optarg) % 8;
#else
					sev = atoi(optarg);
#endif
				break;
		case 'm':	msgs = atoi(optarg);
				break;
#ifdef HAVE_LIBLOGGING_STDLOG
		case 'C':	chandesc = optarg;
				break;
		case 'f':	if(!strcmp(optarg, "syslog_inject-l"))
					fmt = FMT_SYSLOG_INJECT_L;
				else if(!strcmp(optarg, "syslog_inject-c"))
					fmt = FMT_SYSLOG_INJECT_C;
				else
					usage();
				break;
#endif
		default:	usage();
#ifdef HAVE_LIBLOGGING_STDLOG
				exit(1);
#endif
				break;
		}
	}

#ifdef HAVE_LIBLOGGING_STDLOG
	if((logchan = stdlog_open(argv[0], 0, STDLOG_LOCAL1, chandesc)) == NULL) {
		fprintf(stderr, "error opening logchannel '%s': %s\n",
			chandesc, strerror(errno));
		exit(1);
	}
#endif
	for(i = 0 ; i < msgs ; ++i) {
#ifdef HAVE_LIBLOGGING_STDLOG
		genMsg(msgbuf, sev, i);
		if(stdlog_log(logchan, sev, "%s", msgbuf) != 0) {
			perror("error writing log record");
			exit(1);
		}
#else
		syslog(sev % 8, "test message nbr %d, severity=%d", i, sev % 8);
#endif
		if(bRollingSev)
#ifdef HAVE_LIBLOGGING_STDLOG
			sev = (sev + 1) % 8;
#else
		sev++;
#endif
	}
	return(0);
}
