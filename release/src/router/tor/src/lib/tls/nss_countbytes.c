/* Copyright 2018-2021, The Tor Project Inc. */
/* See LICENSE for licensing information */

/**
 * \file nss_countbytes.c
 * \brief A PRFileDesc layer to let us count the number of bytes
 *        bytes actually written on a PRFileDesc.
 **/

#include "orconfig.h"

#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/tls/nss_countbytes.h"

#include <stdlib.h>
#include <string.h>

#include <prio.h>

/** Boolean: have we initialized this module */
static bool countbytes_initialized = false;

/** Integer to identity this layer. */
static PRDescIdentity countbytes_layer_id = PR_INVALID_IO_LAYER;

/** Table of methods for this layer.*/
static PRIOMethods countbytes_methods;

/** Default close function provided by NSPR.  We use this to help
 *  implement our own close function.*/
static PRStatus(*default_close_fn)(PRFileDesc *fd);

static PRStatus countbytes_close_fn(PRFileDesc *fd);
static PRInt32 countbytes_read_fn(PRFileDesc *fd, void *buf, PRInt32 amount);
static PRInt32 countbytes_write_fn(PRFileDesc *fd, const void *buf,
                                   PRInt32 amount);
static PRInt32 countbytes_writev_fn(PRFileDesc *fd, const PRIOVec *iov,
                                    PRInt32 size, PRIntervalTime timeout);
static PRInt32 countbytes_send_fn(PRFileDesc *fd, const void *buf,
                                  PRInt32 amount, PRIntn flags,
                                  PRIntervalTime timeout);
static PRInt32 countbytes_recv_fn(PRFileDesc *fd, void *buf, PRInt32 amount,
                                  PRIntn flags, PRIntervalTime timeout);

/** Private fields for the byte-counter layer.  We cast this to and from
 * PRFilePrivate*, which is supposed to be allowed. */
typedef struct tor_nss_bytecounts_t {
  uint64_t n_read;
  uint64_t n_written;
} tor_nss_bytecounts_t;

/**
 * Initialize this module, if it is not already initialized.
 **/
void
tor_nss_countbytes_init(void)
{
  if (countbytes_initialized)
    return;

  countbytes_layer_id = PR_GetUniqueIdentity("Tor byte-counting layer");
  tor_assert(countbytes_layer_id != PR_INVALID_IO_LAYER);

  memcpy(&countbytes_methods, PR_GetDefaultIOMethods(), sizeof(PRIOMethods));

  default_close_fn = countbytes_methods.close;
  countbytes_methods.close = countbytes_close_fn;
  countbytes_methods.read = countbytes_read_fn;
  countbytes_methods.write = countbytes_write_fn;
  countbytes_methods.writev = countbytes_writev_fn;
  countbytes_methods.send = countbytes_send_fn;
  countbytes_methods.recv = countbytes_recv_fn;
  /* NOTE: We aren't wrapping recvfrom, sendto, or sendfile, since I think
   * NSS won't be using them for TLS connections. */

  countbytes_initialized = true;
}

/**
 * Return the tor_nss_bytecounts_t object for a given IO layer. Asserts that
 * the IO layer is in fact a layer created by this module.
 */
static tor_nss_bytecounts_t *
get_counts(PRFileDesc *fd)
{
  tor_assert(fd->identity == countbytes_layer_id);
  return (tor_nss_bytecounts_t*) fd->secret;
}

/** Helper: increment the read-count of an fd by n. */
#define INC_READ(fd, n) STMT_BEGIN                      \
    get_counts(fd)->n_read += (n);                      \
  STMT_END

/** Helper: increment the write-count of an fd by n. */
#define INC_WRITTEN(fd, n) STMT_BEGIN                      \
    get_counts(fd)->n_written += (n);                      \
  STMT_END

/** Implementation for PR_Close: frees the 'secret' field, then passes control
 * to the default close function */
static PRStatus
countbytes_close_fn(PRFileDesc *fd)
{
  tor_assert(fd);

  tor_nss_bytecounts_t *counts = (tor_nss_bytecounts_t *)fd->secret;
  tor_free(counts);
  fd->secret = NULL;

  return default_close_fn(fd);
}

/** Implementation for PR_Read: Calls the lower-level read function,
 * and records what it said. */
static PRInt32
countbytes_read_fn(PRFileDesc *fd, void *buf, PRInt32 amount)
{
  tor_assert(fd);
  tor_assert(fd->lower);

  PRInt32 result = (fd->lower->methods->read)(fd->lower, buf, amount);
  if (result > 0)
    INC_READ(fd, result);
  return result;
}
/** Implementation for PR_Write: Calls the lower-level write function,
 * and records what it said. */
static PRInt32
countbytes_write_fn(PRFileDesc *fd, const void *buf, PRInt32 amount)
{
  tor_assert(fd);
  tor_assert(fd->lower);

  PRInt32 result = (fd->lower->methods->write)(fd->lower, buf, amount);
  if (result > 0)
    INC_WRITTEN(fd, result);
  return result;
}
/** Implementation for PR_Writev: Calls the lower-level writev function,
 * and records what it said. */
static PRInt32
countbytes_writev_fn(PRFileDesc *fd, const PRIOVec *iov,
                     PRInt32 size, PRIntervalTime timeout)
{
  tor_assert(fd);
  tor_assert(fd->lower);

  PRInt32 result = (fd->lower->methods->writev)(fd->lower, iov, size, timeout);
  if (result > 0)
    INC_WRITTEN(fd, result);
  return result;
}
/** Implementation for PR_Send: Calls the lower-level send function,
 * and records what it said. */
static PRInt32
countbytes_send_fn(PRFileDesc *fd, const void *buf,
                   PRInt32 amount, PRIntn flags, PRIntervalTime timeout)
{
  tor_assert(fd);
  tor_assert(fd->lower);

  PRInt32 result = (fd->lower->methods->send)(fd->lower, buf, amount, flags,
                                              timeout);
  if (result > 0)
    INC_WRITTEN(fd, result);
  return result;
}
/** Implementation for PR_Recv: Calls the lower-level recv function,
 * and records what it said. */
static PRInt32
countbytes_recv_fn(PRFileDesc *fd, void *buf, PRInt32 amount,
                                  PRIntn flags, PRIntervalTime timeout)
{
  tor_assert(fd);
  tor_assert(fd->lower);

  PRInt32 result = (fd->lower->methods->recv)(fd->lower, buf, amount, flags,
                                              timeout);
  if (result > 0)
    INC_READ(fd, result);
  return result;
}

/**
 * Wrap a PRFileDesc from NSPR with a new PRFileDesc that will count the
 * total number of bytes read and written.  Return the new PRFileDesc.
 *
 * This function takes ownership of its input.
 */
PRFileDesc *
tor_wrap_prfiledesc_with_byte_counter(PRFileDesc *stack)
{
  if (BUG(! countbytes_initialized)) {
    tor_nss_countbytes_init();
  }

  tor_nss_bytecounts_t *bytecounts = tor_malloc_zero(sizeof(*bytecounts));

  PRFileDesc *newfd = PR_CreateIOLayerStub(countbytes_layer_id,
                                           &countbytes_methods);
  tor_assert(newfd);
  newfd->secret = (PRFilePrivate *)bytecounts;

  /* This does some complicated messing around with the headers of these
     objects; see the NSPR documentation for more. The upshot is that
     after PushIOLayer, "stack" will be the head of the stack.
  */
  PRStatus status = PR_PushIOLayer(stack, PR_TOP_IO_LAYER, newfd);
  tor_assert(status == PR_SUCCESS);

  return stack;
}

/**
 * Given a PRFileDesc returned by tor_wrap_prfiledesc_with_byte_counter(),
 * or another PRFileDesc wrapping that PRFileDesc, set the provided
 * pointers to the number of bytes read and written on the descriptor since
 * it was created.
 *
 * Return 0 on success, -1 on failure.
 */
int
tor_get_prfiledesc_byte_counts(PRFileDesc *fd,
                               uint64_t *n_read_out,
                               uint64_t *n_written_out)
{
  if (BUG(! countbytes_initialized)) {
    tor_nss_countbytes_init();
  }

  tor_assert(fd);
  PRFileDesc *bclayer = PR_GetIdentitiesLayer(fd, countbytes_layer_id);
  if (BUG(bclayer == NULL))
    return -1;

  tor_nss_bytecounts_t *counts = get_counts(bclayer);

  *n_read_out = counts->n_read;
  *n_written_out = counts->n_written;

  return 0;
}
