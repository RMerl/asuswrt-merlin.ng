/*
 * format.c: Output format definitions for sadf
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

#include "sadf.h"

/*
 ***************************************************************************
 * Definitions of output formats.
 * See sadf.h file for format structure definition.
 ***************************************************************************
 */

/*
 * Display only datafile header.
 */
struct report_format hdr_fmt = {
	.id		= F_HEADER_OUTPUT,
	.options	= FO_HEADER_ONLY + FO_BAD_FILE_FORMAT,
	.f_header	= print_hdr_header,
	.f_statistics	= NULL,
	.f_timestamp	= NULL,
	.f_restart	= NULL,
	.f_comment	= NULL
};

/*
 * Database friendly format.
 */
struct report_format db_fmt = {
	.id		= F_DB_OUTPUT,
	.options	= FO_GROUPED_STATS + FO_TRUE_TIME + FO_HORIZONTALLY +
			  FO_SEC_EPOCH + FO_FIELD_LIST,
	.f_header	= NULL,
	.f_statistics	= NULL,
	.f_timestamp	= NULL,
	.f_restart	= print_db_restart,
	.f_comment	= print_db_comment
};

/*
 * Format easily handled by pattern processing commands like awk.
 */
struct report_format ppc_fmt = {
	.id		= F_PPC_OUTPUT,
	.options	= FO_GROUPED_STATS + FO_TRUE_TIME + FO_SEC_EPOCH,
	.f_header	= NULL,
	.f_statistics	= NULL,
	.f_timestamp	= NULL,
	.f_restart	= print_ppc_restart,
	.f_comment	= print_ppc_comment
};

/*
 * XML output.
 */
struct report_format xml_fmt = {
	.id		= F_XML_OUTPUT,
	.options	= FO_HEADER_ONLY + FO_TRUE_TIME,
	.f_header	= print_xml_header,
	.f_statistics	= print_xml_statistics,
	.f_timestamp	= print_xml_timestamp,
	.f_restart	= print_xml_restart,
	.f_comment	= print_xml_comment
};

/*
 * JSON output.
 */
struct report_format json_fmt = {
	.id		= F_JSON_OUTPUT,
	.options	= FO_HEADER_ONLY + FO_TRUE_TIME,
	.f_header	= print_json_header,
	.f_statistics	= print_json_statistics,
	.f_timestamp	= print_json_timestamp,
	.f_restart	= print_json_restart,
	.f_comment	= print_json_comment
};

/*
 * Array of output formats.
 */
struct report_format *fmt[NR_FMT] = {
	&hdr_fmt,
	&db_fmt,
	&ppc_fmt,
	&xml_fmt,
	&json_fmt
};
