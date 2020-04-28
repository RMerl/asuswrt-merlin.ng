/* arrayfunc.c -- High-level array functions used by other parts of the shell. */

/* Copyright (C) 2001-2016 Free Software Foundation, Inc.

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

#if defined (ARRAY_VARS)

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include <stdio.h>

#include "bashintl.h"

#include "shell.h"
#include "pathexp.h"

#include "shmbutil.h"
#if defined (HAVE_MBSTR_H) && defined (HAVE_MBSCHR)
#  include <mbstr.h>		/* mbschr */
#endif

#include "builtins/common.h"

extern char *this_command_name;
extern int last_command_exit_value;
extern int array_needs_making;

static SHELL_VAR *bind_array_var_internal __P((SHELL_VAR *, arrayind_t, char *, char *, int));
static SHELL_VAR *assign_array_element_internal __P((SHELL_VAR *, char *, char *, char *, int, char *, int));

static char *quote_assign __P((const char *));
static void quote_array_assignment_chars __P((WORD_LIST *));
static char *array_value_internal __P((const char *, int, int, int *, arrayind_t *));

/* Standard error message to use when encountering an invalid array subscript */
const char * const bash_badsub_errmsg = N_("bad array subscript");

/* **************************************************************** */
/*								    */
/*  Functions to manipulate array variables and perform assignments */
/*								    */
/* **************************************************************** */

/* Convert a shell variable to an array variable.  The original value is
   saved as array[0]. */
SHELL_VAR *
convert_var_to_array (var)
     SHELL_VAR *var;
{
  char *oldval;
  ARRAY *array;

  oldval = value_cell (var);
  array = array_create ();
  if (oldval)
    array_insert (array, 0, oldval);

  FREE (value_cell (var));
  var_setarray (var, array);

  /* these aren't valid anymore */
  var->dynamic_value = (sh_var_value_func_t *)NULL;
  var->assign_func = (sh_var_assign_func_t *)NULL;

  INVALIDATE_EXPORTSTR (var);
  if (exported_p (var))
    array_needs_making++;

  VSETATTR (var, att_array);
  VUNSETATTR (var, att_invisible);

  /* Make sure it's not marked as an associative array any more */
  VUNSETATTR (var, att_assoc);

  /* Since namerefs can't be array variables, turn off nameref attribute */
  VUNSETATTR (var, att_nameref);

  return var;
}

/* Convert a shell variable to an array variable.  The original value is
   saved as array[0]. */
SHELL_VAR *
convert_var_to_assoc (var)
     SHELL_VAR *var;
{
  char *oldval;
  HASH_TABLE *hash;

  oldval = value_cell (var);
  hash = assoc_create (0);
  if (oldval)
    assoc_insert (hash, savestring ("0"), oldval);

  FREE (value_cell (var));
  var_setassoc (var, hash);

  /* these aren't valid anymore */
  var->dynamic_value = (sh_var_value_func_t *)NULL;
  var->assign_func = (sh_var_assign_func_t *)NULL;

  INVALIDATE_EXPORTSTR (var);
  if (exported_p (var))
    array_needs_making++;

  VSETATTR (var, att_assoc);
  VUNSETATTR (var, att_invisible);

  /* Make sure it's not marked as an indexed array any more */
  VUNSETATTR (var, att_array);

  /* Since namerefs can't be array variables, turn off nameref attribute */
  VUNSETATTR (var, att_nameref);

  return var;
}

char *
make_array_variable_value (entry, ind, key, value, flags)
     SHELL_VAR *entry;
     arrayind_t ind;
     char *key;
     char *value;
     int flags;
{
  SHELL_VAR *dentry;
  char *newval;

  /* If we're appending, we need the old value of the array reference, so
     fake out make_variable_value with a dummy SHELL_VAR */
  if (flags & ASS_APPEND)
    {
      dentry = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));
      dentry->name = savestring (entry->name);
      if (assoc_p (entry))
	newval = assoc_reference (assoc_cell (entry), key);
      else
	newval = array_reference (array_cell (entry), ind);
      if (newval)
	dentry->value = savestring (newval);
      else
	{
	  dentry->value = (char *)xmalloc (1);
	  dentry->value[0] = '\0';
	}
      dentry->exportstr = 0;
      dentry->attributes = entry->attributes & ~(att_array|att_assoc|att_exported);
      /* Leave the rest of the members uninitialized; the code doesn't look
	 at them. */
      newval = make_variable_value (dentry, value, flags);	 
      dispose_variable (dentry);
    }
  else
    newval = make_variable_value (entry, value, flags);

  return newval;
}
  
static SHELL_VAR *
bind_array_var_internal (entry, ind, key, value, flags)
     SHELL_VAR *entry;
     arrayind_t ind;
     char *key;
     char *value;
     int flags;
{
  char *newval;

  newval = make_array_variable_value (entry, ind, key, value, flags);

  if (entry->assign_func)
    (*entry->assign_func) (entry, newval, ind, key);
  else if (assoc_p (entry))
    assoc_insert (assoc_cell (entry), key, newval);
  else
    array_insert (array_cell (entry), ind, newval);
  FREE (newval);

  VUNSETATTR (entry, att_invisible);	/* no longer invisible */
  return (entry);
}

/* Perform an array assignment name[ind]=value.  If NAME already exists and
   is not an array, and IND is 0, perform name=value instead.  If NAME exists
   and is not an array, and IND is not 0, convert it into an array with the
   existing value as name[0].

   If NAME does not exist, just create an array variable, no matter what
   IND's value may be. */
SHELL_VAR *
bind_array_variable (name, ind, value, flags)
     char *name;
     arrayind_t ind;
     char *value;
     int flags;
{
  SHELL_VAR *entry;

  entry = find_shell_variable (name);

  if (entry == (SHELL_VAR *) 0)
    {
      /* Is NAME a nameref variable that points to an unset variable? */
      entry = find_variable_nameref_for_create (name, 0);
      if (entry == INVALID_NAMEREF_VALUE)
	return ((SHELL_VAR *)0);
      if (entry && nameref_p (entry))
	entry = make_new_array_variable (nameref_cell (entry));
    }
  if (entry == (SHELL_VAR *) 0)
    entry = make_new_array_variable (name);
  else if ((readonly_p (entry) && (flags&ASS_FORCE) == 0) || noassign_p (entry))
    {
      if (readonly_p (entry))
	err_readonly (name);
      return (entry);
    }
  else if (array_p (entry) == 0)
    entry = convert_var_to_array (entry);

  /* ENTRY is an array variable, and ARRAY points to the value. */
  return (bind_array_var_internal (entry, ind, 0, value, flags));
}

SHELL_VAR *
bind_array_element (entry, ind, value, flags)
     SHELL_VAR *entry;
     arrayind_t ind;
     char *value;
     int flags;
{
  return (bind_array_var_internal (entry, ind, 0, value, flags));
}
                    
SHELL_VAR *
bind_assoc_variable (entry, name, key, value, flags)
     SHELL_VAR *entry;
     char *name;
     char *key;
     char *value;
     int flags;
{
  SHELL_VAR *dentry;
  char *newval;

  if ((readonly_p (entry) && (flags&ASS_FORCE) == 0) || noassign_p (entry))
    {
      if (readonly_p (entry))
	err_readonly (name);
      return (entry);
    }

  return (bind_array_var_internal (entry, 0, key, value, flags));
}

/* Parse NAME, a lhs of an assignment statement of the form v[s], and
   assign VALUE to that array element by calling bind_array_variable(). */
SHELL_VAR *
assign_array_element (name, value, flags)
     char *name, *value;
     int flags;
{
  char *sub, *vname;
  int sublen;
  SHELL_VAR *entry, *nv;

  vname = array_variable_name (name, &sub, &sublen);

  if (vname == 0)
    return ((SHELL_VAR *)NULL);

  if ((ALL_ELEMENT_SUB (sub[0]) && sub[1] == ']') || (sublen <= 1))
    {
      free (vname);
      err_badarraysub (name);
      return ((SHELL_VAR *)NULL);
    }

  entry = find_variable (vname);
  entry = assign_array_element_internal (entry, name, vname, sub, sublen, value, flags);

  free (vname);
  return entry;
}

static SHELL_VAR *
assign_array_element_internal (entry, name, vname, sub, sublen, value, flags)
     SHELL_VAR *entry;
     char *name;		/* only used for error messages */
     char *vname;
     char *sub;
     int sublen;
     char *value;
     int flags;
{
  char *akey;
  arrayind_t ind;

  if (entry && assoc_p (entry))
    {
      sub[sublen-1] = '\0';
      akey = expand_assignment_string_to_string (sub, 0);	/* [ */
      sub[sublen-1] = ']';
      if (akey == 0 || *akey == 0)
	{
	  err_badarraysub (name);
	  FREE (akey);
	  return ((SHELL_VAR *)NULL);
	}
      entry = bind_assoc_variable (entry, vname, akey, value, flags);
    }
  else
    {
      ind = array_expand_index (entry, sub, sublen);
      /* negative subscripts to indexed arrays count back from end */
      if (entry && ind < 0)
	ind = (array_p (entry) ? array_max_index (array_cell (entry)) : 0) + 1 + ind;
      if (ind < 0)
	{
	  err_badarraysub (name);
	  return ((SHELL_VAR *)NULL);
	}
      entry = bind_array_variable (vname, ind, value, flags);
    }

  return (entry);
}

/* Find the array variable corresponding to NAME.  If there is no variable,
   create a new array variable.  If the variable exists but is not an array,
   convert it to an indexed array.  If FLAGS&1 is non-zero, an existing
   variable is checked for the readonly or noassign attribute in preparation
   for assignment (e.g., by the `read' builtin).  If FLAGS&2 is non-zero, we
   create an associative array. */
SHELL_VAR *
find_or_make_array_variable (name, flags)
     char *name;
     int flags;
{
  SHELL_VAR *var;

  var = find_variable (name);
  if (var == 0)
    {
      /* See if we have a nameref pointing to a variable that hasn't been
	 created yet. */
      var = find_variable_last_nameref (name, 1);
      if (var && nameref_p (var) && invisible_p (var))
	{
	  internal_warning (_("%s: removing nameref attribute"), name);
	  VUNSETATTR (var, att_nameref);
	}
      if (var && nameref_p (var))
	{
	  if (valid_nameref_value (nameref_cell (var), 2) == 0)
	    {
	      sh_invalidid (nameref_cell (var));
	      return ((SHELL_VAR *)NULL);
	    }
	  var = (flags & 2) ? make_new_assoc_variable (nameref_cell (var)) : make_new_array_variable (nameref_cell (var));
	}
    }

  if (var == 0)
    var = (flags & 2) ? make_new_assoc_variable (name) : make_new_array_variable (name);
  else if ((flags & 1) && (readonly_p (var) || noassign_p (var)))
    {
      if (readonly_p (var))
	err_readonly (name);
      return ((SHELL_VAR *)NULL);
    }
  else if ((flags & 2) && array_p (var))
    {
      last_command_exit_value = 1;
      report_error (_("%s: cannot convert indexed to associative array"), name);
      return ((SHELL_VAR *)NULL);
    }
  else if (array_p (var) == 0 && assoc_p (var) == 0)
    var = convert_var_to_array (var);

  return (var);
}
  
/* Perform a compound assignment statement for array NAME, where VALUE is
   the text between the parens:  NAME=( VALUE ) */
SHELL_VAR *
assign_array_from_string (name, value, flags)
     char *name, *value;
     int flags;
{
  SHELL_VAR *var;
  int vflags;

  vflags = 1;
  if (flags & ASS_MKASSOC)
    vflags |= 2;

  var = find_or_make_array_variable (name, vflags);
  if (var == 0)
    return ((SHELL_VAR *)NULL);

  return (assign_array_var_from_string (var, value, flags));
}

/* Sequentially assign the indices of indexed array variable VAR from the
   words in LIST. */
SHELL_VAR *
assign_array_var_from_word_list (var, list, flags)
     SHELL_VAR *var;
     WORD_LIST *list;
     int flags;
{
  register arrayind_t i;
  register WORD_LIST *l;
  ARRAY *a;

  a = array_cell (var);
  i = (flags & ASS_APPEND) ? array_max_index (a) + 1 : 0;

  for (l = list; l; l = l->next, i++)
    bind_array_var_internal (var, i, 0, l->word->word, flags & ~ASS_APPEND);

  VUNSETATTR (var, att_invisible);	/* no longer invisible */

  return var;
}

WORD_LIST *
expand_compound_array_assignment (var, value, flags)
     SHELL_VAR *var;
     char *value;
     int flags;
{
  WORD_LIST *list, *nlist;
  WORD_LIST *hd, *tl, *t, *n;
  char *val;
  int ni;

  /* This condition is true when invoked from the declare builtin with a
     command like
	declare -a d='([1]="" [2]="bdef" [5]="hello world" "test")' */
  if (*value == '(')	/*)*/
    {
      ni = 1;
      val = extract_array_assignment_list (value, &ni);
      if (val == 0)
	return (WORD_LIST *)NULL;
    }
  else
    val = value;

  /* Expand the value string into a list of words, performing all the
     shell expansions including pathname generation and word splitting. */
  /* First we split the string on whitespace, using the shell parser
     (ksh93 seems to do this). */
  list = parse_string_to_word_list (val, 1, "array assign");

  if (var && assoc_p (var))
    {
      if (val != value)
	free (val);
      return list;
    }

  /* If we're using [subscript]=value, we need to quote each [ and ] to
     prevent unwanted filename expansion.  This doesn't need to be done
     for associative array expansion, since that uses a different expansion
     function (see assign_compound_array_list below). */
  if (list)
    quote_array_assignment_chars (list);

  /* Now that we've split it, perform the shell expansions on each
     word in the list. */
  nlist = list ? expand_words_no_vars (list) : (WORD_LIST *)NULL;

  dispose_words (list);

  if (val != value)
    free (val);

  return nlist;
}

/* Callers ensure that VAR is not NULL */
void
assign_compound_array_list (var, nlist, flags)
     SHELL_VAR *var;
     WORD_LIST *nlist;
     int flags;
{
  ARRAY *a;
  HASH_TABLE *h;
  WORD_LIST *list;
  char *w, *val, *nval, *savecmd;
  int len, iflags, free_val;
  arrayind_t ind, last_ind;
  char *akey;

  a = (var && array_p (var)) ? array_cell (var) : (ARRAY *)0;
  h = (var && assoc_p (var)) ? assoc_cell (var) : (HASH_TABLE *)0;

  akey = (char *)0;
  ind = 0;

  /* Now that we are ready to assign values to the array, kill the existing
     value. */
  if ((flags & ASS_APPEND) == 0)
    {
      if (a && array_p (var))
	array_flush (a);
      else if (h && assoc_p (var))
	assoc_flush (h);
    }

  last_ind = (a && (flags & ASS_APPEND)) ? array_max_index (a) + 1 : 0;

  for (list = nlist; list; list = list->next)
    {
      /* Don't allow var+=(values) to make assignments in VALUES append to
	 existing values by default. */
      iflags = flags & ~ASS_APPEND;
      w = list->word->word;

      /* We have a word of the form [ind]=value */
      if ((list->word->flags & W_ASSIGNMENT) && w[0] == '[')
	{
	  /* Don't have to handle embedded quotes specially any more, since
	     associative array subscripts have not been expanded yet (see
	     above). */
	  len = skipsubscript (w, 0, 0);

	  /* XXX - changes for `+=' */
 	  if (w[len] != ']' || (w[len+1] != '=' && (w[len+1] != '+' || w[len+2] != '=')))
	    {
	      if (assoc_p (var))
		{
		  err_badarraysub (w);
		  continue;
		}
	      nval = make_variable_value (var, w, flags);
	      if (var->assign_func)
		(*var->assign_func) (var, nval, last_ind, 0);
	      else
		array_insert (a, last_ind, nval);
	      FREE (nval);
	      last_ind++;
	      continue;
	    }

	  if (len == 1)
	    {
	      err_badarraysub (w);
	      continue;
	    }

	  if (ALL_ELEMENT_SUB (w[1]) && len == 2)
	    {
	      last_command_exit_value = 1;
	      if (assoc_p (var))
		report_error (_("%s: invalid associative array key"), w);
	      else
		report_error (_("%s: cannot assign to non-numeric index"), w);
	      continue;
	    }

	  if (array_p (var))
	    {
	      ind = array_expand_index (var, w + 1, len);
	      /* negative subscripts to indexed arrays count back from end */
	      if (ind < 0)
		ind = array_max_index (array_cell (var)) + 1 + ind;
	      if (ind < 0)
		{
		  err_badarraysub (w);
		  continue;
		}

	      last_ind = ind;
	    }
	  else if (assoc_p (var))
	    {
	      /* This is not performed above, see expand_compound_array_assignment */
	      w[len] = '\0';	/*[*/
	      akey = expand_assignment_string_to_string (w+1, 0);
	      w[len] = ']';
	      /* And we need to expand the value also, see below */
	      if (akey == 0 || *akey == 0)
		{
		  err_badarraysub (w);
		  FREE (akey);
		  continue;
		}
	    }

	  /* XXX - changes for `+=' -- just accept the syntax.  ksh93 doesn't do this */
	  if (w[len + 1] == '+' && w[len + 2] == '=')
	    {
	      iflags |= ASS_APPEND;
	      val = w + len + 3;
	    }
	  else
	    val = w + len + 2;	    
	}
      else if (assoc_p (var))
	{
	  last_command_exit_value = 1;
	  report_error (_("%s: %s: must use subscript when assigning associative array"), var->name, w);
	  continue;
	}
      else		/* No [ind]=value, just a stray `=' */
	{
	  ind = last_ind;
	  val = w;
	}

      free_val = 0;
      /* See above; we need to expand the value here */
      if (assoc_p (var))
	{
	  val = expand_assignment_string_to_string (val, 0);
	  if (val == 0)
	    {
	      val = (char *)xmalloc (1);
	      val[0] = '\0';	/* like do_assignment_internal */
	    }
	  free_val = 1;
	}

      savecmd = this_command_name;
      if (integer_p (var))
	this_command_name = (char *)NULL;	/* no command name for errors */
      bind_array_var_internal (var, ind, akey, val, iflags);
      last_ind++;
      this_command_name = savecmd;

      if (free_val)
	free (val);
    }
}

/* Perform a compound array assignment:  VAR->name=( VALUE ).  The
   VALUE has already had the parentheses stripped. */
SHELL_VAR *
assign_array_var_from_string (var, value, flags)
     SHELL_VAR *var;
     char *value;
     int flags;
{
  WORD_LIST *nlist;

  if (value == 0)
    return var;

  nlist = expand_compound_array_assignment (var, value, flags);
  assign_compound_array_list (var, nlist, flags);

  if (nlist)
    dispose_words (nlist);

  if (var)
    VUNSETATTR (var, att_invisible);	/* no longer invisible */

  return (var);
}

/* Quote globbing chars and characters in $IFS before the `=' in an assignment
   statement (usually a compound array assignment) to protect them from
   unwanted filename expansion or word splitting. */
static char *
quote_assign (string)
     const char *string;
{
  size_t slen;
  int saw_eq;
  char *temp, *t, *subs;
  const char *s, *send;
  int ss, se;
  DECLARE_MBSTATE;

  slen = strlen (string);
  send = string + slen;

  t = temp = (char *)xmalloc (slen * 2 + 1);
  saw_eq = 0;
  for (s = string; *s; )
    {
      if (*s == '=')
	saw_eq = 1;
      if (saw_eq == 0 && *s == '[')		/* looks like a subscript */
	{
	  ss = s - string;
	  se = skipsubscript (string, ss, 0);
	  subs = substring (s, ss, se);
	  *t++ = '\\';
	  strcpy (t, subs);
	  t += se - ss;
	  *t++ = '\\';
	  *t++ = ']';
	  s += se + 1;
	  free (subs);
	  continue;
	}
      if (saw_eq == 0 && (glob_char_p (s) || isifs (*s)))
	*t++ = '\\';

      COPY_CHAR_P (t, s, send);
    }
  *t = '\0';
  return temp;
}

/* For each word in a compound array assignment, if the word looks like
   [ind]=value, quote globbing chars and characters in $IFS before the `='. */
static void
quote_array_assignment_chars (list)
     WORD_LIST *list;
{
  char *nword;
  WORD_LIST *l;

  for (l = list; l; l = l->next)
    {
      if (l->word == 0 || l->word->word == 0 || l->word->word[0] == '\0')
	continue;	/* should not happen, but just in case... */
      /* Don't bother if it hasn't been recognized as an assignment or
	 doesn't look like [ind]=value */
      if ((l->word->flags & W_ASSIGNMENT) == 0)
	continue;
      if (l->word->word[0] != '[' || mbschr (l->word->word, '=') == 0) /* ] */
	continue;

      nword = quote_assign (l->word->word);
      free (l->word->word);
      l->word->word = nword;
      l->word->flags |= W_NOGLOB;	/* XXX - W_NOSPLIT also? */
    }
}

/* skipsubscript moved to subst.c to use private functions. 2009/02/24. */

/* This function is called with SUB pointing to just after the beginning
   `[' of an array subscript and removes the array element to which SUB
   expands from array VAR.  A subscript of `*' or `@' unsets the array. */
int
unbind_array_element (var, sub)
     SHELL_VAR *var;
     char *sub;
{
  int len;
  arrayind_t ind;
  char *akey;
  ARRAY_ELEMENT *ae;

  len = skipsubscript (sub, 0, (var && assoc_p(var)));
  if (sub[len] != ']' || len == 0)
    {
      builtin_error ("%s[%s: %s", var->name, sub, _(bash_badsub_errmsg));
      return -1;
    }
  sub[len] = '\0';

  if (ALL_ELEMENT_SUB (sub[0]) && sub[1] == 0)
    {
      if (array_p (var) || assoc_p (var))
	{
	  unbind_variable (var->name);	/* XXX -- {array,assoc}_flush ? */
	  return (0);
	}
      else
	return -2;	/* don't allow this to unset scalar variables */
    }

  if (assoc_p (var))
    {
      akey = expand_assignment_string_to_string (sub, 0);     /* [ */
      if (akey == 0 || *akey == 0)
	{
	  builtin_error ("[%s]: %s", sub, _(bash_badsub_errmsg));
	  FREE (akey);
	  return -1;
	}
      assoc_remove (assoc_cell (var), akey);
      free (akey);
    }
  else if (array_p (var))
    {
      ind = array_expand_index (var, sub, len+1);
      /* negative subscripts to indexed arrays count back from end */
      if (ind < 0)
	ind = array_max_index (array_cell (var)) + 1 + ind;
      if (ind < 0)
	{
	  builtin_error ("[%s]: %s", sub, _(bash_badsub_errmsg));
	  return -1;
	}
      ae = array_remove (array_cell (var), ind);
      if (ae)
	array_dispose_element (ae);
    }
  else	/* array_p (var) == 0 && assoc_p (var) == 0 */
    {
      akey = this_command_name;
      ind = array_expand_index (var, sub, len+1);
      this_command_name = akey;
      if (ind == 0)
	{
	  unbind_variable (var->name);
	  return (0);
	}
      else
	return -2;	/* any subscript other than 0 is invalid with scalar variables */
    }

  return 0;
}

/* Format and output an array assignment in compound form VAR=(VALUES),
   suitable for re-use as input. */
void
print_array_assignment (var, quoted)
     SHELL_VAR *var;
     int quoted;
{
  char *vstr;

  vstr = array_to_assign (array_cell (var), quoted);

  if (vstr == 0)
    printf ("%s=%s\n", var->name, quoted ? "'()'" : "()");
  else
    {
      printf ("%s=%s\n", var->name, vstr);
      free (vstr);
    }
}

/* Format and output an associative array assignment in compound form
   VAR=(VALUES), suitable for re-use as input. */
void
print_assoc_assignment (var, quoted)
     SHELL_VAR *var;
     int quoted;
{
  char *vstr;

  vstr = assoc_to_assign (assoc_cell (var), quoted);

  if (vstr == 0)
    printf ("%s=%s\n", var->name, quoted ? "'()'" : "()");
  else
    {
      printf ("%s=%s\n", var->name, vstr);
      free (vstr);
    }
}

/***********************************************************************/
/*								       */
/* Utility functions to manage arrays and their contents for expansion */
/*								       */
/***********************************************************************/

/* Return 1 if NAME is a properly-formed array reference v[sub]. */
int
valid_array_reference (name, flags)
     const char *name;
     int flags;
{
  char *t;
  int r, len;

  t = mbschr (name, '[');	/* ] */
  if (t)
    {
      *t = '\0';
      r = legal_identifier (name);
      *t = '[';
      if (r == 0)
	return 0;
      /* Check for a properly-terminated non-blank subscript. */
      len = skipsubscript (t, 0, 0);
      if (t[len] != ']' || len == 1)
	return 0;
      if (t[len+1] != '\0')
	return 0;
      for (r = 1; r < len; r++)
	if (whitespace (t[r]) == 0)
	  return 1;
      return 0;
    }
  return 0;
}

/* Expand the array index beginning at S and extending LEN characters. */
arrayind_t
array_expand_index (var, s, len)
     SHELL_VAR *var;
     char *s;
     int len;
{
  char *exp, *t, *savecmd;
  int expok;
  arrayind_t val;

  exp = (char *)xmalloc (len);
  strncpy (exp, s, len - 1);
  exp[len - 1] = '\0';
  t = expand_arith_string (exp, Q_DOUBLE_QUOTES|Q_ARITH|Q_ARRAYSUB);	/* XXX - Q_ARRAYSUB for future use */
  savecmd = this_command_name;
  this_command_name = (char *)NULL;
  val = evalexp (t, &expok);
  this_command_name = savecmd;
  free (t);
  free (exp);
  if (expok == 0)
    {
      last_command_exit_value = EXECUTION_FAILURE;

      if (no_longjmp_on_fatal_error)
	return 0;
      top_level_cleanup ();      
      jump_to_top_level (DISCARD);
    }
  return val;
}

/* Return the name of the variable specified by S without any subscript.
   If SUBP is non-null, return a pointer to the start of the subscript
   in *SUBP. If LENP is non-null, the length of the subscript is returned
   in *LENP.  This returns newly-allocated memory. */
char *
array_variable_name (s, subp, lenp)
     const char *s;
     char **subp;
     int *lenp;
{
  char *t, *ret;
  int ind, ni;

  t = mbschr (s, '[');
  if (t == 0)
    {
      if (subp)
      	*subp = t;
      if (lenp)
	*lenp = 0;
      return ((char *)NULL);
    }
  ind = t - s;
  ni = skipsubscript (s, ind, 0);
  if (ni <= ind + 1 || s[ni] != ']')
    {
      err_badarraysub (s);
      if (subp)
      	*subp = t;
      if (lenp)
	*lenp = 0;
      return ((char *)NULL);
    }

  *t = '\0';
  ret = savestring (s);
  *t++ = '[';		/* ] */

  if (subp)
    *subp = t;
  if (lenp)
    *lenp = ni - ind;

  return ret;
}

/* Return the variable specified by S without any subscript.  If SUBP is
   non-null, return a pointer to the start of the subscript in *SUBP.
   If LENP is non-null, the length of the subscript is returned in *LENP. */
SHELL_VAR *
array_variable_part (s, subp, lenp)
     const char *s;
     char **subp;
     int *lenp;
{
  char *t;
  SHELL_VAR *var;

  t = array_variable_name (s, subp, lenp);
  if (t == 0)
    return ((SHELL_VAR *)NULL);
  var = find_variable (t);		/* XXX - handle namerefs here? */

  free (t);
  return var;	/* now return invisible variables; caller must handle */
}

#define INDEX_ERROR() \
  do \
    { \
      if (var) \
	err_badarraysub (var->name); \
      else \
	{ \
	  t[-1] = '\0'; \
	  err_badarraysub (s); \
	  t[-1] = '[';	/* ] */\
	} \
      return ((char *)NULL); \
    } \
  while (0)

/* Return a string containing the elements in the array and subscript
   described by S.  If the subscript is * or @, obeys quoting rules akin
   to the expansion of $* and $@ including double quoting.  If RTYPE
   is non-null it gets 1 if the array reference is name[*], 2 if the
   reference is name[@], and 0 otherwise. */
static char *
array_value_internal (s, quoted, flags, rtype, indp)
     const char *s;
     int quoted, flags, *rtype;
     arrayind_t *indp;
{
  int len;
  arrayind_t ind;
  char *akey;
  char *retval, *t, *temp;
  WORD_LIST *l;
  SHELL_VAR *var;

  var = array_variable_part (s, &t, &len);

  /* Expand the index, even if the variable doesn't exist, in case side
     effects are needed, like ${w[i++]} where w is unset. */
#if 0
  if (var == 0)
    return (char *)NULL;
#endif

  if (len == 0)
    return ((char *)NULL);	/* error message already printed */

  /* [ */
  akey = 0;
  if (ALL_ELEMENT_SUB (t[0]) && t[1] == ']')
    {
      if (rtype)
	*rtype = (t[0] == '*') ? 1 : 2;
      if ((flags & AV_ALLOWALL) == 0)
	{
	  err_badarraysub (s);
	  return ((char *)NULL);
	}
      else if (var == 0 || value_cell (var) == 0)	/* XXX - check for invisible_p(var) ? */
	return ((char *)NULL);
      else if (array_p (var) == 0 && assoc_p (var) == 0)
	l = add_string_to_list (value_cell (var), (WORD_LIST *)NULL);
      else if (assoc_p (var))
	{
	  l = assoc_to_word_list (assoc_cell (var));
	  if (l == (WORD_LIST *)NULL)
	    return ((char *)NULL);
	}
      else
	{
	  l = array_to_word_list (array_cell (var));
	  if (l == (WORD_LIST *)NULL)
	    return ((char *) NULL);
	}

      if (t[0] == '*' && (quoted & (Q_HERE_DOCUMENT|Q_DOUBLE_QUOTES)))
	{
	  temp = string_list_dollar_star (l);
	  retval = quote_string (temp);		/* XXX - leak here */
	  free (temp);
	}
      else	/* ${name[@]} or unquoted ${name[*]} */
        /* XXX - bash-4.4/bash-5.0 test AV_ASSIGNRHS and pass PF_ASSIGNRHS */
	retval = string_list_dollar_at (l, quoted, (flags & AV_ASSIGNRHS) ? PF_ASSIGNRHS : 0);	/* XXX - leak here */

      dispose_words (l);
    }
  else
    {
      if (rtype)
	*rtype = 0;
      if (var == 0 || array_p (var) || assoc_p (var) == 0)
	{
	  if ((flags & AV_USEIND) == 0 || indp == 0)
	    {
	      ind = array_expand_index (var, t, len);
	      if (ind < 0)
		{
		  /* negative subscripts to indexed arrays count back from end */
		  if (var && array_p (var))
		    ind = array_max_index (array_cell (var)) + 1 + ind;
		  if (ind < 0)
		    INDEX_ERROR();
		}
	      if (indp)
		*indp = ind;
	    }
	  else if (indp)
	    ind = *indp;
	}
      else if (assoc_p (var))
	{
	  t[len - 1] = '\0';
	  akey = expand_assignment_string_to_string (t, 0);	/* [ */
	  t[len - 1] = ']';
	  if (akey == 0 || *akey == 0)
	    {
	      FREE (akey);
	      INDEX_ERROR();
	    }
	}
     
      if (var == 0 || value_cell (var) == 0)	/* XXX - check invisible_p(var) ? */
	{
          FREE (akey);
	  return ((char *)NULL);
	}
      if (array_p (var) == 0 && assoc_p (var) == 0)
	return (ind == 0 ? value_cell (var) : (char *)NULL);
      else if (assoc_p (var))
        {
	  retval = assoc_reference (assoc_cell (var), akey);
	  free (akey);
        }
      else
	retval = array_reference (array_cell (var), ind);
    }

  return retval;
}

/* Return a string containing the elements described by the array and
   subscript contained in S, obeying quoting for subscripts * and @. */
char *
array_value (s, quoted, flags, rtype, indp)
     const char *s;
     int quoted, flags, *rtype;
     arrayind_t *indp;
{
  return (array_value_internal (s, quoted, flags|AV_ALLOWALL, rtype, indp));
}

/* Return the value of the array indexing expression S as a single string.
   If (FLAGS & AV_ALLOWALL) is 0, do not allow `@' and `*' subscripts.  This
   is used by other parts of the shell such as the arithmetic expression
   evaluator in expr.c. */
char *
get_array_value (s, flags, rtype, indp)
     const char *s;
     int flags, *rtype;
     arrayind_t *indp;
{
  return (array_value_internal (s, 0, flags, rtype, indp));
}

char *
array_keys (s, quoted)
     char *s;
     int quoted;
{
  int len;
  char *retval, *t, *temp;
  WORD_LIST *l;
  SHELL_VAR *var;

  var = array_variable_part (s, &t, &len);

  /* [ */
  if (var == 0 || ALL_ELEMENT_SUB (t[0]) == 0 || t[1] != ']')
    return (char *)NULL;

  if (var_isset (var) == 0 || invisible_p (var))
    return (char *)NULL;

  if (array_p (var) == 0 && assoc_p (var) == 0)
    l = add_string_to_list ("0", (WORD_LIST *)NULL);
  else if (assoc_p (var))
    l = assoc_keys_to_word_list (assoc_cell (var));
  else
    l = array_keys_to_word_list (array_cell (var));
  if (l == (WORD_LIST *)NULL)
    return ((char *) NULL);

  if (t[0] == '*' && (quoted & (Q_HERE_DOCUMENT|Q_DOUBLE_QUOTES)))
    {
      temp = string_list_dollar_star (l);
      retval = quote_string (temp);
      free (temp);
    }
  else	/* ${!name[@]} or unquoted ${!name[*]} */
    retval = string_list_dollar_at (l, quoted, 0);

  dispose_words (l);
  return retval;
}
#endif /* ARRAY_VARS */
