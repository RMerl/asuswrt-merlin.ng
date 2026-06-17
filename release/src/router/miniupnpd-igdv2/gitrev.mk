# (c) 2019-2021 Thomas Bernard
# For GNU Make

# CI_COMMIT_TAG / CI_COMMIT_BRANCH / CI_COMMIT_SHORT_SHA are gitlab-ci
# predefined variables
# see https://docs.gitlab.com/ee/ci/variables/predefined_variables.html
# GITHUB_SHA / GITHUB_REF_NAME / GITHUB_REF_TYPE are variable defined by
# GitHub Actions
# https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables#default-environment-variables
ifneq ($(CI_COMMIT_TAG),)
GITREF = $(CI_COMMIT_TAG)
else ifneq ($(CI_COMMIT_BRANCH),)
GITREF = $(CI_COMMIT_BRANCH)-$(CI_COMMIT_SHORT_SHA)
else ifeq ($(GITHUB_REF_TYPE),tag)
GITREF = $(GITHUB_REF_NAME)
else ifeq ($(GITHUB_REF_TYPE),branch)
GITREF := $(GITHUB_REF_NAME)-$(shell echo "$(GITHUB_SHA)" | cut -c1-8)
else
ISGITREPO := $(shell git rev-parse --is-inside-work-tree)
ifeq ($(ISGITREPO),true)
# <tag> or <branch>-<short commit ref>
GITREF := $(shell git describe --exact-match --tags 2> /dev/null || echo "`git rev-parse --abbrev-ref HEAD`-`git rev-parse --short HEAD`" )
endif
endif

ifneq ($(GITREF),)
CPPFLAGS += -DMINIUPNPD_GIT_REF=\"$(GITREF)\"
endif
