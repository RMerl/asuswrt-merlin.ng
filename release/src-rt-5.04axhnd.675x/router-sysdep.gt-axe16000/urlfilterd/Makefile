#
# build file 
#
#
CFLAGS = -s -Os -fomit-frame-pointer
CFLAGS += $(BRCM_WERROR_CFLAGS)
LDFLAGS = -Wl,-allow-shlib-undefined -L./libnfnetlink -L./libnetfilter_queue
ifeq ($(strip $(BUILD_URLFILTER)), static)
CFLAGS += -DBUILD_STATIC 
endif

LIBS = -lnetfilter_queue -lnfnetlink
CFLAGS+=-Wall -I./libnetfilter_queue/include -I./libnfnetlink/include

OBJS = filter.o 

all: gen-libs urlfilterd

install: urlfilterd
	install -m 755 urlfilterd $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/urlfilterd

dynamic: all install

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
