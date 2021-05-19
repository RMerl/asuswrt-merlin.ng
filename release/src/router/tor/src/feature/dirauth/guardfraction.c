/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file bwauth.c
 * \brief Code to read and apply guard fraction data.
 **/

#define GUARDFRACTION_PRIVATE
#include "core/or/or.h"
#include "feature/dirauth/guardfraction.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/dirparse/ns_parse.h"

#include "feature/nodelist/vote_routerstatus_st.h"

#include "lib/encoding/confline.h"

/** The guardfraction of the guard with identity fingerprint <b>guard_id</b>
 *  is <b>guardfraction_percentage</b>. See if we have a vote routerstatus for
 *  this guard in <b>vote_routerstatuses</b>, and if we do, register the
 *  information to it.
 *
 *  Return 1 if we applied the information and 0 if we couldn't find a
 *  matching guard.
 *
 * Requires that <b>vote_routerstatuses</b> be sorted.
 */
static int
guardfraction_line_apply(const char *guard_id,
                      uint32_t guardfraction_percentage,
                      smartlist_t *vote_routerstatuses)
{
  vote_routerstatus_t *vrs = NULL;

  tor_assert(vote_routerstatuses);

  vrs = smartlist_bsearch(vote_routerstatuses, guard_id,
                         compare_digest_to_vote_routerstatus_entry);

  if (!vrs) {
    return 0;
  }

  vrs->status.has_guardfraction = 1;
  vrs->status.guardfraction_percentage = guardfraction_percentage;

  return 1;
}

/* Given a guard line from a guardfraction file, parse it and register
 * its information to <b>vote_routerstatuses</b>.
 *
 * Return:
 * * 1 if the line was proper and its information got registered.
 * * 0 if the line was proper but no currently active guard was found
 *     to register the guardfraction information to.
 * * -1 if the line could not be parsed and set <b>err_msg</b> to a
      newly allocated string containing the error message.
 */
static int
guardfraction_file_parse_guard_line(const char *guard_line,
                                    smartlist_t *vote_routerstatuses,
                                    char **err_msg)
{
  char guard_id[DIGEST_LEN];
  uint32_t guardfraction;
  char *inputs_tmp = NULL;
  int num_ok = 1;

  smartlist_t *sl = smartlist_new();
  int retval = -1;

  tor_assert(err_msg);

  /* guard_line should contain something like this:
     <hex digest> <guardfraction> <appearances> */
  smartlist_split_string(sl, guard_line, " ",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 3);
  if (smartlist_len(sl) < 3) {
    tor_asprintf(err_msg, "bad line '%s'", guard_line);
    goto done;
  }

  inputs_tmp = smartlist_get(sl, 0);
  if (strlen(inputs_tmp) != HEX_DIGEST_LEN ||
      base16_decode(guard_id, DIGEST_LEN,
                    inputs_tmp, HEX_DIGEST_LEN) != DIGEST_LEN) {
    tor_asprintf(err_msg, "bad digest '%s'", inputs_tmp);
    goto done;
  }

  inputs_tmp = smartlist_get(sl, 1);
  /* Guardfraction is an integer in [0, 100]. */
  guardfraction =
    (uint32_t) tor_parse_long(inputs_tmp, 10, 0, 100, &num_ok, NULL);
  if (!num_ok) {
    tor_asprintf(err_msg, "wrong percentage '%s'", inputs_tmp);
    goto done;
  }

  /* If routerstatuses were provided, apply this info to actual routers. */
  if (vote_routerstatuses) {
    retval = guardfraction_line_apply(guard_id, guardfraction,
                                      vote_routerstatuses);
  } else {
    retval = 0; /* If we got this far, line was correctly formatted. */
  }

 done:

  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);

  return retval;
}

/** Given an inputs line from a guardfraction file, parse it and
 *  register its information to <b>total_consensuses</b> and
 *  <b>total_days</b>.
 *
 *  Return 0 if it parsed well. Return -1 if there was an error, and
 *  set <b>err_msg</b> to a newly allocated string containing the
 *  error message.
 */
static int
guardfraction_file_parse_inputs_line(const char *inputs_line,
                                     int *total_consensuses,
                                     int *total_days,
                                     char **err_msg)
{
  int retval = -1;
  char *inputs_tmp = NULL;
  int num_ok = 1;
  smartlist_t *sl = smartlist_new();

  tor_assert(err_msg);

  /* Second line is inputs information:
   *   n-inputs <total_consensuses> <total_days>. */
  smartlist_split_string(sl, inputs_line, " ",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 3);
  if (smartlist_len(sl) < 2) {
    tor_asprintf(err_msg, "incomplete line '%s'", inputs_line);
    goto done;
  }

  inputs_tmp = smartlist_get(sl, 0);
  *total_consensuses =
    (int) tor_parse_long(inputs_tmp, 10, 0, INT_MAX, &num_ok, NULL);
  if (!num_ok) {
    tor_asprintf(err_msg, "unparseable consensus '%s'", inputs_tmp);
    goto done;
  }

  inputs_tmp = smartlist_get(sl, 1);
  *total_days =
    (int) tor_parse_long(inputs_tmp, 10, 0, INT_MAX, &num_ok, NULL);
  if (!num_ok) {
    tor_asprintf(err_msg, "unparseable days '%s'", inputs_tmp);
    goto done;
  }

  retval = 0;

 done:
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);

  return retval;
}

/* Maximum age of a guardfraction file that we are willing to accept. */
#define MAX_GUARDFRACTION_FILE_AGE (7*24*60*60) /* approx a week */

/** Static strings of guardfraction files. */
#define GUARDFRACTION_DATE_STR "written-at"
#define GUARDFRACTION_INPUTS "n-inputs"
#define GUARDFRACTION_GUARD "guard-seen"
#define GUARDFRACTION_VERSION "guardfraction-file-version"

/** Given a guardfraction file in a string, parse it and register the
 *  guardfraction information to the provided vote routerstatuses.
 *
 *  This is the rough format of the guardfraction file:
 *
 *      guardfraction-file-version 1
 *      written-at <date and time>
 *      n-inputs <number of consensuses parsed> <number of days considered>
 *
 *      guard-seen <fpr 1> <guardfraction percentage> <consensus appearances>
 *      guard-seen <fpr 2> <guardfraction percentage> <consensus appearances>
 *      guard-seen <fpr 3> <guardfraction percentage> <consensus appearances>
 *      guard-seen <fpr 4> <guardfraction percentage> <consensus appearances>
 *      guard-seen <fpr 5> <guardfraction percentage> <consensus appearances>
 *      ...
 *
 *  Return -1 if the parsing failed and 0 if it went smoothly. Parsing
 *  should tolerate errors in all lines but the written-at header.
 */
STATIC int
dirserv_read_guardfraction_file_from_str(const char *guardfraction_file_str,
                                      smartlist_t *vote_routerstatuses)
{
  config_line_t *front=NULL, *line;
  int ret_tmp;
  int retval = -1;
  int current_line_n = 0; /* line counter for better log messages */

  /* Guardfraction info to be parsed */
  int total_consensuses = 0;
  int total_days = 0;

  /* Stats */
  int guards_read_n = 0;
  int guards_applied_n = 0;

  /* Parse file and split it in lines */
  ret_tmp = config_get_lines(guardfraction_file_str, &front, 0);
  if (ret_tmp < 0) {
    log_warn(LD_CONFIG, "Error reading from guardfraction file");
    goto done;
  }

  /* Sort routerstatuses (needed later when applying guardfraction info) */
  if (vote_routerstatuses)
    smartlist_sort(vote_routerstatuses, compare_vote_routerstatus_entries);

  for (line = front; line; line=line->next) {
    current_line_n++;

    if (!strcmp(line->key, GUARDFRACTION_VERSION)) {
      int num_ok = 1;
      unsigned int version;

      version =
        (unsigned int) tor_parse_long(line->value,
                                      10, 0, INT_MAX, &num_ok, NULL);

      if (!num_ok || version != 1) {
        log_warn(LD_GENERAL, "Got unknown guardfraction version %d.", version);
        goto done;
      }
    } else if (!strcmp(line->key, GUARDFRACTION_DATE_STR)) {
      time_t file_written_at;
      time_t now = time(NULL);

      /* First line is 'written-at <date>' */
      if (parse_iso_time(line->value, &file_written_at) < 0) {
        log_warn(LD_CONFIG, "Guardfraction:%d: Bad date '%s'. Ignoring",
                 current_line_n, line->value);
        goto done; /* don't tolerate failure here. */
      }
      if (file_written_at < now - MAX_GUARDFRACTION_FILE_AGE) {
        log_warn(LD_CONFIG, "Guardfraction:%d: was written very long ago '%s'",
                 current_line_n, line->value);
        goto done; /* don't tolerate failure here. */
      }
    } else if (!strcmp(line->key, GUARDFRACTION_INPUTS)) {
      char *err_msg = NULL;

      if (guardfraction_file_parse_inputs_line(line->value,
                                               &total_consensuses,
                                               &total_days,
                                               &err_msg) < 0) {
        log_warn(LD_CONFIG, "Guardfraction:%d: %s",
                 current_line_n, err_msg);
        tor_free(err_msg);
        continue;
      }

    } else if (!strcmp(line->key, GUARDFRACTION_GUARD)) {
      char *err_msg = NULL;

      ret_tmp = guardfraction_file_parse_guard_line(line->value,
                                                    vote_routerstatuses,
                                                    &err_msg);
      if (ret_tmp < 0) { /* failed while parsing the guard line */
        log_warn(LD_CONFIG, "Guardfraction:%d: %s",
                 current_line_n, err_msg);
        tor_free(err_msg);
        continue;
      }

      /* Successfully parsed guard line. Check if it was applied properly. */
      guards_read_n++;
      if (ret_tmp > 0) {
        guards_applied_n++;
      }
    } else {
      log_warn(LD_CONFIG, "Unknown guardfraction line %d (%s %s)",
               current_line_n, line->key, line->value);
    }
  }

  retval = 0;

  log_info(LD_CONFIG,
           "Successfully parsed guardfraction file with %d consensuses over "
           "%d days. Parsed %d nodes and applied %d of them%s.",
           total_consensuses, total_days, guards_read_n, guards_applied_n,
           vote_routerstatuses ? "" : " (no routerstatus provided)" );

 done:
  config_free_lines(front);

  if (retval < 0) {
    return retval;
  } else {
    return guards_read_n;
  }
}

/** Read a guardfraction file at <b>fname</b> and load all its
 *  information to <b>vote_routerstatuses</b>. */
int
dirserv_read_guardfraction_file(const char *fname,
                             smartlist_t *vote_routerstatuses)
{
  char *guardfraction_file_str;

  /* Read file to a string */
  guardfraction_file_str = read_file_to_str(fname, RFTS_IGNORE_MISSING, NULL);
  if (!guardfraction_file_str) {
      log_warn(LD_FS, "Cannot open guardfraction file '%s'. Failing.", fname);
      return -1;
  }

  return dirserv_read_guardfraction_file_from_str(guardfraction_file_str,
                                               vote_routerstatuses);
}
