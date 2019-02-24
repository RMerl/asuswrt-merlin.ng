AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtpmtss \
	-DIPSEC_SCRIPT=\"${ipsec_script}\" \
	-DSWID_DIRECTORY=\"${prefix}/share\"

AM_CFLAGS = \
	$(json_CFLAGS)

ipseclib_LTLIBRARIES = libimcv.la

libimcv_la_LDFLAGS = \
	-no-undefined

libimcv_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtncif/libtncif.la \
	$(top_builddir)/src/libtpmtss/libtpmtss.la \
	$(json_LIBS)

if USE_WINDOWS
  libimcv_la_LIBADD += -lws2_32
endif

libimcv_la_SOURCES = \
	imcv.h imcv.c \
	imc/imc_agent.h imc/imc_agent.c imc/imc_state.h \
	imc/imc_msg.h imc/imc_msg.c \
	imc/imc_os_info.h imc/imc_os_info.c \
	imv/imv_agent.h imv/imv_agent.c imv/imv_state.h \
	imv/imv_agent_if.h imv/imv_if.h \
	imv/imv_database.h imv/imv_database.c \
	imv/imv_msg.h imv/imv_msg.c \
	imv/imv_lang_string.h imv/imv_lang_string.c \
	imv/imv_os_info.h imv/imv_os_info.c \
	imv/imv_reason_string.h imv/imv_reason_string.c \
	imv/imv_remediation_string.h imv/imv_remediation_string.c \
	imv/imv_session.h imv/imv_session.c \
	imv/imv_session_manager.h imv/imv_session_manager.c \
	imv/imv_workitem.h imv/imv_workitem.c \
	generic/generic_attr_bool.h generic/generic_attr_bool.c \
	generic/generic_attr_chunk.h generic/generic_attr_chunk.c \
	generic/generic_attr_string.h generic/generic_attr_string.c \
	ietf/ietf_attr.h ietf/ietf_attr.c \
	ietf/ietf_attr_assess_result.h ietf/ietf_attr_assess_result.c \
	ietf/ietf_attr_attr_request.h ietf/ietf_attr_attr_request.c \
	ietf/ietf_attr_fwd_enabled.h ietf/ietf_attr_fwd_enabled.c \
	ietf/ietf_attr_installed_packages.h ietf/ietf_attr_installed_packages.c \
	ietf/ietf_attr_numeric_version.h ietf/ietf_attr_numeric_version.c \
	ietf/ietf_attr_op_status.h ietf/ietf_attr_op_status.c \
	ietf/ietf_attr_pa_tnc_error.h ietf/ietf_attr_pa_tnc_error.c \
	ietf/ietf_attr_port_filter.h ietf/ietf_attr_port_filter.c \
	ietf/ietf_attr_product_info.h ietf/ietf_attr_product_info.c \
	ietf/ietf_attr_remediation_instr.h ietf/ietf_attr_remediation_instr.c \
	ietf/ietf_attr_string_version.h ietf/ietf_attr_string_version.c \
	ietf/swima/ietf_swima_attr_req.h ietf/swima/ietf_swima_attr_req.c \
	ietf/swima/ietf_swima_attr_sw_inv.h ietf/swima/ietf_swima_attr_sw_inv.c \
	ietf/swima/ietf_swima_attr_sw_ev.h ietf/swima/ietf_swima_attr_sw_ev.c \
	ita/ita_attr.h ita/ita_attr.c \
	ita/ita_attr_command.h ita/ita_attr_command.c \
	ita/ita_attr_dummy.h ita/ita_attr_dummy.c \
	ita/ita_attr_get_settings.h ita/ita_attr_get_settings.c \
	ita/ita_attr_settings.h ita/ita_attr_settings.c \
	ita/ita_attr_angel.h ita/ita_attr_angel.c \
	os_info/os_info.h os_info/os_info.c \
	pa_tnc/pa_tnc_attr.h \
	pa_tnc/pa_tnc_msg.h pa_tnc/pa_tnc_msg.c \
	pa_tnc/pa_tnc_attr_manager.h pa_tnc/pa_tnc_attr_manager.c \
	pts/pts.h pts/pts.c \
	pts/pts_error.h pts/pts_error.c \
	pts/pts_pcr.h pts/pts_pcr.c \
	pts/pts_proto_caps.h \
	pts/pts_req_func_comp_evid.h \
	pts/pts_creds.h pts/pts_creds.c \
	pts/pts_database.h pts/pts_database.c \
	pts/pts_dh_group.h pts/pts_dh_group.c \
	pts/pts_file_meas.h pts/pts_file_meas.c \
	pts/pts_file_meta.h pts/pts_file_meta.c \
	pts/pts_file_type.h pts/pts_file_type.c \
	pts/pts_ima_bios_list.h pts/pts_ima_bios_list.c \
	pts/pts_ima_event_list.h pts/pts_ima_event_list.c \
	pts/pts_meas_algo.h pts/pts_meas_algo.c \
	pts/components/pts_component.h \
	pts/components/pts_component_manager.h pts/components/pts_component_manager.c \
	pts/components/pts_comp_evidence.h pts/components/pts_comp_evidence.c \
	pts/components/pts_comp_func_name.h pts/components/pts_comp_func_name.c \
	pts/components/ita/ita_comp_func_name.h pts/components/ita/ita_comp_func_name.c \
	pts/components/ita/ita_comp_ima.h pts/components/ita/ita_comp_ima.c \
	pts/components/ita/ita_comp_tboot.h pts/components/ita/ita_comp_tboot.c \
	pts/components/ita/ita_comp_tgrub.h pts/components/ita/ita_comp_tgrub.c \
	pts/components/tcg/tcg_comp_func_name.h pts/components/tcg/tcg_comp_func_name.c \
	pwg/pwg_attr.h pwg/pwg_attr.c \
	pwg/pwg_attr_vendor_smi_code.h pwg/pwg_attr_vendor_smi_code.c \
	rest/rest.h rest/rest.c \
	seg/seg_contract.h seg/seg_contract.c \
	seg/seg_contract_manager.h seg/seg_contract_manager.c \
	seg/seg_env.h seg/seg_env.c \
	swid_gen/swid_gen.h swid_gen/swid_gen.c \
	swid_gen/swid_gen_info.h swid_gen/swid_gen_info.c \
	swima/swima_data_model.h swima/swima_data_model.c \
	swima/swima_record.h swima/swima_record.c \
	swima/swima_event.h swima/swima_event.c \
	swima/swima_events.h swima/swima_events.c \
	swima/swima_inventory.h swima/swima_inventory.c \
	swima/swima_collector.h swima/swima_collector.c \
	swima/swima_error.h swima/swima_error.c \
	tcg/tcg_attr.h tcg/tcg_attr.c \
	tcg/pts/tcg_pts_attr_proto_caps.h tcg/pts/tcg_pts_attr_proto_caps.c \
	tcg/pts/tcg_pts_attr_dh_nonce_params_req.h tcg/pts/tcg_pts_attr_dh_nonce_params_req.c \
	tcg/pts/tcg_pts_attr_dh_nonce_params_resp.h tcg/pts/tcg_pts_attr_dh_nonce_params_resp.c \
	tcg/pts/tcg_pts_attr_dh_nonce_finish.h tcg/pts/tcg_pts_attr_dh_nonce_finish.c \
	tcg/pts/tcg_pts_attr_meas_algo.h tcg/pts/tcg_pts_attr_meas_algo.c \
	tcg/pts/tcg_pts_attr_get_tpm_version_info.h tcg/pts/tcg_pts_attr_get_tpm_version_info.c \
	tcg/pts/tcg_pts_attr_tpm_version_info.h tcg/pts/tcg_pts_attr_tpm_version_info.c \
	tcg/pts/tcg_pts_attr_get_aik.h tcg/pts/tcg_pts_attr_get_aik.c \
	tcg/pts/tcg_pts_attr_aik.h tcg/pts/tcg_pts_attr_aik.c \
	tcg/pts/tcg_pts_attr_req_func_comp_evid.h tcg/pts/tcg_pts_attr_req_func_comp_evid.c \
	tcg/pts/tcg_pts_attr_gen_attest_evid.h tcg/pts/tcg_pts_attr_gen_attest_evid.c \
	tcg/pts/tcg_pts_attr_simple_comp_evid.h tcg/pts/tcg_pts_attr_simple_comp_evid.c \
	tcg/pts/tcg_pts_attr_simple_evid_final.h tcg/pts/tcg_pts_attr_simple_evid_final.c \
	tcg/pts/tcg_pts_attr_req_file_meas.h tcg/pts/tcg_pts_attr_req_file_meas.c \
	tcg/pts/tcg_pts_attr_file_meas.h tcg/pts/tcg_pts_attr_file_meas.c \
	tcg/pts/tcg_pts_attr_req_file_meta.h tcg/pts/tcg_pts_attr_req_file_meta.c \
	tcg/pts/tcg_pts_attr_unix_file_meta.h tcg/pts/tcg_pts_attr_unix_file_meta.c \
	tcg/seg/tcg_seg_attr_max_size.h tcg/seg/tcg_seg_attr_max_size.c \
	tcg/seg/tcg_seg_attr_seg_env.h tcg/seg/tcg_seg_attr_seg_env.c \
	tcg/seg/tcg_seg_attr_next_seg.h tcg/seg/tcg_seg_attr_next_seg.c

ipsec_SCRIPTS = imv/_imv_policy
EXTRA_DIST = imv/_imv_policy Android.mk

templatesdir = $(pkgdatadir)/templates/database/imv
dist_templates_DATA = imv/tables.sql imv/tables-mysql.sql imv/data.sql

ipsec_PROGRAMS = imv_policy_manager
imv_policy_manager_SOURCES = \
	imv/imv_policy_manager.c \
	imv/imv_policy_manager_usage.h imv/imv_policy_manager_usage.c
imv_policy_manager_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtncif/libtncif.la
#imv/imv_policy_manager.o :	$(top_builddir)/config.status

SUBDIRS = .

if USE_IMC_TEST
  SUBDIRS += plugins/imc_test
endif

if USE_IMV_TEST
  SUBDIRS += plugins/imv_test
endif

if USE_IMC_SCANNER
  SUBDIRS += plugins/imc_scanner
endif

if USE_IMV_SCANNER
  SUBDIRS += plugins/imv_scanner
endif

if USE_IMC_OS
  SUBDIRS += plugins/imc_os
endif

if USE_IMV_OS
  SUBDIRS += plugins/imv_os
endif

if USE_IMC_ATTESTATION
  SUBDIRS += plugins/imc_attestation
endif

if USE_IMV_ATTESTATION
  SUBDIRS += plugins/imv_attestation
endif

if USE_IMC_SWIMA
  SUBDIRS += plugins/imc_swima
endif

if USE_IMV_SWIMA
  SUBDIRS += plugins/imv_swima
endif

if USE_IMC_HCD
  SUBDIRS += plugins/imc_hcd
endif

if USE_IMV_HCD
  SUBDIRS += plugins/imv_hcd
endif

TESTS = imcv_tests

check_PROGRAMS = $(TESTS)

imcv_tests_SOURCES = \
	ita/ita_attr_command.c \
	pa_tnc/pa_tnc_attr_manager.c \
	seg/seg_env.c seg/seg_contract.c \
	seg/seg_contract_manager.c \
	swid_gen/swid_gen.c \
	swima/swima_data_model.c \
	swima/swima_event.c \
	swima/swima_events.c \
	swima/swima_record.c \
	swima/swima_inventory.c \
	swima/swima_collector.c \
	suites/test_imcv_seg.c \
	suites/test_imcv_swima.c \
	ietf/ietf_attr_pa_tnc_error.c \
	ietf/swima/ietf_swima_attr_req.c \
	ietf/swima/ietf_swima_attr_sw_inv.c \
	ietf/swima/ietf_swima_attr_sw_ev.c \
	tcg/seg/tcg_seg_attr_seg_env.c \
	imcv.c imcv_tests.h imcv_tests.c

imcv_tests_CFLAGS = \
	-I$(top_srcdir)/src/libimcv \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	@COVERAGE_CFLAGS@

imcv_tests_LDFLAGS = @COVERAGE_LDFLAGS@
imcv_tests_LDADD = \
	$(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libtpmtss/libtpmtss.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la
