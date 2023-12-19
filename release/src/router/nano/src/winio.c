/**************************************************************************
 *   winio.c  --  This file is part of GNU nano.                          *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2023 Free Software Foundation, Inc.    *
 *   Copyright (C) 2014-2022 Benno Schulenberg                            *
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
#include "revision.h"

#include <ctype.h>
#ifdef __linux__
#include <sys/ioctl.h>
#endif
#include <string.h>
#ifdef ENABLE_UTF8
#include <wchar.h>
#endif

#ifdef REVISION
#define BRANDING  REVISION
#else
#define BRANDING  PACKAGE_STRING
#endif

/* When having an older ncurses, then most likely libvte is older too. */
#if defined(NCURSES_VERSION_PATCH) && (NCURSES_VERSION_PATCH < 20200212)
#define USING_OLDER_LIBVTE  yes
#endif

static int *key_buffer = NULL;
		/* A buffer for the keystrokes that haven't been handled yet. */
static int *nextcodes = NULL;
		/* A pointer pointing at the next keycode in the keystroke buffer. */
static size_t capacity = 32;
		/* The size of the keystroke buffer; gets doubled whenever needed. */
static size_t waiting_codes = 0;
		/* The number of key codes waiting in the keystroke buffer. */
#ifdef ENABLE_NANORC
static const char *plants_pointer = NULL;
		/* Points into the expansion string for the current implantation. */
#endif
static int digit_count = 0;
		/* How many digits of a three-digit character code we've eaten. */
static bool reveal_cursor = FALSE;
		/* Whether the cursor should be shown when waiting for input. */
static bool linger_after_escape = FALSE;
		/* Whether to give ncurses some time to get the next code. */
static int statusblank = 0;
		/* The number of keystrokes left before we blank the status bar. */
static size_t from_x = 0;
		/* From where in the relevant line the current row is drawn. */
static size_t till_x = 0;
		/* Until where in the relevant line the current row is drawn. */
static bool has_more = FALSE;
		/* Whether the current line has more text after the displayed part. */
static bool is_shorter = TRUE;
		/* Whether a row's text is narrower than the screen's width. */
#ifndef NANO_TINY
static size_t sequel_column = 0;
		/* The starting column of the next chunk when softwrapping. */
static bool recording = FALSE;
		/* Whether we are in the process of recording a macro. */
static int *macro_buffer = NULL;
		/* A buffer where the recorded key codes are stored. */
static size_t macro_length = 0;
		/* The current length of the macro. */

/* Add the given code to the macro buffer. */
void add_to_macrobuffer(int code)
{
	macro_length++;
	macro_buffer = nrealloc(macro_buffer, macro_length * sizeof(int));
	macro_buffer[macro_length - 1] = code;
}

/* Remove the last key code plus any leading Esc codes from macro buffer. */
void snip_last_keystroke(void)
{
	macro_length--;
	while (macro_length > 0 && macro_buffer[macro_length - 1] == '\x1b')
		macro_length--;
}

/* Start or stop the recording of keystrokes. */
void record_macro(void)
{
	recording = !recording;

	if (recording) {
		macro_length = 0;
		statusline(REMARK, _("Recording a macro..."));
	} else {
		snip_last_keystroke();
		statusline(REMARK, _("Stopped recording"));
	}

	if (ISSET(STATEFLAGS))
		titlebar(NULL);
}

/* Copy the stored sequence of codes into the regular key buffer,
 * so they will be "executed" again. */
void run_macro(void)
{
	if (recording) {
		statusline(AHEM, _("Cannot run macro while recording"));
		snip_last_keystroke();
		return;
	}

	if (macro_length == 0) {
		statusline(REMARK, _("Macro is empty"));
		return;
	}

	if (macro_length > capacity)
		reserve_space_for(macro_length);

	for (size_t i = 0; i < macro_length; i++)
		key_buffer[i] = macro_buffer[i];

	waiting_codes = macro_length;
	nextcodes = key_buffer;
	mute_modifiers = TRUE;
}
#endif /* !NANO_TINY */

/* Allocate the requested space for the keystroke buffer. */
void reserve_space_for(size_t newsize)
{
	if (newsize < capacity)
		die(_("Too much input at once\n"));

	key_buffer = nrealloc(key_buffer, newsize * sizeof(int));
	nextcodes = key_buffer;
	capacity = newsize;
}

/* Control character compatibility:
 *
 * - Ctrl-H is Backspace under ASCII, ANSI, VT100, and VT220.
 * - Ctrl-I is Tab under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-M is Enter under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-Q is XON under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-S is XOFF under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-? is Delete under ASCII, ANSI, VT100, and VT220,
 *          but is Backspace under VT320.
 *
 * Note: the VT220 and VT320 also generate Esc [ 3 ~ for Delete.  By default,
 * xterm assumes it's running on a VT320 and generates Ctrl-? for Backspace
 * and Esc [ 3 ~ for Delete.  This causes problems for VT100-derived terminals
 * such as the FreeBSD console, which expect Ctrl-H for Backspace and Ctrl-?
 * for Delete, and on which ncurses translates the VT320 sequences to KEY_DC
 * and [nothing].  We work around this conflict via the REBIND_DELETE flag:
 * if it's set, we assume VT100 compatibility, and VT320 otherwise.
 *
 * Escape sequence compatibility:
 *
 * We support escape sequences for ANSI, VT100, VT220, VT320, the Linux
 * console, the FreeBSD console, the Mach console, xterm, and Terminal,
 * and some for Konsole, rxvt, Eterm, and iTerm2.  Among these sequences,
 * there are some conflicts:
 *
 * - PageUp on FreeBSD console == Tab on ANSI; the latter is omitted.
 *   (Ctrl-I is also Tab on ANSI, which we already support.)
 * - PageDown on FreeBSD console == Center (5) on numeric keypad with
 *   NumLock off on Linux console; the latter is useless and omitted.
 * - F1 on FreeBSD console == the mouse sequence on xterm/rxvt/Eterm;
 *   the latter is omitted.  (Mouse input works only when KEY_MOUSE
 *   is generated on mouse events, not with the raw escape sequence.)
 * - F9 on FreeBSD console == PageDown on Mach console; the former is
 *   omitted.  (Moving the cursor is more important than a function key.)
 * - F10 on FreeBSD console == PageUp on Mach console; the former is
 *   omitted.  (Same as above.) */

/* Read in at least one keystroke from the given window
 * and save it (or them) in the keystroke buffer. */
void read_keys_from(WINDOW *frame)
{
	int input = ERR;
	size_t errcount = 0;
#ifndef NANO_TINY
	bool timed = FALSE;
#endif

	/* Before reading the first keycode, display any pending screen updates. */
	doupdate();

	if (reveal_cursor && (!spotlighted || ISSET(SHOW_CURSOR) || currmenu == MSPELL) &&
						(LINES > 1 || lastmessage <= HUSH))
		curs_set(1);

#ifndef NANO_TINY
	if (currmenu == MMAIN && (((ISSET(MINIBAR) || ISSET(ZERO) || LINES == 1) &&
						lastmessage > HUSH && lastmessage < ALERT &&
						lastmessage != INFO) || spotlighted)) {
		timed = TRUE;
		halfdelay(ISSET(QUICK_BLANK) ? 8 : 15);
		/* Counteract a side effect of half-delay mode. */
		disable_kb_interrupt();
	}
#endif

	/* Read in the first keycode, waiting for it to arrive. */
	while (input == ERR) {
		input = wgetch(frame);

#ifndef NANO_TINY
		if (the_window_resized) {
			regenerate_screen();
			input = KEY_WINCH;
		}

		if (timed) {
			timed = FALSE;
			/* Leave half-delay mode. */
			raw();

			if (input == ERR) {
				if (spotlighted || ISSET(ZERO) || LINES == 1) {
					if (ISSET(ZERO) && lastmessage > VACUUM)
						wredrawln(midwin, editwinrows - 1, 1);
					lastmessage = VACUUM;
					spotlighted = FALSE;
					update_line(openfile->current, openfile->current_x);
					wnoutrefresh(midwin);
					curs_set(1);
				}
				if (ISSET(MINIBAR) && !ISSET(ZERO) && LINES > 1)
					minibar();
				as_an_at = TRUE;
				place_the_cursor();
				doupdate();
				continue;
			}
		}
#endif
		/* When we've failed to get a keycode millions of times in a row,
		 * assume our input source is gone and die gracefully.  We could
		 * check if errno is set to EIO ("Input/output error") and die in
		 * that case, but it's not always set properly.  Argh. */
		if (input == ERR && ++errcount == 12345678)
			die(_("Too many errors from stdin\n"));
	}

	curs_set(0);

	/* When there is no keystroke buffer yet, allocate one. */
	if (!key_buffer)
		reserve_space_for(capacity);

	key_buffer[0] = input;

	nextcodes = key_buffer;
	waiting_codes = 1;

#ifndef NANO_TINY
	/* Cancel the highlighting of a search match, if there still is one. */
	if (currmenu == MMAIN) {
		refresh_needed |= spotlighted;
		spotlighted = FALSE;
	}

	/* If we got a SIGWINCH, get out as the frame argument is no longer valid. */
	if (input == KEY_WINCH)
		return;
#endif

	/* Read in any remaining key codes using non-blocking input. */
	nodelay(frame, TRUE);

	/* After an ESC, when ncurses does not translate escape sequences,
	 * give the keyboard some time to bring the next code to ncurses. */
	if (input == ESC_CODE && (linger_after_escape || ISSET(RAW_SEQUENCES)))
		napms(20);

	while (TRUE) {
#ifndef NANO_TINY
		if (recording)
			add_to_macrobuffer(input);
#endif
		input = wgetch(frame);

		/* If there aren't any more characters, stop reading. */
		if (input == ERR)
			break;

		/* When the keystroke buffer is full, extend it. */
		if (waiting_codes == capacity)
			reserve_space_for(2 * capacity);

		key_buffer[waiting_codes++] = input;
	}

	/* Restore blocking-input mode. */
	nodelay(frame, FALSE);

#ifdef DEBUG
	fprintf(stderr, "\nSequence of hex codes:");
	for (size_t i = 0; i < waiting_codes; i++)
		fprintf(stderr, " %3x", key_buffer[i]);
	fprintf(stderr, "\n");
#endif
}

/* Return the number of key codes waiting in the keystroke buffer. */
size_t waiting_keycodes(void)
{
	return waiting_codes;
}

/* Add the given keycode to the front of the keystroke buffer. */
void put_back(int keycode)
{
	/* If there is no room at the head of the keystroke buffer, make room. */
	if (nextcodes == key_buffer) {
		if (waiting_codes == capacity)
			reserve_space_for(2 * capacity);
		memmove(key_buffer + 1, key_buffer, waiting_codes * sizeof(int));
	} else
		nextcodes--;

	*nextcodes = keycode;
	waiting_codes++;
}

#ifdef ENABLE_NANORC
/* Set up the given expansion string to be ingested by the keyboard routines. */
void implant(const char *string)
{
	plants_pointer = string;
	put_back(MORE_PLANTS);

	mute_modifiers = TRUE;
}

/* Continue processing an expansion string.  Returns either an error code,
 * a plain keycode, or a placeholder for a command shortcut. */
int get_code_from_plantation(void)
{
	if (*plants_pointer == '{') {
		char *closing = strchr(plants_pointer + 1, '}');

		if (!closing)
			return MISSING_BRACE;

		if (plants_pointer[1] == '{' && plants_pointer[2] == '}') {
			plants_pointer += 3;
			if (*plants_pointer != '\0')
				put_back(MORE_PLANTS);
			return '{';
		}

		free(commandname);
		free(planted_shortcut);

		commandname = measured_copy(plants_pointer + 1, closing - plants_pointer - 1);
		planted_shortcut = strtosc(commandname);

		if (!planted_shortcut)
			return NO_SUCH_FUNCTION;

		plants_pointer = closing + 1;

		if (*plants_pointer != '\0')
			put_back(MORE_PLANTS);

		return PLANTED_COMMAND;
	} else {
		char *opening = strchr(plants_pointer, '{');
		int length;

		if (opening) {
			length = opening - plants_pointer;
			put_back(MORE_PLANTS);
		} else
			length = strlen(plants_pointer);

		for (int index = length - 1; index >= 0; index--)
			put_back((unsigned char)plants_pointer[index]);

		plants_pointer += length;

		return ERR;
	}
}
#endif

/* Return one code from the keystroke buffer.  If the buffer is empty
 * but frame is given, first read more codes from the keyboard. */
int get_input(WINDOW *frame)
{
	if (waiting_codes)
		spotlighted = FALSE;
	else if (frame)
		read_keys_from(frame);

	if (waiting_codes > 0) {
		waiting_codes--;
#ifdef ENABLE_NANORC
		if (*nextcodes == MORE_PLANTS) {
			nextcodes++;
			return get_code_from_plantation();
		} else
#endif
			return *(nextcodes++);
	} else
		return ERR;
}

/* Return the arrow-key code that corresponds to the given letter.
 * (This mapping is common to a handful of escape sequences.) */
int arrow_from_ABCD(int letter)
{
	if (letter < 'C')
		return (letter == 'A' ? KEY_UP : KEY_DOWN);
	else
		return (letter == 'D' ? KEY_LEFT : KEY_RIGHT);
}

/* Translate a sequence that began with "Esc O" to its corresponding key code. */
int convert_SS3_sequence(const int *seq, size_t length, int *consumed)
{
	switch (seq[0]) {
		case '1':
			if (length > 3  && seq[1] == ';') {
				*consumed = 4;
#ifndef NANO_TINY
				switch (seq[2]) {
					case '2':
						if ('A' <= seq[3] && seq[3] <= 'D') {
							/* Esc O 1 ; 2 A == Shift-Up on old Terminal. */
							/* Esc O 1 ; 2 B == Shift-Down on old Terminal. */
							/* Esc O 1 ; 2 C == Shift-Right on old Terminal. */
							/* Esc O 1 ; 2 D == Shift-Left on old Terminal. */
							shift_held = TRUE;
							return arrow_from_ABCD(seq[3]);
						}
						break;
					case '5':
						switch (seq[3]) {
							case 'A': /* Esc O 1 ; 5 A == Ctrl-Up on old Terminal. */
								return CONTROL_UP;
							case 'B': /* Esc O 1 ; 5 B == Ctrl-Down on old Terminal. */
								return CONTROL_DOWN;
							case 'C': /* Esc O 1 ; 5 C == Ctrl-Right on old Terminal. */
								return CONTROL_RIGHT;
							case 'D': /* Esc O 1 ; 5 D == Ctrl-Left on old Terminal. */
								return CONTROL_LEFT;
						}
						break;
				}
#endif
			}
			break;
		case '2':  /* Shift */
		case '3':  /* Alt */
		case '4':  /* Shift+Alt */
		case '5':  /* Ctrl */
		case '6':  /* Shift+Ctrl */
		case '7':  /* Alt+Ctrl */
		case '8':  /* Shift+Alt+Ctrl */
			if (length > 1) {
				*consumed = 2;
				/* Do not accept multiple modifiers. */
				if (seq[0] == '4' || seq[0] > '5')
					return FOREIGN_SEQUENCE;
#ifndef NANO_TINY
				switch (seq[1]) {
					case 'A': /* Esc O 5 A == Ctrl-Up on Haiku. */
						return CONTROL_UP;
					case 'B': /* Esc O 5 B == Ctrl-Down on Haiku. */
						return CONTROL_DOWN;
					case 'C': /* Esc O 5 C == Ctrl-Right on Haiku. */
						return CONTROL_RIGHT;
					case 'D': /* Esc O 5 D == Ctrl-Left on Haiku. */
						return CONTROL_LEFT;
				}
#endif
				/* Translate Shift+digit on the keypad to the digit
				 * (Esc O 2 p == Shift-0, ...), modifier+operator to
				 * the operator, and modifier+Enter to CR. */
				return (seq[1] - 0x40);
			}
			break;
		case 'A': /* Esc O A == Up on VT100/VT320. */
		case 'B': /* Esc O B == Down on VT100/VT320. */
		case 'C': /* Esc O C == Right on VT100/VT320. */
		case 'D': /* Esc O D == Left on VT100/VT320. */
			return arrow_from_ABCD(seq[0]);
#ifndef NANO_TINY
		case 'F': /* Esc O F == End on old xterm. */
			return KEY_END;
		case 'H': /* Esc O H == Home on old xterm. */
			return KEY_HOME;
		case 'M': /* Esc O M == Enter on numeric keypad
				   * with NumLock off on VT100/VT220/VT320. */
			return KEY_ENTER;
#endif
		case 'P': /* Esc O P == F1 on VT100/VT220/VT320/xterm/Mach console. */
		case 'Q': /* Esc O Q == F2 on VT100/VT220/VT320/xterm/Mach console. */
		case 'R': /* Esc O R == F3 on VT100/VT220/VT320/xterm/Mach console. */
		case 'S': /* Esc O S == F4 on VT100/VT220/VT320/xterm/Mach console. */
			return KEY_F(seq[0] - 'O');
#ifndef NANO_TINY
		case 'T': /* Esc O T == F5 on Mach console. */
		case 'U': /* Esc O U == F6 on Mach console. */
		case 'V': /* Esc O V == F7 on Mach console. */
		case 'W': /* Esc O W == F8 on Mach console. */
		case 'X': /* Esc O X == F9 on Mach console. */
		case 'Y': /* Esc O Y == F10 on Mach console. */
			return KEY_F(seq[0] - 'O');
		case 'a': /* Esc O a == Ctrl-Up on rxvt/Eterm. */
			return CONTROL_UP;
		case 'b': /* Esc O b == Ctrl-Down on rxvt/Eterm. */
			return CONTROL_DOWN;
		case 'c': /* Esc O c == Ctrl-Right on rxvt/Eterm. */
			return CONTROL_RIGHT;
		case 'd': /* Esc O d == Ctrl-Left on rxvt/Eterm. */
			return CONTROL_LEFT;
		case 'j': /* Esc O j == '*' on numeric keypad with
				   * NumLock off on xterm/rxvt/Eterm. */
			return '*';
		case 'k': /* Esc O k == '+' on the same. */
			return '+';
		case 'l': /* Esc O l == ',' on VT100/VT220/VT320. */
			return ',';
		case 'm': /* Esc O m == '-' on numeric keypad with
				   * NumLock off on VTnnn/xterm/rxvt/Eterm. */
			return '-';
		case 'n': /* Esc O n == Delete (.) on numeric keypad
				   * with NumLock off on rxvt/Eterm. */
			return KEY_DC;
		case 'o': /* Esc O o == '/' on numeric keypad with
				   * NumLock off on VTnnn/xterm/rxvt/Eterm. */
			return '/';
		case 'p': /* Esc O p == Insert (0) on numeric keypad
				   * with NumLock off on rxvt/Eterm. */
			return KEY_IC;
		case 'q': /* Esc O q == End (1) on the same. */
			return KEY_END;
		case 'r': /* Esc O r == Down (2) on the same. */
			return KEY_DOWN;
		case 's': /* Esc O s == PageDown (3) on the same. */
			return KEY_NPAGE;
		case 't': /* Esc O t == Left (4) on the same. */
			return KEY_LEFT;
		case 'v': /* Esc O v == Right (6) on the same. */
			return KEY_RIGHT;
		case 'w': /* Esc O w == Home (7) on the same. */
			return KEY_HOME;
		case 'x': /* Esc O x == Up (8) on the same. */
			return KEY_UP;
		case 'y': /* Esc O y == PageUp (9) on the same. */
			return KEY_PPAGE;
#endif
	}

	return FOREIGN_SEQUENCE;
}

/* Translate a sequence that began with "Esc [" to its corresponding key code. */
int convert_CSI_sequence(const int *seq, size_t length, int *consumed)
{
	if (seq[0] < '9' && length > 1)
		*consumed = 2;

	switch (seq[0]) {
		case '1':
			if (length > 1 && seq[1] == '~')
				/* Esc [ 1 ~ == Home on VT320/Linux console. */
				return KEY_HOME;
			else if (length > 2 && seq[2] == '~') {
				*consumed = 3;
				switch (seq[1]) {
#ifndef NANO_TINY
					case '1': /* Esc [ 1 1 ~ == F1 on rxvt/Eterm. */
					case '2': /* Esc [ 1 2 ~ == F2 on rxvt/Eterm. */
					case '3': /* Esc [ 1 3 ~ == F3 on rxvt/Eterm. */
					case '4': /* Esc [ 1 4 ~ == F4 on rxvt/Eterm. */
#endif
					case '5': /* Esc [ 1 5 ~ == F5 on xterm/rxvt/Eterm. */
						return KEY_F(seq[1] - '0');
					case '7': /* Esc [ 1 7 ~ == F6 on VT220/VT320/
							   * Linux console/xterm/rxvt/Eterm. */
					case '8': /* Esc [ 1 8 ~ == F7 on the same. */
					case '9': /* Esc [ 1 9 ~ == F8 on the same. */
						return KEY_F(seq[1] - '1');
				}
			} else if (length > 3 && seq[1] == ';') {
				*consumed = 4;
#ifndef NANO_TINY
				switch (seq[2]) {
					case '2':
						switch (seq[3]) {
							case 'A': /* Esc [ 1 ; 2 A == Shift-Up on xterm. */
							case 'B': /* Esc [ 1 ; 2 B == Shift-Down on xterm. */
							case 'C': /* Esc [ 1 ; 2 C == Shift-Right on xterm. */
							case 'D': /* Esc [ 1 ; 2 D == Shift-Left on xterm. */
								shift_held = TRUE;
								return arrow_from_ABCD(seq[3]);
							case 'F': /* Esc [ 1 ; 2 F == Shift-End on xterm. */
								return SHIFT_END;
							case 'H': /* Esc [ 1 ; 2 H == Shift-Home on xterm. */
								return SHIFT_HOME;
						}
						break;
					case '9': /* To accommodate iTerm2 in "xterm mode". */
					case '3':
						switch (seq[3]) {
							case 'A': /* Esc [ 1 ; 3 A == Alt-Up on xterm. */
								return ALT_UP;
							case 'B': /* Esc [ 1 ; 3 B == Alt-Down on xterm. */
								return ALT_DOWN;
							case 'C': /* Esc [ 1 ; 3 C == Alt-Right on xterm. */
								return ALT_RIGHT;
							case 'D': /* Esc [ 1 ; 3 D == Alt-Left on xterm. */
								return ALT_LEFT;
						}
						break;
					case '4':
						/* When the arrow keys are held together with Shift+Meta,
						 * act as if they are Home/End/PgUp/PgDown with Shift. */
						switch (seq[3]) {
							case 'A': /* Esc [ 1 ; 4 A == Shift-Alt-Up on xterm. */
								return SHIFT_PAGEUP;
							case 'B': /* Esc [ 1 ; 4 B == Shift-Alt-Down on xterm. */
								return SHIFT_PAGEDOWN;
							case 'C': /* Esc [ 1 ; 4 C == Shift-Alt-Right on xterm. */
								return SHIFT_END;
							case 'D': /* Esc [ 1 ; 4 D == Shift-Alt-Left on xterm. */
								return SHIFT_HOME;
						}
						break;
					case '5':
						switch (seq[3]) {
							case 'A': /* Esc [ 1 ; 5 A == Ctrl-Up on xterm. */
								return CONTROL_UP;
							case 'B': /* Esc [ 1 ; 5 B == Ctrl-Down on xterm. */
								return CONTROL_DOWN;
							case 'C': /* Esc [ 1 ; 5 C == Ctrl-Right on xterm. */
								return CONTROL_RIGHT;
							case 'D': /* Esc [ 1 ; 5 D == Ctrl-Left on xterm. */
								return CONTROL_LEFT;
							case 'F': /* Esc [ 1 ; 5 F == Ctrl-End on xterm. */
								return CONTROL_END;
							case 'H': /* Esc [ 1 ; 5 H == Ctrl-Home on xterm. */
								return CONTROL_HOME;
						}
						break;
					case '6':
						switch (seq[3]) {
							case 'A': /* Esc [ 1 ; 6 A == Shift-Ctrl-Up on xterm. */
								return shiftcontrolup;
							case 'B': /* Esc [ 1 ; 6 B == Shift-Ctrl-Down on xterm. */
								return shiftcontroldown;
							case 'C': /* Esc [ 1 ; 6 C == Shift-Ctrl-Right on xterm. */
								return shiftcontrolright;
							case 'D': /* Esc [ 1 ; 6 D == Shift-Ctrl-Left on xterm. */
								return shiftcontrolleft;
							case 'F': /* Esc [ 1 ; 6 F == Shift-Ctrl-End on xterm. */
								return shiftcontrolend;
							case 'H': /* Esc [ 1 ; 6 H == Shift-Ctrl-Home on xterm. */
								return shiftcontrolhome;
						}
						break;
				}
#endif /* !NANO-TINY */
			} else if (length > 4 && seq[2] == ';' && seq[4] == '~')
				/* Esc [ 1 n ; 2 ~ == F17...F20 on some terminals. */
				*consumed = 5;
			break;
		case '2':
			if (length > 2 && seq[2] == '~') {
				*consumed = 3;
				switch (seq[1]) {
					case '0': /* Esc [ 2 0 ~ == F9 on VT220/VT320/
							   * Linux console/xterm/rxvt/Eterm. */
						return KEY_F(9);
					case '1': /* Esc [ 2 1 ~ == F10 on the same. */
						return KEY_F(10);
					case '3': /* Esc [ 2 3 ~ == F11 on the same. */
						return KEY_F(11);
					case '4': /* Esc [ 2 4 ~ == F12 on the same. */
						return KEY_F(12);
				}
			} else if (length > 1 && seq[1] == '~')
				/* Esc [ 2 ~ == Insert on VT220/VT320/
				 * Linux console/xterm/Terminal. */
				return KEY_IC;
			else if (length > 3 && seq[1] == ';' && seq[3] == '~') {
				/* Esc [ 2 ; x ~ == modified Insert on xterm. */
				*consumed = 4;
#ifndef NANO_TINY
				if (seq[2] == '3')
					return ALT_INSERT;
#endif
			}
			else if (length > 4 && seq[2] == ';' && seq[4] == '~')
				/* Esc [ 2 n ; 2 ~ == F21...F24 on some terminals. */
				*consumed = 5;
#ifndef NANO_TINY
			else if (length > 3 && seq[1] == '0' && seq[3] == '~') {
				/* Esc [ 2 0 0 ~ == start of a bracketed paste,
				 * Esc [ 2 0 1 ~ == end of a bracketed paste. */
				*consumed = 4;
				if (seq[2] == '0') {
					bracketed_paste = TRUE;
					return BRACKETED_PASTE_MARKER;
				} else if (seq[2] == '1') {
					bracketed_paste = FALSE;
					return BRACKETED_PASTE_MARKER;
				}
			}
#endif
			break;
		case '3': /* Esc [ 3 ~ == Delete on VT220/VT320/
				   * Linux console/xterm/Terminal. */
			if (length > 1 && seq[1] == '~')
				return KEY_DC;
			if (length > 3 && seq[1] == ';' && seq[3] == '~') {
				*consumed = 4;
#ifndef NANO_TINY
				if (seq[2] == '2')
					/* Esc [ 3 ; 2 ~ == Shift-Delete on xterm/Terminal. */
					return SHIFT_DELETE;
				if (seq[2] == '3')
					/* Esc [ 3 ; 3 ~ == Alt-Delete on xterm/rxvt/Eterm/Terminal. */
					return ALT_DELETE;
				if (seq[2] == '5')
					/* Esc [ 3 ; 5 ~ == Ctrl-Delete on xterm. */
					return CONTROL_DELETE;
				if (seq[2] == '6')
					/* Esc [ 3 ; 6 ~ == Ctrl-Shift-Delete on xterm. */
					return controlshiftdelete;
#endif
			}
#ifndef NANO_TINY
			if (length > 1 && seq[1] == '$')
				/* Esc [ 3 $ == Shift-Delete on urxvt. */
				return SHIFT_DELETE;
			if (length > 1 && seq[1] == '^')
				/* Esc [ 3 ^ == Ctrl-Delete on urxvt. */
				return CONTROL_DELETE;
			if (length > 1 && seq[1] == '@')
				/* Esc [ 3 @ == Ctrl-Shift-Delete on urxvt. */
				return controlshiftdelete;
			if (length > 2 && seq[2] == '~')
				/* Esc [ 3 n ~ == F17...F20 on some terminals. */
				*consumed = 3;
#endif
			break;
		case '4': /* Esc [ 4 ~ == End on VT220/VT320/
				   * Linux console/xterm. */
			if (length > 1 && seq[1] == '~')
				return KEY_END;
			break;
		case '5': /* Esc [ 5 ~ == PageUp on VT220/VT320/
				   * Linux console/xterm/Eterm/urxvt/Terminal */
			if (length > 1 && seq[1] == '~')
				return KEY_PPAGE;
			else if (length > 3 && seq[1] == ';' && seq[3] == '~') {
				*consumed = 4;
#ifndef NANO_TINY
				if (seq[2] == '2')
					return shiftaltup;
				if (seq[2] == '3')
					return ALT_PAGEUP;
#endif
			}
			break;
		case '6': /* Esc [ 6 ~ == PageDown on VT220/VT320/
				   * Linux console/xterm/Eterm/urxvt/Terminal */
			if (length > 1 && seq[1] == '~')
				return KEY_NPAGE;
			else if (length > 3 && seq[1] == ';' && seq[3] == '~') {
				*consumed = 4;
#ifndef NANO_TINY
				if (seq[2] == '2')
					return shiftaltdown;
				if (seq[2] == '3')
					return ALT_PAGEDOWN;
#endif
			}
			break;
		case '7': /* Esc [ 7 ~ == Home on Eterm/rxvt;
				   * Esc [ 7 $ == Shift-Home on Eterm/rxvt;
				   * Esc [ 7 ^ == Control-Home on Eterm/rxvt;
				   * Esc [ 7 @ == Shift-Control-Home on same. */
			if (length > 1 && seq[1] == '~')
				return KEY_HOME;
			else if (length > 1 && seq[1] == '$')
				return SHIFT_HOME;
			else if (length > 1 && seq[1] == '^')
				return CONTROL_HOME;
#ifndef NANO_TINY
			else if (length > 1 && seq[1] == '@')
				return shiftcontrolhome;
#endif
			break;
		case '8': /* Esc [ 8 ~ == End on Eterm/rxvt;
				   * Esc [ 8 $ == Shift-End on Eterm/rxvt;
				   * Esc [ 8 ^ == Control-End on Eterm/rxvt;
				   * Esc [ 8 @ == Shift-Control-End on same. */
			if (length > 1 && seq[1] == '~')
				return KEY_END;
			else if (length > 1 && seq[1] == '$')
				return SHIFT_END;
			else if (length > 1 && seq[1] == '^')
				return CONTROL_END;
#ifndef NANO_TINY
			else if (length > 1 && seq[1] == '@')
				return shiftcontrolend;
#endif
			break;
		case '9': /* Esc [ 9 == Delete on Mach console. */
			return KEY_DC;
		case '@': /* Esc [ @ == Insert on Mach console. */
			return KEY_IC;
		case 'A': /* Esc [ A == Up on ANSI/VT220/Linux console/
				   * FreeBSD console/Mach console/xterm/Eterm/
				   * urxvt/Gnome and Xfce Terminal. */
		case 'B': /* Esc [ B == Down on the same. */
		case 'C': /* Esc [ C == Right on the same. */
		case 'D': /* Esc [ D == Left on the same. */
			return arrow_from_ABCD(seq[0]);
		case 'F': /* Esc [ F == End on FreeBSD console/Eterm. */
			return KEY_END;
		case 'G': /* Esc [ G == PageDown on FreeBSD console. */
			return KEY_NPAGE;
		case 'H': /* Esc [ H == Home on ANSI/VT220/FreeBSD
				   * console/Mach console/Eterm. */
			return KEY_HOME;
		case 'I': /* Esc [ I == PageUp on FreeBSD console. */
			return KEY_PPAGE;
		case 'L': /* Esc [ L == Insert on ANSI/FreeBSD console. */
			return KEY_IC;
#ifndef NANO_TINY
		case 'M': /* Esc [ M == F1 on FreeBSD console. */
		case 'N': /* Esc [ N == F2 on FreeBSD console. */
		case 'O': /* Esc [ O == F3 on FreeBSD console. */
		case 'P': /* Esc [ P == F4 on FreeBSD console. */
		case 'Q': /* Esc [ Q == F5 on FreeBSD console. */
		case 'R': /* Esc [ R == F6 on FreeBSD console. */
		case 'S': /* Esc [ S == F7 on FreeBSD console. */
		case 'T': /* Esc [ T == F8 on FreeBSD console. */
			return KEY_F(seq[0] - 'L');
#endif
		case 'U': /* Esc [ U == PageDown on Mach console. */
			return KEY_NPAGE;
		case 'V': /* Esc [ V == PageUp on Mach console. */
			return KEY_PPAGE;
#ifndef NANO_TINY
		case 'W': /* Esc [ W == F11 on FreeBSD console. */
			return KEY_F(11);
		case 'X': /* Esc [ X == F12 on FreeBSD console. */
			return KEY_F(12);
#endif
		case 'Y': /* Esc [ Y == End on Mach console. */
			return KEY_END;
		case 'Z': /* Esc [ Z == Shift-Tab on ANSI/Linux console/
				   * FreeBSD console/xterm/rxvt/Terminal. */
			return SHIFT_TAB;
#ifndef NANO_TINY
		case 'a': /* Esc [ a == Shift-Up on rxvt/Eterm. */
		case 'b': /* Esc [ b == Shift-Down on rxvt/Eterm. */
		case 'c': /* Esc [ c == Shift-Right on rxvt/Eterm. */
		case 'd': /* Esc [ d == Shift-Left on rxvt/Eterm. */
			shift_held = TRUE;
			return arrow_from_ABCD(seq[0] - 0x20);
#endif
		case '[':
			if (length > 1) {
				*consumed = 2;
				if ('@' < seq[1] && seq[1] < 'F')
					/* Esc [ [ A == F1 on Linux console. */
					/* Esc [ [ B == F2 on Linux console. */
					/* Esc [ [ C == F3 on Linux console. */
					/* Esc [ [ D == F4 on Linux console. */
					/* Esc [ [ E == F5 on Linux console. */
					return KEY_F(seq[1] - '@');
			}
			break;
	}

	return FOREIGN_SEQUENCE;
}

/* Interpret an escape sequence that has the given post-ESC starter byte
 * and with the rest of the sequence still in the keystroke buffer. */
int parse_escape_sequence(int starter)
{
	int consumed = 1;
	int keycode = 0;

	if (starter == 'O')
		keycode = convert_SS3_sequence(nextcodes, waiting_codes, &consumed);
	else if (starter == '[')
		keycode = convert_CSI_sequence(nextcodes, waiting_codes, &consumed);

	/* Skip the consumed sequence elements. */
	waiting_codes -= consumed;
	nextcodes += consumed;

	return keycode;
}

#define PROCEED  -44

/* For each consecutive call, gather the given digit into a three-digit
 * decimal byte code (from 000 to 255).  Return the assembled code when
 * it is complete, but until then return PROCEED when the given digit is
 * valid, and the given digit itself otherwise. */
int assemble_byte_code(int keycode)
{
	static int byte = 0;

	digit_count++;

	/* The first digit is either 0, 1, or 2 (checked before the call). */
	if (digit_count == 1) {
		byte = (keycode - '0') * 100;
		return PROCEED;
	}

	/* The second digit may be at most 5 if the first was 2. */
	if (digit_count == 2) {
		if (byte < 200 || keycode <= '5') {
			byte += (keycode - '0') * 10;
			return PROCEED;
		} else
			return keycode;
	}

	/* The third digit may be at most 5 if the first two were 2 and 5. */
	if (byte < 250 || keycode <= '5')
		return (byte + keycode - '0');
	else
		return keycode;
}

/* Translate a normal ASCII character into its corresponding control code.
 * The following groups of control keystrokes are equivalent:
 *   Ctrl-2 == Ctrl-@ == Ctrl-` == Ctrl-Space
 *   Ctrl-3 == Ctrl-[ == <Esc>
 *   Ctrl-4 == Ctrl-\ == Ctrl-|
 *   Ctrl-5 == Ctrl-]
 *   Ctrl-6 == Ctrl-^ == Ctrl-~
 *   Ctrl-7 == Ctrl-/ == Ctrl-_
 *   Ctrl-8 == Ctrl-? */
int convert_to_control(int kbinput)
{
	if ('@' <= kbinput && kbinput <= '_')
		return kbinput - '@';
	if ('`' <= kbinput && kbinput <= '~')
		return kbinput - '`';
	if ('3' <= kbinput && kbinput <= '7')
		return kbinput - 24;
	if (kbinput == '?' || kbinput == '8')
		return DEL_CODE;
	if (kbinput == ' ' || kbinput == '2')
		return 0;
	if (kbinput == '/')
		return 31;

	return kbinput;
}

/* Extract one keystroke from the input stream.  Translate escape sequences
 * and possibly keypad codes into their corresponding values.  Set meta_key
 * to TRUE when appropriate.  Supported keypad keystrokes are: the arrow keys,
 * Insert, Delete, Home, End, PageUp, PageDown, Enter, and Backspace (many of
 * them also when modified with Shift, Ctrl, Alt, Shift+Ctrl, or Shift+Alt),
 * the function keys (F1-F12), and the numeric keypad with NumLock off. */
int parse_kbinput(WINDOW *frame)
{
	static bool first_escape_was_alone = FALSE;
	static bool last_escape_was_alone = FALSE;
	static int escapes = 0;
	int keycode;

	meta_key = FALSE;
	shift_held = FALSE;

	/* Get one code from the input stream. */
	keycode = get_input(frame);

	/* For an Esc, remember whether the last two arrived by themselves.
	 * Then increment the counter, rolling around on three escapes. */
	if (keycode == ESC_CODE) {
		first_escape_was_alone = last_escape_was_alone;
		last_escape_was_alone = (waiting_codes == 0);
		if (digit_count > 0) {
			digit_count = 0;
			escapes = 1;
		} else if (++escapes > 2)
			escapes = (last_escape_was_alone ? 0 : 1);
		return ERR;
	} else if (keycode == ERR)
		return ERR;

	if (escapes == 0) {
		/* Most key codes in byte range cannot be special keys. */
		if (keycode < 0xFF && keycode != '\t' && keycode != DEL_CODE)
			return keycode;
	} else if (escapes == 1) {
		escapes = 0;
		/* Codes out of ASCII printable range cannot form an escape sequence. */
		if (keycode < 0x20 || 0x7E < keycode) {
			if (keycode == '\t')
				return SHIFT_TAB;
#ifndef NANO_TINY
			else if (keycode == KEY_BACKSPACE || keycode == '\b' ||
												keycode == DEL_CODE)
				return CONTROL_SHIFT_DELETE;
#endif
#ifdef ENABLE_UTF8
			else if (0xC0 <= keycode && keycode <= 0xFF && using_utf8()) {
				while (waiting_codes && 0x80 <= nextcodes[0] && nextcodes[0] <= 0xBF)
					get_input(NULL);
				return FOREIGN_SEQUENCE;
			}
#endif
			else if (keycode < 0x20 && !last_escape_was_alone)
				meta_key = TRUE;
		} else if (waiting_codes == 0 || nextcodes[0] == ESC_CODE ||
								(keycode != 'O' && keycode != '[')) {
			if (!shifted_metas)
				keycode = tolower(keycode);
			meta_key = TRUE;
		} else
			keycode = parse_escape_sequence(keycode);
	} else {
		escapes = 0;
		if (keycode == '[' && waiting_codes &&
						(('A' <= nextcodes[0] && nextcodes[0] <= 'D') ||
						('a' <= nextcodes[0] && nextcodes[0] <= 'd'))) {
			/* An iTerm2/Eterm/rxvt double-escape sequence: Esc Esc [ X
			 * for Option+arrow, or Esc Esc [ x for Shift+Alt+arrow. */
			switch (get_input(NULL)) {
				case 'A': return KEY_HOME;
				case 'B': return KEY_END;
				case 'C': return CONTROL_RIGHT;
				case 'D': return CONTROL_LEFT;
#ifndef NANO_TINY
				case 'a': shift_held = TRUE; return KEY_PPAGE;
				case 'b': shift_held = TRUE; return KEY_NPAGE;
				case 'c': shift_held = TRUE; return KEY_HOME;
				case 'd': shift_held = TRUE; return KEY_END;
#endif
			}
		} else if (waiting_codes && nextcodes[0] != ESC_CODE &&
								(keycode == '[' || keycode == 'O')) {
			keycode = parse_escape_sequence(keycode);
			meta_key = TRUE;
		} else if ('0' <= keycode && (keycode <= '2' ||
								(keycode <= '9' && digit_count > 0))) {
			/* Two escapes followed by one digit: byte sequence mode. */
			int byte = assemble_byte_code(keycode);

			/* If the decimal byte value is not yet complete, return nothing. */
			if (byte == PROCEED) {
				escapes = 2;
				return ERR;
			}
#ifdef ENABLE_UTF8
			else if (byte > 0x7F && using_utf8()) {
				/* Convert the code to the corresponding Unicode, and
				 * put the second byte back into the keyboard buffer. */
				if (byte < 0xC0) {
					put_back((unsigned char)byte);
					return 0xC2;
				} else {
					put_back((unsigned char)(byte - 0x40));
					return 0xC3;
				}
			}
#endif
			else if (byte == '\t' || byte == DEL_CODE)
				keycode = byte;
			else
				return byte;
		} else if (digit_count == 0) {
			/* If the first escape arrived alone but not the second, then it
			 * is a Meta keystroke; otherwise, it is an "Esc Esc control". */
			if (first_escape_was_alone && !last_escape_was_alone) {
				if (!shifted_metas)
					keycode = tolower(keycode);
				meta_key = TRUE;
			} else
				keycode = convert_to_control(keycode);
		}
	}

	if (keycode == controlleft)
		return CONTROL_LEFT;
	else if (keycode == controlright)
		return CONTROL_RIGHT;
	else if (keycode == controlup)
		return CONTROL_UP;
	else if (keycode == controldown)
		return CONTROL_DOWN;
	else if (keycode == controlhome)
		return CONTROL_HOME;
	else if (keycode == controlend)
		return CONTROL_END;
#ifndef NANO_TINY
	else if (keycode == controldelete)
		return CONTROL_DELETE;
	else if (keycode == controlshiftdelete)
		return CONTROL_SHIFT_DELETE;
	else if (keycode == shiftup) {
		shift_held = TRUE;
		return KEY_UP;
	} else if (keycode == shiftdown) {
		shift_held = TRUE;
		return KEY_DOWN;
	} else if (keycode == shiftcontrolleft) {
		shift_held = TRUE;
		return CONTROL_LEFT;
	} else if (keycode == shiftcontrolright) {
		shift_held = TRUE;
		return CONTROL_RIGHT;
	} else if (keycode == shiftcontrolup) {
		shift_held = TRUE;
		return CONTROL_UP;
	} else if (keycode == shiftcontroldown) {
		shift_held = TRUE;
		return CONTROL_DOWN;
	} else if (keycode == shiftcontrolhome) {
		shift_held = TRUE;
		return CONTROL_HOME;
	} else if (keycode == shiftcontrolend) {
		shift_held = TRUE;
		return CONTROL_END;
	} else if (keycode == altleft)
		return ALT_LEFT;
	else if (keycode == altright)
		return ALT_RIGHT;
	else if (keycode == altup)
		return ALT_UP;
	else if (keycode == altdown)
		return ALT_DOWN;
	else if (keycode == altpageup)
		return ALT_PAGEUP;
	else if (keycode == altpagedown)
		return ALT_PAGEDOWN;
	else if (keycode == altinsert)
		return ALT_INSERT;
	else if (keycode == altdelete)
		return ALT_DELETE;
	else if (keycode == shiftaltleft) {
		shift_held = TRUE;
		return KEY_HOME;
	} else if (keycode == shiftaltright) {
		shift_held = TRUE;
		return KEY_END;
	} else if (keycode == shiftaltup) {
		shift_held = TRUE;
		return KEY_PPAGE;
	} else if (keycode == shiftaltdown) {
		shift_held = TRUE;
		return KEY_NPAGE;
	} else if ((KEY_F0 + 24) < keycode && keycode < (KEY_F0 + 64))
		return FOREIGN_SEQUENCE;
#endif

#ifdef __linux__
	/* When not running under X, check for the bare arrow keys whether
	 * Shift/Ctrl/Alt are being held together with them. */
	unsigned char modifiers = 6;

	/* Modifiers are: Alt (8), Ctrl (4), Shift (1). */
	if (on_a_vt && !mute_modifiers && ioctl(0, TIOCLINUX, &modifiers) >= 0) {
#ifndef NANO_TINY
		/* Is Shift being held? */
		if (modifiers & 0x01) {
			if (keycode == '\t')
				return SHIFT_TAB;
			if (keycode == KEY_DC && modifiers == 0x01)
				return SHIFT_DELETE;
			if (keycode == KEY_DC && modifiers == 0x05)
				return CONTROL_SHIFT_DELETE;
			if (!meta_key)
				shift_held = TRUE;
		}
		/* Is Alt being held? */
		if (modifiers == 0x08) {
			switch (keycode) {
				case KEY_UP:    return ALT_UP;
				case KEY_DOWN:  return ALT_DOWN;
				case KEY_PPAGE: return ALT_PAGEUP;
				case KEY_NPAGE: return ALT_PAGEDOWN;
				case KEY_DC:    return ALT_DELETE;
				case KEY_IC:    return ALT_INSERT;
			}
		}
#endif
		/* Is Ctrl being held? */
		if (modifiers & 0x04) {
			switch (keycode) {
				case KEY_UP:    return CONTROL_UP;
				case KEY_DOWN:  return CONTROL_DOWN;
				case KEY_LEFT:  return CONTROL_LEFT;
				case KEY_RIGHT: return CONTROL_RIGHT;
				case KEY_HOME:  return CONTROL_HOME;
				case KEY_END:   return CONTROL_END;
				case KEY_DC:    return CONTROL_DELETE;
			}
		}
#ifndef NANO_TINY
		/* Are both Shift and Alt being held? */
		if ((modifiers & 0x09) == 0x09) {
			switch (keycode) {
				case KEY_UP:    return KEY_PPAGE;
				case KEY_DOWN:  return KEY_NPAGE;
				case KEY_LEFT:  return KEY_HOME;
				case KEY_RIGHT: return KEY_END;
			}
		}
#endif
	}
#endif /* __linux__ */

#ifndef NANO_TINY
	/* When <Tab> is pressed while the mark is on, do an indent. */
	if (keycode == '\t' && openfile->mark && currmenu == MMAIN &&
				!bracketed_paste && openfile->mark != openfile->current)
		return INDENT_KEY;
#endif

	switch (keycode) {
		case KEY_SLEFT:
			shift_held = TRUE;
			return KEY_LEFT;
		case KEY_SRIGHT:
			shift_held = TRUE;
			return KEY_RIGHT;
#ifdef KEY_SR
#ifdef KEY_SUP  /* Ncurses doesn't know Shift+Up. */
		case KEY_SUP:
#endif
		case KEY_SR:    /* Scroll backward, on Xfce4-terminal. */
			shift_held = TRUE;
			return KEY_UP;
#endif
#ifdef KEY_SF
#ifdef KEY_SDOWN  /* Ncurses doesn't know Shift+Down. */
		case KEY_SDOWN:
#endif
		case KEY_SF:    /* Scroll forward, on Xfce4-terminal. */
			shift_held = TRUE;
			return KEY_DOWN;
#endif
#ifdef KEY_SHOME  /* HP-UX 10-11 doesn't know Shift+Home. */
		case KEY_SHOME:
#endif
		case SHIFT_HOME:
			shift_held = TRUE;
		case KEY_A1:    /* Home (7) on keypad with NumLock off. */
			return KEY_HOME;
#ifdef KEY_SEND  /* HP-UX 10-11 doesn't know Shift+End. */
		case KEY_SEND:
#endif
		case SHIFT_END:
			shift_held = TRUE;
		case KEY_C1:    /* End (1) on keypad with NumLock off. */
			return KEY_END;
#ifdef KEY_EOL
		case KEY_EOL:    /* Ctrl+End on rxvt-unicode. */
			return CONTROL_END;
#endif
#ifndef NANO_TINY
#ifdef KEY_SPREVIOUS
		case KEY_SPREVIOUS:
#endif
		case SHIFT_PAGEUP:    /* Fake key, from Shift+Alt+Up. */
			shift_held = TRUE;
#endif
		case KEY_A3:    /* PageUp (9) on keypad with NumLock off. */
			return KEY_PPAGE;
#ifndef NANO_TINY
#ifdef KEY_SNEXT
		case KEY_SNEXT:
#endif
		case SHIFT_PAGEDOWN:    /* Fake key, from Shift+Alt+Down. */
			shift_held = TRUE;
#endif
		case KEY_C3:    /* PageDown (3) on keypad with NumLock off. */
			return KEY_NPAGE;
		/* When requested, swap meanings of keycodes for <Bsp> and <Del>. */
		case DEL_CODE:
		case KEY_BACKSPACE:
			return (ISSET(REBIND_DELETE) ? KEY_DC : KEY_BACKSPACE);
		case KEY_DC:
			return (ISSET(REBIND_DELETE) ? KEY_BACKSPACE : KEY_DC);
		case KEY_SDC:
			return SHIFT_DELETE;
		case KEY_SCANCEL:
			return KEY_CANCEL;
		case KEY_SSUSPEND:
		case KEY_SUSPEND:
			return 0x1A;    /* The ASCII code for Ctrl+Z. */
		case KEY_BTAB:
			return SHIFT_TAB;

		case KEY_SBEG:
		case KEY_BEG:
		case KEY_B2:    /* Center (5) on keypad with NumLock off. */
#ifdef PDCURSES
		case KEY_SHIFT_L:
		case KEY_SHIFT_R:
		case KEY_CONTROL_L:
		case KEY_CONTROL_R:
		case KEY_ALT_L:
		case KEY_ALT_R:
#endif
#ifdef KEY_RESIZE  /* SunOS 5.7-5.9 doesn't know KEY_RESIZE. */
		case KEY_RESIZE:
#endif
#ifndef NANO_TINY
		case KEY_FRESH:
#endif
			return ERR;    /* Ignore this keystroke. */
	}

	return keycode;
}

/* Read in a single keystroke, ignoring any that are invalid. */
int get_kbinput(WINDOW *frame, bool showcursor)
{
	int kbinput = ERR;

	reveal_cursor = showcursor;

	/* Extract one keystroke from the input stream. */
	while (kbinput == ERR)
		kbinput = parse_kbinput(frame);

	/* If we read from the edit window, blank the status bar if needed. */
	if (frame == midwin)
		check_statusblank();

	return kbinput;
}

#ifdef ENABLE_UTF8
#define INVALID_DIGIT  -77

/* For each consecutive call, gather the given symbol into a Unicode code point.
 * When it's complete (with six digits, or when Space or Enter is typed), return
 * the assembled code.  Until then, return PROCEED when the symbol is valid, or
 * an error code for anything other than hexadecimal, Space, and Enter. */
long assemble_unicode(int symbol)
{
	static long unicode = 0;
	static int digits = 0;
	long outcome = PROCEED;

	if ('0' <= symbol && symbol <= '9')
		unicode = (unicode << 4) + symbol - '0';
	else if ('a' <= (symbol | 0x20) && (symbol | 0x20) <= 'f')
		unicode = (unicode << 4) + (symbol | 0x20) - 'a' + 10;
	else if (symbol == '\r' || symbol == ' ')
		outcome = unicode;
	else
		outcome = INVALID_DIGIT;

	/* If also the sixth digit was a valid hexadecimal value, then the
	 * Unicode sequence is complete, so return it (when it's valid). */
	if (++digits == 6 && outcome == PROCEED)
		outcome = (unicode < 0x110000) ? unicode : INVALID_DIGIT;

	/* Show feedback only when editing, not when at a prompt. */
	if (outcome == PROCEED && currmenu == MMAIN) {
		char partial[7] = "      ";

		sprintf(partial + 6 - digits, "%0*lX", digits, unicode);

		/* TRANSLATORS: This is shown while a six-digit hexadecimal
		 * Unicode character code (%s) is being typed in. */
		statusline(INFO, _("Unicode Input: %s"), partial);
	}

	/* If we have an end result, reset the value and the counter. */
	if (outcome != PROCEED) {
		unicode = 0;
		digits = 0;
	}

	return outcome;
}
#endif /* ENABLE_UTF8 */

/* Read in one control character (or an iTerm/Eterm/rxvt double Escape),
 * or convert a series of six digits into a Unicode codepoint.  Return
 * in count either 1 (for a control character or the first byte of a
 * multibyte sequence), or 2 (for an iTerm/Eterm/rxvt double Escape). */
int *parse_verbatim_kbinput(WINDOW *frame, size_t *count)
{
	int keycode, *yield;

	reveal_cursor = TRUE;

	keycode = get_input(frame);

#ifndef NANO_TINY
	/* When the window was resized, abort and return nothing. */
	if (keycode == KEY_WINCH) {
		*count = 999;
		return NULL;
	}
#endif

	/* Reserve ample space for the possible result. */
	yield = nmalloc(6 * sizeof(int));

#ifdef ENABLE_UTF8
	/* If the key code is a hexadecimal digit, commence Unicode input. */
	if (using_utf8() && isxdigit(keycode)) {
		long unicode = assemble_unicode(keycode);
		char multibyte[MB_CUR_MAX];

		reveal_cursor = FALSE;

		/* Gather at most six hexadecimal digits. */
		while (unicode == PROCEED) {
			keycode = get_input(frame);
			unicode = assemble_unicode(keycode);
		}

#ifndef NANO_TINY
		if (keycode == KEY_WINCH) {
			*count = 999;
			free(yield);
			return NULL;
		}
#endif
		/* For an invalid keystroke, discard its possible continuation bytes. */
		if (unicode == INVALID_DIGIT) {
			if (keycode == ESC_CODE && waiting_codes) {
				get_input(NULL);
				while (waiting_codes && 0x1F < nextcodes[0] && nextcodes[0] < 0x40)
					get_input(NULL);
				if (waiting_codes && 0x3F < nextcodes[0] && nextcodes[0] < 0x7F)
					get_input(NULL);
			} else if (0xC0 <= keycode && keycode <= 0xFF)
				while (waiting_codes && 0x7F < nextcodes[0] && nextcodes[0] < 0xC0)
					get_input(NULL);
		}

		/* Convert the Unicode value to a multibyte sequence. */
		*count = wctomb(multibyte, unicode);

		if (*count > MAXCHARLEN)
			*count = 0;

		/* Change the multibyte character into a series of integers. */
		for (size_t i = 0; i < *count; i++)
			yield[i] = (int)multibyte[i];

		return yield;
	}
#endif /* ENABLE_UTF8 */

	yield[0] = keycode;

	/* In case of an escape, take also a second code, as it might be another
	 * escape (on iTerm2/rxvt) or a control code (for M-Bsp and M-Enter). */
	if (keycode == ESC_CODE && waiting_codes) {
		yield[1] = get_input(NULL);
		*count = 2;
	}

	return yield;
}

/* Read in one control code, one character byte, or the leading escapes of
 * an escape sequence, and return the resulting number of bytes in count. */
char *get_verbatim_kbinput(WINDOW *frame, size_t *count)
{
	char *bytes = nmalloc(MAXCHARLEN + 2);
	int *input;

	/* Turn off flow control characters if necessary so that we can type
	 * them in verbatim, and turn the keypad off if necessary so that we
	 * don't get extended keypad values. */
	if (ISSET(PRESERVE))
		disable_flow_control();
	if (!ISSET(RAW_SEQUENCES))
		keypad(frame, FALSE);

#ifndef NANO_TINY
	/* Turn bracketed-paste mode off. */
	printf("\x1B[?2004l");
	fflush(stdout);
#endif

	linger_after_escape = TRUE;

	/* Read in a single byte or two escapes. */
	input = parse_verbatim_kbinput(frame, count);

	/* If the byte is invalid in the current mode, discard it;
	 * if it is an incomplete Unicode sequence, stuff it back. */
	if (input != NULL) {
		if (*input >= 0x80 && *count == 1) {
			put_back(*input);
			*count = 999;
		} else if ((*input == '\n' && as_an_at) || (*input == '\0' && !as_an_at))
			*count = 0;
	}

	linger_after_escape = FALSE;

#ifndef NANO_TINY
	/* Turn bracketed-paste mode back on. */
	printf("\x1B[?2004h");
	fflush(stdout);
#endif

	/* Turn flow control characters back on if necessary and turn the
	 * keypad back on if necessary now that we're done. */
	if (ISSET(PRESERVE))
		enable_flow_control();

	/* Use the global window pointers, because a resize may have freed
	 * the data that the frame parameter points to. */
	if (!ISSET(RAW_SEQUENCES)) {
		keypad(midwin, TRUE);
		keypad(footwin, TRUE);
	}

	if (*count < 999) {
		for (size_t i = 0; i < *count; i++)
			bytes[i] = (char)input[i];
		bytes[*count] = '\0';
	}

	free(input);

	return bytes;
}

#ifdef ENABLE_MOUSE
/* Handle any mouse event that may have occurred.  We currently handle
 * releases/clicks of the first mouse button.  If allow_shortcuts is
 * TRUE, releasing/clicking on a visible shortcut will put back the
 * keystroke associated with that shortcut.  If ncurses supports them,
 * we also handle presses of the fourth mouse button (upward rolls of
 * the mouse wheel) by putting back keystrokes to move up, and presses
 * of the fifth mouse button (downward rolls of the mouse wheel) by
 * putting back keystrokes to move down.  We also store the coordinates
 * of a mouse event that needs further handling in mouse_x and mouse_y.
 * Return -1 on error, 0 if the mouse event needs to be handled, 1 if it's
 * been handled by putting back keystrokes, or 2 if it's been ignored. */
int get_mouseinput(int *mouse_y, int *mouse_x, bool allow_shortcuts)
{
	bool in_middle, in_footer;
	MEVENT event;

	/* First, get the actual mouse event. */
	if (getmouse(&event) == ERR)
		return -1;

	in_middle = wenclose(midwin, event.y, event.x);
	in_footer = wenclose(footwin, event.y, event.x);

	/* Copy (and possibly adjust) the coordinates of the mouse event. */
	*mouse_x = event.x - (in_middle ? margin : 0);
	*mouse_y = event.y;

	/* Handle releases/clicks of the first mouse button. */
	if (event.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
		/* If we're allowing shortcuts, and the current shortcut list is
		 * being displayed on the last two lines of the screen, and the
		 * first mouse button was released on/clicked inside it, we need
		 * to figure out which shortcut was released on/clicked and put
		 * back the equivalent keystroke(s) for it. */
		if (allow_shortcuts && !ISSET(NO_HELP) && in_footer) {
			int width;
				/* The width of each shortcut item, except the last two. */
			int index;
				/* The calculated index of the clicked item. */
			size_t number;
				/* The number of shortcut items that get displayed. */

			/* Shift the coordinates to be relative to the bottom window. */
			wmouse_trafo(footwin, mouse_y, mouse_x, FALSE);

			/* Clicks on the status bar are handled elsewhere, so
			 * restore the untranslated mouse-event coordinates. */
			if (*mouse_y == 0) {
				*mouse_x = event.x;
				*mouse_y = event.y;
				return 0;
			}

			/* Determine how many shortcuts are being shown. */
			number = shown_entries_for(currmenu);

			/* Calculate the clickable width of each menu item. */
			if (number < 5)
				width = COLS / 2;
			else
				width = COLS / ((number + 1) / 2);

			/* Calculate the one-based index in the shortcut list. */
			index = (*mouse_x / width) * 2 + *mouse_y;

			/* Adjust the index if we hit the last two wider ones. */
			if ((index > number) && (*mouse_x % width < COLS % width))
				index -= 2;

			/* Ignore clicks beyond the last shortcut. */
			if (index > number)
				return 2;

			/* Search through the list of functions to determine which
			 * shortcut in the current menu the user clicked on; then
			 * put the corresponding keystroke into the keyboard buffer. */
			for (funcstruct *f = allfuncs; f != NULL; f = f->next) {
				if ((f->menus & currmenu) == 0)
					continue;
				if (first_sc_for(currmenu, f->func) == NULL)
					continue;
				if (--index == 0) {
					const keystruct *shortcut = first_sc_for(currmenu, f->func);

					put_back(shortcut->keycode);
					if (0x20 <= shortcut->keycode && shortcut->keycode <= 0x7E)
						put_back(ESC_CODE);
					break;
				}
			}

			return 1;
		} else
			/* Clicks outside of the bottom window are handled elsewhere. */
			return 0;
	}
#if NCURSES_MOUSE_VERSION >= 2
	/* Handle "presses" of the fourth and fifth mouse buttons
	 * (upward and downward rolls of the mouse wheel). */
	else if (event.bstate & (BUTTON4_PRESSED | BUTTON5_PRESSED)) {
		if (in_footer)
			/* Shift the coordinates to be relative to the bottom window. */
			wmouse_trafo(footwin, mouse_y, mouse_x, FALSE);

		if (in_middle || (in_footer && *mouse_y == 0)) {
			int keycode = (event.bstate & BUTTON4_PRESSED) ? KEY_UP : KEY_DOWN;

			/* One roll of the mouse wheel should move three lines. */
			for (int count = 3; count > 0; count--)
				put_back(keycode);

			return 1;
		} else
			/* Ignore "presses" of the fourth and fifth mouse buttons
			 * that aren't on the edit window or the status bar. */
			return 2;
	}
#endif
	/* Ignore all other mouse events. */
	return 2;
}
#endif /* ENABLE_MOUSE */

/* Move (in the given window) to the given row and wipe it clean. */
void blank_row(WINDOW *window, int row)
{
	wmove(window, row, 0);
	wclrtoeol(window);
}

/* Blank the first line of the top portion of the screen. */
void blank_titlebar(void)
{
	mvwprintw(topwin, 0, 0, "%*s", COLS, " ");
}

/* Blank all lines of the middle portion of the screen (the edit window). */
void blank_edit(void)
{
	for (int row = 0; row < editwinrows; row++)
		blank_row(midwin, row);
}

/* Blank the first line of the bottom portion of the screen. */
void blank_statusbar(void)
{
	blank_row(footwin, 0);
}

/* Wipe the status bar clean and include this in the next screen update. */
void wipe_statusbar(void)
{
	lastmessage = VACUUM;

	if ((ISSET(ZERO) || ISSET(MINIBAR) || LINES == 1) && currmenu == MMAIN)
		return;

	blank_row(footwin, 0);
	wnoutrefresh(footwin);
}

/* Blank out the two help lines (when they are present). */
void blank_bottombars(void)
{
	if (!ISSET(NO_HELP) && LINES > 5) {
		blank_row(footwin, 1);
		blank_row(footwin, 2);
	}
}

/* When some number of keystrokes has been reached, wipe the status bar. */
void check_statusblank(void)
{
	if (statusblank == 0)
		return;

	if (--statusblank == 0)
		wipe_statusbar();

	/* When windows overlap, make sure to show the edit window now. */
	if (currmenu == MMAIN && (ISSET(ZERO) || LINES == 1)) {
		wredrawln(midwin, editwinrows - 1, 1);
		wnoutrefresh(midwin);
	}
}

/* Ensure that the status bar will be wiped upon the next keystroke. */
void set_blankdelay_to_one(void)
{
	statusblank = 1;
}

/* Convert text into a string that can be displayed on screen.  The caller
 * wants to display text starting with the given column, and extending for
 * at most span columns.  column is zero-based, and span is one-based, so
 * span == 0 means you get "" returned.  The returned string is dynamically
 * allocated, and should be freed.  If isdata is TRUE, the caller might put
 * "<" at the beginning or ">" at the end of the line if it's too long.  If
 * isprompt is TRUE, the caller might put ">" at the end of the line if it's
 * too long. */
char *display_string(const char *text, size_t column, size_t span,
						bool isdata, bool isprompt)
{
	const char *origin = text;
		/* The beginning of the text, to later determine the covered part. */
	size_t start_x = actual_x(text, column);
		/* The index of the first character that the caller wishes to show. */
	size_t start_col = wideness(text, start_x);
		/* The actual column where that first character starts. */
	size_t stowaways = 20;
		/* The number of zero-width characters for which to reserve space. */
	size_t allocsize = (COLS + stowaways) * MAXCHARLEN + 1;
		/* The amount of memory to reserve for the displayable string. */
	char *converted = nmalloc(allocsize);
		/* The displayable string we will return. */
	size_t index = 0;
		/* Current position in converted. */
	size_t beyond = column + span;
		/* The column number just beyond the last shown character. */

	text += start_x;

#ifndef NANO_TINY
	if (span > HIGHEST_POSITIVE) {
		statusline(ALERT, "Span has underflowed -- please report a bug");
		converted[0] = '\0';
		return converted;
	}
#endif
	/* If the first character starts before the left edge, or would be
	 * overwritten by a "<" token, then show placeholders instead. */
	if ((start_col < column || (start_col > 0 && isdata && !ISSET(SOFTWRAP))) &&
											*text != '\0' && *text != '\t') {
		if (is_cntrl_char(text)) {
			if (start_col < column) {
				converted[index++] = control_mbrep(text, isdata);
				column++;
				text += char_length(text);
			}
		}
#ifdef ENABLE_UTF8
		else if (is_doublewidth(text)) {
			if (start_col == column) {
				converted[index++] = ' ';
				column++;
			}

			/* Display the right half of a two-column character as ']'. */
			converted[index++] = ']';
			column++;
			text += char_length(text);
		}
#endif
	}

#ifdef ENABLE_UTF8
#define ISO8859_CHAR  FALSE
#define ZEROWIDTH_CHAR  (is_zerowidth(text))
#else
#define ISO8859_CHAR  ((unsigned char)*text > 0x9F)
#define ZEROWIDTH_CHAR  FALSE
#endif

	while (*text != '\0' && (column < beyond || ZEROWIDTH_CHAR)) {
		/* A plain printable ASCII character is one byte, one column. */
		if (((signed char)*text > 0x20 && *text != DEL_CODE) || ISO8859_CHAR) {
			converted[index++] = *(text++);
			column++;
			continue;
		}

		/* Show a space as a visible character, or as a space. */
		if (*text == ' ') {
#ifndef NANO_TINY
			if (ISSET(WHITESPACE_DISPLAY)) {
				for (int i = whitelen[0]; i < whitelen[0] + whitelen[1];)
					converted[index++] = whitespace[i++];
			} else
#endif
				converted[index++] = ' ';
			column++;
			text++;
			continue;
		}

		/* Show a tab as a visible character plus spaces, or as just spaces. */
		if (*text == '\t') {
#ifndef NANO_TINY
			if (ISSET(WHITESPACE_DISPLAY) && (index > 0 || !isdata ||
						!ISSET(SOFTWRAP) || column % tabsize == 0 ||
						column == start_col)) {
				for (int i = 0; i < whitelen[0];)
					converted[index++] = whitespace[i++];
			} else
#endif
				converted[index++] = ' ';
			column++;
			/* Fill the tab up with the required number of spaces. */
			while (column % tabsize != 0 && column < beyond) {
				converted[index++] = ' ';
				column++;
			}
			text++;
			continue;
		}

		/* Represent a control character with a leading caret. */
		if (is_cntrl_char(text)) {
			converted[index++] = '^';
			converted[index++] = control_mbrep(text, isdata);
			text += char_length(text);
			column += 2;
			continue;
		}

#ifdef ENABLE_UTF8
		int charlength, charwidth;
		wchar_t wc;

		/* Convert a multibyte character to a single code. */
		charlength = mbtowide(&wc, text);

		/* Represent an invalid character with the Replacement Character. */
		if (charlength < 0) {
			converted[index++] = '\xEF';
			converted[index++] = '\xBF';
			converted[index++] = '\xBD';
			text++;
			column++;
			continue;
		}

		/* Determine whether the character takes zero, one, or two columns. */
		charwidth = wcwidth(wc);

		/* Watch the number of zero-widths, to keep ample memory reserved. */
		if (charwidth == 0 && --stowaways == 0) {
			stowaways = 40;
			allocsize += stowaways * MAXCHARLEN;
			converted = nrealloc(converted, allocsize);
		}

#ifdef __linux__
		/* On a Linux console, skip zero-width characters, as it would show
		 * them WITH a width, thus messing up the display.  See bug #52954. */
		if (on_a_vt && charwidth == 0) {
			text += charlength;
			continue;
		}
#endif
		/* For any valid character, just copy its bytes. */
		for (; charlength > 0; charlength--)
			converted[index++] = *(text++);

		/* If the codepoint is unassigned, assume a width of one. */
		column += (charwidth < 0 ? 1 : charwidth);
#endif /* ENABLE_UTF8 */
	}

	/* If there is more text than can be shown, make room for the ">". */
	if (column > beyond || (*text != '\0' && (isprompt ||
							(isdata && !ISSET(SOFTWRAP))))) {
#ifdef ENABLE_UTF8
		do {
			index = step_left(converted, index);
		} while (is_zerowidth(converted + index));

		/* Display the left half of a two-column character as '['. */
		if (is_doublewidth(converted + index))
			converted[index++] = '[';
#else
		index--;
#endif
		has_more = TRUE;
	} else
		has_more = FALSE;

	is_shorter = (column < beyond);

	/* Null-terminate the converted string. */
	converted[index] = '\0';

	/* Remember what part of the original text is covered by converted. */
	from_x = start_x;
	till_x = text - origin;

	return converted;
}

#ifdef ENABLE_MULTIBUFFER
/* Determine the sequence number of the given buffer in the circular list. */
int buffer_number(openfilestruct *buffer)
{
	int count = 1;

	while (buffer != startfile) {
		buffer = buffer->prev;
		count++;
	}

	return count;
}
#endif

#ifndef NANO_TINY
/* Show the state of auto-indenting, the mark, hard-wrapping, macro recording,
 * and soft-wrapping by showing corresponding letters in the given window. */
void show_states_at(WINDOW *window)
{
	waddstr(window, ISSET(AUTOINDENT) ? "I" : " ");
	waddstr(window, openfile->mark ? "M" : " ");
	waddstr(window, ISSET(BREAK_LONG_LINES) ? "L" : " ");
	waddstr(window, recording ? "R" : " ");
	waddstr(window, ISSET(SOFTWRAP) ? "S" : " ");
}
#endif

/* If path is NULL, we're in normal editing mode, so display the current
 * version of nano, the current filename, and whether the current file
 * has been modified on the title bar.  If path isn't NULL, we're either
 * in the file browser or the help viewer, so show either the current
 * directory or the title of help text, that is: whatever is in path. */
void titlebar(const char *path)
{
	size_t verlen, prefixlen, pathlen, statelen;
		/* The width of the different title-bar elements, in columns. */
	size_t pluglen = 0;
		/* The width that "Modified" would take up. */
	size_t offset = 0;
		/* The position at which the center part of the title bar starts. */
	const char *upperleft = "";
		/* What is shown in the top left corner. */
	const char *prefix = "";
		/* What is shown before the path -- "DIR:" or nothing. */
	const char *state = "";
		/* The state of the current buffer -- "Modified", "View", or "". */
	char *caption;
		/* The presentable form of the pathname. */
	char *ranking = NULL;
		/* The buffer sequence number plus the total buffer count. */

	/* If the screen is too small, there is no title bar. */
	if (topwin == NULL)
		return;

	wattron(topwin, interface_color_pair[TITLE_BAR]);

	blank_titlebar();
	as_an_at = FALSE;

	/* Do as Pico: if there is not enough width available for all items,
	 * first sacrifice the version string, then eat up the side spaces,
	 * then sacrifice the prefix, and only then start dottifying. */

	/* Figure out the path, prefix and state strings. */
#ifdef ENABLE_COLOR
	if (currmenu == MLINTER) {
		/* TRANSLATORS: The next five are "labels" in the title bar. */
		prefix = _("Linting --");
		path = openfile->filename;
	} else
#endif
#ifdef ENABLE_BROWSER
	if (!inhelp && path != NULL)
		prefix = _("DIR:");
	else
#endif
	if (!inhelp) {
#ifdef ENABLE_MULTIBUFFER
		/* If there are/were multiple buffers, show which out of how many. */
		if (more_than_one) {
			ranking = nmalloc(24);
			sprintf(ranking, "[%i/%i]", buffer_number(openfile),
										buffer_number(startfile->prev));
			upperleft = ranking;
		} else
#endif
			upperleft = BRANDING;

		if (openfile->filename[0] == '\0')
			path = _("New Buffer");
		else
			path = openfile->filename;

		if (ISSET(VIEW_MODE))
			state = _("View");
#ifndef NANO_TINY
		else if (ISSET(STATEFLAGS))
			state = "+.xxxxx";
#endif
		else if (openfile->modified)
			state = _("Modified");
		else if (ISSET(RESTRICTED))
			state = _("Restricted");
		else
			pluglen = breadth(_("Modified")) + 1;
	}

	/* Determine the widths of the four elements, including their padding. */
	verlen = breadth(upperleft) + 3;
	prefixlen = breadth(prefix);
	if (prefixlen > 0)
		prefixlen++;
	pathlen = breadth(path);
	statelen = breadth(state) + 2;
	if (statelen > 2)
		pathlen++;

	/* Only print the version message when there is room for it. */
	if (verlen + prefixlen + pathlen + pluglen + statelen <= COLS)
		mvwaddstr(topwin, 0, 2, upperleft);
	else {
		verlen = 2;
		/* If things don't fit yet, give up the placeholder. */
		if (verlen + prefixlen + pathlen + pluglen + statelen > COLS)
			pluglen = 0;
		/* If things still don't fit, give up the side spaces. */
		if (verlen + prefixlen + pathlen + pluglen + statelen > COLS) {
			verlen = 0;
			statelen -= 2;
		}
	}

	free(ranking);

	/* If we have side spaces left, center the path name. */
	if (verlen > 0)
		offset = verlen + (COLS - (verlen + pluglen + statelen) -
										(prefixlen + pathlen)) / 2;

	/* Only print the prefix when there is room for it. */
	if (verlen + prefixlen + pathlen + pluglen + statelen <= COLS) {
		mvwaddstr(topwin, 0, offset, prefix);
		if (prefixlen > 0)
			waddstr(topwin, " ");
	} else
		wmove(topwin, 0, offset);

	/* Print the full path if there's room; otherwise, dottify it. */
	if (pathlen + pluglen + statelen <= COLS) {
		caption = display_string(path, 0, pathlen, FALSE, FALSE);
		waddstr(topwin, caption);
		free(caption);
	} else if (5 + statelen <= COLS) {
		waddstr(topwin, "...");
		caption = display_string(path, 3 + pathlen - COLS + statelen,
										COLS - statelen, FALSE, FALSE);
		waddstr(topwin, caption);
		free(caption);
	}

#ifndef NANO_TINY
	/* When requested, show on the title bar the state of three options and
	 * the state of the mark and whether a macro is being recorded. */
	if (*state && ISSET(STATEFLAGS) && !ISSET(VIEW_MODE)) {
		if (openfile->modified && COLS > 1)
			waddstr(topwin, " *");
		if (statelen < COLS) {
			wmove(topwin, 0, COLS + 2 - statelen);
			show_states_at(topwin);
		}
	} else
#endif
	{
		/* If there's room, right-align the state word; otherwise, clip it. */
		if (statelen > 0 && statelen <= COLS)
			mvwaddstr(topwin, 0, COLS - statelen, state);
		else if (statelen > 0)
			mvwaddnstr(topwin, 0, 0, state, actual_x(state, COLS));
	}

	wattroff(topwin, interface_color_pair[TITLE_BAR]);

	wrefresh(topwin);
}

#ifndef NANO_TINY
/* Draw a bar at the bottom with some minimal state information. */
void minibar(void)
{
	char *thename = NULL, *number_of_lines = NULL, *ranking = NULL;
	char *location = nmalloc(44);
	char *hexadecimal = nmalloc(9);
	char *successor = NULL;
	size_t namewidth, placewidth;
	size_t tallywidth = 0;
	size_t padding = 2;
#ifdef ENABLE_UTF8
	wchar_t widecode;
#endif

	/* Draw a colored bar over the full width of the screen. */
	wattron(footwin, interface_color_pair[MINI_INFOBAR]);
	mvwprintw(footwin, 0, 0, "%*s", COLS, " ");

	if (openfile->filename[0] != '\0') {
		as_an_at = FALSE;
		thename = display_string(openfile->filename, 0, COLS, FALSE, FALSE);
	} else
		thename = copy_of(_("(nameless)"));

	sprintf(location, "%zi,%zi", openfile->current->lineno, xplustabs() + 1);
	placewidth = strlen(location);
	namewidth = breadth(thename);

	/* If the file name is relatively long, drop the side spaces. */
	if (namewidth + 19 > COLS)
		padding = 0;

	/* Display the name of the current file (dottifying it if it doesn't fit),
	 * plus a star when the file has been modified. */
	if (COLS > 4) {
		if (namewidth > COLS - 2) {
			char *shortname = display_string(thename, namewidth - COLS + 5,
												COLS - 5, FALSE, FALSE);
			mvwaddstr(footwin, 0, 0, "...");
			waddstr(footwin, shortname);
			free(shortname);
		} else
			mvwaddstr(footwin, 0, padding, thename);

		waddstr(footwin, openfile->modified ? " *" : "  ");
	}

	/* Right after reading or writing a file, display its number of lines;
	 * otherwise, when there are multiple buffers, display an [x/n] counter. */
	if (report_size && COLS > 35) {
		size_t count = openfile->filebot->lineno - (openfile->filebot->data[0] == '\0');

		number_of_lines = nmalloc(44);
		sprintf(number_of_lines, P_(" (%zu line)", " (%zu lines)", count), count);
		tallywidth = breadth(number_of_lines);
		if (namewidth + tallywidth + 11 < COLS)
			waddstr(footwin, number_of_lines);
		else
			tallywidth = 0;
		report_size = FALSE;
	}
#ifdef ENABLE_MULTIBUFFER
	else if (openfile->next != openfile && COLS > 35) {
		ranking = nmalloc(24);
		sprintf(ranking, " [%i/%i]", buffer_number(openfile), buffer_number(startfile->prev));
		if (namewidth + placewidth + breadth(ranking) + 32 < COLS)
			waddstr(footwin, ranking);
	}
#endif

	/* Display the line/column position of the cursor. */
	if (ISSET(CONSTANT_SHOW) && namewidth + tallywidth + placewidth + 32 < COLS)
		mvwaddstr(footwin, 0, COLS - 27 - placewidth, location);

	/* Display the hexadecimal code of the character under the cursor,
	 * plus the codes of up to two succeeding zero-width characters. */
	if (ISSET(CONSTANT_SHOW) && namewidth + tallywidth + 28 < COLS) {
		char *this_position = openfile->current->data + openfile->current_x;

		if (*this_position == '\0')
			sprintf(hexadecimal, openfile->current->next ?
#ifdef ENABLE_UTF8
											using_utf8() ? "U+000A" :
#endif
											"  0x0A" : "  ----");
		else if (*this_position == '\n')
			sprintf(hexadecimal, "  0x00");
#ifdef ENABLE_UTF8
		else if ((unsigned char)*this_position < 0x80 && using_utf8())
			sprintf(hexadecimal, "U+%04X", (unsigned char)*this_position);
		else if (using_utf8() && mbtowide(&widecode, this_position) > 0)
			sprintf(hexadecimal, "U+%04X", (int)widecode);
#endif
		else
			sprintf(hexadecimal, "  0x%02X", (unsigned char)*this_position);

		mvwaddstr(footwin, 0, COLS - 23, hexadecimal);

#ifdef ENABLE_UTF8
		successor = this_position + char_length(this_position);

		if (*this_position && *successor && is_zerowidth(successor) &&
								mbtowide(&widecode, successor) > 0) {
			sprintf(hexadecimal, "|%04X", (int)widecode);
			waddstr(footwin, hexadecimal);

			successor += char_length(successor);

			if (is_zerowidth(successor) && mbtowide(&widecode, successor) > 0) {
				sprintf(hexadecimal, "|%04X", (int)widecode);
				waddstr(footwin, hexadecimal);
			}
		} else
			successor = NULL;
#endif
	}

	/* Display the state of three flags, and the state of macro and mark. */
	if (ISSET(STATEFLAGS) && !successor && namewidth + tallywidth + 14 + 2 * padding < COLS) {
		wmove(footwin, 0, COLS - 11 - padding);
		show_states_at(footwin);
	}

	/* Display how many percent the current line is into the file. */
	if (namewidth + 6 < COLS) {
		sprintf(location, "%3zi%%", 100 * openfile->current->lineno / openfile->filebot->lineno);
		mvwaddstr(footwin, 0, COLS - 4 - padding, location);
	}

	wattroff(footwin, interface_color_pair[MINI_INFOBAR]);
	wrefresh(footwin);

	free(number_of_lines);
	free(hexadecimal);
	free(location);
	free(thename);
	free(ranking);
}
#endif /* NANO_TINY */

/* Display the given message on the status bar, but only if its importance
 * is higher than that of a message that is already there. */
void statusline(message_type importance, const char *msg, ...)
{
	bool showed_whitespace = ISSET(WHITESPACE_DISPLAY);
	static size_t start_col = 0;
	char *compound, *message;
	bool bracketed;
	int colorpair;
	va_list ap;

	/* Ignore a message with an importance that is lower than the last one. */
	if (importance < lastmessage && lastmessage > NOTICE)
		return;

	/* Construct the message out of all the arguments. */
	compound = nmalloc(MAXCHARLEN * COLS + 1);
	va_start(ap, msg);
	vsnprintf(compound, MAXCHARLEN * COLS + 1, msg, ap);
	va_end(ap);

	/* When not in curses mode, write the message to standard error. */
	if (isendwin()) {
		fprintf(stderr, "\n%s\n", compound);
		free(compound);
		return;
	}

#if defined(ENABLE_MULTIBUFFER) && !defined(NANO_TINY)
	if (!we_are_running && importance == ALERT && openfile && !openfile->fmt &&
						!openfile->errormessage && openfile->next != openfile)
		openfile->errormessage = copy_of(compound);
#endif

	/* On a one-row terminal, ensure that any changes in the edit window are
	 * written out first, to prevent them from overwriting the message. */
	if (LINES == 1 && importance < INFO)
		wnoutrefresh(midwin);

	/* If there are multiple alert messages, add trailing dots to the first. */
	if (lastmessage == ALERT) {
		if (start_col > 4) {
			wmove(footwin, 0, COLS + 2 - start_col);
			wattron(footwin, interface_color_pair[ERROR_MESSAGE]);
			waddstr(footwin, "...");
			wattroff(footwin, interface_color_pair[ERROR_MESSAGE]);
			wnoutrefresh(footwin);
			start_col = 0;
			napms(100);
			beep();
		}
		free(compound);
		return;
	}

	if (importance > NOTICE) {
		if (importance == ALERT)
			beep();
		colorpair = interface_color_pair[ERROR_MESSAGE];
	} else if (importance == NOTICE)
		colorpair = interface_color_pair[SELECTED_TEXT];
	else
		colorpair = interface_color_pair[STATUS_BAR];

	lastmessage = importance;

	blank_statusbar();

	UNSET(WHITESPACE_DISPLAY);

	message = display_string(compound, 0, COLS, FALSE, FALSE);

	if (showed_whitespace)
		SET(WHITESPACE_DISPLAY);

	start_col = (COLS - breadth(message)) / 2;
	bracketed = (start_col > 1);

	wmove(footwin, 0, (bracketed ? start_col - 2 : start_col));
	wattron(footwin, colorpair);
	if (bracketed)
		waddstr(footwin, "[ ");
	waddstr(footwin, message);
	if (bracketed)
		waddstr(footwin, " ]");
	wattroff(footwin, colorpair);

#ifdef USING_OLDER_LIBVTE
	/* Defeat a VTE/Konsole bug, where the cursor can go off-limits. */
	if (ISSET(CONSTANT_SHOW) && ISSET(NO_HELP))
		wmove(footwin, 0, 0);
#endif

	/* Push the message to the screen straightaway. */
	wrefresh(footwin);

	free(compound);
	free(message);

	/* When requested, wipe the status bar after just one keystroke. */
	statusblank = (ISSET(QUICK_BLANK) ? 1 : 20);
}

/* Display a normal message on the status bar, quietly. */
void statusbar(const char *msg)
{
	statusline(HUSH, msg);
}

/* Warn the user on the status bar and pause for a moment, so that the
 * message can be noticed and read. */
void warn_and_briefly_pause(const char *msg)
{
	blank_bottombars();
	statusline(ALERT, msg);
	lastmessage = VACUUM;
	napms(1500);
}

/* Write a key's representation plus a minute description of its function
 * to the screen.  For example, the key could be "^C" and its tag "Cancel".
 * Key plus tag may occupy at most width columns. */
void post_one_key(const char *keystroke, const char *tag, int width)
{
	wattron(footwin, interface_color_pair[KEY_COMBO]);
	waddnstr(footwin, keystroke, actual_x(keystroke, width));
	wattroff(footwin, interface_color_pair[KEY_COMBO]);

	/* If the remaining space is too small, skip the description. */
	width -= breadth(keystroke);
	if (width < 2)
		return;

	waddch(footwin, ' ');
	wattron(footwin, interface_color_pair[FUNCTION_TAG]);
	waddnstr(footwin, tag, actual_x(tag, width - 1));
	wattroff(footwin, interface_color_pair[FUNCTION_TAG]);
}

/* Display the shortcut list corresponding to menu on the last two rows
 * of the bottom portion of the window. */
void bottombars(int menu)
{
	size_t index, number, itemwidth;
	const keystruct *s;
	funcstruct *f;

	/* Set the global variable to the given menu. */
	currmenu = menu;

	if (ISSET(NO_HELP) || LINES < 6)
		return;

	/* Determine how many shortcuts must be shown. */
	number = shown_entries_for(menu);

	/* Compute the width of each keyname-plus-explanation pair. */
	itemwidth = COLS / ((number + 1) / 2);

	/* If there is no room, don't print anything. */
	if (itemwidth == 0)
		return;

	blank_bottombars();

	/* Display the first number of shortcuts in the given menu that
	 * have a key combination assigned to them. */
	for (f = allfuncs, index = 0; f != NULL && index < number; f = f->next) {
		size_t thiswidth = itemwidth;

		if ((f->menus & menu) == 0)
			continue;

		s = first_sc_for(menu, f->func);

		if (s == NULL)
			continue;

		wmove(footwin, 1 + index % 2, (index / 2) * itemwidth);

		/* When the number is uneven, the penultimate item can be double wide. */
		if ((number % 2) == 1 && (index + 2 == number))
			thiswidth += itemwidth;

		/* For the last two items, use also the remaining slack. */
		if (index + 2 >= number)
			thiswidth += COLS % itemwidth;

		post_one_key(s->keystr, _(f->tag), thiswidth);

		index++;
	}

	wrefresh(footwin);
}

/* Redetermine current_y from the position of current relative to edittop,
 * and put the cursor in the edit window at (current_y, "current_x"). */
void place_the_cursor(void)
{
	ssize_t row = 0;
	size_t column = xplustabs();

#ifndef NANO_TINY
	if (ISSET(SOFTWRAP)) {
		linestruct *line = openfile->edittop;
		size_t leftedge;

		row -= chunk_for(openfile->firstcolumn, openfile->edittop);

		/* Calculate how many rows the lines from edittop to current use. */
		while (line != NULL && line != openfile->current) {
			row += 1 + extra_chunks_in(line);
			line = line->next;
		}

		/* Add the number of wraps in the current line before the cursor. */
		row += get_chunk_and_edge(column, openfile->current, &leftedge);
		column -= leftedge;
	} else
#endif
	{
		row = openfile->current->lineno - openfile->edittop->lineno;
		column -= get_page_start(column);
	}

	if (row < editwinrows)
		wmove(midwin, row, margin + column);
#ifndef NANO_TINY
	else
		statusline(ALERT, "Misplaced cursor -- please report a bug");
#endif

	openfile->current_y = row;
}

/* The number of bytes after which to stop painting, to avoid major slowdowns. */
#define PAINT_LIMIT  2000

/* Draw the given text on the given row of the edit window.  line is the
 * line to be drawn, and converted is the actual string to be written with
 * tabs and control characters replaced by strings of regular characters.
 * from_col is the column number of the first character of this "page". */
void draw_row(int row, const char *converted, linestruct *line, size_t from_col)
{
#ifdef ENABLE_LINENUMBERS
	/* If line numbering is switched on, put a line number in front of
	 * the text -- but only for the parts that are not softwrapped. */
	if (margin > 0) {
		wattron(midwin, interface_color_pair[LINE_NUMBER]);
#ifndef NANO_TINY
		if (ISSET(SOFTWRAP) && from_col != 0)
			mvwprintw(midwin, row, 0, "%*s", margin - 1, " ");
		else
#endif
			mvwprintw(midwin, row, 0, "%*zd", margin - 1, line->lineno);
		wattroff(midwin, interface_color_pair[LINE_NUMBER]);
#ifndef NANO_TINY
		if (line->has_anchor && (from_col == 0 || !ISSET(SOFTWRAP)))
#ifdef ENABLE_UTF8
			if (using_utf8())
				wprintw(midwin, "\xE2\xAC\xA5");  /* black medium diamond */
			else
#endif
				wprintw(midwin, "+");
		else
#endif
			wprintw(midwin, " ");
	}
#endif /* ENABLE_LINENUMBERS */

	/* First simply write the converted line -- afterward we'll add colors
	 * and the marking highlight on just the pieces that need it. */
	mvwaddstr(midwin, row, margin, converted);

	/* When needed, clear the remainder of the row. */
	if (is_shorter || ISSET(SOFTWRAP))
		wclrtoeol(midwin);

#ifndef NANO_TINY
	if (thebar)
		mvwaddch(midwin, row, COLS - 1, bardata[row]);
#endif

#ifdef ENABLE_COLOR
	/* If there are color rules (and coloring is turned on), apply them. */
	if (openfile->syntax && !ISSET(NO_SYNTAX)) {
		const colortype *varnish = openfile->syntax->color;

		/* If there are multiline regexes, make sure this line has a cache. */
		if (openfile->syntax->nmultis > 0 && line->multidata == NULL)
			line->multidata = nmalloc(openfile->syntax->nmultis * sizeof(short));

		/* Iterate through all the coloring regexes. */
		for (; varnish != NULL; varnish = varnish->next) {
			size_t index = 0;
				/* Where in the line we currently begin looking for a match. */
			int start_col = 0;
				/* The starting column of a piece to paint.  Zero-based. */
			int paintlen = 0;
				/* The number of characters to paint. */
			const char *thetext;
				/* The place in converted from where painting starts. */
			regmatch_t match;
				/* The match positions of a single-line regex. */
			const linestruct *start_line = line->prev;
				/* The first line before line that matches 'start'. */
			regmatch_t startmatch, endmatch;
				/* The match positions of the start and end regexes. */

			/* First case: varnish is a single-line expression. */
			if (varnish->end == NULL) {
				while (index < PAINT_LIMIT && index < till_x) {
					/* If there is no match, go on to the next line. */
					if (regexec(varnish->start, &line->data[index], 1,
								&match, (index == 0) ? 0 : REG_NOTBOL) != 0)
						break;

					/* Translate the match to the beginning of the line. */
					match.rm_so += index;
					match.rm_eo += index;
					index = match.rm_eo;

					/* If the match is offscreen to the right, this rule is done. */
					if (match.rm_so >= till_x)
						break;

					/* If the match has length zero, advance over it. */
					if (match.rm_so == match.rm_eo) {
						if (line->data[index] == '\0')
							break;
						index = step_right(line->data, index);
						continue;
					}

					/* If the match is offscreen to the left, skip to next. */
					if (match.rm_eo <= from_x)
						continue;

					if (match.rm_so > from_x)
						start_col = wideness(line->data, match.rm_so) - from_col;

					thetext = converted + actual_x(converted, start_col);

					paintlen = actual_x(thetext, wideness(line->data,
										match.rm_eo) - from_col - start_col);

					wattron(midwin, varnish->attributes);
					mvwaddnstr(midwin, row, margin + start_col, thetext, paintlen);
					wattroff(midwin, varnish->attributes);
				}

				continue;
			}

			/* Second case: varnish is a multiline expression. */

			/* Assume nothing gets painted until proven otherwise below. */
			line->multidata[varnish->id] = NOTHING;

			if (start_line && !start_line->multidata)
				statusline(ALERT, "Missing multidata -- please report a bug");
			else

			/* If there is an unterminated start match before the current line,
			 * we need to look for an end match first. */
			if (start_line && (start_line->multidata[varnish->id] == WHOLELINE ||
								start_line->multidata[varnish->id] == STARTSHERE)) {
				/* If there is no end on this line, paint whole line, and be done. */
				if (regexec(varnish->end, line->data, 1, &endmatch, 0) == REG_NOMATCH) {
					wattron(midwin, varnish->attributes);
					mvwaddnstr(midwin, row, margin, converted, -1);
					wattroff(midwin, varnish->attributes);
					line->multidata[varnish->id] = WHOLELINE;
					continue;
				}

				/* Only if it is visible, paint the part to be coloured. */
				if (endmatch.rm_eo > from_x) {
					paintlen = actual_x(converted, wideness(line->data,
													endmatch.rm_eo) - from_col);
					wattron(midwin, varnish->attributes);
					mvwaddnstr(midwin, row, margin, converted, paintlen);
					wattroff(midwin, varnish->attributes);
				}

				line->multidata[varnish->id] = ENDSHERE;
			}

			/* Second step: look for starts on this line, but begin
			 * looking only after an end match, if there is one. */
			index = (paintlen == 0) ? 0 : endmatch.rm_eo;

			while (index < PAINT_LIMIT && regexec(varnish->start, line->data + index,
								1, &startmatch, (index == 0) ? 0 : REG_NOTBOL) == 0) {
				/* Make the match relative to the beginning of the line. */
				startmatch.rm_so += index;
				startmatch.rm_eo += index;

				if (startmatch.rm_so > from_x)
					start_col = wideness(line->data, startmatch.rm_so) - from_col;

				thetext = converted + actual_x(converted, start_col);

				if (regexec(varnish->end, line->data + startmatch.rm_eo, 1, &endmatch,
									(startmatch.rm_eo == 0) ? 0 : REG_NOTBOL) == 0) {
					/* Make the match relative to the beginning of the line. */
					endmatch.rm_so += startmatch.rm_eo;
					endmatch.rm_eo += startmatch.rm_eo;
					/* Only paint the match if it is visible on screen
					 * and it is more than zero characters long. */
					if (endmatch.rm_eo > from_x && endmatch.rm_eo > startmatch.rm_so) {
						paintlen = actual_x(thetext, wideness(line->data,
											endmatch.rm_eo) - from_col - start_col);

						wattron(midwin, varnish->attributes);
						mvwaddnstr(midwin, row, margin + start_col, thetext, paintlen);
						wattroff(midwin, varnish->attributes);

						line->multidata[varnish->id] = JUSTONTHIS;
					}
					index = endmatch.rm_eo;
					/* If both start and end match are anchors, advance. */
					if (startmatch.rm_so == startmatch.rm_eo &&
										endmatch.rm_so == endmatch.rm_eo) {
						if (line->data[index] == '\0')
							break;
						index = step_right(line->data, index);
					}
					continue;
				}

				/* Paint the rest of the line, and we're done. */
				wattron(midwin, varnish->attributes);
				mvwaddnstr(midwin, row, margin + start_col, thetext, -1);
				wattroff(midwin, varnish->attributes);

				line->multidata[varnish->id] = STARTSHERE;
				break;
			}
		}
	}
#endif /* ENABLE_COLOR */

#ifndef NANO_TINY
	if (stripe_column > from_col && !inhelp &&
					(sequel_column == 0 || stripe_column <= sequel_column) &&
					stripe_column <= from_col + editwincols) {
		ssize_t target_column = stripe_column - from_col - 1;
		size_t target_x = actual_x(converted, target_column);
		char striped_char[MAXCHARLEN];
		size_t charlen = 1;

		if (*(converted + target_x) != '\0') {
			charlen = collect_char(converted + target_x, striped_char);
			target_column = wideness(converted, target_x);
#ifdef USING_OLDER_LIBVTE
		} else if (target_column + 1 == editwincols) {
			/* Defeat a VTE bug -- see https://sv.gnu.org/bugs/?55896. */
#ifdef ENABLE_UTF8
			if (using_utf8()) {
				striped_char[0] = '\xC2';
				striped_char[1] = '\xA0';
				charlen = 2;
			} else
#endif
				striped_char[0] = '.';
#endif
		} else
			striped_char[0] = ' ';

		wattron(midwin, interface_color_pair[GUIDE_STRIPE]);
		mvwaddnstr(midwin, row, margin + target_column, striped_char, charlen);
		wattroff(midwin, interface_color_pair[GUIDE_STRIPE]);
	}

	/* If the line is at least partially selected, paint the marked part. */
	if (openfile->mark && ((line->lineno >= openfile->mark->lineno &&
						line->lineno <= openfile->current->lineno) ||
						(line->lineno <= openfile->mark->lineno &&
						line->lineno >= openfile->current->lineno))) {
		linestruct *top, *bot;
			/* The lines where the marked region begins and ends. */
		size_t top_x, bot_x;
			/* The x positions where the marked region begins and ends. */
		int start_col;
			/* The column where painting starts.  Zero-based. */
		const char *thetext;
			/* The place in converted from where painting starts. */
		int paintlen = -1;
			/* The number of characters to paint.  Negative means "all". */

		get_region(&top, &top_x, &bot, &bot_x);

		if (top->lineno < line->lineno || top_x < from_x)
			top_x = from_x;
		if (bot->lineno > line->lineno || bot_x > till_x)
			bot_x = till_x;

		/* Only paint if the marked part of the line is on this page. */
		if (top_x < till_x && bot_x > from_x) {
			/* Compute on which screen column to start painting. */
			start_col = wideness(line->data, top_x) - from_col;

			if (start_col < 0)
				start_col = 0;

			thetext = converted + actual_x(converted, start_col);

			/* If the end of the mark is onscreen, compute how many
			 * characters to paint.  Otherwise, just paint all. */
			if (bot_x < till_x) {
				size_t end_col = wideness(line->data, bot_x) - from_col;
				paintlen = actual_x(thetext, end_col - start_col);
			}

			wattron(midwin, interface_color_pair[SELECTED_TEXT]);
			mvwaddnstr(midwin, row, margin + start_col, thetext, paintlen);
			wattroff(midwin, interface_color_pair[SELECTED_TEXT]);
		}
	}
#endif /* !NANO_TINY */
}

/* Redraw the given line so that the character at the given index is visible
 * -- if necessary, scroll the line horizontally (when not softwrapping).
 * Return the number of rows "consumed" (relevant when softwrapping). */
int update_line(linestruct *line, size_t index)
{
	int row;
		/* The row in the edit window we will be updating. */
	char *converted;
		/* The data of the line with tabs and control characters expanded. */
	size_t from_col;
		/* From which column a horizontally scrolled line is displayed. */

#ifndef NANO_TINY
	if (ISSET(SOFTWRAP))
		return update_softwrapped_line(line);

	sequel_column = 0;
#endif

	row = line->lineno - openfile->edittop->lineno;
	from_col = get_page_start(wideness(line->data, index));

	/* Expand the piece to be drawn to its representable form, and draw it. */
	converted = display_string(line->data, from_col, editwincols, TRUE, FALSE);
	draw_row(row, converted, line, from_col);
	free(converted);

	if (from_col > 0) {
		wattron(midwin, hilite_attribute);
		mvwaddch(midwin, row, margin, '<');
		wattroff(midwin, hilite_attribute);
	}
	if (has_more) {
		wattron(midwin, hilite_attribute);
		mvwaddch(midwin, row, COLS - 1 - thebar, '>');
		wattroff(midwin, hilite_attribute);
	}

	if (spotlighted && line == openfile->current)
		spotlight(light_from_col, light_to_col);

	return 1;
}

#ifndef NANO_TINY
/* Redraw all the chunks of the given line (as far as they fit onscreen),
 * unless it's edittop, which will be displayed from column firstcolumn.
 * Return the number of rows that were "consumed". */
int update_softwrapped_line(linestruct *line)
{
	int row = 0;
		/* The row in the edit window we will write to. */
	linestruct *someline = openfile->edittop;
		/* An iterator needed to find the relevant row. */
	int starting_row;
		/* The first row in the edit window that gets updated. */
	size_t from_col = 0;
		/* The starting column of the current chunk. */
	size_t to_col = 0;
		/* The end column of the current chunk. */
	char *converted;
		/* The data of the chunk with tabs and control characters expanded. */
	bool kickoff = TRUE;
		/* This tells the softwrapping routine to start at beginning-of-line. */
	bool end_of_line = FALSE;
		/* Becomes TRUE when the last chunk of the line has been reached. */

	if (line == openfile->edittop)
		from_col = openfile->firstcolumn;
	else
		row -= chunk_for(openfile->firstcolumn, openfile->edittop);

	/* Find out on which screen row the target line should be shown. */
	while (someline != line && someline != NULL) {
		row += 1 + extra_chunks_in(someline);
		someline = someline->next;
	}

	/* If the first chunk is offscreen, don't even try to display it. */
	if (row < 0 || row >= editwinrows) {
		statusline(ALERT, "Badness: tried to display a chunk on row %i"
								" -- please report a bug", row);
		return 0;
	}

	starting_row = row;

	while (!end_of_line && row < editwinrows) {
		to_col = get_softwrap_breakpoint(line->data, from_col, &kickoff, &end_of_line);

		sequel_column = (end_of_line) ? 0 : to_col;

		/* Convert the chunk to its displayable form and draw it. */
		converted = display_string(line->data, from_col, to_col - from_col,
									TRUE, FALSE);
		draw_row(row++, converted, line, from_col);
		free(converted);

		from_col = to_col;
	}

	if (spotlighted && line == openfile->current)
		spotlight_softwrapped(light_from_col, light_to_col);

	return (row - starting_row);
}
#endif

/* Check whether the mark is on, or whether old_column and new_column are on
 * different "pages" (in softwrap mode, only the former applies), which means
 * that the relevant line needs to be redrawn. */
bool line_needs_update(const size_t old_column, const size_t new_column)
{
#ifndef NANO_TINY
	if (openfile->mark)
		return TRUE;
	else
#endif
		return (get_page_start(old_column) != get_page_start(new_column));
}

/* Try to move up nrows softwrapped chunks from the given line and the
 * given column (leftedge).  After moving, leftedge will be set to the
 * starting column of the current chunk.  Return the number of chunks we
 * couldn't move up, which will be zero if we completely succeeded. */
int go_back_chunks(int nrows, linestruct **line, size_t *leftedge)
{
	int i;

#ifndef NANO_TINY
	if (ISSET(SOFTWRAP)) {
		/* Recede through the requested number of chunks. */
		for (i = nrows; i > 0; i--) {
			size_t chunk = chunk_for(*leftedge, *line);

			*leftedge = 0;

			if (chunk >= i)
				return go_forward_chunks(chunk - i, line, leftedge);

			if (*line == openfile->filetop)
				break;

			i -= chunk;
			*line = (*line)->prev;
			*leftedge = HIGHEST_POSITIVE;
		}

		if (*leftedge == HIGHEST_POSITIVE)
			*leftedge = leftedge_for(*leftedge, *line);
	} else
#endif
		for (i = nrows; i > 0 && (*line)->prev != NULL; i--)
			*line = (*line)->prev;

	return i;
}

/* Try to move down nrows softwrapped chunks from the given line and the
 * given column (leftedge).  After moving, leftedge will be set to the
 * starting column of the current chunk.  Return the number of chunks we
 * couldn't move down, which will be zero if we completely succeeded. */
int go_forward_chunks(int nrows, linestruct **line, size_t *leftedge)
{
	int i;

#ifndef NANO_TINY
	if (ISSET(SOFTWRAP)) {
		size_t current_leftedge = *leftedge;
		bool kickoff = TRUE;

		/* Advance through the requested number of chunks. */
		for (i = nrows; i > 0; i--) {
			bool end_of_line = FALSE;

			current_leftedge = get_softwrap_breakpoint((*line)->data,
									current_leftedge, &kickoff, &end_of_line);

			if (!end_of_line)
				continue;

			if (*line == openfile->filebot)
				break;

			*line = (*line)->next;
			current_leftedge = 0;
			kickoff = TRUE;
		}

		/* Only change leftedge when we actually could move. */
		if (i < nrows)
			*leftedge = current_leftedge;
	} else
#endif
		for (i = nrows; i > 0 && (*line)->next != NULL; i--)
			*line = (*line)->next;

	return i;
}

/* Return TRUE if there are fewer than a screen's worth of lines between
 * the line at line number was_lineno (and column was_leftedge, if we're
 * in softwrap mode) and the line at current[current_x]. */
bool less_than_a_screenful(size_t was_lineno, size_t was_leftedge)
{
#ifndef NANO_TINY
	if (ISSET(SOFTWRAP)) {
		linestruct *line = openfile->current;
		size_t leftedge = leftedge_for(xplustabs(), openfile->current);
		int rows_left = go_back_chunks(editwinrows - 1, &line, &leftedge);

		return (rows_left > 0 || line->lineno < was_lineno ||
				(line->lineno == was_lineno && leftedge <= was_leftedge));
	} else
#endif
		return (openfile->current->lineno - was_lineno < editwinrows);
}

#ifndef NANO_TINY
/* Draw a scroll bar on the righthand side of the screen. */
void draw_scrollbar(void)
{
	int fromline = openfile->edittop->lineno - 1;
	int totallines = openfile->filebot->lineno;
	int coveredlines = editwinrows;

	if (ISSET(SOFTWRAP)) {
		linestruct *line = openfile->edittop;
		int extras = extra_chunks_in(line) - chunk_for(openfile->firstcolumn, line);

		while (line->lineno + extras < fromline + editwinrows && line->next) {
			line = line->next;
			extras += extra_chunks_in(line);
		}

		coveredlines = line->lineno - fromline;
	}

	int lowest = (fromline * editwinrows) / totallines;
	int highest = lowest + (editwinrows * coveredlines) / totallines;

	if (editwinrows > totallines)
		highest = editwinrows;

	for (int row = 0; row < editwinrows; row++) {
		bardata[row] = ' '|interface_color_pair[SCROLL_BAR]|
					((row < lowest || row > highest) ? A_NORMAL : A_REVERSE);
		mvwaddch(midwin, row, COLS - 1, bardata[row]);
	}
}
#endif

/* Scroll the edit window one row in the given direction, and
 * draw the relevant content on the resultant blank row. */
void edit_scroll(bool direction)
{
	linestruct *line;
	size_t leftedge;
	int nrows = 1;

	/* Move the top line of the edit window one row up or down. */
	if (direction == BACKWARD)
		go_back_chunks(1, &openfile->edittop, &openfile->firstcolumn);
	else
		go_forward_chunks(1, &openfile->edittop, &openfile->firstcolumn);

	/* Actually scroll the text of the edit window one row up or down. */
	scrollok(midwin, TRUE);
	wscrl(midwin, (direction == BACKWARD) ? -1 : 1);
	scrollok(midwin, FALSE);

	/* If we're not on the first "page" (when not softwrapping), or the mark
	 * is on, the row next to the scrolled region needs to be redrawn too. */
	if (line_needs_update(openfile->placewewant, 0) && nrows < editwinrows)
		nrows++;

	/* If we scrolled backward, the top row needs to be redrawn. */
	line = openfile->edittop;
	leftedge = openfile->firstcolumn;

	/* If we scrolled forward, the bottom row needs to be redrawn. */
	if (direction == FORWARD)
		go_forward_chunks(editwinrows - nrows, &line, &leftedge);

#ifndef NANO_TINY
	if (thebar)
		draw_scrollbar();

	if (ISSET(SOFTWRAP)) {
		/* Compensate for the earlier chunks of a softwrapped line. */
		nrows += chunk_for(leftedge, line);

		/* Don't compensate for the chunks that are offscreen. */
		if (line == openfile->edittop)
			nrows -= chunk_for(openfile->firstcolumn, line);
	}
#endif

	/* Draw new content on the blank row (and on the bordering row too
	 * when it was deemed necessary). */
	while (nrows > 0 && line != NULL) {
		nrows -= update_line(line, (line == openfile->current) ?
										openfile->current_x : 0);
		line = line->next;
	}
}

#ifndef NANO_TINY
/* Get the column number after leftedge where we can break the given linedata,
 * and return it.  (This will always be at most editwincols after leftedge.)
 * When kickoff is TRUE, start at the beginning of the linedata; otherwise,
 * continue from where the previous call left off.  Set end_of_line to TRUE
 * when end-of-line is reached while searching for a possible breakpoint. */
size_t get_softwrap_breakpoint(const char *linedata, size_t leftedge,
								bool *kickoff, bool *end_of_line)
{
	static const char *text;
		/* Pointer at the current character in this line's data. */
	static size_t column;
		/* Column position that corresponds to the above pointer. */
	size_t goal_column = leftedge + editwincols;
		/* The place at or before which text must be broken. */
	size_t breaking_col = goal_column;
		/* The column where text can be broken, when there's no better. */
	size_t last_blank_col = 0;
		/* The column position of the last seen whitespace character. */
	const char *farthest_blank = NULL;
		/* A pointer to the last seen whitespace character in text. */

	/* Initialize the static variables when it's another line. */
	if (*kickoff) {
		text = linedata;
		column = 0;
		*kickoff = FALSE;
	}

	/* First find the place in text where the current chunk starts. */
	while (*text != '\0' && column < leftedge)
		text += advance_over(text, &column);

	/* Now find the place in text where this chunk should end. */
	while (*text != '\0' && column <= goal_column) {
		/* When breaking at blanks, do it *before* the target column. */
		if (ISSET(AT_BLANKS) && is_blank_char(text) && column < goal_column) {
			farthest_blank = text;
			last_blank_col = column;
		}

		breaking_col = (*text == '\t' ? goal_column : column);
		text += advance_over(text, &column);
	}

	/* If we didn't overshoot the limit, we've found a breaking point;
	 * and we've reached EOL if we didn't even *reach* the limit. */
	if (column <= goal_column) {
		*end_of_line = (column < goal_column);
		return column;
	}

	/* If we're softwrapping at blanks and we found at least one blank, break
	 * after that blank -- if it doesn't overshoot the screen's edge. */
	if (farthest_blank != NULL) {
		advance_over(farthest_blank, &last_blank_col);

		if (last_blank_col <= goal_column)
			return last_blank_col;

		/* If it's a tab that overshoots, break at the screen's edge. */
		if (*farthest_blank == '\t')
			breaking_col = goal_column;
	}

	/* Otherwise, break at the last character that doesn't overshoot. */
	return (editwincols > 1) ? breaking_col : column - 1;
}

/* Return the row number of the softwrapped chunk in the given line that the
 * given column is on, relative to the first row (zero-based).  If leftedge
 * isn't NULL, return in it the leftmost column of the chunk. */
size_t get_chunk_and_edge(size_t column, linestruct *line, size_t *leftedge)
{
	size_t current_chunk = 0;
	bool end_of_line = FALSE;
	bool kickoff = TRUE;
	size_t start_col = 0;
	size_t end_col;

	while (TRUE) {
		end_col = get_softwrap_breakpoint(line->data, start_col, &kickoff, &end_of_line);

		/* When the column is in range or we reached end-of-line, we're done. */
		if (end_of_line || (start_col <= column && column < end_col)) {
			if (leftedge != NULL)
				*leftedge = start_col;
			return current_chunk;
		}

		start_col = end_col;
		current_chunk++;
	}
}

/* Return how many extra rows the given line needs when softwrapping. */
size_t extra_chunks_in(linestruct *line)
{
	return get_chunk_and_edge((size_t)-1, line, NULL);
}

/* Return the row of the softwrapped chunk of the given line that column is on,
 * relative to the first row (zero-based). */
size_t chunk_for(size_t column, linestruct *line)
{
	return get_chunk_and_edge(column, line, NULL);
}

/* Return the leftmost column of the softwrapped chunk of the given line that
 * the given column is on. */
size_t leftedge_for(size_t column, linestruct *line)
{
	size_t leftedge;

	get_chunk_and_edge(column, line, &leftedge);

	return leftedge;
}

/* Ensure that firstcolumn is at the starting column of the softwrapped chunk
 * it's on.  We need to do this when the number of columns of the edit window
 * has changed, because then the width of softwrapped chunks has changed. */
void ensure_firstcolumn_is_aligned(void)
{
	if (ISSET(SOFTWRAP))
		openfile->firstcolumn = leftedge_for(openfile->firstcolumn,
														openfile->edittop);
	else
		openfile->firstcolumn = 0;

	/* If smooth scrolling is on, make sure the viewport doesn't center. */
	focusing = FALSE;
}
#endif /* !NANO_TINY */

/* When in softwrap mode, and the given column is on or after the breakpoint of
 * a softwrapped chunk, shift it back to the last column before the breakpoint.
 * The given column is relative to the given leftedge in current.  The returned
 * column is relative to the start of the text. */
size_t actual_last_column(size_t leftedge, size_t column)
{
#ifndef NANO_TINY
	if (ISSET(SOFTWRAP)) {
		bool kickoff = TRUE;
		bool last_chunk = FALSE;
		size_t end_col = get_softwrap_breakpoint(openfile->current->data,
										leftedge, &kickoff, &last_chunk) - leftedge;

		/* If we're not on the last chunk, we're one column past the end of
		 * the row.  Shifting back one column might put us in the middle of
		 * a multi-column character, but actual_x() will fix that later. */
		if (!last_chunk)
			end_col--;

		if (column > end_col)
			column = end_col;
	}
#endif

	return leftedge + column;
}

/* Return TRUE if current[current_x] is before the viewport. */
bool current_is_above_screen(void)
{
#ifndef NANO_TINY
	if (ISSET(SOFTWRAP))
		return (openfile->current->lineno < openfile->edittop->lineno ||
				(openfile->current->lineno == openfile->edittop->lineno &&
				xplustabs() < openfile->firstcolumn));
	else
#endif
		return (openfile->current->lineno < openfile->edittop->lineno);
}

#define SHIM  (ISSET(ZERO) && (currmenu == MREPLACEWITH || currmenu == MYESNO) ? 1 : 0)

/* Return TRUE if current[current_x] is beyond the viewport. */
bool current_is_below_screen(void)
{
#ifndef NANO_TINY
	if (ISSET(SOFTWRAP)) {
		linestruct *line = openfile->edittop;
		size_t leftedge = openfile->firstcolumn;

		/* If current[current_x] is more than a screen's worth of lines after
		 * edittop at column firstcolumn, it's below the screen. */
		return (go_forward_chunks(editwinrows - 1 - SHIM, &line, &leftedge) == 0 &&
						(line->lineno < openfile->current->lineno ||
						(line->lineno == openfile->current->lineno &&
						leftedge < leftedge_for(xplustabs(), openfile->current))));
	} else
#endif
		return (openfile->current->lineno >=
						openfile->edittop->lineno + editwinrows - SHIM);
}

/* Return TRUE if current[current_x] is outside the viewport. */
bool current_is_offscreen(void)
{
	return (current_is_above_screen() || current_is_below_screen());
}

/* Update any lines between old_current and current that need to be
 * updated.  Use this if we've moved without changing any text. */
void edit_redraw(linestruct *old_current, update_type manner)
{
	size_t was_pww = openfile->placewewant;

	openfile->placewewant = xplustabs();

	/* If the current line is offscreen, scroll until it's onscreen. */
	if (current_is_offscreen()) {
		adjust_viewport(ISSET(JUMPY_SCROLLING) ? CENTERING : manner);
		refresh_needed = TRUE;
		return;
	}

#ifndef NANO_TINY
	/* If the mark is on, update all lines between old_current and current. */
	if (openfile->mark) {
		linestruct *line = old_current;

		while (line != openfile->current) {
			update_line(line, 0);

			line = (line->lineno > openfile->current->lineno) ?
						line->prev : line->next;
		}
	} else
#endif
		/* Otherwise, update old_current only if it differs from current
		 * and was horizontally scrolled. */
		if (old_current != openfile->current && get_page_start(was_pww) > 0)
			update_line(old_current, 0);

	/* Update current if the mark is on or it has changed "page", or if it
	 * differs from old_current and needs to be horizontally scrolled. */
	if (line_needs_update(was_pww, openfile->placewewant) ||
						(old_current != openfile->current &&
						get_page_start(openfile->placewewant) > 0))
		update_line(openfile->current, openfile->current_x);
}

/* Refresh the screen without changing the position of lines.  Use this
 * if we've moved and changed text. */
void edit_refresh(void)
{
	linestruct *line;
	int row = 0;

#ifdef ENABLE_COLOR
	/* When needed and useful, initialize the colors for the current syntax. */
	if (openfile->syntax && !have_palette && !ISSET(NO_SYNTAX) && has_colors())
		prepare_palette();

	if (recook) {
		precalc_multicolorinfo();
		perturbed = FALSE;
		recook = FALSE;
	}
#endif

	/* If the current line is out of view, get it back on screen. */
	if (current_is_offscreen())
		adjust_viewport((focusing || ISSET(JUMPY_SCROLLING)) ? CENTERING : FLOWING);

#ifndef NANO_TINY
	if (thebar)
		draw_scrollbar();
#endif

//#define TIMEREFRESH  123
#ifdef TIMEREFRESH
#include <time.h>
	clock_t start = clock();
#endif

	line = openfile->edittop;

	while (row < editwinrows && line != NULL) {
		if (line == openfile->current)
			row += update_line(line, openfile->current_x);
		else
			row += update_line(line, 0);
		line = line->next;
	}

	while (row < editwinrows) {
		blank_row(midwin, row);
#ifndef NANO_TINY
		if (thebar)
			mvwaddch(midwin, row, COLS - 1, bardata[row]);
#endif
		row++;
	}

#ifdef TIMEREFRESH
	statusline(INFO, "Refresh: %.1f ms", 1000 * (double)(clock() - start) / CLOCKS_PER_SEC);
#endif

	place_the_cursor();
	wnoutrefresh(midwin);

	refresh_needed = FALSE;
}

/* Move edittop so that current is on the screen.  manner says how:
 * STATIONARY means that the cursor should stay on the same screen row,
 * CENTERING means that current should end up in the middle of the screen,
 * and FLOWING means that it should scroll no more than needed to bring
 * current into view. */
void adjust_viewport(update_type manner)
{
	int goal = 0;

	if (manner == STATIONARY)
		goal = openfile->current_y;
	else if (manner == CENTERING)
		goal = editwinrows / 2;
	else if (!current_is_above_screen())
		goal = editwinrows - 1 - SHIM;

	openfile->edittop = openfile->current;
#ifndef NANO_TINY
	if (ISSET(SOFTWRAP))
		openfile->firstcolumn = leftedge_for(xplustabs(), openfile->current);
#endif

	/* Move edittop back goal rows, starting at current[current_x]. */
	go_back_chunks(goal, &openfile->edittop, &openfile->firstcolumn);
}

/* Tell curses to unconditionally redraw whatever was on the screen. */
void full_refresh(void)
{
	wrefresh(curscr);
}

/* Draw all elements of the screen.  That is: the title bar plus the content
 * of the edit window (when not in the file browser), and the bottom bars. */
void draw_all_subwindows(void)
{
	if (currmenu & ~(MBROWSER|MWHEREISFILE|MGOTODIR))
		titlebar(title);
#ifdef ENABLE_HELP
	if (inhelp) {
		close_buffer();
		wrap_help_text_into_buffer();
	} else
#endif
	if (currmenu & ~(MBROWSER|MWHEREISFILE|MGOTODIR))
		edit_refresh();
	bottombars(currmenu);
}

/* Display on the status bar details about the current cursor position. */
void report_cursor_position(void)
{
	size_t fullwidth = breadth(openfile->current->data) + 1;
	size_t column = xplustabs() + 1;
	int linepct, colpct, charpct;
	char saved_byte;
	size_t sum;

	saved_byte = openfile->current->data[openfile->current_x];
	openfile->current->data[openfile->current_x] = '\0';

	/* Determine the size of the file up to the cursor. */
	sum = number_of_characters_in(openfile->filetop, openfile->current);

	openfile->current->data[openfile->current_x] = saved_byte;

	/* Calculate the percentages. */
	linepct = 100 * openfile->current->lineno / openfile->filebot->lineno;
	colpct = 100 * column / fullwidth;
	charpct = (openfile->totsize == 0) ? 0 : 100 * sum / openfile->totsize;

	statusline(INFO,
			_("line %*zd/%zd (%2d%%), col %2zu/%2zu (%3d%%), char %*zu/%zu (%2d%%)"),
			digits(openfile->filebot->lineno),
			openfile->current->lineno, openfile->filebot->lineno, linepct,
			column, fullwidth, colpct,
			digits(openfile->totsize), sum, openfile->totsize, charpct);
}

/* Highlight the text between the given two columns on the current line. */
void spotlight(size_t from_col, size_t to_col)
{
	size_t right_edge = get_page_start(from_col) + editwincols;
	bool overshoots = (to_col > right_edge);
	char *word;

	place_the_cursor();

	/* Limit the end column to the edge of the screen. */
	if (overshoots)
		to_col = right_edge;

	/* If the target text is of zero length, highlight a space instead. */
	if (to_col == from_col) {
		word = copy_of(" ");
		to_col++;
	} else
		word = display_string(openfile->current->data, from_col,
								to_col - from_col, FALSE, overshoots);

	wattron(midwin, interface_color_pair[SPOTLIGHTED]);
	waddnstr(midwin, word, actual_x(word, to_col));
	if (overshoots)
		mvwaddch(midwin, openfile->current_y, COLS - 1 - thebar, '>');
	wattroff(midwin, interface_color_pair[SPOTLIGHTED]);

	free(word);
}

#ifndef NANO_TINY
/* Highlight the text between the given two columns on the current line. */
void spotlight_softwrapped(size_t from_col, size_t to_col)
{
	ssize_t row;
	size_t leftedge = leftedge_for(from_col, openfile->current);
	size_t break_col;
	bool end_of_line = FALSE;
	bool kickoff = TRUE;
	char *word;

	place_the_cursor();
	row = openfile->current_y;

	while (row < editwinrows) {
		break_col = get_softwrap_breakpoint(openfile->current->data,
											leftedge, &kickoff, &end_of_line);

		/* If the highlighting ends on this chunk, we can stop after it. */
		if (break_col >= to_col) {
			end_of_line = TRUE;
			break_col = to_col;
		}

		/* If the target text is of zero length, highlight a space instead. */
		if (break_col == from_col) {
			word = copy_of(" ");
			break_col++;
		} else
			word = display_string(openfile->current->data, from_col,
										break_col - from_col, FALSE, FALSE);

		wattron(midwin, interface_color_pair[SPOTLIGHTED]);
		waddnstr(midwin, word, actual_x(word, break_col));
		wattroff(midwin, interface_color_pair[SPOTLIGHTED]);

		free(word);

		if (end_of_line)
			break;

		wmove(midwin, ++row, margin);

		leftedge = break_col;
		from_col = break_col;
	}
}
#endif

#ifdef ENABLE_EXTRA
#define CREDIT_LEN  52
#define XLCREDIT_LEN  9

/* Fully blank the terminal screen, then slowly "crawl" the credits over it.
 * Abort the crawl upon any keystroke. */
void do_credits(void)
{
	bool with_interface = !ISSET(ZERO);
	bool with_help = !ISSET(NO_HELP);
	int crpos = 0, xlpos = 0;

	const char *credits[CREDIT_LEN] = {
		NULL,                /* "The nano text editor" */
		NULL,                /* "version" */
		VERSION,
		"",
		NULL,                /* "Brought to you by:" */
		"Chris Allegretta",
		"Benno Schulenberg",
		"David Lawrence Ramsey",
		"Jordi Mallach",
		"David Benbennick",
		"Rocco Corsi",
		"Mike Frysinger",
		"Adam Rogoyski",
		"Rob Siemborski",
		"Mark Majeres",
		"Ken Tyler",
		"Sven Guckes",
		"Bill Soudan",
		"Christian Weisgerber",
		"Erik Andersen",
		"Big Gaute",
		"Joshua Jensen",
		"Ryan Krebs",
		"Albert Chin",
		"",
		NULL,                /* "Special thanks to:" */
		"Monique, Brielle & Joseph",
		"Plattsburgh State University",
		"Benet Laboratories",
		"Amy Allegretta",
		"Linda Young",
		"Jeremy Robichaud",
		"Richard Kolb II",
		NULL,                /* "The Free Software Foundation" */
		"Linus Torvalds",
		NULL,                /* "the many translators and the TP" */
		NULL,                /* "For ncurses:" */
		"Thomas Dickey",
		"Pavel Curtis",
		"Zeyd Ben-Halim",
		"Eric S. Raymond",
		NULL,                /* "and anyone else we forgot..." */
		"",
		"",
		NULL,                /* "Thank you for using nano!" */
		"",
		"",
		"(C) 2023",
		"Free Software Foundation, Inc.",
		"",
		"",
		"https://nano-editor.org/"
	};

	const char *xlcredits[XLCREDIT_LEN] = {
		N_("The nano text editor"),
		N_("version"),
		N_("Brought to you by:"),
		N_("Special thanks to:"),
		N_("The Free Software Foundation"),
		N_("the many translators and the TP"),
		N_("For ncurses:"),
		N_("and anyone else we forgot..."),
		N_("Thank you for using nano!")
	};

	if (with_interface || with_help) {
		SET(ZERO);
		SET(NO_HELP);
		window_init();
	}

	nodelay(midwin, TRUE);
	scrollok(midwin, TRUE);

	blank_edit();
	wrefresh(midwin);
	napms(600);

	for (crpos = 0; crpos < CREDIT_LEN + editwinrows / 2; crpos++) {
		if (crpos < CREDIT_LEN) {
			const char *text = credits[crpos];

			if (!text)
				text = _(xlcredits[xlpos++]);

			mvwaddstr(midwin, editwinrows - 1, (COLS - breadth(text)) / 2, text);
			wrefresh(midwin);
		}

		if (wgetch(midwin) != ERR)
			break;

		napms(600);
		wscrl(midwin, 1);
		wrefresh(midwin);

		if (wgetch(midwin) != ERR)
			break;

		napms(600);
		wscrl(midwin, 1);
		wrefresh(midwin);
	}

	if (with_interface)
		UNSET(ZERO);
	if (with_help)
		UNSET(NO_HELP);
	window_init();

	scrollok(midwin, FALSE);
	nodelay(midwin, FALSE);

	draw_all_subwindows();
}
#endif /* ENABLE_EXTRA */
