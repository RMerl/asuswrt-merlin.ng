/*
 * dict.c	Routines to read the dictionary file.
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */
RCSID("$Id$")

#include	<freeradius-devel/libradius.h>

#ifdef WITH_DHCP
#include	<freeradius-devel/dhcp.h>
#endif

#include	<ctype.h>

#ifdef HAVE_MALLOC_H
#include	<malloc.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif


#define DICT_VALUE_MAX_NAME_LEN (128)
#define DICT_VENDOR_MAX_NAME_LEN (128)
#define DICT_ATTR_MAX_NAME_LEN (128)

#define DICT_ATTR_SIZE sizeof(DICT_ATTR) + DICT_ATTR_MAX_NAME_LEN

static fr_hash_table_t *vendors_byname = NULL;
static fr_hash_table_t *vendors_byvalue = NULL;

static fr_hash_table_t *attributes_byname = NULL;
static fr_hash_table_t *attributes_byvalue = NULL;

static fr_hash_table_t *attributes_combo = NULL;

static fr_hash_table_t *values_byvalue = NULL;
static fr_hash_table_t *values_byname = NULL;

static DICT_ATTR *dict_base_attrs[256];

/*
 *	For faster HUP's, we cache the stat information for
 *	files we've $INCLUDEd
 */
typedef struct dict_stat_t {
	struct dict_stat_t *next;
	char	   	   *name;
	time_t		   mtime;
} dict_stat_t;

static char *stat_root_dir = NULL;
static char *stat_root_file = NULL;

static dict_stat_t *stat_head = NULL;
static dict_stat_t *stat_tail = NULL;

typedef struct value_fixup_t {
	char		attrstr[DICT_ATTR_MAX_NAME_LEN];
	DICT_VALUE	*dval;
	struct value_fixup_t *next;
} value_fixup_t;


/*
 *	So VALUEs in the dictionary can have forward references.
 */
static value_fixup_t *value_fixup = NULL;

const FR_NAME_NUMBER dict_attr_types[] = {
	{ "integer",	PW_TYPE_INTEGER },
	{ "string",	PW_TYPE_STRING },
	{ "ipaddr",	PW_TYPE_IPADDR },
	{ "date",	PW_TYPE_DATE },
	{ "abinary",	PW_TYPE_ABINARY },
	{ "octets",	PW_TYPE_OCTETS },
	{ "ifid",	PW_TYPE_IFID },
	{ "ipv6addr",	PW_TYPE_IPV6ADDR },
	{ "ipv6prefix", PW_TYPE_IPV6PREFIX },
	{ "byte",	PW_TYPE_BYTE },
	{ "short",	PW_TYPE_SHORT },
	{ "ether",	PW_TYPE_ETHERNET },
	{ "combo-ip",	PW_TYPE_COMBO_IP },
	{ "tlv",	PW_TYPE_TLV },
	{ "signed",	PW_TYPE_SIGNED },
	{ "extended",	PW_TYPE_EXTENDED },
	{ "long-extended",	PW_TYPE_LONG_EXTENDED },
	{ "evs",	PW_TYPE_EVS },
	{ "uint8",	PW_TYPE_BYTE },
	{ "uint16",	PW_TYPE_SHORT },
	{ "uint32",	PW_TYPE_INTEGER },
	{ "int32",	PW_TYPE_SIGNED },
	{ "integer64",	PW_TYPE_INTEGER64 },
	{ "uint64",	PW_TYPE_INTEGER64 },
	{ "ipv4prefix", PW_TYPE_IPV4PREFIX },
	{ "cidr", 	PW_TYPE_IPV4PREFIX },
	{ "vsa",	PW_TYPE_VSA },
	{ NULL, 0 }
};

/*
 *	Map data types to min / max data sizes.
 */
const size_t dict_attr_sizes[PW_TYPE_MAX][2] = {
	[PW_TYPE_INVALID]	= { ~0, 0 },
	[PW_TYPE_STRING]	= { 0, ~0 },
	[PW_TYPE_INTEGER]	= {4, 4 },
	[PW_TYPE_IPADDR]	= {4, 4},
	[PW_TYPE_DATE]		= {4, 4},
	[PW_TYPE_ABINARY]	= {32, ~0},
	[PW_TYPE_OCTETS]	= {0, ~0},
	[PW_TYPE_IFID]		= {8, 8},
	[PW_TYPE_IPV6ADDR]	= { 16, 16},
	[PW_TYPE_IPV6PREFIX]	= {2, 18},
	[PW_TYPE_BYTE]		= {1, 1},
	[PW_TYPE_SHORT]		= {2, 2},
	[PW_TYPE_ETHERNET]	= {6, 6},
	[PW_TYPE_SIGNED]	= {4, 4},
	[PW_TYPE_COMBO_IP]	= {4, 16},
	[PW_TYPE_TLV]		= {2, ~0},
	[PW_TYPE_EXTENDED]	= {2, ~0},
	[PW_TYPE_LONG_EXTENDED]	= {3, ~0},
	[PW_TYPE_EVS]		= {6, ~0},
	[PW_TYPE_INTEGER64]	= {8, 8},
	[PW_TYPE_IPV4PREFIX]	= {6, 6},
	[PW_TYPE_VSA]		= {4, ~0}
};

/*
 *	For packing multiple TLV numbers into one 32-bit integer.  The
 *	first 3 bytes are just the 8-bit number.  The next two are
 *	more limited.  We only allow 31 attributes nested 3 layers
 *	deep, and only 7 nested 4 layers deep.  This should be
 *	sufficient for most purposes.
 *
 *	For TLVs and extended attributes, we packet the base attribute
 *	number into the upper 8 bits of the "vendor" field.
 *
 *	e.g.	OID		attribute	vendor
 *		241.1		1		(241 << 24)
 *		241.26.9.1	1		(241 << 24) | (9)
 *		241.1.2		1 | (2 << 8)	(241 << 24)
 */
#define MAX_TLV_NEST (4)
/*
 *	Bit packing:
 *	8 bits of base attribute
 *	8 bits for nested TLV 1
 *	8 bits for nested TLV 2
 *	5 bits for nested TLV 3
 *	3 bits for nested TLV 4
 */
const int fr_attr_max_tlv = MAX_TLV_NEST;
const int fr_attr_shift[MAX_TLV_NEST + 1] = {
  0, 8, 16, 24, 29
};

const int fr_attr_mask[MAX_TLV_NEST + 1] = {
  0xff, 0xff, 0xff, 0x1f, 0x07
};


/*
 *	Create the hash of the name.
 *
 *	We copy the hash function here because it's substantially faster.
 */
#define FNV_MAGIC_INIT (0x811c9dc5)
#define FNV_MAGIC_PRIME (0x01000193)

static uint32_t dict_hashname(char const *name)
{
	uint32_t hash = FNV_MAGIC_INIT;
	char const *p;

	for (p = name; *p != '\0'; p++) {
		int c = *(unsigned char const *) p;
		if (isalpha(c)) c = tolower(c);

		hash *= FNV_MAGIC_PRIME;
		hash ^= (uint32_t ) (c & 0xff);
	}

	return hash;
}


/*
 *	Hash callback functions.
 */
static uint32_t dict_attr_name_hash(void const *data)
{
	return dict_hashname(((DICT_ATTR const *)data)->name);
}

static int dict_attr_name_cmp(void const *one, void const *two)
{
	DICT_ATTR const *a = one;
	DICT_ATTR const *b = two;

	return strcasecmp(a->name, b->name);
}

static uint32_t dict_attr_value_hash(void const *data)
{
	uint32_t hash;
	DICT_ATTR const *attr = data;

	hash = fr_hash(&attr->vendor, sizeof(attr->vendor));
	return fr_hash_update(&attr->attr, sizeof(attr->attr), hash);
}

static int dict_attr_value_cmp(void const *one, void const *two)
{
	DICT_ATTR const *a = one;
	DICT_ATTR const *b = two;

	if (a->vendor < b->vendor) return -1;
	if (a->vendor > b->vendor) return +1;

	return a->attr - b->attr;
}

static uint32_t dict_attr_combo_hash(void const *data)
{
	uint32_t hash;
	DICT_ATTR const *attr = data;

	hash = fr_hash(&attr->vendor, sizeof(attr->vendor));
	hash = fr_hash_update(&attr->type, sizeof(attr->type), hash);
	return fr_hash_update(&attr->attr, sizeof(attr->attr), hash);
}

static int dict_attr_combo_cmp(void const *one, void const *two)
{
	DICT_ATTR const *a = one;
	DICT_ATTR const *b = two;

	if (a->type < b->type) return -1;
	if (a->type > b->type) return +1;

	if (a->vendor < b->vendor) return -1;
	if (a->vendor > b->vendor) return +1;

	return a->attr - b->attr;
}

static uint32_t dict_vendor_name_hash(void const *data)
{
	return dict_hashname(((DICT_VENDOR const *)data)->name);
}

static int dict_vendor_name_cmp(void const *one, void const *two)
{
	DICT_VENDOR const *a = one;
	DICT_VENDOR const *b = two;

	return strcasecmp(a->name, b->name);
}

static uint32_t dict_vendor_value_hash(void const *data)
{
	return fr_hash(&(((DICT_VENDOR const *)data)->vendorpec),
			 sizeof(((DICT_VENDOR const *)data)->vendorpec));
}

static int dict_vendor_value_cmp(void const *one, void const *two)
{
	DICT_VENDOR const *a = one;
	DICT_VENDOR const *b = two;

	return a->vendorpec - b->vendorpec;
}

static uint32_t dict_value_name_hash(void const *data)
{
	uint32_t hash;
	DICT_VALUE const *dval = data;

	hash = dict_hashname(dval->name);
	hash = fr_hash_update(&dval->vendor, sizeof(dval->vendor), hash);
	return fr_hash_update(&dval->attr, sizeof(dval->attr), hash);
}

static int dict_value_name_cmp(void const *one, void const *two)
{
	int rcode;
	DICT_VALUE const *a = one;
	DICT_VALUE const *b = two;

	rcode = a->attr - b->attr;
	if (rcode != 0) return rcode;

	rcode = a->vendor - b->vendor;
	if (rcode != 0) return rcode;

	return strcasecmp(a->name, b->name);
}

static uint32_t dict_value_value_hash(void const *data)
{
	uint32_t hash;
	DICT_VALUE const *dval = data;

	hash = fr_hash(&dval->attr, sizeof(dval->attr));
	hash = fr_hash_update(&dval->vendor, sizeof(dval->vendor), hash);
	return fr_hash_update(&dval->value, sizeof(dval->value), hash);
}

static int dict_value_value_cmp(void const *one, void const *two)
{
	int rcode;
	DICT_VALUE const *a = one;
	DICT_VALUE const *b = two;

	if (a->vendor < b->vendor) return -1;
	if (a->vendor > b->vendor) return +1;

	rcode = a->attr - b->attr;
	if (rcode != 0) return rcode;

	return a->value - b->value;
}


/*
 *	Free the list of stat buffers
 */
static void dict_stat_free(void)
{
	dict_stat_t *this, *next;

	free(stat_root_dir);
	stat_root_dir = NULL;
	free(stat_root_file);
	stat_root_file = NULL;

	if (!stat_head) {
		stat_tail = NULL;
		return;
	}

	for (this = stat_head; this != NULL; this = next) {
		next = this->next;
		free(this->name);
		free(this);
	}

	stat_head = stat_tail = NULL;
}


/*
 *	Add an entry to the list of stat buffers.
 */
static void dict_stat_add(char const *name, struct stat const *stat_buf)
{
	dict_stat_t *this;

	this = malloc(sizeof(*this));
	if (!this) return;
	memset(this, 0, sizeof(*this));

	this->name = strdup(name);
	this->mtime = stat_buf->st_mtime;

	if (!stat_head) {
		stat_head = stat_tail = this;
	} else {
		stat_tail->next = this;
		stat_tail = this;
	}
}


/*
 *	See if any dictionaries have changed.  If not, don't
 *	do anything.
 */
static int dict_stat_check(char const *root_dir, char const *root_file)
{
	struct stat buf;
	dict_stat_t *this;

	if (!stat_root_dir) return 0;
	if (!stat_root_file) return 0;

	if (strcmp(root_dir, stat_root_dir) != 0) return 0;
	if (strcmp(root_file, stat_root_file) != 0) return 0;

	if (!stat_head) return 0; /* changed, reload */

	for (this = stat_head; this != NULL; this = this->next) {
		if (stat(this->name, &buf) < 0) return 0;

		if (buf.st_mtime != this->mtime) return 0;
	}

	return 1;
}

typedef struct fr_pool_t {
	void	*page_end;
	void	*free_ptr;
	struct fr_pool_t *page_free;
	struct fr_pool_t *page_next;
} fr_pool_t;

#define FR_POOL_SIZE (32768)
#define FR_ALLOC_ALIGN (8)

static fr_pool_t *dict_pool = NULL;

static fr_pool_t *fr_pool_create(void)
{
	fr_pool_t *fp = malloc(FR_POOL_SIZE);

	if (!fp) return NULL;

	memset(fp, 0, FR_POOL_SIZE);

	fp->page_end = ((uint8_t *) fp) + FR_POOL_SIZE;
	fp->free_ptr = ((uint8_t *) fp) + sizeof(*fp);
	fp->page_free = fp;
	fp->page_next = NULL;
	return fp;
}

static void fr_pool_delete(fr_pool_t **pfp)
{
	fr_pool_t *fp, *next;

	if (!pfp || !*pfp) return;

	for (fp = *pfp; fp != NULL; fp = next) {
		next = fp->page_next;
		fp->page_next = NULL;
		free(fp);
	}
	*pfp = NULL;
}


static void *fr_pool_alloc(size_t size)
{
	void *ptr;

	if (size == 0) return NULL;

	if (size > 256) return NULL; /* shouldn't happen */

	if (!dict_pool) {
		dict_pool = fr_pool_create();
		if (!dict_pool) return NULL;
	}

	if ((size & (FR_ALLOC_ALIGN - 1)) != 0) {
		size += FR_ALLOC_ALIGN - (size & (FR_ALLOC_ALIGN - 1));
	}

	if ((((uint8_t *) dict_pool->page_free->free_ptr) + size) > (uint8_t *) dict_pool->page_free->page_end) {
		dict_pool->page_free->page_next = fr_pool_create();
		if (!dict_pool->page_free->page_next) return NULL;
		dict_pool->page_free = dict_pool->page_free->page_next;
	}

	ptr = dict_pool->page_free->free_ptr;
	dict_pool->page_free->free_ptr = ((uint8_t *) dict_pool->page_free->free_ptr) + size;

	return ptr;
}


static void fr_pool_free(UNUSED void *ptr)
{
	/*
	 *	Place-holder for later code.
	 */
}

/*
 *	Free the dictionary_attributes and dictionary_values lists.
 */
void dict_free(void)
{
	/*
	 *	Free the tables
	 */
	fr_hash_table_free(vendors_byname);
	fr_hash_table_free(vendors_byvalue);
	vendors_byname = NULL;
	vendors_byvalue = NULL;

	fr_hash_table_free(attributes_byname);
	fr_hash_table_free(attributes_byvalue);
	fr_hash_table_free(attributes_combo);
	attributes_byname = NULL;
	attributes_byvalue = NULL;
	attributes_combo = NULL;

	fr_hash_table_free(values_byname);
	fr_hash_table_free(values_byvalue);
	values_byname = NULL;
	values_byvalue = NULL;

	memset(dict_base_attrs, 0, sizeof(dict_base_attrs));

	fr_pool_delete(&dict_pool);

	dict_stat_free();
}

/*
 *	Add vendor to the list.
 */
int dict_addvendor(char const *name, unsigned int value)
{
	size_t length;
	DICT_VENDOR *dv;

	if (value >= FR_MAX_VENDOR) {
		fr_strerror_printf("dict_addvendor: Cannot handle vendor ID larger than 2^24");
		return -1;
	}

	if ((length = strlen(name)) >= DICT_VENDOR_MAX_NAME_LEN) {
		fr_strerror_printf("dict_addvendor: vendor name too long");
		return -1;
	}

	if ((dv = fr_pool_alloc(sizeof(*dv) + length)) == NULL) {
		fr_strerror_printf("dict_addvendor: out of memory");
		return -1;
	}

	strcpy(dv->name, name);
	dv->vendorpec  = value;
	dv->type = dv->length = 1; /* defaults */

	if (!fr_hash_table_insert(vendors_byname, dv)) {
		DICT_VENDOR *old_dv;

		old_dv = fr_hash_table_finddata(vendors_byname, dv);
		if (!old_dv) {
			fr_strerror_printf("dict_addvendor: Failed inserting vendor name %s", name);
			return -1;
		}
		if (old_dv->vendorpec != dv->vendorpec) {
			fr_strerror_printf("dict_addvendor: Duplicate vendor name %s", name);
			return -1;
		}

		/*
		 *	Already inserted.  Discard the duplicate entry.
		 */
		fr_pool_free(dv);
		return 0;
	}

	/*
	 *	Insert the SAME pointer (not free'd when this table is
	 *	deleted), into another table.
	 *
	 *	We want this behaviour because we want OLD names for
	 *	the attributes to be read from the configuration
	 *	files, but when we're printing them, (and looking up
	 *	by value) we want to use the NEW name.
	 */
	if (!fr_hash_table_replace(vendors_byvalue, dv)) {
		fr_strerror_printf("dict_addvendor: Failed inserting vendor %s",
			   name);
		return -1;
	}

	return 0;
}


/*
 *	[a-zA-Z0-9_-:.]+
 */
const int dict_attr_allowed_chars[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/*
 *	Add an attribute to the dictionary.
 */
int dict_addattr(char const *name, int attr, unsigned int vendor, int type,
		 ATTR_FLAGS flags)
{
	size_t namelen;
	static int      max_attr = 0;
	uint8_t const *p;
	DICT_ATTR const	*da;
	DICT_ATTR *n;

	namelen = strlen(name);
	if (namelen >= DICT_ATTR_MAX_NAME_LEN) {
		fr_strerror_printf("dict_addattr: attribute name too long");
		return -1;
	}

	for (p = (uint8_t const *) name; *p != '\0'; p++) {
		if (!dict_attr_allowed_chars[*p]) {
			fr_strerror_printf("dict_addattr: Invalid character '%c' in attribute name", *p);
			return -1;
		}
	}

	if (flags.has_tag &&
	    !((type == PW_TYPE_INTEGER) || (type == PW_TYPE_STRING))) {
		fr_strerror_printf("dict_addattr: Only 'integer' and 'string' attributes can have tags");
		return -1;
	}


	/*
	 *	If the attr is '-1', that means use a pre-existing
	 *	one (if it already exists).  If one does NOT already exist,
	 *	then create a new attribute, with a non-conflicting value,
	 *	and use that.
	 */
	if (attr == -1) {
		if (dict_attrbyname(name)) {
			return 0; /* exists, don't add it again */
		}

		attr = ++max_attr;

	} else if (vendor == 0) {
		/*
		 *  Update 'max_attr'
		 */
		if (attr > max_attr) {
			max_attr = attr;
		}
	}

	/*
	 *	Additional checks for extended attributes.
	 */
	if (flags.extended || flags.long_extended || flags.evs) {
		if (vendor && (vendor < FR_MAX_VENDOR)) {
			fr_strerror_printf("dict_addattr: VSAs cannot use the \"extended\" or \"evs\" attribute formats.");
			return -1;
		}
		if (flags.has_tag
#ifdef WITH_DHCP
		    || flags.array
#endif
		    || (flags.encrypt != FLAG_ENCRYPT_NONE)) {
			fr_strerror_printf("dict_addattr: The \"extended\" attributes MUST NOT have any flags set.");
			return -1;
		}
	}

	if (flags.evs) {
		if (!(flags.extended || flags.long_extended)) {
			fr_strerror_printf("dict_addattr: Attributes of type \"evs\" MUST have a parent of type \"extended\"");
			return -1;
		}

		/* VSAs cannot be of format EVS */
		if ((vendor & (FR_MAX_VENDOR - 1)) != 0) {
			fr_strerror_printf("dict_addattr: Attribute of type \"evs\" fails internal sanity check");
			return -1;
		}
	}

	/*
	 *	Allow for generic pointers
	 */
	switch (type) {
	default:
		break;

	case PW_TYPE_STRING:
	case PW_TYPE_OCTETS:
	case PW_TYPE_TLV:
		flags.is_pointer = true;
		break;
	}

	if (attr < 0) {
		fr_strerror_printf("dict_addattr: ATTRIBUTE has invalid number (less than zero)");
		return -1;
	}

	if (flags.has_tlv && flags.length) {
		fr_strerror_printf("TLVs cannot have a fixed length");
		return -1;
	}

	if (vendor && flags.concat) {
		fr_strerror_printf("VSAs cannot have the \"concat\" flag set.");
		return -1;
	}

	if (flags.concat && (type != PW_TYPE_OCTETS)) {
		fr_strerror_printf("The \"concat\" flag can only be set for attributes of type \"octets\".");
		return -1;
	}

	if (flags.concat && (flags.has_tag || flags.array || flags.is_tlv || flags.has_tlv ||
			     flags.length || flags.evs || flags.extended || flags.long_extended ||
			     (flags.encrypt != FLAG_ENCRYPT_NONE))) {
		fr_strerror_printf("The \"concat\" flag cannot be used with any other flag.");
		return -1;
	}

	if ((vendor & (FR_MAX_VENDOR -1)) != 0) {
		DICT_VENDOR *dv;
		static DICT_VENDOR *last_vendor = NULL;

		if (flags.has_tlv && (flags.encrypt != FLAG_ENCRYPT_NONE)) {
			fr_strerror_printf("TLV's cannot be encrypted");
			return -1;
		}

		if (flags.is_tlv && flags.has_tag) {
			fr_strerror_printf("Sub-TLV's cannot have a tag");
			return -1;
		}

		if (flags.has_tlv && flags.has_tag) {
			fr_strerror_printf("TLV's cannot have a tag");
			return -1;
		}

		/*
		 *	Most ATTRIBUTEs are bunched together by
		 *	VENDOR.  We can save a lot of lookups on
		 *	dictionary initialization by caching the last
		 *	vendor.
		 */
		if (last_vendor &&
		    ((vendor & (FR_MAX_VENDOR - 1)) == last_vendor->vendorpec)) {
			dv = last_vendor;
		} else {
			/*
			 *	Ignore the high byte (sigh)
			 */
			dv = dict_vendorbyvalue(vendor & (FR_MAX_VENDOR - 1));
			last_vendor = dv;
		}

		/*
		 *	If the vendor isn't defined, die.
		 */
		if (!dv) {
			fr_strerror_printf("dict_addattr: Unknown vendor %u",
					   vendor & (FR_MAX_VENDOR - 1));
			return -1;
		}

		/*
		 *	FIXME: Switch over dv->type, and limit things
		 *	properly.
		 */
		if ((dv->type == 1) && (attr >= 256) && !flags.is_tlv) {
			fr_strerror_printf("dict_addattr: ATTRIBUTE has invalid number (larger than 255).");
			return -1;
		} /* else 256..65535 are allowed */

		/*
		 *	If the attribute is in the standard space, AND
		 *	has a sub-type (e.g. 241.1 or 255.3), then its
		 *	number is placed into the upper 8 bits of the
		 *	vendor field.
		 *
		 *	This also happens for the new VSAs.
		 *
		 *	If we find it, then set the various flags
		 *	based on what we see.
		 */
		if (vendor >= FR_MAX_VENDOR) {
			unsigned int parent;

			parent = (vendor / FR_MAX_VENDOR) & 0xff;

			da = dict_attrbyvalue(parent, 0);
			if (!da) {
				fr_strerror_printf("dict_addattr: ATTRIBUTE refers to unknown parent attribute %u.", parent);
				return -1;
			}

			/*
			 *	These flags are inherited from the
			 *	parent.
			 */
			flags.extended = da->flags.extended;
			flags.long_extended = da->flags.long_extended;

			/*
			 *	Non-extended attributes can't have VSAs.
			 */
			if (!flags.extended &&
			    ((vendor & (FR_MAX_VENDOR - 1)) != 0)) {
				fr_strerror_printf("dict_addattr: ATTRIBUTE cannot be a VSA");
				return -1;
			}

			if ((vendor & (FR_MAX_VENDOR - 1)) != 0) {
				flags.evs = 1;
			}
		}

		/*
		 *	<sigh> Alvarion, being *again* a horribly
		 *	broken vendor, has re-used the WiMAX format in
		 *	their proprietary vendor space.  This re-use
		 *	means that there are *multiple* conflicting
		 *	Alvarion dictionaries.
		 */
		flags.wimax = dv->flags;
	}

	/*
	 *	Create a new attribute for the list
	 */
	if ((n = fr_pool_alloc(sizeof(*n) + namelen)) == NULL) {
	oom:
		fr_strerror_printf("dict_adnttr: out of memory");
		return -1;
	}

	memcpy(n->name, name, namelen);
	n->name[namelen] = '\0';
	n->attr = attr;
	n->vendor = vendor;
	n->type = type;
	n->flags = flags;

	/*
	 *	Insert the attribute, only if it's not a duplicate.
	 */
	if (!fr_hash_table_insert(attributes_byname, n)) {
		DICT_ATTR	*a;

		/*
		 *	If the attribute has identical number, then
		 *	ignore the duplicate.
		 */
		a = fr_hash_table_finddata(attributes_byname, n);
		if (a && (strcasecmp(a->name, n->name) == 0)) {
			if (a->attr != n->attr) {
				fr_strerror_printf("dict_adnttr: Duplicate attribute name %s", name);
				fr_pool_free(n);
				return -1;
			}

			/*
			 *	Same name, same vendor, same attr,
			 *	maybe the flags and/or type is
			 *	different.  Let the new value
			 *	over-ride the old one.
			 */
		}


		fr_hash_table_delete(attributes_byvalue, a);

		if (!fr_hash_table_replace(attributes_byname, n)) {
			fr_strerror_printf("dict_adnttr: Internal error storing attribute %s", name);
			fr_pool_free(n);
			return -1;
		}
	}

	/*
	 *	Insert the SAME pointer (not free'd when this entry is
	 *	deleted), into another table.
	 *
	 *	We want this behaviour because we want OLD names for
	 *	the attributes to be read from the configuration
	 *	files, but when we're printing them, (and looking up
	 *	by value) we want to use the NEW name.
	 */
	if (!fr_hash_table_replace(attributes_byvalue, n)) {
		fr_strerror_printf("dict_adnttr: Failed inserting attribute name %s", name);
		return -1;
	}

	/*
	 *	Hacks for combo-IP
	 */
	if (n->type == PW_TYPE_COMBO_IP) {
		DICT_ATTR *v4, *v6;

		v4 = fr_pool_alloc(sizeof(*v4));
		if (!v4) goto oom;

		v6 = fr_pool_alloc(sizeof(*v6));
		if (!v6) {
			free(v4);
			goto oom;
		}

		memcpy(v4, n, sizeof(*v4));
		v4->type = PW_TYPE_IPADDR;

		memcpy(v6, n, sizeof(*v6));
		v6->type = PW_TYPE_IPV6ADDR;

		if (!fr_hash_table_insert(attributes_combo, v4)) {
			fr_strerror_printf("dict_addattr: Failed inserting attribute name %s - IPv4", name);
			free(v4);
			free(v6);
			return -1;
		}

		if (!fr_hash_table_insert(attributes_combo, v6)) {
			fr_strerror_printf("dict_addattr: Failed inserting attribute name %s - IPv6", name);
			free(v6);
			return -1;
		}
	}

	if (!vendor && (attr > 0) && (attr < 256)) {
		 dict_base_attrs[attr] = n;
	}

	return 0;
}


/*
 *	Add a value for an attribute to the dictionary.
 */
int dict_addvalue(char const *namestr, char const *attrstr, int value)
{
	size_t		length;
	DICT_ATTR const	*dattr;
	DICT_VALUE	*dval;

	static DICT_ATTR const *last_attr = NULL;

	if (!*namestr) {
		fr_strerror_printf("dict_addvalue: empty names are not permitted");
		return -1;
	}

	if ((length = strlen(namestr)) >= DICT_VALUE_MAX_NAME_LEN) {
		fr_strerror_printf("dict_addvalue: value name too long");
		return -1;
	}

	if ((dval = fr_pool_alloc(sizeof(*dval) + length)) == NULL) {
		fr_strerror_printf("dict_addvalue: out of memory");
		return -1;
	}
	memset(dval, 0, sizeof(*dval));

	strcpy(dval->name, namestr);
	dval->value = value;

	/*
	 *	Most VALUEs are bunched together by ATTRIBUTE.  We can
	 *	save a lot of lookups on dictionary initialization by
	 *	caching the last attribute.
	 */
	if (last_attr && (strcasecmp(attrstr, last_attr->name) == 0)) {
		dattr = last_attr;
	} else {
		dattr = dict_attrbyname(attrstr);
		last_attr = dattr;
	}

	/*
	 *	Remember which attribute is associated with this
	 *	value, if possible.
	 */
	if (dattr) {
		if (dattr->flags.has_value_alias) {
			fr_strerror_printf("dict_addvalue: Cannot add VALUE for ATTRIBUTE \"%s\": It already has a VALUE-ALIAS", attrstr);
			return -1;
		}

		dval->attr = dattr->attr;
		dval->vendor = dattr->vendor;

		/*
		 *	Enforce valid values
		 *
		 *	Don't worry about fixups...
		 */
		switch (dattr->type) {
			case PW_TYPE_BYTE:
				if (value > 255) {
					fr_pool_free(dval);
					fr_strerror_printf("dict_addvalue: ATTRIBUTEs of type 'byte' cannot have VALUEs larger than 255");
					return -1;
				}
				break;
			case PW_TYPE_SHORT:
				if (value > 65535) {
					fr_pool_free(dval);
					fr_strerror_printf("dict_addvalue: ATTRIBUTEs of type 'short' cannot have VALUEs larger than 65535");
					return -1;
				}
				break;

				/*
				 *	Allow octets for now, because
				 *	of dictionary.cablelabs
				 */
			case PW_TYPE_OCTETS:

			case PW_TYPE_INTEGER:
				break;

			case PW_TYPE_INTEGER64:
			default:
				fr_pool_free(dval);
				fr_strerror_printf("dict_addvalue: VALUEs cannot be defined for attributes of type '%s'",
					   fr_int2str(dict_attr_types, dattr->type, "?Unknown?"));
				return -1;
		}
	} else {
		value_fixup_t *fixup;

		fixup = (value_fixup_t *) malloc(sizeof(*fixup));
		if (!fixup) {
			fr_pool_free(dval);
			fr_strerror_printf("dict_addvalue: out of memory");
			return -1;
		}
		memset(fixup, 0, sizeof(*fixup));

		strlcpy(fixup->attrstr, attrstr, sizeof(fixup->attrstr));
		fixup->dval = dval;

		/*
		 *	Insert to the head of the list.
		 */
		fixup->next = value_fixup;
		value_fixup = fixup;

		return 0;
	}

	/*
	 *	Add the value into the dictionary.
	 */
	{
		DICT_ATTR *tmp;
		memcpy(&tmp, &dval, sizeof(tmp));

		if (!fr_hash_table_insert(values_byname, tmp)) {
			if (dattr) {
				DICT_VALUE *old;

				/*
				 *	Suppress duplicates with the same
				 *	name and value.  There are lots in
				 *	dictionary.ascend.
				 */
				old = dict_valbyname(dattr->attr, dattr->vendor, namestr);
				if (old && (old->value == dval->value)) {
					fr_pool_free(dval);
					return 0;
				}
			}

			fr_pool_free(dval);
			fr_strerror_printf("dict_addvalue: Duplicate value name %s for attribute %s", namestr, attrstr);
			return -1;
		}
	}

	/*
	 *	There are multiple VALUE's, keyed by attribute, so we
	 *	take care of that here.
	 */
	if (!fr_hash_table_replace(values_byvalue, dval)) {
		fr_strerror_printf("dict_addvalue: Failed inserting value %s",
			   namestr);
		return -1;
	}

	return 0;
}

static int sscanf_i(char const *str, unsigned int *pvalue)
{
	int rcode = 0;
	int base = 10;
	static char const *tab = "0123456789";

	if ((str[0] == '0') &&
	    ((str[1] == 'x') || (str[1] == 'X'))) {
		tab = "0123456789abcdef";
		base = 16;

		str += 2;
	}

	while (*str) {
		char const *c;

		if (*str == '.') break;

		c = memchr(tab, tolower((int) *str), base);
		if (!c) return 0;

		rcode *= base;
		rcode += (c - tab);
		str++;
	}

	*pvalue = rcode;
	return 1;
}


/*
 *	Get the OID based on various pieces of information.
 *
 *	Remember, the packing format is weird.
 *
 *	00VID	000000AA	normal VSA for vendor VID
 *	00VID	AABBCCDD	normal VSAs with TLVs
 *	EE000   000000AA	extended attr (241.1)
 *	EE000	AABBCCDD	extended attr with TLVs
 *	EEVID	000000AA	EVS with vendor VID, attr AAA
 *	EEVID	AABBCCDD	EVS with TLVs
 *
 *	<whew>!  Are we crazy, or what?
 */
int dict_str2oid(char const *ptr, unsigned int *pvalue, unsigned int *pvendor,
		 int tlv_depth)
{
	char const *p;
	unsigned int value;
	DICT_ATTR const *da = NULL;

	if (tlv_depth > fr_attr_max_tlv) {
		fr_strerror_printf("Too many sub-attributes");
		return -1;
	}

	/*
	 *	If *pvalue is set, check if the attribute exists.
	 *	Otherwise, check that the vendor exists.
	 */
	if (*pvalue) {
		da = dict_attrbyvalue(*pvalue, *pvendor);
		if (!da) {
			fr_strerror_printf("Parent attribute is undefined.");
			return -1;
		}

		if (!da->flags.has_tlv && !da->flags.extended) {
			fr_strerror_printf("Parent attribute %s cannot have sub-attributes",
					   da->name);
			return -1;
		}

	} else if ((*pvendor & (FR_MAX_VENDOR - 1)) != 0) {
		if (!dict_vendorbyvalue(*pvendor & (FR_MAX_VENDOR - 1))) {
			fr_strerror_printf("Unknown vendor %u",
					   *pvendor & (FR_MAX_VENDOR - 1));
			return -1;
		}
	}

	p = strchr(ptr, '.');

	/*
	 *	Look for 26.VID.x.y
	 *
	 *	If we find it, re-write the parameters, and recurse.
	 */
	if (!*pvendor && (tlv_depth == 0) && (*pvalue == PW_VENDOR_SPECIFIC)) {
		DICT_VENDOR const *dv;

		if (!p) {
			fr_strerror_printf("VSA needs to have sub-attribute");
			return -1;
		}

		if (!sscanf_i(ptr, pvendor)) {
			fr_strerror_printf("Invalid number in attribute");
			return -1;
		}

		if (*pvendor >= FR_MAX_VENDOR) {
			fr_strerror_printf("Cannot handle vendor ID larger than 2^24");

			return -1;
		}

		dv = dict_vendorbyvalue(*pvendor & (FR_MAX_VENDOR - 1));
		if (!dv) {
			fr_strerror_printf("Unknown vendor \"%u\" ",
					   *pvendor  & (FR_MAX_VENDOR - 1));
			return -1;
		}

		/*
		 *	Start off with (attr=0, vendor=VID), and
		 *	recurse.  This causes the various checks above
		 *	to be done.
		 */
		*pvalue = 0;
		return dict_str2oid(p + 1, pvalue, pvendor, 0);
	}

	if (!sscanf_i(ptr, &value)) {
		fr_strerror_printf("Invalid number in attribute");
		return -1;
	}

	if (!*pvendor && (tlv_depth == 1) && da &&
	    (da->flags.has_tlv || da->flags.extended)) {


		*pvendor = *pvalue * FR_MAX_VENDOR;
		*pvalue = value;

		if (!p) return 0;
		return dict_str2oid(p + 1, pvalue, pvendor, 1);
	}

	/*
	 *	And pack the data according to the scheme described in
	 *	the comments at the start of this function.
	 */
	if (*pvalue) {
		*pvalue |= (value & fr_attr_mask[tlv_depth]) << fr_attr_shift[tlv_depth];
	} else {
		*pvalue = value;
	}

	if (p) {
		return dict_str2oid(p + 1, pvalue, pvendor, tlv_depth + 1);
	}

	return tlv_depth;
}

/*
 *	Bamboo skewers under the fingernails in 5, 4, 3, 2, ...
 */
static DICT_ATTR const *dict_parent(unsigned int attr, unsigned int vendor)
{
	if (vendor < FR_MAX_VENDOR) {
		return dict_attrbyvalue(attr & 0xff, vendor);
	}

	if (attr < 256) {
		return dict_attrbyvalue((vendor / FR_MAX_VENDOR) & 0xff, 0);
	}

	return dict_attrbyvalue(attr & 0xff, vendor);
}


/*
 *	Process the ATTRIBUTE command
 */
static int process_attribute(char const* fn, int const line,
			     unsigned int block_vendor,
			     DICT_ATTR const *block_tlv, int tlv_depth,
			     char **argv, int argc)
{
	int		oid = 0;
	unsigned int    vendor = 0;
	unsigned int	value;
	int		type;
	unsigned int	length = 0;
	ATTR_FLAGS	flags;
	char		*p;

	if ((argc < 3) || (argc > 4)) {
		fr_strerror_printf("dict_init: %s[%d]: invalid ATTRIBUTE line",
			fn, line);
		return -1;
	}

	/*
	 *	Dictionaries need to have real names, not shitty ones.
	 */
	if (strncmp(argv[1], "Attr-", 5) == 0) {
		fr_strerror_printf("dict_init: %s[%d]: Invalid attribute name",
				   fn, line);
		return -1;
	}

	memset(&flags, 0, sizeof(flags));

	/*
	 *	Look for OIDs before doing anything else.
	 */
	p = strchr(argv[1], '.');
	if (p) oid = 1;

	/*
	 *	Validate all entries
	 */
	if (!sscanf_i(argv[1], &value)) {
		fr_strerror_printf("dict_init: %s[%d]: invalid value", fn, line);
		return -1;
	}

	if (oid) {
		DICT_ATTR const *da;

		vendor = block_vendor;

		/*
		 *	Parse the rest of the OID.
		 */
		if (dict_str2oid(p + 1, &value, &vendor, tlv_depth + 1) < 0) {
			char buffer[256];

			strlcpy(buffer, fr_strerror(), sizeof(buffer));

			fr_strerror_printf("dict_init: %s[%d]: Invalid attribute identifier: %s", fn, line, buffer);
			return -1;
		}
		block_vendor = vendor;

		/*
		 *	Set the flags based on the parents flags.
		 */
		da = dict_parent(value, vendor);
		if (!da) {
			fr_strerror_printf("dict_init: %s[%d]: Parent attribute is undefined.", fn, line);
			return -1;
		}

		flags.extended = da->flags.extended;
		flags.long_extended = da->flags.long_extended;
		flags.evs = da->flags.evs;
		if (da->flags.has_tlv) flags.is_tlv = 1;
	}

	if (strncmp(argv[2], "octets[", 7) != 0) {
		/*
		 *	find the type of the attribute.
		 */
		type = fr_str2int(dict_attr_types, argv[2], -1);
		if (type < 0) {
			fr_strerror_printf("dict_init: %s[%d]: invalid type \"%s\"",
					   fn, line, argv[2]);
			return -1;
		}

	} else {
		type = PW_TYPE_OCTETS;

		p = strchr(argv[2] + 7, ']');
		if (!p) {
			fr_strerror_printf("dict_init: %s[%d]: Invalid format for octets", fn, line);
			return -1;
		}

		*p = 0;

		if (!sscanf_i(argv[1], &length)) {
			fr_strerror_printf("dict_init: %s[%d]: invalid length", fn, line);
			return -1;
		}

		if ((length == 0) || (length > 253)) {
			fr_strerror_printf("dict_init: %s[%d]: invalid length", fn, line);
			return -1;
		}
	}

	/*
	 *	Only look up the vendor if the string
	 *	is non-empty.
	 */
	if (argc < 4) {
		/*
		 *	Force "length" for data types of fixed length;
		 */
		switch (type) {
		case PW_TYPE_BYTE:
			length = 1;
			break;

		case PW_TYPE_SHORT:
			length = 2;
			break;

		case PW_TYPE_DATE:
		case PW_TYPE_IPADDR:
		case PW_TYPE_INTEGER:
		case PW_TYPE_SIGNED:
			length = 4;
			break;

		case PW_TYPE_INTEGER64:
			length = 8;
			break;

		case PW_TYPE_ETHERNET:
			length = 6;
			break;

		case PW_TYPE_IFID:
			length = 8;
			break;

		case PW_TYPE_IPV6ADDR:
			length = 16;
			break;

		case PW_TYPE_EXTENDED:
			if ((vendor != 0) || (value < 241)) {
				fr_strerror_printf("dict_init: %s[%d]: Attributes of type \"extended\" MUST be RFC attributes with value >= 241.", fn, line);
				return -1;
			}
			flags.extended = 1;
			break;

		case PW_TYPE_LONG_EXTENDED:
			if ((vendor != 0) || (value < 241)) {
				fr_strerror_printf("dict_init: %s[%d]: Attributes of type \"long-extended\" MUST be RFC attributes with value >= 241.", fn, line);
				return -1;
			}
			flags.extended = 1;
			flags.long_extended = 1;
			break;

		case PW_TYPE_EVS:
			flags.extended = 1;
			flags.evs = 1;
			if (value != PW_VENDOR_SPECIFIC) {
				fr_strerror_printf("dict_init: %s[%d]: Attributes of type \"evs\" MUST have attribute code 26.", fn, line);
				return -1;
			}
			break;

		default:
			break;
		}

	  	flags.length = length;

	} else {		/* argc == 4: we have options */
		char *key, *next, *last;

		/*
		 *	Keep it real.
		 */
		if (flags.extended) {
			fr_strerror_printf("dict_init: %s[%d]: Extended attributes cannot use flags", fn, line);
			return -1;
		}

		if (length != 0) {
			fr_strerror_printf("dict_init: %s[%d]: length cannot be used with options", fn, line);
			return -1;
		}

		key = argv[3];
		do {
			next = strchr(key, ',');
			if (next) *(next++) = '\0';

			if (strcmp(key, "has_tag") == 0 ||
			    strcmp(key, "has_tag=1") == 0) {
				/* Boolean flag, means this is a
				   tagged attribute */
				flags.has_tag = 1;

			} else if (strncmp(key, "encrypt=", 8) == 0) {
				/* Encryption method, defaults to 0 (none).
				   Currently valid is just type 2,
				   Tunnel-Password style, which can only
				   be applied to strings. */
				flags.encrypt = strtol(key + 8, &last, 0);
				if (*last) {
					fr_strerror_printf( "dict_init: %s[%d] invalid option %s",
						    fn, line, key);
					return -1;
				}

				if ((flags.encrypt == FLAG_ENCRYPT_ASCEND_SECRET) &&
				    (type != PW_TYPE_STRING)) {
					fr_strerror_printf( "dict_init: %s[%d] Only \"string\" types can have the \"encrypt=3\" flag set.",
							    fn, line);
					return -1;
				}

			} else if (strncmp(key, "array", 6) == 0) {
				flags.array = 1;

				switch (type) {
					case PW_TYPE_IPADDR:
					case PW_TYPE_BYTE:
					case PW_TYPE_SHORT:
					case PW_TYPE_INTEGER:
					case PW_TYPE_DATE:
						break;

					default:
						fr_strerror_printf( "dict_init: %s[%d] Only IP addresses can have the \"array\" flag set.",
							    fn, line);
						return -1;
				}

			} else if (strncmp(key, "concat", 6) == 0) {
				flags.concat = 1;

				if (type != PW_TYPE_OCTETS) {
						fr_strerror_printf( "dict_init: %s[%d] Only \"octets\" type can have the \"concat\" flag set.",
							    fn, line);
						return -1;
				}

				/*
				 *	The only thing is the vendor name,
				 *	and it's a known name: allow it.
				 */
			} else if ((key == argv[3]) && !next) {
				if (oid) {
					fr_strerror_printf( "dict_init: %s[%d] New-style attributes cannot use a vendor flag.",
							    fn, line);
					return -1;
				}

				if (block_vendor) {
					fr_strerror_printf( "dict_init: %s[%d] Vendor flag inside of \"BEGIN-VENDOR\" is not allowed.",
							    fn, line);
					return -1;
				}

				vendor = dict_vendorbyname(key);
				if (!vendor) goto unknown;
				break;

			} else {
			unknown:
				fr_strerror_printf( "dict_init: %s[%d]: unknown option \"%s\"",
					    fn, line, key);
				return -1;
			}

			key = next;
			if (key && !*key) break;
		} while (key);
	}

	if (block_vendor) vendor = block_vendor;

	/*
	 *	Special checks for tags, they make our life much more
	 *	difficult.
	 */
	if (flags.has_tag) {
		/*
		 *	Only string, octets, and integer can be tagged.
		 */
		switch (type) {
		case PW_TYPE_STRING:
		case PW_TYPE_INTEGER:
			break;

		default:
			fr_strerror_printf("dict_init: %s[%d]: Attributes of type %s cannot be tagged.",
				   fn, line,
				   fr_int2str(dict_attr_types, type, "?Unknown?"));
			return -1;
		}
	}

	if (type == PW_TYPE_TLV) {
		if (vendor && (vendor < FR_MAX_VENDOR)
#ifdef WITH_DHCP
		    && (vendor != DHCP_MAGIC_VENDOR)
#endif
			) {
			DICT_VENDOR *dv;

			dv = dict_vendorbyvalue(vendor);
			if (!dv || (dv->type != 1) || (dv->length != 1)) {
				fr_strerror_printf("dict_init: %s[%d]: Type \"tlv\" can only be for \"format=1,1\".",
						   fn, line);
				return -1;
			}

		}
		flags.has_tlv = 1;
	}

	if (block_tlv) {
		/*
		 *	TLV's can be only one octet.
		 */
		if ((value == 0) || ((value & ~fr_attr_mask[tlv_depth]) != 0)) {
			fr_strerror_printf( "dict_init: %s[%d]: sub-tlv has invalid attribute number",
					    fn, line);
			return -1;
		}

		/*
		 *
		 */
		value <<= fr_attr_shift[tlv_depth];
		value |= block_tlv->attr;
		flags.is_tlv = 1;
	}

#ifdef WITH_DICTIONARY_WARNINGS
	/*
	 *	Hack to help us discover which vendors have illegal
	 *	attributes.
	 */
	if (!vendor && (value < 256) &&
	    !strstr(fn, "rfc") && !strstr(fn, "illegal")) {
		fprintf(stderr, "WARNING: Illegal Attribute %s in %s\n",
			argv[0], fn);
	}
#endif

	/*
	 *	Add it in.
	 */
	if (dict_addattr(argv[0], value, vendor, type, flags) < 0) {
		char buffer[256];

		strlcpy(buffer, fr_strerror(), sizeof(buffer));

		fr_strerror_printf("dict_init: %s[%d]: %s",
				   fn, line, buffer);
		return -1;
	}

	return 0;
}


/*
 *	Process the VALUE command
 */
static int process_value(char const* fn, int const line, char **argv,
			 int argc)
{
	unsigned int	value;

	if (argc != 3) {
		fr_strerror_printf("dict_init: %s[%d]: invalid VALUE line",
			fn, line);
		return -1;
	}
	/*
	 *	For Compatibility, skip "Server-Config"
	 */
	if (strcasecmp(argv[0], "Server-Config") == 0)
		return 0;

	/*
	 *	Validate all entries
	 */
	if (!sscanf_i(argv[2], &value)) {
		fr_strerror_printf("dict_init: %s[%d]: invalid value",
			fn, line);
		return -1;
	}

	if (dict_addvalue(argv[1], argv[0], value) < 0) {
		char buffer[256];

		strlcpy(buffer, fr_strerror(), sizeof(buffer));

		fr_strerror_printf("dict_init: %s[%d]: %s",
				   fn, line, buffer);
		return -1;
	}

	return 0;
}


/*
 *	Process the VALUE-ALIAS command
 *
 *	This allows VALUE mappings to be shared among multiple
 *	attributes.
 */
static int process_value_alias(char const* fn, int const line, char **argv,
			       int argc)
{
	DICT_ATTR const *my_da, *da;
	DICT_VALUE *dval;

	if (argc != 2) {
		fr_strerror_printf("dict_init: %s[%d]: invalid VALUE-ALIAS line",
			fn, line);
		return -1;
	}

	my_da = dict_attrbyname(argv[0]);
	if (!my_da) {
		fr_strerror_printf("dict_init: %s[%d]: ATTRIBUTE \"%s\" does not exist",
			   fn, line, argv[1]);
		return -1;
	}

	if (my_da->flags.has_value_alias) {
		fr_strerror_printf("dict_init: %s[%d]: Cannot add VALUE-ALIAS to ATTRIBUTE \"%s\" with pre-existing VALUE-ALIAS",
			   fn, line, argv[0]);
		return -1;
	}

	da = dict_attrbyname(argv[1]);
	if (!da) {
		fr_strerror_printf("dict_init: %s[%d]: Cannot find ATTRIBUTE \"%s\" for alias",
			   fn, line, argv[1]);
		return -1;
	}

	if (da->flags.has_value_alias) {
		fr_strerror_printf("dict_init: %s[%d]: Cannot add VALUE-ALIAS to ATTRIBUTE \"%s\" which itself has a VALUE-ALIAS",
			   fn, line, argv[1]);
		return -1;
	}

	if (my_da->type != da->type) {
		fr_strerror_printf("dict_init: %s[%d]: Cannot add VALUE-ALIAS between attributes of differing type",
			   fn, line);
		return -1;
	}

	if ((dval = fr_pool_alloc(sizeof(*dval))) == NULL) {
		fr_strerror_printf("dict_addvalue: out of memory");
		return -1;
	}

	dval->name[0] = '\0';	/* empty name */
	dval->attr = my_da->attr;
	dval->vendor = my_da->vendor;
	dval->value = da->attr;

	if (!fr_hash_table_insert(values_byname, dval)) {
		fr_strerror_printf("dict_init: %s[%d]: Error create alias",
			   fn, line);
		fr_pool_free(dval);
		return -1;
	}

	return 0;
}


/*
 *	Process the VENDOR command
 */
static int process_vendor(char const* fn, int const line, char **argv,
			  int argc)
{
	int		value;
	int		continuation = 0;
	char const	*format = NULL;

	if ((argc < 2) || (argc > 3)) {
		fr_strerror_printf( "dict_init: %s[%d] invalid VENDOR entry",
			    fn, line);
		return -1;
	}

	/*
	 *	 Validate all entries
	 */
	if (!isdigit((int) argv[1][0])) {
		fr_strerror_printf("dict_init: %s[%d]: invalid value",
			fn, line);
		return -1;
	}
	value = atoi(argv[1]);

	/* Create a new VENDOR entry for the list */
	if (dict_addvendor(argv[0], value) < 0) {
		char buffer[256];

		strlcpy(buffer, fr_strerror(), sizeof(buffer));

		fr_strerror_printf("dict_init: %s[%d]: %s",
			   fn, line, buffer);
		return -1;
	}

	/*
	 *	Look for a format statement
	 */
	if (argc == 3) {
		format = argv[2];

	} else if (value == VENDORPEC_USR) { /* catch dictionary screw-ups */
		format = "format=4,0";

	} else if (value == VENDORPEC_LUCENT) {
		format = "format=2,1";

	} else if (value == VENDORPEC_STARENT) {
		format = "format=2,2";

	} /* else no fixups to do */

	if (format) {
		int type, length;
		char const *p;
		DICT_VENDOR *dv;

		if (strncasecmp(format, "format=", 7) != 0) {
			fr_strerror_printf("dict_init: %s[%d]: Invalid format for VENDOR.  Expected \"format=\", got \"%s\"",
				   fn, line, format);
			return -1;
		}

		p = format + 7;
		if ((strlen(p) < 3) ||
		    !isdigit((int) p[0]) ||
		    (p[1] != ',') ||
		    !isdigit((int) p[2]) ||
		    (p[3] && (p[3] != ','))) {
			fr_strerror_printf("dict_init: %s[%d]: Invalid format for VENDOR.  Expected text like \"1,1\", got \"%s\"",
				   fn, line, p);
			return -1;
		}

		type = (int) (p[0] - '0');
		length = (int) (p[2] - '0');

		if (p[3] == ',') {
			if (!p[4]) {
				fr_strerror_printf("dict_init: %s[%d]: Invalid format for VENDOR.  Expected text like \"1,1\", got \"%s\"",
				   fn, line, p);
				return -1;
			}

			if ((p[4] != 'c') ||
			    (p[5] != '\0')) {
				fr_strerror_printf("dict_init: %s[%d]: Invalid format for VENDOR.  Expected text like \"1,1\", got \"%s\"",
					   fn, line, p);
				return -1;
			}
			continuation = 1;

			if ((value != VENDORPEC_WIMAX) ||
			    (type != 1) || (length != 1)) {
				fr_strerror_printf("dict_init: %s[%d]: Only WiMAX VSAs can have continuations",
					   fn, line);
				return -1;
			}
		}

		dv = dict_vendorbyvalue(value);
		if (!dv) {
			fr_strerror_printf("dict_init: %s[%d]: Failed adding format for VENDOR",
				   fn, line);
			return -1;
		}

		if ((type != 1) && (type != 2) && (type != 4)) {
			fr_strerror_printf("dict_init: %s[%d]: invalid type value %d for VENDOR",
				   fn, line, type);
			return -1;
		}

		if ((length != 0) && (length != 1) && (length != 2)) {
			fr_strerror_printf("dict_init: %s[%d]: invalid length value %d for VENDOR",
				   fn, line, length);
			return -1;
		}

		dv->type = type;
		dv->length = length;
		dv->flags = continuation;
	}

	return 0;
}

/*
 *	String split routine.  Splits an input string IN PLACE
 *	into pieces, based on spaces.
 */
int str2argv(char *str, char **argv, int max_argc)
{
	int argc = 0;

	while (*str) {
		if (argc >= max_argc) break;

		/*
		 *	Chop out comments early.
		 */
		if (*str == '#') {
			*str = '\0';
			break;
		}

		while ((*str == ' ') ||
		       (*str == '\t') ||
		       (*str == '\r') ||
		       (*str == '\n')) *(str++) = '\0';

		if (!*str) break;

		argv[argc] = str;
		argc++;

		while (*str &&
		       (*str != ' ') &&
		       (*str != '\t') &&
		       (*str != '\r') &&
		       (*str != '\n')) str++;
	}

	return argc;
}

static int my_dict_init(char const *parent, char const *filename,
			char const *src_file, int src_line);

int dict_read(char const *dir, char const *filename)
{
	if (!attributes_byname) {
		fr_strerror_printf("Must call dict_init() before dict_read()");
		return -1;
	}

	return my_dict_init(dir, filename, NULL, 0);
}


#define MAX_ARGV (16)

/*
 *	Initialize the dictionary.
 */
static int my_dict_init(char const *parent, char const *filename,
			char const *src_file, int src_line)
{
	FILE	*fp;
	char 	dir[256], fn[256];
	char	buf[256];
	char	*p;
	int	line = 0;
	unsigned int	vendor;
	unsigned int	block_vendor;
	struct stat statbuf;
	char	*argv[MAX_ARGV];
	int	argc;
	DICT_ATTR const *da, *block_tlv[MAX_TLV_NEST + 1];
	int	which_block_tlv = 0;

	block_tlv[0] = NULL;
	block_tlv[1] = NULL;
	block_tlv[2] = NULL;
	block_tlv[3] = NULL;

	if ((strlen(parent) + 3 + strlen(filename)) > sizeof(dir)) {
		fr_strerror_printf("dict_init: filename name too long");
		return -1;
	}

	/*
	 *	If it's an absolute dir, forget the parent dir,
	 *	and remember the new one.
	 *
	 *	If it's a relative dir, tack on the current filename
	 *	to the parent dir.  And use that.
	 */
	if (!FR_DIR_IS_RELATIVE(filename)) {
		strlcpy(dir, filename, sizeof(dir));
		p = strrchr(dir, FR_DIR_SEP);
		if (p) {
			p[1] = '\0';
		} else {
			strlcat(dir, "/", sizeof(dir));
		}

		strlcpy(fn, filename, sizeof(fn));
	} else {
		strlcpy(dir, parent, sizeof(dir));
		p = strrchr(dir, FR_DIR_SEP);
		if (p) {
			if (p[1]) strlcat(dir, "/", sizeof(dir));
		} else {
			strlcat(dir, "/", sizeof(dir));
		}
		strlcat(dir, filename, sizeof(dir));
		p = strrchr(dir, FR_DIR_SEP);
		if (p) {
			p[1] = '\0';
		} else {
			strlcat(dir, "/", sizeof(dir));
		}

		p = strrchr(filename, FR_DIR_SEP);
		if (p) {
			snprintf(fn, sizeof(fn), "%s%s", dir, p);
		} else {
			snprintf(fn, sizeof(fn), "%s%s", dir, filename);
		}

	}

	if ((fp = fopen(fn, "r")) == NULL) {
		if (!src_file) {
			fr_strerror_printf("dict_init: Couldn't open dictionary \"%s\": %s",
				   fn, strerror(errno));
		} else {
			fr_strerror_printf("dict_init: %s[%d]: Couldn't open dictionary \"%s\": %s",
				   src_file, src_line, fn, strerror(errno));
		}
		return -2;
	}

	stat(fn, &statbuf); /* fopen() guarantees this will succeed */
	if (!S_ISREG(statbuf.st_mode)) {
		fclose(fp);
		fr_strerror_printf("dict_init: Dictionary \"%s\" is not a regular file",
			   fn);
		return -1;
	}

	/*
	 *	Globally writable dictionaries means that users can control
	 *	the server configuration with little difficulty.
	 */
#ifdef S_IWOTH
	if ((statbuf.st_mode & S_IWOTH) != 0) {
		fclose(fp);
		fr_strerror_printf("dict_init: Dictionary \"%s\" is globally writable.  Refusing to start due to insecure configuration.",
			   fn);
		return -1;
	}
#endif

	dict_stat_add(fn, &statbuf);

	/*
	 *	Seed the random pool with data.
	 */
	fr_rand_seed(&statbuf, sizeof(statbuf));

	block_vendor = 0;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		line++;
		if (buf[0] == '#' || buf[0] == 0 ||
		    buf[0] == '\n' || buf[0] == '\r')
			continue;

		/*
		 *  Comment characters should NOT be appearing anywhere but
		 *  as start of a comment;
		 */
		p = strchr(buf, '#');
		if (p) *p = '\0';

		argc = str2argv(buf, argv, MAX_ARGV);
		if (argc == 0) continue;

		if (argc == 1) {
			fr_strerror_printf( "dict_init: %s[%d] invalid entry",
				    fn, line);
			fclose(fp);
			return -1;
		}

		/*
		 *	Process VALUE lines.
		 */
		if (strcasecmp(argv[0], "VALUE") == 0) {
			if (process_value(fn, line,
					  argv + 1, argc - 1) == -1) {
				fclose(fp);
				return -1;
			}
			continue;
		}

		/*
		 *	Perhaps this is an attribute.
		 */
		if (strcasecmp(argv[0], "ATTRIBUTE") == 0) {
			if (process_attribute(fn, line, block_vendor,
					      block_tlv[which_block_tlv],
					      which_block_tlv,
					      argv + 1, argc - 1) == -1) {
				fclose(fp);
				return -1;
			}
			continue;
		}

		/*
		 *	See if we need to import another dictionary.
		 */
		if (strcasecmp(argv[0], "$INCLUDE") == 0) {
			if (my_dict_init(dir, argv[1], fn, line) < 0) {
				fclose(fp);
				return -1;
			}
			continue;
		} /* $INCLUDE */

		/*
		 *	Optionally include a dictionary
		 */
		if (strcasecmp(argv[0], "$INCLUDE-") == 0) {
			int rcode = my_dict_init(dir, argv[1], fn, line);

			if (rcode == -2) continue;

			if (rcode < 0) {
				fclose(fp);
				return -1;
			}
			continue;
		} /* $INCLUDE- */

		if (strcasecmp(argv[0], "VALUE-ALIAS") == 0) {
			if (process_value_alias(fn, line,
						argv + 1, argc - 1) == -1) {
				fclose(fp);
				return -1;
			}
			continue;
		}

		/*
		 *	Process VENDOR lines.
		 */
		if (strcasecmp(argv[0], "VENDOR") == 0) {
			if (process_vendor(fn, line,
					   argv + 1, argc - 1) == -1) {
				fclose(fp);
				return -1;
			}
			continue;
		}

		if (strcasecmp(argv[0], "BEGIN-TLV") == 0) {
			if (argc != 2) {
				fr_strerror_printf(
				"dict_init: %s[%d] invalid BEGIN-TLV entry",
					fn, line);
				fclose(fp);
				return -1;
			}

			da = dict_attrbyname(argv[1]);
			if (!da) {
				fr_strerror_printf(
					"dict_init: %s[%d]: unknown attribute %s",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}

			if (da->type != PW_TYPE_TLV) {
				fr_strerror_printf(
					"dict_init: %s[%d]: attribute %s is not of type tlv",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}

			if (which_block_tlv >= MAX_TLV_NEST) {
				fr_strerror_printf(
					"dict_init: %s[%d]: TLVs are nested too deep",
					fn, line);
				fclose(fp);
				return -1;
			}


			block_tlv[++which_block_tlv] = da;
			continue;
		} /* BEGIN-TLV */

		if (strcasecmp(argv[0], "END-TLV") == 0) {
			if (argc != 2) {
				fr_strerror_printf(
				"dict_init: %s[%d] invalid END-TLV entry",
					fn, line);
				fclose(fp);
				return -1;
			}

			da = dict_attrbyname(argv[1]);
			if (!da) {
				fr_strerror_printf(
					"dict_init: %s[%d]: unknown attribute %s",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}

			if (da != block_tlv[which_block_tlv]) {
				fr_strerror_printf(
					"dict_init: %s[%d]: END-TLV %s does not match any previous BEGIN-TLV",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}
			block_tlv[which_block_tlv--] = NULL;
			continue;
		} /* END-VENDOR */

		if (strcasecmp(argv[0], "BEGIN-VENDOR") == 0) {
			if (argc < 2) {
				fr_strerror_printf(
				"dict_init: %s[%d] invalid BEGIN-VENDOR entry",
					fn, line);
				fclose(fp);
				return -1;
			}

			vendor = dict_vendorbyname(argv[1]);
			if (!vendor) {
				fr_strerror_printf(
					"dict_init: %s[%d]: unknown vendor %s",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}

			block_vendor = vendor;

			/*
			 *	Check for extended attr VSAs
			 *
			 *	BEGIN-VENDOR foo format=Foo-Encapsulation-Attr
			 */
			if (argc > 2) {
				if (strncmp(argv[2], "format=", 7) != 0) {
					fr_strerror_printf(
						"dict_init: %s[%d]: Invalid format %s",
						fn, line, argv[2]);
					fclose(fp);
					return -1;
				}

				p = argv[2] + 7;
				da = dict_attrbyname(p);
				if (!da) {
					fr_strerror_printf("dict_init: %s[%d]: Invalid format for BEGIN-VENDOR: unknown attribute \"%s\"",
							   fn, line, p);
					fclose(fp);
					return -1;
				}

				if (!da->flags.evs) {
					fr_strerror_printf("dict_init: %s[%d]: Invalid format for BEGIN-VENDOR.  Attribute \"%s\" is not of \"evs\" data type",
							   fn, line, p);
					fclose(fp);
					return -1;
				}

				/*
				 *	Pack the encapsulating
				 *	attribute into the upper 8
				 *	bits of the vendor ID
				 */
				block_vendor |= (da->attr & fr_attr_mask[0]) * FR_MAX_VENDOR;
			}

			continue;
		} /* BEGIN-VENDOR */

		if (strcasecmp(argv[0], "END-VENDOR") == 0) {
			if (argc != 2) {
				fr_strerror_printf(
				"dict_init: %s[%d] invalid END-VENDOR entry",
					fn, line);
				fclose(fp);
				return -1;
			}

			vendor = dict_vendorbyname(argv[1]);
			if (!vendor) {
				fr_strerror_printf(
					"dict_init: %s[%d]: unknown vendor %s",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}

			if (vendor != (block_vendor & (FR_MAX_VENDOR - 1))) {
				fr_strerror_printf(
					"dict_init: %s[%d]: END-VENDOR %s does not match any previous BEGIN-VENDOR",
					fn, line, argv[1]);
				fclose(fp);
				return -1;
			}
			block_vendor = 0;
			continue;
		} /* END-VENDOR */

		/*
		 *	Any other string: We don't recognize it.
		 */
		fr_strerror_printf("dict_init: %s[%d] invalid keyword \"%s\"",
			   fn, line, argv[0]);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}


/*
 *	Empty callback for hash table initialization.
 */
static int null_callback(UNUSED void *ctx, UNUSED void *data)
{
	return 0;
}


/*
 *	Initialize the directory, then fix the attr member of
 *	all attributes.
 */
int dict_init(char const *dir, char const *fn)
{
	/*
	 *	Check if we need to change anything.  If not, don't do
	 *	anything.
	 */
	if (dict_stat_check(dir, fn)) {
		return 0;
	}

	/*
	 *	Free the dictionaries, and the stat cache.
	 */
	dict_free();
	stat_root_dir = strdup(dir);
	stat_root_file = strdup(fn);

	/*
	 *	Create the table of vendor by name.   There MAY NOT
	 *	be multiple vendors of the same name.
	 *
	 *	Each vendor is malloc'd, so the free function is free.
	 */
	vendors_byname = fr_hash_table_create(dict_vendor_name_hash,
						dict_vendor_name_cmp,
						fr_pool_free);
	if (!vendors_byname) {
		return -1;
	}

	/*
	 *	Create the table of vendors by value.  There MAY
	 *	be vendors of the same value.  If there are, we
	 *	pick the latest one.
	 */
	vendors_byvalue = fr_hash_table_create(dict_vendor_value_hash,
						 dict_vendor_value_cmp,
						 fr_pool_free);
	if (!vendors_byvalue) {
		return -1;
	}

	/*
	 *	Create the table of attributes by name.   There MAY NOT
	 *	be multiple attributes of the same name.
	 *
	 *	Each attribute is malloc'd, so the free function is free.
	 */
	attributes_byname = fr_hash_table_create(dict_attr_name_hash,
						   dict_attr_name_cmp,
						   fr_pool_free);
	if (!attributes_byname) {
		return -1;
	}

	/*
	 *	Create the table of attributes by value.  There MAY
	 *	be attributes of the same value.  If there are, we
	 *	pick the latest one.
	 */
	attributes_byvalue = fr_hash_table_create(dict_attr_value_hash,
						    dict_attr_value_cmp,
						    fr_pool_free);
	if (!attributes_byvalue) {
		return -1;
	}

	/*
	 *	Horrible hacks for combo-IP.
	 */
	attributes_combo = fr_hash_table_create(dict_attr_combo_hash,
						dict_attr_combo_cmp,
						fr_pool_free);
	if (!attributes_combo) {
		return -1;
	}

	values_byname = fr_hash_table_create(dict_value_name_hash,
					       dict_value_name_cmp,
					       fr_pool_free);
	if (!values_byname) {
		return -1;
	}

	values_byvalue = fr_hash_table_create(dict_value_value_hash,
						dict_value_value_cmp,
						fr_pool_free);
	if (!values_byvalue) {
		return -1;
	}

	value_fixup = NULL;	/* just to be safe. */

	if (my_dict_init(dir, fn, NULL, 0) < 0)
		return -1;

	if (value_fixup) {
		DICT_ATTR const *a;
		value_fixup_t *this, *next;

		for (this = value_fixup; this != NULL; this = next) {
			next = this->next;

			a = dict_attrbyname(this->attrstr);
			if (!a) {
				fr_strerror_printf(
					"dict_init: No ATTRIBUTE \"%s\" defined for VALUE \"%s\"",
					this->attrstr, this->dval->name);
				return -1; /* leak, but they should die... */
			}

			this->dval->attr = a->attr;

			/*
			 *	Add the value into the dictionary.
			 */
			if (!fr_hash_table_replace(values_byname,
						     this->dval)) {
				fr_strerror_printf("dict_addvalue: Duplicate value name %s for attribute %s", this->dval->name, a->name);
				return -1;
			}

			/*
			 *	Allow them to use the old name, but
			 *	prefer the new name when printing
			 *	values.
			 */
			if (!fr_hash_table_finddata(values_byvalue, this->dval)) {
				fr_hash_table_replace(values_byvalue,
							this->dval);
			}
			free(this);

			/*
			 *	Just so we don't lose track of things.
			 */
			value_fixup = next;
		}
	}

	/*
	 *	Walk over all of the hash tables to ensure they're
	 *	initialized.  We do this because the threads may perform
	 *	lookups, and we don't want multi-threaded re-ordering
	 *	of the table entries.  That would be bad.
	 */
	fr_hash_table_walk(vendors_byname, null_callback, NULL);
	fr_hash_table_walk(vendors_byvalue, null_callback, NULL);

	fr_hash_table_walk(attributes_byname, null_callback, NULL);
	fr_hash_table_walk(attributes_byvalue, null_callback, NULL);

	fr_hash_table_walk(values_byvalue, null_callback, NULL);
	fr_hash_table_walk(values_byname, null_callback, NULL);

	return 0;
}

static size_t print_attr_oid(char *buffer, size_t size, unsigned int attr,
			     int dv_type)
{
	int nest;
	size_t outlen;
	size_t len;

	switch (dv_type) {
	case 4:
		return snprintf(buffer, size, "%u", attr);

	case 2:
		return snprintf(buffer, size, "%u", attr & 0xffff);

	default:
	case 1:
		len = snprintf(buffer, size, "%u", attr & 0xff);
		break;
	}

	if ((attr >> 8) == 0) return len;

	outlen = len;
	buffer += len;
	size -= len;

	for (nest = 1; nest <= fr_attr_max_tlv; nest++) {
		if (((attr >> fr_attr_shift[nest]) & fr_attr_mask[nest]) == 0) break;

		len = snprintf(buffer, size, ".%u",
			       (attr >> fr_attr_shift[nest]) & fr_attr_mask[nest]);

		outlen = len;
		buffer += len;
		size -= len;
	}

	return outlen;
}

/** Free dynamically allocated (unknown attributes)
 *
 * If the da was dynamically allocated it will be freed, else the function
 * will return without doing anything.
 *
 * @param da to free.
 */
void dict_attr_free(DICT_ATTR const **da)
{
	DICT_ATTR **tmp;

	if (!da || !*da) return;

	/* Don't free real DAs */
	if (!(*da)->flags.is_unknown) {
		return;
	}

	memcpy(&tmp, &da, sizeof(*tmp));
	free(*tmp);

	*tmp = NULL;
}

/** Copies a dictionary attr
 *
 * If the attr is dynamically allocated (unknown attribute), then it will be
 * copied to a new attr.
 *
 * If the attr is known, a pointer to the da will be returned.
 *
 * @param da to copy.
 * @param vp_free if true, da will be freed at the same time as the
 *	VALUE_PAIR which contains it.
 * @return return a copy of the da.
 */
DICT_ATTR const *dict_attr_copy(DICT_ATTR const *da, int vp_free)
{
	DICT_ATTR *copy;

	if (!da) return NULL;

	if (!da->flags.is_unknown) {
		return da;
	}

	copy = malloc(DICT_ATTR_SIZE);
	if (!copy) {
		fr_strerror_printf("Out of memory");
		return NULL;
	}

	memcpy(copy, da, DICT_ATTR_SIZE);
	copy->flags.vp_free = (vp_free != 0);

	return copy;
}


/** Allocs an dictionary attr for unknown attributes
 *
 * Allocates a dict attr for an unknown attribute/vendor/type
 * without adding it to dictionary pools/hashes.
 *
 * @note Must be freed with dict_attr_free if not used as part of a valuepair.
 *
 * @param[in] attr number.
 * @param[in] vendor number.
 * @param[in] vp_free if > 0 DICT_ATTR will be freed on VALUE_PAIR free.
 * @return new dictionary attribute.
 */
DICT_ATTR const *dict_attrunknown(unsigned int attr, unsigned int vendor,
				  int vp_free)
{
	DICT_ATTR *da;
	char *p;
	int dv_type = 1;
	size_t len = 0;
	size_t bufsize = DICT_ATTR_MAX_NAME_LEN;

	da = malloc(DICT_ATTR_SIZE);
	if (!da) {
		fr_strerror_printf("Out of memory");
		return NULL;
	}
	memset(da, 0, DICT_ATTR_SIZE);

	da->attr = attr;
	da->vendor = vendor;
	da->type = PW_TYPE_OCTETS;
	da->flags.is_unknown = true;
	da->flags.vp_free = (vp_free != 0);

	p = da->name;

	len = snprintf(p, bufsize, "Attr-");
	p += len;
	bufsize -= len;

	if (vendor > FR_MAX_VENDOR) {
		len = snprintf(p, bufsize, "%u.", vendor / FR_MAX_VENDOR);
		p += len;
		bufsize -= len;
		vendor &= (FR_MAX_VENDOR) - 1;
	}

	if (vendor) {
		DICT_VENDOR *dv;

		/*
		 *	dv_type is the length of the vendor's type field
		 *	RFC 2865 never defined a mandatory length, so
		 *	different vendors have different length type fields.
		 */
		dv = dict_vendorbyvalue(vendor);
		if (dv) {
			dv_type = dv->type;
		}
		len = snprintf(p, bufsize, "26.%u.", vendor);

		p += len;
		bufsize -= len;
	}

	p += print_attr_oid(p, bufsize , attr, dv_type);

	return da;
}

/** Create a DICT_ATTR from an ASCII attribute and value
 *
 * Where the attribute name is in the form:
 *  - Attr-%d
 *  - Attr-%d.%d.%d...
 *  - Vendor-%d-Attr-%d
 *  - VendorName-Attr-%d
 *
 * @todo should check attr/vendor against dictionary and return the real da.
 *
 * @param[in] attribute name.
 * @param[in] vp_free if > 0 DICT_ATTR will be freed on VALUE_PAIR free.
 * @return new da or NULL on error.
 */
DICT_ATTR const *dict_attrunknownbyname(char const *attribute, int vp_free)
{
	unsigned int   	attr, vendor = 0;
	unsigned int    dv_type = 1;	/* The type of vendor field */

	char const	*p = attribute;
	char		*q;

	DICT_VENDOR	*dv;
	DICT_ATTR const	*da;

	/*
	 *	Pull off vendor prefix first.
	 */
	if (strncasecmp(p, "Attr-", 5) != 0) {
		if (strncasecmp(p, "Vendor-", 7) == 0) {
			vendor = (int) strtol(p + 7, &q, 10);
			if ((vendor == 0) || (vendor > FR_MAX_VENDOR)) {
				fr_strerror_printf("Invalid vendor value in "
						   "attribute name \"%s\"",
						   attribute);
				return NULL;
			}

			p = q;

		/* must be vendor name */
		} else {
			char buffer[256];

			q = strchr(p, '-');

			if (!q) {
				fr_strerror_printf("Invalid vendor name in "
						   "attribute name \"%s\"",
						   attribute);
				return NULL;
			}

			if ((size_t) (q - p) >= sizeof(buffer)) {
				fr_strerror_printf("Vendor name too long "
						   "in attribute name \"%s\"",
						   attribute);
				return NULL;
			}

			memcpy(buffer, p, (q - p));
			buffer[q - p] = '\0';

			vendor = dict_vendorbyname(buffer);
			if (!vendor) {
				fr_strerror_printf("Unknown vendor name in "
						   "attribute name \"%s\"",
						   attribute);
				return NULL;
			}

			p = q;
		}

		if (*p != '-') {
			fr_strerror_printf("Invalid text following vendor "
					   "definition in attribute name "
					   "\"%s\"", attribute);
			return NULL;
		}
		p++;
	}

	/*
	 *	Attr-%d
	 */
	if (strncasecmp(p, "Attr-", 5) != 0) {
		fr_strerror_printf("Invalid format in attribute name \"%s\"",
				   attribute);
		return NULL;
	}

	attr = strtol(p + 5, &q, 10);

	/*
	 *	Invalid attribute.
	 */
	if (attr == 0) {
		fr_strerror_printf("Invalid value in attribute name \"%s\"",
				   attribute);
		return NULL;
	}

	p = q;

	/*
	 *	Vendor-%d-Attr-%d
	 *	VendorName-Attr-%d
	 *	Attr-%d
	 *	Attr-%d.
	 *
	 *	Anything else is invalid.
	 */
	if (((vendor != 0) && (*p != '\0')) ||
	    ((vendor == 0) && *p && (*p != '.'))) {
	invalid:
		fr_strerror_printf("Invalid OID");
		return NULL;
	}

	/*
	 *	Look for OIDs.  Require the "Attr-26.Vendor-Id.type"
	 *	format, and disallow "Vendor-%d-Attr-%d" and
	 *	"VendorName-Attr-%d"
	 *
	 *	This section parses the Vendor-Id portion of
	 *	Attr-%d.%d.  where the first number is 26, *or* an
	 *	extended attribute of the "evs" data type.
	 */
	if (*p == '.') {
		da = dict_attrbyvalue(attr, 0);
		if (!da) {
			fr_strerror_printf("Cannot parse attributes without "
					   "dictionaries");
			return NULL;
		}

		if ((attr != PW_VENDOR_SPECIFIC) &&
		    !(da->flags.extended || da->flags.long_extended)) {
			fr_strerror_printf("Standard attributes cannot use "
					   "OIDs");
			return NULL;
		}

		if ((attr == PW_VENDOR_SPECIFIC) || da->flags.evs) {
			vendor = strtol(p + 1, &q, 10);
			if ((vendor == 0) || (vendor > FR_MAX_VENDOR)) {
				fr_strerror_printf("Invalid vendor");
				return NULL;
			}

			if (*q != '.') goto invalid;

			p = q;

			if (da->flags.evs) {
				vendor |= attr * FR_MAX_VENDOR;
			}
			attr = 0;
		} /* else the second number is a TLV number */
	}

	/*
	 *	Get the expected maximum size of the attribute.
	 */
	if (vendor) {
		dv = dict_vendorbyvalue(vendor & (FR_MAX_VENDOR - 1));
		if (dv) {
			dv_type = dv->type;
			if (dv_type > 3) dv_type = 3; /* hack */
		}
	}

	/*
	 *	Parse the next number.  It could be a Vendor-Type
	 *	of 1..2^24, or it could be a TLV.
	 */
	if (*p == '.') {
		attr = strtol(p + 1, &q, 10);
		if (attr == 0) {
			fr_strerror_printf("Invalid attribute number");
			return NULL;
		}

		if (*q) {
			if (*q != '.') {
				goto invalid;
			}

			if (dv_type != 1) {
				goto invalid;
			}
		}

		p = q;
	}

	/*
	 *	Enforce a maximum value on the attribute number.
	 */
	if (attr >= (unsigned) (1 << (dv_type << 3))) goto invalid;

	if (*p == '.') {
		if (dict_str2oid(p + 1, &attr, &vendor, 1) < 0) {
			return NULL;
		}
	}

	return dict_attrunknown(attr, vendor, vp_free);
}

/*
 *	Get an attribute by its numerical value.
 */
DICT_ATTR const *dict_attrbyvalue(unsigned int attr, unsigned int vendor)
{
	DICT_ATTR dattr;

	if ((attr > 0) && (attr < 256) && !vendor) return dict_base_attrs[attr];

	dattr.attr = attr;
	dattr.vendor = vendor;

	return fr_hash_table_finddata(attributes_byvalue, &dattr);
}


/**
 * @brief Get an attribute by its numerical value. and data type
 *
 *	Used only for COMBO_IP
 *
 * @return The attribute, or NULL if not found
 */
DICT_ATTR const *dict_attrbytype(unsigned int attr, unsigned int vendor,
				 PW_TYPE type)
{
	DICT_ATTR dattr;

	dattr.attr = attr;
	dattr.vendor = vendor;
	dattr.type = type;

	return fr_hash_table_finddata(attributes_combo, &dattr);
}

/**
 * @brief Using a parent and attr/vendor, find a child attr/vendor
 */
int dict_attr_child(DICT_ATTR const *parent,
		    unsigned int *pattr, unsigned int *pvendor)
{
	unsigned int attr, vendor;
	DICT_ATTR dattr;

	if (!parent || !pattr || !pvendor) return false;

	attr = *pattr;
	vendor = *pvendor;

	/*
	 *	Only some types can have children
	 */
	switch (parent->type) {
	default: return false;

	case PW_TYPE_VSA:
	case PW_TYPE_TLV:
	case PW_TYPE_EVS:
	case PW_TYPE_EXTENDED:
	case PW_TYPE_LONG_EXTENDED:
	  break;
	}

	if ((vendor == 0) && (parent->vendor != 0)) return false;

	/*
	 *	Bootstrap by starting off with the parents values.
	 */
	dattr.attr = parent->attr;
	dattr.vendor = parent->vendor;

	/*
	 *	Do various butchery to insert the "attr" value.
	 *
	 *	00VID	000000AA	normal VSA for vendor VID
	 *	00VID	DDCCBBAA	normal VSAs with TLVs
	 *	EE000   000000AA	extended attr (241.1)
	 *	EE000	DDCCBBAA	extended attr with TLVs
	 *	EEVID	000000AA	EVS with vendor VID, attr AAA
	 *	EEVID	DDCCBBAA	EVS with TLVs
	 */
	if (!dattr.vendor) {
		dattr.vendor = parent->attr * FR_MAX_VENDOR;
		dattr.vendor |= vendor;
		dattr.attr = attr;

	} else {
		int i;

		/*
		 *	Trying to nest too deep.  It's an error
		 */
		if (parent->attr & (fr_attr_mask[MAX_TLV_NEST] << fr_attr_shift[MAX_TLV_NEST])) {
			return false;
		}

		for (i = MAX_TLV_NEST - 1; i >= 0; i--) {
			if ((parent->attr & (fr_attr_mask[i] << fr_attr_shift[i]))) {
				dattr.attr |= (attr & fr_attr_mask[i + 1]) << fr_attr_shift[i + 1];
				goto find;
			}
		}

		return false;
	}

find:
#if 0
	fprintf(stderr, "LOOKING FOR %08x %08x + %08x %08x --> %08x %08x\n",
		parent->vendor, parent->attr, attr, vendor,
		dattr.vendor, dattr.attr);
#endif

	*pattr = dattr.attr;
	*pvendor = dattr.vendor;
	return true;
}

/*
 *	Get an attribute by it's numerical value, and the parent
 */
DICT_ATTR const *dict_attrbyparent(DICT_ATTR const *parent, unsigned int attr, unsigned int vendor)
{
	unsigned int my_attr, my_vendor;
	DICT_ATTR dattr;

	my_attr = attr;
	my_vendor = vendor;

	if (!dict_attr_child(parent, &my_attr, &my_vendor)) return NULL;

	dattr.attr = my_attr;
	dattr.vendor = my_vendor;

	return fr_hash_table_finddata(attributes_byvalue, &dattr);
}


/*
 *	Get an attribute by its name.
 */
DICT_ATTR const *dict_attrbyname(char const *name)
{
	DICT_ATTR *da;
	uint32_t buffer[(sizeof(*da) + DICT_ATTR_MAX_NAME_LEN + 3)/4];

	if (!name) return NULL;

	da = (DICT_ATTR *) buffer;
	strlcpy(da->name, name, DICT_ATTR_MAX_NAME_LEN + 1);

	return fr_hash_table_finddata(attributes_byname, da);
}

/*
 *	Associate a value with an attribute and return it.
 */
DICT_VALUE *dict_valbyattr(unsigned int attr, unsigned int vendor, int value)
{
	DICT_VALUE dval, *dv;

	/*
	 *	First, look up aliases.
	 */
	dval.attr = attr;
	dval.vendor = vendor;
	dval.name[0] = '\0';

	/*
	 *	Look up the attribute alias target, and use
	 *	the correct attribute number if found.
	 */
	dv = fr_hash_table_finddata(values_byname, &dval);
	if (dv)	dval.attr = dv->value;

	dval.value = value;

	return fr_hash_table_finddata(values_byvalue, &dval);
}

/*
 *	Associate a value with an attribute and return it.
 */
char const *dict_valnamebyattr(unsigned int attr, unsigned int vendor, int value)
{
	DICT_VALUE *dv;

	dv = dict_valbyattr(attr, vendor, value);
	if (!dv) return "";

	return dv->name;
}

/*
 *	Get a value by its name, keyed off of an attribute.
 */
DICT_VALUE *dict_valbyname(unsigned int attr, unsigned int vendor, char const *name)
{
	DICT_VALUE *my_dv, *dv;
	uint32_t buffer[(sizeof(*my_dv) + DICT_VALUE_MAX_NAME_LEN + 3)/4];

	if (!name) return NULL;

	my_dv = (DICT_VALUE *) buffer;
	my_dv->attr = attr;
	my_dv->vendor = vendor;
	my_dv->name[0] = '\0';

	/*
	 *	Look up the attribute alias target, and use
	 *	the correct attribute number if found.
	 */
	dv = fr_hash_table_finddata(values_byname, my_dv);
	if (dv) my_dv->attr = dv->value;

	strlcpy(my_dv->name, name, DICT_VALUE_MAX_NAME_LEN + 1);

	return fr_hash_table_finddata(values_byname, my_dv);
}

/*
 *	Get the vendor PEC based on the vendor name
 *
 *	This is efficient only for small numbers of vendors.
 */
int dict_vendorbyname(char const *name)
{
	DICT_VENDOR *dv;
	size_t buffer[(sizeof(*dv) + DICT_VENDOR_MAX_NAME_LEN + sizeof(size_t) - 1) / sizeof(size_t)];

	if (!name) return 0;

	dv = (DICT_VENDOR *) buffer;
	strlcpy(dv->name, name, DICT_VENDOR_MAX_NAME_LEN + 1);

	dv = fr_hash_table_finddata(vendors_byname, dv);
	if (!dv) return 0;

	return dv->vendorpec;
}

/*
 *	Return the vendor struct based on the PEC.
 */
DICT_VENDOR *dict_vendorbyvalue(int vendorpec)
{
	DICT_VENDOR dv;

	dv.vendorpec = vendorpec;

	return fr_hash_table_finddata(vendors_byvalue, &dv);
}
