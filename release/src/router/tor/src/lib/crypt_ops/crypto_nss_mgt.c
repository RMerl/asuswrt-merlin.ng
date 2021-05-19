/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_nss_mgt.c
 *
 * \brief Manage the NSS library (if used)
 **/

#include "lib/crypt_ops/crypto_nss_mgt.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/string/printf.h"

DISABLE_GCC_WARNING("-Wstrict-prototypes")
#include <nss.h>
#include <pk11func.h>
#include <ssl.h>

#include <prerror.h>
#include <prtypes.h>
#include <prinit.h>
ENABLE_GCC_WARNING("-Wstrict-prototypes")

const char *
crypto_nss_get_version_str(void)
{
  return NSS_GetVersion();
}
const char *
crypto_nss_get_header_version_str(void)
{
  return NSS_VERSION;
}

/** A password function that always returns NULL. */
static char *
nss_password_func_always_fail(PK11SlotInfo *slot,
                              PRBool retry,
                              void *arg)
{
  (void) slot;
  (void) retry;
  (void) arg;
  return NULL;
}

void
crypto_nss_early_init(int nss_only)
{
  if (! nss_only) {
    PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);
    PK11_SetPasswordFunc(nss_password_func_always_fail);
  }

  /* Eventually we should use NSS_Init() instead -- but that wants a
     directory. The documentation says that we can't use this if we want
     to use OpenSSL. */
  if (NSS_NoDB_Init(NULL) == SECFailure) {
    log_err(LD_CRYPTO, "Unable to initialize NSS.");
    crypto_nss_log_errors(LOG_ERR, "initializing NSS");
    tor_assert_unreached();
  }

  if (NSS_SetDomesticPolicy() == SECFailure) {
    log_err(LD_CRYPTO, "Unable to set NSS cipher policy.");
    crypto_nss_log_errors(LOG_ERR, "setting cipher policy");
    tor_assert_unreached();
  }

  /* We need to override the default here, or NSS will reject all the
   * legacy Tor certificates. */
  SECStatus rv = NSS_OptionSet(NSS_RSA_MIN_KEY_SIZE, 1024);
  if (rv != SECSuccess) {
    log_err(LD_CRYPTO, "Unable to set NSS min RSA key size");
    crypto_nss_log_errors(LOG_ERR, "setting cipher option.");
    tor_assert_unreached();
  }
}

void
crypto_nss_log_errors(int severity, const char *doing)
{
  PRErrorCode code = PR_GetError();
  const char *string = PORT_ErrorToString(code);
  const char *name = PORT_ErrorToName(code);
  char buf[16];
  if (!string)
    string = "<unrecognized>";
  if (!name) {
    tor_snprintf(buf, sizeof(buf), "%d", code);
    name = buf;
  }
  if (doing) {
    tor_log(severity, LD_CRYPTO, "NSS error %s while %s: %s",
            name, doing, string);
  } else {
    tor_log(severity, LD_CRYPTO, "NSS error %s: %s", name, string);
  }
}

int
crypto_nss_late_init(void)
{
  /* Possibly, SSL_OptionSetDefault? */

  return 0;
}

void
crypto_nss_global_cleanup(void)
{
  NSS_Shutdown();
  PL_ArenaFinish();
  PR_Cleanup();
}

void
crypto_nss_prefork(void)
{
  NSS_Shutdown();
}

void
crypto_nss_postfork(void)
{
  crypto_nss_early_init(1);
}
