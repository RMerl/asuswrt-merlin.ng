/* `dir', `vdir' and `ls' directory listing programs for GNU.

   Modified by Chet Ramey for Readline.

   Copyright (C) 1985, 1988, 1990-1991, 1995-2010, 2012, 2015
   Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Richard Stallman and David MacKenzie.  */

/* Color support by Peter Anvin <Peter.Anvin@linux.org> and Dennis
   Flaherty <dennisf@denix.elk.miles.com> based on original patches by
   Greg Lee <lee@uhunix.uhcc.hawaii.edu>.  */

#define READLINE_LIBRARY

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif

#include "rlconf.h"

#include <stdio.h>

#include "posixstat.h" // stat related macros (S_ISREG, ...)
#include <fcntl.h> // S_ISUID

// strlen()
#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

// abort()
#if defined (HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  include "ansi_stdlib.h"
#endif /* HAVE_STDLIB_H */

#include "readline.h"
#include "rldefs.h"

#ifdef COLOR_SUPPORT

#include "xmalloc.h"
#include "colors.h"

static bool is_colored (enum indicator_no type);
static void restore_default_color (void);

COLOR_EXT_TYPE *_rl_color_ext_list = 0;

/* Output a color indicator (which may contain nulls).  */
void
_rl_put_indicator (const struct bin_str *ind) {
  fwrite (ind->string, ind->len, 1, rl_outstream);
}

static bool
is_colored (enum indicator_no colored_filetype)
{
  size_t len = _rl_color_indicator[colored_filetype].len;
  char const *s = _rl_color_indicator[colored_filetype].string;
  return ! (len == 0
            || (len == 1 && strncmp (s, "0", 1) == 0)
            || (len == 2 && strncmp (s, "00", 2) == 0));
}

static void
restore_default_color (void)
{
  _rl_put_indicator (&_rl_color_indicator[C_LEFT]);
  _rl_put_indicator (&_rl_color_indicator[C_RIGHT]);
}

void
_rl_set_normal_color (void)
{
  if (is_colored (C_NORM))
    {
      _rl_put_indicator (&_rl_color_indicator[C_LEFT]);
      _rl_put_indicator (&_rl_color_indicator[C_NORM]);
      _rl_put_indicator (&_rl_color_indicator[C_RIGHT]);
    }
}

bool
_rl_print_prefix_color (void)
{
  struct bin_str *s;

  /* What do we want to use for the prefix? Let's try cyan first, see colors.h */
  s = &_rl_color_indicator[C_PREFIX];
  if (s->string != NULL)
    {
      if (is_colored (C_NORM))
	restore_default_color ();
      _rl_put_indicator (&_rl_color_indicator[C_LEFT]);
      _rl_put_indicator (s);
      _rl_put_indicator (&_rl_color_indicator[C_RIGHT]);
      return 0;
    }
  else
    return 1;
}
  
/* Returns whether any color sequence was printed. */
bool
_rl_print_color_indicator (const char *f)
{
  enum indicator_no colored_filetype;
  COLOR_EXT_TYPE *ext;	/* Color extension */
  size_t len;		/* Length of name */

  const char* name;
  char *filename;
  struct stat astat, linkstat;
  mode_t mode;
  int linkok;	/* 1 == ok, 0 == dangling symlink, -1 == missing */
  int stat_ok;

  name = f;

  /* This should already have undergone tilde expansion */
  filename = 0;
  if (rl_filename_stat_hook)
    {
      filename = savestring (f);
      (*rl_filename_stat_hook) (&filename);
      name = filename;
    }

#if defined (HAVE_LSTAT)
  stat_ok = lstat(name, &astat);
#else
  stat_ok = stat(name, &astat);
#endif
  if (stat_ok == 0)
    {
      mode = astat.st_mode;
#if defined (HAVE_LSTAT)
      if (S_ISLNK (mode))
	{
	  linkok = stat (name, &linkstat) == 0;
	  if (linkok && strncmp (_rl_color_indicator[C_LINK].string, "target", 6) == 0)
	    mode = linkstat.st_mode;
	}
      else
#endif
	linkok = 1;
    }
  else
    linkok = -1;

  /* Is this a nonexistent file?  If so, linkok == -1.  */

  if (linkok == -1 && _rl_color_indicator[C_MISSING].string != NULL)
    colored_filetype = C_MISSING;
  else if (linkok == 0 && S_ISLNK(mode) && _rl_color_indicator[C_ORPHAN].string != NULL)
    colored_filetype = C_ORPHAN;	/* dangling symlink */
  else if(stat_ok != 0)
    {
      static enum indicator_no filetype_indicator[] = FILETYPE_INDICATORS;
      colored_filetype = filetype_indicator[normal]; //f->filetype];
    }
  else
    {
      if (S_ISREG (mode))
        {
          colored_filetype = C_FILE;

          if ((mode & S_ISUID) != 0 && is_colored (C_SETUID))
            colored_filetype = C_SETUID;
          else if ((mode & S_ISGID) != 0 && is_colored (C_SETGID))
            colored_filetype = C_SETGID;
          else if (is_colored (C_CAP) && 0) //f->has_capability)
            colored_filetype = C_CAP;
          else if ((mode & S_IXUGO) != 0 && is_colored (C_EXEC))
            colored_filetype = C_EXEC;
          else if ((1 < astat.st_nlink) && is_colored (C_MULTIHARDLINK))
            colored_filetype = C_MULTIHARDLINK;
        }
      else if (S_ISDIR (mode))
        {
          colored_filetype = C_DIR;

#if defined (S_ISVTX)
          if ((mode & S_ISVTX) && (mode & S_IWOTH)
              && is_colored (C_STICKY_OTHER_WRITABLE))
            colored_filetype = C_STICKY_OTHER_WRITABLE;
          else
#endif
          if ((mode & S_IWOTH) != 0 && is_colored (C_OTHER_WRITABLE))
            colored_filetype = C_OTHER_WRITABLE;
#if defined (S_ISVTX)
          else if ((mode & S_ISVTX) != 0 && is_colored (C_STICKY))
            colored_filetype = C_STICKY;
#endif
        }
      else if (S_ISLNK (mode))
        colored_filetype = C_LINK;
      else if (S_ISFIFO (mode))
        colored_filetype = C_FIFO;
      else if (S_ISSOCK (mode))
        colored_filetype = C_SOCK;
      else if (S_ISBLK (mode))
        colored_filetype = C_BLK;
      else if (S_ISCHR (mode))
        colored_filetype = C_CHR;
      else
        {
          /* Classify a file of some other type as C_ORPHAN.  */
          colored_filetype = C_ORPHAN;
        }
    }

  /* Check the file's suffix only if still classified as C_FILE.  */
  ext = NULL;
  if (colored_filetype == C_FILE)
    {
      /* Test if NAME has a recognized suffix.  */
      len = strlen (name);
      name += len;		/* Pointer to final \0.  */
      for (ext = _rl_color_ext_list; ext != NULL; ext = ext->next)
        {
          if (ext->ext.len <= len
              && strncmp (name - ext->ext.len, ext->ext.string,
                          ext->ext.len) == 0)
            break;
        }
    }

  free (filename);	/* NULL or savestring return value */

  {
    const struct bin_str *const s
      = ext ? &(ext->seq) : &_rl_color_indicator[colored_filetype];
    if (s->string != NULL)
      {
        /* Need to reset so not dealing with attribute combinations */
        if (is_colored (C_NORM))
	  restore_default_color ();
        _rl_put_indicator (&_rl_color_indicator[C_LEFT]);
        _rl_put_indicator (s);
        _rl_put_indicator (&_rl_color_indicator[C_RIGHT]);
        return 0;
      }
    else
      return 1;
  }
}

void
_rl_prep_non_filename_text (void)
{
  if (_rl_color_indicator[C_END].string != NULL)
    _rl_put_indicator (&_rl_color_indicator[C_END]);
  else
    {
      _rl_put_indicator (&_rl_color_indicator[C_LEFT]);
      _rl_put_indicator (&_rl_color_indicator[C_RESET]);
      _rl_put_indicator (&_rl_color_indicator[C_RIGHT]);
    }
}
#endif /* COLOR_SUPPORT */
