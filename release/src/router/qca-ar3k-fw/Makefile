# Convert asuswrt build environment variable to qca-wifi-fw
PKG_BUILD_DIR=$(shell pwd)/source

# Define variables for qca-wifi-fw
INSTALLFWDIR:=$(INSTALLDIR)/lib/firmware

all:
	@#Nothing to do

install:
	install -d $(INSTALLFWDIR)/ar3k
	cp -af $(PKG_BUILD_DIR)/* $(INSTALLFWDIR)/ar3k

clean:
	@#Nothing to do
