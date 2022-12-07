#ifndef _plc_utils_h_
#define _plc_utils_h_

/*
 * The number of bytes in an PLC Key.
 */
#ifndef PLC_KEY_LEN
#define	PLC_KEY_LEN		16
#endif

extern int key_atoe(const char *a, unsigned char *e);
extern char *key_etoa(const unsigned char *e, char *a);

extern int getPLC_MAC(char *abuf);
extern int getPLC_NMK(char *abuf);
extern int getPLC_PWD(void);
extern char *get_plc_ifname(char ifname[]);
extern int is_plc_ifname(char *ifname);
extern int current_nmk(char *nmk);
#ifdef PLAX56_XP4
#define PLC_INTERFACE	"eth1"
#endif

#ifdef RTCONFIG_QCA_PLC2
#define BOOT_NVM_PATH		"/lib/firmware/plc/fw.nvm"
#define BOOT_PIB_PATH		"/tmp/plc.pib"
#define set_plc_flag(a) while(0){}
extern int load_plc_setting(void);
extern void turn_led_pwr_off(void);	//for set "plc_ready"
#else
extern int getPLC_para(int addr);
extern int setPLC_para(const char *abuf, int addr);
extern void ate_ctl_plc_led(void);
extern int set_plc_all_led_onoff(int on);

#define PLC_LOCK_FILE		"/tmp/plc_lock"
#if defined(PLN12)
#define BOOT_NVM_PATH		"/tmp/asus.nvm"
#else
#define BOOT_NVM_PATH		"/usr/local/bin/asus.nvm"
#endif
#define BOOT_PIB_PATH		"/tmp/asus.pib"

extern int default_plc_write_to_flash(void);
extern void set_plc_flag(int flag);
extern int load_plc_setting(void);
extern void save_plc_setting(void);

extern void turn_led_pwr_off(void);

#endif	/* PLC_UTILS */
struct remote_plc {
	char mac[18];
	char pwd[20];
	int status;	/*
			1: connect and known password
			2: disconnect and known password
			3: connect and unknown password
			*/
	int tx;
	int rx;
	int tx_mimo;
	int rx_mimo;
};

extern int apply_private_name(char *pnn, char *nv);
extern int get_connected_plc(struct remote_plc **rplc);
extern int get_known_plc(struct remote_plc **rplc);
extern int trigger_plc_pair(void);
extern int add_remote_plc(char *mac, char *pwd);
extern int chk_plc_alive(void);
extern void do_plc_reset(int force);
extern int plctool_get(const char *cmd, char buf[], int size, const char *chk_str);
extern int plc_wait_busy(void);
/* 
 * int do_plc_pushbutton(int pb_act)
 *
 * pb_act:
 * 	1: join
 * 	2: leave
 * 	3: status
 * 	4: reset
 * 	5: stop
 * 	6: start/extend
 * 	7: pbstat
 */
extern int do_plc_pushbutton(int pb_act);

/* 
	get pbstat

0x001B 1 Current Push Button state 
=0x00 = Idle
=0x01 = In Process
=0x02 = Adder
=0x03 = Joiner
=0x04 = Complete
=0x05 = Timeout
=0x06 = SCError

*/

extern int get_plc_pb_state(void);
#define IS_PLC_JOIN_STOPPED(pb_state) (pb_state == 0 || pb_state == 4 || pb_state == 5 || pb_state == 6)

#endif /* _plc_utils_h_ */
