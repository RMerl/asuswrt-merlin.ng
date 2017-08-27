# Convert asuswrt build environment variable to wigig-firmware
-include ../.config
PKG_BUILD_DIR=$(shell pwd)/source

# Define variables for wigig-firmware
INSTALLFWDIR:=$(INSTALLDIR)/lib/firmware

all:
	@#Nothing to do

install:
	@echo Install WIL6210 Wigig firmware.
	install -d $(INSTALLFWDIR)
	install -m 644 $(PKG_BUILD_DIR)/WIL6210/wil6210.fw $(INSTALLFWDIR)
	install -m 644 $(PKG_BUILD_DIR)/WIL6210/wil6210.board $(INSTALLFWDIR)/wil6210.brd

clean:
	@#Nothing to do
