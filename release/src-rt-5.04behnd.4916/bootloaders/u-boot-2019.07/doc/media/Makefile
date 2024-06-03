# Rules to convert a .h file to inline RST documentation

SRC_DIR=$(srctree)/doc/media
PARSER = $(srctree)/doc/sphinx/parse-headers.pl
API = $(srctree)/include

FILES = linker_lists.h.rst

TARGETS := $(addprefix $(BUILDDIR)/, $(FILES))

gen_rst = \
	echo ${PARSER} $< $@ $(SRC_DIR)/$(notdir $@).exceptions; \
	${PARSER} $< $@ $(SRC_DIR)/$(notdir $@).exceptions

quiet_gen_rst = echo '  PARSE   $(patsubst $(srctree)/%,%,$<)'; \
	${PARSER} $< $@ $(SRC_DIR)/$(notdir $@).exceptions

silent_gen_rst = ${gen_rst}

$(BUILDDIR)/linker_lists.h.rst: ${API}/linker_lists.h ${PARSER} $(SRC_DIR)/linker_lists.h.rst.exceptions
	@$($(quiet)gen_rst)

# Media build rules

.PHONY: all html epub xml latex

all: $(IMGDOT) $(BUILDDIR) ${TARGETS}
html: all
epub: all
xml: all
latex: $(IMGPDF) all
linkcheck:

clean:
	-rm -f $(DOTTGT) $(IMGTGT) ${TARGETS} 2>/dev/null

$(BUILDDIR):
	$(Q)mkdir -p $@
