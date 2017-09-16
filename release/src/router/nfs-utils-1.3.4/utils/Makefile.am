## Process this file with automake to produce Makefile.in

OPTDIRS =

if CONFIG_NFSV4
OPTDIRS += idmapd
if CONFIG_NFSIDMAP
OPTDIRS += nfsidmap
endif
endif

if CONFIG_NFSV41
OPTDIRS += blkmapd
endif

if CONFIG_GSS
OPTDIRS += gssd
endif

if CONFIG_MOUNT
OPTDIRS += mount
endif

if CONFIG_NFSDCLTRACK
OPTDIRS += nfsdcltrack
endif

SUBDIRS = \
	exportfs \
	mountd \
	nfsd \
	nfsstat \
	showmount \
	statd \
	osd_login \
	$(OPTDIRS)

MAINTAINERCLEANFILES = Makefile.in
