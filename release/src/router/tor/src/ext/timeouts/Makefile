# NOTE: GNU Make 3.81 won't export MAKEFLAGS if .POSIX is specified, but
# Solaris make won't export MAKEFLAGS unless .POSIX is specified.
$(firstword ignore).POSIX:

.DEFAULT_GOAL = all

.SUFFIXES:

all:

#
# USER-MODIFIABLE MACROS
#
top_srcdir = .
top_builddir = .

CFLAGS = -O2 -march=native -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
SOFLAGS = $$(auto_soflags)
LIBS = $$(auto_libs)

ALL_CPPFLAGS = -I$(top_srcdir) -DWHEEL_BIT=$(WHEEL_BIT) -DWHEEL_NUM=$(WHEEL_NUM) $(CPPFLAGS)
ALL_CFLAGS = $(CFLAGS)
ALL_SOFLAGS = $(SOFLAGS)
ALL_LDFLAGS = $(LDFLAGS)
ALL_LIBS = $(LIBS)

LUA_API = 5.3
LUA = lua
LUA51_CPPFLAGS = $(LUA_CPPFLAGS)
LUA52_CPPFLAGS = $(LUA_CPPFLAGS)
LUA53_CPPFLAGS = $(LUA_CPPFLAGS)

WHEEL_BIT = 6
WHEEL_NUM = 4

RM = rm -f

# END MACROS

SHRC = \
	top_srcdir="$(top_srcdir)"; \
	top_builddir="$(top_builddir)"; \
	. "$${top_srcdir}/Rules.shrc"

LUA_APIS = 5.1 5.2 5.3

include $(top_srcdir)/lua/Rules.mk
include $(top_srcdir)/bench/Rules.mk

all: test-timeout

timeout.o: $(top_srcdir)/timeout.c
test-timeout.o: $(top_srcdir)/test-timeout.c

timeout.o test-timeout.o:
	@$(SHRC); echo_cmd $(CC) $(ALL_CFLAGS) -c -o $@ $${top_srcdir}/$(@F:%.o=%.c) $(ALL_CPPFLAGS)

test-timeout: timeout.o test-timeout.o
	@$(SHRC); echo_cmd $(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) -o $@ timeout.o test-timeout.o

.PHONY: clean clean~

clean:
	$(RM) $(top_builddir)/test-timeout $(top_builddir)/*.o
	$(RM) -r $(top_builddir)/*.dSYM

clean~:
	find $(top_builddir) $(top_srcdir) -name "*~" -exec $(RM) -- {} "+"
