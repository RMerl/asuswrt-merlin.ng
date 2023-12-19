/**************************************************************************
 *   prompt.c  --  This file is part of GNU nano.                         *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2023 Free Software Foundation, Inc.    *
 *   Copyright (C) 2016, 2018, 2020, 2022 Benno Schulenberg               *
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

#include "prototypes.h"

#include <string.h>

static char *prompt = NULL;
		/* The prompt string used for status-bar questions. */
static size_t typing_x = HIGHEST_POSITIVE;
		/* The cursor position in answer. */

/* Move to the beginning of the answer. */
void do_statusbar_home(void)
{
	typing_x = 0;
}

/* Move to the end of the answer. */
void do_statusbar_end(void)
{
	typing_x = strlen(answer);
}

#ifndef NANO_TINY
/* Move to the previous word in the answer. */
void do_statusbar_prev_word(void)
{
	bool seen_a_word = FALSE, step_forward = FALSE;

	/* Move backward until we pass over the start of a word. */
	while (typing_x != 0) {
		typing_x = step_left(answer, typing_x);

		if (is_word_char(answer + typing_x, FALSE))
			seen_a_word = TRUE;
#ifdef ENABLE_UTF8
		else if (is_zerowidth(answer + typing_x))
			; /* skip */
#endif
		else if (seen_a_word) {
			/* This is space now: we've overshot the start of the word. */
			step_forward = TRUE;
			break;
		}
	}

	if (step_forward)
		/* Move one character forward again to sit on the start of the word. */
		typing_x = step_right(answer, typing_x);
}

/* Move to the next word in the answer. */
void do_statusbar_next_word(void)
{
	bool seen_space = !is_word_char(answer + typing_x, FALSE);
	bool seen_word = !seen_space;

	/* Move forward until we reach either the end or the start of a word,
	 * depending on whether the AFTER_ENDS flag is set or not. */
	while (answer[typing_x] != '\0') {
		typing_x = step_right(answer, typing_x);

		if (ISSET(AFTER_ENDS)) {
			/* If this is a word character, continue; else it's a separator,
			 * and if we've already seen a word, then it's a word end. */
			if (is_word_char(answer + typing_x, FALSE))
				seen_word = TRUE;
#ifdef ENABLE_UTF8
			else if (is_zerowidth(answer + typing_x))
				; /* skip */
#endif
			else if (seen_word)
				break;
		} else {
#ifdef ENABLE_UTF8
			if (is_zerowidth(answer + typing_x))
				; /* skip */
			else
#endif
			/* If this is not a word character, then it's a separator; else
			 * if we've already seen a separator, then it's a word start. */
			if (!is_word_char(answer + typing_x, FALSE))
				seen_space = TRUE;
			else if (seen_space)
				break;
		}
	}
}
#endif /* !NANO_TINY */

/* Move left one character in the answer. */
void do_statusbar_left(void)
{
	if (typing_x > 0) {
		typing_x = step_left(answer, typing_x);
#ifdef ENABLE_UTF8
		while (typing_x > 0 && is_zerowidth(answer + typing_x))
			typing_x = step_left(answer, typing_x);
#endif
	}
}

/* Move right one character in the answer. */
void do_statusbar_right(void)
{
	if (answer[typing_x] != '\0') {
		typing_x = step_right(answer, typing_x);
#ifdef ENABLE_UTF8
		while (answer[typing_x] != '\0' && is_zerowidth(answer + typing_x))
			typing_x = step_right(answer, typing_x);
#endif
	}
}

/* Backspace over one character in the answer. */
void do_statusbar_backspace(void)
{
	if (typing_x > 0) {
		size_t was_x = typing_x;

		typing_x = step_left(answer, typing_x);
		memmove(answer + typing_x, answer + was_x, strlen(answer) - was_x + 1);
	}
}

/* Delete one character in the answer. */
void do_statusbar_delete(void)
{
	if (answer[typing_x] != '\0') {
		int charlen = char_length(answer + typing_x);

		memmove(answer + typing_x, answer + typing_x + charlen,
						strlen(answer) - typing_x - charlen + 1);
#ifdef ENABLE_UTF8
		if (is_zerowidth(answer + typing_x))
			do_statusbar_delete();
#endif
	}
}

/* Zap the part of the answer after the cursor, or the whole answer. */
void lop_the_answer(void)
{
	if (answer[typing_x] == '\0')
		typing_x = 0;

	answer[typing_x] = '\0';
}

#ifndef NANO_TINY
/* Copy the current answer (if any) into the cutbuffer. */
void copy_the_answer(void)
{
	if (*answer) {
		free_lines(cutbuffer);
		cutbuffer = make_new_node(NULL);
		cutbuffer->data = copy_of(answer);
		typing_x = 0;
	}
}

/* Paste the first line of the cutbuffer into the current answer. */
void paste_into_answer(void)
{
	size_t pastelen = strlen(cutbuffer->data);

	answer = nrealloc(answer, strlen(answer) + pastelen + 1);
	memmove(answer + typing_x + pastelen, answer + typing_x,
								strlen(answer) - typing_x + 1);
	strncpy(answer + typing_x, cutbuffer->data, pastelen);

	typing_x += pastelen;
}
#endif

#ifdef ENABLE_MOUSE
/* Handle a mouse click on the status-bar prompt or the shortcut list. */
int do_statusbar_mouse(void)
{
	int click_row, click_col;
	int retval = get_mouseinput(&click_row, &click_col, TRUE);

	/* We can click on the status-bar window text to move the cursor. */
	if (retval == 0 && wmouse_trafo(footwin, &click_row, &click_col, FALSE)) {
		size_t start_col = breadth(prompt) + 2;

		/* Move to where the click occurred. */
		if (click_row == 0 && click_col >= start_col)
			typing_x = actual_x(answer,
							get_statusbar_page_start(start_col, start_col +
							wideness(answer, typing_x)) + click_col - start_col);
	}

	return retval;
}
#endif

/* Insert the given short burst of bytes into the answer. */
void inject_into_answer(char *burst, size_t count)
{
	/* First encode any embedded NUL byte as 0x0A. */
	for (size_t index = 0; index < count; index++)
		if (burst[index] == '\0')
			burst[index] = '\n';

	answer = nrealloc(answer, strlen(answer) + count + 1);
	memmove(answer + typing_x + count, answer + typing_x,
								strlen(answer) - typing_x + 1);
	strncpy(answer + typing_x, burst, count);

	typing_x += count;
}

/* Get a verbatim keystroke and insert it into the answer. */
void do_statusbar_verbatim_input(void)
{
	size_t count = 1;
	char *bytes;

	bytes = get_verbatim_kbinput(footwin, &count);

	if (0 < count && count < 999)
		inject_into_answer(bytes, count);
	else if (count == 0)
		beep();

	free(bytes);
}

/* Add the given input to the input buffer when it's a normal byte,
 * and inject the gathered bytes into the answer when ready. */
void absorb_character(int input, functionptrtype function)
{
	static char *puddle = NULL;
		/* The input buffer. */
	static size_t capacity = 8;
		/* The size of the input buffer; gets doubled whenever needed. */
	static size_t depth = 0;
		/* The length of the input buffer. */

	/* If not a command, discard anything that is not a normal character byte.
	 * Apart from that, only accept input when not in restricted mode, or when
	 * not at the "Write File" prompt, or when there is no filename yet. */
	if (!function) {
		if (input < 0x20 || input > 0xFF || meta_key)
			beep();
		else if (!ISSET(RESTRICTED) || currmenu != MWRITEFILE ||
						openfile->filename[0] == '\0') {
			/* When the input buffer (plus room for terminating NUL) is full,
			 * extend it; otherwise, if it does not exist yet, create it. */
			if (depth + 1 == capacity) {
				capacity = 2 * capacity;
				puddle = nrealloc(puddle, capacity);
			} else if (!puddle)
				puddle = nmalloc(capacity);

			puddle[depth++] = (char)input;
		}
	}

	/* If there are gathered bytes and we have a command or no other key codes
	 * are waiting, it's time to insert these bytes into the answer. */
	if (depth > 0 && (function || waiting_keycodes() == 0)) {
		puddle[depth] = '\0';
		inject_into_answer(puddle, depth);
		depth = 0;
	}
}

/* Handle any editing shortcut, and return TRUE when handled. */
bool handle_editing(functionptrtype function)
{
	if (function == do_left)
		do_statusbar_left();
	else if (function == do_right)
		do_statusbar_right();
#ifndef NANO_TINY
	else if (function == to_prev_word)
		do_statusbar_prev_word();
	else if (function == to_next_word)
		do_statusbar_next_word();
#endif
	else if (function == do_home)
		do_statusbar_home();
	else if (function == do_end)
		do_statusbar_end();
	/* When in restricted mode at the "Write File" prompt and the
	 * filename isn't blank, disallow any input and deletion. */
	else if (ISSET(RESTRICTED) && currmenu == MWRITEFILE &&
							openfile->filename[0] != '\0' &&
							(function == do_verbatim_input ||
							function == do_delete || function == do_backspace ||
							function == cut_text || function == paste_text))
		;
	else if (function == do_verbatim_input)
		do_statusbar_verbatim_input();
	else if (function == do_delete)
		do_statusbar_delete();
	else if (function == do_backspace)
		do_statusbar_backspace();
	else if (function == cut_text)
		lop_the_answer();
#ifndef NANO_TINY
	else if (function == copy_text)
		copy_the_answer();
	else if (function == paste_text) {
		if (cutbuffer != NULL)
			paste_into_answer();
	}
#endif
	else
		return FALSE;

	/* Don't handle any handled function again. */
	return TRUE;
}

/* Return the column number of the first character of the answer that is
 * displayed in the status bar when the cursor is at the given column,
 * with the available room for the answer starting at base.  Note that
 * (0 <= column - get_statusbar_page_start(column) < COLS). */
size_t get_statusbar_page_start(size_t base, size_t column)
{
	if (column == base || column < COLS - 1)
		return 0;
	else if (COLS > base + 2)
		return column - base - 1 - (column - base - 1) % (COLS - base - 2);
	else
		return column - 2;
}

/* Reinitialize the cursor position in the answer. */
void put_cursor_at_end_of_answer(void)
{
	typing_x = HIGHEST_POSITIVE;
}

/* Redraw the prompt bar and place the cursor at the right spot. */
void draw_the_promptbar(void)
{
	size_t base = breadth(prompt) + 2;
	size_t column = base + wideness(answer, typing_x);
	size_t the_page, end_page;
	char *expanded;

	the_page = get_statusbar_page_start(base, column);
	end_page = get_statusbar_page_start(base, base + breadth(answer) - 1);

	/* Color the prompt bar over its full width. */
	wattron(footwin, interface_color_pair[PROMPT_BAR]);
	mvwprintw(footwin, 0, 0, "%*s", COLS, " ");

	mvwaddstr(footwin, 0, 0, prompt);
	waddch(footwin, ':');
	waddch(footwin, (the_page == 0) ? ' ' : '<');

	expanded = display_string(answer, the_page, COLS - base, FALSE, TRUE);
	waddstr(footwin, expanded);
	free(expanded);

	if (the_page < end_page && base + breadth(answer) - the_page > COLS)
		mvwaddch(footwin, 0, COLS - 1, '>');

	wattroff(footwin, interface_color_pair[PROMPT_BAR]);

#if defined(NCURSES_VERSION_PATCH) && (NCURSES_VERSION_PATCH < 20210220)
	/* Work around a cursor-misplacement bug -- https://sv.gnu.org/bugs/?59808. */
	if (ISSET(NO_HELP)) {
		wmove(footwin, 0, 0);
		wrefresh(footwin);
	}
#endif

	/* Place the cursor at the right spot. */
	wmove(footwin, 0, column - the_page);
	wnoutrefresh(footwin);
}

#ifndef NANO_TINY
/* Remove or add the pipe character at the answer's head. */
void add_or_remove_pipe_symbol_from_answer(void)
{
	if (answer[0] == '|') {
		memmove(answer, answer + 1, strlen(answer));
		if (typing_x > 0)
			typing_x--;
	} else {
		answer = nrealloc(answer, strlen(answer) + 2);
		memmove(answer + 1, answer, strlen(answer) + 1);
		answer[0] = '|';
		typing_x++;
	}
}
#endif

/* Get a string of input at the status-bar prompt. */
functionptrtype acquire_an_answer(int *actual, bool *listed,
					linestruct **history_list, void (*refresh_func)(void))
{
#ifdef ENABLE_HISTORIES
	char *stored_string = NULL;
		/* Whatever the answer was before the user foraged into history. */
#ifdef ENABLE_TABCOMP
	bool previous_was_tab = FALSE;
		/* Whether the previous keystroke was an attempt at tab completion. */
	size_t fragment_length = 0;
		/* The length of the fragment that the user tries to tab complete. */
#endif
#endif
	const keystruct *shortcut;
	functionptrtype function;
	int input;

	if (typing_x > strlen(answer))
		typing_x = strlen(answer);

	while (TRUE) {
		draw_the_promptbar();

		/* Read in one keystroke. */
		input = get_kbinput(footwin, VISIBLE);

#ifndef NANO_TINY
		/* If the window size changed, go reformat the prompt string. */
		if (input == KEY_WINCH) {
			refresh_func();  /* Only needed when in file browser. */
			*actual = KEY_WINCH;
#ifdef ENABLE_HISTORIES
			free(stored_string);
#endif
			return NULL;
		}
#endif
#ifdef ENABLE_MOUSE
		/* For a click on a shortcut, read in the resulting keycode. */
		if (input == KEY_MOUSE && do_statusbar_mouse() == 1)
			input = get_kbinput(footwin, BLIND);
		if (input == KEY_MOUSE)
			continue;
#endif

		/* Check for a shortcut in the current list. */
		shortcut = get_shortcut(input);
		function = (shortcut ? shortcut->func : NULL);

		/* When it's a normal character, add it to the answer. */
		absorb_character(input, function);

		if (function == do_cancel || function == do_enter)
			break;

#ifdef ENABLE_TABCOMP
		if (function == do_tab) {
#ifdef ENABLE_HISTORIES
			if (history_list != NULL) {
				if (!previous_was_tab)
					fragment_length = strlen(answer);

				if (fragment_length > 0) {
					answer = get_history_completion(history_list,
													answer, fragment_length);
					typing_x = strlen(answer);
				}
			} else
#endif
			/* Allow tab completion of filenames, but not in restricted mode. */
			if ((currmenu & (MINSERTFILE|MWRITEFILE|MGOTODIR)) && !ISSET(RESTRICTED))
				answer = input_tab(answer, &typing_x, refresh_func, listed);
		} else
#endif
#ifdef ENABLE_HISTORIES
		if (function == get_older_item && history_list != NULL) {
			/* If this is the first step into history, start at the bottom. */
			if (stored_string == NULL)
				reset_history_pointer_for(*history_list);

			/* When moving up from the bottom, remember the current answer. */
			if ((*history_list)->next == NULL)
				stored_string = mallocstrcpy(stored_string, answer);

			/* If there is an older item, move to it and copy its string. */
			if ((*history_list)->prev != NULL) {
				*history_list = (*history_list)->prev;
				answer = mallocstrcpy(answer, (*history_list)->data);
				typing_x = strlen(answer);
			}
		} else if (function == get_newer_item && history_list != NULL) {
			/* If there is a newer item, move to it and copy its string. */
			if ((*history_list)->next != NULL) {
				*history_list = (*history_list)->next;
				answer = mallocstrcpy(answer, (*history_list)->data);
				typing_x = strlen(answer);
			}

			/* When at the bottom of the history list, restore the old answer. */
			if ((*history_list)->next == NULL && stored_string && *answer == '\0') {
				answer = mallocstrcpy(answer, stored_string);
				typing_x = strlen(answer);
			}
		} else
#endif /* ENABLE_HISTORIES */
		if (function == do_help || function == full_refresh)
			function();
#ifndef NANO_TINY
		else if (function == do_toggle && shortcut->toggle == NO_HELP) {
			TOGGLE(NO_HELP);
			window_init();
			focusing = FALSE;
			refresh_func();
			bottombars(currmenu);
		} else if (function == do_nothing)
			;
#endif
#ifdef ENABLE_NANORC
		else if (function == (functionptrtype)implant)
			implant(shortcut->expansion);
#endif
		else if (function && !handle_editing(function)) {
			/* When it's a permissible shortcut, run it and done. */
			if (!ISSET(VIEW_MODE) || !changes_something(function)) {
				function();
				break;
			} else
				beep();
		}

#if defined(ENABLE_HISTORIES) && defined(ENABLE_TABCOMP)
		previous_was_tab = (function == do_tab);
#endif
	}

#ifdef ENABLE_HISTORIES
	/* If the history pointer was moved, point it at the bottom again. */
	if (stored_string != NULL) {
		reset_history_pointer_for(*history_list);
		free(stored_string);
	}
#endif

	*actual = input;

	return function;
}

/* Ask a question on the status bar.  Return 0 when text was entered,
 * -1 for a cancelled entry, -2 for a blank string, and the relevant
 * keycode when a valid shortcut key was pressed.  The 'provided'
 * parameter is the default answer for when simply Enter is typed. */
int do_prompt(int menu, const char *provided, linestruct **history_list,
				void (*refresh_func)(void), const char *msg, ...)
{
	functionptrtype function = NULL;
	bool listed = FALSE;
	va_list ap;
	int retval;
	/* Save a possible current status-bar x position and prompt. */
	size_t was_typing_x = typing_x;
	char *saved_prompt = prompt;

	bottombars(menu);

	if (answer != provided)
		answer = mallocstrcpy(answer, provided);

#ifndef NANO_TINY
  redo_theprompt:
#endif
	prompt = nmalloc((COLS * MAXCHARLEN) + 1);
	va_start(ap, msg);
	vsnprintf(prompt, COLS * MAXCHARLEN, msg, ap);
	va_end(ap);
	/* Reserve five columns for colon plus angles plus answer, ":<aa>". */
	prompt[actual_x(prompt, (COLS < 5) ? 0 : COLS - 5)] = '\0';

	lastmessage = VACUUM;

	function = acquire_an_answer(&retval, &listed, history_list, refresh_func);
	free(prompt);

#ifndef NANO_TINY
	if (retval == KEY_WINCH)
		goto redo_theprompt;
#endif

	/* Restore a possible previous prompt and maybe the typing position. */
	prompt = saved_prompt;
	if (function == do_cancel || function == do_enter)
		typing_x = was_typing_x;

	/* Set the proper return value for Cancel and Enter. */
	if (function == do_cancel)
		retval = -1;
	else if (function == do_enter)
		retval = (*answer == '\0') ? -2 : 0;

	if (lastmessage == VACUUM)
		wipe_statusbar();

#ifdef ENABLE_TABCOMP
	/* If possible filename completions are still listed, clear them off. */
	if (listed)
		refresh_func();
#endif

	return retval;
}

#define UNDECIDED  -2

/* Ask a simple Yes/No (and optionally All) question on the status bar
 * and return the choice -- either YES or NO or ALL or CANCEL. */
int ask_user(bool withall, const char *question)
{
	int choice = UNDECIDED;
	int width = 16;
	/* TRANSLATORS: For the next three strings, specify the starting letters
	 * of the translations for "Yes"/"No"/"All".  The first letter of each of
	 * these strings MUST be a single-byte letter; others may be multi-byte. */
	const char *yesstr = _("Yy");
	const char *nostr = _("Nn");
	const char *allstr = _("Aa");
	const keystruct *shortcut;
	functionptrtype function;

	while (choice == UNDECIDED) {
#ifdef ENABLE_NLS
		char letter[MAXCHARLEN + 1];
		int index = 0;
#endif
		int kbinput;

		if (!ISSET(NO_HELP)) {
			char shortstr[MAXCHARLEN + 2];
				/* Temporary string for (translated) " Y", " N" and " A". */
			const keystruct *cancelshortcut = first_sc_for(MYESNO, do_cancel);
				/* The keystroke that is bound to the Cancel function. */

			if (COLS < 32)
				width = COLS / 2;

			/* Clear the shortcut list from the bottom of the screen. */
			blank_bottombars();

			/* Now show the ones for "Yes", "No", "Cancel" and maybe "All". */
			sprintf(shortstr, " %c", yesstr[0]);
			wmove(footwin, 1, 0);
			post_one_key(shortstr, _("Yes"), width);

			shortstr[1] = nostr[0];
			wmove(footwin, 2, 0);
			post_one_key(shortstr, _("No"), width);

			if (withall) {
				shortstr[1] = allstr[0];
				wmove(footwin, 1, width);
				post_one_key(shortstr, _("All"), width);
			}

			wmove(footwin, 2, width);
			post_one_key(cancelshortcut->keystr, _("Cancel"), width);
		}

		/* Color the prompt bar over its full width and display the question. */
		wattron(footwin, interface_color_pair[PROMPT_BAR]);
		mvwprintw(footwin, 0, 0, "%*s", COLS, " ");
		mvwaddnstr(footwin, 0, 0, question, actual_x(question, COLS - 1));
		wattroff(footwin, interface_color_pair[PROMPT_BAR]);
		wnoutrefresh(footwin);

		currmenu = MYESNO;

		/* When not replacing, show the cursor while waiting for a key. */
		kbinput = get_kbinput(footwin, !withall);

#ifndef NANO_TINY
		if (kbinput == KEY_WINCH)
			continue;

		/* Accept first character of an external paste and ignore the rest. */
		if (bracketed_paste)
			kbinput = get_kbinput(footwin, BLIND);
		while (bracketed_paste)
			get_kbinput(footwin, BLIND);
#endif

#ifdef ENABLE_NLS
		letter[index++] = (unsigned char)kbinput;
#ifdef ENABLE_UTF8
		/* If the received code is a UTF-8 starter byte, get also the
		 * continuation bytes and assemble them into one letter. */
		if (using_utf8() && 0xC0 <= kbinput && kbinput <= 0xF7) {
			int extras = (kbinput / 16) % 4 + (kbinput <= 0xCF ? 1 : 0);

			while (extras <= waiting_keycodes() && extras-- > 0)
				letter[index++] = (unsigned char)get_kbinput(footwin, !withall);
		}
#endif
		letter[index] = '\0';

		/* See if the typed letter is in the Yes, No, or All strings. */
		if (strstr(yesstr, letter) != NULL)
			choice = YES;
		else if (strstr(nostr, letter) != NULL)
			choice = NO;
		else if (withall && strstr(allstr, letter) != NULL)
			choice = ALL;
		else
#endif /* ENABLE_NLS */
		if (strchr("Yy", kbinput) != NULL)
			choice = YES;
		else if (strchr("Nn", kbinput) != NULL)
			choice = NO;
		else if (withall && strchr("Aa", kbinput) != NULL)
			choice = ALL;

		if (choice != UNDECIDED)
			break;

		shortcut = get_shortcut(kbinput);
		function = (shortcut ? shortcut->func : NULL);

		if (function == do_cancel)
			choice = CANCEL;
		else if (function == full_refresh)
			full_refresh();
#ifndef NANO_TINY
		else if (function == do_toggle && shortcut->toggle == NO_HELP) {
			TOGGLE(NO_HELP);
			window_init();
			titlebar(NULL);
			focusing = FALSE;
			edit_refresh();
			focusing = TRUE;
		}
#endif
		/* Interpret ^N and ^Q as "No", to allow exiting in anger. */
		else if (kbinput == '\x0E' || kbinput == '\x11')
			choice = NO;
		/* And interpret ^Y as "Yes". */
		else if (kbinput == '\x19')
			choice = YES;
#ifdef ENABLE_MOUSE
		else if (kbinput == KEY_MOUSE) {
			int mouse_x, mouse_y;
			/* We can click on the Yes/No/All shortcuts to select an answer. */
			if (get_mouseinput(&mouse_y, &mouse_x, FALSE) == 0 &&
						wmouse_trafo(footwin, &mouse_y, &mouse_x, FALSE) &&
						mouse_x < (width * 2) && mouse_y > 0) {
				int x = mouse_x / width;
				int y = mouse_y - 1;

				/* x == 0 means Yes or No, y == 0 means Yes or All. */
				choice = -2 * x * y + x - y + 1;

				if (choice == ALL && !withall)
					choice = UNDECIDED;
			}
		}
#endif
		else
			beep();
	}

	return choice;
}
