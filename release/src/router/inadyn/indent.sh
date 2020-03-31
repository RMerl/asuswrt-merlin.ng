#!/bin/sh
# Linux coding style
#
# With the -T <type> we can inform indent about non-ANSI/ISO types
# that we've added, so indent doesn't insert spaces in odd places. 
# 
indent --linux-style --line-length112 --dont-format-comments \
-T size_t -T sigset_t -T timeval_t -T pid_t -T pthread_t \
-T time_t -T uint32_t -T uint16_t -T uint8_t -T socklen_t \
-T ddns_t -T ddns_user_t -T ddns_creds_t -T ddns_info_t -T ddns_sysinfo_t \
-T ddns_cmd_t -T ddns_system_t -T ddns_server_name_t -T ddns_alias_t \
-T http_client_t -T http_trans_t -T tcp_sock_t \
$*
