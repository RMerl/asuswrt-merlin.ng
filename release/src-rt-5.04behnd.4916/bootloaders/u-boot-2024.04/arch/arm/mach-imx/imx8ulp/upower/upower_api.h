/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020 NXP
 */

enum soc_domain {
	RTD_DOMAIN = 0,
	APD_DOMAIN = 1,
	UPWR_MAIN_DOMAINS,                           /* RTD, AVD */
	AVD_DOMAIN = UPWR_MAIN_DOMAINS,
	UPWR_DOMAIN_COUNT,                           /* RTD, APD, AVD */
	PSD_DOMAIN = UPWR_DOMAIN_COUNT,
	UPWR_ALL_DOMAINS                             /* RTD, APD, AVD, PSD */
};

enum upwr_api_state {
	UPWR_API_INIT_WAIT,        /* waiting for ROM firmware initialization */
	UPWR_API_INITLZED,         /* ROM firmware initialized */
	UPWR_API_START_WAIT,       /* waiting for start services */
	UPWR_API_READY             /* ready to receive service requests */
};

enum upwr_sg { /* Service Groups in priority order, high to low */
	UPWR_SG_EXCEPT,   /* 0 = exception           */
	UPWR_SG_PWRMGMT, /* 1 = power management    */
	UPWR_SG_DELAYM,   /* 2 = delay   measurement */
	UPWR_SG_VOLTM,   /* 3 = voltage measurement */
	UPWR_SG_CURRM,    /* 4 = current measurement */
	UPWR_SG_TEMPM,    /* 5 = temperature measurement */
	UPWR_SG_DIAG,     /* 6 = diagnostic  */
	UPWR_SG_COUNT
};

enum  upwr_xcp_f {		/* Exception Functions */
	/* 0 = init msg (not a service request itself) */
	UPWR_XCP_INIT,
	/* 0 = also ping request, since its response is an init msg */
	UPWR_XCP_PING = UPWR_XCP_INIT,
	UPWR_XCP_START,    /* 1 = service start: upwr_start (not a service request itself) */
	UPWR_XCP_SHUTDOWN, /*  2 = service shutdown: upwr_xcp_shutdown */
	UPWR_XCP_CONFIG,   /*  3 = uPower configuration: upwr_xcp_config */
	UPWR_XCP_SW_ALARM, /*  4 = uPower software alarm: upwr_xcp_sw_alarm */
	UPWR_XCP_I2C,      /*  5 = I2C access: upwr_xcp_i2c_access */
	UPWR_XCP_SPARE_6,  /*  6 = spare */
	UPWR_XCP_SET_DDR_RETN,  /*  7 = set/clear ddr retention */
	UPWR_XCP_SPARE_8,  /*  8 = spare */
	UPWR_XCP_SPARE_9,  /*  9 = spare */
	UPWR_XCP_SPARE_10, /* 10 = spare */
	UPWR_XCP_SPARE_11, /* 11 = spare */
	UPWR_XCP_SPARE_12, /* 12 = spare */
	UPWR_XCP_SPARE_13, /* 13 = spare */
	UPWR_XCP_SPARE_14, /* 14 = spare */
	UPWR_XCP_SPARE_15, /* 15 = spare */
	UPWR_XCP_F_COUNT
};

enum upwr_resp { /* response error codes */
	UPWR_RESP_OK = 0,     /* no error */
	UPWR_RESP_SG_BUSY,    /* service group is busy */
	UPWR_RESP_SHUTDOWN,   /* services not up or shutting down */
	UPWR_RESP_BAD_REQ,    /* invalid request */
	UPWR_RESP_BAD_STATE,  /* system state doesn't allow perform the request */
	UPWR_RESP_UNINSTALLD, /* service or function not installed */
	UPWR_RESP_UNINSTALLED =
	UPWR_RESP_UNINSTALLD, /* service or function not installed (alias) */
	UPWR_RESP_RESOURCE,   /* resource not available */
	UPWR_RESP_TIMEOUT,    /* service timeout */
	UPWR_RESP_COUNT
};

#define UPWR_SRVGROUP_BITS	(4)
#define UPWR_FUNCTION_BITS	(4)
#define UPWR_PWDOMAIN_BITS	(4)
#define UPWR_HEADER_BITS	(UPWR_SRVGROUP_BITS + UPWR_FUNCTION_BITS + UPWR_PWDOMAIN_BITS)
#define UPWR_ARG_BITS		(32 - UPWR_HEADER_BITS)

#define UPWR_DUAL_OFFSET_BITS    ((UPWR_ARG_BITS + 32) >> 1)

struct upwr_msg_hdr {
	u32 domain   :UPWR_PWDOMAIN_BITS;           /* power domain */
	u32 srvgrp   :UPWR_SRVGROUP_BITS;          /* service group */
	u32 function :UPWR_FUNCTION_BITS;             /* function */
	u32 arg      :UPWR_ARG_BITS;     /* function-specific argument */
};

union upwr_down_1w_msg {
	struct upwr_msg_hdr hdr;
	u32 word; /* message first word */
};

#define upwr_start_msg union upwr_down_1w_msg
#define upwr_xcp_ping_msg union upwr_down_1w_msg

#define UPWR_RESP_ERR_BITS (4)
#define UPWR_RESP_HDR_BITS (UPWR_RESP_ERR_BITS + \
			    UPWR_SRVGROUP_BITS + UPWR_FUNCTION_BITS)
#define UPWR_RESP_RET_BITS (32 - UPWR_RESP_HDR_BITS)

struct upwr_resp_hdr {
	u32 errcode :UPWR_RESP_ERR_BITS;
	u32 srvgrp  :UPWR_SRVGROUP_BITS;      /* service group */
	u32 function:UPWR_FUNCTION_BITS;
	u32 ret     :UPWR_RESP_RET_BITS;      /* return value, if any */
};

struct upwr_up_2w_msg {
	struct upwr_resp_hdr   hdr;
	u32               word2;  /* message second word */
};

#define upwr_up_max_msg struct upwr_up_2w_msg

union upwr_2pointer_msg {
	struct upwr_msg_hdr  hdr;
	struct {
		u64:UPWR_HEADER_BITS;
		u64 ptr0:UPWR_DUAL_OFFSET_BITS;
		u64 ptr1:UPWR_DUAL_OFFSET_BITS;
	} ptrs;
};

#define upwr_pwm_pwron_msg union upwr_2pointer_msg

struct upwr_pointer_msg {
	struct upwr_msg_hdr  hdr;
	u32             ptr; /* config struct offset */
};

struct upwr_i2c_access { /* structure pointed by message upwr_xcp_i2c_msg */
	u16         addr;
	s8           data_size;
	u8          subaddr_size;
	u32         subaddr;
	u32         data;
};

enum upwr_req_status {
	UPWR_REQ_OK,     /* request succeeded */
	UPWR_REQ_ERR,    /* request failed */
	UPWR_REQ_BUSY    /* request execution ongoing */
};

#define UPWR_SOC_BITS    (7)
#define UPWR_VMINOR_BITS (4)
#define UPWR_VFIXES_BITS (4)
#define UPWR_VMAJOR_BITS  \
	(32 - UPWR_HEADER_BITS - UPWR_SOC_BITS - UPWR_VMINOR_BITS - UPWR_VFIXES_BITS)
union upwr_init_msg {
	struct upwr_resp_hdr hdr;
	struct {
		u32 rsv:UPWR_RESP_HDR_BITS;
		u32 soc:UPWR_SOC_BITS;        /* SoC identification */
		u32 vmajor:UPWR_VMAJOR_BITS;  /* firmware major version */
		u32 vminor:UPWR_VMINOR_BITS;  /* firmware minor version */
		u32 vfixes:UPWR_VFIXES_BITS;  /* firmware fixes version */
	} args;
};

#define UPWR_RAM_VMINOR_BITS (7)
#define UPWR_RAM_VFIXES_BITS (6)
#define UPWR_RAM_VMAJOR_BITS (32 - UPWR_HEADER_BITS - UPWR_RAM_VFIXES_BITS - UPWR_RAM_VMINOR_BITS)

union upwr_ready_msg {
	struct upwr_resp_hdr hdr;
	struct {
		u32 rsv:UPWR_RESP_HDR_BITS;
		u32 vmajor:UPWR_RAM_VMAJOR_BITS; /* RAM fw major version */
		u32 vminor:UPWR_RAM_VMINOR_BITS; /* RAM fw minor version */
		u32 vfixes:UPWR_RAM_VFIXES_BITS; /* RAM fw fixes version */
	} args;
};

struct upwr_reg_access_t {
	u32 addr;
	u32 data;
	u32 mask; /* mask=0 commands read */
};

union upwr_xcp_union {
	struct upwr_reg_access_t reg_access;
};

enum {			/* Power Management Functions */
	UPWR_PWM_REGCFG,	/* 0 = regulator config: upwr_pwm_reg_config */
	UPWR_PWM_DEVMODE = UPWR_PWM_REGCFG, /* deprecated, for old compile */
	UPWR_PWM_VOLT,		/* 1 = voltage change: upwr_pwm_chng_reg_voltage */
	UPWR_PWM_SWITCH,	/* 2 = switch control: upwr_pwm_chng_switch_mem */
	UPWR_PWM_PWR_ON,	/* 3 = switch/RAM/ROM power on: upwr_pwm_power_on  */
	UPWR_PWM_PWR_OFF,	/* 4 = switch/RAM/ROM power off: upwr_pwm_power_off */
	UPWR_PWM_RETAIN,	/* 5 = retain memory array: upwr_pwm_mem_retain */
	UPWR_PWM_DOM_BIAS,	/* 6 = Domain bias control: upwr_pwm_chng_dom_bias */
	UPWR_PWM_MEM_BIAS,	/* 7 = Memory bias control: upwr_pwm_chng_mem_bias */
	UPWR_PWM_PMICCFG,	/* 8 = PMIC configuration:  upwr_pwm_pmic_config */
	UPWR_PWM_PMICMOD = UPWR_PWM_PMICCFG, /* deprecated, for old compile */
	UPWR_PWM_PES,		/* 9 = Power Event Sequencer */
	UPWR_PWM_CONFIG,	/* 10= apply power mode defined configuration */
	UPWR_PWM_CFGPTR,	/* 11= configuration pointer */
	UPWR_PWM_DOM_PWRON,	/* 12 = domain power on: upwr_pwm_dom_power_on */
	UPWR_PWM_BOOT,		/* 13 = boot start: upwr_pwm_boot_start */
	UPWR_PWM_FREQ,		/* 14 = domain frequency setup */
	UPWR_PWM_PARAM,		/* 15 = power management parameters */
	UPWR_PWM_F_COUNT
};

#ifndef UPWR_PMC_SWT_WORDS
#define UPWR_PMC_SWT_WORDS		(1)
#endif

#ifndef UPWR_PMC_MEM_WORDS
#define UPWR_PMC_MEM_WORDS		(2)
#endif

#define UPWR_API_ASSERT(c) do { } while (0)

struct upwr_code_vers {
	u32 soc_id;
	u32 vmajor;
	u32 vminor;
	u32 vfixes;
};

#define UPWR_MU_MSG_SIZE	(2)

#define UPWR_MU_TSR_EMPTY	((u32)((1 << UPWR_MU_MSG_SIZE) - 1))

#ifndef UPWR_DRAM_SHARED_BASE_ADDR
#define UPWR_DRAM_SHARED_BASE_ADDR	(0x28330000)
#endif

#ifndef UPWR_DRAM_SHARED_SIZE
#define UPWR_DRAM_SHARED_SIZE		(2048)
#endif

#define UPWR_DRAM_SHARED_ENDPLUS	(UPWR_DRAM_SHARED_BASE_ADDR + UPWR_DRAM_SHARED_SIZE)

#ifndef UPWR_API_BUFFER_BASE
#define UPWR_API_BUFFER_BASE		(0x28330600)
#endif

#ifndef UPWR_API_BUFFER_ENDPLUS
#define UPWR_API_BUFFER_ENDPLUS		(UPWR_DRAM_SHARED_ENDPLUS - 64)
#endif

typedef void (*upwr_rdy_callb)(u32 vmajor, u32 vminor, u32 vfixes);
typedef void (*upwr_callb)(enum upwr_sg sg, u32 func, enum upwr_resp errcode, int ret);
int upwr_init(enum soc_domain domain, struct mu_type *muptr);
int upwr_start(u32 launchopt, const upwr_rdy_callb rdycallb);
u32 upwr_rom_version(u32 *vmajor, u32 *vminor, u32 *vfixes);
typedef void (*UPWR_RX_CALLB_FUNC_T)(void);

int upwr_xcp_set_ddr_retention(enum soc_domain domain, u32 enable, const upwr_callb callb);
int upwr_pwm_power_on(const u32 swton[], const u32 memon[], upwr_callb  callb);
int upwr_xcp_i2c_access(u16 addr, s8 data_size, u8 subaddr_size, u32 subaddr,
			u32 wdata, const upwr_callb callb);
enum upwr_req_status upwr_poll_req_status(enum upwr_sg sg, u32 *sgfptr,
					  enum upwr_resp *errptr, int *retptr,
					  u32 attempts);
void upwr_txrx_isr(void);
