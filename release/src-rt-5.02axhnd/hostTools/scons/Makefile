
all: conditional_build

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

APP     := scons
VERSION := 2.4.1
LOCAL_DIR := $(HOSTTOOLS_DIR)/local_install
APP_DIR := $(LOCAL_DIR)/$(APP)

PATH    := $(PATH):$(APP_DIR)/bin/

ifneq ($(strip $(BUILD_ALLJOYN)),)
conditional_build: build_scons
else
conditional_build:
	@echo "skipping scons (not configured)"
endif

build_scons: $(APP)-$(VERSION).tar.gz
	if [ ! -e $(APP_DIR) ]; then \
		if [ ! -e $(LOCAL_DIR) ]; then \
			mkdir $(LOCAL_DIR); \
		fi; \
		mkdir $(APP_DIR); \
		cd $(APP_DIR); \
		tar --strip-components=1 -xzf $(CURR_DIR)/$(APP)-$(VERSION).tar.gz -C $(APP_DIR); \
		echo "$(APP)-$(VERSION) is untarred"; \
		python setup.py install --standard-lib --prefix=$(APP_DIR); \
		cd $(CURR_DIR); \
		echo "Done building $(APP)-$(VERSION)"; \
	fi;

clean:
	rm -rf $(APP_DIR)

shell:
	bash -i

