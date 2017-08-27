#
# Copyright 2014 Trend Micro Incorporated
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice, 
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, 
#    this list of conditions and the following disclaimer in the documentation 
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors 
#    may be used to endorse or promote products derived from this software without 
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
# OF SUCH DAMAGE.
#

## ASUS ##
-include ../../../common.mak
-include $(TOP)/common.mak
ifeq ($(HND_ROUTER),y)
LD = aarch64-linux-ld
endif

TDTS_DIR_PACK := $(CURDIR)/../..
TDTS_DIR_PACK_MOD := $(TDTS_DIR_PACK)/modules
TDTS_DIR_PACK_LIB := $(TDTS_DIR_PACK)/lib
TDTS_DIR_PACK_INC := $(TDTS_DIR_PACK)/include

include Makefile.cfg

CFLAGS += -D_GNU_SOURCE
CFLAGS += $(TMCFG_APP_U_EXTRA_CFLAGS) -I$(TDTS_DIR_PACK_INC)

include build.inc

u-exec := sample.bin
u-obj := $(tdts-udb-sample-obj-y)
u-src := $(patsubst %.o,%.c,$(u-obj))

#
# all
#
.PHONY: all
all: $(u-exec)

$(u-exec): $(u-obj)
	$(TMCFG_TC_PFX)gcc -o $@ $^ $(LDFLAGS)
	$(TMCFG_TC_PFX)strip $@

# Acquire auto-generated dependency targets from .d files.
-include $(u-obj:.o=.d)

%.o: %.c
	$(TMCFG_TC_PFX)gcc -o $@ -c $(patsubst %.o,%.c,$@) $(CFLAGS)
	-@$(TMCFG_TC_PFX)gcc -MP -MM $(patsubst %.o,%.c,$@) -MT $@ $(CFLAGS) > $(patsubst %.o,%.d,$@)

#
# distclean
#
.PHONY: distclean
distclean: clean

#
# clean
#
.PHONY: clean
clean:
	@rm -vf $(u-exec) $(u-obj:.o=.d) $(u-obj) *.o

