#ifndef __ES_H__
#define __ES_H__

#include "ate.h"
#include "es_lib.h"

#define ESD_PID_FILE "/var/run/esd.pid"

extern int esd_main(int argc, char *argv[]);
extern int esr_main(int argc, char *argv[]);
extern void start_esd();
extern void stop_esd();
extern void esd_data_thread();
#ifdef RTCONFIG_CC3220
extern void esd_halt_thread(int *ctrl);
extern int es_init();
extern int es_halt_prepare();
extern void es_halt_action();
extern int esrt_main(int argc, char *argv[]);
extern int ti_get_fwver(char *fwver, size_t len);
extern int ti_get_mac_2g(char *mac, size_t len);
extern int ti_set_led_on(enum ate_led_color color);
extern int ti_set_led_off(enum ate_led_color color);
extern int ti_check_alive();
extern void ti_reset();
extern int ti_test_tx(int channel, int rate_index, int power_index);
extern int ti_test_ap(const char* ssid, int channel, int power_index);
#endif

#endif