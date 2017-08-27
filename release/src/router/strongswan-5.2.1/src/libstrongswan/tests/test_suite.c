/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include <signal.h>
#include <unistd.h>

#ifndef WIN32
#include <pthread.h>
#endif

#include <threading/thread.h>

/**
 * Failure message buf
 */
static char failure_buf[512];

/**
 * Source file failure occurred
 */
static const char *failure_file;

/**
 * Line of source file failure occurred
 */
static int failure_line;

/**
 * Backtrace of failure, if any
 */
static backtrace_t *failure_backtrace;

/**
 * Flag to indicate if a worker thread failed
 */
static bool worker_failed;

/**
 * See header.
 */
test_suite_t* test_suite_create(const char *name)
{
	test_suite_t *suite;

	INIT(suite,
		.name = name,
		.tcases = array_create(0, 0),
	);
	return suite;
}

/**
 * See header.
 */
test_case_t* test_case_create(const char *name)
{
	test_case_t *tcase;

	INIT(tcase,
		.name = name,
		.functions = array_create(sizeof(test_function_t), 0),
		.fixtures = array_create(sizeof(test_fixture_t), 0),
		.timeout = TEST_FUNCTION_DEFAULT_TIMEOUT,
	);
	return tcase;
}

/**
 * See header.
 */
void test_case_add_checked_fixture(test_case_t *tcase, test_fixture_cb_t setup,
								   test_fixture_cb_t teardown)
{
	test_fixture_t fixture = {
		.setup = setup,
		.teardown = teardown,
	};
	array_insert(tcase->fixtures, -1, &fixture);
}

/**
 * See header.
 */
void test_case_add_test_name(test_case_t *tcase, char *name,
							 test_function_cb_t cb, int start, int end)
{
	test_function_t fun = {
		.name = name,
		.cb = cb,
		.start = start,
		.end = end,
	};
	array_insert(tcase->functions, -1, &fun);
}

/**
 * See header.
 */
void test_case_set_timeout(test_case_t *tcase, int s)
{
	tcase->timeout = s;
}

/**
 * See header.
 */
void test_suite_add_case(test_suite_t *suite, test_case_t *tcase)
{
	array_insert(suite->tcases, -1, tcase);
}

#ifdef WIN32

/**
 * Longjump restore point when failing
 */
jmp_buf test_restore_point_env;

/**
 * Thread ID of main thread
 */
static DWORD main_thread;

/**
 * APC routine invoked by main thread on worker failure
 */
static void WINAPI set_worker_failure(ULONG_PTR dwParam)
{
	worker_failed = TRUE;
}

/**
 * Let test case fail
 */
static void test_failure()
{
	if (GetCurrentThreadId() == main_thread)
	{
		longjmp(test_restore_point_env, 1);
	}
	else
	{
		HANDLE *thread;

		thread = OpenThread(THREAD_SET_CONTEXT, FALSE, main_thread);
		if (thread)
		{
			QueueUserAPC(set_worker_failure, thread, (uintptr_t)NULL);
			CloseHandle(thread);
		}
		thread_exit(NULL);
	}
}

/**
 * See header.
 */
void test_fail_if_worker_failed()
{
	if (GetCurrentThreadId() == main_thread && worker_failed)
	{
		test_failure();
	}
}

/**
 * Vectored exception handler
 */
static long WINAPI eh_handler(PEXCEPTION_POINTERS ei)
{
	char *ename;
	bool old = FALSE;

	switch (ei->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			ename = "ACCESS_VIOLATION";
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			ename = "ARRAY_BOUNDS_EXCEEDED";
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			ename = "DATATYPE_MISALIGNMENT";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			ename = "FLT_DENORMAL_OPERAND";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			ename = "FLT_DIVIDE_BY_ZERO";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			ename = "FLT_INEXACT_RESULT";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			ename = "FLT_INVALID_OPERATION";
			break;
		case EXCEPTION_FLT_OVERFLOW:
			ename = "FLT_OVERFLOW";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			ename = "FLT_STACK_CHECK";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			ename = "FLT_UNDERFLOW";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			ename = "ILLEGAL_INSTRUCTION";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			ename = "IN_PAGE_ERROR";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			ename = "INT_DIVIDE_BY_ZERO";
			break;
		case EXCEPTION_INT_OVERFLOW:
			ename = "INT_OVERFLOW";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			ename = "INVALID_DISPOSITION";
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			ename = "NONCONTINUABLE_EXCEPTION";
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			ename = "PRIV_INSTRUCTION";
			break;
		case EXCEPTION_STACK_OVERFLOW:
			ename = "STACK_OVERFLOW";
			break;
		default:
			return EXCEPTION_CONTINUE_EXECUTION;
	}

	if (lib->leak_detective)
	{
		old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
	}
	failure_backtrace = backtrace_create(5);
	if (lib->leak_detective)
	{
		lib->leak_detective->set_state(lib->leak_detective, old);
	}
	failure_line = 0;
	test_fail_msg(NULL, 0, "%s exception", ename);
	/* not reached */
	return EXCEPTION_CONTINUE_EXECUTION;
}

/**
 * See header.
 */
void test_setup_handler()
{
	main_thread = GetCurrentThreadId();
	AddVectoredExceptionHandler(0, eh_handler);
}

/**
 * See header.
 */
void test_setup_timeout(int s)
{
	/* TODO: currently not supported. SetTimer()? */

	worker_failed = FALSE;
}

#else /* !WIN32 */

/**
 * Longjump restore point when failing
 */
sigjmp_buf test_restore_point_env;

/**
 * Main thread performing tests
 */
static pthread_t main_thread;

/**
 * Let test case fail
 */
static inline void test_failure()
{
	if (pthread_self() == main_thread)
	{
		siglongjmp(test_restore_point_env, 1);
	}
	else
	{
		pthread_kill(main_thread, SIGUSR1);
		/* terminate thread to prevent it from going wild */
		pthread_exit(NULL);
	}
}

/**
 * See header.
 */
void test_fail_if_worker_failed()
{
	if (pthread_self() == main_thread && worker_failed)
	{
		test_failure();
	}
}

/**
 * Signal handler catching critical and alarm signals
 */
static void test_sighandler(int signal)
{
	char *signame;
	bool old = FALSE;

	switch (signal)
	{
		case SIGUSR1:
			/* a different thread failed, abort test at the next opportunity */
			worker_failed = TRUE;
			return;
		case SIGSEGV:
			signame = "SIGSEGV";
			break;
		case SIGILL:
			signame = "SIGILL";
			break;
		case SIGBUS:
			signame = "SIGBUS";
			break;
		case SIGALRM:
			signame = "timeout";
			break;
		default:
			signame = "SIG";
			break;
	}
	if (lib->leak_detective)
	{
		old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
	}
	failure_backtrace = backtrace_create(3);
	if (lib->leak_detective)
	{
		lib->leak_detective->set_state(lib->leak_detective, old);
	}
	test_fail_msg(NULL, 0, "%s(%d)", signame, signal);
	/* unable to restore a valid context for that thread, terminate */
	fprintf(stderr, "\n%s(%d) outside of main thread:\n", signame, signal);
	failure_backtrace->log(failure_backtrace, stderr, TRUE);
	fprintf(stderr, "terminating...\n");
	abort();
}

/**
 * See header.
 */
void test_setup_handler()
{
	struct sigaction action = {
		.sa_handler = test_sighandler,
	};

	main_thread = pthread_self();

	/* signal handler inherited by all threads */
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	/* ignore ALRM/USR1, these are catched by main thread only */
	action.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &action, NULL);
	sigaction(SIGUSR1, &action, NULL);
}

/**
 * See header.
 */
void test_setup_timeout(int s)
{
	struct sigaction action = {
		.sa_handler = test_sighandler,
	};

	/* This called by main thread only. Setup handler for timeout and
	 * failure cross-thread signaling. */
	sigaction(SIGALRM, &action, NULL);
	sigaction(SIGUSR1, &action, NULL);

	alarm(s);

	worker_failed = FALSE;
}

#endif /* !WIN32 */

/**
 * See header.
 */
void test_fail_vmsg(const char *file, int line, char *fmt, va_list args)
{
	vsnprintf(failure_buf, sizeof(failure_buf), fmt, args);
	failure_line = line;
	failure_file = file;

	test_failure();
}
/**
 * See header.
 */
void test_fail_msg(const char *file, int line, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(failure_buf, sizeof(failure_buf), fmt, args);
	failure_line = line;
	failure_file = file;
	va_end(args);

	test_failure();
}

/**
 * See header.
 */
int test_failure_get(char *msg, int len, const char **file)
{
	strncpy(msg, failure_buf, len - 1);
	msg[len - 1] = 0;
	*file = failure_file;
	return failure_line;
}

/**
 * See header.
 */
backtrace_t *test_failure_backtrace()
{
	backtrace_t *bt;

	bt = failure_backtrace;
	failure_backtrace = NULL;

	return bt;
}
