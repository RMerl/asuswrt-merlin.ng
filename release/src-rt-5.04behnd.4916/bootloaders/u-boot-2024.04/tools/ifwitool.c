// SPDX-License-Identifier: GPL-2.0
/*
 * ifwitool, CLI utility for Integrated Firmware Image (IFWI) manipulation
 *
 * This is taken from the Coreboot project
 */

#include <assert.h>
#include <stdbool.h>
#include <getopt.h>
#include "imagetool.h"
#include "os_support.h"

#ifndef __packed
#define __packed		__attribute__((packed))
#endif
#define KiB			1024

/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void)&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void)(&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

static int verbose = 1;

/* Buffer and file I/O */
struct buffer {
	char *name;
	char *data;
	size_t offset;
	size_t size;
};

#define ERROR(...) { fprintf(stderr, "E: " __VA_ARGS__); }
#define INFO(...) { if (verbose > 0) fprintf(stderr, "INFO: " __VA_ARGS__); }
#define DEBUG(...) { if (verbose > 1) fprintf(stderr, "DEBUG: " __VA_ARGS__); }

/*
 * BPDT is Boot Partition Descriptor Table. It is located at the start of a
 * logical boot partition(LBP). It stores information about the critical
 * sub-partitions present within the LBP.
 *
 * S-BPDT is Secondary Boot Partition Descriptor Table. It is located after the
 * critical sub-partitions and contains information about the non-critical
 * sub-partitions present within the LBP.
 *
 * Both tables are identified by BPDT_SIGNATURE stored at the start of the
 * table.
 */
#define BPDT_SIGNATURE				(0x000055AA)

/* Parameters passed in by caller */
static struct param {
	const char *file_name;
	const char *subpart_name;
	const char *image_name;
	bool dir_ops;
	const char *dentry_name;
} param;

struct bpdt_header {
	/*
	 * This is used to identify start of BPDT. It should always be
	 * BPDT_SIGNATURE.
	 */
	uint32_t signature;
	/* Count of BPDT entries present */
	uint16_t descriptor_count;
	/* Version - Currently supported = 1 */
	uint16_t bpdt_version;
	/* Unused - Should be 0 */
	uint32_t xor_redundant_block;
	/* Version of IFWI build */
	uint32_t ifwi_version;
	/* Version of FIT tool used to create IFWI */
	uint64_t fit_tool_version;
} __packed;
#define BPDT_HEADER_SIZE			(sizeof(struct bpdt_header))

struct bpdt_entry {
	/* Type of sub-partition */
	uint16_t type;
	/* Attributes of sub-partition */
	uint16_t flags;
	/* Offset of sub-partition from beginning of LBP */
	uint32_t offset;
	/* Size in bytes of sub-partition */
	uint32_t size;
} __packed;
#define BPDT_ENTRY_SIZE			(sizeof(struct bpdt_entry))

struct bpdt {
	struct bpdt_header h;
	/* In practice, this could be an array of 0 to n entries */
	struct bpdt_entry e[0];
} __packed;

static inline size_t get_bpdt_size(struct bpdt_header *h)
{
	return (sizeof(*h) + BPDT_ENTRY_SIZE * h->descriptor_count);
}

/* Minimum size in bytes allocated to BPDT in IFWI */
#define BPDT_MIN_SIZE			((size_t)512)

/* Header to define directory header for sub-partition */
struct subpart_dir_header {
	/* Should be SUBPART_DIR_MARKER */
	uint32_t marker;
	/* Number of directory entries in the sub-partition */
	uint32_t num_entries;
	/* Currenty supported - 1 */
	uint8_t header_version;
	/* Currenty supported - 1 */
	uint8_t entry_version;
	/* Length of directory header in bytes */
	uint8_t header_length;
	/*
	 * 2s complement of 8-bit sum from first byte of header to last byte of
	 * last directory entry.
	 */
	uint8_t checksum;
	/* ASCII short name of sub-partition */
	uint8_t name[4];
} __packed;
#define SUBPART_DIR_HEADER_SIZE			\
					(sizeof(struct subpart_dir_header))
#define SUBPART_DIR_MARKER				0x44504324
#define SUBPART_DIR_HEADER_VERSION_SUPPORTED	1
#define SUBPART_DIR_ENTRY_VERSION_SUPPORTED	1

/* Structure for each directory entry for sub-partition */
struct subpart_dir_entry {
	/* Name of directory entry - Not guaranteed to be NULL-terminated */
	uint8_t name[12];
	/* Offset of entry from beginning of sub-partition */
	uint32_t offset;
	/* Length in bytes of sub-directory entry */
	uint32_t length;
	/* Must be zero */
	uint32_t rsvd;
} __packed;
#define SUBPART_DIR_ENTRY_SIZE			\
					(sizeof(struct subpart_dir_entry))

struct subpart_dir {
	struct subpart_dir_header h;
	/* In practice, this could be an array of 0 to n entries */
	struct subpart_dir_entry e[0];
} __packed;

static inline size_t subpart_dir_size(struct subpart_dir_header *h)
{
	return (sizeof(*h) + SUBPART_DIR_ENTRY_SIZE * h->num_entries);
}

struct manifest_header {
	uint32_t header_type;
	uint32_t header_length;
	uint32_t header_version;
	uint32_t flags;
	uint32_t vendor;
	uint32_t date;
	uint32_t size;
	uint32_t id;
	uint32_t rsvd;
	uint64_t version;
	uint32_t svn;
	uint64_t rsvd1;
	uint8_t rsvd2[64];
	uint32_t modulus_size;
	uint32_t exponent_size;
	uint8_t public_key[256];
	uint32_t exponent;
	uint8_t signature[256];
} __packed;

#define DWORD_SIZE				4
#define MANIFEST_HDR_SIZE			(sizeof(struct manifest_header))
#define MANIFEST_ID_MAGIC			(0x324e4d24)

struct module {
	uint8_t name[12];
	uint8_t type;
	uint8_t hash_alg;
	uint16_t hash_size;
	uint32_t metadata_size;
	uint8_t metadata_hash[32];
} __packed;

#define MODULE_SIZE				(sizeof(struct module))

struct signed_pkg_info_ext {
	uint32_t ext_type;
	uint32_t ext_length;
	uint8_t name[4];
	uint32_t vcn;
	uint8_t bitmap[16];
	uint32_t svn;
	uint8_t rsvd[16];
} __packed;

#define SIGNED_PKG_INFO_EXT_TYPE		0x15
#define SIGNED_PKG_INFO_EXT_SIZE		\
	(sizeof(struct signed_pkg_info_ext))

/*
 * Attributes for various IFWI sub-partitions.
 * LIES_WITHIN_BPDT_4K = Sub-Partition should lie within the same 4K block as
 * BPDT.
 * NON_CRITICAL_SUBPART = Sub-Partition entry should be present in S-BPDT.
 * CONTAINS_DIR = Sub-Partition contains directory.
 * AUTO_GENERATED = Sub-Partition is generated by the tool.
 * MANDATORY_BPDT_ENTRY = Even if sub-partition is deleted, BPDT should contain
 * an entry for it with size 0 and offset 0.
 */
enum subpart_attributes {
	LIES_WITHIN_BPDT_4K = (1 << 0),
	NON_CRITICAL_SUBPART = (1 << 1),
	CONTAINS_DIR = (1 << 2),
	AUTO_GENERATED = (1 << 3),
	MANDATORY_BPDT_ENTRY = (1 << 4),
};

/* Type value for various IFWI sub-partitions */
enum bpdt_entry_type {
	SMIP_TYPE		= 0,
	CSE_RBE_TYPE		= 1,
	CSE_BUP_TYPE		= 2,
	UCODE_TYPE		= 3,
	IBB_TYPE		= 4,
	S_BPDT_TYPE		= 5,
	OBB_TYPE		= 6,
	CSE_MAIN_TYPE		= 7,
	ISH_TYPE		= 8,
	CSE_IDLM_TYPE		= 9,
	IFP_OVERRIDE_TYPE	= 10,
	DEBUG_TOKENS_TYPE	= 11,
	UFS_PHY_TYPE		= 12,
	UFS_GPP_TYPE		= 13,
	PMC_TYPE		= 14,
	IUNIT_TYPE		= 15,
	NVM_CONFIG_TYPE	= 16,
	UEP_TYPE		= 17,
	UFS_RATE_B_TYPE	= 18,
	MAX_SUBPARTS		= 19,
};

/*
 * There are two order requirements for an IFWI image:
 * 1. Order in which the sub-partitions lie within the BPDT entries.
 * 2. Order in which the sub-partitions lie within the image.
 *
 * header_order defines #1 i.e. the order in which the sub-partitions should
 * appear in the BPDT entries. pack_order defines #2 i.e. the order in which
 * sub-partitions appear in the IFWI image. pack_order controls the offset and
 * thus sub-partitions would have increasing offsets as we loop over pack_order.
 */
const enum bpdt_entry_type bpdt_header_order[MAX_SUBPARTS] = {
	/* Order of the following entries is mandatory */
	CSE_IDLM_TYPE,
	IFP_OVERRIDE_TYPE,
	S_BPDT_TYPE,
	CSE_RBE_TYPE,
	UFS_PHY_TYPE,
	UFS_GPP_TYPE,
	/* Order of the following entries is recommended */
	UEP_TYPE,
	NVM_CONFIG_TYPE,
	UFS_RATE_B_TYPE,
	IBB_TYPE,
	SMIP_TYPE,
	PMC_TYPE,
	CSE_BUP_TYPE,
	UCODE_TYPE,
	DEBUG_TOKENS_TYPE,
	IUNIT_TYPE,
	CSE_MAIN_TYPE,
	ISH_TYPE,
	OBB_TYPE,
};

const enum bpdt_entry_type bpdt_pack_order[MAX_SUBPARTS] = {
	/* Order of the following entries is mandatory */
	UFS_GPP_TYPE,
	UFS_PHY_TYPE,
	IFP_OVERRIDE_TYPE,
	UEP_TYPE,
	NVM_CONFIG_TYPE,
	UFS_RATE_B_TYPE,
	/* Order of the following entries is recommended */
	IBB_TYPE,
	SMIP_TYPE,
	CSE_RBE_TYPE,
	PMC_TYPE,
	CSE_BUP_TYPE,
	UCODE_TYPE,
	CSE_IDLM_TYPE,
	DEBUG_TOKENS_TYPE,
	S_BPDT_TYPE,
	IUNIT_TYPE,
	CSE_MAIN_TYPE,
	ISH_TYPE,
	OBB_TYPE,
};

/* Utility functions */
enum ifwi_ret {
	COMMAND_ERR = -1,
	NO_ACTION_REQUIRED = 0,
	REPACK_REQUIRED = 1,
};

struct dir_ops {
	enum ifwi_ret (*dir_add)(int type);
};

static enum ifwi_ret ibbp_dir_add(int type);

const struct subpart_info {
	const char *name;
	const char *readable_name;
	uint32_t attr;
	struct dir_ops dir_ops;
} subparts[MAX_SUBPARTS] = {
	/* OEM SMIP */
	[SMIP_TYPE] = {"SMIP", "SMIP", CONTAINS_DIR, {NULL} },
	/* CSE RBE */
	[CSE_RBE_TYPE] = {"RBEP", "CSE_RBE", CONTAINS_DIR |
			  MANDATORY_BPDT_ENTRY, {NULL} },
	/* CSE BUP */
	[CSE_BUP_TYPE] = {"FTPR", "CSE_BUP", CONTAINS_DIR |
			  MANDATORY_BPDT_ENTRY, {NULL} },
	/* uCode */
	[UCODE_TYPE] = {"UCOD", "Microcode", CONTAINS_DIR, {NULL} },
	/* IBB */
	[IBB_TYPE] = {"IBBP", "Bootblock", CONTAINS_DIR, {ibbp_dir_add} },
	/* S-BPDT */
	[S_BPDT_TYPE] = {"S_BPDT", "S-BPDT", AUTO_GENERATED |
			 MANDATORY_BPDT_ENTRY, {NULL} },
	/* OBB */
	[OBB_TYPE] = {"OBBP", "OEM boot block", CONTAINS_DIR |
		      NON_CRITICAL_SUBPART, {NULL} },
	/* CSE Main */
	[CSE_MAIN_TYPE] = {"NFTP", "CSE_MAIN", CONTAINS_DIR |
			   NON_CRITICAL_SUBPART, {NULL} },
	/* ISH */
	[ISH_TYPE] = {"ISHP", "ISH", NON_CRITICAL_SUBPART, {NULL} },
	/* CSE IDLM */
	[CSE_IDLM_TYPE] = {"DLMP", "CSE_IDLM", CONTAINS_DIR |
			   MANDATORY_BPDT_ENTRY, {NULL} },
	/* IFP Override */
	[IFP_OVERRIDE_TYPE] = {"IFP_OVERRIDE", "IFP_OVERRIDE",
			       LIES_WITHIN_BPDT_4K | MANDATORY_BPDT_ENTRY,
			       {NULL} },
	/* Debug Tokens */
	[DEBUG_TOKENS_TYPE] = {"DEBUG_TOKENS", "Debug Tokens", 0, {NULL} },
	/* UFS Phy Configuration */
	[UFS_PHY_TYPE] = {"UFS_PHY", "UFS Phy", LIES_WITHIN_BPDT_4K |
			  MANDATORY_BPDT_ENTRY, {NULL} },
	/* UFS GPP LUN ID */
	[UFS_GPP_TYPE] = {"UFS_GPP", "UFS GPP", LIES_WITHIN_BPDT_4K |
			  MANDATORY_BPDT_ENTRY, {NULL} },
	/* PMC */
	[PMC_TYPE] = {"PMCP", "PMC firmware", CONTAINS_DIR, {NULL} },
	/* IUNIT */
	[IUNIT_TYPE] = {"IUNP", "IUNIT", NON_CRITICAL_SUBPART, {NULL} },
	/* NVM Config */
	[NVM_CONFIG_TYPE] = {"NVM_CONFIG", "NVM Config", 0, {NULL} },
	/* UEP */
	[UEP_TYPE] = {"UEP", "UEP", LIES_WITHIN_BPDT_4K | MANDATORY_BPDT_ENTRY,
		      {NULL} },
	/* UFS Rate B Config */
	[UFS_RATE_B_TYPE] = {"UFS_RATE_B", "UFS Rate B Config", 0, {NULL} },
};

struct ifwi_image {
	/* Data read from input file */
	struct buffer input_buff;

	/* BPDT header and entries */
	struct buffer bpdt;
	size_t input_ifwi_start_offset;
	size_t input_ifwi_end_offset;

	/* Subpartition content */
	struct buffer subpart_buf[MAX_SUBPARTS];
} ifwi_image;

/* Buffer and file I/O */
static off_t get_file_size(FILE *f)
{
	off_t fsize;

	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	return fsize;
}

static inline void *buffer_get(const struct buffer *b)
{
	return b->data;
}

static inline size_t buffer_size(const struct buffer *b)
{
	return b->size;
}

static inline size_t buffer_offset(const struct buffer *b)
{
	return b->offset;
}

/*
 * Shrink a buffer toward the beginning of its previous space.
 * Afterward, buffer_delete() remains the means of cleaning it up
 */
static inline void buffer_set_size(struct buffer *b, size_t size)
{
	b->size = size;
}

/* Splice a buffer into another buffer. Note that it's up to the caller to
 * bounds check the offset and size. The resulting buffer is backed by the same
 * storage as the original, so although it is valid to buffer_delete() either
 * one of them, doing so releases both simultaneously
 */
static void buffer_splice(struct buffer *dest, const struct buffer *src,
			  size_t offset, size_t size)
{
	dest->name = src->name;
	dest->data = src->data + offset;
	dest->offset = src->offset + offset;
	dest->size = size;
}

/*
 * Shrink a buffer toward the end of its previous space.
 * Afterward, buffer_delete() remains the means of cleaning it up
 */
static inline void buffer_seek(struct buffer *b, size_t size)
{
	b->offset += size;
	b->size -= size;
	b->data += size;
}

/* Returns the start of the underlying buffer, with the offset undone */
static inline void *buffer_get_original_backing(const struct buffer *b)
{
	if (!b)
		return NULL;
	return buffer_get(b) - buffer_offset(b);
}

int buffer_create(struct buffer *buffer, size_t size, const char *name)
{
	buffer->name = strdup(name);
	buffer->offset = 0;
	buffer->size = size;
	buffer->data = (char *)malloc(buffer->size);
	if (!buffer->data) {
		fprintf(stderr, "%s: Insufficient memory (0x%zx).\n", __func__,
			size);
	}

	return !buffer->data;
}

int buffer_write_file(struct buffer *buffer, const char *filename)
{
	FILE *fp = fopen(filename, "wb");

	if (!fp) {
		perror(filename);
		return -1;
	}
	assert(buffer && buffer->data);
	if (fwrite(buffer->data, 1, buffer->size, fp) != buffer->size) {
		fprintf(stderr, "incomplete write: %s\n", filename);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

void buffer_delete(struct buffer *buffer)
{
	assert(buffer);
	if (buffer->name) {
		free(buffer->name);
		buffer->name = NULL;
	}
	if (buffer->data) {
		free(buffer_get_original_backing(buffer));
		buffer->data = NULL;
	}
	buffer->offset = 0;
	buffer->size = 0;
}

int buffer_from_file(struct buffer *buffer, const char *filename)
{
	FILE *fp = fopen(filename, "rb");

	if (!fp) {
		perror(filename);
		return -1;
	}
	buffer->offset = 0;
	off_t file_size = get_file_size(fp);

	if (file_size < 0) {
		fprintf(stderr, "could not determine size of %s\n", filename);
		fclose(fp);
		return -1;
	}
	buffer->size = file_size;
	buffer->name = strdup(filename);
	buffer->data = (char *)malloc(buffer->size);
	assert(buffer->data);
	if (fread(buffer->data, 1, buffer->size, fp) != buffer->size) {
		fprintf(stderr, "incomplete read: %s\n", filename);
		fclose(fp);
		buffer_delete(buffer);
		return -1;
	}
	fclose(fp);
	return 0;
}

static void alloc_buffer(struct buffer *b, size_t s, const char *n)
{
	if (buffer_create(b, s, n) == 0)
		return;

	ERROR("Buffer allocation failure for %s (size = %zx).\n", n, s);
	exit(-1);
}

/* Little-Endian functions */
static inline uint8_t read_ble8(const void *src)
{
	const uint8_t *s = src;
	return *s;
}

static inline uint8_t read_at_ble8(const void *src, size_t offset)
{
	const uint8_t *s = src;

	s += offset;
	return read_ble8(s);
}

static inline void write_ble8(void *dest, uint8_t val)
{
	*(uint8_t *)dest = val;
}

static inline void write_at_ble8(void *dest, uint8_t val, size_t offset)
{
	uint8_t *d = dest;

	d += offset;
	write_ble8(d, val);
}

static inline uint8_t read_at_le8(const void *src, size_t offset)
{
	return read_at_ble8(src, offset);
}

static inline void write_le8(void *dest, uint8_t val)
{
	write_ble8(dest, val);
}

static inline void write_at_le8(void *dest, uint8_t val, size_t offset)
{
	write_at_ble8(dest, val, offset);
}

static inline uint16_t read_le16(const void *src)
{
	const uint8_t *s = src;

	return (((uint16_t)s[1]) << 8) | (((uint16_t)s[0]) << 0);
}

static inline uint16_t read_at_le16(const void *src, size_t offset)
{
	const uint8_t *s = src;

	s += offset;
	return read_le16(s);
}

static inline void write_le16(void *dest, uint16_t val)
{
	write_le8(dest, val >> 0);
	write_at_le8(dest, val >> 8, sizeof(uint8_t));
}

static inline void write_at_le16(void *dest, uint16_t val, size_t offset)
{
	uint8_t *d = dest;

	d += offset;
	write_le16(d, val);
}

static inline uint32_t read_le32(const void *src)
{
	const uint8_t *s = src;

	return (((uint32_t)s[3]) << 24) | (((uint32_t)s[2]) << 16) |
		(((uint32_t)s[1]) << 8) | (((uint32_t)s[0]) << 0);
}

static inline uint32_t read_at_le32(const void *src, size_t offset)
{
	const uint8_t *s = src;

	s += offset;
	return read_le32(s);
}

static inline void write_le32(void *dest, uint32_t val)
{
	write_le16(dest, val >> 0);
	write_at_le16(dest, val >> 16, sizeof(uint16_t));
}

static inline void write_at_le32(void *dest, uint32_t val, size_t offset)
{
	uint8_t *d = dest;

	d += offset;
	write_le32(d, val);
}

static inline uint64_t read_le64(const void *src)
{
	uint64_t val;

	val = read_at_le32(src, sizeof(uint32_t));
	val <<= 32;
	val |= read_le32(src);
	return val;
}

static inline uint64_t read_at_le64(const void *src, size_t offset)
{
	const uint8_t *s = src;

	s += offset;
	return read_le64(s);
}

static inline void write_le64(void *dest, uint64_t val)
{
	write_le32(dest, val >> 0);
	write_at_le32(dest, val >> 32, sizeof(uint32_t));
}

static inline void write_at_le64(void *dest, uint64_t val, size_t offset)
{
	uint8_t *d = dest;

	d += offset;
	write_le64(d, val);
}

/*
 * Read header/entry members in little-endian format.
 * Returns the offset upto which the read was performed.
 */
static size_t read_member(void *src, size_t offset, size_t size_bytes,
			  void *dst)
{
	switch (size_bytes) {
	case 1:
		*(uint8_t *)dst = read_at_le8(src, offset);
		break;
	case 2:
		*(uint16_t *)dst = read_at_le16(src, offset);
		break;
	case 4:
		*(uint32_t *)dst = read_at_le32(src, offset);
		break;
	case 8:
		*(uint64_t *)dst = read_at_le64(src, offset);
		break;
	default:
		ERROR("Read size not supported %zd\n", size_bytes);
		exit(-1);
	}

	return (offset + size_bytes);
}

/*
 * Convert to little endian format.
 * Returns the offset upto which the fixup was performed.
 */
static size_t fix_member(void *data, size_t offset, size_t size_bytes)
{
	void *src = (uint8_t *)data + offset;

	switch (size_bytes) {
	case 1:
		write_at_le8(data, *(uint8_t *)src, offset);
		break;
	case 2:
		write_at_le16(data, *(uint16_t *)src, offset);
		break;
	case 4:
		write_at_le32(data, *(uint32_t *)src, offset);
		break;
	case 8:
		write_at_le64(data, *(uint64_t *)src, offset);
		break;
	default:
		ERROR("Write size not supported %zd\n", size_bytes);
		exit(-1);
	}
	return (offset + size_bytes);
}

static void print_subpart_dir(struct subpart_dir *s)
{
	if (verbose == 0)
		return;

	size_t i;

	printf("%-25s 0x%-23.8x\n", "Marker", s->h.marker);
	printf("%-25s %-25d\n", "Num entries", s->h.num_entries);
	printf("%-25s %-25d\n", "Header Version", s->h.header_version);
	printf("%-25s %-25d\n", "Entry Version", s->h.entry_version);
	printf("%-25s 0x%-23x\n", "Header Length", s->h.header_length);
	printf("%-25s 0x%-23x\n", "Checksum", s->h.checksum);
	printf("%-25s ", "Name");
	for (i = 0; i < sizeof(s->h.name); i++)
		printf("%c", s->h.name[i]);

	printf("\n");

	printf("%-25s%-25s%-25s%-25s%-25s\n", "Entry #", "Name", "Offset",
	       "Length", "Rsvd");

	printf("=========================================================================================================================\n");

	for (i = 0; i < s->h.num_entries; i++) {
		printf("%-25zd%-25.12s0x%-23x0x%-23x0x%-23x\n", i + 1,
		       s->e[i].name, s->e[i].offset, s->e[i].length,
		       s->e[i].rsvd);
	}

	printf("=========================================================================================================================\n");
}

static void bpdt_print_header(struct bpdt_header *h, const char *name)
{
	if (verbose == 0)
		return;

	printf("%-25s %-25s\n", "Header", name);
	printf("%-25s 0x%-23.8x\n", "Signature", h->signature);
	printf("%-25s %-25d\n", "Descriptor count", h->descriptor_count);
	printf("%-25s %-25d\n", "BPDT Version", h->bpdt_version);
	printf("%-25s 0x%-23x\n", "XOR checksum", h->xor_redundant_block);
	printf("%-25s 0x%-23x\n", "IFWI Version", h->ifwi_version);
	printf("%-25s 0x%-23llx\n", "FIT Tool Version",
	       (long long)h->fit_tool_version);
}

static void bpdt_print_entries(struct bpdt_entry *e, size_t count,
			       const char *name)
{
	size_t i;

	if (verbose == 0)
		return;

	printf("%s entries\n", name);

	printf("%-25s%-25s%-25s%-25s%-25s%-25s%-25s%-25s\n", "Entry #",
	       "Sub-Partition", "Name", "Type", "Flags", "Offset", "Size",
	       "File Offset");

	printf("=========================================================================================================================================================================================================\n");

	for (i = 0; i < count; i++) {
		printf("%-25zd%-25s%-25s%-25d0x%-23.08x0x%-23x0x%-23x0x%-23zx\n",
		       i + 1, subparts[e[i].type].name,
		       subparts[e[i].type].readable_name, e[i].type, e[i].flags,
		       e[i].offset, e[i].size,
		       e[i].offset + ifwi_image.input_ifwi_start_offset);
	}

	printf("=========================================================================================================================================================================================================\n");
}

static void bpdt_validate_header(struct bpdt_header *h, const char *name)
{
	assert(h->signature == BPDT_SIGNATURE);

	if (h->bpdt_version != 1) {
		ERROR("Invalid header : %s\n", name);
		exit(-1);
	}

	DEBUG("Validated header : %s\n", name);
}

static void bpdt_read_header(void *data, struct bpdt_header *h,
			     const char *name)
{
	size_t offset = 0;

	offset = read_member(data, offset, sizeof(h->signature), &h->signature);
	offset = read_member(data, offset, sizeof(h->descriptor_count),
			     &h->descriptor_count);
	offset = read_member(data, offset, sizeof(h->bpdt_version),
			     &h->bpdt_version);
	offset = read_member(data, offset, sizeof(h->xor_redundant_block),
			     &h->xor_redundant_block);
	offset = read_member(data, offset, sizeof(h->ifwi_version),
			     &h->ifwi_version);
	read_member(data, offset, sizeof(h->fit_tool_version),
		    &h->fit_tool_version);

	bpdt_validate_header(h, name);
	bpdt_print_header(h, name);
}

static void bpdt_read_entries(void *data, struct bpdt *bpdt, const char *name)
{
	size_t i, offset = 0;
	struct bpdt_entry *e = &bpdt->e[0];
	size_t count = bpdt->h.descriptor_count;

	for (i = 0; i < count; i++) {
		offset = read_member(data, offset, sizeof(e[i].type),
				     &e[i].type);
		offset = read_member(data, offset, sizeof(e[i].flags),
				     &e[i].flags);
		offset = read_member(data, offset, sizeof(e[i].offset),
				     &e[i].offset);
		offset = read_member(data, offset, sizeof(e[i].size),
				     &e[i].size);
	}

	bpdt_print_entries(e, count, name);
}

/*
 * Given type of sub-partition, identify BPDT entry for it.
 * Sub-Partition could lie either within BPDT or S-BPDT.
 */
static struct bpdt_entry *__find_entry_by_type(struct bpdt_entry *e,
					       size_t count, int type)
{
	size_t i;

	for (i = 0; i < count; i++) {
		if (e[i].type == type)
			break;
	}

	if (i == count)
		return NULL;

	return &e[i];
}

static struct bpdt_entry *find_entry_by_type(int type)
{
	struct bpdt *b = buffer_get(&ifwi_image.bpdt);

	if (!b)
		return NULL;

	struct bpdt_entry *curr = __find_entry_by_type(&b->e[0],
						       b->h.descriptor_count,
						       type);

	if (curr)
		return curr;

	b = buffer_get(&ifwi_image.subpart_buf[S_BPDT_TYPE]);
	if (!b)
		return NULL;

	return __find_entry_by_type(&b->e[0], b->h.descriptor_count, type);
}

/*
 * Find sub-partition type given its name. If the name does not exist, returns
 * -1.
 */
static int find_type_by_name(const char *name)
{
	int i;

	for (i = 0; i < MAX_SUBPARTS; i++) {
		if ((strlen(subparts[i].name) == strlen(name)) &&
		    (!strcmp(subparts[i].name, name)))
			break;
	}

	if (i == MAX_SUBPARTS) {
		ERROR("Invalid sub-partition name %s.\n", name);
		return -1;
	}

	return i;
}

/*
 * Read the content of a sub-partition from input file and store it in
 * ifwi_image.subpart_buf[SUB-PARTITION_TYPE].
 *
 * Returns the maximum offset occupied by the sub-partitions.
 */
static size_t read_subpart_buf(void *data, size_t size, struct bpdt_entry *e,
			       size_t count)
{
	size_t i, type;
	struct buffer *buf;
	size_t max_offset = 0;

	for (i = 0; i < count; i++) {
		type = e[i].type;

		if (type >= MAX_SUBPARTS) {
			ERROR("Invalid sub-partition type %zd.\n", type);
			exit(-1);
		}

		if (buffer_size(&ifwi_image.subpart_buf[type])) {
			ERROR("Multiple sub-partitions of type %zd(%s).\n",
			      type, subparts[type].name);
			exit(-1);
		}

		if (e[i].size == 0) {
			INFO("Dummy sub-partition %zd(%s). Skipping.\n", type,
			     subparts[type].name);
			continue;
		}

		assert((e[i].offset + e[i].size) <= size);

		/*
		 * Sub-partitions in IFWI image are not in the same order as
		 * in BPDT entries. BPDT entires are in header_order whereas
		 * sub-partition offsets in the image are in pack_order.
		 */
		if ((e[i].offset + e[i].size) > max_offset)
			max_offset = e[i].offset + e[i].size;

		/*
		 * S-BPDT sub-partition contains information about all the
		 * non-critical sub-partitions. Thus, size of S-BPDT
		 * sub-partition equals size of S-BPDT plus size of all the
		 * non-critical sub-partitions. Thus, reading whole of S-BPDT
		 * here would be redundant as the non-critical partitions are
		 * read and allocated buffers separately. Also, S-BPDT requires
		 * special handling for reading header and entries.
		 */
		if (type == S_BPDT_TYPE)
			continue;

		buf = &ifwi_image.subpart_buf[type];

		alloc_buffer(buf, e[i].size, subparts[type].name);
		memcpy(buffer_get(buf), (uint8_t *)data + e[i].offset,
		       e[i].size);
	}

	assert(max_offset);
	return max_offset;
}

/*
 * Allocate buffer for bpdt header, entries and all sub-partition content.
 * Returns offset in data where BPDT ends.
 */
static size_t alloc_bpdt_buffer(void *data, size_t size, size_t offset,
				struct buffer *b, const char *name)
{
	struct bpdt_header bpdt_header;

	assert((offset + BPDT_HEADER_SIZE) < size);
	bpdt_read_header((uint8_t *)data + offset, &bpdt_header, name);

	/* Buffer to read BPDT header and entries */
	alloc_buffer(b, get_bpdt_size(&bpdt_header), name);

	struct bpdt *bpdt = buffer_get(b);

	memcpy(&bpdt->h, &bpdt_header, BPDT_HEADER_SIZE);

	/*
	 * If no entries are present, maximum offset occupied is (offset +
	 * BPDT_HEADER_SIZE).
	 */
	if (bpdt->h.descriptor_count == 0)
		return (offset + BPDT_HEADER_SIZE);

	/* Read all entries */
	assert((offset + get_bpdt_size(&bpdt->h)) < size);
	bpdt_read_entries((uint8_t *)data + offset + BPDT_HEADER_SIZE, bpdt,
			  name);

	/* Read all sub-partition content in subpart_buf */
	return read_subpart_buf(data, size, &bpdt->e[0],
				bpdt->h.descriptor_count);
}

static void parse_sbpdt(void *data, size_t size)
{
	struct bpdt_entry *s;

	s  = find_entry_by_type(S_BPDT_TYPE);
	if (!s)
		return;

	assert(size > s->offset);

	alloc_bpdt_buffer(data, size, s->offset,
			  &ifwi_image.subpart_buf[S_BPDT_TYPE],
			  "S-BPDT");
}

static uint8_t calc_checksum(struct subpart_dir *s)
{
	size_t size = subpart_dir_size(&s->h);
	uint8_t *data = (uint8_t *)s;
	uint8_t checksum = 0;
	size_t i;
	uint8_t old_checksum = s->h.checksum;

	s->h.checksum = 0;

	for (i = 0; i < size; i++)
		checksum += data[i];

	s->h.checksum = old_checksum;

	/* 2s complement */
	return -checksum;
}

static void validate_subpart_dir(struct subpart_dir *s, const char *name,
				 bool checksum_check)
{
	if (s->h.marker != SUBPART_DIR_MARKER ||
	    s->h.header_version != SUBPART_DIR_HEADER_VERSION_SUPPORTED ||
	    s->h.entry_version != SUBPART_DIR_ENTRY_VERSION_SUPPORTED ||
	    s->h.header_length != SUBPART_DIR_HEADER_SIZE) {
		ERROR("Invalid subpart_dir for %s.\n", name);
		exit(-1);
	}

	if (!checksum_check)
		return;

	uint8_t checksum = calc_checksum(s);

	if (checksum != s->h.checksum)
		ERROR("Invalid checksum for %s (Expected=0x%x, Actual=0x%x).\n",
		      name, checksum, s->h.checksum);
}

static void validate_subpart_dir_without_checksum(struct subpart_dir *s,
						  const char *name)
{
	validate_subpart_dir(s, name, 0);
}

static void validate_subpart_dir_with_checksum(struct subpart_dir *s,
					       const char *name)
{
	validate_subpart_dir(s, name, 1);
}

static void parse_subpart_dir(struct buffer *subpart_dir_buf,
			      struct buffer *input_buf, const char *name)
{
	struct subpart_dir_header hdr;
	size_t offset = 0;
	uint8_t *data = buffer_get(input_buf);
	size_t size = buffer_size(input_buf);

	/* Read Subpart_Dir header */
	assert(size >= SUBPART_DIR_HEADER_SIZE);
	offset = read_member(data, offset, sizeof(hdr.marker), &hdr.marker);
	offset = read_member(data, offset, sizeof(hdr.num_entries),
			     &hdr.num_entries);
	offset = read_member(data, offset, sizeof(hdr.header_version),
			     &hdr.header_version);
	offset = read_member(data, offset, sizeof(hdr.entry_version),
			     &hdr.entry_version);
	offset = read_member(data, offset, sizeof(hdr.header_length),
			     &hdr.header_length);
	offset = read_member(data, offset, sizeof(hdr.checksum), &hdr.checksum);
	memcpy(hdr.name, data + offset, sizeof(hdr.name));
	offset += sizeof(hdr.name);

	validate_subpart_dir_without_checksum((struct subpart_dir *)&hdr, name);

	assert(size > subpart_dir_size(&hdr));
	alloc_buffer(subpart_dir_buf, subpart_dir_size(&hdr), "Subpart Dir");
	memcpy(buffer_get(subpart_dir_buf), &hdr, SUBPART_DIR_HEADER_SIZE);

	/* Read Subpart Dir entries */
	struct subpart_dir *subpart_dir = buffer_get(subpart_dir_buf);
	struct subpart_dir_entry *e = &subpart_dir->e[0];
	uint32_t i;

	for (i = 0; i < hdr.num_entries; i++) {
		memcpy(e[i].name, data + offset, sizeof(e[i].name));
		offset += sizeof(e[i].name);
		offset = read_member(data, offset, sizeof(e[i].offset),
				     &e[i].offset);
		offset = read_member(data, offset, sizeof(e[i].length),
				     &e[i].length);
		offset = read_member(data, offset, sizeof(e[i].rsvd),
				     &e[i].rsvd);
	}

	validate_subpart_dir_with_checksum(subpart_dir, name);

	print_subpart_dir(subpart_dir);
}

/* Parse input image file to identify different sub-partitions */
static int ifwi_parse(void)
{
	struct buffer *buff = &ifwi_image.input_buff;
	const char *image_name = param.image_name;

	DEBUG("Parsing IFWI image...\n");

	/* Read input file */
	if (buffer_from_file(buff, image_name)) {
		ERROR("Failed to read input file %s.\n", image_name);
		return -1;
	}

	INFO("Buffer %p size 0x%zx\n", buff->data, buff->size);

	/* Look for BPDT signature at 4K intervals */
	size_t offset = 0;
	void *data = buffer_get(buff);

	while (offset < buffer_size(buff)) {
		if (read_at_le32(data, offset) == BPDT_SIGNATURE)
			break;
		offset += 4 * KiB;
	}

	if (offset >= buffer_size(buff)) {
		ERROR("Image does not contain BPDT!!\n");
		return -1;
	}

	ifwi_image.input_ifwi_start_offset = offset;
	INFO("BPDT starts at offset 0x%zx.\n", offset);

	data = (uint8_t *)data + offset;
	size_t ifwi_size = buffer_size(buff) - offset;

	/* Read BPDT and sub-partitions */
	uintptr_t end_offset;

	end_offset = ifwi_image.input_ifwi_start_offset +
		alloc_bpdt_buffer(data, ifwi_size, 0, &ifwi_image.bpdt, "BPDT");

	/* Parse S-BPDT, if any */
	parse_sbpdt(data, ifwi_size);

	/*
	 * Store end offset of IFWI. Required for copying any trailing non-IFWI
	 * part of the image.
	 * ASSUMPTION: IFWI image always ends on a 4K boundary.
	 */
	ifwi_image.input_ifwi_end_offset = ALIGN(end_offset, 4 * KiB);
	DEBUG("Parsing done.\n");

	return 0;
}

/*
 * This function is used by repack to count the number of BPDT and S-BPDT
 * entries that are present. It frees the current buffers used by the entries
 * and allocates fresh buffers that can be used for repacking. Returns BPDT
 * entries which are empty and need to be filled in.
 */
static void __bpdt_reset(struct buffer *b, size_t count, size_t size)
{
	size_t bpdt_size = BPDT_HEADER_SIZE + count * BPDT_ENTRY_SIZE;

	assert(size >= bpdt_size);

	/*
	 * If buffer does not have the required size, allocate a fresh buffer.
	 */
	if (buffer_size(b) != size) {
		struct buffer temp;

		alloc_buffer(&temp, size, b->name);
		memcpy(buffer_get(&temp), buffer_get(b), buffer_size(b));
		buffer_delete(b);
		*b = temp;
	}

	struct bpdt *bpdt = buffer_get(b);
	uint8_t *ptr = (uint8_t *)&bpdt->e[0];
	size_t entries_size = BPDT_ENTRY_SIZE * count;

	/* Zero out BPDT entries */
	memset(ptr, 0, entries_size);
	/* Fill any pad-space with FF */
	memset(ptr + entries_size, 0xFF, size - bpdt_size);

	bpdt->h.descriptor_count = count;
}

static void bpdt_reset(void)
{
	size_t i;
	size_t bpdt_count = 0, sbpdt_count = 0, dummy_bpdt_count = 0;

	/* Count number of BPDT and S-BPDT entries */
	for (i = 0; i < MAX_SUBPARTS; i++) {
		if (buffer_size(&ifwi_image.subpart_buf[i]) == 0) {
			if (subparts[i].attr & MANDATORY_BPDT_ENTRY) {
				bpdt_count++;
				dummy_bpdt_count++;
			}
			continue;
		}

		if (subparts[i].attr & NON_CRITICAL_SUBPART)
			sbpdt_count++;
		else
			bpdt_count++;
	}

	DEBUG("Count: BPDT = %zd, Dummy BPDT = %zd, S-BPDT = %zd\n", bpdt_count,
	      dummy_bpdt_count, sbpdt_count);

	/* Update BPDT if required */
	size_t bpdt_size = max(BPDT_MIN_SIZE,
			       BPDT_HEADER_SIZE + bpdt_count * BPDT_ENTRY_SIZE);
	__bpdt_reset(&ifwi_image.bpdt, bpdt_count, bpdt_size);

	/* Update S-BPDT if required */
	bpdt_size = ALIGN(BPDT_HEADER_SIZE + sbpdt_count * BPDT_ENTRY_SIZE,
			  4 * KiB);
	__bpdt_reset(&ifwi_image.subpart_buf[S_BPDT_TYPE], sbpdt_count,
		     bpdt_size);
}

/* Initialize BPDT entries in header order */
static void bpdt_entries_init_header_order(void)
{
	int i, type;
	size_t size;

	struct bpdt *bpdt, *sbpdt, *curr;
	size_t bpdt_curr = 0, sbpdt_curr = 0, *count_ptr;

	bpdt = buffer_get(&ifwi_image.bpdt);
	sbpdt = buffer_get(&ifwi_image.subpart_buf[S_BPDT_TYPE]);

	for (i = 0; i < MAX_SUBPARTS; i++) {
		type = bpdt_header_order[i];
		size = buffer_size(&ifwi_image.subpart_buf[type]);

		if (size == 0 && !(subparts[type].attr & MANDATORY_BPDT_ENTRY))
			continue;

		if (subparts[type].attr & NON_CRITICAL_SUBPART) {
			curr = sbpdt;
			count_ptr = &sbpdt_curr;
		} else {
			curr = bpdt;
			count_ptr = &bpdt_curr;
		}

		assert(*count_ptr < curr->h.descriptor_count);
		curr->e[*count_ptr].type = type;
		curr->e[*count_ptr].flags = 0;
		curr->e[*count_ptr].offset = 0;
		curr->e[*count_ptr].size = size;

		(*count_ptr)++;
	}
}

static void pad_buffer(struct buffer *b, size_t size)
{
	size_t buff_size = buffer_size(b);

	assert(buff_size <= size);

	if (buff_size == size)
		return;

	struct buffer temp;

	alloc_buffer(&temp, size, b->name);
	uint8_t *data = buffer_get(&temp);

	memcpy(data, buffer_get(b), buff_size);
	memset(data + buff_size, 0xFF, size - buff_size);

	*b = temp;
}

/* Initialize offsets of entries using pack order */
static void bpdt_entries_init_pack_order(void)
{
	int i, type;
	struct bpdt_entry *curr;
	size_t curr_offset, curr_end;

	curr_offset = max(BPDT_MIN_SIZE, buffer_size(&ifwi_image.bpdt));

	/*
	 * There are two types of sub-partitions that need to be handled here:
	 *   1. Sub-partitions that lie within the same 4K as BPDT
	 *   2. Sub-partitions that lie outside the 4K of BPDT
	 *
	 * For sub-partitions of type # 1, there is no requirement on the start
	 * or end of the sub-partition. They need to be packed in without any
	 * holes left in between. If there is any empty space left after the end
	 * of the last sub-partition in 4K of BPDT, then that space needs to be
	 * padded with FF bytes, but the size of the last sub-partition remains
	 * unchanged.
	 *
	 * For sub-partitions of type # 2, both the start and end should be a
	 * multiple of 4K. If not, then it needs to be padded with FF bytes and
	 * size adjusted such that the sub-partition ends on 4K boundary.
	 */

	/* #1 Sub-partitions that lie within same 4K as BPDT */
	struct buffer *last_bpdt_buff = &ifwi_image.bpdt;

	for (i = 0; i < MAX_SUBPARTS; i++) {
		type = bpdt_pack_order[i];
		curr = find_entry_by_type(type);

		if (!curr || curr->size == 0)
			continue;

		if (!(subparts[type].attr & LIES_WITHIN_BPDT_4K))
			continue;

		curr->offset = curr_offset;
		curr_offset = curr->offset + curr->size;
		last_bpdt_buff = &ifwi_image.subpart_buf[type];
		DEBUG("type=%d, curr_offset=0x%zx, curr->offset=0x%x, curr->size=0x%x, buff_size=0x%zx\n",
		      type, curr_offset, curr->offset, curr->size,
		      buffer_size(&ifwi_image.subpart_buf[type]));
	}

	/* Pad ff bytes if there is any empty space left in BPDT 4K */
	curr_end = ALIGN(curr_offset, 4 * KiB);
	pad_buffer(last_bpdt_buff,
		   buffer_size(last_bpdt_buff) + (curr_end - curr_offset));
	curr_offset = curr_end;

	/* #2 Sub-partitions that lie outside of BPDT 4K */
	for (i = 0; i < MAX_SUBPARTS; i++) {
		type = bpdt_pack_order[i];
		curr = find_entry_by_type(type);

		if (!curr || curr->size == 0)
			continue;

		if (subparts[type].attr & LIES_WITHIN_BPDT_4K)
			continue;

		assert(curr_offset == ALIGN(curr_offset, 4 * KiB));
		curr->offset = curr_offset;
		curr_end = ALIGN(curr->offset + curr->size, 4 * KiB);
		curr->size = curr_end - curr->offset;

		pad_buffer(&ifwi_image.subpart_buf[type], curr->size);

		curr_offset = curr_end;
		DEBUG("type=%d, curr_offset=0x%zx, curr->offset=0x%x, curr->size=0x%x, buff_size=0x%zx\n",
		      type, curr_offset, curr->offset, curr->size,
		      buffer_size(&ifwi_image.subpart_buf[type]));
	}

	/*
	 * Update size of S-BPDT to include size of all non-critical
	 * sub-partitions.
	 *
	 * Assumption: S-BPDT always lies at the end of IFWI image.
	 */
	curr = find_entry_by_type(S_BPDT_TYPE);
	assert(curr);

	assert(curr_offset == ALIGN(curr_offset, 4 * KiB));
	curr->size = curr_offset - curr->offset;
}

/* Convert all members of BPDT to little-endian format */
static void bpdt_fixup_write_buffer(struct buffer *buf)
{
	struct bpdt *s = buffer_get(buf);

	struct bpdt_header *h = &s->h;
	struct bpdt_entry *e = &s->e[0];

	size_t count = h->descriptor_count;

	size_t offset = 0;

	offset = fix_member(s, offset, sizeof(h->signature));
	offset = fix_member(s, offset, sizeof(h->descriptor_count));
	offset = fix_member(s, offset, sizeof(h->bpdt_version));
	offset = fix_member(s, offset, sizeof(h->xor_redundant_block));
	offset = fix_member(s, offset, sizeof(h->ifwi_version));
	offset = fix_member(s, offset, sizeof(h->fit_tool_version));

	uint32_t i;

	for (i = 0; i < count; i++) {
		offset = fix_member(s, offset, sizeof(e[i].type));
		offset = fix_member(s, offset, sizeof(e[i].flags));
		offset = fix_member(s, offset, sizeof(e[i].offset));
		offset = fix_member(s, offset, sizeof(e[i].size));
	}
}

/* Write BPDT to output buffer after fixup */
static void bpdt_write(struct buffer *dst, size_t offset, struct buffer *src)
{
	bpdt_fixup_write_buffer(src);
	memcpy(buffer_get(dst) + offset, buffer_get(src), buffer_size(src));
}

/*
 * Follows these steps to re-create image:
 * 1. Write any non-IFWI prefix.
 * 2. Write out BPDT header and entries.
 * 3. Write sub-partition buffers to respective offsets.
 * 4. Write any non-IFWI suffix.
 *
 * While performing the above steps, make sure that any empty holes are filled
 * with FF.
 */
static void ifwi_write(const char *image_name)
{
	struct bpdt_entry *s = find_entry_by_type(S_BPDT_TYPE);

	assert(s);

	size_t ifwi_start, ifwi_end, file_end;

	ifwi_start = ifwi_image.input_ifwi_start_offset;
	ifwi_end = ifwi_start + ALIGN(s->offset + s->size, 4 * KiB);
	file_end = ifwi_end + (buffer_size(&ifwi_image.input_buff) -
			       ifwi_image.input_ifwi_end_offset);

	struct buffer b;

	alloc_buffer(&b, file_end, "Final-IFWI");

	uint8_t *input_data = buffer_get(&ifwi_image.input_buff);
	uint8_t *output_data = buffer_get(&b);

	DEBUG("ifwi_start:0x%zx, ifwi_end:0x%zx, file_end:0x%zx\n", ifwi_start,
	      ifwi_end, file_end);

	/* Copy non-IFWI prefix, if any */
	memcpy(output_data, input_data, ifwi_start);

	DEBUG("Copied non-IFWI prefix (offset=0x0, size=0x%zx).\n", ifwi_start);

	struct buffer ifwi;

	buffer_splice(&ifwi, &b, ifwi_start, ifwi_end - ifwi_start);
	uint8_t *ifwi_data = buffer_get(&ifwi);

	/* Copy sub-partitions using pack_order */
	struct bpdt_entry *curr;
	struct buffer *subpart_buf;
	int i, type;

	for (i = 0; i < MAX_SUBPARTS; i++) {
		type = bpdt_pack_order[i];

		if (type == S_BPDT_TYPE)
			continue;

		curr = find_entry_by_type(type);

		if (!curr || !curr->size)
			continue;

		subpart_buf = &ifwi_image.subpart_buf[type];

		DEBUG("curr->offset=0x%x, curr->size=0x%x, type=%d, write_size=0x%zx\n",
		      curr->offset, curr->size, type, buffer_size(subpart_buf));

		assert((curr->offset + buffer_size(subpart_buf)) <=
		       buffer_size(&ifwi));

		memcpy(ifwi_data + curr->offset, buffer_get(subpart_buf),
		       buffer_size(subpart_buf));
	}

	/* Copy non-IFWI suffix, if any */
	if (ifwi_end != file_end) {
		memcpy(output_data + ifwi_end,
		       input_data + ifwi_image.input_ifwi_end_offset,
		       file_end - ifwi_end);
		DEBUG("Copied non-IFWI suffix (offset=0x%zx,size=0x%zx).\n",
		      ifwi_end, file_end - ifwi_end);
	}

	/*
	 * Convert BPDT to little-endian format and write it to output buffer.
	 * S-BPDT is written first and then BPDT.
	 */
	bpdt_write(&ifwi, s->offset, &ifwi_image.subpart_buf[S_BPDT_TYPE]);
	bpdt_write(&ifwi, 0, &ifwi_image.bpdt);

	if (buffer_write_file(&b, image_name)) {
		ERROR("File write error\n");
		exit(-1);
	}

	buffer_delete(&b);
	printf("Image written successfully to %s.\n", image_name);
}

/*
 * Calculate size and offset of each sub-partition again since it might have
 * changed because of add/delete operation. Also, re-create BPDT and S-BPDT
 * entries and write back the new IFWI image to file.
 */
static void ifwi_repack(void)
{
	bpdt_reset();
	bpdt_entries_init_header_order();
	bpdt_entries_init_pack_order();

	struct bpdt *b = buffer_get(&ifwi_image.bpdt);

	bpdt_print_entries(&b->e[0], b->h.descriptor_count, "BPDT");

	b = buffer_get(&ifwi_image.subpart_buf[S_BPDT_TYPE]);
	bpdt_print_entries(&b->e[0], b->h.descriptor_count, "S-BPDT");

	DEBUG("Repack done.. writing image.\n");
	ifwi_write(param.image_name);
}

static void init_subpart_dir_header(struct subpart_dir_header *hdr,
				    size_t count, const char *name)
{
	memset(hdr, 0, sizeof(*hdr));

	hdr->marker = SUBPART_DIR_MARKER;
	hdr->num_entries = count;
	hdr->header_version = SUBPART_DIR_HEADER_VERSION_SUPPORTED;
	hdr->entry_version = SUBPART_DIR_ENTRY_VERSION_SUPPORTED;
	hdr->header_length = SUBPART_DIR_HEADER_SIZE;
	memcpy(hdr->name, name, sizeof(hdr->name));
}

static size_t init_subpart_dir_entry(struct subpart_dir_entry *e,
				     struct buffer *b, size_t offset)
{
	memset(e, 0, sizeof(*e));

	assert(strlen(b->name) <= sizeof(e->name));
	strncpy((char *)e->name, (char *)b->name, sizeof(e->name));
	e->offset = offset;
	e->length = buffer_size(b);

	return (offset + buffer_size(b));
}

static void init_manifest_header(struct manifest_header *hdr, size_t size)
{
	memset(hdr, 0, sizeof(*hdr));

	hdr->header_type = 0x4;
	assert((MANIFEST_HDR_SIZE % DWORD_SIZE) == 0);
	hdr->header_length = MANIFEST_HDR_SIZE / DWORD_SIZE;
	hdr->header_version = 0x10000;
	hdr->vendor = 0x8086;

	struct tm *local_time;
	time_t curr_time;
	char buffer[11];

	curr_time = time(NULL);
	local_time = localtime(&curr_time);
	assert(local_time != NULL);

	strftime(buffer, sizeof(buffer), "0x%Y%m%d", local_time);
	hdr->date = strtoul(buffer, NULL, 16);

	assert((size % DWORD_SIZE) == 0);
	hdr->size = size / DWORD_SIZE;
	hdr->id = MANIFEST_ID_MAGIC;
}

static void init_signed_pkg_info_ext(struct signed_pkg_info_ext *ext,
				     size_t count, const char *name)
{
	memset(ext, 0, sizeof(*ext));

	ext->ext_type = SIGNED_PKG_INFO_EXT_TYPE;
	ext->ext_length = SIGNED_PKG_INFO_EXT_SIZE + count * MODULE_SIZE;
	memcpy(ext->name, name, sizeof(ext->name));
}

static void subpart_dir_fixup_write_buffer(struct buffer *buf)
{
	struct subpart_dir *s = buffer_get(buf);
	struct subpart_dir_header *h = &s->h;
	struct subpart_dir_entry *e = &s->e[0];

	size_t count = h->num_entries;
	size_t offset = 0;

	offset = fix_member(s, offset, sizeof(h->marker));
	offset = fix_member(s, offset, sizeof(h->num_entries));
	offset = fix_member(s, offset, sizeof(h->header_version));
	offset = fix_member(s, offset, sizeof(h->entry_version));
	offset = fix_member(s, offset, sizeof(h->header_length));
	offset = fix_member(s, offset, sizeof(h->checksum));
	offset += sizeof(h->name);

	uint32_t i;

	for (i = 0; i < count; i++) {
		offset += sizeof(e[i].name);
		offset = fix_member(s, offset, sizeof(e[i].offset));
		offset = fix_member(s, offset, sizeof(e[i].length));
		offset = fix_member(s, offset, sizeof(e[i].rsvd));
	}
}

static void create_subpart(struct buffer *dst, struct buffer *info[],
			   size_t count, const char *name)
{
	struct buffer subpart_dir_buff;
	size_t size = SUBPART_DIR_HEADER_SIZE + count * SUBPART_DIR_ENTRY_SIZE;

	alloc_buffer(&subpart_dir_buff, size, "subpart-dir");

	struct subpart_dir_header *h = buffer_get(&subpart_dir_buff);
	struct subpart_dir_entry *e = (struct subpart_dir_entry *)(h + 1);

	init_subpart_dir_header(h, count, name);

	size_t curr_offset = size;
	size_t i;

	for (i = 0; i < count; i++) {
		curr_offset = init_subpart_dir_entry(&e[i], info[i],
						     curr_offset);
	}

	alloc_buffer(dst, curr_offset, name);
	uint8_t *data = buffer_get(dst);

	for (i = 0; i < count; i++) {
		memcpy(data + e[i].offset, buffer_get(info[i]),
		       buffer_size(info[i]));
	}

	h->checksum = calc_checksum(buffer_get(&subpart_dir_buff));

	struct subpart_dir *dir = buffer_get(&subpart_dir_buff);

	print_subpart_dir(dir);

	subpart_dir_fixup_write_buffer(&subpart_dir_buff);
	memcpy(data, dir, buffer_size(&subpart_dir_buff));

	buffer_delete(&subpart_dir_buff);
}

static enum ifwi_ret ibbp_dir_add(int type)
{
	struct buffer manifest;
	struct signed_pkg_info_ext *ext;
	struct buffer ibbl;
	struct buffer ibb;

#define DUMMY_IBB_SIZE			(4 * KiB)

	assert(type == IBB_TYPE);

	/*
	 * Entry # 1 - IBBP.man
	 * Contains manifest header and signed pkg info extension.
	 */
	size_t size = MANIFEST_HDR_SIZE + SIGNED_PKG_INFO_EXT_SIZE;

	alloc_buffer(&manifest, size, "IBBP.man");

	struct manifest_header *man_hdr = buffer_get(&manifest);

	init_manifest_header(man_hdr, size);

	ext = (struct signed_pkg_info_ext *)(man_hdr + 1);

	init_signed_pkg_info_ext(ext, 0, subparts[type].name);

	/* Entry # 2 - IBBL */
	if (buffer_from_file(&ibbl, param.file_name))
		return COMMAND_ERR;

	/* Entry # 3 - IBB */
	alloc_buffer(&ibb, DUMMY_IBB_SIZE, "IBB");
	memset(buffer_get(&ibb), 0xFF, DUMMY_IBB_SIZE);

	/* Create subpartition */
	struct buffer *info[] = {
		&manifest, &ibbl, &ibb,
	};
	create_subpart(&ifwi_image.subpart_buf[type], &info[0],
		       ARRAY_SIZE(info), subparts[type].name);

	return REPACK_REQUIRED;
}

static enum ifwi_ret ifwi_raw_add(int type)
{
	if (buffer_from_file(&ifwi_image.subpart_buf[type], param.file_name))
		return COMMAND_ERR;

	printf("Sub-partition %s(%d) added from file %s.\n", param.subpart_name,
	       type, param.file_name);
	return REPACK_REQUIRED;
}

static enum ifwi_ret ifwi_dir_add(int type)
{
	if (!(subparts[type].attr & CONTAINS_DIR) ||
	    !subparts[type].dir_ops.dir_add) {
		ERROR("Sub-Partition %s(%d) does not support dir ops.\n",
		      subparts[type].name, type);
		return COMMAND_ERR;
	}

	if (!param.dentry_name) {
		ERROR("%s: -e option required\n", __func__);
		return COMMAND_ERR;
	}

	enum ifwi_ret ret = subparts[type].dir_ops.dir_add(type);

	if (ret != COMMAND_ERR)
		printf("Sub-partition %s(%d) entry %s added from file %s.\n",
		       param.subpart_name, type, param.dentry_name,
		       param.file_name);
	else
		ERROR("Sub-partition dir operation failed.\n");

	return ret;
}

static enum ifwi_ret ifwi_add(void)
{
	if (!param.file_name) {
		ERROR("%s: -f option required\n", __func__);
		return COMMAND_ERR;
	}

	if (!param.subpart_name) {
		ERROR("%s: -n option required\n", __func__);
		return COMMAND_ERR;
	}

	int type = find_type_by_name(param.subpart_name);

	if (type == -1)
		return COMMAND_ERR;

	const struct subpart_info *curr_subpart = &subparts[type];

	if (curr_subpart->attr & AUTO_GENERATED) {
		ERROR("Cannot add auto-generated sub-partitions.\n");
		return COMMAND_ERR;
	}

	if (buffer_size(&ifwi_image.subpart_buf[type])) {
		ERROR("Image already contains sub-partition %s(%d).\n",
		      param.subpart_name, type);
		return COMMAND_ERR;
	}

	if (param.dir_ops)
		return ifwi_dir_add(type);

	return ifwi_raw_add(type);
}

static enum ifwi_ret ifwi_delete(void)
{
	if (!param.subpart_name) {
		ERROR("%s: -n option required\n", __func__);
		return COMMAND_ERR;
	}

	int type = find_type_by_name(param.subpart_name);

	if (type == -1)
		return COMMAND_ERR;

	const struct subpart_info *curr_subpart = &subparts[type];

	if (curr_subpart->attr & AUTO_GENERATED) {
		ERROR("Cannot delete auto-generated sub-partitions.\n");
		return COMMAND_ERR;
	}

	if (buffer_size(&ifwi_image.subpart_buf[type]) == 0) {
		printf("Image does not contain sub-partition %s(%d).\n",
		       param.subpart_name, type);
		return NO_ACTION_REQUIRED;
	}

	buffer_delete(&ifwi_image.subpart_buf[type]);
	printf("Sub-Partition %s(%d) deleted.\n", subparts[type].name, type);
	return REPACK_REQUIRED;
}

static enum ifwi_ret ifwi_dir_extract(int type)
{
	if (!(subparts[type].attr & CONTAINS_DIR)) {
		ERROR("Sub-Partition %s(%d) does not support dir ops.\n",
		      subparts[type].name, type);
		return COMMAND_ERR;
	}

	if (!param.dentry_name) {
		ERROR("%s: -e option required.\n", __func__);
		return COMMAND_ERR;
	}

	struct buffer subpart_dir_buff;

	parse_subpart_dir(&subpart_dir_buff, &ifwi_image.subpart_buf[type],
			  subparts[type].name);

	uint32_t i;
	struct subpart_dir *s = buffer_get(&subpart_dir_buff);

	for (i = 0; i < s->h.num_entries; i++) {
		if (!strncmp((char *)s->e[i].name, param.dentry_name,
			     sizeof(s->e[i].name)))
			break;
	}

	if (i == s->h.num_entries) {
		ERROR("Entry %s not found in subpartition for %s.\n",
		      param.dentry_name, param.subpart_name);
		exit(-1);
	}

	struct buffer dst;

	DEBUG("Splicing buffer at 0x%x size 0x%x\n", s->e[i].offset,
	      s->e[i].length);
	buffer_splice(&dst, &ifwi_image.subpart_buf[type], s->e[i].offset,
		      s->e[i].length);

	if (buffer_write_file(&dst, param.file_name))
		return COMMAND_ERR;

	printf("Sub-Partition %s(%d), entry(%s) stored in %s.\n",
	       param.subpart_name, type, param.dentry_name, param.file_name);

	return NO_ACTION_REQUIRED;
}

static enum ifwi_ret ifwi_raw_extract(int type)
{
	if (buffer_write_file(&ifwi_image.subpart_buf[type], param.file_name))
		return COMMAND_ERR;

	printf("Sub-Partition %s(%d) stored in %s.\n", param.subpart_name, type,
	       param.file_name);

	return NO_ACTION_REQUIRED;
}

static enum ifwi_ret ifwi_extract(void)
{
	if (!param.file_name) {
		ERROR("%s: -f option required\n", __func__);
		return COMMAND_ERR;
	}

	if (!param.subpart_name) {
		ERROR("%s: -n option required\n", __func__);
		return COMMAND_ERR;
	}

	int type = find_type_by_name(param.subpart_name);

	if (type == -1)
		return COMMAND_ERR;

	if (type == S_BPDT_TYPE) {
		INFO("Tool does not support raw extract for %s\n",
		     param.subpart_name);
		return NO_ACTION_REQUIRED;
	}

	if (buffer_size(&ifwi_image.subpart_buf[type]) == 0) {
		ERROR("Image does not contain sub-partition %s(%d).\n",
		      param.subpart_name, type);
		return COMMAND_ERR;
	}

	INFO("Extracting sub-partition %s(%d).\n", param.subpart_name, type);
	if (param.dir_ops)
		return ifwi_dir_extract(type);

	return ifwi_raw_extract(type);
}

static enum ifwi_ret ifwi_print(void)
{
	verbose += 2;

	struct bpdt *b = buffer_get(&ifwi_image.bpdt);

	bpdt_print_header(&b->h, "BPDT");
	bpdt_print_entries(&b->e[0], b->h.descriptor_count, "BPDT");

	b = buffer_get(&ifwi_image.subpart_buf[S_BPDT_TYPE]);
	bpdt_print_header(&b->h, "S-BPDT");
	bpdt_print_entries(&b->e[0], b->h.descriptor_count, "S-BPDT");

	if (param.dir_ops == 0) {
		verbose -= 2;
		return NO_ACTION_REQUIRED;
	}

	int i;
	struct buffer subpart_dir_buf;

	for (i = 0; i < MAX_SUBPARTS ; i++) {
		if (!(subparts[i].attr & CONTAINS_DIR) ||
		    (buffer_size(&ifwi_image.subpart_buf[i]) == 0))
			continue;

		parse_subpart_dir(&subpart_dir_buf, &ifwi_image.subpart_buf[i],
				  subparts[i].name);
		buffer_delete(&subpart_dir_buf);
	}

	verbose -= 2;

	return NO_ACTION_REQUIRED;
}

static enum ifwi_ret ifwi_raw_replace(int type)
{
	buffer_delete(&ifwi_image.subpart_buf[type]);
	return ifwi_raw_add(type);
}

static enum ifwi_ret ifwi_dir_replace(int type)
{
	if (!(subparts[type].attr & CONTAINS_DIR)) {
		ERROR("Sub-Partition %s(%d) does not support dir ops.\n",
		      subparts[type].name, type);
		return COMMAND_ERR;
	}

	if (!param.dentry_name) {
		ERROR("%s: -e option required.\n", __func__);
		return COMMAND_ERR;
	}

	struct buffer subpart_dir_buf;

	parse_subpart_dir(&subpart_dir_buf, &ifwi_image.subpart_buf[type],
			  subparts[type].name);

	uint32_t i;
	struct subpart_dir *s = buffer_get(&subpart_dir_buf);

	for (i = 0; i < s->h.num_entries; i++) {
		if (!strcmp((char *)s->e[i].name, param.dentry_name))
			break;
	}

	if (i == s->h.num_entries) {
		ERROR("Entry %s not found in subpartition for %s.\n",
		      param.dentry_name, param.subpart_name);
		exit(-1);
	}

	struct buffer b;

	if (buffer_from_file(&b, param.file_name)) {
		ERROR("Failed to read %s\n", param.file_name);
		exit(-1);
	}

	struct buffer dst;
	size_t dst_size = buffer_size(&ifwi_image.subpart_buf[type]) +
				      buffer_size(&b) - s->e[i].length;
	size_t subpart_start = s->e[i].offset;
	size_t subpart_end = s->e[i].offset + s->e[i].length;

	alloc_buffer(&dst, dst_size, ifwi_image.subpart_buf[type].name);

	uint8_t *src_data = buffer_get(&ifwi_image.subpart_buf[type]);
	uint8_t *dst_data = buffer_get(&dst);
	size_t curr_offset = 0;

	/* Copy data before the sub-partition entry */
	memcpy(dst_data + curr_offset, src_data, subpart_start);
	curr_offset += subpart_start;

	/* Copy sub-partition entry */
	memcpy(dst_data + curr_offset, buffer_get(&b), buffer_size(&b));
	curr_offset += buffer_size(&b);

	/* Copy remaining data */
	memcpy(dst_data + curr_offset, src_data + subpart_end,
	       buffer_size(&ifwi_image.subpart_buf[type]) - subpart_end);

	/* Update sub-partition buffer */
	int offset = s->e[i].offset;

	buffer_delete(&ifwi_image.subpart_buf[type]);
	ifwi_image.subpart_buf[type] = dst;

	/* Update length of entry in the subpartition */
	s->e[i].length = buffer_size(&b);
	buffer_delete(&b);

	/* Adjust offsets of affected entries in subpartition */
	offset = s->e[i].offset - offset;
	for (; i < s->h.num_entries; i++)
		s->e[i].offset += offset;

	/* Re-calculate checksum */
	s->h.checksum = calc_checksum(s);

	/* Convert members to litte-endian */
	subpart_dir_fixup_write_buffer(&subpart_dir_buf);

	memcpy(dst_data, buffer_get(&subpart_dir_buf),
	       buffer_size(&subpart_dir_buf));

	buffer_delete(&subpart_dir_buf);

	printf("Sub-partition %s(%d) entry %s replaced from file %s.\n",
	       param.subpart_name, type, param.dentry_name, param.file_name);

	return REPACK_REQUIRED;
}

static enum ifwi_ret ifwi_replace(void)
{
	if (!param.file_name) {
		ERROR("%s: -f option required\n", __func__);
		return COMMAND_ERR;
	}

	if (!param.subpart_name) {
		ERROR("%s: -n option required\n", __func__);
		return COMMAND_ERR;
	}

	int type = find_type_by_name(param.subpart_name);

	if (type == -1)
		return COMMAND_ERR;

	const struct subpart_info *curr_subpart = &subparts[type];

	if (curr_subpart->attr & AUTO_GENERATED) {
		ERROR("Cannot replace auto-generated sub-partitions.\n");
		return COMMAND_ERR;
	}

	if (buffer_size(&ifwi_image.subpart_buf[type]) == 0) {
		ERROR("Image does not contain sub-partition %s(%d).\n",
		      param.subpart_name, type);
		return COMMAND_ERR;
	}

	if (param.dir_ops)
		return ifwi_dir_replace(type);

	return ifwi_raw_replace(type);
}

static enum ifwi_ret ifwi_create(void)
{
	/*
	 * Create peels off any non-IFWI content present in the input buffer and
	 * creates output file with only the IFWI present.
	 */

	if (!param.file_name) {
		ERROR("%s: -f option required\n", __func__);
		return COMMAND_ERR;
	}

	/* Peel off any non-IFWI prefix */
	buffer_seek(&ifwi_image.input_buff,
		    ifwi_image.input_ifwi_start_offset);
	/* Peel off any non-IFWI suffix */
	buffer_set_size(&ifwi_image.input_buff,
			ifwi_image.input_ifwi_end_offset -
			ifwi_image.input_ifwi_start_offset);

	/*
	 * Adjust start and end offset of IFWI now that non-IFWI prefix is gone.
	 */
	ifwi_image.input_ifwi_end_offset -= ifwi_image.input_ifwi_start_offset;
	ifwi_image.input_ifwi_start_offset = 0;

	param.image_name = param.file_name;

	return REPACK_REQUIRED;
}

struct command {
	const char *name;
	const char *optstring;
	enum ifwi_ret (*function)(void);
};

static const struct command commands[] = {
	{"add", "f:n:e:dvh?", ifwi_add},
	{"create", "f:vh?", ifwi_create},
	{"delete", "f:n:vh?", ifwi_delete},
	{"extract", "f:n:e:dvh?", ifwi_extract},
	{"print", "dh?", ifwi_print},
	{"replace", "f:n:e:dvh?", ifwi_replace},
};

static struct option long_options[] = {
	{"subpart_dentry",  required_argument, 0, 'e'},
	{"file",	    required_argument, 0, 'f'},
	{"help",	    required_argument, 0, 'h'},
	{"name",	    required_argument, 0, 'n'},
	{"dir_ops",         no_argument,       0, 'd'},
	{"verbose",	    no_argument,       0, 'v'},
	{NULL,		    0,                 0,  0 }
};

static void usage(const char *name)
{
	printf("ifwitool: Utility for IFWI manipulation\n\n"
	       "USAGE:\n"
	       " %s [-h]\n"
	       " %s FILE COMMAND [PARAMETERS]\n\n"
	       "COMMANDs:\n"
	       " add -f FILE -n NAME [-d -e ENTRY]\n"
	       " create -f FILE\n"
	       " delete -n NAME\n"
	       " extract -f FILE -n NAME [-d -e ENTRY]\n"
	       " print [-d]\n"
	       " replace -f FILE -n NAME [-d -e ENTRY]\n"
	       "OPTIONs:\n"
	       " -f FILE : File to read/write/create/extract\n"
	       " -d      : Perform directory operation\n"
	       " -e ENTRY: Name of directory entry to operate on\n"
	       " -v      : Verbose level\n"
	       " -h      : Help message\n"
	       " -n NAME : Name of sub-partition to operate on\n",
	       name, name
	       );

	printf("\nNAME should be one of:\n");
	int i;

	for (i = 0; i < MAX_SUBPARTS; i++)
		printf("%s(%s)\n", subparts[i].name, subparts[i].readable_name);
	printf("\n");
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	param.image_name = argv[1];
	char *cmd = argv[2];

	optind += 2;

	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(cmd, commands[i].name) != 0)
			continue;

		int c;

		while (1) {
			int option_index;

			c = getopt_long(argc, argv, commands[i].optstring,
					long_options, &option_index);

			if (c == -1)
				break;

			/* Filter out illegal long options */
			if (!strchr(commands[i].optstring, c)) {
				ERROR("%s: invalid option -- '%c'\n", argv[0],
				      c);
				c = '?';
			}

			switch (c) {
			case 'n':
				param.subpart_name = optarg;
				break;
			case 'f':
				param.file_name = optarg;
				break;
			case 'd':
				param.dir_ops = 1;
				break;
			case 'e':
				param.dentry_name = optarg;
				break;
			case 'v':
				verbose++;
				break;
			case 'h':
			case '?':
				usage(argv[0]);
				return 1;
			default:
				break;
			}
		}

		if (ifwi_parse()) {
			ERROR("%s: ifwi parsing failed\n", argv[0]);
			return 1;
		}

		enum ifwi_ret ret = commands[i].function();

		if (ret == COMMAND_ERR) {
			ERROR("%s: failed execution\n", argv[0]);
			return 1;
		}

		if (ret == REPACK_REQUIRED)
			ifwi_repack();

		return 0;
	}

	ERROR("%s: invalid command\n", argv[0]);
	return 1;
}
