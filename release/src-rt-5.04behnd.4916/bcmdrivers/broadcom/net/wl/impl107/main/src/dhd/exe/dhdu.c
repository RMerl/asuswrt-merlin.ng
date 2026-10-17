/*
 * Common code for DHD command-line utility
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhdu.c 847540 2024-11-06 18:48:37Z $
 */

/* For backwards compatibility, the absence of the define 'BWL_NO_FILESYSTEM_SUPPORT'
 * implies that a filesystem is supported.
 */
#if !defined(BWL_NO_FILESYSTEM_SUPPORT)
#define BWL_FILESYSTEM_SUPPORT
#endif

#ifndef PROP_TXSTATUS
#define PROP_TXSTATUS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <typedefs.h>
#include <epivers.h>
#include <ethernet.h>
#include <dhdioctl.h>
#include <sdiovar.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include "dhdu.h"
#include "bcmstdlib_s.h"
#include "miniopt.h"
#include <bcmip.h>
#include <hnd_debug.h>
#include <hnd_armtrap.h>
#include <hnd_cons.h>
#define IPV4_ADDR_LEN 4
#ifdef WLBTAMP
#include <bt_amp_hci.h>
#endif
#ifdef BWL_FILESYSTEM_SUPPORT
#include <sys/stat.h>
#endif

#include <errno.h>

#include <trxhdr.h>
#include "ucode_download.h"
#include <d11reglist_proto.h>

#if defined(linux)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#elif	defined(BWL_STRICMP)
#define stricmp bcmstricmp
#define strnicmp bcmstrnicmp
#else
#error Needs implementation
#endif

static cmd_func_t dhd_var_void;
static cmd_func_t dhd_varint, dhd_varstr, dhd_varstr_dump;
static cmd_func_t dhd_var_getandprintstr, dhd_var_getint, dhd_var_get;
static cmd_func_t dhd_var_setint;

static cmd_func_t dhd_version, dhd_list, dhd_msglevel;

#ifdef SDTEST
static cmd_func_t dhd_pktgen;
#endif
static cmd_func_t dhd_sprom;
static cmd_func_t dhd_sdreg;
static cmd_func_t dhd_reg2args;
static cmd_func_t dhd_pcie_dmaxfer;
static cmd_func_t dhd_pcie_serdes_reg;
static cmd_func_t dhd_sd_msglevel, dhd_sd_blocksize, dhd_sd_mode, dhd_sd_reg;
static cmd_func_t dhd_dma_mode;
static cmd_func_t dhd_membytes, dhd_download, dhd_dldn,
	dhd_upload, dhd_coredump, dhd_consoledump, dhd_vars, dhd_idleclock, dhd_idletime;
static cmd_func_t dhd_logstamp, dhd_pmac, dhd_psvmp, dhd_d11dump;
#ifdef BCMSPI
static cmd_func_t dhd_spierrstats;
#endif /* BCMSPI */

static cmd_func_t dhd_hostreorder_flows;
static cmd_func_t dhd_hmembytes;

static cmd_func_t dhd_loglevel;

static char *dhd_ether_etoa(const struct ether_addr *n);

#ifdef PROP_TXSTATUS
static cmd_func_t dhd_proptxstatusenable;
static cmd_func_t dhd_proptxstatusmode;
static cmd_func_t dhd_proptxopt;
#endif

#if defined(BCM_DHD_RUNNER)
static cmd_func_t dhd_runner_varstr;
#endif /* BCM_DHD_RUNNER */

static cmd_func_t dhd_almac;
static cmd_func_t dhd_bsscfg_int;
static int dhd_bssiovar_mkbuf(const char *iovar, int bssidx, void *param,
	int paramlen, void *bufptr, int buflen, int *perr);
int dhdu_bssiovar_setbuf(void* dhd, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen);
static int dhd_bssiovar_getbuf(void* dhd, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen);
int dhdu_bssiovar_get(void *dhd, const char *iovar, int bssidx, void *outbuf, int len);
static int dhd_bssiovar_set(void *dhd, const char *iovar, int bssidx, void *param, int paramlen);
static int dhd_bssiovar_getint(void *dhd, const char *iovar, int bssidx, int *pval);
static int dhd_bssiovar_setint(void *dhd, const char *iovar, int bssidx, int val);

static int dhd_var_getbuf(void *dhd, char *iovar, void *param, int param_len, void **bufptr);
static int dhd_var_setbuf(void *dhd, char *iovar, void *param, int param_len);

static uint dhd_iovar_mkbuf(char *name, char *data, uint datalen,
                            char *buf, uint buflen, int *perr);
static int dhd_iovar_getint(void *dhd, char *name, int *var);
static int dhd_iovar_setint(void *dhd, char *name, int var);

#if defined(BWL_FILESYSTEM_SUPPORT)
static int file_size(char *fname);
static int read_vars(char *fname, char *buf, int buf_maxlen);
#endif

#ifdef WLBTAMP
static cmd_func_t wl_HCI_cmd;
static cmd_func_t wl_HCI_ACL_data;
#endif

static cmd_func_t dhd_load_asm;
static cmd_func_t dhd_fis_dump;
static cmd_func_t dhd_rngcsi_dump;

/* dword align allocation */
static union {
	char bufdata[DHD_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_dhd;
static char *buf = (char*) &bufstruct_dhd.bufdata;

/* dump dword align allocation */
static union {
	char bufdata[DHD_IOCTL_MAXLEN_HIGH];
	uint32 alignme;
} bufstruct_dhd_dump;
static char *bufdump = (char*) &bufstruct_dhd_dump.bufdata;

/* integer output format, default to signed integer */
static uint8 int_fmt;

const uint32 dump_info_ptr_ptr[] = {DUMP_INFO_PTR_PTR_LIST};

typedef struct {
	uint value;
	char *string;
} dbg_msg_t;

static int dhd_do_msglevel(void *dhd, cmd_t *cmd, char **argv, dbg_msg_t *dbg_msg);

/* Actual command table */
cmd_t dhd_cmds[] = {
	{ "cmds", dhd_list, -1, -1,
	"generate a short list of available commands"},
	{ "version", dhd_version, DHD_GET_VAR, -1,
	"get version information" },
	{ "lmtest", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set low memory test flag" },
	{ "msglevel", dhd_msglevel, DHD_GET_VAR, DHD_SET_VAR,
	"get/set message bits" },
	{ "bcmerrorstr", dhd_var_getandprintstr, DHD_GET_VAR, -1,
	"errorstring"},
	{ "wdtick", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"watchdog tick time (ms units)"},
	{ "msi_sim", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"simulate MSI interrupts for the PCIE full dongle operation"},
	{ "pcie_lpbk", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"send a loopback message on the bus"},
	{ "pcie_dmaxfer", dhd_pcie_dmaxfer, -1, DHD_SET_VAR,
	"send a dma transfer message on the bus"},
	{ "pcie_tx_lpbk", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable loopback of tx packets on the bus"},
	{ "pcie_suspend", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"suspend/resume pcie bus"},
	{ "oob_bt_reg_on", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set level of BT_REG_ON pin"},
	{ "oob_enable", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Enable OOB DEVICE_WAKE functionality"},
	{ "pciereg", dhd_reg2args, DHD_GET_VAR, DHD_SET_VAR,
	"get/set a pciereg from the device"},
	{ "bar0secwinreg", dhd_sdreg, DHD_GET_VAR, DHD_SET_VAR,
	"g/set any backplane address  across BAR0"},
	{ "pciecorereg", dhd_reg2args, DHD_GET_VAR, DHD_SET_VAR,
	"get/set a pciecorereg from the device"},
	{ "pcieserdesreg", dhd_pcie_serdes_reg, DHD_GET_VAR, DHD_SET_VAR,
	"get/set a pcieserdesreg from the device.\n"
	"first param is the block adress. second is the register offset, third is value of reg"},
	{ "pciecfgreg", dhd_reg2args, DHD_GET_VAR, DHD_SET_VAR,
	"get/set a pciecfgreg from the device"},
	{ "aspm", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set ASPM values" },
	{ "sleep_allowed", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"boolean to indicate whether the device is allowed to go to deep sleep"},
	{ "cc_nvmshadow", dhd_varstr, DHD_GET_VAR, -1,
	"dump OTP/SPROM shadow in chip common enum space"},
	{ "dump_ringupdblk", dhd_varstr, DHD_GET_VAR, -1,
	"dump ring update block in dhd memory"},
	{"dma_ring_indices", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"allow dmaing write/read indices from dongle to host memory"},
	{ "intr", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"use interrupts on the bus"},
	{ "pollrate", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"number of ticks between bus polls (0 means no polling)"},
	{ "idletime", dhd_idletime, DHD_GET_VAR, DHD_SET_VAR,
	"number of ticks for activity timeout (-1: immediate, 0: never)"},
	{ "idleclock", dhd_idleclock, DHD_GET_VAR, DHD_SET_VAR,
	"idleclock active | stopped | <N>\n"
	"\tactive (0)   - do not request any change to the SD clock\n"
	"\tstopped (-1) - request SD clock be stopped on activity timeout\n"
	"\t<N> (other)  - an sd_divisor value to request on activity timeout\n"},
	{ "sd1idle", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"change mode to SD1 when turning off clock at idle"},
	{ "forceeven", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"force SD tx/rx buffers to be even"},
	{ "readahead", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable readahead feature (look for next frame len in headers)"},
	{ "sdrxchain", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable packet chains to SDIO stack for glom receive"},
	{ "alignctl", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"align control frames"},
	{ "sdalign", dhd_varint, DHD_GET_VAR, -1,
	"display the (compiled in) alignment target for sd requests"},
	{ "txbound", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set maximum number of tx frames per scheduling"},
	{ "rxbound", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set maximum number of rx frames per scheduling"},
	{ "txminmax", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set maximum number of tx frames per scheduling while rx frames outstanding"},
	{ "dconpoll", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set dongle console polling interval (ms)"},
	{ "dump", dhd_varstr_dump, DHD_GET_VAR, -1,
	"dump information"},
	{ "cons", dhd_varstr, -1, DHD_SET_VAR,
	"send string to device console (sd only)"},
	{ "clearcounts", dhd_var_void, -1, DHD_SET_VAR,
	"reset the bus stats shown in the dhd dump"},
	{ "logdump", dhd_varstr, DHD_GET_VAR, -1,
	"dump the timestamp logging buffer"},
	{ "logcal", dhd_varint, -1, DHD_SET_VAR,
	"logcal <n>  -- log around an osl_delay of <n> usecs"},
	{ "logstamp", dhd_logstamp, -1, DHD_SET_VAR,
	"logstamp [<n1>] [<n2>]  -- add a message to the log"},
	{ "ramstart", dhd_varint, DHD_GET_VAR, -1,
	"display start address of onchip SOCRAM"},
	{ "ramsize", dhd_varint, DHD_GET_VAR, -1,
	"display size of onchip SOCRAM"},
	{ "membytes", dhd_membytes, DHD_GET_VAR, DHD_SET_VAR,
	"membytes [-h | -r | -i] <address> <length> [<data>]\n"
	"\tread or write data in the dongle ram\n"
	"\t-h   <data> is a sequence of hex digits rather than a char string\n"
	"\t-r   output binary to stdout rather hex\n"},
	{ "buzzz_dump", dhd_varint, DHD_GET_VAR, -1, "buzzz_dump\n"},
	{ "buzzz_file", dhd_varint, DHD_GET_VAR, -1, "buzzz_file\n"},
	{ "csimon", dhd_varint, DHD_GET_VAR, -1, "csimon\n"},
	{ "download", dhd_download, -1, DHD_SET_VAR,
	"download [-a <address>] [--noreset] [--norun] [--verify] <binfile> [<varsfile>]\n"
	"\tdownload file to specified dongle ram address and start CPU\n"
	"\toptional vars file will replace vars parsed from the CIS\n"
	"\t--noreset    do not reset SOCRAM core before download\n"
	"\t--norun      do not start dongle CPU after download\n"
	"\t--verify     do readback verify \n"
	"\t--backdoor   backdoor (Veloce) download \n"
	"\tdefault <address> is 0\n"},
	{ "dldn", dhd_dldn, -1, DHD_SET_VAR,
	"download <binfile>\n"
	"\tdownload file to specified dongle ram address 0\n"},
	{ "vars", dhd_vars, DHD_GET_VAR, DHD_SET_VAR,
	"vars [<file>]\n"
	"\toverride SPROM vars with <file> (before download)\n"},
	{ "coredump", dhd_coredump, -1, -1,
	"coredump <file>\n"
	"\tdump dongle RAM content into a file in dumpfile format\n"
	"\tfor use with ELF core generator"},
	{ "consoledump", dhd_consoledump, -1, -1,
	"consoledump\n"
	"\tdump dongle debug console buffer"},
	{ "upload", dhd_upload, -1, -1,
	"upload [-a <address> ] [-s <size> ] [-r <idx>] <file>\n"
	"\tupload dongle RAM and optionally register contents (-r) into file(s)\n"
	"\tdefault <address> is 0, default <size> is RAM size"},
	{ "srdump", dhd_sprom, DHD_GET_VAR, -1,
	"display SPROM content" },
	{ "srwrite", dhd_sprom, -1, DHD_SET_VAR,
	"write data or file content to SPROM\n"
	"\tsrwrite <word-offset> <word-value> ...\n"
	"\tsrwrite [-c] <srom-file-path>\n"
	"\t  -c means write regardless of crc"},
	{ "sleep", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enter/exit simulated host sleep (bus powerdown w/OOB wakeup)"},
	{ "kso", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"keep sdio on"},
	{ "devcap", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"brcm device capabilities"},
	{ "devsleep", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Sleep CMD14"},
#ifdef SDTEST
	{ "extloop", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"external loopback: convert all tx data to echo test frames"},
	{ "pktgen", dhd_pktgen, DHD_GET_VAR, DHD_SET_VAR,
	"configure/report pktgen status (SDIO)\n"
	"\t-f N     frequency: send/recv a burst every N ticks\n"
	"\t-c N     count: send/recv N packets each burst\n"
	"\t-t N     total: stop after a total of N packets\n"
	"\t-p N     print: display counts on console every N bursts\n"
	"\t-m N     min: set minimum length of packet data\n"
	"\t-M N     Max: set maximum length of packet data\n"
	"\t-l N     len: set fixed length of packet data\n"
	"\t-s N     stop after N tx failures\n"
	"\t-d dir   test direction/type:\n"
	"\t            send -- send packets discarded by dongle\n"
	"\t            echo -- send packets to be echoed by dongle\n"
	"\t            burst -- request bursts (of size <-c>) from dongle\n"
	"\t              one every <-f> ticks, until <-t> total requests\n"
	"\t            recv -- request dongle enter continuous send mode,\n"
	"\t              read up to <-c> pkts every <-f> ticks until <-t>\n"
	"\t              total reads\n"},
#endif /* SDTEST */
	{ "dngl_isolation", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set dongle isolation, so the dev could be disabled with out effecting the dongle state"},
	{ "ltrsleep_on_unload", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set sending ltr sleep message on unload config"},
	{ "sdreg", dhd_sdreg, DHD_GET_VAR, DHD_SET_VAR,
	"g/set sdpcmdev core register (f1) across SDIO (CMD53)"},
	{ "sbreg", dhd_sdreg, DHD_GET_VAR, DHD_SET_VAR,
	"g/set any backplane core register (f1) across SDIO (CMD53)"},
	{ "sd_cis", dhd_var_getandprintstr, DHD_GET_VAR, -1,
	"dump sdio CIS"},
	{ "sd_devreg", dhd_sd_reg, DHD_GET_VAR, DHD_SET_VAR,
	"g/set device register across SDIO bus (CMD52)"},
	{ "sd_hostreg", dhd_sd_reg, DHD_GET_VAR, DHD_SET_VAR,
	"g/set local controller register"},
	{ "sd_blocksize", dhd_sd_blocksize, DHD_GET_VAR, DHD_SET_VAR,
	"g/set block size for a function"},
	{ "sd_blockmode", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set blockmode"},
	{ "sd_ints", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set client ints"},
	{ "sd_dma", dhd_dma_mode, DHD_GET_VAR, DHD_SET_VAR,
	"g/set dma usage: [PIO | SDMA | ADMA1 | ADMA2]"},
	{ "sd_yieldcpu", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"allow blocking (yield of CPU) on data xfer"},
	{ "sd_minyield", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"minimum xfer size to allow CPU yield"},
	{ "sd_forcerb", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"force readback when changing local interrupt settings"},
	{ "sd_numints", dhd_varint, DHD_GET_VAR, -1,
	"number of device interrupts"},
	{ "sd_numlocalints", dhd_varint, DHD_GET_VAR, -1,
	"number of non-device interrupts"},
	{ "sd_divisor", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set the divisor for SDIO clock generation"},
	{ "sd_power", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set the SD Card slot power"},
	{ "sd_power_save", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set the SDIO3.0 power save value"},
	{ "sd_clock", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"turn on/off the SD Clock"},
	{ "sd_crc", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"turn on/off CRC checking in SPI mode"},
	{ "sd_mode", dhd_sd_mode, DHD_GET_VAR, DHD_SET_VAR,
	"g/set SDIO bus mode (spi, sd1, sd4)"},
	{ "sd_highspeed", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set the high-speed clocking mode"},
	{ "sd_msglevel", dhd_sd_msglevel, DHD_GET_VAR, DHD_SET_VAR,
	"g/set debug message level"},
	{ "sd_hciregs", dhd_varstr, DHD_GET_VAR, -1,
	"display host-controller interrupt registers"},
	{ "sdiod_drive", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"SDIO Device drive strength in milliamps. (0=tri-state, 1-12mA)"},
#ifdef BCMSPI
	{ "spi_errstats", dhd_spierrstats, DHD_GET_VAR, DHD_SET_VAR,
	"SPI device status error statistics."},
	{ "spi_respdelay", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set response delay flag."},
#endif /* BCMSPI */
	{ "devreset", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Move device into or out of reset state (1/reset, or 0/operational)"},
	{ "pcie_device_trap", dhd_var_void, -1, DHD_SET_VAR,
	"trap the pcie device via mailbox mechanism"},
	{ "ioctl_timeout", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"IOCTL response timeout (milliseconds)."},
#ifdef WLBTAMP
	{ "HCI_cmd", wl_HCI_cmd, -1, DHD_SET_VAR,
	"carries HCI commands to the driver\n"
	"\tusage: dhd HCI_cmd <command> <args>\n" },
	{ "HCI_ACL_data", wl_HCI_ACL_data, -1, DHD_SET_VAR,
	"carries HCI ACL data packet to the driver\n"
	"\tusage: dhd HCI_ACL_data <logical link handle> <data>\n" },
#endif
#ifdef PROP_TXSTATUS
	{ "proptx", dhd_proptxstatusenable, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable the proptxtstatus feature\n"
	"0 - disabled\n"
	"1 - enabled\n"},
	{ "ptxmode", dhd_proptxstatusmode, DHD_GET_VAR, DHD_SET_VAR,
	"set the proptxtstatus operation mode:\n"
	"0 - Unsupported\n"
	"1 - Use implied credit from a packet status\n"
	"2 - Use explicit credit\n" },
	{ "proptx_opt", dhd_proptxopt, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable proptxtstatus optimizations to increase throughput:\n"
	"0 - Unsupported\n"
	"1 - Enable proptxstatus optimizations to increase throughput\n" },
	{ "pmodule_ignore", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"use/bypass the proptxtstatus module\n"
	"0 - apply\n"
	"1 - bypass\n"},
	{ "pcredit_ignore", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"apply/ignore the proptxtstatus credit control\n"
	"0 - apply\n"
	"1 - ignore\n"},
	{ "ptxstatus_ignore", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"process/ignore the proptxtstatus txstaus\n"
	"0 - process\n"
	"1 - ignore\n"},
	{ "rxpkt_chk", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"check/ignore rx packet for multiple AC detection\n"
	"0 - ignore\n"
	"1 - check\n"},
#endif /* PROP_TXSTATUS */
	{ "sd_uhsimode", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"g/set UHSI Mode"},
	{ "host_reorder_flows", dhd_hostreorder_flows, DHD_GET_VAR, -1,
	"get host reorder flows "},
	{ "txglomsize", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"max glom size for sdio tx\n"},
	{ "txp_thresh", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"max txp glom to flush to device on PCIE Tx\n"},
	{ "txglommode", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"glom mode for sdio tx 0- copy, 1- multidescriptor\n"},
	{ "fw_hang_report", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable report firmware hangs for firmware reload\n"
	"0 - disabled (for testing)\n"
	"1 - enabled (default)\n"},
	{ "tuning_mode", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set tuning mode \n"
	"1 - Disabled periodic tuning \n"
	"0 - Enable periodic tuning \n"},
	{ "tcpack_suppress", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Turn tcpack_suppress on/off\n"
	"0 - off\n"
	"1 - on\n"},
	{ "txinrx_thres", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set threshold for num of in-queued pkts to tx in rx process\n"
	"0 - always send out in rx process\n"
	"num - amount of in-queued pkts\n"},
	{"db1_for_mb", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set doorbell1 option for Mail box interrupt, for PCIE FD"
	"0 - use default config write for mb interrupt\n"
	"1 - use enumeration doorbell1 write for mb interrupt\n"
	},
	{"flow_prio_map", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"0 - Default AC priority map\n"
	"1 - TID based priority map\n"
	},
	{ "op_mode", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"\nget/set operation mode when cfg80211 is used\n"
	"1- STA Only\n"
	"2- SOFT AP Only\n"
	"3- P2P Only\n"
	"4- STA + P2P\n"
	},
	{"assert_type", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set/get the asset_bypass flag\n"
	"assert_type <1/0> (On/Off)\n"
	},
	{"wowl_wakeind", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get wowl wake reason\n"
	"WAKE_EVENT_NLO_DISCOVERY_BIT\t\t\t0x0001\n"
	"WAKE_EVENT_AP_ASSOCIATION_LOST_BIT\t\t0x0002\n"
	"WAKE_EVENT_GTK_HANDSHAKE_ERROR_BIT\t\t0x0004\n"
	"WAKE_EVENT_4WAY_HANDSHAKE_REQUEST_BIT\t\t0x0008\n"
	"WAKE_EVENT_NET_PAKCET_BIT\t\t\t0x0010\n"
	"UNSPECIFIED (dhd not the wake reason)\t\t0x0000\n"
	},
	{ "wmf_bss_enable", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable wmf features" },
	{ "wmf_psta_disable", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"disable/enable wmf for proxy sta" },
	{ "wmf_ucast_igmp", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable wmf unicast igmp features" },
	{ "wmf_mcast_data_sendup", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable wmf multicast data sendup" },
	{ "wmf_ucast_igmp_query", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable wmf unicast igmp query" },
	{ "wmf_ucast_upnp", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable wmf unicast upnp features" },
	{ "ap_isolate", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"set/get AP isolation" },
	{ "proxy_arp", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable proxy arp features" },
	{ "dhcp_unicast", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable dhcp unicast features" },
	{ "block_ping", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable block ping features" },
	{ "grat_arp", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable grat arp features" },
	{ "block_tdls", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable block_tdls feature" },
	{ "dev_def", dhd_bsscfg_int, -1, DHD_SET_VAR,
	"set/get dev_def" },
	{ "pmac", dhd_pmac, -1, -1,
	"Get mac obj values such as of SHM and IHR\n"
	"\tusage: dhd -i IF pmac <type> <address> [-s step_size] [-n num] [-b bitmap] [-v]\n"
	"type: register type"
	"address: register address"
	"step_size: 0 or 2 or 4\n"
	"num : how many to print\n"
	"bitmap : 32-bit value\n"
	"-v: verbose mode"},
	{ "psvmp", dhd_psvmp, -1, -1,
	"Get svmp mem values\n"
	"\tusage: dhd -i IF psvmp [-n num]\n"
	"num : how many to print\n"
	"-v: verbose mode"},
	{ "mcast_regen_bss_enable", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable per bss unicast to multicast pkt conversion feature" },
	{"lbr_aggr_en_mask", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set tid bit mask for low bit rate aggregation b'0: disable b'1: enable " },
	{"lbr_aggr_len", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set number of packets to aggregate" },
	{"lbr_aggr_release_timeout", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set low bit rate aggregation time window to release aggregates" },
#if defined(BCM_DHD_RUNNER)
	{ "rnr_flowring_profile", dhd_runner_varstr, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set DHD Runner Flowring profile\n"
	"\tUsage: dhd -i IF rnr_flowring_profile -id <id> -ac_bk <weight:size>"
	" -ac_be <weight:size> -ac_vi <weight:size> -ac_vo <weight:size> -bc_mc <weight:size>\n"
	"\t<id>            0-2 user profile per radio\n"
	"\t                3-7 Built-in profiles\n"
	"\t<weight>        priority to allocate reserved memory\n"
	"\t                -1: Allocate all ring items\n"
	"\t                0: do not allocate ring items\n"
	"\t                1-n: allocate n items (<size) per round robin allocation\n"
	"\t<size>          numer of items in the flow ring\n"
	"\n"
	"NOTE: Set needs DHD driver unload or system reboot\n"},
	{ "rnr_flowring_policy", dhd_runner_varstr, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set DHD Runner Flowring selection policy for HW or SW\n"
	"\tUsage: dhd -i IF rnr_flowring_policy -<policy> <setting>\n"
	"\t<policy>        policy name (global, intfidx, clients, aclist, maclist, dot11ac)\n"
	"\t<setting>       setting based on the policy\n"
	"\t                <policy>    <setting>\n"
	"\t                 global     <0/1>\n"
	"\t                 intfidx    <interface index>\n"
	"\t                 clients    <number of clients>\n"
	"\t                 aclist     ac_bk:<0/1> ac_be:<0/1>"
	" ac_vi:<0/1> ac_vo:<0/1> bc_mc:<0/1>\n"
	"\t                 maclist    <xx-xx-xx-xx-xx-xx>\n"
	"\t                 dot11ac    <0/1>\n"
	"\n"
	"NOTE: Set needs DHD driver unload or system reboot\n"},
	{ "rnr_rxoffload", dhd_runner_varstr, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set DHD Runner Rx Offload\n"
	"\tUsage: dhd -i IF rnr_rxoffload <1/0>\n"
	"\t  0    disable Rx Offload acceleration\n"
	"\t  1    Enable Rx Offload acceleration\n"
	"\n"
	"NOTE: Set needs DHD driver unload or system reboot\n"},
#endif /* BCM_DHD_RUNNER */
	{ "halt_dongle", dhd_varint, 0, DHD_SET_VAR,
	"halts ARM and PSMs." },
	{ "cap", dhd_varstr, DHD_GET_VAR, -1, "Gets dongle capabilities" },
	{ "idma_enable", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Enable/Disable Implicit DMA functionality"},
	{ "d11dump", dhd_d11dump, DHD_GET_VAR, -1,
	"dump d11 registers in the list\n"
	"\tusage: dhd -i IF d11dump [-k] dumpname\n"
	"-k: Kill ARM processor before start dumping\n"
	"dumpname: e.g) mac, mac1, macx, sctpl, scpsm, svmp"},
	{ "wet", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set DHD WET mode"},
	{ "1905_al_ucast", dhd_almac, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set 1905 AL unicast mac address\n"
	"\tUsage: dhd -i IF 1905_al_ucast <clear/MAC address>\n"},
	{ "1905_al_mcast", dhd_almac, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set 1905 AL multicast mac address\n"
	"\tUsage: dhd -i IF 1905_al_mcast <clear/MAC address>\n"},
	{ "macdbg_dump_level", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Set macdbg dump level for crash dumps"},
	{ "hmembytes", dhd_hmembytes, DHD_GET_VAR, -1,
	"hmembytes <addr> <length>\n"
	"\tread data in the host memory\n"},
	{ "extended_ops_support", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	" 0 - disabled to restrict some ops, 1-support all the ops \n"},
#if defined(DHD_IFE)
	{ "ife_timeout", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set ife timeout in ms. Default is 5s\n"},
	{ "ife_budget", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set ife Budget."
	"Budget is the no of flowrings to process in each timer invocation\n"},
#endif /* DHD_IFE */
#if defined(DHD_IFE_RECLAIM)
	{ "ife_reclaim_hi_thr", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set percentage of consumed flowrings to start reclaiming flowrings. Default 75%\n"},
	{ "ife_reclaim_low_thr", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set percentage of consumed flowrings to stop reclaiming flowrings. Default 25%\n"},
#endif /* DHD_IFE_RECLAIM */
	{ "sbf_interval", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set WLAN Client Buffer Fairness polling interval\n"},
	{ "sbf_graceth", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set WLAN Client Buffer Fairness grace threshold for favored pkts\n"},
	{ "sbf_msglevel", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Get/Set WLAN Client Buffer Fairness message level\n"},
	{ "sbf_dump", dhd_varstr_dump, DHD_GET_VAR, -1,
	"dump sbf information"},
	{ "mlo_event", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"set/get mlo events" },
	{ "mlo_msg_test", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"test mlo ipc messages exchange between peers" },
	{ "loglevel", dhd_loglevel, DHD_GET_VAR, DHD_SET_VAR,
	"Bitvector for logging to memory. When enabled, logs are stored in memory" },
	{ "flowring_ids_dump", dhd_varstr_dump, DHD_GET_VAR, -1,
	"dump flowring ids"},
	{ "bss_fix_ac", dhd_bsscfg_int, DHD_GET_VAR, DHD_SET_VAR,
	"\t Set/Get BSS forced priority \n"
	"\t Usage: dhd -i IF bss_fix_ac <forced priority: 0-7>"
	"\t VI:4,5; VO:6,7; BE:0,3; BK:1,2;"
	"\n"},
	{ "idma_test_next_group", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	" -1 disable idma_test next group, 0-62 - set idma map group \n"},
	{ "cpudbg_kconsole", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"1 - enable MAC CPU DBG prints to kernel console\n"
	"\t0 - disable MAC CPU DBG prints to kernel console\n"},
	{ "cpudbg_filelines", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"<n> = nonzero - <n> = count of 1k lines to log to each cpudbg log ping-pong file\n"
	"\t<n> = 0 - disable cpudbg file logging\n"},
	{ "tmp_data_filter", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	" 0 - disable temp data filter, 1- enable temp data filter \n"},
	{"btrace_enable", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"enable/disable btrace feature" },
	{"btrace_interval", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set btrace polling interval in milliseconds" },
	{ "flring_delete", dhd_varint, -1, DHD_SET_VAR,
	"delete flowrings for a given flow ringid\n"},
	{ "pcap_dump", dhd_varstr_dump, DHD_GET_VAR, -1, "dump pcap dhd counters\n"},
	{ "pcap_drop_thresh", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"Set/Get pcap_tap drop threshold\n"},
	{ "fis_asm_load", dhd_load_asm, DHD_GET_VAR, DHD_SET_VAR,
	"\t Usage : dhd -i wlX fis_asm_load <core>\n"
	"\t\t ex) For rx4 core, dhd -i wl0 fis_asm_lod rx4\n\n"
	"\t cpusub : download wl_chip_pcie_82770_cpusub_inst.h \n"
	"\t dig : download wl_chip_pcie_82771_dig_inst.h \n"
	"\t phy12 : download wl_chip_pcie_82773_phy1phy2_inst.h \n"
	"\t rx1 : download wl_chip_pcie_82774_phyrx1_inst.h \n"
	"\t rx2 : download wl_chip_pcie_82775_phyrx2_inst.h \n"
	"\t rx4 : download wl_chip_pcie_82776_phyrx4_inst.h \n"
	"\t d11mac1 : download wl_chip_pcie_82777_d11mac1_inst.h \n"
	"\t d11mac2 : download wl_chip_pcie_82778_d11mac2_inst.h \n"
	"\t chip_all : download wl_chip_pcie_82779_chip_with_dig_inst.h \n"
	"\t\t - It has all ASMs for all cores\n"
	"\t chip_wo_dig : download wl_chip_pcie_82779_chip_wo_dig_inst.h \n"
	"\t\t - It doesn't include ASM for DIG core\n"
	"\t chip_mac_phy : download wl_chip_pcie_82780_chip_only_mac_phy_inst.h \n"
	"\t\t - Only for MAC and PHY\n" },
	{ "fis_sr_enab", dhd_varint, DHD_GET_VAR, -1,
	"\t Enable SR engine, ASM load is required before SR is enabled\n"
	"\t Usage : dhd -i wlX fis_sr_enab\n" },
	{ "fis_trigger", dhd_varint, -1, DHD_SET_VAR,
	"\t Only FIS trigger, no dump. Before FIS is triggered, "
	"ASM load & SR enable are required\n"
	"\t Usage : dhd -i wlX fis_trigger <type>\n"
	"\t\t 0 : PMU trigger\n"
	"\t\t 1 : PCIE trigger\n"
	"\t\t 2 : MAC trigger\n"
	"\t\t 3 : PHY trigger\n" },
	{ "fis_reset", dhd_varint, DHD_GET_VAR, -1,
	"\t Clear FIS state. After clearing FIS state, "
	"reboot is required to ensure normal operation\n"
	"\t Usage : dhd -i wlX fis_reset\n" },
	{ "fis_dump", dhd_fis_dump, -1, DHD_SET_VAR,
	"\t Create a FIS dump.\n"
	"\t Usage : dhd -i wlX fis_dump <0 or 1>\n"
	"\t\t 0 : ASM isn't included\n"
	"\t\t 1 : ASM is included\n" },
	{"fis_delay", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"delay to save FIS DATA\n"},
	{"fis_reset_status", dhd_varint, DHD_GET_VAR, -1,
	"reset current fis status\n"},
	{"control_logtrace", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"set/get conftrol_logtrace value for dhd\n"
	"\t control_logtrace bit control\n"
	"\t\t bit 0: string log on/off : 0 - disable, 1 - enable\n"
	"\t\t bit 1: string log type: 0 - raw encoded, 1 - parsed\n"
	"\t\t (NIC doesn't refer bit 1 and type is string itself)\n"
	"\t\t bit 2: chatty string log on/off : 0 - disable, 1 - enable\n"
	"\t\t bit 3: binary data on/off: 0 - disable, 1 - enable\n"},
	{ "rngcsi_dump", dhd_rngcsi_dump, -1, DHD_SET_VAR,
	"\t Dump Ranging Channel State Information (CSI) to /var files\n"
	"\t Usage: dhd -i wlX rngcsi_dump <r or i>\n"
	"\t\t r : RSTA\n"
	"\t\t i : ISTA\n"},
	{"ieee1905_pkt_prio_enab", dhd_varint, DHD_GET_VAR, DHD_SET_VAR,
	"get/set ieee1905 pkt prio enabled"},
	{ NULL, NULL, 0, 0, NULL }
};

cmd_t dhd_varcmd = {"var", dhd_varint, -1, -1, "unrecognized name, type -h for help"};
char *dhdu_av0;

#if defined(BWL_FILESYSTEM_SUPPORT)
static int
file_size(char *fname)
{
	FILE *fp;
	long size = -1;

	/* Can't use stat() because of Win CE */

	if ((fp = fopen(fname, "rb")) == NULL ||
	    fseek(fp, 0, SEEK_END) < 0 ||
	    (size = ftell(fp)) < 0)
		fprintf(stderr, "Could not determine size of %s: %s\n",
		        fname, strerror(errno));

	if (fp != NULL)
		fclose(fp);

	return (int)size;
}
#endif   /* BWL_FILESYSTEM_SUPPORT */

/* parse/validate the command line arguments */
/*
* pargv is updated upon return if the first argument is an option.
 * It remains intact otherwise.
 */
int
dhd_option(char ***pargv, char **pifname, int *phelp)
{
	char *ifname = NULL;
	int help = FALSE;
	int status = CMD_OPT;
	char **argv = *pargv;

	int_fmt = INT_FMT_DEC;

	while (*argv) {
		/* select different adapter */
		if (!strncmp(*argv, "-a", 3) || !strncmp(*argv, "-i", 3)) {
			char *opt = *argv++;
			ifname = *argv;
			if (!ifname) {
				fprintf(stderr,
					"error: expected interface name after option %s\n", opt);
				status = CMD_ERR;
				break;
			}
		}

		/* integer output format */
		else if (!strncmp(*argv, "-d", 3))
			int_fmt = INT_FMT_DEC;
		else if (!strncmp(*argv, "-u", 3))
			int_fmt = INT_FMT_UINT;
		else if (!strncmp(*argv, "-x", 3))
			int_fmt = INT_FMT_HEX;
		/* coverity[dont_call] */
		else if (!strncmp(*argv, "-p", 3) || !strncmp(*argv, "--all", sizeof("--all")) ||
			!strncmp(*argv, "-e", 3) || !strncmp(*argv, "--each", sizeof("--each")) ||
			!strncmp(*argv, "--primary", sizeof("--primary"))) {
			/* nothing to do */
		}

		/* command usage */
		else if (!strncmp(*argv, "-h", 3) || !strncmp(*argv, "--help", sizeof("--help")))
			help = TRUE;

		/* done with generic options */
		else {
			status = CMD_DHD;
			break;
		}

		/* consume the argument */
		argv ++;
		break;
	}

	*phelp = help;
	if (pifname) {
		*pifname = ifname;
	}
	*pargv = argv;

	return status;
}

void
dhd_cmd_usage(cmd_t *cmd)
{
	if (strlen(cmd->name) >= 8)
		fprintf(stderr, "%s\n\t%s\n\n", cmd->name, cmd->help);
	else
		fprintf(stderr, "%s\t%s\n\n", cmd->name, cmd->help);
}

/* Dump out short list of commands */
static int
dhd_list(void *dhd, cmd_t *garb, char **argv)
{
	cmd_t *cmd;
	int nrows, i, len;
	char *buf;
	int letter, col, row, pad;

	UNUSED_PARAMETER(dhd);
	UNUSED_PARAMETER(garb);
	UNUSED_PARAMETER(argv);

	for (cmd = dhd_cmds, nrows = 0; cmd->name; cmd++)
		    nrows++;

	nrows /= 4;
	nrows++;

	len = nrows * 80 + 2;
	buf = malloc(len);
	if (buf == NULL) {
		fprintf(stderr, "Failed to allocate buffer of %d bytes\n", len);
		return BCME_NOMEM;
	}
	for (i = 0; i < len; i++)
		*(buf+i) = 0;

	row = col = 0;
	for (letter = 'a'; letter < 'z'; letter++) {
		for (cmd = dhd_cmds; cmd->name; cmd++) {
			if (cmd->name[0] == letter || cmd->name[0] == letter - 0x20) {
				strcat(buf+row*80, cmd->name);
				pad = 18 * (col + 1) - (int)strlen(buf+row*80);
				if (pad < 1)
					pad = 1;
				for (; pad; pad--)
					strcat(buf+row*80, " ");
				row++;
				if (row == nrows) {
					col++; row = 0;
				}
			}
		}
	}
	for (row = 0; row < nrows; row++)
		printf("%s\n", buf+row*80);

	printf("\n");
	free(buf);
	return (0);
}

void
dhd_cmds_usage(cmd_t *port_cmds)
{
	cmd_t *port_cmd;
	cmd_t *cmd;

	/* print usage of port commands */
	for (port_cmd = port_cmds; port_cmd && port_cmd->name; port_cmd++)
		/* Check for wc_cmd */
		dhd_cmd_usage(port_cmd);

	/* print usage of common commands without port counterparts */
	for (cmd = dhd_cmds; cmd->name; cmd++) {
		/* search if port counterpart exists */
		for (port_cmd = port_cmds; port_cmd && port_cmd->name; port_cmd++)
			if (!strcmp(port_cmd->name, cmd->name))
				break;
		if (!port_cmd || !port_cmd->name)
			dhd_cmd_usage(cmd);
	}
}

void
dhd_usage(cmd_t *port_cmds)
{
	fprintf(stderr,
	        "Usage: %s [-a|i <adapter>] [-h] [-d|u|x] <command> [arguments]\n",
		dhdu_av0);

	fprintf(stderr, "\n");
	fprintf(stderr, "  -h		this message\n");
	fprintf(stderr, "  -a, -i	adapter name or number\n");
	fprintf(stderr, "  -d		display values as signed integer\n");
	fprintf(stderr, "  -u		display values as unsigned integer\n");
	fprintf(stderr, "  -x		display values as hexdecimal\n");
	fprintf(stderr, "\n");

	dhd_cmds_usage(port_cmds);
}

void
dhd_printint(int val)
{
	switch (int_fmt) {
	case INT_FMT_UINT:
		printf("%u\n", val);
		break;
	case INT_FMT_HEX:
		printf("0x%x\n", val);
		break;
	case INT_FMT_DEC:
	default:
		printf("%d\n", val);
		break;
	}
}

/* pretty hex print a contiguous buffer (tweaked from wlu) */
void
dhd_hexdump(uchar *buf, uint nbytes, uint saddr)
{
	char line[256];
	char* p;
	uint i;

	if (nbytes == 0) {
		printf("\n");
		return;
	}

	p = line;
	for (i = 0; i < nbytes; i++) {
		if (i % 16 == 0) {
			p += sprintf(p, "%08x: ", saddr + i);	/* line prefix */
		}
		p += sprintf(p, "%02x ", buf[i]);
		if (i % 16 == 15) {
			uint j;
			p += sprintf(p, "  ");
			for (j = i-15; j <= i; j++)
				p += sprintf(p, "%c",
				             ((buf[j] >= 0x20 && buf[j] <= 0x7f) ? buf[j] : '.'));
			printf("%s\n", line);		/* flush line */
			p = line;
		}
	}

	/* flush last partial line */
	if (p != line)
		printf("%s\n", line);
}

/*
*
* Set or Get a BSS Configuration idexed integer
*/
static int
dhd_bsscfg_int(void *dhd, cmd_t *cmd, char **argv)
{
	char *endptr = NULL;
	char *val_name;
	int bsscfg_idx = 0;
	int val = 0;
	int ret;

	val_name = *argv++;

	if (GET_BSSCFG_IDX(dhd, &bsscfg_idx) != BCME_OK) {
		bsscfg_idx = 0;
	}

	if (!*argv) {
		/* This is a GET */
		if (cmd->get == -1) {
			return -1;
		}

		ret = dhd_bssiovar_getint(dhd, val_name, bsscfg_idx, &val);

		if (ret < 0)
			return ret;

		dhd_printint(val);
	} else {
		/* This is a SET */
		if (cmd->set == -1) {
			return -1;
		}

		if (!stricmp(*argv, "on")) {
			val = 1;
		} else if (!stricmp(*argv, "off")) {
			val = 0;
		} else {
			val = strtol(*argv, &endptr, 0);
			if (*endptr != '\0') {
				/* not all the value string was parsed by strtol */
				return BCME_USAGE_ERROR;
			}
		}

		ret = dhd_bssiovar_setint(dhd, val_name, bsscfg_idx, val);
	}

	return ret;
}

#ifdef SDTEST
static int
dhd_pktgen(void *dhd, cmd_t *cmd, char **argv)
{
	int ret = 0;
	void *ptr = NULL;
	dhd_pktgen_t pktgen;
	char *str;

	UNUSED_PARAMETER(dhd);
	UNUSED_PARAMETER(cmd);

	/* Get current settings */
	if ((ret = dhd_var_getbuf(dhd, "pktgen", NULL, 0, &ptr)) != 0)
		return ret;
	memcpy(&pktgen, ptr, sizeof(pktgen));

	if (pktgen.version != DHD_PKTGEN_VERSION) {
		fprintf(stderr, "pktgen version mismatch (module %d app %d)\n",
		        pktgen.version, DHD_PKTGEN_VERSION);
		return BCME_ERROR;
	}

	/* Presence of args implies a set, else a get */
	if (*++argv) {
		miniopt_t opts;
		int opt_err;

		/* Initialize option parser */
		miniopt_init(&opts, "pktgen", "", FALSE);

		while ((opt_err = miniopt(&opts, argv)) != -1) {
			if (opt_err == 1) {
				fprintf(stderr, "pktgen options error\n");
				ret = -1;
				goto exit;
			}
			argv += opts.consumed;

			if (!opts.good_int && opts.opt != 'd') {
				fprintf(stderr, "invalid integer %s\n", opts.valstr);
				ret = -1;
				goto exit;
			}

			switch (opts.opt) {
			case 'f':
				pktgen.freq = opts.uval;
				break;
			case 'c':
				pktgen.count = opts.uval;
				break;
			case 'p':
				pktgen.print = opts.uval;
				break;
			case 't':
				pktgen.total = opts.uval;
				break;
			case 's':
				pktgen.stop = opts.uval;
				break;
			case 'm':
				pktgen.minlen = opts.uval;
				break;
			case 'M':
				pktgen.maxlen = opts.uval;
				break;
			case 'l': case 'L':
				pktgen.minlen = pktgen.maxlen = opts.uval;
				break;
			case 'd':
				if (!strcmp(opts.valstr, "send"))
					pktgen.mode = DHD_PKTGEN_SEND;
				else if (!strcmp(opts.valstr, "echo"))
					pktgen.mode = DHD_PKTGEN_ECHO;
				else if (!strcmp(opts.valstr, "burst"))
					pktgen.mode = DHD_PKTGEN_RXBURST;
				else if (!strcmp(opts.valstr, "recv"))
					pktgen.mode = DHD_PKTGEN_RECV;
				else {
					fprintf(stderr, "unrecognized dir mode %s\n",
					        opts.valstr);
					return BCME_USAGE_ERROR;
				}
				break;

			default:
				fprintf(stderr, "option parsing error (key %s valstr %s)\n",
				        opts.key, opts.valstr);
				ret = BCME_USAGE_ERROR;
				goto exit;
			}
		}

		if (pktgen.maxlen < pktgen.minlen) {
			fprintf(stderr, "min/max error (%d/%d)\n", pktgen.minlen, pktgen.maxlen);
			ret = -1;
			goto exit;
		}

		/* Set the new values */
		ret = dhd_var_setbuf(dhd, "pktgen", &pktgen, sizeof(pktgen));
	} else {
		printf("Counts: %d send attempts, %d received, %d tx failures\n",
		       pktgen.numsent, pktgen.numrcvd, pktgen.numfail);
	}

	/* Show configuration in either case */
	switch (pktgen.mode) {
	case DHD_PKTGEN_ECHO: str = "echo"; break;
	case DHD_PKTGEN_SEND: str = "send"; break;
	case DHD_PKTGEN_RECV: str = "recv"; break;
	case DHD_PKTGEN_RXBURST: str = "burst"; break;
	default: str = "UNKNOWN"; break;
	}

	printf("Config: mode %s %d pkts (len %d-%d) each %d ticks\n",
	       str, pktgen.count, pktgen.minlen, pktgen.maxlen, pktgen.freq);

	/* Second config line for optional items */
	str = "        ";
	if (pktgen.total) {
		printf("%slimit %d", str, pktgen.total);
		str = ", ";
	}
	if (pktgen.print) {
		printf("%sprint every %d ticks", str, (pktgen.freq * pktgen.print));
		str = ", ";
	}
	if (pktgen.stop) {
		printf("%sstop after %d tx failures", str, pktgen.stop);
		str = ", ";
	}
	if (str[0] == ',')
		printf("\n");

exit:
	return ret;
}
#endif /* SDTEST */

static dbg_msg_t dhd_sd_msgs[] = {
	{SDH_ERROR_VAL,	"error"},
	{SDH_TRACE_VAL,	"trace"},
	{SDH_INFO_VAL,	"info"},
	{SDH_DATA_VAL,	"data"},
	{SDH_CTRL_VAL,	"control"},
	{SDH_LOG_VAL,	"log"},
	{SDH_DMA_VAL,	"dma"},
	{0,		NULL}
};

static int
dhd_sd_msglevel(void *dhd, cmd_t *cmd, char **argv)
{
	return dhd_do_msglevel(dhd, cmd, argv, dhd_sd_msgs);
}

static int
dhd_sd_blocksize(void *dhd, cmd_t *cmd, char **argv)
{
	int ret;
	int argc;
	char *endptr = NULL;
	void *ptr = NULL;
	int func, size;

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	if (argc < 1 || argc > 2) {
		printf("required args: function [size] (size 0 means max)\n");
		return BCME_USAGE_ERROR;
	}

	func = strtol(argv[1], &endptr, 0);
	if (*endptr != '\0') {
		printf("Invalid function: %s\n", argv[1]);
		return BCME_USAGE_ERROR;
	}

	if (argc > 1) {
		size = strtol(argv[2], &endptr, 0);
		if (*endptr != '\0') {
			printf("Invalid size: %s\n", argv[1]);
			return BCME_USAGE_ERROR;
		}
	}

	if (argc == 1) {
		if ((ret = dhd_var_getbuf(dhd, cmd->name, &func, sizeof(func), &ptr)) >= 0)
			printf("Function %d block size: %d\n", func, *(int*)ptr);
	} else {
		printf("Setting function %d block size to %d\n", func, size);
		size &= 0x0000ffff; size |= (func << 16);
		ret = dhd_var_setbuf(dhd, cmd->name, &size, sizeof(size));
	}

	return (ret);
}

static int
dhd_sd_mode(void *wl, cmd_t *cmd, char **argv)
{
	int ret;
	int argc;
	int sdmode;

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	if (argv[1]) {
		if (!strcmp(argv[1], "spi")) {
			strcpy(argv[1], "0");
		} else if (!strcmp(argv[1], "sd1")) {
			strcpy(argv[1], "1");
		} else if (!strcmp(argv[1], "sd4")) {
			strcpy(argv[1], "2");
		} else {
			return BCME_USAGE_ERROR;
		}

		ret = dhd_var_setint(wl, cmd, argv);

	} else {
		if ((ret = dhd_var_get(wl, cmd, argv))) {
			return (ret);
		} else {
			sdmode = *(int32*)buf;

			printf("SD Mode is: %s\n",
			       sdmode == 0 ? "SPI"
			       : sdmode == 1 ? "SD1"
				   : sdmode == 2 ? "SD4" : "Unknown");
		}
	}

	return (ret);
}

static int
dhd_dma_mode(void *wl, cmd_t *cmd, char **argv)
{
	int ret;
	int argc;
	int dmamode;

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	if (argv[1]) {
		if (!stricmp(argv[1], "pio")) {
			strcpy(argv[1], "0");
		} else if (!strcmp(argv[1], "0")) {
		} else if (!stricmp(argv[1], "dma")) {
			strcpy(argv[1], "1");
		} else if (!stricmp(argv[1], "sdma")) {
			strcpy(argv[1], "1");
		} else if (!strcmp(argv[1], "1")) {
		} else if (!stricmp(argv[1], "adma1")) {
			strcpy(argv[1], "2");
		} else if (!stricmp(argv[1], "adma")) {
			strcpy(argv[1], "3");
		} else if (!stricmp(argv[1], "adma2")) {
			strcpy(argv[1], "3");
		} else {
			return BCME_USAGE_ERROR;
		}

		ret = dhd_var_setint(wl, cmd, argv);

	} else {
		if ((ret = dhd_var_get(wl, cmd, argv))) {
			return (ret);
		} else {
			dmamode = *(int32*)buf;

			printf("DMA Mode is: %s\n",
			       dmamode == 0 ? "PIO"
			       : dmamode == 1 ? "SDMA"
			       : dmamode == 2 ? "ADMA1"
			       : dmamode == 3 ? "ADMA2"
			       : "Unknown");
		}
	}

	return (ret);
}

/*
 * dhd_regs2args is a generic function that is used for setting/getting
 * DHDIOVAR variables that require address for read, and
 * address + data for write.
 */
static int
dhd_reg2args(void *dhd, cmd_t *cmd, char **argv)
{
	char var[256];
	uint32 int_val;
	bool get = TRUE;
	uint32 len;
	void *ptr = NULL;
	char *endptr;
	int ret = 0;

	if (argv[1]) {
		len = sizeof(int_val);
		int_val = strtoul(argv[1], &endptr, 0);
		memcpy(var, (char *)&int_val, sizeof(int_val));
	}
	else
		return BCME_USAGE_ERROR;

	if (argv[2]) {
		get = FALSE;
		int_val = strtoul(argv[2], &endptr, 0);
		memcpy(&var[len], (char *)&int_val, sizeof(int_val));
		len += sizeof(int_val);
	}
	if (get) {
		if ((ret = dhd_var_getbuf(dhd, cmd->name, var, sizeof(var), &ptr)) < 0)
			return ret;

		printf("0x%x\n", (*(int *)ptr));
	}
	else
		ret = dhd_var_setbuf(dhd, cmd->name, &var, sizeof(var));
	return ret;
}

static int
dhd_pcie_serdes_reg(void *dhd, cmd_t *cmd, char **argv)
{
	char var[256];
	uint32 int_val;
	uint32 len;
	char *endptr;
	int ret = 0;
	void *ptr = NULL;

	bzero(var, sizeof(var));
	if (argv[1]) {
		len = sizeof(int_val);
		int_val = strtoul(argv[1], &endptr, 0);
		memcpy(var, (char *)&int_val, sizeof(int_val));
	}
	else
		return BCME_USAGE_ERROR;

	if (argv[2]) {
		int_val = strtoul(argv[2], &endptr, 0);
		memcpy(&var[len], (char *)&int_val, sizeof(int_val));
		len += sizeof(int_val);
	}
	else
		return BCME_USAGE_ERROR;

	if (argv[3]) {
		int_val = strtoul(argv[3], &endptr, 0);
		memcpy(&var[len], (char *)&int_val, sizeof(int_val));
		len += sizeof(int_val);
		ret = dhd_var_setbuf(dhd, cmd->name, &var, sizeof(var));
	} else {
		ret = dhd_var_getbuf(dhd, cmd->name, &var, sizeof(var), &ptr);
		if (ret < 0)
			return ret;
		printf("0x%x\n", (*(int *)ptr));
	}
	return ret;
}

static int
dhd_pcie_dmaxfer(void *dhd, cmd_t *cmd, char **argv)
{
	char var[256];
	uint32 int_val;
	uint32 len;
	char *endptr;
	int ret = 0;

	bzero(var, sizeof(var));
	if (argv[1]) {
		len = sizeof(int_val);
		int_val = strtoul(argv[1], &endptr, 0);
		memcpy(var, (char *)&int_val, sizeof(int_val));
	}
	else
		return BCME_USAGE_ERROR;

	if (argv[2]) {
		int_val = strtoul(argv[2], &endptr, 0);
		memcpy(&var[len], (char *)&int_val, sizeof(int_val));
		len += sizeof(int_val);
	}

	if (argv[3]) {
		int_val = strtoul(argv[3], &endptr, 0);
		memcpy(&var[len], (char *)&int_val, sizeof(int_val));
		len += sizeof(int_val);
	}
	ret = dhd_var_setbuf(dhd, cmd->name, &var, sizeof(var));
	return ret;
}

static int
dhd_sdreg(void *dhd, cmd_t *cmd, char **argv)
{
	int ret;
	sdreg_t sdreg;
	uint argc;
	char *ptr = NULL;

	UNUSED_PARAMETER(cmd);

	bzero(&sdreg, sizeof(sdreg));

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	/* required args: offset (will default size) */
	if (argc < 1) {
		printf("required args: offset[/size] [value]\n");
		return BCME_USAGE_ERROR;
	}

	sdreg.offset = strtoul(argv[1], &ptr, 0);
	if (*ptr && *ptr != '/') {
		printf("Bad arg: %s\n", argv[1]);
		return BCME_USAGE_ERROR;
	}

	/* read optional /size */
	if (*ptr == '/') {
		sdreg.func = strtol((ptr+1), &ptr, 0);
		if (*ptr || ((sdreg.func != 1) && (sdreg.func != 2) &&
			(sdreg.func != 4) && (sdreg.func != 8))) {
			printf("Bad size option?\n");
			return BCME_USAGE_ERROR;
		}
	}
	else {
		sdreg.func = 4;
		printf("Defaulting to register size 4\n");
	}

	if (argc > 1) {
		sdreg.value = strtoul(argv[2], &ptr, 0);
		if (*ptr) {
			printf("Bad value: %s\n", argv[2]);
			return BCME_USAGE_ERROR;
		}
	}

	if (argc <= 1) {
		ret = dhd_var_getbuf(dhd, argv[0], &sdreg, sizeof(sdreg), (void**)&ptr);
		if (ret >= 0)
			printf("0x%0*x\n", (2 * sdreg.func), *(int *)ptr);
	} else {
		ret = dhd_var_setbuf(dhd, argv[0], &sdreg, sizeof(sdreg));
	}

	return (ret);
}

static int
dhd_membytes(void *dhd, cmd_t *cmd, char **argv)
{
	int ret = -1;
	uint argc;
	char *ptr;
	int params[2];
	uint addr;
	uint len;
	int align;

	int rawout, hexin;

	miniopt_t opts;
	int opt_err;

	/* Parse command-line options */
	miniopt_init(&opts, "membytes", "rh", FALSE);

	rawout = hexin = 0;

	argv++;
	while ((opt_err = miniopt(&opts, argv)) != -1) {
		if (opt_err == 1) {
			fprintf(stderr, "membytes options error\n");
			ret = -1;
			goto exit;
		}

		if (opts.positional)
			break;

		argv += opts.consumed;

		if (opts.opt == 'h') {
			hexin = 1;
		} else if (opts.opt == 'r') {
			rawout = 1;
		} else {
			fprintf(stderr, "membytes command error\n");
			ret = -1;
			goto exit;
		}
	}

	/* arg count */
	for (argc = 0; argv[argc]; argc++);

	/* required args: address size [<data>]] */
	if (argc < 2) {
		fprintf(stderr, "required args: address size [<data>]\n");
		return BCME_USAGE_ERROR;
	}

	if (argc < 3 && hexin) {
		fprintf(stderr, "missing <data> required by -h\n");
		return BCME_USAGE_ERROR;
	}
	if ((argc > 2) && (rawout)) {
		fprintf(stderr, "can't have <data> arg with -r\n");
		return BCME_USAGE_ERROR;
	}

	/* read address */
	addr = strtoul(argv[0], &ptr, 0);
	if (*ptr) {
		fprintf(stderr, "Bad arg: %s\n", argv[0]);
		return BCME_USAGE_ERROR;
	}

	/* read size */
	len = strtoul(argv[1], &ptr, 0);
	if (*ptr) {
		fprintf(stderr, "Bad value: %s\n", argv[1]);
		return BCME_USAGE_ERROR;
	}

	align = addr & 0x03;
	if (align && argc > 2) {
		fprintf(stderr, "Can only write starting at long-aligned addresses.\n");
		return BCME_USAGE_ERROR;
	}

	/* get can just use utility function, set must copy custom buffer */
	if (argc == 2) {
		/* Read */
		uint chunk = DHD_IOCTL_MAXLEN;
		for (addr -= align, len += align; len; addr += chunk, len -= chunk, align = 0) {
			chunk = MIN(chunk, len);
			params[0] = addr;
			params[1] = ROUNDUP(chunk, 4);
			ret = dhd_var_getbuf(dhd, "membytes",
			                     params, (2 * sizeof(int)), (void**)&ptr);
			if (ret < 0)
				goto exit;

			if (rawout) {
				fwrite(ptr + align, sizeof(char), chunk - align, stdout);
			} else {
				dhd_hexdump((uchar*)ptr + align, chunk - align, addr + align);
			}
		}
	} else {
		/* Write */
		uint patlen = (uint)strlen(argv[2]);
		uint chunk, maxchunk;
		char *sptr;

		if (hexin) {
			char *inptr, *outptr;
			if (patlen & 1) {
				fprintf(stderr, "Hex (-h) must consist of whole bytes\n");
				ret = BCME_USAGE_ERROR;
				goto exit;
			}

			for (inptr = outptr = argv[2]; patlen; patlen -= 2) {
				int n1, n2;

				n1 = (int)((unsigned char)*inptr++);
				n2 = (int)((unsigned char)*inptr++);
				if (!isxdigit(n1) || !isxdigit(n2)) {
					fprintf(stderr, "invalid hex digit %c\n",
					        (isxdigit(n1) ? n2 : n1));
					ret = BCME_USAGE_ERROR;
					goto exit;
				}
				n1 = isdigit(n1) ? (n1 - '0')
				        : ((islower(n1) ? (toupper(n1)) : n1) - 'A' + 10);
				n2 = isdigit(n2) ? (n2 - '0')
				        : ((islower(n2) ? (toupper(n2)) : n2) - 'A' + 10);
				*outptr++ = (n1 * 16) + n2;
			}

			patlen = (uint)(outptr - argv[2]);
		}

		sptr = argv[2];
		maxchunk = DHD_IOCTL_MAXLEN - ((uint)strlen(cmd->name) + 1 + (2 * sizeof(int)));

		while (len) {
			chunk = (len > maxchunk) ? (maxchunk & ~0x3) : len;

			/* build the iovar command */
			memset(buf, 0, DHD_IOCTL_MAXLEN);
			strcpy(buf, cmd->name);
			ptr = buf + strlen(buf) + 1;
			params[0] = addr; params[1] = chunk;
			memcpy(ptr, params, (2 * sizeof(int)));
			ptr += (2 * sizeof(int));
			addr += chunk; len -= chunk;

			while (chunk--) {
				*ptr++ = *sptr++;
				if (sptr >= (argv[2] + patlen))
					sptr = argv[2];
			}

			ret = dhd_set(dhd, DHD_SET_VAR, &buf[0], (int)(ptr - buf));
			if (ret < 0)
				goto exit;
		}
	}

exit:
	return ret;
}

static int
dhd_idletime(void *dhd, cmd_t *cmd, char **argv)
{
	int32 idletime;
	char *endptr = NULL;
	int err = 0;

	if (argv[1]) {
		if (!strcmp(argv[1], "never")) {
			idletime = 0;
		} else if (!strcmp(argv[1], "immediate") || !strcmp(argv[1], "immed")) {
			idletime = DHD_IDLE_IMMEDIATE;
		} else {
			idletime = strtol(argv[1], &endptr, 0);
			if (*endptr != '\0') {
				fprintf(stderr, "invalid number %s\n", argv[1]);
				err = BCME_USAGE_ERROR;
			}
		}
		if ((idletime < 0) && (idletime != DHD_IDLE_IMMEDIATE)) {
			fprintf(stderr, "invalid value %s\n", argv[1]);
			err = -1;
		}

		if (!err) {
			strcpy(buf, "idletime");
			endptr = buf + strlen(buf) + 1;
			memcpy(endptr, &idletime, sizeof(uint32));
			endptr += sizeof(uint32);
			err = dhd_set(dhd, DHD_SET_VAR, &buf[0], (int)(endptr - buf));
		}
	} else {
		if ((err = dhd_var_get(dhd, cmd, argv))) {
			return err;
		} else {
			idletime = *(int32*)buf;

			if (idletime == 0) {
				printf("0 (never)\n");
			} else if (idletime == DHD_IDLE_IMMEDIATE) {
				printf("-1 (immediate)\n");
			} else if (idletime > 0) {
				printf("%d\n", idletime);
			} else printf("%d (invalid)\n", idletime);
		}
	}
	return err;
}

static int
dhd_idleclock(void *dhd, cmd_t *cmd, char **argv)
{
	int32 idleclock;
	char *endptr = NULL;
	int err = 0;

	if (argv[1]) {
		if (!strcmp(argv[1], "active")) {
			idleclock = DHD_IDLE_ACTIVE;
		} else if (!strcmp(argv[1], "stopped")) {
			idleclock = DHD_IDLE_STOP;
		} else {
			idleclock = strtol(argv[1], &endptr, 0);
			if (*endptr != '\0') {
				fprintf(stderr, "invalid number %s\n", argv[1]);
				err = BCME_USAGE_ERROR;
			}
		}

		if (!err) {
			strcpy(buf, "idleclock");
			endptr = buf + strlen(buf) + 1;
			memcpy(endptr, &idleclock, sizeof(int32));
			endptr += sizeof(int32);
			err = dhd_set(dhd, DHD_SET_VAR, &buf[0], (int)(endptr - buf));
		}
	} else {
		if ((err = dhd_var_get(dhd, cmd, argv))) {
			return err;
		} else {
			idleclock = *(int32*)buf;

			if (idleclock == DHD_IDLE_ACTIVE)
				printf("Idleclock %d (active)\n", idleclock);
			else if (idleclock == DHD_IDLE_STOP)
				printf("Idleclock %d (stopped)\n", idleclock);
			else
				printf("Idleclock divisor %d\n", idleclock);
		}
	}
	return err;
}

/* Word count for a 4kb SPROM */
#define SPROM_WORDS 256

static int
dhd_sprom(void *dhd, cmd_t *cmd, char **argv)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	int ret, i;
	uint argc;
	char *endptr;
	char *bufp, *countptr;
	uint16 *wordptr;
	uint offset, words, bytes;
	bool nocrc = FALSE;

	char *fname;
	FILE *fp;

	UNUSED_PARAMETER(cmd);

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	/* init buffer */
	bufp = buf;
	memset(bufp, 0, DHD_IOCTL_MAXLEN);
	strcpy(bufp, "sprom");
	bufp += strlen("sprom") + 1;

	if (strcmp(argv[0], "srdump") == 0) {
		if (argc) {
			fprintf(stderr, "Command srdump doesn't take args\n");
			return BCME_USAGE_ERROR;
		}
		offset = 0;
		words = SPROM_WORDS;
		bytes = 2 * words;

		memcpy(bufp, &offset, sizeof(int));
		bufp += sizeof(int);
		memcpy(bufp, &bytes, sizeof(int));
		bufp += sizeof(int);

		if (!ISALIGNED((uintptr)bufp, sizeof(uint16))) {
			fprintf(stderr, "Internal error: unaligned word buffer\n");
			return BCME_ERROR;
		}
	} else {
		if (strcmp(argv[0], "srwrite") != 0) {
			fprintf(stderr, "Unimplemented sprom command: %s\n", argv[0]);
			return BCME_USAGE_ERROR;
		}

		if (argc == 0) {
			return BCME_USAGE_ERROR;
		} else if ((argc == 1) ||
		           ((argc == 2) && ((nocrc = !strcmp(argv[1], "-c"))))) {

			fname = nocrc ? argv[2] : argv[1];

			/* determine and validate file size */
			if ((ret = file_size(fname)) < 0)
				return BCME_ERROR;

			bytes = ret;
			offset = 0;
			words = bytes / 2;

			if (bytes != 2 * SPROM_WORDS) {
				fprintf(stderr, "Bad file size\n");
				return BCME_ERROR;
			}

			memcpy(bufp, &offset, sizeof(int));
			bufp += sizeof(int);
			memcpy(bufp, &bytes, sizeof(int));
			bufp += sizeof(int);

			if (!ISALIGNED((uintptr)bufp, sizeof(uint16))) {
				fprintf(stderr, "Internal error: unaligned word buffer\n");
				return BCME_ERROR;
			}

			if ((fp = fopen(fname, "rb")) == NULL) {
				fprintf(stderr, "Could not open %s: %s\n",
				        fname, strerror(errno));
				return BCME_ERROR;
			}

			if (fread((uint16*)bufp, sizeof(uint16), words, fp) != words) {
				fprintf(stderr, "Could not read %d bytes from %s\n",
				        words * 2, fname);
				fclose(fp);
				return BCME_ERROR;
			}

			fclose(fp);

			if (!nocrc &&
			    hndcrc8((uint8*)bufp, bytes, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE) {
				fprintf(stderr, "CRC check failed: 0x%02x, should be 0x%02x.\n",
				        ((uint8*)bufp)[bytes-1],
				        ~hndcrc8((uint8*)bufp, bytes - 1, CRC8_INIT_VALUE) & 0xff);
				return BCME_ERROR;
			}

			ltoh16_buf(bufp, bytes);
		} else {
			offset = strtoul(*++argv, &endptr, 0) * 2;
			if (*endptr != '\0') {
				fprintf(stderr, "offset %s is not an integer\n", *argv);
				return BCME_USAGE_ERROR;
			}

			memcpy(bufp, &offset, sizeof(int));
			bufp += sizeof(int);
			countptr = bufp;
			bufp += sizeof(int);

			if (!ISALIGNED((uintptr)bufp, sizeof(uint16))) {
				fprintf(stderr, "Internal error: unaligned word buffer\n");
				return BCME_ERROR;
			}

			for (words = 0, wordptr = (uint16*)bufp; *++argv; words++) {
				*wordptr++ = (uint16)strtoul(*argv, &endptr, 0);
				if (*endptr != '\0') {
					fprintf(stderr, "value %s is not an integer\n", *argv);
					return BCME_USAGE_ERROR;
				}
				if (words > SPROM_WORDS) {
					fprintf(stderr, "max of %d words\n", SPROM_WORDS);
					return BCME_USAGE_ERROR;
				}
			}

			bytes = 2 * words;
			memcpy(countptr, &bytes, sizeof(int));
		}
	}

	if (argc) {
		ret = dhd_set(dhd, DHD_SET_VAR, buf,
		              ((int)strlen("sprom") + 1) + (2 * sizeof(int)) + bytes);
		return (ret);
	} else {
		ret = dhd_get(dhd, DHD_GET_VAR, buf,
		              ((int)strlen("sprom") + 1) + (2 * sizeof(int)) + bytes);
		if (ret < 0) {
			return ret;
		}

		for (i = 0; i < (int)words; i++) {
			if ((i % 8) == 0)
				printf("\n  srom[%03d]:  ", i);
			printf("0x%04x  ", ((uint16*)buf)[i]);
		}
		printf("\n");
	}

	return 0;
#endif /* BWL_FILESYSTEM_SUPPORT */
}

/*
 * read_vars: reads an environment variables file into a buffer,
 * reformatting them and returning the length (-1 on error).
 *
 * The input text file consists of lines of the form "<var>=<value>\n".
 * CRs are ignored, as are blank lines and comments beginning with '#'.
 *
 * The output buffer consists of blocks of the form "<var>=<value>\0"
 * (the newlines have been replaced by NULs)
 *
 * Todo: allow quoted variable names and quoted values.
*/

#if defined(BWL_FILESYSTEM_SUPPORT)
static int
read_vars(char *fname, char *buf, int buf_maxlen)
{
	FILE *fp;
	int buf_len;
	struct stat finfo;
	uint file_size;

	if ((fp = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "Cannot open NVRAM file %s: %s\n",
		        fname, strerror(errno));
		exit(1);
	}
	if (fstat(fileno(fp), &finfo)) {
		perror(fname);
		fclose(fp);
		exit(1);
	}
	file_size = finfo.st_size;
	if (file_size > buf_maxlen) {
		fprintf(stderr, "NVRAM file %s (%d bytes) too long for %d bytes buffer\n",
			fname, file_size, buf_maxlen);
		fclose(fp);
		return -1;
	}

	/* read the file and push blocks down to memory */
	if (fread(buf, 1, file_size, fp) != file_size) {
		perror(fname);
		fclose(fp);
		exit(1);
	}

	fclose(fp);
	fp = NULL;

	/* convert linefeeds to nuls */
	buf_len = process_nvram_vars(buf, file_size);

	return buf_len;
}
#endif   /* BWL_FILESYSTEM_SUPPORT */

static int
dhd_vars(void *dhd, cmd_t *cmd, char **argv)
{
	int ret;
	uint argc;
	char *bufp;

	UNUSED_PARAMETER(cmd);

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	switch (argc) {
	case 0: /* get */
	{
		if ((ret = dhd_var_getbuf(dhd, "vars", NULL, 0, (void**)&bufp)))
			break;
		while (*bufp) {
			printf("%s\n", bufp);
			bufp += strlen(bufp) + 1;
		}
	}
	break;

#if defined(BWL_FILESYSTEM_SUPPORT)
	case 1: /* set */
	{
		char *vname;
		uint nvram_len;

		vname = argv[1];

		bufp = buf;
		strcpy(bufp, "vars");
		bufp += strlen("vars") + 1;

		if ((ret = read_vars(vname, bufp,
		                           DHD_IOCTL_MAXLEN - ((int)strlen("vars") + 3))) < 0) {
			ret = -1;
			break;
		}

		nvram_len = ret;
		bufp += nvram_len;
		*bufp++ = 0;

		ret = dhd_set(dhd, DHD_SET_VAR, buf, (int)(bufp - buf));
	}
	break;
#endif   /* BWL_FILESYSTEM_SUPPORT */

	default:
		ret = -1;
		break;
	}

	return ret;
}

#define MEMBLOCK 2048

/* Check that strlen("membytes")+1 + 2*sizeof(int32) + MEMBLOCK <= DHD_IOCTL_MAXLEN */
#if (MEMBLOCK + 17 > DHD_IOCTL_MAXLEN)
#error MEMBLOCK/DHD_IOCTL_MAXLEN sizing
#endif

#if defined(BWL_FILESYSTEM_SUPPORT)
static int
dhd_verify_file_bytes(void *dhd, uint8 *memblock, int start, uint len)
{
	int ret = 0;
	uint i = 0;
	char *ptr;
	int params[2];
	uint8 *src, *dst;

	params[0] = start;
	params[1] = len;
	ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&ptr);
	if (ret) {
		fprintf(stderr, "%s: failed reading %d membytes from 0x%08x\n",
		__FUNCTION__, len, start);
		return ret;
	}

	src = (uint8 *)memblock;
	dst = (uint8 *)ptr;
	while (i < len) {
		if (src[i] != dst[i]) {
			fprintf(stderr, "   0x%x: exp[0x%02X] != got[0x%02X]\n",
				start+i, src[i], dst[i]);
			ret = -1;
		}
		i++;
	}

	return ret;
}

/* Support for hybrid firmware download */
#define PCIE_HYBRIDFW_MAGICNUM		0x434F464Cu
#define PCIE_HYBRIDFW_TYPE_DNGL		0u

static int
dhd_load_hybridfw(void *dhd, char *fname, FILE *fp, int fsize)
{
	uint magic_num;
	int ret, len, buflen;
	char *bufp;

	/* Look for magic number in first 4 bytes and return BCME_UNSUPPORTED
	 * if not found
	 */
	len = (uint)fread(&magic_num, sizeof(uint8), sizeof(magic_num), fp);
	if (len < sizeof(magic_num)) {
		fprintf(stderr, "%s: error reading file %s:%s\n", __FUNCTION__,
			fname, strerror(errno));
		ret = BCME_ERROR;
		goto exit;
	}
	if (magic_num != PCIE_HYBRIDFW_MAGICNUM) {
		ret = BCME_UNSUPPORTED;
		goto exit;
	}

	bufp = buf;
	buflen = DHD_IOCTL_MAXLEN;
	if (strlen(fname) + strlen("hybridfw") + 2 > buflen) {
		ret = BCME_BUFTOOLONG;
		goto exit;
	}
	memset(bufp, 0, buflen);

	strcpy(bufp, "hybridfw");
	bufp += strlen("hybridfw") + 1;
	strcpy(bufp, fname);
	bufp += strlen(fname) + 1;

	ret = dhd_set(dhd, DHD_SET_VAR, &buf[0], (int)(bufp - buf));
	if (ret) {
		fprintf(stderr, "%s: error %d when setting firmware file path\n",
		        __FUNCTION__, ret);
	}

exit:
	/* reset file pointer */
	rewind(fp);
	return ret;
}

static int
dhd_load_file_bytes(void *dhd, cmd_t *cmd, FILE *fp, int fsize, int start, uint blk_sz, bool verify)
{
	int tot_len = 0;
	uint read_len;
	char *bufp;
	uint len;
	uint8 memblock[MEMBLOCK];
	int ret;
	int retry;

	UNUSED_PARAMETER(cmd);

	if (!fsize || !fp)
		return -1;

	assert(blk_sz <= MEMBLOCK);

	while (tot_len < fsize) {
		read_len = fsize - tot_len;
		if (read_len >= blk_sz) {
			read_len = blk_sz;

			if (!ISALIGNED(start, MEMBLOCK))
				read_len = ROUNDUP(start, MEMBLOCK) - start;
		}

		len = (uint)fread(memblock, sizeof(uint8), read_len, fp);
		if ((len < read_len) && !feof(fp)) {
			fprintf(stderr, "%s: error reading file\n", __FUNCTION__);
			return -1;

		}
		retry = 0;
failed_retry:

		bufp = buf;
		memset(bufp, 0, DHD_IOCTL_MAXLEN);
		strcpy(bufp, "membytes");
		bufp += strlen("membytes") + 1;
		memcpy(bufp, &start, sizeof(int));
		bufp += sizeof(int);
		memcpy(bufp, &len, sizeof(int));
		bufp += sizeof(int);
		memcpy(bufp, memblock, len);

		if ((ret = dhd_set(dhd, DHD_SET_VAR, &buf[0], (int)(bufp - buf + len))) < 0) {
			fprintf(stderr, "%s: error %d on writing %d membytes at 0x%08x\n",
			        __FUNCTION__, ret, len, start);
			return ret;
		}

		if (verify == TRUE) {
			if (dhd_verify_file_bytes(dhd, memblock, start, len) != 0) {
				if (retry++ < 5000)
				{
					fprintf(stderr, "%s: verify failed %d membytes "
						"from 0x%08x\n", __FUNCTION__, len, start);
					goto failed_retry;
				}
			}
		}

		start += len;
		tot_len += len;
	}
	return 0;
}
#endif   /* BWL_FILESYSTEM_SUPPORT */

#ifdef PROP_TXSTATUS
static int
dhd_proptxstatusenable(void *dhd, cmd_t *cmd, char **argv)
{
	int flag = 0xdead;
	int ret;

	if (argv[1]) {
		flag = atoi(argv[1]);
		ret = dhd_iovar_setint(dhd, cmd->name, flag);
	}
	else {
		ret = dhd_iovar_getint(dhd, cmd->name, &flag);
		if (ret >= 0)
			printf("proptxstatus: %d\n", flag);
	}
	return ret;
}

static int
dhd_proptxstatusmode(void *dhd, cmd_t *cmd, char **argv)
{
	int mode = 0xdead;
	int ret;

	if (argv[1]) {
		mode = atoi(argv[1]);
		ret = dhd_iovar_setint(dhd, cmd->name, mode);
	}
	else {
		ret = dhd_iovar_getint(dhd, cmd->name, &mode);
		if (ret >= 0)
			printf("proptxstatusmode: %d\n", mode);
	}
	return ret;
}

static int
dhd_proptxopt(void *dhd, cmd_t *cmd, char **argv)
{
	int flag = 0xdead;
	int ret;

	if (argv[1]) {
		flag = atoi(argv[1]);
		ret = dhd_iovar_setint(dhd, cmd->name, flag);
	}
	else {
		ret = dhd_iovar_getint(dhd, cmd->name, &flag);
		if (ret >= 0)
			printf("proptx_opt: %d\n", flag);
	}
	return ret;
}

#endif /* PROP_TXSTATUS */

#if defined(BCM_DHD_RUNNER)
static int
dhd_runner_varstr(void *dhd, cmd_t *cmd, char **argv)
{
	char buff[128];
	uint argc;
	char *p  = NULL;
	char *endp = NULL;
	char *argvp = NULL;
	int ret;

	/* Check if these are part of dhd_runner supported iovar commands */
	if ((strcmp(cmd->name, "rnr_flowring_profile")) &&
	    (strcmp(cmd->name, "rnr_flowring_policy")) &&
	    (strcmp(cmd->name, "rnr_rxoffload"))) {

	    return BCME_USAGE_ERROR;
	}

	if (argv[1]) {
	    /* iovar set command */
	    p = buff;
	    endp = p+sizeof(buff);

	    /* Copy the arguments to a single buffer */
	    for (argc = 1; argv[argc]; argc++) {
	        argvp = argv[argc];
	        while (p != endp && (*p++ = *argvp++) != '\0');
	        if (p == endp)
	            return BCME_BUFTOOLONG;
	        /* replace string delimiter with parameter delimiter */
	        *(p-1) = ' ';
	    }
	    /* replace parameter delimiter with final string delimiter */
	    *(p-1) = '\0';

	    /* call the iovar set function */
	    ret = dhd_var_setbuf(dhd, cmd->name, buff, (p-buff));
	}
	else
	{
	    /* call the iovar get function */
	    ret = dhd_varstr(dhd, cmd, argv);
	}

	return (ret);
}

#endif /* BCM_DHD_RUNNER */

static int
dhd_get_ramstart(void *dhd, uint32 *ramstart)
{
	int ret;
	char *ramstart_args[] = {"ramstart", NULL};

	/* Read the bus type the DHD driver is associated to */
	if ((ret = dhd_var_get(dhd, NULL, ramstart_args)) != BCME_OK) {
		fprintf(stderr, "%s: error obtaining ramstart\n", __FUNCTION__);

		return ret;
	}

	*ramstart = *(uint32 *)buf;

	return BCME_OK;
}

static void
dhd_ramsize_adj(void *dhd, uint32 bustype, uint32 ramsize_org, FILE *fp)
{
	int i, search_len = 0;
	uint8 *memptr = NULL;
	uint32 ramsize_ptr_ptr[] = {RAMSIZE_PTR_PTR_LIST};
	uint32 read_len;
	hnd_ramsize_ptr_t ramsize_info;
	uint32 magic_num;
	struct portion_hdr {
		uint32 type;
		uint32 len;
	} hdr;

	/* Pcie only for now */
	if (bustype != BUS_TYPE_PCIE) {
		fprintf(stderr, "%s: unsupported bus type %d to adjust ram size.\n",
			__FUNCTION__, bustype);
		return;
	}

	/* Get maximum RAMSIZE info search length */
	for (i = 0; ; i++) {
		if (ramsize_ptr_ptr[i] == RAMSIZE_PTR_PTR_END)
			break;

		if (search_len < ramsize_ptr_ptr[i])
			search_len = ramsize_ptr_ptr[i];
	}

	if (!search_len)
		return;

	search_len += sizeof(hnd_ramsize_ptr_t);

	memptr = malloc(search_len);
	if (memptr == NULL) {
		fprintf(stderr, "%s: Failed to allocate memory %d bytes\n",
			__FUNCTION__, search_len);
		goto err;
	}

	/* test for hybrid firmware */
	read_len = (uint32)fread((char *)&magic_num, sizeof(uint8), sizeof(magic_num), fp);
	if (read_len == sizeof(magic_num)) {
		magic_num = ltoh32(magic_num);
		if (magic_num == PCIE_HYBRIDFW_MAGICNUM) {
			read_len = fread((char *)&hdr, sizeof(uint8), sizeof(hdr), fp);
			if (read_len != sizeof(hdr)) {
				fprintf(stderr, "%s: unrecognized hybridfw format\n",
					__FUNCTION__);
				goto err;
			}
			if (ltoh32(hdr.type) == PCIE_HYBRIDFW_TYPE_DNGL) {
				goto ramsize_continue;
			}
			fprintf(stderr, "%s: unexpected hybridfw type: %u\n",
				__FUNCTION__, ltoh32(hdr.type));
			goto err;
		}
	}

	fseek(fp, 0, SEEK_SET);

ramsize_continue:
	/* Read image file */
	read_len = (uint32)fread(memptr, sizeof(uint8), search_len, fp);
	fseek(fp, 0, SEEK_SET);
	if (read_len != search_len) {
		fprintf(stderr, "%s: Failed to allocate memory %d bytes\n",
			__FUNCTION__, search_len);
		goto err;
	}

	/* Check Magic */
	for (i = 0; ; i++) {
		if (ramsize_ptr_ptr[i] == RAMSIZE_PTR_PTR_END)
			break;

		if (ramsize_ptr_ptr[i] + sizeof(hnd_ramsize_ptr_t) > search_len)
			continue;

		memcpy((char *)&ramsize_info, memptr + ramsize_ptr_ptr[i],
			sizeof(hnd_ramsize_ptr_t));

		if (ramsize_info.magic == HND_RAMSIZE_PTR_MAGIC &&
			ramsize_org != ramsize_info.ram_size) {
			if (dhd_var_setbuf(dhd, "ramsize", &ramsize_info.ram_size,
				sizeof(ramsize_info.ram_size))) {
				fprintf(stderr, "%s: error setting ramsize\n", __FUNCTION__);
				goto err;
			}
			fprintf(stderr, "Adjust dongle ram size to 0x%x\n", ramsize_info.ram_size);
			break;
		}
	}

err:
	if (memptr)
		free(memptr);

	return;
}

static int
dhd_download(void *dhd, cmd_t *cmd, char **argv)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	bool reset = TRUE;
	bool run = TRUE;
	bool verify = FALSE;
	bool backdoor = FALSE;
	char *fname = NULL;
	char *vname = NULL;
	uint32 start;
	int ret = 0;
	int fsize;
	uint32 bustype;
	long filepos;

	FILE *fp = NULL;
	uint32 ramsize;
	char *memszargs[] = { "ramsize", NULL };

	char *bufp;

	miniopt_t opts;
	int opt_err;
	uint nvram_len;
	struct trx_header trx_hdr;
	uint32 trx_hdr_len;
	bool trx_file = FALSE;
	uint memblock_sz = MEMBLOCK;
	bool embedded_ucode = FALSE;

	UNUSED_PARAMETER(cmd);

	if ((ret = dhd_get_ramstart(dhd, &start)) != BCME_OK)
		goto exit;

	/* Parse command-line options */
	miniopt_init(&opts, "download", "", TRUE);

	argv++;
	while ((opt_err = miniopt(&opts, argv)) != -1) {
		if (opt_err == 1) {
			fprintf(stderr, "download options error\n");
			ret = -1;
			goto exit;
		}
		argv += opts.consumed;

		if (opts.opt == 'a') {
			if (!opts.good_int) {
				fprintf(stderr, "invalid address %s\n", opts.valstr);
				ret = -1;
				goto exit;
			}
			start = (uint32)opts.uval;
		} else if (opts.positional) {
			if (fname && vname) {
				fprintf(stderr, "extra positional arg, %s\n",
				        opts.valstr);
				ret = -1;
				goto exit;
			}
			if (fname)
				vname = opts.valstr;
			else
				fname = opts.valstr;
		} else if (!opts.opt) {
			if (!strcmp(opts.key, "noreset")) {
				reset = FALSE;
			} else if (!strcmp(opts.key, "norun")) {
				run = FALSE;
			} else if (!strcmp(opts.key, "verify")) {
				verify = TRUE;
			} else if (!strcmp(opts.key, "backdoor")) {
				backdoor = TRUE;
			} else {
				fprintf(stderr, "unrecognized option %s\n", opts.valstr);
				ret = -1;
				goto exit;
			}
		} else {
			fprintf(stderr, "unrecognized option %c\n", opts.opt);
			ret = -1;
			goto exit;
		}
	}

	/* validate arguments */
	if (!fname) {
		fprintf(stderr, "filename required\n");
		ret = -1;
		goto exit;
	}

	/* validate file size compared to memory size */
	if ((fsize = file_size(fname)) < 0) {
		ret = -1;
		goto exit;
	}
	/* read the file and push blocks down to memory */
	if ((fp = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "%s: unable to open %s: %s\n",
		        __FUNCTION__, fname, strerror(errno));
		ret = -1;
		goto exit;
	}
	/* Verify the file is a regular bin file or trx file */
	{
		uint32 tmp_len;
		trx_hdr_len = sizeof(struct trx_header);
		tmp_len = (uint32)fread(&trx_hdr, sizeof(uint8), trx_hdr_len, fp);
		if (tmp_len == trx_hdr_len) {
			if (trx_hdr.magic == TRX_MAGIC) {
				trx_file = TRUE;
				if (trx_hdr.flag_version & TRX_EMBED_UCODE)
					embedded_ucode = TRUE;
			}
			else
				fseek(fp, 0, SEEK_SET);
		}
		else
			fseek(fp, 0, SEEK_SET);
	}

	/* Check on which bus the dhd driver is sitting. Downloading methodology differs from
	 * USB to SDIO.
	 */
	{
		char* bustype_args[] = {"bustype", NULL};

		/* Read the bus type the DHD driver is associated to */
		if ((ret = dhd_var_get(dhd, NULL, bustype_args))) {
			fprintf(stderr, "%s: error obtaining bustype\n", __FUNCTION__);
			goto exit;
		}

		bustype = *(uint32*)buf;
	}

	if (trx_file)
		fsize = (int)(trx_hdr.offsets[0]);
	if (bustype == BUS_TYPE_PCIE) {
		if ((ret = dhd_var_get(dhd, NULL, memszargs))) {
			fprintf(stderr, "%s: error obtaining memsize\n", __FUNCTION__);
			goto exit;
		}
		ramsize = *(uint32*)buf;

		/* Adjust the ramsize if ram image has specified, for now pcie only */
		dhd_ramsize_adj(dhd, bustype, ramsize, fp);
	}

	if (bustype == BUS_TYPE_SDIO) {
		if ((ret = dhd_var_get(dhd, NULL, memszargs))) {
			fprintf(stderr, "%s: error obtaining ramsize\n", __FUNCTION__);
			goto exit;
		}
		ramsize = *(uint32*)buf;
	}

	BCM_REFERENCE(ramsize);

	/* do the download reset if not suppressed */
	if (reset) {
		if ((ret = dhd_iovar_setint(dhd, "dwnldstate", TRUE))) {
			fprintf(stderr, "%s: failed to put dongle in download mode\n",
			        __FUNCTION__);
			goto exit;
		}
	}

	/* Load the ram image */
	if (backdoor) {
		fprintf(stderr, "%s: Backdoor On: bypass FW download\n", __FUNCTION__);
	} else {
		/* Recognize and download hybrid firmware if there is one */
		ret = dhd_load_hybridfw(dhd, fname, fp, fsize);
		/* Return value:
		 * BCME_OK: hybrid firmware found and downloaded, skip dhd_load_file_bytes
		 * BCME_UNSUPPORTED: Not a hybrid firmware, continue to dhd_load_file_bytes
		 * Anything else: errors, return
		 */
		if (ret != BCME_UNSUPPORTED) {
			if (ret != BCME_OK)  {
				fprintf(stderr, "%s: hybridfw load failed %d\n", __FUNCTION__, ret);
				goto exit;
			}
		} else
		if ((ret = dhd_load_file_bytes(dhd, cmd, fp, fsize, start, memblock_sz, verify))) {
			fprintf(stderr, "%s: error loading the ramimage at addr 0x%x\n",
			        __FUNCTION__, start);
			goto exit;
		}
	}

	if (trx_file) {

		filepos = ftell(fp);
		if (filepos == -1) {
			fprintf(stderr, "%s: ftell failed.\n", __FUNCTION__);
		}

		if (BUS_TYPE_SDIO == bustype) {

		}
	}

	fclose(fp);
	fp = NULL;

	/* download the vars file if specified */
	if (vname) {
		bufp = bufdump;
		strcpy(bufp, "vars");
		bufp += strlen("vars") + 1;

		if ((ret = read_vars(vname, bufp,
			DHD_IOCTL_MAXLEN_HIGH - ((int)strlen("vars") + 3))) < 0) {
			ret = -1;
			goto exit;
		}

		nvram_len = ret;
		bufp += nvram_len;
		*bufp++ = 0;

		ret = dhd_set(dhd, DHD_SET_VAR, bufdump, (int)(bufp - bufdump));
		if (ret) {
			fprintf(stderr, "%s: error %d on delivering vars\n",
			        __FUNCTION__, ret);
			goto exit;
		}
	}

	/* start running the downloaded code if not suppressed */
	if (run) {
		if ((ret = dhd_iovar_setint(dhd, "dwnldstate", FALSE))) {

			fprintf(stderr, "%s: failed to take dongle out of download mode\n",
			        __FUNCTION__);
			goto exit;
		}
	}
	if (embedded_ucode) {
	}

exit:
	if (fp)
		fclose(fp);

	return ret;
#endif /* BWL_FILESYSTEM_SUPPORT */
}

static int
dhd_dldn(void *dhd, cmd_t *cmd, char **argv)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	char *fname = NULL;
	uint32 start;
	int ret = 0;
	int fsize;
	int fd = 0;

	FILE *fp = NULL;
	uint32 ramsize;

	uint len;
	uint8 memblock[MEMBLOCK];

	miniopt_t opts;
	int opt_err;

	UNUSED_PARAMETER(cmd);

	/* Parse command-line options */
	miniopt_init(&opts, "download", "", TRUE);
	argv++;

	while ((opt_err = miniopt(&opts, argv)) != -1) {
		if (opt_err == 1) {
			fprintf(stderr, "download options error\n");
			ret = -1;
			goto exit;
		}
		argv += opts.consumed;

		if (opts.positional) {
			if (fname) {
				fprintf(stderr, "extra positional arg, %s\n",
				        opts.valstr);
				ret = -1;
				goto exit;
			}
			if (!fname)
				fname = opts.valstr;
		} else {
			fprintf(stderr, "unrecognized option %c\n", opts.opt);
			ret = -1;
			goto exit;
		}
	}

	fd = dhd_set(dhd, DHD_DLDN_ST, NULL, 0);
	if (fd < 0) {
		ret = -1;
		goto exit;
	}

	/* validate arguments */
	if (!fname) {
		fprintf(stderr, "filename required\n");
		ret = -1;
		goto exit;
	}

	/* validate file size compared to memory size */
	if ((fsize = file_size(fname)) < 0) {
		ret = -1;
		goto exit;
	}

	ramsize = 393216;

	if (ramsize && ((uint32)fsize > ramsize)) {
		fprintf(stderr, "%s: file %s too large (%d > %d)\n",
		        __FUNCTION__, fname, fsize, ramsize);
		ret = -1;
		goto exit;
	}

	/* read the file and push blocks down to memory */
	if ((fp = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "%s: unable to open %s: %s\n",
		        __FUNCTION__, fname, strerror(errno));
		ret = -1;
		goto exit;
	}

	if ((ret = dhd_get_ramstart(dhd, &start)) != BCME_OK)
		goto exit;

	while ((len = (uint)fread(memblock, sizeof(uint8), MEMBLOCK, fp))) {
		if (len < MEMBLOCK && !feof(fp)) {
			fprintf(stderr, "%s: error reading file %s\n", __FUNCTION__, fname);
			ret = -1;
			goto exit;
		}

		ret = dhd_set(dhd, DHD_DLDN_WRITE, memblock, len);
		if (ret) {
			fprintf(stderr, "%s: error %d on writing %d membytes at 0x%08x\n",
			        __FUNCTION__, ret, len, start);
			goto exit;
		}

		start += len;
	}

	if (!feof(fp)) {
		fprintf(stderr, "%s: error reading file %s\n", __FUNCTION__, fname);
		ret = -1;
		goto exit;
	}
	fclose(fp);
	fp = NULL;

exit:
	if (fp)
		fclose(fp);

	if (fd)
		ret = dhd_set(dhd, DHD_DLDN_END, NULL, 0);

	return ret;
#endif /* BWL_FILESYSTEM_SUPPORT */
}

enum file_type_e {
	FILE_TYPE_SREC,      /** Motorola S-record, to be read by e.g. a gdb client */
};

/**
 * Writes register or memory contents of e.g. ChipCommon core into a Motorola S-record file for
 * offline firmware debugging using gdb.
 *
 * @param file_path   File to create
 * @param typ         DUMP_DONGLE_COREREG or DUMP_DONGLE_D11MEM_SHM or ...
 * @param ft          e.g. FILE_TYPE_SREC
 * @param idx_to_dump for regs: 0:dump all cores, >0: dump specific core. For mem: 0: first mem.
 */
static int
dhd_upload_core(void *dhd, char *file_path, enum dump_dongle_e typ, enum file_type_e ft,
	int idx_to_dump)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	int ret = 0;
	dump_dongle_in_t ddi;    /**< from utility to driver */
	dump_dongle_out_t *ddo;  /**< from driver to utility */
	FILE *fp;
	uint8 bbuf[256];         /**< contains one Motorola S-record line */
	int n_srec_bytes;        /**< number of bytes in Motorola S-record line following header */
	int srec_len;
	uint8 srec_checksum;     /**< Motorola S-record */
	int idx = 0;
	uint32 addr_base;
	uint32 coreaddr;

	switch (typ) {
		case DUMP_DONGLE_COREREG:
		case DUMP_DONGLE_D11MEM_AMT:
			break;
		default:
			printf("%s unknown type %d\n", __FUNCTION__, typ);
			return BCME_ERROR;
	}

	ddi.type = typ;
	ddi.index = idx_to_dump;
	fp = fopen(file_path, "w");
	if (fp == NULL) {
		fprintf(stderr, "could not open file %s\n", file_path);
		return BCME_ERROR;
	}

	fprintf(stderr, "dump file %s\n", file_path);
	addr_base = 0;
	do { // iterate over cores
		coreaddr = 0;
		ddi.byte_offset = 0;
		do { // iterate over single register set or single memory
			ret = dhd_var_getbuf(dhd, "dump_dongle", &ddi, sizeof(ddi), (void**)&ddo);
			if (ret != BCME_OK) {
				fprintf(stderr, "%s dump failed: index=%d\n",
					file_path, ddi.index);
				break;
			}

			if (coreaddr == 0) {
				coreaddr = ddo->address;
			}

			if (typ == DUMP_DONGLE_COREREG) {
				if (!addr_base) {
					addr_base = ddo->address & 0xFFFF0000;
				}
				if ((ddo->address & 0xFFFF0000) != addr_base) {
					ddo->n_bytes = 0;
				}
			}

			if (ddo->n_bytes > 0) {
				int i, j;

				srec_len = 4 + 1 + ddo->element_width; /* 4 bytes addr, 1 crc */
				/* the for loop prints one output file line per array element */
				for (i = 0; i < ddo->n_bytes / ddo->element_width; i++) {
					n_srec_bytes = 0;
					bbuf[n_srec_bytes++] = srec_len;
					*((uint32*)&bbuf[n_srec_bytes]) = BCMSWAP32(ddo->address);
					n_srec_bytes += 4; /* 4 address bytes */
					switch (ddo->element_width) {
					case sizeof(uint16):
						*((uint16*)&bbuf[n_srec_bytes]) = ddo->u.ui16[i];
						break;
					case sizeof(uint32):
						*((uint32*)&bbuf[n_srec_bytes]) = ddo->u.ui32[i];
						break;
					case sizeof(uint64):
						*((uint64*)&bbuf[n_srec_bytes]) = ddo->u.ui64[i];
						break;
					}
					n_srec_bytes += ddo->element_width;
					srec_checksum = 0;
					switch (ft) { // different output file formats
					case FILE_TYPE_SREC:
						fprintf(fp, "S3"); // Motorola S3 record
						for (j = 0; j < n_srec_bytes; j++) {
							fprintf(fp, "%02X", bbuf[j]);
							srec_checksum += bbuf[j];
						}
						srec_checksum = ~srec_checksum;
						fprintf(fp, "%02X\n", srec_checksum);
						break;
					}
					ddo->address += ddo->element_width;
					idx++;
				}
			}
			ddi.byte_offset += ddo->n_bytes;
		} while (ddo->n_bytes > 0);
		/* print core info */
		if (ddo->id && ddi.byte_offset) {
			fprintf(stderr, " [%d] core id %x addr %08x size %u\n",
				ddi.index, ddo->id, coreaddr, ddi.byte_offset);
		}
		ddi.index++;
	} while (ret == BCME_OK && (
		(typ == DUMP_DONGLE_COREREG && ddo->id != 0) ||
		(typ != DUMP_DONGLE_COREREG && ddo->n_bytes > 0)));
	fclose(fp);

	return ret;
#endif /* BWL_FILESYSTEM_SUPPORT */
} /* dhd_upload_core */

/**
 * if the option '-r 0' is used, dongle register contents is dumped, and the ARM on the dongle is
 * reset.
 */
static int
dhd_upload(void *dhd, cmd_t *cmd, char **argv)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	char *fname = NULL;
	uint32 start;
	uint32 size = 0;
	int ret = 0;
	FILE *fp;
	uint32 ramsize;
	char *memszargs[] = { "ramsize", NULL };
	char file_path[100];
	uint len;
	miniopt_t opts;
	int opt_err;
	int coreidx_to_dump = -1; /**< -1:no upload, 0:upload all, >0: upload specific core */

	UNUSED_PARAMETER(cmd);
	UNUSED_PARAMETER(argv);

	if ((ret = dhd_get_ramstart(dhd, &start)) != BCME_OK)
		goto exit;

	/* Parse command-line options */
	miniopt_init(&opts, "upload", "", TRUE);

	argv++;
	while ((opt_err = miniopt(&opts, argv)) != -1) {
		if (opt_err == 1) {
			fprintf(stderr, "upload options error\n");
			ret = -1;
			goto exit;
		}
		argv += opts.consumed;

		if (opts.opt == 'a') {
			if (!opts.good_int) {
				fprintf(stderr, "invalid address %s\n", opts.valstr);
				ret = -1;
				goto exit;
			}
			start = (uint32)opts.uval;
		} else if (opts.opt == 's') {
			if (!opts.good_int) {
				fprintf(stderr, "invalid RAM size %s\n", opts.valstr);
				ret = -1;
				goto exit;
			}
			size = (uint32)opts.uval;
		} else if (opts.opt == 'r') {
			if (!opts.good_int) {
				fprintf(stderr, "invalid core index %s\n", opts.valstr);
				ret = -1;
				goto exit;
			}
			coreidx_to_dump = (uint32)opts.uval;
		} else if (opts.positional) {
			if (!fname) {
				fname = opts.valstr;
			} else {
				fprintf(stderr, "upload options error\n");
				ret = -1;
				goto exit;
			}
		} else if (!opts.opt) {
			fprintf(stderr, "unrecognized option %s\n", opts.valstr);
			ret = -1;
			goto exit;
		} else {
			fprintf(stderr, "unrecognized option %c\n", opts.opt);
			ret = -1;
			goto exit;
		}
	}

	/* validate arguments */
	if (!fname) {
		fprintf(stderr, "filename required\n");
		ret = -1;
		goto exit;
	}

	if ((ret = dhd_var_get(dhd, NULL, memszargs))) {
		fprintf(stderr, "%s: error obtaining ramsize\n", __FUNCTION__);
		goto exit;
	}
	ramsize = *(uint32*)buf;

	if (!ramsize)
		ramsize = size;

	strlcpy(file_path, fname, sizeof(file_path));
	if ((fp = fopen(file_path, "wb")) == NULL) {
		fprintf(stderr, "%s: Could not open %s: %s\n",
		        __FUNCTION__, file_path, strerror(errno));
		ret = -1;
		goto exit;
	}

	/* default size to full RAM */
	if (!size)
		size = ramsize;

	if (coreidx_to_dump != -1) {
		dhd_iovar_setint(dhd, "halt_dongle", 1); // for consistent memory / regs dumps
	}

	/* read memory and write to file */
	while (size) {
		char *ptr;
		int params[2];

		len = MIN(MEMBLOCK, size);

		params[0] = start;
		params[1] = len;
		ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&ptr);
		if (ret) {
			fprintf(stderr, "%s: failed reading %d membytes from 0x%08x\n",
			        __FUNCTION__, len, start);
			break;
		}

		if (fwrite(ptr, sizeof(char), len, fp) != len) {
			fprintf(stderr, "%s: error writing to file %s\n", __FUNCTION__, fname);
			ret = -1;
			break;
		}

		start += len;
		size -= len;
	}

	fclose(fp);

	if (ret == BCME_OK && coreidx_to_dump != -1) {
		int i;
		const struct {
			char *ext;               /**< file_extension */
			enum dump_dongle_e typ;  /**< what type of dump */
			enum file_type_e ft;     /**< output file type */
			int idx;                 /**< index of entity to dump */
		} files_to_generate[] = {
			{".core_regs", DUMP_DONGLE_COREREG, FILE_TYPE_SREC, 0},
			{".amt", DUMP_DONGLE_D11MEM_AMT, FILE_TYPE_SREC, 0},
		};
		for (i = 0; i < ARRAYSIZE(files_to_generate); i++) {
			strlcpy(file_path, fname, sizeof(file_path));
			strlcat_s(file_path, files_to_generate[i].ext, sizeof(file_path));
			ret = dhd_upload_core(dhd, file_path, files_to_generate[i].typ,
				files_to_generate[i].ft, files_to_generate[i].idx);
			if (ret != BCME_OK) {
				printf("failed to dump %s\n", file_path);
				break;
			}
		}
		dhd_iovar_setint(dhd, "devreset", 1);
	}

exit:
	return ret;
#endif /* BWL_FILESYSTEM_SUPPORT */
} /* dhd_upload */

#ifdef BWL_FILESYSTEM_SUPPORT
static int
dhd_get_debug_info(void *dhd, hnd_debug_t *debug_info)
{
	int i;
	int ret;
	int params[2];

	uint32 *buffer;
	uint32 ramstart;
	hnd_debug_ptr_t *debugPtr;

	if ((ret = dhd_get_ramstart(dhd, &ramstart)) != BCME_OK)
		return ret;

	/*
	 * Different chips have different fixed debug_info_ptrs
	 * because of different ROM locations/uses.  Try them all looking
	 * for the magic number.
	 */
	for (i = 0; ; i++) {
		if (dump_info_ptr_ptr[i] == DUMP_INFO_PTR_PTR_END) {
			fprintf(stderr, "Error: cannot find pointer to debug_info\n");
			return -1;
		}

		params[0] = dump_info_ptr_ptr[i] + ramstart;
		params[1] = sizeof(hnd_debug_ptr_t);
		ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&debugPtr);
		if ((ret == 0) &&
		    (debugPtr->magic == HND_DEBUG_PTR_PTR_MAGIC))
			break;
	}

	if (debugPtr->hnd_debug_addr == 0) {
		fprintf(stderr, "Error: Debug info pointer is zero\n");
		return -1;
	}

	/* Read the area the debuginfoptr points at */
	params[0] = debugPtr->hnd_debug_addr;
	params[1] = sizeof(hnd_debug_t);
	ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&buffer);
	if (ret) {
		fprintf(stderr, "%s: failed reading %lu membytes from 0x%08lx\n",
			__FUNCTION__, (long unsigned) params[1], (long unsigned) params[0]);
		return ret;
	}

	memcpy((char *) debug_info, buffer, sizeof(hnd_debug_t));

	/* Sanity check the area */
	if ((debug_info->magic != HND_DEBUG_MAGIC) ||
	    (debug_info->version != HND_DEBUG_VERSION)) {
		fprintf(stderr, "Error: Invalid debug info area\n");
		return -1;
	}

	return 0;
}
#endif /* BWL_FILESYSTEM_SUPPORT */

static int
dhd_coredump(void *dhd, cmd_t *cmd, char **argv)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	char *fname = NULL;
	int ret;

	FILE *fp;

	hnd_debug_t debugInfo;

	miniopt_t opts;
	int opt_err;

	int params[2];
	char *ptr;

	unsigned int start;
	unsigned int size;

	prstatus_t prstatus;

	UNUSED_PARAMETER(cmd);
	UNUSED_PARAMETER(argv);

	/* Parse command-line options */
	miniopt_init(&opts, "dump", "", TRUE);

	argv++;
	while ((opt_err = miniopt(&opts, argv)) != -1) {
		if (opt_err == 1) {
			fprintf(stderr, "dump options error\n");
			ret = -1;
			goto exit;
		}
		argv += opts.consumed;

		if (opts.positional) {
			if (!fname) {
				fname = opts.valstr;
			} else {
				fprintf(stderr, "dump options error\n");
				ret = -1;
				goto exit;
			}
		} else if (!opts.opt) {
			fprintf(stderr, "unrecognized option %s\n", opts.valstr);
			ret = -1;
			goto exit;
		} else {
			fprintf(stderr, "unrecognized option %c\n", opts.opt);
			ret = -1;
			goto exit;
		}
	}

	/* validate arguments */
	if (!fname) {
		fprintf(stderr, "filename required\n");
		ret = -1;
		goto exit;
	}

	if ((ret = dhd_get_debug_info(dhd, &debugInfo)) < 0)
		goto exit;

	/* Get the base and size to dump */
	start = debugInfo.ram_base;
	size = debugInfo.ram_size;

	/* Get the arm trap area */
	bzero(&prstatus, sizeof(prstatus_t));
	if (debugInfo.trap_ptr != 0) {
		int i;
		trap_t armtrap;
		uint32 *reg;

		params[0] = debugInfo.trap_ptr;
		params[1] = sizeof(trap_t);
		ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&ptr);
		if (ret) {
			fprintf(stderr, "%s: failed reading %lu membytes from 0x%08lx\n",
				__FUNCTION__, (long unsigned) params[1], (long unsigned) params[0]);
			goto exit;
		}

		memcpy((char *) &armtrap, ptr, sizeof(trap_t));

		/* Populate the prstatus */
		prstatus.si_signo = armtrap.type;
		reg = &armtrap.r0;
		for (i = 0; i < 15; i++, reg++) {
			prstatus.uregs[i] = *reg;
		}
		prstatus.uregs[15] = armtrap.epc;
	}

	if ((fp = fopen(fname, "wb")) == NULL) {
		fprintf(stderr, "%s: Could not open %s: %s\n",
		        __FUNCTION__, fname, strerror(errno));
		ret = -1;
		goto exit;
	}

	/* Write the preamble and debug header */
	fprintf(fp, "Dump starts for version %s FWID 01-%x\n", debugInfo.epivers, debugInfo.fwid);
	fprintf(fp, "XXXXXXXXXXXXXXXXXXXX");
	fprintf(fp, "%8.8lX", (long unsigned) sizeof(debugInfo));
	if (fwrite(&debugInfo, sizeof(unsigned char), sizeof(debugInfo), fp) != sizeof(debugInfo)) {
		fprintf(stderr, "%s: error writing to file %s\n", __FUNCTION__, fname);
		ret = -1;
		fclose(fp);
		goto exit;
	}

	/* Write the prstatus */
	if (fwrite(&prstatus, sizeof(unsigned char), sizeof(prstatus), fp) != sizeof(prstatus)) {
		fprintf(stderr, "%s: error writing to file %s\n", __FUNCTION__, fname);
		ret = -1;
		fclose(fp);
		goto exit;
	}

	/* Write the ram size as another sanity check */
	fprintf(fp, "%8.8X", size);

	/* read memory and write to file */
	while (size) {
		int len;
		len = MIN(MEMBLOCK, size);

		params[0] = start;
		params[1] = len;
		ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&ptr);
		if (ret) {
			fprintf(stderr, "%s: failed reading %d membytes from 0x%08x\n",
			        __FUNCTION__, len, start);
			break;
		}

		if (fwrite(ptr, sizeof(char), len, fp) != (uint) len) {
			fprintf(stderr, "%s: error writing to file %s\n", __FUNCTION__, fname);
			ret = -1;
			break;
		}

		start += len;
		size -= len;
	}

	fclose(fp);
exit:
	return ret;
#endif /* BWL_FILESYSTEM_SUPPORT */
}

static int
dhd_consoledump(void *dhd, cmd_t *cmd, char **argv)
{
#if !defined(BWL_FILESYSTEM_SUPPORT)
	return (-1);
#else
	int ret;

	hnd_debug_t debugInfo;

	miniopt_t opts;
	int opt_err;

	int params[2];
	char *ptr;

	unsigned int start;
	unsigned int size;
	int len;

	UNUSED_PARAMETER(cmd);
	UNUSED_PARAMETER(argv);

	/* Parse command-line options */
	miniopt_init(&opts, "consoledump", "", TRUE);

	argv++;
	while ((opt_err = miniopt(&opts, argv)) != -1) {
		if (opt_err == 1) {
			fprintf(stderr, "dump options error\n");
			ret = -1;
			goto exit;
		}
		argv += opts.consumed;

		if (!opts.opt) {
			fprintf(stderr, "unrecognized option %s\n", opts.valstr);
			ret = -1;
			goto exit;
		} else {
			fprintf(stderr, "unrecognized option %c\n", opts.opt);
			ret = -1;
			goto exit;
		}
	}

	if ((ret = dhd_get_debug_info(dhd, &debugInfo)) < 0)
		goto exit;

	if (debugInfo.console <= debugInfo.ram_base) {
		fprintf(stderr, "%s: console not found\n", __FUNCTION__);
		ret = -1;
		goto exit;
	}

	/* Get the debug console area */
	params[0] = debugInfo.console;
	params[1] = sizeof(hnd_cons_t);
	ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&ptr);
	if (ret) {
		fprintf(stderr, "%s: failed reading %lu membytes from 0x%08lx\n",
			__FUNCTION__, (long unsigned) params[1], (long unsigned) params[0]);
		goto exit;
	}

	if (ptr == NULL) {
		fprintf(stderr, "%s: console not initialised\n", __FUNCTION__);
		ret = -1;
		goto exit;
	}

	start = (unsigned int)((hnd_cons_t *)ptr)->log.buf;
	size = ((hnd_cons_t *)ptr)->log.buf_size;

	if (start <= debugInfo.ram_base) {
		fprintf(stderr, "%s: console buffer not initialised\n", __FUNCTION__);
		ret = -1;
		goto exit;
	}

	/* read memory and write to file */
	while (size > 0) {
		len = MIN(MEMBLOCK, size);

		params[0] = start;
		params[1] = len;
		ret = dhd_var_getbuf(dhd, "membytes", params, 2 * sizeof(int), (void**)&ptr);
		if (ret) {
			fprintf(stderr, "%s: failed reading %d membytes from 0x%08x\n",
			        __FUNCTION__, len, start);
			break;
		}

		printf("%s", ptr);

		start += len;
		size -= len;
	}

exit:
	return ret;
#endif /* BWL_FILESYSTEM_SUPPORT */
}

static int
dhd_logstamp(void *dhd, cmd_t *cmd, char **argv)
{
	int ret;
	char *endptr = NULL;
	uint argc;
	int valn[2] = {0, 0};

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--; argv++;

	if (argc > 2)
		return BCME_USAGE_ERROR;

	if (argc) {
		valn[0] = strtol(argv[0], &endptr, 0);
		if (*endptr != '\0') {
			printf("bad val1: %s\n", argv[0]);
			return BCME_USAGE_ERROR;
		}
	}

	if (argc > 1) {
		valn[1] = strtol(argv[1], &endptr, 0);
		if (*endptr != '\0') {
			printf("bad val2: %s\n", argv[1]);
			return BCME_USAGE_ERROR;
		}
	}

	ret = dhd_var_setbuf(dhd, cmd->name, valn, argc * sizeof(int));

	return (ret);
}

#define MAXPD11REGSCNT 32
static int
dhd_pd11regs_bylist(void *dhd, d11regs_list_t *reglist, uint16 reglist_sz, bool verbose)
{
	uint i;
	int err = BCME_OK;
	dhd_pd11regs_buf_t *pd11regsbuf;
	uint16 param_sz = OFFSETOF(dhd_pd11regs_param_t, plist) + sizeof(*reglist);
	char param_buf[param_sz];
	dhd_pd11regs_param_t *param = (void *)param_buf;

	param->start_idx = 0;
	param->verbose = verbose;

	if (reglist != NULL && reglist_sz > 0) {
		for (i = 0; i < reglist_sz; i++) {
			if (reglist[i].cnt == 0) {
				memcpy(param->plist, &reglist[i], sizeof(*reglist));
				err = dhd_var_getbuf(dhd, "pd11regs", param, param_sz,
					(void **)&pd11regsbuf);
				if (err != BCME_OK) {
					printf("Error %d %d\n", err, pd11regsbuf->idx);
					goto exit;
				}
				param->start_idx += pd11regsbuf->idx;
				printf("%s", pd11regsbuf->pbuf);
			} else {
				uint16 rem_cnt;
				rem_cnt = reglist[i].cnt;
				while (rem_cnt > 0) {
					reglist[i].cnt = MIN(MAXPD11REGSCNT, rem_cnt);
					memcpy(param->plist, &reglist[i], sizeof(*reglist));
					err = dhd_var_getbuf(dhd, "pd11regs", param, param_sz,
						(void **)&pd11regsbuf);
					if (err != BCME_OK) {
						printf("Error %d\n", err);
						goto exit;
					}
					if ((pd11regsbuf->idx == 0) ||
						(pd11regsbuf->idx != reglist[i].cnt)) {
						err = BCME_ERROR;
						goto exit;
					}
					printf("%s", pd11regsbuf->pbuf);
					reglist[i].byte_offset += reglist[i].step * reglist[i].cnt;
					rem_cnt -= reglist[i].cnt;
					param->start_idx += reglist[i].cnt;
				}
			}
			continue;
		}
	}
exit:
	return err;
}

static int
dhd_pmac(void *dhd, cmd_t *cmd, char **argv)
{
	int err = BCME_OK;
	d11regs_list_t reglist = {0, };
	char *p, opt;
	bool verbose = FALSE;

	/* Skip the iovar name and get the type */
	if ((p = *(++argv)) == NULL) {
		err = BCME_USAGE_ERROR;
		goto exit;
	}

	if (!strncmp(p, "ihrcm7", 6)) {
		reglist.type = D11REG_TYPE_IHR16CM7;
		reglist.step = 2;
	} else if (!strncmp(p, "shmcm7", 6)) {
		reglist.type = D11REG_TYPE_SHMCM7;
		reglist.step = 2;
	} else if (!strncmp(p, "scrx", 4)) {
		reglist.type = D11REG_TYPE_SCRX;
		reglist.step = 2;
	} else if (!strncmp(p, "shmx", 4)) {
		reglist.type = D11REG_TYPE_SHMX;
		reglist.step = 2;
	} else if (!strncmp(p, "ihrx32", 6)) {
		/* ihrx32 reads the same space as ihr32 */
		reglist.type = D11REG_TYPE_IHR32;
		reglist.step = 4;
	} else if (!strncmp(p, "ihrx", 4)) {
		reglist.type = D11REG_TYPE_IHRX16;
		reglist.step = 2;
	} else if (!strncmp(p, "ihr1", 4)) {
		reglist.type = D11REG_TYPE_IHR116;
		reglist.step = 2;
	} else if (!strncmp(p, "shm1", 4)) {
		reglist.type = D11REG_TYPE_SHM1;
		reglist.step = 2;
	} else if (!strncmp(p, "scr", 3)) {
		reglist.type = D11REG_TYPE_SCR;
		reglist.step = 1;
	} else if (!strncmp(p, "shm", 3)) {
		reglist.type = D11REG_TYPE_SHM;
		reglist.step = 2;
	} else if (!strncmp(p, "ihr32", 5)) {
		reglist.type = D11REG_TYPE_IHR32;
		reglist.step = 4;
	} else if (!strncmp(p, "ihr", 3)) {
		reglist.type = D11REG_TYPE_IHR16;
		reglist.step = 2;
	} else {
		printf("Invalid selection!!\n");
		err = BCME_USAGE_ERROR;
		goto exit;
	}

	/* skip the type and get the address */
	if ((p = *(++argv)) == NULL) {
		err = BCME_USAGE_ERROR;
		goto exit;
	}

	reglist.byte_offset = strtol(p, NULL, 0);
	/* default cnt 1 in case there is no argument given */
	reglist.cnt = 1;

	/* skip the address */
	argv++;

	/* skip the address and process additional arguments */
	while ((p = *argv)) {
		argv++;
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2 || (p[1] != 'v' && *argv == NULL)) {
				err = BCME_USAGE_ERROR;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 's':
					reglist.step = strtol(*argv, NULL, 0);
					argv++;
					break;
				case 'n':
					reglist.cnt = strtol(*argv, NULL, 0);
					argv++;
					break;
				case 'b':
					reglist.bitmap = strtoul(*argv, NULL, 0);
					argv++;
					break;
				case 'v':
					verbose = TRUE;
					break;
				default:
					printf("Invalid option!!\n");
					err = BCME_USAGE_ERROR;
					goto exit;
			}
		} else {
			printf("Invalid argument %s!!\n", p);
			err = BCME_USAGE_ERROR;
			goto exit;
		}
	}

	err = dhd_pd11regs_bylist(dhd, &reglist, 1, verbose);
exit:
	return err;
}

static int
dhd_d11dump(void *dhd, cmd_t *cmd, char **argv)
{
	int err = BCME_OK;
	char *p, *dumpname, opt;
	bool killarm = FALSE;

	/* skip the iovar name */
	argv++;

	dumpname = NULL;
	/* process additional arguments */
	while ((p = *argv)) {
		argv++;
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2) {
				err = BCME_USAGE_ERROR;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 'k':
					killarm = TRUE;
					break;
				default:
					printf("Invalid option!!\n");
					err = BCME_USAGE_ERROR;
					goto exit;
			}
		} else {
			dumpname = p;
		}
	}

	if (killarm) {
		/* dhd_var_setbuf(dhd, "pcie_device_trap", NULL, 0); */
		printf("Don't kill dongle to avoid race condtion with dhd driver dumping macregs\n"
			"Manually halt dongle instead by:\n"
			"wl bus:disconnect 102 -- dongle will dead spin\n"
			"Retry without -k\n");
		err = BCME_BADARG;
		goto exit;
	}

	if (dumpname == NULL) {
		err = BCME_BADARG;
		goto exit;
	}

	err = dhd_var_getbuf(dhd, "macdbg_dump", dumpname, strlen(dumpname), (void **)&p);
	printf("%s", p);
	while (1) {
		err = dhd_var_getbuf(dhd, "macdbg_dump", "cont", strlen("cont"), (void **)&p);
		if (err != BCME_OK) {
			goto exit;
		}
		if (strlen(p) == 0) {
			break;
		}
		printf("%s", p);
	}
exit:
	return err;
}

#define MAXPSVMPSCNT 32
static int
dhd_psvmps_bylist(void *dhd, svmp_list_t *reglist, uint16 reglist_sz, bool verbose)
{
	uint i;
	int err = BCME_OK;
	dhd_pd11regs_buf_t *psvmpssbuf;
	uint16 param_sz = OFFSETOF(dhd_pd11regs_param_t, plist) + sizeof(*reglist);
	char param_buf[param_sz];
	dhd_pd11regs_param_t *param = (void *)param_buf;

	param->start_idx = 0;
	param->verbose = verbose;

	if (reglist != NULL && reglist_sz > 0) {
		for (i = 0; i < reglist_sz; i++) {
			uint16 rem_cnt;
			rem_cnt = reglist[i].cnt;
			while (rem_cnt > 0) {
				reglist[i].cnt = MIN(MAXPSVMPSCNT, rem_cnt);
				memcpy(param->plist, &reglist[i], sizeof(*reglist));
				err = dhd_var_getbuf(dhd, "psvmpmems", param, param_sz,
					(void **)&psvmpssbuf);
				if (err != BCME_OK) {
					printf("Error %d\n", err);
					goto exit;
				}
				if ((psvmpssbuf->idx == 0) ||
					(psvmpssbuf->idx != reglist[i].cnt)) {
					err = BCME_ERROR;
					goto exit;
				}
				printf("%s", psvmpssbuf->pbuf);
				reglist[i].addr += reglist[i].cnt;
				rem_cnt -= reglist[i].cnt;
				param->start_idx += reglist[i].cnt;
			}
		}
	}
exit:
	return err;
}

static int
dhd_psvmp(void *dhd, cmd_t *cmd, char **argv)
{
	int err = BCME_OK;
	char *p, opt;
	bool verbose = FALSE;
	svmp_list_t svmplist;

	/* skip the iovar name and get the address */
	if ((p = *(++argv)) == NULL) {
		err = BCME_USAGE_ERROR;
		goto exit;
	}
	svmplist.addr = strtol(p, NULL, 0);
	/* default cnt 1 in case there is no argument given */
	svmplist.cnt = 1;

	/* skip the address */
	argv++;

	/* process additional arguments */
	while ((p = *argv)) {
		argv++;
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2) {
				err = BCME_USAGE_ERROR;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 'n':
					svmplist.cnt = strtol(*argv, NULL, 0);
					argv++;
					break;
				case 'v':
					verbose = TRUE;
					break;
				default:
					printf("Invalid option!!\n");
					err = BCME_USAGE_ERROR;
					goto exit;
			}
		} else {
			printf("Invalid argument %s!!\n", p);
			err = BCME_USAGE_ERROR;
			goto exit;
		}
	}

	err = dhd_psvmps_bylist(dhd, &svmplist, 1, verbose);
exit:
	return err;
}

static int
dhd_sd_reg(void *dhd, cmd_t *cmd, char **argv)
{
	int ret;
	sdreg_t sdreg;
	char *endptr = NULL;
	uint argc;
	void *ptr = NULL;

	bzero(&sdreg, sizeof(sdreg));

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	/* hostreg: offset [value]; devreg: func offset [value] */
	if (!strcmp(cmd->name, "sd_hostreg")) {
		argv++;
		if (argc < 1) {
			printf("required args: offset [value]\n");
			return BCME_USAGE_ERROR;
		}

	} else if (!strcmp(cmd->name, "sd_devreg")) {
		argv++;
		if (argc < 2) {
			printf("required args: func offset [value]\n");
			return BCME_USAGE_ERROR;
		}

		sdreg.func = strtoul(*argv++, &endptr, 0);
		if (*endptr != '\0') {
			printf("Invalid function number\n");
			return BCME_USAGE_ERROR;
		}
	} else {
		return BCME_USAGE_ERROR;
	}

	sdreg.offset = strtoul(*argv++, &endptr, 0);
	if (*endptr != '\0') {
		printf("Invalid offset value\n");
		return BCME_USAGE_ERROR;
	}

	/* third arg: value */
	if (*argv) {
		sdreg.value = strtoul(*argv, &endptr, 0);
		if (*endptr != '\0') {
			printf("Invalid value\n");
			return BCME_USAGE_ERROR;
		}
	}

	/* no third arg means get, otherwise set */
	if (!*argv) {
		if ((ret = dhd_var_getbuf(dhd, cmd->name, &sdreg, sizeof(sdreg), &ptr)) >= 0)
			printf("0x%x\n", *(int *)ptr);
	} else {
		ret = dhd_var_setbuf(dhd, cmd->name, &sdreg, sizeof(sdreg));
	}

	return (ret);
}

static dbg_msg_t dhd_msgs[] = {
	{DHD_ERROR_VAL,	"error"},
	{DHD_ERROR_VAL, "err"},
	{DHD_TRACE_VAL, "trace"},
	{DHD_INFO_VAL,	"inform"},
	{DHD_INFO_VAL,	"info"},
	{DHD_INFO_VAL,	"inf"},
	{DHD_DATA_VAL,	"data"},
	{DHD_CTL_VAL,	"ctl"},
	{DHD_TIMER_VAL,	"timer"},
	{DHD_HDRS_VAL,	"hdrs"},
	{DHD_BYTES_VAL,	"bytes"},
	{DHD_INTR_VAL,	"intr"},
	{DHD_LOG_VAL,	"log"},
	{DHD_GLOM_VAL,	"glom"},
	{DHD_EVENT_VAL,	"event"},
	{DHD_BTA_VAL,	"bta"},
	{DHD_ISCAN_VAL,	"iscan"},
	{DHD_ARPOE_VAL,	"arpoe"},
	{DHD_REORDER_VAL, "reorder"},
	{DHD_DNGL_IOVAR_SET_VAL, "s_iov"},
	{DHD_DNGL_IOVAR_GET_VAL, "g_iov"},
	{DHD_DHCP_DUMP_VAL, "dhcp"},
	{DHD_PKT_PCAP_VAL, "pcap"},
	{0,		NULL}
};

static int
dhd_msglevel(void *dhd, cmd_t *cmd, char **argv)
{
	return dhd_do_msglevel(dhd, cmd, argv, dhd_msgs);
}

static int
dhd_do_msglevel(void *dhd, cmd_t *cmd, char **argv, dbg_msg_t *dbg_msg)
{
	int ret, i;
	uint val, last_val = 0, msglevel = 0, msglevel_add = 0, msglevel_del = 0;
	char *endptr = NULL;

	if ((ret = dhd_iovar_getint(dhd, cmd->name, (int*)&msglevel)) < 0)
		return (ret);

	if (!*++argv) {
		printf("0x%x ", msglevel);
		for (i = 0; (val = dbg_msg[i].value); i++) {
			if ((msglevel & val) && (val != last_val))
				printf(" %s", dbg_msg[i].string);
			last_val = val;
		}
		printf("\n");
		return (0);
	}

	while (*argv) {
		char *s = *argv;
		if (*s == '+' || *s == '-')
			s++;
		else
			msglevel_del = ~0;	/* make the whole list absolute */
		val = strtoul(s, &endptr, 0);
		/* not a plain integer if not all the string was parsed by strtoul */
		if (*endptr != '\0') {
			for (i = 0; (val = dbg_msg[i].value); i++)
				if (stricmp(dbg_msg[i].string, s) == 0)
					break;
			if (!val)
				goto usage;
		}
		if (**argv == '-')
			msglevel_del |= val;
		else
			msglevel_add |= val;
		++argv;
	}

	msglevel &= ~msglevel_del;
	msglevel |= msglevel_add;

	return (dhd_iovar_setint(dhd, cmd->name, msglevel));

usage:
	fprintf(stderr, "msg values may be a list of numbers or names from the following set.\n");
	fprintf(stderr, "Use a + or - prefix to make an incremental change.");

	for (i = 0; (val = dbg_msg[i].value); i++) {
		if (val != last_val)
			fprintf(stderr, "\n0x%04x %s", val, dbg_msg[i].string);
		else
			fprintf(stderr, ", %s", dbg_msg[i].string);
		last_val = val;
	}
	fprintf(stderr, "\n");

	return 0;
}

static dbg_msg_t dhdlog_msgs[] = {
	{DHD_DNGL_IOVAR_SET_VAL, "s_iov"},
	{DHD_DNGL_IOVAR_GET_VAL, "g_iov"},
	{0,		NULL}
};

/* Get/set bitvectory for logging to memory. s_iov and g_iov are only supported now */
static int
dhd_loglevel(void *dhd, cmd_t *cmd, char **argv)
{
	return dhd_do_msglevel(dhd, cmd, argv, dhdlog_msgs);
}

static char *
ver2str(unsigned int vms, unsigned int vls)
{
	static char verstr[100];
	unsigned int maj, year, month, day, build;

	maj = (vms >> 16) & 0xFFFF;
	if (maj > 1000) {
		/* it is probably a date... */
		year = (vms >> 16) & 0xFFFF;
		month = vms & 0xFFFF;
		day = (vls >> 16) & 0xFFFF;
		build = vls & 0xFFFF;
		sprintf(verstr, "%d/%d/%d build %d",
			month, day, year, build);
	} else {
		/* it is a tagged release. */
		sprintf(verstr, "%d.%d RC%d.%d",
			(vms>>16)&0xFFFF, vms&0xFFFF,
			(vls>>16)&0xFFFF, vls&0xFFFF);
	}
	return verstr;
}

static int
dhd_version(void *dhd, cmd_t *cmd, char **argv)
{
	int ret;
	char *ptr;

	UNUSED_PARAMETER(cmd);
	UNUSED_PARAMETER(argv);

	/* Display the application version info */
	printf("%s: %s\n", dhdu_av0,
		ver2str((EPI_MAJOR_VERSION << 16)| EPI_MINOR_VERSION,
		(EPI_RC_NUMBER << 16) | EPI_INCREMENTAL_NUMBER));

	if ((ret = dhd_var_getbuf(dhd, cmd->name, NULL, 0, (void**)&ptr)) < 0)
		return ret;

	/* Display the returned string */
	printf("%s\n", ptr);

	return 0;
}

static int
dhd_var_setint(void *dhd, cmd_t *cmd, char **argv)
{
	int32 val;
	int len;
	char *varname;
	char *endptr = NULL;
	char *p;

	if (cmd->set == -1) {
		printf("set not defined for %s\n", cmd->name);
		return BCME_ERROR;
	}

	if (!*argv) {
		printf("set: missing arguments\n");
		return BCME_USAGE_ERROR;
	}

	varname = *argv++;

	if (!*argv) {
		printf("set: missing value argument for set of \"%s\"\n", varname);
		return BCME_USAGE_ERROR;
	}

	val = strtol(*argv, &endptr, 0);
	if (*endptr != '\0') {
		/* not all the value string was parsed by strtol */
		printf("set: error parsing value \"%s\" as an integer for set of \"%s\"\n",
			*argv, varname);
		return BCME_USAGE_ERROR;
	}

	strcpy(buf, varname);
	p = buf;
	while (*p != '\0') {
		*p = tolower(*p);
		p++;
	}

	/* skip the NUL */
	p++;

	memcpy(p, &val, sizeof(uint));
	len = (int)(p - buf) +  sizeof(uint);

	return (dhd_set(dhd, DHD_SET_VAR, &buf[0], len));
}

static int
dhd_var_get(void *dhd, cmd_t *cmd, char **argv)
{
	char *varname;
	char *p;

	UNUSED_PARAMETER(cmd);

	if (!*argv) {
		printf("get: missing arguments\n");
		return BCME_USAGE_ERROR;
	}

	varname = *argv++;

	if (*argv) {
		printf("get: error, extra arg \"%s\"\n", *argv);
		return BCME_USAGE_ERROR;
	}

	strcpy(buf, varname);
	p = buf;
	while (*p != '\0') {
		*p = tolower(*p);
		p++;
	}
	return (dhd_get(dhd, DHD_GET_VAR, &buf[0], DHD_IOCTL_MAXLEN));
}

static int
dhd_var_getint(void *dhd, cmd_t *cmd, char **argv)
{
	int err;
	int32 val;
	if (cmd->get == -1) {
		printf("get not defined for %s\n", cmd->name);
		return BCME_ERROR;
	}

	if ((err = dhd_var_get(dhd, cmd, argv)))
		return (err);

	val = *(int32*)buf;

	if (val < 10)
		printf("%d\n", val);
	else
		printf("%d (0x%x)\n", val, val);

	return (0);
}

static int
dhd_var_getandprintstr(void *dhd, cmd_t *cmd, char **argv)
{
	int err;

	if ((err = dhd_var_get(dhd, cmd, argv)))
		return (err);

	printf("%s\n", buf);
	return (0);
}

void
dhd_printlasterror(void *dhd)
{
	char *cmd[2] = {"bcmerrorstr"};

	if (dhd_var_get(dhd, NULL, cmd) != 0) {
		fprintf(stderr, "%s: \nError getting the last error\n", dhdu_av0);
	} else {
		fprintf(stderr, "%s: %s\n", dhdu_av0, buf);
	}
}

static int
dhd_varint(void *dhd, cmd_t *cmd, char *argv[])
{
	if (argv[1])
		return (dhd_var_setint(dhd, cmd, argv));
	else
		return (dhd_var_getint(dhd, cmd, argv));
}

static int
dhd_var_getbuf(void *dhd, char *iovar, void *param, int param_len, void **bufptr)
{
	int len;

	memset(buf, 0, DHD_IOCTL_MAXLEN);
	strcpy(buf, iovar);

	/* include the NUL */
	len = (int)strlen(iovar) + 1;

	if (param_len)
		memcpy(&buf[len], param, param_len);

	*bufptr = buf;

	return dhd_get(dhd, DHD_GET_VAR, &buf[0], DHD_IOCTL_MAXLEN);
}

static int
dhd_var_getbuf_dump(void *dhd, char *iovar, void *param, int param_len, void **bufptr)
{
	int len;

	memset(bufdump, 0, DHD_IOCTL_MAXLEN_HIGH);
	strcpy(bufdump, iovar);

	/* include the NUL */
	len = (int)strlen(iovar) + 1;

	if (param_len)
		memcpy(&bufdump[len], param, param_len);

	*bufptr = bufdump;

	return dhd_get(dhd, DHD_GET_VAR, &bufdump[0], DHD_IOCTL_MAXLEN_HIGH);
}

static int
dhd_var_setbuf(void *dhd, char *iovar, void *param, int param_len)
{
	int len;

	memset(buf, 0, DHD_IOCTL_MAXLEN);
	strcpy(buf, iovar);

	/* include the NUL */
	len = (int)strlen(iovar) + 1;

	if (param_len)
		memcpy(&buf[len], param, param_len);

	len += param_len;

	return dhd_set(dhd, DHD_SET_VAR, &buf[0], len);
}

static int
dhd_var_void(void *dhd, cmd_t *cmd, char **argv)
{
	UNUSED_PARAMETER(argv);

	if (cmd->set < 0)
		return BCME_ERROR;

	return dhd_var_setbuf(dhd, cmd->name, NULL, 0);
}

/*
 * format an iovar buffer
 */
static uint
dhd_iovar_mkbuf(char *name, char *data, uint datalen, char *buf, uint buflen, int *perr)
{
	uint len;

	len = (uint)strlen(name) + 1;

	/* check for overflow */
	if ((len + datalen) > buflen) {
		*perr = BCME_BUFTOOSHORT;
		return 0;
	}

	strcpy(buf, name);

	/* append data onto the end of the name string */
	if (datalen > 0)
		memcpy(&buf[len], data, datalen);

	len += datalen;

	*perr = 0;
	return len;
}

static int
dhd_iovar_getint(void *dhd, char *name, int *var)
{
	char ibuf[DHD_IOCTL_SMLEN];
	int error;

	dhd_iovar_mkbuf(name, NULL, 0, ibuf, sizeof(ibuf), &error);
	if (error)
		return error;

	if ((error = dhd_get(dhd, DHD_GET_VAR, &ibuf, sizeof(ibuf))) < 0)
		return error;

	memcpy(var, ibuf, sizeof(int));

	return 0;
}

static int
dhd_iovar_setint(void *dhd, char *name, int var)
{
	int len;
	char ibuf[DHD_IOCTL_SMLEN];
	int error;

	len = dhd_iovar_mkbuf(name, (char *)&var, sizeof(var), ibuf, sizeof(ibuf), &error);
	if (error)
		return error;

	if ((error = dhd_set(dhd, DHD_SET_VAR, &ibuf, len)) < 0)
		return error;

	return 0;
}

/*
* format a bsscfg indexed iovar buffer
*/
static int
dhd_bssiovar_mkbuf(const char *iovar, int bssidx, void *param,
	int paramlen, void *bufptr, int buflen, int *perr)
{
	const char *prefix = "bsscfg:";
	int8* p;
	uint prefixlen;
	uint namelen;
	uint iolen;

	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/* check for overflow */
	if (buflen < 0 || iolen > (uint)buflen) {
		*perr = BCME_BUFTOOSHORT;
		return 0;
	}

	p = (int8*)bufptr;

	/* copy prefix, no null */
	memcpy(p, prefix, prefixlen);
	p += prefixlen;

	/* copy iovar name including null */
	memcpy(p, iovar, namelen);
	p += namelen;

	/* bss config index as first param */
	bssidx = bssidx;
	memcpy(p, &bssidx, sizeof(int32));
	p += sizeof(int32);

	/* parameter buffer follows */
	if (paramlen) {
		memcpy(p, param, paramlen);
	}

	*perr = 0;
	return iolen;
}

/*
* set named & bss indexed driver iovar providing both parameter and i/o buffers
*/
int
dhdu_bssiovar_setbuf(void* dhd, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;

	iolen = dhd_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &err);
	if (err) {
		return err;
	}

	return dhd_set(dhd, DHD_SET_VAR, bufptr, iolen);
}

/*
* get named & bss indexed driver iovar providing both parameter and i/o buffers
*/
static int
dhd_bssiovar_getbuf(void* dhd, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;

	dhd_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &err);
	if (err) {
		return err;
	}

	return dhd_get(dhd, DHD_GET_VAR, bufptr, buflen);
}

/*
* get named & bss indexed driver variable to buffer value
*/
int
dhdu_bssiovar_get(void *dhd, const char *iovar, int bssidx, void *outbuf, int len)
{
	char smbuf[DHD_IOCTL_SMLEN] = {0};
	int err;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (len > (int)sizeof(smbuf)) {
		err = dhd_bssiovar_getbuf(dhd, iovar, bssidx, NULL, 0, outbuf, len);
	} else {
		err = dhd_bssiovar_getbuf(dhd, iovar, bssidx, NULL, 0, smbuf, sizeof(smbuf));
		if (err == 0) {
			memcpy(outbuf, smbuf, len);
		}
	}

	return err;
}

/*
* set named & bss indexed driver variable to buffer value
*/
static int
dhd_bssiovar_set(void *dhd, const char *iovar, int bssidx, void *param, int paramlen)
{
	char smbuf[DHD_IOCTL_SMLEN] = {0};

	return dhdu_bssiovar_setbuf(dhd, iovar, bssidx, param, paramlen, smbuf, sizeof(smbuf));
}

/*
* get named & bsscfg indexed driver variable as an int value
*/
static int
dhd_bssiovar_getint(void *dhd, const char *iovar, int bssidx, int *pval)
{
	int ret;

	ret = dhdu_bssiovar_get(dhd, iovar, bssidx, pval, sizeof(int));
	return ret;
}

/*
* set named & bsscfg indexed driver variable to int value
*/
static int
dhd_bssiovar_setint(void *dhd, const char *iovar, int bssidx, int val)
{
	return dhd_bssiovar_set(dhd, iovar, bssidx, &val, sizeof(int));
}

static int
dhd_varstr(void *dhd, cmd_t *cmd, char **argv)
{
	int error;
	char *str;
	char argstr[DHD_IOCTL_SMLEN];
	int i = 0;
	int argstrsize = DHD_IOCTL_SMLEN - 1;
	int argLen = 0;
	int argstrlen = 0;

	if (!*++argv) {
		void *ptr;

		if ((error = dhd_var_getbuf(dhd, cmd->name, NULL, 0, &ptr)) < 0)
			return (error);

		str = (char *)ptr;
		printf("%s\n", str);
		return (0);
	} else {
		while (argv[i] != NULL) {

			if (i > 0) {
				if (argstrsize < 1)
					return BCME_BADARG;

				/* Append a space between arguments */
				memset(argstr + argstrlen, 0x20, 1);
				argstrsize--;
				argstrlen++;
			}

			argLen = strlen(argv[i]);
			if (!(argLen < argstrsize))
				return BCME_BADARG;

			memcpy(argstr + argstrlen, argv[i], argLen);
			argstrsize -= argLen;
			argstrlen += argLen;

			i++;
		}

		/* Append the null terminator */
		memset(argstr + argstrlen, 0x0, 1);
		argstrsize--;
		argstrlen++;

		/* iovar buffer length includes NUL */
		return dhd_var_setbuf(dhd, cmd->name, argstr, argstrlen);
	}
}

static int
dhd_varstr_dump(void *dhd, cmd_t *cmd, char **argv)
{
	int error;
	char *str;

	if (!*++argv) {
		void *ptr;

		if ((error = dhd_var_getbuf_dump(dhd, cmd->name, NULL, 0, &ptr)) < 0)
			return (error);

		str = (char *)ptr;
		printf("%s\n", str);
		return (0);
	} else {
		str = *argv;
		/* iovar buffer length includes NUL */
		return dhd_var_setbuf(dhd, cmd->name, str, (int)strlen(str) + 1);
	}
}

static int
dhd_hostreorder_flows(void *dhd, cmd_t *cmd, char **argv)
{
	int ret, count, i = 0;
	void *ptr;
	uint8 *flow_id;

	if ((ret = dhd_var_getbuf(dhd, cmd->name, NULL, 0, &ptr)) < 0) {
		printf("error getting reorder flows from the host\n");
		return ret;
	}
	flow_id = (uint8 *)ptr;
	count = *flow_id;
	if (!count)
		printf("there are no active flows\n");
	else {
		printf("flows(%d): \t", count);
		while (i++ < count)
			printf("%d  ", *flow_id++);
		printf("\n");
	}
	return 0;
}

#ifdef BCMSPI
static int
dhd_spierrstats(void *dhd, cmd_t *cmd, char **argv)
{
	int ret, i;
	struct spierrstats_t spierrstats, *p;
	uint argc;
	void *ptr = NULL;

	bzero(&spierrstats, sizeof(spierrstats));

	/* arg count */
	for (argc = 0; argv[argc]; argc++);
	argc--;

	if (strcmp(cmd->name, "spi_errstats")) {
		printf("Supported only for SPI. Required cmd name missing\n");
		return BCME_USAGE_ERROR;
	}

	/* Check 3rd argument */
	argv++;

	/* no third arg means get, otherwise set */
	if (!*argv) {
		if ((ret = dhd_var_getbuf(dhd, cmd->name, NULL, 0, &ptr)) < 0)
			return ret;

		p = (struct spierrstats_t *)ptr;

		printf("Last %d device-status-bits\n", NUM_PREV_TRANSACTIONS);

		for (i = 0; i < NUM_PREV_TRANSACTIONS; i++)
			printf("dstatus[%d] = 0x%x, spicmd[%d] = 0x%x\n", i, p->dstatus[i],
			        i, p->spicmd[i]);

		printf("The requested data was not available  = %d\n",
			p->dna);
		printf("FIFO underflow happened due to F2 rd command = %d\n",
			p->rdunderflow);
		printf("FIFO overflow happened due to F1/F2 wr command = %d\n",
			p->wroverflow);
		printf("F2 interrupt (OR of all F2 related intr status bits) = %d\n",
			p->f2interrupt);
		printf("F2 FIFO is not ready to receive data = %d\n",
			p->f2rxnotready);
		printf("Error in command or host data, detected by checksum = %d\n",
			p->hostcmddataerr);
		printf("Packet is available in F2 TX FIFO = %d\n",
			p->f2pktavailable);

	} else {
		ret = dhd_var_setbuf(dhd, cmd->name, &spierrstats, sizeof(spierrstats));
	}

	return (ret);
}
#endif /* BCMSPI */

#ifdef WLBTAMP

#define MATCH_OP(op, opstr)	(strlen(op) == strlen(opstr) && strncmp(op, opstr, strlen(op)) == 0)

static int
wl_HCI_cmd(void *wl, cmd_t *cmd, char **argv)
{
	union {
		char buf[HCI_CMD_PREAMBLE_SIZE + HCI_CMD_DATA_SIZE];
		uint32 alignme;
	} cbuf;
	amp_hci_cmd_t *cpkt = (amp_hci_cmd_t *)&cbuf.buf[0];

	char *op;
	uint8 plen;

	UNUSED_PARAMETER(cmd);

	if (!*++argv)
		return BCME_USAGE_ERROR;

	/* recognize and encode operations */
	op = *argv++;
	if (MATCH_OP(op, "Read_Link_Quality")) {
		cpkt->opcode = HCI_Read_Link_Quality;
	} else if (MATCH_OP(op, "Read_Local_AMP_Info")) {
		cpkt->opcode = HCI_Read_Local_AMP_Info;
	} else if (MATCH_OP(op, "Read_Local_AMP_ASSOC")) {
		cpkt->opcode = HCI_Read_Local_AMP_ASSOC;
	} else if (MATCH_OP(op, "Write_Remote_AMP_ASSOC")) {
		cpkt->opcode = HCI_Write_Remote_AMP_ASSOC;
	} else if (MATCH_OP(op, "Create_Physical_Link")) {
		cpkt->opcode = HCI_Create_Physical_Link;
	} else if (MATCH_OP(op, "Accept_Physical_Link_Request")) {
		cpkt->opcode = HCI_Accept_Physical_Link_Request;
	} else if (MATCH_OP(op, "Disconnect_Physical_Link")) {
		cpkt->opcode = HCI_Disconnect_Physical_Link;
	} else if (MATCH_OP(op, "Create_Logical_Link")) {
		cpkt->opcode = HCI_Create_Logical_Link;
	} else if (MATCH_OP(op, "Accept_Logical_Link")) {
		cpkt->opcode = HCI_Accept_Logical_Link;
	} else if (MATCH_OP(op, "Disconnect_Logical_Link")) {
		cpkt->opcode = HCI_Disconnect_Logical_Link;
	} else if (MATCH_OP(op, "Logical_Link_Cancel")) {
		cpkt->opcode = HCI_Logical_Link_Cancel;
	} else if (MATCH_OP(op, "Short_Range_Mode")) {
		cpkt->opcode = HCI_Short_Range_Mode;
	} else if (MATCH_OP(op, "Read_Connection_Accept_Timeout")) {
		cpkt->opcode = HCI_Read_Connection_Accept_Timeout;
	} else if (MATCH_OP(op, "Write_Connection_Accept_Timeout")) {
		cpkt->opcode = HCI_Write_Connection_Accept_Timeout;
	} else if (MATCH_OP(op, "Read_Link_Supervision_Timeout")) {
		cpkt->opcode = HCI_Read_Link_Supervision_Timeout;
	} else if (MATCH_OP(op, "Write_Link_Supervision_Timeout")) {
		cpkt->opcode = HCI_Write_Link_Supervision_Timeout;
	} else if (MATCH_OP(op, "Reset")) {
		cpkt->opcode = HCI_Reset;
	} else if (MATCH_OP(op, "Enhanced_Flush")) {
		cpkt->opcode = HCI_Enhanced_Flush;
	} else if (MATCH_OP(op, "Read_Best_Effort_Flush_Timeout")) {
		cpkt->opcode = HCI_Read_Best_Effort_Flush_Timeout;
	} else if (MATCH_OP(op, "Write_Best_Effort_Flush_Timeout")) {
		cpkt->opcode = HCI_Write_Best_Effort_Flush_Timeout;
	} else if (MATCH_OP(op, "Read_Logical_Link_Accept_Timeout")) {
		cpkt->opcode = HCI_Read_Logical_Link_Accept_Timeout;
	} else if (MATCH_OP(op, "Write_Logical_Link_Accept_Timeout")) {
		cpkt->opcode = HCI_Write_Logical_Link_Accept_Timeout;
	} else if (MATCH_OP(op, "Read_Buffer_Size")) {
		cpkt->opcode = HCI_Read_Buffer_Size;
	} else if (MATCH_OP(op, "Read_Data_Block_Size")) {
		cpkt->opcode = HCI_Read_Data_Block_Size;
	} else if (MATCH_OP(op, "Set_Event_Mask_Page_2")) {
		cpkt->opcode = HCI_Set_Event_Mask_Page_2;
	} else if (MATCH_OP(op, "Flow_Spec_Modify")) {
		cpkt->opcode = HCI_Flow_Spec_Modify;
	} else if (MATCH_OP(op, "Read_Local_Version_Info")) {
		cpkt->opcode = HCI_Read_Local_Version_Info;
	} else if (MATCH_OP(op, "Read_Local_Supported_Commands")) {
		cpkt->opcode = HCI_Read_Local_Supported_Commands;
	} else if (MATCH_OP(op, "Read_Failed_Contact_Counter")) {
		cpkt->opcode = HCI_Read_Failed_Contact_Counter;
	} else if (MATCH_OP(op, "Reset_Failed_Contact_Counter")) {
		cpkt->opcode = HCI_Reset_Failed_Contact_Counter;
	} else {
		printf("unsupported HCI command: %s\n", op);
		return (-1);
	}

	plen = 0;
	while (*argv && (plen < HCI_CMD_DATA_SIZE)) {
		cpkt->parms[plen++] = (uint8)strtol(*argv++, NULL, 0);
	}
	cpkt->plen = plen;

	return dhd_var_setbuf(wl, cmd->name, cpkt, HCI_CMD_PREAMBLE_SIZE + plen);
}

typedef union {
	uint8 buf[HCI_ACL_DATA_PREAMBLE_SIZE + 2048];
	uint32 alignme;
} g_hci_dbuf_t;

static int
wl_HCI_ACL_data(void *wl, cmd_t *cmd, char **argv)
{
	/* Align struct. Also declare static so that large array isn't allocated
	 * from the stack.
	 */
	static g_hci_dbuf_t g_hci_dbuf;

	amp_hci_ACL_data_t *dpkt = (amp_hci_ACL_data_t *)&g_hci_dbuf.buf[0];
	uint16 dlen;

	if (!*++argv)
		return BCME_USAGE_ERROR;

	/* get logical link handle */
	dpkt->handle = (HCI_ACL_DATA_BC_FLAGS | HCI_ACL_DATA_PB_FLAGS);
	dpkt->handle |= (uint16)strtol(*argv++, NULL, 0);

	/* get data */
	dlen = 0;
	while (*argv && (dlen < 2048)) {
		dpkt->data[dlen++] = (uint8)strtol(*argv++, NULL, 0);
	}
	dpkt->dlen = dlen;

	return dhd_var_setbuf(wl, cmd->name, dpkt, HCI_ACL_DATA_PREAMBLE_SIZE + dlen);
}
#endif /* WLBTAMP */

static int
dhd_almac(void *dhd, cmd_t *cmd, char **argv)
{
	char *iovar;
	uint8 *ea[ETHER_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int bssidx = 0;
	int ret = BCME_OK;

	iovar = *argv++;
	memset(buf, 0, DHD_IOCTL_SMLEN);

	if (GET_BSSCFG_IDX(dhd, &bssidx) != BCME_OK) {
		bssidx = 0;
	}

	if (!*argv) {
		ret = dhdu_bssiovar_get(dhd, iovar, bssidx, buf, DHD_IOCTL_SMLEN);
		if (ret >= 0) {
			printf("%s: %s\n", cmd->name,
				dhd_ether_etoa((const struct ether_addr *)buf));
		}
		return ret;
	} else {
		if (strncmp(*argv, "clear", strlen("clear")) &&
			!dhd_ether_atoe(*argv, (struct ether_addr *)ea)) {
			printf("Invalid MAC address\n");
			return BCME_USAGE_ERROR;
		}
		return dhd_bssiovar_set(dhd, iovar, bssidx, ea, ETHER_ADDR_LEN);
	}
}

/* These two utility functions are used by dhdu_linux.c
 * The code is taken from wlu.c.
 */
int
dhd_atoip(const char *a, struct ipv4_addr *n)
{
	char *c;
	int i = 0;

	for (;;) {
	        n->addr[i++] = (uint8)strtoul(a, &c, 0);
	        if (*c++ != '.' || i == IPV4_ADDR_LEN)
	                break;
	        a = c;
	}
	return (i == IPV4_ADDR_LEN);
}

int
dhd_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c;
	int i = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
	        n->octet[i++] = (uint8)strtoul(a, &c, 16);
	        if (!*c++ || i == ETHER_ADDR_LEN)
	                break;
	        a = c;
	}
	return (i == ETHER_ADDR_LEN);
}

static char *
dhd_ether_etoa(const struct ether_addr *n)
{
	static char etoa_buf[ETHER_ADDR_LEN * 3];
	char *c = etoa_buf;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", n->octet[i] & 0xff);
	}
	return etoa_buf;
}

static int
dhd_hmembytes(void *dhd, cmd_t *cmd, char **argv)
{
	char *ptr;
	int ret = -1, align;
	uint argc, len, chunk, params[3];
	uint64 addr;

	argv++;

	/* arg count */
	for (argc = 0; argv[argc]; argc++);

	/* required args: address size */
	if (argc != 2) {
		fprintf(stderr, "required args: address size\n");
		return BCME_USAGE_ERROR;
	}

	/* read address */
	addr = bcm_strtoull(argv[0], &ptr, 0);
	if (*ptr) {
		fprintf(stderr, "Bad arg: %s\n", argv[0]);
		return BCME_USAGE_ERROR;
	}

	/* read size */
	len = strtoul(argv[1], &ptr, 0);
	if (*ptr) {
		fprintf(stderr, "Bad value: %s\n", argv[1]);
		return BCME_USAGE_ERROR;
	}

	align = addr & 0x03;

	/* Read host memory */
	chunk = DHD_IOCTL_MAXLEN;
	for (addr -= align, len += align; len; addr += chunk, len -= chunk, align = 0) {
		chunk = MIN(chunk, len);
		params[0] = (uint)(addr >> 32);
		params[1] = (uint)addr;
		params[2] = ROUNDUP(chunk, 4);
		ret = dhd_var_getbuf(dhd, "hmembytes", params, (3 * sizeof(uint)), (void**)&ptr);
		if (ret < 0)
			goto exit;

		dhd_hexdump((uchar*)ptr + align, chunk - align, addr + align);
	}

exit:
	return ret;
}

#define CORE_NUM_STR_LEN	3
#define CORE_STR_ARGV_IDX	1

enum asm_core_type {
	NA = 0,
	CPUSUB = 1,
	DIG = 2,
	PHY1PHY2 = 3,
	PHYRX1 = 4,
	PHYRX2 = 5,
	PHYRX4 = 6,
	D11MAC1 = 7,
	D11MAC2 = 8,
	CHIP_ALL = 9,
	CHIP_WO_DIG = 10,
	CHIP_MAC_PHY = 11,
	LAST
};

char *asm_core_type_str[] = {
	"NA", "cpusub", "dig", "phy12",
	"rx1", "rx2", "rx4", "d11mac1",
	"d11mac2", "chip_all", "chip_wo_dig",
	"chip_mac_phy"
};

static int
dhd_asm_to_num_str(char **argv, char *str, int str_len)
{
	int i, len;

	len = strlen(argv[CORE_STR_ARGV_IDX]);
	for (i = 0; i < LAST; i++) {
		if (!strncmp(argv[CORE_STR_ARGV_IDX], asm_core_type_str[i], len)) {
			break;
		}
	}

	if (i == 0 || i == LAST) {
		return BCME_USAGE_ERROR;
	}

	fprintf(stderr, "select asm core str : %s\n", argv[CORE_STR_ARGV_IDX]);
	snprintf(str, str_len, "%d", i);
	str[str_len - 1] = '\0';
	return BCME_OK;
}

static int
dhd_load_asm(void *dhd, cmd_t *cmd, char **argv)
{
	int argc, ret, cur_core;
	char core[CORE_NUM_STR_LEN] = {0, };

	for (argc = 0; argv[argc]; argc++);

	argc--;

	if (argc < 1) {
		ret = dhd_iovar_getint(dhd, cmd->name, &cur_core);
		if (ret != BCME_OK) {
			fprintf(stderr, "failed to get current core, ret = %d\n", ret);
			return ret;
		} else {
			if (cur_core < NA || cur_core >= LAST) {
				fprintf(stderr, "invalid current core = %d\n", cur_core);
				return BCME_ERROR;
			} else {
				fprintf(stderr, "current asm core type %s\n",
					asm_core_type_str[cur_core]);
				return BCME_OK;
			}
		}
	}

	ret = dhd_asm_to_num_str(argv, core, CORE_NUM_STR_LEN);
	if (ret != BCME_OK) {
		fprintf(stderr, "invalid core type %s\n", argv[CORE_STR_ARGV_IDX]);
		return ret;
	}

	strlcpy(argv[1], core, CORE_NUM_STR_LEN);
	fprintf(stderr, "select asm core num = %s\n", argv[CORE_STR_ARGV_IDX]);
	return dhd_var_setint(dhd, cmd, argv);
}

static int
dhd_fis_dump(void *dhd, cmd_t *cmd, char **argv)
{
	int argc;

	for (argc = 0; argv[argc]; argc++);
	argc--;

	/* no argument */
	if (argc < 1) {
		printf("asm isn't included in dump\n");
		return BCME_USAGE_ERROR;
	}

	return dhd_var_setint(dhd, cmd, argv);
}

static int
dhd_rngcsi_dump(void *dhd, cmd_t *cmd, char **argv)
{
	int argc, is_rsta = 0;

	for (argc = 0; argv[argc]; argc++);
	argc--;

	/* no argument */
	if (argc < 1) {
		printf("Missing argument\n");
		return BCME_USAGE_ERROR;
	}

	argv++;

	if (!stricmp(*argv, "r")) {
		is_rsta = 1;
	} else if (!stricmp(*argv, "i")) {
		is_rsta = 0;
	} else {
		printf("Invalid argument: %s\n", *argv);
		return BCME_USAGE_ERROR;
	}

	return dhd_iovar_setint(dhd, cmd->name, is_rsta);
}
