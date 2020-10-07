/**************************************************************************
 *   files.c  --  This file is part of GNU nano.                          *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2020 Free Software Foundation, Inc.    *
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
#include <libgen.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define RW_FOR_ALL  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/* Add an item to the circular list of openfile structs. */
void make_new_buffer(void)
{
	openfilestruct *newnode = nmalloc(sizeof(openfilestruct));

#ifdef ENABLE_MULTIBUFFER
	if (openfile == NULL) {
		/* Make the first open file the only element in the list. */
		newnode->prev = newnode;
		newnode->next = newnode;

		startfile = newnode;
	} else {
		/* Add the new open file after the current one in the list. */
		newnode->prev = openfile;
		newnode->next = openfile->next;
		openfile->next->prev = newnode;
		openfile->next = newnode;

		/* There is more than one file open: show "Close" in help lines. */
		exitfunc->desc = close_tag;
		more_than_one = !inhelp || more_than_one;
	}
#endif
	/* Make the new buffer the current one, and start initializing it. */
	openfile = newnode;

	openfile->filename = copy_of("");

	openfile->filetop = make_new_node(NULL);
	openfile->filetop->data = copy_of("");
	openfile->filebot = openfile->filetop;

	openfile->current = openfile->filetop;
	openfile->current_x = 0;
	openfile->placewewant = 0;
	openfile->current_y = 0;

	openfile->edittop = openfile->filetop;
	openfile->firstcolumn = 0;

	openfile->totsize = 0;
	openfile->modified = FALSE;
#ifdef ENABLE_WRAPPING
	openfile->spillage_line = NULL;
#endif
#ifndef NANO_TINY
	openfile->mark = NULL;

	openfile->fmt = NIX_FILE;

	openfile->undotop = NULL;
	openfile->current_undo = NULL;
	openfile->last_saved = NULL;
	openfile->last_action = OTHER;

	openfile->statinfo = NULL;
	openfile->lock_filename = NULL;
#endif
#ifdef ENABLE_COLOR
	openfile->syntax = NULL;
#endif
}

#ifndef NANO_TINY
/* Delete the lockfile.  Return TRUE on success, and FALSE otherwise. */
bool delete_lockfile(const char *lockfilename)
{
	if (unlink(lockfilename) < 0 && errno != ENOENT) {
		statusline(MILD, _("Error deleting lock file %s: %s"),
							lockfilename, strerror(errno));
		return FALSE;
	} else
		return TRUE;
}

#define LOCKSIZE  1024
#define SKIPTHISFILE  (char *)-1

const char *locking_prefix = ".";
const char *locking_suffix = ".swp";

/* Write a lock file, under the given lockfilename.  This always annihilates an
 * existing version of that file.  Return TRUE on success; FALSE otherwise. */
bool write_lockfile(const char *lockfilename, const char *filename, bool modified)
{
#ifdef HAVE_PWD_H
	pid_t mypid = getpid();
	uid_t myuid = geteuid();
	struct passwd *mypwuid = getpwuid(myuid);
	char myhostname[32];
	int fd;
	FILE *filestream = NULL;
	char *lockdata;
	size_t wroteamt;

	if (mypwuid == NULL) {
		/* TRANSLATORS: Keep the next seven messages at most 76 characters. */
		statusline(MILD, _("Couldn't determine my identity for lock file"));
		return FALSE;
	}

	if (gethostname(myhostname, 31) < 0 && errno != ENAMETOOLONG) {
		statusline(MILD, _("Couldn't determine hostname: %s"), strerror(errno));
		return FALSE;
	} else
		myhostname[31] = '\0';

	/* First make sure to remove an existing lock file. */
	if (!delete_lockfile(lockfilename))
		return FALSE;

	/* Create the lock file -- do not accept an existing one. */
	fd = open(lockfilename, O_WRONLY|O_CREAT|O_EXCL, RW_FOR_ALL);

	if (fd > 0)
		filestream = fdopen(fd, "wb");

	if (filestream == NULL) {
		statusline(MILD, _("Error writing lock file %s: %s"),
							lockfilename, strerror(errno));
		if (fd > 0)
			close(fd);
		return FALSE;
	}

	lockdata = charalloc(LOCKSIZE);
	memset(lockdata, 0, LOCKSIZE);

	/* This is the lock data we will store (other bytes remain 0x00):
	 *
	 *   bytes 0-1     - 0x62 0x30
	 *   bytes 2-11    - name of program that created the lock
	 *   bytes 24-27   - PID (little endian) of creator process
	 *   bytes 28-43   - username of the user who created the lock
	 *   bytes 68-99   - hostname of machine from where the lock was created
	 *   bytes 108-876 - filename that the lock is for
	 *   byte 1007     - 0x55 if file is modified
	 *
	 * Nano does not write the page size (bytes 12-15), nor the modification
	 * time (bytes 16-19), nor the inode of the relevant file (bytes 20-23).
	 * Nano also does not use all available space for user name (40 bytes),
	 * host name (40 bytes), and file name (890 bytes).  Nor does nano write
	 * some byte-order-checking numbers (bytes 1008-1022). */
	lockdata[0] = 0x62;
	lockdata[1] = 0x30;
	snprintf(&lockdata[2], 10, "nano %s", VERSION);
	lockdata[24] = mypid % 256;
	lockdata[25] = (mypid / 256) % 256;
	lockdata[26] = (mypid / (256 * 256)) % 256;
	lockdata[27] = mypid / (256 * 256 * 256);
	strncpy(&lockdata[28], mypwuid->pw_name, 16);
	strncpy(&lockdata[68], myhostname, 32);
	strncpy(&lockdata[108], filename, 768);
	lockdata[1007] = (modified) ? 0x55 : 0x00;

	wroteamt = fwrite(lockdata, sizeof(char), LOCKSIZE, filestream);

	free(lockdata);

	if (fclose(filestream) == EOF || wroteamt < LOCKSIZE) {
		statusline(MILD, _("Error writing lock file %s: %s"),
							lockfilename, strerror(errno));
		return FALSE;
	}
#endif
	return TRUE;
}

/* First check if a lock file already exists.  If so, and ask_the_user is TRUE,
 * then ask whether to open the corresponding file anyway.  Return SKIPTHISFILE
 * when the user answers "No", return the name of the lock file on success, and
 * return NULL on failure. */
char *do_lockfile(const char *filename, bool ask_the_user)
{
	char *namecopy = copy_of(filename);
	char *secondcopy = copy_of(filename);
	size_t locknamesize = strlen(filename) + strlen(locking_prefix) +
							strlen(locking_suffix) + 3;
	char *lockfilename = charalloc(locknamesize);
	struct stat fileinfo;

	snprintf(lockfilename, locknamesize, "%s/%s%s%s", dirname(namecopy),
				locking_prefix, basename(secondcopy), locking_suffix);
	free(secondcopy);
	free(namecopy);

	if (!ask_the_user && stat(lockfilename, &fileinfo) != -1)
		warn_and_briefly_pause(_("Someone else is also editing this file"));
	else if (stat(lockfilename, &fileinfo) != -1) {
		char *lockbuf, *question, *pidstring, *postedname, *promptstr;
		static char lockprog[11], lockuser[17];
		int lockfd, lockpid, room, choice;
		ssize_t readamt;

		if ((lockfd = open(lockfilename, O_RDONLY)) < 0) {
			statusline(ALERT, _("Error opening lock file %s: %s"),
								lockfilename, strerror(errno));
			free(lockfilename);
			return NULL;
		}

		lockbuf = charalloc(LOCKSIZE);

		readamt = read(lockfd, lockbuf, LOCKSIZE);

		close(lockfd);

		/* If not enough data has been read to show the needed things,
		 * or the two magic bytes are not there, skip the lock file. */
		if (readamt < 68 || lockbuf[0] != 0x62 || lockbuf[1] != 0x30) {
			statusline(ALERT, _("Bad lock file is ignored: %s"), lockfilename);
			free(lockfilename);
			free(lockbuf);
			return NULL;
		}

		strncpy(lockprog, &lockbuf[2], 10);
		lockprog[10] = '\0';
		lockpid = (((unsigned char)lockbuf[27] * 256 + (unsigned char)lockbuf[26]) * 256 +
						(unsigned char)lockbuf[25]) * 256 + (unsigned char)lockbuf[24];
		strncpy(lockuser, &lockbuf[28], 16);
		lockuser[16] = '\0';
		free(lockbuf);

		pidstring = charalloc(11);
		sprintf (pidstring, "%u", (unsigned int)lockpid);

		/* Display newlines in filenames as ^J. */
		as_an_at = FALSE;

		/* TRANSLATORS: The second %s is the name of the user, the third that of the editor. */
		question = _("File %s is being edited by %s (with %s, PID %s); open anyway?");
		room = COLS - breadth(question) + 7 - breadth(lockuser) -
								breadth(lockprog) - breadth(pidstring);
		if (room < 4)
			postedname = copy_of("_");
		else if (room < breadth(filename)) {
			char *fragment = display_string(filename,
								breadth(filename) - room + 3, room, FALSE, FALSE);
			postedname = charalloc(strlen(fragment) + 4);
			strcpy(postedname, "...");
			strcat(postedname, fragment);
			free(fragment);
		} else
			postedname = display_string(filename, 0, room, FALSE, FALSE);

		/* Allow extra space for username (14), program name (8), PID (8),
		 * and terminating \0 (1), minus the %s (2) for the file name. */
		promptstr = charalloc(strlen(question) + 29 + strlen(postedname));
		sprintf(promptstr, question, postedname, lockuser, lockprog, pidstring);
		free(postedname);
		free(pidstring);

		choice = do_yesno_prompt(FALSE, promptstr);
		free(promptstr);

		/* When the user cancelled while we're still starting up, quit. */
		if (choice < 0 && !we_are_running)
			finish();

		if (choice < 1) {
			free(lockfilename);
			wipe_statusbar();
			return SKIPTHISFILE;
		}
	}

	if (write_lockfile(lockfilename, filename, FALSE))
		return lockfilename;

	free(lockfilename);
	return NULL;
}

/* Perform a stat call on the given filename, allocating a stat struct
 * if necessary.  On success, *pstat points to the stat's result.  On
 * failure, *pstat is freed and made NULL. */
void stat_with_alloc(const char *filename, struct stat **pstat)
{
	if (*pstat == NULL)
		*pstat = (struct stat *)nmalloc(sizeof(struct stat));

	if (stat(filename, *pstat) != 0) {
		free(*pstat);
		*pstat = NULL;
	}
}
#endif /* !NANO_TINY */

/* Verify that the containing directory of the given filename exists. */
bool has_valid_path(const char *filename)
{
	char *namecopy = copy_of(filename);
	char *parentdir = dirname(namecopy);
	struct stat parentinfo;
	bool validity = FALSE;

	if (stat(parentdir, &parentinfo) == -1) {
		if (errno == ENOENT)
			/* TRANSLATORS: Keep the next ten messages at most 76 characters. */
			statusline(ALERT, _("Directory '%s' does not exist"), parentdir);
		else
			statusline(ALERT, _("Path '%s': %s"), parentdir, strerror(errno));
	} else if (!S_ISDIR(parentinfo.st_mode))
		statusline(ALERT, _("Path '%s' is not a directory"), parentdir);
	else if (access(parentdir, X_OK) == -1)
		statusline(ALERT, _("Path '%s' is not accessible"), parentdir);
#ifndef NANO_TINY
	else if (ISSET(LOCKING) && !ISSET(VIEW_MODE) && access(parentdir, W_OK) < 0)
		statusline(MILD, _("Directory '%s' is not writable"), parentdir);
#endif
	else
		validity = TRUE;

	free(namecopy);

	return validity;
}

/* This does one of three things.  If the filename is "", it just creates
 * a new empty buffer.  When the filename is not empty, it reads that file
 * into a new buffer when requested, otherwise into the existing buffer. */
bool open_buffer(const char *filename, bool new_one)
{
	char *realname;
		/* The filename after tilde expansion. */
#ifndef NANO_TINY
	char *thelocksname = NULL;
#endif
	struct stat fileinfo;
	int descriptor = 0;
		/* Code 0 means new file, -1 means failure, and else it's the fd. */
	FILE *f;

	/* Display newlines in filenames as ^J. */
	as_an_at = FALSE;

#ifdef ENABLE_OPERATINGDIR
	if (outside_of_confinement(filename, FALSE)) {
		statusline(ALERT, _("Can't read file from outside of %s"), operating_dir);
		return FALSE;
	}
#endif

	realname = real_dir_from_tilde(filename);

	/* Don't try to open directories, character files, or block files. */
	if (*filename != '\0' && stat(realname, &fileinfo) == 0) {
		if (S_ISDIR(fileinfo.st_mode)) {
			statusline(ALERT, _("\"%s\" is a directory"), realname);
			free(realname);
			return FALSE;
		}
		if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode)) {
			statusline(ALERT, _("\"%s\" is a device file"), realname);
			free(realname);
			return FALSE;
		}
#ifdef NANO_TINY
		if (S_ISFIFO(fileinfo.st_mode)) {
			statusline(ALERT, _("\"%s\" is a FIFO"), realname);
			free(realname);
			return FALSE;
		}
#else
		if (new_one && !(fileinfo.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH)) &&
						geteuid() == ROOT_UID)
			statusline(ALERT, _("%s is meant to be read-only"), realname);
#endif
	}

	/* When loading into a new buffer, first check the file's path is valid,
	 * and then (if requested and possible) create a lock file for it. */
	if (new_one && has_valid_path(realname)) {
#ifndef NANO_TINY
		if (ISSET(LOCKING) && !ISSET(VIEW_MODE) && filename[0] != '\0') {
			thelocksname = do_lockfile(realname, TRUE);

			/* When not overriding an existing lock, don't open a buffer. */
			if (thelocksname == SKIPTHISFILE) {
				free(realname);
				return FALSE;
			}
		}
#endif
	}

	if (new_one)
		make_new_buffer();

	/* If we have a filename and are not in NOREAD mode, open the file. */
	if (filename[0] != '\0' && !ISSET(NOREAD_MODE))
		descriptor = open_file(realname, new_one, &f);

	/* If we've successfully opened an existing file, read it in. */
	if (descriptor > 0) {
		install_handler_for_Ctrl_C();

		read_file(f, descriptor, realname, !new_one);

		restore_handler_for_Ctrl_C();

#ifndef NANO_TINY
		if (openfile->statinfo == NULL)
			stat_with_alloc(realname, &openfile->statinfo);
#endif
	}

	/* When we've loaded a file into a new buffer, set the filename
	 * and put the cursor at the start of the buffer. */
	if (descriptor >= 0 && new_one) {
		openfile->filename = mallocstrcpy(openfile->filename, realname);
#ifndef NANO_TINY
		openfile->lock_filename = thelocksname;
#endif
		openfile->current = openfile->filetop;
		openfile->current_x = 0;
		openfile->placewewant = 0;
	}

#ifdef ENABLE_COLOR
	/* If a new buffer was opened, check whether a syntax can be applied. */
	if (new_one)
		find_and_prime_applicable_syntax();
#endif
	free(realname);
	return TRUE;
}

/* Mark the current file as modified if it isn't already, and
 * then update the title bar to display the file's new status. */
void set_modified(void)
{
	if (openfile->modified)
		return;

	openfile->modified = TRUE;
	titlebar(NULL);

#ifndef NANO_TINY
	if (openfile->lock_filename != NULL)
		write_lockfile(openfile->lock_filename, openfile->filename, TRUE);
#endif
}

/* Update the title bar and the multiline cache to match the current buffer. */
void prepare_for_display(void)
{
	/* Update the title bar, since the filename may have changed. */
	if (!inhelp)
		titlebar(NULL);

#ifdef ENABLE_COLOR
	/* Precalculate the data for any multiline coloring regexes. */
	precalc_multicolorinfo();
	have_palette = FALSE;
#endif
	refresh_needed = TRUE;
}

#ifdef ENABLE_MULTIBUFFER
/* Show name of current buffer and its number of lines on the status bar. */
void mention_name_and_linecount(void)
{
	size_t count = openfile->filebot->lineno -
						(openfile->filebot->data[0] == '\0' ? 1 : 0);
#ifndef NANO_TINY
	if (openfile->fmt != NIX_FILE)
		/* TRANSLATORS: First %s is file name, second %s is file format. */
		statusline(HUSH, P_("%s -- %zu line (%s)", "%s -- %zu lines (%s)", count),
						openfile->filename[0] == '\0' ?
						_("New Buffer") : tail(openfile->filename), count,
						openfile->fmt == DOS_FILE ? _("DOS") : _("Mac"));
	else
#endif
		statusline(HUSH, P_("%s -- %zu line", "%s -- %zu lines", count),
						openfile->filename[0] == '\0' ?
						_("New Buffer") : tail(openfile->filename), count);
}

/* Update title bar and such after switching to another buffer.*/
void redecorate_after_switch(void)
{
	/* If only one file buffer is open, there is nothing to update. */
	if (openfile == openfile->next) {
		statusbar(_("No more open file buffers"));
		return;
	}

#ifndef NANO_TINY
	/* While in a different buffer, the effective width of the screen may
	 * have changed, so make sure that the softwrapped chunks per line and
	 * the starting column for the first row get corresponding values. */
	compute_the_extra_rows_per_line_from(openfile->filetop);
	ensure_firstcolumn_is_aligned();
#endif

	/* Update title bar and multiline info to match the current buffer. */
	prepare_for_display();

	/* Ensure that the main loop will redraw the help lines. */
	currmenu = MMOST;

	/* Prevent a possible Shift selection from getting cancelled. */
	shift_held = TRUE;

	/* Indicate on the status bar where we switched to. */
	mention_name_and_linecount();
}

/* Switch to the previous entry in the list of open files. */
void switch_to_prev_buffer(void)
{
	openfile = openfile->prev;
	redecorate_after_switch();
}

/* Switch to the next entry in the list of open files. */
void switch_to_next_buffer(void)
{
	openfile = openfile->next;
	redecorate_after_switch();
}

/* Remove the current buffer from the circular list of buffers. */
void close_buffer(void)
{
	openfilestruct *orphan = openfile;

	if (orphan == startfile)
		startfile = startfile->next;

	orphan->prev->next = orphan->next;
	orphan->next->prev = orphan->prev;

	free(orphan->filename);
	free_lines(orphan->filetop);
#ifndef NANO_TINY
	free(orphan->statinfo);
	free(orphan->lock_filename);
	/* Free the undo stack. */
	discard_until(NULL);
#endif

	openfile = orphan->prev;
	free(orphan);

	/* When just one buffer remains open, show "Exit" in the help lines. */
	if (openfile == openfile->next)
		exitfunc->desc = exit_tag;
}
#endif /* ENABLE_MULTIBUFFER */

/* Encode any NUL bytes in the given line of text (of the given length),
 * and return a dynamically allocated copy of the resultant string. */
char *encode_data(char *text, size_t length)
{
	recode_NUL_to_LF(text, length);
	text[length] = '\0';

	return copy_of(text);
}

/* The number of bytes by which we expand the line buffer while reading. */
#define LUMPSIZE  120

/* Read the given open file f into the current buffer.  filename should be
 * set to the name of the file.  undoable means that undo records should be
 * created and that the file does not need to be checked for writability. */
void read_file(FILE *f, int fd, const char *filename, bool undoable)
{
	ssize_t was_lineno = openfile->current->lineno;
		/* The line number where we start the insertion. */
	size_t was_leftedge = 0;
		/* The leftedge where we start the insertion. */
	size_t num_lines = 0;
		/* The number of lines in the file. */
	size_t len = 0;
		/* The length of the current line of the file. */
	size_t bufsize = LUMPSIZE;
		/* The size of the line buffer; increased as needed. */
	char *buf = charalloc(bufsize);
		/* The buffer in which we assemble each line of the file. */
	linestruct *topline;
		/* The top of the new buffer where we store the read file. */
	linestruct *bottomline;
		/* The bottom of the new buffer. */
	int onevalue;
		/* The current value we read from the file, either a byte or EOF. */
	int errornumber;
		/* The error code, in case an error occurred during reading. */
	bool writable = TRUE;
		/* Whether the file is writable (in case we care). */
#ifndef NANO_TINY
	int format = 0;
		/* 0 = Unix, 1 = DOS, 2 = Mac. */
#endif

#ifndef NANO_TINY
	if (undoable)
		add_undo(INSERT, NULL);

	if (ISSET(SOFTWRAP))
		was_leftedge = leftedge_for(xplustabs(), openfile->current);
#endif

	/* Create an empty buffer. */
	topline = make_new_node(NULL);
	bottomline = topline;

#ifndef NANO_TINY
	block_sigwinch(TRUE);
#endif

	/* Lock the file before starting to read it, to avoid the overhead
	 * of locking it for each single byte that we read from it. */
	flockfile(f);

	control_C_was_pressed = FALSE;

	/* Read in the entire file, byte by byte, line by line. */
	while ((onevalue = getc_unlocked(f)) != EOF) {
		char input = (char)onevalue;

		if (control_C_was_pressed)
			break;

		/* When the byte before the current one is a CR and we're doing
		 * format conversion, then strip this CR when it's before a LF
		 * OR when the file is in Mac format.  Also, when still on the
		 * first line, set the format to either DOS (1) or Mac (2). */
		if (input == '\n') {
#ifndef NANO_TINY
			if (len > 0 && buf[len - 1] == '\r' && !ISSET(NO_CONVERT)) {
				if (num_lines == 0)
					format = 1;
				len--;
			}
		} else if ((num_lines == 0 || format == 2) &&
					len > 0 && buf[len - 1] == '\r' && !ISSET(NO_CONVERT)) {
			format = 2;
			len--;
#endif
		} else {
			/* Store the byte. */
			buf[len] = input;

			/* Keep track of the total length of the line.  It might have
			 * NUL bytes in it, so we can't just use strlen() later. */
			len++;

			/* When needed, increase the line-buffer size.  Don't bother
			 * decreasing it -- it gets freed when reading is finished. */
			if (len == bufsize) {
				bufsize += LUMPSIZE;
				buf = charealloc(buf, bufsize);
			}

			continue;
		}

		/* Store the data and make a new line. */
		bottomline->data = encode_data(buf, len);
		bottomline->next = make_new_node(bottomline);
		bottomline = bottomline->next;
		num_lines++;

		/* Reset the length in preparation for the next line. */
		len = 0;

#ifndef NANO_TINY
		/* If it was a Mac line, then store the byte after the \r
		 * as the first byte of the next line. */
		if (input != '\n')
			buf[len++] = input;
#endif
	}

	errornumber = errno;

	/* We are done with the file, unlock it. */
	funlockfile(f);

#ifndef NANO_TINY
	block_sigwinch(FALSE);
#endif

	/* When reading from stdin, restore the terminal and reenter curses mode. */
	if (isendwin()) {
		if (!isatty(STANDARD_INPUT))
			reconnect_and_store_state();
		terminal_init();
		doupdate();
	}

	/* If there was a real error during the reading, let the user know. */
	if (ferror(f) && errornumber != EINTR && errornumber != 0)
		statusline(ALERT, strerror(errornumber));

	if (control_C_was_pressed)
		statusline(ALERT, _("Interrupted"));

	fclose(f);

	if (fd > 0 && !undoable && !ISSET(VIEW_MODE))
		writable = (access(filename, W_OK) == 0);

	/* If the file ended with newline, or it was entirely empty, make the
	 * last line blank.  Otherwise, put the last read data in. */
	if (len == 0)
		bottomline->data = copy_of("");
	else {
#ifndef NANO_TINY
		bool mac_line_needs_newline = FALSE;

		/* If the final character is a CR and file conversion isn't disabled,
		 * strip this CR and indicate that an extra blank line is needed. */
		if (buf[len - 1] == '\r' && !ISSET(NO_CONVERT)) {
			if (num_lines == 0)
				format = 2;
			buf[--len] = '\0';
			mac_line_needs_newline = TRUE;
		}
#endif
		/* Store the data of the final line. */
		bottomline->data = encode_data(buf, len);
		num_lines++;

#ifndef NANO_TINY
		if (mac_line_needs_newline) {
			bottomline->next = make_new_node(bottomline);
			bottomline = bottomline->next;
			bottomline->data = copy_of("");
		}
#endif
	}

	free(buf);

	/* Insert the just read buffer into the current one. */
	ingraft_buffer(topline);

	/* Set the desired x position at the end of what was inserted. */
	openfile->placewewant = xplustabs();

	if (!writable)
		statusline(ALERT, _("File '%s' is unwritable"), filename);
#ifndef NANO_TINY
	else if (format == 2) {
		/* TRANSLATORS: Keep the next three messages at most 78 characters. */
		openfile->fmt = MAC_FILE;
		statusline(HUSH, P_("Read %zu line (Converted from Mac format)",
						"Read %zu lines (Converted from Mac format)",
						num_lines), num_lines);
	} else if (format == 1) {
		openfile->fmt = DOS_FILE;
		statusline(HUSH, P_("Read %zu line (Converted from DOS format)",
						"Read %zu lines (Converted from DOS format)",
						num_lines), num_lines);
	}
#endif
	else
		statusline(HUSH, P_("Read %zu line", "Read %zu lines",
						num_lines), num_lines);

	/* If we inserted less than a screenful, don't center the cursor. */
	if (undoable && less_than_a_screenful(was_lineno, was_leftedge))
		focusing = FALSE;

#ifndef NANO_TINY
	if (undoable)
		update_undo(INSERT);

	if (ISSET(MAKE_IT_UNIX))
		openfile->fmt = NIX_FILE;
#endif
}

/* Open the file with the given name.  If the file does not exist, display
 * "New File" if new_one is TRUE, and say "File not found" otherwise.
 * Return 0 if we say "New File", -1 upon failure, and the obtained file
 * descriptor otherwise.  The opened filestream is returned in *f. */
int open_file(const char *filename, bool new_one, FILE **f)
{
	char *full_filename = get_full_path(filename);
	struct stat fileinfo;
	int fd;

	/* If the absolute path is unusable (due to some component's permissions),
	 * try the given path instead (as it is probably relative). */
	if (full_filename == NULL || stat(full_filename, &fileinfo) == -1)
		full_filename = mallocstrcpy(full_filename, filename);

	if (stat(full_filename, &fileinfo) == -1) {
		free(full_filename);

		if (new_one) {
			statusbar(_("New File"));
			return 0;
		} else {
			statusline(ALERT, _("File \"%s\" not found"), filename);
			return -1;
		}
	}

#ifndef NANO_TINY
	if (S_ISFIFO(fileinfo.st_mode))
		statusbar(_("Reading from FIFO..."));

	block_sigwinch(TRUE);
	install_handler_for_Ctrl_C();
#endif

	/* Try opening the file. */
	fd = open(full_filename, O_RDONLY);

#ifndef NANO_TINY
	restore_handler_for_Ctrl_C();
	block_sigwinch(FALSE);
#endif

	if (fd == -1) {
		if (errno == EINTR || errno == 0)
			statusline(ALERT, _("Interrupted"));
		else
			statusline(ALERT, _("Error reading %s: %s"), filename, strerror(errno));
	} else {
		/* The file is A-OK.  Associate a stream with it. */
		*f = fdopen(fd, "rb");

		if (*f == NULL) {
			statusline(ALERT, _("Error reading %s: %s"), filename, strerror(errno));
			close(fd);
			fd = -1;
		} else
			statusbar(_("Reading..."));
	}

	free(full_filename);

	return fd;
}

/* This function will return the name of the first available extension
 * of a filename (starting with [name][suffix], then [name][suffix].1,
 * etc.).  Memory is allocated for the return value.  If no writable
 * extension exists, we return "". */
char *get_next_filename(const char *name, const char *suffix)
{
	size_t wholenamelen= strlen(name) + strlen(suffix);
	unsigned long i = 0;
	char *buf;

	/* Reserve space for: the name plus the suffix plus a dot plus
	 * possibly five digits plus a null byte. */
	buf = charalloc(wholenamelen + 7);
	sprintf(buf, "%s%s", name, suffix);

	while (TRUE) {
		struct stat fs;

		if (stat(buf, &fs) == -1)
			return buf;

		/* Limit the number of backup files to a hundred thousand. */
		if (++i == 100000)
			break;

		sprintf(buf + wholenamelen, ".%lu", i);
	}

	/* There is no possible save file: blank out the filename. */
	*buf = '\0';

	return buf;
}

#ifndef NANO_TINY
static pid_t pid_of_command = -1;
		/* The PID of a forked process -- needed when wanting to abort it. */

/* Send an unconditional kill signal to the running external command. */
RETSIGTYPE cancel_the_command(int signal)
{
	kill(pid_of_command, SIGKILL);
}

/* Send the text that starts at the given line to file descriptor fd. */
void send_data(const linestruct *line, int fd)
{
	FILE *tube = fdopen(fd, "w");

	if (tube == NULL)
		exit(4);

	/* Send each line, except a final empty line. */
	while (line != NULL && (line->next != NULL || line->data[0] != '\0')) {
		fprintf(tube, "%s%s", line->data, line->next == NULL ? "" : "\n");
		line = line->next;
	}

	fclose(tube);
}

/* Execute the given command in a shell.  Return TRUE on success. */
bool execute_command(const char *command)
{
	int from_fd[2], to_fd[2];
		/* The pipes through which text will be written and read. */
	const bool should_pipe = (command[0] == '|');
	FILE *stream;
	struct sigaction oldaction, newaction = {{0}};
		/* Original and temporary handlers for SIGINT. */

	/* Create a pipe to read the command's output from, and, if needed,
	 * a pipe to feed the command's input through. */
	if (pipe(from_fd) == -1 || (should_pipe && pipe(to_fd) == -1)) {
		statusline(ALERT, _("Could not create pipe: %s"), strerror(errno));
		return FALSE;
	}

	/* Fork a child process to run the command in. */
	if ((pid_of_command = fork()) == 0) {
		const char *theshell = getenv("SHELL");

		if (theshell == NULL)
			theshell = (char *)"/bin/sh";

		/* Child: close the unused read end of the output pipe. */
		close(from_fd[0]);

		/* Connect the write end of the output pipe to the process' output streams. */
		dup2(from_fd[1], fileno(stdout));
		dup2(from_fd[1], fileno(stderr));

		/* If the parent sends text, connect the read end of the
		 * feeding pipe to the child's input stream. */
		if (should_pipe) {
			dup2(to_fd[0], fileno(stdin));
			close(to_fd[1]);
		}

		/* Run the given command inside the preferred shell. */
		execl(theshell, tail(theshell), "-c", should_pipe ? &command[1] : command, NULL);

		/* If the exec call returns, there was an error. */
		exit(1);
	}

	/* Parent: close the unused write end of the pipe. */
	close(from_fd[1]);

	if (pid_of_command == -1) {
		statusline(ALERT, _("Could not fork: %s"), strerror(errno));
		close(from_fd[0]);
		return FALSE;
	}

	statusbar(_("Executing..."));

	/* If the command starts with "|", pipe buffer or region to the command. */
	if (should_pipe) {
		linestruct *was_cutbuffer = cutbuffer;
		bool whole_buffer = FALSE;

		cutbuffer = NULL;

#ifdef ENABLE_MULTIBUFFER
		if (ISSET(MULTIBUFFER)) {
			openfile = openfile->prev;
			if (openfile->mark)
				copy_marked_region();
			else
				whole_buffer = TRUE;
		} else
#endif
		{
			/* TRANSLATORS: This one goes with Undid/Redid messages. */
			add_undo(COUPLE_BEGIN, N_("filtering"));
			if (openfile->mark == NULL) {
				openfile->current = openfile->filetop;
				openfile->current_x = 0;
			}
			add_undo(CUT, NULL);
			do_snip(openfile->mark != NULL, openfile->mark == NULL, FALSE);
			openfile->filetop->has_anchor = FALSE;
			update_undo(CUT);
		}

		/* Create a separate process for piping the data to the command. */
		if (fork() == 0) {
			send_data(whole_buffer ? openfile->filetop : cutbuffer, to_fd[1]);
			exit(0);
		}

		close(to_fd[0]);
		close(to_fd[1]);

#ifdef ENABLE_MULTIBUFFER
		if (ISSET(MULTIBUFFER))
			openfile = openfile->next;
#endif
		free_lines(cutbuffer);
		cutbuffer = was_cutbuffer;
	}

	/* Re-enable interpretation of the special control keys so that we get
	 * SIGINT when Ctrl-C is pressed. */
	enable_kb_interrupt();

	/* Set up a signal handler so that ^C will terminate the forked process. */
	newaction.sa_handler = cancel_the_command;
	newaction.sa_flags = 0;
	sigaction(SIGINT, &newaction, &oldaction);

	stream = fdopen(from_fd[0], "rb");
	if (stream == NULL)
		statusline(ALERT, _("Failed to open pipe: %s"), strerror(errno));
	else
		read_file(stream, 0, "pipe", TRUE);

	if (should_pipe && !ISSET(MULTIBUFFER))
		add_undo(COUPLE_END, N_("filtering"));

	/* Wait for the external command (and possibly data sender) to terminate. */
	wait(NULL);
	if (should_pipe)
		wait(NULL);

	/* Restore the original handler for SIGINT. */
	sigaction(SIGINT, &oldaction, NULL);

	/* Restore the terminal to its desired state, and disable
	 * interpretation of the special control keys again. */
	terminal_init();

	return TRUE;
}
#endif /* NANO_TINY */

/* Insert a file into the current buffer, or into a new buffer when
 * the MULTIBUFFER flag is set. */
void do_insertfile(bool execute)
{
	int response;
	const char *msg;
	char *given = copy_of("");
		/* The last answer the user typed at the status-bar prompt. */
#ifdef ENABLE_MULTIBUFFER
	bool was_multibuffer = ISSET(MULTIBUFFER);
#endif
#ifndef NANO_TINY
	format_type was_fmt = openfile->fmt;
#endif

	/* Display newlines in filenames as ^J. */
	as_an_at = FALSE;

	/* Reset the flag that is set by the Spell Checker and Linter and such. */
	ran_a_tool = FALSE;

	while (TRUE) {
#ifndef NANO_TINY
		if (execute) {
#ifdef ENABLE_MULTIBUFFER
			if (ISSET(MULTIBUFFER))
				/* TRANSLATORS: The next six messages are prompts. */
				msg = _("Command to execute in new buffer");
			else
#endif
				msg = _("Command to execute");
		} else
#endif
		{
#ifdef ENABLE_MULTIBUFFER
			if (ISSET(MULTIBUFFER))
#ifndef NANO_TINY
				if ISSET(NO_CONVERT)
					msg = _("File to read unconverted into new buffer [from %s]");
				else
#endif
					msg = _("File to read into new buffer [from %s]");
			else
#endif
#ifndef NANO_TINY
				if ISSET(NO_CONVERT)
					msg = _("File to insert unconverted [from %s]");
				else
#endif
					msg = _("File to insert [from %s]");
		}

		present_path = mallocstrcpy(present_path, "./");

		response = do_prompt(
#ifndef NANO_TINY
							execute ? MEXECUTE :
#endif
							MINSERTFILE, given,
#ifndef NANO_TINY
							execute ? &execute_history :
#endif
							NULL, edit_refresh, msg,
#ifdef ENABLE_OPERATINGDIR
							operating_dir != NULL ? operating_dir :
#endif
							"./");

		/* If we're in multibuffer mode and the filename or command is
		 * blank, open a new buffer instead of canceling. */
		if (response == -1 || (response == -2 && !ISSET(MULTIBUFFER))) {
			statusbar(_("Cancelled"));
			break;
		} else {
			ssize_t was_current_lineno = openfile->current->lineno;
			size_t was_current_x = openfile->current_x;
#if !defined(NANO_TINY) || defined(ENABLE_BROWSER) || defined(ENABLE_MULTIBUFFER)
			functionptrtype func = func_from_key(&response);
#endif
			given = mallocstrcpy(given, answer);

			if (ran_a_tool)
				break;

#ifdef ENABLE_MULTIBUFFER
			if (func == flip_newbuffer) {
				/* Allow toggling only when not in view mode. */
				if (!ISSET(VIEW_MODE))
					TOGGLE(MULTIBUFFER);
				else
					beep();
				continue;
			}
#endif
#ifndef NANO_TINY
			if (func == flip_convert) {
				TOGGLE(NO_CONVERT);
				continue;
			}
			if (func == flip_execute) {
				execute = !execute;
				continue;
			}
			if (func == flip_pipe) {
				add_or_remove_pipe_symbol_from_answer();
				given = mallocstrcpy(given, answer);
				continue;
			}
#endif
#ifdef ENABLE_BROWSER
			if (func == to_files) {
				char *chosen = browse_in(answer);

				/* If no file was chosen, go back to the prompt. */
				if (chosen == NULL)
					continue;

				free(answer);
				answer = chosen;
				response = 0;
			}
#endif
			/* If we don't have a file yet, go back to the prompt. */
			if (response != 0 && (!ISSET(MULTIBUFFER) || response != -2))
				continue;

#ifndef NANO_TINY
			if (execute) {
#ifdef ENABLE_MULTIBUFFER
				/* When in multibuffer mode, first open a blank buffer. */
				if (ISSET(MULTIBUFFER))
					open_buffer("", TRUE);
#endif
				/* If the command is not empty, execute it and read its output
				 * into the buffer, and add the command to the history list. */
				if (*answer != '\0') {
					execute_command(answer);
#ifdef ENABLE_HISTORIES
					update_history(&execute_history, answer);
#endif
				}

#ifdef ENABLE_MULTIBUFFER
				/* If this is a new buffer, put the cursor at the top. */
				if (ISSET(MULTIBUFFER)) {
					openfile->current = openfile->filetop;
					openfile->current_x = 0;
					openfile->placewewant = 0;

					set_modified();
				}
#endif
			} else
#endif /* !NANO_TINY */
			{
				/* Make sure the specified path is tilde-expanded. */
				answer = free_and_assign(answer, real_dir_from_tilde(answer));

				/* Read the file into a new buffer or into current buffer. */
				open_buffer(answer, ISSET(MULTIBUFFER));
			}

#ifdef ENABLE_MULTIBUFFER
			if (ISSET(MULTIBUFFER)) {
#ifdef ENABLE_HISTORIES
				if (ISSET(POSITIONLOG)) {
					ssize_t priorline, priorcol;
#ifndef NANO_TINY
					if (!execute)
#endif
					if (has_old_position(answer, &priorline, &priorcol))
						do_gotolinecolumn(priorline, priorcol, FALSE, FALSE);
				}
#endif
				/* Update title bar and color info for this new buffer. */
				prepare_for_display();
			} else
#endif /* ENABLE_MULTIBUFFER */
			{
				/* If the file actually changed, mark it as modified. */
				if (openfile->current->lineno != was_current_lineno ||
								openfile->current_x != was_current_x)
					set_modified();
#ifndef NANO_TINY
				/* Ensure that the buffer retains the format that it had. */
				openfile->fmt = was_fmt;
#endif
				refresh_needed = TRUE;
			}

			break;
		}
	}

	free(given);

#ifdef ENABLE_MULTIBUFFER
	if (was_multibuffer)
		SET(MULTIBUFFER);
	else
		UNSET(MULTIBUFFER);
#endif
}

/* If the current mode of operation allows it, go insert a file. */
void do_insertfile_void(void)
{
	if (!in_restricted_mode())
		do_insertfile(FALSE);
}

#ifndef NANO_TINY
/* If the current mode of operation allows it, go prompt for a command. */
void do_execute(void)
{
	if (!in_restricted_mode())
		do_insertfile(TRUE);
}
#endif

/* For the given bare path (or path plus filename), return the canonical,
 * absolute path (plus filename) when the path exists, and NULL when not. */
char *get_full_path(const char *origpath)
{
	char *allocation, *here, *target, *last_slash;
	char *just_filename = NULL;
	int attempts = 0;
	struct stat fileinfo;
	bool path_only;

	if (origpath == NULL)
		return NULL;

	allocation = charalloc(PATH_MAX + 1);
	here = getcwd(allocation, PATH_MAX + 1);

	/* If getting the current directory failed, go up one level and try again,
	 * until we find an existing directory, and use that as the current one. */
	while (here == NULL && attempts < 20) {
		IGNORE_CALL_RESULT(chdir(".."));
		here = getcwd(allocation, PATH_MAX + 1);
		attempts++;
	}

	/* If we found a directory, make sure its path ends in a slash. */
	if (here != NULL) {
		if (strcmp(here, "/") != 0) {
			here = charealloc(here, strlen(here) + 2);
			strcat(here, "/");
		}
	} else {
		here = copy_of("");
		free(allocation);
	}

	target = real_dir_from_tilde(origpath);

	/* Determine whether the target path refers to a directory.  If statting
	 * target fails, however, assume that it refers to a new, unsaved file. */
	path_only = (stat(target, &fileinfo) != -1 && S_ISDIR(fileinfo.st_mode));

	/* If the target is a directory, make sure its path ends in a slash. */
	if (path_only) {
		size_t length = strlen(target);

		if (target[length - 1] != '/') {
			target = charealloc(target, length + 2);
			strcat(target, "/");
		}
	}

	last_slash = strrchr(target, '/');

	/* If the target path does not contain a slash, then it is a bare filename
	 * and must therefore be located in the working directory. */
	if (last_slash == NULL) {
		just_filename = target;
		target = here;
	} else {
		/* If target contains a filename, separate the two. */
		if (!path_only) {
			just_filename = copy_of(last_slash + 1);
			*(last_slash + 1) = '\0';
		}

		/* If we can't change to the target directory, give up.  Otherwise,
		 * get the canonical path to this target directory. */
		if (chdir(target) == -1) {
			free(target);
			target = NULL;
		} else {
			free(target);

			allocation = charalloc(PATH_MAX + 1);
			target = getcwd(allocation, PATH_MAX + 1);

			/* If we got a result, make sure it ends in a slash.
			 * Otherwise, ensure that we return NULL. */
			if (target != NULL) {
				if (strcmp(target, "/") != 0) {
					target = charealloc(target, strlen(target) + 2);
					strcat(target, "/");
				}
			} else {
				path_only = TRUE;
				free(allocation);
			}

			/* Finally, go back to where we were before.  We don't check
			 * for an error, since we can't do anything if we get one. */
			IGNORE_CALL_RESULT(chdir(here));
		}

		free(here);
	}

	/* If we were given more than a bare path, concatenate the target path
	 * with the filename portion to get the full, absolute file path. */
	if (!path_only && target != NULL) {
		target = charealloc(target, strlen(target) + strlen(just_filename) + 1);
		strcat(target, just_filename);
	}

	free(just_filename);

	return target;
}

/* Check whether the given path refers to a directory that is writable.
 * Return the absolute form of the path on success, and NULL on failure. */
char *check_writable_directory(const char *path)
{
	char *full_path = get_full_path(path);

	if (full_path == NULL)
		return NULL;

	if (full_path[strlen(full_path) - 1] != '/' || access(full_path, W_OK) != 0) {
		free(full_path);
		return NULL;
	}

	return full_path;
}

/* Create, safely, a temporary file in the standard temp directory.
 * On success, return the malloc()ed filename, plus the corresponding
 * file stream opened in read-write mode.  On error, return NULL. */
char *safe_tempfile(FILE **stream)
{
	const char *env_dir = getenv("TMPDIR");
	char *tempdir = NULL, *tempfile_name = NULL;
	int fd;

	/* Get the absolute path for the first directory among $TMPDIR
	 * and P_tmpdir that is writable, otherwise use /tmp/. */
	if (env_dir != NULL)
		tempdir = check_writable_directory(env_dir);

	if (tempdir == NULL)
		tempdir = check_writable_directory(P_tmpdir);

	if (tempdir == NULL)
		tempdir = copy_of("/tmp/");

	tempfile_name = charealloc(tempdir, strlen(tempdir) + 12);
	strcat(tempfile_name, "nano.XXXXXX");

	fd = mkstemp(tempfile_name);

	if (fd == -1) {
		free(tempfile_name);
		return NULL;
	}

	*stream = fdopen(fd, "r+b");

	return tempfile_name;
}

#ifdef ENABLE_OPERATINGDIR
/* Change to the specified operating directory, when it's valid. */
void init_operating_dir(void)
{
	char *target = get_full_path(operating_dir);

	/* If the operating directory is inaccessible, fail. */
	if (target == NULL || chdir(target) == -1)
		die(_("Invalid operating directory: %s\n"), operating_dir);

	free(operating_dir);
	operating_dir = charealloc(target, strlen(target) + 1);
}

/* Check whether the given path is outside of the operating directory.
 * Return TRUE if it is, and FALSE otherwise.  If tabbing is TRUE,
 * incomplete names that can grow into matches for the operating directory
 * are considered to be inside, so that tab completion will work. */
bool outside_of_confinement(const char *somepath, bool tabbing)
{
	bool is_inside, begins_to_be;
	char *fullpath;

	/* If no operating directory is set, there is nothing to check. */
	if (operating_dir == NULL)
		return FALSE;

	fullpath = get_full_path(somepath);

	/* When we can't get an absolute path, it means some directory in the path
	 * doesn't exist or is unreadable.  When not doing tab completion, somepath
	 * is what the user typed somewhere.  We don't want to report a non-existent
	 * directory as being outside the operating directory, so we return FALSE.
	 * When the user is doing tab completion, then somepath exists but is not
	 * executable.  So we say it is outside the operating directory. */
	if (fullpath == NULL)
		return tabbing;

	is_inside = (strstr(fullpath, operating_dir) == fullpath);
	begins_to_be = (tabbing && strstr(operating_dir, fullpath) == operating_dir);

	free(fullpath);

	return (!is_inside && !begins_to_be);
}
#endif

#ifndef NANO_TINY
/* Transform the specified backup directory to an absolute path,
 * and verify that it is usable. */
void init_backup_dir(void)
{
	char *target = get_full_path(backup_dir);

	/* If we can't get an absolute path (which means it doesn't exist or
	 * isn't accessible), or it's not a directory, fail. */
	if (target == NULL || target[strlen(target) - 1] != '/')
		die(_("Invalid backup directory: %s\n"), backup_dir);

	free(backup_dir);
	backup_dir = charealloc(target, strlen(target) + 1);
}
#endif

/* Read all data from inn, and write it to out.  File inn must be open for
 * reading, and out for writing.  Return 0 on success, a negative number on
 * read error, and a positive number on write error.  File inn is always
 * closed by this function, out is closed  only if close_out is true. */
int copy_file(FILE *inn, FILE *out, bool close_out)
{
	int retval = 0;
	char buf[BUFSIZ];
	size_t charsread;
	int (*flush_out_fnc)(FILE *) = (close_out) ? fclose : fflush;

	do {
		charsread = fread(buf, sizeof(char), BUFSIZ, inn);
		if (charsread == 0 && ferror(inn)) {
			retval = -1;
			break;
		}
		if (fwrite(buf, sizeof(char), charsread, out) < charsread) {
			retval = 2;
			break;
		}
	} while (charsread > 0);

	if (fclose(inn) == EOF)
		retval = -3;
	if (flush_out_fnc(out) == EOF)
		retval = 4;

	return retval;
}

#ifndef NANO_TINY
/* Create a backup of an existing file.  If the user did not request backups,
 * make a temporary one (trying first in the directory of the original file,
 * then in the user's home directory).  Return TRUE if the save can proceed. */
bool make_backup_of(char *realname)
{
	FILE *original = NULL, *backup_file = NULL;
	int creation_flags, descriptor, verdict;
	static struct timespec filetime[2];
	bool second_attempt = FALSE;
	char *backupname = NULL;

	/* Remember the original file's access and modification times. */
	filetime[0].tv_sec = openfile->statinfo->st_atime;
	filetime[1].tv_sec = openfile->statinfo->st_mtime;

	statusbar(_("Making backup..."));

	/* If no backup directory was specified, we make a simple backup
	 * by appending a tilde to the original file name.  Otherwise,
	 * we create a numbered backup in the specified directory. */
	if (backup_dir == NULL) {
		backupname = charalloc(strlen(realname) + 2);
		sprintf(backupname, "%s~", realname);
	} else {
		char *thename = get_full_path(realname);

		/* If we have a valid absolute path, replace each slash
		 * in this full path with an exclamation mark.  Otherwise,
		 * just use the file-name portion of the given path. */
		if (thename) {
			for (int i = 0; thename[i] != '\0'; i++)
				if (thename[i] == '/')
					thename[i] = '!';
		} else
			thename = copy_of(tail(realname));

		backupname = charalloc(strlen(backup_dir) + strlen(thename) + 1);
		sprintf(backupname, "%s%s", backup_dir, thename);
		free(thename);

		thename = get_next_filename(backupname, "~");
		free(backupname);
		backupname = thename;

		/* If all numbered backup names are taken, the user must
		 * be fond of backups.  Thus, without one, do not go on. */
		if (*backupname == '\0') {
			statusline(ALERT, _("Too many existing backup files"));
			free(backupname);
			return FALSE;
		}
	}

	/* Now first try to delete an existing backup file. */
	if (unlink(backupname) < 0 && errno != ENOENT && !ISSET(INSECURE_BACKUP))
		goto problem;

	creation_flags = O_WRONLY|O_CREAT|(ISSET(INSECURE_BACKUP) ? O_TRUNC : O_EXCL);

	/* Create the backup file (or truncate the existing one). */
	descriptor = open(backupname, creation_flags, S_IRUSR|S_IWUSR);

  retry:
	if (descriptor >= 0)
		backup_file = fdopen(descriptor, "wb");

	if (backup_file == NULL)
		goto problem;

	/* Try to change owner and group to those of the original file;
	 * ignore permission errors, as a normal user cannot change the owner. */
	if (fchown(descriptor, openfile->statinfo->st_uid,
							openfile->statinfo->st_gid) < 0 && errno != EPERM) {
		fclose(backup_file);
		goto problem;
	}

	/* Set the backup's permissions to those of the original file.
	 * It is not a security issue if this fails, as we have created
	 * the file with just read and write permission for the owner. */
	if (fchmod(descriptor, openfile->statinfo->st_mode) < 0 && errno != EPERM) {
		fclose(backup_file);
		goto problem;
	}

	original = fopen(realname, "rb");

	/* If opening succeeded, copy the existing file to the backup. */
	if (original != NULL)
		verdict = copy_file(original, backup_file, FALSE);

	if (original == NULL || verdict < 0) {
		warn_and_briefly_pause(_("Cannot read original file"));
		fclose(backup_file);
		goto failure;
	} else if (verdict > 0) {
		fclose(backup_file);
		goto problem;
	}

	/* Since this backup is a newly created file, explicitly sync it to
	 * permanent storage before starting to write out the actual file. */
	if (fflush(backup_file) != 0 || fsync(fileno(backup_file)) != 0) {
		fclose(backup_file);
		goto problem;
	}

	/* Set the backup's timestamps to those of the original file.
	 * Failure is unimportant: saving the file apparently worked. */
	IGNORE_CALL_RESULT(futimens(descriptor, filetime));

	if (fclose(backup_file) == 0) {
		free(backupname);
		return TRUE;
	}

  problem:
	get_homedir();

	/* If the first attempt of copying the file failed, try again to HOME. */
	if (!second_attempt && homedir) {
		unlink(backupname);
		free(backupname);

		warn_and_briefly_pause(_("Cannot make regular backup"));
		warn_and_briefly_pause(_("Trying again in your home directory"));
		currmenu = MMOST;

		backupname = charalloc(strlen(homedir) + strlen(tail(realname)) + 9);
		sprintf(backupname, "%s/%s~XXXXXX", homedir, tail(realname));

		descriptor = mkstemp(backupname);
		backup_file = NULL;

		second_attempt = TRUE;
		goto retry;
	} else
		warn_and_briefly_pause(_("Cannot make backup"));

  failure:
	warn_and_briefly_pause(strerror(errno));
	currmenu = MMOST;
	free(backupname);

	/* If both attempts failed, and it isn't because of lack of disk space,
	 * ask the user what to do, because if something goes wrong during the
	 * save of the file itself, its contents may be lost. */
	/* TRANSLATORS: Try to keep this message at most 76 characters. */
	if (errno != ENOSPC && do_yesno_prompt(FALSE, _("Cannot make backup; "
								"continue and save actual file? ")) == 1)
		return TRUE;

	/* TRANSLATORS: The %s is the reason of failure. */
	statusline(HUSH, _("Cannot make backup: %s"), strerror(errno));
	return FALSE;
}
#endif /* !NANO_TINY */

/* Write the current buffer to disk.  If thefile isn't NULL, we write to a
 * temporary file that is already open.  If tmp is TRUE (when spell checking
 * or emergency dumping, for example), we don't make a backup and don't give
 * feedback.  If method is APPEND or PREPEND, it means we will be appending
 * or prepending instead of overwriting the given file.  If fullbuffer is TRUE
 * and when writing normally, we set the current filename and stat info.
 * Return TRUE on success, and FALSE otherwise. */
bool write_file(const char *name, FILE *thefile, bool tmp,
		kind_of_writing_type method, bool fullbuffer)
{
#ifndef NANO_TINY
	bool is_existing_file;
		/* Becomes TRUE when the file is non-temporary and exists. */
	struct stat st;
		/* The status fields filled in by statting the file. */
#endif
	char *realname = real_dir_from_tilde(name);
		/* The filename after tilde expansion. */
	char *tempname = NULL;
		/* The name of the temporary file we use when prepending. */
	linestruct *line = openfile->filetop;
		/* An iterator for moving through the lines of the buffer. */
	size_t lineswritten = 0;
		/* The number of lines written, for feedback on the status bar. */
	bool retval = FALSE;
		/* The return value, to become TRUE when writing has succeeded. */

#ifdef ENABLE_OPERATINGDIR
	/* If we're writing a temporary file, we're probably going outside
	 * the operating directory, so skip the operating directory test. */
	if (!tmp && outside_of_confinement(realname, FALSE)) {
		statusline(ALERT, _("Can't write outside of %s"), operating_dir);
		goto cleanup_and_exit;
	}
#endif
#ifndef NANO_TINY
	/* Check whether the file (at the end of the symlink) exists. */
	is_existing_file = (!tmp) && (stat(realname, &st) != -1);

	/* If we haven't statted this file before (say, the user just specified
	 * it interactively), stat and save the value now, or else we will chase
	 * null pointers when we do modtime checks and such during backup. */
	if (openfile->statinfo == NULL && is_existing_file)
		stat_with_alloc(realname, &openfile->statinfo);

	/* When the user requested a backup, we do this only if the file exists and
	 * isn't temporary AND the file has not been modified by someone else since
	 * we opened it (or we are appending/prepending or writing a selection). */
	if (ISSET(MAKE_BACKUP) && is_existing_file && openfile->statinfo &&
						(openfile->statinfo->st_mtime == st.st_mtime ||
						method != OVERWRITE || openfile->mark)) {
		if (!make_backup_of(realname))
			goto cleanup_and_exit;
	}

	/* When prepending, first copy the existing file to a temporary file. */
	if (method == PREPEND) {
		FILE *source = fopen(realname, "rb");
		FILE *target = NULL;
		int verdict;

		if (source == NULL) {
			statusline(ALERT, _("Error reading %s: %s"), realname, strerror(errno));
			goto cleanup_and_exit;
		}

		tempname = safe_tempfile(&target);

		if (tempname == NULL) {
			statusline(ALERT, _("Error writing temp file: %s"), strerror(errno));
			fclose(source);
			goto cleanup_and_exit;
		}

		verdict = copy_file(source, target, TRUE);

		if (verdict < 0) {
			statusline(ALERT, _("Error reading %s: %s"), realname, strerror(errno));
			unlink(tempname);
			goto cleanup_and_exit;
		} else if (verdict > 0) {
			statusline(ALERT, _("Error writing temp file: %s"), strerror(errno));
			unlink(tempname);
			goto cleanup_and_exit;
		}
	}

	if (is_existing_file && S_ISFIFO(st.st_mode))
		statusbar(_("Writing to FIFO..."));
#endif /* !NANO_TINY */

	/* When it's not a temporary file, this is where we open or create it.
	 * For an emergency file, access is restricted to just the owner. */
	if (thefile == NULL) {
		mode_t permissions = (tmp ? S_IRUSR|S_IWUSR : RW_FOR_ALL);
		int fd;

#ifndef NANO_TINY
		block_sigwinch(TRUE);
		install_handler_for_Ctrl_C();
#endif

		/* Now open the file.  Use O_EXCL for an emergency file. */
		fd = open(realname, O_WRONLY | O_CREAT | ((method == APPEND) ?
					O_APPEND : (tmp ? O_EXCL : O_TRUNC)), permissions);

#ifndef NANO_TINY
		restore_handler_for_Ctrl_C();
		block_sigwinch(FALSE);
#endif

		/* If we couldn't open the file, give up. */
		if (fd == -1) {
			if (errno == EINTR || errno == 0)
				statusline(ALERT, _("Interrupted"));
			else
				statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
#ifndef NANO_TINY
			if (tempname != NULL)
				unlink(tempname);
#endif
			goto cleanup_and_exit;
		}

		thefile = fdopen(fd, (method == APPEND) ? "ab" : "wb");

		if (thefile == NULL) {
			statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
			close(fd);
			goto cleanup_and_exit;
		}
	}

	if (!tmp)
		statusbar(_("Writing..."));

	while (TRUE) {
		size_t data_len = strlen(line->data);
		size_t wrote;

		/* Decode LFs as the NULs that they are, before writing to disk. */
		recode_LF_to_NUL(line->data);

		wrote = fwrite(line->data, sizeof(char), data_len, thefile);

		/* Re-encode any embedded NULs as LFs. */
		recode_NUL_to_LF(line->data, data_len);

		if (wrote < data_len) {
			statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
			fclose(thefile);
			goto cleanup_and_exit;
		}

		/* If we've reached the last line of the buffer, don't write a newline
		 * character after it.  If this last line is empty, it means zero bytes
		 * are written for it, and we don't count it in the number of lines. */
		if (line->next == NULL) {
			if (line->data[0] != '\0')
				lineswritten++;
			break;
		}

#ifndef NANO_TINY
		if (openfile->fmt == DOS_FILE || openfile->fmt == MAC_FILE) {
			if (putc('\r', thefile) == EOF) {
				statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
				fclose(thefile);
				goto cleanup_and_exit;
			}
		}

		if (openfile->fmt != MAC_FILE)
#endif
			if (putc('\n', thefile) == EOF) {
				statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
				fclose(thefile);
				goto cleanup_and_exit;
			}

		line = line->next;
		lineswritten++;
	}

#ifndef NANO_TINY
	/* When prepending, append the temporary file to what we wrote above. */
	if (method == PREPEND) {
		FILE *source = fopen(tempname, "rb");
		int verdict;

		if (source == NULL) {
			statusline(ALERT, _("Error reading temp file: %s"), strerror(errno));
			fclose(thefile);
			goto cleanup_and_exit;
		}

		verdict = copy_file(source, thefile, FALSE);

		if (verdict < 0) {
			statusline(ALERT, _("Error reading temp file: %s"), strerror(errno));
			fclose(thefile);
			goto cleanup_and_exit;
		} else if (verdict > 0) {
			statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
			fclose(thefile);
			goto cleanup_and_exit;
		}

		unlink(tempname);
	}
#endif

	/* Ensure the data has reached the disk before reporting it as written. */
	if (fflush(thefile) != 0 || fsync(fileno(thefile)) != 0) {
		statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
		fclose(thefile);
		goto cleanup_and_exit;
	}

	if (fclose(thefile) != 0) {
		statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
		goto cleanup_and_exit;
	}

	/* When having written an entire buffer, update some administrivia. */
	if (fullbuffer && method == OVERWRITE && !tmp) {
		/* If the filename was changed, write a new lockfile when needed,
		 * and check whether it means a different syntax gets used. */
		if (strcmp(openfile->filename, realname) != 0) {
#ifndef NANO_TINY
			if (openfile->lock_filename != NULL) {
				delete_lockfile(openfile->lock_filename);
				free(openfile->lock_filename);
				openfile->lock_filename = do_lockfile(realname, FALSE);
			}
#endif
			openfile->filename = mallocstrcpy(openfile->filename, realname);
#ifdef ENABLE_COLOR
			const char *oldname, *newname;

			oldname = openfile->syntax ? openfile->syntax->name : "";
			find_and_prime_applicable_syntax();
			newname = openfile->syntax ? openfile->syntax->name : "";

			/* If the syntax changed, discard and recompute the multidata. */
			if (strcmp(oldname, newname) != 0) {
				for (line = openfile->filetop; line != NULL; line = line->next) {
					free(line->multidata);
					line->multidata = NULL;
				}

				precalc_multicolorinfo();
				have_palette = FALSE;
				refresh_needed = TRUE;
			}
#endif
		}
#ifndef NANO_TINY
		/* Get or update the stat info to reflect the current state. */
		stat_with_alloc(realname, &openfile->statinfo);

		/* Record at which point in the undo stack the file was saved. */
		openfile->last_saved = openfile->current_undo;
		openfile->last_action = OTHER;
#endif
		openfile->modified = FALSE;
		titlebar(NULL);
	}

	if (!tmp)
		statusline(HUSH, P_("Wrote %zu line", "Wrote %zu lines",
								lineswritten), lineswritten);
	retval = TRUE;

  cleanup_and_exit:
	free(tempname);
	free(realname);

	return retval;
}

#ifndef NANO_TINY
/* Write the marked region of the current buffer out to disk.
 * Return TRUE on success and FALSE on error. */
bool write_marked_file(const char *name, FILE *stream, bool tmp,
		kind_of_writing_type method)
{
	linestruct *birthline, *topline, *botline, *stopper, *afterline;
	char *was_datastart, saved_byte;
	size_t top_x, bot_x;
	bool retval;

	get_region(&topline, &top_x, &botline, &bot_x);

	/* When needed, prepare a magic end line for the region. */
	if (bot_x > 0 && !ISSET(NO_NEWLINES)) {
		stopper = make_new_node(botline);
		stopper->data = copy_of("");
	} else
		stopper = NULL;

	/* Make the marked area look like a separate buffer. */
	afterline = botline->next;
	botline->next = stopper;
	saved_byte = botline->data[bot_x];
	botline->data[bot_x] = '\0';
	was_datastart = topline->data;
	topline->data += top_x;
	birthline = openfile->filetop;
	openfile->filetop = topline;

	retval = write_file(name, stream, tmp, method, FALSE);

	/* Restore the proper state of the buffer. */
	openfile->filetop = birthline;
	topline->data = was_datastart;
	botline->data[bot_x] = saved_byte;
	botline->next = afterline;

	if (stopper)
		delete_node(stopper);

	return retval;
}
#endif /* !NANO_TINY */

/* Write the current file to disk.  If the mark is on, write the current
 * marked selection to disk.  If exiting is TRUE, write the entire file
 * to disk regardless of whether the mark is on.  Do not ask for a name
 * when withprompt is FALSE nor when the SAVE_ON_EXIT flag is set and the
 * file already has a name.  Return 0 on error, 1 on success, and 2 when
 * the buffer is to be discarded. */
int do_writeout(bool exiting, bool withprompt)
{
	bool result = FALSE;
	kind_of_writing_type method = OVERWRITE;
	char *given;
		/* The filename we offer, or what the user typed so far. */
	bool maychange = (openfile->filename[0] == '\0');
		/* Whether it's okay to save the file under a different name. */
#ifdef ENABLE_EXTRA
	static bool did_credits = FALSE;
#endif

	/* Display newlines in filenames as ^J. */
	as_an_at = FALSE;

#ifndef NANO_TINY
	given = copy_of((openfile->mark && !exiting) ? "" : openfile->filename);
#else
	given = copy_of(openfile->filename);
#endif

	while (TRUE) {
		int response = 0, choice = 0;
		functionptrtype func;
		const char *msg;
#ifndef NANO_TINY
		const char *formatstr = (openfile->fmt == DOS_FILE) ? _(" [DOS Format]") :
						(openfile->fmt == MAC_FILE) ? _(" [Mac Format]") : "";
		const char *backupstr = ISSET(MAKE_BACKUP) ? _(" [Backup]") : "";

		/* When the mark is on, offer to write the selection to disk, but
		 * not when in restricted mode, because it would allow writing to
		 * a file not specified on the command line. */
		if (openfile->mark && !exiting && !ISSET(RESTRICTED))
			/* TRANSLATORS: The next six strings are prompts. */
			msg = (method == PREPEND) ? _("Prepend Selection to File") :
						(method == APPEND) ? _("Append Selection to File") :
						_("Write Selection to File");
		else if (method != OVERWRITE)
			msg = (method == PREPEND) ? _("File Name to Prepend to") :
										_("File Name to Append to");
		else
#endif
			msg = _("File Name to Write");

		present_path = mallocstrcpy(present_path, "./");

		/* When we shouldn't prompt, use the existing filename.
		 * Otherwise, ask for (confirmation of) the filename. */
		if ((!withprompt || (ISSET(SAVE_ON_EXIT) && exiting)) &&
								openfile->filename[0] != '\0')
			answer = mallocstrcpy(answer, openfile->filename);
		else
			response = do_prompt(MWRITEFILE, given, NULL,
						edit_refresh, "%s%s%s", msg,
#ifndef NANO_TINY
						formatstr, backupstr
#else
						"", ""
#endif
						);

		if (response < 0) {
			statusbar(_("Cancelled"));
			break;
		}

		func = func_from_key(&response);

		/* Upon request, abandon the buffer. */
		if (func == discard_buffer) {
			free(given);
			return 2;
		}

		given = mallocstrcpy(given, answer);

#ifdef ENABLE_BROWSER
		if (func == to_files) {
			char *chosen = browse_in(answer);

			if (chosen == NULL)
				continue;

			free(answer);
			answer = chosen;
		} else
#endif
#ifndef NANO_TINY
		if (func == dos_format_void) {
			openfile->fmt = (openfile->fmt == DOS_FILE) ? NIX_FILE : DOS_FILE;
			continue;
		} else if (func == mac_format_void) {
			openfile->fmt = (openfile->fmt == MAC_FILE) ? NIX_FILE : MAC_FILE;
			continue;
		} else if (func == backup_file_void) {
			TOGGLE(MAKE_BACKUP);
			continue;
		} else if (func == prepend_void) {
			method = (method == PREPEND) ? OVERWRITE : PREPEND;
			continue;
		} else if (func == append_void) {
			method = (method == APPEND) ? OVERWRITE : APPEND;
			continue;
		}
#endif
		if (func == do_help) {
			continue;
		}

#ifdef ENABLE_EXTRA
		/* If the user pressed Ctrl-X in the edit window, and answered "Y" at
		 * the "Save modified buffer?" prompt, and entered "zzy" as filename,
		 * and this is the first time around, show an Easter egg. */
		if (exiting && !ISSET(SAVE_ON_EXIT) && openfile->filename[0] == '\0' &&
						strcmp(answer, "zzy") == 0 && !did_credits) {
			if (LINES > 5 && COLS > 31) {
				do_credits();
				did_credits = TRUE;
			} else
				/* TRANSLATORS: Concisely say the screen is too small. */
				statusbar(_("Too tiny"));
			break;
		}
#endif

		if (method == OVERWRITE) {
			bool name_exists, do_warning;
			char *full_answer, *full_filename;
			struct stat st;

			full_answer = get_full_path(answer);
			full_filename = get_full_path(openfile->filename);
			name_exists = (stat((full_answer == NULL) ?
								answer : full_answer, &st) != -1);
			if (openfile->filename[0] == '\0')
				do_warning = name_exists;
			else
				do_warning = (strcmp((full_answer == NULL) ?
								answer : full_answer, (full_filename == NULL) ?
								openfile->filename : full_filename) != 0);

			free(full_filename);
			free(full_answer);

			if (do_warning) {
				/* When in restricted mode, we aren't allowed to overwrite
				 * an existing file with the current buffer, nor to change
				 * the name of the current file if it already has one. */
				if (ISSET(RESTRICTED)) {
					/* TRANSLATORS: Restricted mode forbids overwriting. */
					warn_and_briefly_pause(_("File exists -- "
												"cannot overwrite"));
					continue;
				}

				if (!maychange) {
#ifndef NANO_TINY
					if (exiting || !openfile->mark)
#endif
					{
						if (do_yesno_prompt(FALSE, _("Save file under "
												"DIFFERENT NAME? ")) < 1)
							continue;
						maychange = TRUE;
					}
				}

				if (name_exists) {
					char *question = _("File \"%s\" exists; OVERWRITE? ");
					char *name = display_string(answer, 0,
										COLS - breadth(question) + 1, FALSE, FALSE);
					char *message = charalloc(strlen(question) +
												strlen(name) + 1);

					sprintf(message, question, name);

					choice = do_yesno_prompt(FALSE, message);

					free(message);
					free(name);

					if (choice < 1)
						continue;
				}
			}
#ifndef NANO_TINY
			/* Complain if the file exists, the name hasn't changed,
			 * and the stat information we had before does not match
			 * what we have now. */
			else if (name_exists && openfile->statinfo &&
						(openfile->statinfo->st_mtime < st.st_mtime ||
						openfile->statinfo->st_dev != st.st_dev ||
						openfile->statinfo->st_ino != st.st_ino)) {

				warn_and_briefly_pause(_("File on disk has changed"));

				/* TRANSLATORS: Try to keep this at most 76 characters. */
				choice = do_yesno_prompt(FALSE, _("File was modified "
								"since you opened it; continue saving? "));
				wipe_statusbar();

				/* When in tool mode and not called by 'savefile',
				 * overwrite the file right here when requested. */
				if (ISSET(SAVE_ON_EXIT) && withprompt) {
					free(given);
					if (choice == 1)
						return write_file(openfile->filename, NULL,
											FALSE, OVERWRITE, TRUE);
					else if (choice == 0)
						return 2;
					else
						return 0;
				} else if (choice != 1) {
					free(given);
					return 1;
				}
			}
#endif
		}

		/* Here's where we allow the selected text to be written to
		 * a separate file.  If we're using restricted mode, this
		 * function is disabled, since it allows reading from or
		 * writing to files not specified on the command line. */
#ifndef NANO_TINY
		if (openfile->mark && !exiting && withprompt && !ISSET(RESTRICTED))
			result = write_marked_file(answer, NULL, FALSE, method);
		else
#endif
			result = write_file(answer, NULL, FALSE, method, TRUE);

		break;
	}

	free(given);

	return result ? 1 : 0;
}

/* Write the current buffer to disk, or discard it. */
void do_writeout_void(void)
{
	/* If the user chose to discard the buffer, close it. */
	if (do_writeout(FALSE, TRUE) == 2)
		close_and_go();
}

/* If it has a name, write the current file to disk without prompting. */
void do_savefile(void)
{
	if (do_writeout(FALSE, FALSE) == 2)
		close_and_go();
}

/* Convert the tilde notation when the given path begins with ~/ or ~user/.
 * Return an allocated string containing the expanded path. */
char *real_dir_from_tilde(const char *path)
{
	char *tilded, *retval;
	size_t i = 1;

	if (*path != '~')
		return copy_of(path);

	/* Figure out how much of the string we need to compare. */
	while (path[i] != '/' && path[i] != '\0')
		i++;

	if (i == 1) {
		get_homedir();
		tilded = copy_of(homedir);
	} else {
#ifdef HAVE_PWD_H
		const struct passwd *userdata;

		tilded = measured_copy(path, i);

		do {
			userdata = getpwent();
		} while (userdata && strcmp(userdata->pw_name, tilded + 1) != 0);
		endpwent();

		if (userdata != NULL)
			tilded = mallocstrcpy(tilded, userdata->pw_dir);
#else
		tilded = copy_of("");
#endif
	}

	retval = charalloc(strlen(tilded) + strlen(path + i) + 1);
	sprintf(retval, "%s%s", tilded, path + i);

	free(tilded);

	return retval;
}

#if defined(ENABLE_TABCOMP) || defined(ENABLE_BROWSER)
/* Our sort routine for file listings.  Sort alphabetically and
 * case-insensitively, and sort directories before filenames. */
int diralphasort(const void *va, const void *vb)
{
	struct stat fileinfo;
	const char *a = *(const char *const *)va;
	const char *b = *(const char *const *)vb;
	bool aisdir = stat(a, &fileinfo) != -1 && S_ISDIR(fileinfo.st_mode);
	bool bisdir = stat(b, &fileinfo) != -1 && S_ISDIR(fileinfo.st_mode);

	if (aisdir && !bisdir)
		return -1;
	if (!aisdir && bisdir)
		return 1;

	/* Standard function brain damage: We should be sorting
	 * alphabetically and case-insensitively according to the current
	 * locale, but there's no standard strcasecoll() function, so we
	 * have to use multibyte strcasecmp() instead. */
	return mbstrcasecmp(a, b);
}
#endif

#ifdef ENABLE_TABCOMP
/* Return TRUE when the given path is a directory. */
bool is_dir(const char *path)
{
	char *realpath = real_dir_from_tilde(path);
	struct stat fileinfo;
	bool retval;

	retval = (stat(realpath, &fileinfo) != -1 && S_ISDIR(fileinfo.st_mode));

	free(realpath);

	return retval;
}

/* Try to complete the given fragment of given length to a username. */
char **username_completion(const char *morsel, size_t length, size_t *num_matches)
{
	char **matches = NULL;
#ifdef HAVE_PWD_H
	const struct passwd *userdata;

	/* Iterate through the entries in the passwd file, and
	 * add each fitting username to the list of matches. */
	while ((userdata = getpwent()) != NULL) {
		if (strncmp(userdata->pw_name, morsel + 1, length - 1) == 0) {
#ifdef ENABLE_OPERATINGDIR
			/* Skip directories that are outside of the allowed area. */
			if (outside_of_confinement(userdata->pw_dir, TRUE))
				continue;
#endif
			matches = (char **)nrealloc(matches, (*num_matches + 1) * sizeof(char *));
			matches[*num_matches] = charalloc(strlen(userdata->pw_name) + 2);
			sprintf(matches[*num_matches], "~%s", userdata->pw_name);
			++(*num_matches);
		}
	}

	endpwent();
#endif

	return matches;
}

/* The next two functions were adapted from busybox 0.46 (cmdedit.c).
 * Here is the tweaked notice from that file:
 *
 * Termios command-line History and Editing, originally intended for NetBSD.
 * Copyright (C) 1999, 2000
 *      Main code:            Adam Rogoyski <rogoyski@cs.utexas.edu>
 *      Etc:                  Dave Cinege <dcinege@psychosis.com>
 *      Adjusted/rewritten:   Erik Andersen <andersee@debian.org>
 *
 * You may use this code as you wish, so long as the original author(s)
 * are attributed in any redistributions of the source code.
 * This code is 'as is' with no warranty.
 * This code may safely be consumed by a BSD or GPL license. */

/* Try to complete the given fragment of given length to a filename. */
char **filename_completion(const char *morsel, size_t length, size_t *num_matches)
{
	char *dirname = copy_of(morsel);
	char *slash, *filename;
	size_t filenamelen;
	char *fullname = NULL;
	char **matches = NULL;
	const struct dirent *entry;
	DIR *dir;

	/* If there's a / in the name, split out filename and directory parts. */
	slash = strrchr(dirname, '/');
	if (slash != NULL) {
		char *wasdirname = dirname;

		filename = copy_of(++slash);
		/* Cut off the filename part after the slash. */
		*slash = '\0';
		dirname = real_dir_from_tilde(dirname);
		/* A non-absolute path is relative to the current browser directory. */
		if (dirname[0] != '/') {
			dirname = charealloc(dirname, strlen(present_path) + strlen(wasdirname) + 1);
			sprintf(dirname, "%s%s", present_path, wasdirname);
		}
		free(wasdirname);
	} else {
		filename = dirname;
		dirname = copy_of(present_path);
	}

	dir = opendir(dirname);

	if (dir == NULL) {
		beep();
		free(filename);
		free(dirname);
		return NULL;
	}

	filenamelen = strlen(filename);

	/* Iterate through the filenames in the directory,
	 * and add each fitting one to the list of matches. */
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, filename, filenamelen) == 0 &&
									strcmp(entry->d_name, ".") != 0 &&
									strcmp(entry->d_name, "..") != 0) {
			fullname = charealloc(fullname, strlen(dirname) +
											strlen(entry->d_name) + 1);

			sprintf(fullname, "%s%s", dirname, entry->d_name);

#ifdef ENABLE_OPERATINGDIR
			if (outside_of_confinement(fullname, TRUE))
				continue;
#endif
			if (currmenu == MGOTODIR && !is_dir(fullname))
				continue;

			matches = (char **)nrealloc(matches, (*num_matches + 1) * sizeof(char *));
			matches[*num_matches] = copy_of(entry->d_name);
			++(*num_matches);
		}
	}

	closedir(dir);
	free(dirname);
	free(filename);
	free(fullname);

	return matches;
}

/* Do tab completion.  'place' is the position of the status-bar cursor, and
 * 'refresh_func' is the function to be called to refresh the edit window. */
char *input_tab(char *morsel, size_t *place, void (*refresh_func)(void), bool *listed)
{
	size_t num_matches = 0;
	char **matches = NULL;

	/* If the cursor is not at the end of the fragment, do nothing. */
	if (morsel[*place] != '\0') {
		beep();
		return morsel;
	}

	/* If the fragment starts with a tilde and contains no slash,
	 * then try completing it as a username. */
	if (morsel[0] == '~' && strchr(morsel, '/') == NULL)
		matches = username_completion(morsel, *place, &num_matches);

	/* If there are no matches yet, try matching against filenames. */
	if (matches == NULL)
		matches = filename_completion(morsel, *place, &num_matches);

	/* If possible completions were listed before but none will be listed now... */
	if (*listed && num_matches < 2) {
		refresh_func();
		*listed = FALSE;
	}

	if (matches == NULL) {
		beep();
		return morsel;
	}

	const char *lastslash = revstrstr(morsel, "/", morsel + *place);
	size_t length_of_path = (lastslash == NULL) ? 0 : lastslash - morsel + 1;
	size_t match, common_len = 0;
	char *shared, *glued;
	char char1[MAXCHARLEN], char2[MAXCHARLEN];
	int len1, len2;

	/* Determine the number of characters that all matches have in common. */
	while (TRUE) {
		len1 = collect_char(matches[0] + common_len, char1);

		for (match = 1; match < num_matches; match++) {
			len2 = collect_char(matches[match] + common_len, char2);

			if (len1 != len2 || strncmp(char1, char2, len2) != 0)
				break;
		}

		if (match < num_matches || matches[0][common_len] == '\0')
			break;

		common_len += len1;
	}

	shared = charalloc(length_of_path + common_len + 1);

	strncpy(shared, morsel, length_of_path);
	strncpy(shared + length_of_path, matches[0], common_len);

	common_len += length_of_path;
	shared[common_len] = '\0';

	/* Cover also the case of the user specifying a relative path. */
	glued = charalloc(strlen(present_path) + common_len + 1);
	sprintf(glued, "%s%s", present_path, shared);

	if (num_matches == 1 && (is_dir(shared) || is_dir(glued)))
		shared[common_len++] = '/';

	/* If the matches have something in common, copy that part. */
	if (common_len != *place) {
		morsel = charealloc(morsel, common_len + 1);
		strncpy(morsel, shared, common_len);
		morsel[common_len] = '\0';
		*place = common_len;
	} else if (num_matches == 1)
		beep();

	/* If there is more than one possible completion, show a sorted list. */
	if (num_matches > 1) {
		size_t longest_name = 0;
		size_t nrows, ncols;
		int row;

		if (!*listed)
			beep();

		qsort(matches, num_matches, sizeof(char *), diralphasort);

		/* Find the length of the longest name among the matches. */
		for (match = 0; match < num_matches; match++) {
			size_t namelen = breadth(matches[match]);

			if (namelen > longest_name)
				longest_name = namelen;
		}

		if (longest_name > COLS - 1)
			longest_name = COLS - 1;

		/* The columns of names will be separated by two spaces,
		 * but the last column will have just one space after it. */
		ncols = (COLS + 1) / (longest_name + 2);
		nrows = (num_matches + ncols - 1) / ncols;

		row = (nrows < editwinrows - 1) ? editwinrows - nrows - 1 : 0;

		/* Blank the edit window and hide the cursor. */
		blank_edit();
		curs_set(0);

		/* Now print the list of matches out there. */
		for (match = 0; match < num_matches; match++) {
			char *disp;

			wmove(edit, row, (longest_name + 2) * (match % ncols));

			if (row == editwinrows - 1 && (match + 1) % ncols == 0 &&
											match + 1 < num_matches) {
				waddstr(edit, _("(more)"));
				break;
			}

			disp = display_string(matches[match], 0, longest_name, FALSE, FALSE);
			waddstr(edit, disp);
			free(disp);

			if ((match + 1) % ncols == 0)
				row++;
		}

		wnoutrefresh(edit);
		*listed = TRUE;
	}

	free_chararray(matches, num_matches);
	free(glued);
	free(shared);

	return morsel;
}
#endif /* ENABLE_TABCOMP */
