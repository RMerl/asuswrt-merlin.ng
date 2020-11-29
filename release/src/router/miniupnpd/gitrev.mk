# (c) 2019-2020 Thomas Bernard
# For GNU Make

ISGITREPO := $(shell git rev-parse --is-inside-work-tree)
ifeq ($(ISGITREPO),true)
GITREF := $(shell git describe --exact-match --tags 2> /dev/null || echo "`git rev-parse --abbrev-ref HEAD`-`git rev-parse --short HEAD`" )
CPPFLAGS += -DMINIUPNPD_GIT_REF=\"$(GITREF)\"
endif
