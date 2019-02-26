ipsec_PROGRAMS = conftest

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DPLUGINS=\""${charon_plugins}\""

AM_CFLAGS = $(PLUGIN_CFLAGS)

conftest_SOURCES = conftest.c conftest.h config.c config.h actions.c actions.h \
	hooks/hook.h hooks/ike_auth_fill.c hooks/unsort_message.c \
	hooks/add_notify.c hooks/unencrypted_notify.c hooks/ignore_message.c \
	hooks/add_payload.c hooks/set_critical.c hooks/force_cookie.c \
	hooks/set_ike_version.c hooks/pretend_auth.c hooks/set_length.c \
	hooks/log_proposals.c hooks/set_proposal_number.c hooks/log_ke.c \
	hooks/log_id.c hooks/custom_proposal.c hooks/set_ike_spi.c \
	hooks/set_ike_request.c hooks/set_reserved.c hooks/set_ike_initiator.c \
	hooks/log_ts.c hooks/rebuild_auth.c hooks/reset_seq.c

conftest_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libcharon/libcharon.la \
	-lm $(PTHREADLIB) $(ATOMICLIB) $(DLLIB)

EXTRA_DIST = README
