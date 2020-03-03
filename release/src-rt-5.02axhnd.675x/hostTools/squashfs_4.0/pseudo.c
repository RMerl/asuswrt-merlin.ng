/*
 * Create a squashfs filesystem.  This is a highly compressed read only filesystem.
 *
 * Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
 * Phillip Lougher <phillip@lougher.demon.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * pseudo.c
 */

#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "pseudo.h"

#ifdef SQUASHFS_TRACE
#define TRACE(s, args...)		do { \
						printf("mksquashfs: "s, ## args); \
					} while(0)
#else
#define TRACE(s, args...)
#endif

#define ERROR(s, args...)		do { \
						fprintf(stderr, s, ## args); \
					} while(0)
#define EXIT_MKSQUASHFS()		do { \
						exit(1); \
					} while(0)
#define BAD_ERROR(s, args...)		do {\
						fprintf(stderr, "FATAL ERROR:" s, ##args);\
						EXIT_MKSQUASHFS();\
					} while(0);

#define TRUE 1
#define FALSE 0

static void dump_pseudo(struct pseudo *pseudo, char *string)
{
	int i;
	char path[1024];

	for(i = 0; i < pseudo->names; i++) {
		struct pseudo_entry *entry = &pseudo->name[i];
		if(string)
			strcat(strcat(strcpy(path, string), "/"), entry->name);
		else
			strcpy(path, entry->name);
		if(entry->pseudo == NULL)
			ERROR("%s %c %o %d %d %d %d\n", path, entry->dev->type,
				entry->dev->mode, entry->dev->uid,
				entry->dev->gid, entry->dev->major,
				entry->dev->minor);
		else
			dump_pseudo(entry->pseudo, path);
	}
}


static char *get_component(char *target, char *targname)
{
	while(*target == '/')
		target ++;

	while(*target != '/' && *target!= '\0')
		*targname ++ = *target ++;

	*targname = '\0';

	return target;
}


/*
 * Add pseudo device target to the set of pseudo devices.  Pseudo_dev
 * describes the pseudo device attributes.
 */
struct pseudo *add_pseudo(struct pseudo *pseudo, struct pseudo_dev *pseudo_dev,
	char *target, char *alltarget)
{
	char targname[1024];
	int i, error;

	target = get_component(target, targname);

	if(pseudo == NULL) {
		if((pseudo = malloc(sizeof(struct pseudo))) == NULL)
			BAD_ERROR("failed to allocate pseudo file\n");

		pseudo->names = 0;
		pseudo->count = 0;
		pseudo->name = NULL;
	}

	for(i = 0; i < pseudo->names; i++)
		if(strcmp(pseudo->name[i].name, targname) == 0)
			break;

	if(i == pseudo->names) {
		/* allocate new name entry */
		pseudo->names ++;
		pseudo->name = realloc(pseudo->name, (i + 1) *
			sizeof(struct pseudo_entry));
		if(pseudo->name == NULL)
			BAD_ERROR("failed to allocate pseudo file\n");
		pseudo->name[i].name = strdup(targname);

		if(target[0] == '\0') {
			/* at leaf pathname component */
			pseudo->name[i].pseudo = NULL;
			pseudo->name[i].dev = malloc(sizeof(struct pseudo_dev));
			if(pseudo->name[i].dev == NULL)
				BAD_ERROR("failed to allocate pseudo file\n");
			pseudo->name[i].pathname = strdup(alltarget);
			memcpy(pseudo->name[i].dev, pseudo_dev,
				sizeof(struct pseudo_dev));
		} else {
			/* recurse adding child components */
			pseudo->name[i].dev = NULL;
			pseudo->name[i].pseudo = add_pseudo(NULL, pseudo_dev,
				target, alltarget);
		}
	} else {
		/* existing matching entry */
		if(pseudo->name[i].pseudo == NULL) {
			/* No sub-directory which means this is the leaf
			 * component of a pre-existing pseudo file.
			 */
			if(target[0] != '\0') {
				/* entry must exist as a 'd' type pseudo file */
				if(pseudo->name[i].dev->type == 'd')
					/* recurse adding child components */
					pseudo->name[i].pseudo =
						add_pseudo(NULL, pseudo_dev,
						target, alltarget);
				else
					ERROR("%s already exists as a non "
						"directory.  Ignoring %s!\n",
						 targname, alltarget);
			} else if(memcmp(pseudo_dev, pseudo->name[i].dev,
					sizeof(struct pseudo_dev)) != 0)
				ERROR("%s already exists as a different pseudo "
					"definition.  Ignoring!\n", alltarget);
			else ERROR("%s already exists as an identical "
					"pseudo definition!\n", alltarget);
		} else {
			/* sub-directory exists which means this can only be a
			 * 'd' type pseudo file */
			if(target[0] == '\0') {
				if(pseudo->name[i].dev == NULL &&
						pseudo_dev->type == 'd') {
					pseudo->name[i].dev =
						malloc(sizeof(struct pseudo_dev));
					if(pseudo->name[i].dev == NULL)
						BAD_ERROR("failed to allocate "
							"pseudo file\n");
					pseudo->name[i].pathname =
						strdup(alltarget);
					memcpy(pseudo->name[i].dev, pseudo_dev,
						sizeof(struct pseudo_dev));
				} else
					ERROR("%s already exists as a "
						"directory.  Ignoring %s!\n",
						targname, alltarget);
			} else
				/* recurse adding child components */
				add_pseudo(pseudo->name[i].pseudo, pseudo_dev,
					target, alltarget);
		}
	}

	return pseudo;
}


/*
 * Find subdirectory in pseudo directory referenced by pseudo, matching
 * filename.  If filename doesn't exist or if filename is a leaf file
 * return NULL
 */
struct pseudo *pseudo_subdir(char *filename, struct pseudo *pseudo)
{
	int i;

	if(pseudo == NULL)
		return NULL;

	for(i = 0; i < pseudo->names; i++)
		if(strcmp(filename, pseudo->name[i].name) == 0)
			return pseudo->name[i].pseudo;

	return NULL;
}


struct pseudo_entry *pseudo_readdir(struct pseudo *pseudo)
{
	if(pseudo == NULL)
		return NULL;

	while(pseudo->count < pseudo->names) {
		if(pseudo->name[pseudo->count].dev != NULL)
			return &pseudo->name[pseudo->count++];
		else
			pseudo->count++;
	}

	return NULL;
}


int read_pseudo_def(struct pseudo **pseudo, char *def)
{
	int n;
	unsigned int major = 0, minor = 0, mode;
	char filename[2048], type, suid[100], sgid[100], *ptr;
	long long uid, gid;
	struct pseudo_dev dev;

	n = sscanf(def, "%s %c %o %s %s %u %u", filename, &type, &mode, suid, sgid,
			&major, &minor);

	if(n < 5) {
		ERROR("Not enough or invalid arguments in pseudo file "
			"definition\n");
		goto error;
	}

	switch(type) {
	case 'b':
	case 'c':
		if(n < 7) {
			ERROR("Not enough or invalid arguments in pseudo file "
				"definition\n");
			goto error;
		}	
		
		if(major > 0xfff) {
			ERROR("Major %d out of range\n", major);
			goto error;
		}

		if(minor > 0xfffff) {
			ERROR("Minor %d out of range\n", minor);
			goto error;
		}

		/* fall through */
	case 'd':
		if(mode > 0777) {
			ERROR("Mode %o out of range\n", mode);
			goto error;
		}

		uid = strtoll(suid, &ptr, 10);
		if(*ptr == '\0') {
			if(uid < 0 || uid > ((1LL << 32) - 1)) {
				ERROR("Uid %s out of range\n", suid);
				goto error;
			}
		} else {
			struct passwd *pwuid = getpwnam(suid);
			if(pwuid)
				uid = pwuid->pw_uid;
			else {
				ERROR("Uid %s invalid uid or unknown user\n",
					suid);
				goto error;
			}
		}
		
		gid = strtoll(sgid, &ptr, 10);
		if(*ptr == '\0') {
			if(gid < 0 || gid > ((1LL << 32) - 1)) {
				ERROR("Gid %s out of range\n", sgid);
				goto error;
			}
		} else {
			struct group *grgid = getgrnam(sgid);
			if(grgid)
				gid = grgid->gr_gid;
			else {
				ERROR("Gid %s invalid uid or unknown user\n",
					sgid);
				goto error;
			}
		}

		break;
	default:
		ERROR("Unsupported type %c\n", type);
		goto error;
	}


	switch(type) {
	case 'b':
		mode |= S_IFBLK;
		break;
	case 'c':
		mode |= S_IFCHR;
		break;
	case 'd':
		mode |= S_IFDIR;
		break;
	}

	dev.type = type;
	dev.mode = mode;
	dev.uid = uid;
	dev.gid = gid;
	dev.major = major;
	dev.minor = minor;

	*pseudo = add_pseudo(*pseudo, &dev, filename, filename);

	return TRUE;

error:
	ERROR("Bad pseudo file definition \"%s\"\n", def);
	return FALSE;
}
		

int read_pseudo_file(struct pseudo **pseudo, char *filename)
{
	FILE *fd;
	char line[2048];
	int res = TRUE;

	fd = fopen(filename, "r");
	if(fd == NULL) {
		ERROR("Could not open pseudo device file \"%s\" because %s\n",
				filename, strerror(errno));
		return FALSE;
	}
	while(fscanf(fd, "%2047[^\n]\n", line) > 0) {
		res = read_pseudo_def(pseudo, line);
		if(res == FALSE)
			break;
	};
	fclose(fd);
	return res;
}
