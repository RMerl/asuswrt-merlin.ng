// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "client/display.h"

#include "mesh/crypto.c"

struct mesh_crypto_test {
	const char *name;

	const char *dev_key;
	const char *app_key;
	const char *net_key;
	const char *uuid;
	uint32_t iv_index;

	uint8_t net_nid;
	uint8_t key_aid;
	bool md[32];
	bool szmic;
	bool frnd;
	bool ctl;
	bool segmented;
	bool relay;
	bool akf;
	bool kr;
	bool ivu;
	bool network_only;
	uint8_t seg_max;
	uint8_t seg_num;
	uint8_t opcode;
	uint8_t net_ttl;
	uint16_t seqZero;
	uint32_t app_seq;
	uint32_t net_seq[32];
	uint16_t net_src;
	uint16_t net_dst;
	const char *app_msg;

	const char *ikm;
	const char *okm;
	const char *info;
	const char *salt;
	const char *salt_out;
	const char *nid;
	const char *lpn_addr;
	const char *lpn_cntr;
	const char *fn_addr;
	const char *fn_cntr;
	const char *p;
	const char *short_net_id;
	const char *net_key_id;
	const char *aid;

	const char *enc_key;
	const char *net_id;
	const char *net_nonce[32];
	const char *app_nonce;
	const char *priv_key;
	const char *priv_rand[32];

	const char *enc_msg;
	uint32_t app_mic32;
	uint64_t app_mic64;

	const char *trans_pkt[32];
	const char *net_msg[32];
	uint32_t net_mic32[32];
	uint64_t net_mic64;

	const char *packet[32];

	uint8_t beacon_type;
	uint8_t beacon_flags;
	const char *beacon_cmac;
	const char *beacon;

	const char *rand;
	const char *ident_res_key;
	const char *hash_input;
	const char *mesh_id_hash;
	const char *identity_hash;
};

static const struct mesh_crypto_test s8_1_1 = {
	.name		= "8.1.1 s1 SALT generation function",
	.salt		= "test",
	.salt_out	= "b73cefbd641ef2ea598c2b6efb62f79c",
};

static const struct mesh_crypto_test s8_1_2 = {
	.name		= "8.1.2 k1 function",
	.ikm		= "3216d1509884b533248541792b877f98",
	.salt		= "salt",
	.info		= "info",
	.okm		= "f6ed15a8934afbe7d83e8dcb57fcf5d7",
};

static const struct mesh_crypto_test s8_1_3 = {
	.name		= "8.1.3 k2 function (master)",
	.net_key	= "f7a2a44f8e8a8029064f173ddc1e2b00",
	.p		= "00",
	.nid		= "7f",
	.enc_key	= "9f589181a0f50de73c8070c7a6d27f46",
	.priv_key	= "4c715bd4a64b938f99b453351653124f",
};

static const struct mesh_crypto_test s8_1_4 = {
	.name		= "8.1.4 k2 function (friendship)",
	.frnd		= true,
	.lpn_addr	= "0203",
	.fn_addr	= "0405",
	.lpn_cntr	= "0607",
	.fn_cntr	= "0809",
	.net_key	= "f7a2a44f8e8a8029064f173ddc1e2b00",
	.p		= "010203040506070809",
	.nid		= "73",
	.enc_key	= "11efec0642774992510fb5929646df49",
	.priv_key	= "d4d7cc0dfa772d836a8df9df5510d7a7",
};

static const struct mesh_crypto_test s8_1_5 = {
	.name		= "8.1.5 k3 function",
	.net_key	= "f7a2a44f8e8a8029064f173ddc1e2b00",
	.salt		= "smk3",
	.info		= "id64\01",
	.short_net_id	= "ff046958233db014",
};

static const struct mesh_crypto_test s8_1_6 = {
	.name		= "8.1.6 k4 function",
	.app_key	= "3216d1509884b533248541792b877f98",
	.salt		= "smk4",
	.info		= "id6\01",
	.aid		= "38",
};


static const struct mesh_crypto_test s8_2_1 = {
	.name		= "8.2.1 Application key AID",
	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.salt		= "smk4",
	.info		= "id6\01",
	.aid		= "26",
};

static const struct mesh_crypto_test s8_2_2 = {
	.name		= "8.2.2 Encryption and privacy keys (Master)",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.p		= "00",
	.nid		= "68",
	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",
};

static const struct mesh_crypto_test s8_2_3 = {
	.name		= "8.2.3 Encryption and privacy keys (Friendship)",
	.frnd		= true,
	.lpn_addr	= "1201",
	.fn_addr	= "2345",
	.lpn_cntr	= "0000",
	.fn_cntr	= "072f",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.p		= "01120123450000072f",
	.nid		= "5e",
	.enc_key	= "be635105434859f484fc798e043ce40e",
	.priv_key	= "5d396d4b54d3cbafe943e051fe9a4eb8",
};

static const struct mesh_crypto_test s8_2_4 = {
	.name		= "8.2.4 Network ID",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.salt		= "smk3",
	.info		= "id64\01",
	.short_net_id	= "3ecaff672f673370",
};

static const struct mesh_crypto_test s8_2_5 = {
	.name		= "8.2.5 Identity Key",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.salt		= "nkik",
	.info		= "id128\01",
	.enc_key	= "84396c435ac48560b5965385253e210c",
};

static const struct mesh_crypto_test s8_2_6 = {
	.name		= "8.2.6 Beacon Key",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.salt		= "nkbk",
	.info		= "id128\01",
	.enc_key	= "5423d967da639a99cb02231a83f7d254",
};

static const struct mesh_crypto_test s8_3_1 = {
	.name		= "8.3.1 Message #1",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_ttl	= 0x00,
	.net_seq	= {0x000001},
	.net_src	= 0x1201,
	.net_dst	= 0xfffd,
	.opcode		= NET_OP_FRND_REQUEST,
	.trans_pkt	= {"034b50057e400000010000"},
	.net_nid	= 0x68,


	.ctl		= true,
	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.net_nonce	= {"00800000011201000012345678"},
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",
	.priv_rand	= {"000000000012345678b5e5bfdacbaf6c"},

	.net_msg	= {"b5e5bfdacbaf6cb7fb6bff871f"},
	.net_mic64	= 0x035444ce83a670df,

	.packet		= {"68eca487516765b5e5bfdacbaf6cb7fb6bff871f035444ce83a670df"},
};

static const struct mesh_crypto_test s8_3_2 = {
	.name		= "8.3.2 Message #2",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.ctl		= true,
	.net_ttl	= 0x00,
	.net_seq	= {0x014820},
	.net_src	= 0x2345,
	.net_dst	= 0x1201,
	.trans_pkt	= {"04320308ba072f"},
	.net_nid	= 0x68,


	.opcode		= NET_OP_FRND_OFFER,
	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.net_nonce	= {"00800148202345000012345678"},
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",

	.net_msg	= {"79d7dbc0c9b4d43eeb"},
	.net_mic64	= 0xec129d20a620d01e,

	.priv_rand	= {"00000000001234567879d7dbc0c9b4d4"},
	.packet		= {"68d4c826296d7979d7dbc0c9b4d43eebec129d20a620d01e"},
};

static const struct mesh_crypto_test s8_3_3 = {
	.name		= "8.3.3 Message #3",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.ctl		= true,
	.net_ttl	= 0x00,
	.net_seq	= {0x2b3832},
	.net_dst	= 0x1201,
	.net_src	= 0x2fe3,
	.trans_pkt	= {"04fa0205a6000a"},
	.net_nid	= 0x68,


	.opcode		= NET_OP_FRND_OFFER,
	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.net_nonce	= {"00802b38322fe3000012345678"},
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",
	.priv_rand	= {"00000000001234567853273086b8c5ee"},

	.net_msg	= {"53273086b8c5ee00bd"},
	.net_mic64	= 0xd9cfcc62a2ddf572,

	.packet		= {"68da062bc96df253273086b8c5ee00bdd9cfcc62a2ddf572"},
};

static const struct mesh_crypto_test s8_3_4 = {
	.name		= "8.3.4 Message #4",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x5e,
	.net_ttl	= 0x00,
	.net_seq	= {0x000002},
	.net_dst	= 0x2345,
	.net_src	= 0x1201,
	.trans_pkt	= {"0100"},

	.frnd		= true,
	.lpn_addr	= "1201",
	.fn_addr	= "2345",
	.lpn_cntr	= "0000",
	.fn_cntr	= "072f",
	.p		= "01120123450000072f",

	.ctl		= true,
	.opcode		= NET_OP_FRND_POLL,
	.enc_key	= "be635105434859f484fc798e043ce40e",
	.net_nonce	= {"00800000021201000012345678"},
	.priv_key	= "5d396d4b54d3cbafe943e051fe9a4eb8",
	.priv_rand	= {"000000000012345678b0e5d0ad970d57"},

	.net_msg	= {"b0e5d0ad"},
	.net_mic64	= 0x970d579a4e88051c,

	.packet		= {"5e84eba092380fb0e5d0ad970d579a4e88051c"},
};

static const struct mesh_crypto_test s8_3_5 = {
	.name		= "8.3.5 Message #5",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x5e,
	.net_ttl	= 0x00,
	.net_seq	= {0x014834},
	.net_src	= 0x2345,
	.net_dst	= 0x1201,
	.trans_pkt	= {"02001234567800"},

	.frnd		= true,
	.lpn_addr	= "1201",
	.fn_addr	= "2345",
	.lpn_cntr	= "0000",
	.fn_cntr	= "072f",
	.p		= "01120123450000072f",

	.ctl		= true,
	.opcode		= NET_OP_FRND_UPDATE,
	.enc_key	= "be635105434859f484fc798e043ce40e",
	.net_nonce	= {"00800148342345000012345678"},
	.priv_key	= "5d396d4b54d3cbafe943e051fe9a4eb8",
	.priv_rand	= {"0000000000123456785c39da1792b1fe"},

	.net_msg	= {"5c39da1792b1fee9ec"},
	.net_mic64	= 0x74b786c56d3a9dee,

	.packet		= {"5eafd6f53c43db5c39da1792b1fee9ec74b786c56d3a9dee"},
};

static const struct mesh_crypto_test s8_3_6 = {
	.name		= "8.3.6 Message #6",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.segmented	= true,
	.net_nid	= 0x68,
	.net_ttl	= 0x04,
	.app_seq	= 0x3129ab,
	.net_seq	= {0x3129ab,
			   0x3129ac},
	.net_src	= 0x0003,
	.net_dst	= 0x1201,
	.app_msg	= "0056341263964771734fbd76e3b40519d1d94a48",
	.enc_msg	= "ee9dddfd2169326d23f3afdfcfdc18c52fdef772",
	.app_mic32	= 0xe0e17308,

	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.app_nonce	= "02003129ab0003120112345678",

	.priv_key	= "8b84eedec100067d670971dd2aa700cf",

	.net_nonce	= {"00043129ab0003000012345678",
	                   "00043129ac0003000012345678"},

	.priv_rand	= {"0000000000123456780afba8c63d4e68",
	                   "0000000000123456786cae0c032bf074"},

	.trans_pkt	= {"8026ac01ee9dddfd2169326d23f3afdf",
			   "8026ac21cfdc18c52fdef772e0e17308"},

	.net_msg	= {"0afba8c63d4e686364979deaf4fd40961145",
	                   "6cae0c032bf0746f44f1b8cc8ce5edc57e55"},

	.net_mic32	= {0x939cda0e,
	                   0xbeed49c0},

	.packet		= {"68cab5c5348a230afba8c63d4e686364979deaf4fd40961145939cda0e",
	                   "681615b5dd4a846cae0c032bf0746f44f1b8cc8ce5edc57e55beed49c0"},
};

static const struct mesh_crypto_test s8_3_7 = {
	.name		= "8.3.7 Message #7",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x68,
	.relay		= true,
	.ctl		= true,
	.net_ttl	= 0x0b,
	.net_seq	= {0x014835},
	.net_src	= 0x2345,
	.net_dst	= 0x0003,
	.opcode		= NET_OP_SEG_ACKNOWLEDGE,
	.seqZero	= 0x09ab,
	.trans_pkt	= {"00a6ac00000002"},


	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.net_nonce	= {"008b0148352345000012345678"},
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",
	.priv_rand	= {"0000000000123456780d0d730f94d7f3"},

	.net_msg	= {"0d0d730f94d7f3509d"},
	.net_mic64	= 0xf987bb417eb7c05f,

	.packet		= {"68e476b5579c980d0d730f94d7f3509df987bb417eb7c05f"},
};

static const struct mesh_crypto_test s8_3_8 = {
	.name		= "8.3.8 Message #8",
	.network_only	= true,  /* Test has incomplete Access Payload */
	.seg_max	= 1,
	.seg_num	= 0,

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x68,
	.relay		= true,
	.net_ttl	= 0x04,
	.net_seq	= {0x3129ad},
	.net_src	= 0x0003,
	.net_dst	= 0x1201,
	.segmented	= true,
	.seqZero	= 0x09ab,
	.trans_pkt	= {"8026ac01ee9dddfd2169326d23f3afdf"},


	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.net_nonce	= {"00043129ad0003000012345678"},
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",
	.priv_rand	= {"0000000000123456780e2f91add6f06e"},

	.net_msg	= {"0e2f91add6f06e66006844cec97f973105ae"},
	.net_mic32	= {0x2534f958},

	.packet		= {"684daa6267c2cf0e2f91add6f06e66006844cec97f973105ae2534f958"},
};


static const struct mesh_crypto_test s8_3_9 = {
	.name		= "8.3.9 Message #9",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x68,
	.relay		= true,
	.ctl		= true,
	.net_ttl	= 0x0b,
	.net_seq	= {0x014836},
	.net_src	= 0x2345,
	.net_dst	= 0x0003,
	.opcode		= NET_OP_SEG_ACKNOWLEDGE,
	.seqZero	= 0x09ab,
	.trans_pkt	= {"00a6ac00000003"},


	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.net_nonce	= {"008b0148362345000012345678"},
	.priv_key	= "8b84eedec100067d670971dd2aa700cf",
	.priv_rand	= {"000000000012345678d85d806bbed248"},

	.net_msg	= {"d85d806bbed248614f"},
	.net_mic64	= 0x938067b0d983bb7b,

	.packet		= {"68aec467ed4901d85d806bbed248614f938067b0d983bb7b"},
};

static const struct mesh_crypto_test s8_3_10 = {
	.name		= "8.3.10 Message #10",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x5e,
	.net_ttl	= 0x00,
	.net_seq	= {0x000003},
	.net_src	= 0x1201,
	.net_dst	= 0x2345,
	.trans_pkt	= {"0101"},

	.frnd		= true,
	.lpn_addr	= "1201",
	.fn_addr	= "2345",
	.lpn_cntr	= "0000",
	.fn_cntr	= "072f",
	.p		= "01120123450000072f",

	.ctl		= true,
	.opcode		= NET_OP_FRND_POLL,
	.enc_key	= "be635105434859f484fc798e043ce40e",
	.net_nonce	= {"00800000031201000012345678"},
	.priv_key	= "5d396d4b54d3cbafe943e051fe9a4eb8",
	.priv_rand	= {"0000000000123456787777ed355afaf6"},

	.net_msg	= {"7777ed35"},
	.net_mic64	= 0x5afaf66d899c1e3d,

	.packet		= {"5e7b786568759f7777ed355afaf66d899c1e3d"},
};

static const struct mesh_crypto_test s8_3_11 = {
	.name		= "8.3.11 Message #11",

	/* Test has incomplete Access Payload */
	.network_only	= true,
	.seg_max	= 1,
	.seg_num	= 0,

	.frnd		= true,
	.lpn_addr	= "1201",
	.fn_addr	= "2345",
	.lpn_cntr	= "0000",
	.fn_cntr	= "072f",
	.p		= "01120123450000072f",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345678,

	.net_nid	= 0x5e,
	.net_ttl	= 0x03,
	.net_seq	= {0x3129ad},
	.net_src	= 0x0003,
	.net_dst	= 0x1201,
	.akf		= true,
	.key_aid	= 0x00,
	.segmented	= true,
	.seqZero	= 0x09ab,
	.trans_pkt	= {"c026ac01ee9dddfd2169326d23f3afdf"},

	.enc_key	= "be635105434859f484fc798e043ce40e",
	.net_nonce	= {"00033129ad0003000012345678"},
	.priv_key	= "5d396d4b54d3cbafe943e051fe9a4eb8",
	.priv_rand	= {"000000000012345678d5e748a20ecfd9"},

	.net_msg	= {"d5e748a20ecfd98ddfd32de80befb400213d"},
	.net_mic32	= {0x113813b5},

	.packet		= {"5e6ebfc021edf5d5e748a20ecfd98ddfd32de80befb400213d113813b5"},
};

static const struct mesh_crypto_test s8_3_22 = {
	.name		= "8.3.22 Message #22",

	.app_key	= "63964771734fbd76e3b40519d1d94a48",
	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.dev_key	= "9d6dd0e96eb25dc19a40ed9914f8f03f",
	.iv_index	= 0x12345677,

	.net_nid	= 0x68,
	.net_ttl	= 0x03,
	.app_seq	= 0x07080b,
	.net_seq	= {0x07080b},
	.net_src	= 0x1234,
	.net_dst	= 0xb529,
	.uuid		= "0073e7e4d8b9440faf8415df4c56c0e1",
	.akf		= true,
	.key_aid	= 0x26,
	.app_msg	= "d50a0048656c6c6f",
	.enc_msg	= "3871b904d4315263",
	.app_mic32	= 0x16ca48a0,

	.enc_key	= "0953fa93e7caac9638f58820220a398e",
	.app_nonce	= "010007080b1234b52912345677",

	.priv_key	= "8b84eedec100067d670971dd2aa700cf",

	.net_nonce	= {"000307080b1234000012345677"},

	.priv_rand	= {"000000000012345677ed31f3fdcf88a4"},

	.trans_pkt	= {"663871b904d431526316ca48a0"},

	.net_msg	= {"ed31f3fdcf88a411135fea55df730b"},

	.net_mic32	= {0x6b28e255},

	.packet		= {"e8d85caecef1e3ed31f3fdcf88a411135fea55df730b6b28e255" },
};

static const struct mesh_crypto_test s8_4_3 = {
	.name		= "8.4.3 Secure Network Beacon",

	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.iv_index	= 0x12345678,

	.enc_key	= "5423d967da639a99cb02231a83f7d254",
	.net_id		= "3ecaff672f673370",

	.beacon_type	= 0x01,
	.beacon_flags	= 0x00,
	.beacon_cmac	= "8ea261582f364f6f",
	.beacon		= "01003ecaff672f673370123456788ea261582f364f6f",
};

static const struct mesh_crypto_test s8_6_2 = {
	.name		= "8.6.2 Service Data using Node Identity",

	.net_key	= "7dd7364cd842ad18c17c2b820c84c3d6",
	.net_src	= 0x1201,
	.rand		= "34ae608fbbc1f2c6",
	.ident_res_key	= "84396c435ac48560b5965385253e210c",

	.hash_input	= "00000000000034ae608fbbc1f2c61201",
	.identity_hash	= "00861765aefcc57b",
	.beacon 	= "0100861765aefcc57b34ae608fbbc1f2c6",
};

#define PASS	COLOR_GREEN "PASS" COLOR_OFF
#define FAIL	COLOR_RED "FAIL" COLOR_OFF
#define INVAL	COLOR_YELLOW "INVALID" COLOR_OFF

#define EVALCMP(a, b, l)	memcmp((a), (b), (l)) ? FAIL : PASS
#define EXITCMP(a, b, l)	do { if (memcmp((a), (b), (l))) \
					exit(1); \
				} while (0)

#define EVALSTR(a, b)	(((a) && (b)) ? (strcmp((a), (b)) ? FAIL : PASS) \
								: INVAL)
#define EXITSTR(a, b)	do { if ((a) && (b)) { \
					if (strcmp((a), (b))) \
						exit(1); \
				} else \
					exit(1); \
			} while (0)

#define EVALNUM(a, b)	(((a) == (b)) ? PASS : FAIL)
#define EXITNUM(a, b)	do { if (a != b) exit(1); } while (0)

#define EVALBOOLNOTBOTH(a, b)	!(a && b) ? PASS : FAIL
#define EXITBOOLNOTBOTH(a, b)	do { if (!!(a && b)) exit(1); } while (0)

static void verify_data(const char *label, unsigned int indent,
			const char *sample, const uint8_t *data, size_t size)
{
	char *str;

	str = l_util_hexstring(data, size);
	l_info("%-20s =%*c%s", label, 1 + (indent * 2), ' ', sample);
	l_info("%-20s  %*c%s => %s", "", 1 + (indent * 2), ' ', str,
							EVALSTR(sample, str));
	EXITSTR(sample, str);
	l_free(str);
}

static void verify_bool_not_both(const char *label, unsigned int indent,
						bool sample, bool data)
{
	l_info("%-20s =%*c%s", label, 1 + (indent * 2), ' ',
						sample ? "true" : "false");
	l_info("%-20s  %*c%s => %s", "", 1 + (indent * 2), ' ',
						data ? "true" : "false",
						EVALBOOLNOTBOTH(sample, data));
	EXITBOOLNOTBOTH(sample, data);
}

static void verify_uint8(const char *label, unsigned int indent,
						uint8_t sample, uint8_t data)
{
	l_info("%-20s =%*c%02x", label, 1 + (indent * 2), ' ', sample);
	l_info("%-20s  %*c%02x => %s", "", 1 + (indent * 2), ' ', data,
							EVALNUM(sample, data));
	EXITNUM(sample, data);
}

static void verify_uint16(const char *label, unsigned int indent,
						uint16_t sample, uint16_t data)
{
	l_info("%-20s =%*c%04x", label, 1 + (indent * 2), ' ', sample);
	l_info("%-20s  %*c%04x => %s", "", 1 + (indent * 2), ' ', data,
							EVALNUM(sample, data));
	EXITNUM(sample, data);
}

static void verify_uint24(const char *label, unsigned int indent,
						uint32_t sample, uint32_t data)
{
	l_info("%-20s =%*c%06x", label, 1 + (indent * 2), ' ', sample);
	l_info("%-20s  %*c%06x => %s", "", 1 + (indent * 2), ' ', data,
							EVALNUM(sample, data));
	EXITNUM(sample, data);
}

static void verify_uint32(const char *label, unsigned int indent,
						uint32_t sample, uint32_t data)
{
	l_info("%-20s =%*c%08x", label, 1 + (indent * 2), ' ', sample);
	l_info("%-20s  %*c%08x => %s", "", 1 + (indent * 2), ' ', data,
							EVALNUM(sample, data));
	EXITNUM(sample, data);
}

static void verify_uint64(const char *label, unsigned int indent,
						uint64_t sample, uint64_t data)
{
	l_info("%-20s =%*c%16llx", label, 1 + (indent * 2), ' ',
					(long long unsigned int) sample);
	l_info("%-20s  %*c%16llx => %s", "", 1 + (indent * 2), ' ',
						(long long unsigned int) data,
						EVALNUM(sample, data));
	EXITNUM(sample, data);
}

static void show_str(const char *label, unsigned int indent,
						const char *sample)
{
	char *printable = l_malloc(strlen(sample) + 1);
	char *tmp = printable;

	while (*sample) {
		if (l_ascii_isprint(*sample))
			*tmp++ = *sample++;
		else {
			*tmp++ = '?';
			sample++;
		}
	}

	*tmp = '\0';

	l_info("%-20s =%*c%s", label, 1 + (indent * 2), ' ', printable);
	l_free(printable);
}

static void show_data(const char *label, unsigned int indent,
						const void *data, size_t size)
{
	char *str;

	str = l_util_hexstring(data, size);
	l_info("%-20s =%*c%s", label, 1 + (indent * 2), ' ', str);
	l_free(str);
}

static void show_uint8(const char *label, unsigned int indent, uint8_t data)
{
	l_info("%-20s =%*c%2.2x", label, 1 + (indent * 2), ' ', data);
}

static void show_uint32(const char *label, unsigned int indent, uint32_t data)
{
	l_info("%-20s =%*c%8.8x", label, 1 + (indent * 2), ' ', data);
}


static void check_encrypt_segment(const struct mesh_crypto_test *keys,
				uint16_t seg, uint16_t seg_max,
				uint8_t *enc_msg, size_t len,
				uint8_t *enc_key, uint8_t *priv_key,
				uint8_t nid)
{
	uint8_t net_nonce[13];
	uint8_t priv_rand[16];
	uint8_t packet[29];
	uint8_t packet_len;
	uint32_t hdr;
	uint64_t net_mic64, net_mic32;
	size_t net_msg_len;
	uint8_t key_aid = keys->key_aid | (keys->akf ? KEY_ID_AKF : 0x00);

	if (keys->ctl) {
		mesh_crypto_packet_build(keys->ctl, keys->net_ttl,
				keys->net_seq[0],
				keys->net_src, keys->net_dst,
				keys->opcode,
				keys->segmented, key_aid,
				keys->szmic, keys->relay, keys->seqZero,
				seg, seg_max,
				enc_msg, len,
				packet, &packet_len);
	} else {
		mesh_crypto_packet_build(keys->ctl, keys->net_ttl,
				keys->net_seq[0],
				keys->net_src, keys->net_dst,
				keys->opcode,
				keys->segmented, key_aid,
				keys->szmic, keys->relay, keys->seqZero,
				seg, seg_max,
				enc_msg, len,
				packet, &packet_len);
	}

	l_info(COLOR_YELLOW "Segment-%d" COLOR_OFF, seg);

	hdr = l_get_be32(packet + 9);
	verify_uint8("SEG", 9, keys->segmented << (SEG_HDR_SHIFT % 8),
			packet[9] & (1 << (SEG_HDR_SHIFT % 8)));

	if (keys->ctl) {
		verify_uint8("Opcode", 9,
				keys->opcode << (OPCODE_HDR_SHIFT % 8),
				(packet[9] & OPCODE_MASK) <<
				(OPCODE_HDR_SHIFT % 8));
	} else {
		verify_uint8("AKF", 9, keys->akf << (AKF_HDR_SHIFT % 8),
				packet[9] & (1 << (AKF_HDR_SHIFT % 8)));
		verify_uint8("AID", 9,
				keys->key_aid << (KEY_HDR_SHIFT % 8),
				(packet[9] & KEY_AID_MASK) <<
						(KEY_HDR_SHIFT % 8));
	}

	verify_uint8("SZMIC", 10,
			keys->szmic << (SZMIC_HDR_SHIFT % 8),
			packet[10] & (1 << (SZMIC_HDR_SHIFT % 8)));

	/* Awkward shift-by-two for correct display */
	verify_uint16("SeqZero", 10,
			(keys->seqZero & SEQ_ZERO_MASK) << 2,
			((hdr >> SEQ_ZERO_HDR_SHIFT) & SEQ_ZERO_MASK)
			<< 2);
	verify_uint16("SegO", 11,
			(seg & SEG_MASK) << SEGO_HDR_SHIFT,
			hdr & (SEG_MASK << SEGO_HDR_SHIFT));
	verify_uint8("SegN", 12,
			(seg_max & SEG_MASK) << SEGN_HDR_SHIFT,
			hdr & (SEG_MASK << SEGN_HDR_SHIFT));
	show_data("Payload", 13, enc_msg, len);
		len += 4;


	mesh_crypto_network_nonce(keys->ctl, keys->net_ttl,
					keys->net_seq[0], keys->net_src,
					keys->iv_index, net_nonce);

	verify_data("TransportData", 9, keys->trans_pkt[0],
			packet + 9, len);

	verify_uint16("DST", 7, keys->net_dst, l_get_be16(packet + 7));
	net_msg_len = len + 2;
	show_data("TransportPayload", 7, packet + 7, net_msg_len);

	mesh_crypto_packet_encrypt(packet, packet_len,
						enc_key,
						keys->iv_index, false,
						keys->ctl, keys->net_ttl,
						keys->net_seq[0],
						keys->net_src);

	mesh_crypto_privacy_counter(keys->iv_index, packet + 7, priv_rand);

	l_info("");
	show_uint32("IVindex", 0, keys->iv_index);
	verify_data("NetworkNonce", 0, keys->net_nonce[0], net_nonce, 13);
	verify_data("PrivacyRandom", 0, keys->priv_rand[0], priv_rand, 16);

	show_uint8("INI", 0, (keys->iv_index & 0x01) << 7);
	verify_uint8("NID", 0, keys->net_nid, nid);
	verify_uint8("CTL", 1, keys->ctl << 7, packet[1] & 0x80);
	verify_uint8("TTL", 1, keys->net_ttl, packet[1] & 0x7f);
	verify_uint24("SEQ", 2, keys->net_seq[0],
			l_get_be32(packet + 1) & SEQ_MASK);
	verify_uint16("SRC", 5, keys->net_src, l_get_be16(packet + 5));

	verify_data("EncNetworkPayload", 7, keys->net_msg[0],
			packet + 7, net_msg_len);
	if (keys->ctl) {
		net_mic64 = l_get_be64(packet + 7 + net_msg_len);
		verify_uint64("NetworkMIC", 7 + net_msg_len,
						keys->net_mic64, net_mic64);
		net_msg_len += 8;
	} else {
		net_mic32 = l_get_be32(packet + 7 + net_msg_len);
		verify_uint32("NetworkMIC", 7 + net_msg_len,
						keys->net_mic32[0], net_mic32);
		net_msg_len += 4;
	}

	show_data("PreObsPayload", 1, packet + 1, 6 + net_msg_len);
	mesh_crypto_network_obfuscate(packet, priv_key,
					keys->iv_index,
					keys->ctl, keys->net_ttl,
					keys->net_seq[0], keys->net_src);
	show_data("PostObsPayload", 1, packet + 1, 6 + net_msg_len);

	packet[0] = (keys->iv_index & 0x01) << 7 | nid;
	packet_len = 7 + net_msg_len;

	verify_data("Packet", 0, keys->packet[0], packet, packet_len);
	l_info("");

}

static void check_encrypt(const struct mesh_crypto_test *keys)
{
	uint8_t *dev_key;
	uint8_t *app_key;
	uint8_t *net_key;
	uint8_t nid;
	uint8_t enc_key[16];
	uint8_t priv_key[16];
	uint8_t net_nonce[13];
	uint8_t app_nonce[13];
	uint8_t priv_rand[16];
	uint8_t p[9];
	size_t p_len;
	size_t aad_len = 0;
	uint8_t *aad;
	size_t app_msg_len;
	uint8_t *app_msg = NULL;
	uint8_t *enc_msg;
	uint32_t app_mic32;
	uint64_t app_mic64;
	size_t net_msg_len;
	uint32_t net_mic32;
	uint64_t net_mic64;
	uint8_t key_aid;
	uint8_t packet[29];
	uint8_t packet_len;
	uint16_t i, seg_max, seg_len = 0;
	uint32_t seqZero, hdr;

	l_info(COLOR_BLUE "[Encrypt %s]" COLOR_OFF, keys->name);
	verify_bool_not_both("CTL && Segmented", 0, keys->ctl, keys->segmented);

	dev_key = l_util_from_hexstring(keys->dev_key, NULL);
	app_key = l_util_from_hexstring(keys->app_key, NULL);
	net_key = l_util_from_hexstring(keys->net_key, NULL);
	aad = l_util_from_hexstring(keys->uuid, &aad_len);

	show_data("NetworkKey", 0, net_key, 16);

	if (keys->akf) {
		mesh_crypto_k4(app_key, &key_aid);
		key_aid |= KEY_ID_AKF;
	} else {
		key_aid = 0;
	}

	if (keys->frnd) {
		uint8_t *lpn_addr, *fn_addr, *lpn_cntr, *fn_cntr;

		lpn_addr = l_util_from_hexstring(keys->lpn_addr, NULL);
		fn_addr = l_util_from_hexstring(keys->fn_addr, NULL);
		lpn_cntr = l_util_from_hexstring(keys->lpn_cntr, NULL);
		fn_cntr = l_util_from_hexstring(keys->fn_cntr, NULL);

		show_data("LPN Address", 0, lpn_addr, 2);
		show_data("Friend Address", 0, fn_addr, 2);
		show_data("LPN Counter", 0, lpn_cntr, 2);
		show_data("Friend Counter", 0, fn_cntr, 2);
		l_info("");

		p[0] = 1;
		l_put_be16(l_get_be16(lpn_addr), p + 1);
		l_put_be16(l_get_be16(fn_addr), p + 3);
		l_put_be16(l_get_be16(lpn_cntr), p + 5);
		l_put_be16(l_get_be16(fn_cntr), p + 7);
		p_len = 9;

		l_free(fn_cntr);
		l_free(lpn_cntr);
		l_free(fn_addr);
		l_free(lpn_addr);
	} else {
		p[0] = 0;
		p_len = 1;
	}

	if (p_len > 1) verify_data("P", 0, keys->p, p, p_len);

	mesh_crypto_k2(net_key, p, p_len, &nid, enc_key, priv_key);

	verify_data("EncryptionKey", 0, keys->enc_key, enc_key,
							sizeof(enc_key));
	verify_data("PrivacyKey", 0, keys->priv_key, priv_key,
							sizeof(priv_key));
	verify_uint8("NID", 0, keys->net_nid, nid);

	if (keys->network_only) {
		enc_msg = l_util_from_hexstring(keys->trans_pkt[0],
							&app_msg_len);
		check_encrypt_segment(keys, keys->seg_num, keys->seg_max,
				enc_msg + 4, app_msg_len - 4,
				enc_key, priv_key, nid);
		goto done;
	}

	if (keys->akf)
		mesh_crypto_application_nonce(keys->app_seq, keys->net_src,
						keys->net_dst, keys->iv_index,
						keys->szmic, app_nonce);
	else
		mesh_crypto_device_nonce(keys->app_seq, keys->net_src,
						keys->net_dst, keys->iv_index,
						keys->szmic, app_nonce);

	seqZero = keys->app_seq;

	if (!keys->ctl) {
		app_msg = l_util_from_hexstring(keys->app_msg, &app_msg_len);

		if (keys->szmic) {
			seg_max = SEG_MAX(keys->segmented, app_msg_len + 8);
			enc_msg = l_malloc(app_msg_len + 8);

			mesh_crypto_payload_encrypt(aad, app_msg,
					enc_msg, app_msg_len,
					keys->net_src, keys->net_dst, key_aid,
					keys->app_seq, keys->iv_index,
					keys->szmic,
					keys->akf ? app_key : dev_key);
		} else {
			seg_max = SEG_MAX(keys->segmented, app_msg_len + 4);
			enc_msg = l_malloc(app_msg_len + 4);

			mesh_crypto_payload_encrypt(aad, app_msg,
					enc_msg, app_msg_len,
					keys->net_src, keys->net_dst, key_aid,
					keys->app_seq, keys->iv_index,
					keys->szmic,
					keys->akf ? app_key : dev_key);
		}

		if (keys->dev_key && !keys->akf)
			show_data("DeviceKey", 0, dev_key, 16);

		if (keys->app_key && keys->akf)
			show_data("ApplicationKey", 0, app_key, 16);

		if (aad) show_data("UUID", 0, aad, 16);

		verify_data("EncryptionKey", 0, keys->enc_key, enc_key, 16);
		verify_data("ApplicationNonce", 0, keys->app_nonce,
								app_nonce, 13);
		verify_data("PrivacyKey", 0, keys->priv_key, priv_key, 16);

		show_data("AppPayload", 0, app_msg, app_msg_len);
		verify_data("EncryptedAppPayload", 0, keys->enc_msg, enc_msg,
								app_msg_len);
		if (keys->szmic) {
			app_mic64 = l_get_be64(enc_msg + app_msg_len);
			verify_uint64("ApplicationMIC", app_msg_len,
						keys->app_mic64, app_mic64);
			app_msg_len += 8;
		} else {
			app_mic32 = l_get_be32(enc_msg + app_msg_len);
			verify_uint32("ApplicationMIC", app_msg_len,
						keys->app_mic32, app_mic32);
			app_msg_len += 4;
		}
	} else {
		enc_msg = l_util_from_hexstring(keys->trans_pkt[0],
								&app_msg_len);
		seg_max = 0;
		app_msg_len--;
	}

	for (i = 0; i <= seg_max; i++) {
		if (seg_max) {
			if (i < seg_max || !(app_msg_len % 12))
				seg_len = 12;
			else
				seg_len = app_msg_len % 12;
		} else {
			seg_len = app_msg_len;
		}

		if (keys->ctl) {
			mesh_crypto_packet_build(keys->ctl, keys->net_ttl,
					keys->net_seq[i],
					keys->net_src, keys->net_dst,
					keys->opcode,
					keys->segmented, key_aid,
					keys->szmic, keys->relay, keys->seqZero,
					i, seg_max,
					enc_msg + 1, seg_len,
					packet, &packet_len);
		} else {
			mesh_crypto_packet_build(keys->ctl, keys->net_ttl,
					keys->net_seq[i],
					keys->net_src, keys->net_dst,
					keys->opcode,
					keys->segmented, key_aid,
					keys->szmic, keys->relay, seqZero,
					i, seg_max,
					enc_msg + (i * 12), seg_len,
					packet, &packet_len);
		}

		if (seg_max) l_info(COLOR_YELLOW "Segment-%d" COLOR_OFF, i);

		hdr = l_get_be32(packet + 9);
		verify_uint8("SEG", 9, keys->segmented << (SEG_HDR_SHIFT % 8),
					packet[9] & (1 << (SEG_HDR_SHIFT % 8)));

		if (keys->ctl) {
			verify_uint8("Opcode", 9,
				keys->opcode << (OPCODE_HDR_SHIFT % 8),
				(packet[9] & OPCODE_MASK) <<
							(OPCODE_HDR_SHIFT % 8));
		} else {
			verify_uint8("AKF", 9, keys->akf << (AKF_HDR_SHIFT % 8),
				packet[9] & (1 << (AKF_HDR_SHIFT % 8)));
			verify_uint8("AID", 9,
				keys->key_aid << (KEY_HDR_SHIFT % 8),
				(packet[9] & KEY_AID_MASK) <<
							(KEY_HDR_SHIFT % 8));
		}

		if (seg_max == 0) {
			if (!keys->ctl) {
				show_data("Payload", 10, app_msg, seg_len - 4);
				show_data("EncryptedPayload", 10, packet + 10,
								seg_len);
			} else if (keys->opcode == NET_OP_SEG_ACKNOWLEDGE) {
				verify_uint8("Relay", 10,
					keys->relay << (RELAY_HDR_SHIFT % 8),
					packet[10] &
						(1 << (RELAY_HDR_SHIFT % 8)));

				/* Awkward shift-by-two for correct display */
				verify_uint16("SeqZero", 10,
					(keys->seqZero & SEQ_ZERO_MASK) << 2,
					((hdr >> SEQ_ZERO_HDR_SHIFT) &
							SEQ_ZERO_MASK) << 2);

				show_data("Payload", 12, packet + 12, seg_len);
			} else {
				show_data("Payload", 10, packet + 10, seg_len);
			}
			seg_len += 1;
		} else {

			verify_uint8("SZMIC", 10,
				keys->szmic << (SZMIC_HDR_SHIFT % 8),
				packet[10] & (1 << (SZMIC_HDR_SHIFT % 8)));

			/* Awkward shift-by-two for correct display */
			verify_uint16("SeqZero", 10,
				(keys->app_seq & SEQ_ZERO_MASK) << 2,
				((hdr >> SEQ_ZERO_HDR_SHIFT) & SEQ_ZERO_MASK)
									<< 2);
			verify_uint16("SegO", 11,
					(i & SEG_MASK) << SEGO_HDR_SHIFT,
					hdr & (SEG_MASK << SEGO_HDR_SHIFT));
			verify_uint8("SegN", 12,
					(seg_max & SEG_MASK) << SEGN_HDR_SHIFT,
					hdr & (SEG_MASK << SEGN_HDR_SHIFT));
			show_data("Payload", 13, enc_msg + (i * 12), seg_len);
			seg_len += 4;
		}


		mesh_crypto_network_nonce(keys->ctl, keys->net_ttl,
					keys->net_seq[i], keys->net_src,
					keys->iv_index, net_nonce);

		verify_data("TransportData", 9, keys->trans_pkt[i],
							packet + 9, seg_len);

		verify_uint16("DST", 7, keys->net_dst, l_get_be16(packet + 7));
		net_msg_len = seg_len + 2;
		show_data("TransportPayload", 7, packet + 7, net_msg_len);

		mesh_crypto_packet_encrypt(packet, packet_len, enc_key,
						keys->iv_index, false,
						keys->ctl, keys->net_ttl,
						keys->net_seq[i],
						keys->net_src);

		mesh_crypto_privacy_counter(keys->iv_index, packet + 7,
								priv_rand);

		l_info("");
		show_uint32("IVindex", 0, keys->iv_index);
		verify_data("NetworkNonce", 0, keys->net_nonce[i],
								net_nonce, 13);
		verify_data("PrivacyRandom", 0, keys->priv_rand[i],
								priv_rand, 16);

		show_uint8("INI", 0, (keys->iv_index & 0x01) << 7);
		verify_uint8("NID", 0, keys->net_nid, nid);
		verify_uint8("CTL", 1, keys->ctl << 7, packet[1] & 0x80);
		verify_uint8("TTL", 1, keys->net_ttl, packet[1] & 0x7f);
		verify_uint24("SEQ", 2, keys->net_seq[i],
					l_get_be32(packet + 1) & SEQ_MASK);
		verify_uint16("SRC", 5, keys->net_src, l_get_be16(packet + 5));

		verify_data("EncNetworkPayload", 7, keys->net_msg[i],
						packet + 7, net_msg_len);
		if (keys->ctl) {
			net_mic64 = l_get_be64(packet + packet_len - 8);
			verify_uint64("NetworkMIC", 7 + net_msg_len,
						keys->net_mic64, net_mic64);
			net_msg_len += 8;
		} else {
			net_mic32 = l_get_be32(packet + packet_len - 4);
			verify_uint32("NetworkMIC", 7 + net_msg_len,
						keys->net_mic32[i], net_mic32);
			net_msg_len += 4;
		}

		show_data("PreObsPayload", 1, packet + 1, 6 + net_msg_len);
		mesh_crypto_network_obfuscate(packet, priv_key,
					keys->iv_index,
					keys->ctl, keys->net_ttl,
					keys->net_seq[i], keys->net_src);

		show_data("PostObsPayload", 1, packet + 1, 6 + net_msg_len);

		packet[0] = (keys->iv_index & 0x01) << 7 | nid;
		packet_len = 7 + net_msg_len;

		verify_data("Packet", 0, keys->packet[i], packet, packet_len);
		l_info("");
	}

done:
	l_free(dev_key);
	l_free(app_key);
	l_free(aad);
	l_free(net_key);
	l_free(app_msg);
	l_free(enc_msg);
}

static void check_decrypt_segment(const struct mesh_crypto_test *keys,
				uint16_t seg, uint16_t seg_max,
				uint8_t *pkt, uint8_t pkt_len,
				const uint8_t *msg, uint8_t msg_len,
				uint8_t *enc_key, uint8_t *priv_key,
				uint8_t nid)
{
	uint8_t net_clr[29];
	uint64_t net_mic64, calc_net_mic64;
	uint32_t hdr, net_mic32, calc_net_mic32;
	bool ctl, segmented, relay, szmic, key_akf;
	uint8_t ttl, opcode, key_aid, segO, segN;
	uint32_t seq;
	uint16_t src, dst, seqZero;

	memcpy(net_clr, pkt, pkt_len);
	show_data("NetworkMessage", 0, pkt, pkt_len);
	mesh_crypto_packet_decode(pkt, pkt_len,
				false, net_clr, keys->iv_index,
				enc_key, priv_key);
	show_data("Decoded", 0, net_clr, pkt_len);

	mesh_crypto_packet_parse(net_clr, pkt_len,
			&ctl, &ttl, &seq,
			&src, &dst,
			NULL, &opcode,
			&segmented, &key_aid,
			&szmic, &relay, &seqZero,
			&segO, &segN,
			&msg, &msg_len);

	if (ctl) {
		net_mic64 = l_get_be64(pkt + pkt_len - 8);
		show_data("EncryptedPayload", 7, pkt + 7, pkt_len - 7 - 8);

		mesh_crypto_packet_decrypt(pkt, pkt_len,
							enc_key,
							keys->iv_index, false,
							ctl, ttl, seq,
							src);
		calc_net_mic64 = l_get_be64(pkt + pkt_len - 8);

		verify_uint64("NetworkMIC", pkt_len - 8, net_mic64,
						net_mic64 ^ calc_net_mic64);
		show_data("DecryptedPayload", 7, net_clr + 7, pkt_len - 7 - 8);
	} else {
		net_mic32 = l_get_be32(pkt + pkt_len - 4);
		show_data("EncryptedPayload", 7, pkt + 7, pkt_len - 7 - 4);

		mesh_crypto_packet_decrypt(pkt, pkt_len,
							enc_key,
							keys->iv_index, false,
							ctl, ttl, seq,
							src);
		calc_net_mic32 = l_get_be32(pkt + pkt_len - 4);

		verify_uint32("NetworkMIC", pkt_len - 4, net_mic32,
						net_mic32 ^ calc_net_mic32);
		show_data("DecryptedPayload", 7, net_clr + 7, pkt_len - 7 - 4);
	}

	hdr = l_get_be32(net_clr + 9);

	segmented = !!((hdr >> SEG_HDR_SHIFT) & 1);
	if (ctl) {
		opcode = (hdr >> OPCODE_HDR_SHIFT) & OPCODE_MASK;
		if (opcode == NET_OP_SEG_ACKNOWLEDGE) {
			relay = !!((hdr >> RELAY_HDR_SHIFT) & 1);
			seqZero = (hdr >> SEQ_ZERO_HDR_SHIFT) & SEQ_ZERO_MASK;
			verify_uint24("SeqZero", 9,
					((keys->seqZero) & SEQ_ZERO_MASK)
					<< (SEQ_ZERO_HDR_SHIFT - 8),
					seqZero << (SEQ_ZERO_HDR_SHIFT - 8));
			verify_uint24("Relay", 9,
					keys->relay << (RELAY_HDR_SHIFT - 8),
					relay << (RELAY_HDR_SHIFT - 8));
			verify_uint24("Opcode", 9,
					keys->opcode << (OPCODE_HDR_SHIFT - 8),
					opcode << (OPCODE_HDR_SHIFT - 8));
			verify_uint24("SEGMENTED", 9,
					keys->segmented << (SEG_HDR_SHIFT - 8),
					segmented << (SEG_HDR_SHIFT - 8));
		} else {
			verify_uint8("Opcode", 9,
					keys->opcode << (OPCODE_HDR_SHIFT % 8),
					opcode << (OPCODE_HDR_SHIFT % 8));
			verify_uint8("SEGMENTED", 9,
					keys->segmented << (SEG_HDR_SHIFT % 8),
					segmented << (SEG_HDR_SHIFT % 8));
		}
	} else {
		key_akf = !!((hdr >> AKF_HDR_SHIFT) & 1);
		key_aid = (hdr >> KEY_HDR_SHIFT) & KEY_AID_MASK;
		if (segmented) {
			show_data("EncryptedApp", 13, net_clr + 13,
					pkt_len - 13 - 4);
			segN = (hdr >> SEGN_HDR_SHIFT) & SEG_MASK;
			segO = (hdr >> SEGO_HDR_SHIFT) & SEG_MASK;
			seqZero =  ((hdr >> SEQ_ZERO_HDR_SHIFT) &
					SEQ_ZERO_MASK);

			verify_uint32("SegN", 9,
					seg_max << SEGN_HDR_SHIFT,
					segN << SEGN_HDR_SHIFT);
			verify_uint32("SegO", 9, seg << SEGO_HDR_SHIFT,
					segO << SEGO_HDR_SHIFT);
			verify_uint32("SeqZero", 9,
					(keys->seqZero & SEQ_ZERO_MASK)
						<< SEQ_ZERO_HDR_SHIFT,
					(seqZero & SEQ_ZERO_MASK)
						<< SEQ_ZERO_HDR_SHIFT);
			verify_uint32("AID", 9,
					keys->key_aid << KEY_HDR_SHIFT,
					key_aid << KEY_HDR_SHIFT);
			verify_uint32("AKF", 9,
					keys->akf << AKF_HDR_SHIFT,
					key_akf << AKF_HDR_SHIFT);
			verify_uint32("SEGMENTED", 9,
					keys->segmented << SEG_HDR_SHIFT,
					segmented << SEG_HDR_SHIFT);
		} else {
			show_data("EncryptedApp", 10, msg + 3, pkt_len - 3);
			verify_uint8("AID", 9,
					keys->key_aid << (KEY_HDR_SHIFT % 8),
					key_aid << (KEY_HDR_SHIFT % 8));
			verify_uint8("AKF", 9,
					keys->akf << (AKF_HDR_SHIFT % 8),
					key_aid << (AKF_HDR_SHIFT % 8));
			verify_uint8("SEGMENTED", 9,
					keys->segmented << (SEG_HDR_SHIFT % 8),
					segmented << (SEG_HDR_SHIFT % 8));
		}
	}

	dst = l_get_be16(net_clr + 7);

	verify_uint16("DST", 7, keys->net_dst, dst);
	verify_uint16("SRC", 5, keys->net_src, src);
	verify_uint24("SEQ", 2, keys->net_seq[0], seq);
	verify_uint8("TTL", 1, keys->net_ttl, ttl);
	verify_uint8("CTL", 1, keys->ctl << 7, ctl << 7);
	verify_uint8("NID", 0, keys->net_nid, net_clr[0] & 0x7f);
	verify_uint8("INI", 0, (keys->iv_index & 0x01) << 7, net_clr[0] & 0x80);
}

static void check_decrypt(const struct mesh_crypto_test *keys)
{
	uint8_t *dev_key;
	uint8_t *app_key;
	uint8_t *net_key;
	uint8_t enc_key[16];
	uint8_t priv_key[16];
	uint8_t p[9];
	size_t p_len;
	uint8_t *packet = NULL;
	size_t packet_len;
	uint8_t *net_msg;
	uint8_t net_msg_len;
	uint16_t app_msg_len = 0;
	uint32_t calc_net_mic32, net_mic32 = 0;
	uint64_t calc_net_mic64, net_mic64 = 0;
	bool net_ctl, net_segmented, net_rly, net_akf;
	uint8_t net_aid, net_ttl, nid, net_segO, net_segN = 0;
	uint32_t net_seq, hdr, seqZero = 0;
	uint16_t net_src, net_dst;
	uint32_t calc_app_mic32;
	uint64_t calc_app_mic64;
	uint32_t app_mic32;
	uint64_t app_mic64;
	uint8_t *app_msg;
	uint8_t *aad;
	uint8_t *payload;
	size_t trans_msg_len, payload_len, aad_len = 0;
	uint8_t pkt_len, hdr_len, net_op = 0;
	uint16_t i, seg_max;
	uint8_t keys_aid = 0;

	l_info(COLOR_BLUE "[Decrypt %s]" COLOR_OFF, keys->name);
	verify_bool_not_both("CTL && Segmented", 0, keys->ctl, keys->segmented);
	dev_key = l_util_from_hexstring(keys->dev_key, NULL);
	app_key = l_util_from_hexstring(keys->app_key, NULL);
	net_key = l_util_from_hexstring(keys->net_key, NULL);
	aad = l_util_from_hexstring(keys->uuid, &aad_len);

	if (keys->frnd) {
		uint8_t *lpn_addr, *fn_addr, *lpn_cntr, *fn_cntr;

		lpn_addr = l_util_from_hexstring(keys->lpn_addr, NULL);
		fn_addr = l_util_from_hexstring(keys->fn_addr, NULL);
		lpn_cntr = l_util_from_hexstring(keys->lpn_cntr, NULL);
		fn_cntr = l_util_from_hexstring(keys->fn_cntr, NULL);

		show_data("LPN Address", 0, lpn_addr, 2);
		show_data("Friend Address", 0, fn_addr, 2);
		show_data("LPN Counter", 0, lpn_cntr, 2);
		show_data("Friend Counter", 0, fn_cntr, 2);
		l_info("");

		p[0] = 1;
		l_put_be16(l_get_be16(lpn_addr), p + 1);
		l_put_be16(l_get_be16(fn_addr), p + 3);
		l_put_be16(l_get_be16(lpn_cntr), p + 5);
		l_put_be16(l_get_be16(fn_cntr), p + 7);
		p_len = 9;

		l_free(fn_cntr);
		l_free(lpn_cntr);
		l_free(fn_addr);
		l_free(lpn_addr);
	} else {
		p[0] = 0;
		p_len = 1;
	}

	if (p_len > 1) verify_data("P", 0, keys->p, p, p_len);
	mesh_crypto_k2(net_key, p, p_len, &nid, enc_key, priv_key);


	if (keys->network_only) {
		app_msg = l_util_from_hexstring(keys->trans_pkt[0],
							&trans_msg_len);
		packet = l_util_from_hexstring(keys->packet[0], &packet_len);
		check_decrypt_segment(keys, keys->seg_num, keys->seg_max,
				packet, packet_len,
				app_msg + 4, trans_msg_len - 4,
				enc_key, priv_key, nid);
		goto done;
	}

	app_msg = l_malloc(384);

	seg_max = (sizeof(keys->packet) / sizeof(keys->packet[0])) - 1;

	/* Calculate number of segments in sample data */
	while (keys->packet[seg_max] == NULL && seg_max) seg_max--;

	for (i = 0; i <= seg_max; i++) {
		if (keys->segmented)
			l_info(COLOR_YELLOW "Segment-%d" COLOR_OFF, i);

		l_free(packet);
		packet = l_util_from_hexstring(keys->packet[i], &packet_len);

		net_msg = packet + 7;
		net_msg_len = packet_len - 7;

		mesh_crypto_network_clarify(packet, priv_key, keys->iv_index,
				&net_ctl, &net_ttl, &net_seq, &net_src);

		show_str("Packet", 0, keys->packet[i]);

		if (net_ctl) {
			net_mic64 = l_get_be64(packet + packet_len - 8);
			show_data("NetworkMessage", 7, net_msg,
							net_msg_len - 8);
			mesh_crypto_packet_decrypt(packet, packet_len,
						enc_key,
						keys->iv_index, false,
						net_ctl, net_ttl,
						net_seq,
						net_src);
			calc_net_mic64 = l_get_be64(packet + packet_len - 8);
			net_msg_len -= 8;
			verify_uint64("NetworkMIC", 7 + net_msg_len, net_mic64,
						net_mic64 ^ calc_net_mic64);
			show_data("DecryptedNetwork", 7, net_msg, net_msg_len);
		} else {
			net_mic32 = l_get_be32(packet + packet_len - 4);
			show_data("NetworkMessage", 7, net_msg,
							net_msg_len - 4);
			mesh_crypto_packet_decrypt(packet, packet_len,
						enc_key,
						keys->iv_index, false,
						net_ctl, net_ttl,
						net_seq,
						net_src);
			calc_net_mic32 = l_get_be32(packet + packet_len - 4);
			net_msg_len -= 4;
			verify_uint32("NetworkMIC", 7 + net_msg_len, net_mic32,
						net_mic32 ^ calc_net_mic32);
			show_data("DecryptedNetwork", 7, net_msg, net_msg_len);
		}


		hdr = l_get_be32(packet + 9);

		net_segmented = !!((hdr >> SEG_HDR_SHIFT) & 1);
		if (net_ctl) {
			net_op = (hdr >> OPCODE_HDR_SHIFT) & OPCODE_MASK;
			hdr_len = 1;
			if (net_op == NET_OP_SEG_ACKNOWLEDGE) {
				net_rly = !!((hdr >> RELAY_HDR_SHIFT) & 1);
				seqZero = (hdr >> SEQ_ZERO_HDR_SHIFT) &
								SEQ_ZERO_MASK;
				verify_uint24("SeqZero", 9,
					((keys->seqZero) & SEQ_ZERO_MASK)
						<< (SEQ_ZERO_HDR_SHIFT - 8),
					seqZero << (SEQ_ZERO_HDR_SHIFT - 8));
				verify_uint24("Relay", 9,
					keys->relay << (RELAY_HDR_SHIFT - 8),
					net_rly << (RELAY_HDR_SHIFT - 8));
				verify_uint24("Opcode", 9,
					keys->opcode << (OPCODE_HDR_SHIFT - 8),
					net_op << (OPCODE_HDR_SHIFT - 8));
				verify_uint24("SEGMENTED", 9,
					keys->segmented << (SEG_HDR_SHIFT - 8),
					net_segmented << (SEG_HDR_SHIFT - 8));
			} else {
				verify_uint8("Opcode", 9,
					keys->opcode << (OPCODE_HDR_SHIFT % 8),
					net_op << (OPCODE_HDR_SHIFT % 8));
				verify_uint8("SEGMENTED", 9,
					keys->segmented << (SEG_HDR_SHIFT % 8),
					net_segmented << (SEG_HDR_SHIFT % 8));
			}
		} else {
			net_akf = !!((hdr >> AKF_HDR_SHIFT) & 1);
			net_aid = (hdr >> KEY_HDR_SHIFT) & KEY_AID_MASK;
			if (net_segmented) {
				hdr_len = 4;
				show_data("EncryptedApp", 13, net_msg + 6,
							net_msg_len - 6);
				memcpy(app_msg + (12 * i), net_msg + 6,
							net_msg_len - 6);
				app_msg_len += net_msg_len - 6;
				net_segN = (hdr >> SEGN_HDR_SHIFT) & SEG_MASK;
				net_segO = (hdr >> SEGO_HDR_SHIFT) & SEG_MASK;
				seqZero =  ((hdr >> SEQ_ZERO_HDR_SHIFT)
							& SEQ_ZERO_MASK) |
						(net_seq & ~SEQ_ZERO_MASK);

				if (seqZero > net_seq) seqZero -=
							(SEQ_ZERO_MASK + 1);
				verify_uint32("SegN", 9,
						seg_max << SEGN_HDR_SHIFT,
						net_segN << SEGN_HDR_SHIFT);
				verify_uint32("SegO", 9, i << SEGO_HDR_SHIFT,
						net_segO << SEGO_HDR_SHIFT);
				verify_uint32("SeqZero", 9,
					(keys->app_seq & SEQ_ZERO_MASK)
							<< SEQ_ZERO_HDR_SHIFT,
					(seqZero & SEQ_ZERO_MASK)
							<< SEQ_ZERO_HDR_SHIFT);
				verify_uint32("AID", 9,
					keys->key_aid << KEY_HDR_SHIFT,
					net_aid << KEY_HDR_SHIFT);
				verify_uint32("AKF", 9,
					keys->akf << AKF_HDR_SHIFT,
					net_akf << AKF_HDR_SHIFT);
				verify_uint32("SEGMENTED", 9,
					keys->segmented << SEG_HDR_SHIFT,
					net_segmented << SEG_HDR_SHIFT);
			} else {
				hdr_len = 1;
				show_data("EncryptedApp", 10, net_msg + 3,
							net_msg_len - 3);
				memcpy(app_msg + (12 * i), net_msg + 3,
							net_msg_len - 3);
				app_msg_len += net_msg_len - 3;
				seqZero = net_seq;
				verify_uint8("AID", 9,
				keys->key_aid << (KEY_HDR_SHIFT % 8),
				net_aid << (KEY_HDR_SHIFT % 8));
				verify_uint8("AKF", 9,
				keys->akf << (AKF_HDR_SHIFT % 8),
				net_akf << (AKF_HDR_SHIFT % 8));
				verify_uint8("SEGMENTED", 9,
				keys->segmented << (SEG_HDR_SHIFT % 8),
				net_segmented << (SEG_HDR_SHIFT % 8));
			}
		}

		net_dst = l_get_be16(net_msg);

		verify_uint16("DST", 7, keys->net_dst, net_dst);
		verify_uint16("SRC", 5, keys->net_src, net_src);
		verify_uint24("SEQ", 2, keys->net_seq[i], net_seq);
		verify_uint8("TTL", 1, keys->net_ttl, net_ttl);
		verify_uint8("CTL", 1, keys->ctl << 7, net_ctl << 7);
		verify_uint8("NID", 0, keys->net_nid, nid);
		verify_uint8("INI", 0, (keys->iv_index & 0x01) << 7,
							packet[0] & 0x80);

		payload = l_util_from_hexstring(keys->trans_pkt[i],
								&payload_len);
		memset(packet, 0, packet_len);

		mesh_crypto_packet_build(keys->ctl, keys->net_ttl,
				keys->net_seq[i], keys->net_src,
				keys->net_dst, net_op,
				keys->segmented,
				keys->key_aid | (keys->akf ? KEY_ID_AKF : 0),
				keys->szmic, keys->relay, seqZero,
				i, seg_max,
				payload + hdr_len, payload_len - hdr_len,
				packet, &pkt_len);
		verify_data("TransportData", 9, keys->trans_pkt[i], packet + 9,
								payload_len);
		mesh_crypto_packet_encode(packet, pkt_len, keys->iv_index,
							enc_key, priv_key);
		mesh_crypto_packet_label(packet, pkt_len, keys->iv_index, nid);

		verify_data("Encoded-Packet", 0, keys->packet[i], packet,
								pkt_len);

		l_free(payload);

		l_info("");
		mesh_crypto_packet_decode(packet, pkt_len, false, packet,
					keys->iv_index, enc_key, priv_key);
		show_data("Decoded-Packet", 0, packet, pkt_len);
		l_info("");
	}

	if (keys->segmented && keys->szmic) {
		verify_data("EncryptedPayload", 0, keys->enc_msg, app_msg,
							app_msg_len - 8);
		app_mic64 = l_get_be64(app_msg + app_msg_len - 8);

		mesh_crypto_payload_decrypt(
				aad, aad_len,
				app_msg, app_msg_len,
				true,
				net_src, net_dst,
				keys->akf ? keys_aid | KEY_ID_AKF : APP_AID_DEV,
				seqZero,
				keys->iv_index,
				app_msg,
				keys->akf ? app_key : dev_key);

		calc_app_mic64 = l_get_be64(app_msg + app_msg_len - 8);

		verify_data("Payload", 0, keys->app_msg, app_msg,
							app_msg_len - 8);
		verify_uint64("ApplicationMIC", app_msg_len - 8, app_mic64,
						app_mic64 ^ calc_app_mic64);
	} else if (!keys->ctl) {
		verify_data("EncryptedPayload", 0, keys->enc_msg, app_msg,
							app_msg_len - 4);
		app_mic32 = l_get_be32(app_msg + app_msg_len - 4);

		mesh_crypto_payload_decrypt(
				aad, aad_len,
				app_msg, app_msg_len,
				false,
				net_src, net_dst,
				keys->akf ? keys_aid | KEY_ID_AKF : APP_AID_DEV,
				seqZero,
				keys->iv_index,
				app_msg,
				keys->akf ? app_key : dev_key);

		calc_app_mic32 = l_get_be32(app_msg + app_msg_len - 4);

		verify_data("Payload", 0, keys->app_msg, app_msg,
							app_msg_len - 4);
		verify_uint32("ApplicationMIC", app_msg_len - 4, app_mic32,
						app_mic32 ^ calc_app_mic32);
	}

done:

	l_info("");

	l_free(dev_key);
	l_free(aad);
	l_free(app_key);
	l_free(net_key);
	l_free(app_msg);
	l_free(packet);
}

static void check_beacon(const struct mesh_crypto_test *keys)
{
	uint8_t *net_key;
	uint8_t *beacon_cmac;
	uint8_t beacon[22];
	uint8_t enc_key[16];
	uint8_t net_id[8];
	uint8_t cmac[8];
	uint64_t cmac_tmp;

	net_key = l_util_from_hexstring(keys->net_key, NULL);
	beacon_cmac = l_util_from_hexstring(keys->beacon_cmac, NULL);

	mesh_crypto_nkbk(net_key, enc_key);
	mesh_crypto_k3(net_key, net_id);

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	verify_data("NetworkKey", 0, keys->net_key, net_key, 16);
	show_uint32("IVindex", 0, keys->iv_index);

	verify_data("BeaconKey", 0, keys->enc_key, enc_key, 16);
	verify_data("NetworkID", 0, keys->net_id, net_id, 8);

	beacon[0] = keys->beacon_type;
	beacon[1] = keys->beacon_flags;
	memcpy(beacon + 2, net_id, 8);
	l_put_be32(keys->iv_index, beacon + 10);
	mesh_crypto_beacon_cmac(enc_key, net_id, keys->iv_index,
					!!(keys->beacon_flags & 0x01),
					!!(keys->beacon_flags & 0x02),
					&cmac_tmp);

	l_put_be64(cmac_tmp, cmac);
	l_put_be64(cmac_tmp, beacon + 14);
	verify_data("BeaconCMAC", 0, keys->beacon_cmac, cmac, 8);
	verify_data("Beacon", 0, keys->beacon, beacon, sizeof(beacon));

	l_info("");

	l_free(beacon_cmac);
	l_free(net_key);
}

static void check_id_beacon(const struct mesh_crypto_test *keys)
{
	uint8_t *net_key;
	uint8_t *rand;
	uint8_t identity_key[16];
	uint8_t hash_input[16];
	uint8_t hash[16];
	uint8_t beacon[17];

	net_key = l_util_from_hexstring(keys->net_key, NULL);
	rand = l_util_from_hexstring(keys->rand, NULL);

	mesh_crypto_nkik(net_key, identity_key);

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	verify_data("ID Resolving Key", 0, keys->ident_res_key,
							identity_key, 16);
	memset(hash_input, 0, 6);
	memcpy(hash_input + 6, rand, 8);
	l_put_be16(keys->net_src, hash_input + 6 + 8);

	verify_data("Hash Input", 0, keys->hash_input, hash_input, 16);
	aes_ecb_one(identity_key, hash_input, hash);

	verify_data("Hash", 0, keys->identity_hash, hash + 8, 8);

	beacon[0] = 0x01;
	memcpy(beacon + 1, hash + 8, 8);
	memcpy(beacon + 9, rand, 8);

	verify_data("Mesh ID Beacon", 0, keys->beacon, beacon, 17);

	l_info("");

	l_free(rand);
	l_free(net_key);
}

static void check_s1(const struct mesh_crypto_test *keys)
{
	uint8_t salt_out[16];

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	show_str("Salt Input", 0, keys->salt);
	show_data("Salt Input", 0, keys->salt, strlen(keys->salt));
	mesh_crypto_s1(keys->salt, strlen(keys->salt), salt_out);

	verify_data("s1(Salt)", 0, keys->salt_out, salt_out, 16);
	l_info("");
}

static void check_k128(const struct mesh_crypto_test *keys)
{
	uint8_t salt[16];
	uint8_t t[16];
	uint8_t enc_key[16];
	uint8_t *net_key;

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	net_key = l_util_from_hexstring(keys->net_key, NULL);
	show_data("NetKey", 0, net_key, 16);
	l_info("");

	show_str("Salt Input", 0, keys->salt);
	show_data("Salt Input", 0, keys->salt, strlen(keys->salt));
	mesh_crypto_s1(keys->salt, strlen(keys->salt), salt);
	show_data("s1(Salt)", 0, salt, 16);
	l_info("");

	show_str("Info", 0, keys->info);
	show_data("Info", 0, keys->info, strlen(keys->info));
	l_info("");

	mesh_crypto_aes_cmac(salt, net_key, 16, t);
	show_data("T", 0, t, 16);
	l_info("");

	if (memcmp(keys->salt, "nkbk", 4) == 0)
		mesh_crypto_nkbk(net_key, enc_key);
	else if (memcmp(keys->salt, "nkpk", 4) == 0)
		mesh_crypto_nkpk(net_key, enc_key);
	else if (memcmp(keys->salt, "nkik", 4) == 0)
		mesh_crypto_nkik(net_key, enc_key);

	verify_data("k1(N, salt, info)", 0, keys->enc_key, enc_key, 16);

	l_free(net_key);
	l_info("");
}

static void check_k1(const struct mesh_crypto_test *keys)
{
	uint8_t salt[16];
	uint8_t info[16];
	uint8_t t[16];
	uint8_t okm[16];
	uint8_t *ikm;

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	ikm = l_util_from_hexstring(keys->ikm, NULL);
	show_data("IKM", 0, ikm, 16);
	l_info("");

	show_str("Salt Input", 0, keys->salt);
	show_data("Salt Input", 0, keys->salt, strlen(keys->salt));
	mesh_crypto_s1(keys->salt, strlen(keys->salt), salt);
	show_data("s1(Salt)", 0, salt, 16);
	l_info("");

	show_str("Info Input", 0, keys->info);
	show_data("Info Input", 0, keys->info, strlen(keys->info));
	mesh_crypto_s1(keys->info, strlen(keys->info), info);
	show_data("s1(Info)", 0, info, 16);
	l_info("");

	mesh_crypto_aes_cmac(salt, ikm, 16, t);
	show_data("T", 0, t, 16);
	l_info("");

	mesh_crypto_k1(ikm, salt, info, 16, okm);

	verify_data("k1(ikm, salt, info)", 0, keys->okm, okm, 16);

	l_free(ikm);
	l_info("");
}

static void check_k2(const struct mesh_crypto_test *keys)
{
	uint8_t *net_key;
	uint8_t p[9];
	size_t  p_len;
	uint8_t nid;
	uint8_t enc_key[16];
	uint8_t priv_key[16];

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	net_key = l_util_from_hexstring(keys->net_key, NULL);
	show_data("NetKey", 0, net_key, 16);
	l_info("");

	if (keys->frnd) {
		uint8_t *lpn_addr, *fn_addr, *lpn_cntr, *fn_cntr;

		lpn_addr = l_util_from_hexstring(keys->lpn_addr, NULL);
		fn_addr = l_util_from_hexstring(keys->fn_addr, NULL);
		lpn_cntr = l_util_from_hexstring(keys->lpn_cntr, NULL);
		fn_cntr = l_util_from_hexstring(keys->fn_cntr, NULL);

		show_data("LPN Address", 0, lpn_addr, 2);
		show_data("Friend Address", 0, fn_addr, 2);
		show_data("LPN Counter", 0, lpn_cntr, 2);
		show_data("Friend Counter", 0, fn_cntr, 2);
		l_info("");

		p[0] = 1;
		l_put_be16(l_get_be16(lpn_addr), p + 1);
		l_put_be16(l_get_be16(fn_addr), p + 3);
		l_put_be16(l_get_be16(lpn_cntr), p + 5);
		l_put_be16(l_get_be16(fn_cntr), p + 7);
		p_len = 9;

		l_free(fn_cntr);
		l_free(lpn_cntr);
		l_free(fn_addr);
		l_free(lpn_addr);
	} else {
		p[0] = 0;
		p_len = 1;
	}

	mesh_crypto_k2(net_key, p, p_len, &nid, enc_key, priv_key);
	verify_data("P", 0, keys->p, p, p_len);
	l_info("");
	verify_data("NID", 0, keys->nid, &nid, 1);
	verify_data("EncryptionKey", 0, keys->enc_key, enc_key, 16);
	verify_data("PrivacyKey", 0, keys->priv_key, priv_key, 16);
	l_free(net_key);
	l_info("");
}

static void check_k3(const struct mesh_crypto_test *keys)
{
	uint8_t *net_key;
	uint8_t tmp[16];
	uint8_t short_net_id[8];

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	net_key = l_util_from_hexstring(keys->net_key, NULL);

	show_data("NetKey", 0, net_key, 16);
	l_info("");

	show_str("Salt Input", 0, keys->salt);
	show_data("Salt Input", 0, keys->salt, strlen(keys->salt));
	mesh_crypto_s1(keys->salt, strlen(keys->salt), tmp);
	show_data("s1(Salt)", 0, tmp, 16);
	l_info("");

	show_str("Info", 0, keys->info);
	show_data("Info", 0, keys->info, strlen(keys->info));
	l_info("");

	mesh_crypto_aes_cmac(tmp, net_key, 16, tmp);
	show_data("T", 0, tmp, 16);
	l_info("");

	mesh_crypto_k3(net_key, short_net_id);
	verify_data("k3(NetKey)", 0, keys->short_net_id, short_net_id, 8);
	l_free(net_key);
	l_info("");
}

static void check_k4(const struct mesh_crypto_test *keys)
{
	uint8_t *app_key;
	uint8_t tmp[16];
	uint8_t aid;

	l_info(COLOR_BLUE "[%s]" COLOR_OFF, keys->name);

	app_key = l_util_from_hexstring(keys->app_key, NULL);

	show_data("AppKey", 0, app_key, 16);
	l_info("");

	show_str("Salt Input", 0, keys->salt);
	show_data("Salt Input", 0, keys->salt, strlen(keys->salt));
	mesh_crypto_s1(keys->salt, strlen(keys->salt), tmp);
	show_data("s1(Salt)", 0, tmp, 16);
	l_info("");

	show_str("Info", 0, keys->info);
	show_data("Info", 0, keys->info, strlen(keys->info));
	l_info("");

	mesh_crypto_aes_cmac(tmp, app_key, 16, tmp);
	show_data("T", 0, tmp, 16);
	l_info("");

	mesh_crypto_k4(app_key, &aid);
	verify_data("k4(AppKey)", 0, keys->aid, &aid, 1);
	l_free(app_key);
	l_info("");
}

int main(int argc, char *argv[])
{
	l_log_set_stderr();

	/* Section 8.1 Sample Data Tests */
	check_s1(&s8_1_1);
	check_k1(&s8_1_2);
	check_k2(&s8_1_3);
	check_k2(&s8_1_4);
	check_k3(&s8_1_5);
	check_k4(&s8_1_6);

	/* Section 8.2 Sample Data Tests */
	check_k4(&s8_2_1);
	check_k2(&s8_2_2);
	check_k2(&s8_2_3);
	check_k3(&s8_2_4);
	check_k128(&s8_2_5);
	check_k128(&s8_2_6);

	/* Section 8.3 Sample Data Tests */
	check_encrypt(&s8_3_1);
	check_decrypt(&s8_3_1);
	check_encrypt(&s8_3_2);
	check_decrypt(&s8_3_2);
	check_encrypt(&s8_3_3);
	check_decrypt(&s8_3_3);
	check_encrypt(&s8_3_4);
	check_decrypt(&s8_3_4);
	check_encrypt(&s8_3_5);
	check_decrypt(&s8_3_5);
	check_encrypt(&s8_3_6);
	check_decrypt(&s8_3_6);
	check_encrypt(&s8_3_7);
	check_decrypt(&s8_3_7);
	check_encrypt(&s8_3_8); /* Single segment tester unavailable */
	check_decrypt(&s8_3_8); /* Single segment tester unavailable */
	check_encrypt(&s8_3_9);
	check_decrypt(&s8_3_9);
	check_encrypt(&s8_3_10);
	check_decrypt(&s8_3_10);
	check_encrypt(&s8_3_11); /* Single segment tester unavailable */
	check_decrypt(&s8_3_11); /* Single segment tester unavailable */
	check_encrypt(&s8_3_22);
	check_decrypt(&s8_3_22);

	/* Section 8.4 Beacon Sample Data */
	check_beacon(&s8_4_3);

	/* Section 8.6 Mesh Proxy Service sample data */
	check_id_beacon(&s8_6_2);

	return 0;
}
