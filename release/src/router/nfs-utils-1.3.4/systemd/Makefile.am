## Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES = Makefile.in

unit_files =  \
    nfs-client.target \
    \
    nfs-config.service \
    nfs-mountd.service \
    nfs-server.service \
    nfs-utils.service \
    rpc-statd-notify.service \
    rpc-statd.service \
    \
    proc-fs-nfsd.mount \
    var-lib-nfs-rpc_pipefs.mount

if CONFIG_NFSV4
unit_files += \
    nfs-idmapd.service
endif

if CONFIG_NFSV41
unit_files += \
    nfs-blkmap.service
endif

if CONFIG_GSS
unit_files += \
    auth-rpcgss-module.service \
    rpc-gssd.service

if CONFIG_SVCGSS
unit_files += \
    rpc-svcgssd.service
endif
endif

EXTRA_DIST = $(unit_files)

unit_dir = /usr/lib/systemd/system

if INSTALL_SYSTEMD
install-data-hook: $(unit_files)
	mkdir -p $(DESTDIR)/$(unitdir)
	cp $(unit_files) $(DESTDIR)/$(unitdir)
endif
