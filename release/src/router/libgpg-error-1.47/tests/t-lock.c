/* t-lock.c - Check the lock functions
 * Copyright (C) 2013, 2015 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef _WIN32
# include <windows.h>
# include <time.h>
#else
# ifdef USE_POSIX_THREADS
#  include <pthread.h>
# endif
#endif

#define PGM "t-lock"

#include "t-common.h"

#ifdef _WIN32
# define THREAD_RET_TYPE  DWORD WINAPI
# define THREAD_RET_VALUE 0
#else
# define THREAD_RET_TYPE  void *
# define THREAD_RET_VALUE NULL
#endif


/* Our tests works by having a a couple of accountant threads which do
   random transactions between accounts and a revision threads which
   checks that the balance of all accounts is invariant.  The idea for
   this check is due to Bruno Haible.  */
#define N_ACCOUNT 8
#define ACCOUNT_VALUE 42
static int account[N_ACCOUNT];
GPGRT_LOCK_DEFINE (accounts_lock);

/* Number of transactions done by each accountant.  */
#define N_TRANSACTIONS 1000

/* Number of accountants to run.  */
#define N_ACCOUNTANTS 5

/* Maximum transaction value.  A quite low value is used so that we
   would get an integer overflow.  */
#define MAX_TRANSACTION_VALUE 50

/* Flag to tell the revision thread to finish.  */
static volatile int stop_revision_thread;


/* Initialze all accounts.  */
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


#if defined(_WIN32) || defined(USE_POSIX_THREADS)
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
  gpg_err_code_t rc;
  int i = 0;

  (void)arg;

  while (!stop_revision_thread)
    {
      rc = gpgrt_lock_lock (&accounts_lock);
      if (rc)
        fail ("gpgrt_lock_lock failed at %d: %s", __LINE__, gpg_strerror (rc));

      check_accounts ();
      rc = gpgrt_lock_unlock (&accounts_lock);
      if (rc)
        fail ("gpgrt_lock_unlock failed at %d: %s", __LINE__,gpg_strerror (rc));
      if (!(++i%7))
        gpgrt_yield ();
    }
  return THREAD_RET_VALUE;
}


/* This is one of our accountants.  */
static THREAD_RET_TYPE
accountant_thread (void *arg)
{
  gpg_err_code_t rc;
  int i;
  int acc1, acc2;
  int value;

  (void)arg;

#ifdef _WIN32
  srand (time(NULL)*getpid());  /* Windows needs it per thread.  */
#endif
  for (i = 0; i < N_TRANSACTIONS; i++)
    {
      rc = gpgrt_lock_lock (&accounts_lock);
      if (rc)
        fail ("gpgrt_lock_lock failed at %d: %s", __LINE__, gpg_strerror (rc));

      acc1 = pick_account ();
      acc2 = pick_account ();
      value = pick_value ();
      account[acc1] += value;
      account[acc2] -= value;

      rc = gpgrt_lock_unlock (&accounts_lock);
      if (rc)
        fail ("gpgrt_lock_unlock failed at %d: %s", __LINE__,gpg_strerror (rc));
      if (i && !(i%8))
        gpgrt_yield ();
    }
  return THREAD_RET_VALUE;
}
#endif /*_WIN32||USE_POSIX_THREADS*/


static void
run_test (void)
{
#ifdef _WIN32
  HANDLE rthread;
  HANDLE athreads[N_ACCOUNTANTS];
  int i;
  int rc;

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
        show ("accountant thread %d has terminated", i);
      else
        fail ("waiting for accountant thread %d failed: %d",
              i, (int)GetLastError ());
      CloseHandle (athreads[i]);
    }
  stop_revision_thread = 1;

  rc = WaitForSingleObject (rthread, INFINITE);
  if (rc == WAIT_OBJECT_0)
    show ("revision thread has terminated");
  else
    fail ("waiting for revision thread failed: %d", (int)GetLastError ());
  CloseHandle (rthread);

#else /*!_WIN32*/
# ifdef USE_POSIX_THREADS
  pthread_t rthread;
  pthread_t athreads[N_ACCOUNTANTS];
  int i;

  stop_revision_thread = 0;
  pthread_create (&rthread, NULL, revision_thread, NULL);

  for (i=0; i < N_ACCOUNTANTS; i++)
    pthread_create (&athreads[i], NULL, accountant_thread, NULL);

  for (i=0; i < N_ACCOUNTANTS; i++)
    {
      pthread_join (athreads[i], NULL);
      show ("accountant thread %d has terminated", i);
    }

  stop_revision_thread = 1;
  pthread_join (rthread, NULL);
  show ("revision thread has terminated");
# else /*!USE_POSIX_THREADS*/
  verbose++;
  show ("no thread support - skipping test\n", PGM);
  verbose--;
# endif /*!USE_POSIX_THREADS*/
#endif /*!_WIN32*/

  gpgrt_lock_destroy (&accounts_lock);
}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  int rc;

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

  srand (time(NULL)*getpid());

  if (!gpg_error_check_version (GPG_ERROR_VERSION))
    {
      die ("gpg_error_check_version returned an error");
      errorcount++;
    }

  init_accounts ();
  check_accounts ();
  run_test ();
  check_accounts ();
  /* Run a second time to check deinit code.  */
  run_test ();
  check_accounts ();
  /* And a third time to test an explicit init.  */
  rc = gpgrt_lock_init (&accounts_lock);
  if (rc)
    fail ("gpgrt_lock_init failed at %d: %s", __LINE__, gpg_strerror (rc));
  run_test ();
  check_accounts ();
  if (verbose)
    print_accounts ();

  return errorcount ? 1 : 0;
}
