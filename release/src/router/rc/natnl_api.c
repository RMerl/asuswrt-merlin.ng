
#include "rc.h"
#ifdef RTCONFIG_TUNNEL
static int is_mesh_re_mode()
{
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) || defined(MAPAC1750) // Lyra
	return !nvram_get_int("cfg_master");
#elif defined(RTCONFIG_AMAS) && defined(RTCONFIG_DPSTA)	// aimesh
	return (dpsta_mode() && nvram_get_int("re_mode") == 1);
#else
	return 0;
#endif
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

void start_mastiff()
{
#ifdef CONFIG_BCMWL5
#if !(defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG))
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
