
LIB :=  libhttpdshared.so
OBJS := httpdshared.o

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)


all: build_$(LIB)
	@echo "Done Building $@"

install:
	@echo done

.PHONY: all

include $(BUILD_DIR)/make.common

$(info "BUILDING build_$(LIB)")

.PHONY: build_$(LIB)

build_$(LIB): sanity_check $(LIB) generic_public_lib_install
	@echo "Done Building $@"

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/public/$(LIB)

$(LIB):
	cp -f prebuilt/libhttpdshared.so .

