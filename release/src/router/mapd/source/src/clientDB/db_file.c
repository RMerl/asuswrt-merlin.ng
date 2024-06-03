#include "includes.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include <assert.h>
#include <libgen.h>
#include "db.h"
#include "steer_action.h"
#ifdef SUPPORT_MULTI_AP
#include "ch_planning.h"
#endif
#include "wapp_if.h"

static void write_mac(FILE *f, struct client *cli)
{
    char value[20];
    int res;

    res = os_snprintf(value, 20, MACSTR, MAC2STR(cli->mac_addr));
    if (os_snprintf_error(20, res)) {
        mapd_ASSERT(0);
    }
    value[20 - 1] = '\0';
	fprintf(f, "%s", value);
}

static int read_mac(u8 *mac_addr, int line,
        const char *buffer)
{
    if (hwaddr_aton(buffer, mac_addr)) {
        mapd_printf(MSG_ERROR, "Line %d: Invalid mac '%s'.",
                line, buffer);
        return -1;
    }
    mapd_printf(MSG_DEBUG, "MAC parsed succesfully = " MACSTR, MAC2STR(mac_addr));
    return 0;
}

int mapd_client_db_read(struct mapd_global *global)
{
	FILE *f;
	char buf[512] = {0}, *pos = NULL, *token = NULL;
	u8 tmpbuf[256] = {0};
	int line = 0;
	uint32_t num_clients = 0;
	uint32_t client_id = (uint32_t)-1;
	struct client *cli = NULL;
	u8 mac_addr[ETH_ALEN] = {0};
	const char *name = global->params.clientDBname;

	if (name == NULL) {
		mapd_printf(MSG_ERROR, "No DB file?");
		return -1;
	}
	if (global == NULL)
		return -1;

	f = fopen(name, "r");
	if (f == NULL) {
		mapd_printf(MSG_ERROR, "Failed to open DB file '%s', "
				"error: %s", name, strerror(errno));
		return -1;
	}


	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
	while (mapd_config_get_line(buf, sizeof(buf), f, &line, &pos)) {
		//os_strcpy(tmpbuf, pos);
		token = strtok(pos, ";");
		if (token != NULL) {
			/* Ignore the comments */
			if(os_strncmp(token, "#", 1) == 0)
				continue;

			/* Reading client id */
			if (os_strncmp(token, "BS", 2) == 0) {
				token += 2;
				if((*token) != '\0')
					client_id = atoi(token);
				if ((client_id > MAX_STA_SEEN) || (client_id == (uint32_t)-1)) {
					mapd_printf(MSG_INFO, "Invalid client id=%d\n", client_id);
					continue;
				}
			}

			cli = &global->dev.client_db[client_id];

			/* Reading  client MAC address*/
			token = strtok(NULL, ";");
			if(token){
				if (read_mac(mac_addr, line, token)) {
					mapd_printf(MSG_INFO, "Invalid MAC addr");
					continue;
			}}
			os_memcpy(cli->mac_addr, mac_addr, ETH_ALEN);

			/* Reading Known channel bitmap */
			token = strtok(NULL, ";");
			if(token)
				cli->known_channels[0] = (uint8_t)atoi(token);

			token = strtok(NULL, ";");
			if(token)
				cli->known_channels[1] = (uint8_t)atoi(token);
		
			token = strtok(NULL, ";");
			if(token)
				cli->known_channels[2] = (uint8_t)atoi(token);

			token = strtok(NULL, ";");
			if(token)
				cli->known_channels[3] = (uint8_t)atoi(token);

			token = strtok(NULL, ";");
			if(token)
				cli->known_channels[4] = (uint8_t)atoi(token);

			/* Reading VHT/HT cap */
			token = strtok(NULL, ";");
			if(token)
				cli->ht_vht_he_cap = (uint8_t)atoi(token);

			/*Reading Max 2.4 GHz BW*/
			token = strtok(NULL, ";");
			if(token)
				cli->phy_capab.max_bw[0] = atoi(token);

			/*Reading Max 5 GHz BW*/
			token = strtok(NULL, ";");
			if(token)
				cli->phy_capab.max_bw[1] = atoi(token);

			/* Reading CSBC BTM state */
			token = strtok(NULL, ";");
			if(token)
				cli->csbc_data.btm_state = atoi(token);

			/* Reading CSBC Forced Str substate */
			token = strtok(NULL, ";");
			if(token)
				cli->csbc_data.force_str_state = atoi(token);

			/* Reading  BCu */
			token = strtok(NULL, ";");
			if(token)
				cli->csbc_data.BCu = atoi(token);

			/* Reading	BCi */
			token = strtok(NULL, ";");
			if(token)
				cli->csbc_data.BCi = atoi(token);

			/* Reading	BCa */
			token = strtok(NULL, ";");
			if(token) {
				if (cli->csbc_data.btm_state == CSBC_BTM_ACTIVE_ALLOWED)
					cli->csbc_data.consec_btm_act_fail_cnt = atoi(token);
				else
					cli->csbc_data.BCa = atoi(token);
			}

			/*Reading Phy Caps */
			/* 2G Valid */
			token = strtok(NULL, ";");
			if(token)
				cli->phy_cap_known[0] = (uint8_t)atoi(token);
			/* 5G Valid */
			token = strtok(NULL, ";");
			if(token)
				cli->phy_cap_known[1] = (uint8_t)atoi(token);
			/* Phy Mode 2.4 GHz*/
			token = strtok(NULL, ";");
			if(token)
				cli->phy_capab.phy_mode[0] = atoi(token);
			/* Phy Mode 5 GHz */
			token = strtok(NULL, ";");
			if(token)
				cli->phy_capab.phy_mode[1] = atoi(token);
			/* Num_sp_streams */
			token = strtok(NULL, "");
			if(token)
				cli->phy_capab.num_sp_streams = atoi(token);

			num_clients++;

			/* Need to update the client DB for the client id */
			cli->client_id = client_id;
			cli->in_db = IN_DB;
			/* Client at top of the file is the oldest. Add
			 * it to the head */
			dl_list_add(&global->dev.sta_seen_list, &cli->sta_seen_entry);
			/* Update CSBC from past learning */
			steer_action_csbc_init_from_db(global, cli);

		}

	}
	mapd_printf(MSG_ERROR, "Number of clients read from DB file =%d", num_clients);

	fclose(f);
	return 0;
}

static void mapd_update_client_file_entry(FILE *f, struct client *cli)
{
	/* Format of string that can be write on file is 
  	  * BS<client_id 3>;<MAC in : format e.g. XX:XX:XX>;<known channels 5>; /
 	  * <phy mode 1>;<Max 2g BW 1>;<Max 5g BW 1>;<CSBC state 1>;<CSBC substate 1>; /
 	  * <Bcu 3>;<BCi 3>;<BCa 3>\n */

	/* Write CLient ID, Format-> BS<clientid>: */
	fprintf(f, "BS%d;", cli->client_id);

	/* Write Mac Addr, Format-> xx:xx:xx:xx:xx:xx */
	write_mac(f, cli);
	fprintf(f,";");

	/* Write known channels bytes, Format->XXXXX */
	fprintf(f,"%d;%d;%d;%d;%d;",cli->known_channels[0],cli->known_channels[1],
	cli->known_channels[2],cli->known_channels[3],cli->known_channels[4]);

	/* Write ht/vht vap, Format-> X*/
	fprintf(f,"%d;",cli->ht_vht_he_cap);

	/* Write Max 2.4 G BW, Format-> X */
	fprintf(f,"%d;",cli->phy_capab.max_bw[0]);

	/* Write Max 5G BW, Format-> X */
	fprintf(f,"%d;",cli->phy_capab.max_bw[1]);

	/* Write CSBC state, Formate-> X */
	fprintf(f,"%d;",cli->csbc_data.btm_state);

	/* Write CSBC sub state, Formate-> X */
	fprintf(f,"%d;",cli->csbc_data.force_str_state);

	/* Write BCu, Formate->X*/
	fprintf(f,"%d;",cli->csbc_data.BCu);

	/* Write BCi, Formate->X*/
	fprintf(f,"%d;",cli->csbc_data.BCi);

	/* Write BCa, Formate->X*/
	if (cli->csbc_data.btm_state == CSBC_BTM_ACTIVE_ALLOWED)
		fprintf(f,"%d;",cli->csbc_data.consec_btm_act_fail_cnt);
	else
		fprintf(f,"%d;",cli->csbc_data.BCa);

	/* Write Max 2.4G Phy_Caps Valid, Format-> X */
	fprintf(f,"%d;",cli->phy_cap_known[0]);
	/* Write Max 5G Phy_Caps Valid, Format-> X */
	fprintf(f,"%d;",cli->phy_cap_known[1]);
	/* Write Max 2.4G PM, Format-> X */
	fprintf(f,"%d;",cli->phy_capab.phy_mode[0]);
	/* Write Max 5G PM, Format-> X */
	fprintf(f,"%d;",cli->phy_capab.phy_mode[1]);
	/* Write num_sp_streams, Format-> X */
	fprintf(f,"%d\n",cli->phy_capab.num_sp_streams);
}

int mapd_write_client_db_entry(struct mapd_global *global, struct client *cli)
{
	FILE *f = NULL;
	int ret = 0;
	const char *name = global->params.clientDBname;
	const char *orig_name = name;

	if(cli == NULL)
		return -1;

	f = fopen(name, "a");
	if (f == NULL) {
		mapd_printf(MSG_ERROR, "Failed to open '%s' for writing", name);
		return -1;
	}

	/* Update the file entry with client detail */
	mapd_update_client_file_entry(f,cli);

	/* Close the file */
	os_fdatasync(f);
	fclose(f);


	mapd_printf(MSG_EXCESSIVE, "DB file '%s' written %ssuccessfully",
		   orig_name, ret ? "un" : "");
	return ret;
}

int mapd_client_db_flush(struct mapd_global *global, uint8_t all)
{
	int ret = 0;
	struct client *cli = NULL;
	FILE *tf = NULL;
	const char *name = global->params.clientDBname;
	char *dbnamelocal = strdup(name);
	char *dbnamebase = NULL;
	char temp[256] = {0};
	int len = 0;
	uint8_t write_to_db = 0;

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
		free(dbnamelocal);	
		return 0;
	}

	/* Much More efficient to flush the entire list, instead of searching the
	 * changed record(s) and updating the file */
	if (!all) {
		mapd_printf(MSG_DEBUG, "Note 'all' parameter is 0, db has changed, "
						"but will dump the entire"
						"list to %s", name);
	}

	dbnamebase = dirname(dbnamelocal);
	os_snprintf(temp, sizeof(temp),  "%s",dbnamebase);
  	len = os_strlen(temp);
  	if(len < (sizeof(temp) - 1))
		os_snprintf(temp + len, sizeof(temp) - len, "/temp.txt");

	tf = fopen(temp, "w");
	if (tf == NULL) {
		mapd_printf(MSG_ERROR, "Failed to open temp file '%s' for writing", temp);
		free(dbnamelocal);		
		return -1;
	}

	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
			if (cli->in_db == NOT_IN_DB)
				continue;
			if (cli->in_db == IN_DB_DEL) {
				cli->in_db = NOT_IN_DB;
				continue;
			}
			mapd_update_client_file_entry(tf, cli);
			cli->dirty = 0;
	}

	fclose(tf);

	remove(name);

	if(rename(temp, name) != 0)
		mapd_printf(MSG_ERROR, "File rename fail \n");

	free(dbnamelocal);

	mapd_printf(MSG_INFO, "Persistent Client Database updated");

	return ret;

}

int mapd_update_client_db_entry(struct mapd_global *global, struct client *cli)
{
	FILE *f = NULL, *tf = NULL;
	char buf[512], *pos, *token;
	char tmpbuf[256];
	const char *name = global->params.clientDBname;
	const char temp[] = "temp.txt";
	int line = 0;
	uint32_t client_id = (uint32_t)-1;

	if(cli == NULL)
		return -1;

	f = fopen(name, "r");
	if (f == NULL) {
		//mapd_printf(MSG_TRACE, "Failed to open '%s' for writing", name);
		return -1;
	}

	tf = fopen(temp, "w");
	if (tf == NULL) {
		mapd_printf(MSG_ERROR, "Failed to open temp file '%s' for writing", temp);
		fclose(f);
		return -1;
	}

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);

	while (mapd_config_get_line(buf, sizeof(buf), f, &line, &pos)) {
		os_snprintf(tmpbuf, sizeof(tmpbuf), "%s", (const char *)pos);
		token = strtok(pos, ";");
		if (token != NULL) {

			/* Ignore the comments */
			if(os_strncmp((const char *)token, "#", 1) == 0)
				continue;

			/* Reading client id */
			if (os_strncmp((const char *)token, "BS", 2) == 0) {
				token += 2;
				client_id = atoi((const char *)token);
				if ((client_id > MAX_STA_SEEN) || (client_id == (uint32_t)-1)) {
					mapd_printf(MSG_INFO, "Invalid client id=%d\n", client_id);
					continue;
				}
			}

			/* Entry is valid and desired */
			if (client_id == cli->client_id) {

				/* Update the file entry with client detail */
				mapd_update_client_file_entry(tf,cli);
			} else {

				/* Entry is valid but not desired so directly move to tmp file */
				fprintf(tf, "%s\n", tmpbuf);
			}

		}

	}

	fclose(f);
	fclose(tf);

	remove((const char *)name);
	if(rename((const char *)temp, (const char *)name) != 0)
		mapd_printf(MSG_ERROR, "File rename fail \n");

	mapd_printf(MSG_EXCESSIVE, "Updated client %d entry successfully", cli->client_id);
	return 0;
}
