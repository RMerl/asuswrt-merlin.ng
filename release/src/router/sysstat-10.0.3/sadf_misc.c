/*
 * sadf_misc.c: Funtions used by sadf to display special records
 * (C) 2011 by Sebastien GODARD (sysstat <at> orange.fr)
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

#include "sadf.h"
#include "sa.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

extern unsigned int flags;

/*
 ***************************************************************************
 * Display restart messages (database and ppc formats).
 *
 * IN:
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @sep		Character used as separator.
 * @file_hdr	System activity file standard header.
 ***************************************************************************
 */
void print_dbppc_restart(char *cur_date, char *cur_time, int utc, char sep,
			 struct file_header *file_hdr)
{
	printf("%s%c-1%c", file_hdr->sa_nodename, sep, sep);
	if (strlen(cur_date)) {
		printf("%s ", cur_date);
	}
	printf("%s", cur_time);
	if (strlen(cur_date) && utc) {
		printf(" UTC");
	}
	printf("%cLINUX-RESTART\n", sep);
}

/*
 ***************************************************************************
 * Display restart messages (ppc format).
 *
 * IN:
 * @tab		Number of tabulations (unused here).
 * @action	Action expected from current function.
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @file_hdr	System activity file standard header.
 ***************************************************************************
 */
__printf_funct_t print_db_restart(int *tab, int action, char *cur_date,
				  char *cur_time, int utc, struct file_header *file_hdr)
{
	/* Actions F_BEGIN and F_END ignored */
	if (action == F_MAIN) {
		print_dbppc_restart(cur_date, cur_time, utc, ';', file_hdr);
	}
}

/*
 ***************************************************************************
 * Display restart messages (database format).
 *
 * IN:
 * @tab		Number of tabulations (unused here).
 * @action	Action expected from current function.
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @file_hdr	System activity file standard header.
 ***************************************************************************
 */
__printf_funct_t print_ppc_restart(int *tab, int action, char *cur_date,
				   char *cur_time, int utc, struct file_header *file_hdr)
{
	/* Actions F_BEGIN and F_END ignored */
	if (action == F_MAIN) {
		print_dbppc_restart(cur_date, cur_time, utc, '\t', file_hdr);
	}
}

/*
 ***************************************************************************
 * Display restart messages (XML format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @file_hdr	System activity file standard header (unused here).
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_xml_restart(int *tab, int action, char *cur_date,
				   char *cur_time, int utc, struct file_header *file_hdr)
{
	if (action & F_BEGIN) {
		xprintf((*tab)++, "<restarts>");
	}
	if (action & F_MAIN) {
		xprintf(*tab, "<boot date=\"%s\" time=\"%s\" utc=\"%d\"/>",
			cur_date, cur_time, utc ? 1 : 0);
	}
	if (action & F_END) {
		xprintf(--(*tab), "</restarts>");
	}
}

/*
 ***************************************************************************
 * Display restart messages (JSON format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @file_hdr	System activity file standard header (unused here).
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_json_restart(int *tab, int action, char *cur_date,
				    char *cur_time, int utc, struct file_header *file_hdr)
{
	static int sep = FALSE;
	
	if (action & F_BEGIN) {
		printf(",\n");
		xprintf((*tab)++, "\"restarts\": [");
	}
	if (action & F_MAIN) {
		if (sep) {
			printf(",\n");
		}
		xprintf((*tab)++, "{");
		xprintf(*tab, "\"boot\": {\"date\": \"%s\", \"time\": \"%s\", \"utc\": %d}",
			cur_date, cur_time, utc ? 1 : 0);
		xprintf0(--(*tab), "}");
		sep = TRUE;
	}
	if (action & F_END) {
		if (sep) {
			printf("\n");
			sep = FALSE;
		}
		xprintf0(--(*tab), "]");
	}
}

/*
 ***************************************************************************
 * Display comments (database and ppc formats).
 *
 * IN:
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @comment	Comment to display.
 * @sep		Character used as separator.
 * @file_hdr	System activity file standard header.
 ***************************************************************************
 */
void print_dbppc_comment(char *cur_date, char *cur_time, int utc,
			 char *comment, char sep, struct file_header *file_hdr)
{
	printf("%s%c-1%c", file_hdr->sa_nodename, sep, sep);
	if (strlen(cur_date)) {
		printf("%s ", cur_date);
	}
	printf("%s", cur_time);
	if (strlen(cur_date) && utc) {
		printf(" UTC");
	}
	printf("%cCOM %s\n", sep, comment);
}

/*
 ***************************************************************************
 * Display comments (database format).
 *
 * IN:
 * @tab		Number of tabulations (unused here).
 * @action	Action expected from current function.
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @comment	Comment to display.
 * @file_hdr	System activity file standard header.
 ***************************************************************************
 */
__printf_funct_t print_db_comment(int *tab, int action, char *cur_date,
				  char *cur_time, int utc, char *comment,
				  struct file_header *file_hdr)
{
	/* Actions F_BEGIN and F_END ignored */
	if (action & F_MAIN) {
		print_dbppc_comment(cur_date, cur_time, utc, comment,
				    ';', file_hdr);
	}
}

/*
 ***************************************************************************
 * Display comments (ppc format).
 *
 * IN:
 * @tab		Number of tabulations (unused here).
 * @action	Action expected from current function.
 * @cur_date	Date string of current restart message.
 * @cur_time	Time string of current restart message.
 * @utc		True if @cur_time is expressed in UTC.
 * @comment	Comment to display.
 * @file_hdr	System activity file standard header.
 ***************************************************************************
 */
__printf_funct_t print_ppc_comment(int *tab, int action, char *cur_date,
				   char *cur_time, int utc, char *comment,
				   struct file_header *file_hdr)
{
	/* Actions F_BEGIN and F_END ignored */
	if (action & F_MAIN) {
		print_dbppc_comment(cur_date, cur_time, utc, comment,
				    '\t', file_hdr);
	}
}

/*
 ***************************************************************************
 * Display comments (XML format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @cur_date	Date string of current comment.
 * @cur_time	Time string of current comment.
 * @utc		True if @cur_time is expressed in UTC.
 * @comment	Comment to display.
 * @file_hdr	System activity file standard header (unused here).
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_xml_comment(int *tab, int action, char *cur_date,
				   char *cur_time, int utc, char *comment,
				   struct file_header *file_hdr)
{
	if (action & F_BEGIN) {
		xprintf((*tab)++, "<comments>");
	}
	if (action & F_MAIN) {
		xprintf(*tab, "<comment date=\"%s\" time=\"%s\" utc=\"%d\" com=\"%s\"/>",
			cur_date, cur_time, utc ? 1 : 0, comment);
	}
	if (action & F_END) {
		xprintf(--(*tab), "</comments>");
	}
}

/*
 ***************************************************************************
 * Display comments (JSON format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @cur_date	Date string of current comment.
 * @cur_time	Time string of current comment.
 * @utc		True if @cur_time is expressed in UTC.
 * @comment	Comment to display.
 * @file_hdr	System activity file standard header (unused here).
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_json_comment(int *tab, int action, char *cur_date,
				    char *cur_time, int utc, char *comment,
				    struct file_header *file_hdr)
{
	static int sep = FALSE;
	
	if (action & F_BEGIN) {
		printf(",\n");
		xprintf((*tab)++, "\"comments\": [");
	}
	if (action & F_MAIN) {
		if (sep) {
			printf(",\n");
		}
		xprintf((*tab)++, "{");
		xprintf(*tab,
			"\"comment\": {\"date\": \"%s\", \"time\": \"%s\", "
			"\"utc\": %d, \"com\": \"%s\"}",
			cur_date, cur_time, utc ? 1 : 0, comment);
		xprintf0(--(*tab), "}");
		sep = TRUE;
	}
	if (action & F_END) {
		if (sep) {
			printf("\n");
			sep = FALSE;
		}
		xprintf0(--(*tab), "]");
	}
}

/*
 ***************************************************************************
 * Display the "statistics" part of the report (XML format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_xml_statistics(int *tab, int action)
{
	if (action & F_BEGIN) {
		xprintf((*tab)++, "<statistics>");
	}
	if (action & F_END) {
		xprintf(--(*tab), "</statistics>");
	}
}

/*
 ***************************************************************************
 * Display the "statistics" part of the report (JSON format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_json_statistics(int *tab, int action)
{
	static int sep = FALSE;
	
	if (action & F_BEGIN) {
		printf(",\n");
		xprintf((*tab)++, "\"statistics\": [");
	}
	if (action & F_MAIN) {
		if (sep) {
			xprintf(--(*tab), "},");
		}
		xprintf((*tab)++, "{");
		sep = TRUE;
	}
	if (action & F_END) {
		if (sep) {
			xprintf(--(*tab), "}");
			sep = FALSE;
		}
		xprintf0(--(*tab), "]");
	}
}

/*
 ***************************************************************************
 * Display the "timestamp" part of the report (XML format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @cur_date	Date string of current comment.
 * @cur_time	Time string of current comment.
 * @utc		True if @cur_time is expressed in UTC.
 * @itv		Interval of time with preceding record.
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_xml_timestamp(int *tab, int action, char *cur_date,
				     char *cur_time, int utc, unsigned long long itv)
{
	if (action & F_BEGIN) {
		xprintf(*tab, "<timestamp date=\"%s\" time=\"%s\" utc=\"%d\" interval=\"%llu\">",
			cur_date, cur_time, utc ? 1 : 0, itv);
	}
	if (action & F_END) {
		xprintf(--(*tab), "</timestamp>");
	}
}

/*
 ***************************************************************************
 * Display the "timestamp" part of the report (JSON format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @cur_date	Date string of current comment.
 * @cur_time	Time string of current comment.
 * @utc		True if @cur_time is expressed in UTC.
 * @itv		Interval of time with preceding record.
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_json_timestamp(int *tab, int action, char *cur_date,
				      char *cur_time, int utc, unsigned long long itv)
{
	if (action & F_BEGIN) {
		xprintf0(*tab,
			 "\"timestamp\": {\"date\": \"%s\", \"time\": \"%s\", "
			 "\"utc\": %d, \"interval\": %llu}",
			 cur_date, cur_time, utc ? 1 : 0, itv);
	}
	if (action & F_MAIN) {
		printf("\n");
	}
	if (action & F_END) {
		printf("\n");
	}
}

/*
 ***************************************************************************
 * Display the header of the report (XML format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @dfile	Name of system activity data file.
 * @file_magic	System activity file magic header.
 * @file_hdr	System activity file standard header.
 * @cpu_nr	Number of processors for current daily data file.
 * @act		Array of activities (unused here).
 * @id_seq	Activity sequence (unused here).
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_xml_header(int *tab, int action, char *dfile,
				  struct file_magic *file_magic,
				  struct file_header *file_hdr, __nr_t cpu_nr,
				  struct activity *act[], unsigned int id_seq[])
{
	struct tm rectime;
	char cur_time[32];

	if (action & F_BEGIN) {
		printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		printf("<!DOCTYPE sysstat PUBLIC \"DTD v%s sysstat //EN\"\n",
		       XML_DTD_VERSION);
		printf("\"http://pagesperso-orange.fr/sebastien.godard/sysstat.dtd\">\n");
		
		xprintf(*tab, "<sysstat>");

		xprintf(++(*tab), "<sysdata-version>%s</sysdata-version>",
			XML_DTD_VERSION);

		xprintf(*tab, "<host nodename=\"%s\">", file_hdr->sa_nodename);
		xprintf(++(*tab), "<sysname>%s</sysname>", file_hdr->sa_sysname);
		xprintf(*tab, "<release>%s</release>", file_hdr->sa_release);

		xprintf(*tab, "<machine>%s</machine>", file_hdr->sa_machine);
		xprintf(*tab, "<number-of-cpus>%d</number-of-cpus>",
			cpu_nr > 1 ? cpu_nr - 1 : 1);

		/* Fill file timestmap structure (rectime) */
		get_file_timestamp_struct(flags, &rectime, file_hdr);
		strftime(cur_time, 32, "%Y-%m-%d", &rectime);
		xprintf(*tab, "<file-date>%s</file-date>", cur_time);
	}
	if (action & F_END) {
		xprintf(--(*tab), "</host>");
		xprintf(--(*tab), "</sysstat>");
	}
}

/*
 ***************************************************************************
 * Display the header of the report (JSON format).
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Action expected from current function.
 * @dfile	Name of system activity data file.
 * @file_magic	System activity file magic header.
 * @file_hdr	System activity file standard header.
 * @cpu_nr	Number of processors for current daily data file.
 * @act		Array of activities (unused here).
 * @id_seq	Activity sequence (unused here).
 *
 * OUT:
 * @tab		Number of tabulations.
 ***************************************************************************
 */
__printf_funct_t print_json_header(int *tab, int action, char *dfile,
				   struct file_magic *file_magic,
				   struct file_header *file_hdr, __nr_t cpu_nr,
				   struct activity *act[], unsigned int id_seq[])
{
	struct tm rectime;
	char cur_time[32];

	if (action & F_BEGIN) {
		xprintf(*tab, "{\"sysstat\": {");

		xprintf(++(*tab), "\"sysdata-version\": %s,",
			XML_DTD_VERSION);

		xprintf(*tab, "\"hosts\": [");
		xprintf(++(*tab), "{");
		xprintf(++(*tab), "\"nodename\": \"%s\",", file_hdr->sa_nodename);
		xprintf(*tab, "\"sysname\": \"%s\",", file_hdr->sa_sysname);
		xprintf(*tab, "\"release\": \"%s\",", file_hdr->sa_release);

		xprintf(*tab, "\"machine\": \"%s\",", file_hdr->sa_machine);
		xprintf(*tab, "\"number-of-cpus\": %d,",
			cpu_nr > 1 ? cpu_nr - 1 : 1);

		/* Fill file timestmap structure (rectime) */
		get_file_timestamp_struct(flags, &rectime, file_hdr);
		strftime(cur_time, 32, "%Y-%m-%d", &rectime);
		xprintf0(*tab, "\"file-date\": \"%s\"", cur_time);
	}
	if (action & F_END) {
		printf("\n");
		xprintf(--(*tab), "}");
		xprintf(--(*tab), "]");
		xprintf(--(*tab), "}}");
	}
}

/*
 ***************************************************************************
 * Display data file header.
 *
 * IN:
 * @tab		Number of tabulations (unused here).
 * @action	Action expected from current function.
 * @dfile	Name of system activity data file.
 * @file_magic	System activity file magic header.
 * @file_hdr	System activity file standard header.
 * @cpu_nr	Number of processors for current daily data file.
 * @act		Array of activities.
 * @id_seq	Activity sequence.
 ***************************************************************************
 */
__printf_funct_t print_hdr_header(int *tab, int action, char *dfile,
				  struct file_magic *file_magic,
				  struct file_header *file_hdr, __nr_t cpu_nr,
				  struct activity *act[], unsigned int id_seq[])
{
	int i, p;

	/* Actions F_BEGIN and F_END ignored */
	if (action & F_BEGIN) {
		printf(_("System activity data file: %s (%#x)\n"),
		       dfile, file_magic->format_magic);

		display_sa_file_version(file_magic);

		if (file_magic->format_magic != FORMAT_MAGIC) {
			return;
		}

		printf(_("Host: "));
		print_gal_header(localtime((const time_t *) &(file_hdr->sa_ust_time)),
				 file_hdr->sa_sysname, file_hdr->sa_release,
				 file_hdr->sa_nodename, file_hdr->sa_machine,
				 cpu_nr > 1 ? cpu_nr - 1 : 1);

		printf(_("Size of a long int: %d\n"), file_hdr->sa_sizeof_long);

		printf(_("List of activities:\n"));

		for (i = 0; i < NR_ACT; i++) {
			if (!id_seq[i])
				continue;
			if ((p = get_activity_position(act, id_seq[i])) < 0) {
				PANIC(id_seq[i]);
			}
			printf("%02d: %s\t(x%d)", act[p]->id, act[p]->name, act[p]->nr);
			if (act[p]->f_count2 || (act[p]->nr2 > 1)) {
				printf("\t(x%d)", act[p]->nr2);
			}
			if (act[p]->magic == ACTIVITY_MAGIC_UNKNOWN) {
				printf(_("\t[Unknown activity format]"));
			}
			printf("\n");
		}
	}
}
