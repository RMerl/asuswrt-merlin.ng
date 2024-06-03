// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <dm.h>
#include <fsl_validate.h>
#include <fsl_secboot_err.h>
#include <fsl_sfp.h>
#include <fsl_sec.h>
#include <command.h>
#include <malloc.h>
#include <u-boot/rsa-mod-exp.h>
#include <hash.h>
#include <fsl_secboot_err.h>
#ifdef CONFIG_ARCH_LS1021A
#include <asm/arch/immap_ls102xa.h>
#endif

#define SHA256_BITS	256
#define SHA256_BYTES	(256/8)
#define SHA256_NIBBLES	(256/4)
#define NUM_HEX_CHARS	(sizeof(ulong) * 2)

#define CHECK_KEY_LEN(key_len)	(((key_len) == 2 * KEY_SIZE_BYTES / 4) || \
				 ((key_len) == 2 * KEY_SIZE_BYTES / 2) || \
				 ((key_len) == 2 * KEY_SIZE_BYTES))
#if defined(CONFIG_FSL_ISBC_KEY_EXT)
/* Global data structure */
static struct fsl_secboot_glb glb;
#endif

/* This array contains DER value for SHA-256 */
static const u8 hash_identifier[] = { 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60,
		0x86, 0x48, 0x01, 0x65,	0x03, 0x04, 0x02, 0x01, 0x05, 0x00,
		0x04, 0x20
		};

static u8 hash_val[SHA256_BYTES];

#ifdef CONFIG_ESBC_HDR_LS
/* New Barker Code for LS ESBC Header */
static const u8 barker_code[ESBC_BARKER_LEN] = { 0x12, 0x19, 0x20, 0x01 };
#else
static const u8 barker_code[ESBC_BARKER_LEN] = { 0x68, 0x39, 0x27, 0x81 };
#endif

void branch_to_self(void) __attribute__ ((noreturn));

/*
 * This function will put core in infinite loop.
 * This will be called when the ESBC can not proceed further due
 * to some unknown errors.
 */
void branch_to_self(void)
{
	printf("Core is in infinite loop due to errors.\n");
self:
	goto self;
}

#if defined(CONFIG_FSL_ISBC_KEY_EXT)
static u32 check_ie(struct fsl_secboot_img_priv *img)
{
	if (img->hdr.ie_flag & IE_FLAG_MASK)
		return 1;

	return 0;
}

/* This function returns the CSF Header Address of uboot
 * For MPC85xx based platforms, the LAW mapping for NOR
 * flash changes in uboot code. Hence the offset needs
 * to be calculated and added to the new NOR flash base
 * address
 */
#if defined(CONFIG_MPC85xx)
int get_csf_base_addr(u32 *csf_addr, u32 *flash_base_addr)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 csf_hdr_addr = in_be32(&gur->scratchrw[0]);
	u32 csf_flash_offset = csf_hdr_addr & ~(CONFIG_SYS_PBI_FLASH_BASE);
	u32 flash_addr, addr;
	int found = 0;
	int i = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		flash_addr = flash_info[i].start[0];
		addr = flash_info[i].start[0] + csf_flash_offset;
		if (memcmp((u8 *)addr, barker_code, ESBC_BARKER_LEN) == 0) {
			debug("Barker found on addr %x\n", addr);
			found = 1;
			break;
		}
	}

	if (!found)
		return -1;

	*csf_addr = addr;
	*flash_base_addr = flash_addr;

	return 0;
}
#else
/* For platforms like LS1020, correct flash address is present in
 * the header. So the function reqturns flash base address as 0
 */
int get_csf_base_addr(u32 *csf_addr, u32 *flash_base_addr)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 csf_hdr_addr = in_be32(&gur->scratchrw[0]);

	if (memcmp((u8 *)(uintptr_t)csf_hdr_addr,
		   barker_code, ESBC_BARKER_LEN))
		return -1;

	*csf_addr = csf_hdr_addr;
	*flash_base_addr = 0;
	return 0;
}
#endif

#if defined(CONFIG_ESBC_HDR_LS)
static int get_ie_info_addr(uintptr_t *ie_addr)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	/* For LS-CH3, the address of IE Table is
	 * stated in Scratch13 and scratch14 of DCFG.
	 * Bootrom validates this table while validating uboot.
	 * DCFG is LE*/
	*ie_addr = in_le32(&gur->scratchrw[SCRATCH_IE_HIGH_ADR - 1]);
	*ie_addr = *ie_addr << 32;
	*ie_addr |= in_le32(&gur->scratchrw[SCRATCH_IE_LOW_ADR - 1]);
	return 0;
}
#else /* CONFIG_ESBC_HDR_LS */
static int get_ie_info_addr(uintptr_t *ie_addr)
{
	struct fsl_secboot_img_hdr *hdr;
	struct fsl_secboot_sg_table *sg_tbl;
	u32 flash_base_addr, csf_addr;

	if (get_csf_base_addr(&csf_addr, &flash_base_addr))
		return -1;

	hdr = (struct fsl_secboot_img_hdr *)(uintptr_t)csf_addr;

	/* For SoC's with Trust Architecture v1 with corenet bus
	 * the sg table field in CSF header has absolute address
	 * for sg table in memory. In other Trust Architecture,
	 * this field specifies the offset of sg table from the
	 * base address of CSF Header
	 */
#if defined(CONFIG_FSL_TRUST_ARCH_v1) && defined(CONFIG_FSL_CORENET)
	sg_tbl = (struct fsl_secboot_sg_table *)
		 (((u32)hdr->psgtable & ~(CONFIG_SYS_PBI_FLASH_BASE)) +
		  flash_base_addr);
#else
	sg_tbl = (struct fsl_secboot_sg_table *)(uintptr_t)(csf_addr +
						 (u32)hdr->psgtable);
#endif

	/* IE Key Table is the first entry in the SG Table */
#if defined(CONFIG_MPC85xx)
	*ie_addr = (uintptr_t)((sg_tbl->src_addr &
			~(CONFIG_SYS_PBI_FLASH_BASE)) +
			flash_base_addr);
#else
	*ie_addr = (uintptr_t)sg_tbl->src_addr;
#endif

	debug("IE Table address is %lx\n", *ie_addr);
	return 0;
}
#endif /* CONFIG_ESBC_HDR_LS */
#endif

#ifdef CONFIG_KEY_REVOCATION
/* This function checks srk_table_flag in header and set/reset srk_flag.*/
static u32 check_srk(struct fsl_secboot_img_priv *img)
{
#ifdef CONFIG_ESBC_HDR_LS
	/* In LS, No SRK Flag as SRK is always present if IE not present*/
#if defined(CONFIG_FSL_ISBC_KEY_EXT)
	return !check_ie(img);
#endif
	return 1;
#else
	if (img->hdr.len_kr.srk_table_flag & SRK_FLAG)
		return 1;

	return 0;
#endif
}

/* This function returns ospr's key_revoc values.*/
static u32 get_key_revoc(void)
{
	struct ccsr_sfp_regs *sfp_regs = (void *)(CONFIG_SYS_SFP_ADDR);
	return (sfp_in32(&sfp_regs->ospr) & OSPR_KEY_REVOC_MASK) >>
		OSPR_KEY_REVOC_SHIFT;
}

/* This function checks if selected key is revoked or not.*/
static u32 is_key_revoked(u32 keynum, u32 rev_flag)
{
	if (keynum == UNREVOCABLE_KEY)
		return 0;

	if ((u32)(1 << (ALIGN_REVOC_KEY - keynum)) & rev_flag)
		return 1;

	return 0;
}

/* It read validates srk_table key lengths.*/
static u32 read_validate_srk_tbl(struct fsl_secboot_img_priv *img)
{
	int i = 0;
	u32 ret, key_num, key_revoc_flag, size;
	struct fsl_secboot_img_hdr *hdr = &img->hdr;
	void *esbc = (u8 *)(uintptr_t)img->ehdrloc;

	if ((hdr->len_kr.num_srk == 0) ||
	    (hdr->len_kr.num_srk > MAX_KEY_ENTRIES))
		return ERROR_ESBC_CLIENT_HEADER_INVALID_SRK_NUM_ENTRY;

	key_num = hdr->len_kr.srk_sel;
	if (key_num == 0 || key_num > hdr->len_kr.num_srk)
		return ERROR_ESBC_CLIENT_HEADER_INVALID_KEY_NUM;

	/* Get revoc key from sfp */
	key_revoc_flag = get_key_revoc();
	ret = is_key_revoked(key_num, key_revoc_flag);
	if (ret)
		return ERROR_ESBC_CLIENT_HEADER_KEY_REVOKED;

	size = hdr->len_kr.num_srk * sizeof(struct srk_table);

	memcpy(&img->srk_tbl, esbc + hdr->srk_tbl_off, size);

	for (i = 0; i < hdr->len_kr.num_srk; i++) {
		if (!CHECK_KEY_LEN(img->srk_tbl[i].key_len))
			return ERROR_ESBC_CLIENT_HEADER_INV_SRK_ENTRY_KEYLEN;
	}

	img->key_len = img->srk_tbl[key_num - 1].key_len;

	memcpy(&img->img_key, &(img->srk_tbl[key_num - 1].pkey),
	       img->key_len);

	return 0;
}
#endif

#ifndef CONFIG_ESBC_HDR_LS
static u32 read_validate_single_key(struct fsl_secboot_img_priv *img)
{
	struct fsl_secboot_img_hdr *hdr = &img->hdr;
	void *esbc = (u8 *)(uintptr_t)img->ehdrloc;

	/* check key length */
	if (!CHECK_KEY_LEN(hdr->key_len))
		return ERROR_ESBC_CLIENT_HEADER_KEY_LEN;

	memcpy(&img->img_key, esbc + hdr->pkey, hdr->key_len);

	img->key_len = hdr->key_len;

	return 0;
}
#endif /* CONFIG_ESBC_HDR_LS */

#if defined(CONFIG_FSL_ISBC_KEY_EXT)

static void install_ie_tbl(uintptr_t ie_tbl_addr,
		struct fsl_secboot_img_priv *img)
{
	/* Copy IE tbl to Global Data */
	memcpy(&glb.ie_tbl, (u8 *)ie_tbl_addr, sizeof(struct ie_key_info));
	img->ie_addr = (uintptr_t)&glb.ie_tbl;
	glb.ie_addr = img->ie_addr;
}

static u32 read_validate_ie_tbl(struct fsl_secboot_img_priv *img)
{
	struct fsl_secboot_img_hdr *hdr = &img->hdr;
	u32 ie_key_len, ie_revoc_flag, ie_num;
	struct ie_key_info *ie_info;

	if (!img->ie_addr) {
		if (get_ie_info_addr(&img->ie_addr))
			return ERROR_IE_TABLE_NOT_FOUND;
		else
			install_ie_tbl(img->ie_addr, img);
		}

	ie_info = (struct ie_key_info *)(uintptr_t)img->ie_addr;
	if (ie_info->num_keys == 0 || ie_info->num_keys > 32)
		return ERROR_ESBC_CLIENT_HEADER_INVALID_IE_NUM_ENTRY;

	ie_num = hdr->ie_key_sel;
	if (ie_num == 0 || ie_num > ie_info->num_keys)
		return ERROR_ESBC_CLIENT_HEADER_INVALID_IE_KEY_NUM;

	ie_revoc_flag = ie_info->key_revok;
	if ((u32)(1 << (ie_num - 1)) & ie_revoc_flag)
		return ERROR_ESBC_CLIENT_HEADER_IE_KEY_REVOKED;

	ie_key_len = ie_info->ie_key_tbl[ie_num - 1].key_len;

	if (!CHECK_KEY_LEN(ie_key_len))
		return ERROR_ESBC_CLIENT_HEADER_INV_IE_ENTRY_KEYLEN;

	memcpy(&img->img_key, &(ie_info->ie_key_tbl[ie_num - 1].pkey),
	       ie_key_len);

	img->key_len = ie_key_len;
	return 0;
}
#endif


/* This function return length of public key.*/
static inline u32 get_key_len(struct fsl_secboot_img_priv *img)
{
	return img->key_len;
}

/*
 * Handles the ESBC uboot client header verification failure.
 * This  function  handles all the errors which might occur in the
 * parsing and checking of ESBC uboot client header. It will also
 * set the error bits in the SEC_MON.
 */
static void fsl_secboot_header_verification_failure(void)
{
	struct ccsr_sfp_regs *sfp_regs = (void *)(CONFIG_SYS_SFP_ADDR);

	/* 29th bit of OSPR is ITS */
	u32 its = sfp_in32(&sfp_regs->ospr) >> 2;

	if (its == 1)
		set_sec_mon_state(HPSR_SSM_ST_SOFT_FAIL);
	else
		set_sec_mon_state(HPSR_SSM_ST_NON_SECURE);

	printf("Generating reset request\n");
	do_reset(NULL, 0, 0, NULL);
	/* If reset doesn't coocur, halt execution */
	do_esbc_halt(NULL, 0, 0, NULL);
}

/*
 * Handles the ESBC uboot client image verification failure.
 * This  function  handles all the errors which might occur in the
 * public key hash comparison and signature verification of
 * ESBC uboot client image. It will also
 * set the error bits in the SEC_MON.
 */
static void fsl_secboot_image_verification_failure(void)
{
	struct ccsr_sfp_regs *sfp_regs = (void *)(CONFIG_SYS_SFP_ADDR);

	u32 its = (sfp_in32(&sfp_regs->ospr) & ITS_MASK) >> ITS_BIT;

	if (its == 1) {
		set_sec_mon_state(HPSR_SSM_ST_SOFT_FAIL);

		printf("Generating reset request\n");
		do_reset(NULL, 0, 0, NULL);
		/* If reset doesn't coocur, halt execution */
		do_esbc_halt(NULL, 0, 0, NULL);

	} else {
		set_sec_mon_state(HPSR_SSM_ST_NON_SECURE);
	}
}

static void fsl_secboot_bootscript_parse_failure(void)
{
	fsl_secboot_header_verification_failure();
}

/*
 * Handles the errors in esbc boot.
 * This  function  handles all the errors which might occur in the
 * esbc boot phase. It will call the appropriate api to log the
 * errors and set the error bits in the SEC_MON.
 */
void fsl_secboot_handle_error(int error)
{
#ifndef CONFIG_SPL_BUILD
	const struct fsl_secboot_errcode *e;

	for (e = fsl_secboot_errcodes; e->errcode != ERROR_ESBC_CLIENT_MAX;
		e++) {
		if (e->errcode == error)
			printf("ERROR :: %x :: %s\n", error, e->name);
	}
#else
	printf("ERROR :: %x\n", error);
#endif

	/* If Boot Mode is secure, transition the SNVS state and issue
	 * reset based on type of failure and ITS setting.
	 * If Boot mode is non-secure, return from this function.
	 */
	if (fsl_check_boot_mode_secure() == 0)
		return;

	switch (error) {
	case ERROR_ESBC_CLIENT_HEADER_BARKER:
	case ERROR_ESBC_CLIENT_HEADER_IMG_SIZE:
	case ERROR_ESBC_CLIENT_HEADER_KEY_LEN:
	case ERROR_ESBC_CLIENT_HEADER_SIG_LEN:
	case ERROR_ESBC_CLIENT_HEADER_KEY_LEN_NOT_TWICE_SIG_LEN:
	case ERROR_ESBC_CLIENT_HEADER_KEY_MOD_1:
	case ERROR_ESBC_CLIENT_HEADER_KEY_MOD_2:
	case ERROR_ESBC_CLIENT_HEADER_SIG_KEY_MOD:
	case ERROR_ESBC_CLIENT_HEADER_SG_ESBC_EP:
	case ERROR_ESBC_CLIENT_HEADER_SG_ENTIRES_BAD:
	case ERROR_KEY_TABLE_NOT_FOUND:
#ifdef CONFIG_KEY_REVOCATION
	case ERROR_ESBC_CLIENT_HEADER_KEY_REVOKED:
	case ERROR_ESBC_CLIENT_HEADER_INVALID_SRK_NUM_ENTRY:
	case ERROR_ESBC_CLIENT_HEADER_INVALID_KEY_NUM:
	case ERROR_ESBC_CLIENT_HEADER_INV_SRK_ENTRY_KEYLEN:
#endif
#if defined(CONFIG_FSL_ISBC_KEY_EXT)
	/*@fallthrough@*/
	case ERROR_ESBC_CLIENT_HEADER_IE_KEY_REVOKED:
	case ERROR_ESBC_CLIENT_HEADER_INVALID_IE_NUM_ENTRY:
	case ERROR_ESBC_CLIENT_HEADER_INVALID_IE_KEY_NUM:
	case ERROR_ESBC_CLIENT_HEADER_INV_IE_ENTRY_KEYLEN:
	case ERROR_IE_TABLE_NOT_FOUND:
#endif
		fsl_secboot_header_verification_failure();
		break;
	case ERROR_ESBC_SEC_RESET:
	case ERROR_ESBC_SEC_DEQ:
	case ERROR_ESBC_SEC_ENQ:
	case ERROR_ESBC_SEC_DEQ_TO:
	case ERROR_ESBC_SEC_JOBQ_STATUS:
	case ERROR_ESBC_CLIENT_HASH_COMPARE_KEY:
	case ERROR_ESBC_CLIENT_HASH_COMPARE_EM:
		fsl_secboot_image_verification_failure();
		break;
	case ERROR_ESBC_MISSING_BOOTM:
		fsl_secboot_bootscript_parse_failure();
		break;
	case ERROR_ESBC_WRONG_CMD:
	default:
		branch_to_self();
		break;
	}
}

static void fsl_secblk_handle_error(int error)
{
	switch (error) {
	case ERROR_ESBC_SEC_ENQ:
		fsl_secboot_handle_error(ERROR_ESBC_SEC_ENQ);
		break;
	case ERROR_ESBC_SEC_DEQ:
		fsl_secboot_handle_error(ERROR_ESBC_SEC_DEQ);
		break;
	case ERROR_ESBC_SEC_DEQ_TO:
		fsl_secboot_handle_error(ERROR_ESBC_SEC_DEQ_TO);
		break;
	default:
		printf("Job Queue Output status %x\n", error);
		fsl_secboot_handle_error(ERROR_ESBC_SEC_JOBQ_STATUS);
		break;
	}
}

/*
 * Calculate hash of key obtained via offset present in ESBC uboot
 * client hdr. This function calculates the hash of key which is obtained
 * through offset present in ESBC uboot client header.
 */
static int calc_img_key_hash(struct fsl_secboot_img_priv *img)
{
	struct hash_algo *algo;
	void *ctx;
	int i, srk = 0;
	int ret = 0;
	const char *algo_name = "sha256";

	/* Calculate hash of the esbc key */
	ret = hash_progressive_lookup_algo(algo_name, &algo);
	if (ret)
		return ret;

	ret = algo->hash_init(algo, &ctx);
	if (ret)
		return ret;

	/* Update hash for ESBC key */
#ifdef CONFIG_KEY_REVOCATION
	if (check_srk(img)) {
		ret = algo->hash_update(algo, ctx,
		      (u8 *)(uintptr_t)(img->ehdrloc + img->hdr.srk_tbl_off),
		      img->hdr.len_kr.num_srk * sizeof(struct srk_table), 1);
		srk = 1;
	}
#endif
	if (!srk)
		ret = algo->hash_update(algo, ctx,
			img->img_key, img->key_len, 1);
	if (ret)
		return ret;

	/* Copy hash at destination buffer */
	ret = algo->hash_finish(algo, ctx, hash_val, algo->digest_size);
	if (ret)
		return ret;

	for (i = 0; i < SHA256_BYTES; i++)
		img->img_key_hash[i] = hash_val[i];

	return 0;
}

/*
 * Calculate hash of ESBC hdr and ESBC. This function calculates the
 * single hash of ESBC header and ESBC image. If SG flag is on, all
 * SG entries are also hashed alongwith the complete SG table.
 */
static int calc_esbchdr_esbc_hash(struct fsl_secboot_img_priv *img)
{
	struct hash_algo *algo;
	void *ctx;
	int ret = 0;
	int key_hash = 0;
	const char *algo_name = "sha256";

	/* Calculate the hash of the ESBC */
	ret = hash_progressive_lookup_algo(algo_name, &algo);
	if (ret)
		return ret;

	ret = algo->hash_init(algo, &ctx);
	/* Copy hash at destination buffer */
	if (ret)
		return ret;

	/* Update hash for CSF Header */
	ret = algo->hash_update(algo, ctx,
		(u8 *)&img->hdr, sizeof(struct fsl_secboot_img_hdr), 0);
	if (ret)
		return ret;

	/* Update the hash with that of srk table if srk flag is 1
	 * If IE Table is selected, key is not added in the hash
	 * If neither srk table nor IE key table available, add key
	 * from header in the hash calculation
	 */
#ifdef CONFIG_KEY_REVOCATION
	if (check_srk(img)) {
		ret = algo->hash_update(algo, ctx,
		      (u8 *)(uintptr_t)(img->ehdrloc + img->hdr.srk_tbl_off),
		      img->hdr.len_kr.num_srk * sizeof(struct srk_table), 0);
		key_hash = 1;
	}
#endif
#if defined(CONFIG_FSL_ISBC_KEY_EXT)
	if (!key_hash && check_ie(img))
		key_hash = 1;
#endif
#ifndef CONFIG_ESBC_HDR_LS
/* No single key support in LS ESBC header */
	if (!key_hash) {
		ret = algo->hash_update(algo, ctx,
			img->img_key, img->hdr.key_len, 0);
		key_hash = 1;
	}
#endif
	if (ret)
		return ret;
	if (!key_hash)
		return ERROR_KEY_TABLE_NOT_FOUND;

	/* Update hash for actual Image */
	ret = algo->hash_update(algo, ctx,
		(u8 *)(*(img->img_addr_ptr)), img->img_size, 1);
	if (ret)
		return ret;

	/* Copy hash at destination buffer */
	ret = algo->hash_finish(algo, ctx, hash_val, algo->digest_size);
	if (ret)
		return ret;

	return 0;
}

/*
 * Construct encoded hash EM' wrt PKCSv1.5. This function calculates the
 * pointers for padding, DER value and hash. And finally, constructs EM'
 * which includes hash of complete CSF header and ESBC image. If SG flag
 * is on, hash of SG table and entries is also included.
 */
static void construct_img_encoded_hash_second(struct fsl_secboot_img_priv *img)
{
	/*
	 * RSA PKCSv1.5 encoding format for encoded message is below
	 * EM = 0x0 || 0x1 || PS || 0x0 || DER || Hash
	 * PS is Padding String
	 * DER is DER value for SHA-256
	 * Hash is SHA-256 hash
	 * *********************************************************
	 * representative points to first byte of EM initially and is
	 * filled with 0x0
	 * representative is incremented by 1 and second byte is filled
	 * with 0x1
	 * padding points to third byte of EM
	 * digest points to full length of EM - 32 bytes
	 * hash_id (DER value) points to 19 bytes before pDigest
	 * separator is one byte which separates padding and DER
	 */

	size_t len;
	u8 *representative;
	u8 *padding, *digest;
	u8 *hash_id, *separator;
	int i;

	len = (get_key_len(img) / 2) - 1;
	representative = img->img_encoded_hash_second;
	representative[0] = 0;
	representative[1] = 1;  /* block type 1 */

	padding = &representative[2];
	digest = &representative[1] + len - 32;
	hash_id = digest - sizeof(hash_identifier);
	separator = hash_id - 1;

	/* fill padding area pointed by padding with 0xff */
	memset(padding, 0xff, separator - padding);

	/* fill byte pointed by separator */
	*separator = 0;

	/* fill SHA-256 DER value  pointed by HashId */
	memcpy(hash_id, hash_identifier, sizeof(hash_identifier));

	/* fill hash pointed by Digest */
	for (i = 0; i < SHA256_BYTES; i++)
		digest[i] = hash_val[i];
}

/*
 * Reads and validates the ESBC client header.
 * This function reads key and signature from the ESBC client header.
 * If Scatter/Gather flag is on, lengths and offsets of images
 * present as SG entries are also read. This function also checks
 * whether the header is valid or not.
 */
static int read_validate_esbc_client_header(struct fsl_secboot_img_priv *img)
{
	struct fsl_secboot_img_hdr *hdr = &img->hdr;
	void *esbc = (u8 *)(uintptr_t)img->ehdrloc;
	u8 *k, *s;
	u32 ret = 0;

	int  key_found = 0;

	/* check barker code */
	if (memcmp(hdr->barker, barker_code, ESBC_BARKER_LEN))
		return ERROR_ESBC_CLIENT_HEADER_BARKER;

	/* If Image Address is not passed as argument to function,
	 * then Address and Size must be read from the Header.
	 */
	if (*(img->img_addr_ptr) == 0) {
	#ifdef CONFIG_ESBC_ADDR_64BIT
		*(img->img_addr_ptr) = hdr->pimg64;
	#else
		*(img->img_addr_ptr) = hdr->pimg;
	#endif
	}

	if (!hdr->img_size)
		return ERROR_ESBC_CLIENT_HEADER_IMG_SIZE;

	img->img_size = hdr->img_size;

	/* Key checking*/
#ifdef CONFIG_KEY_REVOCATION
	if (check_srk(img)) {
		ret = read_validate_srk_tbl(img);
		if (ret != 0)
			return ret;
		key_found = 1;
	}
#endif

#if defined(CONFIG_FSL_ISBC_KEY_EXT)
	if (!key_found && check_ie(img)) {
		ret = read_validate_ie_tbl(img);
		if (ret != 0)
			return ret;
		key_found = 1;
	}
#endif
#ifndef CONFIG_ESBC_HDR_LS
/* Single Key Feature not available in LS ESBC Header */
	if (key_found == 0) {
		ret = read_validate_single_key(img);
		if (ret != 0)
			return ret;
		key_found = 1;
	}
#endif
	if (!key_found)
		return ERROR_KEY_TABLE_NOT_FOUND;

	/* check signaure */
	if (get_key_len(img) == 2 * hdr->sign_len) {
		/* check signature length */
		if (!((hdr->sign_len == KEY_SIZE_BYTES / 4) ||
		      (hdr->sign_len == KEY_SIZE_BYTES / 2) ||
		      (hdr->sign_len == KEY_SIZE_BYTES)))
			return ERROR_ESBC_CLIENT_HEADER_SIG_LEN;
	} else {
		return ERROR_ESBC_CLIENT_HEADER_KEY_LEN_NOT_TWICE_SIG_LEN;
	}

	memcpy(&img->img_sign, esbc + hdr->psign, hdr->sign_len);
/* No SG support in LS-CH3 */
#ifndef CONFIG_ESBC_HDR_LS
	/* No SG support */
	if (hdr->sg_flag)
		return ERROR_ESBC_CLIENT_HEADER_SG;
#endif

	/* modulus most significant bit should be set */
	k = (u8 *)&img->img_key;

	if ((k[0] & 0x80) == 0)
		return ERROR_ESBC_CLIENT_HEADER_KEY_MOD_1;

	/* modulus value should be odd */
	if ((k[get_key_len(img) / 2 - 1] & 0x1) == 0)
		return ERROR_ESBC_CLIENT_HEADER_KEY_MOD_2;

	/* Check signature value < modulus value */
	s = (u8 *)&img->img_sign;

	if (!(memcmp(s, k, hdr->sign_len) < 0))
		return ERROR_ESBC_CLIENT_HEADER_SIG_KEY_MOD;

	return ESBC_VALID_HDR;
}

static inline int str2longbe(const char *p, ulong *num)
{
	char *endptr;
	ulong tmp;

	if (!p) {
		return 0;
	} else {
		tmp = simple_strtoul(p, &endptr, 16);
		if (sizeof(ulong) == 4)
			*num = cpu_to_be32(tmp);
		else
			*num = cpu_to_be64(tmp);
	}

	return *p != '\0' && *endptr == '\0';
}
/* Function to calculate the ESBC Image Hash
 * and hash from Digital signature.
 * The Two hash's are compared to yield the
 * result of signature validation.
 */
static int calculate_cmp_img_sig(struct fsl_secboot_img_priv *img)
{
	int ret;
	uint32_t key_len;
	struct key_prop prop;
#if !defined(USE_HOSTCC)
	struct udevice *mod_exp_dev;
#endif
	ret = calc_esbchdr_esbc_hash(img);
	if (ret)
		return ret;

	/* Construct encoded hash EM' wrt PKCSv1.5 */
	construct_img_encoded_hash_second(img);

	/* Fill prop structure for public key */
	memset(&prop, 0, sizeof(struct key_prop));
	key_len = get_key_len(img) / 2;
	prop.modulus = img->img_key;
	prop.public_exponent = img->img_key + key_len;
	prop.num_bits = key_len * 8;
	prop.exp_len = key_len;

	ret = uclass_get_device(UCLASS_MOD_EXP, 0, &mod_exp_dev);
	if (ret) {
		printf("RSA: Can't find Modular Exp implementation\n");
		return -EINVAL;
	}

	ret = rsa_mod_exp(mod_exp_dev, img->img_sign, img->hdr.sign_len,
			  &prop, img->img_encoded_hash);
	if (ret)
		return ret;

	/*
	 * compare the encoded messages EM' and EM wrt RSA PKCSv1.5
	 * memcmp returns zero on success
	 * memcmp returns non-zero on failure
	 */
	ret = memcmp(&img->img_encoded_hash_second, &img->img_encoded_hash,
		img->hdr.sign_len);

	if (ret)
		return ERROR_ESBC_CLIENT_HASH_COMPARE_EM;

	return 0;
}
/* Function to initialize img priv and global data structure
 */
static int secboot_init(struct fsl_secboot_img_priv **img_ptr)
{
	*img_ptr = malloc(sizeof(struct fsl_secboot_img_priv));

	struct fsl_secboot_img_priv *img = *img_ptr;

	if (!img)
		return -ENOMEM;
	memset(img, 0, sizeof(struct fsl_secboot_img_priv));

#if defined(CONFIG_FSL_ISBC_KEY_EXT)
	if (glb.ie_addr)
		img->ie_addr = glb.ie_addr;
#endif
	return 0;
}


/* haddr - Address of the header of image to be validated.
 * arg_hash_str - Option hash string. If provided, this
 * overrides the key hash in the SFP fuses.
 * img_addr_ptr - Optional pointer to address of image to be validated.
 * If non zero addr, this overrides the addr of image in header,
 * otherwise updated to image addr in header.
 * Acts as both input and output of function.
 * This pointer shouldn't be NULL.
 */
int fsl_secboot_validate(uintptr_t haddr, char *arg_hash_str,
			uintptr_t *img_addr_ptr)
{
	struct ccsr_sfp_regs *sfp_regs = (void *)(CONFIG_SYS_SFP_ADDR);
	ulong hash[SHA256_BYTES/sizeof(ulong)];
	char hash_str[NUM_HEX_CHARS + 1];
	struct fsl_secboot_img_priv *img;
	struct fsl_secboot_img_hdr *hdr;
	void *esbc;
	int ret, i, hash_cmd = 0;
	u32 srk_hash[8];

	if (arg_hash_str != NULL) {
		const char *cp = arg_hash_str;
		int i = 0;

		if (*cp == '0' && *(cp + 1) == 'x')
			cp += 2;

		/* The input string expected is in hex, where
		 * each 4 bits would be represented by a hex
		 * sha256 hash is 256 bits long, which would mean
		 * num of characters = 256 / 4
		 */
		if (strlen(cp) != SHA256_NIBBLES) {
			printf("%s is not a 256 bits hex string as expected\n",
			       arg_hash_str);
			return -1;
		}

		for (i = 0; i < sizeof(hash)/sizeof(ulong); i++) {
			strncpy(hash_str, cp + (i * NUM_HEX_CHARS),
				NUM_HEX_CHARS);
			hash_str[NUM_HEX_CHARS] = '\0';
			if (!str2longbe(hash_str, &hash[i])) {
				printf("%s is not a 256 bits hex string ",
				       arg_hash_str);
				return -1;
			}
		}

		hash_cmd = 1;
	}

	ret = secboot_init(&img);
	if (ret)
		goto exit;

	/* Update the information in Private Struct */
	hdr = &img->hdr;
	img->ehdrloc = haddr;
	img->img_addr_ptr = img_addr_ptr;
	esbc = (u8 *)img->ehdrloc;

	memcpy(hdr, esbc, sizeof(struct fsl_secboot_img_hdr));

	/* read and validate esbc header */
	ret = read_validate_esbc_client_header(img);

	if (ret != ESBC_VALID_HDR) {
		fsl_secboot_handle_error(ret);
		goto exit;
	}

	/* SRKH present in SFP */
	for (i = 0; i < NUM_SRKH_REGS; i++)
		srk_hash[i] = srk_in32(&sfp_regs->srk_hash[i]);

	/*
	 * Calculate hash of key obtained via offset present in
	 * ESBC uboot client hdr
	 */
	ret = calc_img_key_hash(img);
	if (ret) {
		fsl_secblk_handle_error(ret);
		goto exit;
	}

	/* Compare hash obtained above with SRK hash present in SFP */
	if (hash_cmd)
		ret = memcmp(&hash, &img->img_key_hash, SHA256_BYTES);
	else
		ret = memcmp(srk_hash, img->img_key_hash, SHA256_BYTES);

#if defined(CONFIG_FSL_ISBC_KEY_EXT)
	if (!hash_cmd && check_ie(img))
		ret = 0;
#endif

	if (ret != 0) {
		fsl_secboot_handle_error(ERROR_ESBC_CLIENT_HASH_COMPARE_KEY);
		goto exit;
	}

	ret = calculate_cmp_img_sig(img);
	if (ret) {
		fsl_secboot_handle_error(ret);
		goto exit;
	}

exit:
	/* Free Img as it was malloc'ed*/
	free(img);
	return ret;
}
