ipsec_SCRIPTS = _updown
CLEANFILES = _updown
EXTRA_DIST = _updown.in

_updown : _updown.in
	$(AM_V_GEN) \
	sed \
	-e "s:\@sbindir\@:$(sbindir):" \
	-e "s:\@routing_table\@:$(routing_table):" \
	-e "s:\@routing_table_prio\@:$(routing_table_prio):" \
	$(srcdir)/$@.in > $@
	chmod +x $@
