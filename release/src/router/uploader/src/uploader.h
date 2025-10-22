
#define IS_EULA_OR_PPV2_SIGNED() ((nvram_get_int("ASUS_EULA") == 1) || (get_ASUS_privacy_policy_state(ASUS_PP_CONFIG_TRANSFER) == 1))
