#include "shared.h"

enum {
	OOKLA_PROFILE_DEFAULT = 0,
	OOKLA_PROFILE_IPQ5322_64,
	OOKLA_PROFILE_IPQ5322_32,
	OOKLA_PROFILE_QCN550X,
	OOKLA_PROFILE_WIFI8_4916,
	OOKLA_PROFILE_AXHND_A7,
	OOKLA_PROFILE_MT7986A,
	OOKLA_PROFILE_MT7621A,
};

#define OOKLA_CMD_PROFILE_TUPLE(cfg, tcp_discard, eos) \
	{ \
		.config = (cfg), \
		.use_broadcom_tcp_discard = (tcp_discard), \
		.is_eos = (eos), \
	}

static const struct ookla_cmd_profile s_ookla_cmd_tuple[] = {
	[OOKLA_PROFILE_DEFAULT] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_DEFAULT, 0, 1),
	[OOKLA_PROFILE_IPQ5322_64] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_IPQ5322_64, 0, 0),
	[OOKLA_PROFILE_IPQ5322_32] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_IPQ5322_32, 0, 0),
	[OOKLA_PROFILE_QCN550X] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_QCN550X, 0, 0),
	[OOKLA_PROFILE_WIFI8_4916] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_WIFI8_4916, 1, 0),
	[OOKLA_PROFILE_AXHND_A7] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_AXHND_A7, 0, 0),
	[OOKLA_PROFILE_MT7986A] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_MT7986A, 0, 0),
	[OOKLA_PROFILE_MT7621A] = OOKLA_CMD_PROFILE_TUPLE(OOKLA_CONFIG_MT7621A, 0, 0),
};

struct ookla_cmd_profile ookla_get_cmd_profile(void)
{
#if defined(RTCONFIG_QCA)
#if defined(RTCONFIG_SOC_IPQ53XX)
#if defined(OOKLA_MUSL64)
	return s_ookla_cmd_tuple[OOKLA_PROFILE_IPQ5322_64];
#else
	return s_ookla_cmd_tuple[OOKLA_PROFILE_IPQ5322_32];
#endif
#elif defined(RTCONFIG_QCN550X)
	return s_ookla_cmd_tuple[OOKLA_PROFILE_QCN550X];
#endif
#elif defined(CONFIG_BCMWL5) || defined(RTCONFIG_BCMARM) || defined(HND_ROUTER)
#if defined(RTCONFIG_HND_ROUTER_BE_4916) && (defined(WIFI8_SDK_20260402) || defined(WIFI8_SDK_20260601))
	return s_ookla_cmd_tuple[OOKLA_PROFILE_WIFI8_4916];
#else
	return s_ookla_cmd_tuple[OOKLA_PROFILE_AXHND_A7];
#endif
#elif defined(RTCONFIG_RALINK) || defined(RTCONFIG_MT798X) || defined(RTCONFIG_MT799X)
#if defined(RTCONFIG_MT798X) || defined(RTCONFIG_MT799X)
	return s_ookla_cmd_tuple[OOKLA_PROFILE_MT7986A];
#else
	return s_ookla_cmd_tuple[OOKLA_PROFILE_MT7621A];
#endif
#endif

	return s_ookla_cmd_tuple[OOKLA_PROFILE_DEFAULT];
}

int ookla_eos_support()
{
	struct ookla_cmd_profile profile;

	profile = ookla_get_cmd_profile();

	return profile.is_eos;
}
