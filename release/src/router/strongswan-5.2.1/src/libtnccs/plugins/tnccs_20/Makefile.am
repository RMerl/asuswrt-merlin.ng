AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tnccs-20.la
else
plugin_LTLIBRARIES = libstrongswan-tnccs-20.la
libstrongswan_tnccs_20_la_LIBADD = \
	$(top_builddir)/src/libtncif/libtncif.la \
	$(top_builddir)/src/libtnccs/libtnccs.la
endif

libstrongswan_tnccs_20_la_SOURCES = \
	tnccs_20_plugin.h tnccs_20_plugin.c tnccs_20.h tnccs_20.c \
	batch/pb_tnc_batch.h batch/pb_tnc_batch.c \
	messages/pb_tnc_msg.h messages/pb_tnc_msg.c \
	messages/ietf/pb_experimental_msg.h messages/ietf/pb_experimental_msg.c \
	messages/ietf/pb_pa_msg.h messages/ietf/pb_pa_msg.c \
	messages/ietf/pb_assessment_result_msg.h messages/ietf/pb_assessment_result_msg.c \
	messages/ietf/pb_access_recommendation_msg.h messages/ietf/pb_access_recommendation_msg.c \
	messages/ietf/pb_error_msg.h messages/ietf/pb_error_msg.c \
	messages/ietf/pb_language_preference_msg.h messages/ietf/pb_language_preference_msg.c \
	messages/ietf/pb_reason_string_msg.h messages/ietf/pb_reason_string_msg.c \
	messages/ietf/pb_remediation_parameters_msg.h messages/ietf/pb_remediation_parameters_msg.c \
	messages/tcg/pb_pdp_referral_msg.h messages/tcg/pb_pdp_referral_msg.c \
	state_machine/pb_tnc_state_machine.h state_machine/pb_tnc_state_machine.c

libstrongswan_tnccs_20_la_LDFLAGS = -module -avoid-version
