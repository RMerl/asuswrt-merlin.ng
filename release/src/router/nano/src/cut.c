/**************************************************************************
 *   cut.c  --  This file is part of GNU nano.                            *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2020 Free Software Foundation, Inc.    *
 *   Copyright (C) 2014 Mark Majeres                                      *
 *   Copyright (C) 2016, 2018, 2019 Benno Schulenberg                     *
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

#include <string.h>

/* Delete the character under the cursor. */
void do_deletion(undo_type action)
{
#ifndef NANO_TINY
	size_t old_amount = 0;
#endif

	openfile->placewewant = xplustabs();

	/* When in the middle of a line, delete the current character. */
	if (openfile->current->data[openfile->current_x] != '\0') {
		int charlen = char_length(openfile->current->data + openfile->current_x);
		size_t line_len = strlen(openfile->current->data + openfile->current_x);
#ifndef NANO_TINY
		/* If the type of action changed or the cursor moved to a different
		 * line, create a new undo item, otherwise update the existing item. */
		if (action != openfile->last_action ||
					openfile->current->lineno != openfile->current_undo->head_lineno)
			add_undo(action, NULL);
		else
			update_undo(action);

		if (ISSET(SOFTWRAP))
			old_amount = number_of_chunks_in(openfile->current);
#endif
		/* Move the remainder of the line "in", over the current character. */
		memmove(&openfile->current->data[openfile->current_x],
					&openfile->current->data[openfile->current_x + charlen],
					line_len - charlen + 1);
#ifndef NANO_TINY
		/* Adjust the mark if it is after the cursor on the current line. */
		if (openfile->mark == openfile->current &&
								openfile->mark_x > openfile->current_x)
			openfile->mark_x -= charlen;
#endif
	/* Otherwise, when not at end of buffer, join this line with the next. */
	} else if (openfile->current != openfile->filebot) {
		linestruct *joining = openfile->current->next;

		/* If there is a magic line, and we're before it: don't eat it. */
		if (joining == openfile->filebot && openfile->current_x != 0 &&
				!ISSET(NO_NEWLINES)) {
#ifndef NANO_TINY
			if (action == BACK)
				add_undo(BACK, NULL);
#endif
			return;
		}

#ifndef NANO_TINY
		add_undo(action, NULL);
#endif
		/* Add the contents of the next line to those of the current one. */
		openfile->current->data = charealloc(openfile->current->data,
				strlen(openfile->current->data) + strlen(joining->data) + 1);
		strcat(openfile->current->data, joining->data);

#ifndef NANO_TINY
		/* Adjust the mark if it was on the line that was "eaten". */
		if (openfile->mark == joining) {
			openfile->mark = openfile->current;
			openfile->mark_x += openfile->current_x;
		}
#endif
		unlink_node(joining);
		renumber_from(openfile->current);

		/* Two lines were joined, so we need to refresh the screen. */
		refresh_needed = TRUE;
	} else
		/* We're at the end-of-file: nothing to do. */
		return;

	/* Adjust the file size, and remember it for a possible redo. */
	openfile->totsize--;
#ifndef NANO_TINY
	openfile->current_undo->newsize = openfile->totsize;

	/* If the number of screen rows that a softwrapped line occupies
	 * has changed, we need a full refresh. */
	if (ISSET(SOFTWRAP) && refresh_needed == FALSE &&
				number_of_chunks_in(openfile->current) != old_amount)
		refresh_needed = TRUE;
#endif

	set_modified();
}

/* Delete the character under the cursor. */
void do_delete(void)
{
#ifndef NANO_TINY
	if (openfile->mark && ISSET(LET_THEM_ZAP))
		zap_text();
	else
#endif
		do_deletion(DEL);
}

/* Backspace over one character.  That is, move the cursor left one
 * character, and then delete the character under the cursor. */
void do_backspace(void)
{
#ifndef NANO_TINY
	if (openfile->mark && ISSET(LET_THEM_ZAP))
		zap_text();
	else
#endif
	if (openfile->current_x > 0 || openfile->current != openfile->filetop) {
		do_left();
		do_deletion(BACK);
	}
}

/* Return FALSE when a cut command would not actually cut anything: when
 * on an empty line at EOF, or when the mark covers zero characters, or
 * (when test_cliff is TRUE) when the magic line would be cut. */
bool is_cuttable(bool test_cliff)
{
	size_t from = (test_cliff) ? openfile->current_x : 0;

	if ((openfile->current->next == NULL && openfile->current->data[from] == '\0'
#ifndef NANO_TINY
					&& openfile->mark == NULL) ||
					(openfile->mark == openfile->current &&
					openfile->mark_x == openfile->current_x) ||
					(from > 0 && !ISSET(NO_NEWLINES) &&
					openfile->current->data[from] == '\0' &&
					openfile->current->next == openfile->filebot
#endif
					)) {
#ifndef NANO_TINY
		statusbar(_("Nothing was cut"));
		openfile->mark = NULL;
#endif
		return FALSE;
	} else
		return TRUE;
}

#ifndef NANO_TINY
/* Delete text from the cursor until the first start of a word to
 * the left, or to the right when forward is TRUE. */
void chop_word(bool forward)
{
	/* Remember the current cursor position. */
	linestruct *is_current = openfile->current;
	size_t is_current_x = openfile->current_x;
	/* Remember where the cutbuffer is, then make it seem blank. */
	linestruct *is_cutbuffer = cutbuffer;

	cutbuffer = NULL;

	/* Move the cursor to a word start, to the left or to the right.
	 * If that word is on another line and the cursor was not already
	 * on the edge of the original line, then put the cursor on that
	 * edge instead, so that lines will not be joined unexpectedly. */
	if (!forward) {
		do_prev_word(ISSET(WORD_BOUNDS));
		if (openfile->current != is_current) {
			if (is_current_x > 0) {
				openfile->current = is_current;
				openfile->current_x = 0;
			} else
				openfile->current_x = strlen(openfile->current->data);
		}
	} else {
		do_next_word(FALSE, ISSET(WORD_BOUNDS));
		if (openfile->current != is_current &&
							is_current->data[is_current_x] != '\0') {
			openfile->current = is_current;
			openfile->current_x = strlen(is_current->data);
		}
	}

	/* Set the mark at the start of that word. */
	openfile->mark = openfile->current;
	openfile->mark_x = openfile->current_x;

	/* Put the cursor back where it was, so an undo will put it there too. */
	openfile->current = is_current;
	openfile->current_x = is_current_x;

	/* Now kill the marked region and a word is gone. */
	add_undo(CUT, NULL);
	do_snip(FALSE, TRUE, FALSE, FALSE);
	update_undo(CUT);

	/* Discard the cut word and restore the cutbuffer. */
	free_lines(cutbuffer);
	cutbuffer = is_cutbuffer;
}

/* Delete a word leftward. */
void chop_previous_word(void)
{
	if (openfile->current->prev == NULL && openfile->current_x == 0)
		statusbar(_("Nothing was cut"));
	else
		chop_word(BACKWARD);
}

/* Delete a word rightward. */
void chop_next_word(void)
{
	openfile->mark = NULL;

	if (is_cuttable(TRUE))
		chop_word(FORWARD);
}
#endif /* !NANO_TINY */

/* Move all text between (top, top_x) and (bot, bot_x) from the current buffer
 * into the cutbuffer. */
void extract_segment(linestruct *top, size_t top_x, linestruct *bot, size_t bot_x)
{
	bool edittop_inside = (openfile->edittop->lineno >= top->lineno &&
							openfile->edittop->lineno <= bot->lineno);
#ifndef NANO_TINY
	bool mark_inside = (openfile->mark &&
						openfile->mark->lineno >= top->lineno &&
						openfile->mark->lineno <= bot->lineno &&
						(openfile->mark != top || openfile->mark_x >= top_x) &&
						(openfile->mark != bot || openfile->mark_x <= bot_x));
	bool same_line = (openfile->mark == top);

	if (top == bot && top_x == bot_x)
		return;
#endif

	/* Reduce the buffer to cover just the text that needs to be extracted. */
	partition_buffer(top, top_x, bot, bot_x);

	/* Subtract the number of characters in that text from the file size. */
	openfile->totsize -= get_totsize(top, bot);

	/* If the cutbuffer is currently empty, just move all the text directly
	 * into it; otherwise, append the text to what is already there. */
	if (cutbuffer == NULL) {
		cutbuffer = openfile->filetop;
		cutbottom = openfile->filebot;
	} else {
		/* Tack the data of the first line of the text onto the data of
		 * the last line in the given buffer. */
		cutbottom->data = charealloc(cutbottom->data,
								strlen(cutbottom->data) +
								strlen(openfile->filetop->data) + 1);
		strcat(cutbottom->data, openfile->filetop->data);

		/* Attach the second line of the text (if any) to the last line
		 * of the buffer, then remove the now superfluous first line. */
		cutbottom->next = openfile->filetop->next;
		delete_node(openfile->filetop);

		/* If there is a second line, make the reverse attachment too and
		 * update the buffer pointer to point at the end of the text. */
		if (cutbottom->next != NULL) {
			cutbottom->next->prev = cutbottom;
			cutbottom = openfile->filebot;
		}
	}

	/* Since the text has now been saved, remove it from the file buffer. */
	openfile->filetop = make_new_node(NULL);
	openfile->filetop->data = copy_of("");
	openfile->filebot = openfile->filetop;

	/* Set the cursor at the point where the text was removed. */
	openfile->current = openfile->filetop;
	openfile->current_x = top_x;
#ifndef NANO_TINY
	/* If the mark was inside the partition, put it where the cursor now is. */
	if (mark_inside) {
		openfile->mark = openfile->current;
		openfile->mark_x = openfile->current_x;
	} else if (same_line)
		/* Update the pointer to this partially cut line. */
		openfile->mark = openfile->current;
#endif

	/* Glue the texts before and after the extraction together. */
	unpartition_buffer();

	renumber_from(openfile->current);

	/* If the top of the edit window was inside the old partition, put
	 * it in range of current. */
	if (edittop_inside) {
		adjust_viewport(STATIONARY);
		refresh_needed = TRUE;
	}

	/* If the text doesn't end with a newline, and it should, add one. */
	if (!ISSET(NO_NEWLINES) && openfile->filebot->data[0] != '\0')
		new_magicline();
}

/* Meld the buffer that starts at topline into the current file buffer
 * at the current cursor position. */
void ingraft_buffer(linestruct *topline)
{
	size_t current_x_save = openfile->current_x;
	/* Remember whether the current line is at the top of the edit window. */
	bool edittop_inside = (openfile->edittop == openfile->current);
#ifndef NANO_TINY
	/* Remember whether mark and cursor are on the same line, and their order. */
	bool right_side_up = (openfile->mark && mark_is_before_cursor());
	bool same_line = (openfile->mark == openfile->current);
#endif

	/* Partition the buffer so that it contains no text, then delete it.*/
	partition_buffer(openfile->current, openfile->current_x,
						openfile->current, openfile->current_x);
	delete_node(openfile->filetop);

	/* Replace the current buffer with the passed buffer. */
	openfile->filetop = topline;
	openfile->filebot = topline;
	while (openfile->filebot->next != NULL)
		openfile->filebot = openfile->filebot->next;

	/* Put the cursor at the end of the pasted text. */
	openfile->current = openfile->filebot;
	openfile->current_x = strlen(openfile->filebot->data);

	/* When the pasted stuff contains no newline, adjust the cursor's
	 * x coordinate for the text that is before the pasted stuff. */
	if (openfile->filetop == openfile->filebot)
		openfile->current_x += current_x_save;

#ifndef NANO_TINY
	/* When needed, refresh the mark's pointer and compensate the mark's
	 * x coordinate for the change in the current line. */
	if (same_line) {
		if (!right_side_up) {
			openfile->mark = openfile->filebot;
			openfile->mark_x += openfile->current_x - current_x_save;
		} else
			openfile->mark = openfile->filetop;
	}
#endif

	/* Add the number of characters in the copied text to the file size. */
	openfile->totsize += get_totsize(openfile->filetop, openfile->filebot);

	/* If we pasted onto the first line of the edit window, the corresponding
	 * record has been freed, so... point at the start of the copied text. */
	if (edittop_inside)
		openfile->edittop = openfile->filetop;

	/* Weld the pasted text into the surrounding content of the buffer. */
	unpartition_buffer();

	renumber_from(topline);

	/* If the text doesn't end with a newline, and it should, add one. */
	if (!ISSET(NO_NEWLINES) && openfile->filebot->data[0] != '\0')
		new_magicline();
}

/* Meld a copy of the given buffer into the current file buffer. */
void copy_from_buffer(linestruct *somebuffer)
{
	linestruct *the_copy = copy_buffer(somebuffer);

	ingraft_buffer(the_copy);
}

/* Move the whole current line from the current buffer to the cutbuffer. */
void cut_line(void)
{
	/* When not on the last line of the buffer, move the text from the
	 * head of this line to the head of the next line into the cutbuffer;
	 * otherwise, move all of the text of this line into the cutbuffer. */
	if (openfile->current != openfile->filebot)
		extract_segment(openfile->current, 0, openfile->current->next, 0);
	else
		extract_segment(openfile->current, 0,
				openfile->current, strlen(openfile->current->data));

	openfile->placewewant = 0;
}

#ifndef NANO_TINY
/* Move all text from the cursor position until the end of this line into
 * the cutbuffer.  But when already at the end of a line, then move this
 * "newline" to the cutbuffer. */
void cut_to_eol(void)
{
	size_t data_len = strlen(openfile->current->data);

	/* When not at the end of a line, move the rest of this line into
	 * the cutbuffer.  Otherwise, when not at the end of the buffer,
	 * move the line separation into the cutbuffer. */
	if (openfile->current_x < data_len)
		extract_segment(openfile->current, openfile->current_x,
				openfile->current, data_len);
	else if (openfile->current != openfile->filebot) {
		extract_segment(openfile->current, openfile->current_x,
				openfile->current->next, 0);
		openfile->placewewant = xplustabs();
	}
}

/* Move all marked text from the current buffer into the cutbuffer. */
void cut_marked(bool *right_side_up)
{
	linestruct *top, *bot;
	size_t top_x, bot_x;

	get_region((const linestruct **)&top, &top_x,
				(const linestruct **)&bot, &bot_x, right_side_up);

	extract_segment(top, top_x, bot, bot_x);

	openfile->placewewant = xplustabs();
}

/* Move all text from the cursor position to end-of-file into the cutbuffer. */
void cut_to_eof(void)
{
	extract_segment(openfile->current, openfile->current_x,
				openfile->filebot, strlen(openfile->filebot->data));
}
#endif /* !NANO_TINY */

/* Move text from the current buffer into the cutbuffer.  If
 * copying is TRUE, copy the text back into the buffer afterward.
 * If until_eof is TRUE, move all text from the current cursor
 * position to the end of the file into the cutbuffer.  If append
 * is TRUE (when zapping), always append the cut to the cutbuffer. */
void do_snip(bool copying, bool marked, bool until_eof, bool append)
{
#ifndef NANO_TINY
	linestruct *was_bottom = NULL;
		/* The current end of the cutbuffer, before we add text to it. */
	size_t botlen = 0;
		/* The length of the string at the current end of the cutbuffer,
		 * before we add text to it. */
	bool using_magicline = !ISSET(NO_NEWLINES);
		/* Whether an automatic newline should be added at end-of-buffer. */
	bool right_side_up = TRUE;
		/* There *is* no region, *or* it is marked forward. */
#endif
	static bool precedent = FALSE;
		/* Whether the previous operation was a copying operation. */

	/* If cuts were not continuous, or when cutting a region, clear the slate. */
	if ((!keep_cutbuffer || marked || until_eof || copying != precedent) &&
				!append) {
		free_lines(cutbuffer);
		cutbuffer = NULL;
	}

	/* After a line operation, future ones should add to the cutbuffer. */
	keep_cutbuffer = !marked && !until_eof;
	precedent = copying;

#ifndef NANO_TINY
	if (copying) {
		/* If the cutbuffer isn't empty, remember where it currently ends. */
		if (cutbuffer != NULL) {
			was_bottom = cutbottom;
			botlen = strlen(cutbottom->data);
		}
		/* Don't add a magic line when moving text to the cutbuffer. */
		SET(NO_NEWLINES);
	}

	/* Now move the relevant piece of text into the cutbuffer. */
	if (until_eof)
		cut_to_eof();
	else if (openfile->mark) {
		cut_marked(&right_side_up);
		openfile->mark = NULL;
	} else if (ISSET(CUT_FROM_CURSOR))
		cut_to_eol();
	else
#endif
		cut_line();

#ifndef NANO_TINY
	if (copying) {
		/* Copy the text that was put into the cutbuffer back into the current
		 * file buffer, so that in the end nothing has been deleted. */
		if (cutbuffer != NULL) {
			if (was_bottom != NULL) {
				was_bottom->data += botlen;
				copy_from_buffer(was_bottom);
				was_bottom->data -= botlen;
			} else
				copy_from_buffer(cutbuffer);

			/* If the copied region was marked forward, put the new desired
			 * x position at its end; otherwise, leave it at its beginning. */
			if (right_side_up)
				openfile->placewewant = xplustabs();
		}
		/* Restore the magic-line behavior now that we're done fiddling. */
		if (using_magicline)
			UNSET(NO_NEWLINES);
	} else
#endif
		set_modified();

	refresh_needed = TRUE;
}

/* Move text from the current buffer into the cutbuffer. */
void cut_text(void)
{
#ifndef NANO_TINY
	if (!is_cuttable(ISSET(CUT_FROM_CURSOR) && openfile->mark == NULL))
		return;

	/* Only add a new undo item when the current item is not a CUT or when
	 * the current cut is not contiguous with the previous cutting. */
	if (openfile->last_action != CUT || !keep_cutbuffer) {
		keep_cutbuffer = FALSE;
		add_undo(CUT, NULL);
	}

	do_snip(FALSE, openfile->mark != NULL, FALSE, FALSE);

	update_undo(CUT);
#else
	if (is_cuttable(FALSE))
		do_snip(FALSE, FALSE, FALSE, FALSE);
#endif
	wipe_statusbar();
}

#ifndef NANO_TINY
/* Move text from the current buffer into the cutbuffer, and copy it
 * back into the buffer afterward.  If the mark is set or the cursor
 * was moved, blow away previous contents of the cutbuffer. */
void copy_text(void)
{
	bool mark_is_set = (openfile->mark != NULL);

	/* Remember the current viewport and cursor position. */
	ssize_t is_edittop_lineno = openfile->edittop->lineno;
	size_t is_firstcolumn = openfile->firstcolumn;
	ssize_t is_current_lineno = openfile->current->lineno;
	size_t is_current_x = openfile->current_x;

	do_snip(TRUE, mark_is_set, FALSE, FALSE);

	/* If the mark was set, restore the viewport and cursor position. */
	if (mark_is_set) {
		openfile->edittop = line_from_number(is_edittop_lineno);
		openfile->firstcolumn = is_firstcolumn;
		openfile->current = line_from_number(is_current_lineno);
		openfile->current_x = is_current_x;
	} else
		focusing = FALSE;

	openfile->last_action = OTHER;
}

/* Cut from the current cursor position to the end of the file. */
void cut_till_eof(void)
{
	if (openfile->current->data[openfile->current_x] == '\0' &&
				(openfile->current->next == NULL ||
				(!ISSET(NO_NEWLINES) && openfile->current_x > 0 &&
				openfile->current->next == openfile->filebot))) {
		statusbar(_("Nothing was cut"));
		return;
	}

	add_undo(CUT_TO_EOF, NULL);
	do_snip(FALSE, FALSE, TRUE, FALSE);
	update_undo(CUT_TO_EOF);
	wipe_statusbar();
}

/* Erase text (current line or marked region), sending it into oblivion. */
void zap_text(void)
{
	/* Remember the current cutbuffer so it can be restored after the zap. */
	linestruct *was_cutbuffer = cutbuffer;

	if (!is_cuttable(ISSET(CUT_FROM_CURSOR) && openfile->mark == NULL))
		return;

	/* Add a new undo item only when the current item is not a ZAP or when
	 * the current zap is not contiguous with the previous zapping. */
	if (openfile->last_action != ZAP || !keep_cutbuffer)
		add_undo(ZAP, NULL);

	/* Use the cutbuffer from the ZAP undo item, so the cut can be undone. */
	cutbuffer = openfile->current_undo->cutbuffer;

	do_snip(FALSE, openfile->mark != NULL, FALSE, TRUE);

	update_undo(ZAP);
	wipe_statusbar();

	cutbuffer = was_cutbuffer;
}
#endif /* !NANO_TINY */

/* Copy text from the cutbuffer into the current buffer. */
void paste_text(void)
{
	ssize_t was_lineno = openfile->current->lineno;
		/* The line number where we started the paste. */
	size_t was_leftedge = 0;
		/* The leftedge where we started the paste. */

	if (cutbuffer == NULL) {
		statusbar(_("Cutbuffer is empty"));
		return;
	}

#ifndef NANO_TINY
	add_undo(PASTE, NULL);

	if (ISSET(SOFTWRAP))
		was_leftedge = leftedge_for(xplustabs(), openfile->current);
#endif

	/* Add a copy of the text in the cutbuffer to the current buffer
	 * at the current cursor position. */
	copy_from_buffer(cutbuffer);

#ifndef NANO_TINY
	update_undo(PASTE);
#endif

	/* If we pasted less than a screenful, don't center the cursor. */
	if (less_than_a_screenful(was_lineno, was_leftedge))
		focusing = FALSE;

	/* Set the desired x position to where the pasted text ends. */
	openfile->placewewant = xplustabs();

	set_modified();
	wipe_statusbar();
	refresh_needed = TRUE;
}
