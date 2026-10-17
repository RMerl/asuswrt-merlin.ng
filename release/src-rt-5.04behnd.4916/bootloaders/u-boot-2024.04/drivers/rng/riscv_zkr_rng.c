// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * The RISC-V Zkr extension provides CSR seed which provides access to a
 * random number generator.
 */

#define LOG_CATEGORY UCLASS_RNG

#include <dm.h>
#include <interrupt.h>
#include <log.h>
#include <rng.h>

#define DRIVER_NAME "riscv_zkr"

enum opst {
	/** @BIST: built in self test running */
	BIST = 0b00,
	/** @WAIT: sufficient amount of entropy is not yet available */
	WAIT = 0b01,
	/** @ES16: 16bits of entropy available */
	ES16 = 0b10,
	/** @DEAD: unrecoverable self-test error */
	DEAD = 0b11,
};

static unsigned long read_seed(void)
{
	unsigned long ret;

	__asm__ __volatile__("csrrw %0, seed, x0" : "=r" (ret) : : "memory");

	return ret;
}

static int riscv_zkr_read(struct udevice *dev, void *data, size_t len)
{
	u8 *ptr = data;

	while (len) {
		u32 val;

		val = read_seed();

		switch (val >> 30) {
		case BIST:
			continue;
		case WAIT:
			continue;
		case ES16:
			*ptr++ = val & 0xff;
			if (--len) {
				*ptr++ = val >> 8;
				--len;
			}
			break;
		case DEAD:
			return -ENOENT;
		}
	}

	return 0;
}

/**
 * riscv_zkr_bind() - check if the seed register is available
 *
 * If the SBI software has not set mseccfg.sseed=1 or the Zkr extension is not
 * available, reading the seed register will result in an exception from which
 * this function safely resumes.
 *
 * @dev:	RNG device
 * Return:	0 if successfully probed
 */
static int riscv_zkr_bind(struct udevice *dev)
{
	struct resume_data resume;
	int ret;
	u32 val;

	/* Check if reading seed leads to interrupt */
	set_resume(&resume);
	ret = setjmp(resume.jump);
	if (ret)
		log_debug("Exception %ld reading seed CSR\n", resume.code);
	else
		val = read_seed();
	set_resume(NULL);
	if (ret)
		return -ENOENT;

	return 0;
}

/**
 * riscv_zkr_probe() - check if entropy is available
 *
 * The bind method already checked that the seed register can be read without
 * excpetiong. Here we wait for the self test to finish and entropy becoming
 * available.
 *
 * @dev:	RNG device
 * Return:	0 if successfully probed
 */
static int riscv_zkr_probe(struct udevice *dev)
{
	u32 val;

	do {
		val = read_seed();
		val >>= 30;
	} while (val == BIST || val == WAIT);

	if (val == DEAD)
		return -ENOENT;

	return 0;
}

static const struct dm_rng_ops riscv_zkr_ops = {
	.read = riscv_zkr_read,
};

U_BOOT_DRIVER(riscv_zkr) = {
	.name = DRIVER_NAME,
	.id = UCLASS_RNG,
	.ops = &riscv_zkr_ops,
	.bind = riscv_zkr_bind,
	.probe = riscv_zkr_probe,
};

U_BOOT_DRVINFO(cpu_riscv_zkr) = {
	.name = DRIVER_NAME,
};
