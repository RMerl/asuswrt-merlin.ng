# SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
include ../config.mk

TARGETS :=

ifeq ($(HAVE_MNL),y)
CFLAGS += -I./include/uapi/

RDMA_OBJ = rdma.o utils.o dev.o link.o res.o res-pd.o res-mr.o res-cq.o \
	   res-cmid.o res-qp.o sys.o stat.o stat-mr.o

TARGETS += rdma
endif

all:	$(TARGETS) $(LIBS)

rdma:	$(RDMA_OBJ) $(LIBS)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	for i in $(TARGETS); \
	do install -m 0755 $$i $(DESTDIR)$(SBINDIR); \
	done

clean:
	rm -f $(RDMA_OBJ) $(TARGETS)
