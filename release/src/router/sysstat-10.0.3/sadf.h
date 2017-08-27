/*
 * sadf: System activity data formatter
 * (C) 1999-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _SADF_H
#define _SADF_H

#include "sa.h"

/* DTD version for XML output */
#define XML_DTD_VERSION	"2.13"

/* Possible actions for functions used to display reports */
#define F_BEGIN	0x01
#define F_MAIN	0x02
#define F_END	0x04

/*
 ***************************************************************************
 * Output format identification values.
 ***************************************************************************
 */

/* Number of output formats */
#define NR_FMT	5

/* Output formats */
#define F_DB_OUTPUT	1
#define F_HEADER_OUTPUT	2
#define F_PPC_OUTPUT	3
#define F_XML_OUTPUT	4
#define F_JSON_OUTPUT	5

/*
 ***************************************************************************
 * Generic description of an output format.
 ***************************************************************************
 */

/* Format options */
#define FO_NULL			0x00

/*
 * Indicate that all statistics data for one activity should be displayed before
 * displaying stats for next activity. This is what sar does in its report.
 * Example: If stats for activities A and B at time t and t' have been collected,
 * setting AO_GROUPED_STATS for a format will result in the following output:
 * stats for activity A at t
 * stats for activity A at t'
 * stats for activity B at t
 * stats for activity B at t'
 * Without this option, output would be:
 * stats for activity A at t
 * stats for activity B at t
 * stats for activity A at t'
 * stats for activity B at t'
 */
#define FO_GROUPED_STATS	0x01

/*
 * Indicate that output should stop after the header is displayed.
 */
#define FO_HEADER_ONLY		0x02

/*
 * Indicate that a true sysstat activity file but with a bad
 * format should not yield an error message.
 */
#define FO_BAD_FILE_FORMAT	0x04

/*
 * Indicate that timestamp can be displayed in local time instead of UTC
 * if option -t has been used.
 */
#define FO_TRUE_TIME		0x08

/*
 * Indicate that all activities will be displayed horizontally
 * if option -h is used.
 */
#define FO_HORIZONTALLY		0x10

/*
 * Indicate that the timestamp can be displayed in seconds since the epoch
 * if option -T has been used.
 */
#define FO_SEC_EPOCH		0x20

/*
 * Indicate that the list of fields should be displayed before the first
 * line of statistics.
 */
#define FO_FIELD_LIST		0x40

#define DISPLAY_GROUPED_STATS(m)	(((m) & FO_GROUPED_STATS)	== FO_GROUPED_STATS)
#define ACCEPT_HEADER_ONLY(m)		(((m) & FO_HEADER_ONLY)		== FO_HEADER_ONLY)
#define ACCEPT_BAD_FILE_FORMAT(m)	(((m) & FO_BAD_FILE_FORMAT)	== FO_BAD_FILE_FORMAT)
#define ACCEPT_TRUE_TIME(m)		(((m) & FO_TRUE_TIME)		== FO_TRUE_TIME)
#define ACCEPT_HORIZONTALLY(m)		(((m) & FO_HORIZONTALLY)	== FO_HORIZONTALLY)
#define ACCEPT_SEC_EPOCH(m)		(((m) & FO_SEC_EPOCH)		== FO_SEC_EPOCH)
#define DISPLAY_FIELD_LIST(m)		(((m) & FO_FIELD_LIST)		== FO_FIELD_LIST)

/* Type for all functions used by sadf to display stats in various formats */
#define __printf_funct_t void

/*
 * Structure used to define a report.
 * A XML-like report has the following format:
 *       __
 *      |
 *      | Header block
 *      |  __
 *      | |
 *      | | Statistics block
 *      | |  __
 *      | | |
 *      | | | Timestamp block
 *      | | |  __
 *      | | | |
 *      | | | | Activity #1
 *      | | | |__
 *      | | | |
 *      | | | | ...
 *      | | | |__
 *      | | | |
 *      | | | | Activity #n
 *      | | | |__
 *      | | |__
 *      | |__
 *      | |
 *      | | Restart messages block
 *      | |__
 *      | |
 *      | | Comments block
 *      | |__
 *      |__
 */
struct report_format {
	/*
	 * This variable contains the identification value (F_...) for this report format.
	 */
	unsigned int id;
	/*
	 * Format options (FO_...).
	 */
	unsigned int options;
	/*
	 * This function displays the report header
	 * (data displayed once at the beginning of the report).
	 */
	__printf_funct_t (*f_header) (int *, int, char *, struct file_magic *, struct file_header *,
				      __nr_t, struct activity * [], unsigned int []);
	/*
	 * This function defines the statistics part of the report.
	 * Used only with textual (XML-like) reports.
	 */
	__printf_funct_t (*f_statistics) (int *, int);
	/*
	 * This function defines the timestamp part of the report.
	 * Used only with textual (XML-like) reports.
	 */
	__printf_funct_t (*f_timestamp) (int *, int, char *, char *, int, unsigned long long);
	/*
	 * This function displays the restart messages.
	 */
	__printf_funct_t (*f_restart) (int *, int, char *, char *, int, struct file_header *);
	/*
	 * This function displays the comments.
	 */
	__printf_funct_t (*f_comment) (int *, int, char *, char *, int, char *, struct file_header *);
};

/*
 ***************************************************************************
 * Various function prototypes
 ***************************************************************************
 */

extern void
	xprintf(int, const char *, ...);
extern void
	xprintf0(int, const char *, ...);

/*
 * Prototypes used to display restart messages
 */
__printf_funct_t
	print_db_restart(int *, int, char *, char *, int, struct file_header *);
__printf_funct_t
	print_ppc_restart(int *, int, char *, char *, int, struct file_header *);
__printf_funct_t
	print_xml_restart(int *, int, char *, char *, int, struct file_header *);
__printf_funct_t
	print_json_restart(int *, int, char *, char *, int, struct file_header *);

/*
 * Prototypes used to display comments
 */
__printf_funct_t
	print_db_comment(int *, int, char *, char *, int, char *, struct file_header *);
__printf_funct_t
	print_ppc_comment(int *, int, char *, char *, int, char *, struct file_header *);
__printf_funct_t
	print_xml_comment(int *, int, char *, char *, int, char *, struct file_header *);
__printf_funct_t
	print_json_comment(int *, int, char *, char *, int, char *, struct file_header *);

/*
 * Prototypes used to display the statistics part of the report
 */
__printf_funct_t
	print_xml_statistics(int *, int);
__printf_funct_t
	print_json_statistics(int *, int);

/*
 * Prototypes used to display the timestamp part of the report
 */
__printf_funct_t
	print_xml_timestamp(int *, int, char *, char *, int, unsigned long long);
__printf_funct_t
	print_json_timestamp(int *, int, char *, char *, int, unsigned long long);

/*
 * Prototypes used to display the report header
 */
__printf_funct_t
	print_xml_header(int *, int, char *, struct file_magic *, struct file_header *,
			 __nr_t, struct activity * [], unsigned int []);
__printf_funct_t
	print_json_header(int *, int, char *, struct file_magic *, struct file_header *,
			  __nr_t, struct activity * [], unsigned int []);
__printf_funct_t
	print_hdr_header(int *, int, char *, struct file_magic *, struct file_header *,
			 __nr_t, struct activity * [], unsigned int []);

#endif  /* _SADF_H */
