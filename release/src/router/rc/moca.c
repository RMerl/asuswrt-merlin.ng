#include <rc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/mount.h>

#include <disk_io_tools.h>	//mkdir_if_none()

#define MOCA_MPS_WALKTIME_DEFAULT	120
#define MOCA_PASSWORD_LEN	17
#define MOCA_EPASSWORD_LEN	64

typedef struct _MOCA_ONE_NODE_INFO
{
	int active;
	int node_id;
	char macaddr[18];
	char moca_ver[8];
	int phyrate[MAX_MOCA_NODES];
	unsigned char rx_snr[MAX_MOCA_NUM_CHANNELS];
}MOCA_ONE_NODE_INFO;

static int moca_monitor_stop = 0;
static void _dump_one_node_info(const MOCA_ONE_NODE_INFO *node_info);
static void _dump_node_info(const MOCA_NODE_INFO *node_info);
static void _dump_node_array(const MOCA_ONE_NODE_INFO *node_info, const size_t sz);
static void _dump_mps_report(const int report);
static void _dump_mib(const MOCA_MIB_DATA *mib);
static size_t _gen_random_password(char *password, const size_t len, const int type);
static int _convert_moca_passwd(const char* prelink_key, char *buf, const size_t sz, const int enhance_flag);
static int _moca_cmd(const int cmd, void *data);

enum{
	MOCA_CMD_GET_MOCA_FW_VER,
	MOCA_CMD_GET_CONN_STATE,
	MOCA_CMD_GET_MPS_REPORT,
	MOCA_CMD_SET_MPS_WALKTIME,
	MOCA_CMD_MPS_TRIGGER,
	MOCA_CMD_RESET_DEF_PRIVACY_CONF,
	MOCA_CMD_SET_PRIVACY,
	MOCA_CMD_GET_NODE_STATE,	//data: MOCA_ALL_NODE_INFO
	MOCA_CMD_GET_LOCAL_NODE_STATE,	//data: MOCA_ONE_NODE_INFO
	MOCA_CMD_GET_MIB_DATA,	//data: MOCA_MIB_DATA
};

typedef struct _MOCA_ALL_NODE_INFO
{
	MOCA_ONE_NODE_INFO *array;
	size_t sz;
}MOCA_ALL_NODE_INFO;

#if defined(RTCONFIG_MXL371X)
#include <ClnkLibApi.h>
#include <mxl_moca_config.h>

#define MAX_BANDS_SUPPORTED 8
#define MAX_PASSWORD_RANGE        10 // '0' ~ '9'
#define MAX_EP_PASSWORD_RANGE     94 // '!' ~ '~'
#define MS_TIMO_VAL                      10
#define NETWORK_BYTE_ORDER(BYTEPTR, INDEX)  ((*(((uint32_t *)(BYTEPTR)) + (INDEX) / 4) >> (24 - 8 * ((INDEX) - (INDEX) / 4 * 4))) & 0xFF)
#define MOCA25_UNICAST_FOUR_CHS_PROFILE    0x20
#define MOCA25_UNICAST_FIVE_CHS_PROFILE    0x21

static int _mxl371x_get_mps_state(ClnkLib_Handle_t *clnkIf)
{
	MXL_MOCA_SOC_GET_MPS_STATE_RSP_T getMpsState;
	int err = CLNK_OK;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return MOCA_CONN_STATE_UNKNOWN;
	}
	memset(&getMpsState, 0, sizeof(MXL_MOCA_SOC_GET_MPS_STATE_RSP_T));
	err = ClnkLib_GetMpsState(clnkIf, &getMpsState);
	if(err != CLNK_OK)
	{
		_dprintf("[%s]Error! ClnkLib_GetMpsState.\n", __FUNCTION__);
	}
	else
	{
		if(!getMpsState.mpsState)
		{
			return MOCA_CONN_STATE_UNPAIRED;
		}
		else
		{
			return MOCA_CONN_STATE_PAIRED;
		}
	}
	return MOCA_CONN_STATE_UNKNOWN;
}

static int _mxl371x_set_mps_walk_time(ClnkLib_Handle_t *clnkIf, int walkTime)
{
	ClnkLib_ClnkCfgV2_t clnkConf;
	clnk_param_mask_t mask;
	int32_t err;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(walkTime < 12 || walkTime > 1200)
	{
		walkTime = MOCA_MPS_WALKTIME_DEFAULT;
	}

	CLEAR_PARAM_BITS(&mask);
	SET_PARAM_BIT(&mask, OPT_MPS_WALK_TIME);

	err = ClnkLib_GetClnkCfgV2Params(clnkIf, &mask, &clnkConf);

	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! ClnkLib_GetClnkCfgV2Params\n", __FUNCTION__);
	}
	else
	{
		if(clnkConf.mpsWalkTime != walkTime)
		{
			clnkConf.mpsWalkTime = walkTime;

			err = ClnkLib_SetClnkCfgV2Params(clnkIf, &mask, &clnkConf);

			if ( err != CLNK_OK )
			{
				_dprintf("[%s]Error! ClnkLib_SetClnkCfgV2Params\n", __FUNCTION__);
			}
		}
	}
	return err;
}

static int _mxl371x_set_mps_trigger(ClnkLib_Handle_t *clnkIf)
{
	MXL_MOCA_SOC_SET_MPS_TRIGGER_RSP_T mpsTrigger;
	int err   = CLNK_OK;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}
	memset(&mpsTrigger, 0, sizeof(MXL_MOCA_SOC_SET_MPS_TRIGGER_RSP_T));

	err = ClnkLib_SetMpsTrigger(clnkIf, &mpsTrigger);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! Can't set Node MPS trigger\n", __FUNCTION__);
	}
	else if (mpsTrigger.mpsTriggerResult != 0)
	{
		_dprintf("[%s]trigger MPS failed = %d\n", __FUNCTION__, mpsTrigger.mpsTriggerResult);
		err = CLNK_ERR;
	}
	return err;
}

static int _mxl371x_get_mps_report(ClnkLib_Handle_t *clnkIf)
{
	MXL_MOCA_SOC_GET_MPS_REPORT_RSP_T mpsReport;
	int ret   = MOCA_MPS_REPORT_INVALID, i, tmp, err = CLNK_OK;
	clnk_param_mask_t mask;
	ClnkLib_ClnkCfgV2_t clinkConf;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return MOCA_MPS_REPORT_FAIL;
	}

	memset(&mpsReport, 0, sizeof(MXL_MOCA_SOC_GET_MPS_REPORT_RSP_T));
	err = ClnkLib_GetMpsReport(clnkIf, &mpsReport);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! ClnkLib_GetMpsReport\n", __FUNCTION__);
		return MOCA_MPS_REPORT_FAIL;
	}

	if (mpsReport.isMpsReportValid == 1)
	{
		for (i = 0; i < MPS_REPORT_TYPE_MAX; i++)
		{
			if (((mpsReport.mpsReportTypeBitMask >> i) & 0x1) == 0x1)
			{
				switch(i)
				{
					case MPS_REPORT_TYPE_SCAN_PAYLOAD:
						//_dprintf("[%s, %d]NC MoCA Version: 0x%x.\n", __FUNCTION__, __LINE__, mpsReport.mpsInitScanPayLoad.nc_moca_version);
						//_dprintf("[%s, %d]channel number: %d.\n", __FUNCTION__, __LINE__, mpsReport.mpsInitScanPayLoad.chn_num);
						//_dprintf("[%s, %d]Network MPS is %s triggered.\n", __FUNCTION__, __LINE__, (mpsReport.mpsInitScanPayLoad.code == 0) ? "not" : "");
						//_dprintf("[%s, %d]MPS Params: 0x%x.\n", __FUNCTION__, __LINE__, mpsReport.mpsInitScanPayLoad.mps_params);
						ret = MOCA_MPS_REPORT_SCANNING;
						break;

					case MPS_REPORT_TYPE_NETWORK_NAME_PAYLOAD:
						//_dprintf("[%s, %d]Network Name: %s.\n", __FUNCTION__, __LINE__, mpsReport.networkName);
						ret = MOCA_MPS_REPORT_SCANNING;
						break;

					case MPS_REPORT_TYPE_MPS_PAIR_STATUS:
						//_dprintf("[%s, %d]MPS_PAIR_%s\n", __FUNCTION__, __LINE__, (mpsReport.mpsPairStatus == 0) ? "FAIL" : "SUCCESS");
						ret = (mpsReport.mpsPairStatus == 0) ? MOCA_MPS_REPORT_FAIL: MOCA_MPS_REPORT_SUCCESS;
						break;

					case MPS_REPORT_TYPE_MPS_IN_PROGRESS:
						//_dprintf("[%s, %d]MPS is in Progress by Push button from node id = %d\n", __FUNCTION__, __LINE__,mpsReport.mpsInPrgPbNodeId);
						ret = MOCA_MPS_REPORT_IN_PROGRESS;
						break;
					case MPS_REPORT_TYPE_MPS_SESSION_TIMEOUT:
						//_dprintf("[%s, %d]No other Node trying to join. MPS Session timeout(Default 2Min)\n", __FUNCTION__, __LINE__);
						ret = MOCA_MPS_REPORT_TIMEOUT;
						break;
					case MPS_REPORT_TYPE_MPS_TRIGGER_FWD_NC:
						//_dprintf("[%s, %d]MPS Trigger Forwarded to NC\n", __FUNCTION__, __LINE__);
						ret = MOCA_MPS_REPORT_MPS_TRIGGER_FWD_NC;
						break;
					case MPS_REPORT_TYPE_MPS_FAILED:
						//_dprintf("[%s, %d]MPS Session Failed\n", __FUNCTION__, __LINE__);
						ret = MOCA_MPS_REPORT_FAIL;
						break;
					case MPS_REPORT_TYPE_MPS_PRIVACY_CHANGE:
						if (mpsReport.mpsPrivacyChanged == 1)
						{
							//_dprintf("[%s, %d]MPS Node Privacy Settings Changed\n", __FUNCTION__, __LINE__);
							/* Get mpsPrivacyParams and save to conf files*/
							CLEAR_PARAM_BITS(&mask);
							SET_PARAM_BIT(&mask, OPT_MOCA_PASSWD);
							SET_PARAM_BIT(&mask, OPT_SECURITY_MODE);
							SET_PARAM_BIT(&mask, OPT_FREQ_BAND_MASK);
							SET_PARAM_BIT(&mask, OPT_PRIVACY_SUPPORTED);
							SET_PARAM_BIT(&mask, OPT_ENHANCED_PASSWORD);
							err = ClnkLib_GetClnkCfgV2Params(clnkIf, &mask, &clinkConf);
							if (err == CLNK_OK)
							{
								for(tmp = 0; tmp < MAX_BANDS_SUPPORTED; tmp++)
								{
									if ((clinkConf.freqBandMask8 >> tmp) & 1)
									{
										break;
									}
								}
								if (tmp == MAX_BANDS_SUPPORTED)
								{
									_dprintf("[%s]Error:invalid band\n", __FUNCTION__);
									ret = MOCA_MPS_REPORT_FAIL;
									break;
								}
								clinkConf.privacySupported = mpsReport.mpsPrivacyParams.privacySupport;
								if (mpsReport.mpsPrivacyParams.privacyEn == 0)
								{
									// Reset Privacy En for band position.
									clinkConf.securityMode &= ~(1 << tmp);
								}
								else
								{
									// Set Privacy En for band position.
									clinkConf.securityMode |= (1 << tmp);
								}

								memcpy(clinkConf.mocaPassword[tmp], mpsReport.mpsPrivacyParams.Pswd, MOCA_PASSWORD_MAX_LEN);
								memcpy(clinkConf.mocaEPPassword, mpsReport.mpsPrivacyParams.Epswd, MAX_MOCA_EP_PASSWORD_LEN_PADDED);

								err = ClnkLib_SetClnkCfgV2Params(clnkIf, &mask, &clinkConf);
								if (err != CLNK_OK)
								{
									_dprintf("[%s]Error! Can't save configuration changes permanently\n", __FUNCTION__);
									ret = MOCA_MPS_REPORT_FAIL;
									break;
								}
								else
								{
									nvram_set_int("moca_privacy_enable", clinkConf.securityMode? 1: 0);
									nvram_set_int("moca_sceu_mode", clinkConf.privacySupported);
									nvram_set("moca_password", (char *)mpsReport.mpsPrivacyParams.Pswd);
									nvram_set("moca_epassword", (char *)mpsReport.mpsPrivacyParams.Epswd);
								}
							}
							else
							{
								_dprintf("[%s]Error! Can't read configuration\n", __FUNCTION__);
								ret = MOCA_MPS_REPORT_FAIL;
								break;
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	//_dprintf("[%s, %d]ret = %d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}

static int _mxl371x_reset_default_privacy_conf(ClnkLib_Handle_t *clnkIf)
{
	int err   = CLNK_OK, tmp, i;
	ClnkLib_ClnkCfgV2_t currCfg;
	clnk_param_mask_t mask;
	char password[MAX_PASSWORD_LENGTH + 1];
	char epassword[MAX_MOCA_EP_PASSWORD_LEN_PADDED + 1];
	int privacySupported = nvram_get_int("moca_sceu_mode");

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}
	memset(password, 0, sizeof(password));
	memset(epassword, 0, sizeof(epassword));
	if(!_gen_random_password(password, MAX_PASSWORD_LENGTH, TYPE_OF_LEGACY_PASSWORD))
	{
		_dprintf("[%s]Error! Fail to get random password.\n", __FUNCTION__);
		return CLNK_ERR;
	}
	if(!_gen_random_password(epassword, MAX_MOCA_EP_PASSWORD_LEN_PADDED, TYPE_OF_ENHANCED_PASSWORD))
	{
		_dprintf("[%s]Error! Fail to get random enhanced password.\n", __FUNCTION__);
		return CLNK_ERR;
	}
	CLEAR_PARAM_BITS(&mask);
	SET_PARAM_BIT(&mask, OPT_MOCA_PASSWD);
	SET_PARAM_BIT(&mask, OPT_SECURITY_MODE);
	SET_PARAM_BIT(&mask, OPT_PRIVACY_SUPPORTED);
	SET_PARAM_BIT(&mask, OPT_ENHANCED_PASSWORD);

	nvram_set_int("moca_dev_state", MOCA_STATE_INIT_DEV);
	nvram_set_int("moca_mps_trigger", 0);
	nvram_set_int("moca_conn_state", MOCA_CONN_STATE_UNKNOWN);
	nvram_set_int("moca_mps_report", MOCA_MPS_REPORT_INVALID);
	err = ClnkLib_GetClnkCfgV2Params(clnkIf, &mask, &currCfg);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Failed to get clink config file\n", __FUNCTION__);
		return CLNK_ERR;
	}
	for(tmp = 0; tmp < MAX_BANDS_SUPPORTED; tmp++)
	{
		// The Default setting is PrivacyEn=Enabled as per Table17.1 in MOCA 2.5 Spec
		currCfg.securityMode |= (1 << tmp);                // Set Privacy bit
	}
	currCfg.privacySupported = privacySupported;
	memcpy(&currCfg.mocaEPPassword[0], epassword, MAX_MOCA_EP_PASSWORD_LEN_PADDED);
	for(i=0; i<MAX_BANDS; i++)
	{
		memcpy(currCfg.mocaPassword[i], password, sizeof(password));
	}
	err = ClnkLib_SetClnkCfgV2Params(clnkIf, &mask, &currCfg);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]SetClnkCfgV2 failed\n", __FUNCTION__);
		return CLNK_ERR;
	}
	err = ClnkLib_Reset(clnkIf);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! Can't restart SoC\n", __FUNCTION__);
		nvram_set_int("moca_dev_state", MOCA_STATE_ERR_DEV_FAIL);
	}
	else
	{
		nvram_set_int("moca_dev_state", MOCA_STATE_DEV_READY);
		nvram_set("moca_privacy_enable", "1");
		nvram_set_int("moca_sceu_mode", privacySupported);
		nvram_set("moca_password", password);
		nvram_set("moca_epassword", epassword);
	}
	return err;
}

static int _mxl371x_set_privacy(ClnkLib_Handle_t *clnkIf)
{
	int err = CLNK_OK, tmp;
	ClnkLib_ClnkCfgV2_t clinkConf;
	clnk_param_mask_t mask;
	int is_enable, is_pwd_given = 0, supported;
	char password[20] = {0}, epassword[68] = {0};

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	is_enable = nvram_get_int("moca_privacy_enable");
	strlcpy(password,  nvram_safe_get("moca_password"), sizeof(password));
	strlcpy(epassword,  nvram_safe_get("moca_epassword"), sizeof(epassword));
	supported = nvram_get_int("moca_sceu_mode");

	if(strlen(password) > 0)
	{
		is_pwd_given = 1;
	}
	if(strlen(epassword) > 0)
	{
		is_pwd_given |= 0x02;
	}

	CLEAR_PARAM_BITS(&mask);
	SET_PARAM_BIT(&mask, OPT_MOCA_PASSWD);
	SET_PARAM_BIT(&mask, OPT_SECURITY_MODE);
	SET_PARAM_BIT(&mask, OPT_PRIVACY_SUPPORTED);
	SET_PARAM_BIT(&mask, OPT_ENHANCED_PASSWORD);
	err = ClnkLib_GetClnkCfgV2Params(clnkIf, &mask, &clinkConf);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! GetClnkCfgV2 failed\n", __FUNCTION__);
		return err;
	}

	for(tmp = 0; tmp < MAX_BANDS_SUPPORTED; tmp++)
	{
		//if((clinkConf.freqBandMask8 >> tmp) & 0x1)
		{
			if(is_enable || (is_pwd_given & 0x01))
			{
				memcpy(&clinkConf.mocaPassword[tmp], password, 18);
			}

			if(is_enable)
			{
				clinkConf.securityMode |= (1 << tmp);                // Set Privacy bit
			}
			else
			{
				clinkConf.securityMode &= ~(0x1 << tmp);             // Clear Privacy bit
			}
		}
	}

	if(is_enable || (is_pwd_given & 0x02))
	{
		memcpy(&clinkConf.mocaEPPassword[0], epassword, MOCA_EPASSWORD_LEN);
	}
	if(is_enable || (is_pwd_given & 0x04))
	{
		clinkConf.privacySupported = supported;
	}

	err = ClnkLib_SetClnkCfgV2Params(clnkIf, &mask, &clinkConf);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! Can't save configuration changes permanently\n", __FUNCTION__);
		return err;
	}

	nvram_set_int("moca_dev_state", MOCA_STATE_INIT_DEV);
	nvram_set_int("moca_mps_trigger", 0);
	nvram_set_int("moca_conn_state", MOCA_CONN_STATE_UNKNOWN);
	nvram_set_int("moca_mps_report", MOCA_MPS_REPORT_INVALID);

	err = ClnkLib_Reset(clnkIf);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! Can't restart SoC\n", __FUNCTION__);
		nvram_set_int("moca_dev_state", MOCA_STATE_ERR_DEV_FAIL);
	}
	else
	{
		nvram_set_int("moca_dev_state", MOCA_STATE_DEV_READY);
	}
	return err;
}

static int _mxl371x_get_node_info(ClnkLib_Handle_t *clnkIf, int nodeID, MOCA_ONE_NODE_INFO *node_info)
{
	ClnkLib_NetInfo_t     myNetworkNodeInfo = {0};

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}
	if(!node_info)
	{
		_dprintf("[%s]Error! No node_info\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(ClnkLib_GetNetInfo(clnkIf, nodeID, &myNetworkNodeInfo))
	{
		_dprintf("[%s]Error Get NetInfo\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(0 == myNetworkNodeInfo.state) // Inactive
	{
		return CLNK_ERR; // don't show info for inactive nodes
	}

	node_info->active = 1;
	node_info->node_id = nodeID;
	snprintf(node_info->macaddr, sizeof(node_info->macaddr), "%02x:%02x:%02x:%02x:%02x:%02x\n",
		(myNetworkNodeInfo.guidHi >> 24),
		(myNetworkNodeInfo.guidHi >> 16) & 0xff,
		(myNetworkNodeInfo.guidHi >> 8) & 0xff,
		(myNetworkNodeInfo.guidHi >> 0) & 0xff,
		(myNetworkNodeInfo.guidLo >> 24),
		(myNetworkNodeInfo.guidLo >> 16) & 0xff);
	snprintf(node_info->moca_ver, sizeof(node_info->moca_ver), "%d.%d", 
		(myNetworkNodeInfo.mocaVer>>4)&0x0f, (myNetworkNodeInfo.mocaVer&0x0f));

	return CLNK_OK;
}

static int _mxl371x_get_fmrInfo(ClnkLib_Handle_t *clnkIf, ClnkLib_FmrInfo_t *myFmrInfo, int *collectedNodeMask, const int activeNodeBitmask, const int version)
{
	int nodeBitMask, nodesPerSession = 0, currPosition = 0, currFmrSessionMask = 0;
	int numOfCollectedNodes = 0, addedNodes = 0;
	int i;

	if(!clnkIf || !myFmrInfo || !collectedNodeMask)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return 0;
	}

	nodeBitMask = activeNodeBitmask;
	nodesPerSession = (1 == version)? 10 : 2;
	do
	{
		currFmrSessionMask = 0;
		addedNodes = 0;
		for (i=currPosition; ((addedNodes < nodesPerSession) && (currPosition < MAX_MOCA_NODES)) ; i++, currPosition++)
		{
			if (nodeBitMask & (1 << currPosition))
			{
				currFmrSessionMask |= (1 << currPosition);
				addedNodes++;
			}
		}

		if (currFmrSessionMask & activeNodeBitmask)
		{
			ClnkLib_GetFmrInfo(clnkIf, currFmrSessionMask, version, myFmrInfo);
		}

		numOfCollectedNodes += myFmrInfo->numOfNodes;
		*collectedNodeMask |= myFmrInfo->nodeMask;
	} while (currPosition < MAX_MOCA_NODES);
	return numOfCollectedNodes;
}

static int _mxl371x_get_node_phyrate(ClnkLib_Handle_t *clnkIf, const int activeNodeBitmask, MOCA_ONE_NODE_INFO *node_info_array, const size_t array_sz)
{
	ClnkLib_FmrInfo_t myFmrInfo = {0,};
	int collectedNodeMask = 0;
	int i, j;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}
	if(!node_info_array)
	{
		_dprintf("[%s]Error! No node_info_array\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(_mxl371x_get_fmrInfo(clnkIf, &myFmrInfo, &collectedNodeMask, activeNodeBitmask, 2) > 0)
	{
		for(i = 0; i < array_sz; ++i)
		{
			if(collectedNodeMask & (1 << i))
			{
				if (0 == myFmrInfo.nodeFmr[i].validInfo)
				{
					_dprintf("[%s]Failed to retrieve node %d FMR information\n", __FUNCTION__, i);
					continue;
				}

				for(j = 0; j < MAX_MOCA_NODES; ++j)
				{
					node_info_array[i].phyrate[j] = myFmrInfo.nodeFmr[i].p2p[j].rateNper;
				}
			}
		}
	}
	return CLNK_OK;
}

static int _mxl371x_get_node_phyrate_by_id(ClnkLib_Handle_t *clnkIf, const int activeNodeBitmask, const int node_id, MOCA_ONE_NODE_INFO *node_info)
{
	ClnkLib_FmrInfo_t myFmrInfo = {0,};
	int collectedNodeMask = 0;
	int i;
	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(!node_info)
	{
		_dprintf("[%s]Error! No node_info\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(_mxl371x_get_fmrInfo(clnkIf, &myFmrInfo, &collectedNodeMask, activeNodeBitmask, 2) > 0)
	{
			if(collectedNodeMask & (1 << node_id))
			{
				if (0 == myFmrInfo.nodeFmr[node_id].validInfo)
				{
					_dprintf("[%s]Failed to retrieve node %d FMR information\n", __FUNCTION__, i);
					return CLNK_ERR;
				}

				for(i = 0; i < MAX_MOCA_NODES; ++i)
				{
					node_info->phyrate[i] = myFmrInfo.nodeFmr[node_id].p2p[i].rateNper;
				}
			}
	}
	return CLNK_OK;
}

static int _mxl371x_get_node_snr(ClnkLib_Handle_t *clnkIf, int nodeID, MOCA_ONE_NODE_INFO *node_info)
{
	int i;
	ClnkLib_PhyRxProfile_t rxProfileRsp;
	unsigned int mocaProfileId = MOCA25_UNICAST_FIVE_CHS_PROFILE;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}
	if(!node_info)
	{
		_dprintf("[%s]Error! No node_info\n", __FUNCTION__);
		return CLNK_ERR;
	}

	for(i = 0; i < MAX_MOCA_NUM_CHANNELS; ++i)
	{
		mocaProfileId = MOCA25_UNICAST_FIVE_CHS_PROFILE | i << 16;
		if(ClnkLib_GetPhyRxProfile(clnkIf, nodeID, mocaProfileId, 1, &rxProfileRsp) == CLNK_OK)
		{
			if(rxProfileRsp.rxProfileValid)
			{
				node_info->rx_snr[i] = (unsigned char)rxProfileRsp.rxChnSnr;
			}
		}
	}
	return CLNK_OK;
}

static int _mxl371x_get_node_state(ClnkLib_Handle_t *clnkIf, MOCA_ONE_NODE_INFO *node, const size_t sz)
{
	int i;
	ClnkLib_LocalInfo_t localInfo;
	MOCA_ONE_NODE_INFO moca_node[MAX_MOCA_NODES];

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	memset(moca_node, 0, sizeof(moca_node));
	if (ClnkLib_GetLocalInfo(clnkIf, &localInfo) != CLNK_OK)
	{
		_dprintf("\n[%s] Error! Can't get Local Info\n", __FUNCTION__);
		return CLNK_ERR;
	}

	for(i = 0; i < MAX_MOCA_NODES; ++i)
	{
		if(localInfo.activeNodeBitmask & (1 << i))
		{
			_mxl371x_get_node_info(clnkIf, i, &moca_node[i]);
			_mxl371x_get_node_snr(clnkIf, i, &moca_node[i]);
		}
	}
	_mxl371x_get_node_phyrate(clnkIf, localInfo.activeNodeBitmask, moca_node, MAX_MOCA_NODES);

	if(node)
	{
		memcpy(node, moca_node, sizeof(MOCA_ONE_NODE_INFO) * (sz > MAX_MOCA_NODES? MAX_MOCA_NODES: sz));
	}
	else	//only dump info
	{
		_dump_node_array(moca_node, MAX_MOCA_NODES);
	}
	return CLNK_OK;
}

static int _mxl371x_get_local_node_state(ClnkLib_Handle_t *clnkIf, MOCA_ONE_NODE_INFO *local_node)
{
	ClnkLib_LocalInfo_t localInfo;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(!local_node)
	{
		_dprintf("[%s]Error! No local_node\n", __FUNCTION__);
		return CLNK_ERR;
	}

	memset(local_node, 0, sizeof(MOCA_ONE_NODE_INFO));
	if (ClnkLib_GetLocalInfo(clnkIf, &localInfo) != CLNK_OK)
	{
		_dprintf("\n[%s] Error! Can't get Local Info\n", __FUNCTION__);
		return CLNK_ERR;
	}

	_mxl371x_get_node_info(clnkIf, localInfo.nodeId, local_node);
	_mxl371x_get_node_snr(clnkIf, localInfo.nodeId, local_node);
	_mxl371x_get_node_phyrate_by_id(clnkIf, localInfo.activeNodeBitmask, localInfo.nodeId, local_node);
	_dump_one_node_info(local_node);
	return CLNK_OK;
}

static int _mxl371x_get_moca_fw_ver(ClnkLib_Handle_t *clnkIf)
{
	char ver[MAX_VERSION_STR_SIZE];
	ClnkLib_LocalInfo_t   myNodeInfo;
	int i;

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(ClnkLib_GetLocalInfo(clnkIf, &myNodeInfo) != CLNK_OK)
	{
		_dprintf("[%s]Error Getting LocalInfo\n", __FUNCTION__);
		return CLNK_ERR;
	}

	memset(ver, 0, sizeof(ver));
	for (i = 0; i < MAX_VERSION_STR_SIZE; i++)
	{
		ver[i] = NETWORK_BYTE_ORDER(myNodeInfo.socVersionStr, i);
	}

	nvram_set("moca_fw_ver", ver);
	return CLNK_OK;
}

static int _mxl371x_get_mib_data(ClnkLib_Handle_t *clnkIf, MOCA_MIB_DATA *mib)
{
	ClnkLib_EcbStats_t  ecb_stats = {0};
	ClnkLib_DataStats_t  stats = {0};

	if(!clnkIf)
	{
		_dprintf("[%s]Error! No clnkIf\n", __FUNCTION__);
		return CLNK_ERR;
	}

	if(!mib)
	{
		_dprintf("[%s]Error! No mib\n", __FUNCTION__);
		return CLNK_ERR;
	}

	memset(mib, 0, sizeof(MOCA_MIB_DATA));

	if (ClnkLib_GetEcbStats(clnkIf, 0, &ecb_stats) != CLNK_OK
		|| ClnkLib_GetDataStats(clnkIf, 0, &stats) != CLNK_OK)
	{
		_dprintf("[%s]Error getting ECB stats\n", __FUNCTION__);
		return CLNK_ERR;
	}
	mib->tx_bytes = SWAPLONG(stats.txTotalBytes);
	mib->rx_bytes = SWAPLONG(stats.rxTotalBytes);
	mib->tx_packets = SWAPLONG(stats.txTotalPkts);
	mib->rx_packets = SWAPLONG(stats.rxTotalPkts);
	mib->rx_crc = ecb_stats.debug14;
	return CLNK_OK;
}

static int _init_mxl371x()
{
	char cmd[512], path[128];
	char amas_bdlkey[64] = {0};
	const char workdir[] = "/jffs/mxl371x";
	const char confdir[] = "conf";
	const char fwdir[] = "fw";
	const char rom_dir[] = "/rom/etc/clink";
	const char fwpath[] = "/lib/firmware/ccpu.elf.leucadia";
	const char default_conf_file[] = "/jffs/mxl371x/conf/clink.backup";
	const char *passwd_name[] = {"mocapasswordbandd", "mocapasswordbanddlo", "mocapasswordbanddhi", 
		"mocapasswordbande", "mocapasswordbandfsat", "mocapasswordbandfcbl", "mocapasswordbandh", "mocapasswordcustom", ""};
	char passwd[20] = {0}, epasswd[68] = {0}, pwd[68] = {0};
	int flag = 0, i;
	FILE *fp;

	nvram_set_int("moca_dev_state", MOCA_STATE_UNKNOW);

	//check config files and fw file exist
	snprintf(path, sizeof(path), "%s/%s", workdir, fwdir);
	if(!f_exists(default_conf_file) || !d_exists(workdir))
	{
		flag = 1;
	}

	if(nvram_match("x_Setting", "0") || flag)
	{
		//default state, remove the config file in workdir
		eval("rm", "-rf", (char*)workdir);

		//create working directory, May need to check default state
		mkdir_if_none(workdir);
	}

	if(d_exists(workdir))	//update the default config files and fw file for moca driver update
	{
		//restore backup config file
		snprintf(path, sizeof(path), "%s/%s", workdir, confdir);
		mkdir_if_none(path);
		snprintf(cmd, sizeof(cmd), "cp -af %s/* %s", rom_dir, path);
		system(cmd);

		//restore moca firmware file
		snprintf(path, sizeof(path), "%s/%s", workdir, fwdir);
		mkdir_if_none(path);
		snprintf(cmd, sizeof(cmd), "cp -af %s %s", fwpath, path);
		system(cmd);

		//confirm prelink key
		if(nvram_match("x_Setting", "0"))
		{
			strlcpy(amas_bdlkey, nvram_safe_get("amas_bdlkey"), sizeof(amas_bdlkey));
			if(amas_bdlkey[0] != '\0')
			{
				memset(passwd, 0, sizeof(passwd));
				memset(epasswd, 0, sizeof(epasswd));
				if(_convert_moca_passwd(amas_bdlkey, passwd, sizeof(passwd), 0) > 0)
				{
					nvram_set("moca_password", passwd);
				}
				if(_convert_moca_passwd(amas_bdlkey, epasswd, sizeof(epasswd), 1) > 0)
				{
					nvram_set("moca_epassword", epasswd);
				}
			}
		}

		//update password to the default config file from nvram whether there is a bundle key.
		fp = fopen(default_conf_file, "a");
		if(fp)
		{
			strlcpy(pwd, nvram_safe_get("moca_password"), sizeof(pwd));
			if(pwd[0] != '\0')
			{
				for(i = 0; passwd_name[i][0] != '\0'; ++i)
				{
					fprintf(fp, "%s 17 %s\n", passwd_name[i], pwd);
				}
			}
			strlcpy(pwd, nvram_safe_get("moca_epassword"), sizeof(pwd));
			if(pwd[0] != '\0')
			{
				fprintf(fp, "enhancedpassword 64 %s\n", pwd);
			}
			fclose(fp);
		}
	}

	nvram_set_int("moca_dev_state", MOCA_STATE_INIT_ENV);
	return 0;
}

static void _start_mxl371x()
{
	_dprintf("[%s]\n", __FUNCTION__);
	nvram_set_int("moca_dev_state", MOCA_STATE_INIT_DEV);
	//mount /lib/firmware for moca driver
	mount ("mdev", "/lib/firmware", "tmpfs", 0, NULL);
	//eval("mount", "-t",  "tmpfs", "mdev", "/lib/firmware");
	eval("ln", "-sf", "/jffs/mxl371x/fw", "/lib/firmware/updates");
	//insert moca module
	eval("insmod", "mxl_moca_ctrl.ko");
	//init moca device 
	eval("/bin/clnkrst", "-d", "--dlverify", "-a", nvram_safe_get("et0macaddr"), "--confdir", "/jffs/mxl371x/conf");
	//system("nohup updatelof.sh > /dev/null 2>&1 &");
	//config speed
	nvram_set_int("moca_dev_state", MOCA_STATE_DEV_READY);
	nvram_set_int("moca_mps_trigger", 0);
	nvram_set_int("moca_conn_state", MOCA_CONN_STATE_UNKNOWN);
	nvram_set_int("moca_mps_report", MOCA_MPS_REPORT_INVALID);
	_moca_cmd(MOCA_CMD_GET_MOCA_FW_VER, NULL);
	moca_monitor_stop = 0;
	xstart("/sbin/moca_monitor");
}

static void _stop_mxl371x()
{
	_dprintf("[%s]\n", __FUNCTION__);
	nvram_set_int("moca_dev_state", MOCA_STATE_UNKNOW);
	sleep(1);
	killall_tk("moca_monitor");
	eval("rmmod", "mxl_moca_ctrl");
	eval("umount", "/lib/firmware")	;
}

static int _mxl371x_cmd(const int cmd, void *data)
{
	ClnkLib_Handle_t clnkIf;
	int err = CLNK_OK, ret;
	MOCA_ONE_NODE_INFO *local_node;
	MOCA_ALL_NODE_INFO *all_node;
	MOCA_MIB_DATA *mib_data;

	if(nvram_get_int("moca_dev_state") != MOCA_STATE_DEV_READY)
	{
		return -1;
	}
	/* Initialize connection to driver */
	err = ClnkLib_OpenByName(&clnkIf, NULL);
	if (err != CLNK_OK)
	{
		_dprintf("[%s]Error! Can't find c.LINK device\n", __FUNCTION__);
		nvram_set_int("moca_dev_state", MOCA_STATE_ERR_DEV_FAIL);
	}
	else if (clnkIf.socChipType == MXL_MOCA_SOC_TYPE_CARDIFF)
	{
		_dprintf("[%s]Error! Does not apply for Cardiff.\n", __FUNCTION__);
		err = CLNK_ERR;
		ClnkLib_Close(&clnkIf);
		nvram_set_int("moca_dev_state", MOCA_STATE_ERR_DEV_FAIL);
	}
	else
	{
		switch(cmd)
		{
			case MOCA_CMD_GET_MOCA_FW_VER:
				err = _mxl371x_get_moca_fw_ver(&clnkIf);
				break;
			case MOCA_CMD_GET_CONN_STATE:
				ret = _mxl371x_get_mps_state(&clnkIf);
				if(ret != MOCA_STATE_ERR)
				{
					//_dprintf("[%s, %d] moca_conn_state=%d\n", __FUNCTION__, __LINE__, ret);
					nvram_set_int("moca_conn_state", ret);
					err = CLNK_OK;
				}
				else
				{
					nvram_set_int("moca_dev_state", ret);
					err = CLNK_ERR;
				}
				break;
			case MOCA_CMD_SET_MPS_WALKTIME:
				err = _mxl371x_set_mps_walk_time(&clnkIf, nvram_get_int("moca_mps_walktime"));
				break;
			case MOCA_CMD_MPS_TRIGGER:
				err = _mxl371x_set_mps_trigger(&clnkIf);
				if(err == CLNK_OK)
				{
					nvram_set("moca_mps_trigger", "1");
				}
				break;
			case MOCA_CMD_GET_MPS_REPORT:
				if(nvram_match("moca_mps_trigger", "1"))
				{
					ret = _mxl371x_get_mps_report(&clnkIf);
					_dump_mps_report(ret);
					if(ret != MOCA_MPS_REPORT_INVALID)
					{
						nvram_set_int("moca_mps_report", ret);
						err = CLNK_OK;
					}
					else
					{
						err = CLNK_ERR;
					}
					if(ret == MOCA_MPS_REPORT_IN_PROGRESS ||
						ret == MOCA_MPS_REPORT_MPS_TRIGGER_FWD_NC ||
						ret == MOCA_MPS_REPORT_SUCCESS ||
						ret == MOCA_MPS_REPORT_TIMEOUT ||
						ret == MOCA_MPS_REPORT_FAIL)
						{
							nvram_set("moca_mps_trigger", "0");
						}
				}
				break;
			case MOCA_CMD_RESET_DEF_PRIVACY_CONF:
				err = _mxl371x_reset_default_privacy_conf(&clnkIf);
				break;
			case MOCA_CMD_SET_PRIVACY:
				err = _mxl371x_set_privacy(&clnkIf);
				break;
			case MOCA_CMD_GET_NODE_STATE:
				if(data)
				{
					all_node = (MOCA_ALL_NODE_INFO*)data;
					err = _mxl371x_get_node_state(&clnkIf, all_node->array, all_node->sz);
				}
				else
				{
					err = _mxl371x_get_node_state(&clnkIf, NULL, 0);
				}
				break;
			case MOCA_CMD_GET_LOCAL_NODE_STATE:
				if(data)
				{
					local_node = (MOCA_ONE_NODE_INFO*)data;
					err = _mxl371x_get_local_node_state(&clnkIf, local_node);
				}
				break;
			case MOCA_CMD_GET_MIB_DATA:
				if(data)
				{
					mib_data = (MOCA_MIB_DATA*)data;
					err = _mxl371x_get_mib_data(&clnkIf, mib_data);
				}
				break;
			default:
				_dprintf("[%s]Error! Unknown command(%d).\n", __FUNCTION__, cmd);
				break;
		}
		ClnkLib_Close(&clnkIf);
	}
	return (err == CLNK_OK)? 0: -1;
}
#endif

static void _dump_one_node_info(const MOCA_ONE_NODE_INFO *node_info)
{
	int i;
	if(node_info)
	{
		_dprintf("[%s] dump node(%d)\n", __FUNCTION__, node_info->node_id);
		_dprintf("\tMAC address: %s\n", node_info->macaddr);
		_dprintf("\tMoCA version: %s\n", node_info->moca_ver);
		_dprintf("\tPhy rate:");
		for(i = 0; i < MAX_MOCA_NODES; ++i)
		{
			_dprintf(" %04d", node_info->phyrate[i]);
			if(i && !(i % (MAX_MOCA_NODES/2)))
				_dprintf("\n\t\t");
		}
		_dprintf("\n\tRx Snr:");
		for(i = 0; i < MAX_MOCA_NUM_CHANNELS; ++i)
		{
			_dprintf(" %04d", node_info->rx_snr[i]);
		}
		_dprintf("\n");
	}
}

static void _dump_node_info(const MOCA_NODE_INFO *node_info)
{
	int i, j;
	if(node_info)
	{
		_dprintf("[%s] dump node(%d)\n", __FUNCTION__, node_info->node_id);
		_dprintf("\tMAC address: %s\n", node_info->macaddr);
		_dprintf("\tMoCA version: %s\n", node_info->moca_ver);
		_dprintf("\tNode MAC address:");
		for(i = 0; i < MAX_MOCA_NODES; ++i)
		{
			_dprintf(" %s", node_info->node_mac[i]);
			if(i && !((i + 1) % (MAX_MOCA_NODES/4)))
				_dprintf("\n\t\t");
		}
		_dprintf("\n\tNode MoCA ver:");
		for(i = 0; i < MAX_MOCA_NODES; ++i)
		{
			_dprintf(" %s", node_info->node_moca_ver[i]);
			if(i && !((i + 1) % (MAX_MOCA_NODES/4)))
				_dprintf("\n\t\t");
		}
		_dprintf("\n\tPhy rate:");
		for(i = 0; i < MAX_MOCA_NODES; ++i)
		{
			_dprintf(" %04d", node_info->phyrate[i]);
			if(i && !((i + 1) % (MAX_MOCA_NODES/2)))
				_dprintf("\n\t\t");
		}
		_dprintf("\n\tRx Snr:");
		for(i = 0; i < MAX_MOCA_NODES; ++i)
		{
			for(j = 0; j < MAX_MOCA_NUM_CHANNELS; ++j)
			{
				_dprintf(" %02u", node_info->rx_snr[i][j]);
			}
			_dprintf("\t");
			if(i && !((i + 1) % (MAX_MOCA_NODES/4)))
				_dprintf("\n\t\t");
		}
		_dprintf("\n");
	}
}

static void _dump_node_array(const MOCA_ONE_NODE_INFO *node_info, const size_t sz)
{
	int i;
	if(node_info)
	{
		for(i = 0; i < sz; ++i)
		{
			if(node_info[i].active)
				_dump_one_node_info(&node_info[i]);
		}
	}
}

static void _dump_mps_report(const int report)
{
	switch(report)
	{
		case MOCA_MPS_REPORT_INVALID:
			//_dprintf("[%s]MOCA_MPS_REPORT_INVALID\n", __FUNCTION__);
			break;
		case MOCA_MPS_REPORT_SCANNING:
			_dprintf("[%s]MOCA_MPS_REPORT_SCANNING\n", __FUNCTION__);
			break;
		case MOCA_MPS_REPORT_IN_PROGRESS:
			_dprintf("[%s]MOCA_MPS_REPORT_IN_PROGRESS\n", __FUNCTION__);
			break;
		case MOCA_MPS_REPORT_MPS_TRIGGER_FWD_NC:
			_dprintf("[%s]MOCA_MPS_REPORT_MPS_TRIGGER_FWD_NC\n", __FUNCTION__);
			break;
		case MOCA_MPS_REPORT_MPS_PRIVACY_CHANGE:
			_dprintf("[%s]MOCA_MPS_REPORT_MPS_PRIVACY_CHANGE\n", __FUNCTION__);
			break;
		case MOCA_MPS_REPORT_SUCCESS:
			_dprintf("[%s]MOCA_MPS_REPORT_SUCCESS\n", __FUNCTION__);
			break;
		case MOCA_MPS_REPORT_TIMEOUT:
			_dprintf("[%s]MOCA_MPS_REPORT_TIMEOUT\n", __FUNCTION__);
			break;
		default :
			_dprintf("[%s]MOCA_MPS_REPORT_FAIL\n", __FUNCTION__);
			break;
	}
}

static void _dump_mib(const MOCA_MIB_DATA *mib)
{
	if(mib)
	{
		_dprintf("[%s] dump mib, crc:%u\n", __FUNCTION__, mib->rx_crc);
		_dprintf("tx bytes: %u\ttx packets:%u\n", mib->tx_bytes, mib->tx_packets);
		_dprintf("rx bytes: %u\trx packets:%u\n", mib->rx_bytes, mib->rx_packets);
	}
}

static size_t _gen_random_password(char *password, const size_t len, const int type)
{
	int i;
	if(!password)
	{
		_dprintf("[%s]Error! password is NULL\n", __FUNCTION__);
		return 0;
	}

	memset(password, 0, len);
	srand((unsigned int)time((time_t*) NULL));
	if(type == TYPE_OF_LEGACY_PASSWORD)
	{
		if(len > MAX_PASSWORD_LENGTH)
		{
			_dprintf("[%s]Error! length is invalid.\n", __FUNCTION__);
			return 0;
		}
		for(i = 0; i < len; ++i)
		{
			password[i] = rand() % MAX_PASSWORD_RANGE + '0';
		}
	}
	else
	{
		if(len > MAX_MOCA_EP_PASSWORD_LEN_PADDED)
		{
			_dprintf("[%s]Error! length is invalid.\n", __FUNCTION__);
			return 0;
		}
		for(i = 0; i < len; ++i)
		{
			password[i] = rand() % MAX_EP_PASSWORD_RANGE + '!';
		}
	}
	return strlen(password);
}

static int _convert_moca_passwd(const char* prelink_key, char *buf, const size_t sz, const int enhance_flag)
{
	int i, cnt, len;
	char tmp[4];
	if(!prelink_key || !buf)
		return 0;

	buf[0] = '\0';
	len = strlen(prelink_key);
	if(!enhance_flag && sz > MOCA_PASSWORD_LEN)	//17 ASCII numeric characters
	{
		for(i = 0, cnt = 0; i < len; ++i)
		{
			if(prelink_key[i] >= '0' && prelink_key[i] <= '9')
			{
				buf[cnt++] = prelink_key[i];
			}
			else
			{
				snprintf(tmp, sizeof(tmp), "%02d", prelink_key[i]);
				if(cnt <= 15)
				{
					strlcat(buf, tmp, sz);
					cnt += strlen(tmp);
				}
				else	//cnt == 16
				{
					buf[cnt] = tmp[0];
				}
			}
			if(cnt == MOCA_PASSWORD_LEN)
				break;
		}
	}
	else if(enhance_flag && sz > MOCA_EPASSWORD_LEN)	//64 ASCII printable characters
	{
		if(len > MOCA_EPASSWORD_LEN)
		{
			strlcpy(buf, prelink_key, MOCA_EPASSWORD_LEN + 1);
		}
		else
		{
			len = 0;
			while(strlen(buf) < MOCA_EPASSWORD_LEN)
			{
				strlcpy(buf + len, prelink_key, MOCA_EPASSWORD_LEN + 1 -len);
				len = strlen(buf);
			}
		}
	}
	else
	{
		return 0;
	}
	return strlen(buf);
}

static int _moca_cmd(const int cmd, void* data)
{
#ifdef RTCONFIG_MXL371X
	return _mxl371x_cmd(cmd, data);
#endif
}

int init_moca()
{
#ifdef RTCONFIG_MXL371X
	_init_mxl371x();
#endif
	if(nvram_match("x_Setting", "0"))	//remove moca interface from br0
	{
		eval("brctl", "delif", nvram_safe_get("lan_ifname"), nvram_safe_get("moca_ifname"));
		nvram_set("moca_br_init", "1");
	}
	return 0;
}

void start_moca()
{
#ifdef RTCONFIG_MXL371X
	_start_mxl371x();
#endif
}

void stop_moca()
{
#ifdef RTCONFIG_MXL371X
	_stop_mxl371x();
#endif
}

void start_moca_mps()
{
	_dprintf("[%s]\n", __FUNCTION__);
	//set mps timeout
	_moca_cmd(MOCA_CMD_SET_MPS_WALKTIME, NULL);
	//set mps timeout
	_moca_cmd(MOCA_CMD_MPS_TRIGGER, NULL);
}

void reset_moca_default_privacy()
{
	_dprintf("[%s]\n", __FUNCTION__);
	_moca_cmd(MOCA_CMD_RESET_DEF_PRIVACY_CONF, NULL);
	nvram_set_int("moca_conn_state", MOCA_CONN_STATE_UNKNOWN);
}

void moca_node_state(MOCA_ONE_NODE_INFO *node, const size_t sz)
{
	MOCA_ALL_NODE_INFO all_node;
	//_dprintf("[%s]\n", __FUNCTION__);
	if(node)
	{
		all_node.sz = sz;
		all_node.array = node;
		_moca_cmd(MOCA_CMD_GET_NODE_STATE, &all_node);
	}
	else
	{
		_moca_cmd(MOCA_CMD_GET_NODE_STATE, NULL);
	}
}

void moca_set_privacy()
{
	_dprintf("[%s]\n", __FUNCTION__);
	_moca_cmd(MOCA_CMD_SET_PRIVACY, NULL);
}

void moca_local_node_state(MOCA_NODE_INFO *node)
{
	MOCA_NODE_INFO local_node;
	FILE *fp;

	memset(&local_node, 0, sizeof(MOCA_NODE_INFO));
	get_moca_devices(&local_node, 0, 0);
	//_dump_node_info(&local_node);
	if(node)
	{
		memcpy(node, &local_node, sizeof(MOCA_NODE_INFO));
	}
	else
	{
		//dump the structure in a file
		fp = fopen("/tmp/moca_node_state", "wb+");
		if(fp)
		{
			fwrite(&local_node, sizeof(MOCA_NODE_INFO), 1, fp);
			fclose(fp);
		}
	}
}

void moca_mib(MOCA_MIB_DATA *mib)
{
	MOCA_MIB_DATA mib_data;

	memset(&mib_data, 0, sizeof(mib_data));
	_moca_cmd(MOCA_CMD_GET_MIB_DATA, &mib_data);
	_dump_mib(&mib_data);
	if(mib)
	{
		memcpy(mib, &mib_data, sizeof(MOCA_MIB_DATA));
	}
}

static void _interrupt_handler(int sig)
{
	if(sig == SIGTERM || sig == SIGINT)
	{
		moca_monitor_stop = 1;
	}
}

static int _handle_moca_if_status(const int conn)
{
	int flags = 0;
	if(!_ifconfig_get(nvram_safe_get("moca_ifname"), &flags, NULL, NULL, NULL, NULL))
	{
		if((flags & IFF_UP) && conn != MOCA_CONN_STATE_PAIRED)
		{
			flags &= ~IFF_UP;
			if(!ifconfig(nvram_safe_get("moca_ifname"), flags, NULL, NULL))
			{
				_dprintf("[%s]Set %s as DOWN!\n", __FUNCTION__, nvram_safe_get("moca_ifname"));
			}
			else
			{
				return -1;
			}
		}
		else if(!(flags & IFF_UP) && conn == MOCA_CONN_STATE_PAIRED)
		{
			flags |= IFF_UP;
			if(!ifconfig(nvram_safe_get("moca_ifname"), flags, NULL, NULL))
			{
				_dprintf("[%s]Set %s as UP!\n", __FUNCTION__, nvram_safe_get("moca_ifname"));
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

static int _handle_moca_if_bridge()
{
	char path[256], brif[IFNAMSIZ];
	static int moca_br_init;
	moca_br_init  = nvram_get_int("moca_br_init");

	if(moca_br_init && nvram_match("x_Setting", "1"))
	{
		nvram_unset("moca_br_init");
		moca_br_init = 0;
		strlcpy(brif, nvram_safe_get("lan_ifname"), sizeof(brif));
		snprintf(path, sizeof(path), "/sys/class/net/%s/brif/%s", brif, nvram_safe_get("moca_ifname") );
		if(!check_if_dir_exist(path))
		{
			eval("brctl", "addif", brif, nvram_safe_get("moca_ifname"));
		}
	}
	return 0;
}

int moca_monitor_main(int argc, char *argv[])
{
	FILE *fp;
	const static int period = 1;

	signal(SIGUSR1, _interrupt_handler);

	/* write pid */
	if ((fp = fopen("/var/run/moca_monitor.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	while(!moca_monitor_stop)
	{
		if(nvram_get_int("moca_dev_state") == MOCA_STATE_DEV_READY)
		{
			if(nvram_match("moca_mps_trigger", "1"))
			{
				_moca_cmd(MOCA_CMD_GET_MPS_REPORT, NULL);
			}
			else
			{
				_moca_cmd(MOCA_CMD_GET_CONN_STATE, NULL);
				moca_local_node_state(NULL);
			}
		}
		_handle_moca_if_status(nvram_get_int("moca_conn_state"));
		_handle_moca_if_bridge();
		sleep(period);
	}
	return 0;
}

void get_moca_devices(MOCA_NODE_INFO *moca_devices, int max_devices, int port_idx)
{
	MOCA_ONE_NODE_INFO node[MAX_MOCA_NODES];
	int i;
	char mac[18] = {0};
	// dean : Please fill current node information, currently max_devices is always 1 and port_idx is unused.
	// port_idx is not used currently
	if (moca_devices) {
		memset(moca_devices, 0, sizeof(MOCA_NODE_INFO));
		memset(node, 0, sizeof(node) );
		if(nvram_get_int("moca_conn_state") != MOCA_CONN_STATE_PAIRED)
			return;
		moca_node_state(node, MAX_MOCA_NODES);
		//_dump_node_array(node, MAX_MOCA_NODES);
		strlcpy(mac, nvram_safe_get("et0macaddr"), sizeof(mac));
		for(i = 0; i < MAX_MOCA_NODES; ++i)
		{
			if(!strcasecmp(node[i].macaddr, mac))
			{
				moca_devices->active = node[i].active;
				moca_devices->node_id = node[i].node_id;
				memcpy(moca_devices->macaddr,  node[i].macaddr, sizeof(char) * 18);
				memcpy(moca_devices->moca_ver, node[i].moca_ver, sizeof(char) * 8);
				memcpy(moca_devices->phyrate, node[i].phyrate, sizeof(int) * MAX_MOCA_NODES);
			}
			memcpy(moca_devices->node_mac[i], node[i].macaddr, sizeof(char) * 18);
			memcpy(moca_devices->node_moca_ver[i], node[i].moca_ver, sizeof(char) * 8);
			memcpy(&(moca_devices->rx_snr[i]), &(node[i].rx_snr), sizeof(char) * MAX_MOCA_NUM_CHANNELS);
		}
		//_dump_node_info(moca_devices);
	}
}

#ifdef RTCONFIG_NEW_PHYMAP
void get_moca_status(phy_info_list *list)
{
	// dean : Please fill current node state(up or down), link_rate(by moca version?), mib data
	int i, j;
	char cap_buf[64] = {0};
	phy_port_mapping port_mapping;
	MOCA_MIB_DATA mib;
	MOCA_NODE_INFO moca_node;
	if (!list)
		return;
	get_phy_port_mapping(&port_mapping);
	// Clean all state and dupex.
	for (i = 0; i < port_mapping.count; i++) {
		if ((port_mapping.port[i].cap & PHY_PORT_CAP_MOCA) > 0) {
			if (list->phy_info[i].cap == 0) {
				snprintf(list->phy_info[i].label_name, sizeof(list->phy_info[i].label_name), "%s", 
					port_mapping.port[i].label_name);
				list->phy_info[i].cap = port_mapping.port[i].cap;
				snprintf(list->phy_info[i].cap_name, sizeof(list->phy_info[i].cap_name), "%s", 
					get_phy_port_cap_name(port_mapping.port[i].cap, cap_buf, sizeof(cap_buf)));
				list->count++;
			}
			snprintf(list->phy_info[i].duplex, sizeof(list->phy_info[i].duplex), "none");

			if (nvram_get_int("moca_conn_state") == 1) {
				snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "up");
				get_moca_devices(&moca_node, 0, 0);
				for(j = 0; j < MAX_MOCA_NODES; ++j)
				{
					if(moca_node.node_id != j && moca_node.phyrate[j] != 0)
					{
						list->phy_info[i].link_rate = moca_node.phyrate[j] > 1000? 2500: 1000;
						break;
					}
				}
				if (list->status_and_speed_only == 0) {
					moca_mib(&mib);
					list->phy_info[i].tx_bytes = mib.tx_bytes;
					list->phy_info[i].rx_bytes = mib.rx_bytes;
					list->phy_info[i].tx_packets = mib.tx_packets;
					list->phy_info[i].rx_packets = mib.rx_packets;
					list->phy_info[i].crc_errors = mib.rx_crc;
				}
			} else {
				snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
				list->phy_info[i].link_rate = 0;
			}
		}
	}
}
#endif

int create_moca_log(const char *log_path)
{
	char cmd[512];
	if(!log_path)
		return -1;

	//remove exist file
	unlink(log_path);
	f_write_string(log_path, "<clnkstat -d>\n", FW_APPEND, 0);
	snprintf(cmd, sizeof(cmd), "clnkstat -d >> %s", log_path);
	system(cmd);

	f_write_string(log_path, "\n\n<clnkstat>\n", FW_APPEND, 0);
	snprintf(cmd, sizeof(cmd), "clnkstat >> %s", log_path);
	system(cmd);

	f_write_string(log_path, "\n\n<clnkstat -p>\n", FW_APPEND, 0);
	snprintf(cmd, sizeof(cmd), "clnkstat -p >> %s", log_path);
	system(cmd);

	f_write_string(log_path, "\n\n<clnkstat -n>\n", FW_APPEND, 0);
	snprintf(cmd, sizeof(cmd), "clnkstat -n >> %s", log_path);
	system(cmd);

	f_write_string(log_path, "\n\n<clnkstat --fmr2>\n", FW_APPEND, 0);
	snprintf(cmd, sizeof(cmd), "clnkstat --fmr2 >> %s", log_path);
	system(cmd);

	return 0;
}
