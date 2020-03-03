
#include "rc.h"
#include "mastiff.h"
#ifdef RTCONFIG_TUNNEL
static int is_mesh_re_mode()
{
	int re_mode = 0;
#if defined(RTCONFIG_AMAS) // aimesh
	re_mode |= nvram_get_int("re_mode");
#elif defined(RTCONFIG_WIFI_SON) // Lyra
	re_mode |= !nvram_get_int("cfg_master");
#endif
	return re_mode;
}

void start_aae()
{
	if (is_mesh_re_mode())
		return;

	if(nvram_get_int("aae_disable_force"))
		return;

	if( getpid()!=1 ) {
		notify_rc("start_aae");
		return;
	}

	if ( !pids("aaews" )){
		// add enable
		//nvram_set_int("aae_enable", (nvram_get_int("aae_enable") | 1));
		system("aaews &");
		logmessage("AAE", "AAE Service is started");
	}
}

void stop_aae()
{
	if( getpid()!=1 ) {
		notify_rc("stop_aae");
		return;
	}
	
	// remove enable
	nvram_set_int("aae_enable", (nvram_get_int("aae_enable") & ~1));
	killall_tk("aaews");
	logmessage("NAT Tunnel", "AAE Service is stopped");
}

void start_aae_sip_conn(int sdk_init)
{
#define WAIT_TIMEOUT 5
	int time_count = 0;
	if (pids("aaews")) {
		if (sdk_init)
			nvram_set_int("aae_action", AAEWS_ACTION_SDK_INIT);
		else
			nvram_set_int("aae_action", AAEWS_ACTION_SIP_REGISTER);
		killall("aaews", AAEWS_SIG_ACTION);
		while(time_count < WAIT_TIMEOUT && nvram_invmatch("aae_sip_connected", "1")) {
			sleep(1);
			//_dprintf("%s: wait sip register...\n", __FUNCTION__);
			time_count++;
		}
	}
}

void stop_aae_sip_conn(int sdk_deinit)
{
#define WAIT_TIMEOUT 5
	int time_count = 0;
	if (pids("aaews")) {
		if (sdk_deinit)
			nvram_set_int("aae_action", AAEWS_ACTION_SDK_DEINIT);
		else
			nvram_set_int("aae_action", AAEWS_ACTION_SIP_UNREGISTER);
		killall("aaews", AAEWS_SIG_ACTION);
		while(time_count < WAIT_TIMEOUT && nvram_match("aae_sip_connected", "1")) {
			sleep(1);
			//_dprintf("%s: wait sip unregister...\n", __FUNCTION__);
			time_count++;
		}
	}
}

void stop_aae_gently()
{
#define WAIT_TIMEOUT 5
	int time_count = 0;
	if (pids("aaews")) {
		nvram_set_int("aae_action", AAEWS_ACTION_SIP_UNREGISTER);
		killall("aaews", AAEWS_SIG_ACTION);
		while(time_count < WAIT_TIMEOUT && nvram_match("aae_sip_connected", "1")) {
			sleep(1);
			//_dprintf("%s: wait sip unregister...\n", __FUNCTION__);
			time_count++;
		}

		time_count = 0;
		killall("aaews", SIGTERM);
		while(time_count < WAIT_TIMEOUT && pids("aaews")) {
			sleep(1);
			//_dprintf("%s: wait aaews end...\n", __FUNCTION__);
			time_count++;
		}
	}
}

void start_mastiff()
{
#ifdef CONFIG_BCMWL5
#ifndef RTCONFIG_BCM_MFG
	if (factory_debug())
#endif
#else
	if (IS_ATE_FACTORY_MODE())
#endif
	return;

	if (is_mesh_re_mode())
		return;

	if(nvram_get_int("aae_disable_force"))
		return;

	stop_aae();
	
	if( getpid()!=1 ) {
		notify_rc("start_mastiff");
		return;
	}

	if ( !pids("mastiff" )){
		system("mastiff &");
		logmessage("AAE", "AAE Service is started");
		//start_aae();
	}

}

void stop_mastiff()
{
	if( getpid()!=1 ) {
		notify_rc("stop_mastiff");
		return;
	}
	
	killall_tk("mastiff");
	logmessage("NAT Tunnel", "AAE Service is stopped");
	
	stop_aae();
}
#endif
