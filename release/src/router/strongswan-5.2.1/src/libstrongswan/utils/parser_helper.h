/*
 * Copyright (C) 2014 Tobias Brunner
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

/**
 * @defgroup parser_helper parser_helper
 * @{ @ingroup utils
 */

#ifndef PARSER_HELPER_H_
#define PARSER_HELPER_H_

#include <utils/debug.h>
#include <collections/array.h>
#include <bio/bio_writer.h>

typedef struct parser_helper_t parser_helper_t;

/**
 * Helper class for flex/bison based parsers.
 *
 * <code>PREFIX</code> equals whatever is configure with
 * <code>%option prefix</code> resp. <code>%name-prefix</code>.
 */
struct parser_helper_t {

	/**
	 * A user defined parser context object.
	 */
	const void *context;

	/**
	 * Opaque object allocated by the lexer, should be set with:
	 * @code
	 * PREFIXlex_init_extra(helper, &helper->scanner).
	 * @endcode
	 */
	void *scanner;

	/**
	 * Function to determine the current line number (defined by the lexer).
	 *
	 * Basically, this should be assigned to <code>PREFIXget_lineno</code>.
	 *
	 * @param scanner	the lexer
	 * @return			current line number
	 */
	int (*get_lineno)(void *scanner);

	/**
	 * Resolves the given include pattern, relative to the location of the
	 * current file.
	 *
	 * Call file_next() to open the next file.
	 *
	 * @param pattern	file pattern
	 */
	void (*file_include)(parser_helper_t *this, char *pattern);

	/**
	 * Get the next file to process.
	 *
	 * This will return NULL if all files matching the most recent pattern
	 * have been handled. If there are other patterns the next call will then
	 * return the next file matching the previous pattern.
	 *
	 * When hitting <code>\<\<EOF\>\></code> first call
	 * @code
	 * PREFIXpop_buffer_state(yyscanner);
	 * @endcode
	 * then call this method to check if there are more files to include for
	 * the most recent call to file_include(), if so, call
	 * @code
	 * PREFIXset_in(file, helper->scanner);
	 * PREFIXpush_buffer_state(PREFIX_create_buffer(file, YY_BUF_SIZE,
	 * 						helper->scanner), helper->scanner);
	 * @endcode
	 *
	 * If there are no more files to process check
	 * <code>YY_CURRENT_BUFFER</code> and if it is FALSE call yyterminate().
	 *
	 * @return			next file to process, or NULL (see comment)
	 */
	FILE *(*file_next)(parser_helper_t *this);

	/**
	 * Start parsing a string, discards any currently stored data.
	 */
	void (*string_init)(parser_helper_t *this);

	/**
	 * Append the given string.
	 *
	 * @param str		string to append
	 */
	void (*string_add)(parser_helper_t *this, char *str);

	/**
	 * Extract the current string buffer as null-terminated string. Can only
	 * be called once per string.
	 *
	 * @return			allocated string
	 */
	char *(*string_get)(parser_helper_t *this);

	/**
	 * Destroy this instance.
	 */
	void (*destroy)(parser_helper_t *this);
};

/**
 * Log the given message either as error or warning
 *
 * @param level		log level
 * @param ctx		current parser context
 * @param fmt		error message format
 * @param ...		additional arguments
 */
void parser_helper_log(int level, parser_helper_t *ctx, char *fmt, ...);

#if DEBUG_LEVEL >= 1
# define PARSER_DBG1(ctx, fmt, ...) parser_helper_log(1, ctx, fmt, ##__VA_ARGS__)
#endif
#if DEBUG_LEVEL >= 2
# define PARSER_DBG2(ctx, fmt, ...) parser_helper_log(2, ctx, fmt, ##__VA_ARGS__)
#endif
#if DEBUG_LEVEL >= 3
# define PARSER_DBG3(ctx, fmt, ...) parser_helper_log(3, ctx, fmt, ##__VA_ARGS__)
#endif

#ifndef PARSER_DBG1
# define PARSER_DBG1(...) {}
#endif
#ifndef PARSER_DBG2
# define PARSER_DBG2(...) {}
#endif
#ifndef PARSER_DBG3
# define PARSER_DBG3(...) {}
#endif

/**
 * Create a parser helper object
 *
 * @param context		user defined parser context
 * @return				parser helper
 */
parser_helper_t *parser_helper_create(void *context);

#endif /** PARSER_HELPER_H_ @}*/
