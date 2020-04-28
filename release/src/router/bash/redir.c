/* redir.c -- Functions to perform input and output redirection. */

/* Copyright (C) 1997-2015 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#if !defined (__GNUC__) && !defined (HAVE_ALLOCA_H) && defined (_AIX)
  #pragma alloca
#endif /* _AIX && RISC6000 && !__GNUC__ */

#include <stdio.h>
#include "bashtypes.h"
#if !defined (_MINIX) && defined (HAVE_SYS_FILE_H)
#  include <sys/file.h>
#endif
#include "filecntl.h"
#include "posixstat.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <errno.h>

#if !defined (errno)
extern int errno;
#endif

#include "bashansi.h"
#include "bashintl.h"
#include "memalloc.h"

#define NEED_FPURGE_DECL

#include "shell.h"
#include "flags.h"
#include "execute_cmd.h"
#include "redir.h"
#include "trap.h"

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

#define SHELL_FD_BASE	10

int expanding_redir;

extern int posixly_correct;
extern int last_command_exit_value;
extern int executing_builtin;
extern REDIRECT *redirection_undo_list;
extern REDIRECT *exec_redirection_undo_list;

/* Static functions defined and used in this file. */
static void add_exec_redirect __P((REDIRECT *));
static int add_undo_redirect __P((int, enum r_instruction, int));
static int add_undo_close_redirect __P((int));
static int expandable_redirection_filename __P((REDIRECT *));
static int stdin_redirection __P((enum r_instruction, int));
static int undoablefd __P((int));
static int do_redirection_internal __P((REDIRECT *, int));

static int write_here_document __P((int, WORD_DESC *));
static int write_here_string __P((int, WORD_DESC *));
static int here_document_to_fd __P((WORD_DESC *, enum r_instruction));

static int redir_special_open __P((int, char *, int, int, enum r_instruction));
static int noclobber_open __P((char *, int, int, enum r_instruction));
static int redir_open __P((char *, int, int, enum r_instruction));

static int redir_varassign __P((REDIRECT *, int));
static int redir_varvalue __P((REDIRECT *));

/* Spare redirector used when translating [N]>&WORD[-] or [N]<&WORD[-] to
   a new redirection and when creating the redirection undo list. */
static REDIRECTEE rd;

/* Set to errno when a here document cannot be created for some reason.
   Used to print a reasonable error message. */
static int heredoc_errno;

#define REDIRECTION_ERROR(r, e, fd) \
do { \
  if ((r) < 0) \
    { \
      if (fd >= 0) \
	close (fd); \
      last_command_exit_value = EXECUTION_FAILURE;\
      return ((e) == 0 ? EINVAL : (e));\
    } \
} while (0)

void
redirection_error (temp, error)
     REDIRECT *temp;
     int error;
{
  char *filename, *allocname;
  int oflags;

  allocname = 0;
  if ((temp->rflags & REDIR_VARASSIGN) && error < 0)
    filename = allocname = savestring (temp->redirector.filename->word);
  else if ((temp->rflags & REDIR_VARASSIGN) == 0 && temp->redirector.dest < 0)
    /* This can happen when read_token_word encounters overflow, like in
       exec 4294967297>x */
    filename = _("file descriptor out of range");
#ifdef EBADF
  /* This error can never involve NOCLOBBER */
  else if (error != NOCLOBBER_REDIRECT && temp->redirector.dest >= 0 && error == EBADF)
    {
      /* If we're dealing with two file descriptors, we have to guess about
         which one is invalid; in the cases of r_{duplicating,move}_input and
         r_{duplicating,move}_output we're here because dup2() failed. */
      switch (temp->instruction)
        {
        case r_duplicating_input:
        case r_duplicating_output:
        case r_move_input:
        case r_move_output:
	  filename = allocname = itos (temp->redirectee.dest);
	  break;
	case r_duplicating_input_word:
	  if (temp->redirector.dest == 0)	/* Guess */
	    filename = temp->redirectee.filename->word;	/* XXX */
	  else
	    filename = allocname = itos (temp->redirector.dest);
	  break;
	case r_duplicating_output_word:
	  if (temp->redirector.dest == 1)	/* Guess */
	    filename = temp->redirectee.filename->word;	/* XXX */
	  else
	    filename = allocname = itos (temp->redirector.dest);
	  break;
	default:
	  filename = allocname = itos (temp->redirector.dest);
	  break;
        }
    }
#endif
  else if (expandable_redirection_filename (temp))
    {
expandable_filename:
      oflags = temp->redirectee.filename->flags;
      if (posixly_correct && interactive_shell == 0)
	temp->redirectee.filename->flags |= W_NOGLOB;
      temp->redirectee.filename->flags |= W_NOCOMSUB;
      filename = allocname = redirection_expand (temp->redirectee.filename);
      temp->redirectee.filename->flags = oflags;
      if (filename == 0)
	filename = temp->redirectee.filename->word;
    }
  else if (temp->redirectee.dest < 0)
    filename = _("file descriptor out of range");
  else
    filename = allocname = itos (temp->redirectee.dest);

  switch (error)
    {
    case AMBIGUOUS_REDIRECT:
      internal_error (_("%s: ambiguous redirect"), filename);
      break;

    case NOCLOBBER_REDIRECT:
      internal_error (_("%s: cannot overwrite existing file"), filename);
      break;

#if defined (RESTRICTED_SHELL)
    case RESTRICTED_REDIRECT:
      internal_error (_("%s: restricted: cannot redirect output"), filename);
      break;
#endif /* RESTRICTED_SHELL */

    case HEREDOC_REDIRECT:
      internal_error (_("cannot create temp file for here-document: %s"), strerror (heredoc_errno));
      break;

    case BADVAR_REDIRECT:
      internal_error (_("%s: cannot assign fd to variable"), filename);
      break;

    default:
      internal_error ("%s: %s", filename, strerror (error));
      break;
    }

  FREE (allocname);
}

/* Perform the redirections on LIST.  If flags & RX_ACTIVE, then actually
   make input and output file descriptors, otherwise just do whatever is
   necessary for side effecting.  flags & RX_UNDOABLE says to remember
   how to undo the redirections later, if non-zero.  If flags & RX_CLEXEC
   is non-zero, file descriptors opened in do_redirection () have their
   close-on-exec flag set. */
int
do_redirections (list, flags)
     REDIRECT *list;
     int flags;
{
  int error;
  REDIRECT *temp;

  if (flags & RX_UNDOABLE)
    {
      if (redirection_undo_list)
	{
	  dispose_redirects (redirection_undo_list);
	  redirection_undo_list = (REDIRECT *)NULL;
	}
      if (exec_redirection_undo_list)
	dispose_exec_redirects ();
    }

  for (temp = list; temp; temp = temp->next)
    {
      error = do_redirection_internal (temp, flags);
      if (error)
	{
	  redirection_error (temp, error);
	  return (error);
	}
    }
  return (0);
}

/* Return non-zero if the redirection pointed to by REDIRECT has a
   redirectee.filename that can be expanded. */
static int
expandable_redirection_filename (redirect)
     REDIRECT *redirect;
{
  switch (redirect->instruction)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:
    case r_append_err_and_out:
    case r_input_output:
    case r_output_force:
    case r_duplicating_input_word:
    case r_duplicating_output_word:
    case r_move_input_word:
    case r_move_output_word:
      return 1;

    default:
      return 0;
    }
}

/* Expand the word in WORD returning a string.  If WORD expands to
   multiple words (or no words), then return NULL. */
char *
redirection_expand (word)
     WORD_DESC *word;
{
  char *result;
  WORD_LIST *tlist1, *tlist2;
  WORD_DESC *w;
  int old;

  w = copy_word (word);
  if (posixly_correct)
    w->flags |= W_NOSPLIT;

  tlist1 = make_word_list (w, (WORD_LIST *)NULL);
  expanding_redir = 1;
  /* Now that we've changed the variable search order to ignore the temp
     environment, see if we need to change the cached IFS values. */
  sv_ifs ("IFS");
  tlist2 = expand_words_no_vars (tlist1);
  expanding_redir = 0;
  /* Now we need to change the variable search order back to include the temp
     environment.  We force the temp environment search by forcing
     executing_builtin to 1.  This is what makes `read' get the right values
     for the IFS-related cached variables, for example. */
  old = executing_builtin;
  executing_builtin = 1;
  sv_ifs ("IFS");
  executing_builtin = old;
  dispose_words (tlist1);

  if (tlist2 == 0 || tlist2->next)
    {
      /* We expanded to no words, or to more than a single word.
	 Dispose of the word list and return NULL. */
      if (tlist2)
	dispose_words (tlist2);
      return ((char *)NULL);
    }
  result = string_list (tlist2);  /* XXX savestring (tlist2->word->word)? */
  dispose_words (tlist2);
  return (result);
}

static int
write_here_string (fd, redirectee)
     int fd;
     WORD_DESC *redirectee;
{
  char *herestr;
  int herelen, n, e, old;

  expanding_redir = 1;
  /* Now that we've changed the variable search order to ignore the temp
     environment, see if we need to change the cached IFS values. */
  sv_ifs ("IFS");
  herestr = expand_string_unsplit_to_string (redirectee->word, 0);
  expanding_redir = 0;
  /* Now we need to change the variable search order back to include the temp
     environment.  We force the temp environment search by forcing
     executing_builtin to 1.  This is what makes `read' get the right values
     for the IFS-related cached variables, for example. */
  old = executing_builtin;
  executing_builtin = 1;
  sv_ifs ("IFS");
  executing_builtin = old;

  herelen = STRLEN (herestr);

  n = write (fd, herestr, herelen);
  if (n == herelen)
    {
      n = write (fd, "\n", 1);
      herelen = 1;
    }
  e = errno;
  FREE (herestr);
  if (n != herelen)
    {
      if (e == 0)
	e = ENOSPC;
      return e;
    }
  return 0;
}  

/* Write the text of the here document pointed to by REDIRECTEE to the file
   descriptor FD, which is already open to a temp file.  Return 0 if the
   write is successful, otherwise return errno. */
static int
write_here_document (fd, redirectee)
     int fd;
     WORD_DESC *redirectee;
{
  char *document;
  int document_len, fd2, old;
  FILE *fp;
  register WORD_LIST *t, *tlist;

  /* Expand the text if the word that was specified had
     no quoting.  The text that we expand is treated
     exactly as if it were surrounded by double quotes. */

  if (redirectee->flags & W_QUOTED)
    {
      document = redirectee->word;
      document_len = strlen (document);
      /* Set errno to something reasonable if the write fails. */
      if (write (fd, document, document_len) < document_len)
	{
	  if (errno == 0)
	    errno = ENOSPC;
	  return (errno);
	}
      else
	return 0;
    }

  expanding_redir = 1;
  /* Now that we've changed the variable search order to ignore the temp
     environment, see if we need to change the cached IFS values. */
  sv_ifs ("IFS");
  tlist = expand_string (redirectee->word, Q_HERE_DOCUMENT);
  expanding_redir = 0;
  /* Now we need to change the variable search order back to include the temp
     environment.  We force the temp environment search by forcing
     executing_builtin to 1.  This is what makes `read' get the right values
     for the IFS-related cached variables, for example. */
  old = executing_builtin;
  executing_builtin = 1;
  sv_ifs ("IFS");
  executing_builtin = old;

  if (tlist)
    {
      /* Try using buffered I/O (stdio) and writing a word
	 at a time, letting stdio do the work of buffering
	 for us rather than managing our own strings.  Most
	 stdios are not particularly fast, however -- this
	 may need to be reconsidered later. */
      if ((fd2 = dup (fd)) < 0 || (fp = fdopen (fd2, "w")) == NULL)
	{
	  old = errno;
	  if (fd2 >= 0)
	    close (fd2);
	  dispose_words (tlist);
	  errno = old;
	  return (errno);
	}
      errno = 0;
      for (t = tlist; t; t = t->next)
	{
	  /* This is essentially the body of
	     string_list_internal expanded inline. */
	  document = t->word->word;
	  document_len = strlen (document);
	  if (t != tlist)
	    putc (' ', fp);	/* separator */
	  fwrite (document, document_len, 1, fp);
	  if (ferror (fp))
	    {
	      if (errno == 0)
		errno = ENOSPC;
	      fd2 = errno;
	      fclose(fp);
	      dispose_words (tlist);
	      return (fd2);
	    }
	}
      dispose_words (tlist);
      if (fclose (fp) != 0)
	{
	  if (errno == 0)
	    errno = ENOSPC;
	  return (errno);
	}
    }
  return 0;
}

/* Create a temporary file holding the text of the here document pointed to
   by REDIRECTEE, and return a file descriptor open for reading to the temp
   file.  Return -1 on any error, and make sure errno is set appropriately. */
static int
here_document_to_fd (redirectee, ri)
     WORD_DESC *redirectee;
     enum r_instruction ri;
{
  char *filename;
  int r, fd, fd2;

  fd = sh_mktmpfd ("sh-thd", MT_USERANDOM|MT_USETMPDIR, &filename);

  /* If we failed for some reason other than the file existing, abort */
  if (fd < 0)
    {
      FREE (filename);
      return (fd);
    }

  SET_CLOSE_ON_EXEC (fd);

  errno = r = 0;		/* XXX */
  /* write_here_document returns 0 on success, errno on failure. */
  if (redirectee->word)
    r = (ri != r_reading_string) ? write_here_document (fd, redirectee)
				 : write_here_string (fd, redirectee);

  if (r)
    {
      close (fd);
      unlink (filename);
      free (filename);
      errno = r;
      return (-1);
    }

  /* In an attempt to avoid races, we close the first fd only after opening
     the second. */
  /* Make the document really temporary.  Also make it the input. */
  fd2 = open (filename, O_RDONLY|O_BINARY, 0600);

  if (fd2 < 0)
    {
      r = errno;
      unlink (filename);
      free (filename);
      close (fd);
      errno = r;
      return -1;
    }

  close (fd);
  if (unlink (filename) < 0)
    {
      r = errno;
      close (fd2);
      free (filename);
      errno = r;
      return (-1);
    }

  free (filename);
  return (fd2);
}

#define RF_DEVFD	1
#define RF_DEVSTDERR	2
#define RF_DEVSTDIN	3
#define RF_DEVSTDOUT	4
#define RF_DEVTCP	5
#define RF_DEVUDP	6

/* A list of pattern/value pairs for filenames that the redirection
   code handles specially. */
static STRING_INT_ALIST _redir_special_filenames[] = {
#if !defined (HAVE_DEV_FD)
  { "/dev/fd/[0-9]*", RF_DEVFD },
#endif
#if !defined (HAVE_DEV_STDIN)
  { "/dev/stderr", RF_DEVSTDERR },
  { "/dev/stdin", RF_DEVSTDIN },
  { "/dev/stdout", RF_DEVSTDOUT },
#endif
#if defined (NETWORK_REDIRECTIONS)
  { "/dev/tcp/*/*", RF_DEVTCP },
  { "/dev/udp/*/*", RF_DEVUDP },
#endif
  { (char *)NULL, -1 }
};

static int
redir_special_open (spec, filename, flags, mode, ri)
     int spec;
     char *filename;
     int flags, mode;
     enum r_instruction ri;
{
  int fd;
#if !defined (HAVE_DEV_FD)
  intmax_t lfd;
#endif

  fd = -1;
  switch (spec)
    {
#if !defined (HAVE_DEV_FD)
    case RF_DEVFD:
      if (all_digits (filename+8) && legal_number (filename+8, &lfd) && lfd == (int)lfd)
	{
	  fd = lfd;
	  fd = fcntl (fd, F_DUPFD, SHELL_FD_BASE);
	}
      else
	fd = AMBIGUOUS_REDIRECT;
      break;
#endif

#if !defined (HAVE_DEV_STDIN)
    case RF_DEVSTDIN:
      fd = fcntl (0, F_DUPFD, SHELL_FD_BASE);
      break;
    case RF_DEVSTDOUT:
      fd = fcntl (1, F_DUPFD, SHELL_FD_BASE);
      break;
    case RF_DEVSTDERR:
      fd = fcntl (2, F_DUPFD, SHELL_FD_BASE);
      break;
#endif

#if defined (NETWORK_REDIRECTIONS)
    case RF_DEVTCP:
    case RF_DEVUDP:
#if defined (HAVE_NETWORK)
      fd = netopen (filename);
#else
      internal_warning (_("/dev/(tcp|udp)/host/port not supported without networking"));
      fd = open (filename, flags, mode);
#endif
      break;
#endif /* NETWORK_REDIRECTIONS */
    }

  return fd;
}
      
/* Open FILENAME with FLAGS in noclobber mode, hopefully avoiding most
   race conditions and avoiding the problem where the file is replaced
   between the stat(2) and open(2). */
static int
noclobber_open (filename, flags, mode, ri)
     char *filename;
     int flags, mode;
     enum r_instruction ri;
{
  int r, fd;
  struct stat finfo, finfo2;

  /* If the file exists and is a regular file, return an error
     immediately. */
  r = stat (filename, &finfo);
  if (r == 0 && (S_ISREG (finfo.st_mode)))
    return (NOCLOBBER_REDIRECT);

  /* If the file was not present (r != 0), make sure we open it
     exclusively so that if it is created before we open it, our open
     will fail.  Make sure that we do not truncate an existing file.
     Note that we don't turn on O_EXCL unless the stat failed -- if
     the file was not a regular file, we leave O_EXCL off. */
  flags &= ~O_TRUNC;
  if (r != 0)
    {
      fd = open (filename, flags|O_EXCL, mode);
      return ((fd < 0 && errno == EEXIST) ? NOCLOBBER_REDIRECT : fd);
    }
  fd = open (filename, flags, mode);

  /* If the open failed, return the file descriptor right away. */
  if (fd < 0)
    return (errno == EEXIST ? NOCLOBBER_REDIRECT : fd);

  /* OK, the open succeeded, but the file may have been changed from a
     non-regular file to a regular file between the stat and the open.
     We are assuming that the O_EXCL open handles the case where FILENAME
     did not exist and is symlinked to an existing file between the stat
     and open. */

  /* If we can open it and fstat the file descriptor, and neither check
     revealed that it was a regular file, and the file has not been replaced,
     return the file descriptor. */
  if ((fstat (fd, &finfo2) == 0) && (S_ISREG (finfo2.st_mode) == 0) &&
      r == 0 && (S_ISREG (finfo.st_mode) == 0) &&
      same_file (filename, filename, &finfo, &finfo2))
    return fd;

  /* The file has been replaced.  badness. */
  close (fd);  
  errno = EEXIST;
  return (NOCLOBBER_REDIRECT);
}

static int
redir_open (filename, flags, mode, ri)
     char *filename;
     int flags, mode;
     enum r_instruction ri;
{
  int fd, r, e;

  r = find_string_in_alist (filename, _redir_special_filenames, 1);
  if (r >= 0)
    return (redir_special_open (r, filename, flags, mode, ri));

  /* If we are in noclobber mode, you are not allowed to overwrite
     existing files.  Check before opening. */
  if (noclobber && CLOBBERING_REDIRECT (ri))
    {
      fd = noclobber_open (filename, flags, mode, ri);
      if (fd == NOCLOBBER_REDIRECT)
	return (NOCLOBBER_REDIRECT);
    }
  else
    {
      do
	{
	  fd = open (filename, flags, mode);
	  e = errno;
	  if (fd < 0 && e == EINTR)
	    {
	      QUIT;
	      run_pending_traps ();
	    }
	  errno = e;
	}
      while (fd < 0 && errno == EINTR);

#if defined (AFS)
      if ((fd < 0) && (errno == EACCES))
	{
	  fd = open (filename, flags & ~O_CREAT, mode);
	  errno = EACCES;	/* restore errno */
	}
#endif /* AFS */
    }

  return fd;
}

static int
undoablefd (fd)
     int fd;
{
  int clexec;

  clexec = fcntl (fd, F_GETFD, 0);
  if (clexec == -1 || (fd >= SHELL_FD_BASE && clexec == 1))
    return 0;
  return 1;
}

/* Do the specific redirection requested.  Returns errno or one of the
   special redirection errors (*_REDIRECT) in case of error, 0 on success.
   If flags & RX_ACTIVE is zero, then just do whatever is necessary to
   produce the appropriate side effects.   flags & RX_UNDOABLE, if non-zero,
   says to remember how to undo each redirection.  If flags & RX_CLEXEC is
   non-zero, then we set all file descriptors > 2 that we open to be
   close-on-exec.  */
static int
do_redirection_internal (redirect, flags)
     REDIRECT *redirect;
     int flags;
{
  WORD_DESC *redirectee;
  int redir_fd, fd, redirector, r, oflags;
  intmax_t lfd;
  char *redirectee_word;
  enum r_instruction ri;
  REDIRECT *new_redirect;
  REDIRECTEE sd;

  redirectee = redirect->redirectee.filename;
  redir_fd = redirect->redirectee.dest;
  redirector = redirect->redirector.dest;
  ri = redirect->instruction;

  if (redirect->flags & RX_INTERNAL)
    flags |= RX_INTERNAL;

  if (TRANSLATE_REDIRECT (ri))
    {
      /* We have [N]>&WORD[-] or [N]<&WORD[-] (or {V}>&WORD[-] or {V}<&WORD-).
         and WORD, then translate the redirection into a new one and 
	 continue. */
      redirectee_word = redirection_expand (redirectee);

      /* XXX - what to do with [N]<&$w- where w is unset or null?  ksh93
	       closes N. */
      if (redirectee_word == 0)
	return (AMBIGUOUS_REDIRECT);
      else if (redirectee_word[0] == '-' && redirectee_word[1] == '\0')
	{
	  sd = redirect->redirector;
	  rd.dest = 0;
	  new_redirect = make_redirection (sd, r_close_this, rd, 0);
	}
      else if (all_digits (redirectee_word))
	{
	  sd = redirect->redirector;
	  if (legal_number (redirectee_word, &lfd) && (int)lfd == lfd)
	    rd.dest = lfd;
	  else
	    rd.dest = -1;	/* XXX */
	  switch (ri)
	    {
	    case r_duplicating_input_word:
	      new_redirect = make_redirection (sd, r_duplicating_input, rd, 0);
	      break;
	    case r_duplicating_output_word:
	      new_redirect = make_redirection (sd, r_duplicating_output, rd, 0);
	      break;
	    case r_move_input_word:
	      new_redirect = make_redirection (sd, r_move_input, rd, 0);
	      break;
	    case r_move_output_word:
	      new_redirect = make_redirection (sd, r_move_output, rd, 0);
	      break;
	    }
	}
      else if (ri == r_duplicating_output_word && (redirect->rflags & REDIR_VARASSIGN) == 0 && redirector == 1)
	{
	  sd = redirect->redirector;
	  rd.filename = make_bare_word (redirectee_word);
	  new_redirect = make_redirection (sd, r_err_and_out, rd, 0);
	}
      else
	{
	  free (redirectee_word);
	  return (AMBIGUOUS_REDIRECT);
	}

      free (redirectee_word);

      /* Set up the variables needed by the rest of the function from the
	 new redirection. */
      if (new_redirect->instruction == r_err_and_out)
	{
	  char *alloca_hack;

	  /* Copy the word without allocating any memory that must be
	     explicitly freed. */
	  redirectee = (WORD_DESC *)alloca (sizeof (WORD_DESC));
	  xbcopy ((char *)new_redirect->redirectee.filename,
		 (char *)redirectee, sizeof (WORD_DESC));

	  alloca_hack = (char *)
	    alloca (1 + strlen (new_redirect->redirectee.filename->word));
	  redirectee->word = alloca_hack;
	  strcpy (redirectee->word, new_redirect->redirectee.filename->word);
	}
      else
	/* It's guaranteed to be an integer, and shouldn't be freed. */
	redirectee = new_redirect->redirectee.filename;

      redir_fd = new_redirect->redirectee.dest;
      redirector = new_redirect->redirector.dest;
      ri = new_redirect->instruction;

      /* Overwrite the flags element of the old redirect with the new value. */
      redirect->flags = new_redirect->flags;
      dispose_redirects (new_redirect);
    }

  switch (ri)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:		/* command &>filename */
    case r_append_err_and_out:	/* command &>> filename */
    case r_input_output:
    case r_output_force:
      if (posixly_correct && interactive_shell == 0)
	{
	  oflags = redirectee->flags;
	  redirectee->flags |= W_NOGLOB;
	}
      redirectee_word = redirection_expand (redirectee);
      if (posixly_correct && interactive_shell == 0)
	redirectee->flags = oflags;

      if (redirectee_word == 0)
	return (AMBIGUOUS_REDIRECT);

#if defined (RESTRICTED_SHELL)
      if (restricted && (WRITE_REDIRECT (ri)))
	{
	  free (redirectee_word);
	  return (RESTRICTED_REDIRECT);
	}
#endif /* RESTRICTED_SHELL */

      fd = redir_open (redirectee_word, redirect->flags, 0666, ri);
      free (redirectee_word);

      if (fd == NOCLOBBER_REDIRECT)
	return (fd);

      if (fd < 0)
	return (errno);

      if (flags & RX_ACTIVE)
	{
	  if (redirect->rflags & REDIR_VARASSIGN)
	    {
	      redirector = fcntl (fd, F_DUPFD, SHELL_FD_BASE);		/* XXX try this for now */
	      r = errno;
	      if (redirector < 0)
		sys_error (_("redirection error: cannot duplicate fd"));
	      REDIRECTION_ERROR (redirector, r, fd);
	    }

	  if ((flags & RX_UNDOABLE) && (redirect->rflags & REDIR_VARASSIGN) == 0)
	    {
	      /* Only setup to undo it if the thing to undo is active. */
	      if ((fd != redirector) && (fcntl (redirector, F_GETFD, 0) != -1))
		r = add_undo_redirect (redirector, ri, -1);
	      else
		r = add_undo_close_redirect (redirector);
	      REDIRECTION_ERROR (r, errno, fd);
	    }

#if defined (BUFFERED_INPUT)
	  /* inhibit call to sync_buffered_stream() for async processes */
	  if (redirector != 0 || (subshell_environment & SUBSHELL_ASYNC) == 0)
	    check_bash_input (redirector);
#endif

	  /* Make sure there is no pending output before we change the state
	     of the underlying file descriptor, since the builtins use stdio
	     for output. */
	  if (redirector == 1 && fileno (stdout) == redirector)
	    {
	      fflush (stdout);
	      fpurge (stdout);
	    }
	  else if (redirector == 2 && fileno (stderr) == redirector)
	    {
	      fflush (stderr);
	      fpurge (stderr);
	    }

	  if (redirect->rflags & REDIR_VARASSIGN)
	    {
	      if ((r = redir_varassign (redirect, redirector)) < 0)
		{
		  close (redirector);
		  close (fd);
		  return (r);	/* XXX */
		}
	    }
	  else if ((fd != redirector) && (dup2 (fd, redirector) < 0))
	    {
	      close (fd);	/* dup2 failed? must be fd limit issue */
	      return (errno);
	    }

#if defined (BUFFERED_INPUT)
	  /* Do not change the buffered stream for an implicit redirection
	     of /dev/null to fd 0 for asynchronous commands without job
	     control (r_inputa_direction). */
	  if (ri == r_input_direction || ri == r_input_output)
	    duplicate_buffered_stream (fd, redirector);
#endif /* BUFFERED_INPUT */

	  /*
	   * If we're remembering, then this is the result of a while, for
	   * or until loop with a loop redirection, or a function/builtin
	   * executing in the parent shell with a redirection.  In the
	   * function/builtin case, we want to set all file descriptors > 2
	   * to be close-on-exec to duplicate the effect of the old
	   * for i = 3 to NOFILE close(i) loop.  In the case of the loops,
	   * both sh and ksh leave the file descriptors open across execs.
	   * The Posix standard mentions only the exec builtin.
	   */
	  if ((flags & RX_CLEXEC) && (redirector > 2))
	    SET_CLOSE_ON_EXEC (redirector);
	}

      if (fd != redirector)
	{
#if defined (BUFFERED_INPUT)
	  if (INPUT_REDIRECT (ri))
	    close_buffered_fd (fd);
	  else
#endif /* !BUFFERED_INPUT */
	    close (fd);		/* Don't close what we just opened! */
	}

      /* If we are hacking both stdout and stderr, do the stderr
	 redirection here.  XXX - handle {var} here? */
      if (ri == r_err_and_out || ri == r_append_err_and_out)
	{
	  if (flags & RX_ACTIVE)
	    {
	      if (flags & RX_UNDOABLE)
		add_undo_redirect (2, ri, -1);
	      if (dup2 (1, 2) < 0)
		return (errno);
	    }
	}
      break;

    case r_reading_until:
    case r_deblank_reading_until:
    case r_reading_string:
      /* REDIRECTEE is a pointer to a WORD_DESC containing the text of
	 the new input.  Place it in a temporary file. */
      if (redirectee)
	{
	  fd = here_document_to_fd (redirectee, ri);

	  if (fd < 0)
	    {
	      heredoc_errno = errno;
	      return (HEREDOC_REDIRECT);
	    }

	  if (redirect->rflags & REDIR_VARASSIGN)
	    {
	      redirector = fcntl (fd, F_DUPFD, SHELL_FD_BASE);		/* XXX try this for now */
	      r = errno;
	      if (redirector < 0)
		sys_error (_("redirection error: cannot duplicate fd"));
	      REDIRECTION_ERROR (redirector, r, fd);
	    }

	  if (flags & RX_ACTIVE)
	    {
	      if ((flags & RX_UNDOABLE) && (redirect->rflags & REDIR_VARASSIGN) == 0)
	        {
		  /* Only setup to undo it if the thing to undo is active. */
		  if ((fd != redirector) && (fcntl (redirector, F_GETFD, 0) != -1))
		    r = add_undo_redirect (redirector, ri, -1);
		  else
		    r = add_undo_close_redirect (redirector);
		  REDIRECTION_ERROR (r, errno, fd);
	        }

#if defined (BUFFERED_INPUT)
	      check_bash_input (redirector);
#endif
	      if (redirect->rflags & REDIR_VARASSIGN)
		{
		  if ((r = redir_varassign (redirect, redirector)) < 0)
		    {
		      close (redirector);
		      close (fd);
		      return (r);	/* XXX */
		    }
		}
	      else if (fd != redirector && dup2 (fd, redirector) < 0)
		{
		  r = errno;
		  close (fd);
		  return (r);
		}

#if defined (BUFFERED_INPUT)
	      duplicate_buffered_stream (fd, redirector);
#endif

	      if ((flags & RX_CLEXEC) && (redirector > 2))
		SET_CLOSE_ON_EXEC (redirector);
	    }

	  if (fd != redirector)
#if defined (BUFFERED_INPUT)
	    close_buffered_fd (fd);
#else
	    close (fd);
#endif
	}
      break;

    case r_duplicating_input:
    case r_duplicating_output:
    case r_move_input:
    case r_move_output:
      if ((flags & RX_ACTIVE) && (redirect->rflags & REDIR_VARASSIGN))
        {
	  redirector = fcntl (redir_fd, F_DUPFD, SHELL_FD_BASE);		/* XXX try this for now */
	  r = errno;
	  if (redirector < 0)
	    sys_error (_("redirection error: cannot duplicate fd"));
	  REDIRECTION_ERROR (redirector, r, -1);
        }

      if ((flags & RX_ACTIVE) && (redir_fd != redirector))
	{
	  if ((flags & RX_UNDOABLE) && (redirect->rflags & REDIR_VARASSIGN) == 0)
	    {
	      /* Only setup to undo it if the thing to undo is active. */
	      if (fcntl (redirector, F_GETFD, 0) != -1)
		r = add_undo_redirect (redirector, ri, redir_fd);
	      else
		r = add_undo_close_redirect (redirector);
	      REDIRECTION_ERROR (r, errno, -1);
	    }
	  if ((flags & RX_UNDOABLE) && (ri == r_move_input || ri == r_move_output))
	    {
	      /* r_move_input and r_move_output add an additional close()
		 that needs to be undone */
	      if (fcntl (redirector, F_GETFD, 0) != -1)
		{
		  r = add_undo_redirect (redir_fd, r_close_this, -1);
		  REDIRECTION_ERROR (r, errno, -1);
		}
	    }
#if defined (BUFFERED_INPUT)
	  /* inhibit call to sync_buffered_stream() for async processes */
	  if (redirector != 0 || (subshell_environment & SUBSHELL_ASYNC) == 0)
	    check_bash_input (redirector);
#endif
	  if (redirect->rflags & REDIR_VARASSIGN)
	    {
	      if ((r = redir_varassign (redirect, redirector)) < 0)
		{
		  close (redirector);
		  return (r);	/* XXX */
		}
	    }
	  /* This is correct.  2>&1 means dup2 (1, 2); */
	  else if (dup2 (redir_fd, redirector) < 0)
	    return (errno);

#if defined (BUFFERED_INPUT)
	  if (ri == r_duplicating_input || ri == r_move_input)
	    duplicate_buffered_stream (redir_fd, redirector);
#endif /* BUFFERED_INPUT */

	  /* First duplicate the close-on-exec state of redirectee.  dup2
	     leaves the flag unset on the new descriptor, which means it
	     stays open.  Only set the close-on-exec bit for file descriptors
	     greater than 2 in any case, since 0-2 should always be open
	     unless closed by something like `exec 2<&-'.  It should always
	     be safe to set fds > 2 to close-on-exec if they're being used to
	     save file descriptors < 2, since we don't need to preserve the
	     state of the close-on-exec flag for those fds -- they should
	     always be open. */
	  /* if ((already_set || set_unconditionally) && (ok_to_set))
		set_it () */
#if 0
	  if (((fcntl (redir_fd, F_GETFD, 0) == 1) || redir_fd < 2 || (flags & RX_CLEXEC)) &&
	       (redirector > 2))
#else
	  if (((fcntl (redir_fd, F_GETFD, 0) == 1) || (redir_fd < 2 && (flags & RX_INTERNAL)) || (flags & RX_CLEXEC)) &&
	       (redirector > 2))
#endif
	    SET_CLOSE_ON_EXEC (redirector);

	  /* When undoing saving of non-standard file descriptors (>=3) using
	     file descriptors >= SHELL_FD_BASE, we set the saving fd to be
	     close-on-exec and use a flag to decide how to set close-on-exec
	     when the fd is restored. */
	  if ((redirect->flags & RX_INTERNAL) && (redirect->flags & RX_SAVCLEXEC) && redirector >= 3 && (redir_fd >= SHELL_FD_BASE || (redirect->flags & RX_SAVEFD)))
	    SET_OPEN_ON_EXEC (redirector);
	    
	  /* dup-and-close redirection */
	  if (ri == r_move_input || ri == r_move_output)
	    {
	      xtrace_fdchk (redir_fd);

	      close (redir_fd);
#if defined (COPROCESS_SUPPORT)
	      coproc_fdchk (redir_fd);	/* XXX - loses coproc fds */
#endif
	    }
	}
      break;

    case r_close_this:
      if (flags & RX_ACTIVE)
	{
	  if (redirect->rflags & REDIR_VARASSIGN)
	    {
	      redirector = redir_varvalue (redirect);
	      if (redirector < 0)
		return AMBIGUOUS_REDIRECT;
	    }

	  r = 0;
	  /* XXX - only if REDIR_VARASSIGN not set? */
	  if ((flags & RX_UNDOABLE) && (fcntl (redirector, F_GETFD, 0) != -1))
	    {
	      r = add_undo_redirect (redirector, ri, -1);
	      REDIRECTION_ERROR (r, errno, redirector);
	    }

#if defined (COPROCESS_SUPPORT)
	  coproc_fdchk (redirector);
#endif
	  xtrace_fdchk (redirector);

#if defined (BUFFERED_INPUT)
	  /* inhibit call to sync_buffered_stream() for async processes */
	  if (redirector != 0 || (subshell_environment & SUBSHELL_ASYNC) == 0)
	    check_bash_input (redirector);
	  r = close_buffered_fd (redirector);
#else /* !BUFFERED_INPUT */
	  r = close (redirector);
#endif /* !BUFFERED_INPUT */

	  if (r < 0 && (flags & RX_INTERNAL) && (errno == EIO || errno == ENOSPC))
	    REDIRECTION_ERROR (r, errno, -1);
	}
      break;

    case r_duplicating_input_word:
    case r_duplicating_output_word:
      break;
    }
  return (0);
}

/* Remember the file descriptor associated with the slot FD,
   on REDIRECTION_UNDO_LIST.  Note that the list will be reversed
   before it is executed.  Any redirections that need to be undone
   even if REDIRECTION_UNDO_LIST is discarded by the exec builtin
   are also saved on EXEC_REDIRECTION_UNDO_LIST.  FDBASE says where to
   start the duplicating.  If it's less than SHELL_FD_BASE, we're ok,
   and can use SHELL_FD_BASE (-1 == don't care).  If it's >= SHELL_FD_BASE,
   we have to make sure we don't use fdbase to save a file descriptor,
   since we're going to use it later (e.g., make sure we don't save fd 0
   to fd 10 if we have a redirection like 0<&10).  If the value of fdbase
   puts the process over its fd limit, causing fcntl to fail, we try
   again with SHELL_FD_BASE.  Return 0 on success, -1 on error. */
static int
add_undo_redirect (fd, ri, fdbase)
     int fd;
     enum r_instruction ri;
     int fdbase;
{
  int new_fd, clexec_flag, savefd_flag;
  REDIRECT *new_redirect, *closer, *dummy_redirect;
  REDIRECTEE sd;

  savefd_flag = 0;
  new_fd = fcntl (fd, F_DUPFD, (fdbase < SHELL_FD_BASE) ? SHELL_FD_BASE : fdbase+1);
  if (new_fd < 0)
    new_fd = fcntl (fd, F_DUPFD, SHELL_FD_BASE);
  if (new_fd < 0)
    {
      new_fd = fcntl (fd, F_DUPFD, 0);
      savefd_flag = 1;
    }

  if (new_fd < 0)
    {
      sys_error (_("redirection error: cannot duplicate fd"));
      return (-1);
    }

  clexec_flag = fcntl (fd, F_GETFD, 0);

  sd.dest = new_fd;
  rd.dest = 0;
  closer = make_redirection (sd, r_close_this, rd, 0);
  closer->flags |= RX_INTERNAL;
  dummy_redirect = copy_redirects (closer);

  sd.dest = fd;
  rd.dest = new_fd;
  if (fd == 0)
    new_redirect = make_redirection (sd, r_duplicating_input, rd, 0);
  else
    new_redirect = make_redirection (sd, r_duplicating_output, rd, 0);
  new_redirect->flags |= RX_INTERNAL;
  if (savefd_flag)
    new_redirect->flags |= RX_SAVEFD;
  if (clexec_flag == 0 && fd >= 3 && (new_fd >= SHELL_FD_BASE || savefd_flag))
    new_redirect->flags |= RX_SAVCLEXEC;
  new_redirect->next = closer;

  closer->next = redirection_undo_list;
  redirection_undo_list = new_redirect;

  /* Save redirections that need to be undone even if the undo list
     is thrown away by the `exec' builtin. */
  add_exec_redirect (dummy_redirect);

  /* experimental:  if we're saving a redirection to undo for a file descriptor
     above SHELL_FD_BASE, add a redirection to be undone if the exec builtin
     causes redirections to be discarded.  There needs to be a difference
     between fds that are used to save other fds and then are the target of
     user redirections and fds that are just the target of user redirections.
     We use the close-on-exec flag to tell the difference; fds > SHELL_FD_BASE
     that have the close-on-exec flag set are assumed to be fds used internally
     to save others. */
  if (fd >= SHELL_FD_BASE && ri != r_close_this && clexec_flag)
    {
      sd.dest = fd;
      rd.dest = new_fd;
      new_redirect = make_redirection (sd, r_duplicating_output, rd, 0);
      new_redirect->flags |= RX_INTERNAL;

      add_exec_redirect (new_redirect);
    }

  /* File descriptors used only for saving others should always be
     marked close-on-exec.  Unfortunately, we have to preserve the
     close-on-exec state of the file descriptor we are saving, since
     fcntl (F_DUPFD) sets the new file descriptor to remain open
     across execs.  If, however, the file descriptor whose state we
     are saving is <= 2, we can just set the close-on-exec flag,
     because file descriptors 0-2 should always be open-on-exec,
     and the restore above in do_redirection() will take care of it. */
  if (clexec_flag || fd < 3)
    SET_CLOSE_ON_EXEC (new_fd);
  else if (redirection_undo_list->flags & RX_SAVCLEXEC)
    SET_CLOSE_ON_EXEC (new_fd);

  return (0);
}

/* Set up to close FD when we are finished with the current command
   and its redirections.  Return 0 on success, -1 on error. */
static int
add_undo_close_redirect (fd)
     int fd;
{
  REDIRECT *closer;
  REDIRECTEE sd;

  sd.dest = fd;
  rd.dest = 0;
  closer = make_redirection (sd, r_close_this, rd, 0);
  closer->flags |= RX_INTERNAL;
  closer->next = redirection_undo_list;
  redirection_undo_list = closer;

  return 0;
}

static void
add_exec_redirect (dummy_redirect)
     REDIRECT *dummy_redirect;
{
  dummy_redirect->next = exec_redirection_undo_list;
  exec_redirection_undo_list = dummy_redirect;
}

/* Return 1 if the redirection specified by RI and REDIRECTOR alters the
   standard input. */
static int
stdin_redirection (ri, redirector)
     enum r_instruction ri;
     int redirector;
{
  switch (ri)
    {
    case r_input_direction:
    case r_inputa_direction:
    case r_input_output:
    case r_reading_until:
    case r_deblank_reading_until:
    case r_reading_string:
      return (1);
    case r_duplicating_input:
    case r_duplicating_input_word:
    case r_close_this:
      return (redirector == 0);
    case r_output_direction:
    case r_appending_to:
    case r_duplicating_output:
    case r_err_and_out:
    case r_append_err_and_out:
    case r_output_force:
    case r_duplicating_output_word:
      return (0);
    }
  return (0);
}

/* Return non-zero if any of the redirections in REDIRS alter the standard
   input. */
int
stdin_redirects (redirs)
     REDIRECT *redirs;
{
  REDIRECT *rp;
  int n;

  for (n = 0, rp = redirs; rp; rp = rp->next)
    if ((rp->rflags & REDIR_VARASSIGN) == 0)
      n += stdin_redirection (rp->instruction, rp->redirector.dest);
  return n;
}
/* bind_var_to_int handles array references */
static int
redir_varassign (redir, fd)
     REDIRECT *redir;
     int fd;
{
  WORD_DESC *w;
  SHELL_VAR *v;

  w = redir->redirector.filename;
  v = bind_var_to_int (w->word, fd);
  if (v == 0 || readonly_p (v) || noassign_p (v))
    return BADVAR_REDIRECT;

  stupidly_hack_special_variables (w->word);
  return 0;
}

/* Handles {array[ind]} for redirection words */
static int
redir_varvalue (redir)
     REDIRECT *redir;
{
  SHELL_VAR *v;
  char *val, *w;
  intmax_t vmax;
  int i;
#if defined (ARRAY_VARS)
  char *sub;
  int len, vr;
#endif

  w = redir->redirector.filename->word;		/* shorthand */
  /* XXX - handle set -u here? */
#if defined (ARRAY_VARS)
  if (vr = valid_array_reference (w, 0))
    {
      v = array_variable_part (w, &sub, &len);
    }
  else
#endif
    {
      v = find_variable (w);
#if defined (ARRAY_VARS)
      if (v == 0)
	{
	  v = find_variable_last_nameref (w, 0);
	  if (v && nameref_p (v))
	    {
	      w = nameref_cell (v);
	      if (vr = valid_array_reference (w, 0))
		v = array_variable_part (w, &sub, &len);
	      else
	        v = find_variable (w);
	    }
	}
#endif
    }
	
  if (v == 0 || invisible_p (v))
    return -1;

#if defined (ARRAY_VARS)
  /* get_variable_value handles references to array variables without
     subscripts */
  if (vr && (array_p (v) || assoc_p (v)))
    val = get_array_value (w, 0, (int *)NULL, (arrayind_t *)0);
  else
#endif
  val = get_variable_value (v);
  if (val == 0 || *val == 0)
    return -1;

  if (legal_number (val, &vmax) < 0)
    return -1;

  i = vmax;	/* integer truncation */
  return i;
}
