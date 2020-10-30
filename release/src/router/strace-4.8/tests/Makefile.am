# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = net-accept-connect

TESTS = ptrace_setoptions strace-f qual_syscall stat net

EXTRA_DIST = init.sh $(TESTS)

CLEANFILES = check.log
