#
# For the new Modular Builds, also copy header files from
# shared/{opensource,broadcom} to EXT_BUILD_DIR/targets/$(PROFILE)/fs.build
# And to allow the app makefiles to remain consistent, do the same for our
# "old"/existing builds.
#

ifneq ($(BCM_MODULAR_BUILD),)
shared_headers_install:
	# New Modular Build way:
	mkdir -p $(EXT_BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/opensource/include
	$(INSTALL_HEADERS_WITH_CP) opensource/include/bcm963xx $(EXT_BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/opensource/include
#	Broadcom proprietary headers not available in consumer builds
#	mkdir -p $(EXT_BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/broadcom/include
#	$(INSTALL_HEADERS_WITH_CP) broadcom/include/bcm963xx $(EXT_BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/broadcom/include
else
shared_headers_install:
	# Do the same in our "old" builds
	mkdir -p $(BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/opensource/include
	$(INSTALL_HEADERS_WITH_CP) opensource/include/bcm963xx $(BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/opensource/include
#	Broadcom proprietary headers not available in consumer builds
#	mkdir -p $(BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/broadcom/include
#	$(INSTALL_HEADERS_WITH_CP) broadcom/include/bcm963xx $(BUILD_DIR)/targets/$(PROFILE)/fs.build/shared/broadcom/include
endif #BCM_MODULAR_BUILD
