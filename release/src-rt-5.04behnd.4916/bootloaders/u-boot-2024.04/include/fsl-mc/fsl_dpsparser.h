/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Data Path Soft Parser API
 *
 * Copyright 2018, 2023 NXP
 */
#ifndef _FSL_DPSPARSER_H
#define _FSL_DPSPARSER_H

/* DPSPARSER last supported API version */
#define DPSPARSER_VER_MAJOR				1
#define DPSPARSER_VER_MINOR				0

/* Command IDs */
#define DPSPARSER_CMDID_CLOSE				0x8001
#define DPSPARSER_CMDID_OPEN				0x8111
#define DPSPARSER_CMDID_CREATE				0x9111
#define DPSPARSER_CMDID_DESTROY				0x9911
#define DPSPARSER_CMDID_GET_API_VERSION			0xa111

#define DPSPARSER_CMDID_APPLY_SPB			0x1181

#pragma pack(push, 1)

struct dpsparser_cmd_destroy {
	__le32 dpsparser_id;
};

struct dpsparser_cmd_blob_set_address {
	__le64 blob_addr;
};

struct dpsparser_rsp_blob_report_error {
	__le16 error;
};

struct dpsparser_rsp_get_api_version {
	__le16 major;
	__le16 minor;
};

#pragma pack(pop)

/* Data Path Soft Parser API
 * Contains initialization APIs and runtime control APIs for DPSPARSER
 */

struct fsl_mc_io;

/* MC Unknown error: */
#define MC_ERROR_MSG_SPB_UNKNOWN	"Unknown MC error\n"

/* MC Error messages (in order for each error code defined above): */
#define MC_ERROR_MSG_APPLY_SPB \
{ \
	"OK\n", \
	"BLOB : Magic number does not match\n", \
	"BLOB : Version does not match MC API version\n", \
	"BLOB : IP revision does not match HW revision\n", \
	"BLOB : Blob length is not a multiple of 4\n", \
	"BLOB : Invalid length detected\n", \
	"BLOB : Name length < 0 in 'blob-name'\n", \
	"BLOB : Name length not a 4 multiple in 'blob-name'\n", \
	"BLOB : No target HW parser selected\n", \
	"BLOB : SP size is negative\n", \
	"BLOB : Size is zero\n", \
	"BLOB : Number of protocols is negative\n", \
	"BLOB : Zero protocols\n", \
	"BLOB : Protocol name is null\n", \
	"BLOB : SP 'seq-start' is not in [0x40, 0xffc0) range\n", \
	"BLOB : Invalid base protocol\n", \
	"BLOB : Invalid parameters section\n", \
	"BLOB : Invalid parameter\n", \
	"BLOB : Invalid parameter configuration\n", \
	"BLOB : Not aligned value\n", \
	"BLOB : Invalid section TAG detected\n", \
	"BLOB : Section size is zero\n", \
	"BLOB : Section size not a 4 multiple\n", \
	"BLOB : Section size is too big\n", \
	"BLOB : No 'bytecode' section before\n", \
	"BLOB : No 'sp-protocols' section before\n", \
	"BLOB : No 'bytecode' section defined\n", \
	"BLOB : No 'sp-protocols' section defined\n", \
	"BLOB : Soft Parser BLOB parsing : Error detected\n", \
	"apply spb : Soft Parser BLOB is already applied\n", \
	"apply spb : BLOB address is not set\n", \
	"BLOB : SP parameter offset is not a 4 multiple\n", \
	"BLOB : SP parameter offset can't be less than 0x40\n", \
	"BLOB : Bytecode size is not a 4 multiple\n", \
	"BLOB : Bytecode size cannot be zero\n", \
	"BLOB : Bytecode can't overwrite the 0xFFE address\n", \
	"BLOB : No hardware parser selected as target\n", \
	"BLOB : Bytecode overlap detected\n", \
	"BLOB : No parser support\n", \
	"BLOB : Too many bytecode sections on WRIOP ingress\n", \
	"BLOB : Too many bytecode sections on WRIOP egress\n", \
	"BLOB : Too many bytecode sections on AIOP\n", \
	"BLOB : Duplicated protocol is already registered\n", \
	"BLOB : Maximum number of allowed protocols was exceeded\n", \
	"BLOB : Protocols limit exceeded\n", \
	"BLOB : Protocol is linked twice\n", \
	"BLOB : Soft parser is linked twice\n", \
	"BLOB : Parameter offset exceeds the maximum parameters limit\n", \
	"BLOB : Parameter size can't be 0 or greater than 64\n", \
	"BLOB : Parameter offset plus size exceeds the maximum limit\n", \
	"BLOB : Parameters number exceeds the maximum limit\n", \
	"BLOB : Duplicated parameter name\n", \
	"BLOB : Parameters overlapped detected\n", \
	"apply spb : No dpsparser handle.\n", \
	\
	MC_ERROR_MSG_SPB_UNKNOWN, \
	NULL, \
}

int dpsparser_open(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 *token);

int dpsparser_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

int dpsparser_create(struct fsl_mc_io *mc_io, u16 token, u32 cmd_flags,
		     u32 *obj_id);

int dpsparser_destroy(struct fsl_mc_io *mc_io, u16 token, u32 cmd_flags,
		      u32 obj_id);

int dpsparser_apply_spb(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			u64 blob_addr, u16 *error);

int dpsparser_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			      u16 *major_ver, u16 *minor_ver);

#endif /* _FSL_DPSPARSER_H */
