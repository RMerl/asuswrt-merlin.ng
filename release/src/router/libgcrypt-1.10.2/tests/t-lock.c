/* t-lock.c - Check the lock functions
 * Copyright (C) 2014 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#if HAVE_PTHREAD
# include <pthread.h>
#endif

#define PGM "t-lock"

#include "t-common.h"
#include "../src/gcrypt-testapi.h"

/* Mingw requires us to include windows.h after winsock2.h which is
   included by gcrypt.h.  */
#ifdef _WIN32
# include <windows.h>
#endif

#ifdef _WIN32
# define THREAD_RET_TYPE  DWORD WINAPI
# define THREAD_RET_VALUE 0
#else
# define THREAD_RET_TYPE  void *
# define THREAD_RET_VALUE NULL
#endif


/* Number of threads to run.  */
#define N_NONCE_THREADS 8
/* Number of interations.  */
#define N_NONCE_ITERATIONS 1000
/* Requested nonce size.  */
#define NONCE_SIZE  11


/* This tests works by having a a couple of accountant threads which do
   random transactions between accounts and a revision threads which
   checks that the balance of all accounts is invariant.  The idea for
   this check is due to Bruno Haible.  */
#define N_ACCOUNT 8
#define ACCOUNT_VALUE 42
static int account[N_ACCOUNT];

/* Number of transactions done by each accountant.  */
#define N_TRANSACTIONS 1000

/* Number of accountants to run.  */
#define N_ACCOUNTANTS 5

/* Maximum transaction value.  A quite low value is used so that we
   would get an integer overflow.  */
#define MAX_TRANSACTION_VALUE 50

/* Flag to tell the revision thread to finish.  */
static volatile int stop_revision_thread;


struct thread_arg_s
{
  int no;
};




#if defined(HAVE_PTHREAD) || defined(_WIN32)
/* Wrapper functions to access Libgcrypt's internal test lock.  */
static void
external_lock_test_init (int line)
{
  gpg_error_t err;

  err = gcry_control (PRIV_CTL_EXTERNAL_LOCK_TEST, EXTERNAL_LOCK_TEST_INIT);
  if (err)
    fail ("init lock failed at %d: %s", line, gpg_strerror (err));
}

static void
external_lock_test_lock (int line)
{
  gpg_error_t err;

  err = gcry_control (PRIV_CTL_EXTERNAL_LOCK_TEST, EXTERNAL_LOCK_TEST_LOCK);
  if (err)
    fail ("taking lock failed at %d: %s", line, gpg_strerror (err));
}

static void
external_lock_test_unlock (int line)
{
  gpg_error_t err;

  err = gcry_control (PRIV_CTL_EXTERNAL_LOCK_TEST, EXTERNAL_LOCK_TEST_UNLOCK);
  if (err)
    fail ("releasing lock failed at %d: %s", line, gpg_strerror (err));

}

static void
external_lock_test_destroy (int line)
{
  gpg_error_t err;

  err = gcry_control (PRIV_CTL_EXTERNAL_LOCK_TEST, EXTERNAL_LOCK_TEST_DESTROY);
  if (err)
    fail ("destroying lock failed at %d: %s", line, gpg_strerror (err));
}
#endif



#if defined(HAVE_PTHREAD) || defined(_WIN32)
/* The nonce thread.  We simply request a couple of nonces and
   return.  */
static THREAD_RET_TYPE
nonce_thread (void *argarg)
{
  struct thread_arg_s *arg = argarg;
  int i;
  char nonce[NONCE_SIZE];

  for (i = 0; i < N_NONCE_ITERATIONS; i++)
    {
      gcry_create_nonce (nonce, sizeof nonce);
      if (i && !(i%100))
        info ("thread %d created %d nonces so far", arg->no, i);
    }

  gcry_free (arg);
  return THREAD_RET_VALUE;
}
#endif


/* To check our locking function we run several threads all accessing
   the nonce functions.  If this function returns we know that there
   are no obvious deadlocks or failed lock initialization.  */
static void
check_nonce_lock (void)
{
  struct thread_arg_s *arg;
#ifdef _WIN32
  HANDLE threads[N_NONCE_THREADS];
  int i;
  int rc;

  for (i=0; i < N_NONCE_THREADS; i++)
    {
      arg = gcry_xmalloc (sizeof *arg);
      arg->no = i;
      threads[i] = CreateThread (NULL, 0, nonce_thread, arg, 0, NULL);
      if (!threads[i])
        die ("error creating nonce thread %d: rc=%d",
             i, (int)GetLastError ());
    }

  for (i=0; i < N_NONCE_THREADS; i++)
    {
      rc = WaitForSingleObject (threads[i], INFINITE);
      if (rc == WAIT_OBJECT_0)
        info ("nonce thread %d has terminated", i);
      else
        fail ("waiting for nonce thread %d failed: %d",
              i, (int)GetLastError ());
      CloseHandle (threads[i]);
    }

#elif HAVE_PTHREAD
  pthread_t threads[N_NONCE_THREADS];
  int rc, i;

  for (i=0; i < N_NONCE_THREADS; i++)
    {
      arg = gcry_xmalloc (sizeof *arg);
      arg->no = i;
      pthread_create (&threads[i], NULL, nonce_thread, arg);
    }

  for (i=0; i < N_NONCE_THREADS; i++)
    {
      rc = pthread_join (threads[i], NULL);
      if (rc)
        fail ("pthread_join failed for nonce thread %d: %s",
              i, strerror (errno));
      else
        info ("nonce thread %d has terminated", i);
    }
#else
  (void)arg;
#endif /*!_WIN32*/
}


/* Initialize all accounts.  */
static void
init_accounts (void)
{
  int i;

  for (i=0; i < N_ACCOUNT; i++)
    account[i] = ACCOUNT_VALUE;
}


/* Check that the sum of all accounts matches the initial sum.  */
static void
check_accounts (void)
{
  int i, sum;

  sum = 0;
  for (i = 0; i < N_ACCOUNT; i++)
    sum += account[i];
  if (sum != N_ACCOUNT * ACCOUNT_VALUE)
    die ("accounts out of balance");
}


static void
print_accounts (void)
{
  int i;

  for (i=0; i < N_ACCOUNT; i++)
    printf ("account %d: %6d\n", i, account[i]);
}


#if defined(HAVE_PTHREAD) || defined(_WIN32)
/* Get a a random integer value in the range 0 to HIGH.  */
static unsigned int
get_rand (int high)
{
  return (unsigned int)(1+(int)((double)(high+1)*rand ()/(RAND_MAX+1.0))) - 1;
}


/* Pick a random account.  Note that this function is not
   thread-safe. */
static int
pick_account (void)
{
  return get_rand (N_ACCOUNT - 1);
}


/* Pick a random value for a transaction.  This is not thread-safe.  */
static int
pick_value (void)
{
  return get_rand (MAX_TRANSACTION_VALUE);
}


/* This is the revision department.  */
static THREAD_RET_TYPE
revision_thread (void *arg)
{
  (void)arg;

  while (!stop_revision_thread)
    {
      external_lock_test_lock (__LINE__);
      check_accounts ();
      external_lock_test_unlock (__LINE__);
    }
  return THREAD_RET_VALUE;
}


/* This is one of our accountants.  */
static THREAD_RET_TYPE
accountant_thread (void *arg)
{
  int i;
  int acc1, acc2;
  int value;

  (void)arg;

  for (i = 0; i < N_TRANSACTIONS; i++)
    {
      external_lock_test_lock (__LINE__);
      acc1 = pick_account ();
      acc2 = pick_account ();
      value = pick_value ();
      account[acc1] += value;
      account[acc2] -= value;
      external_lock_test_unlock (__LINE__);
    }
  return THREAD_RET_VALUE;
}
#endif


static void
run_test (void)
{
#ifdef _WIN32
  HANDLE rthread;
  HANDLE athreads[N_ACCOUNTANTS];
  int i;
  int rc;

  external_lock_test_init (__LINE__);
  stop_revision_thread = 0;
  rthread = CreateThread (NULL, 0, revision_thread, NULL, 0, NULL);
  if (!rthread)
    die ("error creating revision thread: rc=%d", (int)GetLastError ());

  for (i=0; i < N_ACCOUNTANTS; i++)
    {
      athreads[i] = CreateThread (NULL, 0, accountant_thread, NULL, 0, NULL);
      if (!athreads[i])
        die ("error creating accountant thread %d: rc=%d",
             i, (int)GetLastError ());
    }

  for (i=0; i < N_ACCOUNTANTS; i++)
    {
      rc = WaitForSingleObject (athreads[i], INFINITE);
      if (rc == WAIT_OBJECT_0)
        info ("accountant thread %d has terminated", i);
      else
        fail ("waiting for accountant thread %d failed: %d",
              i, (int)GetLastError ());
      CloseHandle (athreads[i]);
    }
  stop_revision_thread = 1;

  rc = WaitForSingleObject (rthread, INFINITE);
  if (rc == WAIT_OBJECT_0)
    info ("revision thread has terminated");
  else
    fail ("waiting for revision thread failed: %d", (int)GetLastError ());
  CloseHandle (rthread);

  external_lock_test_destroy (__LINE__);
#elif HAVE_PTHREAD
  pthread_t rthread;
  pthread_t athreads[N_ACCOUNTANTS];
  int rc, i;

  external_lock_test_init (__LINE__);
  stop_revision_thread = 0;
  pthread_create (&rthread, NULL, revision_thread, NULL);

  for (i=0; i < N_ACCOUNTANTS; i++)
    pthread_create (&athreads[i], NULL, accountant_thread, NULL);

  for (i=0; i < N_ACCOUNTANTS; i++)
    {
      rc = pthread_join (athreads[i], NULL);
      if (rc)
        fail ("pthread_join failed for accountant thread %d: %s",
              i, strerror (errno));
      else
        info ("accountant thread %d has terminated", i);
    }

  stop_revision_thread = 1;
  rc = pthread_join (rthread, NULL);
  if (rc)
    fail ("pthread_join failed for the revision thread: %s", strerror (errno));
  else
    info ("revision thread has terminated");

  external_lock_test_destroy (__LINE__);
#endif /*!_WIN32*/
}



int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    {
      argc--; argv++;
    }
  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--help"))
        {
          puts (
"usage: ./t-lock [options]\n"
"\n"
"Options:\n"
"  --verbose      Show what is going on\n"
"  --debug        Flyswatter\n"
);
          exit (0);
        }
      if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = debug = 1;
          argc--; argv++;
        }
    }

  srand ((unsigned int)time(NULL)*getpid());

  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));
  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch");
  /* We are using non-public interfaces - check the exact version.  */
  if (strcmp (gcry_check_version (NULL), GCRYPT_VERSION))
    die ("exact version match failed");
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  check_nonce_lock ();

  init_accounts ();
  check_accounts ();

  run_test ();
  check_accounts ();

  /* Run a second time to check deinit code.  */
  run_test ();
  check_accounts ();

  if (verbose)
    print_accounts ();

  return error_count ? 1 : 0;
}
