#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

# This file is for RDP projects that support cmake-based fw build.
#
# The build system must satisfy minimal cmake version requirement.
# Otherwise, firmware build will fall back to legacy build recipes.
# Use cmake-based build only when building firmware or ut
ifneq ("$(filter ut,$(MAKECMDGOALS))$(filter firmware,$(MAKECMDGOALS))$(filter hal_generator,$(MAKECMDGOALS))", )

CMAKE_VERSION_MAJOR_MIN := 3
CMAKE_VERSION_MAJOR := $(shell cmake --version | grep 'cmake version' | awk '{print $$3}' | awk -F'.' '{print $$1}')
CMAKE_FW_BUILD := $(shell test "$(CMAKE_VERSION_MAJOR)" -ge "$(CMAKE_VERSION_MAJOR_MIN)" 2> /dev/null && echo -n "y")

# Build firmware using cmake?
ifeq ("$(CMAKE_FW_BUILD)", "y")
    ifeq ("$(V)", "1")
        CMAKE_MAKE_OPTS := VERBOSE=1
    endif
    CMAKE_USER_VARS := $(filter-out CMAKE_FW_BUILD=%,$(MAKEOVERRIDES))
    CMAKE_USER_VARS := $(patsubst %,-D%,$(filter-out V=%,$(CMAKE_USER_VARS)))

$(PROJECT_DIR)/target/proj_flags.cmake: $(PROJECT_DIR)/make.proj_flags $(TOP_DIR)/make_to_cmake.sed
	mkdir -p $(PROJECT_DIR)/target
	sed -f $(TOP_DIR)/make_to_cmake.sed $< > $(PROJECT_DIR)/target/proj_flags.cmake

cmake_prepare: $(PROJECT_DIR)/target/proj_flags.cmake
	cd $(PROJECT_DIR)/target && cmake ../../.. $(CMAKE_USER_VARS)

firmware: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

hw_dts_gen: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

project_target_prepare: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

project_prepare_dirs: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

run_table_manager: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

prepare_fw_headers: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

prepare_fw_links: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

prepare_rdd_links: cmake_prepare
	$(MAKE) -C $(PROJECT_DIR)/target $(CMAKE_MAKE_OPTS) $@

else

$(info "!!! $(shell cmake --version | grep 'cmake version') less than $(CMAKE_VERSION_MAJOR_MIN).0. cmake build was disabled")

endif

endif # ifneq ("$(filter ut,$(MAKECMDGOALS))$(filter firmware,$(MAKECMDGOALS))", )
