/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file confline.c
 *
 * \brief Functions to manipulate a linked list of key-value pairs, of the
 *   type used in Tor's configuration files.
 *
 * Tor uses the config_line_t type and its associated serialized format for
 * human-readable key-value pairs in many places, including its configuration,
 * its state files, its consensus cache, and so on.
 **/

#include "lib/encoding/confline.h"
#include "lib/encoding/cstring.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/compat_ctype.h"
#include "lib/string/compat_string.h"
#include "lib/string/util_string.h"

#include <string.h>

/** Helper: allocate a new configuration option mapping 'key' to 'val',
 * append it to *<b>lst</b>. */
void
config_line_append(config_line_t **lst,
                   const char *key,
                   const char *val)
{
  tor_assert(lst);

  config_line_t *newline;

  newline = tor_malloc_zero(sizeof(config_line_t));
  newline->key = tor_strdup(key);
  newline->value = tor_strdup(val);
  newline->next = NULL;
  while (*lst)
    lst = &((*lst)->next);

  (*lst) = newline;
}

/** Helper: allocate a new configuration option mapping 'key' to 'val',
 * and prepend it to *<b>lst</b> */
void
config_line_prepend(config_line_t **lst,
                    const char *key,
                    const char *val)
{
  tor_assert(lst);

  config_line_t *newline;

  newline = tor_malloc_zero(sizeof(config_line_t));
  newline->key = tor_strdup(key);
  newline->value = tor_strdup(val);
  newline->next = *lst;
  *lst = newline;
}

/** Return the first line in <b>lines</b> whose key is exactly <b>key</b>, or
 * NULL if no such key exists.
 *
 * (In options parsing, this is for handling commandline-only options only;
 * other options should be looked up in the appropriate data structure.) */
const config_line_t *
config_line_find(const config_line_t *lines,
                 const char *key)
{
  const config_line_t *cl;
  for (cl = lines; cl; cl = cl->next) {
    if (!strcmp(cl->key, key))
      return cl;
  }
  return NULL;
}

/** As config_line_find(), but perform a case-insensitive comparison. */
const config_line_t *
config_line_find_case(const config_line_t *lines,
                      const char *key)
{
  const config_line_t *cl;
  for (cl = lines; cl; cl = cl->next) {
    if (!strcasecmp(cl->key, key))
      return cl;
  }
  return NULL;
}

/** Auxiliary function that does all the work of config_get_lines.
 * <b>recursion_level</b> is the count of how many nested %includes we have.
 * <b>opened_lst</b> will have a list of opened files if provided.
 * Returns the a pointer to the last element of the <b>result</b> in
 * <b>last</b>. */
int
config_get_lines_aux(const char *string, config_line_t **result, int extended,
                     int allow_include, int *has_include,
                     struct smartlist_t *opened_lst, int recursion_level,
                     config_line_t **last,
                     include_handler_fn handle_include)
{
  config_line_t *list = NULL, **next, *list_last = NULL;
  char *k, *v;
  const char *parse_err;
  int include_used = 0;

  if (recursion_level > MAX_INCLUDE_RECURSION_LEVEL) {
    log_warn(LD_CONFIG, "Error while parsing configuration: more than %d "
             "nested %%includes.", MAX_INCLUDE_RECURSION_LEVEL);
    return -1;
  }

  next = &list;
  do {
    k = v = NULL;
    string = parse_config_line_from_str_verbose(string, &k, &v, &parse_err);
    if (!string) {
      log_warn(LD_CONFIG, "Error while parsing configuration: %s",
               parse_err?parse_err:"<unknown>");
      config_free_lines(list);
      tor_free(k);
      tor_free(v);
      return -1;
    }
    if (k && v) {
      unsigned command = CONFIG_LINE_NORMAL;
      if (extended) {
        if (k[0] == '+') {
          char *k_new = tor_strdup(k+1);
          tor_free(k);
          k = k_new;
          command = CONFIG_LINE_APPEND;
        } else if (k[0] == '/') {
          char *k_new = tor_strdup(k+1);
          tor_free(k);
          k = k_new;
          tor_free(v);
          v = tor_strdup("");
          command = CONFIG_LINE_CLEAR;
        }
      }

      if (allow_include && !strcmp(k, "%include") && handle_include) {
        tor_free(k);
        include_used = 1;
        log_notice(LD_CONFIG, "Processing configuration path \"%s\" at "
                   "recursion level %d.", v, recursion_level);

        config_line_t *include_list;
        if (handle_include(v, recursion_level, extended, &include_list,
                           &list_last, opened_lst) < 0) {
          log_warn(LD_CONFIG, "Error reading included configuration "
                   "file or directory: \"%s\".", v);
          config_free_lines(list);
          tor_free(v);
          return -1;
        }
        *next = include_list;
        if (list_last)
          next = &list_last->next;
        tor_free(v);
      } else {
        /* This list can get long, so we keep a pointer to the end of it
         * rather than using config_line_append over and over and getting
         * n^2 performance. */
        *next = tor_malloc_zero(sizeof(**next));
        (*next)->key = k;
        (*next)->value = v;
        (*next)->next = NULL;
        (*next)->command = command;
        list_last = *next;
        next = &((*next)->next);
      }
    } else {
      tor_free(k);
      tor_free(v);
    }
  } while (*string);

  if (last) {
    *last = list_last;
  }
  if (has_include) {
    *has_include = include_used;
  }
  *result = list;
  return 0;
}

/** Same as config_get_lines_include but does not allow %include */
int
config_get_lines(const char *string, config_line_t **result, int extended)
{
  return config_get_lines_aux(string, result, extended, 0, NULL, NULL, 1,
                              NULL, NULL);
}

/**
 * Free all the configuration lines on the linked list <b>front</b>.
 */
void
config_free_lines_(config_line_t *front)
{
  config_line_t *tmp;

  while (front) {
    tmp = front;
    front = tmp->next;

    tor_free(tmp->key);
    tor_free(tmp->value);
    tor_free(tmp);
  }
}

/** Return a newly allocated deep copy of the lines in <b>inp</b>. */
config_line_t *
config_lines_dup(const config_line_t *inp)
{
  return config_lines_dup_and_filter(inp, NULL);
}

/** Return a newly allocated deep copy of the lines in <b>inp</b>,
 * but only the ones whose keys begin with <b>key</b> (case-insensitive).
 * If <b>key</b> is NULL, do not filter. */
config_line_t *
config_lines_dup_and_filter(const config_line_t *inp,
                            const char *key)
{
  config_line_t *result = NULL;
  config_line_t **next_out = &result;
  while (inp) {
    if (key && strcasecmpstart(inp->key, key)) {
      inp = inp->next;
      continue;
    }
    *next_out = tor_malloc_zero(sizeof(config_line_t));
    (*next_out)->key = tor_strdup(inp->key);
    (*next_out)->value = tor_strdup(inp->value);
    inp = inp->next;
    next_out = &((*next_out)->next);
  }
  (*next_out) = NULL;
  return result;
}

/**
 * Given a linelist <b>inp</b> beginning with the key <b>header</b>, find the
 * next line with that key, and remove that instance and all following lines
 * from the list.  Return the lines that were removed.  Operate
 * case-insensitively.
 *
 * For example, if the header is "H", and <b>inp</b> contains "H, A, B, H, C,
 * H, D", this function will alter <b>inp</b> to contain only "H, A, B", and
 * return the elements "H, C, H, D" as a separate list.
 **/
config_line_t *
config_lines_partition(config_line_t *inp, const char *header)
{
  if (BUG(inp == NULL))
    return NULL;
  if (BUG(strcasecmp(inp->key, header)))
    return NULL;

  /* Advance ptr until it points to the link to the next segment of this
     list. */
  config_line_t **ptr = &inp->next;
  while (*ptr && strcasecmp((*ptr)->key, header)) {
    ptr = &(*ptr)->next;
  }
  config_line_t *remainder = *ptr;
  *ptr = NULL;
  return remainder;
}

/** Return true iff a and b contain identical keys and values in identical
 * order. */
int
config_lines_eq(const config_line_t *a, const config_line_t *b)
{
  while (a && b) {
    if (strcasecmp(a->key, b->key) || strcmp(a->value, b->value))
      return 0;
    a = a->next;
    b = b->next;
  }
  if (a || b)
    return 0;
  return 1;
}

/** Return the number of lines in <b>a</b> whose key is <b>key</b>. */
int
config_count_key(const config_line_t *a, const char *key)
{
  int n = 0;
  while (a) {
    if (!strcasecmp(a->key, key)) {
      ++n;
    }
    a = a->next;
  }
  return n;
}

/** Given a string containing part of a configuration file or similar format,
 * advance past comments and whitespace and try to parse a single line.  If we
 * parse a line successfully, set *<b>key_out</b> to a new string holding the
 * key portion and *<b>value_out</b> to a new string holding the value portion
 * of the line, and return a pointer to the start of the next line.  If we run
 * out of data, return a pointer to the end of the string.  If we encounter an
 * error, return NULL and set *<b>err_out</b> (if provided) to an error
 * message.
 */
const char *
parse_config_line_from_str_verbose(const char *line, char **key_out,
                                   char **value_out,
                                   const char **err_out)
{
  /*
    See torrc_format.txt for a description of the (silly) format this parses.
   */
  const char *key, *val, *cp;
  int continuation = 0;

  tor_assert(key_out);
  tor_assert(value_out);

  *key_out = *value_out = NULL;
  key = val = NULL;
  /* Skip until the first keyword. */
  while (1) {
    while (TOR_ISSPACE(*line))
      ++line;
    if (*line == '#') {
      while (*line && *line != '\n')
        ++line;
    } else {
      break;
    }
  }

  if (!*line) { /* End of string? */
    *key_out = *value_out = NULL;
    return line;
  }

  /* Skip until the next space or \ followed by newline. */
  key = line;
  while (*line && !TOR_ISSPACE(*line) && *line != '#' &&
         ! (line[0] == '\\' && line[1] == '\n'))
    ++line;
  *key_out = tor_strndup(key, line-key);

  /* Skip until the value. */
  while (*line == ' ' || *line == '\t')
    ++line;

  val = line;

  /* Find the end of the line. */
  if (*line == '\"') { // XXX No continuation handling is done here
    if (!(line = unescape_string(line, value_out, NULL))) {
      if (err_out)
        *err_out = "Invalid escape sequence in quoted string";
      return NULL;
    }
    while (*line == ' ' || *line == '\t')
      ++line;
    if (*line == '\r' && *(++line) == '\n')
      ++line;
    if (*line && *line != '#' && *line != '\n') {
      if (err_out)
        *err_out = "Excess data after quoted string";
      return NULL;
    }
  } else {
    /* Look for the end of the line. */
    while (*line && *line != '\n' && (*line != '#' || continuation)) {
      if (*line == '\\' && line[1] == '\n') {
        continuation = 1;
        line += 2;
      } else if (*line == '#') {
        do {
          ++line;
        } while (*line && *line != '\n');
        if (*line == '\n')
          ++line;
      } else {
        ++line;
      }
    }

    if (*line == '\n') {
      cp = line++;
    } else {
      cp = line;
    }
    /* Now back cp up to be the last nonspace character */
    while (cp>val && TOR_ISSPACE(*(cp-1)))
      --cp;

    tor_assert(cp >= val);

    /* Now copy out and decode the value. */
    *value_out = tor_strndup(val, cp-val);
    if (continuation) {
      char *v_out, *v_in;
      v_out = v_in = *value_out;
      while (*v_in) {
        if (*v_in == '#') {
          do {
            ++v_in;
          } while (*v_in && *v_in != '\n');
          if (*v_in == '\n')
            ++v_in;
        } else if (v_in[0] == '\\' && v_in[1] == '\n') {
          v_in += 2;
        } else {
          *v_out++ = *v_in++;
        }
      }
      *v_out = '\0';
    }
  }

  if (*line == '#') {
    do {
      ++line;
    } while (*line && *line != '\n');
  }
  while (TOR_ISSPACE(*line)) ++line;

  return line;
}
