#include "includes.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include <assert.h>
#include "db.h"
#include "steer_action.h"
#include "nvram.h"

static int read_mac(u8 *mac_addr, const char *buffer)
{
    if (hwaddr_aton(buffer, mac_addr)) {
        mapd_printf(MSG_ERROR, "Invalid mac '%s'.",
                buffer);
        return -1;
    }
    mapd_printf(MSG_DEBUG, "MAC parsed succesfully = " MACSTR, MAC2STR(mac_addr));
    return 0;
}

int mapd_client_db_read(struct mapd_global *global)
{
	uint32_t i = 0;
	char buf[200];
	char tmp_buf[100];
	char *nl = NULL;
	FILE *fp;
	struct client *cli = NULL;
	char *token = NULL;
	u8 mac_addr[ETH_ALEN] = {0};
	uint8_t j = 0, k = 0;
	int invalid_mac = 0;
	uint8_t magic_word = 0;

	for (i = 0; i < MAX_STA_SEEN; i++) {

		memset(buf, 0, sizeof(buf));
		memset(tmp_buf, 0, sizeof(tmp_buf));
		os_snprintf(tmp_buf, sizeof(tmp_buf), "nvram_get 2860 BS%d", i);
		
		if( (fp = popen(tmp_buf, "r")) == NULL )
				break;
		if(!fgets(buf, sizeof(buf), fp)){
				pclose(fp);
				break;
		}

		if(!strlen(buf)){
				pclose(fp);
				break;
		}
		pclose(fp);
		if((nl = strchr(buf, '\n')) != NULL)
				*nl = '\0';
		else {
			mapd_printf(MSG_ERROR, "Something wrong");
			break;
		}
		mapd_printf(MSG_DEBUG, "%s", buf);


		// Returns first token 
		token = strtok(buf, ";");
		if(token)
			magic_word = atoi(token);
		else
			continue;

		if(magic_word != 128)
			continue;

		cli = &global->dev.client_db[i];
		token = strtok(NULL, ";");
		k = 0;
		j = 0;
		invalid_mac = 0;
		while (token != NULL)
		{
				j++;
				switch(j) {
					case 1:
						mapd_printf(MSG_DEBUG, "mac_addr = %s", token);
						if (read_mac(mac_addr, token)) {
								mapd_printf(MSG_INFO, "Invalid MAC addr");
								invalid_mac = 1;
								break;
						}
						os_memcpy(cli->mac_addr, mac_addr, ETH_ALEN);
						break;
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
						mapd_printf(MSG_DEBUG, "known_channels[%d] = %s", k, token);
						cli->known_channels[k++] = atoi(token);
						break;
					case 7:
						mapd_printf(MSG_DEBUG, "HT/VHT/HE Cap = %s", token);
						cli->ht_vht_he_cap = atoi(token);
						break;
					case 8:
						mapd_printf(MSG_DEBUG, "max_bw(2.4G) = %s", token);
						cli->phy_capab.max_bw[0] = atoi(token);
						break;
					case 9:
						mapd_printf(MSG_DEBUG, "max_bw(5G) = %s", token);
						cli->phy_capab.max_bw[1] = atoi(token);
						break;
					case 10:
						mapd_printf(MSG_DEBUG, "csbc_btm_state = %s", token);
						cli->csbc_data.btm_state = atoi(token);
						break;
					case 11:
						mapd_printf(MSG_DEBUG, "csbc_force_str_state = %s", token);
						cli->csbc_data.force_str_state = atoi(token);
						break;
					case 12:
						mapd_printf(MSG_DEBUG, "BCu = %s", token);
						cli->csbc_data.BCu = atoi(token);
						break;
					case 13:
						mapd_printf(MSG_DEBUG, "BCi = %s", token);
						cli->csbc_data.BCi = atoi(token);
						break;
					case 14:
						mapd_printf(MSG_DEBUG, "BCa = %s", token);
						if (cli->csbc_data.btm_state == CSBC_BTM_ACTIVE_ALLOWED)
							cli->csbc_data.consec_btm_act_fail_cnt = atoi(token);
						else
							cli->csbc_data.BCa = atoi(token);
						break;
					case 15:
						mapd_printf(MSG_DEBUG, "PHY_CAP_VALID(2.4G) = %s", token);
						cli->phy_cap_known[0] = atoi(token);
						break;
					case 16:
						mapd_printf(MSG_DEBUG, "PHY_CAP_VALID(5G) = %s", token);
						cli->phy_cap_known[1] = atoi(token);
						break;
					case 17:
						mapd_printf(MSG_DEBUG, "PHY_MODE(2.4G) = %s", token);
						cli->phy_capab.phy_mode[0] = atoi(token);
						break;
					case 18:
						mapd_printf(MSG_DEBUG, "PHY_MODE(5G) = %s", token);
						cli->phy_capab.phy_mode[1] = atoi(token);
						break;
					case 19:
						mapd_printf(MSG_DEBUG, "NSS = %s", token);
						cli->phy_capab.num_sp_streams = atoi(token);
						break;
					}
				if (invalid_mac)
					break;
				token = strtok(NULL, ";");
		}
		/* Need to update the client DB for the client id */
		if (!invalid_mac) {
			cli->client_id = i;
			cli->in_db = IN_DB;
			dl_list_add(&global->dev.sta_seen_list, &cli->sta_seen_entry);
			/* Update CSBC from past learning */
			steer_action_csbc_init_from_db(global, cli);

		}
	}
	return 0;
}

int mapd_update_client_db_entry(struct mapd_global *global, struct client *cli)
{
	/* Update is same as write for nvram */
	mapd_write_client_db_entry(global, cli);
	return 0;
}

int mapd_write_client_db_entry(struct mapd_global *global, struct client *cli)
{
	char cmd[200];
	char *pos = cmd;
	char *end = cmd + 200;
	int ret = 0;

	ret = os_snprintf(pos, end - pos, "nvram_set 2860 BS%d \"128;" MACSTR
					";%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\" &",
					cli->client_id, MAC2STR(cli->mac_addr), cli->known_channels[0],
					cli->known_channels[1], cli->known_channels[2], cli->known_channels[3],
					cli->known_channels[4], cli->ht_vht_he_cap, cli->phy_capab.max_bw[0],
					cli->phy_capab.max_bw[1],cli->csbc_data.btm_state,
					cli->csbc_data.force_str_state, cli->csbc_data.BCu,
					cli->csbc_data.BCi,
					((cli->csbc_data.btm_state == CSBC_BTM_ACTIVE_ALLOWED) ?
					cli->csbc_data.consec_btm_act_fail_cnt : cli->csbc_data.BCa),
					cli->phy_cap_known[0], cli->phy_cap_known[1],
					cli->phy_capab.phy_mode[0], cli->phy_capab.phy_mode[1], cli->phy_capab.num_sp_streams);
	if (os_snprintf_error(end - pos, ret)) {
		return -1;
	}
	mapd_printf(MSG_DEBUG, "%s", cmd);
	system(cmd);
	return ret;
}

int mapd_client_db_flush(struct mapd_global *global, uint8_t all)
{
	int ret = 0;
	struct client *cli = NULL;
	uint8_t write_to_db = 0;
	char val[200];
	char attr[10];

	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
		if (cli->in_db == NOT_IN_DB)
			continue;
		if (!all && !cli->dirty && (cli->in_db != IN_DB_DEL))
			continue;
		write_to_db = 1;
		break;
	}

	if (write_to_db == 0) {
		mapd_printf(MSG_INFO, "No write required to Persistent Client Database");
		return 0;
	}
	nvram_init(RT2860_NVRAM);

	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
		if (cli->in_db == NOT_IN_DB)
			continue;
		if (!all && !cli->dirty && (cli->in_db != IN_DB_DEL))
			continue;
		os_snprintf(attr, sizeof(attr), "BS%d", cli->client_id);
		if (cli->in_db == IN_DB) {
			os_snprintf(val, sizeof(val), "128;" MACSTR
					";%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
					MAC2STR(cli->mac_addr), cli->known_channels[0], 
					cli->known_channels[1], cli->known_channels[2], cli->known_channels[3],
					cli->known_channels[4], cli->ht_vht_he_cap, cli->phy_capab.max_bw[0],
					cli->phy_capab.max_bw[1],cli->csbc_data.btm_state,
					cli->csbc_data.force_str_state, cli->csbc_data.BCu,
					cli->csbc_data.BCi,
					((cli->csbc_data.btm_state == CSBC_BTM_ACTIVE_ALLOWED) ?
					cli->csbc_data.consec_btm_act_fail_cnt : cli->csbc_data.BCa),
					cli->phy_cap_known[0], cli->phy_cap_known[1],
					cli->phy_capab.phy_mode[0], cli->phy_capab.phy_mode[1], cli->phy_capab.num_sp_streams);
		} else  if (cli->in_db == IN_DB_DEL) {
			os_snprintf(val, sizeof(val), " ");
			cli->in_db = NOT_IN_DB;
		}

		nvram_bufset(RT2860_NVRAM, attr, val);
		cli->dirty = 0;
	}

	nvram_commit(RT2860_NVRAM);
	nvram_close(RT2860_NVRAM);

	mapd_printf(MSG_INFO, "Persistent Client Database updated");
	return ret;
}
