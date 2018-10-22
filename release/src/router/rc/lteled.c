
#include <stdio.h>	     
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include <rtconfig.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>
#include <rc.h>
#include <at_cmd.h>


static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	struct itimerval itv;
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void catch_sig(int sig)
{
	//_dprintf("[lteled] sig(%d)\n", sig);
}

enum {
	STATE_SIM_NOT_READY = 0,
	STATE_CONNECTING,
	STATE_CONNECTED,
};

#define CHK_LTE_COUNT		(30)
#define SET_LONG_PERIOD()	{ alarmtimer(1, 0);        cnt = CHK_LTE_COUNT/10; long_period = 1; }
#define SET_SHORT_PERIOD()	{ alarmtimer(0, 100*1000); cnt = CHK_LTE_COUNT   ; long_period = 0; }
#define NEED_LONG_PERIOD	(state != STATE_CONNECTING && lighting_time == 0)

int lteled_main(int argc, char **argv)
{
	int percent = 0, old_percent = -100;
	int modem_signal = 0;
	int cnt = 0;
	int state = -1;
	int lighting_time = 0;
#ifdef RT4GAC55U
	int lighting_cnt = 0;
#endif
	int long_period = 1;
	int old_state = state;
	int usb_unit;
	char tmp2[100], prefix2[32];
	int modem_unit = -1;
	int sim_state;

	if(argc == 2)
		modem_unit = atoi(argv[1]);

	if(modem_unit < 0)
		modem_unit = MODEM_UNIT_FIRST;

	signal(SIGALRM, catch_sig);

	usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

	nvram_set_int(strcat_r(prefix2, "act_signal", tmp2), modem_signal);

	if((usb_unit = get_wanunit_by_type(get_wantype_by_modemunit(modem_unit))) == WAN_UNIT_NONE){
		_dprintf("lteled: in the current dual wan mode, didn't support the USB modem.\n");
		return 0;
	}

	while (1)
	{
		if (nvram_match("asus_mfg", "1"))
		{
			pause();
			continue;
		}

		if(strcmp(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)), "gobi")){
			SET_LONG_PERIOD();
			pause();
			continue;
		}

#if defined(RTCONFIG_WPS_ALLLED_BTN)
		if (nvram_match("AllLED", "0")) {
			state = -1;
			percent = 0;
			old_percent = -100;
			continue;
		}
#endif

		if (lighting_time == 0 && --cnt <= 0)
		{ //every 3 seconds
			old_state = state;

			sim_state = nvram_get_int(strcat_r(prefix2, "act_sim", tmp2));
			if(state != STATE_CONNECTED && sim_state != 1 && sim_state != 2 && sim_state != 3)
			{ //Sim Card not ready
				if(state != STATE_SIM_NOT_READY)
				{
					state = STATE_SIM_NOT_READY;
					percent = 0;
					old_percent = -100;
#if defined(RT4GAC53U)
					led_control(LED_LTE_OFF, LED_ON);
#else
					led_control(LED_LTE, LED_OFF);
#endif
					led_control(LED_SIG1, LED_OFF);
					led_control(LED_SIG2, LED_OFF);
					led_control(LED_SIG3, LED_OFF);
#if defined(RT4GAC53U)
					led_control(LED_SIG4, LED_OFF);
#elif defined(RT4GAC68U)
					led_control(LED_3G, LED_OFF);
#endif
				}
			}
			else if(!is_wan_connect(usb_unit)
					|| (percent = nvram_get_int(strcat_r(prefix2, "act_signal", tmp2))*25) < 0
					)
			{ //Not connected
				if(state != STATE_CONNECTING)
				{
					state = STATE_CONNECTING;
					percent = 0;
					old_percent = -100;
					led_control(LED_SIG1, LED_OFF);
					led_control(LED_SIG2, LED_OFF);
					led_control(LED_SIG3, LED_OFF);
#if defined(RT4GAC53U)
					led_control(LED_SIG4, LED_OFF);
					led_control(LED_LTE_OFF, LED_OFF);
#endif
				}
			}
			else
			{ //connect and has signal strength
#ifdef RT4GAC68U
				if(!strcmp(nvram_safe_get(strcat_r(prefix2, "act_operation", tmp2)), "LTE")){
					led_control(LED_3G, LED_OFF);
					led_control(LED_LTE, LED_ON);
				}
				else{
					led_control(LED_3G, LED_ON);
					led_control(LED_LTE, LED_ON);
				}
#endif

				if (state != STATE_CONNECTED)
				{
					state = STATE_CONNECTED;
#if defined(RT4GAC53U)
					led_control(LED_LTE_OFF, LED_OFF);
#elif defined(RT4GAC68U)
#else /* 4G-AC55U */
					led_control(LED_LTE, LED_ON);
#endif
				}
#if defined(RT4GAC53U)
				if ((percent/20) != (old_percent/20))
				{
					led_control(LED_SIG1, (percent >= 20)? LED_ON : LED_OFF);
					led_control(LED_SIG2, (percent >= 40)? LED_ON : LED_OFF);
					led_control(LED_SIG3, (percent >= 60)? LED_ON : LED_OFF);
					led_control(LED_SIG4, (percent >= 80)? LED_ON : LED_OFF);
					old_percent = percent;
				}
#else /* 4G-AC55U || 4G-AC68U */
				if ((percent/25) != (old_percent/25))
				{
					led_control(LED_SIG1, (percent > 0)? LED_ON : LED_OFF);
					led_control(LED_SIG2, (percent > 25)? LED_ON : LED_OFF);
					led_control(LED_SIG3, (percent > 75)? LED_ON : LED_OFF);
					old_percent = percent;
				}
#endif
			}

			if (old_state != state)
				cprintf("%s: state(%d --> %d)\n", __func__, old_state, state);

			if(NEED_LONG_PERIOD)
			{
				SET_LONG_PERIOD();
			}
			else
			{
				SET_SHORT_PERIOD();
			}
		}

#ifdef RT4GAC55U
		if (long_period || (cnt % 10) == 0)
		{ //every second
			if (button_pressed(BTN_LTE))
			{
				lighting_time = 3*10;
				SET_SHORT_PERIOD();
			}
		}
#endif

		if (!long_period)
		{
#ifdef RT4GAC55U
			if (lighting_time > 0)			//handle BTN_LTE
			{
				void led_control_lte(int percent);

				lighting_time--;
				if (lighting_time == 0)
				{
					led_control_lte(-1);
					lighting_cnt  = 0;
				}
				else if (lighting_cnt < 10)
				{
					led_control_lte((++lighting_cnt)*9+1);
				}
				else if (state != STATE_CONNECTED)
				{
					led_control_lte(0);
				}
				else
				{
					led_control_lte(percent);
				}
			}
			else
#endif
			if (state == STATE_CONNECTING)	//handle lte led blink
			{
#ifdef RT4GAC68U
				if(!strcmp(nvram_safe_get(strcat_r(prefix2, "act_operation", tmp2)), "LTE")){
					led_control(LED_3G, LED_OFF);
					led_control(LED_LTE, ((cnt % 5) < 3)? LED_ON : LED_OFF);
				}
				else{
					led_control(LED_3G, ((cnt % 5) < 3)? LED_ON : LED_OFF);
					led_control(LED_LTE, ((cnt % 5) < 3)? LED_ON : LED_OFF);
				}
#elif defined(RT4GAC53U)
				switch (cnt % 5) {
				case 3:
					led_control(LED_SIG1, LED_ON);
					led_control(LED_SIG2, LED_OFF);
					led_control(LED_SIG3, LED_OFF);
					led_control(LED_SIG4, LED_OFF);
					break;
				case 2:
					led_control(LED_SIG1, LED_OFF);
					led_control(LED_SIG2, LED_ON);
					led_control(LED_SIG3, LED_OFF);
					led_control(LED_SIG4, LED_OFF);
					break;
				case 1:
					led_control(LED_SIG1, LED_OFF);
					led_control(LED_SIG2, LED_OFF);
					led_control(LED_SIG3, LED_ON);
					led_control(LED_SIG4, LED_OFF);
					break;
				case 0:
					led_control(LED_SIG1, LED_OFF);
					led_control(LED_SIG2, LED_OFF);
					led_control(LED_SIG3, LED_OFF);
					led_control(LED_SIG4, LED_ON);
					break;
				default:
					led_control(LED_SIG1, LED_OFF);
					led_control(LED_SIG2, LED_OFF);
					led_control(LED_SIG3, LED_OFF);
					led_control(LED_SIG4, LED_OFF);
				}
#else /* 4G-AC55U */
				led_control(LED_LTE, ((cnt % 5) < 3)? LED_ON : LED_OFF);
#endif
			}
		}

		pause();
	}
	return 0;
}

