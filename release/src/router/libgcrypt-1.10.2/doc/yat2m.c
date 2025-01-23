/* yat2m.c - Yet Another Texi 2 Man converter
 *	Copyright (C) 2005, 2013, 2015, 2016, 2017 g10 Code GmbH
 *      Copyright (C) 2006, 2008, 2011 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

/*
    This is a simple texinfo to man page converter.  It needs some
    special markup in th e texinfo and tries best to get a create man
    page.  It has been designed for the GnuPG man pages and thus only
    a few texinfo commands are supported.

    To use this you need to add the following macros into your texinfo
    source:

      @macro manpage {a}
      @end macro
      @macro mansect {a}
      @end macro
      @macro manpause
      @end macro
      @macro mancont
      @end macro

    They are used by yat2m to select parts of the Texinfo which should
    go into the man page. These macros need to be used without leading
    left space. Processing starts after a "manpage" macro has been
    seen.  "mansect" identifies the section and yat2m make sure to
    emit the sections in the proper order.  Note that @mansect skips
    the next input line if that line begins with @section, @subsection or
    @chapheading.

    To insert verbatim troff markup, the following texinfo code may be
    used:

      @ifset manverb
      .B whateever you want
      @end ifset

    alternativly a special comment may be used:

      @c man:.B whatever you want

    This is useful in case you need just one line. If you want to
    include parts only in the man page but keep the texinfo
    translation you may use:

      @ifset isman
      stuff to be rendered only on man pages
      @end ifset

    or to exclude stuff from man pages:

      @ifclear isman
      stuff not to be rendered on man pages
      @end ifclear

    the keyword @section is ignored, however @subsection gets rendered
    as ".SS".  @menu is completely skipped. Several man pages may be
    extracted from one file, either using the --store or the --select
    option.

    If you want to indent tables in the source use this style:

      @table foo
        @item
        @item
        @table
          @item
        @end
      @end

    Don't change the indentation within a table and keep the same
    number of white space at the start of the line.  yat2m simply
    detects the number of white spaces in front of an @item and remove
    this number of spaces from all following lines until a new @item
    is found or there are less spaces than for the last @item.

    Note that @* does only work correctly if used at the end of an
    input line.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>


#if __GNUC__
# define MY_GCC_VERSION (__GNUC__ * 10000 \
                         + __GNUC_MINOR__ * 100         \
                         + __GNUC_PATCHLEVEL__)
#else
# define MY_GCC_VERSION 0
#endif

#if MY_GCC_VERSION >= 20500
# define ATTR_PRINTF(f, a) __attribute__ ((format(printf,f,a)))
# define ATTR_NR_PRINTF(f, a) __attribute__ ((noreturn, format(printf,f,a)))
#else
# define ATTR_PRINTF(f, a)
# define ATTR_NR_PRINTF(f, a)
#endif
#if MY_GCC_VERSION >= 30200
# define ATTR_MALLOC  __attribute__ ((__malloc__))
#else
# define ATTR_MALLOC
#endif



#define PGM "yat2m"
#ifdef PACKAGE_VERSION
# define VERSION PACKAGE_VERSION
#else
# define VERSION "1.0"
#endif

/* The maximum length of a line including the linefeed and one extra
   character. */
#define LINESIZE 1024

/* Number of allowed condition nestings.  */
#define MAX_CONDITION_NESTING  10

/* Option flags. */
static int verbose;
static int quiet;
static int debug;
static const char *opt_source;
static const char *opt_release;
static const char *opt_date;
static const char *opt_select;
static const char *opt_include;
static int opt_store;

/* Flag to keep track whether any error occurred.  */
static int any_error;


/* Object to keep macro definitions.  */
struct macro_s
{
  struct macro_s *next;
  char *value;    /* Malloced value. */
  char name[1];
};
typedef struct macro_s *macro_t;

/* List of all defined macros. */
static macro_t macrolist;

/* List of variables set by @set. */
static macro_t variablelist;

/* List of global macro names.  The value part is not used.  */
static macro_t predefinedmacrolist;

/* Object to keep track of @isset and @ifclear.  */
struct condition_s
{
  int manverb;   /* "manverb" needs special treatment.  */
  int isset;     /* This is an @isset condition.  */
  char name[1];  /* Name of the condition macro.  */
};
typedef struct condition_s *condition_t;

/* The stack used to evaluate conditions.  And the current states. */
static condition_t condition_stack[MAX_CONDITION_NESTING];
static int condition_stack_idx;
static int cond_is_active;     /* State of ifset/ifclear */
static int cond_in_verbatim;   /* State of "manverb".  */


/* Object to store one line of content.  */
struct line_buffer_s
{
  struct line_buffer_s *next;
  int verbatim;  /* True if LINE contains verbatim data.  The default
                    is Texinfo source.  */
  char *line;
};
typedef struct line_buffer_s *line_buffer_t;


/* Object to collect the data of a section.  */
struct section_buffer_s
{
  char *name;           /* Malloced name of the section. This may be
                           NULL to indicate this slot is not used.  */
  line_buffer_t lines;  /* Linked list with the lines of the section.  */
  line_buffer_t *lines_tail; /* Helper for faster appending to the
                                linked list.  */
  line_buffer_t last_line;   /* Points to the last line appended.  */
};
typedef struct section_buffer_s *section_buffer_t;

/* Variable to keep info about the current page together.  */
static struct
{
  /* Filename of the current page or NULL if no page is active.  Malloced. */
  char *name;

  /* Number of allocated elements in SECTIONS below.  */
  size_t n_sections;
  /* Array with the data of the sections.  */
  section_buffer_t sections;

} thepage;


/* The list of standard section names.  COMMANDS and ASSUAN are GnuPG
   specific. */
static const char * const standard_sections[] =
  { "NAME",  "SYNOPSIS",  "DESCRIPTION",
    "RETURN VALUE", "EXIT STATUS", "ERROR HANDLING", "ERRORS",
    "COMMANDS", "OPTIONS", "USAGE", "EXAMPLES", "FILES",
    "ENVIRONMENT", "DIAGNOSTICS", "SECURITY", "CONFORMING TO",
    "ASSUAN", "NOTES", "BUGS", "AUTHOR", "SEE ALSO", NULL };


/*-- Local prototypes.  --*/
static void proc_texi_buffer (FILE *fp, const char *line, size_t len,
                              int *table_level, int *eol_action);

static void die (const char *format, ...) ATTR_NR_PRINTF(1,2);
static void err (const char *format, ...) ATTR_PRINTF(1,2);
static void inf (const char *format, ...) ATTR_PRINTF(1,2);
static void *xmalloc (size_t n) ATTR_MALLOC;
static void *xcalloc (size_t n, size_t m) ATTR_MALLOC;



/*-- Functions --*/

/* Print diagnostic message and exit with failure. */
static void
die (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  fprintf (stderr, "%s: ", PGM);

  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  putc ('\n', stderr);

  exit (1);
}


/* Print diagnostic message. */
static void
err (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  if (strncmp (format, "%s:%d:", 6))
    fprintf (stderr, "%s: ", PGM);

  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  putc ('\n', stderr);
  any_error = 1;
}

/* Print diagnostic message. */
static void
inf (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  fprintf (stderr, "%s: ", PGM);

  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  putc ('\n', stderr);
}


static void *
xmalloc (size_t n)
{
  void *p = malloc (n);
  if (!p)
    die ("out of core: %s", strerror (errno));
  return p;
}

static void *
xcalloc (size_t n, size_t m)
{
  void *p = calloc (n, m);
  if (!p)
    die ("out of core: %s", strerror (errno));
  return p;
}

static void *
xrealloc (void *old, size_t n)
{
  void *p = realloc (old, n);
  if (!p)
    die ("out of core: %s", strerror (errno));
  return p;
}

static char *
xstrdup (const char *string)
{
  void *p = malloc (strlen (string)+1);
  if (!p)
    die ("out of core: %s", strerror (errno));
  strcpy (p, string);
  return p;
}


/* Uppercase the ascii characters in STRING.  */
static char *
ascii_strupr (char *string)
{
  char *p;

  for (p = string; *p; p++)
    if (!(*p & 0x80))
      *p = toupper (*p);
  return string;
}


/* Return the current date as an ISO string.  */
const char *
isodatestring (void)
{
  static char buffer[36];
  struct tm *tp;
  time_t atime;

  if (opt_date && *opt_date)
    atime = strtoul (opt_date, NULL, 10);
  else
    atime = time (NULL);
  if (atime < 0)
    strcpy (buffer, "????" "-??" "-??");
  else
    {
      tp = gmtime (&atime);
      sprintf (buffer,"%04d-%02d-%02d",
               1900+tp->tm_year, tp->tm_mon+1, tp->tm_mday );
    }
  return buffer;
}


/* Add NAME to the list of predefined macros which are global for all
   files.  */
static void
add_predefined_macro (const char *name)
{
  macro_t m;

  for (m=predefinedmacrolist; m; m = m->next)
    if (!strcmp (m->name, name))
      break;
  if (!m)
    {
      m = xcalloc (1, sizeof *m + strlen (name));
      strcpy (m->name, name);
      m->next = predefinedmacrolist;
      predefinedmacrolist = m;
    }
}


/* Create or update a macro with name MACRONAME and set its values TO
   MACROVALUE.  Note that ownership of the macro value is transferred
   to this function.  */
static void
set_macro (const char *macroname, char *macrovalue)
{
  macro_t m;

  for (m=macrolist; m; m = m->next)
    if (!strcmp (m->name, macroname))
      break;
  if (m)
    free (m->value);
  else
    {
      m = xcalloc (1, sizeof *m + strlen (macroname));
      strcpy (m->name, macroname);
      m->next = macrolist;
      macrolist = m;
    }
  m->value = macrovalue;
  macrovalue = NULL;
}


/* Create or update a variable with name and value given in NAMEANDVALUE.  */
static void
set_variable (char *nameandvalue)
{
  macro_t m;
  const char *value;
  char *p;

  for (p = nameandvalue; *p && *p != ' ' && *p != '\t'; p++)
    ;
  if (!*p)
    value = "";
  else
    {
      *p++ = 0;
      while (*p == ' ' || *p == '\t')
        p++;
      value = p;
    }

  for (m=variablelist; m; m = m->next)
    if (!strcmp (m->name, nameandvalue))
      break;
  if (m)
    free (m->value);
  else
    {
      m = xcalloc (1, sizeof *m + strlen (nameandvalue));
      strcpy (m->name, nameandvalue);
      m->next = variablelist;
      variablelist = m;
    }
  m->value = xstrdup (value);
}


/* Return true if the macro or variable NAME is set, i.e. not the
   empty string and not evaluating to 0.  */
static int
macro_set_p (const char *name)
{
  macro_t m;

  for (m = macrolist; m ; m = m->next)
    if (!strcmp (m->name, name))
      break;
  if (!m)
    for (m = variablelist; m ; m = m->next)
      if (!strcmp (m->name, name))
        break;
  if (!m || !m->value || !*m->value)
    return 0;
  if ((*m->value & 0x80) || !isdigit (*m->value))
    return 1; /* Not a digit but some other string.  */
  return !!atoi (m->value);
}


/* Evaluate the current conditions.  */
static void
evaluate_conditions (const char *fname, int lnr)
{
  int i;

  (void)fname;
  (void)lnr;

  /* for (i=0; i < condition_stack_idx; i++) */
  /*   inf ("%s:%d:   stack[%d] %s %s %c", */
  /*        fname, lnr, i, condition_stack[i]->isset? "set":"clr", */
  /*        condition_stack[i]->name, */
  /*        (macro_set_p (condition_stack[i]->name) */
  /*         ^ !condition_stack[i]->isset)? 't':'f'); */

  cond_is_active = 1;
  cond_in_verbatim = 0;
  if (condition_stack_idx)
    {
      for (i=0; i < condition_stack_idx; i++)
        {
          if (condition_stack[i]->manverb)
            cond_in_verbatim = (macro_set_p (condition_stack[i]->name)
                                ^ !condition_stack[i]->isset);
          else if (!(macro_set_p (condition_stack[i]->name)
                     ^ !condition_stack[i]->isset))
            {
              cond_is_active = 0;
              break;
            }
        }
    }

  /* inf ("%s:%d:   active=%d verbatim=%d", */
  /*      fname, lnr, cond_is_active, cond_in_verbatim); */
}


/* Push a condition with condition macro NAME onto the stack.  If
   ISSET is true, a @isset condition is pushed.  */
static void
push_condition (const char *name, int isset, const char *fname, int lnr)
{
  condition_t cond;
  int manverb = 0;

  if (condition_stack_idx >= MAX_CONDITION_NESTING)
    {
      err ("%s:%d: condition nested too deep", fname, lnr);
      return;
    }

  if (!strcmp (name, "manverb"))
    {
      if (!isset)
        {
          err ("%s:%d: using \"@ifclear manverb\" is not allowed", fname, lnr);
          return;
        }
      manverb = 1;
    }

  cond = xcalloc (1, sizeof *cond + strlen (name));
  cond->manverb = manverb;
  cond->isset = isset;
  strcpy (cond->name, name);

  condition_stack[condition_stack_idx++] = cond;
  evaluate_conditions (fname, lnr);
}


/* Remove the last condition from the stack.  ISSET is used for error
   reporting.  */
static void
pop_condition (int isset, const char *fname, int lnr)
{
  if (!condition_stack_idx)
    {
      err ("%s:%d: unbalanced \"@end %s\"",
           fname, lnr, isset?"isset":"isclear");
      return;
    }
  condition_stack_idx--;
  free (condition_stack[condition_stack_idx]);
  condition_stack[condition_stack_idx] = NULL;
  evaluate_conditions (fname, lnr);
}



/* Return a section buffer for the section NAME.  Allocate a new buffer
   if this is a new section.  Keep track of the sections in THEPAGE.
   This function may reallocate the section array in THEPAGE.  */
static section_buffer_t
get_section_buffer (const char *name)
{
  int i;
  section_buffer_t sect;

  /* If there is no section we put everything into the required NAME
     section.  Given that this is the first one listed it is likely
     that error are easily visible.  */
  if (!name)
    name = "NAME";

  for (i=0; i < thepage.n_sections; i++)
    {
      sect = thepage.sections + i;
      if (sect->name && !strcmp (name, sect->name))
        return sect;
    }
  for (i=0; i < thepage.n_sections; i++)
    if (!thepage.sections[i].name)
      break;
  if (thepage.n_sections && i < thepage.n_sections)
    sect = thepage.sections + i;
  else
    {
      /* We need to allocate or reallocate the section array.  */
      size_t old_n = thepage.n_sections;
      size_t new_n = 20;

      if (!old_n)
        thepage.sections = xcalloc (new_n, sizeof *thepage.sections);
      else
        {
          thepage.sections = xrealloc (thepage.sections,
                                       ((old_n + new_n)
                                        * sizeof *thepage.sections));
          memset (thepage.sections + old_n, 0,
                  new_n * sizeof *thepage.sections);
        }
      thepage.n_sections += new_n;

      /* Setup the tail pointers.  */
      for (i=old_n; i < thepage.n_sections; i++)
        {
          sect = thepage.sections + i;
          sect->lines_tail = &sect->lines;
        }
      sect = thepage.sections + old_n;
    }

  /* Store the name.  */
  assert (!sect->name);
  sect->name = xstrdup (name);
  return sect;
}



/* Add the content of LINE to the section named SECTNAME.  */
static void
add_content (const char *sectname, char *line, int verbatim)
{
  section_buffer_t sect;
  line_buffer_t lb;

  sect = get_section_buffer (sectname);
  if (sect->last_line && !sect->last_line->verbatim == !verbatim)
    {
      /* Lets append that line to the last one.  We do this to keep
         all lines of the same kind (i.e.verbatim or not) together in
         one large buffer.  */
      size_t n1, n;

      lb = sect->last_line;
      n1 = strlen (lb->line);
      n = n1 + 1 + strlen (line) + 1;
      lb->line = xrealloc (lb->line, n);
      strcpy (lb->line+n1, "\n");
      strcpy (lb->line+n1+1, line);
    }
  else
    {
      lb = xcalloc (1, sizeof *lb);
      lb->verbatim = verbatim;
      lb->line = xstrdup (line);
      sect->last_line = lb;
      *sect->lines_tail = lb;
      sect->lines_tail = &lb->next;
    }
}


/* Prepare for a new man page using the filename NAME. */
static void
start_page (char *name)
{
  if (verbose)
    inf ("starting page '%s'", name);
  assert (!thepage.name);
  thepage.name = xstrdup (name);
  thepage.n_sections = 0;
}


/* Write the .TH entry of the current page.  Return -1 if there is a
   problem with the page. */
static int
write_th (FILE *fp)
{
  char *name, *p;

  fputs (".\\\" Created from Texinfo source by yat2m " VERSION "\n", fp);

  name = ascii_strupr (xstrdup (thepage.name));
  p = strrchr (name, '.');
  if (!p || !p[1])
    {
      err ("no section name in man page '%s'", thepage.name);
      free (name);
      return -1;
    }
  *p++ = 0;
  fprintf (fp, ".TH %s %s %s \"%s\" \"%s\"\n",
           name, p, isodatestring (), opt_release, opt_source);
  free (name);
  return 0;
}


/* Process the texinfo command COMMAND (without the leading @) and
   write output if needed to FP. REST is the remainer of the line
   which should either point to an opening brace or to a white space.
   The function returns the number of characters already processed
   from REST.  LEN is the usable length of REST.  TABLE_LEVEL is used to
   control the indentation of tables.  */
static size_t
proc_texi_cmd (FILE *fp, const char *command, const char *rest, size_t len,
               int *table_level, int *eol_action)
{
  static struct {
    const char *name;    /* Name of the command.  */
    int what;            /* What to do with this command. */
    const char *lead_in; /* String to print with a opening brace.  */
    const char *lead_out;/* String to print with the closing brace. */
  } cmdtbl[] = {
    { "command", 0, "\\fB", "\\fR" },
    { "code",    0, "\\fB", "\\fR" },
    { "url",     0, "\\fB", "\\fR" },
    { "sc",      0, "\\fB", "\\fR" },
    { "var",     0, "\\fI", "\\fR" },
    { "samp",    0, "\\(aq", "\\(aq"  },
    { "file",    0, "\\(oq\\fI","\\fR\\(cq" },
    { "env",     0, "\\(oq\\fI","\\fR\\(cq" },
    { "acronym", 0 },
    { "dfn",     0 },
    { "option",  0, "\\fB", "\\fR"   },
    { "example", 1, ".RS 2\n.nf\n" },
    { "smallexample", 1, ".RS 2\n.nf\n" },
    { "asis",    7 },
    { "anchor",  7 },
    { "cartouche", 1 },
    { "ref",     0, "[", "]" },
    { "xref",    0, "See: [", "]" },
    { "pxref",   0, "see: [", "]" },
    { "uref",    0, "(\\fB", "\\fR)" },
    { "footnote",0, " ([", "])" },
    { "emph",    0, "\\fI", "\\fR" },
    { "w",       1 },
    { "c",       5 },
    { "efindex", 1 },
    { "opindex", 1 },
    { "cpindex", 1 },
    { "cindex",  1 },
    { "noindent", 0 },
    { "section", 1 },
    { "chapter", 1 },
    { "subsection", 6, "\n.SS " },
    { "chapheading", 0},
    { "item",    2, ".TP\n.B " },
    { "itemx",   2, ".TQ\n.B " },
    { "table",   3 },
    { "itemize",   3 },
    { "bullet",  0, "* " },
    { "*",       0, "\n.br"},
    { "/",       0 },
    { "end",     4 },
    { "quotation",1, ".RS\n\\fB" },
    { "value", 8 },
    { NULL }
  };
  size_t n;
  int i;
  const char *s;
  const char *lead_out = NULL;
  int ignore_args = 0;

  for (i=0; cmdtbl[i].name && strcmp (cmdtbl[i].name, command); i++)
    ;
  if (cmdtbl[i].name)
    {
      s = cmdtbl[i].lead_in;
      if (s)
        fputs (s, fp);
      lead_out = cmdtbl[i].lead_out;
      switch (cmdtbl[i].what)
        {
        case 1: /* Throw away the entire line.  */
          s = memchr (rest, '\n', len);
          return s? (s-rest)+1 : len;
        case 2: /* Handle @item.  */
          break;
        case 3: /* Handle table.  */
          if (++(*table_level) > 1)
            fputs (".RS\n", fp);
          /* Now throw away the entire line. */
          s = memchr (rest, '\n', len);
          return s? (s-rest)+1 : len;
          break;
        case 4: /* Handle end.  */
          for (s=rest, n=len; n && (*s == ' ' || *s == '\t'); s++, n--)
            ;
          if (n >= 5 && !memcmp (s, "table", 5)
              && (!n || s[5] == ' ' || s[5] == '\t' || s[5] == '\n'))
            {
              if ((*table_level)-- > 1)
                fputs (".RE\n", fp);
              else
                fputs (".P\n", fp);
            }
          else if (n >= 7 && !memcmp (s, "example", 7)
              && (!n || s[7] == ' ' || s[7] == '\t' || s[7] == '\n'))
            {
              fputs (".fi\n.RE\n", fp);
            }
          else if (n >= 12 && !memcmp (s, "smallexample", 12)
              && (!n || s[12] == ' ' || s[12] == '\t' || s[12] == '\n'))
            {
              fputs (".fi\n.RE\n", fp);
            }
          else if (n >= 9 && !memcmp (s, "quotation", 9)
              && (!n || s[9] == ' ' || s[9] == '\t' || s[9] == '\n'))
            {
              fputs ("\\fR\n.RE\n", fp);
            }
          /* Now throw away the entire line. */
          s = memchr (rest, '\n', len);
          return s? (s-rest)+1 : len;
        case 5: /* Handle special comments. */
          for (s=rest, n=len; n && (*s == ' ' || *s == '\t'); s++, n--)
            ;
          if (n >= 4 && !memcmp (s, "man:", 4))
            {
              for (s+=4, n-=4; n && *s != '\n'; n--, s++)
                putc (*s, fp);
              putc ('\n', fp);
            }
          /* Now throw away the entire line. */
          s = memchr (rest, '\n', len);
          return s? (s-rest)+1 : len;
        case 6:
          *eol_action = 1;
          break;
        case 7:
          ignore_args = 1;
          break;
        case 8:
          ignore_args = 1;
          if (*rest != '{')
            {
              err ("opening brace for command '%s' missing", command);
              return len;
            }
          else
            {
              /* Find closing brace.  */
              for (s=rest+1, n=1; *s && n < len; s++, n++)
                if (*s == '}')
                  break;
              if (*s != '}')
                {
                  err ("closing brace for command '%s' not found", command);
                  return len;
                }
              else
                {
                  size_t rlen = s - (rest + 1);
                  macro_t m;

                  for (m = variablelist; m; m = m->next)
                    {
                      if (strlen (m->name) == rlen
                          && !strncmp (m->name, rest+1, rlen))
                        break;
                    }
                  if (m)
                    fputs (m->value, fp);
                  else
                    inf ("texinfo variable '%.*s' is not set",
                         (int)rlen, rest+1);
                }
            }
          break;
        default:
          break;
        }
    }
  else /* macro */
    {
      macro_t m;

      for (m = macrolist; m ; m = m->next)
        if (!strcmp (m->name, command))
            break;
      if (m)
        {
          proc_texi_buffer (fp, m->value, strlen (m->value),
                            table_level, eol_action);
          ignore_args = 1; /* Parameterized macros are not yet supported. */
        }
      else
        inf ("texinfo command '%s' not supported (%.*s)", command,
             (int)((s = memchr (rest, '\n', len)), (s? (s-rest) : len)), rest);
    }

  if (*rest == '{')
    {
      /* Find matching closing brace.  */
      for (s=rest+1, n=1, i=1; i && *s && n < len; s++, n++)
        if (*s == '{')
          i++;
        else if (*s == '}')
          i--;
      if (i)
        {
          err ("closing brace for command '%s' not found", command);
          return len;
        }
      if (n > 2 && !ignore_args)
        proc_texi_buffer (fp, rest+1, n-2, table_level, eol_action);
    }
  else
    n = 0;

  if (lead_out)
    fputs (lead_out, fp);

  return n;
}



/* Process the string LINE with LEN bytes of Texinfo content. */
static void
proc_texi_buffer (FILE *fp, const char *line, size_t len,
                  int *table_level, int *eol_action)
{
  const char *s;
  char cmdbuf[256];
  int cmdidx = 0;
  int in_cmd = 0;
  size_t n;

  for (s=line; *s && len; s++, len--)
    {
      if (in_cmd)
        {
          if (in_cmd == 1)
            {
              switch (*s)
                {
                case '@': case '{': case '}':
                  putc (*s, fp); in_cmd = 0;
                  break;
                case ':': /* Not ending a sentence flag.  */
                  in_cmd = 0;
                  break;
                case '.': case '!': case '?': /* Ending a sentence. */
                  putc (*s, fp); in_cmd = 0;
                  break;
                case ' ': case '\t': case '\n': /* Non collapsing spaces.  */
                  putc (*s, fp); in_cmd = 0;
                  break;
                default:
                  cmdidx = 0;
                  cmdbuf[cmdidx++] = *s;
                  in_cmd++;
                  break;
                }
            }
          else if (*s == '{' || *s == ' ' || *s == '\t' || *s == '\n')
            {
              cmdbuf[cmdidx] = 0;
              n = proc_texi_cmd (fp, cmdbuf, s, len, table_level, eol_action);
              assert (n <= len);
              s += n; len -= n;
              s--; len++;
              in_cmd = 0;
            }
          else if (cmdidx < sizeof cmdbuf -1)
            cmdbuf[cmdidx++] = *s;
          else
            {
              err ("texinfo command too long - ignored");
              in_cmd = 0;
            }
        }
      else if (*s == '@')
        in_cmd = 1;
      else if (*s == '\n')
        {
          switch (*eol_action)
            {
            case 1: /* Create a dummy paragraph. */
              fputs ("\n\\ \n", fp);
              break;
            default:
              putc (*s, fp);
            }
          *eol_action = 0;
        }
      else if (*s == '\\')
        fputs ("\\\\", fp);
      else
        putc (*s, fp);
    }

  if (in_cmd > 1)
    {
      cmdbuf[cmdidx] = 0;
      n = proc_texi_cmd (fp, cmdbuf, s, len, table_level, eol_action);
      assert (n <= len);
      s += n; len -= n;
      s--; len++;
      /* in_cmd = 0; -- doc only */
    }
}


/* Do something with the Texinfo line LINE.  */
static void
parse_texi_line (FILE *fp, const char *line, int *table_level)
{
  int eol_action = 0;

  /* A quick test whether there are any texinfo commands.  */
  if (!strchr (line, '@'))
    {
      fputs (line, fp);
      putc ('\n', fp);
      return;
    }
  proc_texi_buffer (fp, line, strlen (line), table_level, &eol_action);
  putc ('\n', fp);
}


/* Write all the lines LINES to FP.  */
static void
write_content (FILE *fp, line_buffer_t lines)
{
  line_buffer_t line;
  int table_level = 0;

  for (line = lines; line; line = line->next)
    {
      if (line->verbatim)
        {
          fputs (line->line, fp);
          putc ('\n', fp);
        }
      else
        {
/*           fputs ("TEXI---", fp); */
/*           fputs (line->line, fp); */
/*           fputs ("---\n", fp); */
          parse_texi_line (fp, line->line, &table_level);
        }
    }
}



static int
is_standard_section (const char *name)
{
  int i;
  const char *s;

  for (i=0; (s=standard_sections[i]); i++)
    if (!strcmp (s, name))
      return 1;
  return 0;
}


/* Finish a page; that is sort the data and write it out to the file.  */
static void
finish_page (void)
{
  FILE *fp;
  section_buffer_t sect = NULL;
  int idx;
  const char *s;
  int i;

  if (!thepage.name)
    return; /* No page active.  */

  if (verbose)
    inf ("finishing page '%s'", thepage.name);

  if (opt_select)
    {
      if (!strcmp (opt_select, thepage.name))
        {
          inf ("selected '%s'", thepage.name );
          fp = stdout;
        }
      else
        {
          fp = fopen ( "/dev/null", "w" );
          if (!fp)
            die ("failed to open /dev/null: %s\n", strerror (errno));
        }
    }
  else if (opt_store)
    {
      inf ("writing '%s'", thepage.name );
      fp = fopen ( thepage.name, "w" );
      if (!fp)
        die ("failed to create '%s': %s\n", thepage.name, strerror (errno));
    }
  else
    fp = stdout;

  if (write_th (fp))
    goto leave;

  for (idx=0; (s=standard_sections[idx]); idx++)
    {
      for (i=0; i < thepage.n_sections; i++)
        {
          sect = thepage.sections + i;
          if (sect->name && !strcmp (s, sect->name))
            break;
        }
      if (i == thepage.n_sections)
        sect = NULL;

      if (sect)
        {
          fprintf (fp, ".SH %s\n", sect->name);
          write_content (fp, sect->lines);
          /* Now continue with all non standard sections directly
             following this one. */
          for (i++; i < thepage.n_sections; i++)
            {
              sect = thepage.sections + i;
              if (sect->name && is_standard_section (sect->name))
                break;
              if (sect->name)
                {
                  fprintf (fp, ".SH %s\n", sect->name);
                  write_content (fp, sect->lines);
                }
            }

        }
    }


 leave:
  if (fp != stdout)
    fclose (fp);
  free (thepage.name);
  thepage.name = NULL;
  /* FIXME: Cleanup the content.  */
}




/* Parse one Texinfo file and create manpages according to the
   embedded instructions.  */
static void
parse_file (const char *fname, FILE *fp, char **section_name, int in_pause)
{
  char *line;
  int lnr = 0;
  /* Fixme: The following state variables don't carry over to include
     files. */
  int skip_to_end = 0;        /* Used to skip over menu entries. */
  int skip_sect_line = 0;     /* Skip after @mansect.  */
  int item_indent = 0;        /* How far is the current @item indented.  */

  /* Helper to define a macro. */
  char *macroname = NULL;
  char *macrovalue = NULL;
  size_t macrovaluesize = 0;
  size_t macrovalueused = 0;

  line = xmalloc (LINESIZE);
  while (fgets (line, LINESIZE, fp))
    {
      size_t n = strlen (line);
      int got_line = 0;
      char *p, *pend;

      lnr++;
      if (!n || line[n-1] != '\n')
        {
          err ("%s:%d: trailing linefeed missing, line too long or "
               "embedded Nul character", fname, lnr);
          break;
        }
      line[--n] = 0;

      /* Kludge to allow indentation of tables.  */
      for (p=line; *p == ' ' || *p == '\t'; p++)
        ;
      if (*p)
        {
          if (*p == '@' && !strncmp (p+1, "item", 4))
            item_indent = p - line;  /* Set a new indent level.  */
          else if (p - line < item_indent)
            item_indent = 0;         /* Switch off indention.  */

          if (item_indent)
            {
              memmove (line, line+item_indent, n - item_indent + 1);
              n -= item_indent;
            }
        }


      if (*line == '@')
        {
          for (p=line+1, n=1; *p && *p != ' ' && *p != '\t'; p++)
            n++;
          while (*p == ' ' || *p == '\t')
            p++;
        }
      else
        p = line;

      /* Take action on macro.  */
      if (macroname)
        {
          if (n == 4 && !memcmp (line, "@end", 4)
              && (line[4]==' '||line[4]=='\t'||!line[4])
              && !strncmp (p, "macro", 5)
              && (p[5]==' '||p[5]=='\t'||!p[5]))
            {
              if (macrovalueused)
                macrovalue[--macrovalueused] = 0; /* Kill the last LF. */
              macrovalue[macrovalueused] = 0;     /* Terminate macro. */
              macrovalue = xrealloc (macrovalue, macrovalueused+1);

              set_macro (macroname, macrovalue);
              macrovalue = NULL;
              free (macroname);
              macroname = NULL;
            }
          else
            {
              if (macrovalueused + strlen (line) + 2 >= macrovaluesize)
                {
                  macrovaluesize += strlen (line) + 256;
                  macrovalue = xrealloc (macrovalue,  macrovaluesize);
                }
              strcpy (macrovalue+macrovalueused, line);
              macrovalueused += strlen (line);
              macrovalue[macrovalueused++] = '\n';
            }
          continue;
        }


      if (n >= 5 && !memcmp (line, "@node", 5)
          && (line[5]==' '||line[5]=='\t'||!line[5]))
        {
          /* Completey ignore @node lines.  */
          continue;
        }


      if (skip_sect_line)
        {
          skip_sect_line = 0;
          if (!strncmp (line, "@section", 8)
              || !strncmp (line, "@subsection", 11)
              || !strncmp (line, "@chapheading", 12))
            continue;
        }

      /* We only parse lines we need and ignore the rest.  There are a
         few macros used to control this as well as one @ifset
         command.  Parts we know about are saved away into containers
         separate for each section. */

      /* First process ifset/ifclear commands. */
      if (*line == '@')
        {
          if (n == 6 && !memcmp (line, "@ifset", 6)
                   && (line[6]==' '||line[6]=='\t'))
            {
              for (p=line+7; *p == ' ' || *p == '\t'; p++)
                ;
              if (!*p)
                {
                  err ("%s:%d: name missing after \"@ifset\"", fname, lnr);
                  continue;
                }
              for (pend=p; *pend && *pend != ' ' && *pend != '\t'; pend++)
                ;
              *pend = 0;  /* Ignore rest of the line.  */
              push_condition (p, 1, fname, lnr);
              continue;
            }
          else if (n == 8 && !memcmp (line, "@ifclear", 8)
                   && (line[8]==' '||line[8]=='\t'))
            {
              for (p=line+9; *p == ' ' || *p == '\t'; p++)
                ;
              if (!*p)
                {
                  err ("%s:%d: name missing after \"@ifsclear\"", fname, lnr);
                  continue;
                }
              for (pend=p; *pend && *pend != ' ' && *pend != '\t'; pend++)
                ;
              *pend = 0;  /* Ignore rest of the line.  */
              push_condition (p, 0, fname, lnr);
              continue;
            }
          else if (n == 4 && !memcmp (line, "@end", 4)
                   && (line[4]==' '||line[4]=='\t')
                   && !strncmp (p, "ifset", 5)
                   && (p[5]==' '||p[5]=='\t'||!p[5]))
            {
              pop_condition (1, fname, lnr);
              continue;
            }
          else if (n == 4 && !memcmp (line, "@end", 4)
                   && (line[4]==' '||line[4]=='\t')
                   && !strncmp (p, "ifclear", 7)
                   && (p[7]==' '||p[7]=='\t'||!p[7]))
            {
              pop_condition (0, fname, lnr);
              continue;
            }
        }

      /* Take action on ifset/ifclear.  */
      if (!cond_is_active)
        continue;

      /* Process commands. */
      if (*line == '@')
        {
          if (skip_to_end
              && n == 4 && !memcmp (line, "@end", 4)
              && (line[4]==' '||line[4]=='\t'||!line[4]))
            {
              skip_to_end = 0;
            }
          else if (cond_in_verbatim)
            {
                got_line = 1;
            }
          else if (n == 6 && !memcmp (line, "@macro", 6))
            {
              macroname = xstrdup (p);
              macrovalue = xmalloc ((macrovaluesize = 1024));
              macrovalueused = 0;
            }
          else if (n == 4 && !memcmp (line, "@set", 4))
            {
              set_variable (p);
            }
          else if (n == 8 && !memcmp (line, "@manpage", 8))
            {
              free (*section_name);
              *section_name = NULL;
              finish_page ();
              start_page (p);
              in_pause = 0;
            }
          else if (n == 8 && !memcmp (line, "@mansect", 8))
            {
              if (!thepage.name)
                err ("%s:%d: section outside of a man page", fname, lnr);
              else
                {
                  free (*section_name);
                  *section_name = ascii_strupr (xstrdup (p));
                  in_pause = 0;
                  skip_sect_line = 1;
                }
            }
          else if (n == 9 && !memcmp (line, "@manpause", 9))
            {
              if (!*section_name)
                err ("%s:%d: pausing outside of a man section", fname, lnr);
              else if (in_pause)
                err ("%s:%d: already pausing", fname, lnr);
              else
                in_pause = 1;
            }
          else if (n == 8 && !memcmp (line, "@mancont", 8))
            {
              if (!*section_name)
                err ("%s:%d: continue outside of a man section", fname, lnr);
              else if (!in_pause)
                err ("%s:%d: continue while not pausing", fname, lnr);
              else
                in_pause = 0;
            }
          else if (n == 5 && !memcmp (line, "@menu", 5)
                   && (line[5]==' '||line[5]=='\t'||!line[5]))
            {
              skip_to_end = 1;
            }
          else if (n == 8 && !memcmp (line, "@include", 8)
                   && (line[8]==' '||line[8]=='\t'||!line[8]))
            {
              char *incname = xstrdup (p);
              FILE *incfp = fopen (incname, "r");

              if (!incfp && opt_include && *opt_include && *p != '/')
                {
                  free (incname);
                  incname = xmalloc (strlen (opt_include) + 1
                                     + strlen (p) + 1);
                  strcpy (incname, opt_include);
                  if ( incname[strlen (incname)-1] != '/' )
                    strcat (incname, "/");
                  strcat (incname, p);
                  incfp = fopen (incname, "r");
                }

              if (!incfp)
                err ("can't open include file '%s': %s",
                     incname, strerror (errno));
              else
                {
                  parse_file (incname, incfp, section_name, in_pause);
                  fclose (incfp);
                }
              free (incname);
            }
          else if (n == 4 && !memcmp (line, "@bye", 4)
                   && (line[4]==' '||line[4]=='\t'||!line[4]))
            {
              break;
            }
          else if (!skip_to_end)
            got_line = 1;
        }
      else if (!skip_to_end)
        got_line = 1;

      if (got_line && cond_in_verbatim)
        add_content (*section_name, line, 1);
      else if (got_line && thepage.name && *section_name && !in_pause)
        add_content (*section_name, line, 0);

    }
  if (ferror (fp))
    err ("%s:%d: read error: %s", fname, lnr, strerror (errno));
  free (macroname);
  free (macrovalue);
  free (line);
}


static void
top_parse_file (const char *fname, FILE *fp)
{
  char *section_name = NULL;  /* Name of the current section or NULL
                                 if not in a section.  */
  macro_t m;

  while (macrolist)
    {
      macro_t next = macrolist->next;
      free (macrolist->value);
      free (macrolist);
      macrolist = next;
    }
  while (variablelist)
    {
      macro_t next = variablelist->next;
      free (variablelist->value);
      free (variablelist);
      variablelist = next;
    }
  for (m=predefinedmacrolist; m; m = m->next)
    set_macro (m->name, xstrdup ("1"));
  cond_is_active = 1;
  cond_in_verbatim = 0;

  parse_file (fname, fp, &section_name, 0);
  free (section_name);
  finish_page ();
}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  const char *s;

  opt_source = "GNU";
  opt_release = "";

  /* Define default macros.  The trick is that these macros are not
     defined when using the actual texinfo renderer. */
  add_predefined_macro ("isman");
  add_predefined_macro ("manverb");

  /* Option parsing.  */
  if (argc)
    {
      argc--; argv++;
    }
  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--help"))
        {
          puts (
                "Usage: " PGM " [OPTION] [FILE]\n"
                "Extract man pages from a Texinfo source.\n\n"
                "  --source NAME    use NAME as source field\n"
                "  --release STRING use STRING as the release field\n"
                "  --date EPOCH     use EPOCH as publication date\n"
                "  --store          write output using @manpage name\n"
                "  --select NAME    only output pages with @manpage NAME\n"
                "  --verbose        enable extra informational output\n"
                "  --debug          enable additional debug output\n"
                "  --help           display this help and exit\n"
                "  -I DIR           also search in include DIR\n"
                "  -D gpgone        the only usable define\n\n"
                "With no FILE, or when FILE is -, read standard input.\n\n"
                "Report bugs to <https://bugs.gnupg.org>.");
          exit (0);
        }
      else if (!strcmp (*argv, "--version"))
        {
          puts (PGM " " VERSION "\n"
               "Copyright (C) 2005, 2017 g10 Code GmbH\n"
               "This program comes with ABSOLUTELY NO WARRANTY.\n"
               "This is free software, and you are welcome to redistribute it\n"
                "under certain conditions. See the file COPYING for details.");
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--quiet"))
        {
          quiet = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = debug = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--source"))
        {
          argc--; argv++;
          if (argc)
            {
              opt_source = *argv;
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--release"))
        {
          argc--; argv++;
          if (argc)
            {
              opt_release = *argv;
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--date"))
        {
          argc--; argv++;
          if (argc)
            {
              opt_date = *argv;
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--store"))
        {
          opt_store = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--select"))
        {
          argc--; argv++;
          if (argc)
            {
              opt_select = strrchr (*argv, '/');
              if (opt_select)
                opt_select++;
              else
                opt_select = *argv;
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "-I"))
        {
          argc--; argv++;
          if (argc)
            {
              opt_include = *argv;
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "-D"))
        {
          argc--; argv++;
          if (argc)
            {
              add_predefined_macro (*argv);
              argc--; argv++;
            }
        }
    }

  if (argc > 1)
    die ("usage: " PGM " [OPTION] [FILE] (try --help for more information)\n");

  /* Take care of supplied timestamp for reproducible builds.  See
   * https://reproducible-builds.org/specs/source-date-epoch/  */
  if (!opt_date && (s = getenv ("SOURCE_DATE_EPOCH")) && *s)
    opt_date = s;

  /* Start processing. */
  if (argc && strcmp (*argv, "-"))
    {
      FILE *fp = fopen (*argv, "rb");
      if (!fp)
        die ("%s:0: can't open file: %s", *argv, strerror (errno));
      top_parse_file (*argv, fp);
      fclose (fp);
    }
  else
    top_parse_file ("-", stdin);

  return !!any_error;
}


/*
Local Variables:
compile-command: "gcc -Wall -g -Wall -o yat2m yat2m.c"
End:
*/
