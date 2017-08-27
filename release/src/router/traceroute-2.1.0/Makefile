#
#   Copyright (c)  2000, 2001		Dmitry Butskoy
#					<buc@citadel.stu.neva.ru>
#   License:  GPL v2 or any later
#
#   See COPYING for the status of this software.
#

#
#   Global Makefile.
#   Global rules, targets etc.
#
#   See Make.defines for specific configs.
#


srcdir = $(CURDIR)

override TARGET := .MAIN

dummy: all

include ./Make.rules


targets = $(EXEDIRS) $(LIBDIRS) $(MODDIRS)


# be happy, easy, perfomancy...
.PHONY: $(subdirs) dummy all force
.PHONY: depend indent clean distclean libclean release store libs mods


allprereq := $(EXEDIRS)

ifneq ($(LIBDIRS),)
libs: $(LIBDIRS)
ifneq ($(EXEDIRS),)
$(EXEDIRS): libs
else
allprereq += libs
endif
endif

ifneq ($(MODDIRS),)
mods: $(MODDIRS)
ifneq ($(MODUSERS),)
$(MODUSERS): mods
else
allprereq += mods
endif
ifneq ($(LIBDIRS),)
$(MODDIRS): libs
endif
endif

all: $(allprereq)

depend install: $(allprereq)

$(foreach goal,$(filter install-%,$(MAKECMDGOALS)),\
    $(eval $(goal): $(patsubst install-%,%,$(goal))))


what = all
depend: what = depend
install install-%: what = install

ifneq ($(share),)
$(share): shared = yes
endif
ifneq ($(noshare),)
$(noshare): shared = 
endif


$(targets): mkfile = $(if $(wildcard $@/Makefile),,-f $(srcdir)/default.rules)

$(targets): force
	@$(MAKE) $(mkfile) -C $@ $(what) TARGET=$@

force:


indent:
	find . -type f -name "*.[ch]" -print -exec $(INDENT) {} \;

clean:
	rm -f $(foreach exe, $(EXEDIRS), ./$(exe)/$(exe)) nohup.out
	rm -f `find . \( -name "*.[oa]" -o -name "*.[ls]o" \
		-o -name core -o -name "core.[0-9]*" -o -name a.out \) -print`

distclean: clean
	rm -f `find $(foreach dir, $(subdirs), $(dir)/.) \
		\( -name "*.[oa]" -o -name "*.[ls]o" \
		-o -name core -o -name "core.[0-9]*" -o -name a.out \
		-o -name .depend -o -name "_*" -o -name ".cross:*" \) \
		-print`


libclean:
	rm -f $(foreach lib, $(LIBDIRS), ./$(lib)/$(lib).a ./$(lib)/$(lib).so)


#  Rules to make whole-distributive operations.
#

STORE_DIR = $(HOME)/pub

release release1 release2 release3:
	@./chvers.sh $@
	@$(MAKE) store

store: distclean
	@./store.sh $(NAME) $(STORE_DIR)


