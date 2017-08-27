#
# Version:	$Id$
#

HEADERS	= conf.h conffile.h detail.h event.h features.h hash.h heap.h \
	libradius.h md4.h md5.h missing.h modcall.h modules.h \
	packet.h rad_assert.h radius.h radiusd.h radpaths.h \
	radutmp.h realms.h sha1.h stats.h sysutmp.h token.h \
	udpfromto.h base64.h map.h

#
#  Build dynamic headers by substituting various values from autoconf.h, these
#  get installed with the library files, so external programs can tell what
#  the server library was built with.
#

HEADERS_DY = src/include/features.h src/include/missing.h src/include/tls.h \
	src/include/radpaths.h

src/include/autoconf.sed: src/include/autoconf.h
	@grep ^#define $< | sed 's,/\*\*/,1,;' | awk '{print "\
	s,#[[:blank:]]*ifdef[[:blank:]]*" $$2 ",#if "$$3 ",g;\
	s,#[[:blank:]]*ifndef[[:blank:]]*" $$2 ",#if !"$$3 ",g;\
	s,defined(" $$2 ")," $$3 ",g;\
	s," $$2 ","$$3 ",g;"}' > $@

src/freeradius-devel/features.h: src/include/features.h src/freeradius-devel

src/include/features.h: src/include/features-h src/include/autoconf.h
	@$(ECHO) HEADER $@
	@cp $< $@
	@grep "^#define[[:blank:]]\{1,\}WITH_" src/include/autoconf.h >> $@
	@grep "^#define[[:blank:]]\{1,\}RADIUSD_VERSION" src/include/autoconf.h >> $@

src/freeradius-devel/missing.h: src/include/missing.h src/freeradius-devel

src/include/missing.h: src/include/missing-h src/include/autoconf.sed
	@$(ECHO) HEADER $@
	@sed -f src/include/autoconf.sed < $< > $@

src/freeradius-devel/tls.h: src/include/tls.h src/freeradius-devel

src/include/tls.h: src/include/tls-h src/include/autoconf.sed
	@$(ECHO) HEADER $@
	@sed -f src/include/autoconf.sed < $< > $@

src/freeradius-devel/radpaths.h: src/include/radpaths.h src/freeradius-devel

src/include/radpaths.h: src/include/build-radpaths-h
	@$(ECHO) HEADER $@
	@cd src/include && /bin/sh build-radpaths-h

${BUILD_DIR}/make/jlibtool: $(HEADERS_DY)

#
#  Installation
#
# define the installation directory
SRC_INCLUDE_DIR := ${R}${includedir}/freeradius

$(SRC_INCLUDE_DIR):
	@$(INSTALL) -d -m 755 ${SRC_INCLUDE_DIR}

# install the headers by re-writing the local files
#
# install-sh function for creating directories gets confused
# if there's a trailing slash, tries to create a directory
# it already created, and fails...
${SRC_INCLUDE_DIR}/%.h: ${top_srcdir}/src/include/%.h | $(SRC_INCLUDE_DIR)
	@echo INSTALL $(notdir $<)
	@$(INSTALL) -d -m 755 `echo $(dir $@) | sed 's/\/$$//'`
	@sed 's/^#include <freeradius-devel/#include <freeradius/' < $< > $@
	@chmod 644 $@

install.src.include: $(addprefix ${SRC_INCLUDE_DIR}/,${HEADERS})
install: install.src.include

#
#  Cleaning
#
.PHONY: clean.src.include distclean.src.include
clean.src.include:
	@rm -f $(HEADERS_DY)

clean: clean.src.include

distclean.src.include: clean.src.include
	@rm -f autoconf.sed

distclean: distclean.src.include
