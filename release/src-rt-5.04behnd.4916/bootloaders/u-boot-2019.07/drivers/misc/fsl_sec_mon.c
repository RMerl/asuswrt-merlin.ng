// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <fsl_sec_mon.h>

static u32 get_sec_mon_state(void)
{
	struct ccsr_sec_mon_regs *sec_mon_regs = (void *)
						(CONFIG_SYS_SEC_MON_ADDR);
	return sec_mon_in32(&sec_mon_regs->hp_stat) & HPSR_SSM_ST_MASK;
}

static int set_sec_mon_state_non_sec(void)
{
	u32 sts;
	int timeout = 10;
	struct ccsr_sec_mon_regs *sec_mon_regs = (void *)
						(CONFIG_SYS_SEC_MON_ADDR);

	sts = get_sec_mon_state();

	switch (sts) {
	/*
	 * If initial state is check or Non-Secure, then set the Software
	 * Security Violation Bit and transition to Non-Secure State.
	 */
	case HPSR_SSM_ST_CHECK:
		printf("SEC_MON state transitioning to Non Secure.\n");
		sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SW_SV);

		/* polling loop till SEC_MON is in Non Secure state */
		while (timeout) {
			sts = get_sec_mon_state();

			if ((sts & HPSR_SSM_ST_MASK) ==
				HPSR_SSM_ST_NON_SECURE)
				break;

			udelay(10);
			timeout--;
		}

		if (timeout == 0) {
			printf("SEC_MON state transition timeout.\n");
			return -1;
		}
		break;

	/*
	 * If initial state is Trusted, Secure or Soft-Fail, then first set
	 * the Software Security Violation Bit and transition to Soft-Fail
	 * State.
	 */
	case HPSR_SSM_ST_TRUST:
	case HPSR_SSM_ST_SECURE:
	case HPSR_SSM_ST_SOFT_FAIL:
		printf("SEC_MON state transitioning to Soft Fail.\n");
		sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SW_SV);

		/* polling loop till SEC_MON is in Soft-Fail state */
		while (timeout) {
			sts = get_sec_mon_state();

			if ((sts & HPSR_SSM_ST_MASK) ==
				HPSR_SSM_ST_SOFT_FAIL)
				break;

			udelay(10);
			timeout--;
		}

		if (timeout == 0) {
			printf("SEC_MON state transition timeout.\n");
			return -1;
		}

		timeout = 10;

		/*
		 * If SSM Soft Fail to Non-Secure State Transition
		 * disable is not set, then set SSM_ST bit and
		 * transition to Non-Secure State.
		 */
		if ((sec_mon_in32(&sec_mon_regs->hp_com) &
			HPCOMR_SSM_SFNS_DIS) == 0) {
			printf("SEC_MON state transitioning to Non Secure.\n");
			sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SSM_ST);

			/* polling loop till SEC_MON is in Non Secure*/
			while (timeout) {
				sts = get_sec_mon_state();

				if ((sts & HPSR_SSM_ST_MASK) ==
					HPSR_SSM_ST_NON_SECURE)
					break;

				udelay(10);
				timeout--;
			}

			if (timeout == 0) {
				printf("SEC_MON state transition timeout.\n");
				return -1;
			}
		}
		break;
	default:
		printf("SEC_MON already in Non Secure state.\n");
		return 0;
	}
	return 0;
}

static int set_sec_mon_state_soft_fail(void)
{
	u32 sts;
	int timeout = 10;
	struct ccsr_sec_mon_regs *sec_mon_regs = (void *)
						(CONFIG_SYS_SEC_MON_ADDR);

	printf("SEC_MON state transitioning to Soft Fail.\n");
	sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SW_FSV);

	/* polling loop till SEC_MON is in Soft-Fail state */
	while (timeout) {
		sts = get_sec_mon_state();

		if ((sts & HPSR_SSM_ST_MASK) ==
			HPSR_SSM_ST_SOFT_FAIL)
			break;

		udelay(10);
		timeout--;
	}

	if (timeout == 0) {
		printf("SEC_MON state transition timeout.\n");
		return -1;
	}
	return 0;
}

int set_sec_mon_state(u32 state)
{
	int ret = -1;

	switch (state) {
	case HPSR_SSM_ST_NON_SECURE:
		ret = set_sec_mon_state_non_sec();
		break;
	case HPSR_SSM_ST_SOFT_FAIL:
		ret = set_sec_mon_state_soft_fail();
		break;
	default:
		printf("SEC_MON state transition not supported.\n");
		return 0;
	}

	return ret;
}
