// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application console interface
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <charset.h>
#include <dm/device.h>
#include <efi_loader.h>
#include <stdio_dev.h>
#include <video_console.h>

#define EFI_COUT_MODE_2 2
#define EFI_MAX_COUT_MODE 3

struct cout_mode {
	unsigned long columns;
	unsigned long rows;
	int present;
};

static struct cout_mode efi_cout_modes[] = {
	/* EFI Mode 0 is 80x25 and always present */
	{
		.columns = 80,
		.rows = 25,
		.present = 1,
	},
	/* EFI Mode 1 is always 80x50 */
	{
		.columns = 80,
		.rows = 50,
		.present = 0,
	},
	/* Value are unknown until we query the console */
	{
		.columns = 0,
		.rows = 0,
		.present = 0,
	},
};

const efi_guid_t efi_guid_text_input_ex_protocol =
			EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
const efi_guid_t efi_guid_text_input_protocol =
			EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID;
const efi_guid_t efi_guid_text_output_protocol =
			EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

#define cESC '\x1b'
#define ESC "\x1b"

/* Default to mode 0 */
static struct simple_text_output_mode efi_con_mode = {
	.max_mode = 1,
	.mode = 0,
	.attribute = 0,
	.cursor_column = 0,
	.cursor_row = 0,
	.cursor_visible = 1,
};

static int term_get_char(s32 *c)
{
	u64 timeout;

	/* Wait up to 100 ms for a character */
	timeout = timer_get_us() + 100000;

	while (!tstc())
		if (timer_get_us() > timeout)
			return 1;

	*c = getc();
	return 0;
}

/*
 * Receive and parse a reply from the terminal.
 *
 * @n:		array of return values
 * @num:	number of return values expected
 * @end_char:	character indicating end of terminal message
 * @return:	non-zero indicates error
 */
static int term_read_reply(int *n, int num, char end_char)
{
	s32 c;
	int i = 0;

	if (term_get_char(&c) || c != cESC)
		return -1;

	if (term_get_char(&c) || c != '[')
		return -1;

	n[0] = 0;
	while (1) {
		if (!term_get_char(&c)) {
			if (c == ';') {
				i++;
				if (i >= num)
					return -1;
				n[i] = 0;
				continue;
			} else if (c == end_char) {
				break;
			} else if (c > '9' || c < '0') {
				return -1;
			}

			/* Read one more decimal position */
			n[i] *= 10;
			n[i] += c - '0';
		} else {
			return -1;
		}
	}
	if (i != num - 1)
		return -1;

	return 0;
}

static efi_status_t EFIAPI efi_cout_output_string(
			struct efi_simple_text_output_protocol *this,
			const efi_string_t string)
{
	struct simple_text_output_mode *con = &efi_con_mode;
	struct cout_mode *mode = &efi_cout_modes[con->mode];
	char *buf, *pos;
	u16 *p;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p", this, string);

	if (!this || !string) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	buf = malloc(utf16_utf8_strlen(string) + 1);
	if (!buf) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	pos = buf;
	utf16_utf8_strcpy(&pos, string);
	fputs(stdout, buf);
	free(buf);

	/*
	 * Update the cursor position.
	 *
	 * The UEFI spec provides advance rules for U+0000, U+0008, U+000A,
	 * and U000D. All other characters, including control characters
	 * U+0007 (BEL) and U+0009 (TAB), have to increase the column by one.
	 */
	for (p = string; *p; ++p) {
		switch (*p) {
		case '\b':	/* U+0008, backspace */
			con->cursor_column = max(0, con->cursor_column - 1);
			break;
		case '\n':	/* U+000A, newline */
			con->cursor_column = 0;
			con->cursor_row++;
			break;
		case '\r':	/* U+000D, carriage-return */
			con->cursor_column = 0;
			break;
		case 0xd800 ... 0xdbff:
			/*
			 * Ignore high surrogates, we do not want to count a
			 * Unicode character twice.
			 */
			break;
		default:
			con->cursor_column++;
			break;
		}
		if (con->cursor_column >= mode->columns) {
			con->cursor_column = 0;
			con->cursor_row++;
		}
		con->cursor_row = min(con->cursor_row, (s32)mode->rows - 1);
	}

out:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_cout_test_string(
			struct efi_simple_text_output_protocol *this,
			const efi_string_t string)
{
	EFI_ENTRY("%p, %p", this, string);
	return EFI_EXIT(EFI_SUCCESS);
}

static bool cout_mode_matches(struct cout_mode *mode, int rows, int cols)
{
	if (!mode->present)
		return false;

	return (mode->rows == rows) && (mode->columns == cols);
}

/**
 * query_console_serial() - query console size
 *
 * @rows	pointer to return number of rows
 * @columns	pointer to return number of columns
 * Returns	0 on success
 */
static int query_console_serial(int *rows, int *cols)
{
	int ret = 0;
	int n[2];

	/* Empty input buffer */
	while (tstc())
		getc();

	/*
	 * Not all terminals understand CSI [18t for querying the console size.
	 * We should adhere to escape sequences documented in the console_codes
	 * man page and the ECMA-48 standard.
	 *
	 * So here we follow a different approach. We position the cursor to the
	 * bottom right and query its position. Before leaving the function we
	 * restore the original cursor position.
	 */
	printf(ESC "7"		/* Save cursor position */
	       ESC "[r"		/* Set scrolling region to full window */
	       ESC "[999;999H"	/* Move to bottom right corner */
	       ESC "[6n");	/* Query cursor position */

	/* Read {rows,cols} */
	if (term_read_reply(n, 2, 'R')) {
		ret = 1;
		goto out;
	}

	*cols = n[1];
	*rows = n[0];
out:
	printf(ESC "8");	/* Restore cursor position */
	return ret;
}

/*
 * Update the mode table.
 *
 * By default the only mode available is 80x25. If the console has at least 50
 * lines, enable mode 80x50. If we can query the console size and it is neither
 * 80x25 nor 80x50, set it as an additional mode.
 */
static void query_console_size(void)
{
	const char *stdout_name = env_get("stdout");
	int rows = 25, cols = 80;

	if (stdout_name && !strcmp(stdout_name, "vidconsole") &&
	    IS_ENABLED(CONFIG_DM_VIDEO)) {
		struct stdio_dev *stdout_dev =
			stdio_get_by_name("vidconsole");
		struct udevice *dev = stdout_dev->priv;
		struct vidconsole_priv *priv =
			dev_get_uclass_priv(dev);
		rows = priv->rows;
		cols = priv->cols;
	} else if (query_console_serial(&rows, &cols)) {
		return;
	}

	/* Test if we can have Mode 1 */
	if (cols >= 80 && rows >= 50) {
		efi_cout_modes[1].present = 1;
		efi_con_mode.max_mode = 2;
	}

	/*
	 * Install our mode as mode 2 if it is different
	 * than mode 0 or 1 and set it as the currently selected mode
	 */
	if (!cout_mode_matches(&efi_cout_modes[0], rows, cols) &&
	    !cout_mode_matches(&efi_cout_modes[1], rows, cols)) {
		efi_cout_modes[EFI_COUT_MODE_2].columns = cols;
		efi_cout_modes[EFI_COUT_MODE_2].rows = rows;
		efi_cout_modes[EFI_COUT_MODE_2].present = 1;
		efi_con_mode.max_mode = EFI_MAX_COUT_MODE;
		efi_con_mode.mode = EFI_COUT_MODE_2;
	}
}

static efi_status_t EFIAPI efi_cout_query_mode(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number, unsigned long *columns,
			unsigned long *rows)
{
	EFI_ENTRY("%p, %ld, %p, %p", this, mode_number, columns, rows);

	if (mode_number >= efi_con_mode.max_mode)
		return EFI_EXIT(EFI_UNSUPPORTED);

	if (efi_cout_modes[mode_number].present != 1)
		return EFI_EXIT(EFI_UNSUPPORTED);

	if (columns)
		*columns = efi_cout_modes[mode_number].columns;
	if (rows)
		*rows = efi_cout_modes[mode_number].rows;

	return EFI_EXIT(EFI_SUCCESS);
}

static const struct {
	unsigned int fg;
	unsigned int bg;
} color[] = {
	{ 30, 40 },     /* 0: black */
	{ 34, 44 },     /* 1: blue */
	{ 32, 42 },     /* 2: green */
	{ 36, 46 },     /* 3: cyan */
	{ 31, 41 },     /* 4: red */
	{ 35, 45 },     /* 5: magenta */
	{ 33, 43 },     /* 6: brown, map to yellow as EDK2 does*/
	{ 37, 47 },     /* 7: light gray, map to white */
};

/* See EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetAttribute(). */
static efi_status_t EFIAPI efi_cout_set_attribute(
			struct efi_simple_text_output_protocol *this,
			unsigned long attribute)
{
	unsigned int bold = EFI_ATTR_BOLD(attribute);
	unsigned int fg = EFI_ATTR_FG(attribute);
	unsigned int bg = EFI_ATTR_BG(attribute);

	EFI_ENTRY("%p, %lx", this, attribute);

	efi_con_mode.attribute = attribute;
	if (attribute)
		printf(ESC"[%u;%u;%um", bold, color[fg].fg, color[bg].bg);
	else
		printf(ESC"[0;37;40m");

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_clear_screen(
			struct efi_simple_text_output_protocol *this)
{
	EFI_ENTRY("%p", this);

	printf(ESC"[2J");
	efi_con_mode.cursor_column = 0;
	efi_con_mode.cursor_row = 0;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_set_mode(
			struct efi_simple_text_output_protocol *this,
			unsigned long mode_number)
{
	EFI_ENTRY("%p, %ld", this, mode_number);

	if (mode_number >= efi_con_mode.max_mode)
		return EFI_EXIT(EFI_UNSUPPORTED);
	efi_con_mode.mode = mode_number;
	EFI_CALL(efi_cout_clear_screen(this));

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_reset(
			struct efi_simple_text_output_protocol *this,
			char extended_verification)
{
	EFI_ENTRY("%p, %d", this, extended_verification);

	/* Clear screen */
	EFI_CALL(efi_cout_clear_screen(this));
	/* Set default colors */
	efi_con_mode.attribute = 0x07;
	printf(ESC "[0;37;40m");

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_cout_set_cursor_position(
			struct efi_simple_text_output_protocol *this,
			unsigned long column, unsigned long row)
{
	efi_status_t ret = EFI_SUCCESS;
	struct simple_text_output_mode *con = &efi_con_mode;
	struct cout_mode *mode = &efi_cout_modes[con->mode];

	EFI_ENTRY("%p, %ld, %ld", this, column, row);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (row >= mode->rows || column >= mode->columns) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	/*
	 * Set cursor position by sending CSI H.
	 * EFI origin is [0, 0], terminal origin is [1, 1].
	 */
	printf(ESC "[%d;%dH", (int)row + 1, (int)column + 1);
	efi_con_mode.cursor_column = column;
	efi_con_mode.cursor_row = row;
out:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_cout_enable_cursor(
			struct efi_simple_text_output_protocol *this,
			bool enable)
{
	EFI_ENTRY("%p, %d", this, enable);

	printf(ESC"[?25%c", enable ? 'h' : 'l');
	efi_con_mode.cursor_visible = !!enable;

	return EFI_EXIT(EFI_SUCCESS);
}

struct efi_simple_text_output_protocol efi_con_out = {
	.reset = efi_cout_reset,
	.output_string = efi_cout_output_string,
	.test_string = efi_cout_test_string,
	.query_mode = efi_cout_query_mode,
	.set_mode = efi_cout_set_mode,
	.set_attribute = efi_cout_set_attribute,
	.clear_screen = efi_cout_clear_screen,
	.set_cursor_position = efi_cout_set_cursor_position,
	.enable_cursor = efi_cout_enable_cursor,
	.mode = (void*)&efi_con_mode,
};

/**
 * struct efi_cin_notify_function - registered console input notify function
 *
 * @link:	link to list
 * @data:	key to notify
 * @function:	function to call
 */
struct efi_cin_notify_function {
	struct list_head link;
	struct efi_key_data key;
	efi_status_t (EFIAPI *function)
		(struct efi_key_data *key_data);
};

static bool key_available;
static struct efi_key_data next_key;
static LIST_HEAD(cin_notify_functions);

/**
 * set_shift_mask() - set shift mask
 *
 * @mod:	Xterm shift mask
 */
void set_shift_mask(int mod, struct efi_key_state *key_state)
{
	key_state->key_shift_state = EFI_SHIFT_STATE_VALID;
	if (mod) {
		--mod;
		if (mod & 1)
			key_state->key_shift_state |= EFI_LEFT_SHIFT_PRESSED;
		if (mod & 2)
			key_state->key_shift_state |= EFI_LEFT_ALT_PRESSED;
		if (mod & 4)
			key_state->key_shift_state |= EFI_LEFT_CONTROL_PRESSED;
		if (!mod || (mod & 8))
			key_state->key_shift_state |= EFI_LEFT_LOGO_PRESSED;
	}
}

/**
 * analyze_modifiers() - analyze modifiers (shift, alt, ctrl) for function keys
 *
 * This gets called when we have already parsed CSI.
 *
 * @modifiers:  bit mask (shift, alt, ctrl)
 * @return:	the unmodified code
 */
static int analyze_modifiers(struct efi_key_state *key_state)
{
	int c, mod = 0, ret = 0;

	c = getc();

	if (c != ';') {
		ret = c;
		if (c == '~')
			goto out;
		c = getc();
	}
	for (;;) {
		switch (c) {
		case '0'...'9':
			mod *= 10;
			mod += c - '0';
		/* fall through */
		case ';':
			c = getc();
			break;
		default:
			goto out;
		}
	}
out:
	set_shift_mask(mod, key_state);
	if (!ret)
		ret = c;
	return ret;
}

/**
 * efi_cin_read_key() - read a key from the console input
 *
 * @key:	- key received
 * Return:	- status code
 */
static efi_status_t efi_cin_read_key(struct efi_key_data *key)
{
	struct efi_input_key pressed_key = {
		.scan_code = 0,
		.unicode_char = 0,
	};
	s32 ch;

	if (console_read_unicode(&ch))
		return EFI_NOT_READY;

	key->key_state.key_shift_state = EFI_SHIFT_STATE_INVALID;
	key->key_state.key_toggle_state = EFI_TOGGLE_STATE_INVALID;

	/* We do not support multi-word codes */
	if (ch >= 0x10000)
		ch = '?';

	switch (ch) {
	case 0x1b:
		/*
		 * Xterm Control Sequences
		 * https://www.xfree86.org/4.8.0/ctlseqs.html
		 */
		ch = getc();
		switch (ch) {
		case cESC: /* ESC */
			pressed_key.scan_code = 23;
			break;
		case 'O': /* F1 - F4, End */
			ch = getc();
			/* consider modifiers */
			if (ch == 'F') { /* End */
				pressed_key.scan_code = 6;
				break;
			} else if (ch < 'P') {
				set_shift_mask(ch - '0', &key->key_state);
				ch = getc();
			}
			pressed_key.scan_code = ch - 'P' + 11;
			break;
		case '[':
			ch = getc();
			switch (ch) {
			case 'A'...'D': /* up, down right, left */
				pressed_key.scan_code = ch - 'A' + 1;
				break;
			case 'F': /* End */
				pressed_key.scan_code = 6;
				break;
			case 'H': /* Home */
				pressed_key.scan_code = 5;
				break;
			case '1':
				ch = analyze_modifiers(&key->key_state);
				switch (ch) {
				case '1'...'5': /* F1 - F5 */
					pressed_key.scan_code = ch - '1' + 11;
					break;
				case '6'...'9': /* F5 - F8 */
					pressed_key.scan_code = ch - '6' + 15;
					break;
				case 'A'...'D': /* up, down right, left */
					pressed_key.scan_code = ch - 'A' + 1;
					break;
				case 'F': /* End */
					pressed_key.scan_code = 6;
					break;
				case 'H': /* Home */
					pressed_key.scan_code = 5;
					break;
				case '~': /* Home */
					pressed_key.scan_code = 5;
					break;
				}
				break;
			case '2':
				ch = analyze_modifiers(&key->key_state);
				switch (ch) {
				case '0'...'1': /* F9 - F10 */
					pressed_key.scan_code = ch - '0' + 19;
					break;
				case '3'...'4': /* F11 - F12 */
					pressed_key.scan_code = ch - '3' + 21;
					break;
				case '~': /* INS */
					pressed_key.scan_code = 7;
					break;
				}
				break;
			case '3': /* DEL */
				pressed_key.scan_code = 8;
				analyze_modifiers(&key->key_state);
				break;
			case '5': /* PG UP */
				pressed_key.scan_code = 9;
				analyze_modifiers(&key->key_state);
				break;
			case '6': /* PG DOWN */
				pressed_key.scan_code = 10;
				analyze_modifiers(&key->key_state);
				break;
			} /* [ */
			break;
		default:
			/* ALT key */
			set_shift_mask(3, &key->key_state);
		}
		break;
	case 0x7f:
		/* Backspace */
		ch = 0x08;
	}
	if (pressed_key.scan_code) {
		key->key_state.key_shift_state |= EFI_SHIFT_STATE_VALID;
	} else {
		pressed_key.unicode_char = ch;

		/*
		 * Assume left control key for control characters typically
		 * entered using the control key.
		 */
		if (ch >= 0x01 && ch <= 0x1f) {
			key->key_state.key_shift_state |=
					EFI_SHIFT_STATE_VALID;
			switch (ch) {
			case 0x01 ... 0x07:
			case 0x0b ... 0x0c:
			case 0x0e ... 0x1f:
				key->key_state.key_shift_state |=
						EFI_LEFT_CONTROL_PRESSED;
			}
		}
	}
	key->key = pressed_key;

	return EFI_SUCCESS;
}

/**
 * efi_cin_notify() - notify registered functions
 */
static void efi_cin_notify(void)
{
	struct efi_cin_notify_function *item;

	list_for_each_entry(item, &cin_notify_functions, link) {
		bool match = true;

		/* We do not support toggle states */
		if (item->key.key.unicode_char || item->key.key.scan_code) {
			if (item->key.key.unicode_char !=
			    next_key.key.unicode_char ||
			    item->key.key.scan_code != next_key.key.scan_code)
				match = false;
		}
		if (item->key.key_state.key_shift_state &&
		    item->key.key_state.key_shift_state !=
		    next_key.key_state.key_shift_state)
			match = false;

		if (match)
			/* We don't bother about the return code */
			EFI_CALL(item->function(&next_key));
	}
}

/**
 * efi_cin_check() - check if keyboard input is available
 */
static void efi_cin_check(void)
{
	efi_status_t ret;

	if (key_available) {
		efi_signal_event(efi_con_in.wait_for_key);
		return;
	}

	if (tstc()) {
		ret = efi_cin_read_key(&next_key);
		if (ret == EFI_SUCCESS) {
			key_available = true;

			/* Notify registered functions */
			efi_cin_notify();

			/* Queue the wait for key event */
			if (key_available)
				efi_signal_event(efi_con_in.wait_for_key);
		}
	}
}

/**
 * efi_cin_empty_buffer() - empty input buffer
 */
static void efi_cin_empty_buffer(void)
{
	while (tstc())
		getc();
	key_available = false;
}

/**
 * efi_cin_reset_ex() - reset console input
 *
 * @this:			- the extended simple text input protocol
 * @extended_verification:	- extended verification
 *
 * This function implements the reset service of the
 * EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: old value of the task priority level
 */
static efi_status_t EFIAPI efi_cin_reset_ex(
		struct efi_simple_text_input_ex_protocol *this,
		bool extended_verification)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %d", this, extended_verification);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	efi_cin_empty_buffer();
out:
	return EFI_EXIT(ret);
}

/**
 * efi_cin_read_key_stroke_ex() - read key stroke
 *
 * @this:	instance of the EFI_SIMPLE_TEXT_INPUT_PROTOCOL
 * @key_data:	key read from console
 * Return:	status code
 *
 * This function implements the ReadKeyStrokeEx service of the
 * EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static efi_status_t EFIAPI efi_cin_read_key_stroke_ex(
		struct efi_simple_text_input_ex_protocol *this,
		struct efi_key_data *key_data)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p", this, key_data);

	/* Check parameters */
	if (!this || !key_data) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	/* Enable console input after ExitBootServices */
	efi_cin_check();

	if (!key_available) {
		ret = EFI_NOT_READY;
		goto out;
	}
	/*
	 * CTRL+A - CTRL+Z have to be signaled as a - z.
	 * SHIFT+CTRL+A - SHIFT+CTRL+Z have to be signaled as A - Z.
	 */
	switch (next_key.key.unicode_char) {
	case 0x01 ... 0x07:
	case 0x0b ... 0x0c:
	case 0x0e ... 0x1a:
		if (!(next_key.key_state.key_toggle_state &
		      EFI_CAPS_LOCK_ACTIVE) ^
		    !(next_key.key_state.key_shift_state &
		      (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED)))
			next_key.key.unicode_char += 0x40;
		else
			next_key.key.unicode_char += 0x60;
	}
	*key_data = next_key;
	key_available = false;
	efi_con_in.wait_for_key->is_signaled = false;

out:
	return EFI_EXIT(ret);
}

/**
 * efi_cin_set_state() - set toggle key state
 *
 * @this:		instance of the EFI_SIMPLE_TEXT_INPUT_PROTOCOL
 * @key_toggle_state:	pointer to key toggle state
 * Return:		status code
 *
 * This function implements the SetState service of the
 * EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static efi_status_t EFIAPI efi_cin_set_state(
		struct efi_simple_text_input_ex_protocol *this,
		u8 *key_toggle_state)
{
	EFI_ENTRY("%p, %p", this, key_toggle_state);
	/*
	 * U-Boot supports multiple console input sources like serial and
	 * net console for which a key toggle state cannot be set at all.
	 *
	 * According to the UEFI specification it is allowable to not implement
	 * this service.
	 */
	return EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_cin_register_key_notify() - register key notification function
 *
 * @this:			instance of the EFI_SIMPLE_TEXT_INPUT_PROTOCOL
 * @key_data:			key to be notified
 * @key_notify_function:	function to be called if the key is pressed
 * @notify_handle:		handle for unregistering the notification
 * Return:			status code
 *
 * This function implements the SetState service of the
 * EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static efi_status_t EFIAPI efi_cin_register_key_notify(
		struct efi_simple_text_input_ex_protocol *this,
		struct efi_key_data *key_data,
		efi_status_t (EFIAPI *key_notify_function)(
			struct efi_key_data *key_data),
		void **notify_handle)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_cin_notify_function *notify_function;

	EFI_ENTRY("%p, %p, %p, %p",
		  this, key_data, key_notify_function, notify_handle);

	/* Check parameters */
	if (!this || !key_data || !key_notify_function || !notify_handle) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	EFI_PRINT("u+%04x, sc %04x, sh %08x, tg %02x\n",
		  key_data->key.unicode_char,
	       key_data->key.scan_code,
	       key_data->key_state.key_shift_state,
	       key_data->key_state.key_toggle_state);

	notify_function = calloc(1, sizeof(struct efi_cin_notify_function));
	if (!notify_function) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	notify_function->key = *key_data;
	notify_function->function = key_notify_function;
	list_add_tail(&notify_function->link, &cin_notify_functions);
	*notify_handle = notify_function;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_cin_unregister_key_notify() - unregister key notification function
 *
 * @this:			instance of the EFI_SIMPLE_TEXT_INPUT_PROTOCOL
 * @notification_handle:	handle received when registering
 * Return:			status code
 *
 * This function implements the SetState service of the
 * EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static efi_status_t EFIAPI efi_cin_unregister_key_notify(
		struct efi_simple_text_input_ex_protocol *this,
		void *notification_handle)
{
	efi_status_t ret = EFI_INVALID_PARAMETER;
	struct efi_cin_notify_function *item, *notify_function =
			notification_handle;

	EFI_ENTRY("%p, %p", this, notification_handle);

	/* Check parameters */
	if (!this || !notification_handle)
		goto out;

	list_for_each_entry(item, &cin_notify_functions, link) {
		if (item == notify_function) {
			ret = EFI_SUCCESS;
			break;
		}
	}
	if (ret != EFI_SUCCESS)
		goto out;

	/* Remove the notify function */
	list_del(&notify_function->link);
	free(notify_function);
out:
	return EFI_EXIT(ret);
}


/**
 * efi_cin_reset() - drain the input buffer
 *
 * @this:			instance of the EFI_SIMPLE_TEXT_INPUT_PROTOCOL
 * @extended_verification:	allow for exhaustive verification
 * Return:			status code
 *
 * This function implements the Reset service of the
 * EFI_SIMPLE_TEXT_INPUT_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static efi_status_t EFIAPI efi_cin_reset
			(struct efi_simple_text_input_protocol *this,
			 bool extended_verification)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %d", this, extended_verification);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	efi_cin_empty_buffer();
out:
	return EFI_EXIT(ret);
}

/**
 * efi_cin_read_key_stroke() - read key stroke
 *
 * @this:	instance of the EFI_SIMPLE_TEXT_INPUT_PROTOCOL
 * @key:	key read from console
 * Return:	status code
 *
 * This function implements the ReadKeyStroke service of the
 * EFI_SIMPLE_TEXT_INPUT_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 */
static efi_status_t EFIAPI efi_cin_read_key_stroke
			(struct efi_simple_text_input_protocol *this,
			 struct efi_input_key *key)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p", this, key);

	/* Check parameters */
	if (!this || !key) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	/* Enable console input after ExitBootServices */
	efi_cin_check();

	if (!key_available) {
		ret = EFI_NOT_READY;
		goto out;
	}
	*key = next_key.key;
	key_available = false;
	efi_con_in.wait_for_key->is_signaled = false;
out:
	return EFI_EXIT(ret);
}

static struct efi_simple_text_input_ex_protocol efi_con_in_ex = {
	.reset = efi_cin_reset_ex,
	.read_key_stroke_ex = efi_cin_read_key_stroke_ex,
	.wait_for_key_ex = NULL,
	.set_state = efi_cin_set_state,
	.register_key_notify = efi_cin_register_key_notify,
	.unregister_key_notify = efi_cin_unregister_key_notify,
};

struct efi_simple_text_input_protocol efi_con_in = {
	.reset = efi_cin_reset,
	.read_key_stroke = efi_cin_read_key_stroke,
	.wait_for_key = NULL,
};

static struct efi_event *console_timer_event;

/*
 * efi_console_timer_notify() - notify the console timer event
 *
 * @event:	console timer event
 * @context:	not used
 */
static void EFIAPI efi_console_timer_notify(struct efi_event *event,
					    void *context)
{
	EFI_ENTRY("%p, %p", event, context);
	efi_cin_check();
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_key_notify() - notify the wait for key event
 *
 * @event:	wait for key event
 * @context:	not used
 */
static void EFIAPI efi_key_notify(struct efi_event *event, void *context)
{
	EFI_ENTRY("%p, %p", event, context);
	efi_cin_check();
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_console_register() - install the console protocols
 *
 * This function is called from do_bootefi_exec().
 *
 * Return:	status code
 */
efi_status_t efi_console_register(void)
{
	efi_status_t r;
	efi_handle_t console_output_handle;
	efi_handle_t console_input_handle;

	/* Set up mode information */
	query_console_size();

	/* Create handles */
	r = efi_create_handle(&console_output_handle);
	if (r != EFI_SUCCESS)
		goto out_of_memory;

	r = efi_add_protocol(console_output_handle,
			     &efi_guid_text_output_protocol, &efi_con_out);
	if (r != EFI_SUCCESS)
		goto out_of_memory;
	systab.con_out_handle = console_output_handle;
	systab.stderr_handle = console_output_handle;

	r = efi_create_handle(&console_input_handle);
	if (r != EFI_SUCCESS)
		goto out_of_memory;

	r = efi_add_protocol(console_input_handle,
			     &efi_guid_text_input_protocol, &efi_con_in);
	if (r != EFI_SUCCESS)
		goto out_of_memory;
	systab.con_in_handle = console_input_handle;
	r = efi_add_protocol(console_input_handle,
			     &efi_guid_text_input_ex_protocol, &efi_con_in_ex);
	if (r != EFI_SUCCESS)
		goto out_of_memory;

	/* Create console events */
	r = efi_create_event(EVT_NOTIFY_WAIT, TPL_CALLBACK, efi_key_notify,
			     NULL, NULL, &efi_con_in.wait_for_key);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register WaitForKey event\n");
		return r;
	}
	efi_con_in_ex.wait_for_key_ex = efi_con_in.wait_for_key;
	r = efi_create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
			     efi_console_timer_notify, NULL, NULL,
			     &console_timer_event);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register console event\n");
		return r;
	}
	/* 5000 ns cycle is sufficient for 2 MBaud */
	r = efi_set_timer(console_timer_event, EFI_TIMER_PERIODIC, 50);
	if (r != EFI_SUCCESS)
		printf("ERROR: Failed to set console timer\n");
	return r;
out_of_memory:
	printf("ERROR: Out of memory\n");
	return r;
}
