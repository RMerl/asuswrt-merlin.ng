/*
 * files.c	Read config files into memory.
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 * Copyright 2000  Miquel van Smoorenburg <miquels@cistron.nl>
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>

/*
 *	Free a PAIR_LIST
 */
void pairlist_free(PAIR_LIST **pl)
{
	talloc_free(*pl);
	*pl = NULL;
}


#define FIND_MODE_NAME  0
#define FIND_MODE_REPLY 1

/*
 *	Read the users, huntgroups or hints file.
 *	Return a PAIR_LIST.
 */
int pairlist_read(TALLOC_CTX *ctx, char const *file, PAIR_LIST **list, int complain)
{
	FILE *fp;
	int mode = FIND_MODE_NAME;
	char entry[256];
	char buffer[8192];
	char const *ptr;
	VALUE_PAIR *check_tmp;
	VALUE_PAIR *reply_tmp;
	PAIR_LIST *pl = NULL, *t;
	PAIR_LIST **last = &pl;
	int lineno = 0;
	int old_lineno = 0;
	FR_TOKEN parsecode;
	char newfile[8192];

	DEBUG2("reading pairlist file %s", file);

	/*
	 *	Open the file.  The error message should be a little
	 *	more useful...
	 */
	if ((fp = fopen(file, "r")) == NULL) {
		if (!complain)
			return -1;
		ERROR("Couldn't open %s for reading: %s",
				file, strerror(errno));
		return -1;
	}

	parsecode = T_EOL;

	/*
	 *	Read the entire file into memory for speed.
	 */
	while(fgets(buffer, sizeof(buffer), fp) != NULL) {
		lineno++;
		if (!feof(fp) && (strchr(buffer, '\n') == NULL)) {
			fclose(fp);
			ERROR("%s[%d]: line too long", file, lineno);
			pairlist_free(&pl);
			return -1;
		}
		if (buffer[0] == '#' || buffer[0] == '\n') continue;

		/*
		 *	If the line contains nothing but whitespace,
		 *	ignore it.
		 */
		ptr = buffer;
		while (isspace((int) *ptr)) ptr++;
		if (*ptr == '\0') continue;

parse_again:
		if(mode == FIND_MODE_NAME) {
			/*
			 *	Find the entry starting with the users name
			 */
			if (isspace((int) buffer[0]))  {
				if (parsecode != T_EOL) {
					ERROR("%s[%d]: Unexpected trailing comma for entry %s",
					       file, lineno, entry);
					fclose(fp);
					return -1;
				}
				continue;
			}

			ptr = buffer;
			getword(&ptr, entry, sizeof(entry));

			/*
			 *	Include another file if we see
			 *	$INCLUDE filename
			 */
			if (strcasecmp(entry, "$INCLUDE") == 0) {
				while(isspace((int) *ptr))
					ptr++;

				/*
				 *	If it's an absolute pathname,
				 *	then use it verbatim.
				 *
				 *	If not, then make the $include
				 *	files *relative* to the current
				 *	file.
				 */
				if (FR_DIR_IS_RELATIVE(ptr)) {
					char *p;

					strlcpy(newfile, file,
						sizeof(newfile));
					p = strrchr(newfile, FR_DIR_SEP);
					if (!p) {
						p = newfile + strlen(newfile);
						*p = FR_DIR_SEP;
					}
					getword(&ptr, p + 1,
						sizeof(newfile) - 1 - (p - newfile));
				} else {
					getword(&ptr, newfile,
						sizeof(newfile));
				}

				t = NULL;

				if (pairlist_read(ctx, newfile, &t, 0) != 0) {
					pairlist_free(&pl);
					ERROR("%s[%d]: Could not open included file %s: %s",
					       file, lineno, newfile, strerror(errno));
					fclose(fp);
					return -1;
				}
				*last = t;

				/*
				 *	t may be NULL, it may have one
				 *	entry, or it may be a linked list
				 *	of entries.  Go to the end of the
				 *	list.
				 */
				while (*last)
					last = &((*last)->next);
				continue;
			}

			/*
			 *	Parse the check values
			 */
			check_tmp = NULL;
			reply_tmp = NULL;
			old_lineno = lineno;
			parsecode = userparse(ctx, ptr, &check_tmp);
			if (parsecode == T_OP_INVALID) {
				pairlist_free(&pl);
				ERROR("%s[%d]: Parse error (check) for entry %s: %s",
					file, lineno, entry, fr_strerror());
				fclose(fp);
				return -1;
			} else if (parsecode == T_COMMA) {
				ERROR("%s[%d]: Unexpected trailing comma in check item list for entry %s",
				       file, lineno, entry);
				fclose(fp);
				return -1;
			}
			mode = FIND_MODE_REPLY;
			parsecode = T_COMMA;
		}
		else {
			if(*buffer == ' ' || *buffer == '\t') {
				if (parsecode != T_COMMA) {
					ERROR("%s[%d]: Syntax error: Previous line is missing a trailing comma for entry %s",
					       file, lineno, entry);
					fclose(fp);
					return -1;
				}

				/*
				 *	Parse the reply values
				 */
				parsecode = userparse(ctx, buffer, &reply_tmp);
				/* valid tokens are 1 or greater */
				if (parsecode < 1) {
					pairlist_free(&pl);
					ERROR("%s[%d]: Parse error (reply) for entry %s: %s",
					       file, lineno, entry, fr_strerror());
					fclose(fp);
					return -1;
				}
			} else {
				/*
				 *	Done with this entry...
				 */
				MEM(t = talloc_zero(ctx, PAIR_LIST));

				t->check = check_tmp;
				t->reply = reply_tmp;
				t->lineno = old_lineno;
				check_tmp = NULL;
				reply_tmp = NULL;

				t->name = talloc_strdup(t, entry);

				*last = t;
				last = &(t->next);

				mode = FIND_MODE_NAME;
				if (buffer[0] != 0)
					goto parse_again;
			}
		}
	}
	/*
	 *	Make sure that we also read the last line of the file!
	 */
	if (mode == FIND_MODE_REPLY) {
		buffer[0] = 0;
		goto parse_again;
	}
	fclose(fp);

	*list = pl;
	return 0;
}


/*
 *	Debug code.
 */
#if 0
static void debug_pair_list(PAIR_LIST *pl)
{
	VALUE_PAIR *vp;

	while(pl) {
		printf("Pair list: %s\n", pl->name);
		printf("** Check:\n");
		for(vp = pl->check; vp; vp = vp->next) {
			printf("    ");
			fprint_attr_val(stdout, vp);
			printf("\n");
		}
		printf("** Reply:\n");
		for(vp = pl->reply; vp; vp = vp->next) {
			printf("    ");
			fprint_attr_val(stdout, vp);
			printf("\n");
		}
		pl = pl->next;
	}
}
#endif
