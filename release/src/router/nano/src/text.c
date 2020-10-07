/**************************************************************************
 *   text.c  --  This file is part of GNU nano.                           *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2020 Free Software Foundation, Inc.    *
 *   Copyright (C) 2014-2015 Mark Majeres                                 *
 *   Copyright (C) 2016 Mike Scalora                                      *
 *   Copyright (C) 2016 Sumedh Pendurkar                                  *
 *   Copyright (C) 2018 Marco Diego Aurélio Mesquita                      *
 *   Copyright (C) 2015-2020 Benno Schulenberg                            *
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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#if defined(__APPLE__) && !defined(st_mtim)
#define st_mtim  st_mtimespec
#endif

#ifdef ENABLE_WORDCOMPLETION
static int pletion_x = 0;
		/* The x position in pletion_line of the last found completion. */
static completion_word *list_of_completions;
		/* A linked list of the completions that have been attempted. */
#endif

#ifndef NANO_TINY
/* Toggle the mark. */
void do_mark(void)
{
	if (!openfile->mark) {
		openfile->mark = openfile->current;
		openfile->mark_x = openfile->current_x;
		statusbar(_("Mark Set"));
		openfile->kind_of_mark = HARDMARK;
	} else {
		openfile->mark = NULL;
		statusbar(_("Mark Unset"));
		refresh_needed = TRUE;
	}
}
#endif /* !NANO_TINY */

/* Insert a tab.  If the TABS_TO_SPACES flag is set, insert the number
 * of spaces that a tab would normally take up. */
void do_tab(void)
{
#ifdef ENABLE_COLOR
	if (openfile->syntax && openfile->syntax->tab)
		inject(openfile->syntax->tab, strlen(openfile->syntax->tab));
	else
#endif
#ifndef NANO_TINY
	if (ISSET(TABS_TO_SPACES)) {
		char *spaces = charalloc(tabsize + 1);
		size_t length = tabsize - (xplustabs() % tabsize);

		memset(spaces, ' ', length);
		spaces[length] = '\0';

		inject(spaces, length);

		free(spaces);
	} else
#endif
		inject((char *)"\t", 1);
}

#ifndef NANO_TINY
/* Add an indent to the given line. */
void indent_a_line(linestruct *line, char *indentation)
{
	size_t length = strlen(line->data);
	size_t indent_len = strlen(indentation);

	/* If the requested indentation is empty, don't change the line. */
	if (indent_len == 0)
		return;

	/* Add the fabricated indentation to the beginning of the line. */
	line->data = charealloc(line->data, length + indent_len + 1);
	memmove(line->data + indent_len, line->data, length + 1);
	memcpy(line->data, indentation, indent_len);

	openfile->totsize += indent_len;

	if (ISSET(SOFTWRAP))
		line->extrarows = extra_chunks_in(line);

	/* Compensate for the change in the current line. */
	if (line == openfile->mark && openfile->mark_x > 0)
		openfile->mark_x += indent_len;
	if (line == openfile->current && openfile->current_x > 0) {
		openfile->current_x += indent_len;
		openfile->placewewant = xplustabs();
	}
}

/* Indent the current line (or the marked lines) by tabsize columns.
 * This inserts either a tab character or a tab's worth of spaces,
 * depending on whether --tabstospaces is in effect. */
void do_indent(void)
{
	char *indentation;
	linestruct *top, *bot, *line;

	/* Use either all the marked lines or just the current line. */
	get_range(&top, &bot);

	/* Skip any leading empty lines. */
	while (top != bot->next && top->data[0] == '\0')
		top = top->next;

	/* If all lines are empty, there is nothing to do. */
	if (top == bot->next)
		return;

	indentation = charalloc(tabsize + 1);

	/* Set the indentation to either a bunch of spaces or a single tab. */
#ifdef ENABLE_COLOR
	if (openfile->syntax && openfile->syntax->tab)
		indentation = mallocstrcpy(indentation, openfile->syntax->tab);
	else
#endif
	if (ISSET(TABS_TO_SPACES)) {
		memset(indentation, ' ', tabsize);
		indentation[tabsize] = '\0';
	} else {
		indentation[0] = '\t';
		indentation[1] = '\0';
	}

	add_undo(INDENT, NULL);

	/* Go through each of the lines, adding an indent to the non-empty ones,
	 * and recording whatever was added in the undo item. */
	for (line = top; line != bot->next; line = line->next) {
		char *real_indent = (line->data[0] == '\0') ? "" : indentation;

		indent_a_line(line, real_indent);
		update_multiline_undo(line->lineno, real_indent);
	}

	free(indentation);

	set_modified();
	ensure_firstcolumn_is_aligned();
	refresh_needed = TRUE;
	shift_held = TRUE;
}

/* Return the number of bytes of whitespace at the start of the given text,
 * but at most a tab's worth. */
size_t length_of_white(const char *text)
{
	size_t white_count = 0;

#ifdef ENABLE_COLOR
	if (openfile->syntax && openfile->syntax->tab) {
		size_t thelength = strlen(openfile->syntax->tab);

		while (text[white_count] == openfile->syntax->tab[white_count])
			if (++white_count == thelength)
				return thelength;

		white_count = 0;
	}
#endif

	while (TRUE) {
		if (*text == '\t')
			return ++white_count;

		if (*text != ' ')
			return white_count;

		if (++white_count == tabsize)
			return tabsize;

		text++;
	}
}

/* Adjust the positions of mark and cursor when they are on the given line. */
void compensate_leftward(linestruct *line, size_t leftshift)
{
	if (line == openfile->mark) {
		if (openfile->mark_x < leftshift)
			openfile->mark_x = 0;
		else
			openfile->mark_x -= leftshift;
	}

	if (line == openfile->current) {
		if (openfile->current_x < leftshift)
			openfile->current_x = 0;
		else
			openfile->current_x -= leftshift;
		openfile->placewewant = xplustabs();
	}
}

/* Remove an indent from the given line. */
void unindent_a_line(linestruct *line, size_t indent_len)
{
	size_t length = strlen(line->data);

	/* If the indent is empty, don't change the line. */
	if (indent_len == 0)
		return;

	/* Remove the first tab's worth of whitespace from this line. */
	memmove(line->data, line->data + indent_len, length - indent_len + 1);

	openfile->totsize -= indent_len;

	if (ISSET(SOFTWRAP))
		line->extrarows = extra_chunks_in(line);

	/* Adjust the positions of mark and cursor, when they are affected. */
	compensate_leftward(line, indent_len);
}

/* Unindent the current line (or the marked lines) by tabsize columns.
 * The removed indent can be a mixture of spaces plus at most one tab. */
void do_unindent(void)
{
	linestruct *top, *bot, *line;

	/* Use either all the marked lines or just the current line. */
	get_range(&top, &bot);

	/* Skip any leading lines that cannot be unindented. */
	while (top != bot->next && length_of_white(top->data) == 0)
		top = top->next;

	/* If none of the lines can be unindented, there is nothing to do. */
	if (top == bot->next)
		return;

	add_undo(UNINDENT, NULL);

	/* Go through each of the lines, removing their leading indent where
	 * possible, and saving the removed whitespace in the undo item. */
	for (line = top; line != bot->next; line = line->next) {
		size_t indent_len = length_of_white(line->data);
		char *indentation = measured_copy(line->data, indent_len);

		unindent_a_line(line, indent_len);
		update_multiline_undo(line->lineno, indentation);

		free(indentation);
	}

	set_modified();
	ensure_firstcolumn_is_aligned();
	refresh_needed = TRUE;
	shift_held = TRUE;
}

/* Perform an undo or redo for an indent or unindent action. */
void handle_indent_action(undostruct *u, bool undoing, bool add_indent)
{
	groupstruct *group = u->grouping;
	linestruct *line = line_from_number(group->top_line);

	if (group->next != NULL)
		die("Multiple groups -- please report a bug\n");

	/* When redoing, reposition the cursor and let the indenter adjust it. */
	if (!undoing)
		goto_line_posx(u->head_lineno, u->head_x);

	/* For each line in the group, add or remove the individual indent. */
	while (line != NULL && line->lineno <= group->bottom_line) {
		char *blanks = group->indentations[line->lineno - group->top_line];

		if (undoing ^ add_indent)
			indent_a_line(line, blanks);
		else
			unindent_a_line(line, strlen(blanks));

		line = line->next;
	}

	/* When undoing, reposition the cursor to the recorded location. */
	if (undoing)
		goto_line_posx(u->head_lineno, u->head_x);

	refresh_needed = TRUE;
}
#endif /* !NANO_TINY */

#ifdef ENABLE_COMMENT
/* Test whether the given line can be uncommented, or add or remove a comment,
 * depending on action.  Return TRUE if the line is uncommentable, or when
 * anything was added or removed; FALSE otherwise. */
bool comment_line(undo_type action, linestruct *line, const char *comment_seq)
{
	size_t comment_seq_len = strlen(comment_seq);
	const char *post_seq = strchr(comment_seq, '|');
		/* The postfix, if this is a bracketing type comment sequence. */
	size_t pre_len = post_seq ? post_seq++ - comment_seq : comment_seq_len;
		/* Length of prefix. */
	size_t post_len = post_seq ? comment_seq_len - pre_len - 1 : 0;
		/* Length of postfix. */
	size_t line_len = strlen(line->data);

	if (!ISSET(NO_NEWLINES) && line == openfile->filebot)
		return FALSE;

	if (action == COMMENT) {
		/* Make room for the comment sequence(s), move the text right and
		 * copy them in. */
		line->data = charealloc(line->data, line_len + pre_len + post_len + 1);
		memmove(line->data + pre_len, line->data, line_len + 1);
		memmove(line->data, comment_seq, pre_len);
		if (post_len > 0)
			memmove(line->data + pre_len + line_len, post_seq, post_len + 1);

		openfile->totsize += pre_len + post_len;

		/* If needed, adjust the position of the mark and of the cursor. */
		if (line == openfile->mark && openfile->mark_x > 0)
			openfile->mark_x += pre_len;
		if (line == openfile->current && openfile->current_x > 0) {
			openfile->current_x += pre_len;
			openfile->placewewant = xplustabs();
		}

		return TRUE;
	}

	/* If the line is commented, report it as uncommentable, or uncomment it. */
	if (strncmp(line->data, comment_seq, pre_len) == 0 && (post_len == 0 ||
				strcmp(line->data + line_len - post_len, post_seq) == 0)) {

		if (action == PREFLIGHT)
			return TRUE;

		/* Erase the comment prefix by moving the non-comment part. */
		memmove(line->data, line->data + pre_len, line_len - pre_len);
		/* Truncate the postfix if there was one. */
		line->data[line_len - pre_len - post_len] = '\0';

		openfile->totsize -= pre_len + post_len;

		/* Adjust the positions of mark and cursor, when needed. */
		compensate_leftward(line, pre_len);

		return TRUE;
	}

	return FALSE;
}

/* Comment or uncomment the current line or the marked lines. */
void do_comment(void)
{
	const char *comment_seq = GENERAL_COMMENT_CHARACTER;
	undo_type action = UNCOMMENT;
	linestruct *top, *bot, *line;
	bool empty, all_empty = TRUE;

#ifdef ENABLE_COLOR
	if (openfile->syntax)
		comment_seq = openfile->syntax->comment;

	if (*comment_seq == '\0') {
		statusbar(_("Commenting is not supported for this file type"));
		return;
	}
#endif

	/* Determine which lines to work on. */
	get_range(&top, &bot);

	/* If only the magic line is selected, don't do anything. */
	if (top == bot && bot == openfile->filebot && !ISSET(NO_NEWLINES)) {
		statusbar(_("Cannot comment past end of file"));
		return;
	}

	/* Figure out whether to comment or uncomment the selected line or lines. */
	for (line = top; line != bot->next; line = line->next) {
		empty = white_string(line->data);

		/* If this line is not blank and not commented, we comment all. */
		if (!empty && !comment_line(PREFLIGHT, line, comment_seq)) {
			action = COMMENT;
			break;
		}
		all_empty = all_empty && empty;
	}

	/* If all selected lines are blank, we comment them. */
	action = all_empty ? COMMENT : action;

	add_undo(action, NULL);

	/* Store the comment sequence used for the operation, because it could
	 * change when the file name changes; we need to know what it was. */
	openfile->current_undo->strdata = copy_of(comment_seq);

	/* Comment/uncomment each of the selected lines when possible, and
	 * store undo data when a line changed. */
	for (line = top; line != bot->next; line = line->next) {
		if (comment_line(action, line, comment_seq)) {
#ifndef NANO_TINY
			if (ISSET(SOFTWRAP))
				line->extrarows = extra_chunks_in(line);
#endif
			update_multiline_undo(line->lineno, "");
		}
	}

	set_modified();
	ensure_firstcolumn_is_aligned();
	refresh_needed = TRUE;
	shift_held = TRUE;
}

/* Perform an undo or redo for a comment or uncomment action. */
void handle_comment_action(undostruct *u, bool undoing, bool add_comment)
{
	groupstruct *group = u->grouping;

	/* When redoing, reposition the cursor and let the commenter adjust it. */
	if (!undoing)
		goto_line_posx(u->head_lineno, u->head_x);

	while (group) {
		linestruct *line = line_from_number(group->top_line);

		while (line != NULL && line->lineno <= group->bottom_line) {
			comment_line(undoing ^ add_comment ?
								COMMENT : UNCOMMENT, line, u->strdata);
			line = line->next;
		}

		group = group->next;
	}

	/* When undoing, reposition the cursor to the recorded location. */
	if (undoing)
		goto_line_posx(u->head_lineno, u->head_x);

	refresh_needed = TRUE;
}
#endif /* ENABLE_COMMENT */

#ifndef NANO_TINY
#define redo_paste  undo_cut
#define undo_paste  redo_cut

/* Undo a cut, or redo a paste. */
void undo_cut(undostruct *u)
{
	goto_line_posx(u->head_lineno, (u->xflags & WAS_WHOLE_LINE) ? 0 : u->head_x);

	if (u->cutbuffer)
		copy_from_buffer(u->cutbuffer);

	/* If originally the last line was cut too, remove an extra magic line. */
	if ((u->xflags & INCLUDED_LAST_LINE) && !ISSET(NO_NEWLINES) &&
						openfile->filebot != openfile->current &&
						openfile->filebot->prev->data[0] == '\0')
		remove_magicline();

	if (u->xflags & CURSOR_WAS_AT_HEAD)
		goto_line_posx(u->head_lineno, u->head_x);
}

/* Redo a cut, or undo a paste. */
void redo_cut(undostruct *u)
{
	linestruct *oldcutbuffer = cutbuffer;

	cutbuffer = NULL;

	openfile->mark = line_from_number(u->head_lineno);
	openfile->mark_x = (u->xflags & WAS_WHOLE_LINE) ? 0 : u->head_x;

	goto_line_posx(u->tail_lineno, u->tail_x);

	do_snip(TRUE, FALSE, u->type == ZAP);

	free_lines(cutbuffer);
	cutbuffer = oldcutbuffer;
}

/* Undo the last thing(s) we did. */
void do_undo(void)
{
	undostruct *u = openfile->current_undo;
	linestruct *line = NULL, *intruder;
	linestruct *oldcutbuffer;
	char *data, *undidmsg = NULL;
	size_t original_x, regain_from_x;

	if (u == NULL) {
		statusbar(_("Nothing to undo"));
		return;
	}

	if (u->type <= REPLACE)
		line = line_from_number(u->tail_lineno);

	switch (u->type) {
	case ADD:
		/* TRANSLATORS: The next thirteen strings describe actions
		 * that are undone or redone.  They are all nouns, not verbs. */
		undidmsg = _("addition");
		if ((u->xflags & INCLUDED_LAST_LINE) && !ISSET(NO_NEWLINES))
			remove_magicline();
		memmove(line->data + u->head_x, line->data + u->head_x + strlen(u->strdata),
						strlen(line->data + u->head_x) - strlen(u->strdata) + 1);
		goto_line_posx(u->head_lineno, u->head_x);
		break;
	case ENTER:
		undidmsg = _("line break");
		/* An <Enter> at the end of leading whitespace while autoindenting has
		 * deleted the whitespace, and stored an x position of zero.  In that
		 * case, adjust the positions to return to and to scoop data from. */
		original_x = (u->head_x == 0) ? u->tail_x : u->head_x;
		regain_from_x = (u->head_x == 0) ? 0 : u->tail_x;
		line->data = charealloc(line->data, strlen(line->data) +
								strlen(&u->strdata[regain_from_x]) + 1);
		strcat(line->data, &u->strdata[regain_from_x]);
		line->has_anchor |= line->next->has_anchor;
		unlink_node(line->next);
		renumber_from(line);
		goto_line_posx(u->head_lineno, original_x);
		break;
	case BACK:
	case DEL:
		undidmsg = _("deletion");
		data = charalloc(strlen(line->data) + strlen(u->strdata) + 1);
		strncpy(data, line->data, u->head_x);
		strcpy(&data[u->head_x], u->strdata);
		strcpy(&data[u->head_x + strlen(u->strdata)], &line->data[u->head_x]);
		free(line->data);
		line->data = data;
		goto_line_posx(u->tail_lineno, u->tail_x);
		break;
	case JOIN:
		undidmsg = _("line join");
		/* When the join was done by a Backspace at the tail of the file,
		 * and the nonewlines flag isn't set, do not re-add a newline that
		 * wasn't actually deleted; just position the cursor. */
		if ((u->xflags & WAS_BACKSPACE_AT_EOF) && !ISSET(NO_NEWLINES)) {
			goto_line_posx(openfile->filebot->lineno, 0);
			break;
		}
		line->data[u->tail_x] = '\0';
		if (ISSET(SOFTWRAP))
			line->extrarows = extra_chunks_in(line);
		intruder = make_new_node(line);
		intruder->data = copy_of(u->strdata);
		splice_node(line, intruder);
		renumber_from(intruder);
		goto_line_posx(u->head_lineno, u->head_x);
		break;
	case REPLACE:
		undidmsg = _("replacement");
		data = u->strdata;
		u->strdata = line->data;
		line->data = data;
		goto_line_posx(u->head_lineno, u->head_x);
		break;
#ifdef ENABLE_WRAPPING
	case SPLIT_BEGIN:
		undidmsg = _("addition");
		break;
	case SPLIT_END:
		openfile->current_undo = openfile->current_undo->next;
		while (openfile->current_undo->type != SPLIT_BEGIN)
			do_undo();
		u = openfile->current_undo;
		break;
#endif
	case ZAP:
		undidmsg = _("erasure");
		undo_cut(u);
		break;
	case CUT_TO_EOF:
	case CUT:
		/* TRANSLATORS: Remember: these are nouns, NOT verbs. */
		undidmsg = _("cut");
		undo_cut(u);
		break;
	case PASTE:
		undidmsg = _("paste");
		undo_paste(u);
		if ((u->xflags & INCLUDED_LAST_LINE) && !ISSET(NO_NEWLINES) &&
							openfile->filebot != openfile->current)
			remove_magicline();
		break;
	case INSERT:
		undidmsg = _("insertion");
		oldcutbuffer = cutbuffer;
		cutbuffer = NULL;
		goto_line_posx(u->head_lineno, u->head_x);
		openfile->mark = line_from_number(u->tail_lineno);
		openfile->mark_x = u->tail_x;
		cut_marked_region();
		u->cutbuffer = cutbuffer;
		cutbuffer = oldcutbuffer;
		if ((u->xflags & INCLUDED_LAST_LINE) && !ISSET(NO_NEWLINES) &&
							openfile->filebot != openfile->current)
			remove_magicline();
		break;
	case COUPLE_BEGIN:
		undidmsg = u->strdata;
		goto_line_posx(u->head_lineno, u->head_x);
		openfile->current_y = u->tail_lineno;
		adjust_viewport(STATIONARY);
		break;
	case COUPLE_END:
		/* Remember the row of the cursor for a possible redo. */
		openfile->current_undo->head_lineno = openfile->current_y;
		openfile->current_undo = openfile->current_undo->next;
		do_undo();
		do_undo();
		do_undo();
		return;
	case INDENT:
		handle_indent_action(u, TRUE, TRUE);
		undidmsg = _("indent");
		break;
	case UNINDENT:
		handle_indent_action(u, TRUE, FALSE);
		undidmsg = _("unindent");
		break;
#ifdef ENABLE_COMMENT
	case COMMENT:
		handle_comment_action(u, TRUE, TRUE);
		undidmsg = _("comment");
		break;
	case UNCOMMENT:
		handle_comment_action(u, TRUE, FALSE);
		undidmsg = _("uncomment");
		break;
#endif
	default:
		break;
	}

	if (undidmsg && !pletion_line)
		statusline(HUSH, _("Undid %s"), undidmsg);

	openfile->current_undo = openfile->current_undo->next;
	openfile->last_action = OTHER;
	openfile->mark = NULL;
	openfile->placewewant = xplustabs();

	if (ISSET(SOFTWRAP))
		openfile->current->extrarows = extra_chunks_in(openfile->current);

	openfile->totsize = u->wassize;

	/* When at the point where the file was last saved, unset "Modified". */
	if (openfile->current_undo == openfile->last_saved) {
		openfile->modified = FALSE;
		titlebar(NULL);
	} else
		set_modified();
}

/* Redo the last thing(s) we undid. */
void do_redo(void)
{
	linestruct *line = NULL, *intruder;
	char *data, *redidmsg = NULL;
	bool suppress_modification = FALSE;
	undostruct *u = openfile->undotop;

	if (u == NULL || u == openfile->current_undo) {
		statusbar(_("Nothing to redo"));
		return;
	}

	/* Find the item before the current one in the undo stack. */
	while (u->next != openfile->current_undo)
		u = u->next;

	if (u->type <= REPLACE)
		line = line_from_number(u->tail_lineno);

	switch (u->type) {
	case ADD:
		redidmsg = _("addition");
		if ((u->xflags & INCLUDED_LAST_LINE) && !ISSET(NO_NEWLINES))
			new_magicline();
		data = charalloc(strlen(line->data) + strlen(u->strdata) + 1);
		strncpy(data, line->data, u->head_x);
		strcpy(&data[u->head_x], u->strdata);
		strcpy(&data[u->head_x + strlen(u->strdata)], &line->data[u->head_x]);
		free(line->data);
		line->data = data;
		goto_line_posx(u->tail_lineno, u->tail_x);
		break;
	case ENTER:
		redidmsg = _("line break");
		line->data[u->head_x] = '\0';
		if (ISSET(SOFTWRAP))
			line->extrarows = extra_chunks_in(line);
		intruder = make_new_node(line);
		intruder->data = copy_of(u->strdata);
		splice_node(line, intruder);
		renumber_from(intruder);
		goto_line_posx(u->head_lineno + 1, u->tail_x);
		break;
	case BACK:
	case DEL:
		redidmsg = _("deletion");
		memmove(line->data + u->head_x, line->data + u->head_x + strlen(u->strdata),
						strlen(line->data + u->head_x) - strlen(u->strdata) + 1);
		goto_line_posx(u->head_lineno, u->head_x);
		break;
	case JOIN:
		redidmsg = _("line join");
		/* When the join was done by a Backspace at the tail of the file,
		 * and the nonewlines flag isn't set, do not join anything, as
		 * nothing was actually deleted; just position the cursor. */
		if ((u->xflags & WAS_BACKSPACE_AT_EOF) && !ISSET(NO_NEWLINES)) {
			goto_line_posx(u->tail_lineno, u->tail_x);
			break;
		}
		line->data = charealloc(line->data, strlen(line->data) + strlen(u->strdata) + 1);
		strcat(line->data, u->strdata);
		unlink_node(line->next);
		renumber_from(line);
		goto_line_posx(u->tail_lineno, u->tail_x);
		break;
	case REPLACE:
		redidmsg = _("replacement");
		data = u->strdata;
		u->strdata = line->data;
		line->data = data;
		goto_line_posx(u->head_lineno, u->head_x);
		break;
#ifdef ENABLE_WRAPPING
	case SPLIT_BEGIN:
		openfile->current_undo = u;
		while (openfile->current_undo->type != SPLIT_END)
			do_redo();
		u = openfile->current_undo;
		goto_line_posx(u->head_lineno, u->head_x);
		break;
	case SPLIT_END:
		redidmsg = _("addition");
		break;
#endif
	case ZAP:
		redidmsg = _("erasure");
		redo_cut(u);
		break;
	case CUT_TO_EOF:
	case CUT:
		redidmsg = _("cut");
		redo_cut(u);
		break;
	case PASTE:
		redidmsg = _("paste");
		redo_paste(u);
		break;
	case INSERT:
		redidmsg = _("insertion");
		goto_line_posx(u->head_lineno, u->head_x);
		if (u->cutbuffer)
			copy_from_buffer(u->cutbuffer);
		else
			suppress_modification = TRUE;
		free_lines(u->cutbuffer);
		u->cutbuffer = NULL;
		break;
	case COUPLE_BEGIN:
		openfile->current_undo = u;
		do_redo();
		do_redo();
		do_redo();
		return;
	case COUPLE_END:
		redidmsg = u->strdata;
		goto_line_posx(u->tail_lineno, u->tail_x);
		openfile->current_y = u->head_lineno;
		adjust_viewport(STATIONARY);
		break;
	case INDENT:
		handle_indent_action(u, FALSE, TRUE);
		redidmsg = _("indent");
		break;
	case UNINDENT:
		handle_indent_action(u, FALSE, FALSE);
		redidmsg = _("unindent");
		break;
#ifdef ENABLE_COMMENT
	case COMMENT:
		handle_comment_action(u, FALSE, TRUE);
		redidmsg = _("comment");
		break;
	case UNCOMMENT:
		handle_comment_action(u, FALSE, FALSE);
		redidmsg = _("uncomment");
		break;
#endif
	default:
		break;
	}

	if (redidmsg)
		statusline(HUSH, _("Redid %s"), redidmsg);

	openfile->current_undo = u;
	openfile->last_action = OTHER;
	openfile->mark = NULL;
	openfile->placewewant = xplustabs();

	if (ISSET(SOFTWRAP))
		openfile->current->extrarows = extra_chunks_in(openfile->current);

	openfile->totsize = u->newsize;

	/* When at the point where the file was last saved, unset "Modified". */
	if (openfile->current_undo == openfile->last_saved) {
		openfile->modified = FALSE;
		titlebar(NULL);
	} else if (!suppress_modification)
		set_modified();
}
#endif /* !NANO_TINY */

/* Break the current line at the cursor position. */
void do_enter(void)
{
	linestruct *newnode = make_new_node(openfile->current);
	size_t extra = 0;
#ifndef NANO_TINY
	linestruct *sampleline = openfile->current;
	bool allblanks = FALSE;

	if (ISSET(AUTOINDENT)) {
#ifdef ENABLE_JUSTIFY
		/* When doing automatic long-line wrapping and the next line is
		 * in this same paragraph, use its indentation as the model. */
		if (ISSET(BREAK_LONG_LINES) && sampleline->next != NULL &&
					inpar(sampleline->next) && !begpar(sampleline->next, 0))
			sampleline = sampleline->next;
#endif
		extra = indent_length(sampleline->data);

		/* When breaking in the indentation, limit the automatic one. */
		if (extra > openfile->current_x)
			extra = openfile->current_x;
		else if (extra == openfile->current_x)
			allblanks = TRUE;
	}
#endif /* NANO_TINY */
	newnode->data = charalloc(strlen(openfile->current->data +
										openfile->current_x) + extra + 1);
	strcpy(&newnode->data[extra], openfile->current->data +
										openfile->current_x);
#ifndef NANO_TINY
	if (ISSET(AUTOINDENT)) {
		/* Copy the whitespace from the sample line to the new one. */
		strncpy(newnode->data, sampleline->data, extra);
		/* If there were only blanks before the cursor, trim them. */
		if (allblanks)
			openfile->current_x = 0;
	}
#endif

	/* Make the current line end at the cursor position. */
	openfile->current->data[openfile->current_x] = '\0';

#ifndef NANO_TINY
	add_undo(ENTER, NULL);

	/* Adjust the mark if it was on the current line after the cursor. */
	if (openfile->mark == openfile->current &&
				openfile->mark_x > openfile->current_x) {
		openfile->mark = newnode;
		openfile->mark_x += extra - openfile->current_x;
	}

	if (ISSET(SOFTWRAP)) {
		openfile->current->extrarows = extra_chunks_in(openfile->current);
		newnode->extrarows = extra_chunks_in(newnode);
	}
#endif

	/* Insert the newly created line after the current one and renumber. */
	splice_node(openfile->current, newnode);
	renumber_from(newnode);

	/* Put the cursor on the new line, after any automatic whitespace. */
	openfile->current = newnode;
	openfile->current_x = extra;
	openfile->placewewant = xplustabs();

	openfile->totsize++;
	set_modified();

#ifndef NANO_TINY
	if (ISSET(AUTOINDENT) && !allblanks)
		openfile->totsize += extra;
	update_undo(ENTER);
#endif

	refresh_needed = TRUE;
	focusing = FALSE;
}

#ifndef NANO_TINY
/* Discard undo items that are newer than the given one, or all if NULL. */
void discard_until(const undostruct *thisitem)
{
	undostruct *dropit = openfile->undotop;
	groupstruct *group;

	while (dropit != NULL && dropit != thisitem) {
		openfile->undotop = dropit->next;
		free(dropit->strdata);
		free_lines(dropit->cutbuffer);
		group = dropit->grouping;
		while (group != NULL) {
			groupstruct *next = group->next;
			free_chararray(group->indentations,
								group->bottom_line - group->top_line);
			free(group);
			group = next;
		}
		free(dropit);
		dropit = openfile->undotop;
	}

	/* Adjust the pointer to the top of the undo stack. */
	openfile->current_undo = (undostruct *)thisitem;

	/* Prevent a chain of editing actions from continuing. */
	openfile->last_action = OTHER;
}

/* Add a new undo item of the given type to the top of the current pile. */
void add_undo(undo_type action, const char *message)
{
	undostruct *u = nmalloc(sizeof(undostruct));
	linestruct *thisline = openfile->current;

	/* Initialize the newly allocated undo item. */
	u->type = action;
	u->strdata = NULL;
	u->cutbuffer = NULL;
	u->head_lineno = thisline->lineno;
	u->head_x = openfile->current_x;
	u->tail_lineno = thisline->lineno;
	u->tail_x = openfile->current_x;
	u->wassize = openfile->totsize;
	u->newsize = openfile->totsize;
	u->grouping = NULL;
	u->xflags = 0;

	/* Blow away any undone items. */
	discard_until(openfile->current_undo);

#ifdef ENABLE_WRAPPING
	/* If some action caused automatic long-line wrapping, insert the
	 * SPLIT_BEGIN item underneath that action's undo item.  Otherwise,
	 * just add the new item to the top of the undo stack. */
	if (u->type == SPLIT_BEGIN) {
		action = openfile->undotop->type;
		u->wassize = openfile->undotop->wassize;
		u->next = openfile->undotop->next;
		openfile->undotop->next = u;
	} else
#endif
	{
		u->next = openfile->undotop;
		openfile->undotop = u;
		openfile->current_undo = u;
	}

	/* Record the info needed to be able to undo each possible action. */
	switch (u->type) {
	case ADD:
		/* If a new magic line will be added, an undo should remove it. */
		if (thisline == openfile->filebot)
			u->xflags |= INCLUDED_LAST_LINE;
		break;
	case ENTER:
		break;
	case BACK:
		/* If the next line is the magic line, don't ever undo this
		 * backspace, as it won't actually have deleted anything. */
		if (thisline->next == openfile->filebot && thisline->data[0] != '\0')
			u->xflags |= WAS_BACKSPACE_AT_EOF;
		/* Fall-through. */
	case DEL:
		/* When not at the end of a line, store the deleted character;
		 * otherwise, morph the undo item into a line join. */
		if (thisline->data[openfile->current_x] != '\0') {
			int charlen = char_length(thisline->data + u->head_x);

			u->strdata = measured_copy(thisline->data + u->head_x, charlen);
			if (u->type == BACK)
				u->tail_x += charlen;
			break;
		}
		action = JOIN;
		if (thisline->next != NULL) {
			if (u->type == BACK) {
				u->head_lineno = thisline->next->lineno;
				u->head_x = 0;
			}
			u->strdata = copy_of(thisline->next->data);
		}
		u->type = JOIN;
		break;
	case REPLACE:
		u->strdata = copy_of(thisline->data);
		break;
#ifdef ENABLE_WRAPPING
	case SPLIT_BEGIN:
	case SPLIT_END:
		break;
#endif
	case CUT_TO_EOF:
		u->xflags |= (INCLUDED_LAST_LINE | CURSOR_WAS_AT_HEAD);
		break;
	case ZAP:
	case CUT:
		if (openfile->mark) {
			if (mark_is_before_cursor()){
				u->head_lineno = openfile->mark->lineno;
				u->head_x = openfile->mark_x;
				u->xflags |= MARK_WAS_SET;
			} else {
				u->tail_lineno = openfile->mark->lineno;
				u->tail_x = openfile->mark_x;
				u->xflags |= (MARK_WAS_SET | CURSOR_WAS_AT_HEAD);
			}
			if (u->tail_lineno == openfile->filebot->lineno)
				u->xflags |= INCLUDED_LAST_LINE;
		} else if (!ISSET(CUT_FROM_CURSOR)) {
			/* The entire line is being cut regardless of the cursor position. */
			u->xflags |= (WAS_WHOLE_LINE | CURSOR_WAS_AT_HEAD);
			u->tail_x = 0;
		} else
			u->xflags |= CURSOR_WAS_AT_HEAD;
		break;
	case PASTE:
		u->cutbuffer = copy_buffer(cutbuffer);
		if (thisline == openfile->filebot)
			u->xflags |= INCLUDED_LAST_LINE;
		break;
	case INSERT:
		if (thisline == openfile->filebot)
			u->xflags |= INCLUDED_LAST_LINE;
		break;
	case COUPLE_BEGIN:
		u->tail_lineno = openfile->current_y;
		/* Fall-through. */
	case COUPLE_END:
		u->strdata = copy_of(_(message));
		break;
	case INDENT:
	case UNINDENT:
#ifdef ENABLE_COMMENT
	case COMMENT:
	case UNCOMMENT:
#endif
		break;
	default:
		die("Bad undo type -- please report a bug\n");
	}

	openfile->last_action = action;
}

/* Update a multiline undo item.  This should be called once for each line
 * affected by a multiple-line-altering feature.  The indentation that is
 * added or removed is saved separately for each line in the undo item. */
void update_multiline_undo(ssize_t lineno, char *indentation)
{
	undostruct *u = openfile->current_undo;

	/* If there already is a group and the current line is contiguous with it,
	 * extend the group; otherwise, create a new group. */
	if (u->grouping && u->grouping->bottom_line + 1 == lineno) {
		size_t number_of_lines;

		u->grouping->bottom_line++;

		number_of_lines = u->grouping->bottom_line - u->grouping->top_line + 1;
		u->grouping->indentations = (char **)nrealloc(u->grouping->indentations,
										number_of_lines * sizeof(char *));
		u->grouping->indentations[number_of_lines - 1] = copy_of(indentation);
	} else {
		groupstruct *born = nmalloc(sizeof(groupstruct));

		born->next = u->grouping;
		u->grouping = born;
		born->top_line = lineno;
		born->bottom_line = lineno;

		u->grouping->indentations = (char **)nmalloc(sizeof(char *));
		u->grouping->indentations[0] = copy_of(indentation);
	}

	/* Store the file size after the change, to be used when redoing. */
	u->newsize = openfile->totsize;
}

/* Update an undo item with (among other things) the file size and
 * cursor position after the given action. */
void update_undo(undo_type action)
{
	undostruct *u = openfile->undotop;
	size_t datalen, newlen;
	char *textposition;
	int charlen;

	if (u->type != action)
		die("Mismatching undo type -- please report a bug\n");

	u->newsize = openfile->totsize;

	switch (u->type) {
	case ADD:
		newlen = openfile->current_x - u->head_x;
		u->strdata = charealloc(u->strdata, newlen + 1);
		strncpy(u->strdata, openfile->current->data + u->head_x, newlen);
		u->strdata[newlen] = '\0';
		u->tail_x = openfile->current_x;
		break;
	case ENTER:
		u->strdata = copy_of(openfile->current->data);
		u->tail_x = openfile->current_x;
		break;
	case BACK:
	case DEL:
		textposition = openfile->current->data + openfile->current_x;
		charlen = char_length(textposition);
		datalen = strlen(u->strdata);
		if (openfile->current_x == u->head_x) {
			/* They deleted more: add removed character after earlier stuff. */
			u->strdata = charealloc(u->strdata, datalen + charlen + 1);
			strncpy(u->strdata + datalen, textposition, charlen);
			u->strdata[datalen + charlen] = '\0';
			u->tail_x = openfile->current_x;
		} else if (openfile->current_x == u->head_x - charlen) {
			/* They backspaced further: add removed character before earlier. */
			u->strdata = charealloc(u->strdata, datalen + charlen + 1);
			memmove(u->strdata + charlen, u->strdata, datalen + 1);
			strncpy(u->strdata, textposition, charlen);
			u->head_x = openfile->current_x;
		} else
			/* They deleted *elsewhere* on the line: start a new undo item. */
			add_undo(u->type, NULL);
		break;
	case REPLACE:
		break;
#ifdef ENABLE_WRAPPING
	case SPLIT_BEGIN:
	case SPLIT_END:
		break;
#endif
	case ZAP:
	case CUT_TO_EOF:
	case CUT:
		if (u->type == ZAP)
			u->cutbuffer = cutbuffer;
		else if (cutbuffer != NULL) {
			free_lines(u->cutbuffer);
			u->cutbuffer = copy_buffer(cutbuffer);
		}
		if (!(u->xflags & MARK_WAS_SET)) {
			linestruct *bottomline = u->cutbuffer;
			size_t count = 0;

			/* Find the end of the cut for the undo/redo, using our copy. */
			while (bottomline->next != NULL) {
				bottomline = bottomline->next;
				count++;
			}
			u->tail_lineno = u->head_lineno + count;
			if (ISSET(CUT_FROM_CURSOR) || u->type == CUT_TO_EOF) {
				u->tail_x = strlen(bottomline->data);
				if (count == 0)
					u->tail_x += u->head_x;
			} else if (openfile->current == openfile->filebot && ISSET(NO_NEWLINES))
				u->tail_x = strlen(bottomline->data);
		}
		break;
	case PASTE:
		u->tail_lineno = openfile->current->lineno;
		u->tail_x = openfile->current_x;
		break;
	case INSERT:
		u->tail_lineno = openfile->current->lineno;
		u->tail_x = openfile->current_x;
		break;
	case COUPLE_BEGIN:
		break;
	case COUPLE_END:
		u->tail_lineno = openfile->current->lineno;
		u->tail_x = openfile->current_x;
		break;
	default:
		die("Bad undo type -- please report a bug\n");
	}
}
#endif /* !NANO_TINY */

#ifdef ENABLE_WRAPPING
/* When the current line is overlong, hard-wrap it at the furthest possible
 * whitespace character, and (if possible) prepend the remainder of the line
 * to the next line.  Return TRUE if wrapping occurred, and FALSE otherwise. */
bool do_wrap(void)
{
	linestruct *line = openfile->current;
		/* The line to be wrapped, if needed and possible. */
	size_t line_len = strlen(line->data);
		/* The length of this line. */
#ifdef ENABLE_JUSTIFY
	size_t quot_len = quote_length(line->data);
		/* The length of the quoting part of this line. */
	size_t lead_len = quot_len + indent_length(line->data + quot_len);
		/* The length of the quoting part plus subsequent whitespace. */
#else
	size_t lead_len = indent_length(line->data);
#endif
	size_t cursor_x = openfile->current_x;
		/* The current cursor position, for comparison with the wrap point. */
	ssize_t wrap_loc;
		/* The position in the line's text where we wrap. */
	const char *remainder;
		/* The text after the wrap point. */
	size_t rest_length;
		/* The length of the remainder. */

	/* First find the last blank character where we can break the line. */
	wrap_loc = break_line(line->data + lead_len,
							wrap_at - wideness(line->data, lead_len), FALSE);

	/* If no wrapping point was found before end-of-line, we don't wrap. */
	if (wrap_loc < 0 || lead_len + wrap_loc == line_len)
		return FALSE;

	/* Adjust the wrap location to its position in the full line,
	 * and step forward to the character just after the blank. */
	wrap_loc = lead_len + step_right(line->data + lead_len, wrap_loc);

	/* When now at end-of-line, no need to wrap. */
	if (line->data[wrap_loc] == '\0')
		return FALSE;

#ifndef NANO_TINY
	add_undo(SPLIT_BEGIN, NULL);
#endif
#ifdef ENABLE_JUSTIFY
	bool autowhite = ISSET(AUTOINDENT);

	if (quot_len > 0)
		UNSET(AUTOINDENT);
#endif

	/* The remainder is the text that will be wrapped to the next line. */
	remainder = line->data + wrap_loc;
	rest_length = line_len - wrap_loc;

	/* When prepending and the remainder of this line will not make the next
	 * line too long, then join the two lines, so that, after the line wrap,
	 * the remainder will effectively have been prefixed to the next line. */
	if (openfile->spillage_line && openfile->spillage_line == line->next &&
				rest_length + breadth(line->next->data) <= wrap_at) {
		/* Go to the end of this line. */
		openfile->current_x = line_len;

		/* If the remainder doesn't end in a blank, add a space. */
		if (!is_blank_char(remainder + step_left(remainder, rest_length))) {
#ifndef NANO_TINY
			add_undo(ADD, NULL);
#endif
			line->data = charealloc(line->data, line_len + 2);
			line->data[line_len] = ' ';
			line->data[line_len + 1] = '\0';
			rest_length++;
			openfile->totsize++;
			openfile->current_x++;
#ifndef NANO_TINY
			update_undo(ADD);
#endif
		}

		/* Join the next line to this one. */
		do_delete();

#ifdef ENABLE_JUSTIFY
		/* If the leading part of the current line equals the leading part of
		 * what was the next line, then strip this second leading part. */
		if (strncmp(line->data, line->data + openfile->current_x, lead_len) == 0)
			for (size_t i = lead_len; i > 0; i--)
				do_delete();
#endif
		/* Remove any extra blanks. */
		while (is_blank_char(&line->data[openfile->current_x]))
			do_delete();
	}

	/* Go to the wrap location. */
	openfile->current_x = wrap_loc;

	/* When requested, snip trailing blanks off the wrapped line. */
	if (ISSET(TRIM_BLANKS)) {
		size_t rear_x = step_left(line->data, wrap_loc);
		size_t typed_x = step_left(line->data, cursor_x);

		while ((rear_x != typed_x || cursor_x >= wrap_loc) &&
						is_blank_char(line->data + rear_x)) {
			openfile->current_x = rear_x;
			do_delete();
			rear_x = step_left(line->data, rear_x);
		}
	}

	/* Now split the line. */
	do_enter();

#ifdef ENABLE_JUSTIFY
	/* If the original line has quoting, copy it to the spillage line. */
	if (quot_len > 0) {
		line = line->next;
		line_len = strlen(line->data);
		line->data = charealloc(line->data, lead_len + line_len + 1);

		memmove(line->data + lead_len, line->data, line_len + 1);
		strncpy(line->data, line->prev->data, lead_len);

		openfile->current_x += lead_len;
#ifndef NANO_TINY
		update_undo(ENTER);
#endif
		if (autowhite)
			SET(AUTOINDENT);
	}
#endif

	openfile->spillage_line = openfile->current;

	if (cursor_x < wrap_loc) {
		openfile->current = openfile->current->prev;
		openfile->current_x = cursor_x;
	} else
		openfile->current_x += (cursor_x - wrap_loc);

	openfile->placewewant = xplustabs();

#ifndef NANO_TINY
	add_undo(SPLIT_END, NULL);
#endif

	return TRUE;
}
#endif /* ENABLE_WRAPPING */

#if defined(ENABLE_HELP) || defined(ENABLED_WRAPORJUSTIFY)
/* Find the last blank in the given piece of text such that the display width
 * to that point is at most (goal + 1).  When there is no such blank, then find
 * the first blank.  Return the index of the last blank in that group of blanks.
 * When snap_at_nl is TRUE, a newline character counts as a blank too. */
ssize_t break_line(const char *textstart, ssize_t goal, bool snap_at_nl)
{
	const char *lastblank = NULL;
		/* The point where the last blank was found, if any. */
	const char *pointer = textstart;
		/* An iterator through the given line of text. */
	size_t column = 0;
		/* The column number that corresponds to the position of the pointer. */

	/* Skip over leading whitespace, where a line should never be broken. */
	while (*pointer != '\0' && is_blank_char(pointer))
		pointer += advance_over(pointer, &column);

	/* Find the last blank that does not overshoot the target column.
	 * When treating a help text, do not break in the keystrokes area. */
	while (*pointer != '\0' && ((ssize_t)column <= goal)) {
		if (is_blank_char(pointer) && (!inhelp || column > 17 || goal < 40))
			lastblank = pointer;
#ifdef ENABLE_HELP
		else if (snap_at_nl && *pointer == '\n') {
			lastblank = pointer;
			break;
		}
#endif
		pointer += advance_over(pointer, &column);
	}

	/* If the whole line displays shorter than goal, we're done. */
	if ((ssize_t)column <= goal)
		return (pointer - textstart);

#ifdef ENABLE_HELP
	/* When wrapping a help text and no blank was found, force a line break. */
	if (snap_at_nl && lastblank == NULL)
		return step_left(textstart, pointer - textstart);
#endif

	/* If no blank was found within the goal width, seek one after it. */
	while (lastblank == NULL) {
		if (*pointer == '\0')
			return -1;

		if (is_blank_char(pointer))
			lastblank = pointer;
		else
			pointer += char_length(pointer);
	}

	pointer = lastblank + char_length(lastblank);

	/* Skip any consecutive blanks after the last blank. */
	while (*pointer != '\0' && is_blank_char(pointer)) {
		lastblank = pointer;
		pointer += char_length(pointer);
	}

	return (lastblank - textstart);
}
#endif /* ENABLE_HELP || ENABLED_WRAPORJUSTIFY */

#if !defined(NANO_TINY) || defined(ENABLED_WRAPORJUSTIFY)
/* Return the length of the indentation part of the given line.  The
 * "indentation" of a line is the leading consecutive whitespace. */
size_t indent_length(const char *line)
{
	size_t len = 0;
	char onechar[MAXCHARLEN];
	int charlen;

	while (*line != '\0') {
		charlen = collect_char(line, onechar);

		if (!is_blank_char(onechar))
			break;

		line += charlen;
		len += charlen;
	}

	return len;
}
#endif

#ifdef ENABLE_JUSTIFY
/* Return the length of the quote part of the given line.  The "quote part"
 * of a line is the largest initial substring matching the quoting regex. */
size_t quote_length(const char *line)
{
	regmatch_t matches;
	int rc = regexec(&quotereg, line, 1, &matches, 0);

	if (rc == REG_NOMATCH || matches.rm_so == (regoff_t)-1)
		return 0;

	return matches.rm_eo;
}

/* The maximum depth of recursion.  This must be an even number. */
#define RECURSION_LIMIT  222

/* Return TRUE when the given line is the beginning of a paragraph (BOP). */
bool begpar(const linestruct *const line, int depth)
{
	size_t quot_len, indent_len, prev_dent_len;

	/* If this is the very first line of the buffer, it counts as a BOP
	 * even when it contains no text. */
	if (line == openfile->filetop)
		return TRUE;

	/* If recursion is going too deep, just say it's not a BOP. */
	if (depth > RECURSION_LIMIT)
		return FALSE;

	quot_len = quote_length(line->data);
	indent_len = indent_length(line->data + quot_len);

	/* If this line contains no text, it is not a BOP. */
	if (line->data[quot_len + indent_len] == '\0')
		return FALSE;

	/* When requested, treat a line that starts with whitespace as a BOP. */
	if (ISSET(BOOKSTYLE) && !ISSET(AUTOINDENT) && is_blank_char(line->data))
		return TRUE;

	/* If the quote part of the preceding line differs, this is a BOP. */
	if (quot_len != quote_length(line->prev->data) ||
					strncmp(line->data, line->prev->data, quot_len) != 0)
		return TRUE;

	prev_dent_len = indent_length(line->prev->data + quot_len);

	/* If the preceding line contains no text, this is a BOP. */
	if (line->prev->data[quot_len + prev_dent_len] == '\0')
		return TRUE;

	/* If indentation of this and preceding line are equal, this is not a BOP. */
	if (wideness(line->prev->data, quot_len + prev_dent_len) ==
						wideness(line->data, quot_len + indent_len))
		return FALSE;

	/* Otherwise, this is a BOP if the preceding line is not. */
	return !begpar(line->prev, depth + 1);
}

/* Return TRUE when the given line is part of a paragraph: when it
 * contains something more than quoting and leading whitespace. */
bool inpar(const linestruct *const line)
{
	size_t quot_len = quote_length(line->data);
	size_t indent_len = indent_length(line->data + quot_len);

	return (line->data[quot_len + indent_len] != '\0');
}

/* Find the first occurring paragraph in the forward direction.  Return TRUE
 * when a paragraph was found, and FALSE otherwise.  Furthermore, return the
 * first line and the number of lines of the paragraph. */
bool find_paragraph(linestruct **firstline, size_t *const linecount)
{
	linestruct *line = *firstline;

	/* When not currently in a paragraph, move forward to a line that is. */
	while (!inpar(line) && line->next != NULL)
		line = line->next;

	*firstline = line;

	/* Move down to the last line of the paragraph (if any). */
	do_para_end(&line);

	/* When not in a paragraph now, there aren't any paragraphs left. */
	if (!inpar(line))
		return FALSE;

	/* We found a paragraph; determine its number of lines. */
	*linecount = line->lineno - (*firstline)->lineno + 1;

	return TRUE;
}

/* Concatenate into a single line all the lines of the paragraph that starts at
 * *line and consists of 'count' lines, skipping the quoting and indentation on
 * all lines after the first. */
void concat_paragraph(linestruct *line, size_t count)
{
	while (count > 1) {
		linestruct *next_line = line->next;
		size_t next_line_len = strlen(next_line->data);
		size_t next_quot_len = quote_length(next_line->data);
		size_t next_lead_len = next_quot_len +
							indent_length(next_line->data + next_quot_len);
		size_t line_len = strlen(line->data);

		/* We're just about to tack the next line onto this one.  If
		 * this line isn't empty, make sure it ends in a space. */
		if (line_len > 0 && line->data[line_len - 1] != ' ') {
			line->data = charealloc(line->data, line_len + 2);
			line->data[line_len++] = ' ';
			line->data[line_len] = '\0';
		}

		line->data = charealloc(line->data,
								line_len + next_line_len - next_lead_len + 1);
		strcat(line->data, next_line->data + next_lead_len);
#ifndef NANO_TINY
		line->has_anchor |= next_line->has_anchor;
#endif
		unlink_node(next_line);
		count--;
	}
}

/* Copy a character from one place to another. */
void copy_character(char **from, char **to)
{
	int charlen = char_length(*from);

	if (*from == *to) {
		*from += charlen;
		*to += charlen;
	} else
		while (--charlen >= 0)
			*((*to)++) = *((*from)++);
}

/* In the given line, replace any series of blanks with a single space,
 * but keep two spaces (if there are two) after any closing punctuation,
 * and remove all blanks from the end of the line.  Leave the first skip
 * number of characters untreated. */
void squeeze(linestruct *line, size_t skip)
{
	char *start = line->data + skip;
	char *from = start, *to = start;

	/* For each character, 1) when a blank, change it to a space, and pass over
	 * all blanks after it; 2) if it is punctuation, copy it plus a possible
	 * tailing bracket, and change at most two subsequent blanks to spaces, and
	 * pass over all blanks after these; 3) leave anything else unchanged. */
	while (*from != '\0') {
		if (is_blank_char(from)) {
			from += char_length(from);
			*(to++) = ' ';

			while (*from != '\0' && is_blank_char(from))
				from += char_length(from);
		} else if (mbstrchr(punct, from) != NULL) {
			copy_character(&from, &to);

			if (*from != '\0' && mbstrchr(brackets, from) != NULL)
				copy_character(&from, &to);

			if (*from != '\0' && is_blank_char(from)) {
				from += char_length(from);
				*(to++) = ' ';
			}
			if (*from != '\0' && is_blank_char(from)) {
				from += char_length(from);
				*(to++) = ' ';
			}

			while (*from != '\0' && is_blank_char(from))
				from += char_length(from);
		} else
			copy_character(&from, &to);
	}

	/* If there are spaces at the end of the line, remove them. */
	while (to > start && *(to - 1) == ' ')
		to--;

	*to = '\0';
}

/* Rewrap the given line (that starts with the given lead string which is of
 * the given length), into lines that fit within the target width (wrap_at). */
void rewrap_paragraph(linestruct **line, char *lead_string, size_t lead_len)
{
	ssize_t break_pos;
		/* The x-coordinate where the current line is to be broken. */

	while (breadth((*line)->data) > wrap_at) {
		size_t line_len = strlen((*line)->data);

		/* Find a point in the line where it can be broken. */
		break_pos = break_line((*line)->data + lead_len,
						wrap_at - wideness((*line)->data, lead_len), FALSE);

		/* If we can't break the line, or don't need to, we're done. */
		if (break_pos < 0 || lead_len + break_pos == line_len)
			break;

		/* Adjust the breaking position for the leading part and
		 * move it beyond the found whitespace character. */
		break_pos += lead_len + 1;

		/* Insert a new line after the current one, and copy the leading part
		 * plus the text after the breaking point into it. */
		splice_node(*line, make_new_node(*line));
		(*line)->next->data = charalloc(lead_len + line_len - break_pos + 1);
		strncpy((*line)->next->data, lead_string, lead_len);
		strcpy((*line)->next->data + lead_len, (*line)->data + break_pos);

		/* When requested, snip the one or two trailing spaces. */
		if (ISSET(TRIM_BLANKS)) {
			while (break_pos > 0 && (*line)->data[break_pos - 1] == ' ')
				break_pos--;
		}

		/* Now actually break the current line, and go to the next. */
		(*line)->data[break_pos] = '\0';
		*line = (*line)->next;
	}

	/* When possible, go to the line after the rewrapped paragraph. */
	if ((*line)->next != NULL)
		*line = (*line)->next;
}

/* Justify the lines of the given paragraph (that starts at *line, and consists
 * of 'count' lines) so they all fit within the target width (wrap_at) and have
 * their whitespace normalized. */
void justify_paragraph(linestruct **line, size_t count)
{
	linestruct *sampleline;
		/* The line from which the indentation is copied. */
	size_t quot_len;
		/* Length of the quote part. */
	size_t lead_len;
		/* Length of the quote part plus the indentation part. */
	char *lead_string;
		/* The quote+indent stuff that is copied from the sample line. */

	/* The sample line is either the only line or the second line. */
	sampleline = (count == 1 ? *line : (*line)->next);

	/* Copy the leading part (quoting + indentation) of the sample line. */
	quot_len = quote_length(sampleline->data);
	lead_len = quot_len + indent_length(sampleline->data + quot_len);
	lead_string = measured_copy(sampleline->data, lead_len);

	/* Concatenate all lines of the paragraph into a single line. */
	concat_paragraph(*line, count);

	/* Change all blank characters to spaces and remove excess spaces. */
	squeeze(*line, quot_len + indent_length((*line)->data + quot_len));

	/* Rewrap the line into multiple lines, accounting for the leading part. */
	rewrap_paragraph(line, lead_string, lead_len);

	free(lead_string);
}

/* Justify the current paragraph, or the entire buffer when full_justify is
 * TRUE.  But if the mark is on, justify only the marked text instead. */
void do_justify(bool full_justify)
{
	size_t linecount;
		/* The number of lines in the original paragraph. */
	linestruct *startline;
		/* The line where the paragraph or region starts. */
	linestruct *endline;
		/* The line where the paragraph or region ends. */
	size_t start_x;
		/* The x position where the paragraph or region starts. */
	size_t end_x;
		/* The x position where the paragraph or region ends. */
	linestruct *was_cutbuffer = cutbuffer;
		/* The old cutbuffer, so we can justify in the current cutbuffer. */
	linestruct *jusline;
		/* The line that we're justifying in the current cutbuffer. */
#ifndef NANO_TINY
	bool before_eol = FALSE;
		/* Whether the end of a marked region is before the end of its line. */
	char *primary_lead = NULL;
		/* The leading part (quoting + indentation) of the first line
		 * of the paragraph where the marked region begins. */
	size_t primary_len = 0;
		/* The length (in bytes) of the above first-line leading part. */
	char *secondary_lead = NULL;
		/* The leading part for lines after the first one. */
	size_t secondary_len = 0;
		/* The length of that later lead. */

	/* TRANSLATORS: This one goes with Undid/Redid messages. */
	add_undo(COUPLE_BEGIN, N_("justification"));

	/* If the mark is on, do as Pico: treat all marked text as one paragraph. */
	if (openfile->mark) {
		size_t quot_len, fore_len, other_quot_len, other_white_len;
		linestruct *sampleline;

		get_region(&startline, &start_x, &endline, &end_x);

		/* When the marked region is empty, do nothing. */
		if (startline == endline && start_x == end_x) {
			statusline(ALERT, _("Selection is empty"));
			discard_until(openfile->undotop->next);
			return;
		}

		quot_len = quote_length(startline->data);
		fore_len = quot_len + indent_length(startline->data + quot_len);

		/* When the region starts IN the lead, take the whole lead. */
		if (start_x <= fore_len)
			start_x = 0;

		/* Recede over blanks before the region.  This effectively snips
		 * trailing blanks from what will become the preceding paragraph. */
		while (start_x > 0 && is_blank_char(&startline->data[start_x - 1]))
			start_x = step_left(startline->data, start_x);

		quot_len = quote_length(endline->data);
		fore_len = quot_len + indent_length(endline->data + quot_len);

		/* When the region ends IN the lead, take the whole lead. */
		if (0 < end_x && end_x < fore_len)
			end_x = fore_len;

		/* When not at the left edge, advance over blanks after the region. */
		while (end_x > 0 && is_blank_char(&endline->data[end_x]))
			end_x = step_right(endline->data, end_x);

		sampleline = startline;

		/* Find the first line of the paragraph in which the region starts. */
		while (sampleline->prev && inpar(sampleline) && !begpar(sampleline, 0))
			sampleline = sampleline->prev;

		/* Ignore lines that contain no text. */
		while (sampleline->next && !inpar(sampleline))
			sampleline = sampleline->next;

		/* Store the leading part that is to be used for the new paragraph. */
		quot_len = quote_length(sampleline->data);
		primary_len = quot_len + indent_length(sampleline->data + quot_len);
		primary_lead = measured_copy(sampleline->data, primary_len);

		if (sampleline->next && startline != endline)
			sampleline = sampleline->next;

		/* Copy the leading part that is to be used for the new paragraph after
		 * its first line (if any): the quoting of the first line, plus the
		 * indentation of the second line. */
		other_quot_len = quote_length(sampleline->data);
		other_white_len = indent_length(sampleline->data + other_quot_len);

		secondary_len = quot_len + other_white_len;
		secondary_lead = charalloc(secondary_len + 1);

		strncpy(secondary_lead, startline->data, quot_len);
		strncpy(secondary_lead + quot_len, sampleline->data + other_quot_len,
													other_white_len);
		secondary_lead[secondary_len] = '\0';

		/* Include preceding and succeeding leads into the marked region. */
		openfile->mark = startline;
		openfile->mark_x = start_x;
		openfile->current = endline;
		openfile->current_x = end_x;

		linecount = endline->lineno - startline->lineno + (end_x > 0 ? 1 : 0);

		/* Remember whether the end of the region was before the end-of-line. */
		before_eol = endline->data[end_x] != '\0';
	} else
#endif /* NANO_TINY */
	{
		/* When justifying the entire buffer, start at the top.  Otherwise, when
		 * in a paragraph but not at its beginning, move back to its first line. */
		if (full_justify)
			openfile->current = openfile->filetop;
		else if (inpar(openfile->current) && !begpar(openfile->current, 0))
			do_para_begin(&openfile->current);

		/* Find the first line of the paragraph(s) to be justified.  If the
		 * search fails, there is nothing to justify, and we will be on the
		 * last line of the file, so put the cursor at the end of it. */
		if (!find_paragraph(&openfile->current, &linecount)) {
			openfile->current_x = strlen(openfile->filebot->data);
#ifndef NANO_TINY
			discard_until(openfile->undotop->next);
#endif
			refresh_needed = TRUE;
			return;
		}

		/* Set the starting point of the paragraph. */
		startline = openfile->current;
		start_x = 0;

		/* Set the end point of the paragraph. */
		if (full_justify)
			endline = openfile->filebot;
		else {
			endline = startline;
			for (size_t count = linecount; count > 1; count--)
				endline = endline->next;
		}

		/* When possible, step one line further; otherwise, to line's end. */
		if (endline->next != NULL) {
			endline = endline->next;
			end_x = 0;
		} else
			end_x = strlen(endline->data);
	}

#ifndef NANO_TINY
	add_undo(CUT, NULL);
#endif
	/* Do the equivalent of a marked cut into an empty cutbuffer. */
	cutbuffer = NULL;
	extract_segment(startline, start_x, endline, end_x);
#ifndef NANO_TINY
	update_undo(CUT);

	if (openfile->mark) {
		linestruct *line = cutbuffer;
		size_t quot_len = quote_length(line->data);
		size_t fore_len = quot_len + indent_length(line->data + quot_len);
		size_t text_len = strlen(line->data) - fore_len;

		/* If the extracted region begins with any leading part, trim it. */
		if (fore_len > 0)
			memmove(line->data, line->data + fore_len, text_len + 1);

		/* Then copy back in the leading part that it should have. */
		if (primary_len > 0) {
			line->data = charealloc(line->data, primary_len + text_len + 1);
			memmove(line->data + primary_len, line->data, text_len + 1);
			strncpy(line->data, primary_lead, primary_len);
		}

		/* Now justify the extracted region. */
		concat_paragraph(cutbuffer, linecount);
		squeeze(cutbuffer, primary_len);
		rewrap_paragraph(&line, secondary_lead, secondary_len);

		/* If the marked region started in the middle of a line,
		 * insert a newline before the new paragraph. */
		if (start_x > 0) {
			cutbuffer->prev = make_new_node(NULL);
			cutbuffer->prev->data = copy_of("");
			cutbuffer->prev->next = cutbuffer;
			cutbuffer = cutbuffer->prev;
		}

		/* If the marked region ended in the middle of a line,
		 * insert a newline after the new paragraph. */
		if (end_x > 0 && before_eol) {
			line->next = make_new_node(line);
			line->next->data = copy_of(primary_lead);
		}

		free(secondary_lead);
		free(primary_lead);
	} else
#endif
	{
		/* Prepare to justify the text we just put in the cutbuffer. */
		jusline = cutbuffer;

		/* Justify the current paragraph. */
		justify_paragraph(&jusline, linecount);

		/* When justifying the entire buffer, find and justify all paragraphs. */
		if (full_justify) {
			while (find_paragraph(&jusline, &linecount)) {
				justify_paragraph(&jusline, linecount);

				if (jusline->next == NULL)
					break;
			}
		}
	}

#ifndef NANO_TINY
	add_undo(PASTE, NULL);
	if (full_justify && !openfile->mark && !cutbuffer->has_anchor)
		openfile->current->has_anchor = FALSE;
#endif
	/* Do the equivalent of a paste of the justified text. */
	ingraft_buffer(cutbuffer);
#ifndef NANO_TINY
	update_undo(PASTE);

	/* After justifying a backward-marked text, swap mark and cursor. */
	if (openfile->mark && !mark_is_before_cursor()) {
		linestruct *bottom = openfile->current;
		size_t bottom_x = openfile->current_x;

		openfile->current = openfile->mark;
		openfile->current_x = openfile->mark_x;
		openfile->mark = bottom;
		openfile->mark_x = bottom_x;
	}

	add_undo(COUPLE_END, N_("justification"));
#endif

	/* We're done justifying.  Restore the old cutbuffer. */
	cutbuffer = was_cutbuffer;

	/* Show what we justified on the status bar. */
#ifndef NANO_TINY
	if (openfile->mark)
		statusbar(_("Justified selection"));
	else
#endif
	if (full_justify)
		statusbar(_("Justified file"));
	else
		statusbar(_("Justified paragraph"));

	/* Set the desired screen column (always zero, except at EOF). */
	openfile->placewewant = xplustabs();

	set_modified();
	refresh_needed = TRUE;
	shift_held = TRUE;
}

/* Justify the current paragraph. */
void do_justify_void(void)
{
	do_justify(FALSE);
}

/* Justify the entire file. */
void do_full_justify(void)
{
	do_justify(TRUE);
	ran_a_tool = TRUE;
}
#endif /* ENABLE_JUSTIFY */

#if defined(ENABLE_SPELLER) || defined (ENABLE_COLOR)
/* Set up an argument list for executing the given command. */
void construct_argument_list(char ***arguments, char *command, char *filename)
{
	char *copy_of_command = copy_of(command);
	char *element = strtok(copy_of_command, " ");
	int count = 2;

	while (element != NULL) {
		*arguments = (char **)nrealloc(*arguments, ++count * sizeof(char *));
		(*arguments)[count - 3] = element;
		element = strtok(NULL, " ");
	}

	(*arguments)[count - 2] = filename;
	(*arguments)[count - 1] = NULL;
}

/* Open the specified file, and if that succeeds, remove the text of the marked
 * region or of the entire buffer and read the file contents into its place. */
bool replace_buffer(const char *filename, undo_type action, const char *operation)
{
	linestruct *was_cutbuffer = cutbuffer;
	int descriptor;
	FILE *stream;

	descriptor = open_file(filename, FALSE, &stream);

	if (descriptor < 0)
		return FALSE;

	cutbuffer = NULL;

#ifndef NANO_TINY
	add_undo(COUPLE_BEGIN, operation);

	/* Cut either the marked region or the whole buffer. */
	add_undo(action, NULL);
	do_snip(openfile->mark != NULL, openfile->mark == NULL, FALSE);
	update_undo(action);
#else
	do_snip(FALSE, TRUE, FALSE);
#endif

	/* Discard what was cut. */
	free_lines(cutbuffer);
	cutbuffer = was_cutbuffer;

	/* Insert the spell-checked file into the cleared area. */
	read_file(stream, descriptor, filename, TRUE);

#ifndef NANO_TINY
	add_undo(COUPLE_END, operation);
#endif
	return TRUE;
}

/* Execute the given program, with the given temp file as last argument. */
void treat(char *tempfile_name, char *theprogram, bool spelling)
{
	ssize_t lineno_save = openfile->current->lineno;
	size_t current_x_save = openfile->current_x;
	size_t pww_save = openfile->placewewant;
	bool was_at_eol = (openfile->current->data[openfile->current_x] == '\0');
	struct stat fileinfo;
	long timestamp_sec = 0;
	long timestamp_nsec = 0;
	static char **arguments = NULL;
	pid_t thepid;
	int program_status, errornumber;
	bool replaced = FALSE;

	/* Stat the temporary file.  If that succeeds and its size is zero,
	 * there is nothing to do; otherwise, store its time of modification. */
	if (stat(tempfile_name, &fileinfo) == 0) {
		if (fileinfo.st_size == 0) {
#ifndef NANO_TINY
			if (spelling && openfile->mark)
				statusline(ALERT, _("Selection is empty"));
			else
#endif
				statusbar(_("Buffer is empty"));
			return;
		}

		timestamp_sec = (long)fileinfo.st_mtim.tv_sec;
		timestamp_nsec = (long)fileinfo.st_mtim.tv_nsec;
	}

	/* Exit from curses mode to give the program control of the terminal. */
	endwin();

	construct_argument_list(&arguments, theprogram, tempfile_name);

	/* Fork a child process and run the given program in it. */
	if ((thepid = fork()) == 0) {
		execvp(arguments[0], arguments);

		/* Terminate the child if the program is not found. */
		exit(9);
	} else if (thepid > 0) {
		/* Block SIGWINCHes while waiting for the forked program to end,
		 * so nano doesn't get pushed past the wait(). */
		block_sigwinch(TRUE);
		wait(&program_status);
		block_sigwinch(FALSE);
	}

	errornumber = errno;

	/* Restore the terminal state and reenter curses mode. */
	terminal_init();
	doupdate();

	if (thepid < 0) {
		statusline(ALERT, _("Could not fork: %s"), strerror(errornumber));
		return;
	} else if (!WIFEXITED(program_status) || WEXITSTATUS(program_status) > 2) {
		statusline(ALERT, _("Error invoking '%s'"), arguments[0]);
		return;
	} else if (WEXITSTATUS(program_status) != 0)
		statusline(ALERT, _("Program '%s' complained"), arguments[0]);

	/* When the temporary file wasn't touched, say so and leave. */
	if (timestamp_sec > 0 && stat(tempfile_name, &fileinfo) == 0 &&
					(long)fileinfo.st_mtim.tv_sec == timestamp_sec &&
					(long)fileinfo.st_mtim.tv_nsec == timestamp_nsec) {
		statusbar(_("Nothing changed"));
		return;
	}

#ifndef NANO_TINY
	/* Replace the marked text (or entire text) with the corrected text. */
	if (spelling && openfile->mark) {
		ssize_t was_mark_lineno = openfile->mark->lineno;
		bool upright = mark_is_before_cursor();

		replaced = replace_buffer(tempfile_name, CUT, "spelling correction");

		/* Adjust the end point of the marked region for any change in
		 * length of the region's last line. */
		if (upright)
			current_x_save = openfile->current_x;
		else
			openfile->mark_x = openfile->current_x;

		/* Restore the mark. */
		openfile->mark = line_from_number(was_mark_lineno);
	} else
#endif
	{
		openfile->current = openfile->filetop;
		openfile->current_x = 0;

		replaced = replace_buffer(tempfile_name, CUT_TO_EOF,
					/* TRANSLATORS: The next two go with Undid/Redid messages. */
					(spelling ? N_("spelling correction") : N_("formatting")));
	}

	/* Go back to the old position. */
	goto_line_posx(lineno_save, current_x_save);
	if (was_at_eol || openfile->current_x > strlen(openfile->current->data))
		openfile->current_x = strlen(openfile->current->data);

	if (replaced) {
#ifndef NANO_TINY
		openfile->filetop->has_anchor = FALSE;
		update_undo(COUPLE_END);
#endif
	}

	openfile->placewewant = pww_save;
	adjust_viewport(STATIONARY);

	if (spelling)
		statusbar(_("Finished checking spelling"));
	else
		statusbar(_("Buffer has been processed"));
}
#endif /* ENABLE_SPELLER || ENABLE_COLOR */

#ifdef ENABLE_SPELLER
/* Let the user edit the misspelled word.  Return FALSE if the user cancels. */
bool fix_spello(const char *word)
{
	char *save_search;
	size_t firstcolumn_save = openfile->firstcolumn;
	size_t current_x_save = openfile->current_x;
	linestruct *edittop_save = openfile->edittop;
	linestruct *current_save = openfile->current;
		/* Save where we are. */
	bool proceed = FALSE;
		/* The return value of this function. */
	bool result;
		/* The return value of searching for a misspelled word. */
#ifndef NANO_TINY
	linestruct *top, *bot;
	size_t top_x, bot_x;
	bool right_side_up = (openfile->mark && mark_is_before_cursor());
#endif

	/* Save the current search string, then set it to the misspelled word. */
	save_search = last_search;
	last_search = copy_of(word);

#ifndef NANO_TINY
	/* If the mark is on, start at the beginning of the marked region. */
	if (openfile->mark) {
		get_region(&top, &top_x, &bot, &bot_x);
		/* If the region is marked normally, swap the end points, so that
		 * (current, current_x) (where searching starts) is at the top. */
		if (right_side_up) {
			openfile->current = top;
			openfile->current_x = top_x;
			openfile->mark = bot;
			openfile->mark_x = bot_x;
		}
	} else
#endif
	/* Otherwise, start from the top of the file. */
	{
		openfile->current = openfile->filetop;
		openfile->current_x = 0;
	}

	/* Find the first whole occurrence of word. */
	result = findnextstr(word, TRUE, INREGION, NULL, FALSE, NULL, 0);

	/* If the word isn't found, alert the user; if it is, allow correction. */
	if (result == 0) {
		statusline(ALERT, _("Unfindable word: %s"), word);
		lastmessage = VACUUM;
		proceed = TRUE;
		napms(2800);
	} else if (result == 1) {
		spotlighted = TRUE;
		light_from_col = xplustabs();
		light_to_col = light_from_col + breadth(word);
#ifndef NANO_TINY
		linestruct *saved_mark = openfile->mark;
		openfile->mark = NULL;
#endif
		edit_refresh();

		put_cursor_at_end_of_answer();

		/* Let the user supply a correctly spelled alternative. */
		proceed = (do_prompt(MSPELL, word, NULL, edit_refresh,
								/* TRANSLATORS: This is a prompt. */
								_("Edit a replacement")) != -1);

		spotlighted = FALSE;

#ifndef NANO_TINY
		openfile->mark = saved_mark;
#endif

		/* If a replacement was given, go through all occurrences. */
		if (proceed && strcmp(word, answer) != 0) {
			do_replace_loop(word, TRUE, current_save, &current_x_save);

			/* TRANSLATORS: Shown after fixing misspellings in one word. */
			statusbar(_("Next word..."));
			napms(400);
		}
	}

#ifndef NANO_TINY
	if (openfile->mark) {
		/* Restore the (compensated) end points of the marked region. */
		if (right_side_up) {
			openfile->current = openfile->mark;
			openfile->current_x = openfile->mark_x;
			openfile->mark = top;
			openfile->mark_x = top_x;
		} else {
			openfile->current = top;
			openfile->current_x = top_x;
		}
	} else
#endif
	{
		/* Restore the (compensated) cursor position. */
		openfile->current = current_save;
		openfile->current_x = current_x_save;
	}

	/* Restore the string that was last searched for. */
	free(last_search);
	last_search = save_search;

	/* Restore the viewport to where it was. */
	openfile->edittop = edittop_save;
	openfile->firstcolumn = firstcolumn_save;

	return proceed;
}

/* Run a spell-check on the given file, using 'spell' to produce a list of all
 * misspelled words, then feeding those through 'sort' and 'uniq' to obtain an
 * alphabetical list, which words are then offered one by one to the user for
 * correction. */
void do_int_speller(const char *tempfile_name)
{
	char *misspellings, *pointer, *oneword;
	long pipesize;
	size_t buffersize, bytesread, totalread;
	int spell_fd[2], sort_fd[2], uniq_fd[2], tempfile_fd = -1;
	pid_t pid_spell, pid_sort, pid_uniq;
	int spell_status, sort_status, uniq_status;

	/* Create all three pipes up front. */
	if (pipe(spell_fd) == -1 || pipe(sort_fd) == -1 || pipe(uniq_fd) == -1) {
		statusline(ALERT, _("Could not create pipe: %s"), strerror(errno));
		return;
	}

	statusbar(_("Invoking spell checker..."));

	/* Fork a process to run spell in. */
	if ((pid_spell = fork()) == 0) {
		/* Child: open the temporary file that holds the text to be checked. */
		if ((tempfile_fd = open(tempfile_name, O_RDONLY)) == -1)
			exit(6);

		/* Connect standard input to the temporary file. */
		if (dup2(tempfile_fd, STDIN_FILENO) != STDIN_FILENO)
			exit(7);

		/* Connect standard output to the write end of the first pipe. */
		if (dup2(spell_fd[1], STDOUT_FILENO) != STDOUT_FILENO)
			exit(8);

		close(tempfile_fd);
		close(spell_fd[0]);
		close(spell_fd[1]);

		/* Try to run 'hunspell'; if that fails, fall back to 'spell'. */
		execlp("hunspell", "hunspell", "-l", NULL);
		execlp("spell", "spell", NULL);

		/* Indicate failure when neither speller was found. */
		exit(9);
	}

	/* Parent: close the unused write end of the first pipe. */
	close(spell_fd[1]);

	/* Fork a process to run sort in. */
	if ((pid_sort = fork()) == 0) {
		/* Connect standard input to the read end of the first pipe. */
		if (dup2(spell_fd[0], STDIN_FILENO) != STDIN_FILENO)
			exit(7);

		/* Connect standard output to the write end of the second pipe. */
		if (dup2(sort_fd[1], STDOUT_FILENO) != STDOUT_FILENO)
			exit(8);

		close(spell_fd[0]);
		close(sort_fd[0]);
		close(sort_fd[1]);

		/* Now run the sort program.  Use -f to mix upper and lower case. */
		execlp("sort", "sort", "-f", NULL);

		exit(9);
	}

	close(spell_fd[0]);
	close(sort_fd[1]);

	/* Fork a process to run uniq in. */
	if ((pid_uniq = fork()) == 0) {
		if (dup2(sort_fd[0], STDIN_FILENO) != STDIN_FILENO)
			exit(7);

		if (dup2(uniq_fd[1], STDOUT_FILENO) != STDOUT_FILENO)
			exit(8);

		close(sort_fd[0]);
		close(uniq_fd[0]);
		close(uniq_fd[1]);

		execlp("uniq", "uniq", NULL);

		exit(9);
	}

	close(sort_fd[0]);
	close(uniq_fd[1]);

	/* When some child process was not forked successfully... */
	if (pid_spell < 0 || pid_sort < 0 || pid_uniq < 0) {
		statusline(ALERT, _("Could not fork: %s"), strerror(errno));
		close(uniq_fd[0]);
		return;
	}

	/* Get the system pipe buffer size. */
	pipesize = fpathconf(uniq_fd[0], _PC_PIPE_BUF);

	if (pipesize < 1) {
		statusline(ALERT, _("Could not get size of pipe buffer"));
		close(uniq_fd[0]);
		return;
	}

	/* Leave curses mode so that error messages go to the original screen. */
	endwin();

	/* Block SIGWINCHes while reading misspelled words from the third pipe. */
	block_sigwinch(TRUE);

	totalread = 0;
	buffersize = pipesize + 1;
	misspellings = charalloc(buffersize);
	pointer = misspellings;

	while ((bytesread = read(uniq_fd[0], pointer, pipesize)) > 0) {
		totalread += bytesread;
		buffersize += pipesize;
		misspellings = charealloc(misspellings, buffersize);
		pointer = misspellings + totalread;
	}

	*pointer = '\0';
	close(uniq_fd[0]);

	block_sigwinch(FALSE);

	/* Re-enter curses mode. */
	terminal_init();
	doupdate();

	/* Do any replacements case-sensitively, forward, and without regexes. */
	SET(CASE_SENSITIVE);
	UNSET(BACKWARDS_SEARCH);
	UNSET(USE_REGEXP);

	pointer = misspellings;
	oneword = misspellings;

	/* Process each of the misspelled words. */
	while (*pointer != '\0') {
		if ((*pointer == '\r') || (*pointer == '\n')) {
			*pointer = '\0';
			if (oneword != pointer) {
				if (!fix_spello(oneword)) {
					oneword = pointer;
					break;
				}
			}
			oneword = pointer + 1;
		}
		pointer++;
	}

	/* Special case: the last word doesn't end with '\r' or '\n'. */
	if (oneword != pointer)
		fix_spello(oneword);

	free(misspellings);
	refresh_needed = TRUE;

	/* Process the end of the three processes. */
	waitpid(pid_spell, &spell_status, 0);
	waitpid(pid_sort, &sort_status, 0);
	waitpid(pid_uniq, &uniq_status, 0);

	if (WIFEXITED(uniq_status) == 0 || WEXITSTATUS(uniq_status))
		statusline(ALERT, _("Error invoking \"uniq\""));
	else if (WIFEXITED(sort_status) == 0 || WEXITSTATUS(sort_status))
		statusline(ALERT, _("Error invoking \"sort\""));
	else if (WIFEXITED(spell_status) == 0 || WEXITSTATUS(spell_status))
		statusline(ALERT, _("Error invoking \"spell\""));
	else
		statusbar(_("Finished checking spelling"));
}

/* Spell check the current file.  If an alternate spell checker is
 * specified, use it.  Otherwise, use the internal spell checker. */
void do_spell(void)
{
	FILE *stream;
	char *temp_name;
	unsigned stash[sizeof(flags) / sizeof(flags[0])];
	bool okay;

	ran_a_tool = TRUE;

	if (in_restricted_mode())
		return;

	temp_name = safe_tempfile(&stream);

	if (temp_name == NULL) {
		statusline(ALERT, _("Error writing temp file: %s"), strerror(errno));
		return;
	}

	/* Save the settings of the global flags. */
	memcpy(stash, flags, sizeof(flags));

	/* Don't add an extra newline when writing out the (selected) text. */
	SET(NO_NEWLINES);

#ifndef NANO_TINY
	if (openfile->mark)
		okay = write_marked_file(temp_name, stream, TRUE, OVERWRITE);
	else
#endif
		okay = write_file(temp_name, stream, TRUE, OVERWRITE, TRUE);

	if (!okay) {
		statusline(ALERT, _("Error writing temp file: %s"), strerror(errno));
		free(temp_name);
		return;
	}

	blank_bottombars();

	if (alt_speller)
		treat(temp_name, alt_speller, TRUE);
	else
		do_int_speller(temp_name);

	unlink(temp_name);
	free(temp_name);

	/* Restore the settings of the global flags. */
	memcpy(flags, stash, sizeof(flags));

	/* Ensure the help lines will be redrawn and a selection is retained. */
	currmenu = MMOST;
	shift_held = TRUE;
}
#endif /* ENABLE_SPELLER */

#ifdef ENABLE_COLOR
/* Run a linting program on the current buffer. */
void do_linter(void)
{
	char *lintings, *pointer, *onelint;
	long pipesize;
	size_t buffersize, bytesread, totalread;
	bool parsesuccess = FALSE;
	int lint_status, lint_fd[2];
	pid_t pid_lint;
	bool helpless = ISSET(NO_HELP);
	static char **lintargs = NULL;
	lintstruct *lints = NULL, *tmplint = NULL, *curlint = NULL;
	time_t last_wait = 0;

	ran_a_tool = TRUE;

	if (in_restricted_mode())
		return;

	if (!openfile->syntax || !openfile->syntax->linter) {
		statusbar(_("No linter is defined for this type of file"));
		return;
	}

#ifndef NANO_TINY
	openfile->mark = NULL;
#endif
	edit_refresh();

	if (openfile->modified) {
		int choice = do_yesno_prompt(FALSE, _("Save modified buffer before linting?"));

		if (choice == -1) {
			statusbar(_("Cancelled"));
			return;
		} else if (choice == 1 && (do_writeout(FALSE, FALSE) != 1))
			return;
	}

	/* Create a pipe up front. */
	if (pipe(lint_fd) == -1) {
		statusline(ALERT, _("Could not create pipe: %s"), strerror(errno));
		return;
	}

	blank_bottombars();
	currmenu = MLINTER;
	statusbar(_("Invoking linter..."));

	construct_argument_list(&lintargs, openfile->syntax->linter, openfile->filename);

	/* Fork a process to run the linter in. */
	if ((pid_lint = fork()) == 0) {
		/* Redirect standard output and standard error into the pipe. */
		if (dup2(lint_fd[1], STDOUT_FILENO) != STDOUT_FILENO)
			exit(7);
		if (dup2(lint_fd[1], STDERR_FILENO) != STDERR_FILENO)
			exit(8);

		close(lint_fd[0]);
		close(lint_fd[1]);

		/* Start the linter program; we are using $PATH. */
		execvp(lintargs[0], lintargs);

		/* This is only reached when the linter is not found. */
		exit(9);
	}

	/* Parent continues here. */
	close(lint_fd[1]);

	/* If the child process was not forked successfully... */
	if (pid_lint < 0) {
		statusline(ALERT, _("Could not fork: %s"), strerror(errno));
		close(lint_fd[0]);
		return;
	}

	/* Get the system pipe buffer size. */
	pipesize = fpathconf(lint_fd[0], _PC_PIPE_BUF);

	if (pipesize < 1) {
		statusline(ALERT, _("Could not get size of pipe buffer"));
		close(lint_fd[0]);
		return;
	}

	/* Read in the returned syntax errors. */
	totalread = 0;
	buffersize = pipesize + 1;
	lintings = charalloc(buffersize);
	pointer = lintings;

	while ((bytesread = read(lint_fd[0], pointer, pipesize)) > 0) {
		totalread += bytesread;
		buffersize += pipesize;
		lintings = charealloc(lintings, buffersize);
		pointer = lintings + totalread;
	}

	*pointer = '\0';
	close(lint_fd[0]);

	/* Process the linter output. */
	pointer = lintings;
	onelint = lintings;

	while (*pointer != '\0') {
		if ((*pointer == '\r') || (*pointer == '\n')) {
			*pointer = '\0';
			if (onelint != pointer) {
				char *filename = NULL, *linestr = NULL, *maybecol = NULL;
				char *message = copy_of(onelint);

				/* At the moment we handle the following formats:
				 *
				 * filenameorcategory:line:column:message (e.g. splint)
				 * filenameorcategory:line,column:message (e.g. pylint)
				 * filenameorcategory:line:message        (e.g. pyflakes) */
				if (strstr(message, ": ") != NULL) {
					filename = strtok(onelint, ":");
					if ((linestr = strtok(NULL, ":")) != NULL) {
						if ((maybecol = strtok(NULL, ":")) != NULL) {
							ssize_t tmplineno = 0, tmpcolno = 0;
							char *tmplinecol;

							tmplineno = strtol(linestr, NULL, 10);
							if (tmplineno <= 0) {
								pointer++;
								free(message);
								continue;
							}

							tmpcolno = strtol(maybecol, NULL, 10);
							/* Check if the middle field is in comma format. */
							if (tmpcolno <= 0) {
								strtok(linestr, ",");
								if ((tmplinecol = strtok(NULL, ",")) != NULL)
									tmpcolno = strtol(tmplinecol, NULL, 10);
								else
									tmpcolno = 1;
							}

							/* Nice.  We have a lint message we can use. */
							parsesuccess = TRUE;
							tmplint = curlint;
							curlint = nmalloc(sizeof(lintstruct));
							curlint->next = NULL;
							curlint->prev = tmplint;
							if (curlint->prev != NULL)
								curlint->prev->next = curlint;
							curlint->msg = copy_of(message);
							curlint->lineno = tmplineno;
							curlint->colno = tmpcolno;
							curlint->filename = copy_of(filename);

							if (lints == NULL)
								lints = curlint;
						}
					}
				}
				free(message);
			}
			onelint = pointer + 1;
		}
		pointer++;
	}

	free(lintings);

	/* Process the end of the linting process. */
	waitpid(pid_lint, &lint_status, 0);

	if (!WIFEXITED(lint_status) || WEXITSTATUS(lint_status) > 2) {
		statusline(ALERT, _("Error invoking '%s'"), openfile->syntax->linter);
		return;
	}

	if (!parsesuccess) {
		statusline(HUSH, _("Got 0 parsable lines from command: %s"),
						openfile->syntax->linter);
		return;
	}

	if (helpless && LINES > 4) {
		UNSET(NO_HELP);
		window_init();
	}

	/* Show that we are in the linter now. */
	titlebar(NULL);
	bottombars(MLINTER);

	tmplint = NULL;
	curlint = lints;

	while (TRUE) {
		int kbinput;
		functionptrtype func;
		struct stat lintfileinfo;

		if (stat(curlint->filename, &lintfileinfo) != -1 &&
					(openfile->statinfo == NULL ||
					openfile->statinfo->st_ino != lintfileinfo.st_ino)) {
#ifdef ENABLE_MULTIBUFFER
			const openfilestruct *started_at = openfile;

			openfile = openfile->next;
			while (openfile != started_at && (openfile->statinfo == NULL ||
						openfile->statinfo->st_ino != lintfileinfo.st_ino))
				openfile = openfile->next;

			if (openfile->statinfo == NULL ||
						openfile->statinfo->st_ino != lintfileinfo.st_ino) {
				char *msg = charalloc(1024 + strlen(curlint->filename));
				int choice;

				sprintf(msg, _("This message is for unopened file %s,"
							" open it in a new buffer?"), curlint->filename);
				choice = do_yesno_prompt(FALSE, msg);
				currmenu = MLINTER;
				free(msg);

				if (choice == -1) {
					statusbar(_("Cancelled"));
					break;
				} else if (choice == 1) {
					open_buffer(curlint->filename, TRUE);
				} else {
#endif
					char *dontwantfile = copy_of(curlint->filename);
					lintstruct *restlint = NULL;

					while (curlint != NULL) {
						if (strcmp(curlint->filename, dontwantfile) == 0) {
							if (curlint == lints)
								lints = curlint->next;
							else
								curlint->prev->next = curlint->next;
							if (curlint->next != NULL)
								curlint->next->prev = curlint->prev;
							tmplint = curlint;
							curlint = curlint->next;
							free(tmplint->msg);
							free(tmplint->filename);
							free(tmplint);
						} else {
							if (restlint == NULL)
								restlint = curlint;
							curlint = curlint->next;
						}
					}

					free(dontwantfile);

					if (restlint == NULL) {
						statusbar(_("No messages for this file"));
						break;
					} else {
						curlint = restlint;
						continue;
					}
#ifdef ENABLE_MULTIBUFFER
				}
			}
#endif
		}

		if (tmplint != curlint) {
			goto_line_posx(curlint->lineno, curlint->colno - 1);
			titlebar(NULL);
			adjust_viewport(CENTERING);
#ifdef ENABLE_LINENUMBERS
			confirm_margin();
#endif
			edit_refresh();
			statusline(NOTICE, curlint->msg);
			bottombars(MLINTER);
		}

		/* Place the cursor to indicate the affected line. */
		place_the_cursor();
		wnoutrefresh(edit);

		kbinput = get_kbinput(bottomwin, VISIBLE);

#ifndef NANO_TINY
		if (kbinput == KEY_WINCH)
			continue;
#endif
		func = func_from_key(&kbinput);
		tmplint = curlint;

		if (func == do_cancel || func == do_enter) {
			wipe_statusbar();
			break;
		} else if (func == do_help) {
			tmplint = NULL;
			do_help();
		} else if (func == do_page_up || func == to_prev_block) {
			if (curlint->prev != NULL)
				curlint = curlint->prev;
			else if (last_wait != time(NULL)) {
				statusbar(_("At first message"));
				beep();
				napms(600);
				last_wait = time(NULL);
				statusline(NOTICE, curlint->msg);
			}
		} else if (func == do_page_down || func == to_next_block) {
			if (curlint->next != NULL)
				curlint = curlint->next;
			else if (last_wait != time(NULL)) {
				statusbar(_("At last message"));
				beep();
				napms(600);
				last_wait = time(NULL);
				statusline(NOTICE, curlint->msg);
			}
		} else
			beep();
	}

	for (curlint = lints; curlint != NULL;) {
		tmplint = curlint;
		curlint = curlint->next;
		free(tmplint->msg);
		free(tmplint->filename);
		free(tmplint);
	}

	if (helpless) {
		SET(NO_HELP);
		window_init();
		refresh_needed = TRUE;
	}

	currmenu = MMOST;
	titlebar(NULL);
}

/* Run a manipulation program on the contents of the buffer. */
void do_formatter(void)
{
	FILE *stream;
	char *temp_name;
	bool okay = FALSE;

	ran_a_tool = TRUE;

	if (in_restricted_mode())
		return;

	if (!openfile->syntax || !openfile->syntax->formatter) {
		statusbar(_("No formatter is defined for this type of file"));
		return;
	}

#ifndef NANO_TINY
	openfile->mark = NULL;
#endif

	temp_name = safe_tempfile(&stream);

	if (temp_name != NULL)
		okay = write_file(temp_name, stream, TRUE, OVERWRITE, TRUE);

	if (!okay) {
		statusline(ALERT, _("Error writing temp file: %s"), strerror(errno));
		free(temp_name);
		return;
	}

	treat(temp_name, openfile->syntax->formatter, FALSE);

	unlink(temp_name);
	free(temp_name);
}
#endif /* ENABLE_COLOR */

#ifndef NANO_TINY
/* Our own version of "wc".  Note that its character counts are in
 * multibyte characters instead of single-byte characters. */
void do_wordlinechar_count(void)
{
	linestruct *was_current = openfile->current;
	size_t was_x = openfile->current_x;
	linestruct *topline, *botline;
	size_t top_x, bot_x;
	size_t words = 0, chars = 0;
	ssize_t lines = 0;

	/* Set the start and end point of the area to measure: either the marked
	 * region or the whole buffer.  Then compute the number of characters. */
	if (openfile->mark) {
		get_region(&topline, &top_x, &botline, &bot_x);

		if (topline != botline)
			chars = number_of_characters_in(topline->next, botline) + 1;

		chars += mbstrlen(topline->data + top_x) - mbstrlen(botline->data + bot_x);
	} else {
		topline = openfile->filetop;
		top_x = 0;
		botline = openfile->filebot;
		bot_x = strlen(botline->data);

		chars = openfile->totsize;
	}

	/* Compute the number of lines. */
	lines = botline->lineno - topline->lineno;
	lines += (bot_x == 0 || (topline == botline && top_x == bot_x)) ? 0 : 1;

	openfile->current = topline;
	openfile->current_x = top_x;

	/* Keep stepping to the next word (considering punctuation as part of a
	 * word, as "wc -w" does), until we reach the end of the relevant area,
	 * incrementing the word count for each successful step. */
	while (openfile->current->lineno < botline->lineno ||
				(openfile->current == botline && openfile->current_x < bot_x)) {
		if (do_next_word(FALSE, TRUE))
			words++;
	}

	/* Restore where we were. */
	openfile->current = was_current;
	openfile->current_x = was_x;

	/* Display the total word, line, and character counts on the status bar. */
	statusline(HUSH, _("%sWords: %zu  Lines: %zd  Chars: %zu"), openfile->mark ?
						_("In Selection:  ") : "", words, lines, chars);
}
#endif /* !NANO_TINY */

/* Get verbatim input. */
void do_verbatim_input(void)
{
	size_t count = 1;
	char *bytes;

	/* TRANSLATORS: Shown when the next keystroke will be inserted verbatim. */
	statusbar(_("Verbatim Input"));
	place_the_cursor();

	/* Read in the first one or two bytes of the next keystroke. */
	bytes = get_verbatim_kbinput(edit, &count);

	/* When something valid was obtained, unsuppress cursor-position display,
	 * insert the bytes into the edit buffer, and blank the status bar. */
	if (count > 0) {
		if (ISSET(CONSTANT_SHOW))
			lastmessage = VACUUM;

		if (count < 999)
			inject(bytes, count);

		wipe_statusbar();
	} else
		/* TRANSLATORS: An invalid verbatim Unicode code was typed. */
		statusline(ALERT, _("Invalid code"));

	free(bytes);
}

#ifdef ENABLE_WORDCOMPLETION
/* Return a copy of the found completion candidate. */
char *copy_completion(char *text)
{
	char *word;
	size_t length = 0, index = 0;

	/* Find the end of the candidate word to get its length. */
	while (is_word_char(&text[length], FALSE))
		length = step_right(text, length);

	/* Now copy this candidate to a new string. */
	word = charalloc(length + 1);
	while (index < length)
		word[index++] = *(text++);
	word[index] = '\0';

	return word;
}

/* Look at the fragment the user has typed, then search the current buffer for
 * the first word that starts with this fragment, and tentatively complete the
 * fragment.  If the user types 'Complete' again, search and paste the next
 * possible completion. */
void complete_a_word(void)
{
	char *shard, *completion = NULL;
	size_t start_of_shard, shard_length = 0;
	size_t i = 0, j = 0;
	completion_word *some_word;
#ifdef ENABLE_WRAPPING
	bool was_set_wrapping = ISSET(BREAK_LONG_LINES);
#endif

	/* If this is a fresh completion attempt... */
	if (pletion_line == NULL) {
		/* Clear the list of words of a previous completion run. */
		while (list_of_completions != NULL) {
			completion_word *dropit = list_of_completions;
			list_of_completions = list_of_completions->next;
			free(dropit->word);
			free(dropit);
		}

		/* Prevent a completion from being merged with typed text. */
		openfile->last_action = OTHER;

		/* Initialize the starting point for searching. */
		pletion_line = openfile->filetop;
		pletion_x = 0;

		/* Wipe the "No further matches" message. */
		wipe_statusbar();
	} else {
		/* Remove the attempted completion from the buffer. */
		do_undo();
	}

	/* Find the start of the fragment that the user typed. */
	start_of_shard = openfile->current_x;
	while (start_of_shard > 0) {
		size_t oneleft = step_left(openfile->current->data, start_of_shard);

		if (!is_word_char(&openfile->current->data[oneleft], FALSE))
			break;
		start_of_shard = oneleft;
	}

	/* If there is no word fragment before the cursor, do nothing. */
	if (start_of_shard == openfile->current_x) {
		/* TRANSLATORS: Shown when no text is directly left of the cursor. */
		statusbar(_("No word fragment"));
		pletion_line = NULL;
		return;
	}

	shard = charalloc(openfile->current_x - start_of_shard + 1);

	/* Copy the fragment that has to be searched for. */
	while (start_of_shard < openfile->current_x)
		shard[shard_length++] = openfile->current->data[start_of_shard++];
	shard[shard_length] = '\0';

	/* Run through all of the lines in the buffer, looking for shard. */
	while (pletion_line != NULL) {
		ssize_t threshold = strlen(pletion_line->data) - shard_length - 1;
				/* The point where we can stop searching for shard. */

		/* Traverse the whole line, looking for shard. */
		for (i = pletion_x; (ssize_t)i < threshold; i++) {
			/* If the first byte doesn't match, run on. */
			if (pletion_line->data[i] != shard[0])
				continue;

			/* Compare the rest of the bytes in shard. */
			for (j = 1; j < shard_length; j++)
				if (pletion_line->data[i + j] != shard[j])
					break;

			/* If not all of the bytes matched, continue searching. */
			if (j < shard_length)
				continue;

			/* If the found match is not /longer/ than shard, skip it. */
			if (!is_word_char(&pletion_line->data[i + j], FALSE))
				continue;

			/* If the match is not a separate word, skip it. */
			if (i > 0 && is_word_char(&pletion_line->data[
								step_left(pletion_line->data, i)], FALSE))
				continue;

			/* If this match is the shard itself, ignore it. */
			if (pletion_line == openfile->current &&
								i == openfile->current_x - shard_length)
				continue;

			completion = copy_completion(pletion_line->data + i);

			/* Look among earlier attempted completions for a duplicate. */
			some_word = list_of_completions;
			while (some_word && strcmp(some_word->word, completion) != 0)
				some_word = some_word->next;

			/* If we've already tried this word, skip it. */
			if (some_word != NULL) {
				free(completion);
				continue;
			}

			/* Add the found word to the list of completions. */
			some_word = (completion_word *)nmalloc(sizeof(completion_word));
			some_word->word = completion;
			some_word->next = list_of_completions;
			list_of_completions = some_word;

#ifdef ENABLE_WRAPPING
			/* Temporarily disable wrapping so only one undo item is added. */
			UNSET(BREAK_LONG_LINES);
#endif
			/* Inject the completion into the buffer. */
			inject(&completion[shard_length], strlen(completion) - shard_length);

#ifdef ENABLE_WRAPPING
			/* If needed, reenable wrapping and wrap the current line. */
			if (was_set_wrapping) {
				SET(BREAK_LONG_LINES);
				do_wrap();
			}
#endif
			/* Set the position for a possible next search attempt. */
			pletion_x = ++i;

			free(shard);
			return;
		}

		pletion_line = pletion_line->next;
		pletion_x = 0;
	}

	/* The search has reached the end of the file. */
	if (list_of_completions != NULL) {
		statusline(ALERT, _("No further matches"));
		refresh_needed = TRUE;
	} else
		/* TRANSLATORS: Shown when there are zero possible completions. */
		statusline(ALERT, _("No matches"));

	free(shard);
}
#endif /* ENABLE_WORDCOMPLETION */
