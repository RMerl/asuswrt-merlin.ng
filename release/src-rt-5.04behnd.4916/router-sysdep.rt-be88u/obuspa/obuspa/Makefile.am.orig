AM_CPPFLAGS = -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/core \
              -I$(top_srcdir)/src/protobuf-c \
              -I$(top_srcdir)/src/vendor \
              -I$(top_srcdir)/src/libjson/ccan/json \
              -I$(top_srcdir)/src/include

# Enable warnings, treat warnings as errors, and stop compilation at
# first error.
# Sub-makes are free to supplement AM_CFLAGS, e.g.
#         AM_CFLAGS += -Wno-error
# as necessary.
AM_CFLAGS = -Wall -Wfatal-errors

bin_PROGRAMS = obuspa

OBUSPA_LOCAL_STATE_DIR = "$(localstatedir)/obuspa"

SOURCES = src/core/main.c \
                    src/core/mtp_exec.c \
                    src/core/dm_exec.c \
                    src/core/bdc_exec.c \
                    src/core/stomp.c \
                    src/core/socket_set.c \
                    src/core/msg_handler.c \
                    src/core/handle_get.c \
                    src/core/handle_set.c \
                    src/core/handle_add.c \
                    src/core/handle_delete.c \
                    src/core/handle_notify.c \
                    src/core/handle_operate.c \
                    src/core/handle_get_supported_protocol.c \
                    src/core/handle_get_instances.c \
                    src/core/handle_get_supported_dm.c \
                    src/core/group_get_vector.c \
                    src/core/group_set_vector.c \
                    src/core/proto_trace.c \
                    src/core/data_model.c \
                    src/core/error_resp.c \
                    src/core/usp_register.c \
                    src/core/usp_api.c \
                    src/core/dm_access.c \
                    src/core/device_local_agent.c \
                    src/core/device_controller.c \
                    src/core/device_mtp.c \
                    src/core/device_stomp.c \
                    src/core/device_subscription.c \
                    src/core/device_security.c \
                    src/core/device_ctrust.c \
                    src/core/device_bulkdata.c \
                    src/core/device_selftest_example.c \
                    src/core/device_time.c \
                    src/core/uptime.c \
                    src/core/rfc1123.c \
                    src/core/database.c \
                    src/core/usp_err.c \
                    src/core/usp_log.c \
                    src/core/usp_mem.c \
                    src/core/nu_ipaddr.c \
                    src/core/nu_macaddr.c \
                    src/core/retry_wait.c \
                    src/core/path_resolver.c \
                    src/core/str_vector.c \
                    src/core/int_vector.c \
                    src/core/kv_vector.c \
                    src/core/dm_inst_vector.c \
                    src/core/expr_vector.c \
                    src/core/dm_trans.c \
                    src/core/subs_vector.c \
                    src/core/subs_retry.c \
                    src/core/sync_timer.c \
                    src/core/cli_server.c \
                    src/core/cli_client.c \
                    src/core/iso8601.c \
                    src/core/text_utils.c \
                    src/core/os_utils.c \
                    src/core/device_request.c \
                    src/core/dllist.c \
                    src/libjson/ccan/json/json.c \
                    src/protobuf-c/usp-msg.pb-c.c \
                    src/protobuf-c/usp-record.pb-c.c \
                    src/protobuf-c/protobuf-c.c \
                    src/core/coap_common.c \
                    src/core/coap_client.c \
                    src/core/coap_server.c \
                    src/core/uri.c \
                    src/core/mqtt.c \
                    src/core/device_mqtt.c \
                    src/core/wsclient.c \
                    src/core/wsserver.c \
                    src/core/usp_record.c \
                    src/core/sar_vector.c \
                    src/core/e2e_context.c

obuspa_SOURCES = $(SOURCES)
obuspa_CPPFLAGS = $(openssl_CFLAGS) $(sqlite3_CFLAGS) $(libcurl_CFLAGS) $(zlib_CFLAGS) $(libmosquitto_CFLAGS) $(libwebsockets_CFLAGS)
obuspa_CPPFLAGS += -DOBUSPA_LOCAL_STATE_DIR=\"$(OBUSPA_LOCAL_STATE_DIR)\"
obuspa_CPPFLAGS +=  $(AM_CPPFLAGS) \
                      -Werror \
                      -Werror=unused-value \
                      -Werror=format \
                      -Winit-self \
                      -Wparentheses -Werror=parentheses \
                      -Wuninitialized -Werror=uninitialized -Wpointer-arith
# NOTE: In order for GCC to check for uninitialised variables, you need to turn optimisation on i.e. "-O2"

# Turn on rdynamic linker flags so that backtrace_symbols() call works
obuspa_LDFLAGS = -rdynamic

obuspa_LDADD = -lm -ldl -lpthread -lrt
obuspa_LDADD += $(openssl_LIBS) $(sqlite3_LIBS) $(libcurl_LIBS) $(zlib_LIBS) $(libmosquitto_LIBS) $(libwebsockets_LIBS)

obuspa_LDFLAGS += -Wl,-rpath=/usr/local/lib

# Create obuspa directory for usp.db etc on install
# This depends on your prefix setting (default localstatedir=/usr/local/var/)
# Default OBUSPA_LOCAL_STATE_DIR=/usr/local/var/obuspa
install-data-local:
	${mkdir_p} "${DESTDIR}/${OBUSPA_LOCAL_STATE_DIR}"
	if [ -n "${SUDO_USER}" ]; then \
		chown "${SUDO_USER}" "${DESTDIR}/${OBUSPA_LOCAL_STATE_DIR}"; \
	fi

uninstall-local:
	rm -rf "${DESTDIR}/${OBUSPA_LOCAL_STATE_DIR}"

# Import vendor makefile
include src/vendor/vendor.am
