// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */

#include <linux/types.h>
#include <string.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include "upower_api.h"

enum upwr_api_state api_state;
enum soc_domain pwr_domain;
void *sh_buffer[UPWR_SG_COUNT];
struct upwr_code_vers fw_rom_version;
struct upwr_code_vers fw_ram_version;
u32 fw_launch_option;
u32 sg_busy;
struct mu_type *mu;
upwr_up_max_msg sg_rsp_msg[UPWR_SG_COUNT];
upwr_callb user_callback[UPWR_SG_COUNT];
UPWR_RX_CALLB_FUNC_T  sgrp_callback[UPWR_SG_COUNT];
u32 sg_rsp_siz[UPWR_SG_COUNT];

#define UPWR_MU_MSG_SIZE            (2)
#define UPWR_SG_BUSY(sg) (sg_busy & (1 << (sg)))
#define UPWR_USR_CALLB(sg, cb)		\
	do {				\
		user_callback[sg] = cb;	\
	} while (0)
#define UPWR_MSG_HDR(hdr, sg, fn)		\
	(hdr).domain   = (u32)pwr_domain;	\
	(hdr).srvgrp   = sg;			\
	(hdr).function = fn

static u32 upwr_ptr2offset(u64 ptr, enum upwr_sg sg, size_t siz, size_t offset, const void *vptr)
{
	if (ptr >= UPWR_DRAM_SHARED_BASE_ADDR &&
	    ((ptr - UPWR_DRAM_SHARED_BASE_ADDR) < UPWR_DRAM_SHARED_SIZE)) {
		return (u32)(ptr - UPWR_DRAM_SHARED_BASE_ADDR);
	}

	/* pointer is outside the shared memory, copy the struct to buffer */
	memcpy(offset + (char *)sh_buffer[sg], (void *)vptr, siz);

	return (u32)((u64)sh_buffer[sg] + offset - UPWR_DRAM_SHARED_BASE_ADDR);
}

enum upwr_req_status upwr_req_status(enum upwr_sg sg, u32 *sgfptr, enum upwr_resp *errptr,
				     int *retptr)
{
	enum upwr_req_status status;

	status = (sg_rsp_msg[sg].hdr.errcode == UPWR_RESP_OK) ? UPWR_REQ_OK : UPWR_REQ_ERR;

	return status;
}

void upwr_copy2tr(struct mu_type *mu, const u32 *msg, u32 size)
{
	int i;

	for (i = size - 1; i > -1; i--)
		writel(msg[i], &mu->tr[i]);
}

int upwr_tx(const u32 *msg, u32 size)
{
	if (size > UPWR_MU_MSG_SIZE)
		return -2;
	if (!size)
		return -2;

	if (readl(&mu->tsr) != UPWR_MU_TSR_EMPTY)
		return -1;  /* not all TE bits in 1: some data to send still */

	upwr_copy2tr(mu, msg, size);
	writel(1 << (size - 1), &mu->tcr);

	return 0;
}

void upwr_srv_req(enum upwr_sg sg, u32 *msg, u32 size)
{
	sg_busy |= 1 << sg;

	upwr_tx(msg, size);
}

int upwr_pwm_power_on(const u32 swton[], const u32 memon[], upwr_callb callb)
{
	upwr_pwm_pwron_msg txmsg;
	u64 ptrval; /* needed for X86, ARM64 */
	size_t stsize = 0;

	if (api_state != UPWR_API_READY)
		return -3;
	if (UPWR_SG_BUSY(UPWR_SG_PWRMGMT))
		return -1;

	UPWR_USR_CALLB(UPWR_SG_PWRMGMT, callb);

	UPWR_MSG_HDR(txmsg.hdr, UPWR_SG_PWRMGMT, UPWR_PWM_PWR_ON);

	if (!swton)
		txmsg.ptrs.ptr0 = 0; /* NULL pointer -> 0 offset */
	else
		txmsg.ptrs.ptr0 = upwr_ptr2offset(ptrval, UPWR_SG_PWRMGMT,
						  (stsize = UPWR_PMC_SWT_WORDS * 4), 0, swton);

	if (!memon)
		txmsg.ptrs.ptr1 = 0; /* NULL pointer -> 0 offset */
	else
		txmsg.ptrs.ptr1 = upwr_ptr2offset(ptrval, UPWR_SG_PWRMGMT, UPWR_PMC_MEM_WORDS * 4,
						  stsize, memon);

	upwr_srv_req(UPWR_SG_PWRMGMT, (u32 *)&txmsg, sizeof(txmsg) / 4);

	return 0;
}

enum upwr_req_status upwr_poll_req_status(enum upwr_sg sg, u32 *sgfptr,
					  enum upwr_resp *errptr, int *retptr,
					  u32 attempts)
{
	u32 i;
	enum upwr_req_status ret;

	if (!attempts) {
		ret = UPWR_REQ_BUSY;
		while (ret == UPWR_REQ_BUSY)
			ret = upwr_req_status(sg, sgfptr, errptr, retptr);
		return ret;
	}

	for (i = 0; i < attempts; i++) {
		ret = upwr_req_status(sg, sgfptr, errptr, retptr);
		if (ret != UPWR_REQ_BUSY)
			break;
	}

	return ret;
}

int upwr_xcp_i2c_access(u16 addr, int8_t data_size, uint8_t subaddr_size, u32 subaddr,
			u32 wdata, const upwr_callb callb)
{
	u64 ptrval = (u64)sh_buffer[UPWR_SG_EXCEPT];
	struct upwr_i2c_access *i2c_acc_ptr = (struct upwr_i2c_access *)ptrval;
	struct upwr_pointer_msg txmsg;

	if (api_state != UPWR_API_READY)
		return -3;
	if (UPWR_SG_BUSY(UPWR_SG_EXCEPT))
		return -1;

	UPWR_USR_CALLB(UPWR_SG_EXCEPT, callb);

	UPWR_MSG_HDR(txmsg.hdr, UPWR_SG_EXCEPT, UPWR_XCP_I2C);

	i2c_acc_ptr->addr = addr;
	i2c_acc_ptr->subaddr = subaddr;
	i2c_acc_ptr->subaddr_size = subaddr_size;
	i2c_acc_ptr->data = wdata;
	i2c_acc_ptr->data_size = data_size;

	txmsg.ptr = upwr_ptr2offset(ptrval,
				    UPWR_SG_EXCEPT,
				    (size_t)sizeof(struct upwr_i2c_access),
				    0,
				    i2c_acc_ptr);

	upwr_srv_req(UPWR_SG_EXCEPT, (u32 *)&txmsg, sizeof(txmsg) / 4);

	return 0;
}

int upwr_xcp_set_ddr_retention(enum soc_domain domain, u32 enable, const upwr_callb callb)
{
	union upwr_down_1w_msg txmsg;

	if (api_state != UPWR_API_READY)
		return -3;
	if (UPWR_SG_BUSY(UPWR_SG_EXCEPT))
		return -1;

	UPWR_USR_CALLB(UPWR_SG_EXCEPT, callb);

	UPWR_MSG_HDR(txmsg.hdr, UPWR_SG_EXCEPT, UPWR_XCP_SET_DDR_RETN);
	txmsg.hdr.domain = (u32)domain;
	txmsg.hdr.arg    = (u32)enable;

	upwr_srv_req(UPWR_SG_EXCEPT, (u32 *)&txmsg, sizeof(txmsg) / 4);

	return 0;
}

int upwr_rx(u32 *msg, u32 *size)
{
	u32 len = readl(&mu->rsr);

	len = (len == 0x0) ? 0 :
	      (len == 0x1) ? 1 :
	      #if UPWR_MU_MSG_SIZE > 1
	      (len == 0x3) ? 2 :
	      #if UPWR_MU_MSG_SIZE > 2
	      (len == 0x7) ? 3 :
	      #if UPWR_MU_MSG_SIZE > 3
	      (len == 0xF) ? 4 :
	      #endif
	      #endif
	      #endif
	      0xFFFFFFFF; /* something wrong */

	if (len == 0xFFFFFFFF)
		return -3;

	*size = len;
	if (!len)
		return -1;

	/* copy the received message to the rx queue, so the interrupts are cleared; */
	for (u32 i = 0; i < len; i++)
		msg[i] = readl(&mu->rr[i]);

	return 0;
}

void msg_copy(u32 *dest, u32 *src, u32 size)
{
	*dest = *src;
	if (size > 1)
		*(dest + 1) = *(src + 1);
}

void upwr_mu_int_callback(void)
{
	enum upwr_sg sg;	/* service group number */
	UPWR_RX_CALLB_FUNC_T sg_callb; /* service group callback */
	struct upwr_up_2w_msg rxmsg;
	u32 size;	/* in words */

	if (upwr_rx((u32 *)&rxmsg, &size) < 0) {
		UPWR_API_ASSERT(0);
		return;
	}

	sg = (enum upwr_sg)rxmsg.hdr.srvgrp;

	/* copy msg to the service group buffer */
	msg_copy((u32 *)&sg_rsp_msg[sg], (u32 *)&rxmsg, size);
	sg_rsp_siz[sg] = size;
	sg_busy &= ~(1 << sg);

	sg_callb = sgrp_callback[sg];
	if (!sg_callb) {
		upwr_callb user_callb = user_callback[sg];

		/* no service group callback; call the user callback if any */
		if (!user_callb)
			goto done; /* no user callback */

		/* make the user callback */
		user_callb(sg, rxmsg.hdr.function, (enum upwr_resp)rxmsg.hdr.errcode,
			   (int)(size == 2) ? rxmsg.word2 : rxmsg.hdr.ret);
		goto done;
	}

	/* finally make the group callback */
	sg_callb();
	/* don't uninstall the group callback, it's permanent */
done:
	if (rxmsg.hdr.errcode == UPWR_RESP_SHUTDOWN) /* shutdown error: */
		api_state = UPWR_API_INITLZED;
}

void upwr_txrx_isr(void)
{
	if (readl(&mu->rsr))
		upwr_mu_int_callback();
}

void upwr_start_callb(void)
{
	switch (api_state) {
	case UPWR_API_START_WAIT:
	{
		upwr_rdy_callb start_callb = (upwr_rdy_callb)user_callback[UPWR_SG_EXCEPT];

		union upwr_ready_msg *msg = (union upwr_ready_msg *)&sg_rsp_msg[UPWR_SG_EXCEPT];

		/* message sanity check */
		UPWR_API_ASSERT(msg->hdr.srvgrp   == UPWR_SG_EXCEPT);
		UPWR_API_ASSERT(msg->hdr.function == UPWR_XCP_START);
		UPWR_API_ASSERT(msg->hdr.errcode  == UPWR_RESP_OK);

		fw_ram_version.soc_id = fw_rom_version.soc_id;
		fw_ram_version.vmajor = msg->args.vmajor;
		fw_ram_version.vminor = msg->args.vminor;
		fw_ram_version.vfixes = msg->args.vfixes;

		/*
		 * vmajor == vminor == vfixes == 0 indicates start error
		 * in this case, go back to the INITLZED state
		 */

		if (fw_ram_version.vmajor || fw_ram_version.vminor || fw_ram_version.vfixes) {
			api_state = UPWR_API_READY;

			/* initialization is over: uninstall the callbacks just in case */
			UPWR_USR_CALLB(UPWR_SG_EXCEPT, NULL);
			sgrp_callback[UPWR_SG_EXCEPT] = NULL;

			if (!fw_launch_option) {
				/* launched ROM firmware: RAM fw versions must be all 0s */
				fw_ram_version.vmajor =
				fw_ram_version.vminor =
				fw_ram_version.vfixes = 0;
			}
		} else {
			api_state = UPWR_API_INITLZED;
		}

		start_callb(msg->args.vmajor, msg->args.vminor, msg->args.vfixes);
	}
	break;

	default:
		UPWR_API_ASSERT(0);
		break;
	}
}

int upwr_init(enum soc_domain domain, struct mu_type *muptr)
{
	u32 dom_buffer_base = ((UPWR_API_BUFFER_ENDPLUS + UPWR_API_BUFFER_BASE) / 2);
	union upwr_init_msg *msg = (union upwr_init_msg *)&sg_rsp_msg[UPWR_SG_EXCEPT];
	enum upwr_sg sg; /* service group number */
	u32 size; /* in words */
	int j;

	mu = muptr;
	writel(0, &mu->tcr);
	writel(0, &mu->rcr);

	api_state = UPWR_API_INIT_WAIT;
	pwr_domain = domain;
	sg_busy = 0;

	/* initialize the versions, in case they are polled */
	fw_rom_version.soc_id =
	fw_rom_version.vmajor =
	fw_rom_version.vminor =
	fw_rom_version.vfixes = 0;

	fw_ram_version.soc_id =
	fw_ram_version.vmajor =
	fw_ram_version.vminor =
	fw_ram_version.vfixes = 0;

	sh_buffer[UPWR_SG_EXCEPT] = (void *)(ulong)dom_buffer_base;
	sh_buffer[UPWR_SG_PWRMGMT] = (void *)(ulong)(dom_buffer_base +
						     sizeof(union upwr_xcp_union));
	sh_buffer[UPWR_SG_DELAYM] = NULL;
	sh_buffer[UPWR_SG_VOLTM] = NULL;
	sh_buffer[UPWR_SG_CURRM] = NULL;
	sh_buffer[UPWR_SG_TEMPM] = NULL;
	sh_buffer[UPWR_SG_DIAG] = NULL;
	/* (no buffers service groups other than xcp and pwm for now) */

	for (j = 0; j < UPWR_SG_COUNT; j++) {
		user_callback[j] = NULL;
		/* service group Exception gets the initialization callbacks */
		sgrp_callback[j] = (j == UPWR_SG_EXCEPT) ? upwr_start_callb : NULL;

		/* response messages with an initial consistent content */
		sg_rsp_msg[j].hdr.errcode = UPWR_RESP_SHUTDOWN;
	}

	if (readl(&mu->fsr) & BIT(0)) {
		/* send a ping message down to get the ROM version back */
		upwr_xcp_ping_msg ping_msg;

		ping_msg.hdr.domain = pwr_domain;
		ping_msg.hdr.srvgrp = UPWR_SG_EXCEPT;
		ping_msg.hdr.function = UPWR_XCP_PING;

		if (readl(&mu->rsr) & BIT(0)) /* first clean any Rx message left over */
			upwr_rx((u32 *)msg, &size);

		while (readl(&mu->tsr) != UPWR_MU_TSR_EMPTY)
			;

		/*
		 * now send the ping message;
		 * do not use upwr_tx, which needs API initilized;
		 * just write to the MU TR register(s)
		 */
		setbits_le32(&mu->fcr, BIT(0)); /* flag urgency status */
		upwr_copy2tr(mu, (u32 *)&ping_msg, sizeof(ping_msg) / 4);
	}

	do {
		/* poll for the MU Rx status: wait for an init message, either
		 * 1st sent from uPower after reset or as a response to a ping
		 */
		while (!readl(&mu->rsr) & BIT(0))
			;

		clrbits_le32(&mu->fcr, BIT(0));

		if (upwr_rx((u32 *)msg, &size) < 0)
			return -4;

		if (size != (sizeof(union upwr_init_msg) / 4)) {
			if (readl(&mu->fsr) & BIT(0))
				continue; /* discard left over msg */
			else
				return -4;
		}

		sg = (enum upwr_sg)msg->hdr.srvgrp;
		if (sg != UPWR_SG_EXCEPT) {
			if (readl(&mu->fsr) & BIT(0))
				continue;
			else
				return -4;
		}

		if ((enum upwr_xcp_f)msg->hdr.function   != UPWR_XCP_INIT) {
			if (readl(&mu->fsr) & BIT(0))
				continue;
			else
				return -4;
		}

		break;
	} while (true);

	fw_rom_version.soc_id = msg->args.soc;
	fw_rom_version.vmajor = msg->args.vmajor;
	fw_rom_version.vminor = msg->args.vminor;
	fw_rom_version.vfixes = msg->args.vfixes;

	api_state = UPWR_API_INITLZED;

	return 0;
} /* upwr_init */

int upwr_start(u32 launchopt, const upwr_rdy_callb rdycallb)
{
	upwr_start_msg txmsg;

	if (api_state != UPWR_API_INITLZED)
		return -3;

	UPWR_USR_CALLB(UPWR_SG_EXCEPT, (upwr_callb)rdycallb);

	UPWR_MSG_HDR(txmsg.hdr, UPWR_SG_EXCEPT, UPWR_XCP_START);

	txmsg.hdr.arg = launchopt;
	fw_launch_option = launchopt;

	if (upwr_tx((u32 *)&txmsg, sizeof(txmsg) / 4) < 0) {
		/* catastrophic error, but is it possible to happen? */
		UPWR_API_ASSERT(0);
		return -1;
	}

	api_state = UPWR_API_START_WAIT;

	return 0;
}

u32 upwr_rom_version(u32 *vmajor, u32 *vminor, u32 *vfixes)
{
	u32 soc;

	soc = fw_rom_version.soc_id;
	*vmajor = fw_rom_version.vmajor;
	*vminor = fw_rom_version.vminor;
	*vfixes = fw_rom_version.vfixes;

	return soc;
}
