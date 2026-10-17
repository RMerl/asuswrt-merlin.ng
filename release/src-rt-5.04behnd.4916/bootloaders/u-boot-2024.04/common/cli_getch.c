// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#include <common.h>
#include <cli.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
enum cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

void cli_ch_init(struct cli_ch_state *cch)
{
	memset(cch, '\0', sizeof(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
static int cli_ch_esc(struct cli_ch_state *cch, int ichar,
		      enum cli_esc_state_t *actp)
{
	enum cli_esc_state_t act = ESC_REJECT;

	switch (cch->esc_len) {
	case 1:
		if (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		else
			act = ESC_CONVERTED;
		break;
	case 2:
		switch (ichar) {
		case 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			break;	/* pass off to ^B handler */
		case 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			break;	/* pass off to ^F handler */
		case 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			break;	/* pass off to ^A handler */
		case 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			break;	/* pass off to ^E handler */
		case 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			break;	/* pass off to ^P handler */
		case 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			break;	/* pass off to ^N handler */
		case '1':
		case '2':
		case '3':
		case '4':
		case '7':
		case '8':
			if (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			break;
		}
		break;
	case 3:
		switch (ichar) {
		case '~':
			switch (cch->esc_save[2]) {
			case '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				break;	/* pass to ^D handler */
			case '1':	/* Home key */
			case '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				break;	/* pass to ^A handler */
			case '4':	/* End key */
			case '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				break;	/* pass to ^E handler */
			}
			break;
		case '0':
			if (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			break;
		}
		break;
	case 4:
		switch (ichar) {
		case '0':
		case '1':
			act = ESC_SAVE;
			break;		/* bracketed paste */
		}
		break;
	case 5:
		if (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	return ichar;
}

int cli_ch_process(struct cli_ch_state *cch, int ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	if (!ichar) {
		if (cch->emitting) {
			if (cch->emit_upto < cch->esc_len)
				return cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		return 0;
	} else if (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		if (cch->esc_len) {
			cch->esc_len = 0;
			return '\e';
		}

		/* Otherwise there is nothing to return */
		return 0;
	}

	if (ichar == '\n' || ichar == '\r')
		return '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	if (cch->esc_len != 0) {
		enum cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		switch (act) {
		case ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			break;
		case ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			return ichar;
		case ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			break;
		}
	}

	if (ichar == '\e') {
		if (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} else {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		return 0;
	}

	return ichar;
}
