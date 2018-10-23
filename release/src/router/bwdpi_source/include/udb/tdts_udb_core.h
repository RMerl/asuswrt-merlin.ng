/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef TDTS_UDB_CORE_H_
#define TDTS_UDB_CORE_H_

/* autoconf */
#include "udb/tmcfg_udb.h"

/*
 * avoid compile warnings
 */
#ifndef TMCFG_E_CORE_METADATA_EXTRACT
#define TMCFG_E_CORE_METADATA_EXTRACT 0
#endif

#ifndef TMCFG_E_UDB_CORE_VIRTUAL_PATCH
#define TMCFG_E_UDB_CORE_VIRTUAL_PATCH 0
#endif

#ifndef TMCFG_E_UDB_CORE_DC_FULL_URL
#define TMCFG_E_UDB_CORE_DC_FULL_URL 0
#endif

#ifndef TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID
#define TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID 0
#endif

#ifndef TMCFG_E_UDB_CORE_DC_FULL_URL
#define TMCFG_E_UDB_CORE_DC_FULL_URL 0
#endif

#ifndef TMCFG_E_UDB_CORE_DC_DNS_REPLY
#define TMCFG_E_UDB_CORE_DC_DNS_REPLY 0
#endif

#ifndef TMCFG_E_UDB_CORE_WBL
#define TMCFG_E_UDB_CORE_WBL 0
#endif

#ifndef TMCFG_E_UDB_CORE_APP_WBL
#define TMCFG_E_UDB_CORE_APP_WBL 0
#endif

#ifndef TMCFG_E_UDB_CORE_HWNAT
#define TMCFG_E_UDB_CORE_HWNAT 0
#endif

#ifndef TMCFG_E_UDB_CORE_SWNAT
#define TMCFG_E_UDB_CORE_SWNAT 0
#endif

#ifndef TMCFG_E_UDB_CORE_HWQOS
#define TMCFG_E_UDB_CORE_HWQOS 0
#endif

#ifndef TMCFG_E_UDB_CORE_PROG_LIC_CTRL
#define TMCFG_E_UDB_CORE_PROG_LIC_CTRL 0
#endif

#ifndef TMCFG_E_UDB_CORE_PROG_LIC_CTRL_STAGE_2
#define TMCFG_E_UDB_CORE_PROG_LIC_CTRL_STAGE_2 0
#endif

#if !TMCFG_E_CORE_APPID
#error "TMCFG_E_CORE_APPID is not enabled!"
#endif

#if !TMCFG_E_CORE_DEVID
#error "TMCFG_E_CORE_DEVID is not enabled!"
#endif

#if !TMCFG_E_CORE_DNS_DECODE
#warning "TMCFG_E_CORE_DNS_DECODE is not enabled!"
#endif

#if !TMCFG_E_CORE_APP_SET_BY_DNS
#warning "TMCFG_E_CORE_APP_SET_BY_DNS is not enabled!"
#endif

#if !TMCFG_E_CORE_SIP_DECODE
#warning "TMCFG_E_CORE_SIP_DECODE is not enabled!"
#endif

#if !TMCFG_E_CORE_STREAMING_DECODE
#warning "TMCFG_E_CORE_STREAMING_DECODE is not enabled!"
#endif

#if !TMCFG_E_CORE_TLS_DECODE
#warning "TMCFG_E_CORE_TLS_DECODE is not enabled!"
#endif

#if TMCFG_E_UDB_CORE_VIRTUAL_PATCH && !TMCFG_E_CORE_IPS
#error "TMCFG_E_UDB_CORE_VIRTUAL_PATCH depends on TMCFG_E_CORE_IPS, which is not enabled!"
#endif

#if TMCFG_E_UDB_CORE_ANOMALY_PREVENT && !TMCFG_E_CORE_PORT_SCAN_DETECTION
#error "TMCFG_E_UDB_CORE_ANOMALY_PREVENT depends on TMCFG_E_CORE_PORT_SCAN_DETECTION, which is not enabled!"
#endif

#if TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID && !TMCFG_E_CORE_METADATA_EXTRACT
#error "TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID depends on TMCFG_E_CORE_METADATA_EXTRACT, which is not enabled!"
#endif

#if TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID && !TMCFG_E_CORE_DEVID_COLLECT_UN
#error "TMCFG_E_UDB_CORE_DC_UNKNOWN_DEVID depends on TMCFG_E_CORE_DEVID_COLLECT_UN, which is not enabled!"
#endif

#if TMCFG_E_UDB_CORE_RULE_FORMAT_V2 && !TMCFG_E_CORE_RULE_FORMAT_V2
#error "TMCFG_E_UDB_CORE_RULE_FORMAT_V2 depends on TMCFG_E_CORE_RULE_FORMAT_V2, which is not enabled!"
#endif

#if TMCFG_E_CORE_RULE_FORMAT_V2 && !TMCFG_E_UDB_CORE_RULE_FORMAT_V2
#error "TMCFG_E_UDB_CORE_RULE_FORMAT_V2 has to be enabled to support TMCFG_E_CORE_RULE_FORMAT_V2"
#endif

#include "udb/core/syscall.h"

#include "tdts/tdts_core.h"

#include "udb/core/udb_common.h"

#if TMCFG_E_UDB_CORE_URL_QUERY
#include "udb/core/udb_wrs.h"
#endif

#if TMCFG_E_UDB_CORE_WBL
#include "udb/core/udb_wbl.h"
#endif

#if TMCFG_E_UDB_CORE_APP_WBL
#include "udb/core/udb_app_wbl.h"
#endif

#if TMCFG_E_UDB_CORE_IQOS_SUPPORT
#include "udb/core/udb_qos.h"
#if TMCFG_E_UDB_CORE_GCTRL_SUPPORT
#include "udb/core/udb_gctrl.h"
#endif
#endif

#if TMCFG_E_UDB_CORE_PATROL_TIME_QUOTA
#include "udb/core/udb_patrol_tq.h"
#endif

#if TMCFG_E_UDB_CORE_VIRTUAL_PATCH
#include "udb/core/udb_vp.h"
#endif

#if TMCFG_E_UDB_CORE_ANOMALY_PREVENT
#include "udb/core/udb_anomaly.h"
#endif

#if TMCFG_E_UDB_CORE_TMDBG
#include "udb/core/udb_dlog.h"
#endif

#if TMCFG_E_UDB_CORE_MESH
#include "udb/core/udb_mesh.h"
#endif

#if TMCFG_E_UDB_CORE_HWNAT
#include "hwnat_core.h"
#endif

#if TMCFG_E_UDB_CORE_SWNAT
#include "swnat_core.h"
#endif

#endif /* UDB_CORE_H_ */

