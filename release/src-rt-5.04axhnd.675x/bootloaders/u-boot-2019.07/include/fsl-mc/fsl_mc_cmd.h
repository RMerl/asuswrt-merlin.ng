/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */
#ifndef __FSL_MC_CMD_H
#define __FSL_MC_CMD_H

#define MC_CMD_NUM_OF_PARAMS	7

#define MAKE_UMASK64(_width) \
	((uint64_t)((_width) < 64 ? ((uint64_t)1 << (_width)) - 1 : -1))

static inline uint64_t mc_enc(int lsoffset, int width, uint64_t val)
{
	return (uint64_t)(((uint64_t)val & MAKE_UMASK64(width)) << lsoffset);
}
static inline uint64_t mc_dec(uint64_t val, int lsoffset, int width)
{
	return (uint64_t)((val >> lsoffset) & MAKE_UMASK64(width));
}

struct mc_command {
	uint64_t header;
	uint64_t params[MC_CMD_NUM_OF_PARAMS];
};

struct mc_rsp_create {
	__le32 object_id;
};

struct mc_rsp_api_ver {
	__le16 major_ver;
	__le16 minor_ver;
};

enum mc_cmd_status {
	MC_CMD_STATUS_OK = 0x0, /*!< Completed successfully */
	MC_CMD_STATUS_READY = 0x1, /*!< Ready to be processed */
	MC_CMD_STATUS_AUTH_ERR = 0x3, /*!< Authentication error */
	MC_CMD_STATUS_NO_PRIVILEGE = 0x4, /*!< No privilege */
	MC_CMD_STATUS_DMA_ERR = 0x5, /*!< DMA or I/O error */
	MC_CMD_STATUS_CONFIG_ERR = 0x6, /*!< Configuration error */
	MC_CMD_STATUS_TIMEOUT = 0x7, /*!< Operation timed out */
	MC_CMD_STATUS_NO_RESOURCE = 0x8, /*!< No resources */
	MC_CMD_STATUS_NO_MEMORY = 0x9, /*!< No memory available */
	MC_CMD_STATUS_BUSY = 0xA, /*!< Device is busy */
	MC_CMD_STATUS_UNSUPPORTED_OP = 0xB, /*!< Unsupported operation */
	MC_CMD_STATUS_INVALID_STATE = 0xC /*!< Invalid state */
};

/*
 * MC command flags
 */

/* High priority flag */
#define MC_CMD_FLAG_PRI		0x00008000
/* No flags */
#define MC_CMD_NO_FLAGS		0x00000000
/* Command completion flag */
#define MC_CMD_FLAG_INTR_DIS	0x01000000


#define MC_CMD_HDR_CMDID_O	48	/* Command ID field offset */
#define MC_CMD_HDR_CMDID_S	16	/* Command ID field size */
#define MC_CMD_HDR_STATUS_O	16	/* Status field offset */
#define MC_CMD_HDR_TOKEN_O	32	/* Token field offset */
#define MC_CMD_HDR_TOKEN_S	16	/* Token field size */
#define MC_CMD_HDR_STATUS_S	8	/* Status field size*/
#define MC_CMD_HDR_FLAGS_O	0	/* Flags field offset */
#define MC_CMD_HDR_FLAGS_S	32	/* Flags field size*/
#define MC_CMD_HDR_FLAGS_MASK	0x0000FFFF /* Command flags mask */

#define MC_CMD_HDR_READ_STATUS(_hdr) \
	((enum mc_cmd_status)mc_dec((_hdr), \
		MC_CMD_HDR_STATUS_O, MC_CMD_HDR_STATUS_S))

#define MC_CMD_HDR_READ_TOKEN(_hdr) \
	((uint16_t)mc_dec((_hdr), MC_CMD_HDR_TOKEN_O, MC_CMD_HDR_TOKEN_S))

#define MC_PREP_OP(_ext, _param, _offset, _width, _type, _arg) \
	((_ext)[_param] |= cpu_to_le64(mc_enc((_offset), (_width), _arg)))

#define MC_EXT_OP(_ext, _param, _offset, _width, _type, _arg) \
	(_arg = (_type)mc_dec(cpu_to_le64(_ext[_param]), (_offset), (_width)))

#define MC_CMD_OP(_cmd, _param, _offset, _width, _type, _arg) \
	((_cmd).params[_param] |= mc_enc((_offset), (_width), _arg))

#define MC_RSP_OP(_cmd, _param, _offset, _width, _type, _arg) \
	(_arg = (_type)mc_dec(_cmd.params[_param], (_offset), (_width)))

/*                cmd, param, offset, width, type, arg_name */
#define MC_CMD_READ_OBJ_ID(cmd, obj_id) \
	MC_RSP_OP(cmd, 0, 0,  32,  uint32_t,	    obj_id)

/* cmd, param, offset, width, type, arg_name */
#define CMD_DESTROY_SET_OBJ_ID_PARAM0(cmd, object_id) \
	MC_CMD_OP(cmd, 0, 0,  32,  uint32_t,  object_id)

static inline uint64_t mc_encode_cmd_header(uint16_t cmd_id,
					    uint32_t cmd_flags,
					    uint16_t token)
{
	uint64_t hdr = 0;

	hdr = mc_enc(MC_CMD_HDR_CMDID_O, MC_CMD_HDR_CMDID_S, cmd_id);
	hdr |= mc_enc(MC_CMD_HDR_FLAGS_O, MC_CMD_HDR_FLAGS_S,
		       (cmd_flags & MC_CMD_HDR_FLAGS_MASK));
	hdr |= mc_enc(MC_CMD_HDR_TOKEN_O, MC_CMD_HDR_TOKEN_S, token);
	hdr |= mc_enc(MC_CMD_HDR_STATUS_O, MC_CMD_HDR_STATUS_S,
		       MC_CMD_STATUS_READY);

	return hdr;
}

/**
 * mc_write_command - writes a command to a Management Complex (MC) portal
 *
 * @portal: pointer to an MC portal
 * @cmd: pointer to a filled command
 */
static inline void mc_write_command(struct mc_command __iomem *portal,
				    struct mc_command *cmd)
{
	int i;

	/* copy command parameters into the portal */
	for (i = 0; i < MC_CMD_NUM_OF_PARAMS; i++)
		writeq(cmd->params[i], &portal->params[i]);

	/* submit the command by writing the header */
	writeq(cmd->header, &portal->header);
}

/**
 * mc_read_response - reads the response for the last MC command from a
 * Management Complex (MC) portal
 *
 * @portal: pointer to an MC portal
 * @resp: pointer to command response buffer
 *
 * Returns MC_CMD_STATUS_OK on Success; Error code otherwise.
 */
static inline enum mc_cmd_status mc_read_response(
					struct mc_command __iomem *portal,
					struct mc_command *resp)
{
	int i;
	enum mc_cmd_status status;

	/* Copy command response header from MC portal: */
	resp->header = readq(&portal->header);
	status = MC_CMD_HDR_READ_STATUS(resp->header);
	if (status != MC_CMD_STATUS_OK)
		return status;

	/* Copy command response data from MC portal: */
	for (i = 0; i < MC_CMD_NUM_OF_PARAMS; i++)
		resp->params[i] = readq(&portal->params[i]);

	return status;
}

/**
 * mc_read_version - read version of the given cmd
 *
 * @cmd: pointer to a filled command
 * @major_version: major version value for the given cmd
 * @minor_version: minor version value for the given cmd
 */
static inline void mc_cmd_read_api_version(struct mc_command *cmd,
					   u16 *major_ver,
					   u16 *minor_ver)
{
	struct mc_rsp_api_ver *rsp_params;

	rsp_params = (struct mc_rsp_api_ver *)cmd->params;
	*major_ver = le16_to_cpu(rsp_params->major_ver);
	*minor_ver = le16_to_cpu(rsp_params->minor_ver);
}

#endif /* __FSL_MC_CMD_H */
