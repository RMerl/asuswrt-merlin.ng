/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Wi-Fi SmartMesh & QoS Management
 * shared IPC definitions & functions
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: wbd_qmd_shared.h 764230 2018-05-24 08:40:37Z $
 */

#ifndef _WBD_QMD_SHARED_H_
#define _WBD_QMD_SHARED_H_

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

#define WBD_QMD_IPC_CMD_VER	1

/* Definition of Generic IPC Command Header */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_hdr {
	unsigned short version;	/* Version of CLI */
	unsigned char cmd_id;	/* Command ID */
	unsigned int len;	/* Length of the whole data including header */
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_hdr wbd_qmd_ipc_cmd_hdr_t;
#define WBD_QMD_IPC_HDR_FIXED_LEN			7u

/* ============================================================================ */
/* ------------------  Direction : SmartMesh Agent to QMD  -------------------- */

/* Type of IPC Command ID */
typedef enum wbd_qmd_ipc_type {
	WBD_TO_QMD_MSCS_SUCCESS			= 0,
	WBD_TO_QMD_MSCS_UNSOLICITED_RESP	= 1,
	WBD_TO_QMD_SCS_SUCCESS			= 2,
	WBD_TO_QMD_SCS_UNSOLICITED_RESP		= 3,
	WBD_TO_QMD_DSCP_POLICY_UPDATE		= 4,
	WBD_TO_QMD_POLICY_UPDATE		= 5
} wbd_qmd_ipc_cmd_type_t;

/* Definition of IPC Command : { 0 = } WBD_TO_QMD_MSCS_SUCCESS */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_mscs_success {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_mscs_success wbd_qmd_ipc_cmd_mscs_success_t;
#define WBD_QMD_IPC_MSCS_SUCCESS_FIXED_LEN		10u

/* Definition of IPC Command : { 1 = } WBD_TO_QMD_MSCS_UNSOLICITED_RESP */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_mscs_unsolicited_resp {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_mscs_unsolicited_resp wbd_qmd_ipc_cmd_mscs_unsolicited_resp_t;
#define WBD_QMD_IPC_MSCS_UNSOLICITED_RESP_FIXED_LEN	10u

/* Definition of IPC Command : { 2 = } WBD_TO_QMD_SCS_SUCCESS */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_scs_success {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_scs_success wbd_qmd_ipc_cmd_scs_success_t;
#define WBD_QMD_IPC_SCS_SUCCESS_FIXED_LEN		10u

/* Definition of IPC Command : { 3 = } WBD_TO_QMD_SCS_UNSOLICITED_RESP */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_scs_unsolicited_resp {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_scs_unsolicited_resp wbd_qmd_ipc_cmd_scs_unsolicited_resp_t;
#define WBD_QMD_IPC_SCS_UNSOLICITED_RESP_FIXED_LEN	10u

/* Definition of IPC Command : { 4 = } WBD_TO_QMD_DSCP_POLICY_UPDATE */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_dscp_policy_update {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_dscp_policy_update wbd_qmd_ipc_cmd_dscp_policy_update_t;
#define WBD_QMD_IPC_DSCP_POLICY_UPDATE_FIXED_LEN	10u

/* Definition of IPC Command : { 5 = } WBD_TO_QMD_POLICY_UPDATE */
BWL_PRE_PACKED_STRUCT struct wbd_qmd_ipc_cmd_policy_update {
	unsigned char updated;
} BWL_POST_PACKED_STRUCT;
typedef struct wbd_qmd_ipc_cmd_policy_update wbd_qmd_ipc_cmd_policy_update_t;
#define WBD_QMD_IPC_POLICY_UPDATE_FIXED_LEN		1u

/* ============================================================================ */

/* ============================================================================ */
/* ------------------  Direction : QMD to SmartMesh Agent  -------------------- */

/* Type of IPC Command ID */
typedef enum qmd_wbd_ipc_cmd_type {
	QMD_TO_WBD_MSCS_SUCCESS			= 0,
	QMD_TO_WBD_MSCS_UNSOLICITED_RESP	= 1,
	QMD_TO_WBD_SCS_SUCCESS			= 2,
	QMD_TO_WBD_SCS_UNSOLICITED_RESP		= 3
} qmd_wbd_ipc_cmd_type_t;

/* Definition of IPC Command : { 0 = } QMD_TO_WBD_MSCS_SUCCESS */
BWL_PRE_PACKED_STRUCT struct qmd_wbd_ipc_cmd_mscs_success {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct qmd_wbd_ipc_cmd_mscs_success qmd_wbd_ipc_cmd_mscs_success_t;
#define QMD_WBD_IPC_MSCS_SUCCESS_FIXED_LEN		10u

/* Definition of IPC Command : { 1 = } QMD_TO_WBD_MSCS_UNSOLICITED_RESP */
BWL_PRE_PACKED_STRUCT struct qmd_wbd_ipc_cmd_mscs_unsolicited_resp {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct qmd_wbd_ipc_cmd_mscs_unsolicited_resp qmd_wbd_ipc_cmd_mscs_unsolicited_resp_t;
#define QMD_WBD_IPC_MSCS_UNSOLICITED_RESP_FIXED_LEN	10u

/* Definition of IPC Command : { 2 = } QMD_TO_WBD_SCS_SUCCESS */
BWL_PRE_PACKED_STRUCT struct qmd_wbd_ipc_cmd_scs_success {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct qmd_wbd_ipc_cmd_scs_success qmd_wbd_ipc_cmd_scs_success_t;
#define QMD_WBD_IPC_SCS_SUCCESS_FIXED_LEN		10u

/* Definition of IPC Command : { 3 = } QMD_TO_WBD_SCS_UNSOLICITED_RESP */
BWL_PRE_PACKED_STRUCT struct qmd_wbd_ipc_cmd_scs_unsolicited_resp {
	unsigned char sta_mac[6];
	unsigned int descriptor_len;
	unsigned char descriptor[1];
} BWL_POST_PACKED_STRUCT;
typedef struct qmd_wbd_ipc_cmd_scs_unsolicited_resp qmd_wbd_ipc_cmd_scs_unsolicited_resp_t;
#define QMD_WBD_IPC_SCS_UNSOLICITED_RESP_FIXED_LEN	10u

/* ============================================================================ */

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

/* Agent to QMD : Gets the Name of IPC Command from the IPC Command ID */
extern const char* wbd_qmd_ipc_get_command_name(const wbd_qmd_ipc_cmd_type_t id);

/* QMD to Agent : Gets the Name of IPC Command from the IPC Command ID */
extern const char* qmd_wbd_ipc_get_command_name(const qmd_wbd_ipc_cmd_type_t id);

/* Common : Encode Generic IPC Command Header */
extern void wbd_qmd_ipc_encode_cmd_hdr(wbd_qmd_ipc_cmd_hdr_t *in_cmd_hdr,
	unsigned short in_version, unsigned char in_cmd_id, unsigned int in_len);

/* Common : Send IPC Command Data */
extern int wbd_qmd_ipc_send_data(int port_no, unsigned char *ipc_data, unsigned int ipc_data_len);

/* Common : Receive IPC Command Data */
extern int wbd_qmd_ipc_recv_data(const int connfd, wbd_qmd_ipc_cmd_hdr_t *cmd_hdr,
	char **out_ipc_data);

#endif /* _WBD_QMD_SHARED_H_ */
