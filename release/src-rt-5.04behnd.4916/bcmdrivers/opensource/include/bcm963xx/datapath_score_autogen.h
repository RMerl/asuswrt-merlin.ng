/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/



/* This is an automated file. Do not edit its contents. */


#ifndef _DATAPATH_SCORE_AUTOGEN_H_
#define _DATAPATH_SCORE_AUTOGEN_H_

typedef struct score_report_header
{
	uint16_t	stream_key       	;
	uint16_t	length           	;
	uint32_t	timestamp        	;
	union {
		uint32_t	status_union     	;
		struct {
			uint32_t	data1_length               	:8;
			uint32_t	data2_length               	:8;
			uint32_t	data3_length               	:8;
			uint32_t	reserved                   	:5;
			uint32_t	reports_dropped            	:1;
			uint32_t	ip_checksum_error          	:1;
			uint32_t	parsing_error              	:1;
		};
		struct {
			uint32_t	status                     	:32;
		};
	};
} __attribute__((packed)) score_report_header_t;

typedef struct score_report_tcp_fields
{
	uint32_t	sequence_number	;
	uint16_t	window         	;
	union {
		uint8_t	flags          	;
		struct {
			uint8_t	fin                      	:1;
			uint8_t	sys                      	:1;
			uint8_t	rst                      	:1;
			uint8_t	psh                      	:1;
			uint8_t	ack                      	:1;
			uint8_t	urg                      	:1;
			uint8_t	ece                      	:1;
			uint8_t	cwr                      	:1;
		};
	};
	uint8_t	reserved       	;
} __attribute__((packed)) score_report_tcp_fields_t;

#define SCORE_REPORT_BODY_REGION1_NUMBER	64
#define SCORE_REPORT_BODY_REGION2_NUMBER	64
#define SCORE_REPORT_BODY_REGION3_NUMBER	64
typedef struct score_report_body
{
	uint8_t	region1[SCORE_REPORT_BODY_REGION1_NUMBER];
	uint8_t	region2[SCORE_REPORT_BODY_REGION2_NUMBER];
	uint8_t	region3[SCORE_REPORT_BODY_REGION3_NUMBER];
	uint32_t	score_report_tcp_fields_sequence_number          	;
	uint16_t	score_report_tcp_fields_window                   	;
	union {
		uint8_t	score_report_tcp_fields_flags                    	;
		struct {
			uint8_t	score_report_tcp_fields_fin                                	:1;
			uint8_t	score_report_tcp_fields_sys                                	:1;
			uint8_t	score_report_tcp_fields_rst                                	:1;
			uint8_t	score_report_tcp_fields_psh                                	:1;
			uint8_t	score_report_tcp_fields_ack                                	:1;
			uint8_t	score_report_tcp_fields_urg                                	:1;
			uint8_t	score_report_tcp_fields_ece                                	:1;
			uint8_t	score_report_tcp_fields_cwr                                	:1;
		};
	};
	uint8_t	score_report_tcp_fields_reserved                 	;
	uint16_t	reserved1                                        	;
	uint8_t	tos                                              	;
	uint8_t	ttl                                              	;
} __attribute__((packed)) score_report_body_t;


#define SCORE_PROFILE_DATA_REGION_OFFSET_FIRST	0
#define SCORE_PROFILE_DATA_REGION_OFFSET_L2   	0
#define SCORE_PROFILE_DATA_REGION_OFFSET_L3   	1
#define SCORE_PROFILE_DATA_REGION_OFFSET_L4   	2
#define SCORE_PROFILE_DATA_REGION_OFFSET_LAST	2


#define SCORE_PROFILE_LENGTH_TYPE_FIRST	0
#define SCORE_PROFILE_LENGTH_TYPE_L2   	0
#define SCORE_PROFILE_LENGTH_TYPE_L3   	1
#define SCORE_PROFILE_LENGTH_TYPE_L4   	2
#define SCORE_PROFILE_LENGTH_TYPE_L5   	3
#define SCORE_PROFILE_LENGTH_TYPE_LAST	3


#define SCORE_PROFILE_TYPE_FIRST	0
#define SCORE_PROFILE_TYPE_SCORE   	0
#define SCORE_PROFILE_TYPE_QSTAT   	1
#define SCORE_PROFILE_TYPE_BNL     	2
#define SCORE_PROFILE_TYPE_LAST	2


#define SCORE_DEFS_FIRST	4
#define SCORE_DEFS_PROFILE_MAX_REGIONS   	4
#define SCORE_DEFS_MAX_PROFILES          	8
#define SCORE_DEFS_SCORE_BIT_MASK        	255
#define SCORE_DEFS_QSTAT_BIT_MASK        	256
#define SCORE_DEFS_BNL_BIT_MASK          	512
#define SCORE_DEFS_LAST	512

#endif /* _DATAPATH_SCORE_AUTOGEN_H_ */
