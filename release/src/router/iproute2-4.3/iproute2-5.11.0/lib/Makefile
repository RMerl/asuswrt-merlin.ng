# SPDX-License-Identifier: GPL-2.0
include ../config.mk

CFLAGS += -fPIC

UTILOBJ = utils.o utils_math.o rt_names.o ll_map.o ll_types.o ll_proto.o ll_addr.o \
	inet_proto.o namespace.o json_writer.o json_print.o json_print_math.o \
	names.o color.o bpf_legacy.o bpf_glue.o exec.o fs.o cg_map.o

ifeq ($(HAVE_ELF),y)
ifeq ($(HAVE_LIBBPF),y)
UTILOBJ += bpf_libbpf.o
endif
endif

NLOBJ=libgenl.o libnetlink.o
ifeq ($(HAVE_MNL),y)
NLOBJ += mnl_utils.o
endif

all: libnetlink.a libutil.a

libnetlink.a: $(NLOBJ)
	$(QUIET_AR)$(AR) rcs $@ $^

libutil.a: $(UTILOBJ) $(ADDLIB)
	$(QUIET_AR)$(AR) rcs $@ $^

install:

clean:
	rm -f $(NLOBJ) $(UTILOBJ) $(ADDLIB) libnetlink.a libutil.a
