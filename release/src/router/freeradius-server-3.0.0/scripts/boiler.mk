# boilermake: A reusable, but flexible, boilerplate Makefile.
#
# Copyright 2008, 2009, 2010 Dan Moulding, Alan T. DeKok
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Caution: Don't edit this Makefile! Create your own main.mk and other
#          submakefiles, which will be included by this Makefile.
#          Only edit this if you need to modify boilermake's behavior (fix
#          bugs, add features, etc).

# Note: Parameterized "functions" in this makefile that are marked with
#       "USE WITH EVAL" are only useful in conjuction with eval. This is
#       because those functions result in a block of Makefile syntax that must
#       be evaluated after expansion. Since they must be used with eval, most
#       instances of "$" within them need to be escaped with a second "$" to
#       accomodate the double expansion that occurs when eval is invoked.

# ADD_CLEAN_RULE - Parameterized "function" that adds a new rule and phony
#   target for cleaning the specified target (removing its build-generated
#   files).
#
#   USE WITH EVAL
#
define ADD_CLEAN_RULE
    clean: clean_$(notdir ${1})
    .PHONY: clean_$(notdir ${1})
    clean_$(notdir ${1}):
	@$(strip rm -f ${${1}_BUILD}/${1} ${${1}_NOLIBTOOL} ${${1}_BUILD}/${${1}_RELINK} $${${1}_OBJS} $${${1}_DEPS} $${${1}_OBJS:%.${OBJ_EXT}=%.[do]}) $(if ${TARGET_DIR},$${TARGET_DIR}/$(notdir ${1}))
	$${${1}_POSTCLEAN}

endef

# FILTER_DEPENDS: function to turn a *.d file into a *.mk file.
#   We start off with the dependencies as created by the compiler,
#   CPP, or makedepend.  We then ensure that there is an empty dependency
#   for each header file.  The blank target ensures that the build
#   can proceed even if the header file has been deleted.
#
#  COMMON filters:
#	remove comments
#	remove dependencies on global include files
#	remove empty dependencies
#	remove CPP hacks like "foo: <built-in>"
#
#  1) Filter the .d file to remove unnecessary cruft
#
#	COMMON
#	Replace ".o" with "${OBJ_EXT}"
#	delete empty continuation lines
#	delete blank lines
#	replace "build/" with "${BUILD_DIR}/" when it's at the start of a line
#	delete references to ${BUILD_DIR}/make/include, the "config.mk"
#	file adds these dependencies automatically.
#	replace "build/" with "${BUILD_DIR}/" when it's in the middle of a line
#	
#	remove sequential duplicate lines
#	
#  2) Create empty dependencies from the files
#
#	COMMON
#	remove existing targets
#	remove continuations (to make the targets stand by themselves)
#	delete blank lines
#	add in empty dependency for each file.
#	remove sequential duplicate lines
#
define FILTER_DEPENDS
	@mkdir -p $$(dir $${BUILD_DIR}/make/src/$$*)
	@mkdir -p $$(dir $${BUILD_DIR}/objs/$$*)
	@sed  -e 's/#.*//' \
	  -e 's,^$${top_srcdir},$$$${top_srcdir},' \
	  -e 's, $${top_srcdir}, $$$${top_srcdir},' \
	  -e 's,^$${BUILD_DIR},$$$${BUILD_DIR},' \
	  -e 's, $${BUILD_DIR}/make/include/[^ :]*,,' \
	  -e 's, $${BUILD_DIR}, $$$${BUILD_DIR},' \
	  -e 's, /[^: ]*,,g' \
	  -e 's,^ *[^:]* *: *$$$$,,' \
	  -e '/: </ d' \
	  -e 's/\.o: /.$$$${OBJ_EXT}: /' \
	  -e '/^ *\\$$$$/ d' \
	  < $${BUILD_DIR}/objs/$$*.d | sed -e '$$$$!N; /^\(.*\)\n\1$$$$/!P; D' \
	  >  $${BUILD_DIR}/make/src/$$*.mk
	@sed -e 's/#.*//' \
	  -e 's, $${BUILD_DIR}/make/include/[^ :]*,,' \
	  -e 's, /[^: ]*,,g' \
	  -e 's,^ *[^:]* *: *$$$$,,' \
	  -e '/: </ d' \
	  -e 's/^[^:]*: *//' \
	  -e 's/ *\\$$$$//' \
	  -e 's/$$$$/ :/' \
	  < $${BUILD_DIR}/objs/$$*.d | sed -e '$$$$!N; /^\(.*\)\n\1$$$$/!P; D' \
	 >> $${BUILD_DIR}/make/src/$$*.mk
	 @rm -f $${BUILD_DIR}/objs/$$*.d
endef

# ADD_OBJECT_RULE - Parameterized "function" that adds a pattern rule, using
#   the commands from the second argument, for building object files from
#   source files with the filename extension specified in the first argument.
#
#   This function assumes that the C/C++ sources files have filenames
#   *relative* to the source root.  If they have absolute pathnames, it
#   creates the wrong filenames...
#
#   USE WITH EVAL
#
ifeq "${CPP_MAKEDEPEND}" "yes"
define ADD_OBJECT_RULE
$${BUILD_DIR}/objs/%.${OBJ_EXT} $${BUILD_DIR}/objs/%.d: ${1} ${JLIBTOOL}
	${2}
	$${CPP} $${CPPFLAGS} $${SRC_INCDIRS} $${SRC_DEFS} $$< | sed \
	  -n 's,^\# *[0-9][0-9]* *"\([^"]*\)".*,$$@: \1,p' > $${BUILD_DIR}/objs/$$*.d
${FILTER_DEPENDS}
endef

else
define ADD_OBJECT_RULE
$${BUILD_DIR}/objs/%.${OBJ_EXT} $${BUILD_DIR}/objs/%.d: ${1} ${JLIBTOOL}
	${2}
${FILTER_DEPENDS}
endef
endif

# ADD_TARGET_DIR - Parameterized "function" that makes a link from
#   TARGET_DIR to the executable or library in the BUILD_DIR directory.
#
#   USE WITH EVAL
#
ifneq "${TARGET_DIR}" ""
    define ADD_TARGET_DIR
        all: $${TARGET_DIR}/$$(notdir ${1})

        $${TARGET_DIR}/$$(notdir ${1}): ${1}
	    [ -f $${TARGET_DIR}/$$(notdir ${1}) ] || ln -s ${1} $${TARGET_DIR}/$$(notdir ${1})

    endef
endif

# ADD_TARGET_TO_ALL - Parameterized "function" that adds the target,
#   and makes "all" depend on it.
#
#   USE WITH EVAL
#
define ADD_TARGET_TO_ALL
    all: ${1}

endef

# ADD_TARGET_RULE.* - Parameterized "functions" that adds a new target to the
#   Makefile.  There should be one ADD_TARGET_RULE definition for each
#   type of target that is used in the build.  
#
#   New rules can be added by copying one of the existing ones, and
#   replacing the line after the "mkdir"
#

# ADD_TARGET_RULE.exe - Build an executable target.
#
#   USE WITH EVAL
#
define ADD_TARGET_RULE.exe
    # So "make ${1}" works
    .PHONY: ${1}
    ${1}: $${${1}_BUILD}/${1}

    # Create executable ${1}
    $${${1}_BUILD}/${1}: $${${1}_OBJS} $${${1}_PRBIN} $${${1}_PRLIBS}
	    @$(strip mkdir -p $(dir $${${1}_BUILD}/${1}))
	    @$(ECHO) LINK $${${1}_BUILD}/${1}
	    @$${${1}_LINKER} -o $${${1}_BUILD}/${1} $${RPATH_FLAGS} $${LDFLAGS} \
                $${${1}_LDFLAGS} $${${1}_OBJS} $${${1}_PRLIBS} \
                $${LDLIBS} $${${1}_LDLIBS}
	    @$${${1}_POSTMAKE}

endef

# ADD_TARGET_RULE.a - Build a static library target.
#
#   USE WITH EVAL
#
define ADD_TARGET_RULE.a
    # So "make ${1}" works
    .PHONY: ${1}
    ${1}: $${${1}_BUILD}/${1}

    # Create static library ${1}
    $${${1}_BUILD}/${1}: $${${1}_OBJS} $${${1}_PRLIBS}
	    @$(strip mkdir -p $(dir $${${1}_BUILD}/${1}))
	    @$(ECHO) LINK $${${1}_BUILD}/${1}
	    @$${AR} $${ARFLAGS} $${${1}_BUILD}/${1} $${${1}_OBJS}
	    @$${${1}_POSTMAKE}

endef

# ADD_TARGET_RULE.so - Build a ".so" target.
#
#   USE WITH EVAL
#
define ADD_TARGET_RULE.so
$(error Please add rules to build a ".so" file.)
endef

# ADD_TARGET_RULE.dll - Build a ".dll" target.
#
#   USE WITH EVAL
#
define ADD_TARGET_RULE.dll
$(error Please add rules to build a ".dll" file.)
endef

# ADD_TARGET_RULE.dylib - Build a ".dylib" target.
#
#   USE WITH EVAL
#
define ADD_TARGET_RULE.dylib
$(error Please add rules to build a ".dylib" file.)
endef

# CANONICAL_PATH - Given one or more paths, converts the paths to the canonical
#   form. The canonical form is the path, relative to the project's top-level
#   directory (the directory from which "make" is run), and without
#   any "./" or "../" sequences. For paths that are not  located below the
#   top-level directory, the canonical form is the absolute path (i.e. from
#   the root of the filesystem) also without "./" or "../" sequences.
define CANONICAL_PATH
$(patsubst ${CURDIR}/%,%,$(abspath ${1}))
endef

# COMPILE_C_CMDS - Commands for compiling C source code.
define COMPILE_C_CMDS
	@mkdir -p $(dir $@)
	@$(ECHO) CC $<
	@$(strip ${COMPILE.c} -o $@ -c -MD ${CFLAGS} ${SRC_CFLAGS} ${INCDIRS} \
	    ${SRC_INCDIRS} ${SRC_DEFS} ${DEFS} $<)
endef

# COMPILE_CXX_CMDS - Commands for compiling C++ source code.
define COMPILE_CXX_CMDS
	@mkdir -p $(dir $@)
	@$(strip ${COMPILE.cxx} -o $@ -c -MD ${CXXFLAGS} ${SRC_CXXFLAGS} ${INCDIRS} \
	    ${SRC_INCDIRS} ${SRC_DEFS} ${DEFS} $<)
endef

# INCLUDE_SUBMAKEFILE - Parameterized "function" that includes a new
#   "submakefile" fragment into the overall Makefile. It also recursively
#   includes all submakefiles of the specified submakefile fragment.
#
#   USE WITH EVAL
#
define INCLUDE_SUBMAKEFILE
    # Initialize all variables that can be defined by a makefile fragment, then
    # include the specified makefile fragment.
    TARGET :=
    TGT_LDFLAGS :=
    TGT_LDLIBS :=
    TGT_LINKER :=
    TGT_POSTCLEAN :=
    TGT_POSTMAKE :=
    TGT_PREREQS :=
    TGT_POSTINSTALL :=
    TGT_INSTALLDIR := ..
    TGT_CHECK_HEADERS :=
    TGT_CHECK_LIBS :=

    SOURCES :=
    SRC_CFLAGS :=
    SRC_CXXFLAGS :=
    SRC_DEFS :=
    SRC_INCDIRS :=
    MAN :=

    SUBMAKEFILES :=

    # A directory stack is maintained so that the correct paths are used as we
    # recursively include all submakefiles. Get the makefile's directory and
    # push it onto the stack.
    DIR := $(call CANONICAL_PATH,$(dir ${1}))
    DIR_STACK := $$(call PUSH,$${DIR_STACK},$${DIR})

    include ${1}

    # Initialize internal local variables.
    OBJS :=

    # Determine which target this makefile's variables apply to. A stack is
    # used to keep track of which target is the "current" target as we
    # recursively include other submakefiles.
    ifneq "$$(strip $${TARGET})" ""
        # This makefile defined a new target. Target variables defined by this
        # makefile apply to this new target. Initialize the target's variables.

        # libs go into ${BUILD_DIR}/lib
        # everything else goes into ${BUILD_DIR}/bin
#        TGT := $$(strip $$(if $$(suffix $${TARGET}),$${BUILD_DIR}/lib,$${BUILD_DIR}/bin)/$${TARGET})
        TGT := $${TARGET}

        # A "hook" to rewrite "libfoo.a" -> "libfoo.la" when using libtool
        $$(eval $$(call ADD_LIBTOOL_SUFFIX))

        ALL_TGTS += $${TGT}
        $${TGT}_LDFLAGS := $${TGT_LDFLAGS}
        $${TGT}_LDLIBS := $${TGT_LDLIBS}
        $${TGT}_LINKER := $${TGT_LINKER}
        $${TGT}_POSTMAKE := $${TGT_POSTMAKE}
        $${TGT}_POSTCLEAN := $${TGT_POSTCLEAN}
        $${TGT}_POSTINSTALL := $${TGT_POSTINSTALL}
        $${TGT}_PREREQS := $${TGT_PREREQS}
        $${TGT}_PRBIN := $$(addprefix $${BUILD_DIR}/bin/,$$(filter-out %.a %.so %.la,$${TGT_PREREQS}))
        $${TGT}_PRLIBS := $$(addprefix $${BUILD_DIR}/lib/,$$(filter %.a %.so %.la,$${TGT_PREREQS}))
        $${TGT}_DEPS :=
        $${TGT}_OBJS :=
        $${TGT}_SOURCES :=
        $${TGT}_MAN := $${MAN}
        $${TGT}_SUFFIX := $$(if $$(suffix $${TGT}),$$(suffix $${TGT}),.exe)
        $${TGT}_BUILD := $$(if $$(suffix $${TGT}),$${BUILD_DIR}/lib,$${BUILD_DIR}/bin)
        $${TGT}_MAKEFILES += ${1}
        $${TGT}_CHECK_HEADERS := $${TGT_CHECK_HEADERS}
        $${TGT}_CHECK_LIBS := $${TGT_CHECK_LIBS}
    else
        # The values defined by this makefile apply to the the "current" target
        # as determined by which target is at the top of the stack.
        TGT := $$(strip $$(call PEEK,$${TGT_STACK}))
        $${TGT}_LDFLAGS   += $${TGT_LDFLAGS}
        $${TGT}_LDLIBS    += $${TGT_LDLIBS}
        $${TGT}_POSTCLEAN += $${TGT_POSTCLEAN}
        $${TGT}_POSTMAKE  += $${TGT_POSTMAKE}
        $${TGT}_PREREQS   += $${TGT_PREREQS}
    endif

    # Push the current target onto the target stack.
    TGT_STACK := $$(call PUSH,$${TGT_STACK},$${TGT})

    # If there's no target, don't build the sources.
    ifneq "$$(strip $${TARGET})" ""

    # if there's no sources, don't do the automatic object build
    ifneq "$$(strip $${SOURCES})" ""
        # This makefile builds one or more objects from source. Validate the
        # specified sources against the supported source file types.
        BAD_SRCS := $$(strip $$(filter-out $${ALL_SRC_EXTS},$${SOURCES}))
        ifneq "$${BAD_SRCS}" ""
            $$(error Unsupported source file(s) found in ${1} [$${BAD_SRCS}])
        endif

        # Qualify and canonicalize paths.
        SOURCES     := $$(call QUALIFY_PATH,$${DIR},$${SOURCES})
        SOURCES     := $$(call CANONICAL_PATH,$${SOURCES})
        SRC_INCDIRS := $$(call QUALIFY_PATH,$${DIR},$${SRC_INCDIRS})
        SRC_INCDIRS := $$(call CANONICAL_PATH,$${SRC_INCDIRS})

        # Save the list of source files for this target.
        $${TGT}_SOURCES += $${SOURCES}

        # Convert the source file names to their corresponding object file
        # names.
        OBJS := $$(addprefix $${BUILD_DIR}/objs/,\
                   $$(addsuffix .${OBJ_EXT},$$(basename $${SOURCES})))

        # Add the objects to the current target's list of objects, and create
        # target-specific variables for the objects based on any source
        # variables that were defined.
        $${TGT}_OBJS += $${OBJS}
        $${TGT}_DEPS += $$(addprefix $${BUILD_DIR}/make/src/,\
                   $$(addsuffix .mk,$$(basename $${SOURCES})))

        # A "hook" to define variables needed by the "legacy" makefiles.
        $$(eval $$(call ADD_LEGACY_VARIABLES,$$(dir ${1}),$${TGT}))

        $${OBJS}: SRC_CFLAGS := $${SRC_CFLAGS}
        $${OBJS}: SRC_CXXFLAGS := $${SRC_CXXFLAGS}
        $${OBJS}: SRC_DEFS := $$(addprefix -D,$${SRC_DEFS})
        $${OBJS}: SRC_INCDIRS := $$(addprefix -I,$${SRC_INCDIRS})
        $${OBJS}: ${1}
    endif
    endif

    ifneq "$$(strip $${SUBMAKEFILES})" ""
        # This makefile has submakefiles. Recursively include them.
        $$(foreach MK,$${SUBMAKEFILES},\
           $$(eval $$(call INCLUDE_SUBMAKEFILE,\
                      $$(call CANONICAL_PATH,\
                         $$(call QUALIFY_PATH,$${DIR},$${MK})))))
    endif

    # Reset the "current" target to it's previous value.
    TGT_STACK := $$(call POP,$${TGT_STACK})
    # If we're about to change targets, create the rules for the target
    ifneq "$${TGT}" "$$(call PEEK,$${TGT_STACK})"
        # add rules to build the target, and have "all" depend on it.
        $$(eval $$(call ADD_TARGET_TO_ALL,$${TGT}))

        # A "hook" to add rules for ${TARGET_DIR}/foo, if TARGET_DIR
        # is defined.  Otherwise, we leave the source directory untouched.
        $$(eval $$(call ADD_TARGET_DIR,$${TGT}))

        # A "hook" to build the libtool target.
        $$(eval $$(call ADD_LIBTOOL_TARGET))

        # Choose the correct linker.
        ifeq "$$(strip $$(filter $${CXX_SRC_EXTS},$${$${TGT}_SOURCES}))" ""
            ifeq "$${$${TGT}_LINKER}" ""
                $${TGT}_LINKER := ${LL}$${LINK.c}
            endif
        else
            ifeq "$${$${TGT}_LINKER}" ""
                $${TGT}_LINKER := ${LL}$${LINK.cxx}
            endif
        endif

        # add rules to build the target
        $$(eval $$(call ADD_TARGET_RULE$${$${TGT}_SUFFIX},$${TGT}))

        # generate the clean rule for this target.
        $$(eval $$(call ADD_CLEAN_RULE,$${TGT}))

        # Hook to add an installation target
        $$(eval $$(call ADD_INSTALL_TARGET,$${TGT}))

        # Hook to add a configuration target
        $$(eval $$(call ADD_TARGET_CONFIG,$${TGT}))

        # "hook" for legacy Makefiles
        $$(eval $$(call ADD_LEGACY_RULE,$${TGT}))
    endif

    TGT := $$(call PEEK,$${TGT_STACK})

    # Reset the "current" directory to it's previous value.
    DIR_STACK := $$(call POP,$${DIR_STACK})
    DIR := $$(call PEEK,$${DIR_STACK})
endef

# MIN - Parameterized "function" that results in the minimum lexical value of
#   the two values given.
define MIN
$(firstword $(sort ${1} ${2}))
endef

# PEEK - Parameterized "function" that results in the value at the top of the
#   specified colon-delimited stack.
define PEEK
$(lastword $(subst :, ,${1}))
endef

# POP - Parameterized "function" that pops the top value off of the specified
#   colon-delimited stack, and results in the new value of the stack. Note that
#   the popped value cannot be obtained using this function; use peek for that.
define POP
${1:%:$(lastword $(subst :, ,${1}))=%}
endef

# PUSH - Parameterized "function" that pushes a value onto the specified colon-
#   delimited stack, and results in the new value of the stack.
define PUSH
${2:%=${1}:%}
endef

# QUALIFY_PATH - Given a "root" directory and one or more paths, qualifies the
#   paths using the "root" directory (i.e. appends the root directory name to
#   the paths) except for paths that are absolute.
define QUALIFY_PATH
$(addprefix ${1}/,$(filter-out /%,${2})) $(filter /%,${2})
endef

###############################################################################
#
# Start of Makefile Evaluation
#
###############################################################################

# Older versions of GNU Make lack capabilities needed by boilermake.
# With older versions, "make" may simply output "nothing to do", likely leading
# to confusion. To avoid this, check the version of GNU make up-front and
# inform the user if their version of make doesn't meet the minimum required.
MIN_MAKE_VERSION := 3.81
MIN_MAKE_VER_MSG := boilermake requires GNU Make ${MIN_MAKE_VERSION} or greater
ifeq "${MAKE_VERSION}" ""
    $(info GNU Make not detected)
    $(error ${MIN_MAKE_VER_MSG})
endif
ifneq "${MIN_MAKE_VERSION}" "$(call MIN,${MIN_MAKE_VERSION},${MAKE_VERSION})"
    $(info This is GNU Make version ${MAKE_VERSION})
    $(error ${MIN_MAKE_VER_MSG})
endif

# Define the source file extensions that we know how to handle.
OBJ_EXT := o
C_SRC_EXTS := %.c
CXX_SRC_EXTS := %.C %.cc %.cp %.cpp %.CPP %.cxx %.c++
ALL_SRC_EXTS := ${C_SRC_EXTS} ${CXX_SRC_EXTS}

# Initialize global variables.
ALL_TGTS :=
DEFS :=
DIR_STACK :=
INCDIRS :=
TGT_STACK :=

ifeq "${top_builddir}" ""
    top_builddir := .
endif

# Ensure that valid values are set for BUILD_DIR
ifeq "$(strip ${BUILD_DIR})" ""
    ifeq "${top_builddir}" "${PWD}"
        BUILD_DIR := build
    else
        BUILD_DIR := ${top_builddir}/build
    endif
else
    BUILD_DIR := $(call CANONICAL_PATH,${BUILD_DIR})
endif

# Define compilers and linkers
#
COMPILE.c = ${CC}
COMPILE.cxx = ${CXX}
CPP = cc -E
LINK.c = ${CC}
LINK.cxx = ${CXX}

# Set ECHO to "true" for *very* quiet builds
ECHO = echo

# Define the "all" target (which simply builds all user-defined targets) as the
# default goal.
.PHONY: all
all: 

# Add "clean" rules to remove all build-generated files.
.PHONY: clean
clean:

top_makedir := $(dir $(lastword ${MAKEFILE_LIST}))

-include ${top_makedir}/install.mk
-include ${top_makedir}/libtool.mk

# Include the main user-supplied submakefile. This also recursively includes
# all other user-supplied submakefiles.
$(eval $(call INCLUDE_SUBMAKEFILE,${top_builddir}/main.mk))

# Perform post-processing on global variables as needed.
DEFS := $(addprefix -D,${DEFS})
INCDIRS := $(addprefix -I,$(call CANONICAL_PATH,${INCDIRS}))

# Add pattern rule(s) for creating compiled object code from C source.
$(foreach EXT,${C_SRC_EXTS},\
  $(eval $(call ADD_OBJECT_RULE,${EXT},$${COMPILE_C_CMDS})))

# Add pattern rule(s) for creating compiled object code from C++ source.
$(foreach EXT,${CXX_SRC_EXTS},\
  $(eval $(call ADD_OBJECT_RULE,${EXT},$${COMPILE_CXX_CMDS})))

# Don't include the target dependencies if we're doing a "make clean"
# Future: have a list of targets that don't require dependency generation,
#  and see if MAKECMDGOALS is one of them.
ifneq "$(MAKECMDGOALS)" "clean"
    $(foreach TGT,${ALL_TGTS},\
      $(eval -include ${${TGT}_DEPS}))
endif
