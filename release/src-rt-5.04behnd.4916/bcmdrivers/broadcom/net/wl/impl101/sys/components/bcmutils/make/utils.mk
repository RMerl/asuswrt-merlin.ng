#
# Copyright (C) 2023, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual:>>

###############################################################################
# This make include file contains generic configuration and macros
# of potential use to any WCC makefile. Be thoughtful about namespace:
# macros should have a leading dot e.g. ".macro", internal variables
# should have a leading underscore, public vars are WLAN_*, etc.
#
# It should be possible to include this file in any other makefile
# with no effect other than to define the variables and macros it
# provides. In other words this file must not define any new rules,
# modify variables in a well-known namespace like CFLAGS, etc.
# No change which violates this rule should be accepted.
#
# Be careful defining macros: if the invocation has a line wrap e.g.:
#
# $(call .foobar,AAA BBB, \
#   CCC DDD)
#
# Whitespace will be prepended to the parameter, in this case $2.
# If that's a concern $(strip $2) must be used within the macro body.
###############################################################################

ifndef _utils_mk_
_utils_mk_ := 1

utils_mk := $(lastword $(MAKEFILE_LIST))
UtMkdir := $(abspath $(dir $(utils_mk)))/

# Default name of the dir at base of checkout containing build products.
# May be overridden at the command line.
WLAN_BLDDIR := build

# Conventional marker file dropped at the base of the checkout.
WLAN_BASE_MARKER := .wlbase

# Determine host type without needing a shell to invoke uname.
ifneq ($(CYGWIN),)
  WLAN_CYGWIN_HOST := 1
  WLAN_WINDOWS_HOST := 1
else ifneq ($(SYSTEMDRIVE),)
  WLAN_WINDOWS_HOST := 1
else ifneq ($(wildcard /Library),)
  WLAN_MACOS_HOST := 1
else
  WLAN_LINUX_HOST := 1
endif

# Custom variables potentially useful in complex recipes.
# E.g. $(^3) is the third prerequisite, $(^-1) is the last,
# $(^-2) is next to last, and $(^2-) is everything from 2nd through last.
.prereq = $(word $1,$^)
^1 = $(word 1,$^)
^2 = $(word 2,$^)
^3 = $(word 3,$^)
^4 = $(word 4,$^)
^5 = $(word 5,$^)
^6 = $(word 6,$^)
^7 = $(word 7,$^)
^8 = $(word 8,$^)
^9 = $(word 9,$^)
^-1 = $(word 1,$(call .reverse,$^))
^-2 = $(word 2,$(call .reverse,$^))
^-3 = $(word 3,$(call .reverse,$^))
^-4 = $(word 4,$(call .reverse,$^))
^-5 = $(word 5,$(call .reverse,$^))
^-6 = $(word 6,$(call .reverse,$^))
^-7 = $(word 7,$(call .reverse,$^))
^-8 = $(word 8,$(call .reverse,$^))
^-9 = $(word 9,$(call .reverse,$^))
^2- = $(wordlist 2,$(words $^),$^)

# Used in multiple places, defined here. Dev use only.
BINCMP_DFLAGS := -DBINCMP

# Name of the directory holding mogrified sources per customer.
mogsrc := SOURCE

# The standard way of defining $(space) and $(comma) variables.
# Also a few other occasionally helpful variables.
empty   :=
space   := $(empty) $(empty)
tab     := $(empty)	$(empty)
at      := @
colon   := :
comma   := ,
dollar  := $$
equals  := =
hash    := \#
lp      := (
rp      := )
dq      := "
sq      := '

# This variable evaluates to a single \n.
define newline
$(empty)
$(empty)
endef

# Potentially useful for makefiles needing to work on Windows (\) and Unix (/).
/ := /

# Derive the pid of the make process. Useful for uniqifying filenames.
# Invoke this just once and store the result in a := variable.
define .pid
$(shell echo $$PPID)
endef

# Split a string $1 into words by the separator $2 (default: comma).
# Usage: $(call .split,xx-yy-zz,-)
#   $1: text to be split by $2
#   $2: separator (default=,)
.split = $(subst $(or $(strip $2),$(comma)),$(space),$(strip $1))

# Join words in $1 into a string by the separator $2 (default: comma).
# Usage: $(call .join,word ...,-)
#   $1: text potentially containing whitespace
#   $2: separator (default=,)
.join = $(subst $(space),$(or $(strip $2),$(comma)),$(strip $1))

# Replace whitespace with underscores, primarily for comparing file contents.
# Usage: $(call .flatten,<text>)
#   $1: text potentially containing whitespace
.flatten = $(subst $(space),_,$(subst $(tab),$(space),$(subst \
  $(newline),$(space),$(strip $1))))

# Uniqify a list without sorting it.
# Usage: $(call .uniq,$(list))
#   $1: list to be uniqified
# This walks through the input list recording whether each element has
# been seen yet and echoing it if not. For each element "X" a variable
# ".uniq_X" is created to mark that X has been seen. However, this would
# only be safe the first time the macro is used since if we uniqify the
# list "A B C" and later "C D E", C would be recorded in the first use
# and incorrectly dropped from the second. Therefore each call to this
# macro increments a counter and the marker variable actually becomes
# ".uniq_X_N" where N is the unique counter value.
define .uniq
$(strip $(eval _$0 += .)$(foreach x,$1,$(if \
  $($0_$x_$(words $(_$0))),,$x$(eval $0_$x_$(words $(_$0)) := 1))))
endef

# Remove words containing the $2 substring from the $1 list.
# Analogous to $(filter ...) but does not require a % pattern match;
# if the substring is present anywhere in the word it's filtered out.
# Usage: $(call .filter_out_substr,$(list))
#   $1: substring to look for
#   $2: list to be filtered
define .filter_out_substr
$(strip $(foreach x,$2,$(if $(filter $x,$(subst $1,,$x)),$x)))
endef

# Break a file path into a sorted list of its component subdirectories.
# Usage: $(call .subdirs,path)
#   $1: path to a file
# E.g. $(call .subdirs,A/B/C/D/file.c) returns "A A/B A/B/C A/B/C/D".
define .subdirs
$(strip \
  $(sort $(foreach p,$1,$(filter-out .,$(if $(filter .,$p),,$(call \
    $0,$(patsubst %/,%,$(dir $p))) $(patsubst %/,%,$(dir $p)))))))
endef

# Insert a string between the dir and file parts of each path.
# Usage: $(call .insert,<paths>,<string>)
# E.g. $(call .insert,basedir/aaa basedir/bbb,blah/) returns
# "basedir/blah/aaa basedir/blah/bbb".
.insert = $(patsubst ./%,%,$(foreach p,$1,$(dir $p)$2$(notdir $p)))

# Reverse the order of a list.
# Usage: $(call .reverse,A B C D)
# returns "D C B A"
#   $1: list to be reversed
# NOTE: this recursive macro has been seen to cause a *lot* of
# realloc-ing when reversing very long lists. Dozens: fine.
# Hundreds: probably ok. Thousands: nyet.
.reverse = $(strip $(if $(wordlist 2,2,$(1)),$(call $0,$(wordlist \
  2,$(words $(1)),$(1))) $(firstword $(1)),$(1)))

# Macros to die or warn with a standard-format error message.
.die = $(error $(strip Error: $1))
.warn = $(warning $(strip Warning: $1))
.warning = $(call .die, use .warn() for warnings)

####################################################################
# Macros for make debugging, e.g. $(call .show,VAR1 VAR2 VAR3)
####################################################################
# Track [recursion-depth/#-of-restarts] for use in debugging messages.
_ut_pfx := $(if $(filter 0,$(MAKELEVEL)$(MAKE_RESTARTS)),,[$(MAKELEVEL)$(if \
  $(MAKE_RESTARTS),/$(MAKE_RESTARTS))])
ut_pfx := $(if $(_ut_pfx),$(_ut_pfx)$(space))
# A conventional marker for easy grepping.
_dbgmark := $(ut_pfx)=-=
# Print a custom message to stderr with the standard marker.
.dump = $(warning $(_dbgmark) $1)
# Print values of each provided variable to stdout, with marker.
# The flavor and origin of each variable is shown within [].
.show_i = $(foreach v,$1,$(info \
  $(_dbgmark) $v="$($v)" [$(flavor $v)/$(origin $v)]))
# As above but print to stderr giving file and line. If $1 is empty,
# give only file and line.
.show_w = $(if $1,$(foreach v,$1,$(warning \
  $(_dbgmark) $v="$($v)" [$(flavor $v)/$(origin $v)])),$(warning \
  $(_dbgmark) REACHED THIS LINE))
# As above but abort afterward.
.show_e = $(if $1,$(call .show_w,$1)$(error EXIT),$(error \
  $(_dbgmark) REACHED THIS LINE))
# Convenience alias.
.show = $(.show_w)
# Show every assigned variable (!) with its value and type.
.showvars = $(warning $(_dbgmark) SHOWVARS$(MAKE_RESTARTS):)$(foreach \
  v,$(sort $(filter-out .%,$(.VARIABLES))),$(warning $(_dbgmark) $v$(if \
  $(filter simple,$(flavor $v)),:=,=)"""$($v)"""))

# Convenience wrapper over the two-stage method for deriving a
# clean (no trailing slash) dirname, even though we generally
# prefer to keep the trailing slash.
# Usage: $(call .dirname,<path>)
#   $1: directory path
.dirname = $(patsubst %/,%,$(dir $(patsubst %/,%,$1)))

# Chop off all trailing / from each argument.
# Usage: $(call .chopd,dir ...)
#   $1: list of dirs which should have a trailing / removed
.chopd = $(strip $(if $(filter $1,$(patsubst \
  %/,%,$1)),$1,$(call $0,$(patsubst %/,%,$1))))

# Standardize each arg dir to have a single / at the end.
# Suppress multiple successive / in path except leading // (UNC path)
# under Windows (where $(OS) is set).
# Usage: $(call .stdd,dir ...)
#   $1: list of dirs which should have a trailing /
.stdd = $(strip $(foreach p,$1,$(if $(and \
  $(OS),$(filter //%,$p)),/)$(subst \
  //,/,$(subst //,/,$(if $p,$(addsuffix /,$(call .chopd,$p)))))))

# Make the specified directory paths absolute, which has the side effect
# of normalizing them to remove double slashes, trailing slashes, and
# runs of "..", then append a single trailing / to each.
# Usage: $(call .absd,dir)
#   $1: list of dirs to be made absolute and normalized
.absd = $(strip $(foreach p,$1,$(addsuffix /,$(abspath $p))))

# Expand the $2 text as many times as requested by $1.
# Usage: list := $(call .for,10,text ...)
#   $1: number of expansions requested
#   $2: text to expand $1 times
.for = $(if $1,$(foreach i,$(wordlist $(or $3,1),$1,$(_sh_vblevels)),$2))

# These settings support (but do not require) a verbosity scheme in which
# recipes are made silent by default using .SILENT and V=<level> is used
# to crank up verbosity per target/recipe by selectively enabling the
# shell's -x mode. It also supports V=-1 which is regular, naive make
# verbosity (no .SILENT, no @ prefixes, no -x mode).
# The $Q variable is provided because some makefiles may choose to use
# .SILENT while others may want line by line verbosity suppression via
# "@" (.SILENT propagates to recursive makes prior to 4.3, "@" does not).
# Both may be used.

# List of supported verbosity levels (VLs):
_sh_vblevels := 1 2 3 4 5 6 7 8 9
Q := $(if $(filter-out -1 9,$V),@)
ifeq ($V,-1)
  .vb =
else # V
  # Usage: $(call .vb,3[,message[,linenumbers]])
  # This is the main verbosity control macro. If $V < $1 it evaluates to null.
  # If $V >= $1 and no $2 is passed it returns 1; if $2 is given it's a
  # message to be printed with either $(info $2) or $(warning $2) depending
  # on the presence of $3.
  .vb = $(strip $(if $(filter $1,$(wordlist 1,$(or \
    $V,1),$(_sh_vblevels))),$(if $2,$(if $3,$(warning $2),$(info $2)),1)))
  $(if $V,$(if $(filter $V,0 $(_sh_vblevels)),,$(call \
    .die,unknown verbosity level V=$V)))
endif # V

# This "verbose-target" macro limits shell verbosity per target
# according to VL, on the assumption that make verbosity is turned
# off with .SILENT. A value of N means that the named rule becomes
# verbose when $V >= N.
# Usage: $(call .vbt,<target>,5)
#   $1: GNU make target or pattern
#   $2: $V level at which <target> becomes verbose (default=4)
.vbt = $(eval $1: xflag := $(if $(call .vb,$(or $2,4)),-x))

# This "verbose-begin" macro prints "$1" iff summary verbosity is requested.
# With MAKE_DEBUG=1 <path>:<line> data is prepended.
# Usage: $(call .vbb,Generating $@)
#   $1: message to print (default="Making $(@F)")
#   $2: override default vb level for this message
.vbb = $(if $(filter $(or $2,1 2 3 9 -1),$(or \
  $V,1)),$(eval $$($(if $(MAKE_DEBUG),warning,info) $$(or \
    $1,Making $$(@F))$(if $(filter $(or $2,2 3 9 -1),$(or \
      $V,1)),$(space)DUE TO $$(or $$?,.PHONY)))))

# Usage: $(call .vbflag,<vbflag>,<floor>)
#   $1: flag to be passed N times when $V >= $2
#   $2: level to trigger this verbosity (default: 2)
# Expands to as many <vbflag> options as the value of ($V - $2) iff $V >= $2
# where $2 defaults to 2.
.vbflag = $(call .for,$(subst -1,10,$(or $V,1)),$1,$(or $2,2))

# For explicit optional shell verbosity eg "$(call .shell-e,$(shell-x) cmd...)".
shell-x = $(if $(call .vb,4),set -x;)

# Usage: $(call .true,FOOBAR)
# Return 1 iff $(FOOBAR) is equal to 1 plus potential whitespace.
# Takes the *name* of the variable, e.g. FOOBAR not $(FOOBAR).
# Preferred to "ifeq ($(var),1)" because it can be a one-liner and
# because the $(strip ...) handles trailing whitespace from comments.
# New, preferred name:
.true = $(if $(filter "1","$(strip $($(strip $1)))"),1)
# Older name:
.is1 = $(.true)

# Behave like $(shell ...) but abort make if the shell exits nonzero.
# Defaults to "pipefail" mode; use an explicit "set +o pipefail" to override.
# Usage: $(call .shell-e,<command>)
#   $1: command text passed to shell
.shell-e = $(shell $(shell-x)set -o pipefail;$1)$(if $(filter \
  $(.SHELLSTATUS),0),,$(error Error: [$(.SHELLSTATUS)] "$1"))

# This macro calls $(eval $1) and will do so verbosely if $V >= $2.
# Usage: $(call .evalx,<text>[,vblevel])
#   $1: text passed to the $(eval ...) function
#   $2: verbosity level at which <text> is also echoed (default=6)
#   $3: optional string to bracket the optional verbosity
define .evalx
  $(if $(strip $1), \
    $(if $(call .vb,$(or $2,6)), \
      $(and $3,$(info $3)) \
      $(warning $$(eval $(strip $1))) \
      $(and $3,$(info $3)))) \
  $(eval $1)
endef

# Die if any of the specified variables are null.
# A variable may properly be null until the nth recursive pass,
# in which case $3 can be used to skip earlier passes.
# Usage: $(call .assert,var1 var2 ...)
#   $1: list of variable names to check
#   $2: optional message to die with
#   $3: list of $(MAKELEVEL) values to skip
.assert = $(if $(if $3,$(filter-out $3,$(or $(MAKELEVEL),0)),1),$(foreach \
  _var,$1,$(if $($(_var)),,$(call .die,$(or $2,$$($(_var)) is null)))))

# Die if any of the specified paths does not exist.
# Usage: $(call .assert_exists,var1 var2 ...)
#   $1: list of paths to check
#   $2: optional message to die with
.assert_exists = $(foreach p,$1,$(if $$(wildcard $p),,$(call \
  .die,$(or $2,$(abspath $($p)): no such file or directory))))

# Perform a standard GNU make assignment with optional verbosity.
# Usage: $(call .assign,<name>,<value>,[<verbosity-var>],[<assignment-type>])
# Or:    $(call .assign,<assignment>,,[<verbosity-var>])
# When used as "$(call .assign,FOO,bar)" it will assign FOO:=bar.
# When used as "$(call .assign,FOO:=bar)" it will do the same thing.
# In either form, if $3 is provided it's the name of a variable which
# may enable verbosity. In the first form an assignment type other
# than ":=" may be provided as $4. The second form supports only ":="
# and disallows $4.
# A comment may be supplied as $5; it will be added to the verbosity.
#
# The macro is complex but its usage is simple; $(call .assign,foo,bar)
# is basically the same as "foo := bar". The value-added by this macro
# is the potential for debug verbosity. However, a brief explanation is
# that the variable $_ is used to represent the passed-in variable name
# so it follows that $($_) will evaluate to its value. The variable name
# "__$_" holds the previous value of $_ (if any) in order to provide a
# warning if it changes. That value will be $(__$_).
define .assign
$(call .evalx,
  $(if $2,
    _=$1
    __$1 := $($1)
    $1$(or $4,:=)$2
  ,
    $(if $4,$(error Error: $0: use of $$4 without $$2))
    _=$$(firstword $$(subst :=,$$(space),$1))
    __$_ := $($_)
    $1
  )
)
$(if $(MAKE_RESTARTS),$(if $($3),$(if $(filter $(__$_),$($_)),,
$(if $($_),$(warning $(_dbgmark) $0 $_$(or $4,:=)$($_)$(if $5,$(space)($5))))
  $(if $(__$_),$(warning
  Warning: $_ $0ed from "$(__$_)" to "$($_)"$(if $5,$(space)($5)))))
))
endef # .assign

# Die or warn if the specified variable contains an unsorted list.
# Usage: $(call .require_sorted,var-name,[warn],[verbose])
#   $1: *name* of variable to test
#   $2: if non-null, unsorted becomes just a warning
#   $3: print the incorrectly-sorted data if non-null
# Note that $1 is a variable *name*; expansion is handled internally.
define .require_sorted
  $(if $(filter -$(subst \
    $(space),-,$(strip $($1)))-,-$(subst \
    $(space),-,$(strip $(sort $($1))))-),,$(call $(if
      $2,.warn,.die),$$($1) not uniquely sorted$(if $3, ($($1)))))
endef

# Compare two dotted numeric strings (e.g 2.3.16.1) for str1 >= str2.
# Usage: $(call .version_ge,str1,str2)
#   $1: string 1
#   $2: string 2
#   $3: optional string to return if true [yes]
.version_ge = $(shell echo -e '$1\n$2' | \
  sort -cnu -t. -k1,1 -k2,2 -k3,3 -k4,4 -k5,5 2>/dev/null || echo $(or $3,yes))

# Return an absolute path (requires use of cygpath on Cygwin).
# Usage: $(call .abspath,<path>)
#   $1: path to be made absolute
define .abspath
$(if $(WLAN_CYGWIN_HOST),$(call .shell-e,$(strip \
  cygpath -a $2 $1)),$(abspath $1))
endef

# Add a maximum of 20 integral values (negatives allowed).
# Usage: $(call .sum,x,y,z,...)
#   $1 ... $20: list of numeric values to add up
.sum = $(call .shell-e,echo $$(($(foreach x,$1 $2 $3 $4 $5 $6 $7 $8 $9 $(10) \
  $(11) $(12) $(13) $(14) $(15) $(16) $(17) $(18) $(19) $(20),+$(or $x,0)))))

# This macro searches up recursively from the dir named by $1 until it
# finds a parent dir containing one of the files named in $2 and returns
# that path.
# Usage: $(call .find_up,<start-dir>,<files>...)
#   $1: directory to start search from
#   $2: list of file names, any of which will end search
define ..find_up
$(if $(wildcard $(addprefix $(or $1,.)/,$2)),$1,$(if $(wildcard \
  $(or $1,.)/etc/rc.d),$(call \
  .warn,$2: not found in $0 search),$(call $0,$(or $1,.)/..,$2)))
endef
.find_up = $(strip $(call .stdd,$(if $1,$(call \
  .$0,$1,$2),$(patsubst ./%,%,$(call .$0,$1,$2)))))

# Usage: $(call .generate_mkdir_rules,<target-dir-list>,<exclusions>)
#   $1: list of directories which may need creating
#   $2: list of directories to exclude from $1
#
# Tell make how to create the directory set specified as $1. This
# does not create any dirs itself, it simply generates rules to do
# so. Each required directory is given an order-only dependency on
# its parent recursively to ensure they'll be created on demand in
# an ordered, threadsafe way.
#
# Makefiles which use this macro should give targets an order-only
# dependency on their parent dir e.g.:
#   <target>: <prereqs>... | $$(@D)
# in order to trigger the logic generated here. This in turn requires
# that .SECONDEXPANSION be in effect which is our convention.
#
# Directory dependencies are generated both with and without a trailing
# slash for convenience, because make does lexical matching and thus
# doesn't recognize "dir" and "dir/" as the same path.
#
# Because this macro may be used in multiple places there's a risk of
# generating redundant rules to create common parent directories and
# unfortunately make gives spurious warnings in that case. Therefore
# the variable $(mkdir_rules_done) is used to cache directories which have
# already been processed. Directories are recorded internally with
# no trailing slash but filtering is done both with and without it.
# Note: this variable should logically be private but it's exposed
# (no underscore) because certain builds which are sometimes standalone
# and sometimes not, such as ucode, may want to check it.
#
# If there are directories in this hierarchy which should NOT have a
# rule generated, presumably because a custom rule is written elsewhere,
# they may be listed as exclusions in $2.
#
# Note that despite the fact that this design creates directories one
# by one in a threadsafe way it still uses mkdir -p. This is to account
# for the possibility of multiple make processes being run in parallel
# in the same checkout for different architectures, in which case they
# would not share a DAG and could still collide in creating a base dir.
# GNU mkdir has a builtin retry feature for such collisions but other
# mkdir programs may not.
define .generate_mkdir_rules
  $(call .evalx, \
    mkdir_rules_done := $$(sort $$(mkdir_rules_done) $(2:%/=%))
    _targdirs := $$(sort $(1:%/=%))
    _targdirs := $$(sort $$(_targdirs) $$(foreach d,$$(_targdirs), \
      $$(call .subdirs,$$d)))
    _targdirs := $$(filter-out . .. ../.. ../../.. ../../../..,$$(_targdirs))
    _targdirs := $$(filter-out ../../../../.. ../../../../../..,$$(_targdirs))
    _targdirs := $$(filter-out \
      $$(mkdir_rules_done) $$(mkdir_rules_done:%=%/),$$(_targdirs))
    mkdir_rules_done := $$(sort $$(mkdir_rules_done) $$(_targdirs))
    $$(foreach d,$$(_targdirs),$$(eval $$d: | $$(patsubst %/,%,$$(dir $$d))))
    $$(sort $$(_targdirs) $$(_targdirs:%=%/)):; +mkdir -p $$(@:%/=%)$3
  )
endef # .generate_mkdir_rules

# These two functions can work together to WAR a weakness in gcc
# command line parsing. Unfortunately a few gcc flags require a
# space between option and argument: -D, -U, -I allow "cuddling"
# but "-imacros foo.h" does not, and gcc doesn't implement the
# --foo=bar style either. Whitespace can cause trouble with sorting
# etc so one solution is bind them together with "=" for makefile
# purposes and split them back apart before passing to the shell.
# Update: it turns out that -imacros etc *can* be cuddled though
# it's really obfuscated: "-imacrosfoobar.h". Thus something like
# "-imacros./foobar.h" is recommended to make split/join unnecessary
# and be at least a bit more readable.

# Usage: $(call .gcc_join,<compiler-command-line>)
define .gcc_join
$(patsubst \
  -imacros$(space)%,-imacros=%,$(patsubst \
  -include$(space)%,-include=%,$(patsubst \
  -MF$(space)%,-MF=%,$(patsubst \
  -MT$(space)%,-MT=%,$(patsubst \
  -o$(space)%,-o=%,$1)))))
endef

# Usage: $(call .gcc_split,<compiler-command-line>)
define .gcc_split
$(patsubst \
  -imacros=%,-imacros$(space)%,$(patsubst \
  -include=%,-include$(space)%,$(patsubst \
  -MF=%,-MF$(space)%,$(patsubst \
  -MT=%,-MT$(space)%,$(patsubst \
  -o=%,-o$(space)%,$1)))))
endef

# This is a useful macro to wrap around a compiler command line, e.g.
# "$(CC) $(call .ccline,$(CFLAGS)) -o foo.o foo.c". It makes the command
# line more readable, re-sorts and uniqifies those flags for which it can
# be done safely (-D, -U) while taking care not to change ordering that
# matters. Requirement: all flags which take an argument such as -[DUI],
# must be "cuddled"; -imacros/-include/etc may use an artificial = in
# place of whitespace.
# Usage: $(call .ccline,-c -DFOO -Ixx/include -DBAR -Iyy/include -g)
#   $1: compiler command line or fragment thereof
define .ccline
$(strip $(call .gcc_split, \
  $(filter -imacros=% -include=%,$1) \
  $(filter -O%,$1) \
  $(filter -g%,$1) \
  $(filter-out \
    -imacros=% -include=% -f% -g% -m% -O% -W% -D% -U% -I% @% -o=%,$1)) \
  $(filter -f%,$1) \
  $(filter -m%,$1) \
  $(filter -W%,$1) \
  $(sort $(filter -D%,$1)) \
  $(sort $(filter -U%,$1)) \
  $(filter -I%,$1) \
  $(filter @%,$1) \
  $(filter -o=%,$1))
endef

# Compare the contents of the file named by $1 with the proposed new
# contents $2. If they differ, or if $1 doesn't exist, force $1 to be
# rebuilt by marking it phony thus causing everything which depends on
# it to rebuild also.
# Optional $3 and $4 args are allowed for data which cannot go in file $1
# since it may vary per object or similar. The state in $4 is maintained
# in file $3 and triggers a (pessimistic) rebuild on any change. Any data
# (compiler flags, list of object files, etc) may be used in $4.
#
# Usage: $(call .rebuild_if_changed,<flags-file>,<flags>)
#   $1: path to file containing control data
#   $2: data to be placed in $1
#   $3: path to file containing additional data
#   $4: data to be placed in $3
define .rebuild_if_changed
  $(if $(filter-out $(call .flatten,$2),$(call .flatten,$(file < $1))), \
    $(info $1 has changed, forcing rebuild)$(eval .PHONY: $1), \
      $(if $3, \
        $(if $(filter-out $(call .flatten,$4),$(call .flatten,$(file < $3))), \
          $(info flags changed, forcing rebuild of $1)$(eval .PHONY: $1))))
endef

# Change case without requiring a shell.
# Usage: $(call .tolower,<string>)
#   $1: string to be lowercased
.tolower = $(strip \
  $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e, \
  $(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k, \
  $(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q, \
  $(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w, \
  $(subst X,x,$(subst Y,y,$(subst Z,z,$1)))))))))))))))))))))))))))
# Usage: $(call .toupper,<string>)
#   $1: string to be uppercased
.toupper = $(strip \
  $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E, \
  $(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K, \
  $(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q, \
  $(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W, \
  $(subst x,X,$(subst y,Y,$(subst z,Z,$1)))))))))))))))))))))))))))

# Return the relative path from <from-dir> to <to-dir> .
# <from-dir> may be elided in which case it defaults to $(CURDIR).
#
# Usage: $(call .relpath,[<from-dir>,]<to-dir>)
#
._rp_compose = $(subst $(space),$(strip $1),$(strip $2))
._rp_endlist = $(wordlist $1,$(words $2),$2)
._rp_canonpath = $(abspath $1)
# .....relpath:  Recursive macro which compares the first element
#                of two given paths, then calls itself with the next two
#                elements, and so on, until a difference is found.
#                At each step, if the first element of both paths matches,
#                that element is produced.
.....relpath = $(if $(filter $(firstword $1),$(firstword $2)), \
                $(firstword $1) \
                $(call $0,$(call ._rp_endlist,2,$1),$(call \
                ._rp_endlist,2,$2)) \
                )
# ....relpath:   This macro removes $1 from the front of both $2 and
#                $3 (removes common path prefix) and generates a relative
#                path between the locations given by $2 and $3, by
#                replacing each remaining element of $2 (after common
#                prefix removal) with '..', then appending the remainder
#                of $3 (after common prefix removal) to the string
#                of '..'s
....relpath  = $(foreach e,$(subst /,$(space),$(patsubst $(if \
                $1,/)$1/%,%,$2)),..) $(if $3,$(patsubst $(if \
                $1,/)$1/%,%,$3))
# ...relpath:    This macro runs the output of .....relpath() through
#                ....relpath(), and turns the result into an actual
#                relative path string, separated by '/'.
...relpath    = $(call ._rp_compose,/, \
                  $(call .$0,$(call ._rp_compose,/,$(call \
                    ..$0,$3,$4)),$1,$2) \
                )
# ..relpath:     This macro makes a determination about the two given
#                paths: does one strictly prefix the other?  If so, this
#                macro produces a relative path between the two inputs,
#                without calling ...relpath() and taking the "long road".
#                If $1 prefixes $2, the result is the remainder of $2
#                after removing $1.  If $2 prefixes $1, the result is the
#                remainder of $1 after removing $2, but with each element
#                in that remainder converted to '..'.
..relpath     = $(if $(filter $1,$2),., \
                $(if $(filter $1/%,$2), \
                  $(patsubst $1/%,%,$2), \
                  $(if $(filter $2/%,$1), \
                    $(call ._rp_compose,/, \
                      $(foreach e,$(subst \
			/,$(space),$(patsubst $2/%,%,$1)),..) \
                    ), \
                    $(call .$0,$1,$2,$(subst \
                      /,$(space),$1),$(subst \
                      /,$(space),$2)) \
                  ) \
                ) \
              )
# .relpath:       This macro loops over each element in $2, calculating
#                 the relative path from $1 to each element of $2.
.relpath     = $(if $1,,$(error \
                Error: missing first parameter to $0))$(strip $(if $2, \
                $(foreach d,$2,$(call .$0,$(call \
                ._rp_canonpath,$1),$(call ._rp_canonpath,$d))), $(foreach \
                d,$1,$(call .$0,$(call ._rp_canonpath,${CURDIR}),$(call \
                ._rp_canonpath,$d))) \
                  ) \
                )

# This is a wrapper over the regular include statement. It started out as
# a pretty thin wrapper intended only to provide visual distinction from
# the word "include" which also appears in -I paths etc. but over time a
# number of other useful features have been added:
#
# 1. Visual distinction, as above.
# 2. Verbosity. A sufficient value of $V will show includes as they happen.
#    If $1 is a relative path "from $(CURDIR)" is appended to the message.
# 3. Allow a synchronous warning when the file is not found, whereas a
#    direct make include would defer the message and may never show it if an
#    $(error ...) is encountered first.
# 4. Automatic include-guarding. The include is skipped if $1 is present
#    in MAKEFILE_LIST, i.e. has already been included. Simpler and more
#    scalable than putting traditional include guards in each file.
#
# The Usage example below includes <filename> and gives a synchronous
# warning if not found; passing "error" would change the warning to an
# immediate fatal error. Passing nothing leads to default behavior in
# which no message is printed until after a recipe is found and run.
#
# Usage: $(call .include,<filename>,,warning)
#   $1: pathname of file to include.
#   $2: may be "-" to use "-include".
#   $3: may be "warning" or "error", or null.
#   $4: force re-include even if $1 has already been included.
#
# All parameters after $1 are optional.
#
define .include
  $(if $(strip $1), \
    $(if $(filter $(and $4,!)$(abspath $1),$(abspath $(MAKEFILE_LIST))), \
      $(call .vb,3,$(if $(filter \
        0,$(MAKELEVEL)),,[$(MAKELEVEL)] )NOT $(if $2,-)Including $1$(if \
        $(filter /%,$1),, from $(CURDIR)),1), \
      $(call .vb,3,$(if $(filter \
        0,$(MAKELEVEL)),,[$(MAKELEVEL)] )$(if $2,-)Including $1$(if \
        $(filter /%,$1),, from $(CURDIR)),1) \
      $(eval $(if $2,-)include $1) \
      $(if $3,$(if $2,,$(if $(filter $(abspath $1),$(abspath \
	$(MAKEFILE_LIST))),,$(eval $$($3 $3: $1: include file not found)))))))
endef

# Identical to .include except that $2 defaults to "-". By analogy with
# GNU make's "sinclude" compatibility name.
define .sinclude
  $(call .include,$1,-,$3,$4)
endef

# Naturally, the .include macro cannot be used to include the file which
# defines it. Thus verbosity for inclusion of this file is handled here.
$(call .vb,6,$(if $(filter \
  1,$(MAKELEVEL)),,[$(MAKELEVEL)] )Including from "$(abspath \
  $(word 2,$(call .reverse,$(MAKEFILE_LIST))))",1)

# This is a custom include statement which removes comments that may lead to
# extraneous whitespace. E.g. "MYVAR = 1    # ..."  causes MYVAR to be
# defined as "1    ". When this happens any string comparison such as
# "ifeq ($(MYVAR),1)" will fail which can lead to very subtle bugs. The
# 's/$/@@@@/' inserts @@@@ at the end of each line (before \n) and the
# $(shell ...) function converts \n to space so we use $(subst @@@@ ,@@@@,...)
# to remove the space before $(subst @@@@,$(newline),...) replaces
# @@@@ back with \n.
# 1. ':x; /\\[[:space:]]*$/ { N; s/\\[[:space:]]*\n//; tx }' combines
#    continued lines
# 2. '/^[[:blank:]]*#/d' deletes comment only lines
# 3. 's/[[:blank:]]*#.*//' removes trailing # ... and spaces before it
# 4. 's/$/@@@@/' inserts @@@@ before \n
# We also have to do some fancy stepping to handle no-such-file errors
# from this; if the shell fails it leaves a marker file which make later
# finds/reports/removes. Last, note that all errors from the include file
# will be reported from the line of inclusion in the parent.
# TODO: *may* be obsolete-able since the .true() macro strips whitespace
# but that would require changing all ifeq($(FOOBAR),1) tests to use .true.
# TODO: even if still needed, since 4.2.1+ is now the baseline it may be
# possible to rewrite this using $(file ...) instead of $(shell ...)
# which would be simpler, safer, and faster.
define ._load_cfg_file
MAKEFILE_LIST += $(abspath $(firstword $(wildcard $(addsuffix \
  /$1,$(MAKE_INCLUDE_PATH)) $1)))
$(subst @@@@,$(newline),$(subst @@@@ ,@@@@,$(shell set +x; sed \
  -e ':x; /\\[[:space:]]*$$/ { N; s/\\[[:space:]]*\n//; tx }' \
  -e '/^[[:space:]]*\#/d' \
  -e 's/[[:space:]]*\#.*//' \
  -e 's/$$/@@@@/' $(firstword $(wildcard \
    $(addsuffix /$1,$(MAKE_INCLUDE_PATH))) $1) 2>/dev/null || touch $1.FAIL)))
endef

# See full comments at helper macro above.
# Usage: $(call .include_cfg_file,filename)
#   $1: path of file to include
define .include_cfg_file
$(call .vb,3,INCLUDING $1,1) \
$(eval $(call ._load_cfg_file,$1)) \
$(if $(wildcard $1.FAIL),$(shell $(RM) $1.FAIL)$(call \
  .die,$1: no such file or directory))
endef

# There's a surprising and undocumented behavior, arguably a bug, in GNU
# make which they cannot fix for compatibility reasons even though nobody
# remembers why it exists: variables overridden on the command line are
# implicitly exported. Exported make vars are deprecated because they
# can cause trouble in recursive use so this macro will unexport them
# immediately. Explicit exports are also discouraged.
#
# Usage: $(call .unexport_overrides)
#
define .unexport_overrides
  $(call .evalx, \
    unexport $$(foreach x,$$(MAKEOVERRIDES),$$(firstword $$(call .split,$$x,=)))
  )
endef # .unexport_overrides

# Require GNU make 4.2.1 as a floor version.
# Usage: $(call .make_4.2.1+_required)
define .make_4.2.1+_required
$(if $(filter 3.% 4.0 4.1% 4.2,$(MAKEVER)),$(error \
  Error: GNU make 4.2.1+ required))
endef

# Declare the invoking make to be one that follows our standard/preferred
# conventions for strictness, verbosity, etc. Details:
# - A default phony goal of "all" is an industry-wide convention.
# - Our verbosity model is driven by $V and may make recipes silent by default.
#   However, this cannot be done where the Linux kernel makefile system is
#   involved because it doesn't use our system so a $1 flag is supported
#   to suppress .SILENT (note: GNU make 4.4 may fix this).
# - Strictness is cranked up with .DELETE_ON_ERROR.
# - Our convention is to have .SECONDEXPANSION on for all targets.
# - Builtin defaults are cleared out with .SUFFIXES and -r.
# - Setting SHELL (and .SHELLFLAGS) disables fast-path but it's worth it to
#   get consistent verbosity behavior and shellflag enhancements.
# - .SHELLFLAGS gets extra flags to make recipes stricter: the shell is
#   always invoked with -e (errexit) and -u (nounset) to abort on unassigned
#   shell variables. Also pipefail becomes the default so pipes don't lose
#   exit status. The -x flag is varied on a per-target basis by the
#   .vbt macro unless V=9 in which case it's always on.
# - The DEVMODE (developer mode) flag is defaulted to 1. DEVMODE affects
#   mogrification and is documented over there. It also toggles generation
#   of dependency (*.d) files.
# - Force the locale and in particular sorting to "C" during the build.
#   Can't risk having subtle variations from user to user based on
#   their personal locale settings.
# - The BRCM LSF infrastructure has a way of passing LD_LIBRARY_PATH
#   values from the submission host to the execution host which can
#   cause problems if they're different arch types. Strip LSF-added paths
#   at the start of the build.
#
# Usage: $(call .bcmstd[,no_silent])
#   $1: suppress .SILENT default if non-null
#
define .bcmstd
  $(eval
    .DEFAULT_GOAL ?= all
    .PHONY: all

    # Remove any cruft added to this EV by LSF.
    ifneq ($$(LD_LIBRARY_PATH),)
      LD_LIBRARY_PATH := $(subst $(space),$(colon),$(filter-out \
        /tools/lsf/%,$(subst $(colon),$(space),$(LD_LIBRARY_PATH))))
    endif

    ifeq ($$(filter -1 9,$$V),)
      ifeq ($1,)
        .SILENT:
      else
        Q := @
      endif
    endif # V == -1|9

    ifeq ($$(filter r,$$(firstword $$(MAKEFLAGS))),)
      MAKEFLAGS += -r
    endif

    .DELETE_ON_ERROR:
    .SECONDEXPANSION:
    .SUFFIXES:

    SHELL := /bin/bash
    .SHELLFLAGS = $$(strip \
      $$(if $$(filter 9,$$V),-x,$$(xflag)) -e -u -o pipefail -c)

    # Tuned for developer use by default. Automated builds should override.
    # Allow dev mode to be controlled from the environment.
    DEVMODE ?= 1

    # This turns so-called "smart quotes" in gcc toolchain output back
    # to dumb ones which are much preferred and more standard.
    export LC_ALL := C

    # There are places (e.g. *.flist) where we require a stable sort order.
    # Standardize on C sorting in case the user has a personal locale.
    export LC_COLLATE := C

    # These are annoying and can in some cases be left behind by other
    # users and be hard to remove for umask reasons. We haven't seen
    # a significant speed difference between having them and not.
    # But currently commented out for caution.
    # export PYTHONDONTWRITEBYTECODE ?= 1
  )
endef # .bcmstd

define vb_usage
Verbosity is configured by passing V=n. Levels other than -1 are incremental.
  V=0:   Minimal output
  V=1:   Summary info only e.g. "Compiling ..." messages. (DEFAULT)
  V=2:   Add "DUE TO" explanation of rebuild reasons ($$?)
  V=3:   Add extra artificial messages: PATH=, FEATURES=, etc
  V=4:   Add full command lines for all recipes
  V=5:   Additional rule debugging data such as showing timestamps
  V=6:   Add makefile-debugging data such as $$(eval ...) commands
  V=7:   Reserved for future use
  V=8:   Reserved for future use
  V=9:   All available verbosity (includes -1 output)
  V=-1:  Standard GNU make verbosity plus summary info
endef # vb_usage

# Translate our V=[0-9] verbosity model to the Linux kernel's
# recursive make system which understands only V= and V=1.
# Usage: $(call .vx2lx,$V[,suppress])
#   $1: should be literally $V or $(V)
#   $2: do not generate anything for V=0 and V=1 if set.
define .vx2lx
  $(strip $(if $(filter 0 1,$1),$(if $2,,-s V=),$(if \
    $(filter 9,$1),V=1 .SHELLFLAGS="-x $(.SHELLFLAGS)",$(if \
      $1,V=1))))
endef # .vx2lx

# Figure out who we are (default to "brcm" but allow override).
_ourselves_mk := $(dir $(utils_mk))ourselves.mk
ifeq ($(wildcard $(_ourselves_mk)),)
  ourselves := brcm
else  # ourselves.mk
  $(call .include,$(_ourselves_mk),,error)
endif # ourselves.mk

# Derive the set of customers.
_mog_cfg_dir := $(dir $(utils_mk))../../mogtools/cfg
_customers := $(sort $(filter-out _% $(ourselves), \
  $(basename $(notdir $(wildcard $(_mog_cfg_dir)/*.mog)))))
customers := $(sort $(or $(call .split,$(CUSTOMERS)),$(_customers)))

# Usage: $(call .mogsel,<devmode-dir>,<prod-dir>)
#   $1: value to return if dev mode is ON
#   $2: value to return if dev mode is OFF
# Select $1 or $2 according to the value of DEVMODE.
.mogsel = $(strip $(if $(filter 1,$(DEVMODE)),$1,$2))

# Usage: $(call .do_python,script)
# Run script using python3.
#   $1: script text
.do_python = $(call .shell-e,python3 -c '$1')

# Usage: $(call .print_hex,num)
# Print number in hex using python.
#   $1: number to format as hex
.print_hex = print("0x%x" % ($1))

# Usage: $(call .revid2mask,bitmask,uc_revid)
#   $1: bitmask
#   $1: uc_revid
# Add uCode revid major bitmask to full bitmask of arbitrary size.
# uCode revid is a number, possibly floating point: major.minor; use major
# (integer) part. Return updated bitmask (in hex for easier debugging.)
.revid2mask = $(call .do_python,$(call .print_hex,$1 | 1 << $(firstword \
  $(subst .,$(space),$2))))

# Usage: $(call .revid_minor2mask,bitmask,uc_revid)
#   $1: bitmask
#   $1: uc_revid
# Add uCode minor revid bitmask to full bitmask of arbitrary size.
# uCode revid is a number, possibly floating point: major.minor; use minor
# (fraction) part if any. Return updated bitmask (in hex for easier debugging.)
.revid_minor2mask = $(eval _minor := \
  $(or $(wordlist 2,2,$(subst .,$(space),$2)),0)) $(call \
  .do_python,$(call .print_hex,$1 | 1 << $(_minor)))

# Usage: $(call .mask2word,bitmask)
#   $1: bitmask
# Select lower 32 bits of full bitmask and drop them from it.
# Return selected bits and bitmask (in hex for easier debugging.)
.mask2word = $(if $1,$(call .do_python,$(call \
  .print_hex,$1 & 0xffffffff); $(call \
  .print_hex,$1 >>32)),0 0)

# Usage: $(call .varlist2mkvars,VAR ...)
#   $1: list of vars
# Return VAR='$(VAR)' for each VAR. Useful for recursive make invocations.
.varlist2mkvars = $(foreach _var,$1,$(_var)='$($(_var))')

# Return 1 if $(TARGET_FEATURES) contains the named feature, else 0.
# But please don't rely on 0; test for either "1" or "" (null).
# The name of a variable to check other than TARGET_FEATURES may
# be specified as $2.
# Usage: $(call opt,foobar)
#   $1: name of feature to test
#   $2: override default variable name
opt = $(if $(filter $(foreach x,$1,"$x"),$(foreach \
  o,$($(or $2,TARGET_FEATURES)),"$o")),1,0)

# GNU make macro to do compilation and dependency generation in one step.
# At least partially obsoleted by more featureful .generate_c2o_rules below.
# Usage: $(call .compile_and_makedep,$(CC),$(CFLAGS) -o $@) $<
#   $1: the compiler itself
#   $2: rest of compiler command line or fragment thereof
define .compile_and_makedep
$(call .vbb)
$(strip $1 $(call .ccline,-c -MMD -MF $(@D)/.$(@F).d -MP $2))
endef

# This macro supports the traditional gcc -MD -MF -MP flags for dependency
# generation during C compilation steps combined with a pattern adapted
# from the Linux kernel designed to trigger rebuilds on recipe changes.
# See http://make.mad-scientist.us/autodep.html and
# https://gcc.gnu.org/onlinedocs/gcc-10.4.0/gcc/Preprocessor-Options.html
# for details on the former and a modern Linux kernel tree for the latter.
#
# HOW IT WORKS:
#
# The make code below looks very weird, like monkeys-on-typewriters weird,
# but it makes sense when you go through it in detail.
#
# First, the special target .SECONDEXPANSION is enabled with scope from
# where it's given to the end of the makefile. Second expansion allows
# escaped automatic variables to be referenced in the prerequisite list.
# We believe .SECONDEXPANSION should always be enabled.
#
# This implementation gives generated dependency files a .d extension and
# places them in a subdirectory. A rule is generated to create the deps
# dir on demand.
#
# A pattern-specific setting is used to define a variable $(recipe_c2o)
# for all "%.o: %.c" rules. Note that $(recipe_c2o) uses deferred (=)
# expansion which allows it to refer to automatic variables like $@ even
# though they aren't defined yet.
#
# Then a special phony target "RECIPE_CHANGED" is declared which will be
# used to force a rebuild when the recipe changes. Here is the main magic:
#
#     %.o: %.c $(<recipe-compare-logic>)
#
# The trick is that when the recipe hasn't changed the recipe comparison
# evaluates to null so this becomes just a "%.o: %.c" pattern rule, but if
# the recipe HAS changed it evaluates to "RECIPE_CHANGED". Since that's a
# phony target it's always considered out of date and thus causes the .o
# file which depends on it to be rebuilt.
#
# Now comes the recipe. The first line is $(recipe_c2o) which expands
# to the compile command. Remember this must be defined with deferred
# expansion so its references to automatic variables are only expanded
# when they have values.
#
# The special gcc -M* flags are inserted into $(recipe_c2o) to write
# prereq aka dependency data into a separate file which is assigned to $@.d,
# e.g. "foo.o.d". The last line of the recipe knows the $@.d file has
# already been made so it can append a variable assignment to it. The value
# assigned is essentially the recipe with certain special characters
# replaced by a caret (^).
#
# Finally, "-include" is used to read the *.d files if they exist. Since
# they're created by the same recipe that makes the .o files they should
# exist IFF their corresponding .o file exists.
#
# Summary: a given .o file will either exist or not. If it does there
# should also be a .d file listing its prerequisites. If the .o doesn't
# exist it will obviously need to be built no matter what. And if somehow
# the .o exists and the .d doesn't the recipe comparison would fail
# forcing a rebuild. All updates of the .o must also update the .d.
#
# The .DELETE_ON_ERROR special target is used to ensure that a corrupted
# foo.o and foo.o.d combination can never break the build. Make will delete
# only foo.o on recipe failure since it doesn't even know about foo.o.d
# (which is called a "sibling" target). But since foo.o will be deleted
# automatically on failure, foo.o.d cannot cause bad build decisions even
# if left around since the named target foo.o must be rebuilt on the next
# pass even if the dependency data in foo.o.d is bad, and when that happens
# foo.o.d will be regenerated too.

# Private support macro, not used directly. See callers for parameters.
define ._generate_x2o_rules
  $(eval \
    $$(call .generate_mkdir_rules,$4 $5)

    .SECONDEXPANSION:

    .DELETE_ON_ERROR:

    .PHONY: RECIPE_CHANGED
    RECIPE_CHANGED:

    $(if $7,$$(call .vbt,$7))
    $(call .stdd,$4)%.o: recipe_c2o = $$(strip $1 -o $$@ -c $2 \
      $$(if $$(DEVMODE),-MD -MF $(call .stdd,$5)$$(@F).d -MP))
    $(call .stdd,$4)%.o: new_recipe = $$(subst $$(space),^,$$(subst \
      $$(comma),^,$$(subst $$(dq),,$$(recipe_c2o))))
    $(call .stdd,$4)%.o: $9 $3 $$(if $$(DEVMODE),$$$$(if $$$$(filter \
      $$$$(old_recipe_for_$$$$@),$$$$(new_recipe)),,RECIPE_CHANGED)) \
      | $$$$(@D) $5
	$(if $7,$$(call .vbb,$8))
	$$(recipe_c2o) $$<
	$$(if $$(DEVMODE),@echo -e \
	  "\nold_recipe_for_$$@ := $$(subst $$(space),^,$$(subst \
	  $$(comma),^,$$(subst $$$$,$$$$$$$$,$$(recipe_c2o))))" >> $5/$$(@F).d)

    $(if $6,$$(eval -include $(call .stdd,$5)*.d))
  )
endef # ._generate_x2o_rules

# Usage: $(call .generate_c2o_rules,<parameters>)
#   $1: path to compiler
#   $2: rest of compiler cmdline without -c and -o flags
#   $3: additional prerequisites [none]
#   $4: path to directory for *.o files with trailing / [./]
#   $5: path to directory for *.d files with trailing / [./]
#   $6: include generated .d files automatically if set
#   $7: use .vbt and .vbb macros if non-null
#   $8: optional .vbb message if $7 enabled
define .generate_c2o_rules
$(call ._generate_x2o_rules,$1,$2,$3,$4,$5,$6,$7,,%.c)
endef

# Usage: same as above but used for .S files.
define .generate_s2o_rules
$(call ._generate_x2o_rules,$1,$2,$3,$4,$5,$6,$7,,%.S)
endef

endif # _utils_mk_

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: GNUmakefile
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 noet
