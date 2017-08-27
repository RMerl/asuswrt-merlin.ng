/* Analyzes the concurrent use of charon's threads
 *
 * Copyright (C) 2008 Andreas Steffen
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGFILE		"moon.daemon.log"
#define LINE_LEN	2048
#define THREADS		99

typedef enum state_t state_t;

enum state_t {
	STATE_IDLE  = 0,
	STATE_INIT  = 1,
	STATE_AUTH  = 2,
	STATE_BUSY  = 3,
	STATE_RETRY = 4,
	STATE_ERROR = 5
};

typedef enum print_t print_t;

enum print_t {
	MODE_ANY	= 0,
	MODE_ADD	= 1,
	MODE_DEL	= 2
};

static char *state_names[] = { "idle", "init", "auth", "busy", "retry", "error" };

static int readline(FILE *fd, char *line)
{
	while (fread(line, 1, 1, fd))
	{
		if (*line == '\n')
		{
			*line = '\0';
			return 1;
		}
		line++;
	}
	*line = '\0';
	return 0;
}

static void printline(state_t *state, char *timestamp)
{
	int states[] = { 0, 0, 0, 0, 0};
	int th, total ;

	printf("    <tr>\n");
	printf("      <td class=\"log\">%.15s</td>", timestamp);

	for (th = 1; th <= THREADS; th++)
	{
		states[state[th]]++;
		printf("<td class=\"%s\"></td>", state_names[state[th]]);
	}
	total = states[STATE_INIT] + states[STATE_AUTH] + states[STATE_BUSY] + states[STATE_RETRY];
	printf("<td class=\"init\">%d</td><td class=\"auth\">%d</td><td class=\"busy\">%d</td>",
			states[STATE_INIT], states[STATE_AUTH], total);
	for (th = 10; th <= (THREADS + 2); th += 5)
	{
		printf("<td class=\"%s\"></td>", (th <= total + 2)? "busy":"idle");
	}
	printf("\n");
	printf("    </tr>\n");
}

int main(int argc, char *argv[])
{
	char line[LINE_LEN];
	int section = 0;
	int mode = MODE_ANY;
	int th;
	FILE *fd;

	state_t state[THREADS + 1];

	/* threads 1..5 and 9 are always busy */
	for (th = 1; th <= THREADS; th++)
	{
		state[th] = (th <= 7 && th != 3)? STATE_BUSY : STATE_IDLE;
	}

	/* open the log file */
	fd = fopen(LOGFILE, "r");
	if (!fd)
	{
		printf("could not open log file '%s'\n", LOGFILE);
		return 1;
	}

	printf("<html>\n");
	printf("<head>\n");
	printf("  <title>Charon Thread Analysis</title>\n");
	printf("  <style>\n");
	printf("    body     { font: 11px verdana,arial, helvetica, sans-serif }\n");
	printf("    td       { font: 10px verdana,arial, helvetica, sans-serif;\n");
	printf("               text-align: center;  padding: 0px 1px 0px 1px; background: #FFFF66 }\n");
	printf("    td.log   { text-align: left; background: #FFFF66; white-space: nowrap }\n");
	printf("    td.idle  { background: #FFFF66 }\n");
	printf("    td.init  { background: #FFA522 }\n");
	printf("    td.auth  { background: #90EE90 }\n");
	printf("    td.busy  { background: #ADD8E6 }\n");
	printf("    td.retry { background: #7B68EE }\n");
	printf("    td.error { background: #FF6347 }\n");
	printf("    hr { background-color: #000000; border: 0; height: 1px; }\n");
	printf("    a:visited { color: #000000 }\n");
    printf("    a:link    { color: #000000 }\n");
	printf("    a:hover   { color: #0000FF }\n");
	printf("  </style>\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("  <h1>Charon Thread Analysis</h1>\n");
	printf("  <table>\n");

	/* print table header */
	printf("    <tr>\n");
	printf("      <td class=\"log\">Timestamp</td>");
	for (th = 1 ; th <= THREADS; th++)
	{
		printf("<td>%02d</td>", th);
	}
	printf("<td class=\"init\">I</td><td class=\"auth\">A</td><td class=\"busy\">B</td>");
	for (th = 10; th <= (THREADS + 2); th += 5)
	{
		printf("<td class=\"busy\">%d</td>", (th == 100)? 99:th);
	}
	printf("\n");
	printf("    </tr>\n");

	while (readline(fd, line))
	{
		char *p_section, *p_charon, *p_thread, *p_log;

		p_section = strstr(line, "---");
		if (p_section)
		{
			printline(state, line);
			mode = MODE_ANY;

			if (section++ < 1)
			{
				continue;
			}
			else
			{
				break;
			}
		}

		p_charon = strstr(line, "charon");
		if (!p_charon)
		{
			continue;
		}

		/* determine thread */
		p_thread = p_charon + 8;
		th = atol(p_thread);

		/* determine log message */
		p_log = p_charon + 16;
		if (strstr(p_log, "received packet"))
		{
			if (mode == MODE_DEL)
			{
				printline(state, line);
			}
			mode = MODE_ADD;
			if (state[th] != STATE_IDLE)
			{
				state[th] = STATE_ERROR;
			}
			else
			{
				state[th] = STATE_BUSY;
			}
		}
		if (strstr(p_log, "sending packet"))
		{
			if (mode == MODE_ADD)
			{
				printline(state, line);
			}
			mode = MODE_DEL;
			if (state[th] == STATE_IDLE)
			{
				state[th] = STATE_ERROR;
			}
			else
			{
				state[th] = STATE_IDLE;
			}
		}
		if (strstr(p_log, "parsed IKE_SA_INIT request"))
		{
			if (state[th] != STATE_BUSY)
			{
				state[th] = STATE_ERROR;
			}
			else
			{
				state[th] = STATE_INIT;
			}
		}
		if (strstr(p_log, "parsed IKE_AUTH request"))
		{
			if (state[th] != STATE_BUSY)
			{
				state[th] = STATE_ERROR;
			}
			else
			{
				state[th] = STATE_AUTH;
			}
		}
		if (strstr(p_log, "already processing"))
		{
			if (state[th] != STATE_IDLE)
			{
				state[th] = STATE_ERROR;
			}
			else
			{
				state[th] = STATE_RETRY;
			}
			printline(state, line);
			mode = MODE_ANY;
			state[th] = STATE_IDLE;
		}
	}
	printf("  </table>\n");
	printf("  <p>\n");
	printf("  <table>\n");
	printf("    <tr>\n");
	printf("      <td class=\"init\">&nbsp;I&nbsp;</td><td>IKE_SA_INIT Thread &nbsp;</td>\n");
	printf("      <td class=\"auth\">&nbsp;A&nbsp;</td><td>IKE_AUTH Thread &nbsp;</td>\n");
	printf("      <td class=\"retry\">&nbsp;R&nbsp;</td><td>Retransmit Thread &nbsp;</td>\n");
	printf("      <td class=\"busy\">&nbsp;B&nbsp;</td><td>Busy Thread &nbsp;</td>\n");
	printf("      <td class=\"error\">&nbsp;E&nbsp;</td><td>State Error &nbsp;</td>\n");
	printf("    </tr>\n");
	printf("  </table>\n");
	printf("  <p>\n");
	printf("  <hr/>\n");
	printf("  <em>&copy; 2008\n");
	printf("    <a href=\"http://ita.hsr.ch?&L=1\" target=\"popup\">\n");
	printf("       ITA Institute for Internet Technologies and Applications</a> -\n");
	printf("    <a href=\"http://www.hsr.ch/?&L=1\" target=\"popup\">\n");
	printf("       HSR Hochschule f&uuml;r Technik Rapperswil</a>\n");
	printf("  </em>\n");
	printf("</body>\n");
	printf("</html>\n");

	fclose(fd);
	return 0;
}
