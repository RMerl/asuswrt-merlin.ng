# Compile Noise Source as user space application

CC ?= $(CROSS_COMPILE)gcc
STRIP ?= $(CROSS_COMPILE)strip
CFLAGS ?=-Wextra -Wall -pedantic -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -fwrapv --param ssp-buffer-size=4 -fvisibility=hidden -fPIE -Wcast-align -Wmissing-field-initializers -Wshadow -Wswitch-enum -O2
LDFLAGS ?=-Wl,-z,relro,-z,now -pie

GCCVERSIONFORMAT := $(shell echo `$(CC) -dumpversion | sed 's/\./\n/g' | wc -l`)
ifeq "$(GCCVERSIONFORMAT)" "3"
  GCC_GTEQ_490 := $(shell expr `$(CC) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
else
  GCC_GTEQ_490 := $(shell expr `$(CC) -dumpfullversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
endif

ifeq "$(GCC_GTEQ_490)" "1"
  CFLAGS += -fstack-protector-strong
else
  CFLAGS += -fstack-protector-all
endif

# Change as necessary
DESTDIR :=
INSTALL ?= install
PREFIX := /usr/local
UNITDIR ?= $(shell pkg-config --variable=systemdsystemunitdir systemd 2>/dev/null || echo /usr/lib/systemd/system)

NAME := jitterentropy-rngd
#C_SRCS := $(wildcard *.c)
C_SRCS := jitterentropy-base.c jitterentropy-rngd.c
C_OBJS := ${C_SRCS:.c=.o}
OBJS := $(C_OBJS)

INCLUDE_DIRS :=
LIBRARY_DIRS :=
LIBRARIES := rt pthread

CFLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(LIBRARIES),-l$(library))

.PHONY: all clean distclean

all: $(NAME)

$(NAME): $(OBJS)
#	scan-build --use-analyzer=/usr/bin/clang $(CC) $(OBJS) -o $(NAME) $(LDFLAGS)
	$(CC) $(OBJS) -o $(NAME) $(LDFLAGS)

strip: $(NAME)
	$(STRIP) --strip-unneeded $(NAME)

install: strip
	$(INSTALL) -D -m 0755 $(NAME) $(DESTDIR)$(PREFIX)/sbin/$(NAME)
	$(INSTALL) -D -m 0644 $(NAME).1 $(DESTDIR)$(PREFIX)/share/man/man1/$(NAME).1
	gzip -9 $(DESTDIR)$(PREFIX)/share/man/man1/$(NAME).1
	sed "s|@PATH@|$(PREFIX)/sbin|" jitterentropy.service.in > jitterentropy.service
	$(INSTALL) -D -m 0644 jitterentropy.service $(DESTDIR)$(UNITDIR)/jitterentropy.service

clean:
	@- $(RM) $(NAME)
	@- $(RM) $(OBJS)
	@- $(RM) jitterentropy.service

distclean: clean
