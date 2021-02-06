/**************************************************************************
 *   browser.c  --  This file is part of GNU nano.                        *
 *                                                                        *
 *   Copyright (C) 2001-2011, 2013-2021 Free Software Foundation, Inc.    *
 *   Copyright (C) 2015-2016, 2020 Benno Schulenberg                      *
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

#ifdef ENABLE_BROWSER

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static char **filelist = NULL;
		/* The list of files to display in the file browser. */
static size_t list_length = 0;
		/* The number of files in the list. */
static size_t width = 0;
		/* The number of files that we can display per screen row. */
static size_t longest = 0;
		/* The number of columns in the longest filename in the list. */
static size_t selected = 0;
		/* The currently selected filename in the list; zero-based. */

/* Set filelist to the list of files contained in the directory path,
 * set list_length to the number of files in that list, set longest to
 * the width in columns of the longest filename in that list (between 15
 * and COLS), and set width to the number of files that we can display
 * per screen row.  And sort the list too. */
void read_the_list(const char *path, DIR *dir)
{
	size_t path_len = strlen(path), index = 0;
	const struct dirent *nextdir;

	longest = 0;

	/* Find the length of the longest filename in the current folder. */
	while ((nextdir = readdir(dir)) != NULL) {
		size_t name_len = breadth(nextdir->d_name);

		if (name_len > longest)
			longest = name_len;

		index++;
	}

	/* Put 10 characters' worth of blank space between columns of filenames
	 * in the list whenever possible, as Pico does. */
	longest += 10;

	/* If needed, make room for ".. (parent dir)". */
	if (longest < 15)
		longest = 15;
	/* Make sure we're not wider than the window. */
	if (longest > COLS)
		longest = COLS;

	rewinddir(dir);

	free_chararray(filelist, list_length);

	list_length = index;
	index = 0;

	filelist = nmalloc(list_length * sizeof(char *));

	while ((nextdir = readdir(dir)) != NULL && index < list_length) {
		/* Don't show the "." entry. */
		if (strcmp(nextdir->d_name, ".") == 0)
			continue;

		filelist[index] = nmalloc(path_len + strlen(nextdir->d_name) + 1);
		sprintf(filelist[index], "%s%s", path, nextdir->d_name);

		index++;
	}

	/* Maybe the number of files in the directory decreased between the
	 * first time we scanned and the second time.  index is the actual
	 * length of the file list, so record it. */
	list_length = index;

	/* Sort the list of names. */
	qsort(filelist, list_length, sizeof(char *), diralphasort);

	/* Calculate how many files fit on a line -- feigning room for two
	 * spaces beyond the right edge, and adding two spaces of padding
	 * between columns. */
	width = (COLS + 2) / (longest + 2);
}

/* Look for needle.  If we find it, set selected to its location.
 * Note that needle must be an exact match for a file in the list. */
void browser_select_dirname(const char *needle)
{
	size_t looking_at = 0;

	for (; looking_at < list_length; looking_at++) {
		if (strcmp(filelist[looking_at], needle) == 0) {
			selected = looking_at;
			break;
		}
	}

	/* If the sought name isn't found, move the highlight so that the
	 * changed selection will be noticed. */
	if (looking_at == list_length) {
		--selected;

		/* Make sure we stay within the available range. */
		if (selected >= list_length)
			selected = list_length - 1;
	}
}

/* Display at most a screenful of filenames from the gleaned filelist. */
void browser_refresh(void)
{
	int row = 0, col = 0;
		/* The current row and column while the list is getting displayed. */
	int the_row = 0, the_column = 0;
		/* The row and column of the selected item. */
	char *info;
		/* The additional information that we'll display about a file. */

	titlebar(present_path);
	blank_edit();

	for (size_t index = selected - selected % (editwinrows * width);
					index < list_length && row < editwinrows; index++) {
		const char *thename = tail(filelist[index]);
				/* The filename we display, minus the path. */
		size_t namelen = breadth(thename);
				/* The length of the filename in columns. */
		size_t infolen;
				/* The length of the file information in columns. */
		size_t infomaxlen = 7;
				/* The maximum length of the file information in columns:
				 * normally seven, but will be twelve for "(parent dir)". */
		bool dots = (COLS >= 15 && namelen >= longest - infomaxlen);
				/* Whether to put an ellipsis before the filename?  We don't
				 * waste space on dots when there are fewer than 15 columns. */
		char *disp = display_string(thename, dots ?
				namelen + infomaxlen + 4 - longest : 0, longest, FALSE, FALSE);
				/* The filename (or a fragment of it) in displayable format.
				 * When a fragment, account for dots plus one space padding. */
		struct stat state;

		/* If this is the selected item, draw its highlighted bar upfront, and
		 * remember its location to be able to place the cursor on it. */
		if (index == selected) {
			wattron(edit, interface_color_pair[SELECTED_TEXT]);
			mvwprintw(edit, row, col, "%*s", longest, " ");
			the_row = row;
			the_column = col;
		}

		/* If the name is too long, we display something like "...ename". */
		if (dots)
			mvwaddstr(edit, row, col, "...");
		mvwaddstr(edit, row, dots ? col + 3 : col, disp);

		col += longest;

		/* Show information about the file: "--" for symlinks (except when
		 * they point to a directory) and for files that have disappeared,
		 * "(dir)" for directories, and the file size for normal files. */
		if (lstat(filelist[index], &state) == -1 || S_ISLNK(state.st_mode)) {
			if (stat(filelist[index], &state) == -1 || !S_ISDIR(state.st_mode))
				info = copy_of("--");
			else
				/* TRANSLATORS: Try to keep this at most 7 characters. */
				info = copy_of(_("(dir)"));
		} else if (S_ISDIR(state.st_mode)) {
			if (strcmp(thename, "..") == 0) {
				/* TRANSLATORS: Try to keep this at most 12 characters. */
				info = copy_of(_("(parent dir)"));
				infomaxlen = 12;
			} else
				info = copy_of(_("(dir)"));
		} else {
			off_t result = state.st_size;
			char modifier;

			info = nmalloc(infomaxlen + 1);

			/* Massage the file size into a human-readable form. */
			if (state.st_size < (1 << 10))
				modifier = ' ';  /* bytes */
			else if (state.st_size < (1 << 20)) {
				result >>= 10;
				modifier = 'K';  /* kilobytes */
			} else if (state.st_size < (1 << 30)) {
				result >>= 20;
				modifier = 'M';  /* megabytes */
			} else {
				result >>= 30;
				modifier = 'G';  /* gigabytes */
			}

			/* Show the size if less than a terabyte, else show "(huge)". */
			if (result < (1 << 10))
				sprintf(info, "%4ju %cB", (intmax_t)result, modifier);
			else
				/* TRANSLATORS: Try to keep this at most 7 characters.
				 * If necessary, you can leave out the parentheses. */
				info = mallocstrcpy(info, _("(huge)"));
		}

		/* Make sure info takes up no more than infomaxlen columns. */
		infolen = breadth(info);
		if (infolen > infomaxlen) {
			info[actual_x(info, infomaxlen)] = '\0';
			infolen = infomaxlen;
		}

		mvwaddstr(edit, row, col - infolen, info);

		/* If this is the selected item, finish its highlighting. */
		if (index == selected)
			wattroff(edit, interface_color_pair[SELECTED_TEXT]);

		free(disp);
		free(info);

		/* Add some space between the columns. */
		col += 2;

		/* If the next entry will not fit on this row, move to next row. */
		if (col > COLS - longest) {
			row++;
			col = 0;
		}
	}

	/* If requested, put the cursor on the selected item and switch it on. */
	if (ISSET(SHOW_CURSOR)) {
		wmove(edit, the_row, the_column);
		curs_set(1);
	}

	wnoutrefresh(edit);
}

/* Look for the given needle in the list of files.  If forwards is TRUE,
 * search forward in the list; otherwise, search backward. */
void findfile(const char *needle, bool forwards)
{
	size_t looking_at = selected;
		/* The location in the file list of the filename we're looking at. */
	const char *thename;
		/* The plain filename, without the path. */
	unsigned stash[sizeof(flags) / sizeof(flags[0])];
		/* A storage place for the current flag settings. */

	/* Save the settings of all flags. */
	memcpy(stash, flags, sizeof(flags));

	/* Search forward, case insensitive, and without regexes. */
	UNSET(BACKWARDS_SEARCH);
	UNSET(CASE_SENSITIVE);
	UNSET(USE_REGEXP);

	/* Step through each filename in the list until a match is found or
	 * we've come back to the point where we started. */
	while (TRUE) {
		if (forwards) {
			if (looking_at++ == list_length - 1) {
				looking_at = 0;
				statusbar(_("Search Wrapped"));
			}
		} else {
			if (looking_at-- == 0) {
				looking_at = list_length - 1;
				statusbar(_("Search Wrapped"));
			}
		}

		/* Get the bare filename, without the path. */
		thename = tail(filelist[looking_at]);

		/* If the needle matches, we're done.  And if we're back at the file
		 * where we started, it is the only occurrence. */
		if (strstrwrapper(thename, needle, thename)) {
			if (looking_at == selected)
				statusbar(_("This is the only occurrence"));
			break;
		}

		/* If we're back at the beginning and didn't find any match... */
		if (looking_at == selected) {
			not_found_msg(needle);
			break;
		}
	}

	/* Restore the settings of all flags. */
	memcpy(flags, stash, sizeof(flags));

	/* Select the one we've found. */
	selected = looking_at;
}

/* Prepare the prompt and ask the user what to search for; then search for it.
 * If forwards is TRUE, search forward in the list; otherwise, search backward. */
void search_filename(bool forwards)
{
	char *thedefault;
	int response;

	/* If something was searched for before, show it between square brackets. */
	if (*last_search != '\0') {
		char *disp = display_string(last_search, 0, COLS / 3, FALSE, FALSE);

		thedefault = nmalloc(strlen(disp) + 7);
		/* We use (COLS / 3) here because we need to see more on the line. */
		sprintf(thedefault, " [%s%s]", disp,
				(breadth(last_search) > COLS / 3) ? "..." : "");
		free(disp);
	} else
		thedefault = copy_of("");

	/* Now ask what to search for. */
	response = do_prompt(MWHEREISFILE, "", &search_history,
						browser_refresh, "%s%s%s", _("Search"),
						/* TRANSLATORS: A modifier of the Search prompt. */
						!forwards ? _(" [Backwards]") : "", thedefault);
	free(thedefault);

	/* If the user cancelled, or typed <Enter> on a blank answer and
	 * nothing was searched for yet during this session, get out. */
	if (response == -1 || (response == -2 && *last_search == '\0')) {
		statusbar(_("Cancelled"));
		return;
	}

	/* If the user typed an answer, remember it. */
	if (*answer != '\0') {
		last_search = mallocstrcpy(last_search, answer);
#ifdef ENABLE_HISTORIES
		update_history(&search_history, answer);
#endif
	}

	findfile(last_search, forwards);
}

/* Search again without prompting for the last given search string,
 * either forwards or backwards. */
void research_filename(bool forwards)
{
#ifdef ENABLE_HISTORIES
	/* If nothing was searched for yet, take the last item from history. */
	if (*last_search == '\0' && searchbot->prev != NULL)
		last_search = mallocstrcpy(last_search, searchbot->prev->data);
#endif

	if (*last_search == '\0')
		statusbar(_("No current search pattern"));
	else {
		wipe_statusbar();
		findfile(last_search, forwards);
	}
}

/* Strip one element from the end of path, and return the stripped path.
 * The returned string is dynamically allocated, and should be freed. */
char *strip_last_component(const char *path)
{
	char *copy = copy_of(path);
	char *last_slash = strrchr(copy, '/');

	if (last_slash != NULL)
		*last_slash = '\0';

	return copy;
}

/* Allow the user to browse through the directories in the filesystem,
 * starting at the given path. */
char *browse(char *path)
{
	char *present_name = NULL;
		/* The name of the currently selected file, or of the directory we
		 * were in before backing up to "..". */
	size_t old_selected;
		/* The number of the selected file before the current selected file. */
	DIR *dir;
		/* The directory whose contents we are showing. */
	char *chosen = NULL;
		/* The name of the file that the user picked, or NULL if none. */

  read_directory_contents:
		/* We come here when the user refreshes or selects a new directory. */

	path = free_and_assign(path, get_full_path(path));

	if (path != NULL)
		dir = opendir(path);

	if (path == NULL || dir == NULL) {
		statusline(ALERT, _("Cannot open directory: %s"), strerror(errno));
		/* If we don't have a file list yet, there is nothing to show. */
		if (filelist == NULL) {
			lastmessage = VACUUM;
			free(present_name);
			free(path);
			napms(1200);
			return NULL;
		}
		path = mallocstrcpy(path, present_path);
		present_name = mallocstrcpy(present_name, filelist[selected]);
	}

	if (dir != NULL) {
		/* Get the file list, and set longest and width in the process. */
		read_the_list(path, dir);
		closedir(dir);
		dir = NULL;
	}

	/* If given, reselect the present_name and then discard it. */
	if (present_name != NULL) {
		browser_select_dirname(present_name);

		free(present_name);
		present_name = NULL;
	/* Otherwise, select the first file or directory in the list. */
	} else
		selected = 0;

	old_selected = (size_t)-1;

	present_path = mallocstrcpy(present_path, path);

	titlebar(path);

	while (TRUE) {
		functionptrtype func;
		int kbinput;

		lastmessage = VACUUM;

		bottombars(MBROWSER);

		/* Display (or redisplay) the file list if the list itself or
		 * the selected file has changed. */
		if (old_selected != selected || ISSET(SHOW_CURSOR))
			browser_refresh();

		old_selected = selected;

		kbinput = get_kbinput(edit, ISSET(SHOW_CURSOR));

#ifdef ENABLE_MOUSE
		if (kbinput == KEY_MOUSE) {
			int mouse_x, mouse_y;

			/* We can click on the edit window to select a filename. */
			if (get_mouseinput(&mouse_y, &mouse_x, TRUE) == 0 &&
						wmouse_trafo(edit, &mouse_y, &mouse_x, FALSE)) {
				/* longest is the width of each column.  There
				 * are two spaces between each column. */
				selected = selected - selected % (editwinrows * width) +
								(mouse_y * width) + (mouse_x / (longest + 2));

				/* If they clicked beyond the end of a row,
				 * select the last filename in that row. */
				if (mouse_x > width * (longest + 2))
					selected--;

				/* If we're beyond the list, select the last filename. */
				if (selected > list_length - 1)
					selected = list_length - 1;

				/* If we selected the same filename as last time, fake a
				 * press of the Enter key so that the file is read in. */
				if (old_selected == selected)
					kbinput = KEY_ENTER;
			}

			if (kbinput == KEY_MOUSE)
				continue;
		}
#endif /* ENABLE_MOUSE */
#ifndef NANO_TINY
		if (bracketed_paste || kbinput == BRACKETED_PASTE_MARKER) {
			beep();
			continue;
		}
#endif
		func = interpret(&kbinput);

		if (func == full_refresh) {
			full_refresh();
#ifndef NANO_TINY
			/* Simulate a window resize to force a directory reread. */
			kbinput = KEY_WINCH;
#endif
		} else if (func == do_help) {
			do_help();
#ifndef NANO_TINY
			/* The window dimensions might have changed, so act as if. */
			kbinput = KEY_WINCH;
#endif
#ifndef NANO_TINY
		} else if (func == do_toggle_void) {
			TOGGLE(NO_HELP);
			window_init();
			kbinput = KEY_WINCH;
#endif
		} else if (func == do_search_backward) {
			search_filename(BACKWARD);
		} else if (func == do_search_forward) {
			search_filename(FORWARD);
		} else if (func == do_findprevious) {
			research_filename(BACKWARD);
		} else if (func == do_findnext) {
			research_filename(FORWARD);
		} else if (func == do_left) {
			if (selected > 0)
				selected--;
		} else if (func == do_right) {
			if (selected < list_length - 1)
				selected++;
		} else if (func == to_prev_word) {
			selected -= (selected % width);
		} else if (func == to_next_word) {
			selected += width - 1 - (selected % width);
			if (selected >= list_length)
				selected = list_length - 1;
		} else if (func == do_up) {
			if (selected >= width)
				selected -= width;
		} else if (func == do_down) {
			if (selected + width <= list_length - 1)
				selected += width;
		} else if (func == to_prev_block) {
			selected = ((selected / (editwinrows * width)) *
								editwinrows * width) + selected % width;
		} else if (func == to_next_block) {
			selected = ((selected / (editwinrows * width)) *
								editwinrows * width) + selected % width +
								editwinrows * width - width;
			if (selected >= list_length)
				selected = (list_length / width) * width + selected % width;
			if (selected >= list_length)
				selected -= width;
		} else if (func == do_page_up) {
			if (selected < width)
				selected = 0;
			else if (selected < editwinrows * width)
				selected = selected % width;
			else
				selected -= editwinrows * width;
		} else if (func == do_page_down) {
			if (selected + width >= list_length - 1)
				selected = list_length - 1;
			else if (selected + editwinrows * width >= list_length)
				selected = (selected + editwinrows * width - list_length) %
								width + list_length - width;
			else
				selected += editwinrows * width;
		} else if (func == to_first_file) {
			selected = 0;
		} else if (func == to_last_file) {
			selected = list_length - 1;
		} else if (func == goto_dir) {
			/* Ask for the directory to go to. */
			if (do_prompt(MGOTODIR, "", NULL,
							/* TRANSLATORS: This is a prompt. */
							browser_refresh, _("Go To Directory")) < 0) {
				statusbar(_("Cancelled"));
				continue;
			}

			path = free_and_assign(path, real_dir_from_tilde(answer));

			/* If the given path is relative, join it with the current path. */
			if (*path != '/') {
				path = nrealloc(path, strlen(present_path) +
												strlen(answer) + 1);
				sprintf(path, "%s%s", present_path, answer);
			}

#ifdef ENABLE_OPERATINGDIR
			if (outside_of_confinement(path, FALSE)) {
				/* TRANSLATORS: This refers to the confining effect of the
				 * option --operatingdir, not of --restricted. */
				statusline(ALERT, _("Can't go outside of %s"), operating_dir);
				path = mallocstrcpy(path, present_path);
				continue;
			}
#endif
			/* Snip any trailing slashes, so the name can be compared. */
			while (strlen(path) > 1 && path[strlen(path) - 1] == '/')
				path[strlen(path) - 1] = '\0';

			/* In case the specified directory cannot be entered, select it
			 * (if it is in the current list) so it will be highlighted. */
			for (size_t j = 0; j < list_length; j++)
				if (strcmp(filelist[j], path) == 0)
					selected = j;

			/* Try opening and reading the specified directory. */
			goto read_directory_contents;
		} else if (func == do_enter) {
			struct stat st;

			/* It isn't possible to move up from the root directory. */
			if (strcmp(filelist[selected], "/..") == 0) {
				statusline(ALERT, _("Can't move up a directory"));
				continue;
			}

#ifdef ENABLE_OPERATINGDIR
			/* Note: The selected file can be outside the operating
			 * directory if it's ".." or if it's a symlink to a
			 * directory outside the operating directory. */
			if (outside_of_confinement(filelist[selected], FALSE)) {
				statusline(ALERT, _("Can't go outside of %s"), operating_dir);
				continue;
			}
#endif
			/* If for some reason the file is inaccessible, complain. */
			if (stat(filelist[selected], &st) == -1) {
				statusline(ALERT, _("Error reading %s: %s"),
								filelist[selected], strerror(errno));
				continue;
			}

			/* If it isn't a directory, a file was selected -- we're done. */
			if (!S_ISDIR(st.st_mode)) {
				chosen = copy_of(filelist[selected]);
				break;
			}

			/* If we are moving up one level, remember where we came from, so
			 * this directory can be highlighted and easily reentered. */
			if (strcmp(tail(filelist[selected]), "..") == 0)
				present_name = strip_last_component(filelist[selected]);

			/* Try opening and reading the selected directory. */
			path = mallocstrcpy(path, filelist[selected]);
			goto read_directory_contents;
#ifdef ENABLE_NANORC
		} else if (func == (functionptrtype)implant) {
			implant(first_sc_for(MBROWSER, func)->expansion);
#endif
#ifndef NANO_TINY
		} else if (kbinput == KEY_WINCH) {
			;  /* Nothing to do. */
#endif
		} else if (func == do_exit) {
			break;
		} else
			unbound_key(kbinput);

#ifndef NANO_TINY
		/* If the window resized, refresh the file list. */
		if (kbinput == KEY_WINCH) {
			/* Remember the selected file, to be able to reselect it. */
			present_name = copy_of(filelist[selected]);
			/* Reread the contents of the current directory. */
			goto read_directory_contents;
		}
#endif
	}

	titlebar(NULL);
	edit_refresh();

	free(path);

	free_chararray(filelist, list_length);
	filelist = NULL;
	list_length = 0;

	return chosen;
}

/* Prepare to start browsing.  If the given path has a directory part,
 * start browsing in that directory, otherwise in the current directory. */
char *browse_in(const char *inpath)
{
	char *path = real_dir_from_tilde(inpath);
	struct stat fileinfo;

	/* If path is not a directory, try to strip a filename from it; if then
	 * still not a directory, use the current working directory instead. */
	if (stat(path, &fileinfo) == -1 || !S_ISDIR(fileinfo.st_mode)) {
		path = free_and_assign(path, strip_last_component(path));

		if (stat(path, &fileinfo) == -1 || !S_ISDIR(fileinfo.st_mode)) {
			char *currentdir = nmalloc(PATH_MAX + 1);

			path = free_and_assign(path, getcwd(currentdir, PATH_MAX + 1));

			if (path == NULL) {
				statusline(MILD, _("The working directory has disappeared"));
				free(currentdir);
				beep();
				napms(1200);
				return NULL;
			}
		}
	}

#ifdef ENABLE_OPERATINGDIR
	/* If the resulting path isn't in the operating directory, use
	 * the operating directory instead. */
	if (outside_of_confinement(path, FALSE))
		path = mallocstrcpy(path, operating_dir);
#endif

	return browse(path);
}

#endif /* ENABLE_BROWSER */
