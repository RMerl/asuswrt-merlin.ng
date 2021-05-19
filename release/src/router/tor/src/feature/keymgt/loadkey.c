/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file loadkey.c
 * \brief Read keys from disk, creating as needed
 *
 * This code is shared by relays and onion services, which both need
 * this functionality.
 **/

#include "core/or/or.h"
#include "app/config/config.h"
#include "app/main/main.h"
#include "feature/keymgt/loadkey.h"
#include "feature/nodelist/torcert.h"

#include "lib/crypt_ops/crypto_pwbox.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/term/getpass.h"
#include "lib/crypt_ops/crypto_format.h"

#define ENC_KEY_HEADER "Boxed Ed25519 key"
#define ENC_KEY_TAG "master"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/** Try to read an RSA key from <b>fname</b>.  If <b>fname</b> doesn't exist
 * and <b>generate</b> is true, create a new RSA key and save it in
 * <b>fname</b>.  Return the read/created key, or NULL on error.  Log all
 * errors at level <b>severity</b>. If <b>created_out</b> is non-NULL and a
 * new key was created, set *<b>created_out</b> to true.
 */
crypto_pk_t *
init_key_from_file(const char *fname, int generate, int severity,
                   bool *created_out)
{
  crypto_pk_t *prkey = NULL;

  if (created_out) {
    *created_out = false;
  }

  if (!(prkey = crypto_pk_new())) {
    tor_log(severity, LD_GENERAL,"Error constructing key");
    goto error;
  }

  switch (file_status(fname)) {
    case FN_DIR:
    case FN_ERROR:
      tor_log(severity, LD_FS,"Can't read key from \"%s\"", fname);
      goto error;
    /* treat empty key files as if the file doesn't exist, and,
     * if generate is set, replace the empty file in
     * crypto_pk_write_private_key_to_filename() */
    case FN_NOENT:
    case FN_EMPTY:
      if (generate) {
        if (!have_lockfile()) {
          if (try_locking(get_options(), 0)<0) {
            /* Make sure that --list-fingerprint only creates new keys
             * if there is no possibility for a deadlock. */
            tor_log(severity, LD_FS, "Another Tor process has locked \"%s\". "
                    "Not writing any new keys.", fname);
            /*XXXX The 'other process' might make a key in a second or two;
             * maybe we should wait for it. */
            goto error;
          }
        }
        log_info(LD_GENERAL, "No key found in \"%s\"; generating fresh key.",
                 fname);
        if (crypto_pk_generate_key(prkey)) {
          tor_log(severity, LD_GENERAL,"Error generating onion key");
          goto error;
        }
        if (! crypto_pk_is_valid_private_key(prkey)) {
          tor_log(severity, LD_GENERAL,"Generated key seems invalid");
          goto error;
        }
        log_info(LD_GENERAL, "Generated key seems valid");
        if (created_out) {
          *created_out = true;
        }
        if (crypto_pk_write_private_key_to_filename(prkey, fname)) {
          tor_log(severity, LD_FS,
              "Couldn't write generated key to \"%s\".", fname);
          goto error;
        }
      } else {
        tor_log(severity, LD_GENERAL, "No key found in \"%s\"", fname);
        goto error;
      }
      return prkey;
    case FN_FILE:
      if (crypto_pk_read_private_key_from_filename(prkey, fname)) {
        tor_log(severity, LD_GENERAL,"Error loading private key.");
        goto error;
      }
      return prkey;
    default:
      tor_assert(0);
  }

 error:
  if (prkey)
    crypto_pk_free(prkey);
  return NULL;
}

/* DOCDOC */
static ssize_t
do_getpass(const char *prompt, char *buf, size_t buflen,
           int twice, const or_options_t *options)
{
  if (options->keygen_force_passphrase == FORCE_PASSPHRASE_OFF) {
    tor_assert(buflen);
    buf[0] = 0;
    return 0;
  }

  char *prompt2 = NULL;
  char *buf2 = NULL;
  int fd = -1;
  ssize_t length = -1;

  if (options->use_keygen_passphrase_fd) {
    twice = 0;
    fd = options->keygen_passphrase_fd;
    length = read_all_from_fd(fd, buf, buflen-1);
    if (length >= 0)
      buf[length] = 0;
    goto done_reading;
  }

  if (twice) {
    const char msg[] = "One more time:";
    size_t p2len = strlen(prompt) + 1;
    if (p2len < sizeof(msg))
      p2len = sizeof(msg);
    prompt2 = tor_malloc(p2len);
    memset(prompt2, ' ', p2len);
    memcpy(prompt2 + p2len - sizeof(msg), msg, sizeof(msg));

    buf2 = tor_malloc_zero(buflen);
  }

  while (1) {
    length = tor_getpass(prompt, buf, buflen);
    if (length < 0)
      goto done_reading;

    if (! twice)
      break;

    ssize_t length2 = tor_getpass(prompt2, buf2, buflen);

    if (length != length2 || tor_memneq(buf, buf2, length)) {
      fprintf(stderr, "That didn't match.\n");
    } else {
      break;
    }
  }

 done_reading:
  if (twice) {
    tor_free(prompt2);
    memwipe(buf2, 0, buflen);
    tor_free(buf2);
  }

  if (options->keygen_force_passphrase == FORCE_PASSPHRASE_ON && length == 0)
    return -1;

  return length;
}

/* DOCDOC */
int
read_encrypted_secret_key(ed25519_secret_key_t *out,
                          const char *fname)
{
  int r = -1;
  uint8_t *secret = NULL;
  size_t secret_len = 0;
  char pwbuf[256];
  uint8_t encrypted_key[256];
  char *tag = NULL;
  int saved_errno = 0;

  ssize_t encrypted_len = crypto_read_tagged_contents_from_file(fname,
                                          ENC_KEY_HEADER,
                                          &tag,
                                          encrypted_key,
                                          sizeof(encrypted_key));
  if (encrypted_len < 0) {
    saved_errno = errno;
    log_info(LD_OR, "%s is missing", fname);
    r = 0;
    goto done;
  }
  if (strcmp(tag, ENC_KEY_TAG)) {
    saved_errno = EINVAL;
    goto done;
  }

  while (1) {
    ssize_t pwlen =
      do_getpass("Enter passphrase for master key:", pwbuf, sizeof(pwbuf), 0,
                 get_options());
    if (pwlen < 0) {
      saved_errno = EINVAL;
      goto done;
    }
    const int r_unbox = crypto_unpwbox(&secret, &secret_len,
                                       encrypted_key, encrypted_len,
                                       pwbuf, pwlen);
    if (r_unbox == UNPWBOX_CORRUPTED) {
      log_err(LD_OR, "%s is corrupted.", fname);
      saved_errno = EINVAL;
      goto done;
    } else if (r_unbox == UNPWBOX_OKAY) {
      break;
    }

    /* Otherwise, passphrase is bad, so try again till user does ctrl-c or gets
     * it right. */
  }

  if (secret_len != ED25519_SECKEY_LEN) {
    log_err(LD_OR, "%s is corrupted.", fname);
    saved_errno = EINVAL;
    goto done;
  }
  memcpy(out->seckey, secret, ED25519_SECKEY_LEN);
  r = 1;

 done:
  memwipe(encrypted_key, 0, sizeof(encrypted_key));
  memwipe(pwbuf, 0, sizeof(pwbuf));
  tor_free(tag);
  if (secret) {
    memwipe(secret, 0, secret_len);
    tor_free(secret);
  }
  if (saved_errno)
    errno = saved_errno;
  return r;
}

/* DOCDOC */
int
write_encrypted_secret_key(const ed25519_secret_key_t *key,
                           const char *fname)
{
  int r = -1;
  char pwbuf0[256];
  uint8_t *encrypted_key = NULL;
  size_t encrypted_len = 0;

  if (do_getpass("Enter new passphrase:", pwbuf0, sizeof(pwbuf0), 1,
                 get_options()) < 0) {
    log_warn(LD_OR, "NO/failed passphrase");
    return -1;
  }

  if (strlen(pwbuf0) == 0) {
    if (get_options()->keygen_force_passphrase == FORCE_PASSPHRASE_ON)
      return -1;
    else
      return 0;
  }

  if (crypto_pwbox(&encrypted_key, &encrypted_len,
                   key->seckey, sizeof(key->seckey),
                   pwbuf0, strlen(pwbuf0),  0) < 0) {
    log_warn(LD_OR, "crypto_pwbox failed!?");
    goto done;
  }
  if (crypto_write_tagged_contents_to_file(fname,
                                           ENC_KEY_HEADER,
                                           ENC_KEY_TAG,
                                           encrypted_key, encrypted_len) < 0)
    goto done;
  r = 1;
 done:
  if (encrypted_key) {
    memwipe(encrypted_key, 0, encrypted_len);
    tor_free(encrypted_key);
  }
  memwipe(pwbuf0, 0, sizeof(pwbuf0));
  return r;
}

/* DOCDOC */
static int
write_secret_key(const ed25519_secret_key_t *key, int encrypted,
                 const char *fname,
                 const char *fname_tag,
                 const char *encrypted_fname)
{
  if (encrypted) {
    int r = write_encrypted_secret_key(key, encrypted_fname);
    if (r == 1) {
      /* Success! */

      /* Try to unlink the unencrypted key, if any existed before */
      if (strcmp(fname, encrypted_fname))
        unlink(fname);
      return r;
    } else if (r != 0) {
      /* Unrecoverable failure! */
      return r;
    }

    fprintf(stderr, "Not encrypting the secret key.\n");
  }
  return ed25519_seckey_write_to_file(key, fname, fname_tag);
}

/**
 * Read an ed25519 key and associated certificates from files beginning with
 * <b>fname</b>, with certificate type <b>cert_type</b>.  On failure, return
 * NULL; on success return the keypair.
 *
 * The <b>options</b> is used to look at the change_key_passphrase value when
 * writing to disk a secret key. It is safe to be NULL even in that case.
 *
 * If INIT_ED_KEY_CREATE is set in <b>flags</b>, then create the key (and
 * certificate if requested) if it doesn't exist, and save it to disk.
 *
 * If INIT_ED_KEY_NEEDCERT is set in <b>flags</b>, load/create a certificate
 * too and store it in *<b>cert_out</b>.  Fail if the cert can't be
 * found/created.  To create a certificate, <b>signing_key</b> must be set to
 * the key that should sign it; <b>now</b> to the current time, and
 * <b>lifetime</b> to the lifetime of the key.
 *
 * If INIT_ED_KEY_REPLACE is set in <b>flags</b>, then create and save new key
 * whether we can read the old one or not.
 *
 * If INIT_ED_KEY_EXTRA_STRONG is set in <b>flags</b>, set the extra_strong
 * flag when creating the secret key.
 *
 * If INIT_ED_KEY_INCLUDE_SIGNING_KEY_IN_CERT is set in <b>flags</b>, and
 * we create a new certificate, create it with the signing key embedded.
 *
 * If INIT_ED_KEY_SPLIT is set in <b>flags</b>, and we create a new key,
 * store the public key in a separate file from the secret key.
 *
 * If INIT_ED_KEY_MISSING_SECRET_OK is set in <b>flags</b>, and we find a
 * public key file but no secret key file, return successfully anyway.
 *
 * If INIT_ED_KEY_OMIT_SECRET is set in <b>flags</b>, do not try to load a
 * secret key unless no public key is found.  Do not return a secret key. (but
 * create and save one if needed).
 *
 * If INIT_ED_KEY_TRY_ENCRYPTED is set, we look for an encrypted secret key
 * and consider encrypting any new secret key.
 *
 * If INIT_ED_KEY_NO_REPAIR is set, and there is any issue loading the keys
 * from disk _other than their absence_ (full or partial), we do not try to
 * replace them.
 *
 * If INIT_ED_KEY_SUGGEST_KEYGEN is set, have log messages about failures
 * refer to the --keygen option.
 *
 * If INIT_ED_KEY_EXPLICIT_FNAME is set, use the provided file name for the
 * secret key file, encrypted or not.
 *
 * If INIT_ED_KEY_OFFLINE_SECRET is set, we won't try to load the master
 * secret key and we log a message at <b>severity</b> that we've done so.
 */
ed25519_keypair_t *
ed_key_init_from_file(const char *fname, uint32_t flags,
                      int severity,
                      const ed25519_keypair_t *signing_key,
                      time_t now,
                      time_t lifetime,
                      uint8_t cert_type,
                      struct tor_cert_st **cert_out,
                      const or_options_t *options)
{
  char *secret_fname = NULL;
  char *encrypted_secret_fname = NULL;
  char *public_fname = NULL;
  char *cert_fname = NULL;
  const char *loaded_secret_fname = NULL;
  int created_pk = 0, created_sk = 0, created_cert = 0;
  const int try_to_load = ! (flags & INIT_ED_KEY_REPLACE);
  const int encrypt_key = !! (flags & INIT_ED_KEY_TRY_ENCRYPTED);
  const int norepair = !! (flags & INIT_ED_KEY_NO_REPAIR);
  const int split = !! (flags & INIT_ED_KEY_SPLIT);
  const int omit_secret = !! (flags & INIT_ED_KEY_OMIT_SECRET);
  const int offline_secret = !! (flags & INIT_ED_KEY_OFFLINE_SECRET);
  const int explicit_fname = !! (flags & INIT_ED_KEY_EXPLICIT_FNAME);

  /* we don't support setting both of these flags at once. */
  tor_assert((flags & (INIT_ED_KEY_NO_REPAIR|INIT_ED_KEY_NEEDCERT)) !=
                      (INIT_ED_KEY_NO_REPAIR|INIT_ED_KEY_NEEDCERT));

  char tag[8];
  tor_snprintf(tag, sizeof(tag), "type%d", (int)cert_type);

  tor_cert_t *cert = NULL;
  char *got_tag = NULL;
  ed25519_keypair_t *keypair = tor_malloc_zero(sizeof(ed25519_keypair_t));

  if (explicit_fname) {
    secret_fname = tor_strdup(fname);
    encrypted_secret_fname = tor_strdup(fname);
  } else {
    tor_asprintf(&secret_fname, "%s_secret_key", fname);
    tor_asprintf(&encrypted_secret_fname, "%s_secret_key_encrypted", fname);
  }
  tor_asprintf(&public_fname, "%s_public_key", fname);
  tor_asprintf(&cert_fname, "%s_cert", fname);

  /* Try to read the secret key. */
  int have_secret = 0;
  int load_secret = try_to_load &&
    !offline_secret &&
    (!omit_secret || file_status(public_fname)==FN_NOENT);
  if (load_secret) {
    int rv = ed25519_seckey_read_from_file(&keypair->seckey,
                                           &got_tag, secret_fname);
    if (rv == 0) {
      have_secret = 1;
      loaded_secret_fname = secret_fname;
      tor_assert(got_tag);
    } else {
      if (errno != ENOENT && norepair) {
        tor_log(severity, LD_OR, "Unable to read %s: %s", secret_fname,
                strerror(errno));
        goto err;
      }
    }
  }

  /* Should we try for an encrypted key? */
  int have_encrypted_secret_file = 0;
  if (!have_secret && try_to_load && encrypt_key) {
    int r = read_encrypted_secret_key(&keypair->seckey,
                                      encrypted_secret_fname);
    if (r > 0) {
      have_secret = 1;
      have_encrypted_secret_file = 1;
      tor_free(got_tag); /* convince coverity we aren't leaking */
      got_tag = tor_strdup(tag);
      loaded_secret_fname = encrypted_secret_fname;
    } else if (errno != ENOENT && norepair) {
      tor_log(severity, LD_OR, "Unable to read %s: %s",
              encrypted_secret_fname, strerror(errno));
      goto err;
    }
  } else {
    if (try_to_load) {
      /* Check if it's there anyway, so we don't replace it. */
      if (file_status(encrypted_secret_fname) != FN_NOENT)
        have_encrypted_secret_file = 1;
    }
  }

  if (have_secret) {
    if (strcmp(got_tag, tag)) {
      tor_log(severity, LD_OR, "%s has wrong tag", loaded_secret_fname);
      goto err;
    }
    /* Derive the public key */
    if (ed25519_public_key_generate(&keypair->pubkey, &keypair->seckey)<0) {
      tor_log(severity, LD_OR, "%s can't produce a public key",
              loaded_secret_fname);
      goto err;
    }
  }

  /* If we do split keys here, try to read the pubkey. */
  int found_public = 0;
  if (try_to_load && (!have_secret || split)) {
    ed25519_public_key_t pubkey_tmp;
    tor_free(got_tag);
    found_public = ed25519_pubkey_read_from_file(&pubkey_tmp,
                                                 &got_tag, public_fname) == 0;
    if (!found_public && errno != ENOENT && norepair) {
      tor_log(severity, LD_OR, "Unable to read %s: %s", public_fname,
              strerror(errno));
      goto err;
    }
    if (found_public && strcmp(got_tag, tag)) {
      tor_log(severity, LD_OR, "%s has wrong tag", public_fname);
      goto err;
    }
    if (found_public) {
      if (have_secret) {
        /* If we have a secret key and we're reloading the public key,
         * the key must match! */
        if (! ed25519_pubkey_eq(&keypair->pubkey, &pubkey_tmp)) {
          tor_log(severity, LD_OR, "%s does not match %s!  If you are trying "
                  "to restore from backup, make sure you didn't mix up the "
                  "key files. If you are absolutely sure that %s is the right "
                  "key for this relay, delete %s or move it out of the way.",
                  public_fname, loaded_secret_fname,
                  loaded_secret_fname, public_fname);
          goto err;
        }
      } else {
        /* We only have the public key; better use that. */
        tor_assert(split);
        memcpy(&keypair->pubkey, &pubkey_tmp, sizeof(pubkey_tmp));
      }
    } else {
      /* We have no public key file, but we do have a secret key, make the
       * public key file! */
      if (have_secret) {
        if (ed25519_pubkey_write_to_file(&keypair->pubkey, public_fname, tag)
            < 0) {
          tor_log(severity, LD_OR, "Couldn't repair %s", public_fname);
          goto err;
        } else {
          tor_log(LOG_NOTICE, LD_OR,
                  "Found secret key but not %s. Regenerating.",
                  public_fname);
        }
      }
    }
  }

  /* If the secret key is absent and it's not allowed to be, fail. */
  if (!have_secret && found_public &&
      !(flags & INIT_ED_KEY_MISSING_SECRET_OK)) {
    if (have_encrypted_secret_file) {
      tor_log(severity, LD_OR, "We needed to load a secret key from %s, "
              "but it was encrypted. Try 'tor --keygen' instead, so you "
              "can enter the passphrase.",
              secret_fname);
    } else if (offline_secret) {
      tor_log(severity, LD_OR, "We wanted to load a secret key from %s, "
              "but you're keeping it offline. (OfflineMasterKey is set.)",
              secret_fname);
    } else {
      tor_log(severity, LD_OR, "We needed to load a secret key from %s, "
              "but couldn't find it. %s", secret_fname,
              (flags & INIT_ED_KEY_SUGGEST_KEYGEN) ?
              "If you're keeping your master secret key offline, you will "
              "need to run 'tor --keygen' to generate new signing keys." :
              "Did you forget to copy it over when you copied the rest of the "
              "signing key material?");
    }
    goto err;
  }

  /* If it's absent, and we're not supposed to make a new keypair, fail. */
  if (!have_secret && !found_public && !(flags & INIT_ED_KEY_CREATE)) {
    if (split) {
      tor_log(severity, LD_OR, "No key found in %s or %s.",
              secret_fname, public_fname);
    } else {
      tor_log(severity, LD_OR, "No key found in %s.", secret_fname);
    }
    goto err;
  }

  /* If the secret key is absent, but the encrypted key would be present,
   * that's an error */
  if (!have_secret && !found_public && have_encrypted_secret_file) {
    tor_assert(!encrypt_key);
    tor_log(severity, LD_OR, "Found an encrypted secret key, "
            "but not public key file %s!", public_fname);
    goto err;
  }

  /* if it's absent, make a new keypair... */
  if (!have_secret && !found_public) {
    tor_free(keypair);
    keypair = ed_key_new(signing_key, flags, now, lifetime,
                         cert_type, &cert);
    if (!keypair) {
      tor_log(severity, LD_OR, "Couldn't create keypair");
      goto err;
    }
    created_pk = created_sk = created_cert = 1;
  }

  /* Write it to disk if we're supposed to do with a new passphrase, or if
   * we just created it. */
  if (created_sk || (have_secret && options != NULL &&
                     options->change_key_passphrase)) {
    if (write_secret_key(&keypair->seckey,
                         encrypt_key,
                         secret_fname, tag, encrypted_secret_fname) < 0
        ||
        (split &&
         ed25519_pubkey_write_to_file(&keypair->pubkey, public_fname, tag) < 0)
        ||
        (cert &&
         crypto_write_tagged_contents_to_file(cert_fname, "ed25519v1-cert",
                                 tag, cert->encoded, cert->encoded_len) < 0)) {
      tor_log(severity, LD_OR, "Couldn't write keys or cert to file.");
      goto err;
    }
    goto done;
  }

  /* If we're not supposed to get a cert, we're done. */
  if (! (flags & INIT_ED_KEY_NEEDCERT))
    goto done;

  /* Read a cert. */
  tor_free(got_tag);
  uint8_t certbuf[256];
  ssize_t cert_body_len = crypto_read_tagged_contents_from_file(
                 cert_fname, "ed25519v1-cert",
                 &got_tag, certbuf, sizeof(certbuf));
  if (cert_body_len >= 0 && !strcmp(got_tag, tag))
    cert = tor_cert_parse(certbuf, cert_body_len);

  /* If we got it, check it to the extent we can. */
  int bad_cert = 0;

  if (! cert) {
    tor_log(severity, LD_OR, "Cert was unparseable");
    bad_cert = 1;
  } else if (!tor_memeq(cert->signed_key.pubkey, keypair->pubkey.pubkey,
                        ED25519_PUBKEY_LEN)) {
    tor_log(severity, LD_OR, "Cert was for wrong key");
    bad_cert = 1;
  } else if (signing_key &&
             tor_cert_checksig(cert, &signing_key->pubkey, now) < 0) {
    tor_log(severity, LD_OR, "Can't check certificate: %s",
            tor_cert_describe_signature_status(cert));
    bad_cert = 1;
  } else if (cert->cert_expired) {
    tor_log(severity, LD_OR, "Certificate is expired");
    bad_cert = 1;
  } else if (signing_key && cert->signing_key_included &&
             ! ed25519_pubkey_eq(&signing_key->pubkey, &cert->signing_key)) {
    tor_log(severity, LD_OR, "Certificate signed by unexpected key!");
    bad_cert = 1;
  }

  if (bad_cert) {
    tor_cert_free(cert);
    cert = NULL;
  }

  /* If we got a cert, we're done. */
  if (cert)
    goto done;

  /* If we didn't get a cert, and we're not supposed to make one, fail. */
  if (!signing_key || !(flags & INIT_ED_KEY_CREATE)) {
    tor_log(severity, LD_OR, "Without signing key, can't create certificate");
    goto err;
  }

  /* We have keys but not a certificate, so make one. */
  uint32_t cert_flags = 0;
  if (flags & INIT_ED_KEY_INCLUDE_SIGNING_KEY_IN_CERT)
    cert_flags |= CERT_FLAG_INCLUDE_SIGNING_KEY;
  cert = tor_cert_create_ed25519(signing_key, cert_type,
                         &keypair->pubkey,
                         now, lifetime,
                         cert_flags);

  if (! cert) {
    tor_log(severity, LD_OR, "Couldn't create certificate");
    goto err;
  }

  /* Write it to disk. */
  created_cert = 1;
  if (crypto_write_tagged_contents_to_file(cert_fname, "ed25519v1-cert",
                             tag, cert->encoded, cert->encoded_len) < 0) {
    tor_log(severity, LD_OR, "Couldn't write cert to disk.");
    goto err;
  }

 done:
  if (cert_out)
    *cert_out = cert;
  else
    tor_cert_free(cert);

  goto cleanup;

 err:
  if (keypair)
    memwipe(keypair, 0, sizeof(*keypair));
  tor_free(keypair);
  tor_cert_free(cert);
  if (cert_out)
    *cert_out = NULL;
  if (created_sk)
    unlink(secret_fname);
  if (created_pk)
    unlink(public_fname);
  if (created_cert)
    unlink(cert_fname);

 cleanup:
  tor_free(encrypted_secret_fname);
  tor_free(secret_fname);
  tor_free(public_fname);
  tor_free(cert_fname);
  tor_free(got_tag);

  return keypair;
}

/**
 * Create a new signing key and (optionally) certficiate; do not read or write
 * from disk.  See ed_key_init_from_file() for more information.
 */
ed25519_keypair_t *
ed_key_new(const ed25519_keypair_t *signing_key,
           uint32_t flags,
           time_t now,
           time_t lifetime,
           uint8_t cert_type,
           struct tor_cert_st **cert_out)
{
  if (cert_out)
    *cert_out = NULL;

  const int extra_strong = !! (flags & INIT_ED_KEY_EXTRA_STRONG);
  ed25519_keypair_t *keypair = tor_malloc_zero(sizeof(ed25519_keypair_t));
  if (ed25519_keypair_generate(keypair, extra_strong) < 0)
    goto err;

  if (! (flags & INIT_ED_KEY_NEEDCERT))
    return keypair;

  tor_assert(signing_key);
  tor_assert(cert_out);
  uint32_t cert_flags = 0;
  if (flags & INIT_ED_KEY_INCLUDE_SIGNING_KEY_IN_CERT)
    cert_flags |= CERT_FLAG_INCLUDE_SIGNING_KEY;
  tor_cert_t *cert = tor_cert_create_ed25519(signing_key, cert_type,
                                     &keypair->pubkey,
                                     now, lifetime,
                                     cert_flags);
  if (! cert)
    goto err;

  *cert_out = cert;
  return keypair;

 err:
  tor_free(keypair);
  return NULL;
}
