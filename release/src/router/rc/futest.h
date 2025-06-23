#ifndef __FUTEST_TESTH__
#define __FUTEST_TESTH__

#include <stdio.h>
#include <json.h>

#define LIB_GET_MEM_INFO "get_mem_info"
typedef unsigned long (*GET_MEM_INFO) (char *name);
typedef int (*TC_FUNCTION)(char* func_name, int argc, char *argv[], char *outmsg);

#define DECLARE_CLEAR_MEM(type, var, len) \
    type var[len]; \
    memset(var, 0, len );

#define CLEAR_MEM(var, len) \
    memset(var, 0, len );

enum {
	TC_NONE = 0,

	/* sysdep function test */
	TC_SYSDEP_FUNC = 0x1,

	TC_GET_HW_ACCELERATION = TC_SYSDEP_FUNC+1,
	TC_GET_SYS_CLK = TC_SYSDEP_FUNC+2,
	TC_GET_SYS_TEMP = TC_SYSDEP_FUNC+3,
	TC_GET_WIFI_CHIP = TC_SYSDEP_FUNC+4,
	TC_GET_WIFI_TEMP = TC_SYSDEP_FUNC+5,
	TC_GET_WIFI_COUNTRY = TC_SYSDEP_FUNC+6,
	TC_GET_WIFI_NOISE = TC_SYSDEP_FUNC+7,
	TC_GET_WIFI_MCS = TC_SYSDEP_FUNC+8,
	TC_GET_WIFI_STATUS = TC_SYSDEP_FUNC+9,

	TC_GET_PHY_INFO = TC_SYSDEP_FUNC+10,
	TC_GET_PORT_STATUS = TC_SYSDEP_FUNC+11,

	/* conndiag library function test */
	TC_CONNDIAG_LIB = 0x100,

	TC_GET_MEM_INFO = TC_CONNDIAG_LIB+1,
	
	/* process test */
	TC_PROCESS = 0x200,

	TC_TUNNEL_ENABLE = TC_PROCESS+1,
	TC_TUNNEL_TEST = TC_PROCESS+2,
	TC_REFRESH_USERTICKET = TC_PROCESS+3,
	TC_REFRESH_DEVICETICKET = TC_PROCESS+4,

	TC_11K_REQ = TC_PROCESS+11,
	TC_11V_REQ = TC_PROCESS+12,

	TC_UPLOADER_LIST_FILES = TC_PROCESS+21,

	TC_NETWORKMAP_SCAN = TC_PROCESS+31,
	TC_NETWORKMAP_GET_CLIENT_LIST = TC_PROCESS+32,

	TC_MAX
};

typedef enum __testcase_type_t_ {
	TYPE_NONE = 0,
	TYPE_CONNDIAG_LIB = 1,
	TYPE_SYSDEP_FUNC,
	TYPE_PROCESS
} TESTCASE_TYPE_T;

typedef struct __testcase_t_ {
	int id;
	int type;
    char* name;
    char* description;
} TESTCASE;

// #define GET_HW_ACCELERATION "get_hw_acceleration"
// #define GET_SYS_CLK "get_sys_clk"
// #define GET_SYS_TEMP "get_sys_temp"
// #define GET_WIFI_CHIP "get_wifi_chip"

TESTCASE CHK_TESTCASE[] = {
    { TC_GET_MEM_INFO,         		   TYPE_CONNDIAG_LIB, "get_mem_info",         		   "Test get_mem_info()" },

    { TC_GET_HW_ACCELERATION,  		   TYPE_SYSDEP_FUNC,  "get_hw_acceleration",  		   "Test get_hw_acceleration()" },
    { TC_GET_SYS_CLK,          		   TYPE_SYSDEP_FUNC,  "get_sys_clk",          		   "Test get_sys_clk()" },
    { TC_GET_SYS_TEMP,         		   TYPE_SYSDEP_FUNC,  "get_sys_temp",         		   "Test get_sys_temp()" },
    { TC_GET_WIFI_CHIP,        		   TYPE_SYSDEP_FUNC,  "get_wifi_chip",        		   "Test get_wifi_chip(), params ifname" },
    { TC_GET_WIFI_TEMP,        		   TYPE_SYSDEP_FUNC,  "get_wifi_temp",        		   "Test get_wifi_temp(), params ifname" },
    { TC_GET_WIFI_COUNTRY,     		   TYPE_SYSDEP_FUNC,  "get_wifi_country",     		   "Test get_wifi_country(), params ifname" },
    { TC_GET_WIFI_NOISE,       		   TYPE_SYSDEP_FUNC,  "get_wifi_noise",       		   "Test get_wifi_noise(), params ifname" },
    { TC_GET_WIFI_MCS,         		   TYPE_SYSDEP_FUNC,  "get_wifi_mcs",         		   "Test get_wifi_mcs(), params ifname" },
    { TC_GET_WIFI_STATUS,         	   TYPE_SYSDEP_FUNC,  "get_wifi_status",         	   "Test get_wifi_status(), params ifname" },

    { TC_GET_PHY_INFO,         TYPE_SYSDEP_FUNC,  "get_phy_info",         "Test get_phy_info()" },
    { TC_GET_PORT_STATUS,      TYPE_SYSDEP_FUNC,  "get_port_status",      "Test get_port_status()" },
    
    { TC_TUNNEL_ENABLE,        TYPE_PROCESS,      "tunnel_enable",        "Enable tunnel" },
    { TC_TUNNEL_TEST,          TYPE_PROCESS,      "tunnel_test",          "Test tunnel" },
    { TC_REFRESH_USERTICKET,   TYPE_PROCESS,      "refresh_userticket",   "Update user ticket by refresh ticket API" },
    { TC_REFRESH_DEVICETICKET, TYPE_PROCESS,      "refresh_deviceticket", "Test DM login API" },
    
    { TC_11K_REQ, TYPE_PROCESS,      "11k_req", "Send 11k request to specified client" },
    { TC_11V_REQ, TYPE_PROCESS,      "11v_req", "Send 11v request to specified client" },

    { TC_UPLOADER_LIST_FILES, TYPE_PROCESS,      "uploader_list_files", "list all files" },

    { TC_NETWORKMAP_SCAN, TYPE_PROCESS,      "networkmap_scan", "run networkmap scan" },
    { TC_NETWORKMAP_GET_CLIENT_LIST, TYPE_PROCESS,    "networkmap_get_client_list", "Get client list"},

    { -1, -1, NULL, NULL }
};

static char* do_make_str(const char *fmt, ...);
static void free_test_result_data(char* test_result_data);
#define get_test_result_data(...) do_make_str(__VA_ARGS__)

const char* test_json_result_template = 
"{ \"id\": %d,"
" \"name\": \"%s\","
" \"res\": %d,"
" \"output\": %s"
"}"
;

const char* test_int_result_template = 
"{ \"id\": %d,"
" \"name\": \"%s\","
" \"res\": %d,"
" \"output\": %d"
"}"
;

const char* test_result_template = 
"{ \"id\": %d,"
" \"name\": \"%s\","
" \"res\": %d,"
" \"output\": \"%s\""
"}"
;

//- conndiag sysdep function
extern int get_hw_acceleration(char *output, int size);
extern int get_sys_clk(char *output, int size);
extern int get_sys_temp(unsigned int *temp);
extern int get_wifi_chip(char *ifname, char *output, int size);

extern int get_wifi_temp(char *ifname, unsigned int *temp);
extern int get_wifi_country(char *ifname, char *output, int len);
extern int get_wifi_noise(char *ifname, char *output, int len);
extern int get_wifi_mcs(char *ifname, char *output, int len);

extern int get_wifi_status(char *ifname, char *output, int len);

#ifdef RTCONFIG_BCMARM
extern int get_bss_info(char *ifname, int *capability); // TODO: non-Brcm needs to do.
extern int get_subif_count(char *target, int *count); // TODO: non-Brcm needs to do.
extern int get_subif_ssid(char *target, char *output, int outputlen); // TODO: non-Brcm needs to do.
extern int get_wifi_txop(char *ifname,int *txop, int outputlen);
extern int get_wifi_glitch(char *ifname,int *glitch, int outputlen);
extern int get_wifi_chanim(char *ifname, char *output, int outputlen);
extern int get_wifi_counters_info(char *ifname, char *info_name, int *value);
#endif
extern int get_wifi_dfs_status(char *output, int len, char *node, char *lan_ipaddr, char *lan_hwaddr);

extern char *diag_get_wifi_fh_ifnames(int wifi_unit, char *buffer, size_t buffer_size);
extern char *diag_get_eth_bh_ifnames(char *buffer, size_t buffer_size);
extern char *diag_get_eth_fh_ifnames(char *buffer, size_t buffer_size);
extern int diag_is_wlif(char *ifname);
extern int diag_is_uplink_ethif(char *ifname);
extern int get_plc_phy_rate(unsigned long *tx_rate, unsigned long *rx_rate);

// extern uint8 rast_get_rclass(int bssidx, int vifidx);

#endif /* __FUTEST_TESTH__ */
