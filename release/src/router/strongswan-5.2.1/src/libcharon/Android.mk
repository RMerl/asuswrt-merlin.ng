LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libcharon_la_SOURCES := \
bus/bus.c bus/bus.h \
bus/listeners/listener.h \
bus/listeners/logger.h \
bus/listeners/file_logger.c bus/listeners/file_logger.h \
config/backend_manager.c config/backend_manager.h config/backend.h \
config/child_cfg.c config/child_cfg.h \
config/ike_cfg.c config/ike_cfg.h \
config/peer_cfg.c config/peer_cfg.h \
config/proposal.c config/proposal.h \
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
kernel/kernel_handler.c kernel/kernel_handler.h \
network/receiver.c network/receiver.h network/sender.c network/sender.h \
network/socket.c network/socket.h \
network/socket_manager.c network/socket_manager.h \
processing/jobs/acquire_job.c processing/jobs/acquire_job.h \
processing/jobs/delete_child_sa_job.c processing/jobs/delete_child_sa_job.h \
processing/jobs/delete_ike_sa_job.c processing/jobs/delete_ike_sa_job.h \
processing/jobs/migrate_job.c processing/jobs/migrate_job.h \
processing/jobs/process_message_job.c processing/jobs/process_message_job.h \
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
sa/task_manager.h sa/task_manager.c \
sa/shunt_manager.c sa/shunt_manager.h \
sa/trap_manager.c sa/trap_manager.h \
sa/task.c sa/task.h

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
sa/ikev2/tasks/ike_mobike.c sa/ikev2/tasks/ike_mobike.h \
sa/ikev2/tasks/ike_rekey.c sa/ikev2/tasks/ike_rekey.h \
sa/ikev2/tasks/ike_reauth.c sa/ikev2/tasks/ike_reauth.h \
sa/ikev2/tasks/ike_auth_lifetime.c sa/ikev2/tasks/ike_auth_lifetime.h \
sa/ikev2/tasks/ike_vendor.c sa/ikev2/tasks/ike_vendor.h

libcharon_la_SOURCES += \
sa/ikev1/keymat_v1.c sa/ikev1/keymat_v1.h \
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

libcharon_la_SOURCES += \
    bus/listeners/sys_logger.c bus/listeners/sys_logger.h

LOCAL_SRC_FILES := $(filter %.c,$(libcharon_la_SOURCES))

# adding the plugin source files

LOCAL_SRC_FILES += $(call add_plugin, android-dns)
ifneq ($(call plugin_enabled, android-dns),)
LOCAL_SHARED_LIBRARIES += libcutils
endif

LOCAL_SRC_FILES += $(call add_plugin, android-log)
ifneq ($(call plugin_enabled, android-log),)
LOCAL_LDLIBS += -llog
endif

LOCAL_SRC_FILES += $(call add_plugin, attr)

LOCAL_SRC_FILES += $(call add_plugin, eap-aka)

LOCAL_SRC_FILES += $(call add_plugin, eap-aka-3gpp2)
ifneq ($(call plugin_enabled, eap-aka-3gpp2),)
LOCAL_C_INCLUDES += $(libgmp_PATH)
LOCAL_SHARED_LIBRARIES += libgmp
endif

LOCAL_SRC_FILES += $(call add_plugin, eap-gtc)

LOCAL_SRC_FILES += $(call add_plugin, eap-identity)

LOCAL_SRC_FILES += $(call add_plugin, eap-md5)

LOCAL_SRC_FILES += $(call add_plugin, eap-mschapv2)

LOCAL_SRC_FILES += $(call add_plugin, eap-sim)

LOCAL_SRC_FILES += $(call add_plugin, eap-simaka-sql)

LOCAL_SRC_FILES += $(call add_plugin, eap-simaka-pseudonym)

LOCAL_SRC_FILES += $(call add_plugin, eap-simaka-reauth)

LOCAL_SRC_FILES += $(call add_plugin, eap-sim-file)

# adding libakasim if either eap-aka or eap-sim is enabled
ifneq ($(or $(call plugin_enabled, eap-aka), $(call plugin_enabled, eap-sim)),)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libsimaka/
LOCAL_SRC_FILES += $(addprefix ../libsimaka/, \
		simaka_message.h simaka_message.c \
		simaka_crypto.h simaka_crypto.c \
		simaka_manager.h simaka_manager.c \
		simaka_card.h simaka_provider.h simaka_hooks.h \
	)
endif

LOCAL_SRC_FILES += $(call add_plugin, eap-tls)

LOCAL_SRC_FILES += $(call add_plugin, eap-ttls)
ifneq ($(call plugin_enabled, eap-ttls),)
# for radius_message.h
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libradius/
endif

LOCAL_SRC_FILES += $(call add_plugin, eap-peap)

LOCAL_SRC_FILES += $(call add_plugin, eap-tnc)
ifneq ($(call plugin_enabled, eap-tnc),)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libtnccs/ $(LOCAL_PATH)/../libtncif/
LOCAL_SHARED_LIBRARIES += libtnccs libtncif
endif

# adding libtls if any of the four plugins above is enabled
ifneq ($(or $(call plugin_enabled, eap-tls), $(call plugin_enabled, eap-ttls), \
			$(call plugin_enabled, eap-peap), $(call plugin_enabled, eap-tnc)),)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libtls/
LOCAL_SRC_FILES += $(addprefix ../libtls/, \
		tls_protection.c tls_compression.c tls_fragmentation.c tls_alert.c \
		tls_crypto.c tls_prf.c tls_socket.c tls_eap.c tls_cache.c tls_peer.c \
		tls_aead_expl.c tls_aead_impl.c tls_aead_null.c tls_aead.c \
		tls_server.c tls.c \
	)
endif

LOCAL_SRC_FILES += $(call add_plugin, load-tester)

LOCAL_SRC_FILES += $(call add_plugin, socket-default)

LOCAL_SRC_FILES += $(call add_plugin, socket-dynamic)

LOCAL_SRC_FILES += $(call add_plugin, stroke)
ifneq ($(call plugin_enabled, stroke),)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../stroke/
endif

# build libcharon --------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/include \
	$(strongswan_PATH)/src/libhydra \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libcharon

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan libhydra

include $(BUILD_SHARED_LIBRARY)

