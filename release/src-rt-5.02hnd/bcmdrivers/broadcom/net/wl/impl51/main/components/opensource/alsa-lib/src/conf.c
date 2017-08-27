/**
 * \file conf.c
 * \ingroup Configuration
 * \brief Configuration helper functions
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2001
 *
 * Tree based, full nesting configuration functions.
 *
 * See the \ref conf page for more details.
 */
/*
 *  Configuration helper functions
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>,
 *			  Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*! \page conf Configuration files

<P>Configuration files use a simple format allowing modern
data description like nesting and array assignments.</P>

\section conf_whitespace Whitespace

Whitespace is the collective name given to spaces (blanks), horizontal and
vertical tabs, newline characters, and comments. Whitespace can
indicate where configuration tokens start and end, but beyond this function,
any surplus whitespace is discarded. For example, the two sequences

\code
  a 1 b 2
\endcode

and

\code
  a 1 
     b 2
\endcode

are lexically equivalent and parse identically to give the four tokens:

\code
a
1
b
2
\endcode

The ASCII characters representing whitespace can occur within literal
strings, in which case they are protected from the normal parsing process
(they remain as part of the string). For example:

\code
  name "John Smith"
\endcode

parses to two tokens, including the single literal-string token "John
Smith".

\section conf_linesplicing Line continuation with \

A special case occurs if a newline character in a string is preceded
by a backslash (\). The backslash and the new line are both discarded,
allowing two physical lines of text to be treated as one unit.

\code
"John \
Smith"
\endcode

is parsed as "John Smith".

\section conf_comments Comments

A single-line comment begins with the character #. The comment can start
at any position, and extends to the end of the line.

\code
  a 1  # this is a comment
\endcode

\section conf_include Including configuration files

To include another configuration file, write the file name in angle brackets.
The prefix \c confdir: will reference the global configuration directory.

\code
</etc/alsa1.conf>
<confdir:pcm/surround.conf>
\endcode

\section conf_punctuators Punctuators

The configuration punctuators (also known as separators) are:

\code
  {} [] , ; = . ' " new-line form-feed carriage-return whitespace
\endcode

\subsection conf_braces Braces

Opening and closing braces { } indicate the start and end of a compound
statement:

\code
a {
  b 1
}
\endcode

\subsection conf_brackets Brackets

Opening and closing brackets indicate a single array definition. The
identifiers are automatically generated starting with zero.

\code
a [
  "first"
  "second"
]
\endcode

The above code is equal to

\code
a.0 "first"
a.1 "second"
\endcode

\subsection conf_comma_semicolon Comma and semicolon

The comma (,) or semicolon (;) can separate value assignments. It is not
strictly required to use these separators because whitespace suffices to
separate tokens.

\code
a 1;
b 1,
\endcode

\subsection conf_equal Equal sign

The equal sign (=) can separate variable declarations from
initialization lists:

\code
a=1
b=2
\endcode

Using equal signs is not required because whitespace suffices to separate
tokens.

\section conf_assigns Assignments

The configuration file defines id (key) and value pairs. The id (key) can be
composed from ASCII digits, characters from a to z and A to Z, and the
underscore (_). The value can be either a string, an integer, a real number,
or a compound statement.

\subsection conf_single Single assignments

\code
a 1	# is equal to
a=1	# is equal to
a=1;	# is equal to
a 1,
\endcode

\subsection conf_compound Compound assignments (definitions using braces)

\code
a {
  b = 1
}
a={
  b 1,
}
\endcode

\section conf_compound1 Compound assignments (one key definitions)

\code
a.b 1
a.b=1
\endcode

\subsection conf_array Array assignments (definitions using brackets)

\code
a [
  "first"
  "second"
]
\endcode

\subsection conf_array1 Array assignments (one key definitions)

\code
a.0 "first"
a.1 "second"
\endcode

\section conf_mode Operation modes for parsing nodes

By default, the node operation mode is 'merge+create', i.e., if
a configuration node is not present a new one is created, otherwise
the latest assignment is merged (if possible - type checking). The
'merge+create' operation mode is specified with the prefix character plus (+).

The operation mode 'merge' merges the node with the old one (which must
exist). Type checking is done, so strings cannot be assigned to integers
and so on. This mode is specified with the prefix character minus (-).

The operation mode 'do not override' ignores a new configuration node
if a configuration node with the same name exists. This mode is specified with
the prefix character question mark (?).

The operation mode 'override' always overrides the old configuration node
with new contents. This mode is specified with the prefix character
exclamation mark (!).

\code
defaults.pcm.!device 1
\endcode

\section conf_syntax_summary Syntax summary

\code
# Configuration file syntax

# Include a new configuration file
<filename>

# Simple assignment
name [=] value [,|;]

# Compound assignment (first style)
name [=] {
        name1 [=] value [,|;]
        ...
}

# Compound assignment (second style)
name.name1 [=] value [,|;]

# Array assignment (first style)
name [
        value0 [,|;]
        value1 [,|;]
        ...
]

# Array assignment (second style)
name.0 [=] value0 [,|;]
name.1 [=] value1 [,|;]
\endcode

\section conf_syntax_ref References

\ref confarg
\ref conffunc
\ref confhooks

*/

/*! \page confarg Runtime arguments in configuration files

<P>The ALSA library can accept runtime arguments for some configuration
blocks. This extension is built on top of the basic configuration file
syntax.<P>

\section confarg_define Defining arguments

Arguments are defined using the id (key) \c \@args and array values containing
the string names of the arguments:

\code
@args [ CARD ]	# or
@args.0 CARD
\endcode

\section confarg_type Defining argument types and default values

An argument's type is specified with the id (key) \c \@args and the argument
name. The type and the default value are specified in the compound block:

\code
@args.CARD {
  type string
  default "abcd"
}
\endcode

\section confarg_refer Referring to arguments

Arguments are referred to with a dollar-sign ($) and the name of the argument:

\code
  card $CARD
\endcode

\section confarg_usage Usage

To use a block with arguments, write the argument values after the key,
separated with a colon (:). For example, all these names for PCM interfaces
give the same result:

\code
hw:0,1
hw:CARD=0,DEV=1
hw:{CARD 0 DEV 1}
plug:"hw:0,1"
plug:{SLAVE="hw:{CARD 0 DEV 1}"}
\endcode

As you see, arguments can be specified in their proper order or by name.
Note that arguments enclosed in braces are parsed in the same way as in
configuration files, but using the override method by default.

\section confarg_example Example

\code
pcm.demo {
	@args [ CARD DEVICE ]
	@args.CARD {
		type string
		default "supersonic"
	}
	@args.DEVICE {
		type integer
		default 0
	}
	type hw
	card $CARD
	device $DEVICE
}
\endcode

*/

/*! \page conffunc Runtime functions in configuration files

<P>The ALSA library can modify the configuration at runtime.
Several built-in functions are available.</P>

<P>A function is defined with the id \c \@func and the function name. All other
values in the current compound are used as configuration for the function.
If the compound func.\<function_name\> is defined in the root node, then the
library and function from this compound configuration are used, otherwise
'snd_func_' is prefixed to the string and code from the ALSA library is used.
The definition of a function looks like:</P> 

\code
func.remove_first_char {
	lib "/usr/lib/libasoundextend.so"
	func "extend_remove_first_char"
}
\endcode

*/

/*! \page confhooks Hooks in configuration files

<P>The hook extension in the ALSA library allows expansion of configuration
nodes at run-time. The existence of a hook is determined by the
presence of a \@hooks compound node.</P>

<P>This example defines a hook which loads two configuration files at the
beginning:</P>

\code
@hooks [
	{
		func load
		files [
			"/etc/asound.conf"
			"~/.asoundrc"
		]
		errors false
	}
]
\endcode

\section confhooks_ref Function reference

<UL>
  <LI>The function load - \c snd_config_hook_load() - loads and parses the
      given configuration files.
  <LI>The function load_for_all_cards - \c snd_config_hook_load_for_all_cards() -
      loads and parses the given configuration files for each installed sound
      card. The driver name (the type of the sound card) is passed in the
      private configuration node.
</UL>

*/


#include <stdarg.h>
#include <limits.h>
#include <sys/stat.h>
#include <locale.h>
#include "local.h"
#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#ifndef DOC_HIDDEN

struct _snd_config {
	char *id;
	snd_config_type_t type;
	union {
		long integer;
		long long integer64;
		char *string;
		double real;
		const void *ptr;
		struct {
			struct list_head fields;
			int join;
		} compound;
	} u;
	struct list_head list;
	snd_config_t *parent;
	int hop;
};

struct filedesc {
	char *name;
	snd_input_t *in;
	unsigned int line, column;
	struct filedesc *next;
};

#define LOCAL_ERROR			(-0x68000000)

#define LOCAL_UNTERMINATED_STRING 	(LOCAL_ERROR - 0)
#define LOCAL_UNTERMINATED_QUOTE	(LOCAL_ERROR - 1)
#define LOCAL_UNEXPECTED_CHAR		(LOCAL_ERROR - 2)
#define LOCAL_UNEXPECTED_EOF		(LOCAL_ERROR - 3)

typedef struct {
	struct filedesc *current;
	int unget;
	int ch;
} input_t;

static int safe_strtoll(const char *str, long long *val)
{
	long long v;
	int endidx;
	if (!*str)
		return -EINVAL;
	errno = 0;
	if (sscanf(str, "%Li%n", &v, &endidx) < 1)
		return -EINVAL;
	if (str[endidx])
		return -EINVAL;
	*val = v;
	return 0;
}

int safe_strtol(const char *str, long *val)
{
	char *end;
	long v;
	if (!*str)
		return -EINVAL;
	errno = 0;
	v = strtol(str, &end, 0);
	if (errno)
		return -errno;
	if (*end)
		return -EINVAL;
	*val = v;
	return 0;
}

static int safe_strtod(const char *str, double *val)
{
	char *end;
	double v;
	char *saved_locale;
	char locstr[64]; /* enough? */
	int err;

	if (!*str)
		return -EINVAL;
	saved_locale = setlocale(LC_NUMERIC, NULL);
	if (saved_locale) {
		snprintf(locstr, sizeof(locstr), "%s", saved_locale);
		setlocale(LC_NUMERIC, "C");
	}
	errno = 0;
	v = strtod(str, &end);
	err = -errno;
	if (saved_locale)
		setlocale(LC_NUMERIC, locstr);
	if (err)
		return err;
	if (*end)
		return -EINVAL;
	*val = v;
	return 0;
}

static int get_char(input_t *input)
{
	int c;
	struct filedesc *fd;
	if (input->unget) {
		input->unget = 0;
		return input->ch;
	}
 again:
	fd = input->current;
	c = snd_input_getc(fd->in);
	switch (c) {
	case '\n':
		fd->column = 0;
		fd->line++;
		break;
	case '\t':
		fd->column += 8 - fd->column % 8;
		break;
	case EOF:
		if (fd->next) {
			snd_input_close(fd->in);
			free(fd->name);
			input->current = fd->next;
			free(fd);
			goto again;
		}
		return LOCAL_UNEXPECTED_EOF;
	default:
		fd->column++;
		break;
	}
	return (unsigned char)c;
}

static void unget_char(int c, input_t *input)
{
	assert(!input->unget);
	input->ch = c;
	input->unget = 1;
}

static int get_delimstring(char **string, int delim, input_t *input);

static int get_char_skip_comments(input_t *input)
{
	int c;
	while (1) {
		c = get_char(input);
		if (c == '<') {
			char *str;
			snd_input_t *in;
			struct filedesc *fd;
			int err = get_delimstring(&str, '>', input);
			if (err < 0)
				return err;
			if (!strncmp(str, "confdir:", 8)) {
				char *tmp = malloc(strlen(ALSA_CONFIG_DIR) + 1 + strlen(str + 8) + 1);
				if (tmp == NULL) {
					free(str);
					return -ENOMEM;
				}
				sprintf(tmp, ALSA_CONFIG_DIR "/%s", str + 8);
				free(str);
				str = tmp;
			}
			err = snd_input_stdio_open(&in, str, "r");
			if (err < 0) {
				SNDERR("Cannot access file %s", str);
				free(str);
				return err;
			}
			fd = malloc(sizeof(*fd));
			if (!fd) {
				free(str);
				return -ENOMEM;
			}
			fd->name = str;
			fd->in = in;
			fd->next = input->current;
			fd->line = 1;
			fd->column = 0;
			input->current = fd;
			continue;
		}
		if (c != '#')
			break;
		while (1) {
			c = get_char(input);
			if (c < 0)
				return c;
			if (c == '\n')
				break;
		}
	}
		
	return c;
}
			

static int get_nonwhite(input_t *input)
{
	int c;
	while (1) {
		c = get_char_skip_comments(input);
		switch (c) {
		case ' ':
		case '\f':
		case '\t':
		case '\n':
		case '\r':
			break;
		default:
			return c;
		}
	}
}

static int get_quotedchar(input_t *input)
{
	int c;
	c = get_char(input);
	switch (c) {
	case 'n':
		return '\n';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case 'b':
		return '\b';
	case 'r':
		return '\r';
	case 'f':
		return '\f';
	case '0' ... '7':
	{
		int num = c - '0';
		int i = 1;
		do {
			c = get_char(input);
			if (c < '0' || c > '7') {
				unget_char(c, input);
				break;
			}
			num = num * 8 + c - '0';
			i++;
		} while (i < 3);
		return num;
	}
	default:
		return c;
	}
}

#define LOCAL_STR_BUFSIZE	64
struct local_string {
	char *buf;
	size_t alloc;
	size_t idx;
	char tmpbuf[LOCAL_STR_BUFSIZE];
};

static void init_local_string(struct local_string *s)
{
	memset(s, 0, sizeof(*s));
	s->buf = s->tmpbuf;
	s->alloc = LOCAL_STR_BUFSIZE;
}

static void free_local_string(struct local_string *s)
{
	if (s->buf != s->tmpbuf)
		free(s->buf);
}

static int add_char_local_string(struct local_string *s, int c)
{
	if (s->idx >= s->alloc) {
		size_t nalloc = s->alloc * 2;
		if (s->buf == s->tmpbuf) {
			s->buf = malloc(nalloc);
			if (s->buf == NULL)
				return -ENOMEM;
			memcpy(s->buf, s->tmpbuf, s->alloc);
		} else {
			char *ptr = realloc(s->buf, nalloc);
			if (ptr == NULL)
				return -ENOMEM;
			s->buf = ptr;
		}
		s->alloc = nalloc;
	}
	s->buf[s->idx++] = c;
	return 0;
}

static char *copy_local_string(struct local_string *s)
{
	char *dst = malloc(s->idx + 1);
	if (dst) {
		memcpy(dst, s->buf, s->idx);
		dst[s->idx] = '\0';
	}
	return dst;
}

static int get_freestring(char **string, int id, input_t *input)
{
	struct local_string str;
	int c;

	init_local_string(&str);
	while (1) {
		c = get_char(input);
		if (c < 0) {
			if (c == LOCAL_UNEXPECTED_EOF) {
				*string = copy_local_string(&str);
				if (! *string)
					c = -ENOMEM;
				else
					c = 0;
			}
			break;
		}
		switch (c) {
		case '.':
			if (!id)
				break;
		case ' ':
		case '\f':
		case '\t':
		case '\n':
		case '\r':
		case '=':
		case ',':
		case ';':
		case '{':
		case '}':
		case '[':
		case ']':
		case '\'':
		case '"':
		case '\\':
		case '#':
			*string = copy_local_string(&str);
			if (! *string)
				c = -ENOMEM;
			else {
				unget_char(c, input);
				c = 0;
			}
			goto _out;
		default:
			break;
		}
		if (add_char_local_string(&str, c) < 0) {
			c = -ENOMEM;
			break;
		}
	}
 _out:
	free_local_string(&str);
	return c;
}
			
static int get_delimstring(char **string, int delim, input_t *input)
{
	struct local_string str;
	int c;

	init_local_string(&str);
	while (1) {
		c = get_char(input);
		if (c < 0)
			break;
		if (c == '\\') {
			c = get_quotedchar(input);
			if (c < 0)
				break;
			if (c == '\n')
				continue;
		} else if (c == delim) {
			*string = copy_local_string(&str);
			if (! *string)
				c = -ENOMEM;
			else
				c = 0;
			break;
		}
		if (add_char_local_string(&str, c) < 0) {
			c = -ENOMEM;
			break;
		}
	}
	 free_local_string(&str);
	 return c;
}

/* Return 0 for free string, 1 for delimited string */
static int get_string(char **string, int id, input_t *input)
{
	int c = get_nonwhite(input), err;
	if (c < 0)
		return c;
	switch (c) {
	case '=':
	case ',':
	case ';':
	case '.':
	case '{':
	case '}':
	case '[':
	case ']':
	case '\\':
		return LOCAL_UNEXPECTED_CHAR;
	case '\'':
	case '"':
		err = get_delimstring(string, c, input);
		if (err < 0)
			return err;
		return 1;
	default:
		unget_char(c, input);
		err = get_freestring(string, id, input);
		if (err < 0)
			return err;
		return 0;
	}
}

static int _snd_config_make(snd_config_t **config, char **id, snd_config_type_t type)
{
	snd_config_t *n;
	assert(config);
	n = calloc(1, sizeof(*n));
	if (n == NULL) {
		if (*id) {
			free(*id);
			*id = NULL;
		}
		return -ENOMEM;
	}
	if (id) {
		n->id = *id;
		*id = NULL;
	}
	n->type = type;
	if (type == SND_CONFIG_TYPE_COMPOUND)
		INIT_LIST_HEAD(&n->u.compound.fields);
	*config = n;
	return 0;
}
	

static int _snd_config_make_add(snd_config_t **config, char **id,
				snd_config_type_t type, snd_config_t *parent)
{
	snd_config_t *n;
	int err;
	assert(parent->type == SND_CONFIG_TYPE_COMPOUND);
	err = _snd_config_make(&n, id, type);
	if (err < 0)
		return err;
	n->parent = parent;
	list_add_tail(&n->list, &parent->u.compound.fields);
	*config = n;
	return 0;
}

static int _snd_config_search(snd_config_t *config, 
			      const char *id, int len, snd_config_t **result)
{
	snd_config_iterator_t i, next;
	snd_config_for_each(i, next, config) {
		snd_config_t *n = snd_config_iterator_entry(i);
		if (len < 0) {
			if (strcmp(n->id, id) != 0)
				continue;
		} else if (strlen(n->id) != (size_t) len ||
			   memcmp(n->id, id, (size_t) len) != 0)
				continue;
		if (result)
			*result = n;
		return 0;
	}
	return -ENOENT;
}

static int parse_value(snd_config_t **_n, snd_config_t *parent, input_t *input, char **id, int skip)
{
	snd_config_t *n = *_n;
	char *s;
	int err;

	err = get_string(&s, 0, input);
	if (err < 0)
		return err;
	if (skip) {
		free(s);
		return 0;
	}
	if (err == 0 && ((s[0] >= '0' && s[0] <= '9') || s[0] == '-')) {
		long long i;
		errno = 0;
		err = safe_strtoll(s, &i);
		if (err < 0) {
			double r;
			err = safe_strtod(s, &r);
			if (err >= 0) {
				free(s);
				if (n) {
					if (n->type != SND_CONFIG_TYPE_REAL) {
						SNDERR("%s is not a real", *id);
						return -EINVAL;
					}
				} else {
					err = _snd_config_make_add(&n, id, SND_CONFIG_TYPE_REAL, parent);
					if (err < 0)
						return err;
				}
				n->u.real = r;
				*_n = n;
				return 0;
			}
		} else {
			free(s);
			if (n) {
				if (n->type != SND_CONFIG_TYPE_INTEGER && n->type != SND_CONFIG_TYPE_INTEGER64) {
					SNDERR("%s is not an integer", *id);
					return -EINVAL;
				}
			} else {
				if (i <= INT_MAX) 
					err = _snd_config_make_add(&n, id, SND_CONFIG_TYPE_INTEGER, parent);
				else
					err = _snd_config_make_add(&n, id, SND_CONFIG_TYPE_INTEGER64, parent);
				if (err < 0)
					return err;
			}
			if (n->type == SND_CONFIG_TYPE_INTEGER) 
				n->u.integer = (long) i;
			else 
				n->u.integer64 = i;
			*_n = n;
			return 0;
		}
	}
	if (n) {
		if (n->type != SND_CONFIG_TYPE_STRING) {
			SNDERR("%s is not a string", *id);
			free(s);
			return -EINVAL;
		}
	} else {
		err = _snd_config_make_add(&n, id, SND_CONFIG_TYPE_STRING, parent);
		if (err < 0)
			return err;
	}
	free(n->u.string);
	n->u.string = s;
	*_n = n;
	return 0;
}

static int parse_defs(snd_config_t *parent, input_t *input, int skip, int override);
static int parse_array_defs(snd_config_t *farther, input_t *input, int skip, int override);

static int parse_array_def(snd_config_t *parent, input_t *input, int idx, int skip, int override)
{
	char *id = NULL;
	int c;
	int err;
	snd_config_t *n = NULL;

	if (!skip) {
		char static_id[12];
		snprintf(static_id, sizeof(static_id), "%i", idx);
		id = strdup(static_id);
		if (id == NULL)
			return -ENOMEM;
	}
	c = get_nonwhite(input);
	if (c < 0) {
		err = c;
		goto __end;
	}
	switch (c) {
	case '{':
	case '[':
	{
		char endchr;
		if (!skip) {
			if (n) {
				if (n->type != SND_CONFIG_TYPE_COMPOUND) {
					SNDERR("%s is not a compound", id);
					err = -EINVAL;
					goto __end;
				}
			} else {
				err = _snd_config_make_add(&n, &id, SND_CONFIG_TYPE_COMPOUND, parent);
				if (err < 0)
					goto __end;
			}
		}
		if (c == '{') {
			err = parse_defs(n, input, skip, override);
			endchr = '}';
		} else {
			err = parse_array_defs(n, input, skip, override);
			endchr = ']';
		}
		c = get_nonwhite(input);
		if (c < 0) {
			err = c;
			goto __end;
		}
		if (c != endchr) {
			if (n)
				snd_config_delete(n);
			err = LOCAL_UNEXPECTED_CHAR;
			goto __end;
		}
		break;
	}
	default:
		unget_char(c, input);
		err = parse_value(&n, parent, input, &id, skip);
		if (err < 0)
			goto __end;
		break;
	}
	err = 0;
      __end:
	free(id);
      	return err;
}

static int parse_array_defs(snd_config_t *parent, input_t *input, int skip, int override)
{
	int idx = 0;
	while (1) {
		int c = get_nonwhite(input), err;
		if (c < 0)
			return c;
		unget_char(c, input);
		if (c == ']')
			return 0;
		err = parse_array_def(parent, input, idx++, skip, override);
		if (err < 0)
			return err;
	}
	return 0;
}

static int parse_def(snd_config_t *parent, input_t *input, int skip, int override)
{
	char *id = NULL;
	int c;
	int err;
	snd_config_t *n;
	enum {MERGE_CREATE, MERGE, OVERRIDE, DONT_OVERRIDE} mode;
	while (1) {
		c = get_nonwhite(input);
		if (c < 0)
			return c;
		switch (c) {
		case '+':
			mode = MERGE_CREATE;
			break;
		case '-':
			mode = MERGE;
			break;
		case '?':
			mode = DONT_OVERRIDE;
			break;
		case '!':
			mode = OVERRIDE;
			break;
		default:
			mode = !override ? MERGE_CREATE : OVERRIDE;
			unget_char(c, input);
		}
		err = get_string(&id, 1, input);
		if (err < 0)
			return err;
		c = get_nonwhite(input);
		if (c != '.')
			break;
		if (skip) {
			free(id);
			continue;
		}
		if (_snd_config_search(parent, id, -1, &n) == 0) {
			if (mode == DONT_OVERRIDE) {
				skip = 1;
				free(id);
				continue;
			}
			if (mode != OVERRIDE) {
				if (n->type != SND_CONFIG_TYPE_COMPOUND) {
					SNDERR("%s is not a compound", id);
					return -EINVAL;
				}
				n->u.compound.join = 1;
				parent = n;
				free(id);
				continue;
			}
			snd_config_delete(n);
		}
		if (mode == MERGE) {
			SNDERR("%s does not exists", id);
			err = -ENOENT;
			goto __end;
		}
		err = _snd_config_make_add(&n, &id, SND_CONFIG_TYPE_COMPOUND, parent);
		if (err < 0)
			goto __end;
		n->u.compound.join = 1;
		parent = n;
	}
	if (c == '=') {
		c = get_nonwhite(input);
		if (c < 0)
			return c;
	}
	if (!skip) {
		if (_snd_config_search(parent, id, -1, &n) == 0) {
			if (mode == DONT_OVERRIDE) {
				skip = 1;
				n = NULL;
			} else if (mode == OVERRIDE) {
				snd_config_delete(n);
				n = NULL;
			}
		} else {
			n = NULL;
			if (mode == MERGE) {
				SNDERR("%s does not exists", id);
				err = -ENOENT;
				goto __end;
			}
		}
	}
	switch (c) {
	case '{':
	case '[':
	{
		char endchr;
		if (!skip) {
			if (n) {
				if (n->type != SND_CONFIG_TYPE_COMPOUND) {
					SNDERR("%s is not a compound", id);
					err = -EINVAL;
					goto __end;
				}
			} else {
				err = _snd_config_make_add(&n, &id, SND_CONFIG_TYPE_COMPOUND, parent);
				if (err < 0)
					goto __end;
			}
		}
		if (c == '{') {
			err = parse_defs(n, input, skip, override);
			endchr = '}';
		} else {
			err = parse_array_defs(n, input, skip, override);
			endchr = ']';
		}
		c = get_nonwhite(input);
		if (c != endchr) {
			if (n)
				snd_config_delete(n);
			err = LOCAL_UNEXPECTED_CHAR;
			goto __end;
		}
		break;
	}
	default:
		unget_char(c, input);
		err = parse_value(&n, parent, input, &id, skip);
		if (err < 0)
			goto __end;
		break;
	}
	c = get_nonwhite(input);
	switch (c) {
	case ';':
	case ',':
		break;
	default:
		unget_char(c, input);
	}
      __end:
	free(id);
	return err;
}
		
static int parse_defs(snd_config_t *parent, input_t *input, int skip, int override)
{
	int c, err;
	while (1) {
		c = get_nonwhite(input);
		if (c < 0)
			return c == LOCAL_UNEXPECTED_EOF ? 0 : c;
		unget_char(c, input);
		if (c == '}')
			return 0;
		err = parse_def(parent, input, skip, override);
		if (err < 0)
			return err;
	}
	return 0;
}

static void string_print(char *str, int id, snd_output_t *out)
{
	unsigned char *p = (unsigned char *)str;
	if (!p || !*p) {
		snd_output_puts(out, "''");
		return;
	}
	if (!id) {
		switch (*p) {
		case '0' ... '9':
		case '-':
			goto quoted;
		}
	}
 loop:
	switch (*p) {
	case 0:
		goto nonquoted;
	case 1 ... 31:
	case 127 ... 255:
	case ' ':
	case '=':
	case ';':
	case ',':
	case '.':
	case '{':
	case '}':
	case '\'':
	case '"':
		goto quoted;
	default:
		p++;
		goto loop;
	}
 nonquoted:
	snd_output_puts(out, str);
	return;
 quoted:
	snd_output_putc(out, '\'');
	p = (unsigned char *)str;
	while (*p) {
		int c;
		c = *p;
		switch (c) {
		case '\n':
			snd_output_putc(out, '\\');
			snd_output_putc(out, 'n');
			break;
		case '\t':
			snd_output_putc(out, '\\');
			snd_output_putc(out, 't');
			break;
		case '\v':
			snd_output_putc(out, '\\');
			snd_output_putc(out, 'v');
			break;
		case '\b':
			snd_output_putc(out, '\\');
			snd_output_putc(out, 'b');
			break;
		case '\r':
			snd_output_putc(out, '\\');
			snd_output_putc(out, 'r');
			break;
		case '\f':
			snd_output_putc(out, '\\');
			snd_output_putc(out, 'f');
			break;
		case '\'':
			snd_output_putc(out, '\\');
			snd_output_putc(out, c);
			break;
		case 32 ... '\'' - 1:
		case '\'' + 1 ... 126:
			snd_output_putc(out, c);
			break;
		default:
			snd_output_printf(out, "\\%04o", c);
			break;
		}
		p++;
	}
	snd_output_putc(out, '\'');
}

static int _snd_config_save_children(snd_config_t *config, snd_output_t *out,
				     unsigned int level, unsigned int joins);

static int _snd_config_save_node_value(snd_config_t *n, snd_output_t *out,
				       unsigned int level)
{
	int err;
	unsigned int k;
	switch (n->type) {
	case SND_CONFIG_TYPE_INTEGER:
		snd_output_printf(out, "%ld", n->u.integer);
		break;
	case SND_CONFIG_TYPE_INTEGER64:
		snd_output_printf(out, "%Ld", n->u.integer64);
		break;
	case SND_CONFIG_TYPE_REAL:
		snd_output_printf(out, "%-16g", n->u.real);
		break;
	case SND_CONFIG_TYPE_STRING:
		string_print(n->u.string, 0, out);
		break;
	case SND_CONFIG_TYPE_POINTER:
		SNDERR("cannot save runtime pointer type");
		return -EINVAL;
	case SND_CONFIG_TYPE_COMPOUND:
		snd_output_putc(out, '{');
		snd_output_putc(out, '\n');
		err = _snd_config_save_children(n, out, level + 1, 0);
		if (err < 0)
			return err;
		for (k = 0; k < level; ++k) {
			snd_output_putc(out, '\t');
		}
		snd_output_putc(out, '}');
		break;
	}
	return 0;
}

static void id_print(snd_config_t *n, snd_output_t *out, unsigned int joins)
{
	if (joins > 0) {
		assert(n->parent);
		id_print(n->parent, out, joins - 1);
		snd_output_putc(out, '.');
	}
	string_print(n->id, 1, out);
}

static int _snd_config_save_children(snd_config_t *config, snd_output_t *out,
				     unsigned int level, unsigned int joins)
{
	unsigned int k;
	int err;
	snd_config_iterator_t i, next;
	assert(config && out);
	snd_config_for_each(i, next, config) {
		snd_config_t *n = snd_config_iterator_entry(i);
		if (n->type == SND_CONFIG_TYPE_COMPOUND &&
		    n->u.compound.join) {
			err = _snd_config_save_children(n, out, level, joins + 1);
			if (err < 0)
				return err;
			continue;
		}
		for (k = 0; k < level; ++k) {
			snd_output_putc(out, '\t');
		}
		id_print(n, out, joins);
		snd_output_putc(out, ' ');
		err = _snd_config_save_node_value(n, out, level);
		if (err < 0)
			return err;
		snd_output_putc(out, '\n');
	}
	return 0;
}
#endif


/**
 * \brief Substitutes one configuration node to another.
 * \param dst Handle to the destination node.
 * \param src Handle to the source node. Must not be the same as \a dst.
 * \return Zero if successful, otherwise a negative error code.
 *
 * If both nodes are compounds, the source compound node members are
 * appended to the destination compound node.
 *
 * If the destination node is a compound and the source node is
 * an ordinary type, the compound members are deleted (including
 * their contents).
 *
 * Otherwise, the source node's value replaces the destination node's
 * value.
 *
 * In any case, a successful call to this function frees the source
 * node.
 */
int snd_config_substitute(snd_config_t *dst, snd_config_t *src)
{
	assert(dst && src);
	if (dst->type == SND_CONFIG_TYPE_COMPOUND &&
	    src->type == SND_CONFIG_TYPE_COMPOUND) {	/* append */
		snd_config_iterator_t i, next;
		snd_config_for_each(i, next, src) {
			snd_config_t *n = snd_config_iterator_entry(i);
			n->parent = dst;
		}
		src->u.compound.fields.next->prev = &dst->u.compound.fields;
		src->u.compound.fields.prev->next = &dst->u.compound.fields;
	} else if (dst->type == SND_CONFIG_TYPE_COMPOUND) {
		int err;
		err = snd_config_delete_compound_members(dst);
		if (err < 0)
			return err;
	}
	free(dst->id);
	dst->id = src->id;
	dst->type = src->type;
	dst->u = src->u;
	free(src);
	return 0;
}

/**
 * \brief Converts an ASCII string to a configuration node type.
 * \param[in] ascii A string containing a configuration node type.
 * \param[out] type The node type corresponding to \a ascii.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function recognizes at least the following node types:
 * <dl>
 * <dt>integer<dt>#SND_CONFIG_TYPE_INTEGER
 * <dt>integer64<dt>#SND_CONFIG_TYPE_INTEGER64
 * <dt>real<dt>#SND_CONFIG_TYPE_REAL
 * <dt>string<dt>#SND_CONFIG_TYPE_STRING
 * <dt>compound<dt>#SND_CONFIG_TYPE_COMPOUND
 * </dl>
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>Unknown note type in \a type.
 * </dl>
 */
int snd_config_get_type_ascii(const char *ascii, snd_config_type_t *type)
{
	assert(ascii && type);
	if (!strcmp(ascii, "integer")) {
		*type = SND_CONFIG_TYPE_INTEGER;
		return 0;
	}
	if (!strcmp(ascii, "integer64")) {
		*type = SND_CONFIG_TYPE_INTEGER64;
		return 0;
	}
	if (!strcmp(ascii, "real")) {
		*type = SND_CONFIG_TYPE_REAL;
		return 0;
	}
	if (!strcmp(ascii, "string")) {
		*type = SND_CONFIG_TYPE_STRING;
		return 0;
	}
	if (!strcmp(ascii, "compound")) {
		*type = SND_CONFIG_TYPE_COMPOUND;
		return 0;
	}
	return -EINVAL;
}

/**
 * \brief Returns the type of a configuration node.
 * \param config Handle to the configuration node.
 * \return The node's type.
 *
 * \par Conforming to:
 * LSB 3.2
 */
snd_config_type_t snd_config_get_type(const snd_config_t *config)
{
	return config->type;
}

/**
 * \brief Returns the id of a configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] id The function puts the pointer to the id string at the
 *                address specified by \a id.
 * \return Zero if successful, otherwise a negative error code.
 *
 * The returned string is owned by the configuration node; the application
 * must not modify or delete it, and the string becomes invalid when the
 * node's id changes or when the node is freed.
 *
 * If the node does not have an id, \a *id is set to \c NULL.
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_get_id(const snd_config_t *config, const char **id)
{
	assert(config && id);
	*id = config->id;
	return 0;
}

/**
 * \brief Sets the id of a configuration node.
 * \param config Handle to the configuration node.
 * \param id The new node id, must not be \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function stores a copy of \a id in the node.
 *
 * \par Errors:
 * <dl>
 * <dt>-EEXIST<dd>One of \a config's siblings already has the id \a id.
 * <dt>-EINVAL<dd>The id of a node with a parent cannot be set to \c NULL.
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 */
int snd_config_set_id(snd_config_t *config, const char *id)
{
	snd_config_iterator_t i, next;
	char *new_id;
	assert(config);
	if (id) {
		if (config->parent) {
			snd_config_for_each(i, next, config->parent) {
				snd_config_t *n = snd_config_iterator_entry(i);
				if (n != config && strcmp(id, n->id) == 0)
					return -EEXIST;
			}
		}
		new_id = strdup(id);
		if (!new_id)
			return -ENOMEM;
	} else {
		if (config->parent)
			return -EINVAL;
		new_id = NULL;
	}
	free(config->id);
	config->id = new_id;
	return 0;
}

/**
 * \brief Creates a top level configuration node.
 * \param[out] config Handle to the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * The returned node is an empty compound node without a parent and
 * without an id.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_top(snd_config_t **config)
{
	assert(config);
	return _snd_config_make(config, 0, SND_CONFIG_TYPE_COMPOUND);
}

static int snd_config_load1(snd_config_t *config, snd_input_t *in, int override)
{
	int err;
	input_t input;
	struct filedesc *fd, *fd_next;
	assert(config && in);
	fd = malloc(sizeof(*fd));
	if (!fd)
		return -ENOMEM;
	fd->name = NULL;
	fd->in = in;
	fd->line = 1;
	fd->column = 0;
	fd->next = NULL;
	input.current = fd;
	input.unget = 0;
	err = parse_defs(config, &input, 0, override);
	fd = input.current;
	if (err < 0) {
		const char *str;
		switch (err) {
		case LOCAL_UNTERMINATED_STRING:
			str = "Unterminated string";
			err = -EINVAL;
			break;
		case LOCAL_UNTERMINATED_QUOTE:
			str = "Unterminated quote";
			err = -EINVAL;
			break;
		case LOCAL_UNEXPECTED_CHAR:
			str = "Unexpected char";
			err = -EINVAL;
			break;
		case LOCAL_UNEXPECTED_EOF:
			str = "Unexpected end of file";
			err = -EINVAL;
			break;
		default:
			str = strerror(-err);
			break;
		}
		SNDERR("%s:%d:%d:%s", fd->name ? fd->name : "_toplevel_", fd->line, fd->column, str);
		goto _end;
	}
	if (get_char(&input) != LOCAL_UNEXPECTED_EOF) {
		SNDERR("%s:%d:%d:Unexpected }", fd->name ? fd->name : "", fd->line, fd->column);
		err = -EINVAL;
		goto _end;
	}
 _end:
	while (fd->next) {
		fd_next = fd->next;
		snd_input_close(fd->in);
		free(fd->name);
		free(fd);
		fd = fd_next;
	}
	free(fd);
	return err;
}

/**
 * \brief Loads a configuration tree.
 * \param config Handle to a top level configuration node.
 * \param in Input handle to read the configuration from.
 * \return Zero if successful, otherwise a negative error code.
 *
 * The definitions loaded from the input are added to \a config, which
 * must be a compound node.
 *
 * \par Errors:
 * Any errors encountered when parsing the input or returned by hooks or
 * functions.
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_load(snd_config_t *config, snd_input_t *in)
{
	return snd_config_load1(config, in, 0);
}

/**
 * \brief Loads a configuration tree and overrides existing configuration nodes.
 * \param config Handle to a top level configuration node.
 * \param in Input handle to read the configuration from.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function loads definitions from \a in into \a config like
 * #snd_config_load, but the default mode for input nodes is 'override'
 * (!) instead of 'merge+create' (+).
 */
int snd_config_load_override(snd_config_t *config, snd_input_t *in)
{
	return snd_config_load1(config, in, 1);
}

/**
 * \brief Adds a child to a compound configuration node.
 * \param parent Handle to a compound configuration node.
 * \param child Handle to the configuration node to be added.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function makes the node \a child a child of the node \a parent.
 *
 * The parent node then owns the child node, i.e., the child node gets
 * deleted together with its parent.
 *
 * \a child must have an id.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a child does not have an id.
 * <dt>-EINVAL<dd>\a child already has a parent.
 * <dt>-EEXIST<dd>\a parent already contains a child node with the same
 *                id as \a child.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_add(snd_config_t *parent, snd_config_t *child)
{
	snd_config_iterator_t i, next;
	assert(parent && child);
	if (!child->id || child->parent)
		return -EINVAL;
	snd_config_for_each(i, next, parent) {
		snd_config_t *n = snd_config_iterator_entry(i);
		if (strcmp(child->id, n->id) == 0)
			return -EEXIST;
	}
	child->parent = parent;
	list_add_tail(&child->list, &parent->u.compound.fields);
	return 0;
}

/**
 * \brief Removes a configuration node from its tree.
 * \param config Handle to the configuration node to be removed.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function makes \a config a top-level node, i.e., if \a config
 * has a parent, then \a config is removed from the list of the parent's
 * children.
 *
 * This functions does \e not free the removed node.
 *
 * \sa snd_config_delete
 */
int snd_config_remove(snd_config_t *config)
{
	assert(config);
	if (config->parent)
		list_del(&config->list);
	config->parent = NULL;
	return 0;
}

/**
 * \brief Frees a configuration node.
 * \param config Handle to the configuration node to be deleted.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function frees a configuration node and all its resources.
 *
 * If the node is a child node, it is removed from the tree before being
 * deleted.
 *
 * If the node is a compound node, its descendants (the whole subtree)
 * are deleted recursively.
 *
 * \par Conforming to:
 * LSB 3.2
 *
 * \sa snd_config_remove
 */
int snd_config_delete(snd_config_t *config)
{
	assert(config);
	switch (config->type) {
	case SND_CONFIG_TYPE_COMPOUND:
	{
		int err;
		struct list_head *i;
		i = config->u.compound.fields.next;
		while (i != &config->u.compound.fields) {
			struct list_head *nexti = i->next;
			snd_config_t *child = snd_config_iterator_entry(i);
			err = snd_config_delete(child);
			if (err < 0)
				return err;
			i = nexti;
		}
		break;
	}
	case SND_CONFIG_TYPE_STRING:
		free(config->u.string);
		break;
	default:
		break;
	}
	if (config->parent)
		list_del(&config->list);
	free(config->id);
	free(config);
	return 0;
}

/**
 * \brief Deletes the children of a node.
 * \param config Handle to the compound configuration node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function removes and frees all children of a configuration node.
 *
 * Any compound nodes among the children of \a config are deleted
 * recursively.
 *
 * After a successful call to this function, \a config is an empty
 * compound node.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a compound node.
 * </dl>
 */
int snd_config_delete_compound_members(const snd_config_t *config)
{
	int err;
	struct list_head *i;

	assert(config);
	if (config->type != SND_CONFIG_TYPE_COMPOUND)
		return -EINVAL;
	i = config->u.compound.fields.next;
	while (i != &config->u.compound.fields) {
		struct list_head *nexti = i->next;
		snd_config_t *child = snd_config_iterator_entry(i);
		err = snd_config_delete(child);
		if (err < 0)
			return err;
		i = nexti;
	}
	return 0;
}

/**
 * \brief Creates a configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] type The type of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions creates a new node of the specified type.
 * The new node has id \a id, which may be \c NULL.
 *
 * The value of the new node is zero (for numbers), or \c NULL (for
 * strings and pointers), or empty (for compound nodes).
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 */
int snd_config_make(snd_config_t **config, const char *id,
		    snd_config_type_t type)
{
	char *id1;
	assert(config);
	if (id) {
		id1 = strdup(id);
		if (!id1)
			return -ENOMEM;
	} else
		id1 = NULL;
	return _snd_config_make(config, &id1, type);
}

/**
 * \brief Creates an integer configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_INTEGER and
 * with value \c 0.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 *
 * \sa snd_config_imake_integer
 */
int snd_config_make_integer(snd_config_t **config, const char *id)
{
	return snd_config_make(config, id, SND_CONFIG_TYPE_INTEGER);
}

/**
 * \brief Creates a 64-bit-integer configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_INTEGER64
 * and with value \c 0.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 *
 * \sa snd_config_imake_integer64
 */
int snd_config_make_integer64(snd_config_t **config, const char *id)
{
	return snd_config_make(config, id, SND_CONFIG_TYPE_INTEGER64);
}

/**
 * \brief Creates a real number configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_REAL and
 * with value \c 0.0.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \sa snd_config_imake_real
 */
int snd_config_make_real(snd_config_t **config, const char *id)
{
	return snd_config_make(config, id, SND_CONFIG_TYPE_REAL);
}

/**
 * \brief Creates a string configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_STRING and
 * with value \c NULL.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 *
 * \sa snd_config_imake_string
 */
int snd_config_make_string(snd_config_t **config, const char *id)
{
	return snd_config_make(config, id, SND_CONFIG_TYPE_STRING);
}

/**
 * \brief Creates a pointer configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_POINTER and
 * with value \c NULL.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \sa snd_config_imake_pointer
 */
int snd_config_make_pointer(snd_config_t **config, const char *id)
{
	return snd_config_make(config, id, SND_CONFIG_TYPE_POINTER);
}

/**
 * \brief Creates an empty compound configuration node.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] join Join flag.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new empty node of type
 * #SND_CONFIG_TYPE_COMPOUND.
 *
 * \a join determines how the compound node's id is printed when the
 * configuration is saved to a text file.  For example, if the join flag
 * of compound node \c a is zero, the output will look as follows:
 * \code
 * a {
 *     b "hello"
 *     c 42
 * }
 * \endcode
 * If, however, the join flag of \c a is nonzero, its id will be joined
 * with its children's ids, like this:
 * \code
 * a.b "hello"
 * a.c 42
 * \endcode
 * An \e empty compound node with its join flag set would result in no
 * output, i.e., after saving and reloading the configuration file, that
 * compound node would be lost.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_make_compound(snd_config_t **config, const char *id,
			     int join)
{
	int err;
	err = snd_config_make(config, id, SND_CONFIG_TYPE_COMPOUND);
	if (err < 0)
		return err;
	(*config)->u.compound.join = join;
	return 0;
}

/**
 * \brief Creates an integer configuration node with the given initial value.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] value The initial value of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_INTEGER and
 * with value \a value.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_imake_integer(snd_config_t **config, const char *id, const long value)
{
	int err;
	
	err = snd_config_make(config, id, SND_CONFIG_TYPE_INTEGER);
	if (err < 0)
		return err;
	(*config)->u.integer = value;
	return 0;
}

/**
 * \brief Creates a 64-bit-integer configuration node with the given initial value.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] value The initial value of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_INTEGER64
 * and with value \a value.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_imake_integer64(snd_config_t **config, const char *id, const long long value)
{
	int err;
	
	err = snd_config_make(config, id, SND_CONFIG_TYPE_INTEGER64);
	if (err < 0)
		return err;
	(*config)->u.integer64 = value;
	return 0;
}

/**
 * \brief Creates a real number configuration node with the given initial value.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] value The initial value of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_REAL and
 * with value \a value.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 */
int snd_config_imake_real(snd_config_t **config, const char *id, const double value)
{
	int err;
	
	err = snd_config_make(config, id, SND_CONFIG_TYPE_REAL);
	if (err < 0)
		return err;
	(*config)->u.real = value;
	return 0;
}

/**
 * \brief Creates a string configuration node with the given initial value.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] value The initial value of the new node.  May be \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_STRING and
 * with a copy of the string \c value.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_imake_string(snd_config_t **config, const char *id, const char *value)
{
	int err;
	snd_config_t *tmp;
	
	err = snd_config_make(&tmp, id, SND_CONFIG_TYPE_STRING);
	if (err < 0)
		return err;
	if (value) {
		tmp->u.string = strdup(value);
		if (!tmp->u.string) {
			snd_config_delete(tmp);
			return -ENOMEM;
		}
	} else {
		tmp->u.string = NULL;
	}
	*config = tmp;
	return 0;
}

/**
 * \brief Creates a pointer configuration node with the given initial value.
 * \param[out] config The function puts the handle to the new node at
 *                    the address specified by \a config.
 * \param[in] id The id of the new node.
 * \param[in] value The initial value of the new node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function creates a new node of type #SND_CONFIG_TYPE_POINTER and
 * with value \c value.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 */
int snd_config_imake_pointer(snd_config_t **config, const char *id, const void *value)
{
	int err;
	
	err = snd_config_make(config, id, SND_CONFIG_TYPE_POINTER);
	if (err < 0)
		return err;
	(*config)->u.ptr = value;
	return 0;
}

/**
 * \brief Changes the value of an integer configuration node.
 * \param config Handle to the configuration node.
 * \param value The new value for the node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not an integer node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_set_integer(snd_config_t *config, long value)
{
	assert(config);
	if (config->type != SND_CONFIG_TYPE_INTEGER)
		return -EINVAL;
	config->u.integer = value;
	return 0;
}

/**
 * \brief Changes the value of a 64-bit-integer configuration node.
 * \param config Handle to the configuration node.
 * \param value The new value for the node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a 64-bit-integer node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_set_integer64(snd_config_t *config, long long value)
{
	assert(config);
	if (config->type != SND_CONFIG_TYPE_INTEGER64)
		return -EINVAL;
	config->u.integer64 = value;
	return 0;
}

/**
 * \brief Changes the value of a real-number configuration node.
 * \param config Handle to the configuration node.
 * \param value The new value for the node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a real-number node.
 * </dl>
 */
int snd_config_set_real(snd_config_t *config, double value)
{
	assert(config);
	if (config->type != SND_CONFIG_TYPE_REAL)
		return -EINVAL;
	config->u.real = value;
	return 0;
}

/**
 * \brief Changes the value of a string configuration node.
 * \param config Handle to the configuration node.
 * \param value The new value for the node.  May be \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function deletes the old string in the node and stores a copy of
 * \a value string in the node.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a string node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_set_string(snd_config_t *config, const char *value)
{
	char *new_string;
	assert(config);
	if (config->type != SND_CONFIG_TYPE_STRING)
		return -EINVAL;
	if (value) {
		new_string = strdup(value);
		if (!new_string)
			return -ENOMEM;
	} else {
		new_string = NULL;
	}
	free(config->u.string);
	config->u.string = new_string;
	return 0;
}

/**
 * \brief Changes the value of a pointer configuration node.
 * \param config Handle to the configuration node.
 * \param value The new value for the node.  May be \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function does not free the old pointer in the node.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a pointer node.
 * </dl>
 */
int snd_config_set_pointer(snd_config_t *config, const void *value)
{
	assert(config);
	if (config->type != SND_CONFIG_TYPE_POINTER)
		return -EINVAL;
	config->u.ptr = value;
	return 0;
}

/**
 * \brief Changes the value of a configuration node.
 * \param config Handle to the configuration node.
 * \param ascii The new value for the node, as an ASCII string.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function changes the node's value to a new value that is parsed
 * from the string \a ascii.  \a ascii must not be \c NULL, not even for
 * a string node.
 *
 * The node's type does not change, i.e., the string must contain a
 * valid value with the same type as the node's type.  For a string
 * node, the node's new value is a copy of \a ascii.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a number or string node.
 * <dt>-EINVAL<dd>The value in \a ascii cannot be parsed.
 * <dt>-ERANGE<dd>The value in \a ascii is too big for the node's type.
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_set_ascii(snd_config_t *config, const char *ascii)
{
	assert(config && ascii);
	switch (config->type) {
	case SND_CONFIG_TYPE_INTEGER:
		{
			long i;
			int err = safe_strtol(ascii, &i);
			if (err < 0)
				return err;
			config->u.integer = i;
		}
		break;
	case SND_CONFIG_TYPE_INTEGER64:
		{
			long long i;
			int err = safe_strtoll(ascii, &i);
			if (err < 0)
				return err;
			config->u.integer64 = i;
		}
		break;
	case SND_CONFIG_TYPE_REAL:
		{
			double d;
			int err = safe_strtod(ascii, &d);
			if (err < 0)
				return err;
			config->u.real = d;
			break;
		}
	case SND_CONFIG_TYPE_STRING:
		{
			char *ptr = strdup(ascii);
			if (ptr == NULL)
				return -ENOMEM;
			free(config->u.string);
			config->u.string = ptr;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * \brief Returns the value of an integer configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] ptr The node's value.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not an integer node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_get_integer(const snd_config_t *config, long *ptr)
{
	assert(config && ptr);
	if (config->type != SND_CONFIG_TYPE_INTEGER)
		return -EINVAL;
	*ptr = config->u.integer;
	return 0;
}

/**
 * \brief Returns the value of a 64-bit-integer configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] ptr The node's value.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a 64-bit-integer node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_get_integer64(const snd_config_t *config, long long *ptr)
{
	assert(config && ptr);
	if (config->type != SND_CONFIG_TYPE_INTEGER64)
		return -EINVAL;
	*ptr = config->u.integer64;
	return 0;
}

/**
 * \brief Returns the value of a real-number configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] ptr The node's value.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a real-number node.
 * </dl>
 */
int snd_config_get_real(const snd_config_t *config, double *ptr)
{
	assert(config && ptr);
	if (config->type != SND_CONFIG_TYPE_REAL)
		return -EINVAL;
	*ptr = config->u.real;
	return 0;
}

/**
 * \brief Returns the value of a real or integer configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] ptr The node's value.
 * \return Zero if successful, otherwise a negative error code.
 *
 * If the node's type is integer or integer64, the value is converted
 * to the \c double type on the fly.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a number node.
 * </dl>
 */
int snd_config_get_ireal(const snd_config_t *config, double *ptr)
{
	assert(config && ptr);
	if (config->type == SND_CONFIG_TYPE_REAL)
		*ptr = config->u.real;
	else if (config->type == SND_CONFIG_TYPE_INTEGER)
		*ptr = config->u.integer;
	else if (config->type == SND_CONFIG_TYPE_INTEGER64)
		*ptr = config->u.integer64;
	else
		return -EINVAL;
	return 0;
}

/**
 * \brief Returns the value of a string configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] ptr The function puts the node's value at the address
 *                 specified by \a ptr.
 * \return Zero if successful, otherwise a negative error code.
 *
 * The returned string is owned by the configuration node; the
 * application must not modify or delete it, and the string becomes
 * invalid when the node's value changes or when the node is freed.
 *
 * The string may be \c NULL.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a string node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_get_string(const snd_config_t *config, const char **ptr)
{
	assert(config && ptr);
	if (config->type != SND_CONFIG_TYPE_STRING)
		return -EINVAL;
	*ptr = config->u.string;
	return 0;
}

/**
 * \brief Returns the value of a pointer configuration node.
 * \param[in] config Handle to the configuration node.
 * \param[out] ptr The function puts the node's value at the address
 *                 specified by \a ptr.
 * \return Zero if successful, otherwise a negative error code.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a string node.
 * </dl>
 */
int snd_config_get_pointer(const snd_config_t *config, const void **ptr)
{
	assert(config && ptr);
	if (config->type != SND_CONFIG_TYPE_POINTER)
		return -EINVAL;
	*ptr = config->u.ptr;
	return 0;
}

/**
 * \brief Returns the value of a configuration node as a string.
 * \param[in] config Handle to the configuration node.
 * \param[out] ascii The function puts the pointer to the returned
 *                   string at the address specified by \a ascii.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function dynamically allocates the returned string.  The
 * application is responsible for deleting it with \c free() when it is
 * no longer used.
 *
 * For a string node with \c NULL value, the returned string is \c NULL.
 *
 * Supported node types are #SND_CONFIG_TYPE_INTEGER,
 * #SND_CONFIG_TYPE_INTEGER64, #SND_CONFIG_TYPE_REAL, and
 * #SND_CONFIG_TYPE_STRING.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>\a config is not a (64-bit) integer or real number or
 *                string node.
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_get_ascii(const snd_config_t *config, char **ascii)
{
	assert(config && ascii);
	switch (config->type) {
	case SND_CONFIG_TYPE_INTEGER:
		{
			char res[12];
			int err;
			err = snprintf(res, sizeof(res), "%li", config->u.integer);
			if (err < 0 || err == sizeof(res)) {
				assert(0);
				return -ENOMEM;
			}
			*ascii = strdup(res);
		}
		break;
	case SND_CONFIG_TYPE_INTEGER64:
		{
			char res[32];
			int err;
			err = snprintf(res, sizeof(res), "%Li", config->u.integer64);
			if (err < 0 || err == sizeof(res)) {
				assert(0);
				return -ENOMEM;
			}
			*ascii = strdup(res);
		}
		break;
	case SND_CONFIG_TYPE_REAL:
		{
			char res[32];
			int err;
			err = snprintf(res, sizeof(res), "%-16g", config->u.real);
			if (err < 0 || err == sizeof(res)) {
				assert(0);
				return -ENOMEM;
			}
			if (res[0]) {		/* trim the string */
				char *ptr;
				ptr = res + strlen(res) - 1;
				while (ptr != res && *ptr == ' ')
					ptr--;
				if (*ptr != ' ')
					ptr++;
				*ptr = '\0';
			}
			*ascii = strdup(res);
		}
		break;
	case SND_CONFIG_TYPE_STRING:
		if (config->u.string)
			*ascii = strdup(config->u.string);
		else {
			*ascii = NULL;
			return 0;
		}
		break;
	default:
		return -EINVAL;
	}
	if (*ascii == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * \brief Compares the id of a configuration node to a given string.
 * \param config Handle to the configuration node.
 * \param id ASCII id.
 * \return The same value as the result of the \c strcmp function, i.e.,
 *         less than zero if \a config's id is lexicographically less
 *         than \a id, zero if \a config's id is equal to id, greater
 *         than zero otherwise.
 */
int snd_config_test_id(const snd_config_t *config, const char *id)
{
	assert(config && id);
	if (config->id)
		return strcmp(config->id, id);
	else
		return -1;
}

/**
 * \brief Dumps the contents of a configuration node or tree.
 * \param config Handle to the (root) configuration node.
 * \param out Output handle.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function writes a textual representation of \a config's value to
 * the output \a out.
 *
 * \par Errors:
 * <dl>
 * <dt>-EINVAL<dd>A node in the tree has a type that cannot be printed,
 *                i.e., #SND_CONFIG_TYPE_POINTER.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_save(snd_config_t *config, snd_output_t *out)
{
	assert(config && out);
	if (config->type == SND_CONFIG_TYPE_COMPOUND)
		return _snd_config_save_children(config, out, 0, 0);
	else
		return _snd_config_save_node_value(config, out, 0);
}

/*
 *  *** search macros ***
 */

#ifndef DOC_HIDDEN

#define SND_CONFIG_SEARCH(config, key, result, extra_code) \
{ \
	snd_config_t *n; \
	int err; \
	const char *p; \
	assert(config && key); \
	while (1) { \
		if (config->type != SND_CONFIG_TYPE_COMPOUND) \
			return -ENOENT; \
		{ extra_code ; } \
		p = strchr(key, '.'); \
		if (p) { \
			err = _snd_config_search(config, key, p - key, &n); \
			if (err < 0) \
				return err; \
			config = n; \
			key = p + 1; \
		} else \
			return _snd_config_search(config, key, -1, result); \
	} \
}

#define SND_CONFIG_SEARCHA(root, config, key, result, fcn, extra_code) \
{ \
	snd_config_t *n; \
	int err; \
	const char *p; \
	assert(config && key); \
	while (1) { \
		if (config->type != SND_CONFIG_TYPE_COMPOUND) { \
			if (snd_config_get_string(config, &p) < 0) \
				return -ENOENT; \
			err = fcn(root, root, p, &config); \
			if (err < 0) \
				return err; \
		} \
		{ extra_code ; } \
		p = strchr(key, '.'); \
		if (p) { \
			err = _snd_config_search(config, key, p - key, &n); \
			if (err < 0) \
				return err; \
			config = n; \
			key = p + 1; \
		} else \
			return _snd_config_search(config, key, -1, result); \
	} \
}

#define SND_CONFIG_SEARCHV(config, result, fcn) \
{ \
	snd_config_t *n; \
	va_list arg; \
	assert(config); \
	va_start(arg, result); \
	while (1) { \
		const char *k = va_arg(arg, const char *); \
		int err; \
		if (!k) \
			break; \
		err = fcn(config, k, &n); \
		if (err < 0) \
			return err; \
		config = n; \
	} \
	va_end(arg); \
	if (result) \
		*result = n; \
	return 0; \
}

#define SND_CONFIG_SEARCHVA(root, config, result, fcn) \
{ \
	snd_config_t *n; \
	va_list arg; \
	assert(config); \
	va_start(arg, result); \
	while (1) { \
		const char *k = va_arg(arg, const char *); \
		int err; \
		if (!k) \
			break; \
		err = fcn(root, config, k, &n); \
		if (err < 0) \
			return err; \
		config = n; \
	} \
	va_end(arg); \
	if (result) \
		*result = n; \
	return 0; \
}

#define SND_CONFIG_SEARCH_ALIAS(config, base, key, result, fcn1, fcn2) \
{ \
	snd_config_t *res = NULL; \
	char *old_key; \
	int err, first = 1, maxloop = 1000; \
	assert(config && key); \
	while (1) { \
		old_key = strdup(key); \
		if (old_key == NULL) { \
			err = -ENOMEM; \
			res = NULL; \
			break; \
		} \
		err = first && base ? -EIO : fcn1(config, config, key, &res); \
		if (err < 0) { \
			if (!base) \
				break; \
			err = fcn2(config, config, &res, base, key, NULL); \
			if (err < 0) \
				break; \
		} \
		if (snd_config_get_string(res, &key) < 0) \
			break; \
		assert(key); \
		if (!first && (strcmp(key, old_key) == 0 || maxloop <= 0)) { \
			if (maxloop == 0) \
				SNDERR("maximum loop count reached (circular configuration?)"); \
			else \
				SNDERR("key %s refers to itself", key); \
			err = -EINVAL; \
			res = NULL; \
			break; \
		} \
		free(old_key); \
		first = 0; \
		maxloop--; \
	} \
	free(old_key); \
	if (!res) \
		return err; \
	if (result) \
		*result = res; \
	return 0; \
}

#endif /* DOC_HIDDEN */

/**
 * \brief Searches for a node in a configuration tree.
 * \param[in] config Handle to the root of the configuration (sub)tree to search.
 * \param[in] key Search key: one or more node ids, separated with dots.
 * \param[out] result When \a result != \c NULL, the function puts the
 *                    handle to the node found at the address specified
 *                    by \a result.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function searches for a child node of \a config that is
 * identified by \a key, which contains either the id of a direct child
 * node of \a config, or a series of ids, separated with dots, where
 * each id specifies a node that is contained in the previous compound
 * node.
 *
 * In the following example, the comment after each node shows the
 * search key to find that node, assuming that \a config is a handle to
 * the compound node with id \c config:
 * \code
 * config {
 *     a 42               # "a"
 *     b {                # "b"
 *         c "cee"        # "b.c"
 *         d {            # "b.d"
 *             e 2.71828  # "b.d.e"
 *         }
 *     }
 * }
 * \endcode
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_search(snd_config_t *config, const char *key, snd_config_t **result)
{
	SND_CONFIG_SEARCH(config, key, result, );
}

/**
 * \brief Searches for a node in a configuration tree, expanding aliases.
 * \param[in] root Handle to the root configuration node containing
 *                 alias definitions.
 * \param[in] config Handle to the root of the configuration (sub)tree to search.
 * \param[in] key Search key: one or more node keys, separated with dots.
 * \param[out] result When \a result != \c NULL, the function puts the
 *                    handle to the node found at the address specified
 *                    by \a result.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions searches for a child node of \a config like
 * #snd_config_search.  However, any compound node can also be
 * identified by an alias, which is a string node whose value is taken
 * as the id of a compound node below \a root.
 *
 * \a root must be a compound node.
 * \a root and \a config may be the same node.
 *
 * For example, with the following configuration, the call
 * \code
 * snd_config_searcha(root, config, "a.b.c.d", &result);
 * \endcode
 * would return the node with id \c d:
 * \code
 * config {
 *     a {
 *         b bb
 *     }
 * }
 * root {
 *     bb {
 *         c cc
 *     }
 *     cc ccc
 *     ccc {
 *         d {
 *             x "icks"
 *         }
 *     }
 * }
 * \endcode
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key or an alias id does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound or string node.
 * </dl>
 */
int snd_config_searcha(snd_config_t *root, snd_config_t *config, const char *key, snd_config_t **result)
{
	SND_CONFIG_SEARCHA(root, config, key, result, snd_config_searcha, );
}

/**
 * \brief Searches for a node in a configuration tree.
 * \param[in] config Handle to the root of the configuration (sub)tree to search.
 * \param[out] result When \a result != \c NULL, the function puts the
 *                    handle to the node found at the address specified
 *                    by \a result.
 * \param[in] ... One or more concatenated dot-separated search keys,
 *                terminated with \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions searches for a child node of \a config like
 * #snd_config_search, but the search key is the concatenation of all
 * passed search key strings.  For example, the call
 * \code
 * snd_config_searchv(cfg, &res, "a", "b.c", "d.e", NULL);
 * \endcode
 * is equivalent to the call
 * \code
 * snd_config_search(cfg, "a.b.c.d.e", &res);
 * \endcode
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in a search key does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_searchv(snd_config_t *config, snd_config_t **result, ...)
{
	SND_CONFIG_SEARCHV(config, result, snd_config_search);
}

/**
 * \brief Searches for a node in a configuration tree, expanding aliases.
 * \param[in] root Handle to the root configuration node containing
 *                 alias definitions.
 * \param[in] config Handle to the root of the configuration (sub)tree to search.
 * \param[out] result When \a result != \c NULL, the function puts the
 *                    handle to the node found at the address specified
 *                    by \a result.
 * \param[in] ... One or more concatenated dot separated search keys,
 *                terminated with \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function searches for a child node of \a config, allowing
 * aliases, like #snd_config_searcha, but the search key is the
 * concatenation of all passed seach key strings, like with
 * #snd_config_searchv.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in a search key does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound or string node.
 * </dl>
 */
int snd_config_searchva(snd_config_t *root, snd_config_t *config, snd_config_t **result, ...)
{
	SND_CONFIG_SEARCHVA(root, config, result, snd_config_searcha);
}

/**
 * \brief Searches for a node in a configuration tree, expanding aliases.
 * \param[in] config Handle to the root of the configuration (sub)tree to search.
 * \param[in] base Search key base, or \c NULL.
 * \param[in] key Search key suffix.
 * \param[out] result When \a result != \c NULL, the function puts the
 *                    handle to the node found at the address specified
 *                    by \a result.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions searches for a child node of \a config, allowing
 * aliases, like #snd_config_searcha.  However, alias definitions are
 * searched below \a config (there is no separate \a root parameter),
 * and \a base specifies a seach key that identifies a compound node
 * that is used to search for an alias definitions that is not found
 * directly below \a config and that does not contain a period.  In
 * other words, when \c "id" is not found in \a config, this function
 * also tries \c "base.id".
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key or an alias id does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound or string node.
 * </dl>
 */
int snd_config_search_alias(snd_config_t *config,
			    const char *base, const char *key,
			    snd_config_t **result)
{
	SND_CONFIG_SEARCH_ALIAS(config, base, key, result,
				snd_config_searcha, snd_config_searchva);
}

static int snd_config_hooks(snd_config_t *config, snd_config_t *private_data);

/**
 * \brief Searches for a node in a configuration tree and expands hooks.
 * \param[in,out] config Handle to the root of the configuration
 *                       (sub)tree to search.
 * \param[in] key Search key: one or more node keys, separated with dots.
 * \param[out] result The function puts the handle to the node found at
 *                    the address specified by \a result.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions searches for a child node of \a config like
 * #snd_config_search, but any compound nodes to be searched that
 * contain hooks are modified by the respective hook functions.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 * Additionally, any errors encountered when parsing the hook
 * definitions or returned by the hook functions.
 */
int snd_config_search_hooks(snd_config_t *config, const char *key, snd_config_t **result)
{
	SND_CONFIG_SEARCH(config, key, result, \
					err = snd_config_hooks(config, NULL); \
					if (err < 0) \
						return err; \
			 );
}

/**
 * \brief Searches for a node in a configuration tree, expanding aliases and hooks.
 * \param[in] root Handle to the root configuration node containing
 *                 alias definitions.
 * \param[in,out] config Handle to the root of the configuration
 *                       (sub)tree to search.
 * \param[in] key Search key: one or more node keys, separated with dots.
 * \param[out] result The function puts the handle to the node found at
 *                    the address specified by \a result.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function searches for a child node of \a config, allowing
 * aliases, like #snd_config_searcha, and expanding hooks, like
 * #snd_config_search_hooks.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key or an alias id does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 * Additionally, any errors encountered when parsing the hook
 * definitions or returned by the hook functions.
 */
int snd_config_searcha_hooks(snd_config_t *root, snd_config_t *config, const char *key, snd_config_t **result)
{
	SND_CONFIG_SEARCHA(root, config, key, result,
					snd_config_searcha_hooks,
					err = snd_config_hooks(config, NULL); \
					if (err < 0) \
						return err; \
			 );
}

/**
 * \brief Searches for a node in a configuration tree, expanding aliases and hooks.
 * \param[in] root Handle to the root configuration node containing
 *                 alias definitions.
 * \param[in,out] config Handle to the root of the configuration
 *                       (sub)tree to search.
 * \param[out] result The function puts the handle to the node found at
 *                    the address specified by \a result.
 * \param[in] ... One or more concatenated dot separated search keys,
 *                terminated with \c NULL.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function searches for a child node of \a config, allowing
 * aliases and expanding hooks like #snd_config_searcha_hooks, but the
 * search key is the concatenation of all passed seach key strings, like
 * with #snd_config_searchv.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key or an alias id does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 * Additionally, any errors encountered when parsing the hook
 * definitions or returned by the hook functions.
 */
int snd_config_searchva_hooks(snd_config_t *root, snd_config_t *config,
			      snd_config_t **result, ...)
{
	SND_CONFIG_SEARCHVA(root, config, result, snd_config_searcha_hooks);
}

/**
 * \brief Searches for a node in a configuration tree, using an alias and expanding hooks.
 * \param[in] config Handle to the root of the configuration (sub)tree
 *                   to search.
 * \param[in] base Search key base, or \c NULL.
 * \param[in] key Search key suffix.
 * \param[out] result The function puts the handle to the node found at
 *                    the address specified by \a result.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions searches for a child node of \a config, allowing
 * aliases, like #snd_config_search_alias, and expanding hooks, like
 * #snd_config_search_hooks.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key or an alias id does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 * Additionally, any errors encountered when parsing the hook
 * definitions or returned by the hook functions.
 */
int snd_config_search_alias_hooks(snd_config_t *config,
				  const char *base, const char *key,
				  snd_config_t **result)
{
	SND_CONFIG_SEARCH_ALIAS(config, base, key, result,
				snd_config_searcha_hooks,
				snd_config_searchva_hooks);
}

/** The name of the environment variable containing the files list for #snd_config_update. */
#define ALSA_CONFIG_PATH_VAR "ALSA_CONFIG_PATH"

/** The name of the default files used by #snd_config_update. */
#define ALSA_CONFIG_PATH_DEFAULT ALSA_CONFIG_DIR "/alsa.conf"

/**
 * \ingroup Config
 * \brief Configuration top-level node (the global configuration).
 *
 * This variable contains a handle to the top-level configuration node,
 * as loaded from global configuration file.
 *
 * This variable is initialized or updated by #snd_config_update.
 * Functions like #snd_pcm_open (that use a device name from the global
 * configuration) automatically call #snd_config_update.  Before the
 * first call to #snd_config_update, this variable is \c NULL.
 *
 * The global configuration files are specified in the environment
 * variable \c ALSA_CONFIG_PATH.  If this is not set, the default value
 * is "/usr/share/alsa/alsa.conf".
 *
 * \warning Whenever the configuration tree is updated, all string
 * pointers and configuration node handles previously obtained from this
 * variable may become invalid.
 *
 * \par Conforming to:
 * LSB 3.2
 */
snd_config_t *snd_config = NULL;

#ifndef DOC_HIDDEN
struct finfo {
	char *name;
	dev_t dev;
	ino_t ino;
	time_t mtime;
};

struct _snd_config_update {
	unsigned int count;
	struct finfo *finfo;
};
#endif /* DOC_HIDDEN */

static snd_config_update_t *snd_config_global_update = NULL;

static int snd_config_hooks_call(snd_config_t *root, snd_config_t *config, snd_config_t *private_data)
{
	void *h = NULL;
	snd_config_t *c, *func_conf = NULL;
	char *buf = NULL;
	const char *lib = NULL, *func_name = NULL;
	const char *str;
	int (*func)(snd_config_t *root, snd_config_t *config, snd_config_t **dst, snd_config_t *private_data) = NULL;
	int err;

	err = snd_config_search(config, "func", &c);
	if (err < 0) {
		SNDERR("Field func is missing");
		return err;
	}
	err = snd_config_get_string(c, &str);
	if (err < 0) {
		SNDERR("Invalid type for field func");
		return err;
	}
	assert(str);
	err = snd_config_search_definition(root, "hook_func", str, &func_conf);
	if (err >= 0) {
		snd_config_iterator_t i, next;
		if (snd_config_get_type(func_conf) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Invalid type for func %s definition", str);
			goto _err;
		}
		snd_config_for_each(i, next, func_conf) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id = n->id;
			if (strcmp(id, "comment") == 0)
				continue;
			if (strcmp(id, "lib") == 0) {
				err = snd_config_get_string(n, &lib);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			if (strcmp(id, "func") == 0) {
				err = snd_config_get_string(n, &func_name);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
		}
	}
	if (!func_name) {
		int len = 16 + strlen(str) + 1;
		buf = malloc(len);
		if (! buf) {
			err = -ENOMEM;
			goto _err;
		}
		snprintf(buf, len, "snd_config_hook_%s", str);
		buf[len-1] = '\0';
		func_name = buf;
	}
	h = snd_dlopen(lib, RTLD_NOW);
	func = h ? snd_dlsym(h, func_name, SND_DLSYM_VERSION(SND_CONFIG_DLSYM_VERSION_HOOK)) : NULL;
	err = 0;
	if (!h) {
		SNDERR("Cannot open shared library %s", lib);
		err = -ENOENT;
	} else if (!func) {
		SNDERR("symbol %s is not defined inside %s", func_name, lib);
		snd_dlclose(h);
		err = -ENXIO;
	}
	_err:
	if (func_conf)
		snd_config_delete(func_conf);
	if (err >= 0) {
		snd_config_t *nroot;
		err = func(root, config, &nroot, private_data);
		if (err < 0)
			SNDERR("function %s returned error: %s", func_name, snd_strerror(err));
		snd_dlclose(h);
		if (err >= 0 && nroot)
			err = snd_config_substitute(root, nroot);
	}
	free(buf);
	if (err < 0)
		return err;
	return 0;
}

static int snd_config_hooks(snd_config_t *config, snd_config_t *private_data)
{
	snd_config_t *n;
	snd_config_iterator_t i, next;
	int err, hit, idx = 0;

	if ((err = snd_config_search(config, "@hooks", &n)) < 0)
		return 0;
	snd_config_remove(n);
	do {
		hit = 0;
		snd_config_for_each(i, next, n) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id = n->id;
			long i;
			err = safe_strtol(id, &i);
			if (err < 0) {
				SNDERR("id of field %s is not and integer", id);
				err = -EINVAL;
				goto _err;
			}
			if (i == idx) {
				err = snd_config_hooks_call(config, n, private_data);
				if (err < 0)
					return err;
				idx++;
				hit = 1;
			}
		}
	} while (hit);
	err = 0;
       _err:
	snd_config_delete(n);
	return err;
}

/**
 * \brief Loads and parses the given configurations files.
 * \param[in] root Handle to the root configuration node.
 * \param[in] config Handle to the configuration node for this hook.
 * \param[out] dst The function puts the handle to the configuration
 *                 node loaded from the file(s) at the address specified
 *                 by \a dst.
 * \param[in] private_data Handle to the private data configuration node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * See \ref confhooks for an example.
 */
int snd_config_hook_load(snd_config_t *root, snd_config_t *config, snd_config_t **dst, snd_config_t *private_data)
{
	snd_config_t *n;
	snd_config_iterator_t i, next;
	struct finfo *fi = NULL;
	int err, idx = 0, fi_count = 0, errors = 1, hit;

	assert(root && dst);
	if ((err = snd_config_search(config, "errors", &n)) >= 0) {
		char *tmp;
		err = snd_config_get_ascii(n, &tmp);
		if (err < 0)
			return err;
		errors = snd_config_get_bool_ascii(tmp);
		free(tmp);
		if (errors < 0) {
			SNDERR("Invalid bool value in field errors");
			return errors;
		}
	}
	if ((err = snd_config_search(config, "files", &n)) < 0) {
		SNDERR("Unable to find field files in the pre-load section");
		return -EINVAL;
	}
	if ((err = snd_config_expand(n, root, NULL, private_data, &n)) < 0) {
		SNDERR("Unable to expand filenames in the pre-load section");
		return err;
	}
	if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("Invalid type for field filenames");
		goto _err;
	}
	snd_config_for_each(i, next, n) {
		snd_config_t *c = snd_config_iterator_entry(i);
		const char *str;
		if ((err = snd_config_get_string(c, &str)) < 0) {
			SNDERR("Field %s is not a string", c->id);
			goto _err;
		}
		fi_count++;
	}
	fi = calloc(fi_count, sizeof(*fi));
	if (fi == NULL) {
		err = -ENOMEM;
		goto _err;
	}
	do {
		hit = 0;
		snd_config_for_each(i, next, n) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id = n->id;
			long i;
			err = safe_strtol(id, &i);
			if (err < 0) {
				SNDERR("id of field %s is not and integer", id);
				err = -EINVAL;
				goto _err;
			}
			if (i == idx) {
				char *name;
				if ((err = snd_config_get_ascii(n, &name)) < 0)
					goto _err;
				if ((err = snd_user_file(name, &fi[idx].name)) < 0)
					fi[idx].name = name;
				else
					free(name);
				idx++;
				hit = 1;
			}
		}
	} while (hit);
	for (idx = 0; idx < fi_count; idx++) {
		snd_input_t *in;
		if (!errors && access(fi[idx].name, R_OK) < 0)
			continue;
		err = snd_input_stdio_open(&in, fi[idx].name, "r");
		if (err >= 0) {
			err = snd_config_load(root, in);
			snd_input_close(in);
			if (err < 0) {
				SNDERR("%s may be old or corrupted: consider to remove or fix it", fi[idx].name);
				goto _err;
			}
		} else {
			SNDERR("cannot access file %s", fi[idx].name);
		}
	}
	*dst = NULL;
	err = 0;
       _err:
	if (fi)
		for (idx = 0; idx < fi_count; idx++)
			free(fi[idx].name);
	free(fi);
	snd_config_delete(n);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_config_hook_load, SND_CONFIG_DLSYM_VERSION_HOOK);
#endif

#ifndef DOC_HIDDEN
int snd_determine_driver(int card, char **driver);
#endif

/**
 * \brief Loads and parses the given configurations files for each
 *        installed sound card.
 * \param[in] root Handle to the root configuration node.
 * \param[in] config Handle to the configuration node for this hook.
 * \param[out] dst The function puts the handle to the configuration
 *                 node loaded from the file(s) at the address specified
 *                 by \a dst.
 * \param[in] private_data Handle to the private data configuration node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This function works like #snd_config_hook_load, but the files are
 * loaded once for each sound card.  The driver name is available with
 * the \c private_string function to customize the file name.
 */
int snd_config_hook_load_for_all_cards(snd_config_t *root, snd_config_t *config, snd_config_t **dst, snd_config_t *private_data ATTRIBUTE_UNUSED)
{
	int card = -1, err;
	
	do {
		err = snd_card_next(&card);
		if (err < 0)
			return err;
		if (card >= 0) {
			snd_config_t *n, *private_data = NULL;
			const char *driver;
			char *fdriver = NULL;
			err = snd_determine_driver(card, &fdriver);
			if (err < 0)
				return err;
			if (snd_config_search(root, fdriver, &n) >= 0) {
				if (snd_config_get_string(n, &driver) < 0)
					goto __err;
				assert(driver);
				while (1) {
					char *s = strchr(driver, '.');
					if (s == NULL)
						break;
					driver = s + 1;
				}
				if (snd_config_search(root, driver, &n) >= 0)
					goto __err;
			} else {
				driver = fdriver;
			}
			err = snd_config_imake_string(&private_data, "string", driver);
			if (err < 0)
				goto __err;
			err = snd_config_hook_load(root, config, &n, private_data);
		      __err:
			if (private_data)
				snd_config_delete(private_data);
			free(fdriver);
			if (err < 0)
				return err;
		}
	} while (card >= 0);
	*dst = NULL;
	return 0;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_config_hook_load_for_all_cards, SND_CONFIG_DLSYM_VERSION_HOOK);
#endif

/** 
 * \brief Updates a configuration tree by rereading the configuration files (if needed).
 * \param[in,out] _top Address of the handle to the top-level node.
 * \param[in,out] _update Address of a pointer to private update information.
 * \param[in] cfgs A list of configuration file names, delimited with ':'.
 *                 If \p cfgs is \c NULL, the default global
 *                 configuration file is used.
 * \return 0 if \a _top was up to date, 1 if the configuration files
 *         have been reread, otherwise a negative error code.
 *
 * The variables pointed to by \a _top and \a _update can be initialized
 * to \c NULL before the first call to this function.  The private
 * update information holds information about all used configuration
 * files that allows this function to detects changes to them; this data
 * can be freed with #snd_config_update_free.
 *
 * The global configuration files are specified in the environment variable
 * \c ALSA_CONFIG_PATH.
 *
 * \warning If the configuration tree is reread, all string pointers and
 * configuration node handles previously obtained from this tree become
 * invalid.
 *
 * \par Errors:
 * Any errors encountered when parsing the input or returned by hooks or
 * functions.
 */
int snd_config_update_r(snd_config_t **_top, snd_config_update_t **_update, const char *cfgs)
{
	int err;
	const char *configs, *c;
	unsigned int k;
	size_t l;
	snd_config_update_t *local;
	snd_config_update_t *update;
	snd_config_t *top;
	
	assert(_top && _update);
	top = *_top;
	update = *_update;
	configs = cfgs;
	if (!configs) {
		configs = getenv(ALSA_CONFIG_PATH_VAR);
		if (!configs || !*configs)
			configs = ALSA_CONFIG_PATH_DEFAULT;
	}
	for (k = 0, c = configs; (l = strcspn(c, ": ")) > 0; ) {
		c += l;
		k++;
		if (!*c)
			break;
		c++;
	}
	if (k == 0) {
		local = NULL;
		goto _reread;
	}
	local = (snd_config_update_t *)calloc(1, sizeof(snd_config_update_t));
	if (!local)
		return -ENOMEM;
	local->count = k;
	local->finfo = calloc(local->count, sizeof(struct finfo));
	if (!local->finfo) {
		free(local);
		return -ENOMEM;
	}
	for (k = 0, c = configs; (l = strcspn(c, ": ")) > 0; ) {
		char name[l + 1];
		memcpy(name, c, l);
		name[l] = 0;
		err = snd_user_file(name, &local->finfo[k].name);
		if (err < 0)
			goto _end;
		c += l;
		k++;
		if (!*c)
			break;
		c++;
	}
	for (k = 0; k < local->count; ++k) {
		struct stat st;
		struct finfo *lf = &local->finfo[k];
		if (stat(lf->name, &st) >= 0) {
			lf->dev = st.st_dev;
			lf->ino = st.st_ino;
			lf->mtime = st.st_mtime;
		} else {
			SNDERR("Cannot access file %s", lf->name);
			free(lf->name);
			memmove(&local->finfo[k], &local->finfo[k+1], sizeof(struct finfo) * (local->count - k - 1));
			k--;
			local->count--;
		}
	}
	if (!update)
		goto _reread;
	if (local->count != update->count)
		goto _reread;
	for (k = 0; k < local->count; ++k) {
		struct finfo *lf = &local->finfo[k];
		struct finfo *uf = &update->finfo[k];
		if (strcmp(lf->name, uf->name) != 0 ||
		    lf->dev != uf->dev ||
		    lf->ino != uf->ino ||
		    lf->mtime != uf->mtime)
			goto _reread;
	}
	err = 0;

 _end:
	if (err < 0) {
		if (top) {
			snd_config_delete(top);
			*_top = NULL;
		}
		if (update) {
			snd_config_update_free(update);
			*_update = NULL;
		}
	}
	if (local)
		snd_config_update_free(local);
	return err;

 _reread:
 	*_top = NULL;
 	*_update = NULL;
 	if (update) {
 		snd_config_update_free(update);
 		update = NULL;
 	}
	if (top) {
		snd_config_delete(top);
		top = NULL;
	}
	err = snd_config_top(&top);
	if (err < 0)
		goto _end;
	if (!local)
		goto _skip;
	for (k = 0; k < local->count; ++k) {
		snd_input_t *in;
		err = snd_input_stdio_open(&in, local->finfo[k].name, "r");
		if (err >= 0) {
			err = snd_config_load(top, in);
			snd_input_close(in);
			if (err < 0) {
				SNDERR("%s may be old or corrupted: consider to remove or fix it", local->finfo[k].name);
				goto _end;
			}
		} else {
			SNDERR("cannot access file %s", local->finfo[k].name);
		}
	}
 _skip:
	err = snd_config_hooks(top, NULL);
	if (err < 0) {
		SNDERR("hooks failed, removing configuration");
		goto _end;
	}
	*_top = top;
	*_update = local;
	return 1;
}

#ifdef HAVE_LIBPTHREAD
static pthread_mutex_t snd_config_update_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/** 
 * \brief Updates #snd_config by rereading the global configuration files (if needed).
 * \return 0 if #snd_config was up to date, 1 if #snd_config was
 *         updated, otherwise a negative error code.
 *
 * \warning Whenever #snd_config is updated, all string pointers and
 * configuration node handles previously obtained from it may become
 * invalid.
 *
 * \par Errors:
 * Any errors encountered when parsing the input or returned by hooks or
 * functions.
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_update(void)
{
	int err;

#ifdef HAVE_LIBPTHREAD
	pthread_mutex_lock(&snd_config_update_mutex);
#endif
	err = snd_config_update_r(&snd_config, &snd_config_global_update, NULL);
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_unlock(&snd_config_update_mutex);
#endif
	return err;
}

/** 
 * \brief Frees a private update structure.
 * \param[in] update The private update structure to free.
 * \return Zero if successful, otherwise a negative error code.
 */
int snd_config_update_free(snd_config_update_t *update)
{
	unsigned int k;

	assert(update);
	for (k = 0; k < update->count; k++)
		free(update->finfo[k].name);
	free(update->finfo);
	free(update);
	return 0;
}

/** 
 * \brief Frees the global configuration tree in #snd_config.
 * \return Zero if successful, otherwise a negative error code.
 *
 * This functions releases all resources of the global configuration
 * tree, and sets #snd_config to \c NULL.
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_update_free_global(void)
{
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_lock(&snd_config_update_mutex);
#endif
	if (snd_config)
		snd_config_delete(snd_config);
	snd_config = NULL;
	if (snd_config_global_update)
		snd_config_update_free(snd_config_global_update);
	snd_config_global_update = NULL;
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_unlock(&snd_config_update_mutex);
#endif
	snd_dlobj_cache_cleanup();

	return 0;
}

/**
 * \brief Returns an iterator pointing to a node's first child.
 * \param[in] config Handle to a configuration node.
 * \return An iterator pointing to \a config's first child.
 *
 * \a config must be a compound node.
 *
 * The returned iterator is valid if it is not equal to the return value
 * of #snd_config_iterator_end on \a config.
 *
 * Use #snd_config_iterator_entry to get the handle of the node pointed
 * to.
 *
 * \par Conforming to:
 * LSB 3.2
 */
snd_config_iterator_t snd_config_iterator_first(const snd_config_t *config)
{
	assert(config->type == SND_CONFIG_TYPE_COMPOUND);
	return config->u.compound.fields.next;
}

/**
 * \brief Returns an iterator pointing to the next sibling.
 * \param[in] iterator An iterator pointing to a child configuration node.
 * \return An iterator pointing to the next sibling of \a iterator.
 *
 * The returned iterator is valid if it is not equal to the return value
 * of #snd_config_iterator_end on the node's parent.
 *
 * Use #snd_config_iterator_entry to get the handle of the node pointed
 * to.
 *
 * \par Conforming to:
 * LSB 3.2
 */
snd_config_iterator_t snd_config_iterator_next(const snd_config_iterator_t iterator)
{
	return iterator->next;
}

/**
 * \brief Returns an iterator that ends a node's children list.
 * \param[in] config Handle to a configuration node.
 * \return An iterator that indicates the end of \a config's children list.
 *
 * \a config must be a compound node.
 *
 * The return value can be understood as pointing past the last child of
 * \a config.
 *
 * \par Conforming to:
 * LSB 3.2
 */
snd_config_iterator_t snd_config_iterator_end(const snd_config_t *config)
{
	assert(config->type == SND_CONFIG_TYPE_COMPOUND);
	return (const snd_config_iterator_t)&config->u.compound.fields;
}

/**
 * \brief Returns the configuration node handle pointed to by an iterator.
 * \param[in] iterator A configuration node iterator.
 * \return The configuration node handle pointed to by \a iterator.
 *
 * \par Conforming to:
 * LSB 3.2
 */
snd_config_t *snd_config_iterator_entry(const snd_config_iterator_t iterator)
{
	return list_entry(iterator, snd_config_t, list);
}

#ifndef DOC_HIDDEN
typedef enum _snd_config_walk_pass {
	SND_CONFIG_WALK_PASS_PRE,
	SND_CONFIG_WALK_PASS_POST,
	SND_CONFIG_WALK_PASS_LEAF,
} snd_config_walk_pass_t;
#endif

/* Return 1 if node needs to be attached to parent */
/* Return 2 if compound is replaced with standard node */
#ifndef DOC_HIDDEN
typedef int (*snd_config_walk_callback_t)(snd_config_t *src,
					  snd_config_t *root,
					  snd_config_t **dst,
					  snd_config_walk_pass_t pass,
					  snd_config_t *private_data);
#endif

static int snd_config_walk(snd_config_t *src,
			   snd_config_t *root,
			   snd_config_t **dst, 
			   snd_config_walk_callback_t callback,
			   snd_config_t *private_data)
{
	int err;
	snd_config_iterator_t i, next;

	switch (snd_config_get_type(src)) {
	case SND_CONFIG_TYPE_COMPOUND:
		err = callback(src, root, dst, SND_CONFIG_WALK_PASS_PRE, private_data);
		if (err <= 0)
			return err;
		snd_config_for_each(i, next, src) {
			snd_config_t *s = snd_config_iterator_entry(i);
			snd_config_t *d = NULL;

			err = snd_config_walk(s, root, (dst && *dst) ? &d : NULL,
					      callback, private_data);
			if (err < 0)
				goto _error;
			if (err && d) {
				err = snd_config_add(*dst, d);
				if (err < 0)
					goto _error;
			}
		}
		err = callback(src, root, dst, SND_CONFIG_WALK_PASS_POST, private_data);
		if (err <= 0) {
		_error:
			if (dst && *dst)
				snd_config_delete(*dst);
		}
		break;
	default:
		err = callback(src, root, dst, SND_CONFIG_WALK_PASS_LEAF, private_data);
		break;
	}
	return err;
}

static int _snd_config_copy(snd_config_t *src,
			    snd_config_t *root ATTRIBUTE_UNUSED,
			    snd_config_t **dst,
			    snd_config_walk_pass_t pass,
			    snd_config_t *private_data ATTRIBUTE_UNUSED)
{
	int err;
	const char *id = src->id;
	snd_config_type_t type = snd_config_get_type(src);
	switch (pass) {
	case SND_CONFIG_WALK_PASS_PRE:
		err = snd_config_make_compound(dst, id, src->u.compound.join);
		if (err < 0)
			return err;
		break;
	case SND_CONFIG_WALK_PASS_LEAF:
		err = snd_config_make(dst, id, type);
		if (err < 0)
			return err;
		switch (type) {
		case SND_CONFIG_TYPE_INTEGER:
		{
			long v;
			err = snd_config_get_integer(src, &v);
			assert(err >= 0);
			snd_config_set_integer(*dst, v);
			break;
		}
		case SND_CONFIG_TYPE_INTEGER64:
		{
			long long v;
			err = snd_config_get_integer64(src, &v);
			assert(err >= 0);
			snd_config_set_integer64(*dst, v);
			break;
		}
		case SND_CONFIG_TYPE_REAL:
		{
			double v;
			err = snd_config_get_real(src, &v);
			assert(err >= 0);
			snd_config_set_real(*dst, v);
			break;
		}
		case SND_CONFIG_TYPE_STRING:
		{
			const char *s;
			err = snd_config_get_string(src, &s);
			assert(err >= 0);
			err = snd_config_set_string(*dst, s);
			if (err < 0)
				return err;
			break;
		}
		default:
			assert(0);
		}
		break;
	default:
		break;
	}
	return 1;
}

/**
 * \brief Creates a copy of a configuration node.
 * \param[out] dst The function puts the handle to the new configuration
 *                 node at the address specified by \a dst.
 * \param[in] src Handle to the source configuration node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * This function creates a deep copy, i.e., if \a src is a compound
 * node, all children are copied recursively.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOMEM<dd>Out of memory.
 * </dl>
 *
 * \par Conforming to:
 * LSB 3.2
 */
int snd_config_copy(snd_config_t **dst,
		    snd_config_t *src)
{
	return snd_config_walk(src, NULL, dst, _snd_config_copy, NULL);
}

static int _snd_config_expand(snd_config_t *src,
			      snd_config_t *root ATTRIBUTE_UNUSED,
			      snd_config_t **dst,
			      snd_config_walk_pass_t pass,
			      snd_config_t *private_data)
{
	int err;
	const char *id = src->id;
	snd_config_type_t type = snd_config_get_type(src);
	switch (pass) {
	case SND_CONFIG_WALK_PASS_PRE:
	{
		if (id && strcmp(id, "@args") == 0)
			return 0;
		err = snd_config_make_compound(dst, id, src->u.compound.join);
		if (err < 0)
			return err;
		break;
	}
	case SND_CONFIG_WALK_PASS_LEAF:
		switch (type) {
		case SND_CONFIG_TYPE_INTEGER:
		{
			long v;
			err = snd_config_get_integer(src, &v);
			assert(err >= 0);
			err = snd_config_imake_integer(dst, id, v);
			if (err < 0)
				return err;
			break;
		}
		case SND_CONFIG_TYPE_INTEGER64:
		{
			long long v;
			err = snd_config_get_integer64(src, &v);
			assert(err >= 0);
			err = snd_config_imake_integer64(dst, id, v);
			if (err < 0)
				return err;
			break;
		}
		case SND_CONFIG_TYPE_REAL:
		{
			double v;
			err = snd_config_get_real(src, &v);
			assert(err >= 0);
			err = snd_config_imake_real(dst, id, v);
			if (err < 0)
				return err;
			break;
		}
		case SND_CONFIG_TYPE_STRING:
		{
			const char *s;
			snd_config_t *val;
			snd_config_t *vars = private_data;
			snd_config_get_string(src, &s);
			if (s && *s == '$') {
				s++;
				if (snd_config_search(vars, s, &val) < 0)
					return 0;
				err = snd_config_copy(dst, val);
				if (err < 0)
					return err;
				err = snd_config_set_id(*dst, id);
				if (err < 0) {
					snd_config_delete(*dst);
					return err;
				}
			} else {
				err = snd_config_imake_string(dst, id, s);
				if (err < 0)
					return err;
			}
			break;
		}
		default:
			assert(0);
		}
		break;
	default:
		break;
	}
	return 1;
}

static int _snd_config_evaluate(snd_config_t *src,
				snd_config_t *root,
				snd_config_t **dst ATTRIBUTE_UNUSED,
				snd_config_walk_pass_t pass,
				snd_config_t *private_data)
{
	int err;
	if (pass == SND_CONFIG_WALK_PASS_PRE) {
		char *buf = NULL;
		const char *lib = NULL, *func_name = NULL;
		const char *str;
		int (*func)(snd_config_t **dst, snd_config_t *root,
			    snd_config_t *src, snd_config_t *private_data) = NULL;
		void *h = NULL;
		snd_config_t *c, *func_conf = NULL;
		err = snd_config_search(src, "@func", &c);
		if (err < 0)
			return 1;
		err = snd_config_get_string(c, &str);
		if (err < 0) {
			SNDERR("Invalid type for @func");
			return err;
		}
		assert(str);
		err = snd_config_search_definition(root, "func", str, &func_conf);
		if (err >= 0) {
			snd_config_iterator_t i, next;
			if (snd_config_get_type(func_conf) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for func %s definition", str);
				goto _err;
			}
			snd_config_for_each(i, next, func_conf) {
				snd_config_t *n = snd_config_iterator_entry(i);
				const char *id = n->id;
				if (strcmp(id, "comment") == 0)
					continue;
				if (strcmp(id, "lib") == 0) {
					err = snd_config_get_string(n, &lib);
					if (err < 0) {
						SNDERR("Invalid type for %s", id);
						goto _err;
					}
					continue;
				}
				if (strcmp(id, "func") == 0) {
					err = snd_config_get_string(n, &func_name);
					if (err < 0) {
						SNDERR("Invalid type for %s", id);
						goto _err;
					}
					continue;
				}
				SNDERR("Unknown field %s", id);
			}
		}
		if (!func_name) {
			int len = 9 + strlen(str) + 1;
			buf = malloc(len);
			if (! buf) {
				err = -ENOMEM;
				goto _err;
			}
			snprintf(buf, len, "snd_func_%s", str);
			buf[len-1] = '\0';
			func_name = buf;
		}
		h = snd_dlopen(lib, RTLD_NOW);
		if (h)
			func = snd_dlsym(h, func_name, SND_DLSYM_VERSION(SND_CONFIG_DLSYM_VERSION_EVALUATE));
		err = 0;
		if (!h) {
			SNDERR("Cannot open shared library %s", lib);
			err = -ENOENT;
			goto _errbuf;
		} else if (!func) {
			SNDERR("symbol %s is not defined inside %s", func_name, lib);
			snd_dlclose(h);
			err = -ENXIO;
			goto _errbuf;
		}
	       _err:
		if (func_conf)
			snd_config_delete(func_conf);
		if (err >= 0) {
			snd_config_t *eval;
			err = func(&eval, root, src, private_data);
			if (err < 0)
				SNDERR("function %s returned error: %s", func_name, snd_strerror(err));
			snd_dlclose(h);
			if (err >= 0 && eval) {
				/* substitute merges compound members */
				/* we don't want merging at all */
				err = snd_config_delete_compound_members(src);
				if (err >= 0)
					err = snd_config_substitute(src, eval);
			}
		}
	       _errbuf:
		free(buf);
		if (err < 0)
			return err;
		return 0;
	}
	return 1;
}

/**
 * \brief Evaluates a configuration node at runtime.
 * \param[in,out] config Handle to the source configuration node.
 * \param[in] root Handle to the root of the source configuration.
 * \param[in] private_data Handle to the private data node for runtime evaluation.
 * \param result Must be \c NULL.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * This function evaluates any functions (\c \@func) in \a config and
 * replaces those nodes with the respective function results.
 */
int snd_config_evaluate(snd_config_t *config, snd_config_t *root,
		        snd_config_t *private_data, snd_config_t **result)
{
	assert(result == NULL);
	return snd_config_walk(config, root, result, _snd_config_evaluate, private_data);
}

static int load_defaults(snd_config_t *subs, snd_config_t *defs)
{
	snd_config_iterator_t d, dnext;
	snd_config_for_each(d, dnext, defs) {
		snd_config_t *def = snd_config_iterator_entry(d);
		snd_config_iterator_t f, fnext;
		if (snd_config_get_type(def) != SND_CONFIG_TYPE_COMPOUND)
			continue;
		snd_config_for_each(f, fnext, def) {
			snd_config_t *fld = snd_config_iterator_entry(f);
			const char *id = fld->id;
			if (strcmp(id, "type") == 0)
				continue;
			if (strcmp(id, "default") == 0) {
				snd_config_t *deflt;
				int err;
				err = snd_config_copy(&deflt, fld);
				if (err < 0)
					return err;
				err = snd_config_set_id(deflt, def->id);
				if (err < 0) {
					snd_config_delete(deflt);
					return err;
				}
				err = snd_config_add(subs, deflt);
				if (err < 0) {
					snd_config_delete(deflt);
					return err;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
			return -EINVAL;
		}
	}
	return 0;
}

static void skip_blank(const char **ptr)
{
	while (1) {
		switch (**ptr) {
		case ' ':
		case '\f':
		case '\t':
		case '\n':
		case '\r':
			break;
		default:
			return;
		}
		(*ptr)++;
	}
}

static int parse_char(const char **ptr)
{
	int c;
	assert(**ptr == '\\');
	(*ptr)++;
	c = **ptr;
	switch (c) {
	case 'n':
		c = '\n';
		break;
	case 't':
		c = '\t';
		break;
	case 'v':
		c = '\v';
		break;
	case 'b':
		c = '\b';
		break;
	case 'r':
		c = '\r';
		break;
	case 'f':
		c = '\f';
		break;
	case '0' ... '7':
	{
		int num = c - '0';
		int i = 1;
		(*ptr)++;
		do {
			c = **ptr;
			if (c < '0' || c > '7')
				break;
			num = num * 8 + c - '0';
			i++;
			(*ptr)++;
		} while (i < 3);
		return num;
	}
	default:
		break;
	}
	(*ptr)++;
	return c;
}

static int parse_id(const char **ptr)
{
	if (!**ptr)
		return -EINVAL;
	while (1) {
		switch (**ptr) {
		case '\f':
		case '\t':
		case '\n':
		case '\r':
		case ',':
		case '=':
		case '\0':
			return 0;
		default:
			break;
		}
		(*ptr)++;
	}
}

static int parse_string(const char **ptr, char **val)
{
	const size_t bufsize = 256;
	char _buf[bufsize];
	char *buf = _buf;
	size_t alloc = bufsize;
	char delim = **ptr;
	size_t idx = 0;
	(*ptr)++;
	while (1) {
		int c = **ptr;
		switch (c) {
		case '\0':
			SNDERR("Unterminated string");
			return -EINVAL;
		case '\\':
			c = parse_char(ptr);
			if (c < 0)
				return c;
			break;
		default:
			(*ptr)++;
			if (c == delim) {
				*val = malloc(idx + 1);
				if (!*val)
					return -ENOMEM;
				memcpy(*val, buf, idx);
				(*val)[idx] = 0;
				if (alloc > bufsize)
					free(buf);
				return 0;
			}
		}
		if (idx >= alloc) {
			size_t old_alloc = alloc;
			alloc *= 2;
			if (old_alloc == bufsize) {
				buf = malloc(alloc);
				memcpy(buf, _buf, old_alloc);
			} else {
				buf = realloc(buf, alloc);
			}
			if (!buf)
				return -ENOMEM;
		}
		buf[idx++] = c;
	}
}
				

/* Parse var=val or val */
static int parse_arg(const char **ptr, unsigned int *varlen, char **val)
{
	const char *str;
	int err, vallen;
	skip_blank(ptr);
	str = *ptr;
	if (*str == '"' || *str == '\'') {
		err = parse_string(ptr, val);
		if (err < 0)
			return err;
		*varlen = 0;
		return 0;
	}
	err = parse_id(ptr);
	if (err < 0)
		return err;
	vallen = *ptr - str;
	skip_blank(ptr);
	if (**ptr != '=') {
		*varlen = 0;
		goto _value;
	}
	*varlen = vallen;
	(*ptr)++;
	skip_blank(ptr);
	str = *ptr;
	if (*str == '"' || *str == '\'') {
		err = parse_string(ptr, val);
		if (err < 0)
			return err;
		return 0;
	}
	err = parse_id(ptr);
	if (err < 0)
		return err;
	vallen = *ptr - str;
 _value:
	*val = malloc(vallen + 1);
	if (!*val)
		return -ENOMEM;
	memcpy(*val, str, vallen);
	(*val)[vallen] = 0;
	return 0;
}


/* val1, val2, ...
 * var1=val1,var2=val2,...
 * { conf syntax }
 */
static int parse_args(snd_config_t *subs, const char *str, snd_config_t *defs)
{
	int err;
	int arg = 0;
	if (str == NULL)
		return 0;
	skip_blank(&str);
	if (!*str)
		return 0;
	if (*str == '{') {
		int len = strlen(str);
		snd_input_t *input;
		snd_config_iterator_t i, next;
		while (1) {
			switch (str[--len]) {
			case ' ':
			case '\f':
			case '\t':
			case '\n':
			case '\r':
				continue;
			default:
				break;
			}
			break;
		}
		if (str[len] != '}')
			return -EINVAL;
		err = snd_input_buffer_open(&input, str + 1, len - 1);
		if (err < 0)
			return err;
		err = snd_config_load_override(subs, input);
		snd_input_close(input);
		if (err < 0)
			return err;
		snd_config_for_each(i, next, subs) {
			snd_config_t *n = snd_config_iterator_entry(i);
			snd_config_t *d;
			const char *id = n->id;
			err = snd_config_search(defs, id, &d);
			if (err < 0) {
				SNDERR("Unknown parameter %s", id);
				return err;
			}
		}
		return 0;
	}
	
	while (1) {
		char buf[256];
		const char *var = buf;
		unsigned int varlen;
		snd_config_t *def, *sub, *typ;
		const char *new = str;
		const char *tmp;
		char *val = NULL;
		err = parse_arg(&new, &varlen, &val);
		if (err < 0)
			goto _err;
		if (varlen > 0) {
			assert(varlen < sizeof(buf));
			memcpy(buf, str, varlen);
			buf[varlen] = 0;
		} else {
			sprintf(buf, "%d", arg);
		}
		err = snd_config_search_alias(defs, NULL, var, &def);
		if (err < 0) {
			SNDERR("Unknown parameter %s", var);
			goto _err;
		}
		if (snd_config_get_type(def) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Parameter %s definition is not correct", var);
			err = -EINVAL;
			goto _err;
		}
		var = def->id;
		err = snd_config_search(subs, var, &sub);
		if (err >= 0)
			snd_config_delete(sub);
		err = snd_config_search(def, "type", &typ);
		if (err < 0) {
		_invalid_type:
			SNDERR("Parameter %s definition is missing a valid type info", var);
			goto _err;
		}
		err = snd_config_get_string(typ, &tmp);
		if (err < 0 || !tmp)
			goto _invalid_type;
		if (strcmp(tmp, "integer") == 0) {
			long v;
			err = snd_config_make(&sub, var, SND_CONFIG_TYPE_INTEGER);
			if (err < 0)
				goto _err;
			err = safe_strtol(val, &v);
			if (err < 0) {
				SNDERR("Parameter %s must be an integer", var);
				goto _err;
			}
			err = snd_config_set_integer(sub, v);
			if (err < 0)
				goto _err;
		} else if (strcmp(tmp, "integer64") == 0) {
			long long v;
			err = snd_config_make(&sub, var, SND_CONFIG_TYPE_INTEGER64);
			if (err < 0)
				goto _err;
			err = safe_strtoll(val, &v);
			if (err < 0) {
				SNDERR("Parameter %s must be an integer", var);
				goto _err;
			}
			err = snd_config_set_integer64(sub, v);
			if (err < 0)
				goto _err;
		} else if (strcmp(tmp, "real") == 0) {
			double v;
			err = snd_config_make(&sub, var, SND_CONFIG_TYPE_REAL);
			if (err < 0)
				goto _err;
			err = safe_strtod(val, &v);
			if (err < 0) {
				SNDERR("Parameter %s must be a real", var);
				goto _err;
			}
			err = snd_config_set_real(sub, v);
			if (err < 0)
				goto _err;
		} else if (strcmp(tmp, "string") == 0) {
			err = snd_config_make(&sub, var, SND_CONFIG_TYPE_STRING);
			if (err < 0)
				goto _err;
			err = snd_config_set_string(sub, val);
			if (err < 0)
				goto _err;
		} else {
			err = -EINVAL;
			goto _invalid_type;
		}
		err = snd_config_set_id(sub, var);
		if (err < 0)
			goto _err;
		err = snd_config_add(subs, sub);
		if (err < 0) {
		_err:
			free(val);
			return err;
		}
		free(val);
		if (!*new)
			break;
		if (*new != ',')
			return -EINVAL;
		str = new + 1;
		arg++;
	}
	return 0;
}

/**
 * \brief Expands a configuration node, applying arguments and functions.
 * \param[in] config Handle to the configuration node.
 * \param[in] root Handle to the root configuration node.
 * \param[in] args Arguments string, can be \c NULL.
 * \param[in] private_data Handle to the private data node for functions.
 * \param[out] result The function puts the handle to the result
 *                    configuration node at the address specified by
 *                    \a result.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * If \a config has arguments (defined by a child with id \c \@args),
 * this function replaces any string node beginning with $ with the
 * respective argument value, or the default argument value, or nothing.
 * Furthermore, any functions are evaluated (see #snd_config_evaluate).
 * The resulting copy of \a config is returned in \a result.
 */
int snd_config_expand(snd_config_t *config, snd_config_t *root, const char *args,
		      snd_config_t *private_data, snd_config_t **result)
{
	int err;
	snd_config_t *defs, *subs = NULL, *res;
	err = snd_config_search(config, "@args", &defs);
	if (err < 0) {
		if (args != NULL) {
			SNDERR("Unknown parameters %s", args);
			return -EINVAL;
		}
		err = snd_config_copy(&res, config);
		if (err < 0)
			return err;
	} else {
		err = snd_config_top(&subs);
		if (err < 0)
			return err;
		err = load_defaults(subs, defs);
		if (err < 0) {
			SNDERR("Load defaults error: %s", snd_strerror(err));
			goto _end;
		}
		err = parse_args(subs, args, defs);
		if (err < 0) {
			SNDERR("Parse arguments error: %s", snd_strerror(err));
			goto _end;
		}
		err = snd_config_evaluate(subs, root, private_data, NULL);
		if (err < 0) {
			SNDERR("Args evaluate error: %s", snd_strerror(err));
			goto _end;
		}
		err = snd_config_walk(config, root, &res, _snd_config_expand, subs);
		if (err < 0) {
			SNDERR("Expand error (walk): %s", snd_strerror(err));
			goto _end;
		}
	}
	err = snd_config_evaluate(res, root, private_data, NULL);
	if (err < 0) {
		SNDERR("Evaluate error: %s", snd_strerror(err));
		snd_config_delete(res);
		goto _end;
	}
	*result = res;
	err = 1;
 _end:
 	if (subs)
		snd_config_delete(subs);
	return err;
}
	
/**
 * \brief Searches for a definition in a configuration tree, using
 *        aliases and expanding hooks and arguments.
 * \param[in] config Handle to the configuration (sub)tree to search.
 * \param[in] base Implicit key base, or \c NULL for none.
 * \param[in] name Key suffix, optionally with arguments.
 * \param[out] result The function puts the handle to the expanded found
 *                    node at the address specified by \a result.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * This functions searches for a child node of \a config, allowing
 * aliases and expanding hooks, like #snd_config_search_alias_hooks.
 *
 * If \a name contains a colon (:), the rest of the string after the
 * colon contains arguments that are expanded as with
 * #snd_config_expand.
 *
 * In any case, \a result is a new node that must be freed by the
 * caller.
 *
 * \par Errors:
 * <dl>
 * <dt>-ENOENT<dd>An id in \a key or an alias id does not exist.
 * <dt>-ENOENT<dd>\a config or one of its child nodes to be searched is
 *                not a compound node.
 * </dl>
 * Additionally, any errors encountered when parsing the hook
 * definitions or arguments, or returned by (hook) functions.
 */
int snd_config_search_definition(snd_config_t *config,
				 const char *base, const char *name,
				 snd_config_t **result)
{
	snd_config_t *conf;
	char *key;
	const char *args = strchr(name, ':');
	int err;
	if (args) {
		args++;
		key = alloca(args - name);
		memcpy(key, name, args - name - 1);
		key[args - name - 1] = '\0';
	} else {
		key = (char *) name;
	}
	/*
	 *  if key contains dot (.), the implicit base is ignored
	 *  and the key starts from root given by the 'config' parameter
	 */
	err = snd_config_search_alias_hooks(config, strchr(key, '.') ? NULL : base, key, &conf);
	if (err < 0)
		return err;
	return snd_config_expand(conf, config, args, NULL, result);
}

#ifndef DOC_HIDDEN
void snd_config_set_hop(snd_config_t *conf, int hop)
{
	conf->hop = hop;
}

int snd_config_check_hop(snd_config_t *conf)
{
	if (conf) {
		if (conf->hop >= SND_CONF_MAX_HOPS) {
			SYSERR("Too many definition levels (looped?)\n");
			return -EINVAL;
		}
		return conf->hop;
	}
	return 0;
}
#endif
