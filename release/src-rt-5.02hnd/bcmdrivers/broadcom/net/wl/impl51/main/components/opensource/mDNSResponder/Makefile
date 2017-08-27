#
# Top level makefile for Build & Integration.
# 
# This file is used to facilitate checking the mDNSResponder project
# directly out of CVS and submitting to B&I at Apple.
#
# The various platform directories contain makefiles or projects
# specific to that platform.
#
#    B&I builds must respect the following target:
#         install:
#         installsrc:
#         installhdrs:
#         clean:
#

include /Developer/Makefiles/pb_makefiles/platform.make

MVERS = "mDNSResponder-320.10.80"

DDNSWRITECONFIG = "$(DSTROOT)/Library/Application Support/Bonjour/ddnswriteconfig"

installSome:
	cd "$(SRCROOT)/mDNSMacOSX"; xcodebuild install     OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT) DSTROOT=$(DSTROOT) MVERS=$(MVERS) SDKROOT=$(SDKROOT) -target Build\ Some

SystemLibraries:
	cd "$(SRCROOT)/mDNSMacOSX"; xcodebuild install     OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT) DSTROOT=$(DSTROOT) MVERS=$(MVERS) SDKROOT=$(SDKROOT) -target SystemLibraries

install:
	cd "$(SRCROOT)/mDNSMacOSX"; xcodebuild install     OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT) DSTROOT=$(DSTROOT) MVERS=$(MVERS) SDKROOT=$(SDKROOT)
	# Make sure ddnswriteconfig is owned by root:wheel, then make it setuid root executable
	if test -e $(DDNSWRITECONFIG) ; then chown 0:80 $(DDNSWRITECONFIG) ; chmod 4555 $(DDNSWRITECONFIG) ; fi

installsrc:
	ditto . "$(SRCROOT)"

installhdrs::
	cd "$(SRCROOT)/mDNSMacOSX"; xcodebuild installhdrs OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT) DSTROOT=$(DSTROOT) MVERS=$(MVERS) SDKROOT=$(SDKROOT)  -target SystemLibraries

clean::
	echo clean
