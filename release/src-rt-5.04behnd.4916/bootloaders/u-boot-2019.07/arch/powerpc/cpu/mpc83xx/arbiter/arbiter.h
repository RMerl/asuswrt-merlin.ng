	const __be32 acr_mask =
#ifndef CONFIG_ACR_PIPE_DEP_UNSET
		ACR_PIPE_DEP |
#endif
#ifndef CONFIG_ACR_RPTCNT_UNSET
		ACR_RPTCNT |
#endif
#ifndef CONFIG_ACR_APARK_UNSET
		ACR_APARK |
#endif
#ifndef CONFIG_ACR_PARKM_UNSET
		ACR_PARKM |
#endif
		0;
	const __be32 acr_val =
#ifndef CONFIG_ACR_PIPE_DEP_UNSET
		CONFIG_ACR_PIPE_DEP |
#endif
#ifndef CONFIG_ACR_RPTCNT_UNSET
		CONFIG_ACR_RPTCNT |
#endif
#ifndef CONFIG_ACR_APARK_UNSET
		CONFIG_ACR_APARK |
#endif
#ifndef CONFIG_ACR_PARKM_UNSET
		CONFIG_ACR_PARKM |
#endif
		0;
