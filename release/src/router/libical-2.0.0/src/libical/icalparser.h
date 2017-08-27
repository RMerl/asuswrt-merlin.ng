/*======================================================================
 FILE: icalparser.h
 CREATOR: eric 20 April 1999

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalparser.h
======================================================================*/

#ifndef ICALPARSER_H
#define ICALPARSER_H

#include "libical_ical_export.h"
#include "icalcomponent.h"

typedef struct icalparser_impl icalparser;

/**
 * @file  icalparser.h
 * @brief Line-oriented parsing.
 *
 * Create a new parser via icalparse_new_parser, then add lines one at
 * a time with icalparse_add_line(). icalparser_add_line() will return
 * non-zero when it has finished with a component.
 */

typedef enum icalparser_state
{
    ICALPARSER_ERROR,
    ICALPARSER_SUCCESS,
    ICALPARSER_BEGIN_COMP,
    ICALPARSER_END_COMP,
    ICALPARSER_IN_PROGRESS
} icalparser_state;

LIBICAL_ICAL_EXPORT icalparser *icalparser_new(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalparser_add_line(icalparser *parser, char *str);

LIBICAL_ICAL_EXPORT icalcomponent *icalparser_clean(icalparser *parser);

LIBICAL_ICAL_EXPORT icalparser_state icalparser_get_state(icalparser *parser);

LIBICAL_ICAL_EXPORT void icalparser_free(icalparser *parser);

/**
 * Message oriented parsing.  icalparser_parse takes a string that
 * holds the text ( in RFC 5545 format ) and returns a pointer to an
 * icalcomponent. The caller owns the memory. line_gen_func is a
 * pointer to a function that returns one content line per invocation
 */

LIBICAL_ICAL_EXPORT icalcomponent *icalparser_parse(icalparser *parser,
                                                    char *(*line_gen_func) (char *s,
                                                                            size_t size, void *d));

/**
   Set the data that icalparser_parse will give to the line_gen_func
   as the parameter 'd'
 */
LIBICAL_ICAL_EXPORT void icalparser_set_gen_data(icalparser *parser, void *data);

LIBICAL_ICAL_EXPORT icalcomponent *icalparser_parse_string(const char *str);

/***********************************************************************
 * Parser support functions
 ***********************************************************************/

/** Use the flex/bison parser to turn a string into a value type */
LIBICAL_ICAL_EXPORT icalvalue *icalparser_parse_value(icalvalue_kind kind,
                                                      const char *str, icalcomponent ** errors);

/** Given a line generator function, return a single iCal content line.*/
LIBICAL_ICAL_EXPORT char *icalparser_get_line(icalparser *parser,
                                              char *(*line_gen_func) (char *s,
                                                                      size_t size, void *d));

LIBICAL_ICAL_EXPORT char *icalparser_string_line_generator(char *out, size_t buf_size, void *d);

#endif /* !ICALPARSE_H */
