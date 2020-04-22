/**************************************************************************
 *   global.c  --  This file is part of GNU nano.                         *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2020 Free Software Foundation, Inc.    *
 *   Copyright (C) 2014-2020 Benno Schulenberg                            *
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
#include <string.h>
#include <strings.h>

/* Global variables. */
#ifndef NANO_TINY
volatile sig_atomic_t the_window_resized = FALSE;
		/* Set to TRUE by the handler whenever a SIGWINCH occurs. */
#endif

bool on_a_vt = FALSE;
		/* Whether we're running on a Linux console (a VT). */
bool shifted_metas = FALSE;
		/* Whether any Sh-M-<letter> combo has been bound. */

bool meta_key;
		/* Whether the current keystroke is a Meta key. */
bool shift_held;
		/* Whether Shift was being held together with a movement key. */
bool mute_modifiers = FALSE;
		/* Whether to ignore modifier keys while running a macro or string bind. */
bool bracketed_paste = FALSE;
		/* Whether text is being pasted into nano from outside. */

bool started_curses = FALSE;
		/* Becomes TRUE when curses mode has been entered the first time. */
bool we_are_running = FALSE;
		/* Becomes TRUE as soon as all options and files have been read. */
bool more_than_one = FALSE;
		/* Whether more than one buffer is or has been open. */

bool inhelp = FALSE;
		/* Whether we are in the help viewer. */
char *title = NULL;
		/* When not NULL: the title of the current help text. */

bool refresh_needed = FALSE;
		/* Did a command mangle enough of the buffer that we should
		 * repaint the screen? */
bool focusing = TRUE;
		/* Whether an update of the edit window should center the cursor. */

bool as_an_at = TRUE;
		/* Whether a 0x0A byte should be shown as a ^@ instead of a ^J. */

bool control_C_was_pressed = FALSE;
		/* Whether Ctrl+C was pressed (when a keyboard interrupt is enabled). */

bool suppress_cursorpos = FALSE;
		/* Should we skip constant position display for current keystroke? */

message_type lastmessage = HUSH;
		/* Messages of type HUSH should not overwrite type MILD nor ALERT. */

linestruct *pletion_line = NULL;
		/* The line where the last completion was found, if any. */

bool also_the_last = FALSE;
		/* Whether indenting/commenting should include the last line of
		 * the marked region. */

char *answer = NULL;
		/* The answer string used by the status-bar prompt. */

char *last_search = NULL;
		/* The last string we searched for. */
int didfind = 0;
		/* Whether the last search found something. */

char *present_path = NULL;
		/* The current browser directory when trying to do tab completion. */

unsigned flags[4] = {0, 0, 0, 0};
		/* Our flags array, containing the states of all global options. */

int controlleft, controlright, controlup, controldown;
int controlhome, controlend;
#ifndef NANO_TINY
int controldelete, controlshiftdelete;
int shiftleft, shiftright, shiftup, shiftdown;
int shiftcontrolleft, shiftcontrolright, shiftcontrolup, shiftcontroldown;
int shiftcontrolhome, shiftcontrolend;
int altleft, altright, altup, altdown;
int altdelete;
int shiftaltleft, shiftaltright, shiftaltup, shiftaltdown;
#endif

#ifdef ENABLED_WRAPORJUSTIFY
ssize_t fill = -COLUMNS_FROM_EOL;
		/* The relative column where we will wrap lines. */
size_t wrap_at = 0;
		/* The actual column where we will wrap lines, based on fill. */
#endif

WINDOW *topwin = NULL;
		/* The top portion of the screen, showing the version number of nano,
		 * the name of the file, and whether the buffer was modified. */
WINDOW *edit = NULL;
		/* The middle portion of the screen: the edit window, showing the
		 * contents of the current buffer, the file we are editing. */
WINDOW *bottomwin = NULL;
		/* The bottom portion of the screen, where we display statusbar
		 * messages, the status-bar prompt, and a list of shortcuts. */
int editwinrows = 0;
		/* How many rows does the edit window take up? */
int editwincols = -1;
		/* The number of usable columns in the edit window: COLS - margin. */
int margin = 0;
		/* The amount of space reserved at the left for line numbers. */
#ifndef NANO_TINY
ssize_t stripe_column = 0;
		/* The column at which a vertical bar will be drawn. */
#endif

linestruct *cutbuffer = NULL;
		/* The buffer where we store cut text. */
linestruct *cutbottom = NULL;
		/* The last line in the cutbuffer. */
bool keep_cutbuffer = FALSE;
		/* Whether to add to the cutbuffer instead of clearing it first. */

openfilestruct *openfile = NULL;
		/* The list of all open file buffers. */
#ifdef ENABLE_MULTIBUFFER
openfilestruct *startfile = NULL;
		/* The first open buffer. */
#endif

#ifndef NANO_TINY
char *matchbrackets = NULL;
		/* The opening and closing brackets that bracket searches can find. */
char *whitespace = NULL;
		/* The characters used when visibly showing tabs and spaces. */
int whitelen[2];
		/* The length in bytes of these characters. */
#endif

#ifdef ENABLE_JUSTIFY
char *punct = NULL;
		/* The closing punctuation that can end sentences. */
char *brackets = NULL;
		/* The closing brackets that can follow closing punctuation and
		 * can end sentences. */
char *quotestr = NULL;
		/* The quoting string.  The default value is set in main(). */
regex_t quotereg;
		/* The compiled regular expression from the quoting string. */
#endif

char *word_chars = NULL;
		/* Nonalphanumeric characters that also form words. */

ssize_t tabsize = -1;
		/* The width of a tab in spaces.  The default is set in main(). */

#ifndef NANO_TINY
char *backup_dir = NULL;
		/* The directory where we store backup files. */
#endif
#ifdef ENABLE_OPERATINGDIR
char *operating_dir = NULL;
		/* The path to our confining "operating" directory, when given. */
#endif

#ifdef ENABLE_SPELLER
char *alt_speller = NULL;
		/* The command to use for the alternate spell checker. */
#endif

#ifdef ENABLE_COLOR
syntaxtype *syntaxes = NULL;
		/* The global list of color syntaxes. */
char *syntaxstr = NULL;
		/* The color syntax name specified on the command line. */
bool have_palette = FALSE;
		/* Whether the colors for the current syntax have been initialized. */
#endif

int currmenu = MMOST;
		/* The currently active menu, initialized to a dummy value. */
keystruct *sclist = NULL;
		/* The start of the shortcuts list. */
funcstruct *allfuncs = NULL;
		/* The start of the functions list. */
funcstruct *tailfunc;
		/* The last function in the list. */
funcstruct *exitfunc;
		/* A pointer to the special Exit/Close item. */

linestruct *search_history = NULL;
		/* The current item in the list of strings that were searched for. */
linestruct *execute_history = NULL;
		/* The current item in the list of commands that were run with ^R ^X. */
linestruct *replace_history = NULL;
		/* The current item in the list of replace strings. */
#ifdef ENABLE_HISTORIES
linestruct *searchtop = NULL;
		/* The oldest item in the list of search strings. */
linestruct *searchbot = NULL;
		/* The newest item in the list of search strings. */

linestruct *replacetop = NULL;
linestruct *replacebot = NULL;

linestruct *executetop = NULL;
linestruct *executebot = NULL;
#endif

regex_t search_regexp;
		/* The compiled regular expression to use in searches. */
regmatch_t regmatches[10];
		/* The match positions for parenthetical subexpressions, 10
		 * maximum, used in regular expression searches. */

int hilite_attribute = A_REVERSE;
		/* The curses attribute we use to highlight something. */
#ifdef ENABLE_COLOR
colortype* color_combo[NUMBER_OF_ELEMENTS] = {NULL};
		/* The color combinations for interface elements given in the rcfile. */
#endif
int interface_color_pair[NUMBER_OF_ELEMENTS] = {0};
		/* The processed color pairs for the interface elements. */

char *homedir = NULL;
		/* The user's home directory, from $HOME or /etc/passwd. */
char *statedir = NULL;
		/* The directory for nano's history files. */

#if defined(ENABLE_NANORC) || defined(ENABLE_HISTORIES)
char *startup_problem = NULL;
		/* An error message (if any) about nanorc files or history files. */
#endif
#ifdef ENABLE_NANORC
char *custom_nanorc = NULL;
#endif

bool spotlighted = FALSE;
	/* Whether any text is spotlighted. */
size_t light_from_col = 0;
	/* Where the spotlighted text starts. */
size_t light_to_col = 0;
	/* Where the spotlighted text ends. */

/* To make the functions and shortcuts lists clearer. */
#define VIEW  TRUE    /* Is allowed in view mode. */
#define NOVIEW  FALSE
#define BLANKAFTER  TRUE    /* A blank line after this one. */
#define TOGETHER  FALSE
#ifdef ENABLE_MULTIBUFFER
#define CAN_OPEN_OTHER_BUFFER TRUE
#else
#define CAN_OPEN_OTHER_BUFFER FALSE
#endif

/* Empty functions, for the most part corresponding to toggles. */
void case_sens_void(void)
{
}
void regexp_void(void)
{
}
void backwards_void(void)
{
}
void flip_replace(void)
{
}
void flip_goto(void)
{
}
#ifdef ENABLE_BROWSER
void to_files(void)
{
}
void goto_dir(void)
{
}
#endif
#ifndef NANO_TINY
void do_nothing(void)
{
}
void do_toggle_void(void)
{
}
void dos_format_void(void)
{
}
void mac_format_void(void)
{
}
void append_void(void)
{
}
void prepend_void(void)
{
}
void backup_file_void(void)
{
}
void flip_execute(void)
{
}
void flip_pipe(void)
{
}
void flip_convert(void)
{
}
#endif
#ifdef ENABLE_MULTIBUFFER
void flip_newbuffer(void)
{
}
#endif
void discard_buffer(void)
{
}
void do_cancel(void)
{
}

/* Add a function to the linked list of functions. */
void add_to_funcs(void (*func)(void), int menus, const char *desc,
					const char *help, bool blank_after, bool viewok)
{
	funcstruct *f = nmalloc(sizeof(funcstruct));

	if (allfuncs == NULL)
		allfuncs = f;
	else
		tailfunc->next = f;
	tailfunc = f;

	f->next = NULL;
	f->func = func;
	f->menus = menus;
	f->desc = desc;
	f->viewok = viewok;
#ifdef ENABLE_HELP
	f->help = help;
	f->blank_after = blank_after;
#endif
}

/* Parse the given keystring and return the corresponding keycode,
 * or return -1 when the string is invalid. */
int keycode_from_string(const char *keystring)
{
	if (keystring[0] == '^') {
		if (keystring[2] == '\0') {
			if (keystring[1] == '/')
				return 31;
			if (keystring[1] <= '_')
				return keystring[1] - 64;
			if (keystring[1] == '`')
				return 0;
			else
				return -1;
		} else if (strcasecmp(keystring, "^Space") == 0)
			return 0;
		else
			return -1;
	} else if (keystring[0] == 'M') {
		if (keystring[1] == '-' && keystring[3] == '\0')
			return tolower((unsigned char)keystring[2]);
		if (strcasecmp(keystring, "M-Space") == 0)
			return (int)' ';
		else
			return -1;
#ifdef ENABLE_NANORC
	} else if (strncasecmp(keystring, "Sh-M-", 5) == 0 &&
				'a' <= (keystring[5] | 0x20) && (keystring[5] | 0x20) <= 'z' &&
				keystring[6] == '\0') {
		shifted_metas = TRUE;
		return (keystring[5] & 0x5F);
#endif
	} else if (keystring[0] == 'F') {
		int fn = atoi(&keystring[1]);
		if (fn < 1 || fn > 24)
			return -1;
		return KEY_F0 + fn;
	} else if (strcasecmp(keystring, "Ins") == 0)
		return KEY_IC;
	else if (strcasecmp(keystring, "Del") == 0)
		return KEY_DC;
	else
		return -1;
}

/* Add a key combo to the linked list of shortcuts. */
void add_to_sclist(int menus, const char *scstring, const int keycode,
						void (*func)(void), int toggle)
{
	static keystruct *tailsc;
#ifndef NANO_TINY
	static int counter = 0;
#endif
	keystruct *sc = nmalloc(sizeof(keystruct));

	/* Start the list, or tack on the next item. */
	if (sclist == NULL)
		sclist = sc;
	else
		tailsc->next = sc;
	sc->next = NULL;

	/* Fill in the data. */
	sc->menus = menus;
	sc->func = func;
#ifndef NANO_TINY
	sc->toggle = toggle;
	/* When not the same toggle as the previous one, increment the ID. */
	if (toggle)
		sc->ordinal = (tailsc->toggle == toggle) ? counter : ++counter;
#endif
	sc->keystr = scstring;
	sc->keycode = (keycode ? keycode : keycode_from_string(scstring));

	tailsc = sc;
}

/* Return the first shortcut in the list of shortcuts that
 * matches the given func in the given menu. */
const keystruct *first_sc_for(int menu, void (*func)(void))
{
	for (keystruct *sc = sclist; sc != NULL; sc = sc->next)
		if ((sc->menus & menu) && sc->func == func)
			return sc;

	return NULL;
}

/* Return the first keycode that is bound to the given function in the
 * current menu, if any; otherwise, return the given default value. */
int the_code_for(void (*func)(void), int defaultval)
{
	const keystruct *sc = first_sc_for(currmenu, func);

	if (sc == NULL)
		return defaultval;

	meta_key = (0x20 <= sc->keycode && sc->keycode <= 0x7E);

	return sc->keycode;
}

/* Return the number of entries that can be shown in the given menu. */
size_t shown_entries_for(int menu)
{
	funcstruct *item = allfuncs;
	size_t maximum = ((COLS + 40) / 20) * 2;
	size_t count = 0;

	while (count < maximum && item != NULL) {
		if (item->menus & menu)
			count++;
		item = item->next;
	}

	/* When --tempfile is not used, widen the grid of the WriteOut menu. */
	if (menu == MWRITEFILE && first_sc_for(menu, discard_buffer) == NULL)
		count--;

	return count;
}

/* Return the first shortcut in the current menu that matches the given input. */
const keystruct *get_shortcut(int *keycode)
{
	/* Plain characters and upper control codes cannot be shortcuts. */
	if (!meta_key && 0x20 <= *keycode && *keycode <= 0xFF)
		return NULL;

	/* Lower control codes with Meta cannot be shortcuts either. */
	if (meta_key && *keycode < 0x20)
		return NULL;

	/* During a paste at a prompt, ignore all command keycodes. */
	if (bracketed_paste && *keycode != BRACKETED_PASTE_MARKER)
		return NULL;

	for (keystruct *sc = sclist; sc != NULL; sc = sc->next) {
		if ((sc->menus & currmenu) && *keycode == sc->keycode)
			return sc;
	}

	return NULL;
}

/* Return a pointer to the function that is bound to the given key. */
functionptrtype func_from_key(int *keycode)
{
	const keystruct *sc = get_shortcut(keycode);

	return (sc) ? sc->func : NULL;
}

#if defined(ENABLE_BROWSER) || defined(ENABLE_HELP)
/* Return the function that is bound to the given key in the file browser or
 * the help viewer.  Accept also certain plain characters, for compatibility
 * with Pico or to mimic 'less' and similar text viewers. */
functionptrtype interpret(int *keycode)
{
	if (!meta_key) {
		if (*keycode == 'N')
			return do_findprevious;
		if (*keycode == 'n')
			return do_findnext;

		switch (tolower(*keycode)) {
			case '-':
				return do_page_up;
			case ' ':
				return do_page_down;
			case 'w':
			case '/':
				return do_search_forward;
#ifdef ENABLE_BROWSER
			case 'g':
				return goto_dir;
#endif
			case '?':
				return do_help;
			case 's':
				return do_enter;
			case 'e':
			case 'q':
			case 'x':
				return do_exit;
		}
	}

	return func_from_key(keycode);
}
#endif /* ENABLE_BROWSER || ENABLE_HELP */

/* These two tags are used elsewhere too, so they are global. */
/* TRANSLATORS: Try to keep the next two strings at most 10 characters. */
const char *exit_tag = N_("Exit");
const char *close_tag = N_("Close");

/* Initialize the list of functions and the list of shortcuts. */
void shortcut_init(void)
{
#ifdef ENABLE_HELP
	/* TRANSLATORS: The next long series of strings are shortcut descriptions;
	 * they are best kept shorter than 56 characters, but may be longer. */
	const char *cancel_gist = N_("Cancel the current function");
	const char *help_gist = N_("Display this help text");
	const char *exit_gist = N_("Close the current buffer / Exit from nano");
	const char *writeout_gist =
		N_("Write the current buffer (or the marked region) to disk");
	const char *readfile_gist =
		N_("Insert another file into current buffer (or into new buffer)");
	const char *whereis_gist =
		N_("Search forward for a string or a regular expression");
	const char *wherewas_gist =
		N_("Search backward for a string or a regular expression");
	const char *cut_gist =
		N_("Cut current line (or marked region) and store it in cutbuffer");
	const char *paste_gist =
		N_("Paste the contents of cutbuffer at current cursor position");
	const char *cursorpos_gist = N_("Display the position of the cursor");
#ifdef ENABLE_SPELLER
	const char *spell_gist = N_("Invoke the spell checker, if available");
#endif
	const char *replace_gist = N_("Replace a string or a regular expression");
	const char *gotoline_gist = N_("Go to line and column number");
#ifndef NANO_TINY
	const char *mark_gist = N_("Mark text starting from the cursor position");
	const char *copy_gist =
		N_("Copy current line (or marked region) and store it in cutbuffer");
	const char *zap_gist = N_("Throw away the current line (or marked region)");
	const char *indent_gist = N_("Indent the current line (or marked lines)");
	const char *unindent_gist = N_("Unindent the current line (or marked lines)");
	const char *undo_gist = N_("Undo the last operation");
	const char *redo_gist = N_("Redo the last undone operation");
#endif
	const char *back_gist = N_("Go back one character");
	const char *forward_gist = N_("Go forward one character");
	const char *prevword_gist = N_("Go back one word");
	const char *nextword_gist = N_("Go forward one word");
	const char *prevline_gist = N_("Go to previous line");
	const char *nextline_gist = N_("Go to next line");
	const char *home_gist = N_("Go to beginning of current line");
	const char *end_gist = N_("Go to end of current line");
	const char *prevblock_gist = N_("Go to previous block of text");
	const char *nextblock_gist = N_("Go to next block of text");
#ifdef ENABLE_JUSTIFY
	const char *parabegin_gist =
		N_("Go to beginning of paragraph; then of previous paragraph");
	const char *paraend_gist =
		N_("Go just beyond end of paragraph; then of next paragraph");
#endif
	const char *prevpage_gist = N_("Go one screenful up");
	const char *nextpage_gist = N_("Go one screenful down");
	const char *firstline_gist = N_("Go to the first line of the file");
	const char *lastline_gist = N_("Go to the last line of the file");
#ifndef NANO_TINY
	const char *bracket_gist = N_("Go to the matching bracket");
#endif
#if !defined(NANO_TINY) || defined(ENABLE_HELP)
	const char *scrollup_gist =
		N_("Scroll up one line without moving the cursor textually");
	const char *scrolldown_gist =
		N_("Scroll down one line without moving the cursor textually");
#endif
#ifdef ENABLE_MULTIBUFFER
	const char *prevfile_gist = N_("Switch to the previous file buffer");
	const char *nextfile_gist = N_("Switch to the next file buffer");
#endif
	const char *verbatim_gist = N_("Insert the next keystroke verbatim");
	const char *tab_gist = N_("Insert a tab at the cursor position");
	const char *enter_gist = N_("Insert a newline at the cursor position");
	const char *delete_gist = N_("Delete the character under the cursor");
	const char *backspace_gist =
		N_("Delete the character to the left of the cursor");
#ifndef NANO_TINY
	const char *chopwordleft_gist =
		N_("Delete backward from cursor to word start");
	const char *chopwordright_gist =
		N_("Delete forward from cursor to next word start");
	const char *cuttilleof_gist =
		N_("Cut from the cursor position to the end of the file");
#endif
#ifdef ENABLE_JUSTIFY
	const char *justify_gist = N_("Justify the current paragraph");
	const char *fulljustify_gist = N_("Justify the entire file");
#endif
#ifndef NANO_TINY
	const char *wordcount_gist =
		N_("Count the number of words, lines, and characters");
#endif
	const char *refresh_gist = N_("Refresh (redraw) the current screen");
	const char *suspend_gist = N_("Suspend the editor (if suspension is enabled)");
#ifdef ENABLE_WORDCOMPLETION
	const char *completion_gist = N_("Try and complete the current word");
#endif
#ifdef ENABLE_COMMENT
	const char *comment_gist =
		N_("Comment/uncomment the current line (or marked lines)");
#endif
	const char *savefile_gist = N_("Save file without prompting");
	const char *findprev_gist = N_("Search next occurrence backward");
	const char *findnext_gist = N_("Search next occurrence forward");
#ifndef NANO_TINY
	const char *recordmacro_gist = N_("Start/stop recording a macro");
	const char *runmacro_gist = N_("Run the last recorded macro");
#endif
	const char *case_gist = N_("Toggle the case sensitivity of the search");
	const char *reverse_gist = N_("Reverse the direction of the search");
	const char *regexp_gist = N_("Toggle the use of regular expressions");
#ifdef ENABLE_HISTORIES
	const char *older_gist = N_("Recall the previous search/replace string");
	const char *newer_gist = N_("Recall the next search/replace string");
#endif
#ifndef NANO_TINY
	const char *dos_gist = N_("Toggle the use of DOS format");
	const char *mac_gist = N_("Toggle the use of Mac format");
	const char *append_gist = N_("Toggle appending");
	const char *prepend_gist = N_("Toggle prepending");
	const char *backup_gist = N_("Toggle backing up of the original file");
	const char *execute_gist = N_("Execute external command");
	const char *pipe_gist =
		N_("Pipe the current buffer (or marked region) to the command");
	const char *convert_gist = N_("Do not convert from DOS/Mac format");
#endif
#ifdef ENABLE_MULTIBUFFER
	const char *newbuffer_gist = N_("Toggle the use of a new buffer");
#endif
	const char *discardbuffer_gist = N_("Close buffer without saving it");
#ifdef ENABLE_BROWSER
	const char *tofiles_gist = N_("Go to file browser");
	const char *exitbrowser_gist = N_("Exit from the file browser");
	const char *firstfile_gist = N_("Go to the first file in the list");
	const char *lastfile_gist = N_("Go to the last file in the list");
	const char *backfile_gist = N_("Go to the previous file in the list");
	const char *forwardfile_gist = N_("Go to the next file in the list");
#ifndef NANO_TINY
	const char *browserlefthand_gist = N_("Go to lefthand column");
	const char *browserrighthand_gist = N_("Go to righthand column");
	const char *browsertoprow_gist = N_("Go to first row in this column");
	const char *browserbottomrow_gist = N_("Go to last row in this column");
#endif
	const char *browserwhereis_gist = N_("Search forward for a string");
	const char *browserwherewas_gist = N_("Search backward for a string");
	const char *browserrefresh_gist = N_("Refresh the file list");
	const char *gotodir_gist = N_("Go to directory");
#endif
#ifdef ENABLE_COLOR
	const char *lint_gist = N_("Invoke the linter, if available");
	const char *prevlint_gist = N_("Go to previous linter msg");
	const char *nextlint_gist = N_("Go to next linter msg");
#ifdef ENABLE_SPELLER
	const char *formatter_gist =
		N_("Invoke a program to format/arrange/manipulate the buffer");
#endif
#endif
#endif /* ENABLE_HELP */

#ifdef ENABLE_HELP
#define WITHORSANS(help) help
#else
#define WITHORSANS(help) ""
#endif

	/* Start populating the different menus with functions. */

	add_to_funcs(do_help, (MMOST | MBROWSER) & ~MFINDINHELP,
		/* TRANSLATORS: Try to keep the next eleven strings at most 10 characters. */
		N_("Get Help"), WITHORSANS(help_gist), TOGETHER, VIEW);

	add_to_funcs(do_cancel, ((MMOST & ~MMAIN) | MYESNO),
		N_("Cancel"), WITHORSANS(cancel_gist), BLANKAFTER, VIEW);

	add_to_funcs(do_exit, MMAIN,
		exit_tag, WITHORSANS(exit_gist), TOGETHER, VIEW);
	/* Remember the entry for Exit, to be able to replace it with Close. */
	exitfunc = tailfunc;

#ifdef ENABLE_BROWSER
	add_to_funcs(do_exit, MBROWSER,
		close_tag, WITHORSANS(exitbrowser_gist), TOGETHER, VIEW);
#endif

	add_to_funcs(do_writeout_void, MMAIN,
		N_("Write Out"), WITHORSANS(writeout_gist), TOGETHER, NOVIEW);

#ifdef ENABLE_JUSTIFY
	/* In restricted mode, replace Insert with Justify, when possible;
	 * otherwise, show Insert anyway, to keep the help items paired. */
	if (!ISSET(RESTRICTED))
#endif
		add_to_funcs(do_insertfile_void, MMAIN,
				N_("Read File"), WITHORSANS(readfile_gist), BLANKAFTER,
				/* We allow inserting files in view mode if multibuffer mode
				 * is available, so that the user can view multiple files. */
				CAN_OPEN_OTHER_BUFFER);
#ifdef ENABLE_JUSTIFY
	else
		add_to_funcs(do_justify_void, MMAIN,
				N_("Justify"), WITHORSANS(justify_gist), BLANKAFTER, NOVIEW);
#endif

#ifdef ENABLE_HELP
	/* The description ("x") and blank_after (0) are irrelevant,
	 * because the help viewer does not have a help text. */
	add_to_funcs(total_refresh, MHELP, N_("Refresh"), "x", 0, VIEW);
	add_to_funcs(do_exit, MHELP, close_tag, "x", 0, VIEW);
#endif

	add_to_funcs(do_search_forward, MMAIN|MHELP,
		N_("Where Is"), WITHORSANS(whereis_gist), TOGETHER, VIEW);

	add_to_funcs(do_replace, MMAIN,
		N_("Replace"), WITHORSANS(replace_gist), TOGETHER, NOVIEW);

	add_to_funcs(cut_text, MMAIN,
		N_("Cut Text"), WITHORSANS(cut_gist), TOGETHER, NOVIEW);

	add_to_funcs(paste_text, MMAIN,
		N_("Paste Text"), WITHORSANS(paste_gist), BLANKAFTER, NOVIEW);

	if (!ISSET(RESTRICTED)) {
#ifdef ENABLE_JUSTIFY
		add_to_funcs(do_justify_void, MMAIN,
				N_("Justify"), WITHORSANS(justify_gist), TOGETHER, NOVIEW);
#endif
#ifdef ENABLE_SPELLER
		add_to_funcs(do_spell, MMAIN,
				N_("To Spell"), WITHORSANS(spell_gist), BLANKAFTER, NOVIEW);
#endif
	}

	add_to_funcs(do_cursorpos_void, MMAIN,
		/* TRANSLATORS: Try to keep the next thirteen strings at most 12 characters. */
		N_("Cur Pos"), WITHORSANS(cursorpos_gist), TOGETHER, VIEW);

#if ((defined(ENABLE_JUSTIFY) && defined(ENABLE_SPELLER)) || \
		(!defined(ENABLE_JUSTIFY) && !defined(ENABLE_SPELLER)))
	/* Conditionally placing this one here or further on, to keep the
	 * help items nicely paired in most conditions. */
	add_to_funcs(do_gotolinecolumn_void, MMAIN,
		N_("Go To Line"), WITHORSANS(gotoline_gist), BLANKAFTER, VIEW);
#endif

#ifndef NANO_TINY
	add_to_funcs(do_undo, MMAIN,
		N_("Undo"), WITHORSANS(undo_gist), TOGETHER, NOVIEW);
	add_to_funcs(do_redo, MMAIN,
		N_("Redo"), WITHORSANS(redo_gist), BLANKAFTER, NOVIEW);

	add_to_funcs(do_mark, MMAIN,
		N_("Mark Text"), WITHORSANS(mark_gist), TOGETHER, VIEW);
	add_to_funcs(copy_text, MMAIN,
		N_("Copy Text"), WITHORSANS(copy_gist), BLANKAFTER, NOVIEW);
#endif

	add_to_funcs(case_sens_void, MWHEREIS|MREPLACE,
		N_("Case Sens"), WITHORSANS(case_gist), TOGETHER, VIEW);
	add_to_funcs(regexp_void, MWHEREIS|MREPLACE,
		N_("Regexp"), WITHORSANS(regexp_gist), TOGETHER, VIEW);
	add_to_funcs(backwards_void, MWHEREIS|MREPLACE,
		N_("Backwards"), WITHORSANS(reverse_gist), TOGETHER, VIEW);

	add_to_funcs(flip_replace, MWHEREIS,
		N_("Replace"), WITHORSANS(replace_gist), BLANKAFTER, VIEW);

	add_to_funcs(flip_replace, MREPLACE,
		N_("No Replace"), WITHORSANS(whereis_gist), BLANKAFTER, VIEW);

#ifdef ENABLE_HISTORIES
	add_to_funcs(get_history_older_void, MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE,
		N_("Older"), WITHORSANS(older_gist), TOGETHER, VIEW);
	add_to_funcs(get_history_newer_void, MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE,
		N_("Newer"), WITHORSANS(newer_gist), BLANKAFTER, VIEW);
#endif

	add_to_funcs(flip_goto, MWHEREIS,
		N_("Go To Line"), WITHORSANS(gotoline_gist), TOGETHER, VIEW);

#ifdef ENABLE_JUSTIFY
	add_to_funcs(do_full_justify, MWHEREIS,
		N_("FullJstify"), WITHORSANS(fulljustify_gist), BLANKAFTER, NOVIEW);
#endif

#ifdef ENABLE_BROWSER
	add_to_funcs(goto_dir, MBROWSER,
		/* TRANSLATORS: Try to keep the next seven strings at most 10 characters. */
		N_("Go To Dir"), WITHORSANS(gotodir_gist), TOGETHER, VIEW);

	add_to_funcs(total_refresh, MBROWSER,
		N_("Refresh"), WITHORSANS(browserrefresh_gist), BLANKAFTER, VIEW);

	add_to_funcs(do_search_forward, MBROWSER,
		N_("Where Is"), WITHORSANS(browserwhereis_gist), TOGETHER, VIEW);
	add_to_funcs(do_search_backward, MBROWSER,
		N_("Where Was"), WITHORSANS(browserwherewas_gist), TOGETHER, VIEW);
#endif

#ifndef NANO_TINY
	add_to_funcs(do_find_bracket, MMAIN,
		N_("To Bracket"), WITHORSANS(bracket_gist), BLANKAFTER, VIEW);

	add_to_funcs(do_search_backward, MMAIN|MHELP,
		/* TRANSLATORS: This starts a backward search. */
		N_("Where Was"), WITHORSANS(wherewas_gist), TOGETHER, VIEW);
#endif
	add_to_funcs(do_findprevious, MMAIN|MBROWSER|MHELP,
		/* TRANSLATORS: This refers to searching the preceding occurrence. */
		N_("Previous"), WITHORSANS(findprev_gist), TOGETHER, VIEW);
#ifndef NANO_TINY
	add_to_funcs(do_findnext, MMAIN|MBROWSER|MHELP,
		N_("Next"), WITHORSANS(findnext_gist), BLANKAFTER, VIEW);
#else
	/* In the tiny version, arrange things a bit differently, so that
	 * the useful M-W and M-Q are shown in the help lines. */
	add_to_funcs(do_findnext, MMAIN|MBROWSER|MHELP,
		N_("Next"), WITHORSANS(findnext_gist), TOGETHER, VIEW);
	add_to_funcs(do_search_backward, MMAIN|MHELP,
		N_("Where Was"), WITHORSANS(wherewas_gist), BLANKAFTER, VIEW);
	add_to_funcs(total_refresh, MMAIN,
		N_("Refresh"), WITHORSANS(refresh_gist), BLANKAFTER, VIEW);
#endif

	add_to_funcs(do_left, MMAIN,
		/* TRANSLATORS: This means move the cursor one character back. */
		N_("Back"), WITHORSANS(back_gist), TOGETHER, VIEW);
	add_to_funcs(do_right, MMAIN,
		N_("Forward"), WITHORSANS(forward_gist), TOGETHER, VIEW);
#ifdef ENABLE_BROWSER
	add_to_funcs(do_left, MBROWSER,
		N_("Back"), WITHORSANS(backfile_gist), TOGETHER, VIEW);
	add_to_funcs(do_right, MBROWSER,
		N_("Forward"), WITHORSANS(forwardfile_gist), TOGETHER, VIEW);
#endif

	add_to_funcs(to_prev_word, MMAIN,
		/* TRANSLATORS: Try to keep the next eighteen strings at most 12 characters. */
		N_("Prev Word"), WITHORSANS(prevword_gist), TOGETHER, VIEW);
	add_to_funcs(to_next_word, MMAIN,
		N_("Next Word"), WITHORSANS(nextword_gist), TOGETHER, VIEW);

	add_to_funcs(do_home, MMAIN,
		N_("Home"), WITHORSANS(home_gist), TOGETHER, VIEW);
	add_to_funcs(do_end, MMAIN,
		N_("End"), WITHORSANS(end_gist), BLANKAFTER, VIEW);

	add_to_funcs(do_up, MMAIN|MBROWSER|MHELP,
		N_("Prev Line"), WITHORSANS(prevline_gist), TOGETHER, VIEW);
	add_to_funcs(do_down, MMAIN|MBROWSER|MHELP,
		N_("Next Line"), WITHORSANS(nextline_gist), TOGETHER, VIEW);
#if !defined(NANO_TINY) || defined(ENABLE_HELP)
	add_to_funcs(do_scroll_up, MMAIN,
		N_("Scroll Up"), WITHORSANS(scrollup_gist), TOGETHER, VIEW);
	add_to_funcs(do_scroll_down, MMAIN,
		N_("Scroll Down"), WITHORSANS(scrolldown_gist), BLANKAFTER, VIEW);
#endif

	add_to_funcs(to_prev_block, MMAIN,
		N_("Prev Block"), WITHORSANS(prevblock_gist), TOGETHER, VIEW);
	add_to_funcs(to_next_block, MMAIN,
		N_("Next Block"), WITHORSANS(nextblock_gist), TOGETHER, VIEW);
#ifdef ENABLE_JUSTIFY
	add_to_funcs(to_para_begin, MMAIN|MGOTOLINE,
		N_("Beg of Par"), WITHORSANS(parabegin_gist), TOGETHER, VIEW);
	add_to_funcs(to_para_end, MMAIN|MGOTOLINE,
		N_("End of Par"), WITHORSANS(paraend_gist), BLANKAFTER, VIEW);
#endif

	add_to_funcs(do_page_up, MMAIN|MHELP,
		N_("Prev Page"), WITHORSANS(prevpage_gist), TOGETHER, VIEW);
	add_to_funcs(do_page_down, MMAIN|MHELP,
		N_("Next Page"), WITHORSANS(nextpage_gist), TOGETHER, VIEW);

	add_to_funcs(to_first_line, MMAIN|MHELP|MGOTOLINE,
		N_("First Line"), WITHORSANS(firstline_gist), TOGETHER, VIEW);
	add_to_funcs(to_last_line, MMAIN|MHELP|MGOTOLINE,
		N_("Last Line"), WITHORSANS(lastline_gist), BLANKAFTER, VIEW);

#ifdef ENABLE_MULTIBUFFER
	add_to_funcs(switch_to_prev_buffer, MMAIN,
		N_("Prev File"), WITHORSANS(prevfile_gist), TOGETHER, VIEW);
	add_to_funcs(switch_to_next_buffer, MMAIN,
		N_("Next File"), WITHORSANS(nextfile_gist), BLANKAFTER, VIEW);
#endif

#if ((!defined(ENABLE_JUSTIFY) && defined(ENABLE_SPELLER)) || \
		(defined(ENABLE_JUSTIFY) && !defined(ENABLE_SPELLER)))
	add_to_funcs(do_gotolinecolumn_void, MMAIN,
		N_("Go To Line"), WITHORSANS(gotoline_gist), BLANKAFTER, VIEW);
#endif

	add_to_funcs(do_tab, MMAIN,
		/* TRANSLATORS: The next four strings are names of keyboard keys. */
		N_("Tab"), WITHORSANS(tab_gist), TOGETHER, NOVIEW);
	add_to_funcs(do_enter, MMAIN,
		N_("Enter"), WITHORSANS(enter_gist), BLANKAFTER, NOVIEW);

	add_to_funcs(do_backspace, MMAIN,
		N_("Backspace"), WITHORSANS(backspace_gist), TOGETHER, NOVIEW);
	add_to_funcs(do_delete, MMAIN,
		N_("Delete"), WITHORSANS(delete_gist),
#ifndef NANO_TINY
		TOGETHER,
#else
		BLANKAFTER,
#endif
		NOVIEW);

#ifndef NANO_TINY
	add_to_funcs(chop_previous_word, MMAIN,
		/* TRANSLATORS: The next two strings refer to deleting words. */
		N_("Chop Left"), WITHORSANS(chopwordleft_gist), TOGETHER, NOVIEW);
	add_to_funcs(chop_next_word, MMAIN,
		N_("Chop Right"), WITHORSANS(chopwordright_gist), TOGETHER, NOVIEW);
	add_to_funcs(cut_till_eof, MMAIN,
		N_("CutTillEnd"), WITHORSANS(cuttilleof_gist), BLANKAFTER, NOVIEW);
#endif

#ifdef ENABLE_JUSTIFY
	add_to_funcs(do_full_justify, MMAIN,
		N_("FullJstify"), WITHORSANS(fulljustify_gist), TOGETHER, NOVIEW);
#endif

#ifndef NANO_TINY
	add_to_funcs(do_wordlinechar_count, MMAIN,
		N_("Word Count"), WITHORSANS(wordcount_gist), TOGETHER, VIEW);
#endif

	add_to_funcs(do_verbatim_input, MMAIN,
		N_("Verbatim"), WITHORSANS(verbatim_gist), BLANKAFTER, NOVIEW);

#ifndef NANO_TINY
	add_to_funcs(total_refresh, MMAIN,
		N_("Refresh"), WITHORSANS(refresh_gist), TOGETHER, VIEW);
#endif

	add_to_funcs(do_suspend_void, MMAIN,
		N_("Suspend"), WITHORSANS(suspend_gist), BLANKAFTER, VIEW);

#ifndef NANO_TINY
	add_to_funcs(do_indent, MMAIN,
		N_("Indent"), WITHORSANS(indent_gist), TOGETHER, NOVIEW);
	add_to_funcs(do_unindent, MMAIN,
		N_("Unindent"), WITHORSANS(unindent_gist), BLANKAFTER, NOVIEW);
#endif
#ifdef ENABLE_COMMENT
	add_to_funcs(do_comment, MMAIN,
		N_("Comment Lines"), WITHORSANS(comment_gist), TOGETHER, NOVIEW);
#endif
#ifdef ENABLE_WORDCOMPLETION
	add_to_funcs(complete_a_word, MMAIN,
		N_("Complete"), WITHORSANS(completion_gist), BLANKAFTER, NOVIEW);
#endif
#ifndef NANO_TINY
	add_to_funcs(record_macro, MMAIN,
		N_("Record"), WITHORSANS(recordmacro_gist), TOGETHER, VIEW);
	add_to_funcs(run_macro, MMAIN,
		N_("Run Macro"), WITHORSANS(runmacro_gist), BLANKAFTER, VIEW);

	add_to_funcs(zap_text, MMAIN,
		N_("Zap Text"), WITHORSANS(zap_gist), BLANKAFTER, NOVIEW);

#ifdef ENABLE_COLOR
	if (!ISSET(RESTRICTED)) {
		add_to_funcs(do_linter, MMAIN,
				N_("To Linter"), WITHORSANS(lint_gist), TOGETHER, NOVIEW);
#ifdef ENABLE_SPELLER
		add_to_funcs(do_formatter, MMAIN,
				N_("Formatter"), WITHORSANS(formatter_gist), BLANKAFTER, NOVIEW);
#endif
	}
#endif
#endif
	add_to_funcs(do_savefile, MMAIN,
		N_("Save"), WITHORSANS(savefile_gist), BLANKAFTER, NOVIEW);

	add_to_funcs(flip_goto, MGOTOLINE,
		N_("Go To Text"), WITHORSANS(whereis_gist), BLANKAFTER, VIEW);

#ifndef NANO_TINY
	add_to_funcs(dos_format_void, MWRITEFILE,
		N_("DOS Format"), WITHORSANS(dos_gist), TOGETHER, NOVIEW);
	add_to_funcs(mac_format_void, MWRITEFILE,
		N_("Mac Format"), WITHORSANS(mac_gist), TOGETHER, NOVIEW);

	/* If we're using restricted mode, the Append, Prepend, and Backup toggles
	 * are disabled.  The first and second are not useful as they only allow
	 * reduplicating the current file, and the third is not allowed as it
	 * would write to a file not specified on the command line. */
	if (!ISSET(RESTRICTED)) {
		add_to_funcs(append_void, MWRITEFILE,
			N_("Append"), WITHORSANS(append_gist), TOGETHER, NOVIEW);
		add_to_funcs(prepend_void, MWRITEFILE,
			N_("Prepend"), WITHORSANS(prepend_gist), TOGETHER, NOVIEW);

		add_to_funcs(backup_file_void, MWRITEFILE,
			N_("Backup File"), WITHORSANS(backup_gist), BLANKAFTER, NOVIEW);
	}
#endif /* !NANO_TINY */
#ifdef ENABLE_MULTIBUFFER
	/* Multiple buffers are only available when not in restricted mode. */
	if (!ISSET(RESTRICTED))
		add_to_funcs(flip_newbuffer, MINSERTFILE|MEXTCMD,
			N_("New Buffer"), WITHORSANS(newbuffer_gist), TOGETHER, NOVIEW);
#endif
#ifndef NANO_TINY
	add_to_funcs(flip_convert, MINSERTFILE,
		N_("No Conversion"), WITHORSANS(convert_gist), TOGETHER, NOVIEW);

	/* Command execution is only available when not in restricted mode. */
	if (!ISSET(RESTRICTED) && !ISSET(VIEW_MODE)) {
		add_to_funcs(flip_execute, MINSERTFILE,
			N_("Execute Command"), WITHORSANS(execute_gist), TOGETHER, NOVIEW);

		add_to_funcs(flip_pipe, MEXTCMD,
			N_("Pipe Text"), WITHORSANS(pipe_gist), TOGETHER, NOVIEW);

		add_to_funcs(flip_execute, MEXTCMD,
			N_("Read File"), WITHORSANS(readfile_gist), TOGETHER, NOVIEW);
	}
#endif
#ifdef ENABLE_BROWSER
	/* The file browser is only available when not in restricted mode. */
	if (!ISSET(RESTRICTED))
		add_to_funcs(to_files, MWRITEFILE|MINSERTFILE,
			N_("To Files"), WITHORSANS(tofiles_gist), TOGETHER, VIEW);

	add_to_funcs(do_page_up, MBROWSER,
		N_("Prev Page"), WITHORSANS(prevpage_gist), TOGETHER, VIEW);
	add_to_funcs(do_page_down, MBROWSER,
		N_("Next Page"), WITHORSANS(nextpage_gist), TOGETHER, VIEW);

	add_to_funcs(to_first_file, MBROWSER|MWHEREISFILE,
		N_("First File"), WITHORSANS(firstfile_gist), TOGETHER, VIEW);
	add_to_funcs(to_last_file, MBROWSER|MWHEREISFILE,
		N_("Last File"), WITHORSANS(lastfile_gist), BLANKAFTER, VIEW);
#ifndef NANO_TINY
	add_to_funcs(to_prev_word, MBROWSER,
		N_("Left Column"), WITHORSANS(browserlefthand_gist), TOGETHER, VIEW);
	add_to_funcs(to_next_word, MBROWSER,
		N_("Right Column"), WITHORSANS(browserrighthand_gist), TOGETHER, VIEW);
	add_to_funcs(to_prev_block, MBROWSER,
		N_("Top Row"), WITHORSANS(browsertoprow_gist), TOGETHER, VIEW);
	add_to_funcs(to_next_block, MBROWSER,
		N_("Bottom Row"), WITHORSANS(browserbottomrow_gist), BLANKAFTER, VIEW);
#endif
#endif /* ENABLE_BROWSER */

	add_to_funcs(discard_buffer, MWRITEFILE,
		N_("Discard buffer"), WITHORSANS(discardbuffer_gist), BLANKAFTER, NOVIEW);

#ifdef ENABLE_COLOR
	add_to_funcs(do_page_up, MLINTER,
		/* TRANSLATORS: The next two strings may be up to 37 characters each. */
		N_("Prev Lint Msg"), WITHORSANS(prevlint_gist), TOGETHER, VIEW);
	add_to_funcs(do_page_down, MLINTER,
		N_("Next Lint Msg"), WITHORSANS(nextlint_gist), TOGETHER, VIEW);
#endif

	/* Link key combos to functions in certain menus. */
	add_to_sclist(MMOST|MBROWSER, "^M", '\r', do_enter, 0);
	add_to_sclist(MMOST|MBROWSER, "Enter", KEY_ENTER, do_enter, 0);
	add_to_sclist(MMOST, "^H", '\b', do_backspace, 0);
	add_to_sclist(MMOST, "Bsp", KEY_BACKSPACE, do_backspace, 0);
	add_to_sclist(MMOST, "Sh-Del", SHIFT_DELETE, do_backspace, 0);
	add_to_sclist(MMOST, "^D", 0, do_delete, 0);
	add_to_sclist(MMOST, "Del", KEY_DC, do_delete, 0);
	add_to_sclist(MMOST, "^I", '\t', do_tab, 0);
	add_to_sclist(MMOST, "Tab", '\t', do_tab, 0);
	add_to_sclist((MMOST|MBROWSER) & ~MFINDINHELP, "^G", 0, do_help, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "^X", 0, do_exit, 0);
	if (!ISSET(PRESERVE))
		add_to_sclist(MMAIN, "^S", 0, do_savefile, 0);
	add_to_sclist(MMAIN, "^O", 0, do_writeout_void, 0);
	add_to_sclist(MMAIN, "^R", 0, do_insertfile_void, 0);
	add_to_sclist(MMAIN, "Ins", KEY_IC, do_insertfile_void, 0);
	if (!ISSET(PRESERVE))
		add_to_sclist(MMAIN|MBROWSER|MHELP, "^Q", 0, do_search_backward, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "^W", 0, do_search_forward, 0);
	add_to_sclist(MMAIN, "^\\", 0, do_replace, 0);
	add_to_sclist(MMAIN, "M-R", 0, do_replace, 0);
	add_to_sclist(MMOST, "^K", 0, cut_text, 0);
	add_to_sclist(MMOST, "^U", 0, paste_text, 0);
#ifdef ENABLE_JUSTIFY
	add_to_sclist(MMAIN, "^J", '\n', do_justify_void, 0);
#endif
#ifdef ENABLE_SPELLER
	add_to_sclist(MMAIN, "^T", 0, do_spell, 0);
#endif
#ifdef ENABLE_COLOR
	add_to_sclist(MMAIN, "M-B", 0, do_linter, 0);
#ifdef ENABLE_SPELLER
	add_to_sclist(MMAIN, "M-F", 0, do_formatter, 0);
#endif
#endif
	add_to_sclist(MMAIN, "^C", 0, do_cursorpos_void, 0);
	add_to_sclist(MMAIN, "^_", 0, do_gotolinecolumn_void, 0);
	add_to_sclist(MMAIN, "M-G", 0, do_gotolinecolumn_void, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP|MLINTER, "^Y", 0, do_page_up, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP|MLINTER, "PgUp", KEY_PPAGE, do_page_up, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP|MLINTER, "^V", 0, do_page_down, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP|MLINTER, "PgDn", KEY_NPAGE, do_page_down, 0);
	add_to_sclist(MBROWSER|MHELP, "Bsp", KEY_BACKSPACE, do_page_up, 0);
	add_to_sclist(MBROWSER|MHELP, "Sh-Del", SHIFT_DELETE, do_page_up, 0);
	add_to_sclist(MBROWSER|MHELP, "Space", 0x20, do_page_down, 0);
	add_to_sclist(MMAIN|MHELP, "M-\\", 0, to_first_line, 0);
	add_to_sclist(MMAIN|MHELP, "^Home", CONTROL_HOME, to_first_line, 0);
	add_to_sclist(MMAIN|MHELP, "M-/", 0, to_last_line, 0);
	add_to_sclist(MMAIN|MHELP, "^End", CONTROL_END, to_last_line, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "M-W", 0, do_findnext, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "M-Q", 0, do_findprevious, 0);
#ifndef NANO_TINY
	add_to_sclist(MMAIN, "M-]", 0, do_find_bracket, 0);
	add_to_sclist(MMAIN, "M-A", 0, do_mark, 0);
	add_to_sclist(MMAIN, "^6", 0, do_mark, 0);
	add_to_sclist(MMAIN, "^^", 0, do_mark, 0);
	add_to_sclist(MMAIN, "M-6", 0, copy_text, 0);
	add_to_sclist(MMAIN, "M-^", 0, copy_text, 0);
	add_to_sclist(MMAIN, "M-}", 0, do_indent, 0);
	add_to_sclist(MMAIN, "Tab", INDENT_KEY, do_indent, 0);
	add_to_sclist(MMAIN, "M-{", 0, do_unindent, 0);
	add_to_sclist(MMAIN, "Sh-Tab", SHIFT_TAB, do_unindent, 0);
	add_to_sclist(MMAIN, "M-:", 0, record_macro, 0);
	add_to_sclist(MMAIN, "M-;", 0, run_macro, 0);
	add_to_sclist(MMAIN, "M-U", 0, do_undo, 0);
	add_to_sclist(MMAIN, "M-E", 0, do_redo, 0);
	add_to_sclist(MMAIN, "Sh-^Del", CONTROL_SHIFT_DELETE, chop_previous_word, 0);
	add_to_sclist(MMAIN, "^Del", CONTROL_DELETE, chop_next_word, 0);
	add_to_sclist(MMAIN, "M-Del", ALT_DELETE, zap_text, 0);
#endif
#ifdef ENABLE_WORDCOMPLETION
	add_to_sclist(MMAIN, "^]", 0, complete_a_word, 0);
#endif
#ifdef ENABLE_COMMENT
	add_to_sclist(MMAIN, "M-3", 0, do_comment, 0);
#endif
	add_to_sclist(MMOST|MBROWSER, "^B", 0, do_left, 0);
	add_to_sclist(MMOST|MBROWSER, "^F", 0, do_right, 0);
#ifdef ENABLE_UTF8
	if (using_utf8()) {
		add_to_sclist(MMOST|MBROWSER|MHELP, "\xE2\x97\x80", KEY_LEFT, do_left, 0);
		add_to_sclist(MMOST|MBROWSER|MHELP, "\xE2\x96\xb6", KEY_RIGHT, do_right, 0);
		add_to_sclist(MSOME, "^\xE2\x97\x80", CONTROL_LEFT, to_prev_word, 0);
		add_to_sclist(MSOME, "^\xE2\x96\xb6", CONTROL_RIGHT, to_next_word, 0);
#if !defined(NANO_TINY) && defined(ENABLE_MULTIBUFFER)
		if (!on_a_vt) {
			add_to_sclist(MMAIN, "M-\xE2\x97\x80", ALT_LEFT, switch_to_prev_buffer, 0);
			add_to_sclist(MMAIN, "M-\xE2\x96\xb6", ALT_RIGHT, switch_to_next_buffer, 0);
		}
#endif
	} else
#endif
	{
		add_to_sclist(MMOST|MBROWSER|MHELP, "Left", KEY_LEFT, do_left, 0);
		add_to_sclist(MMOST|MBROWSER|MHELP, "Right", KEY_RIGHT, do_right, 0);
		add_to_sclist(MSOME, "^Left", CONTROL_LEFT, to_prev_word, 0);
		add_to_sclist(MSOME, "^Right", CONTROL_RIGHT, to_next_word, 0);
#ifdef ENABLE_MULTIBUFFER
		if (!on_a_vt) {
			add_to_sclist(MMAIN, "M-Left", ALT_LEFT, switch_to_prev_buffer, 0);
			add_to_sclist(MMAIN, "M-Right", ALT_RIGHT, switch_to_next_buffer, 0);
		}
#endif
	}
#ifdef NANO_TINY
	add_to_sclist(MMAIN, "M-B", 0, to_prev_word, 0);
	add_to_sclist(MMAIN, "M-D", 0, to_prev_word, 0);
	add_to_sclist(MMAIN, "M-F", 0, to_next_word, 0);
	add_to_sclist(MMAIN, "M-N", 0, to_next_word, 0);
#endif
	add_to_sclist(MMOST, "M-Space", 0, to_prev_word, 0);
	add_to_sclist(MMOST, "^Space", 0, to_next_word, 0);
	add_to_sclist(MMOST, "^A", 0, do_home, 0);
	add_to_sclist(MMOST, "Home", KEY_HOME, do_home, 0);
	add_to_sclist(MMOST, "^E", 0, do_end, 0);
	add_to_sclist(MMOST, "End", KEY_END, do_end, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "^P", 0, do_up, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "^N", 0, do_down, 0);
#ifdef ENABLE_UTF8
	if (using_utf8()) {
		add_to_sclist(MMAIN|MBROWSER|MHELP, "\xE2\x96\xb2", KEY_UP, do_up, 0);
		add_to_sclist(MMAIN|MBROWSER|MHELP, "\xE2\x96\xbc", KEY_DOWN, do_down, 0);
		add_to_sclist(MMAIN|MBROWSER|MLINTER, "^\xE2\x96\xb2", CONTROL_UP, to_prev_block, 0);
		add_to_sclist(MMAIN|MBROWSER|MLINTER, "^\xE2\x96\xbc", CONTROL_DOWN, to_next_block, 0);
	} else
#endif
	{
		add_to_sclist(MMAIN|MBROWSER|MHELP, "Up", KEY_UP, do_up, 0);
		add_to_sclist(MMAIN|MBROWSER|MHELP, "Down", KEY_DOWN, do_down, 0);
		add_to_sclist(MMAIN|MBROWSER|MLINTER, "^Up", CONTROL_UP, to_prev_block, 0);
		add_to_sclist(MMAIN|MBROWSER|MLINTER, "^Down", CONTROL_DOWN, to_next_block, 0);
	}
	add_to_sclist(MMAIN, "M-7", 0, to_prev_block, 0);
	add_to_sclist(MMAIN, "M-8", 0, to_next_block, 0);
#ifdef ENABLE_JUSTIFY
	add_to_sclist(MMAIN, "M-(", 0, to_para_begin, 0);
	add_to_sclist(MMAIN, "M-9", 0, to_para_begin, 0);
	add_to_sclist(MMAIN, "M-)", 0, to_para_end, 0);
	add_to_sclist(MMAIN, "M-0", 0, to_para_end, 0);
#endif
#ifndef NANO_TINY
#ifdef ENABLE_UTF8
	if (using_utf8()) {
		add_to_sclist(MMAIN|MHELP, "M-\xE2\x96\xb2", ALT_UP, do_scroll_up, 0);
		add_to_sclist(MMAIN|MHELP, "M-\xE2\x96\xbc", ALT_DOWN, do_scroll_down, 0);
	} else
#endif
	{
		add_to_sclist(MMAIN|MHELP, "M-Up", ALT_UP, do_scroll_up, 0);
		add_to_sclist(MMAIN|MHELP, "M-Down", ALT_DOWN, do_scroll_down, 0);
	}
#endif
#if !defined(NANO_TINY) || defined(ENABLE_HELP)
	add_to_sclist(MMAIN|MHELP, "M--", 0, do_scroll_up, 0);
	add_to_sclist(MMAIN|MHELP, "M-_", 0, do_scroll_up, 0);
	add_to_sclist(MMAIN|MHELP, "M-+", 0, do_scroll_down, 0);
	add_to_sclist(MMAIN|MHELP, "M-=", 0, do_scroll_down, 0);
#endif
#ifdef ENABLE_MULTIBUFFER
	add_to_sclist(MMAIN, "M-<", 0, switch_to_prev_buffer, 0);
	add_to_sclist(MMAIN, "M-,", 0, switch_to_prev_buffer, 0);
	add_to_sclist(MMAIN, "M->", 0, switch_to_next_buffer, 0);
	add_to_sclist(MMAIN, "M-.", 0, switch_to_next_buffer, 0);
#endif
	add_to_sclist(MMOST, "M-V", 0, do_verbatim_input, 0);
#ifndef NANO_TINY
	add_to_sclist(MMAIN, "M-T", 0, cut_till_eof, 0);
	add_to_sclist(MMAIN, "M-D", 0, do_wordlinechar_count, 0);
#endif
#ifdef ENABLE_JUSTIFY
	if (!ISSET(VIEW_MODE))
		add_to_sclist(MMAIN|MWHEREIS, "M-J", 0, do_full_justify, 0);
#endif
	if (!ISSET(PRESERVE))
		add_to_sclist(MMAIN|MBROWSER|MHELP, "^L", 0, total_refresh, 0);
	else
		add_to_sclist(MMAIN|MBROWSER, "^L", 0, total_refresh, 0);
	add_to_sclist(MMAIN, "^Z", 0, do_suspend_void, 0);

#ifndef NANO_TINY
	/* Group of "Appearance" toggles. */
	add_to_sclist(MMAIN, "M-X", 0, do_toggle_void, NO_HELP);
	add_to_sclist(MMAIN, "M-C", 0, do_toggle_void, CONSTANT_SHOW);
	add_to_sclist(MMAIN, "M-S", 0, do_toggle_void, SOFTWRAP);
	add_to_sclist(MMAIN, "M-$", 0, do_toggle_void, SOFTWRAP);
#ifdef ENABLE_LINENUMBERS
	add_to_sclist(MMAIN, "M-N", 0, do_toggle_void, LINE_NUMBERS);
	add_to_sclist(MMAIN, "M-#", 0, do_toggle_void, LINE_NUMBERS);
#endif
	add_to_sclist(MMAIN, "M-P", 0, do_toggle_void, WHITESPACE_DISPLAY);
#ifdef ENABLE_COLOR
	add_to_sclist(MMAIN, "M-Y", 0, do_toggle_void, NO_COLOR_SYNTAX);
#endif

	/* Group of "Editing-behavior" toggles. */
	add_to_sclist(MMAIN, "M-H", 0, do_toggle_void, SMART_HOME);
	add_to_sclist(MMAIN, "M-I", 0, do_toggle_void, AUTOINDENT);
	add_to_sclist(MMAIN, "M-K", 0, do_toggle_void, CUT_FROM_CURSOR);
#ifdef ENABLE_WRAPPING
	add_to_sclist(MMAIN, "M-L", 0, do_toggle_void, BREAK_LONG_LINES);
#endif
	add_to_sclist(MMAIN, "M-O", 0, do_toggle_void, TABS_TO_SPACES);

	/* Group of "Peripheral-feature" toggles. */
#ifdef ENABLE_MOUSE
	add_to_sclist(MMAIN, "M-M", 0, do_toggle_void, USE_MOUSE);
#endif
	add_to_sclist(MMAIN, "M-Z", 0, do_toggle_void, SUSPENDABLE);
#endif /* !NANO_TINY */

	add_to_sclist(((MMOST & ~MMAIN) | MYESNO), "^C", 0, do_cancel, 0);

	add_to_sclist(MWHEREIS|MREPLACE, "M-C", 0, case_sens_void, 0);
	add_to_sclist(MWHEREIS|MREPLACE, "M-R", 0, regexp_void, 0);
	add_to_sclist(MWHEREIS|MREPLACE, "M-B", 0, backwards_void, 0);
	add_to_sclist(MWHEREIS|MREPLACE, "^R", 0, flip_replace, 0);
	add_to_sclist(MWHEREIS|MGOTOLINE, "^T", 0, flip_goto, 0);
#ifdef ENABLE_HISTORIES
	add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE|MFINDINHELP|MEXTCMD, "^P", 0, get_history_older_void, 0);
	add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE|MFINDINHELP|MEXTCMD, "^N", 0, get_history_newer_void, 0);
#ifdef ENABLE_UTF8
	if (using_utf8()) {
		add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE|MFINDINHELP|MEXTCMD, "\xE2\x96\xb2", KEY_UP, get_history_older_void, 0);
		add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE|MFINDINHELP|MEXTCMD, "\xE2\x96\xbc", KEY_DOWN, get_history_newer_void, 0);
	} else
#endif
	{
		add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE|MFINDINHELP|MEXTCMD, "Up", KEY_UP, get_history_older_void, 0);
		add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE|MFINDINHELP|MEXTCMD, "Down", KEY_DOWN, get_history_newer_void, 0);
	}
#endif
#ifdef ENABLE_JUSTIFY
	add_to_sclist(MGOTOLINE, "^W", 0, to_para_begin, 0);
	add_to_sclist(MGOTOLINE, "^O", 0, to_para_end, 0);
#endif
	add_to_sclist(MGOTOLINE, "^Y", 0, to_first_line, 0);
	add_to_sclist(MGOTOLINE, "^V", 0, to_last_line, 0);
	/* Some people are used to having these keystrokes in the Search menu. */
	add_to_sclist(MWHEREIS, "^Y", 0, to_first_line, 0);
	add_to_sclist(MWHEREIS, "^V", 0, to_last_line, 0);
#ifdef ENABLE_BROWSER
	add_to_sclist(MWHEREISFILE, "^Y", 0, to_first_file, 0);
	add_to_sclist(MWHEREISFILE, "^V", 0, to_last_file, 0);
	add_to_sclist(MBROWSER|MWHEREISFILE, "M-\\", 0, to_first_file, 0);
	add_to_sclist(MBROWSER|MWHEREISFILE, "M-/", 0, to_last_file, 0);
	add_to_sclist(MBROWSER, "Home", KEY_HOME, to_first_file, 0);
	add_to_sclist(MBROWSER, "End", KEY_END, to_last_file, 0);
	add_to_sclist(MBROWSER, "^Home", CONTROL_HOME, to_first_file, 0);
	add_to_sclist(MBROWSER, "^End", CONTROL_END, to_last_file, 0);
	add_to_sclist(MBROWSER, "^_", 0, goto_dir, 0);
	add_to_sclist(MBROWSER, "M-G", 0, goto_dir, 0);
#endif
	if (ISSET(TEMP_FILE) && !ISSET(PRESERVE))
		add_to_sclist(MWRITEFILE, "^Q", 0, discard_buffer, 0);
#ifndef NANO_TINY
	add_to_sclist(MWRITEFILE, "M-D", 0, dos_format_void, 0);
	add_to_sclist(MWRITEFILE, "M-M", 0, mac_format_void, 0);
	/* Only when not in restricted mode, allow Appending, Prepending,
	 * making backups, and executing a command. */
	if (!ISSET(RESTRICTED) && !ISSET(VIEW_MODE)) {
		add_to_sclist(MWRITEFILE, "M-A", 0, append_void, 0);
		add_to_sclist(MWRITEFILE, "M-P", 0, prepend_void, 0);
		add_to_sclist(MWRITEFILE, "M-B", 0, backup_file_void, 0);
		add_to_sclist(MINSERTFILE|MEXTCMD, "^X", 0, flip_execute, 0);
	}
	add_to_sclist(MINSERTFILE, "M-N", 0, flip_convert, 0);
#endif
#ifdef ENABLE_MULTIBUFFER
	/* Only when not in restricted mode, allow multiple buffers. */
	if (!ISSET(RESTRICTED)) {
		add_to_sclist(MINSERTFILE|MEXTCMD, "M-F", 0, flip_newbuffer, 0);
#ifndef NANO_TINY
		add_to_sclist(MEXTCMD, "M-\\", 0, flip_pipe, 0);
#endif
	}
#endif
#ifdef ENABLE_BROWSER
	/* Only when not in restricted mode, allow entering the file browser. */
	if (!ISSET(RESTRICTED))
		add_to_sclist(MWRITEFILE|MINSERTFILE, "^T", 0, to_files, 0);
#endif
	add_to_sclist(MBROWSER|MHELP, "^C", 0, do_exit, 0);
	/* Allow exiting from the file browser and the help viewer with
	 * the same key as they were entered. */
#ifdef ENABLE_BROWSER
	add_to_sclist(MBROWSER, "^T", 0, do_exit, 0);
#endif
#ifdef ENABLE_HELP
	add_to_sclist(MHELP, "^G", 0, do_exit, 0);
	add_to_sclist(MHELP, "F1", KEY_F(1), do_exit, 0);
	add_to_sclist(MHELP, "Home", KEY_HOME, to_first_line, 0);
	add_to_sclist(MHELP, "End", KEY_END, to_last_line, 0);
#endif
#ifdef ENABLE_COLOR
	add_to_sclist(MLINTER, "^X", 0, do_cancel, 0);
#endif
	add_to_sclist(MMOST & ~MFINDINHELP, "F1", KEY_F(1), do_help, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "F2", KEY_F(2), do_exit, 0);
	add_to_sclist(MMAIN, "F3", KEY_F(3), do_writeout_void, 0);
#ifdef ENABLE_JUSTIFY
	add_to_sclist(MMAIN, "F4", KEY_F(4), do_justify_void, 0);
#endif
	add_to_sclist(MMAIN, "F5", KEY_F(5), do_insertfile_void, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP, "F6", KEY_F(6), do_search_forward, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP|MLINTER, "F7", KEY_F(7), do_page_up, 0);
	add_to_sclist(MMAIN|MBROWSER|MHELP|MLINTER, "F8", KEY_F(8), do_page_down, 0);
	add_to_sclist(MMOST, "F9", KEY_F(9), cut_text, 0);
	add_to_sclist(MMOST, "F10", KEY_F(10), paste_text, 0);
	add_to_sclist(MMAIN, "F11", KEY_F(11), do_cursorpos_void, 0);
#ifdef ENABLE_SPELLER
	add_to_sclist(MMAIN, "F12", KEY_F(12), do_spell, 0);
#endif
#ifndef NANO_TINY
	/* Catch and ignore bracketed paste marker keys. */
	add_to_sclist(MMOST|MBROWSER|MHELP|MYESNO, "", BRACKETED_PASTE_MARKER, do_nothing, 0);
#endif
}

#ifndef NANO_TINY
/* Return the textual description that corresponds to the given flag. */
const char *flagtostr(int flag)
{
	switch (flag) {
		case NO_HELP:
			/* TRANSLATORS: The next thirteen strings are toggle descriptions;
			 * they are best kept shorter than 40 characters, but may be longer. */
			return N_("Help mode");
		case CONSTANT_SHOW:
			return N_("Constant cursor position display");
		case SOFTWRAP:
			return N_("Soft wrapping of overlong lines");
		case WHITESPACE_DISPLAY:
			return N_("Whitespace display");
		case NO_COLOR_SYNTAX:
			return N_("Color syntax highlighting");
		case SMART_HOME:
			return N_("Smart home key");
		case AUTOINDENT:
			return N_("Auto indent");
		case CUT_FROM_CURSOR:
			return N_("Cut to end");
		case BREAK_LONG_LINES:
			return N_("Hard wrapping of overlong lines");
		case TABS_TO_SPACES:
			return N_("Conversion of typed tabs to spaces");
		case USE_MOUSE:
			return N_("Mouse support");
		case SUSPENDABLE:
			return N_("Suspension");
		case LINE_NUMBERS:
			return N_("Line numbering");
		default:
			die("Bad toggle -- please report a bug\n");
			return "";
	}
}
#endif /* !NANO_TINY */
