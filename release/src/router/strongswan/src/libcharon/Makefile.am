ipseclib_LTLIBRARIES = libcharon.la

libcharon_la_SOURCES = \
attributes/attributes.c attributes/attributes.h \
attributes/attribute_provider.h attributes/attribute_handler.h \
attributes/attribute_manager.c attributes/attribute_manager.h \
attributes/mem_pool.c attributes/mem_pool.h \
bus/bus.c bus/bus.h \
bus/listeners/listener.h \
bus/listeners/logger.h \
bus/listeners/custom_logger.h \
bus/listeners/file_logger.c bus/listeners/file_logger.h \
config/backend_manager.c config/backend_manager.h config/backend.h \
config/child_cfg.c config/child_cfg.h \
config/ike_cfg.c config/ike_cfg.h \
config/peer_cfg.c config/peer_cfg.h \
control/controller.c control/controller.h \
daemon.c daemon.h \
encoding/generator.c encoding/generator.h \
encoding/message.c encoding/message.h \
encoding/parser.c encoding/parser.h \
encoding/payloads/auth_payload.c encoding/payloads/auth_payload.h \
encoding/payloads/cert_payload.c encoding/payloads/cert_payload.h \
encoding/payloads/certreq_payload.c encoding/payloads/certreq_payload.h \
encoding/payloads/configuration_attribute.c encoding/payloads/configuration_attribute.h \
encoding/payloads/cp_payload.c encoding/payloads/cp_payload.h \
encoding/payloads/delete_payload.c encoding/payloads/delete_payload.h \
encoding/payloads/eap_payload.c encoding/payloads/eap_payload.h \
encoding/payloads/encodings.c encoding/payloads/encodings.h \
encoding/payloads/encrypted_payload.c encoding/payloads/encrypted_payload.h \
encoding/payloads/encrypted_fragment_payload.h \
encoding/payloads/id_payload.c encoding/payloads/id_payload.h \
encoding/payloads/ike_header.c encoding/payloads/ike_header.h \
encoding/payloads/ke_payload.c  encoding/payloads/ke_payload.h \
encoding/payloads/nonce_payload.c encoding/payloads/nonce_payload.h \
encoding/payloads/notify_payload.c encoding/payloads/notify_payload.h \
encoding/payloads/payload.c encoding/payloads/payload.h \
encoding/payloads/proposal_substructure.c encoding/payloads/proposal_substructure.h \
encoding/payloads/sa_payload.c encoding/payloads/sa_payload.h \
encoding/payloads/traffic_selector_substructure.c encoding/payloads/traffic_selector_substructure.h \
encoding/payloads/transform_attribute.c encoding/payloads/transform_attribute.h \
encoding/payloads/transform_substructure.c encoding/payloads/transform_substructure.h \
encoding/payloads/ts_payload.c encoding/payloads/ts_payload.h \
encoding/payloads/unknown_payload.c encoding/payloads/unknown_payload.h \
encoding/payloads/vendor_id_payload.c encoding/payloads/vendor_id_payload.h \
encoding/payloads/hash_payload.c encoding/payloads/hash_payload.h \
encoding/payloads/fragment_payload.c encoding/payloads/fragment_payload.h \
kernel/kernel_interface.c kernel/kernel_interface.h \
kernel/kernel_ipsec.c kernel/kernel_ipsec.h \
kernel/kernel_net.c kernel/kernel_net.h \
kernel/kernel_listener.h kernel/kernel_handler.c kernel/kernel_handler.h \
network/receiver.c network/receiver.h network/sender.c network/sender.h \
network/socket.c network/socket.h \
network/socket_manager.c network/socket_manager.h \
processing/jobs/acquire_job.c processing/jobs/acquire_job.h \
processing/jobs/delete_child_sa_job.c processing/jobs/delete_child_sa_job.h \
processing/jobs/delete_ike_sa_job.c processing/jobs/delete_ike_sa_job.h \
processing/jobs/migrate_job.c processing/jobs/migrate_job.h \
processing/jobs/process_message_job.c processing/jobs/process_message_job.h \
processing/jobs/redirect_job.c processing/jobs/redirect_job.h \
processing/jobs/rekey_child_sa_job.c processing/jobs/rekey_child_sa_job.h \
processing/jobs/rekey_ike_sa_job.c processing/jobs/rekey_ike_sa_job.h \
processing/jobs/retransmit_job.c processing/jobs/retransmit_job.h \
processing/jobs/retry_initiate_job.c processing/jobs/retry_initiate_job.h \
processing/jobs/send_dpd_job.c processing/jobs/send_dpd_job.h \
processing/jobs/send_keepalive_job.c processing/jobs/send_keepalive_job.h \
processing/jobs/start_action_job.c processing/jobs/start_action_job.h \
processing/jobs/roam_job.c processing/jobs/roam_job.h \
processing/jobs/update_sa_job.c processing/jobs/update_sa_job.h \
processing/jobs/inactivity_job.c processing/jobs/inactivity_job.h \
processing/jobs/initiate_tasks_job.c processing/jobs/initiate_tasks_job.h \
sa/eap/eap_method.c sa/eap/eap_method.h sa/eap/eap_inner_method.h \
sa/eap/eap_manager.c sa/eap/eap_manager.h \
sa/xauth/xauth_method.c sa/xauth/xauth_method.h \
sa/xauth/xauth_manager.c sa/xauth/xauth_manager.h \
sa/authenticator.c sa/authenticator.h \
sa/child_sa.c sa/child_sa.h \
sa/ike_sa.c sa/ike_sa.h \
sa/ike_sa_id.c sa/ike_sa_id.h \
sa/keymat.h sa/keymat.c \
sa/ike_sa_manager.c sa/ike_sa_manager.h \
sa/child_sa_manager.c sa/child_sa_manager.h \
sa/task_manager.h sa/task_manager.c \
sa/shunt_manager.c sa/shunt_manager.h \
sa/trap_manager.c sa/trap_manager.h \
sa/redirect_provider.h sa/redirect_manager.c sa/redirect_manager.h \
sa/task.c sa/task.h

if USE_IKEV2
libcharon_la_SOURCES += \
sa/ikev2/keymat_v2.c sa/ikev2/keymat_v2.h \
sa/ikev2/task_manager_v2.c sa/ikev2/task_manager_v2.h \
sa/ikev2/authenticators/eap_authenticator.c sa/ikev2/authenticators/eap_authenticator.h \
sa/ikev2/authenticators/psk_authenticator.c sa/ikev2/authenticators/psk_authenticator.h \
sa/ikev2/authenticators/pubkey_authenticator.c sa/ikev2/authenticators/pubkey_authenticator.h \
sa/ikev2/tasks/child_create.c sa/ikev2/tasks/child_create.h \
sa/ikev2/tasks/child_delete.c sa/ikev2/tasks/child_delete.h \
sa/ikev2/tasks/child_rekey.c sa/ikev2/tasks/child_rekey.h \
sa/ikev2/tasks/ike_auth.c sa/ikev2/tasks/ike_auth.h \
sa/ikev2/tasks/ike_cert_pre.c sa/ikev2/tasks/ike_cert_pre.h \
sa/ikev2/tasks/ike_cert_post.c sa/ikev2/tasks/ike_cert_post.h \
sa/ikev2/tasks/ike_config.c sa/ikev2/tasks/ike_config.h \
sa/ikev2/tasks/ike_delete.c sa/ikev2/tasks/ike_delete.h \
sa/ikev2/tasks/ike_dpd.c sa/ikev2/tasks/ike_dpd.h \
sa/ikev2/tasks/ike_init.c sa/ikev2/tasks/ike_init.h \
sa/ikev2/tasks/ike_natd.c sa/ikev2/tasks/ike_natd.h \
sa/ikev2/tasks/ike_mid_sync.c sa/ikev2/tasks/ike_mid_sync.h \
sa/ikev2/tasks/ike_mobike.c sa/ikev2/tasks/ike_mobike.h \
sa/ikev2/tasks/ike_rekey.c sa/ikev2/tasks/ike_rekey.h \
sa/ikev2/tasks/ike_reauth.c sa/ikev2/tasks/ike_reauth.h \
sa/ikev2/tasks/ike_reauth_complete.c sa/ikev2/tasks/ike_reauth_complete.h \
sa/ikev2/tasks/ike_redirect.c sa/ikev2/tasks/ike_redirect.h \
sa/ikev2/tasks/ike_auth_lifetime.c sa/ikev2/tasks/ike_auth_lifetime.h \
sa/ikev2/tasks/ike_vendor.c sa/ikev2/tasks/ike_vendor.h \
sa/ikev2/tasks/ike_verify_peer_cert.c sa/ikev2/tasks/ike_verify_peer_cert.h
endif

if USE_IKEV1
libcharon_la_SOURCES += \
sa/ikev1/keymat_v1.c sa/ikev1/keymat_v1.h \
sa/ikev1/iv_manager.c sa/ikev1/iv_manager.h \
sa/ikev1/task_manager_v1.c sa/ikev1/task_manager_v1.h \
sa/ikev1/authenticators/psk_v1_authenticator.c sa/ikev1/authenticators/psk_v1_authenticator.h \
sa/ikev1/authenticators/pubkey_v1_authenticator.c sa/ikev1/authenticators/pubkey_v1_authenticator.h \
sa/ikev1/authenticators/hybrid_authenticator.c sa/ikev1/authenticators/hybrid_authenticator.h \
sa/ikev1/phase1.c sa/ikev1/phase1.h \
sa/ikev1/tasks/main_mode.c sa/ikev1/tasks/main_mode.h \
sa/ikev1/tasks/aggressive_mode.c sa/ikev1/tasks/aggressive_mode.h \
sa/ikev1/tasks/informational.c sa/ikev1/tasks/informational.h \
sa/ikev1/tasks/isakmp_cert_pre.c sa/ikev1/tasks/isakmp_cert_pre.h \
sa/ikev1/tasks/isakmp_cert_post.c sa/ikev1/tasks/isakmp_cert_post.h \
sa/ikev1/tasks/isakmp_natd.c sa/ikev1/tasks/isakmp_natd.h \
sa/ikev1/tasks/isakmp_vendor.c sa/ikev1/tasks/isakmp_vendor.h \
sa/ikev1/tasks/isakmp_delete.c sa/ikev1/tasks/isakmp_delete.h \
sa/ikev1/tasks/isakmp_dpd.c sa/ikev1/tasks/isakmp_dpd.h \
sa/ikev1/tasks/xauth.c sa/ikev1/tasks/xauth.h \
sa/ikev1/tasks/quick_mode.c sa/ikev1/tasks/quick_mode.h \
sa/ikev1/tasks/quick_delete.c sa/ikev1/tasks/quick_delete.h \
sa/ikev1/tasks/mode_config.c sa/ikev1/tasks/mode_config.h \
processing/jobs/dpd_timeout_job.c processing/jobs/dpd_timeout_job.h \
processing/jobs/adopt_children_job.c processing/jobs/adopt_children_job.h
endif

if USE_SYSLOG
  libcharon_la_SOURCES += \
    bus/listeners/sys_logger.c bus/listeners/sys_logger.h
endif

daemon.lo :		$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DIPSEC_DIR=\"${ipsecdir}\" \
	-DIPSEC_PIDDIR=\"${piddir}\"

AM_CFLAGS = \
  @COVERAGE_CFLAGS@

AM_LDFLAGS = \
  -no-undefined

libcharon_la_LIBADD = \
  $(top_builddir)/src/libstrongswan/libstrongswan.la \
  -lm $(PTHREADLIB) $(ATOMICLIB) $(DLLIB) $(SOCKLIB)

if USE_WINDOWS
  libcharon_la_LIBADD += -lws2_32
endif

EXTRA_DIST = Android.mk

# compile options
#################

if USE_ME
  libcharon_la_SOURCES += encoding/payloads/endpoint_notify.c encoding/payloads/endpoint_notify.h \
    processing/jobs/initiate_mediation_job.c processing/jobs/initiate_mediation_job.h \
    processing/jobs/mediation_job.c processing/jobs/mediation_job.h \
    sa/ikev2/connect_manager.c sa/ikev2/connect_manager.h \
    sa/ikev2/mediation_manager.c sa/ikev2/mediation_manager.h \
    sa/ikev2/tasks/ike_me.c sa/ikev2/tasks/ike_me.h
endif

if STATIC_PLUGIN_CONSTRUCTORS
BUILT_SOURCES = $(srcdir)/plugin_constructors.c
CLEANFILES = $(srcdir)/plugin_constructors.c

$(srcdir)/plugin_constructors.c: $(top_srcdir)/src/libstrongswan/plugins/plugin_constructors.py
		$(AM_V_GEN) \
		$(PYTHON) $(top_srcdir)/src/libstrongswan/plugins/plugin_constructors.py ${c_plugins} > $@
endif

# build optional plugins
########################

if MONOLITHIC
SUBDIRS =
else
SUBDIRS = .
endif

if USE_LOAD_TESTER
  SUBDIRS += plugins/load_tester
if MONOLITHIC
  libcharon_la_LIBADD += plugins/load_tester/libstrongswan-load-tester.la
endif
endif

if USE_SAVE_KEYS
 SUBDIRS += plugins/save_keys
if MONOLITHIC
 libcharon_la_LIBADD += plugins/save_keys/libstrongswan-save-keys.la
endif
endif

if USE_SOCKET_DEFAULT
  SUBDIRS += plugins/socket_default
if MONOLITHIC
  libcharon_la_LIBADD += plugins/socket_default/libstrongswan-socket-default.la
endif
endif

if USE_SOCKET_DYNAMIC
  SUBDIRS += plugins/socket_dynamic
if MONOLITHIC
  libcharon_la_LIBADD += plugins/socket_dynamic/libstrongswan-socket-dynamic.la
endif
endif

if USE_SOCKET_WIN
  SUBDIRS += plugins/socket_win
if MONOLITHIC
  libcharon_la_LIBADD += plugins/socket_win/libstrongswan-socket-win.la
endif
endif

if USE_CONNMARK
  SUBDIRS += plugins/connmark
if MONOLITHIC
  libcharon_la_LIBADD += plugins/connmark/libstrongswan-connmark.la
endif
endif

if USE_BYPASS_LAN
  SUBDIRS += plugins/bypass_lan
if MONOLITHIC
  libcharon_la_LIBADD += plugins/bypass_lan/libstrongswan-bypass-lan.la
endif
endif

if USE_FORECAST
  SUBDIRS += plugins/forecast
if MONOLITHIC
  libcharon_la_LIBADD += plugins/forecast/libstrongswan-forecast.la
endif
endif

if USE_FARP
  SUBDIRS += plugins/farp
if MONOLITHIC
  libcharon_la_LIBADD += plugins/farp/libstrongswan-farp.la
endif
endif

if USE_COUNTERS
  SUBDIRS += plugins/counters
if MONOLITHIC
  libcharon_la_LIBADD += plugins/counters/libstrongswan-counters.la
endif
endif

if USE_STROKE
  SUBDIRS += plugins/stroke
if MONOLITHIC
  libcharon_la_LIBADD += plugins/stroke/libstrongswan-stroke.la
endif
endif

if USE_VICI
  SUBDIRS += plugins/vici
if MONOLITHIC
  libcharon_la_LIBADD += plugins/vici/libstrongswan-vici.la
endif
endif

if USE_SMP
  SUBDIRS += plugins/smp
if MONOLITHIC
  libcharon_la_LIBADD += plugins/smp/libstrongswan-smp.la
endif
endif

if USE_SQL
  SUBDIRS += plugins/sql
if MONOLITHIC
  libcharon_la_LIBADD += plugins/sql/libstrongswan-sql.la
endif
endif

if USE_DNSCERT
  SUBDIRS += plugins/dnscert
if MONOLITHIC
  libcharon_la_LIBADD += plugins/dnscert/libstrongswan-dnscert.la
endif
endif

if USE_IPSECKEY
  SUBDIRS += plugins/ipseckey
if MONOLITHIC
  libcharon_la_LIBADD += plugins/ipseckey/libstrongswan-ipseckey.la
endif
endif

if USE_UPDOWN
  SUBDIRS += plugins/updown
if MONOLITHIC
  libcharon_la_LIBADD += plugins/updown/libstrongswan-updown.la
endif
endif

if USE_EXT_AUTH
  SUBDIRS += plugins/ext_auth
if MONOLITHIC
  libcharon_la_LIBADD += plugins/ext_auth/libstrongswan-ext-auth.la
endif
endif

if USE_EAP_IDENTITY
  SUBDIRS += plugins/eap_identity
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_identity/libstrongswan-eap-identity.la
endif
endif

if USE_EAP_SIM
  SUBDIRS += plugins/eap_sim
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_sim/libstrongswan-eap-sim.la
endif
endif

if USE_EAP_SIM_FILE
  SUBDIRS += plugins/eap_sim_file
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_sim_file/libstrongswan-eap-sim-file.la
endif
endif

if USE_EAP_SIM_PCSC
  SUBDIRS += plugins/eap_sim_pcsc
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_sim_pcsc/libstrongswan-eap-sim-pcsc.la
endif
endif

if USE_EAP_SIMAKA_SQL
  SUBDIRS += plugins/eap_simaka_sql
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_simaka_sql/libstrongswan-eap-simaka-sql.la
endif
endif

if USE_EAP_SIMAKA_PSEUDONYM
  SUBDIRS += plugins/eap_simaka_pseudonym
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_simaka_pseudonym/libstrongswan-eap-simaka-pseudonym.la
endif
endif

if USE_EAP_SIMAKA_REAUTH
  SUBDIRS += plugins/eap_simaka_reauth
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_simaka_reauth/libstrongswan-eap-simaka-reauth.la
endif
endif

if USE_EAP_AKA
  SUBDIRS += plugins/eap_aka
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_aka/libstrongswan-eap-aka.la
endif
endif

if USE_EAP_AKA_3GPP
  SUBDIRS += plugins/eap_aka_3gpp
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_aka_3gpp/libstrongswan-eap-aka-3gpp.la
endif
endif

if USE_EAP_AKA_3GPP2
  SUBDIRS += plugins/eap_aka_3gpp2
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_aka_3gpp2/libstrongswan-eap-aka-3gpp2.la
endif
endif

if USE_SIMAKA
if MONOLITHIC
  # otherwise this library is linked to both the eap_aka and the eap_sim plugin
  libcharon_la_LIBADD += $(top_builddir)/src/libsimaka/libsimaka.la
endif
endif

if USE_EAP_MD5
  SUBDIRS += plugins/eap_md5
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_md5/libstrongswan-eap-md5.la
endif
endif

if USE_EAP_GTC
  SUBDIRS += plugins/eap_gtc
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_gtc/libstrongswan-eap-gtc.la
endif
endif

if USE_EAP_MSCHAPV2
  SUBDIRS += plugins/eap_mschapv2
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_mschapv2/libstrongswan-eap-mschapv2.la
endif
endif

if USE_EAP_DYNAMIC
  SUBDIRS += plugins/eap_dynamic
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_dynamic/libstrongswan-eap-dynamic.la
endif
endif

if USE_EAP_RADIUS
  SUBDIRS += plugins/eap_radius
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_radius/libstrongswan-eap-radius.la
endif
endif

if USE_EAP_TLS
  SUBDIRS += plugins/eap_tls
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_tls/libstrongswan-eap-tls.la
endif
endif

if USE_EAP_TTLS
  SUBDIRS += plugins/eap_ttls
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_ttls/libstrongswan-eap-ttls.la
endif
endif

if USE_EAP_PEAP
  SUBDIRS += plugins/eap_peap
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_peap/libstrongswan-eap-peap.la
endif
endif

if USE_EAP_TNC
  SUBDIRS += plugins/eap_tnc
if MONOLITHIC
  libcharon_la_LIBADD += plugins/eap_tnc/libstrongswan-eap-tnc.la
endif
endif

if USE_TLS
if MONOLITHIC
  # otherwise this library is linked to eap_tls
  libcharon_la_LIBADD += $(top_builddir)/src/libtls/libtls.la
endif
endif

if USE_RADIUS
if MONOLITHIC
  # otherwise this library is linked to eap_radius
  libcharon_la_LIBADD += $(top_builddir)/src/libradius/libradius.la
endif
endif

if USE_TNC_IFMAP
  SUBDIRS += plugins/tnc_ifmap
if MONOLITHIC
  libcharon_la_LIBADD += plugins/tnc_ifmap/libstrongswan-tnc-ifmap.la
endif
endif

if USE_TNC_PDP
  SUBDIRS += plugins/tnc_pdp
if MONOLITHIC
  libcharon_la_LIBADD += plugins/tnc_pdp/libstrongswan-tnc-pdp.la
endif
endif

if USE_LIBTNCCS
if MONOLITHIC
  # otherwise this library is linked to the respective plugins
  libcharon_la_LIBADD += $(top_builddir)/src/libtnccs/libtnccs.la
endif
endif

if USE_MEDSRV
  SUBDIRS += plugins/medsrv
if MONOLITHIC
  libcharon_la_LIBADD += plugins/medsrv/libstrongswan-medsrv.la
endif
endif

if USE_MEDCLI
  SUBDIRS += plugins/medcli
if MONOLITHIC
  libcharon_la_LIBADD += plugins/medcli/libstrongswan-medcli.la
endif
endif

if USE_DHCP
  SUBDIRS += plugins/dhcp
if MONOLITHIC
  libcharon_la_LIBADD += plugins/dhcp/libstrongswan-dhcp.la
endif
endif

if USE_OSX_ATTR
  SUBDIRS += plugins/osx_attr
if MONOLITHIC
  libcharon_la_LIBADD += plugins/osx_attr/libstrongswan-osx-attr.la
endif
endif

if USE_P_CSCF
  SUBDIRS += plugins/p_cscf
if MONOLITHIC
  libcharon_la_LIBADD += plugins/p_cscf/libstrongswan-p-cscf.la
endif
endif

if USE_ANDROID_DNS
  SUBDIRS += plugins/android_dns
if MONOLITHIC
  libcharon_la_LIBADD += plugins/android_dns/libstrongswan-android-dns.la
endif
endif

if USE_ANDROID_LOG
  SUBDIRS += plugins/android_log
if MONOLITHIC
  libcharon_la_LIBADD += plugins/android_log/libstrongswan-android-log.la
endif
endif

if USE_HA
  SUBDIRS += plugins/ha
if MONOLITHIC
  libcharon_la_LIBADD += plugins/ha/libstrongswan-ha.la
endif
endif

if USE_KERNEL_PFKEY
  SUBDIRS += plugins/kernel_pfkey
if MONOLITHIC
  libcharon_la_LIBADD += plugins/kernel_pfkey/libstrongswan-kernel-pfkey.la
endif
endif

if USE_KERNEL_PFROUTE
  SUBDIRS += plugins/kernel_pfroute
if MONOLITHIC
  libcharon_la_LIBADD += plugins/kernel_pfroute/libstrongswan-kernel-pfroute.la
endif
endif

if USE_KERNEL_NETLINK
  SUBDIRS += plugins/kernel_netlink
if MONOLITHIC
  libcharon_la_LIBADD += plugins/kernel_netlink/libstrongswan-kernel-netlink.la
endif
endif

if USE_KERNEL_LIBIPSEC
  SUBDIRS += plugins/kernel_libipsec
if MONOLITHIC
  libcharon_la_LIBADD += plugins/kernel_libipsec/libstrongswan-kernel-libipsec.la
endif
endif

if USE_KERNEL_WFP
  SUBDIRS += plugins/kernel_wfp
if MONOLITHIC
  libcharon_la_LIBADD += plugins/kernel_wfp/libstrongswan-kernel-wfp.la
endif
endif

if USE_KERNEL_IPH
  SUBDIRS += plugins/kernel_iph
if MONOLITHIC
  libcharon_la_LIBADD += plugins/kernel_iph/libstrongswan-kernel-iph.la
endif
endif

if USE_WHITELIST
  SUBDIRS += plugins/whitelist
if MONOLITHIC
  libcharon_la_LIBADD += plugins/whitelist/libstrongswan-whitelist.la
endif
endif

if USE_LOOKIP
  SUBDIRS += plugins/lookip
if MONOLITHIC
  libcharon_la_LIBADD += plugins/lookip/libstrongswan-lookip.la
endif
endif

if USE_ERROR_NOTIFY
  SUBDIRS += plugins/error_notify
if MONOLITHIC
  libcharon_la_LIBADD += plugins/error_notify/libstrongswan-error-notify.la
endif
endif

if USE_CERTEXPIRE
  SUBDIRS += plugins/certexpire
if MONOLITHIC
  libcharon_la_LIBADD += plugins/certexpire/libstrongswan-certexpire.la
endif
endif

if USE_SYSTIME_FIX
  SUBDIRS += plugins/systime_fix
if MONOLITHIC
  libcharon_la_LIBADD += plugins/systime_fix/libstrongswan-systime-fix.la
endif
endif

if USE_LED
  SUBDIRS += plugins/led
if MONOLITHIC
  libcharon_la_LIBADD += plugins/led/libstrongswan-led.la
endif
endif

if USE_DUPLICHECK
  SUBDIRS += plugins/duplicheck
if MONOLITHIC
  libcharon_la_LIBADD += plugins/duplicheck/libstrongswan-duplicheck.la
endif
endif

if USE_COUPLING
  SUBDIRS += plugins/coupling
if MONOLITHIC
  libcharon_la_LIBADD += plugins/coupling/libstrongswan-coupling.la
endif
endif

if USE_RADATTR
  SUBDIRS += plugins/radattr
if MONOLITHIC
  libcharon_la_LIBADD += plugins/radattr/libstrongswan-radattr.la
endif
endif

if USE_UCI
  SUBDIRS += plugins/uci
if MONOLITHIC
  libcharon_la_LIBADD += plugins/uci/libstrongswan-uci.la
endif
endif

if USE_ADDRBLOCK
  SUBDIRS += plugins/addrblock
if MONOLITHIC
  libcharon_la_LIBADD += plugins/addrblock/libstrongswan-addrblock.la
endif
endif

if USE_UNITY
  SUBDIRS += plugins/unity
if MONOLITHIC
  libcharon_la_LIBADD += plugins/unity/libstrongswan-unity.la
endif
endif

if USE_XAUTH_GENERIC
  SUBDIRS += plugins/xauth_generic
if MONOLITHIC
  libcharon_la_LIBADD += plugins/xauth_generic/libstrongswan-xauth-generic.la
endif
endif

if USE_XAUTH_EAP
  SUBDIRS += plugins/xauth_eap
if MONOLITHIC
  libcharon_la_LIBADD += plugins/xauth_eap/libstrongswan-xauth-eap.la
endif
endif

if USE_XAUTH_PAM
  SUBDIRS += plugins/xauth_pam
if MONOLITHIC
  libcharon_la_LIBADD += plugins/xauth_pam/libstrongswan-xauth-pam.la
endif
endif

if USE_XAUTH_NOAUTH
  SUBDIRS += plugins/xauth_noauth
if MONOLITHIC
  libcharon_la_LIBADD += plugins/xauth_noauth/libstrongswan-xauth-noauth.la
endif
endif

if USE_RESOLVE
  SUBDIRS += plugins/resolve
if MONOLITHIC
  libcharon_la_LIBADD += plugins/resolve/libstrongswan-resolve.la
endif
endif

if USE_ATTR
  SUBDIRS += plugins/attr
if MONOLITHIC
  libcharon_la_LIBADD += plugins/attr/libstrongswan-attr.la
endif
endif

if USE_ATTR_SQL
  SUBDIRS += plugins/attr_sql
if MONOLITHIC
  libcharon_la_LIBADD += plugins/attr_sql/libstrongswan-attr-sql.la
endif
endif

if MONOLITHIC
  SUBDIRS += .
endif

# build unit tests
##################

SUBDIRS += tests

if USE_EAP_AKA_3GPP
  SUBDIRS += plugins/eap_aka_3gpp/tests
endif
