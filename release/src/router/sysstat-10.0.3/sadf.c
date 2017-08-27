/*
 * sadf: system activity data formatter
 * (C) 1999-2011 by Sebastien GODARD (sysstat <at> orange.fr)
 *
 ***************************************************************************
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published  by  the *
 * Free Software Foundation; either version 2 of the License, or (at  your *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it  will  be  useful,  but *
 * WITHOUT ANY WARRANTY; without the implied warranty  of  MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License *
 * for more details.                                                       *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                   *
 ***************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "version.h"
#include "sadf.h"
#include "sa.h"
#include "common.h"
#include "ioconf.h"

#ifdef USE_NLS
# include <locale.h>
# include <libintl.h>
# define _(string) gettext(string)
#else
# define _(string) (string)
#endif

#define SCCSID "@(#)sysstat-" VERSION ": " __FILE__ " compiled " __DATE__ " " __TIME__
char *sccsid(void) { return (SCCSID); }

long interval = -1, count = 0;

unsigned int flags = 0;
unsigned int dm_major;		/* Device-mapper major number */
unsigned int format = 0;	/* Output format */
unsigned int f_position = 0;	/* Output format position in array */

/* File header */
struct file_header file_hdr;

static char *seps[] =  {"\t", ";"};

/*
 * Activity sequence.
 * This array must always be entirely filled (even with trailing zeros).
 */
unsigned int id_seq[NR_ACT];

/* Current record header */
struct record_header record_hdr[3];

/* Contain the date specified by -s and -e options */
struct tstamp tm_start, tm_end;
char *args[MAX_ARGV_NR];

extern struct activity *act[];
extern struct report_format *fmt[];

/*
 ***************************************************************************
 * Print usage and exit.
 *
 * IN:
 * @progname	Name of sysstat command.
 ***************************************************************************
 */
void usage(char *progname)
{
	fprintf(stderr,
		_("Usage: %s [ options ] [ <interval> [ <count> ] ] [ <datafile> ]\n"),
		progname);

	fprintf(stderr, _("Options are:\n"
			  "[ -d | -j | -p | -x ] [ -C ] [ -H ] [ -h ] [ -T ] [ -t ] [ -V ]\n"
			  "[ -P { <cpu> [,...] | ALL } ] [ -s [ <hh:mm:ss> ] ] [ -e [ <hh:mm:ss> ] ]\n"
			  "[ -- <sar_options> ]\n"));
	exit(1);
}

/*
 ***************************************************************************
 * Init structures.
 ***************************************************************************
 */
void init_structures(void)
{
	int i;
	
	for (i = 0; i < 3; i++) {
		memset(&record_hdr[i], 0, RECORD_HEADER_SIZE);
	}
}

/*
 ***************************************************************************
 * Look for output format in array.
 *
 * IN:
 * @fmt		Array of output formats.
 * @format	Output format to look for.
 *
 * RETURNS:
 * Position of output format in array.
 ***************************************************************************
 */
int get_format_position(struct report_format *fmt[], unsigned int format)
{
        int i;

        for (i = 0; i < NR_FMT; i++) {
                if (fmt[i]->id == format)
                        break;
        }

        if (i == NR_FMT)
		/* Should never happen */
                return 0;

        return i;
}

/*
 ***************************************************************************
 * Check that options entered on the command line are consistent with
 * selected output format. If no output format has been explicitly entered,
 * then select a default one.
 ***************************************************************************
 */
void check_format_options(void)
{
	if (!format) {
		/* Select output format if none has been selected */
		if (DISPLAY_HDR_ONLY(flags)) {
			format = F_HEADER_OUTPUT;
		}
		else {
			format = F_PPC_OUTPUT;
		}
	}

	/* Get format position in array */
	f_position = get_format_position(fmt, format);
	
	/* Check options consistency wrt output format */
	if (!ACCEPT_HEADER_ONLY(fmt[f_position]->options)) {
		/* Remove option -H */
		flags &= ~S_F_HDR_ONLY;
	}
	if (!ACCEPT_HORIZONTALLY(fmt[f_position]->options)) {
		/* Remove option -h */
		flags &= ~S_F_HORIZONTALLY;
	}
	if (!ACCEPT_TRUE_TIME(fmt[f_position]->options)) {
		/* Remove option -t */
		flags &= ~S_F_TRUE_TIME;
	}
	if (!ACCEPT_SEC_EPOCH(fmt[f_position]->options)) {
		/* Remove option -T */
		flags &= ~S_F_SEC_EPOCH;
	}
}

/*
 ***************************************************************************
 * Fill the rectime and loctime structures with current record's date and
 * time, based on current record's "number of seconds since the epoch" saved
 * in file.
 * The resulting timestamp is expressed in UTC or in local time, depending
 * on whether option -t has been used or not.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @rectime	Structure where timestamp (expressed in local time or in UTC
 *		depending on whether option -t has been used or not) can be
 *		saved for current record.
 * @loctime	Structure where timestamp (expressed in local time) can be
 *		saved for current record.
 *
 * OUT:
 * @rectime	Structure where timestamp for current record has been saved.
 * @loctime	Structure where timestamp for current record has been saved.
 ***************************************************************************
*/
void sadf_get_record_timestamp_struct(int curr, struct tm *rectime, struct tm *loctime)
{
	struct tm *ltm;

	if ((ltm = localtime((const time_t *) &record_hdr[curr].ust_time)) != NULL) {
		*loctime = *ltm;
	}

	if (!PRINT_TRUE_TIME(flags)) {
		/* Option -t not used: Display timestamp in UTC */
		ltm = gmtime((const time_t *) &record_hdr[curr].ust_time);
	}

	if (ltm) {
		*rectime = *ltm;
	}
}

/*
 ***************************************************************************
 * Set current record's timestamp strings (date and time). This timestamp is
 * expressed in UTC or in local time, depending on whether option -t has
 * been used or not.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @cur_date	String where timestamp's date will be saved.
 * @cur_time	String where timestamp's time will be saved.
 * @len		Maximum length of timestamp strings.
 * @rectime	Structure with current timestamp (expressed in local time or
 *		in UTC depending on whether option -t has been used or not)
 *		that should be broken down in date and time strings.
 *
 * OUT:
 * @cur_date	Timestamp's date string.
 * @cur_time	Timestamp's time string. May contain the number of seconds
 *		since the epoch (01-01-1970) if option -T has been used.
 ***************************************************************************
*/
void set_record_timestamp_string(int curr, char *cur_date, char *cur_time, int len,
				 struct tm *rectime)
{
	/* Set cur_time date value */
	if (PRINT_SEC_EPOCH(flags)) {
		sprintf(cur_time, "%ld", record_hdr[curr].ust_time);
		strcpy(cur_date, "");
	}
	else {
		/*
		 * If PRINT_TRUE_TIME(flags) is true (ie. option -t has been used) then
		 * cur_time is expressed in local time. Else it is expressed in UTC.
		 */
		strftime(cur_date, len, "%Y-%m-%d", rectime);
		strftime(cur_time, len, "%H-%M-%S", rectime);
	}
}

/*
 ***************************************************************************
 * Print tabulations
 *
 * IN:
 * @nr_tab	Number of tabs to print.
 ***************************************************************************
 */
void prtab(int nr_tab)
{
	int i;

	for (i = 0; i < nr_tab; i++) {
		printf("\t");
	}
}

/*
 ***************************************************************************
 * printf() function modified for textual (XML-like) display. Don't print a
 * CR at the end of the line.
 *
 * IN:
 * @nr_tab	Number of tabs to print.
 * @fmtf	printf() format.
 ***************************************************************************
 */
void xprintf0(int nr_tab, const char *fmtf, ...)
{
	static char buf[1024];
	va_list args;

	va_start(args, fmtf);
	vsnprintf(buf, sizeof(buf), fmtf, args);
	va_end(args);

	prtab(nr_tab);
	printf("%s", buf);
}

/*
 ***************************************************************************
 * printf() function modified for textual (XML-like) display. Print a CR
 * at the end of the line.
 *
 * IN:
 * @nr_tab	Number of tabs to print.
 * @fmtf	printf() format.
 ***************************************************************************
 */
void xprintf(int nr_tab, const char *fmtf, ...)
{
	static char buf[1024];
	va_list args;

	va_start(args, fmtf);
	vsnprintf(buf, sizeof(buf), fmtf, args);
	va_end(args);

	prtab(nr_tab);
	printf("%s\n", buf);
}

/*
 ***************************************************************************
 * Display restart records for textual (XML-like) reports.
 *
 * IN:
 * @curr		Index in array for current sample statistics.
 * @use_tm_start	Set to TRUE if option -s has been used.
 * @use_tm_end		Set to TRUE if option -e has been used.
 * @tab			Number of tabulations to print.
 * @rectime		Structure where timestamp (expressed in local time
 *			or in UTC depending on whether option -t has been
 *			used or not) can be saved for current record.
 * @loctime		Structure where timestamp (expressed in local time)
 *			can be saved for current record.
 *
 * OUT:
 * @rectime		Structure where timestamp for current record has
 *			been saved.
 * @loctime		Structure where timestamp for current record has
 *			been saved.
 ***************************************************************************
 */
void write_textual_restarts(int curr, int use_tm_start, int use_tm_end, int tab,
			    struct tm *rectime, struct tm *loctime)
{
	char cur_date[32], cur_time[32];

	/* Fill timestamp structure for current record */
	sadf_get_record_timestamp_struct(curr, rectime, loctime);

	/* The record must be in the interval specified by -s/-e options */
	if ((use_tm_start && (datecmp(loctime, &tm_start) < 0)) ||
	    (use_tm_end && (datecmp(loctime, &tm_end) > 0)))
		return;

	set_record_timestamp_string(curr, cur_date, cur_time, 32, rectime);

	if (*fmt[f_position]->f_restart) {
		(*fmt[f_position]->f_restart)(&tab, F_MAIN, cur_date, cur_time,
					      !PRINT_TRUE_TIME(flags), &file_hdr);
	}
}

/*
 ***************************************************************************
 * Display COMMENT records for textual (XML-like) reports.
 *
 * IN:
 * @curr		Index in array for current sample statistics.
 * @use_tm_start	Set to TRUE if option -s has been used.
 * @use_tm_end		Set to TRUE if option -e has been used.
 * @tab			Number of tabulations to print.
 * @ifd			Input file descriptor.
 * @rectime		Structure where timestamp (expressed in local time
 *			or in UTC depending on whether option -t has been
 *			used or not) can be saved for current record.
 * @loctime		Structure where timestamp (expressed in local time)
 *			can be saved for current record.
 *
 * OUT:
 * @rectime		Structure where timestamp for current record has
 *			been saved.
 * @loctime		Structure where timestamp for current record has
 *			been saved.
 ***************************************************************************
 */
void write_textual_comments(int curr, int use_tm_start, int use_tm_end, int tab, int ifd,
			    struct tm *rectime, struct tm *loctime)
{
	char cur_date[32], cur_time[32];
	char file_comment[MAX_COMMENT_LEN];

	sa_fread(ifd, file_comment, MAX_COMMENT_LEN, HARD_SIZE);
	file_comment[MAX_COMMENT_LEN - 1] = '\0';

	/* Fill timestamp structure for current record */
	sadf_get_record_timestamp_struct(curr, rectime, loctime);

	/* The record must be in the interval specified by -s/-e options */
	if ((use_tm_start && (datecmp(loctime, &tm_start) < 0)) ||
	    (use_tm_end && (datecmp(loctime, &tm_end) > 0)))
		return;

	set_record_timestamp_string(curr, cur_date, cur_time, 32, rectime);

	if (*fmt[f_position]->f_comment) {
		(*fmt[f_position]->f_comment)(&tab, F_MAIN, cur_date, cur_time,
					      !PRINT_TRUE_TIME(flags), file_comment,
					      &file_hdr);
	}
}

/*
 ***************************************************************************
 * Display the field list (used eg. in database format).
 *
 * IN:
 * @act_d	Activity to display, or ~0 for all.
 ***************************************************************************
 */
void list_fields(unsigned int act_id)
{
	int i;
	unsigned int msk;
	char *hl;
	char hline[HEADER_LINE_LEN];
	
	printf("# hostname;interval;timestamp");
	
	for (i = 0; i < NR_ACT; i++) {
		
		if ((act_id != ALL_ACTIVITIES) && (act[i]->id != act_id))
			continue;
		
		if (IS_SELECTED(act[i]->options) && (act[i]->nr > 0)) {
			if (!HAS_MULTIPLE_OUTPUTS(act[i]->options)) {
				printf(";%s", act[i]->hdr_line);
				if ((act[i]->nr > 1) && DISPLAY_HORIZONTALLY(flags)) {
					printf("[...]");
				}
			}
			else {
				msk = 1;
				strcpy(hline, act[i]->hdr_line);
				for (hl = strtok(hline, "|"); hl; hl = strtok(NULL, "|"), msk <<= 1) {
					if ((hl != NULL) && (act[i]->opt_flags & msk)) {
						printf(";%s", hl);
						if ((act[i]->nr > 1) && DISPLAY_HORIZONTALLY(flags)) {
							printf("[...]");
						}
					}
				}
			}
		}
	}
	printf("\n");
}

/*
 ***************************************************************************
 * write_mech_stats() -
 * Replace the old write_stats_for_ppc() and write_stats_for_db(),
 * making it easier for them to remain in sync and print the same data.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @dt		Interval of time in seconds.
 * @itv		Interval of time in jiffies.
 * @g_itv	Interval of time in jiffies multiplied by the number of
 * 		processors.
 * @cur_date	Date string for current record.
 * @cur_time	Time string for current record.
 * @act_id	Activity to display, or ~0 for all.
 ***************************************************************************
 */
void write_mech_stats(int curr, unsigned long dt, unsigned long long itv,
		      unsigned long long g_itv, char *cur_date, char *cur_time,
		      unsigned int act_id)
{
	int i;
	char pre[80], temp[80];	/* Text at beginning of each line */
	int isdb = (format == F_DB_OUTPUT);
	
	/* This substring appears on every output line, preformat it here */
	snprintf(pre, 80, "%s%s%ld%s",
		 file_hdr.sa_nodename, seps[isdb], dt, seps[isdb]);
	if (strlen(cur_date)) {
		snprintf(temp, 80, "%s%s ", pre, cur_date);
	}
	else {
		strcpy(temp, pre);
	}
	snprintf(pre, 80, "%s%s%s", temp, cur_time,
		 strlen(cur_date) && !PRINT_TRUE_TIME(flags) ? " UTC" : "");
	pre[79] = '\0';

	if (DISPLAY_HORIZONTALLY(flags)) {
		printf("%s", pre);
	}

	for (i = 0; i < NR_ACT; i++) {
		
		if ((act_id != ALL_ACTIVITIES) && (act[i]->id != act_id))
			continue;
		
		if (IS_SELECTED(act[i]->options) && (act[i]->nr > 0)) {
			(*act[i]->f_render)(act[i], isdb, pre, curr,
					    NEED_GLOBAL_ITV(act[i]->options) ? g_itv : itv);
		}
	}

	if (DISPLAY_HORIZONTALLY(flags)) {
		printf("\n");
	}
}

/*
 ***************************************************************************
 * Write system statistics.
 *
 * IN:
 * @curr		Index in array for current sample statistics.
 * @reset		Set to TRUE if last_uptime variable should be
 * 			reinitialized (used in next_slice() function).
 * @use_tm_start	Set to TRUE if option -s has been used.
 * @use_tm_end		Set to TRUE if option -e has been used.
 * @act_id		Activities to display.
 * @cpu_nr		Number of processors for current activity data file.
 * @rectime		Structure where timestamp (expressed in local time
 *			or in UTC depending on whether option -t has been
 *			used or not) can be saved for current record.
 * @loctime		Structure where timestamp (expressed in local time)
 *			can be saved for current record.
 *
 * OUT:
 * @cnt			Set to 0 to indicate that no other lines of stats
 * 			should be displayed.
 *
 * RETURNS:
 * 1 if a line of stats has been displayed, and 0 otherwise.
 ***************************************************************************
 */
int write_parsable_stats(int curr, int reset, long *cnt, int use_tm_start,
			 int use_tm_end, unsigned int act_id, __nr_t cpu_nr,
			 struct tm *rectime, struct tm *loctime)
{
	unsigned long long dt, itv, g_itv;
	char cur_date[32], cur_time[32];
	static int cross_day = FALSE;

	/*
	 * Check time (1).
	 * For this first check, we use the time interval entered on
	 * the command line. This is equivalent to sar's option -i which
	 * selects records at seconds as close as possible to the number
	 * specified by the interval parameter.
	 */
	if (!next_slice(record_hdr[2].uptime0, record_hdr[curr].uptime0,
			reset, interval))
		/* Not close enough to desired interval */
		return 0;

	/* Fill timestamp structure for current record */
	sadf_get_record_timestamp_struct(curr, rectime, loctime);

	/* Check if we are beginning a new day */
	if (use_tm_start && record_hdr[!curr].ust_time &&
	    (record_hdr[curr].ust_time > record_hdr[!curr].ust_time) &&
	    (record_hdr[curr].hour < record_hdr[!curr].hour)) {
		cross_day = TRUE;
	}

	if (cross_day) {
		/*
		 * This is necessary if we want to properly handle something like:
		 * sar -s time_start -e time_end with
		 * time_start(day D) > time_end(day D+1)
		 */
		loctime->tm_hour += 24;
	}

	/* Check time (2) */
	if (use_tm_start && (datecmp(loctime, &tm_start) < 0))
		/* it's too soon... */
		return 0;

	/* Get interval values */
	get_itv_value(&record_hdr[curr], &record_hdr[!curr],
		      cpu_nr, &itv, &g_itv);

	/* Check time (3) */
	if (use_tm_end && (datecmp(loctime, &tm_end) > 0)) {
		/* It's too late... */
		*cnt = 0;
		return 0;
	}

	dt = itv / HZ;
	/* Correct rounding error for dt */
	if ((itv % HZ) >= (HZ / 2)) {
		dt++;
	}

	/* Set current timestamp string */
	set_record_timestamp_string(curr, cur_date, cur_time, 32, rectime);

	write_mech_stats(curr, dt, itv, g_itv, cur_date, cur_time, act_id);

	return 1;
}

/*
 ***************************************************************************
 * Display activity records for textual (XML-like) formats.
 *
 * IN:
 * @curr		Index in array for current sample statistics.
 * @use_tm_start	Set to TRUE if option -s has been used.
 * @use_tm_end		Set to TRUE if option -e has been used.
 * @reset		Set to TRUE if last_uptime should be reinitialized
 *			(used in next_slice() function).
 * @tab			Number of tabulations to print.
 * @cpu_nr		Number of processors.
 * @rectime		Structure where timestamp (expressed in local time
 *			or in UTC depending on whether option -t has been
 *			used or not) can be saved for current record.
 * @loctime		Structure where timestamp (expressed in local time)
 *			can be saved for current record.
 *
 * OUT:
 * @cnt			Set to 0 to indicate that no other lines of stats
 * 			should be displayed.
 *
 * RETURNS:
 * 1 if stats have been successfully displayed.
 ***************************************************************************
 */
int write_textual_stats(int curr, int use_tm_start, int use_tm_end, int reset,
			long *cnt, int tab, __nr_t cpu_nr, struct tm *rectime,
			struct tm *loctime)
{
	int i;
	unsigned long long dt, itv, g_itv;
	char cur_date[32], cur_time[32];
	static int cross_day = FALSE;

	/* Fill timestamp structure (rectime) for current record */
	sadf_get_record_timestamp_struct(curr, rectime, loctime);

	/*
	 * Check time (1).
	 * For this first check, we use the time interval entered on
	 * the command line. This is equivalent to sar's option -i which
	 * selects records at seconds as close as possible to the number
	 * specified by the interval parameter.
	 */
	if (!next_slice(record_hdr[2].uptime0, record_hdr[curr].uptime0,
			reset, interval))
		/* Not close enough to desired interval */
		return 0;

	/* Check if we are beginning a new day */
	if (use_tm_start && record_hdr[!curr].ust_time &&
	    (record_hdr[curr].ust_time > record_hdr[!curr].ust_time) &&
	    (record_hdr[curr].hour < record_hdr[!curr].hour)) {
		cross_day = TRUE;
	}

	if (cross_day) {
		/*
		 * This is necessary if we want to properly handle something like:
		 * sar -s time_start -e time_end with
		 * time_start(day D) > time_end(day D+1)
		 */
		loctime->tm_hour += 24;
	}

	/* Check time (2) */
	if (use_tm_start && (datecmp(loctime, &tm_start) < 0))
		/* it's too soon... */
		return 0;

	/* Get interval values */
	get_itv_value(&record_hdr[curr], &record_hdr[!curr],
		      cpu_nr, &itv, &g_itv);

	/* Check time (3) */
	if (use_tm_end && (datecmp(loctime, &tm_end) > 0)) {
		/* It's too late... */
		*cnt = 0;
		return 0;
	}

	dt = itv / HZ;
	/* Correct rounding error for dt */
	if ((itv % HZ) >= (HZ / 2)) {
		dt++;
	}

	/* Set date and time strings for current record */
	set_record_timestamp_string(curr, cur_date, cur_time, 32, rectime);

	if (*fmt[f_position]->f_timestamp) {
		(*fmt[f_position]->f_timestamp)(&tab, F_BEGIN, cur_date, cur_time,
						!PRINT_TRUE_TIME(flags), dt);
	}
	if (format == F_XML_OUTPUT) {
		tab++;
	}

	/* Display textual statistics */
	for (i = 0; i < NR_ACT; i++) {

		/* This code is not generic at all...! */
		if (format == F_JSON_OUTPUT) {
			/* JSON output */
			if (CLOSE_MARKUP(act[i]->options) ||
			    (IS_SELECTED(act[i]->options) && (act[i]->nr > 0))) {
				
				if (IS_SELECTED(act[i]->options) && (act[i]->nr > 0)) {
					printf(",");

					if (*fmt[f_position]->f_timestamp) {
						(*fmt[f_position]->f_timestamp)(&tab, F_MAIN, cur_date, cur_time,
										!PRINT_TRUE_TIME(flags), dt);
					}
				}
				(*act[i]->f_json_print)(act[i], curr, tab, NEED_GLOBAL_ITV(act[i]->options) ?
							g_itv : itv);
			}
		}
		else {
			/* XML output */
			if (CLOSE_MARKUP(act[i]->options) ||
			    (IS_SELECTED(act[i]->options) && (act[i]->nr > 0))) {
				(*act[i]->f_xml_print)(act[i], curr, tab, NEED_GLOBAL_ITV(act[i]->options) ?
						       g_itv : itv);
			}
		}
	}

	if (*fmt[f_position]->f_timestamp) {
		(*fmt[f_position]->f_timestamp)(&tab, F_END, cur_date, cur_time,
						!PRINT_TRUE_TIME(flags), dt);
	}

	return 1;
}

/*
 ***************************************************************************
 * Print contents of a special (RESTART or COMMENT) record.
 *
 * IN:
 * @curr		Index in array for current sample statistics.
 * @use_tm_start	Set to TRUE if option -s has been used.
 * @use_tm_end		Set to TRUE if option -e has been used.
 * @rtype		Record type (RESTART or COMMENT).
 * @ifd			Input file descriptor.
 * @rectime		Structure where timestamp (expressed in local time
 *			or in UTC depending on whether option -t has been
 *			used or not) can be saved for current record.
 * @loctime		Structure where timestamp (expressed in local time)
 *			can be saved for current record.
 ***************************************************************************
 */
void sadf_print_special(int curr, int use_tm_start, int use_tm_end, int rtype, int ifd,
			struct tm *rectime, struct tm *loctime)
{
	char cur_date[32], cur_time[32];
	int dp = 1;

	/* Fill timestamp structure (rectime) for current record */
	sadf_get_record_timestamp_struct(curr, rectime, loctime);

	/* Set date and time strings for current record */
	set_record_timestamp_string(curr, cur_date, cur_time, 32, rectime);

	/* The record must be in the interval specified by -s/-e options */
	if ((use_tm_start && (datecmp(loctime, &tm_start) < 0)) ||
	    (use_tm_end && (datecmp(loctime, &tm_end) > 0))) {
		dp = 0;
	}

	if (rtype == R_RESTART) {
		if (!dp)
			return;

		if (*fmt[f_position]->f_restart) {
			(*fmt[f_position]->f_restart)(NULL, F_MAIN, cur_date, cur_time,
						      !PRINT_TRUE_TIME(flags), &file_hdr);
		}
	}
	else if (rtype == R_COMMENT) {
		char file_comment[MAX_COMMENT_LEN];

		sa_fread(ifd, file_comment, MAX_COMMENT_LEN, HARD_SIZE);
		file_comment[MAX_COMMENT_LEN - 1] = '\0';

		if (!dp || !DISPLAY_COMMENT(flags))
			return;

		if (*fmt[f_position]->f_comment) {
			(*fmt[f_position]->f_comment)(NULL, F_MAIN, cur_date, cur_time,
						      !PRINT_TRUE_TIME(flags), file_comment,
						      &file_hdr);
		}
	}
}

/*
 ***************************************************************************
 * Read stats for current activity from file and write them.
 *
 * IN:
 * @ifd		File descriptor of input file.
 * @fpos	Position in file where reading must start.
 * @curr	Index in array for current sample statistics.
 * @act_id	Activity to display, or ~0 for all.
 * @file_actlst	List of (known or unknown) activities in file.
 * @cpu_nr	Number of processors for current activity data file.
 * @rectime	Structure where timestamp (expressed in local time or in UTC
 *		depending on whether option -t has been used or not) can be
 *		saved for current record.
 * @loctime	Structure where timestamp (expressed in local time) can be
 *		saved for current record.
 *
 * OUT:
 * @curr	Index in array for next sample statistics.
 * @cnt		Number of lines of stats remaining to write.
 * @eosaf	Set to TRUE if EOF (end of file) has been reached.
 * @reset	Set to TRUE if last_uptime variable should be
 * 		reinitialized (used in next_slice() function).
 ***************************************************************************
 */
void rw_curr_act_stats(int ifd, off_t fpos, int *curr, long *cnt, int *eosaf,
		       unsigned int act_id, int *reset, struct file_activity *file_actlst,
		        __nr_t cpu_nr, struct tm *rectime, struct tm *loctime)
{
	unsigned char rtype;
	int next;

	if (lseek(ifd, fpos, SEEK_SET) < fpos) {
		perror("lseek");
		exit(2);
	}
	
	if (DISPLAY_FIELD_LIST(fmt[f_position]->options)) {
		/* Print field list */
		list_fields(act_id);
	}

	/*
	 * Restore the first stats collected.
	 * Used to compute the rate displayed on the first line.
	 */
	copy_structures(act, id_seq, record_hdr, !*curr, 2);

	*cnt  = count;

	do {
		/* Display <count> lines of stats */
		*eosaf = sa_fread(ifd, &record_hdr[*curr], RECORD_HEADER_SIZE,
				  SOFT_SIZE);
		rtype = record_hdr[*curr].record_type;

		if (!*eosaf && (rtype != R_RESTART) && (rtype != R_COMMENT)) {
			/* Read the extra fields since it's not a RESTART record */
			read_file_stat_bunch(act, *curr, ifd, file_hdr.sa_nr_act,
					     file_actlst);
		}

		if (!*eosaf && (rtype != R_RESTART)) {

			if (rtype == R_COMMENT) {
				sadf_print_special(*curr, tm_start.use, tm_end.use,
						   R_COMMENT, ifd, rectime, loctime);
				continue;
			}

			next = write_parsable_stats(*curr, *reset, cnt,
						    tm_start.use, tm_end.use, act_id,
						    cpu_nr, rectime, loctime);

			if (next) {
				/*
				 * next is set to 1 when we were close enough to desired interval.
				 * In this case, the call to write_parsable_stats() has actually
				 * displayed a line of stats.
				 */
				*curr ^=1;
				if (*cnt > 0) {
					(*cnt)--;
				}
			}
			*reset = FALSE;
		}
	}
	while (*cnt && !*eosaf && (rtype != R_RESTART));

	*reset = TRUE;
}

/*
 ***************************************************************************
 * Display activities for textual (XML-like) formats.
 *
 * IN:
 * @ifd		File descriptor of input file.
 * @file_actlst	List of (known or unknown) activities in file.
 * @dfile	System activity data file name.
 * @file_magic	System activity file magic header.
 * @cpu_nr	Number of processors for current activity data file.
 * @rectime	Structure where timestamp (expressed in local time or in UTC
 *		depending on whether option -t has been used or not) can be
 *		saved for current record.
 * @loctime	Structure where timestamp (expressed in local time) can be
 *		saved for current record.
 ***************************************************************************
 */
void textual_display_loop(int ifd, struct file_activity *file_actlst, char *dfile,
			  struct file_magic *file_magic, __nr_t cpu_nr,
			  struct tm *rectime, struct tm *loctime)
{
	int curr, tab = 0, rtype;
	int eosaf = TRUE, next, reset = FALSE;
	long cnt = 1;
	off_t fpos;

	/* Save current file position */
	if ((fpos = lseek(ifd, 0, SEEK_CUR)) < 0) {
		perror("lseek");
		exit(2);
	}

	/* Print header (eg. XML file header) */
	if (*fmt[f_position]->f_header) {
		(*fmt[f_position]->f_header)(&tab, F_BEGIN, dfile, file_magic,
					     &file_hdr, cpu_nr, act, id_seq);
	}

	/* Process activities */
	if (*fmt[f_position]->f_statistics) {
		(*fmt[f_position]->f_statistics)(&tab, F_BEGIN);
	}

	do {
		/*
		 * If this record is a special (RESTART or COMMENT) one,
		 * skip it and try to read the next record in file.
		 */
		do {
			eosaf = sa_fread(ifd, &record_hdr[0], RECORD_HEADER_SIZE, SOFT_SIZE);
			rtype = record_hdr[0].record_type;

			if (!eosaf && (rtype != R_RESTART) && (rtype != R_COMMENT)) {
				/*
				 * OK: Previous record was not a special one.
				 * So read now the extra fields.
				 */
				read_file_stat_bunch(act, 0, ifd, file_hdr.sa_nr_act,
						     file_actlst);
				sadf_get_record_timestamp_struct(0, rectime, loctime);
			}

			if (!eosaf && (rtype == R_COMMENT)) {
				/*
				 * Ignore COMMENT record.
				 * (Unlike RESTART records, COMMENT records have an additional
				 * comment field).
				 */
				if (lseek(ifd, MAX_COMMENT_LEN, SEEK_CUR) < MAX_COMMENT_LEN) {
					perror("lseek");
				}
			}
		}
		while (!eosaf && ((rtype == R_RESTART) || (rtype == R_COMMENT) ||
			(tm_start.use && (datecmp(loctime, &tm_start) < 0)) ||
			(tm_end.use && (datecmp(loctime, &tm_end) >=0))));

		/* Save the first stats collected. Used for example in next_slice() function */
		copy_structures(act, id_seq, record_hdr, 2, 0);

		curr = 1;
		cnt = count;
		reset = TRUE;

		if (!eosaf) {
			do {
				eosaf = sa_fread(ifd, &record_hdr[curr], RECORD_HEADER_SIZE,
						 SOFT_SIZE);
				rtype = record_hdr[curr].record_type;

				if (!eosaf && (rtype != R_RESTART) && (rtype != R_COMMENT)) {
					/* Read the extra fields since it's not a special record */
					read_file_stat_bunch(act, curr, ifd, file_hdr.sa_nr_act,
							     file_actlst);

					if (*fmt[f_position]->f_statistics) {
						(*fmt[f_position]->f_statistics)(&tab, F_MAIN);
					}

					/* next is set to 1 when we were close enough to desired interval */
					next = write_textual_stats(curr, tm_start.use, tm_end.use, reset,
								   &cnt, tab, cpu_nr, rectime, loctime);

					if (next) {
						curr ^= 1;
						if (cnt > 0) {
							cnt--;
						}
					}
					reset = FALSE;
				}

				if (!eosaf && (rtype == R_COMMENT)) {
					/* Ignore COMMENT record */
					if (lseek(ifd, MAX_COMMENT_LEN, SEEK_CUR) < MAX_COMMENT_LEN) {
						perror("lseek");
					}
				}
			}
			while (cnt && !eosaf && (rtype != R_RESTART));

			if (!cnt) {
				/* Go to next Linux restart, if possible */
				do {
					eosaf = sa_fread(ifd, &record_hdr[curr], RECORD_HEADER_SIZE,
							 SOFT_SIZE);
					rtype = record_hdr[curr].record_type;
					if (!eosaf && (rtype != R_RESTART) && (rtype != R_COMMENT)) {
						read_file_stat_bunch(act, curr, ifd, file_hdr.sa_nr_act,
								     file_actlst);
					}
					else if (!eosaf && (rtype == R_COMMENT)) {
						/* Ignore COMMENT record */
						if (lseek(ifd, MAX_COMMENT_LEN, SEEK_CUR) < MAX_COMMENT_LEN) {
							perror("lseek");
						}
					}
				}
				while (!eosaf && (rtype != R_RESTART));
			}
			reset = TRUE;
		}
	}
	while (!eosaf);

	if (*fmt[f_position]->f_statistics) {
		(*fmt[f_position]->f_statistics)(&tab, F_END);
	}

	/* Rewind file */
	if (lseek(ifd, fpos, SEEK_SET) < fpos) {
		perror("lseek");
		exit(2);
	}

	/* Process now RESTART entries to display restart messages */
	if (*fmt[f_position]->f_restart) {
		(*fmt[f_position]->f_restart)(&tab, F_BEGIN, NULL, NULL, FALSE,
					      &file_hdr);
	}

	do {
		if ((eosaf = sa_fread(ifd, &record_hdr[0], RECORD_HEADER_SIZE,
				      SOFT_SIZE)) == 0) {

			rtype = record_hdr[0].record_type;
			if ((rtype != R_RESTART) && (rtype != R_COMMENT)) {
				read_file_stat_bunch(act, 0, ifd, file_hdr.sa_nr_act,
						     file_actlst);
			}
			if (rtype == R_RESTART) {
				write_textual_restarts(0, tm_start.use, tm_end.use, tab,
						       rectime, loctime);
			}
			else if (rtype == R_COMMENT) {
				/* Ignore COMMENT record */
				if (lseek(ifd, MAX_COMMENT_LEN, SEEK_CUR) < MAX_COMMENT_LEN) {
					perror("lseek");
				}
			}
		}
	}
	while (!eosaf);

	if (*fmt[f_position]->f_restart) {
		(*fmt[f_position]->f_restart)(&tab, F_END, NULL, NULL, FALSE, &file_hdr);
	}

	/* Rewind file */
	if (lseek(ifd, fpos, SEEK_SET) < fpos) {
		perror("lseek");
		exit(2);
	}

	/* Last, process COMMENT entries to display comments */
	if (DISPLAY_COMMENT(flags)) {
		if (*fmt[f_position]->f_comment) {
			(*fmt[f_position]->f_comment)(&tab, F_BEGIN, NULL, NULL, 0, NULL,
						      &file_hdr);
		}
		do {
			if ((eosaf = sa_fread(ifd, &record_hdr[0], RECORD_HEADER_SIZE,
				              SOFT_SIZE)) == 0) {

				rtype = record_hdr[0].record_type;
				if ((rtype != R_RESTART) && (rtype != R_COMMENT)) {
					read_file_stat_bunch(act, 0, ifd, file_hdr.sa_nr_act,
							     file_actlst);
				}
				if (rtype == R_COMMENT) {
					write_textual_comments(0, tm_start.use, tm_end.use,
							       tab, ifd, rectime, loctime);
				}
			}
		}
		while (!eosaf);

		if (*fmt[f_position]->f_comment) {
			(*fmt[f_position]->f_comment)(&tab, F_END, NULL, NULL, 0, NULL,
						      &file_hdr);
		}
	}

	/* Print header trailer */
	if (*fmt[f_position]->f_header) {
		(*fmt[f_position]->f_header)(&tab, F_END, dfile, file_magic,
					     &file_hdr, cpu_nr, act, id_seq);
	}
}

/*
 ***************************************************************************
 * Display activities for non textual formats.
 *
 * IN:
 * @ifd		File descriptor of input file.
 * @file_actlst	List of (known or unknown) activities in file.
 * @cpu_nr	Number of processors for current activity data file.
 * @rectime	Structure where timestamp (expressed in local time or in UTC
 *		depending on whether option -t has been used or not) can be
 *		saved for current record.
 * @loctime	Structure where timestamp (expressed in local time) can be
 *		saved for current record.
 ***************************************************************************
 */
void main_display_loop(int ifd, struct file_activity *file_actlst, __nr_t cpu_nr,
		       struct tm *rectime, struct tm *loctime)
{
	int i, p;
	int curr = 1, rtype;
	int eosaf = TRUE, reset = FALSE;
	long cnt = 1;
	off_t fpos;
	
	/* Read system statistics from file */
	do {
		/*
		 * If this record is a special (RESTART or COMMENT) one, print it and
		 * (try to) get another one.
		 */
		do {
			if (sa_fread(ifd, &record_hdr[0], RECORD_HEADER_SIZE, SOFT_SIZE))
				/* End of sa data file */
				return;

			rtype = record_hdr[0].record_type;
			if ((rtype == R_RESTART) || (rtype == R_COMMENT)) {
				sadf_print_special(0, tm_start.use, tm_end.use, rtype, ifd,
						   rectime, loctime);
			}
			else {
				/*
				 * OK: Previous record was not a special one.
				 * So read now the extra fields.
				 */
				read_file_stat_bunch(act, 0, ifd, file_hdr.sa_nr_act,
						     file_actlst);
				sadf_get_record_timestamp_struct(0, rectime, loctime);
			}
		}
		while ((rtype == R_RESTART) || (rtype == R_COMMENT) ||
		       (tm_start.use && (datecmp(loctime, &tm_start) < 0)) ||
		       (tm_end.use && (datecmp(loctime, &tm_end) >= 0)));

		/* Save the first stats collected. Will be used to compute the average */
		copy_structures(act, id_seq, record_hdr, 2, 0);

		/* Set flag to reset last_uptime variable. Should be done after a LINUX RESTART record */
		reset = TRUE;

		/* Save current file position */
		if ((fpos = lseek(ifd, 0, SEEK_CUR)) < 0) {
			perror("lseek");
			exit(2);
		}

		/* Read and write stats located between two possible Linux restarts */

		if (DISPLAY_HORIZONTALLY(flags)) {
			/*
			 * If stats are displayed horizontally, then all activities
			 * are printed on the same line.
			 */
			rw_curr_act_stats(ifd, fpos, &curr, &cnt, &eosaf,
					  ALL_ACTIVITIES, &reset, file_actlst,
					  cpu_nr, rectime, loctime);
		}
		else {
			/* For each requested activity... */
			for (i = 0; i < NR_ACT; i++) {
				
				if (!id_seq[i])
					continue;
				
				if ((p = get_activity_position(act, id_seq[i])) < 0) {
					/* Should never happen */
					PANIC(1);
				}
				if (!IS_SELECTED(act[p]->options))
					continue;

				if (!HAS_MULTIPLE_OUTPUTS(act[p]->options)) {
					rw_curr_act_stats(ifd, fpos, &curr, &cnt, &eosaf,
							  act[p]->id, &reset, file_actlst,
							  cpu_nr, rectime, loctime);
				}
				else {
					unsigned int optf, msk;
					
					optf = act[p]->opt_flags;
					
					for (msk = 1; msk < 0x10; msk <<= 1) {
						if (act[p]->opt_flags & msk) {
							act[p]->opt_flags &= msk;
							
							rw_curr_act_stats(ifd, fpos, &curr, &cnt, &eosaf,
									  act[p]->id, &reset, file_actlst,
									  cpu_nr, rectime, loctime);
							act[p]->opt_flags = optf;
						}
					}
				}
			}
		}

		if (!cnt) {
			/* Go to next Linux restart, if possible */
			do {
				eosaf = sa_fread(ifd, &record_hdr[curr], RECORD_HEADER_SIZE,
						 SOFT_SIZE);
				rtype = record_hdr[curr].record_type;
				if (!eosaf && (rtype != R_RESTART) && (rtype != R_COMMENT)) {
					read_file_stat_bunch(act, curr, ifd, file_hdr.sa_nr_act,
							     file_actlst);
				}
				else if (!eosaf && (rtype == R_COMMENT)) {
					/* This was a COMMENT record: print it */
					sadf_print_special(curr, tm_start.use, tm_end.use,
							   R_COMMENT, ifd, rectime, loctime);
				}
			}
			while (!eosaf && (rtype != R_RESTART));
		}

		/* The last record we read was a RESTART one: Print it */
		if (!eosaf && (record_hdr[curr].record_type == R_RESTART)) {
			sadf_print_special(curr, tm_start.use, tm_end.use,
					   R_RESTART, ifd, rectime, loctime);
		}
	}
	while (!eosaf);
}

/*
 ***************************************************************************
 * Read statistics from a system activity data file.
 *
 * IN:
 * @dfile	System activity data file name.
 ***************************************************************************
 */
void read_stats_from_file(char dfile[])
{
	struct file_magic file_magic;
	struct file_activity *file_actlst = NULL;
	struct tm rectime, loctime;
	int ifd, ignore, tab = 0;
	__nr_t cpu_nr;

	/* Prepare file for reading and read its headers */
	ignore = ACCEPT_BAD_FILE_FORMAT(fmt[f_position]->options);
	check_file_actlst(&ifd, dfile, act, &file_magic, &file_hdr,
			  &file_actlst, id_seq, ignore);

	/* Now pick up number of proc for this file */
	cpu_nr = act[get_activity_position(act, A_CPU)]->nr;

	if (DISPLAY_HDR_ONLY(flags)) {
		if (*fmt[f_position]->f_header) {
			/* Display only data file header then exit */
			(*fmt[f_position]->f_header)(&tab, F_BEGIN + F_END, dfile, &file_magic,
						     &file_hdr, cpu_nr, act, id_seq);
		}
		exit(0);
	}

	/* Perform required allocations */
	allocate_structures(act);

	if (DISPLAY_GROUPED_STATS(fmt[f_position]->options)) {
		main_display_loop(ifd, file_actlst, cpu_nr,
				  &rectime, &loctime);
	}
	else {
		textual_display_loop(ifd, file_actlst, dfile,
				     &file_magic, cpu_nr, &rectime, &loctime);
	}

	close(ifd);
	
	if (file_actlst) {
		free(file_actlst);
	}
	free_structures(act);
}

/*
 ***************************************************************************
 * Main entry to the sadf program
 ***************************************************************************
 */
int main(int argc, char **argv)
{
	int opt = 1, sar_options = 0;
	int i;
	char dfile[MAX_FILE_LEN];
	struct tm rectime;

	/* Get HZ */
	get_HZ();

	/* Compute page shift in kB */
	get_kb_shift();

	dfile[0] = '\0';

#ifdef USE_NLS
	/* Init National Language Support */
	init_nls();
#endif

	tm_start.use = tm_end.use = FALSE;
	
	/* Allocate and init activity bitmaps */
	allocate_bitmaps(act);

	/* Init some structures */
	init_structures();

	/* Process options */
	while (opt < argc) {

		if (!strcmp(argv[opt], "-I")) {
			if (argv[++opt] && sar_options) {
				if (parse_sar_I_opt(argv, &opt, act)) {
					usage(argv[0]);
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-P")) {
			if (parse_sa_P_opt(argv, &opt, &flags, act)) {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-s")) {
			/* Get time start */
			if (parse_timestamp(argv, &opt, &tm_start, DEF_TMSTART)) {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-e")) {
			/* Get time end */
			if (parse_timestamp(argv, &opt, &tm_end, DEF_TMEND)) {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "--")) {
			sar_options = 1;
			opt++;
		}

		else if (!strcmp(argv[opt], "-m")) {
			if (argv[++opt] && sar_options) {
				/* Parse sar's option -m */
				if (parse_sar_m_opt(argv, &opt, act)) {
					usage(argv[0]);
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-n")) {
			if (argv[++opt] && sar_options) {
				/* Parse sar's option -n */
				if (parse_sar_n_opt(argv, &opt, act)) {
					usage(argv[0]);
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strncmp(argv[opt], "-", 1)) {
			/* Other options not previously tested */
			if (sar_options) {
				if (parse_sar_opt(argv, &opt, act, &flags, C_SADF)) {
					usage(argv[0]);
				}
			}
			else {

				for (i = 1; *(argv[opt] + i); i++) {

					switch (*(argv[opt] + i)) {

					case 'C':
						flags |= S_F_COMMENT;
						break;

					case 'd':
						if (format) {
							usage(argv[0]);
						}
						format = F_DB_OUTPUT;
						break;

					case 'h':
						flags |= S_F_HORIZONTALLY;
						break;

					case 'H':
						flags |= S_F_HDR_ONLY;
						break;

					case 'j':
						if (format) {
							usage(argv[0]);
						}
						format = F_JSON_OUTPUT;
						break;

					case 'p':
						if (format) {
							usage(argv[0]);
						}
						format = F_PPC_OUTPUT;
						break;

					case 't':
						flags |= S_F_TRUE_TIME;
						break;
					
					case 'T':
						flags |= S_F_SEC_EPOCH;
						break;

					case 'x':
						if (format) {
							usage(argv[0]);
						}
						format = F_XML_OUTPUT;
						break;

					case 'V':
						print_version();
						break;

					default:
						usage(argv[0]);
					}
				}
			}
			opt++;
		}

		/* Get data file name */
		else if (strspn(argv[opt], DIGITS) != strlen(argv[opt])) {
			if (!dfile[0]) {
				if (!strcmp(argv[opt], "-")) {
					/* File name set to '-' */
					set_default_file(&rectime, dfile);
					opt++;
				}
				else if (!strncmp(argv[opt], "-", 1)) {
					/* Bad option */
					usage(argv[0]);
				}
				else {
					/* Write data to file */
					strncpy(dfile, argv[opt++], MAX_FILE_LEN);
					dfile[MAX_FILE_LEN - 1] = '\0';
				}
			}
			else {
				/* File already specified */
				usage(argv[0]);
			}
		}

		else if (interval < 0) {
			/* Get interval */
			if (strspn(argv[opt], DIGITS) != strlen(argv[opt])) {
				usage(argv[0]);
			}
			interval = atol(argv[opt++]);
			if (interval <= 0) {
				usage(argv[0]);
			}
		}

		else {
			/* Get count value */
			if (strspn(argv[opt], DIGITS) != strlen(argv[opt])) {
				usage(argv[0]);
			}
			if (count) {
				/* Count parameter already set */
				usage(argv[0]);
			}
			count = atol(argv[opt++]);
			if (count < 0) {
				usage(argv[0]);
			}
			else if (!count) {
				count = -1;	/* To generate a report continuously */
			}
		}
	}

	/* sadf reads current daily data file by default */
	if (!dfile[0]) {
		set_default_file(&rectime, dfile);
	}

	if (tm_start.use && tm_end.use && (tm_end.tm_hour < tm_start.tm_hour)) {
		tm_end.tm_hour += 24;
	}

	if (USE_PRETTY_OPTION(flags)) {
		dm_major = get_devmap_major();
	}

	/*
	 * Display all the contents of the daily data file if the count parameter
	 * was not set on the command line.
	 */
	if (!count) {
		count = -1;
	}

	/* Default is CPU activity */
	select_default_activity(act);

	/* Check options consistency with selected output format. Default is PPC display */
	check_format_options();

	if (interval < 0) {
		interval = 1;
	}

	/* Read stats from file */
	read_stats_from_file(dfile);

	/* Free bitmaps */
	free_bitmaps(act);

	return 0;
}
