/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Defines APIs and structures that allow software to interact with a
 * TPM2 device
 *
 * Copyright (c) 2020 Linaro
 * Copyright (c) 2018 Bootlin
 *
 * https://trustedcomputinggroup.org/resource/tss-overview-common-structures-specification/
 *
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#ifndef __TPM_V2_H
#define __TPM_V2_H

#include <tpm-common.h>

struct udevice;

#define TPM2_DIGEST_LEN		32

#define TPM2_SHA1_DIGEST_SIZE 20
#define TPM2_SHA256_DIGEST_SIZE	32
#define TPM2_SHA384_DIGEST_SIZE	48
#define TPM2_SHA512_DIGEST_SIZE	64
#define TPM2_SM3_256_DIGEST_SIZE 32

#define TPM2_MAX_PCRS 32
#define TPM2_PCR_SELECT_MAX ((TPM2_MAX_PCRS + 7) / 8)
#define TPM2_MAX_CAP_BUFFER 1024
#define TPM2_MAX_TPM_PROPERTIES ((TPM2_MAX_CAP_BUFFER - sizeof(u32) /* TPM2_CAP */ - \
				 sizeof(u32)) / sizeof(struct tpms_tagged_property))

#define TPM2_HDR_LEN		10

/*
 *  We deviate from this draft of the specification by increasing the value of
 *  TPM2_NUM_PCR_BANKS from 3 to 16 to ensure compatibility with TPM2
 *  implementations that have enabled a larger than typical number of PCR
 *  banks. This larger value for TPM2_NUM_PCR_BANKS is expected to be included
 *  in a future revision of the specification.
 */
#define TPM2_NUM_PCR_BANKS 16

/* Definition of (UINT32) TPM2_CAP Constants */
#define TPM2_CAP_PCRS 0x00000005U
#define TPM2_CAP_TPM_PROPERTIES 0x00000006U

/* Definition of (UINT32) TPM2_PT Constants */
#define TPM2_PT_GROUP			(u32)(0x00000100)
#define TPM2_PT_FIXED			(u32)(TPM2_PT_GROUP * 1)
#define TPM2_PT_MANUFACTURER		(u32)(TPM2_PT_FIXED + 5)
#define TPM2_PT_PCR_COUNT		(u32)(TPM2_PT_FIXED + 18)
#define TPM2_PT_MAX_COMMAND_SIZE	(u32)(TPM2_PT_FIXED + 30)
#define TPM2_PT_MAX_RESPONSE_SIZE	(u32)(TPM2_PT_FIXED + 31)

/*
 * event types, cf.
 * "TCG Server Management Domain Firmware Profile Specification",
 * rev 1.00, 2020-05-01
 */
#define EV_POST_CODE			((u32)0x00000001)
#define EV_NO_ACTION			((u32)0x00000003)
#define EV_SEPARATOR			((u32)0x00000004)
#define EV_ACTION			((u32)0x00000005)
#define EV_TAG				((u32)0x00000006)
#define EV_S_CRTM_CONTENTS		((u32)0x00000007)
#define EV_S_CRTM_VERSION		((u32)0x00000008)
#define EV_CPU_MICROCODE		((u32)0x00000009)
#define EV_PLATFORM_CONFIG_FLAGS	((u32)0x0000000A)
#define EV_TABLE_OF_DEVICES		((u32)0x0000000B)
#define EV_COMPACT_HASH			((u32)0x0000000C)

/*
 * event types, cf.
 * "TCG PC Client Platform Firmware Profile Specification", Family "2.0"
 * Level 00 Version 1.05 Revision 23, May 7, 2021
 */
#define EV_EFI_EVENT_BASE			((u32)0x80000000)
#define EV_EFI_VARIABLE_DRIVER_CONFIG		((u32)0x80000001)
#define EV_EFI_VARIABLE_BOOT			((u32)0x80000002)
#define EV_EFI_BOOT_SERVICES_APPLICATION	((u32)0x80000003)
#define EV_EFI_BOOT_SERVICES_DRIVER		((u32)0x80000004)
#define EV_EFI_RUNTIME_SERVICES_DRIVER		((u32)0x80000005)
#define EV_EFI_GPT_EVENT			((u32)0x80000006)
#define EV_EFI_ACTION				((u32)0x80000007)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB		((u32)0x80000008)
#define EV_EFI_HANDOFF_TABLES			((u32)0x80000009)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB2		((u32)0x8000000A)
#define EV_EFI_HANDOFF_TABLES2			((u32)0x8000000B)
#define EV_EFI_VARIABLE_BOOT2			((u32)0x8000000C)
#define EV_EFI_HCRTM_EVENT			((u32)0x80000010)
#define EV_EFI_VARIABLE_AUTHORITY		((u32)0x800000E0)
#define EV_EFI_SPDM_FIRMWARE_BLOB		((u32)0x800000E1)
#define EV_EFI_SPDM_FIRMWARE_CONFIG		((u32)0x800000E2)

#define EFI_CALLING_EFI_APPLICATION         \
	"Calling EFI Application from Boot Option"
#define EFI_RETURNING_FROM_EFI_APPLICATION  \
	"Returning from EFI Application from Boot Option"
#define EFI_EXIT_BOOT_SERVICES_INVOCATION   \
	"Exit Boot Services Invocation"
#define EFI_EXIT_BOOT_SERVICES_FAILED       \
	"Exit Boot Services Returned with Failure"
#define EFI_EXIT_BOOT_SERVICES_SUCCEEDED    \
	"Exit Boot Services Returned with Success"
#define EFI_DTB_EVENT_STRING \
	"DTB DATA"

/* TPMS_TAGGED_PROPERTY Structure */
struct tpms_tagged_property {
	u32 property;
	u32 value;
} __packed;

/* TPMS_PCR_SELECTION Structure */
struct tpms_pcr_selection {
	u16 hash;
	u8 size_of_select;
	u8 pcr_select[TPM2_PCR_SELECT_MAX];
} __packed;

/* TPML_PCR_SELECTION Structure */
struct tpml_pcr_selection {
	u32 count;
	struct tpms_pcr_selection selection[TPM2_NUM_PCR_BANKS];
} __packed;

/* TPML_TAGGED_TPM_PROPERTY Structure */
struct tpml_tagged_tpm_property {
	u32 count;
	struct tpms_tagged_property tpm_property[TPM2_MAX_TPM_PROPERTIES];
} __packed;

/* TPMU_CAPABILITIES Union */
union tpmu_capabilities {
	/*
	 * Non exhaustive. Only added the structs needed for our
	 * current code
	 */
	struct tpml_pcr_selection assigned_pcr;
	struct tpml_tagged_tpm_property tpm_properties;
} __packed;

/* TPMS_CAPABILITY_DATA Structure */
struct tpms_capability_data {
	u32 capability;
	union tpmu_capabilities data;
} __packed;

/**
 * SHA1 Event Log Entry Format
 *
 * @pcr_index:	PCRIndex event extended to
 * @event_type:	Type of event (see EFI specs)
 * @digest:	Value extended into PCR index
 * @event_size:	Size of event
 * @event:	Event data
 */
struct tcg_pcr_event {
	u32 pcr_index;
	u32 event_type;
	u8 digest[TPM2_SHA1_DIGEST_SIZE];
	u32 event_size;
	u8 event[];
} __packed;

/**
 * Definition of TPMU_HA Union
 */
union tpmu_ha {
	u8 sha1[TPM2_SHA1_DIGEST_SIZE];
	u8 sha256[TPM2_SHA256_DIGEST_SIZE];
	u8 sm3_256[TPM2_SM3_256_DIGEST_SIZE];
	u8 sha384[TPM2_SHA384_DIGEST_SIZE];
	u8 sha512[TPM2_SHA512_DIGEST_SIZE];
} __packed;

/**
 * Definition of TPMT_HA Structure
 *
 * @hash_alg:	Hash algorithm defined in enum tpm2_algorithms
 * @digest:	Digest value for a given algorithm
 */
struct tpmt_ha {
	u16 hash_alg;
	union tpmu_ha digest;
} __packed;

/**
 * Definition of TPML_DIGEST_VALUES Structure
 *
 * @count:	Number of algorithms supported by hardware
 * @digests:	struct for algorithm id and hash value
 */
struct tpml_digest_values {
	u32 count;
	struct tpmt_ha digests[TPM2_NUM_PCR_BANKS];
} __packed;

/**
 * Crypto Agile Log Entry Format
 *
 * @pcr_index:	PCRIndex event extended to
 * @event_type:	Type of event
 * @digests:	List of digestsextended to PCR index
 * @event_size: Size of the event data
 * @event:	Event data
 */
struct tcg_pcr_event2 {
	u32 pcr_index;
	u32 event_type;
	struct tpml_digest_values digests;
	u32 event_size;
	u8 event[];
} __packed;

/**
 *  struct TCG_EfiSpecIdEventAlgorithmSize - hashing algorithm information
 *
 *  @algorithm_id:	algorithm defined in enum tpm2_algorithms
 *  @digest_size:	size of the algorithm
 */
struct tcg_efi_spec_id_event_algorithm_size {
	u16      algorithm_id;
	u16      digest_size;
} __packed;

#define TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03 "Spec ID Event03"
#define TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2 2
#define TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2 0
#define TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_ERRATA_TPM2 2

/**
 * struct TCG_EfiSpecIDEventStruct - content of the event log header
 *
 * @signature:			signature, set to Spec ID Event03
 * @platform_class:		class defined in TCG ACPI Specification
 *				Client  Common Header.
 * @spec_version_minor:		minor version
 * @spec_version_major:		major version
 * @spec_version_errata:	major version
 * @uintn_size:			size of the efi_uintn_t fields used in various
 *				data structures used in this specification.
 *				0x01 indicates u32  and 0x02  indicates u64
 * @number_of_algorithms:	hashing algorithms used in this event log
 * @digest_sizes:		array of number_of_algorithms pairs
 *				1st member defines the algorithm id
 *				2nd member defines the algorithm size
 */
struct tcg_efi_spec_id_event {
	u8 signature[16];
	u32 platform_class;
	u8 spec_version_minor;
	u8 spec_version_major;
	u8 spec_errata;
	u8 uintn_size;
	u32 number_of_algorithms;
	struct tcg_efi_spec_id_event_algorithm_size digest_sizes[];
} __packed;

/**
 * TPM2 Structure Tags for command/response buffers.
 *
 * @TPM2_ST_NO_SESSIONS: the command does not need an authentication.
 * @TPM2_ST_SESSIONS: the command needs an authentication.
 */
enum tpm2_structures {
	TPM2_ST_NO_SESSIONS	= 0x8001,
	TPM2_ST_SESSIONS	= 0x8002,
};

/**
 * TPM2 type of boolean.
 */
enum tpm2_yes_no {
	TPMI_YES		= 1,
	TPMI_NO			= 0,
};

/**
 * TPM2 startup values.
 *
 * @TPM2_SU_CLEAR: reset the internal state.
 * @TPM2_SU_STATE: restore saved state (if any).
 */
enum tpm2_startup_types {
	TPM2_SU_CLEAR		= 0x0000,
	TPM2_SU_STATE		= 0x0001,
};

/**
 * TPM2 permanent handles.
 *
 * @TPM2_RH_OWNER: refers to the 'owner' hierarchy.
 * @TPM2_RS_PW: indicates a password.
 * @TPM2_RH_LOCKOUT: refers to the 'lockout' hierarchy.
 * @TPM2_RH_ENDORSEMENT: refers to the 'endorsement' hierarchy.
 * @TPM2_RH_PLATFORM: refers to the 'platform' hierarchy.
 */
enum tpm2_handles {
	TPM2_RH_OWNER		= 0x40000001,
	TPM2_RS_PW		= 0x40000009,
	TPM2_RH_LOCKOUT		= 0x4000000A,
	TPM2_RH_ENDORSEMENT	= 0x4000000B,
	TPM2_RH_PLATFORM	= 0x4000000C,
};

/**
 * TPM2 command codes used at the beginning of a buffer, gives the command.
 *
 * @TPM2_CC_STARTUP: TPM2_Startup().
 * @TPM2_CC_SELF_TEST: TPM2_SelfTest().
 * @TPM2_CC_CLEAR: TPM2_Clear().
 * @TPM2_CC_CLEARCONTROL: TPM2_ClearControl().
 * @TPM2_CC_HIERCHANGEAUTH: TPM2_HierarchyChangeAuth().
 * @TPM2_CC_PCR_SETAUTHPOL: TPM2_PCR_SetAuthPolicy().
 * @TPM2_CC_DAM_RESET: TPM2_DictionaryAttackLockReset().
 * @TPM2_CC_DAM_PARAMETERS: TPM2_DictionaryAttackParameters().
 * @TPM2_CC_GET_CAPABILITY: TPM2_GetCapibility().
 * @TPM2_CC_GET_RANDOM: TPM2_GetRandom().
 * @TPM2_CC_PCR_READ: TPM2_PCR_Read().
 * @TPM2_CC_PCR_EXTEND: TPM2_PCR_Extend().
 * @TPM2_CC_PCR_SETAUTHVAL: TPM2_PCR_SetAuthValue().
 */
enum tpm2_command_codes {
	TPM2_CC_STARTUP		= 0x0144,
	TPM2_CC_SELF_TEST	= 0x0143,
	TPM2_CC_HIER_CONTROL	= 0x0121,
	TPM2_CC_CLEAR		= 0x0126,
	TPM2_CC_CLEARCONTROL	= 0x0127,
	TPM2_CC_HIERCHANGEAUTH	= 0x0129,
	TPM2_CC_NV_DEFINE_SPACE	= 0x012a,
	TPM2_CC_PCR_SETAUTHPOL	= 0x012C,
	TPM2_CC_NV_WRITE	= 0x0137,
	TPM2_CC_NV_WRITELOCK	= 0x0138,
	TPM2_CC_DAM_RESET	= 0x0139,
	TPM2_CC_DAM_PARAMETERS	= 0x013A,
	TPM2_CC_NV_READ         = 0x014E,
	TPM2_CC_GET_CAPABILITY	= 0x017A,
	TPM2_CC_GET_RANDOM      = 0x017B,
	TPM2_CC_PCR_READ	= 0x017E,
	TPM2_CC_PCR_EXTEND	= 0x0182,
	TPM2_CC_PCR_SETAUTHVAL	= 0x0183,
};

/**
 * TPM2 return codes.
 */
enum tpm2_return_codes {
	TPM2_RC_SUCCESS		= 0x0000,
	TPM2_RC_BAD_TAG		= 0x001E,
	TPM2_RC_FMT1		= 0x0080,
	TPM2_RC_HASH		= TPM2_RC_FMT1 + 0x0003,
	TPM2_RC_VALUE		= TPM2_RC_FMT1 + 0x0004,
	TPM2_RC_SIZE		= TPM2_RC_FMT1 + 0x0015,
	TPM2_RC_BAD_AUTH	= TPM2_RC_FMT1 + 0x0022,
	TPM2_RC_HANDLE		= TPM2_RC_FMT1 + 0x000B,
	TPM2_RC_VER1		= 0x0100,
	TPM2_RC_INITIALIZE	= TPM2_RC_VER1 + 0x0000,
	TPM2_RC_FAILURE		= TPM2_RC_VER1 + 0x0001,
	TPM2_RC_DISABLED	= TPM2_RC_VER1 + 0x0020,
	TPM2_RC_AUTH_MISSING	= TPM2_RC_VER1 + 0x0025,
	TPM2_RC_COMMAND_CODE	= TPM2_RC_VER1 + 0x0043,
	TPM2_RC_AUTHSIZE	= TPM2_RC_VER1 + 0x0044,
	TPM2_RC_AUTH_CONTEXT	= TPM2_RC_VER1 + 0x0045,
	TPM2_RC_NV_DEFINED	= TPM2_RC_VER1 + 0x004c,
	TPM2_RC_NEEDS_TEST	= TPM2_RC_VER1 + 0x0053,
	TPM2_RC_WARN		= 0x0900,
	TPM2_RC_TESTING		= TPM2_RC_WARN + 0x000A,
	TPM2_RC_REFERENCE_H0	= TPM2_RC_WARN + 0x0010,
	TPM2_RC_LOCKOUT		= TPM2_RC_WARN + 0x0021,
};

/**
 * TPM2 algorithms.
 */
enum tpm2_algorithms {
	TPM2_ALG_SHA1		= 0x04,
	TPM2_ALG_XOR		= 0x0A,
	TPM2_ALG_SHA256		= 0x0B,
	TPM2_ALG_SHA384		= 0x0C,
	TPM2_ALG_SHA512		= 0x0D,
	TPM2_ALG_NULL		= 0x10,
	TPM2_ALG_SM3_256	= 0x12,
};

extern const enum tpm2_algorithms tpm2_supported_algorithms[4];

static inline u16 tpm2_algorithm_to_len(enum tpm2_algorithms a)
{
	switch (a) {
	case TPM2_ALG_SHA1:
		return TPM2_SHA1_DIGEST_SIZE;
	case TPM2_ALG_SHA256:
		return TPM2_SHA256_DIGEST_SIZE;
	case TPM2_ALG_SHA384:
		return TPM2_SHA384_DIGEST_SIZE;
	case TPM2_ALG_SHA512:
		return TPM2_SHA512_DIGEST_SIZE;
	default:
		return 0;
	}
}

#define tpm2_algorithm_to_mask(a)	(1 << (a))

/* NV index attributes */
enum tpm_index_attrs {
	TPMA_NV_PPWRITE		= 1UL << 0,
	TPMA_NV_OWNERWRITE	= 1UL << 1,
	TPMA_NV_AUTHWRITE	= 1UL << 2,
	TPMA_NV_POLICYWRITE	= 1UL << 3,
	TPMA_NV_COUNTER		= 1UL << 4,
	TPMA_NV_BITS		= 1UL << 5,
	TPMA_NV_EXTEND		= 1UL << 6,
	TPMA_NV_POLICY_DELETE	= 1UL << 10,
	TPMA_NV_WRITELOCKED	= 1UL << 11,
	TPMA_NV_WRITEALL	= 1UL << 12,
	TPMA_NV_WRITEDEFINE	= 1UL << 13,
	TPMA_NV_WRITE_STCLEAR	= 1UL << 14,
	TPMA_NV_GLOBALLOCK	= 1UL << 15,
	TPMA_NV_PPREAD		= 1UL << 16,
	TPMA_NV_OWNERREAD	= 1UL << 17,
	TPMA_NV_AUTHREAD	= 1UL << 18,
	TPMA_NV_POLICYREAD	= 1UL << 19,
	TPMA_NV_NO_DA		= 1UL << 25,
	TPMA_NV_ORDERLY		= 1UL << 26,
	TPMA_NV_CLEAR_STCLEAR	= 1UL << 27,
	TPMA_NV_READLOCKED	= 1UL << 28,
	TPMA_NV_WRITTEN		= 1UL << 29,
	TPMA_NV_PLATFORMCREATE	= 1UL << 30,
	TPMA_NV_READ_STCLEAR	= 1UL << 31,

	TPMA_NV_MASK_READ	= TPMA_NV_PPREAD | TPMA_NV_OWNERREAD |
				TPMA_NV_AUTHREAD | TPMA_NV_POLICYREAD,
	TPMA_NV_MASK_WRITE	= TPMA_NV_PPWRITE | TPMA_NV_OWNERWRITE |
					TPMA_NV_AUTHWRITE | TPMA_NV_POLICYWRITE,
};

enum {
	TPM_ACCESS_VALID		= 1 << 7,
	TPM_ACCESS_ACTIVE_LOCALITY	= 1 << 5,
	TPM_ACCESS_REQUEST_PENDING	= 1 << 2,
	TPM_ACCESS_REQUEST_USE		= 1 << 1,
	TPM_ACCESS_ESTABLISHMENT	= 1 << 0,
};

enum {
	TPM_STS_FAMILY_SHIFT		= 26,
	TPM_STS_FAMILY_MASK		= 0x3 << TPM_STS_FAMILY_SHIFT,
	TPM_STS_FAMILY_TPM2		= 1 << TPM_STS_FAMILY_SHIFT,
	TPM_STS_RESE_TESTABLISMENT_BIT	= 1 << 25,
	TPM_STS_COMMAND_CANCEL		= 1 << 24,
	TPM_STS_BURST_COUNT_SHIFT	= 8,
	TPM_STS_BURST_COUNT_MASK	= 0xffff << TPM_STS_BURST_COUNT_SHIFT,
	TPM_STS_VALID			= 1 << 7,
	TPM_STS_COMMAND_READY		= 1 << 6,
	TPM_STS_GO			= 1 << 5,
	TPM_STS_DATA_AVAIL		= 1 << 4,
	TPM_STS_DATA_EXPECT		= 1 << 3,
	TPM_STS_SELF_TEST_DONE		= 1 << 2,
	TPM_STS_RESPONSE_RETRY		= 1 << 1,
	TPM_STS_READ_ZERO               = 0x23
};

enum {
	TPM_CMD_COUNT_OFFSET	= 2,
	TPM_CMD_ORDINAL_OFFSET	= 6,
	TPM_MAX_BUF_SIZE	= 1260,
};

enum {
	/* Secure storage for firmware settings */
	TPM_HT_PCR = 0,
	TPM_HT_NV_INDEX,
	TPM_HT_HMAC_SESSION,
	TPM_HT_POLICY_SESSION,

	HR_SHIFT		= 24,
	HR_PCR			= TPM_HT_PCR << HR_SHIFT,
	HR_HMAC_SESSION		= TPM_HT_HMAC_SESSION << HR_SHIFT,
	HR_POLICY_SESSION	= TPM_HT_POLICY_SESSION << HR_SHIFT,
	HR_NV_INDEX		= TPM_HT_NV_INDEX << HR_SHIFT,
};

/**
 * struct tcg2_event_log - Container for managing the platform event log
 *
 * @log:		Address of the log
 * @log_position:	Current entry position
 * @log_size:		Log space available
 * @found:		Boolean indicating if an existing log was discovered
 */
struct tcg2_event_log {
	u8 *log;
	u32 log_position;
	u32 log_size;
	bool found;
};

/**
 * Create a list of digests of the supported PCR banks for a given input data
 *
 * @dev		TPM device
 * @input	Data
 * @length	Length of the data to calculate the digest
 * @digest_list	List of digests to fill in
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_create_digest(struct udevice *dev, const u8 *input, u32 length,
		       struct tpml_digest_values *digest_list);

/**
 * Get the event size of the specified digests
 *
 * @digest_list	List of digests for the event
 *
 * Return: Size in bytes of the event
 */
u32 tcg2_event_get_size(struct tpml_digest_values *digest_list);

/**
 * tcg2_get_active_pcr_banks
 *
 * @dev			TPM device
 * @active_pcr_banks	Bitmask of PCR algorithms supported
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_get_active_pcr_banks(struct udevice *dev, u32 *active_pcr_banks);

/**
 * tcg2_log_append - Append an event to an event log
 *
 * @pcr_index	Index of the PCR
 * @event_type	Type of event
 * @digest_list List of digests to add
 * @size	Size of event
 * @event	Event data
 * @log		Log buffer to append the event to
 */
void tcg2_log_append(u32 pcr_index, u32 event_type,
		     struct tpml_digest_values *digest_list, u32 size,
		     const u8 *event, u8 *log);

/**
 * Extend the PCR with specified digests
 *
 * @dev		TPM device
 * @pcr_index	Index of the PCR
 * @digest_list	List of digests to extend
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_pcr_extend(struct udevice *dev, u32 pcr_index,
		    struct tpml_digest_values *digest_list);

/**
 * Read the PCR into a list of digests
 *
 * @dev		TPM device
 * @pcr_index	Index of the PCR
 * @digest_list	List of digests to extend
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_pcr_read(struct udevice *dev, u32 pcr_index,
		  struct tpml_digest_values *digest_list);

/**
 * Measure data into the TPM PCRs and the platform event log.
 *
 * @dev		TPM device
 * @log		Platform event log
 * @pcr_index	Index of the PCR
 * @size	Size of the data or 0 for event only
 * @data	Pointer to the data or NULL for event only
 * @event_type	Event log type
 * @event_size	Size of the event
 * @event	Pointer to the event
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_measure_data(struct udevice *dev, struct tcg2_event_log *elog,
		      u32 pcr_index, u32 size, const u8 *data, u32 event_type,
		      u32 event_size, const u8 *event);

#define tcg2_measure_event(dev, elog, pcr_index, event_type, size, event) \
	tcg2_measure_data(dev, elog, pcr_index, 0, NULL, event_type, size, \
			  event)

/**
 * Prepare the event log buffer. This function tries to discover an existing
 * event log in memory from a previous bootloader stage. If such a log exists
 * and the PCRs are not extended, the log is "replayed" to extend the PCRs.
 * If no log is discovered, create the log header.
 *
 * @dev			TPM device
 * @elog		Platform event log. The log pointer and log_size
 *			members must be initialized to either 0 or to a valid
 *			memory region, in which case any existing log
 *			discovered will be copied to the specified memory
 *			region.
 * @ignore_existing_log	Boolean to indicate whether or not to ignore an
 *			existing platform log in memory
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_log_prepare_buffer(struct udevice *dev, struct tcg2_event_log *elog,
			    bool ignore_existing_log);

/**
 * Begin measurements.
 *
 * @dev			TPM device
 * @elog		Platform event log. The log pointer and log_size
 *			members must be initialized to either 0 or to a valid
 *			memory region, in which case any existing log
 *			discovered will be copied to the specified memory
 *			region.
 * @ignore_existing_log Boolean to indicate whether or not to ignore an
 *			existing platform log in memory
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_measurement_init(struct udevice **dev, struct tcg2_event_log *elog,
			  bool ignore_existing_log);

/**
 * Stop measurements and record separator events.
 *
 * @dev		TPM device
 * @elog	Platform event log
 * @error	Boolean to indicate whether an error ocurred or not
 */
void tcg2_measurement_term(struct udevice *dev, struct tcg2_event_log *elog,
			   bool error);

/**
 * Get the platform event log address and size.
 *
 * @dev		TPM device
 * @addr	Address of the log
 * @size	Size of the log
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_platform_get_log(struct udevice *dev, void **addr, u32 *size);

/**
 * Get the first TPM2 device found.
 *
 * @dev		TPM device
 *
 * Return: zero on success, negative errno otherwise
 */
int tcg2_platform_get_tpm2(struct udevice **dev);

/**
 * Platform-specific function for handling TPM startup errors
 *
 * @dev		TPM device
 * @rc		The TPM response code
 */
void tcg2_platform_startup_error(struct udevice *dev, int rc);

/**
 * Issue a TPM2_Startup command.
 *
 * @dev		TPM device
 * @mode	TPM startup mode
 *
 * Return: code of the operation
 */
u32 tpm2_startup(struct udevice *dev, enum tpm2_startup_types mode);

/**
 * Issue a TPM2_SelfTest command.
 *
 * @dev		TPM device
 * @full_test	Asking to perform all tests or only the untested ones
 *
 * Return: code of the operation
 */
u32 tpm2_self_test(struct udevice *dev, enum tpm2_yes_no full_test);

/**
 * Issue a TPM2_Clear command.
 *
 * @dev		TPM device
 * @handle	Handle
 * @pw		Password
 * @pw_sz	Length of the password
 *
 * Return: code of the operation
 */
u32 tpm2_clear(struct udevice *dev, u32 handle, const char *pw,
	       const ssize_t pw_sz);

/**
 * Issue a TPM_NV_DefineSpace command
 *
 * This allows a space to be defined with given attributes and policy
 *
 * @dev			TPM device
 * @space_index		index of the area
 * @space_size		size of area in bytes
 * @nv_attributes	TPM_NV_ATTRIBUTES of the area
 * @nv_policy		policy to use
 * @nv_policy_size	size of the policy
 * Return: return code of the operation
 */
u32 tpm2_nv_define_space(struct udevice *dev, u32 space_index,
			 size_t space_size, u32 nv_attributes,
			 const u8 *nv_policy, size_t nv_policy_size);

/**
 * Issue a TPM2_PCR_Extend command.
 *
 * @dev		TPM device
 * @index	Index of the PCR
 * @algorithm	Algorithm used, defined in 'enum tpm2_algorithms'
 * @digest	Value representing the event to be recorded
 * @digest_len  len of the hash
 *
 * Return: code of the operation
 */
u32 tpm2_pcr_extend(struct udevice *dev, u32 index, u32 algorithm,
		    const u8 *digest, u32 digest_len);

/**
 * Read data from the secure storage
 *
 * @dev		TPM device
 * @index	Index of data to read
 * @data	Place to put data
 * @count	Number of bytes of data
 * Return: code of the operation
 */
u32 tpm2_nv_read_value(struct udevice *dev, u32 index, void *data, u32 count);

/**
 * Write data to the secure storage
 *
 * @dev		TPM device
 * @index	Index of data to write
 * @data	Data to write
 * @count	Number of bytes of data
 * Return: code of the operation
 */
u32 tpm2_nv_write_value(struct udevice *dev, u32 index, const void *data,
			u32 count);

/**
 * Issue a TPM2_PCR_Read command.
 *
 * @dev		TPM device
 * @idx		Index of the PCR
 * @idx_min_sz	Minimum size in bytes of the pcrSelect array
 * @algorithm	Algorithm used, defined in 'enum tpm2_algorithms'
 * @data	Output buffer for contents of the named PCR
 * @digest_len  len of the data
 * @updates	Optional out parameter: number of updates for this PCR
 *
 * Return: code of the operation
 */
u32 tpm2_pcr_read(struct udevice *dev, u32 idx, unsigned int idx_min_sz,
		  u16 algorithm, void *data, u32 digest_len,
		  unsigned int *updates);

/**
 * Issue a TPM2_GetCapability command.  This implementation is limited
 * to query property index that is 4-byte wide.
 *
 * @dev		TPM device
 * @capability	Partition of capabilities
 * @property	Further definition of capability, limited to be 4 bytes wide
 * @buf		Output buffer for capability information
 * @prop_count	Size of output buffer
 *
 * Return: code of the operation
 */
u32 tpm2_get_capability(struct udevice *dev, u32 capability, u32 property,
			void *buf, size_t prop_count);

/**
 * tpm2_get_pcr_info() - get the supported, active PCRs and number of banks
 *
 * @dev:		TPM device
 * @supported_pcr:	bitmask with the algorithms supported
 * @active_pcr:		bitmask with the active algorithms
 * @pcr_banks:		number of PCR banks
 *
 * @return 0 on success, code of operation or negative errno on failure
 */
int tpm2_get_pcr_info(struct udevice *dev, u32 *supported_pcr, u32 *active_pcr,
		      u32 *pcr_banks);

/**
 * Issue a TPM2_DictionaryAttackLockReset command.
 *
 * @dev		TPM device
 * @pw		Password
 * @pw_sz	Length of the password
 *
 * Return: code of the operation
 */
u32 tpm2_dam_reset(struct udevice *dev, const char *pw, const ssize_t pw_sz);

/**
 * Issue a TPM2_DictionaryAttackParameters command.
 *
 * @dev		TPM device
 * @pw		Password
 * @pw_sz	Length of the password
 * @max_tries	Count of authorizations before lockout
 * @recovery_time Time before decrementation of the failure count
 * @lockout_recovery Time to wait after a lockout
 *
 * Return: code of the operation
 */
u32 tpm2_dam_parameters(struct udevice *dev, const char *pw,
			const ssize_t pw_sz, unsigned int max_tries,
			unsigned int recovery_time,
			unsigned int lockout_recovery);

/**
 * Issue a TPM2_HierarchyChangeAuth command.
 *
 * @dev		TPM device
 * @handle	Handle
 * @newpw	New password
 * @newpw_sz	Length of the new password
 * @oldpw	Old password
 * @oldpw_sz	Length of the old password
 *
 * Return: code of the operation
 */
int tpm2_change_auth(struct udevice *dev, u32 handle, const char *newpw,
		     const ssize_t newpw_sz, const char *oldpw,
		     const ssize_t oldpw_sz);

/**
 * Issue a TPM_PCR_SetAuthPolicy command.
 *
 * @dev		TPM device
 * @pw		Platform password
 * @pw_sz	Length of the password
 * @index	Index of the PCR
 * @digest	New key to access the PCR
 *
 * Return: code of the operation
 */
u32 tpm2_pcr_setauthpolicy(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, u32 index, const char *key);

/**
 * Issue a TPM_PCR_SetAuthValue command.
 *
 * @dev		TPM device
 * @pw		Platform password
 * @pw_sz	Length of the password
 * @index	Index of the PCR
 * @digest	New key to access the PCR
 * @key_sz	Length of the new key
 *
 * Return: code of the operation
 */
u32 tpm2_pcr_setauthvalue(struct udevice *dev, const char *pw,
			  const ssize_t pw_sz, u32 index, const char *key,
			  const ssize_t key_sz);

/**
 * Issue a TPM2_GetRandom command.
 *
 * @dev		TPM device
 * @param data		output buffer for the random bytes
 * @param count		size of output buffer
 *
 * Return: return code of the operation
 */
u32 tpm2_get_random(struct udevice *dev, void *data, u32 count);

/**
 * Lock data in the TPM
 *
 * Once locked the data cannot be written until after a reboot
 *
 * @dev		TPM device
 * @index	Index of data to lock
 * Return: code of the operation
 */
u32 tpm2_write_lock(struct udevice *dev, u32 index);

/**
 * Disable access to any platform data
 *
 * This can be called to close off access to the firmware data in the data,
 * before calling the kernel.
 *
 * @dev		TPM device
 * Return: code of the operation
 */
u32 tpm2_disable_platform_hierarchy(struct udevice *dev);

/**
 * submit user specified data to the TPM and get response
 *
 * @dev		TPM device
 * @sendbuf:	Buffer of the data to send
 * @recvbuf:	Buffer to save the response to
 * @recv_size:	Pointer to the size of the response buffer
 *
 * Return: code of the operation
 */
u32 tpm2_submit_command(struct udevice *dev, const u8 *sendbuf,
			u8 *recvbuf, size_t *recv_size);

/**
 * tpm_cr50_report_state() - Report the Cr50 internal state
 *
 * @dev:	TPM device
 * @vendor_cmd:	Vendor command number to send
 * @vendor_subcmd: Vendor sub-command number to send
 * @recvbuf:	Buffer to save the response to
 * @recv_size:	Pointer to the size of the response buffer
 * Return: result of the operation
 */
u32 tpm2_report_state(struct udevice *dev, uint vendor_cmd, uint vendor_subcmd,
		      u8 *recvbuf, size_t *recv_size);

/**
 * tpm2_enable_nvcommits() - Tell TPM to commit NV data immediately
 *
 * For Chromium OS verified boot, we may reboot or reset at different times,
 * possibly leaving non-volatile data unwritten by the TPM.
 *
 * This vendor command is used to indicate that non-volatile data should be
 * written to its store immediately.
 *
 * @dev		TPM device
 * @vendor_cmd:	Vendor command number to send
 * @vendor_subcmd: Vendor sub-command number to send
 * Return: result of the operation
 */
u32 tpm2_enable_nvcommits(struct udevice *dev, uint vendor_cmd,
			  uint vendor_subcmd);

/**
 * tpm2_auto_start() - start up the TPM and perform selftests.
 *                     If a testable function has not been tested and is
 *                     requested the TPM2  will return TPM_RC_NEEDS_TEST.
 *
 * @param dev		TPM device
 * Return: TPM2_RC_TESTING, if TPM2 self-test is in progress.
 *         TPM2_RC_SUCCESS, if testing of all functions is complete without
 *         functional failures.
 *         TPM2_RC_FAILURE, if any test failed.
 *         TPM2_RC_INITIALIZE, if the TPM has not gone through the Startup
 *         sequence

 */
u32 tpm2_auto_start(struct udevice *dev);

#endif /* __TPM_V2_H */
