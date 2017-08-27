SUBDIRS = init

sbin_PROGRAMS=alsactl
man_MANS=alsactl.1
if USE_XMLTO
man_MANS += alsactl_init.7
endif
EXTRA_DIST=alsactl.1 alsactl_init.xml

alsactl_SOURCES=alsactl.c state.c utils.c init_parse.c
noinst_HEADERS=alsactl.h list.h init_sysdeps.c init_utils_string.c init_utils_run.c init_sysfs.c

%.7: %.xml
	xmlto man $?
