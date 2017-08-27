#
# Router flow userspace example
#
# Copyright (C) 2016, Broadcom. All Rights Reserved.
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
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: $
#

include $(TOP)/.config

CTF_INCLUDES := -I$(SRCBASE)/include/ctf -I..

CCFLAGS = -Os -Wall $(CTF_INCLUDES)
ALLOBJS = $(FLOW_TEST_OBJS) $(FLOW_FWD_OBJS)
FLOW_FWD_OBJS = flow_fwd.o
FLOW_TEST_OBJS = flow_test.o
LDFLAGS += -L.. -lflow $(NETFILTER_LIBS)

.PHONY: all
all: flow_test flow_fwd

flow_fwd: $(FLOW_FWD_OBJS) ../libflow.so
	$(CC) -o $@ $(FLOW_FWD_OBJS) $(LDFLAGS)

flow_test: $(FLOW_TEST_OBJS) ../libflow.so
	$(CC) -o $@ $(FLOW_TEST_OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) flow_test $(ALLOBJS) .*.depend

%.o: %.c
	@echo " "
	@echo "Making $<"
	$(CC) $(CCFLAGS) -c $<

%.o: .%.depend

.%.depend: %.c
	$(CC) $(CCFLAGS) -M $< > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(ALLOBJS:%.o=.%.depend)
endif
