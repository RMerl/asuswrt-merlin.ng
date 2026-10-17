// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <dm.h>
#include <hang.h>
#include <handoff.h>
#include <init.h>
#include <log.h>
#include <os.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/spl.h>
#include <asm/state.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

int sandbox_find_next_phase(char *fname, int maxlen, bool use_img)
{
	const char *cur_prefix, *next_prefix;
	int ret;

	cur_prefix = spl_phase_prefix(spl_phase());
	next_prefix = spl_phase_prefix(spl_next_phase());
	ret = os_find_u_boot(fname, maxlen, use_img, cur_prefix, next_prefix);
	if (ret)
		return log_msg_ret("find", ret);

	return 0;
}

/* SPL / TPL / VPL init function */
void board_init_f(ulong flag)
{
	struct sandbox_state *state = state_get_current();
	int ret;

	gd->arch.ram_buf = state->ram_buf;
	gd->ram_size = state->ram_size;

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}
	preloader_console_init();
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_VBE;
	spl_boot_list[1] = BOOT_DEVICE_BOARD;
}

static int spl_board_load_file(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	char fname[256];
	int ret;

	ret = sandbox_find_next_phase(fname, sizeof(fname), false);
	if (ret) {
		printf("(%s not found, error %d)\n", fname, ret);
		return ret;
	}

	/*
	 * Set up spl_image to boot from jump_to_image_no_args(). Allocate this
	 * outsdide the RAM buffer (i.e. don't use strdup()).
	 */
	spl_image->arg = os_malloc(strlen(fname) + 1);
	if (!spl_image->arg)
		return log_msg_ret("exec", -ENOMEM);
	strcpy(spl_image->arg, fname);
	spl_image->flags = SPL_SANDBOXF_ARG_IS_FNAME;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("sandbox_file", 9, BOOT_DEVICE_BOARD,
		      spl_board_load_file);

static int load_from_image(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev)
{
	struct sandbox_state *state = state_get_current();
	enum u_boot_phase next_phase;
	const char *fname;
	ulong pos, size;
	int full_size;
	void *buf;
	int ret;

	if (!IS_ENABLED(CONFIG_SANDBOX_VPL))
		return -ENOENT;

	next_phase = spl_next_phase();
	pos = spl_get_image_pos();
	size = spl_get_image_size();
	if (pos == BINMAN_SYM_MISSING || size == BINMAN_SYM_MISSING) {
		log_debug("No image found\n");
		return -ENOENT;
	}
	log_info("Reading from pos %lx size %lx\n", pos, size);

	/*
	 * Set up spl_image to boot from jump_to_image_no_args(). Allocate this
	 * outside the RAM buffer (i.e. don't use strdup()).
	 */
	fname = state->prog_fname ? state->prog_fname : state->argv[0];
	ret = os_read_file(fname, &buf, &full_size);
	if (ret)
		return log_msg_ret("rd", -ENOMEM);
	spl_image->flags = SPL_SANDBOXF_ARG_IS_BUF;
	spl_image->arg = buf;
	spl_image->offset = pos;
	spl_image->size = size;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("sandbox_image", 7, BOOT_DEVICE_BOARD, load_from_image);

void spl_board_init(void)
{
	struct sandbox_state *state = state_get_current();

	if (!CONFIG_IS_ENABLED(UNIT_TEST))
		return;

	/* These are necessary so TFTP can use LMBs to check its load address */
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	if (state->run_unittests) {
		struct unit_test *tests = UNIT_TEST_ALL_START();
		const int count = UNIT_TEST_ALL_COUNT();
		int ret;

		ret = ut_run_list("spl", NULL, tests, count,
				  state->select_unittests, 1, false, NULL);
		/* continue execution into U-Boot */
	}
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	switch (spl_image->flags) {
	case SPL_SANDBOXF_ARG_IS_FNAME: {
		const char *fname = spl_image->arg;

		if (fname) {
			os_fd_restore();
			os_spl_to_uboot(fname);
		} else {
			log_err("No filename provided for U-Boot\n");
		}
		break;
	}
	case SPL_SANDBOXF_ARG_IS_BUF: {
		int ret;

		ret = os_jump_to_image(spl_image->arg + spl_image->offset,
				       spl_image->size);
		if (ret)
			log_err("Failed to load image\n");
		break;
	}
	default:
		log_err("Invalid flags\n");
		break;
	}
	hang();
}

int handoff_arch_save(struct spl_handoff *ho)
{
	ho->arch.magic = TEST_HANDOFF_MAGIC;

	return 0;
}
