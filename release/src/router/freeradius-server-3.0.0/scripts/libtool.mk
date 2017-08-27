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

# Add these rules only when LIBTOOL is being used.
ifneq "${LIBTOOL}" ""

# JLIBTOOL - check if we're using the local (fast) jlibtool, rather
#   than the GNU (slow) libtool shell script.  If so, add rules
#   to build it.

ifeq "${LIBTOOL}" "JLIBTOOL"
    JLIBTOOL := ${BUILD_DIR}/make/jlibtool

    # Add a rule to build jlibtool BEFORE any other targets.  This
    # means that we can use it to build the later targets.
    all install: ${JLIBTOOL}

    # Note that we need to use a compilation rule that does NOT
    # include referencing ${LIBTOOL}, as we don't have a jlibtool
    # binary!
    ${JLIBTOOL}: ${top_makedir}/jlibtool.c
	@mkdir -p $(dir $@)
	@echo CC jlibtool.c
	@$gcc $< -o $@

    clean: jlibtool_clean

    .PHONY: jlibtool_clean
    jlibtool_clean:
	@rm -f ${JLIBTOOL}

    # Tell GNU Make to use this value, rather than anything specified
    # on the command line.
    override LIBTOOL := ${JLIBTOOL}
endif    # else we're not using jlibtool

# When using libtool, it produces a '.libs' directory.  Ensure that it
# is removed on "make clean", too.
#
clean: .libs_clean

.PHONY: .libs_clean
.libs_clean:
	@rm -rf ${BUILD_DIR}/.libs/

# Re-define compilers and linkers
#
OBJ_EXT = lo
COMPILE.c = ${LIBTOOL} --silent --mode=compile ${CC}
COMPILE.cxx = ${LIBTOOL} --mode=compile ${CXX}
LINK.c = ${LIBTOOL} --silent --mode=link ${CC}
LINK.cxx = ${LIBTOOL} --mode=link ${CXX}
PROGRAM_INSTALL = ${LIBTOOL} --silent --mode=install ${INSTALL}


# LIBTOOL_ENDINGS - Given a library ending in ".a" or ".so", replace that
#   extension with ".la".
#
define LIBTOOL_ENDINGS
$(patsubst %.a,%.la,$(patsubst %.so,%.la,${1}))
endef

# ADD_TARGET_RULE.la - Build a ".la" target.
#
#   USE WITH EVAL
#
define ADD_TARGET_RULE.la
    # So "make ${1}" works
    .PHONY: ${1}
    ${1}: $${${1}_BUILD}/${1}

    # Create libtool library ${1}
    $${${1}_BUILD}/${1}: $${${1}_OBJS} $${${1}_PRLIBS}
	    @$(strip mkdir -p $(dir $${${1}_BUILD}/${1}))
	    @$(ECHO) LINK $${${1}_BUILD}/${1}
	    @$${${1}_LINKER} -o $${${1}_BUILD}/${1} $${RPATH_FLAGS} $${LDFLAGS} \
                $${${1}_LDFLAGS} $${${1}_OBJS} $${LDLIBS} $${${1}_LDLIBS} \
		$${${1}_PRLIBS}
	    @$${${1}_POSTMAKE}

endef

# ADD_RELINK_RULE.exe - Parametric "function" that adds a rule to relink
#   the target before installation, so that the paths are correct.
#
#   USE WITH EVAL
#
define ADD_RELINK_RULE.exe
    ${1}: $${${1}_BUILD}/$${RELINK}${1}

    # used to fix up RPATH for ${1} on install.
    $${${1}_BUILD}/$${${1}_RELINK}: $${${1}_OBJS} $${${1}_PRBIN} $${${1}_R_PRLIBS}
	    @$(strip mkdir -p $${${1}_BUILD}/${RELINK}/)
	    @$${${1}_LINKER} -o $${${1}_BUILD}/$${RELINK}${1} $${RELINK_FLAGS} $${LDFLAGS} \
                $${${1}_LDFLAGS} $${${1}_OBJS} $${${1}_R_PRLIBS} \
                $${LDLIBS} $${${1}_LDLIBS}
	    @$${${1}_POSTMAKE}
endef

# ADD_RELINK_RULE.la - Parametric "function" that adds a rule to relink
#   the target before installation, so that the paths are correct.
#
#   USE WITH EVAL
#
define ADD_RELINK_RULE.la
    ${1}: $${${1}_BUILD}/$${RELINK}${1}

    # used to fix up RPATH for ${1} on install.
    $${${1}_BUILD}/$${${1}_RELINK}: $${${1}_OBJS} $${${1}_PRLIBS}
	    @$(strip mkdir -p $${${1}_BUILD}/${RELINK}/)
	    @$${${1}_LINKER} -o $${${1}_BUILD}/$${RELINK}${1} $${RELINK_FLAGS} $${LDFLAGS} \
                $${${1}_LDFLAGS} $${${1}_OBJS} $${LDLIBS} $${${1}_LDLIBS}
	    @$${${1}_POSTMAKE}

endef

# By default, if libdir is defined, we build shared libraries.
# However, we can disable shared libraries if explicitly told to.
ifneq "${libdir}" ""
    ifneq "${bm_shared_libs}" "no"
        bm_shared_libs := yes
    endif
endif

# Default to building static libraries, too.
ifneq "${bm_static_libs}" "no"
    bm_static_libs := yes
endif

# Check if we build shared libraries.
ifeq "${bm_shared_libs}" "yes"
    RELINK := local/

    # RPATH  : flags use to build executables that are installed,
    #          with no dependency on the source. 
    # RELINL : flags use to build executables that can be run
    #          from the build directory / source tree.
    RPATH_FLAGS := -rpath ${libdir}
    RELINK_FLAGS := -rpath $(abspath ${BUILD_DIR})/lib/${RELINK}/.libs
    
    RELINK_FLAGS_MIN := -rpath ${libdir}

    ifneq "${bm_static_libs}" "yes"
        RPATH_FLAGS += --shared
        RELINK_FLAGS += --shared
    endif
else
    ifneq "${bm_static_libs}" "yes"
        $(error Building without static libraries requires you to set 'INSTALL' or 'libdir')
    endif

    RPATH_FLAGS := -static
endif

# UPDATE_TARGET_ENDINGS - Function to turn target into a libtool target
#   e.g. "libfoo.a" -> libfoo.la"
#
#   If the target is an executable, then its extension doesn't change
#   when we use libtool, and we don't do any re-writing.
#
#   USE WITH EVAL
#
define ADD_LIBTOOL_SUFFIX
    ifneq "$$(call LIBTOOL_ENDINGS,$${TGT})" "$${TGT}"
        TGT_NOLIBTOOL := $${TGT}
        TGT := $$(call LIBTOOL_ENDINGS,$${TGT})
        $${TGT}_NOLIBTOOL := $${TGT_NOLIBTOOL}
    endif

    ifneq "$${RELINK_FLAGS}" ""
        $${TGT}_RELINK := ${RELINK}$${TGT}
    endif

    # re-write all of the dependencies to have the libtool endings.
    TGT_PREREQS := $$(call LIBTOOL_ENDINGS,$${TGT_PREREQS})
endef

# ADD_LIBTOOL_TARGET - Function to ensure that the object files depend
#   on our jlibtool target.  This ensures that jlibtool is built before
#   it's used to build the object files.
#
#   USE WITH EVAL
#
define ADD_LIBTOOL_TARGET
    ifneq "${JLIBTOOL}" ""
        $${$${TGT}_OBJS}: $${JLIBTOOL}
    endif

    ifneq "$${$${TGT}_NOLIBTOOL}" ""
        $$(notdir $${$${TGT}_NOLIBTOOL}): $${TGT}
    endif

    # If we need to relink, add the relink targets now.
    ifneq "$${$${TGT}_RELINK}" ""
        # add rules to relink the target

        $${TGT}_R_PRLIBS := $$(subst /lib/,/lib/${RELINK},$${$${TGT}_PRLIBS})

        $$(eval $$(call ADD_RELINK_RULE$${$${TGT}_SUFFIX},$${TGT}))
    endif

endef


endif
