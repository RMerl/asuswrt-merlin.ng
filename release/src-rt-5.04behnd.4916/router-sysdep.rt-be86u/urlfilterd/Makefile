
all dynamic install: conditional_build

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

CFLAGS = -s -Os -fomit-frame-pointer -D_GNU_SOURCE
CFLAGS += $(BRCM_WERROR_CFLAGS)
LDFLAGS = -Wl,-allow-shlib-undefined -L./libnfnetlink -L./libnetfilter_queue
ifeq ($(strip $(BUILD_URLFILTER)), static)
CFLAGS += -DBUILD_STATIC 
endif

LIBS = -lnetfilter_queue -lnfnetlink
CFLAGS+=-Wall -I./libnetfilter_queue/include -I./libnfnetlink/include

export $(CFLAGS)

APP = urlfilterd
ifneq ($(strip $(BUILD_URLFILTER)),)
conditional_build: install

else
conditional_build: sanity_check
	@echo "skipping $(APP) (not configured)"

endif

OBJS = filter.o 

install: urlfilterd
	install -m 755 urlfilterd $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/urlfilterd

static: urlfilterd.a

.c.o:
	$(CC) $(CFLAGS) -c $<

gen-libs: 
	$(MAKE) -C libnfnetlink 
	$(MAKE) -C libnetfilter_queue

urlfilterd: filter.o gen-libs
	$(CC) -o urlfilterd filter.o $(LDFLAGS) $(LIBS)

urlfilterd.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	$(MAKE) -C libnfnetlink clean
	$(MAKE) -C libnetfilter_queue clean
	-rm -f urlfilterd *.o  urlfilterd.a
