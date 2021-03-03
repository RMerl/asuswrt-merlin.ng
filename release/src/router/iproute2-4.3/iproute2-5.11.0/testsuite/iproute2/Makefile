# SPDX-License-Identifier: GPL-2.0
SUBDIRS := $(filter-out Makefile,$(wildcard *))
.PHONY: all configure clean distclean show $(SUBDIRS)

all: configure
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

link:
	@if [ ! -L iproute2-this ]; then \
		ln -s ../.. iproute2-this; \
	fi

configure: link
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && if [ -f configure ]; then ./configure; fi && cd ..; \
	done

clean: link
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

distclean: clean
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir distclean; \
	done

show: link
	@echo "$(SUBDIRS)"

$(SUBDIRS):
	cd $@ && $(MAKE)
