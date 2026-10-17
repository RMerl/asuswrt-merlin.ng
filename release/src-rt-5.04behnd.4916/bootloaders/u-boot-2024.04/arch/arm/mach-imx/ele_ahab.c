// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <imx_container.h>
#include <asm/io.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/mach-imx/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/arch/sys_proto.h>
#include <console.h>
#include <cpu_func.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define IMG_CONTAINER_END_BASE         (IMG_CONTAINER_BASE + 0xFFFFUL)

#define AHAB_MAX_EVENTS 8

static char *ele_ipc_str[] = {
	"IPC = MU RTD (0x1)\n",
	"IPC = MU APD (0x2)\n",
	"IPC = INVALID\n",
	NULL
};

static char *ele_status_str[] = {
	"STA = ELE_SUCCESS_IND (0xD6)\n",
	"STA = ELE_FAILURE_IND (0x29)\n",
	"STA = INVALID\n",
	NULL
};

static char *ele_cmd_str[] = {
	"CMD = ELE_PING_REQ (0x01)\n",
	"CMD = ELE_FW_AUTH_REQ (0x02)\n",
	"CMD = ELE_RESTART_RST_TIMER_REQ (0x04)\n",
	"CMD = ELE_DUMP_DEBUG_BUFFER_REQ (0x21)\n",
	"CMD = ELE_OEM_CNTN_AUTH_REQ (0x87)\n",
	"CMD = ELE_VERIFY_IMAGE_REQ (0x88)\n",
	"CMD = ELE_RELEASE_CONTAINER_REQ (0x89)\n",
	"CMD = ELE_WRITE_SECURE_FUSE_REQ (0x91)\n",
	"CMD = ELE_FWD_LIFECYCLE_UP_REQ (0x95)\n",
	"CMD = ELE_READ_FUSE_REQ (0x97)\n",
	"CMD = ELE_GET_FW_VERSION_REQ (0x9D)\n",
	"CMD = ELE_RET_LIFECYCLE_UP_REQ (0xA0)\n",
	"CMD = ELE_GET_EVENTS_REQ (0xA2)\n",
	"CMD = ELE_ENABLE_PATCH_REQ (0xC3)\n",
	"CMD = ELE_RELEASE_RDC_REQ (0xC4)\n",
	"CMD = ELE_GET_FW_STATUS_REQ (0xC5)\n",
	"CMD = ELE_ENABLE_OTFAD_REQ (0xC6)\n",
	"CMD = ELE_RESET_REQ (0xC7)\n",
	"CMD = ELE_UPDATE_OTP_CLKDIV_REQ (0xD0)\n",
	"CMD = ELE_POWER_DOWN_REQ (0xD1)\n",
	"CMD = ELE_ENABLE_APC_REQ (0xD2)\n",
	"CMD = ELE_ENABLE_RTC_REQ (0xD3)\n",
	"CMD = ELE_DEEP_POWER_DOWN_REQ (0xD4)\n",
	"CMD = ELE_STOP_RST_TIMER_REQ (0xD5)\n",
	"CMD = ELE_WRITE_FUSE_REQ (0xD6)\n",
	"CMD = ELE_RELEASE_CAAM_REQ (0xD7)\n",
	"CMD = ELE_RESET_A35_CTX_REQ (0xD8)\n",
	"CMD = ELE_MOVE_TO_UNSECURED_REQ (0xD9)\n",
	"CMD = ELE_GET_INFO_REQ (0xDA)\n",
	"CMD = ELE_ATTEST_REQ (0xDB)\n",
	"CMD = ELE_RELEASE_PATCH_REQ (0xDC)\n",
	"CMD = ELE_OTP_SEQ_SWITH_REQ (0xDD)\n",
	"CMD = INVALID\n",
	NULL
};

static char *ele_ind_str[] = {
	"IND = ELE_ROM_PING_FAILURE_IND (0x0A)\n",
	"IND = ELE_FW_PING_FAILURE_IND (0x1A)\n",
	"IND = ELE_BAD_SIGNATURE_FAILURE_IND (0xF0)\n",
	"IND = ELE_BAD_HASH_FAILURE_IND (0xF1)\n",
	"IND = ELE_INVALID_LIFECYCLE_IND (0xF2)\n",
	"IND = ELE_PERMISSION_DENIED_FAILURE_IND (0xF3)\n",
	"IND = ELE_INVALID_MESSAGE_FAILURE_IND (0xF4)\n",
	"IND = ELE_BAD_VALUE_FAILURE_IND (0xF5)\n",
	"IND = ELE_BAD_FUSE_ID_FAILURE_IND (0xF6)\n",
	"IND = ELE_BAD_CONTAINER_FAILURE_IND (0xF7)\n",
	"IND = ELE_BAD_VERSION_FAILURE_IND (0xF8)\n",
	"IND = ELE_INVALID_KEY_FAILURE_IND (0xF9)\n",
	"IND = ELE_BAD_KEY_HASH_FAILURE_IND (0xFA)\n",
	"IND = ELE_NO_VALID_CONTAINER_FAILURE_IND (0xFB)\n",
	"IND = ELE_BAD_CERTIFICATE_FAILURE_IND (0xFC)\n",
	"IND = ELE_BAD_UID_FAILURE_IND (0xFD)\n",
	"IND = ELE_BAD_MONOTONIC_COUNTER_FAILURE_IND (0xFE)\n",
	"IND = ELE_MUST_SIGNED_FAILURE_IND (0xE0)\n",
	"IND = ELE_NO_AUTHENTICATION_FAILURE_IND (0xEE)\n",
	"IND = ELE_BAD_SRK_SET_FAILURE_IND (0xEF)\n",
	"IND = ELE_UNALIGNED_PAYLOAD_FAILURE_IND (0xA6)\n",
	"IND = ELE_WRONG_SIZE_FAILURE_IND (0xA7)\n",
	"IND = ELE_ENCRYPTION_FAILURE_IND (0xA8)\n",
	"IND = ELE_DECRYPTION_FAILURE_IND (0xA9)\n",
	"IND = ELE_OTP_PROGFAIL_FAILURE_IND (0xAA)\n",
	"IND = ELE_OTP_LOCKED_FAILURE_IND (0xAB)\n",
	"IND = ELE_OTP_INVALID_IDX_FAILURE_IND (0xAD)\n",
	"IND = ELE_TIME_OUT_FAILURE_IND (0xB0)\n",
	"IND = ELE_BAD_PAYLOAD_FAILURE_IND (0xB1)\n",
	"IND = ELE_WRONG_ADDRESS_FAILURE_IND (0xB4)\n",
	"IND = ELE_DMA_FAILURE_IND (0xB5)\n",
	"IND = ELE_DISABLED_FEATURE_FAILURE_IND (0xB6)\n",
	"IND = ELE_MUST_ATTEST_FAILURE_IND (0xB7)\n",
	"IND = ELE_RNG_NOT_STARTED_FAILURE_IND (0xB8)\n",
	"IND = ELE_CRC_ERROR_IND (0xB9)\n",
	"IND = ELE_AUTH_SKIPPED_OR_FAILED_FAILURE_IND (0xBB)\n",
	"IND = ELE_INCONSISTENT_PAR_FAILURE_IND (0xBC)\n",
	"IND = ELE_RNG_INST_FAILURE_FAILURE_IND (0xBD)\n",
	"IND = ELE_LOCKED_REG_FAILURE_IND (0xBE)\n",
	"IND = ELE_BAD_ID_FAILURE_IND (0xBF)\n",
	"IND = ELE_INVALID_OPERATION_FAILURE_IND (0xC0)\n",
	"IND = ELE_NON_SECURE_STATE_FAILURE_IND (0xC1)\n",
	"IND = ELE_MSG_TRUNCATED_IND (0xC2)\n",
	"IND = ELE_BAD_IMAGE_NUM_FAILURE_IND (0xC3)\n",
	"IND = ELE_BAD_IMAGE_ADDR_FAILURE_IND (0xC4)\n",
	"IND = ELE_BAD_IMAGE_PARAM_FAILURE_IND (0xC5)\n",
	"IND = ELE_BAD_IMAGE_TYPE_FAILURE_IND (0xC6)\n",
	"IND = ELE_CORRUPTED_SRK_FAILURE_IND (0xD0)\n",
	"IND = ELE_OUT_OF_MEMORY_IND (0xD1)\n",
	"IND = ELE_CSTM_FAILURE_IND (0xCF)\n",
	"IND = ELE_OLD_VERSION_FAILURE_IND (0xCE)\n",
	"IND = ELE_WRONG_BOOT_MODE_FAILURE_IND (0xCD)\n",
	"IND = ELE_APC_ALREADY_ENABLED_FAILURE_IND (0xCB)\n",
	"IND = ELE_RTC_ALREADY_ENABLED_FAILURE_IND (0xCC)\n",
	"IND = ELE_ABORT_IND (0xFF)\n",
	"IND = INVALID\n",
	NULL
};

static u8 ele_cmd[] = {
	ELE_PING_REQ,
	ELE_FW_AUTH_REQ,
	ELE_RESTART_RST_TIMER_REQ,
	ELE_DUMP_DEBUG_BUFFER_REQ,
	ELE_OEM_CNTN_AUTH_REQ,
	ELE_VERIFY_IMAGE_REQ,
	ELE_RELEASE_CONTAINER_REQ,
	ELE_WRITE_SECURE_FUSE_REQ,
	ELE_FWD_LIFECYCLE_UP_REQ,
	ELE_READ_FUSE_REQ,
	ELE_GET_FW_VERSION_REQ,
	ELE_RET_LIFECYCLE_UP_REQ,
	ELE_GET_EVENTS_REQ,
	ELE_ENABLE_PATCH_REQ,
	ELE_RELEASE_RDC_REQ,
	ELE_GET_FW_STATUS_REQ,
	ELE_ENABLE_OTFAD_REQ,
	ELE_RESET_REQ,
	ELE_UPDATE_OTP_CLKDIV_REQ,
	ELE_POWER_DOWN_REQ,
	ELE_ENABLE_APC_REQ,
	ELE_ENABLE_RTC_REQ,
	ELE_DEEP_POWER_DOWN_REQ,
	ELE_STOP_RST_TIMER_REQ,
	ELE_WRITE_FUSE_REQ,
	ELE_RELEASE_CAAM_REQ,
	ELE_RESET_A35_CTX_REQ,
	ELE_MOVE_TO_UNSECURED_REQ,
	ELE_GET_INFO_REQ,
	ELE_ATTEST_REQ,
	ELE_RELEASE_PATCH_REQ,
	ELE_OTP_SEQ_SWITH_REQ
};

static u8 ele_ind[] = {
	ELE_ROM_PING_FAILURE_IND,
	ELE_FW_PING_FAILURE_IND,
	ELE_BAD_SIGNATURE_FAILURE_IND,
	ELE_BAD_HASH_FAILURE_IND,
	ELE_INVALID_LIFECYCLE_IND,
	ELE_PERMISSION_DENIED_FAILURE_IND,
	ELE_INVALID_MESSAGE_FAILURE_IND,
	ELE_BAD_VALUE_FAILURE_IND,
	ELE_BAD_FUSE_ID_FAILURE_IND,
	ELE_BAD_CONTAINER_FAILURE_IND,
	ELE_BAD_VERSION_FAILURE_IND,
	ELE_INVALID_KEY_FAILURE_IND,
	ELE_BAD_KEY_HASH_FAILURE_IND,
	ELE_NO_VALID_CONTAINER_FAILURE_IND,
	ELE_BAD_CERTIFICATE_FAILURE_IND,
	ELE_BAD_UID_FAILURE_IND,
	ELE_BAD_MONOTONIC_COUNTER_FAILURE_IND,
	ELE_MUST_SIGNED_FAILURE_IND,
	ELE_NO_AUTHENTICATION_FAILURE_IND,
	ELE_BAD_SRK_SET_FAILURE_IND,
	ELE_UNALIGNED_PAYLOAD_FAILURE_IND,
	ELE_WRONG_SIZE_FAILURE_IND,
	ELE_ENCRYPTION_FAILURE_IND,
	ELE_DECRYPTION_FAILURE_IND,
	ELE_OTP_PROGFAIL_FAILURE_IND,
	ELE_OTP_LOCKED_FAILURE_IND,
	ELE_OTP_INVALID_IDX_FAILURE_IND,
	ELE_TIME_OUT_FAILURE_IND,
	ELE_BAD_PAYLOAD_FAILURE_IND,
	ELE_WRONG_ADDRESS_FAILURE_IND,
	ELE_DMA_FAILURE_IND,
	ELE_DISABLED_FEATURE_FAILURE_IND,
	ELE_MUST_ATTEST_FAILURE_IND,
	ELE_RNG_NOT_STARTED_FAILURE_IND,
	ELE_CRC_ERROR_IND,
	ELE_AUTH_SKIPPED_OR_FAILED_FAILURE_IND,
	ELE_INCONSISTENT_PAR_FAILURE_IND,
	ELE_RNG_INST_FAILURE_FAILURE_IND,
	ELE_LOCKED_REG_FAILURE_IND,
	ELE_BAD_ID_FAILURE_IND,
	ELE_INVALID_OPERATION_FAILURE_IND,
	ELE_NON_SECURE_STATE_FAILURE_IND,
	ELE_MSG_TRUNCATED_IND,
	ELE_BAD_IMAGE_NUM_FAILURE_IND,
	ELE_BAD_IMAGE_ADDR_FAILURE_IND,
	ELE_BAD_IMAGE_PARAM_FAILURE_IND,
	ELE_BAD_IMAGE_TYPE_FAILURE_IND,
	ELE_CORRUPTED_SRK_FAILURE_IND,
	ELE_OUT_OF_MEMORY_IND,
	ELE_CSTM_FAILURE_IND,
	ELE_OLD_VERSION_FAILURE_IND,
	ELE_WRONG_BOOT_MODE_FAILURE_IND,
	ELE_APC_ALREADY_ENABLED_FAILURE_IND,
	ELE_RTC_ALREADY_ENABLED_FAILURE_IND,
	ELE_ABORT_IND
};

static u8 ele_ipc[] = {
	ELE_IPC_MU_RTD,
	ELE_IPC_MU_APD
};

static u8 ele_status[] = {
	ELE_SUCCESS_IND,
	ELE_FAILURE_IND
};

static inline u32 get_idx(u8 *list, u8 tgt, u32 size)
{
	u32 i;

	for (i = 0; i < size; i++) {
		if (list[i] == tgt)
			return i;
	}

	return i; /* last str is invalid */
}

static void display_ahab_auth_ind(u32 event)
{
	u8 resp_ind = (event >> 8) & 0xff;

	printf("%s\n", ele_ind_str[get_idx(ele_ind, resp_ind, ARRAY_SIZE(ele_ind))]);
}

int ahab_auth_cntr_hdr(struct container_hdr *container, u16 length)
{
	int err;
	u32 resp;

	memcpy((void *)IMG_CONTAINER_BASE, (const void *)container,
	       ALIGN(length, CONFIG_SYS_CACHELINE_SIZE));

	flush_dcache_range(IMG_CONTAINER_BASE,
			   IMG_CONTAINER_BASE + ALIGN(length, CONFIG_SYS_CACHELINE_SIZE) - 1);

	err = ele_auth_oem_ctnr(IMG_CONTAINER_BASE, &resp);
	if (err) {
		printf("Authenticate container hdr failed, return %d, resp 0x%x\n",
		       err, resp);
		display_ahab_auth_ind(resp);
	}

	return err;
}

int ahab_auth_release(void)
{
	int err;
	u32 resp;

	err = ele_release_container(&resp);
	if (err) {
		printf("Error: release container failed, resp 0x%x!\n", resp);
		display_ahab_auth_ind(resp);
	}

	return err;
}

int ahab_verify_cntr_image(struct boot_img_t *img, int image_index)
{
	int err;
	u32 resp;

	err = ele_verify_image(image_index, &resp);
	if (err) {
		printf("Authenticate img %d failed, return %d, resp 0x%x\n",
		       image_index, err, resp);
		display_ahab_auth_ind(resp);

		return -EIO;
	}

	return 0;
}

static inline bool check_in_dram(ulong addr)
{
	int i;
	struct bd_info *bd = gd->bd;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			if (addr >= bd->bi_dram[i].start &&
			    addr < (bd->bi_dram[i].start + bd->bi_dram[i].size))
				return true;
		}
	}

	return false;
}

int authenticate_os_container(ulong addr)
{
	struct container_hdr *phdr;
	int i, ret = 0;
	int err;
	u16 length;
	struct boot_img_t *img;
	unsigned long s, e;

	if (addr % 4) {
		puts("Error: Image's address is not 4 byte aligned\n");
		return -EINVAL;
	}

	if (!check_in_dram(addr)) {
		puts("Error: Image's address is invalid\n");
		return -EINVAL;
	}

	phdr = (struct container_hdr *)addr;
	if (!valid_container_hdr(phdr)) {
		printf("Error: Wrong container header\n");
		return -EFAULT;
	}

	if (!phdr->num_images) {
		printf("Error: Wrong container, no image found\n");
		return -EFAULT;
	}

	length = phdr->length_lsb + (phdr->length_msb << 8);

	debug("container length %u\n", length);

	err = ahab_auth_cntr_hdr(phdr, length);
	if (err) {
		ret = -EIO;
		goto exit;
	}

	debug("Verify images\n");

	/* Copy images to dest address */
	for (i = 0; i < phdr->num_images; i++) {
		img = (struct boot_img_t *)(addr +
					    sizeof(struct container_hdr) +
					    i * sizeof(struct boot_img_t));

		debug("img %d, dst 0x%x, src 0x%lx, size 0x%x\n",
		      i, (uint32_t)img->dst, img->offset + addr, img->size);

		memcpy((void *)img->dst, (const void *)(img->offset + addr),
		       img->size);

		s = img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
		e = ALIGN(img->dst + img->size, CONFIG_SYS_CACHELINE_SIZE) - 1;

		flush_dcache_range(s, e);

		ret = ahab_verify_cntr_image(img, i);
		if (ret)
			goto exit;
	}

exit:
	debug("ahab_auth_release, 0x%x\n", ret);
	ahab_auth_release();

	return ret;
}

static int do_authenticate(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	ulong addr;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);

	printf("Authenticate OS container at 0x%lx\n", addr);

	if (authenticate_os_container(addr))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static void display_life_cycle(u32 lc)
{
	printf("Lifecycle: 0x%08X, ", lc);
	switch (lc) {
	case 0x1:
		printf("BLANK\n\n");
		break;
	case 0x2:
		printf("FAB\n\n");
		break;
	case 0x4:
		printf("NXP Provisioned\n\n");
		break;
	case 0x8:
		printf("OEM Open\n\n");
		break;
	case 0x20:
		printf("OEM closed\n\n");
		break;
	case 0x40:
		printf("Field Return OEM\n\n");
		break;
	case 0x80:
		printf("Field Return NXP\n\n");
		break;
	case 0x100:
		printf("OEM Locked\n\n");
		break;
	case 0x200:
		printf("BRICKED\n\n");
		break;
	default:
		printf("Unknown\n\n");
		break;
	}
}

static int confirm_close(void)
{
	puts("Warning: Please ensure your sample is in NXP closed state, "
	     "OEM SRK hash has been fused, \n"
	     "         and you are able to boot a signed image successfully "
	     "without any SECO events reported.\n"
	     "         If not, your sample will be unrecoverable.\n"
	     "\nReally perform this operation? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Ahab close aborted\n");
	return 0;
}

static int do_ahab_close(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int err;
	u32 resp;
	u32 lc;

	if (!confirm_close())
		return -EACCES;

	lc = readl(FSB_BASE_ADDR + 0x41c);
	lc &= 0x3ff;

	if (lc != 0x8) {
		puts("Current lifecycle is NOT OEM open, can't move to OEM closed\n");
		display_life_cycle(lc);
		return -EPERM;
	}

	err = ele_forward_lifecycle(8, &resp);
	if (err != 0) {
		printf("Error in forward lifecycle to OEM closed\n");
		return -EIO;
	}

	printf("Change to OEM closed successfully\n");

	return 0;
}

int ahab_dump(void)
{
	u32 buffer[32];
	int ret, i = 0;

	do {
		ret = ele_dump_buffer(buffer, 32);
		if (ret < 0) {
			printf("Error in dump AHAB log\n");
			return -EIO;
		}

		if (ret == 1)
			break;
		for (i = 0; i < ret; i++)
			printf("0x%x\n", buffer[i]);
	} while (ret >= 21);

	return 0;
}

static int do_ahab_dump(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return ahab_dump();
}

static void display_event(u32 event)
{
	printf("\n\t0x%08x\n", event);
	printf("\t%s", ele_ipc_str[get_idx(ele_ipc,
		(event >> 24) & 0xFF, ARRAY_SIZE(ele_ipc))]);
	printf("\t%s", ele_cmd_str[get_idx(ele_cmd,
		(event >> 16) & 0xFF, ARRAY_SIZE(ele_cmd))]);
	printf("\t%s", ele_ind_str[get_idx(ele_ind,
		(event >> 8) & 0xFF, ARRAY_SIZE(ele_ind))]);
	printf("\t%s", ele_status_str[get_idx(ele_status,
		event & 0xFF, ARRAY_SIZE(ele_status))]);
}

static int do_ahab_status(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 lc, i;
	u32 events[AHAB_MAX_EVENTS];
	u32 cnt = AHAB_MAX_EVENTS;
	int ret;

	lc = readl(FSB_BASE_ADDR + 0x41c);
	lc &= 0x3ff;

	display_life_cycle(lc);

	ret = ele_get_events(events, &cnt, NULL);
	if (ret) {
		printf("Get ELE EVENTS error %d\n", ret);
		return CMD_RET_FAILURE;
	}

	if (!cnt) {
		puts("\n\tNo Events Found!\n");
		return 0;
	}

	for (i = 0; i < cnt; i++)
		display_event(events[i]);

	return 0;
}

static int do_sec_fuse_prog(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr;
	u32 header, response;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);
	header = *(u32 *)addr;

	if ((header & 0xff0000ff) != 0x89000000) {
		printf("Wrong Signed message block format, header 0x%x\n", header);
		return CMD_RET_FAILURE;
	}

	header = (header & 0xffff00) >> 8;

	printf("Signed Message block at 0x%lx, size 0x%x\n", addr, header);
	flush_dcache_range(addr, addr + header - 1);

	if (ele_write_secure_fuse(addr, &response)) {
		printf("Program secure fuse failed, response 0x%x\n", response);
		return CMD_RET_FAILURE;
	}

	printf("Program secure fuse completed, response 0x%x\n", response);

	return CMD_RET_SUCCESS;
}

static int do_ahab_return_lifecycle(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr;
	u32 header, response;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);
	header = *(u32 *)addr;

	if ((header & 0xff0000ff) != 0x89000000) {
		printf("Wrong Signed message block format, header 0x%x\n", header);
		return CMD_RET_FAILURE;
	}

	header = (header & 0xffff00) >> 8;

	printf("Signed Message block at 0x%lx, size 0x%x\n", addr, header);
	flush_dcache_range(addr, addr + header - 1);

	if (ele_return_lifecycle_update(addr, &response)) {
		printf("Return lifecycle failed, response 0x%x\n", response);
		return CMD_RET_FAILURE;
	}

	printf("Return lifecycle completed, response 0x%x\n", response);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(auth_cntr, CONFIG_SYS_MAXARGS, 1, do_authenticate,
	   "autenticate OS container via AHAB",
	   "addr\n"
	   "addr - OS container hex address\n"
);

U_BOOT_CMD(ahab_close, CONFIG_SYS_MAXARGS, 1, do_ahab_close,
	   "Change AHAB lifecycle to OEM closed",
	   ""
);

U_BOOT_CMD(ahab_dump, CONFIG_SYS_MAXARGS, 1, do_ahab_dump,
	   "Dump AHAB log for debug",
	   ""
);

U_BOOT_CMD(ahab_status, CONFIG_SYS_MAXARGS, 1, do_ahab_status,
	   "display AHAB lifecycle only",
	   ""
);

U_BOOT_CMD(ahab_sec_fuse_prog, CONFIG_SYS_MAXARGS, 1, do_sec_fuse_prog,
	   "Program secure fuse via signed message block",
	   "addr\n"
	   "addr - Signed message block for secure fuse\n"
);

U_BOOT_CMD(ahab_return_lifecycle, CONFIG_SYS_MAXARGS, 1, do_ahab_return_lifecycle,
	   "Return lifecycle to OEM field return via signed message block",
	   "addr\n"
	   "addr - Return lifecycle message block signed by OEM SRK\n"
);
