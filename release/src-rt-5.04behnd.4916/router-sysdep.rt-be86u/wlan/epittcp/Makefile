# build file for epi_ttcp
#

PROGRAM_NAME:= epi_ttcp

CFLAGS= -Wall -s -Os -fomit-frame-pointer
LDFLAGS=-Os -Wl,-allow-shlib-undefined

ifeq ($(strip $(BUILD_EPITTCP)), static)
CFLAGS += -DBUILD_STATIC
endif

CFLAGS += -I$(TOOLCHAIN)/include

ifeq ($(wildcard *.c),)
all:
else
all: $(PROGRAM_NAME)
endif

ifeq ($(wildcard *.c),)
install:
	-cp -f prebuilt/$(PROGRAM_NAME) $(PROGRAM_NAME)
else
install: $(PROGRAM_NAME)
endif
	install -m 755  $(PROGRAM_NAME) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(PROGRAM_NAME)

ifeq ($(wildcard *.c),)
dynamic: install
else
dynamic: $(PROGRAM_NAME) install
endif

static: $(PROGRAM_NAME).a

$(PROGRAM_NAME): $(PROGRAM_NAME).o
	$(CC) $(LDFLAGS) -o $(PROGRAM_NAME)  $(PROGRAM_NAME).o

$(PROGRAM_NAME).a: $(PROGRAM_NAME).o
	$(AR) rcs $(PROGRAM_NAME).a  $(PROGRAM_NAME).o

clean:
	-rm -f $(PROGRAM_NAME) $(PROGRAM_NAME).a  $(PROGRAM_NAME).o
