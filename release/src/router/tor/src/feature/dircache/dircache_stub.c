/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dircache_stub.c
 * @brief Stub declarations for use when dircache module is disabled.
 **/

#include "core/or/or.h"
#include "feature/dircache/consdiffmgr.h"
#include "feature/dircache/dircache.h"
#include "feature/dircache/dirserv.h"
#include "feature/dircommon/dir_connection_st.h"

DISABLE_GCC_WARNING("-Wmissing-noreturn")
int
directory_handle_command(dir_connection_t *conn)
{
  (void) conn;
  tor_assert_nonfatal_unreached_once();
  return -1;
}

int
connection_dirserv_flushed_some(dir_connection_t *conn)
{
  (void) conn;
  tor_assert_nonfatal_unreached_once();
  return -1;
}
ENABLE_GCC_WARNING("-Wmissing-noreturn")

void
dir_conn_clear_spool(dir_connection_t *conn)
{
  if (!conn)
    return;
  tor_assert_nonfatal_once(conn->spool == NULL);
}

void
consdiffmgr_enable_background_compression(void)
{
}

int
consdiffmgr_add_consensus(const char *consensus,
                          size_t consensus_len,
                          const networkstatus_t *as_parsed)
{
  (void)consensus;
  (void)consensus_len;
  (void)as_parsed;
  return 0;
}

int
consdiffmgr_register_with_sandbox(struct sandbox_cfg_elem_t **cfg)
{
  (void)cfg;
  return 0;
}

int
consdiffmgr_cleanup(void)
{
  return 0;
}

void
consdiffmgr_free_all(void)
{
}

void
dirserv_free_all(void)
{
}
