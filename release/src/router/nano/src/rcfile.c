/**************************************************************************
 *   rcfile.c  --  This file is part of GNU nano.                         *
 *                                                                        *
 *   Copyright (C) 2001-2011, 2013-2018 Free Software Foundation, Inc.    *
 *   Copyright (C) 2014 Mike Frysinger                                    *
 *   Copyright (C) 2014-2018 Benno Schulenberg                            *
 *                                                                        *
 *   GNU nano is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published    *
 *   by the Free Software Foundation, either version 3 of the License,    *
 *   or (at your option) any later version.                               *
 *                                                                        *
 *   GNU nano is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#include "proto.h"

#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <string.h>
#include <unistd.h>

#ifdef ENABLE_NANORC

#ifndef RCFILE_NAME
#define HOME_RC_NAME ".nanorc"
#define RCFILE_NAME "nanorc"
#else
#define HOME_RC_NAME RCFILE_NAME
#endif

static const rcoption rcopts[] = {
	{"boldtext", BOLD_TEXT},
#ifdef ENABLE_JUSTIFY
	{"brackets", 0},
#endif
	{"constantshow", CONSTANT_SHOW},
#ifdef ENABLED_WRAPORJUSTIFY
	{"fill", 0},
#endif
#ifdef ENABLE_HISTORIES
	{"historylog", HISTORYLOG},
#endif
#ifdef ENABLE_LINENUMBERS
	{"linenumbers", LINE_NUMBERS},
#endif
	{"morespace", MORE_SPACE},
#ifdef ENABLE_MOUSE
	{"mouse", USE_MOUSE},
#endif
#ifdef ENABLE_MULTIBUFFER
	{"multibuffer", MULTIBUFFER},
#endif
	{"nohelp", NO_HELP},
	{"nonewlines", NO_NEWLINES},
	{"nopauses", NO_PAUSES},
#ifdef ENABLE_WRAPPING
	{"nowrap", NO_WRAP},
#endif
#ifdef ENABLE_OPERATINGDIR
	{"operatingdir", 0},
#endif
#ifdef ENABLE_HISTORIES
	{"positionlog", POS_HISTORY},
#endif
	{"preserve", PRESERVE},
#ifdef ENABLE_JUSTIFY
	{"punct", 0},
	{"quotestr", 0},
#endif
	{"quickblank", QUICK_BLANK},
	{"rebinddelete", REBIND_DELETE},
	{"rebindkeypad", REBIND_KEYPAD},
	{"regexp", USE_REGEXP},
#ifdef ENABLE_SPELLER
	{"speller", 0},
#endif
	{"suspend", SUSPEND},
	{"tabsize", 0},
	{"tempfile", TEMP_FILE},
	{"view", VIEW_MODE},
#ifndef NANO_TINY
	{"afterends", AFTER_ENDS},
	{"allow_insecure_backup", INSECURE_BACKUP},
	{"atblanks", AT_BLANKS},
	{"autoindent", AUTOINDENT},
	{"backup", BACKUP_FILE},
	{"backupdir", 0},
	{"casesensitive", CASE_SENSITIVE},
	{"cut", CUT_FROM_CURSOR},  /* deprecated form, remove end of 2018 */
	{"cutfromcursor", CUT_FROM_CURSOR},
	{"justifytrim", TRIM_BLANKS},  /* deprecated form, remove end of 2018 */
	{"locking", LOCKING},
	{"matchbrackets", 0},
	{"noconvert", NO_CONVERT},
	{"quiet", QUIET},
	{"showcursor", SHOW_CURSOR},
	{"smarthome", SMART_HOME},
	{"smooth", SMOOTH_SCROLL},
	{"softwrap", SOFTWRAP},
	{"tabstospaces", TABS_TO_SPACES},
	{"trimblanks", TRIM_BLANKS},
	{"unix", MAKE_IT_UNIX},
	{"whitespace", 0},
	{"wordbounds", WORD_BOUNDS},
	{"wordchars", 0},
#endif
#ifdef ENABLE_COLOR
	{"titlecolor", 0},
	{"numbercolor", 0},
	{"selectedcolor", 0},
	{"statuscolor", 0},
	{"errorcolor", 0},
	{"keycolor", 0},
	{"functioncolor", 0},
#endif
	{NULL, 0}
};

static size_t lineno = 0;
		/* The line number of the last encountered error. */
static char *nanorc = NULL;
		/* The path to the rcfile we're parsing. */
#ifdef ENABLE_COLOR
static bool opensyntax = FALSE;
		/* Whether we're allowed to add to the last syntax.  When a file ends,
		 * or when a new syntax command is seen, this bool becomes FALSE. */
static syntaxtype *live_syntax;
		/* The syntax that is currently being parsed. */
static colortype *lastcolor = NULL;
		/* The end of the color list for the current syntax. */
#endif

/* Report an error in an rcfile, printing it to stderr. */
void rcfile_error(const char *msg, ...)
{
	va_list ap;

	if (rcfile_with_errors == NULL)
		rcfile_with_errors = strdup(nanorc);

	if (lineno > 0)
		fprintf(stderr, _("Error in %s on line %zu: "), nanorc, lineno);

	va_start(ap, msg);
	vfprintf(stderr, _(msg), ap);
	va_end(ap);

	fprintf(stderr, "\n");
}
#endif /* ENABLE_NANORC */

#if defined(ENABLE_NANORC) || defined(ENABLE_HISTORIES)
/* Parse the next word from the string, null-terminate it, and return
 * a pointer to the first character after the null terminator.  The
 * returned pointer will point to '\0' if we hit the end of the line. */
char *parse_next_word(char *ptr)
{
	while (!isblank((unsigned char)*ptr) && *ptr != '\0')
		ptr++;

	if (*ptr == '\0')
		return ptr;

	/* Null-terminate and advance ptr. */
	*ptr++ = '\0';

	while (isblank((unsigned char)*ptr))
		ptr++;

	return ptr;
}
#endif /* ENABLE_NANORC || ENABLE_HISTORIES */

#ifdef ENABLE_NANORC
/* Parse an argument, with optional quotes, after a keyword that takes
 * one.  If the next word starts with a ", we say that it ends with the
 * last " of the line.  Otherwise, we interpret it as usual, so that the
 * arguments can contain "'s too. */
char *parse_argument(char *ptr)
{
	const char *ptr_save = ptr;
	char *last_quote = NULL;

	if (*ptr != '"')
		return parse_next_word(ptr);

	while (*ptr != '\0') {
		if (*++ptr == '"')
			last_quote = ptr;
	}

	if (last_quote == NULL) {
		rcfile_error(N_("Argument '%s' has an unterminated \""), ptr_save);
		return NULL;
	}

	*last_quote = '\0';
	ptr = last_quote + 1;

	while (isblank((unsigned char)*ptr))
		ptr++;

	return ptr;
}

#ifdef ENABLE_COLOR
/* Pass over the current regex string in the line starting at ptr,
 * null-terminate it, and return a pointer to the /next/ word. */
char *parse_next_regex(char *ptr)
{
	/* Continue until the end of line, or until a " followed by a
	 * blank character or the end of line. */
	while (*ptr != '\0' && (*ptr != '"' ||
				(*(ptr + 1) != '\0' && !isblank((unsigned char)ptr[1]))))
		ptr++;

	if (*ptr == '\0') {
		rcfile_error(N_("Regex strings must begin and end with a \" character"));
		return NULL;
	}

	/* Null-terminate and advance ptr. */
	*ptr++ = '\0';

	while (isblank((unsigned char)*ptr))
		ptr++;

	return ptr;
}

/* Compile the regular expression regex to see if it's valid.  Return
 * TRUE if it is, and FALSE otherwise. */
bool nregcomp(const char *regex, int compile_flags)
{
	regex_t preg;
	const char *r = fixbounds(regex);
	int rc = regcomp(&preg, r, compile_flags);

	if (rc != 0) {
		size_t len = regerror(rc, &preg, NULL, 0);
		char *str = charalloc(len);

		regerror(rc, &preg, str, len);
		rcfile_error(N_("Bad regex \"%s\": %s"), r, str);
		free(str);
	}

	regfree(&preg);
	return (rc == 0);
}

/* Parse the next syntax name and its possible extension regexes from the
 * line at ptr, and add it to the global linked list of color syntaxes. */
void parse_syntax(char *ptr)
{
	char *nameptr = ptr;

	opensyntax = FALSE;

	/* Check that the syntax name is not empty. */
	if (*ptr == '\0' || (*ptr == '"' &&
						(*(ptr + 1) == '\0' || *(ptr + 1) == '"'))) {
		rcfile_error(N_("Missing syntax name"));
		return;
	}

	ptr = parse_next_word(ptr);

	/* Check that quotes around the name are either paired or absent. */
	if ((*nameptr == '\x22') ^ (nameptr[strlen(nameptr) - 1] == '\x22')) {
		rcfile_error(N_("Unpaired quote in syntax name"));
		return;
	}

	/* If the name is quoted, strip the quotes. */
	if (*nameptr == '\x22') {
		nameptr++;
		nameptr[strlen(nameptr) - 1] = '\0';
	}

	/* Redefining the "none" syntax is not allowed. */
	if (strcmp(nameptr, "none") == 0) {
		rcfile_error(N_("The \"none\" syntax is reserved"));
		return;
	}

	/* Initialize a new syntax struct. */
	live_syntax = (syntaxtype *)nmalloc(sizeof(syntaxtype));
	live_syntax->name = mallocstrcpy(NULL, nameptr);
	live_syntax->extensions = NULL;
	live_syntax->headers = NULL;
	live_syntax->magics = NULL;
	live_syntax->linter = NULL;
#ifdef ENABLE_COMMENT
	live_syntax->comment = mallocstrcpy(NULL, GENERAL_COMMENT_CHARACTER);
#endif
	live_syntax->color = NULL;
	lastcolor = NULL;
	live_syntax->nmultis = 0;

	/* Hook the new syntax in at the top of the list. */
	live_syntax->next = syntaxes;
	syntaxes = live_syntax;

	opensyntax = TRUE;

	/* The default syntax should have no associated extensions. */
	if (strcmp(live_syntax->name, "default") == 0 && *ptr != '\0') {
		rcfile_error(N_("The \"default\" syntax does not accept extensions"));
		return;
	}

	/* If there seem to be extension regexes, pick them up. */
	if (*ptr != '\0')
		grab_and_store("extension", ptr, &live_syntax->extensions);
}
#endif /* ENABLE_COLOR */

/* Return TRUE when the given function is present in almost all menus. */
bool is_universal(void (*func)(void))
{
	return (func == do_left || func == do_right ||
		func == do_home || func == do_end ||
#ifndef NANO_TINY
		func == do_prev_word_void || func == do_next_word_void ||
#endif
		func == do_delete || func == do_backspace ||
		func == do_cut_text_void || func == do_uncut_text ||
		func == do_tab || func == do_enter || func == do_verbatim_input);
}

/* Bind or unbind a key combo, to or from a function. */
void parse_binding(char *ptr, bool dobind)
{
	char *keyptr = NULL, *keycopy = NULL, *funcptr = NULL, *menuptr = NULL;
	sc *s, *newsc = NULL;
	int menu;

	if (*ptr == '\0') {
		rcfile_error(N_("Missing key name"));
		return;
	}

	keyptr = ptr;
	ptr = parse_next_word(ptr);
	keycopy = mallocstrcpy(NULL, keyptr);

	if (strlen(keycopy) < 2) {
		rcfile_error(N_("Key name is too short"));
		goto free_things;
	}

	/* Uppercase only the first two or three characters of the key name. */
	keycopy[0] = toupper((unsigned char)keycopy[0]);
	keycopy[1] = toupper((unsigned char)keycopy[1]);
	if (keycopy[0] == 'M' && keycopy[1] == '-') {
		if (strlen(keycopy) > 2)
			keycopy[2] = toupper((unsigned char)keycopy[2]);
		else {
			rcfile_error(N_("Key name is too short"));
			goto free_things;
		}
	}

	/* Allow the codes for Insert and Delete to be rebound, but apart
	 * from those two only Control, Meta and Function sequences. */
	if (!strcasecmp(keycopy, "Ins") || !strcasecmp(keycopy, "Del"))
		keycopy[1] = tolower((unsigned char)keycopy[1]);
	else if (keycopy[0] != '^' && keycopy[0] != 'M' && keycopy[0] != 'F') {
		rcfile_error(N_("Key name must begin with \"^\", \"M\", or \"F\""));
		goto free_things;
	} else if (keycode_from_string(keycopy) < 0) {
		rcfile_error(N_("Key name %s is invalid"), keycopy);
		goto free_things;
	}

	if (dobind) {
		funcptr = ptr;
		ptr = parse_argument(ptr);

		if (funcptr[0] == '\0') {
			rcfile_error(N_("Must specify a function to bind the key to"));
			goto free_things;
		} else if (ptr == NULL)
			goto free_things;
	}

	menuptr = ptr;
	ptr = parse_next_word(ptr);

	if (menuptr[0] == '\0') {
		/* TRANSLATORS: Do not translate the word "all". */
		rcfile_error(N_("Must specify a menu (or \"all\") in which to bind/unbind the key"));
		goto free_things;
	}

	if (dobind) {
		/* If the thing to bind starts with a double quote, it is a string,
		 * otherwise it is the name of a function. */
		if (*funcptr == '"') {
			newsc = nmalloc(sizeof(sc));
			newsc->func = (functionptrtype)implant;
			newsc->expansion = mallocstrcpy(NULL, funcptr + 1);
#ifndef NANO_TINY
			newsc->toggle = 0;
#endif
		} else
			newsc = strtosc(funcptr);

		if (newsc == NULL) {
			rcfile_error(N_("Cannot map name \"%s\" to a function"), funcptr);
			goto free_things;
		}
	}

	menu = name_to_menu(menuptr);
	if (menu < 1) {
		rcfile_error(N_("Cannot map name \"%s\" to a menu"), menuptr);
		goto free_things;
	}

	if (dobind) {
		subnfunc *f;
		int mask = 0;

		/* Tally up the menus where the function exists. */
		for (f = allfuncs; f != NULL; f = f->next)
			if (f->func == newsc->func)
				mask = mask | f->menus;

#ifndef NANO_TINY
		/* Handle the special case of the toggles. */
		if (newsc->func == do_toggle_void)
			mask = MMAIN;
#endif
#ifdef ENABLE_NANORC
		/* Handle the special case of a key defined as a string. */
		if (newsc->func == (functionptrtype)implant)
			mask = MMOST | MHELP;
#endif
		/* Now limit the given menu to those where the function exists. */
		menu = menu & (is_universal(newsc->func) ? MMOST : mask);

		if (!menu) {
			rcfile_error(N_("Function '%s' does not exist in menu '%s'"), funcptr, menuptr);
			goto free_things;
		}

		newsc->menus = menu;
		assign_keyinfo(newsc, keycopy, 0);

		/* Do not allow rebinding a frequent escape-sequence starter: Esc [. */
		if (newsc->meta && newsc->keycode == 91) {
			rcfile_error(N_("Sorry, keystroke \"%s\" may not be rebound"), newsc->keystr);
			goto free_things;
		}
	}

	/* Now find and delete any existing same shortcut in the menu(s). */
	for (s = sclist; s != NULL; s = s->next) {
		if ((s->menus & menu) && !strcmp(s->keystr, keycopy))
			s->menus &= ~menu;
	}

	if (dobind) {
#ifndef NANO_TINY
		/* If this is a toggle, copy its sequence number. */
		if (newsc->func == do_toggle_void) {
			for (s = sclist; s != NULL; s = s->next)
				if (s->func == do_toggle_void && s->toggle == newsc->toggle)
					newsc->ordinal = s->ordinal;
		} else
			newsc->ordinal = 0;
#endif
		/* Add the new shortcut at the start of the list. */
		newsc->next = sclist;
		sclist = newsc;
		return;
	}

  free_things:
	free(newsc);
	free(keycopy);
}

/* Verify that the given file exists, is not a folder nor a device. */
bool is_good_file(char *file)
{
	struct stat rcinfo;

	/* First check that the file exists and is readable. */
	if (access(file, R_OK) != 0)
		return FALSE;

	/* If the thing exists, it may be neither a directory nor a device. */
	if (stat(file, &rcinfo) != -1 && (S_ISDIR(rcinfo.st_mode) ||
				S_ISCHR(rcinfo.st_mode) || S_ISBLK(rcinfo.st_mode))) {
		rcfile_error(S_ISDIR(rcinfo.st_mode) ? _("\"%s\" is a directory") :
										_("\"%s\" is a device file"), file);
		return FALSE;
	} else
		return TRUE;
}

#ifdef ENABLE_COLOR
/* Read and parse one included syntax file. */
static void parse_one_include(char *file)
{
	FILE *rcstream;

	/* Don't open directories, character files, or block files. */
	if (!is_good_file(file))
		return;

	/* Open the included syntax file. */
	rcstream = fopen(file, "rb");

	if (rcstream == NULL) {
		rcfile_error(_("Error reading %s: %s"), file, strerror(errno));
		return;
	}

	/* Use the name and line number position of the included syntax file
	 * while parsing it, so we can know where any errors in it are. */
	nanorc = file;
	lineno = 0;

	parse_rcfile(rcstream, TRUE);
}

/* Expand globs in the passed name, and parse the resultant files. */
void parse_includes(char *ptr)
{
	char *option, *nanorc_save = nanorc, *expanded;
	size_t lineno_save = lineno, i;
	glob_t files;

	option = ptr;
	if (*option == '"')
		option++;
	ptr = parse_argument(ptr);

	/* Expand tildes first, then the globs. */
	expanded = real_dir_from_tilde(option);

	if (glob(expanded, GLOB_ERR|GLOB_NOSORT, NULL, &files) == 0) {
		for (i = 0; i < files.gl_pathc; ++i)
			parse_one_include(files.gl_pathv[i]);
	} else
		rcfile_error(_("Error expanding %s: %s"), option, strerror(errno));

	globfree(&files);
	free(expanded);

	/* We're done with the included file(s).  Restore the original
	 * filename and line number position. */
	nanorc = nanorc_save;
	lineno = lineno_save;
}

/* Return the short value corresponding to the color named in colorname,
 * and set bright to TRUE if that color is bright. */
short color_to_short(const char *colorname, bool *bright)
{
	if (strncasecmp(colorname, "bright", 6) == 0) {
		*bright = TRUE;
		colorname += 6;
	} else
		*bright = FALSE;

	if (strcasecmp(colorname, "green") == 0)
		return COLOR_GREEN;
	else if (strcasecmp(colorname, "red") == 0)
		return COLOR_RED;
	else if (strcasecmp(colorname, "blue") == 0)
		return COLOR_BLUE;
	else if (strcasecmp(colorname, "white") == 0)
		return COLOR_WHITE;
	else if (strcasecmp(colorname, "yellow") == 0)
		return COLOR_YELLOW;
	else if (strcasecmp(colorname, "cyan") == 0)
		return COLOR_CYAN;
	else if (strcasecmp(colorname, "magenta") == 0)
		return COLOR_MAGENTA;
	else if (strcasecmp(colorname, "black") == 0)
		return COLOR_BLACK;
	else if (strcasecmp(colorname, "normal") == 0)
		return USE_THE_DEFAULT;

	rcfile_error(N_("Color \"%s\" not understood"), colorname);
	return BAD_COLOR;
}

/* Parse the color name (or pair of color names) in the given string.
 * Return FALSE when any color name is invalid; otherwise return TRUE. */
bool parse_color_names(char *combostr, short *fg, short *bg, int *attributes)
{
	char *comma = strchr(combostr, ',');
	bool bright;

	*attributes = A_NORMAL;

	if (comma != NULL) {
		*bg = color_to_short(comma + 1, &bright);
		if (bright) {
			rcfile_error(N_("A background color cannot be bright"));
			return FALSE;
		}
		if (*bg == BAD_COLOR)
			return FALSE;
		*comma = '\0';
	} else
		*bg = USE_THE_DEFAULT;

	if (comma != combostr) {
		*fg = color_to_short(combostr, &bright);
		if (*fg == BAD_COLOR)
			return FALSE;

		if (bright)
			*attributes = A_BOLD;
	} else
		*fg = USE_THE_DEFAULT;

	return TRUE;
}

/* Parse the color string in the line at ptr, and add it to the current
 * file's associated colors.  rex_flags are the regex compilation flags
 * to use, excluding or including REG_ICASE for case (in)sensitivity. */
void parse_colors(char *ptr, int rex_flags)
{
	short fg, bg;
	int attributes;
	char *item;

	if (!opensyntax) {
		rcfile_error(N_("A '%s' command requires a preceding 'syntax' command"),
						"color");
		return;
	}

	if (*ptr == '\0') {
		rcfile_error(N_("Missing color name"));
		return;
	}

	item = ptr;
	ptr = parse_next_word(ptr);
	if (!parse_color_names(item, &fg, &bg, &attributes))
		return;

	if (*ptr == '\0') {
		rcfile_error(N_("Missing regex string after '%s' command"), "color");
		return;
	}

	/* Now for the fun part.  Start adding regexes to individual strings
	 * in the colorstrings array, woo! */
	while (ptr != NULL && *ptr != '\0') {
		colortype *newcolor = NULL;
			/* The container for a color plus its regexes. */
		bool goodstart;
			/* Whether the start expression was valid. */
		bool expectend = FALSE;
			/* Whether to expect an end= line. */

		if (strncasecmp(ptr, "start=", 6) == 0) {
			ptr += 6;
			expectend = TRUE;
		}

		if (*ptr != '"') {
			rcfile_error(N_("Regex strings must begin and end with a \" character"));
			ptr = parse_next_regex(ptr);
			continue;
		}

		item = ++ptr;
		ptr = parse_next_regex(ptr);
		if (ptr == NULL)
			break;

		if (*item == '\0') {
			rcfile_error(N_("Empty regex string"));
			goodstart = FALSE;
		} else
			goodstart = nregcomp(item, rex_flags);

		/* If the starting regex is valid, initialize a new color struct,
		 * and hook it in at the tail of the linked list. */
		if (goodstart) {
			newcolor = (colortype *)nmalloc(sizeof(colortype));

			newcolor->fg = fg;
			newcolor->bg = bg;
			newcolor->attributes = attributes;
			newcolor->rex_flags = rex_flags;

			newcolor->start_regex = mallocstrcpy(NULL, item);
			newcolor->start = NULL;

			newcolor->end_regex = NULL;
			newcolor->end = NULL;

			newcolor->next = NULL;

			if (lastcolor == NULL)
				live_syntax->color = newcolor;
			else
				lastcolor->next = newcolor;

			lastcolor = newcolor;
		}

		if (!expectend)
			continue;

		if (ptr == NULL || strncasecmp(ptr, "end=", 4) != 0) {
			rcfile_error(N_("\"start=\" requires a corresponding \"end=\""));
			return;
		}

		ptr += 4;
		if (*ptr != '"') {
			rcfile_error(N_("Regex strings must begin and end with a \" character"));
			continue;
		}

		item = ++ptr;
		ptr = parse_next_regex(ptr);
		if (ptr == NULL)
			break;

		if (*item == '\0') {
			rcfile_error(N_("Empty regex string"));
			continue;
		}

		/* If the start regex was invalid, skip past the end regex
		 * to stay in sync. */
		if (!goodstart)
			continue;

		/* If it's valid, save the ending regex string. */
		if (nregcomp(item, rex_flags))
			newcolor->end_regex = mallocstrcpy(NULL, item);

		/* Lame way to skip another static counter. */
		newcolor->id = live_syntax->nmultis;
		live_syntax->nmultis++;
	}
}

/* Parse the argument of an interface color option. */
colortype *parse_interface_color(char *combostr)
{
	colortype *trio = nmalloc(sizeof(colortype));

	if (parse_color_names(combostr, &trio->fg, &trio->bg, &trio->attributes)) {
		free(combostr);
		return trio;
	} else {
		free(combostr);
		free(trio);
		return NULL;
	}
}

/* Read regex strings enclosed in double quotes from the line pointed at
 * by ptr, and store them quoteless in the passed storage place. */
void grab_and_store(const char *kind, char *ptr, regexlisttype **storage)
{
	regexlisttype *lastthing;

	if (!opensyntax) {
		rcfile_error(
				N_("A '%s' command requires a preceding 'syntax' command"), kind);
		return;
	}

	/* The default syntax doesn't take any file matching stuff. */
	if (strcmp(live_syntax->name, "default") == 0 && *ptr != '\0') {
		rcfile_error(
				N_("The \"default\" syntax does not accept '%s' regexes"), kind);
		return;
	}

	if (*ptr == '\0') {
		rcfile_error(N_("Missing regex string after '%s' command"), kind);
		return;
	}

	lastthing = *storage;

	/* If there was an earlier command, go to the last of those regexes. */
	while (lastthing != NULL && lastthing->next != NULL)
		lastthing = lastthing->next;

	/* Now gather any valid regexes and add them to the linked list. */
	while (*ptr != '\0') {
		const char *regexstring;
		regexlisttype *newthing;

		if (*ptr != '"') {
			rcfile_error(N_("Regex strings must begin and end with a \" character"));
			return;
		}

		regexstring = ++ptr;
		ptr = parse_next_regex(ptr);
		if (ptr == NULL)
			return;

		/* If the regex string is malformed, skip it. */
		if (!nregcomp(regexstring, NANO_REG_EXTENDED | REG_NOSUB))
			continue;

		/* Copy the regex into a struct, and hook this in at the end. */
		newthing = (regexlisttype *)nmalloc(sizeof(regexlisttype));
		newthing->full_regex = mallocstrcpy(NULL, regexstring);
		newthing->next = NULL;

		if (lastthing == NULL)
			*storage = newthing;
		else
			lastthing->next = newthing;

		lastthing = newthing;
	}
}

/* Gather and store the string after a comment/linter command. */
void pick_up_name(const char *kind, char *ptr, char **storage)
{
	if (!opensyntax) {
		rcfile_error(
				N_("A '%s' command requires a preceding 'syntax' command"), kind);
		return;
	}

	if (*ptr == '\0') {
		rcfile_error(N_("Missing argument after '%s'"), kind);
		return;
	}

	/* If the argument starts with a quote, find the terminating quote. */
	if (*ptr == '"') {
		char *look = ptr + strlen(ptr);

		while (*look != '"') {
			if (--look == ptr) {
				rcfile_error(N_("Argument of '%s' lacks closing \""), kind);
				return;
			}
		}

		*look = '\0';
		ptr++;
	}

	*storage = mallocstrcpy(*storage, ptr);
}
#endif /* ENABLE_COLOR */

/* Verify that the user has not unmapped every shortcut for a
 * function that we consider 'vital' (such as "Exit"). */
static void check_vitals_mapped(void)
{
	subnfunc *f;
	int v;
#define VITALS 3
	void (*vitals[VITALS])(void) = { do_exit, do_exit, do_cancel };
	int inmenus[VITALS] = { MMAIN, MHELP, MYESNO };

	for  (v = 0; v < VITALS; v++) {
		for (f = allfuncs; f != NULL; f = f->next) {
			if (f->func == vitals[v] && f->menus & inmenus[v]) {
				const sc *s = first_sc_for(inmenus[v], f->func);
				if (!s) {
					fprintf(stderr, _("No key is bound to function '%s' in "
										"menu '%s'.  Exiting.\n"), f->desc,
										menu_to_name(inmenus[v]));
					fprintf(stderr, _("If needed, use nano with the -I option "
										"to adjust your nanorc settings.\n"));
					exit(1);
				}
				break;
			}
		}
	}
}

/* Parse the rcfile, once it has been opened successfully at rcstream,
 * and close it afterwards.  If syntax_only is TRUE, allow the file to
 * to contain only color syntax commands. */
void parse_rcfile(FILE *rcstream, bool syntax_only)
{
	char *buf = NULL;
	ssize_t len;
	size_t n = 0;

	while ((len = getline(&buf, &n, rcstream)) > 0) {
		char *ptr, *keyword, *option;
		int set = 0;
		size_t i;

		/* Ignore the newline. */
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		lineno++;
		ptr = buf;
		while (isblank((unsigned char)*ptr))
			ptr++;

		/* If we have a blank line or a comment, skip to the next
		 * line. */
		if (*ptr == '\0' || *ptr == '#')
			continue;

		/* Otherwise, skip to the next space. */
		keyword = ptr;
		ptr = parse_next_word(ptr);

#ifdef ENABLE_COLOR
		/* Handle extending first... */
		if (strcasecmp(keyword, "extendsyntax") == 0 && !syntax_only) {
			syntaxtype *sint;
			char *syntaxname = ptr;

			ptr = parse_next_word(ptr);

			for (sint = syntaxes; sint != NULL; sint = sint->next)
				if (!strcmp(sint->name, syntaxname))
					break;

			if (sint == NULL) {
				rcfile_error(N_("Could not find syntax \"%s\" to extend"),
								syntaxname);
				opensyntax = FALSE;
				continue;
			}

			live_syntax = sint;
			opensyntax = TRUE;

			/* Refind the tail of the color list for this syntax. */
			lastcolor = sint->color;
			if (lastcolor != NULL)
				while (lastcolor->next != NULL)
					lastcolor = lastcolor->next;

			keyword = ptr;
			ptr = parse_next_word(ptr);
		}

		/* Try to parse the keyword. */
		if (strcasecmp(keyword, "syntax") == 0) {
			if (opensyntax && lastcolor == NULL)
				rcfile_error(N_("Syntax \"%s\" has no color commands"),
								live_syntax->name);
			parse_syntax(ptr);
		}
		else if (strcasecmp(keyword, "header") == 0)
			grab_and_store("header", ptr, &live_syntax->headers);
		else if (strcasecmp(keyword, "magic") == 0)
#ifdef HAVE_LIBMAGIC
			grab_and_store("magic", ptr, &live_syntax->magics);
#else
			;
#endif
		else if (strcasecmp(keyword, "comment") == 0)
#ifdef ENABLE_COMMENT
			pick_up_name("comment", ptr, &live_syntax->comment);
#else
			;
#endif
		else if (strcasecmp(keyword, "color") == 0)
			parse_colors(ptr, NANO_REG_EXTENDED);
		else if (strcasecmp(keyword, "icolor") == 0)
			parse_colors(ptr, NANO_REG_EXTENDED | REG_ICASE);
		else if (strcasecmp(keyword, "linter") == 0)
			pick_up_name("linter", ptr, &live_syntax->linter);
		else if (syntax_only && (strcasecmp(keyword, "set") == 0 ||
								strcasecmp(keyword, "unset") == 0 ||
								strcasecmp(keyword, "bind") == 0 ||
								strcasecmp(keyword, "unbind") == 0 ||
								strcasecmp(keyword, "include") == 0 ||
								strcasecmp(keyword, "extendsyntax") == 0))
			rcfile_error(N_("Command \"%s\" not allowed in included file"),
										keyword);
		else if (strcasecmp(keyword, "include") == 0)
			parse_includes(ptr);
		else
#endif /* ENABLE_COLOR */
		if (strcasecmp(keyword, "set") == 0)
			set = 1;
		else if (strcasecmp(keyword, "unset") == 0)
			set = -1;
		else if (strcasecmp(keyword, "bind") == 0)
			parse_binding(ptr, TRUE);
		else if (strcasecmp(keyword, "unbind") == 0)
			parse_binding(ptr, FALSE);
		else
			rcfile_error(N_("Command \"%s\" not understood"), keyword);

#ifdef ENABLE_COLOR
		/* If a syntax was extended, it stops at the end of the command. */
		if (live_syntax != syntaxes)
			opensyntax = FALSE;
#endif

		if (set == 0)
			continue;

		if (*ptr == '\0') {
			rcfile_error(N_("Missing option"));
			continue;
		}

		option = ptr;
		ptr = parse_next_word(ptr);

		/* Find the just read name among the existing options. */
		for (i = 0; rcopts[i].name != NULL; i++) {
			if (strcasecmp(option, rcopts[i].name) == 0)
				break;
		}

		if (rcopts[i].name == NULL) {
			rcfile_error(N_("Unknown option \"%s\""), option);
			continue;
		}

		/* First handle unsetting. */
		if (set == -1) {
			if (rcopts[i].flag != 0)
				UNSET(rcopts[i].flag);
			else
				rcfile_error(N_("Cannot unset option \"%s\""), rcopts[i].name);
			continue;
		}

		/* If the option has a flag, it doesn't take an argument. */
		if (rcopts[i].flag != 0) {
			SET(rcopts[i].flag);
			continue;
		}

		/* The option doesn't have a flag, so it takes an argument. */
		if (*ptr == '\0') {
			rcfile_error(N_("Option \"%s\" requires an argument"),
								rcopts[i].name);
			continue;
		}

		option = ptr;
		if (*option == '"')
			option++;
		ptr = parse_argument(ptr);

		option = mallocstrcpy(NULL, option);

#ifdef ENABLE_UTF8
		/* When in a UTF-8 locale, ignore arguments with invalid sequences. */
		if (using_utf8() && mbstowcs(NULL, option, 0) == (size_t)-1) {
			rcfile_error(N_("Argument is not a valid multibyte string"));
			continue;
		}
#endif
#ifdef ENABLE_COLOR
		if (strcasecmp(rcopts[i].name, "titlecolor") == 0)
			color_combo[TITLE_BAR] = parse_interface_color(option);
		else if (strcasecmp(rcopts[i].name, "numbercolor") == 0)
			color_combo[LINE_NUMBER] = parse_interface_color(option);
		else if (strcasecmp(rcopts[i].name, "selectedcolor") == 0)
			color_combo[SELECTED_TEXT] = parse_interface_color(option);
		else if (strcasecmp(rcopts[i].name, "statuscolor") == 0)
			color_combo[STATUS_BAR] = parse_interface_color(option);
		else if (strcasecmp(rcopts[i].name, "errorcolor") == 0)
			color_combo[ERROR_MESSAGE] = parse_interface_color(option);
		else if (strcasecmp(rcopts[i].name, "keycolor") == 0)
			color_combo[KEY_COMBO] = parse_interface_color(option);
		else if (strcasecmp(rcopts[i].name, "functioncolor") == 0)
			color_combo[FUNCTION_TAG] = parse_interface_color(option);
		else
#endif
#ifdef ENABLE_OPERATINGDIR
		if (strcasecmp(rcopts[i].name, "operatingdir") == 0)
			operating_dir = option;
		else
#endif
#ifdef ENABLED_WRAPORJUSTIFY
		if (strcasecmp(rcopts[i].name, "fill") == 0) {
			if (!parse_num(option, &fill)) {
				rcfile_error(N_("Requested fill size \"%s\" is invalid"),
								option);
				fill = -COLUMNS_FROM_EOL;
			} else
				UNSET(NO_WRAP);
			free(option);
		} else
#endif
#ifndef NANO_TINY
		if (strcasecmp(rcopts[i].name, "matchbrackets") == 0) {
			matchbrackets = option;
			if (has_blank_mbchars(matchbrackets)) {
				rcfile_error(N_("Non-blank characters required"));
				free(matchbrackets);
				matchbrackets = NULL;
			}
		} else if (strcasecmp(rcopts[i].name, "whitespace") == 0) {
			whitespace = option;
			if (mbstrlen(whitespace) != 2 || strlenpt(whitespace) != 2) {
				rcfile_error(N_("Two single-column characters required"));
				free(whitespace);
				whitespace = NULL;
			} else {
				whitespace_len[0] = parse_mbchar(whitespace, NULL, NULL);
				whitespace_len[1] = parse_mbchar(whitespace +
										whitespace_len[0], NULL, NULL);
			}
		} else
#endif
#ifdef ENABLE_JUSTIFY
		if (strcasecmp(rcopts[i].name, "punct") == 0) {
			if (has_blank_mbchars(option)) {
				rcfile_error(N_("Non-blank characters required"));
				free(option);
			} else
				punct = option;
		} else if (strcasecmp(rcopts[i].name, "brackets") == 0) {
			if (has_blank_mbchars(option)) {
				rcfile_error(N_("Non-blank characters required"));
				free(option);
			} else
				brackets = option;
		} else if (strcasecmp(rcopts[i].name, "quotestr") == 0)
			quotestr = option;
		else
#endif
#ifndef NANO_TINY
		if (strcasecmp(rcopts[i].name, "backupdir") == 0)
			backup_dir = option;
		else
		if (strcasecmp(rcopts[i].name, "wordchars") == 0)
			word_chars = option;
		else
#endif
#ifdef ENABLE_SPELLER
		if (strcasecmp(rcopts[i].name, "speller") == 0)
			alt_speller = option;
		else
#endif
		if (strcasecmp(rcopts[i].name, "tabsize") == 0) {
			if (!parse_num(option, &tabsize) || tabsize <= 0) {
				rcfile_error(N_("Requested tab size \"%s\" is invalid"),
								option);
				tabsize = -1;
			}
			free(option);
		}
	}

#ifdef ENABLE_COLOR
	if (opensyntax && lastcolor == NULL)
		rcfile_error(N_("Syntax \"%s\" has no color commands"),
						live_syntax->name);

	opensyntax = FALSE;
#endif

	free(buf);
	fclose(rcstream);
	lineno = 0;

	return;
}

/* Read and interpret one of the two nanorc files. */
void parse_one_nanorc(void)
{
	FILE *rcstream;

	/* Don't try to open directories nor devices. */
	if (!is_good_file(nanorc))
		return;

	rcstream = fopen(nanorc, "rb");

	/* If opening the file succeeded, parse it.  Otherwise, only
	 * complain if the file actually exists. */
	if (rcstream != NULL)
		parse_rcfile(rcstream, FALSE);
	else if (errno != ENOENT)
		rcfile_error(N_("Error reading %s: %s"), nanorc, strerror(errno));
}

bool have_nanorc(const char *path, char *name)
{
	if (path == NULL)
		return FALSE;

	free(nanorc);
	nanorc = concatenate(path, name);

	return is_good_file(nanorc);
}

/* First read the system-wide rcfile, then the user's rcfile. */
void do_rcfiles(void)
{
	const char *xdgconfdir;

	/* First process the system-wide nanorc, if there is one. */
	nanorc = mallocstrcpy(nanorc, SYSCONFDIR "/nanorc");
	parse_one_nanorc();

	/* When configured with --disable-wrapping-as-root, turn wrapping off
	 * for root, so that only root's .nanorc or --fill can turn it on. */
#ifdef DISABLE_ROOTWRAPPING
	if (geteuid() == NANO_ROOT_UID)
		SET(NO_WRAP);
#endif

	get_homedir();
	xdgconfdir = getenv("XDG_CONFIG_HOME");

	/* Now try the to find a nanorc file in the user's home directory or in
	 * the XDG configuration directories, and process the first one found. */
	if (have_nanorc(homedir, "/" HOME_RC_NAME))
		parse_one_nanorc();
	else if (have_nanorc(xdgconfdir, "/nano/" RCFILE_NAME))
		parse_one_nanorc();
	else if (have_nanorc(homedir, "/.config/nano/" RCFILE_NAME))
		parse_one_nanorc();
	else if (homedir == NULL && xdgconfdir == NULL)
		rcfile_error(N_("I can't find my home directory!  Wah!"));

	check_vitals_mapped();

#ifdef __linux__
	/* On a Linux console, don't start nano when there are rcfile errors,
	 * because otherwise these error messages get wiped. */
	if (on_a_vt && rcfile_with_errors) {
		fprintf(stderr, _("If needed, use nano with the -I option "
							"to adjust your nanorc settings.\n"));
		exit(1);
	}
#endif

	free(nanorc);
}

#endif /* ENABLE_NANORC */
