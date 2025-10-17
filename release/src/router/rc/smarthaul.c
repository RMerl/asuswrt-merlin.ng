/*
	nvram list:
	nvram name				default value	comment
	sh_algo_type			2				1: static anchor link, 2: dynamic anchor link, 0: disable smarthaul
	sh_algo_txop_hi			60
	sh_algo_txop_lo			40
	sh_algo_anchor_band		0				link index(0~2), not band(2/5/6)
	sh_algo_cycle_ms		3000
	sh_algo_min_cnt			0				minimum cycle in a row to change tidmap
	sh_algo_disable_band	N/A				valuse: 2/5/6, froced to set tidmap of specific band as 0x00
	sh_algo_stop			N/A				1: pause algorithm in next cycle and reset tidmap
	sh_algo_tx_sum			20				only allow high retry link be added back when sum of all link's tx and inbss above this value
	sh_algo_obss			N/A				the function of this nvram is replaced by the two nvram below
	sh_algo_obss_6G			3				the obss multiplier of 6G band (on BQ16 pro, this default value is 6)
	sh_algo_obss_5G			3				the obss multiplier of 5G and 2G band
	sh_algo_retry			0				if the retry of 2G link above this threshold, remove it and mark as high retry link
	smarthaul_msglevel		0x01			set to 0xff to enable all smarthaul debug log
 */
#include <stdio.h>
#include <stdlib.h>
#include <ethernet.h>
#include <sys/time.h>
#include "smarthaul.h"
#include <rc.h>
#include <json.h>
#include <sys/un.h>
#include <pthread.h>


#define SH_ROUNDUP(x, y)		((((x) + ((y) - 1)) / (y)) * (y))
#define MAP_MAX_AGENT_COUNT 6u
#define ETHER_ADDR_STR_LEN	18
#define NBBY    8
#define SH_IDX_MASK_LEN	(SH_ROUNDUP(MAP_MAX_AGENT_COUNT, NBBY) / NBBY)
#define SETBIT(a, i)    (((uint8 *)a)[(i) / NBBY] |= 1 << ((i) % NBBY))
#define CLRBIT(a, i)    (((uint8 *)a)[(i) / NBBY] &= ~(1 << ((i) % NBBY)))
#define ISSET(a, i)     (((const uint8 *)a)[(i) / NBBY] & (1 << ((i) % NBBY)))
#define ISCLR(a, i)     ((((const uint8 *)a)[(i) / NBBY] & (1 << ((i) % NBBY))) == 0)

#define SH_STATS_FLAG_SCB (1 << 1)

uint8 sh_idx_bitvec[SH_IDX_MASK_LEN];

sh_repeater_node_t	*active_bh;
smarthaul_t			g_smarthaul;

static int thread_term = 0;

static bool timer_running = FALSE;
static bool sh_stopped = FALSE;
sh_algo_info_t *algo_info;

static struct sh_algo g_algo;

/* forward function declaration for algo type 1 */
static int sh_ls_prep_1(struct sh_mbh *mbh);
static int sh_ls_expand_score_1(struct sh_mbh *mbh, uint8 link_id);
static int sh_ls_reduce_score_1(struct sh_mbh *mbh, uint8 link_id);

/* forward function declaration for algo type 2 */
static int sh_ls_prep_2(struct sh_mbh *mbh);
static int sh_ls_expand_score_2(struct sh_mbh *mbh, uint8 link_id);
static int sh_ls_reduce_score_2(struct sh_mbh *mbh, uint8 link_id);

const static struct sh_ls_algo g_sh_ls_algo[] = {
	{1, sh_ls_prep_1, sh_ls_expand_score_1, sh_ls_reduce_score_1},
	{2, sh_ls_prep_2, sh_ls_expand_score_2, sh_ls_reduce_score_2},
	{0, NULL, },
};

struct eventHandler
{
    int event_id;
    int (*func)(void *arg, char *data);
};

struct eventHandler SH_CFG_EVENTS[] = {
	{ EID_SH_STA_ADD, sh_sta_add },
    { EID_SH_STA_UPD, sh_sta_upd },
    { EID_SH_STA_DEL, sh_sta_del }
};

void sh_ipc_socket_thread(smarthaul_t *smarthaul)
{
    pthread_t thread;
    pthread_attr_t attr;

    SMARTHAUL_TRACE("Start ipc socket thread.\n");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, NULL, (void *)&sh_start_ipc_socket, (void*)smarthaul);
    pthread_attr_destroy(&attr);
}

int sh_start_ipc_socket(void* arg)
{

    smarthaul_t *smarthaul = (smarthaul_t *)arg;

    struct sockaddr_un addr;
    int sockfd, newsockfd;

    if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	SMARTHAUL_ERROR("ipc create socket error!\n");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SH_IPC_SOCKET_PATH, sizeof(addr.sun_path)-1);

    unlink(SH_IPC_SOCKET_PATH);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        SMARTHAUL_ERROR("ipc bind socket error!\n");
		exit(-1);
    }

    if (listen(sockfd, SH_IPC_MAX_CONNECTION) == -1) {
        SMARTHAUL_ERROR("ipc listen socket error!\n");
		exit(-1);
    }

    while (!thread_term) {
	    SMARTHAUL_TRACE("smarthaul ipc accept socket...\n");
        if ( (newsockfd = accept(sockfd, NULL, NULL)) == -1) {
		    SMARTHAUL_ERROR("ipc accept socket error!\n");
            continue;
        }

        sh_ipc_receive(smarthaul, newsockfd);
        close(newsockfd);

	}

	return 0;
}

void sh_ipc_receive(void* arg, int sockfd)
{
    smarthaul_t *smarthaul = (smarthaul_t *)arg;

	int length = 0;
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	if ((length = read(sockfd, buf, sizeof(buf))) <= 0)
	{
		SMARTHAUL_ERROR("ipc read socket error!\n");
		return;
	}

	SMARTHAUL_TRACE("IPC Receive: %s <<< RCV EVENT >>>\n", buf);

	json_object *rootObj = json_tokener_parse(buf);
	json_object *cfgObj = NULL;
	json_object *eidObj = NULL;
	json_object_object_get_ex(rootObj, "CFG", &cfgObj);
	json_object_object_get_ex(cfgObj, "eid", &eidObj);

	int EID = 0;
	struct eventHandler *handler = NULL;

	if(eidObj) {
		EID = atoi(json_object_get_string(eidObj));
		for(handler = &SH_CFG_EVENTS[0]; handler->event_id > 0; handler++)
		{
			if (handler->event_id == EID)
			break;
		}

		if (handler == NULL || handler->event_id < 0)
			SMARTHAUL_TRACE("no corresponding function pointer(%d)", EID);
		else
		{
			SMARTHAUL_TRACE("process event (%d)\n", EID);
 			if (!handler->func(smarthaul, buf)) {
				SMARTHAUL_ERROR("fail to process event(%d)\n", EID);
			}
		}
	}

	json_object_put(rootObj);
}

int sh_send_tidmap2re(sh_repeater_node_t *dev_info, sh_tidmap_t *tm)
{
    int i,j;
    char re_mac[18], wlc_unitStr[2], json_data[256];

    json_object *root = json_object_new_object();
    json_object *shObj = json_object_new_object();
    json_object *mapObj = json_object_new_object();

    for(i = 0; i < dev_info->mlo_info.mlo_num_links; i++){
        for(j = 0; j < dev_info->mlo_info.mlo_num_links; j++){
            if(dev_info->mlo_info.mlo_link_info[j].link_id == i){
                snprintf(wlc_unitStr, sizeof(wlc_unitStr), "%d", dev_info->mlo_info.mlo_link_info[j].wlc_unit);
                if(tm->tids[j] == 0xff)
                    json_object_object_add(mapObj, wlc_unitStr, json_object_new_int(256));
                else   
                    json_object_object_add(mapObj, wlc_unitStr, json_object_new_int(192));
            }
        }
        
        snprintf(wlc_unitStr, sizeof(wlc_unitStr), "%d", dev_info->mlo_info.mlo_link_info[i].wlc_unit);
        if(tm->tids[i] == 0xff)
            json_object_object_add(mapObj, wlc_unitStr, json_object_new_int(256));
        else   
            json_object_object_add(mapObj, wlc_unitStr, json_object_new_int(192));
    }
    ether_etoa(dev_info->self_mac.octet,re_mac);

    json_object_object_add(shObj, "eid", json_object_new_int(4));
    json_object_object_add(shObj, "re_mac", json_object_new_string(re_mac));
    json_object_object_add(shObj, "tidmap_info", mapObj);
    json_object_object_add(root, "SMARTHAUL", shObj);    

    memset(json_data, 0, sizeof(json_data));
	snprintf(json_data, sizeof(json_data), "%s", json_object_to_json_string(root));
	json_object_put(root);

    return sh_ipc_send_event(CFGMNT_IPC_SOCKET_PATH, &json_data[0]);
}

int sh_ipc_send_event(const char *ipc_path, char *data)
{
    int fd, length;
	int ret = -1;
	struct sockaddr_un addr_un;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		SMARTHAUL_ERROR("ipc socket error!\n");
		goto error;
	}

	memset(&addr_un, 0, sizeof(addr_un));
	addr_un.sun_family = AF_UNIX;
	snprintf(addr_un.sun_path, sizeof(addr_un.sun_path), ipc_path);
	if (connect(fd, (struct sockaddr *)&addr_un, sizeof(addr_un)) < 0) {
		SMARTHAUL_ERROR("ipc connect error\n");
		goto error;
	}

	SMARTHAUL_TRACE("IPC Send: %s  <<< SEND EVENT >>>\n", data);

	length = write(fd, data, strlen(data));

        if(length < 0) {
                SMARTHAUL_ERROR("[%s:(%d)] ERROR writing:%s.\n", __FUNCTION__, __LINE__, strerror(errno));
                goto error;
        }

	ret = SH_OK;

error:
        close(fd);
        return ret;
}

int sh_sta_add(void *arg, char *data)
{
    smarthaul_t *smarthaul = (smarthaul_t *)arg;
    sh_dev_info_t *new_dev;
    new_dev = calloc(1, sizeof(sh_dev_info_t));

    if(!sh_process_ipc_data(new_dev, data)){
        return 0;
    }
	sh_add_dev2list(smarthaul, new_dev);

    free(new_dev);
    return 1;
}

int sh_sta_upd(void *arg, char *data)
{ 
    smarthaul_t *smarthaul = (smarthaul_t *)arg;
    sh_repeater_node_t *curr;
    sh_root_info_t* root_info = (sh_root_info_t *)smarthaul->dev_info;
    sh_dev_info_t *new_dev;
    new_dev = calloc(1, sizeof(sh_dev_info_t));
    char new_mac[18], list_mac[18];
    sh_process_ipc_data(new_dev, data);

    ether_etoa(new_dev->self_mac.octet, new_mac);
    curr = root_info->rpt_list;
    while(curr){
        ether_etoa(curr->self_mac.octet, list_mac);
        if(!strcmp(new_mac, list_mac)){
            SMARTHAUL_TRACE("Update mlo info of %s\n", new_mac);
            if(!memcmp(&curr->mlo_info, &new_dev->mlo_info, sizeof(sh_mlo_info_t))){
                SMARTHAUL_TRACE("No info of %s need to update\n", new_mac);
                return 1;
            }
            if (curr == active_bh){
                sh_deactivate_bh(smarthaul, curr);
                memcpy(&curr->mlo_info, &new_dev->mlo_info, sizeof(sh_mlo_info_t));
                sh_activate_bh(smarthaul, curr, &root_info->local_dev);
            }
            else    
                memcpy(&curr->mlo_info, &new_dev->mlo_info, sizeof(sh_mlo_info_t));

            free(new_dev);
            return 1;
        }
        else{
            curr = curr->next;
        }    
    }

    SMARTHAUL_TRACE("No match node in rpt list, adding dev to list instead of updating\n");
    sh_add_dev2list(smarthaul, new_dev);

    free(new_dev);
    return 1;
}

int sh_sta_del(void *arg, char *data)
{
    smarthaul_t *smarthaul = (smarthaul_t *)arg;
    sh_root_info_t* root_info = (sh_root_info_t *)smarthaul->dev_info;
    sh_repeater_node_t *curr, *pre = NULL;
    char del_mac[18], list_mac[18];

    json_object *rootObj = json_tokener_parse(data);
    json_object *cfgObj = NULL;
    json_object *re_macObj = NULL;
    
    if(rootObj == NULL){
        SMARTHAUL_ERROR("fail to parsing json");
        return 0;
    }

    json_object_object_get_ex(rootObj, "CFG", &cfgObj);
    json_object_object_get_ex(cfgObj, "re_mac", &re_macObj);
    snprintf(del_mac, sizeof(del_mac), json_object_get_string(re_macObj));
    json_object_put(rootObj);

    curr = root_info->rpt_list;
    while(curr){
        ether_etoa(curr->self_mac.octet, list_mac);
        if(!strcmp(del_mac, list_mac)){
            SMARTHAUL_TRACE("Delete all info of %s\n", del_mac);
            //Delete the node from list
            if(curr == root_info->rpt_list)
                root_info->rpt_list = curr -> next;
            else 
                pre -> next = curr->next;

            root_info->num_repeaters--;
            CLRBIT(sh_idx_bitvec, curr->sh_idx);
            dump_sh(smarthaul);

            //Stop the algo if the node is the active_bh
            // if (curr == active_bh){
                sh_deactivate_bh(smarthaul, curr);
                //Activate the algo if there is node in list
                // if (root_info->rpt_list) {
			    //     sh_activate_bh(smarthaul, root_info->rpt_list,
				//         &root_info->local_dev);
		        // }
            // }
            free(curr);
            return 1;
        }
        else{
            pre = curr;
            curr = curr->next;
        }    
    }
    SMARTHAUL_TRACE("%s not found. No RE be deleted\n", del_mac);
    return 0;
}

int sh_process_ipc_data(sh_dev_info_t *new_dev, char *data)
{
    int cnt = 0;
    char cap_bssid[18], mld_mac[18], re_mac[18]/*, wlc_unitStr[2]*/;

    json_object *rootObj = json_tokener_parse(data);
	json_object *cfgObj = NULL;
    json_object *re_macObj = NULL;
    json_object *dev_infoObj = NULL;
    json_object *mld_macObj = NULL;
    json_object *map_unitObj = NULL;
    json_object *mlo_link_infoObj = NULL;
    //json_object *wlc_unitObj = NULL;
    json_object *pap_bssidObj = NULL;

    if(rootObj == NULL){
        SMARTHAUL_ERROR("fail to parsing json");
        return 0;
    }

    json_object_object_get_ex(rootObj, "CFG", &cfgObj);
    json_object_object_get_ex(cfgObj, "re_mac", &re_macObj);
    json_object_object_get_ex(cfgObj, "device_info", &dev_infoObj);
    json_object_object_get_ex(dev_infoObj, "mld_mac", &mld_macObj);
    json_object_object_get_ex(dev_infoObj, "map_unit", &map_unitObj);
    json_object_object_get_ex(dev_infoObj, "mlo_link_info", &mlo_link_infoObj);

    snprintf(re_mac, sizeof(re_mac), json_object_get_string(re_macObj));
    ether_atoe(re_mac, new_dev->self_mac.octet);
    snprintf(mld_mac, sizeof(mld_mac), json_object_get_string(mld_macObj));
    ether_atoe(mld_mac, new_dev->mlo_info.self_mld_addr.octet);
    //char *map_wlc_unit = json_object_get_string(json_object_object_get(root, "map_unit"));
    //idk where it should be for now

    json_object_object_foreach(mlo_link_infoObj, key, value){
        json_object_object_get_ex(value, "pap_bssid", &pap_bssidObj);
        SMARTHAUL_TRACE("pap :%s\n", json_object_to_json_string(pap_bssidObj));
        snprintf(cap_bssid, sizeof(cap_bssid), json_object_get_string(pap_bssidObj));
        ether_atoe(cap_bssid, new_dev->mlo_info.mlo_link_info[cnt].pap_bssid.octet);
        new_dev->mlo_info.mlo_link_info[cnt].wlc_unit = atoi(key);
        new_dev->mlo_info.mlo_link_info[cnt].link_id = cnt; 
        cnt++;    
    }

    new_dev->mlo_info.mlo_num_links = cnt;

    return cnt;
}

void sh_add_dev2list(smarthaul_t *smarthaul, sh_dev_info_t *new_dev)
{
	sh_root_info_t* root_info = (sh_root_info_t *)smarthaul->dev_info;
	sh_repeater_node_t *new_node;
	int i,j;

	SMARTHAUL_TRACE("New added device info\n");
	dump_sh_dev_info(new_dev);

    new_node = calloc(1, sizeof(sh_repeater_node_t));
	memcpy(&new_node->mlo_info, &new_dev->mlo_info, sizeof(sh_mlo_info_t));
	eacopy(new_dev->self_mac.octet, new_node->self_mac.octet);
	for(i = 0; i < root_info->local_dev.mlo_info.mlo_num_links; i++){
		for(j = 0; j < root_info->local_dev.mlo_info.mlo_num_links; j++){
			if(!eacmp(&root_info->local_dev.mlo_info.mlo_link_info[i].pap_bssid, &new_dev->mlo_info.mlo_link_info[j].pap_bssid)){
				memcpy(&new_node->mlo_info.mlo_link_info[i], &new_dev->mlo_info.mlo_link_info[j], sizeof(sh_mlo_link_info_t));
				new_node->mlo_info.mlo_link_info[i].link_id = i;
				continue;
			}
		}
	}
	for (i = 0; i < SH_IDX_MASK_LEN * NBBY; i++) {
		if (ISCLR(sh_idx_bitvec, i)) {
			new_node->sh_idx = i;
			SETBIT(sh_idx_bitvec, i);
			break;
		}
	}
	//ASSERT(i != SH_IDX_MASK_LEN * NBBY);

	if (root_info->rpt_list == NULL) {
        root_info->rpt_list = new_node;
    } else {
        new_node->next = root_info->rpt_list;
        root_info->rpt_list = new_node;
    }
    root_info->num_repeaters++;

    SMARTHAUL_TRACE("Dumping smarthaul info\n");
    dump_sh(smarthaul);

	// if (active_bh == NULL) {
		/* No active link. Record this one for optimization */
		SMARTHAUL_TRACE("Activate SH bSta with MLD: " MACF "\n",
            ETHER_TO_MACF(new_node->mlo_info.self_mld_addr));
		sh_activate_bh(smarthaul, new_node, &root_info->local_dev);
	// } else {
		/* If new repeater is user preferred for SH link optimzation,
		*  evict current active one and add new one.
		*/
		/*if (!eacmp(&new_node->mlo_info.self_mld_addr, &sh_usr_cfg_mld)) {
			SMARTHAUL_TRACE("Deactive SH bSta with MLD: " MACF "\n",
				ETHER_TO_MACF(active_bh->mlo_info.self_mld_addr));
			sh_deactivate_bh(smarthaul, active_bh);

			SMARTHAUL_TRACE("Activate SH bSta with MLD: " MACF "\n",
				ETHER_TO_MACF(new_node->mlo_info.self_mld_addr));
			sh_activate_bh(smarthaul, new_node, &root_info->local_dev);
		}*/
	// }
}

int sh_activate_bh(smarthaul_t *smarthaul, sh_repeater_node_t *new_dev,
	sh_dev_info_t *local_dev)
{
	int ret = SH_OK;

	if (!sh_stopped) {
		//ASSERT(!timer_running);
		SMARTHAUL_SYSLOG("Add bSta: " MACF ", smart haul index : %d\n",
			ETHER_TO_MACF(new_dev->self_mac), new_dev->sh_idx);

		ret = sh_add_bh(new_dev->sh_idx, &local_dev->mlo_info, &new_dev->mlo_info);
		if (ret != SH_OK)
			return ret;

		ret = sh_add_timer((int)algo_info->cycle_ms);

		if (ret != SH_OK) {
			sh_del_bh(new_dev->sh_idx);
			SMARTHAUL_ERROR("Can't set timer. SH feature is not working\n");
			ret = SH_ERROR;
		} else {
			// active_bh = new_dev;
			ret = SH_OK;
		}
	} else {  /* for stoping smarthual */
		// active_bh = new_dev;
		timer_running = FALSE;
		ret = SH_OK;
	}
	return ret;
}

void sh_deactivate_bh(smarthaul_t *smarthaul, sh_repeater_node_t *rpt_node)
{
	/* IMPORTNANT: set active bh to NULL before remove timer */
	int ret = SH_OK;
	// ASSERT(rpt_node == active_bh);
	if (!sh_stopped) {
		SMARTHAUL_SYSLOG("Del bSta: " MACF " smart haul index : %d\n",
			ETHER_TO_MACF(rpt_node->self_mac), rpt_node->sh_idx);
		sh_del_bh(rpt_node->sh_idx);
		active_bh = NULL; /* Reset active repeater */
		ret = sh_remove_timer();
		if (ret == SH_OK)
			timer_running = FALSE;
	}
}

static int sh_mbh_add_link(struct sh_mbh *mbh, int link_id, const sh_mlo_link_info_t *li)
{
	struct sh_link *sl = &mbh->link[link_id];

	//ASSERT(link_id < MAX_MLO_LINKS);
	if (sl->valid)
		return SH_ERROR;

	sl->link_id = link_id;
	sl->wlc_unit = li->wlc_unit;
	//sl->chspec = li->chspec;
	sl->band_ix = li->bandidx;
	sl->mbh = mbh;

	mbh->cfg_linkset |= (1 << link_id);
	mbh->link_cnt++;

	sl->valid = TRUE;
	SMARTHAUL_TRACE("mbh %d add link %u, wl %u, band_ix%u\n", mbh->ix, link_id,
			sl->wlc_unit, sl->band_ix);
	return SH_OK;
}

static int sh_bh_add_links(struct sh_mbh *mbh)
{
	const sh_mlo_info_t *mi = mbh->mlo_info;
	const sh_mlo_link_info_t *li;
	int i, err = SH_OK;

	//ASSERT(mi->mlo_num_links < MAX_MLO_LINKS);
	for (i = 0; i < mi->mlo_num_links; i++) {
		li = &mi->mlo_link_info[i];
		err = sh_mbh_add_link(mbh, i, li);
		if (err) {
			SMARTHAUL_ERROR("mbh %d failed to add link %u, err %d\n",
					mbh->ix, i, err);
			return err;
		}

	}

	mbh->cur_linkset = mbh->cfg_linkset;
	mbh->can_linkset = mbh->cfg_linkset;
	SMARTHAUL_INFO("mbh %d added %d links, linkset %x\n", mbh->ix,
			mbh->link_cnt, mbh->cfg_linkset);
	return SH_OK;
}

int sh_add_bh(uint32 sh_idx, const sh_mlo_info_t *bap, const sh_mlo_info_t *bsta)
{
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *ap_bh =  NULL, *sta_bh = NULL;
	int err = SH_OK;

	SMARTHAUL_TRACE("sh_idx %d, bap %p, bsta %p\n", sh_idx, bap, bsta);

	/* sanity check, no duplicate sh bh is allowed */
	if (sh_algo_find_mbh_by_ix(sh_idx)) {
		SMARTHAUL_ERROR("duplicate sh_idx %d\n", sh_idx);
		return SH_ERROR;
	}

	//ASSERT(bap);
	ap_bh = calloc(1, sizeof(*ap_bh));
	if (!ap_bh)
		return SH_NOMEM;

	if (bsta) {
		sta_bh = calloc(1, sizeof(*sta_bh));
		if (!sta_bh) {
			free(ap_bh);
			return SH_NOMEM;
		}
	}

	ap_bh->algo = algo;
	ap_bh->ix = sh_idx;
	ap_bh->mlo_info = bap;
	ap_bh->is_bap = TRUE;
	ap_bh->exp_cnt = 0;
	ap_bh->red_cnt = 0;
	err = sh_bh_add_links(ap_bh);
	if (err)
		goto exit;

	if (sta_bh) {
		sta_bh->algo = algo;
		sta_bh->ix = sh_idx;
		sta_bh->mlo_info = bsta;
		sta_bh->is_bap = FALSE;
		err = sh_bh_add_links(sta_bh);
		if (err)
			goto exit;
	}
	ap_bh->bsta = sta_bh;

	/* anchar initialization */
	if (algo->info.anchor_band)
		sh_mbh_fix_anchor_band(ap_bh, algo->info.anchor_band);

	/* set up the bap lists */
	ap_bh->next = algo->mbh;
	algo->mbh = ap_bh;
	algo->mbh_cnt++;
	SMARTHAUL_TRACE("mbh_cnt %d\n", algo->mbh_cnt);
	if (algo->mbh->link[2].band_ix == 2) { //if bh has 2G band, remove it as default
		sh_tidmap_t tm;
		algo->mbh->cur_linkset &= ~(1 << 2);
		SMARTHAUL_TRACE("mbh %d has 2G link, set linkset to %x\n", algo->mbh->ix, algo->mbh->cur_linkset);
		sh_mbh_get_tidmap(algo->mbh, algo->mbh->cur_linkset, &tm);
		sh_tidmap_set(&tm);
	}
	return SH_OK;

exit:
	if (ap_bh) {
		//ASSERT(!ap_bh->bsta);	/* no dbl free */
		sh_mbh_free(ap_bh);
	}

	if (sta_bh)
		sh_mbh_free(sta_bh);

	SMARTHAUL_ERROR("failed to add bh %d, err %d\n", sh_idx, err);
	return err;
}

static void sh_mbh_del_link(struct sh_mbh *mbh, int link_id)
{
	struct sh_link *sl = &mbh->link[link_id];
	//ASSERT(link_id < MAX_MLO_LINKS);
	mbh->cfg_linkset &= ~(1 << link_id);
	mbh->link_cnt--;
	sl->valid = FALSE;
	SMARTHAUL_TRACE("mbh %d del link %d\n", mbh->ix, link_id);
}

int sh_del_bh(uint32 sh_idx)
{
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh, *nlist;

	mbh = sh_algo_find_mbh_by_ix(sh_idx);
	if (!mbh) {
		return SH_ERROR;
	}

	/* detach mbh from the list */
	nlist = mbh->next;
	mbh->next = NULL;
	while (algo->mbh) {
		struct sh_mbh *cur = algo->mbh;
		algo->mbh = cur->next;

		/* take the head of algo->mbh, insert into head of nlist */
		if (cur != mbh) {
			cur->next = nlist;
			nlist = cur;
		} else
			break;
	}
	/* now algo->mbh list only has the mbh to be deleted */
	algo->mbh = nlist;

	for (int i = 0; i < MAX_MLO_LINKS; i++) {
		if (mbh->link[i].valid)
			sh_mbh_del_link(mbh, i);
	}
	sh_mbh_free(mbh);
	algo->mbh_cnt--;
	return SH_OK;
}

void sh_mbh_free(struct sh_mbh *mbh)
{
	char mld_str[ETHER_ADDR_STR_LEN];
	ether_etoa((uint8 *)&mbh->mlo_info->self_mld_addr, mld_str);
	SMARTHAUL_INFO("%p, ix=%d, mld=%s\n", mbh, mbh->ix, mld_str);

	if (mbh->bsta) {
		sh_mbh_free(mbh->bsta);
		mbh->bsta = NULL;
	}

	free(mbh);
}

static void sh_mbh_fix_anchor_band(struct sh_mbh *mbh, uint8 band)
{
	mbh->anchor_link = band;
}

static struct sh_mbh *sh_algo_find_mbh_by_ix(int mbh_ix)
{
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh = algo->mbh;

	while (mbh) {
		if (mbh->ix == mbh_ix)
			return mbh;
		mbh = mbh->next;
	}

	return NULL;
}

int sh_mbh_chanim_stats_upd(uint32 sh_idx)
{
	struct sh_mbh *mbh;
	int i, ret = SH_OK;
	uint8 txop, obss, inbss, tx;
	uint8 nocat, nopkt;

	mbh = sh_algo_find_mbh_by_ix(sh_idx);
	if (!mbh) {
		SMARTHAUL_ERROR("no bh for ix %d\n", sh_idx);
		return SH_ERROR;
	}

	for (i = 0; i < mbh->link_cnt; i++){
		/* sysdeps */
#if defined (RTCONFIG_HND_ROUTER_BE_4916)
		ret = sh_get_chanim_stats(&txop, &obss, &inbss, &tx, &nocat, &nopkt, mbh->link[i].wlc_unit);
#else
		ret = sh_get_chanim_stats(&txop, &obss, &inbss, &tx, mbh->link[i].wlc_unit);
		nocat = 0;
		nopkt = 0;
#endif
		if (ret == SH_OK){
			mbh->link[i].stats.txop = txop;
			mbh->link[i].stats.obss = obss;
			mbh->link[i].stats.inbss = inbss;
			mbh->link[i].stats.tx = tx;
			mbh->link[i].stats.nocat = nocat;
			mbh->link[i].stats.nopkt = nopkt;
			SMARTHAUL_INFO("mbh %d link%d wl%d %dG, txop %d, obss %d, tx%d, inbss %d, nocat %d, nopkt %d\n",
							sh_idx, i, mbh->link[i].wlc_unit, mbh->link[i].band_ix,
							mbh->link[i].stats.txop, mbh->link[i].stats.obss, mbh->link[i].stats.tx,
							mbh->link[i].stats.inbss, mbh->link[i].stats.nocat, mbh->link[i].stats.nopkt);
		}
	}
	return ret;
}

static void sh_dbg_dump_chanim(struct sh_mbh *mbh)
{
	struct sh_link *sl;
	int i;
	for (i = 0; i < mbh->link_cnt; i++) {
		sl = &mbh->link[i];
		SMARTHAUL_DBGLOG("mbh %d link%d wl%d %dG, txop %d, obss %d, tx%d, inbss %d, nocat %d, nopkt %d, retries %.1f%%\n",
			sl->mbh->ix, sl->link_id, sl->wlc_unit, sl->band_ix,
			sl->stats.txop, sl->stats.obss, sl->stats.tx, sl->stats.inbss, sl->stats.nocat, sl->stats.nopkt, sl->stats.retries);
	}
}

int sh_mbh_bs_data_upd(uint32 sh_idx)
{
	return 0;
}

int sh_mbh_scb_upd(uint32 sh_idx)
{
	struct sh_mbh *mbh;
	struct sh_link *sl;
	int i;
	uint32 ntx[MAX_MLO_LINKS];
	uint32 nrx[MAX_MLO_LINKS];
	int rssi[MAX_MLO_LINKS];
	mbh = sh_algo_find_mbh_by_ix(sh_idx);
	if (!mbh) {
		SMARTHAUL_ERROR("no bh for ix %d\n", sh_idx);
		return SH_ERROR;
	}
#if defined (RTCONFIG_HND_ROUTER_BE_4916)
	sh_get_mlo_scb_stats(ntx, nrx, rssi, &mbh->bsta->mlo_info->self_mld_addr, mbh->link[0].wlc_unit);
	for (i = 0; i < mbh->mlo_info->mlo_num_links; i++) {
		sl = &mbh->link[i];

		sl->stats.last_ntx = sl->stats.ntx;
		sl->stats.last_nrx = sl->stats.nrx;

		sl->stats.ntx = ntx[i];
		sl->stats.nrx = nrx[i];
		sl->stats.rssi = rssi[i];

		SMARTHAUL_INFO("mbh %d link%d wl%d %dG, ntx: %d, nrx: %d, rssi: %d\n",
						sh_idx, i, mbh->link[i].wlc_unit, mbh->link[i].band_ix, sl->stats.ntx, sl->stats.nrx, sl->stats.rssi);
		sl->stats.flags |= SH_STATS_FLAG_SCB;
	}
	return SH_OK;
#else
	/* collect throughput */
	return SH_OK;
#endif
}

static void sh_collect_stats(void)
{
	/* TBD: add a for loop to collect all mbh stats, such like mbh->next */
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh = algo->mbh;
	while (mbh) {
		/* chanim_stat collcting */
		/* txop obss inbss tx nocat nopkt*/
		sh_mbh_chanim_stats_upd(mbh->ix);

		/* bs_data collecting */
		/* retry */
#if defined (RTCONFIG_HND_ROUTER_BE_4916)
		sh_mbh_bs_data_upd(mbh->ix);
#endif
		/* mlo_scb data collecting */
		/* ntx nrx last_ntx last_nrx*/
		sh_mbh_scb_upd(mbh->ix);
		mbh = mbh->next;
	}
	sh_algo_run();
}

int sh_add_timer(int cycle_ms)
{
	struct itimerval timer;
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh = algo->mbh;
	if (!timer_running) {
		SMARTHAUL_TRACE("Start stats collection timer at intevals (us): %d\n", cycle_ms);
		timer.it_interval.tv_sec = cycle_ms / 1000;
		timer.it_interval.tv_usec = 0;
		timer.it_value.tv_sec = cycle_ms / 1000;
		timer.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, NULL);
		timer_running = TRUE;
		return SH_OK;
	}
	else if (mbh) {
		SMARTHAUL_TRACE("Already have active backhaul and timer is running\n");
		return SH_OK;
	}
	else {
		SMARTHAUL_ERROR("No active backhaul but timer is running!\n");
		return SH_ERROR;
	}
}

int sh_remove_timer(void)
{
	struct itimerval timer;
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh = algo->mbh;
	if (timer_running && !mbh) {
		SMARTHAUL_TRACE("No one using timer, stop it\n");
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, NULL);
		timer_running = FALSE;
		return SH_OK;
	}
	else if (timer_running) {
		SMARTHAUL_TRACE("Still have active backahul using timer, keep it\n");
		return SH_ERROR;
	}	
	else {
		SMARTHAUL_ERROR("No timer is running\n");
		return SH_ERROR;
	}
}

/* Dump of remote device record in the root AP */
void dump_sh_rpt_list(sh_root_info_t *root_info)
{
	int i;
	sh_repeater_node_t *curr = root_info->rpt_list;

	SMARTHAUL_PRINT("Number of smart haul capable repeaters: %d\n", root_info->num_repeaters);

	while (curr) {
		SMARTHAUL_PRINT("Self  MLD address: " MACF " num links: %d\n",
			ETHER_TO_MACF(curr->mlo_info.self_mld_addr), curr->mlo_info.mlo_num_links);
		for (i = 0; i < curr->mlo_info.mlo_num_links; i++) {
			SMARTHAUL_PRINT("wlc_unit: %d, link %d, pap bssid: " MACF "\n",
				curr->mlo_info.mlo_link_info[i].wlc_unit, curr->mlo_info.mlo_link_info[i].link_id, ETHER_TO_MACF(curr->mlo_info.mlo_link_info[i].pap_bssid));
		}
		curr = curr->next;
	}
}

/* Dump device info */
void dump_sh_dev_info(sh_dev_info_t *dev)
{
	int i;

	SMARTHAUL_PRINT("Self MAC assress" MACF ", Self MLD address: " MACF " num links: %d\n",
		ETHER_TO_MACF(dev->self_mac),
		ETHER_TO_MACF(dev->mlo_info.self_mld_addr),
		dev->mlo_info.mlo_num_links);
	for (i = 0; i < dev->mlo_info.mlo_num_links; i++) {
		SMARTHAUL_PRINT("wlc_unit: %d, link %d, pap bssid: " MACF "\n",
			dev->mlo_info.mlo_link_info[i].wlc_unit, dev->mlo_info.mlo_link_info[i].link_id, ETHER_TO_MACF(dev->mlo_info.mlo_link_info[i].pap_bssid));
	}
}

/* Dump whole info */
void dump_sh(smarthaul_t *smarthaul)
{
	sh_dev_info_t *dev_info = NULL;
	sh_root_info_t *root_info = NULL;

	if (nvram_get_int("re_mode") != 1) {
		root_info = (sh_root_info_t *)smarthaul->dev_info;
		SMARTHAUL_PRINT("Root AP agent device dump: \n");
		//ASSERT(root_info);
		if (root_info) {
			dev_info = &root_info->local_dev;
		}
	} else {
		dev_info = (sh_dev_info_t *)smarthaul->dev_info;
		SMARTHAUL_PRINT("Repeater AP agent device dump: \n");
	}

	if (dev_info == NULL) {
		SMARTHAUL_PRINT("dev_info is NULL. Not a MLO device\n");
		return;
	}
	dump_sh_dev_info(dev_info);
	if (root_info) {
		dump_sh_rpt_list(root_info);
	}
}

int sh_init(smarthaul_t *smarthaul)
{
	int ret = SH_OK;
	sh_mlo_info_t* sh_mlo_info;
	sh_root_info_t* root_info;
	sh_dev_info_t* dev_info;
	int i;

	/* Allocate memory */
	bzero(sh_idx_bitvec, sizeof(sh_idx_bitvec));
	active_bh = NULL;
	
	smarthaul->dev_info = calloc(1, sizeof(sh_root_info_t));
	if (smarthaul->dev_info == NULL) {
		return SH_NOMEM;
	}
	root_info = (sh_root_info_t *) smarthaul->dev_info;
	sh_mlo_info = &root_info->local_dev.mlo_info;
	
	/* MLO checking */
	/* sysdeps */
	ret = sh_mlo_support(); /* Return number of MLO link */
	if (ret == 0) {	/* MLO not support */
		SMARTHAUL_ERROR("Not a MLO device. Smart Haul is disabled\n");
		smarthaul->enable_smart_haul = FALSE;
		free(smarthaul->dev_info);
		smarthaul->dev_info = NULL;
		return SH_ERROR;
	}
	/* seem like smarthaul now support with only two link, remove this limitation temporary
	else if (ret < 3) {
		SMARTHAUL_ERROR("Smart Haul not support dual band MLO\n");
		smarthaul->enable_smart_haul = FALSE;
		free(smarthaul->dev_info);
		smarthaul->dev_info = NULL;
		return SH_ERROR;
	}
	*/
	else
		sh_mlo_info->mlo_num_links = ret;

	/* Collection CAP's MLO info */ 
	/* TBD: Add some sanity check */
	SMARTHAUL_TRACE("Collecting mlo link info\n");
	uint8 link_id, wlc_unit, bandidx;
	for (i = 0; i < sh_mlo_info->mlo_num_links; i++){
		/* sysdeps */
		sh_mlo_link_info_collect(&link_id, &wlc_unit, &bandidx, &sh_mlo_info->mlo_link_info[i].pap_bssid, i);
		sh_mlo_info->mlo_link_info[i].link_id = link_id;
		sh_mlo_info->mlo_link_info[i].wlc_unit = wlc_unit;
		sh_mlo_info->mlo_link_info[i].bandidx = bandidx;

		SMARTHAUL_TRACE("MLO link%d info: wl%d, %dG band, mac_addr" MACF "\n", 
			sh_mlo_info->mlo_link_info[i].link_id,
			sh_mlo_info->mlo_link_info[i].wlc_unit,
			sh_mlo_info->mlo_link_info[i].bandidx,
			ETHER_TO_MACF(sh_mlo_info->mlo_link_info[i].pap_bssid));
	}
	
	SMARTHAUL_TRACE("Calling Smart Haul algorithm initialization routine\n");
	ret = sh_algo_init((void *)smarthaul);
	if (ret != SH_OK) {
		SMARTHAUL_ERROR("Smart Haul algo init failed. Disable Smart Haul");
		smarthaul->enable_smart_haul = FALSE;
		free(smarthaul->dev_info);
		smarthaul->dev_info = NULL;
		return ret;
	}
	/* If nvram includes a user configured repeater MLD, record it */
	/*if ((nvram_val = nvram_get("sh_algo_mld"))) {
		if (!ether_atoe(nvram_val, sh_usr_cfg_mld.octet)) {
			SMARTHAUL_ERROR("Smart haul sh_algo_mld nvram format error\n");
		}
	}*/
	SMARTHAUL_TRACE("Starting to create ipc socket\n");
	sh_ipc_socket_thread(smarthaul);
	algo_info = sh_algo_info_get();
	
	SMARTHAUL_SYSLOG("Smart Haul initial done, waiting for MLO connection\n");
	return SH_OK;
}

int sh_algo_init(void *arg)/* no need callback I guess */
{
	struct sh_algo *algo = &g_algo;
	const static struct sh_ls_algo *ls_algo = &g_sh_ls_algo[0];
	char *nvram_str;
	uint8 def_anchor = 0;	/* link 0 */

	bzero(&g_algo, sizeof(g_algo));

	algo->arg = arg;

	/* read nvram into algo info */
	/* 1: static anchor link, 2: dynamic anchor link, default: 1 */
	nvram_str = nvram_get("sh_algo_type");
	algo->info.type = nvram_str ? strtoul(nvram_str, NULL, 0) : 2;
	if (!algo->info.type) {
		SMARTHAUL_INFO("Smarthaul algo type 0, disabled\n");
		return SH_ERROR;
	} else for (; ls_algo->type; ls_algo++) {
		if (ls_algo->type == algo->info.type) {
			algo->ls_algo = ls_algo;
			break;
		}
	}
	if (!algo->ls_algo) {
		SMARTHAUL_ERROR("bad algo type %u\n", algo->info.type);
		return SH_ERROR;
	}

	if (algo->info.type == 2) /* dynamoc anchor link */
		def_anchor = 0;

	nvram_str = nvram_get("sh_algo_txop_hi");
	algo->info.txop_hi = nvram_str ? strtoul(nvram_str, NULL, 0) : 60;

	nvram_str = nvram_get("sh_algo_txop_lo");
	algo->info.txop_lo = nvram_str ? strtoul(nvram_str, NULL, 0) : 40;

	nvram_str = nvram_get("sh_algo_anchor_band");
	algo->info.anchor_band = nvram_str ? strtoul(nvram_str, NULL, 0) : def_anchor;

	nvram_str = nvram_get("sh_algo_cycle_ms");
	algo->info.cycle_ms = nvram_str ? strtoul(nvram_str, NULL, 0) : 3000;

	nvram_str = nvram_get("sh_algo_min_cnt");
	algo->info.min_cnt = nvram_str ? strtoul(nvram_str, NULL, 0) : 0;

	/* TBD: add sanity check, 0 <= txop <= 100, anchor = 0/1/2/3 */
	/* Mayby we should rename "anchor band" to "anchor link" */

	SMARTHAUL_INFO("algo type %u, txop: hi %u lo %u, band %u, cycle_ms %d\n",
			algo->info.type, algo->info.txop_hi, algo->info.txop_lo,
			algo->info.anchor_band, algo->info.cycle_ms);
	return SH_OK;
}

int sh_algo_run(void)
{
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh = algo->mbh;
	struct sh_mbh *best_mbh = NULL;
	sh_tidmap_t tm;
	uint8 new_linkset UNUSED_VAR;
	int score, best_score = 0, err = SH_OK;
	bool cnt_reset_flg = TRUE;
	static int stop_flg = 0;

    if (nvram_get_int("sh_algo_stop") == 1) {
        SMARTHAUL_TRACE("sh_algo_stop == 1, skip\n");
        if (stop_flg == 0){
            SMARTHAUL_TRACE("first time stop, reset tidmap\n");
            sh_reset_tidmap();
            stop_flg = 1;
        }
        goto exit;
    }
    else
        stop_flg = 0;

	/* check if there could be a linkset expansion */
	while (mbh) {
		err = sh_algo_prep(mbh);
		if (err) {
			SMARTHAUL_ERROR("mbh %d prep failed, err=%d\n", mbh->ix, err);
			goto exit;
		}

		score = sh_algo_expand_score(mbh);

		if (score > best_score) {
			SMARTHAUL_TRACE("mbh %d expand score %d beats %d\n", mbh->ix, score, best_score);
			best_mbh = mbh;
		}

		mbh = mbh->next;
	}

	if (best_mbh) {
		mbh = best_mbh;
		if (++(mbh->exp_cnt) < algo->info.min_cnt){
			//SMARTHAUL_TRACE("mbh%d expand conut: %d/%d\n", mbh->ix, mbh->exp_cnt, algo->info.min_cnt);
			mbh->red_cnt = 0;
			cnt_reset_flg = FALSE;
			goto exit;
		}
		//SMARTHAUL_TRACE("mbh%d expand conut: %d/%d\n", mbh->ix, mbh->exp_cnt, algo->info.min_cnt);
		new_linkset = sh_algo_expand_linkset(mbh);

		sh_mbh_get_tidmap(mbh, new_linkset, &tm);
		SMARTHAUL_SYSLOG("cycle %d: mbh %d linkset exp to %x, new tidmap %02x%02x%02x(L0L1L2)\n",
				algo->cycle, mbh->ix, new_linkset, tm.tids[0],
				tm.tids[1], tm.tids[2]);
		sh_dbg_dump_chanim(mbh);
		sh_tidmap_set(&tm);
		err = SH_OK;
		goto exit;
	}

	ASSERT(best_score == 0);
	mbh = algo->mbh;

	/* check if there could be a linkset reduction */
	while (mbh) {
		score = sh_algo_reduce_score(mbh);

		if (score > best_score) {
			SMARTHAUL_TRACE("mbh %d reduce score %d beats %d\n", mbh->ix, score, best_score);
			best_mbh = mbh;
		}

		mbh = mbh->next;
	}

	if (best_mbh) {
		mbh = best_mbh;
		if (++(mbh->red_cnt) < algo->info.min_cnt){
			//SMARTHAUL_TRACE("mbh%d reduce conut: %d/%d\n", mbh->ix, mbh->red_cnt, algo->info.min_cnt);
			mbh->exp_cnt = 0;
			cnt_reset_flg = FALSE;
			goto exit;
		}
		//SMARTHAUL_TRACE("mbh%d reduce conut: %d/%d\n", mbh->ix, mbh->red_cnt, algo->info.min_cnt);
		new_linkset = sh_algo_reduce_linkset(mbh);

		sh_mbh_get_tidmap(mbh, new_linkset, &tm);
		SMARTHAUL_SYSLOG("cycle %d: mbh %d linkset red to %x, new tidmap %02x%02x%02x(L0L1L2)\n",
				algo->cycle, mbh->ix, new_linkset, tm.tids[0],
				tm.tids[1], tm.tids[2]);
		sh_dbg_dump_chanim(mbh);
		sh_tidmap_set(&tm);
		err = SH_OK;
		goto exit;
	}

exit:
	/* move the cycle counter */
	algo->cycle++;
	SMARTHAUL_TRACE("next cycle is %d\n\n", algo->cycle);

	/*
	if (cnt_reset_flg){
		mbh->exp_cnt = 0;
		mbh->red_cnt = 0;
	}
	*/
	/* all stats have been used, mark it invalid for future cycles */
	mbh = algo->mbh;
	while (mbh) {
		sh_mbh_invalidate_stats(mbh);
		mbh = mbh->next;
	}
	return err;
}

static void sh_mbh_get_tidmap(struct sh_mbh *mbh, uint8 linkset, sh_tidmap_t *map)
{
	sh_link_t *sl;
	int dis_band = 0;
	dis_band = nvram_get_int("sh_algo_disable_band");
	map->sh_idx = mbh->ix;
	memset(map->tids, 0xF0, sizeof(map->tids));

	for (int i = 0; i < MAX_MLO_LINKS; i++) {
		sl = &mbh->link[i];
		//ASSERT(sl->valid);
		if (sl->band_ix == dis_band){
			map->tids[i] = 0x00;
			continue;
		}
		else if (sl->band_ix == 2 && !(linkset & (1 << i))) {
			map->tids[i] = 0x00;
			continue;
		}
		else if (!(linkset & (1 << i)))
			continue;

		map->tids[i] = 0xFF;
	}
}

int sh_tidmap_set(sh_tidmap_t *tm)
{
	int ret = SH_OK;
	int i;
	smarthaul_t *smarthaul = &g_smarthaul;
	sh_root_info_t *root_info = (sh_root_info_t *)smarthaul->dev_info;
	sh_repeater_node_t *rpt_list = root_info->rpt_list;
	while(rpt_list){
		if(rpt_list->sh_idx == tm->sh_idx)
			break;
		rpt_list = rpt_list->next;
	}
	sh_mlo_info_t *mlo_info = &rpt_list->mlo_info;
	SMARTHAUL_TRACE("MLD: " MACF "\n",
            ETHER_TO_MACF(mlo_info->self_mld_addr));

	int num = mlo_info->mlo_num_links;

	/* sysdeps */
	ret = sh_set_cap_tidmap(num, mlo_info->mlo_link_info[0].wlc_unit, tm->tids, &mlo_info->self_mld_addr);
	if (ret != SH_OK){
		SMARTHAUL_ERROR("Setting tidmap on CAP failed!\n");
	}

	ret = sh_send_tidmap2re(rpt_list, tm);
	if (ret != SH_OK){
		SMARTHAUL_ERROR("Setting tidmap on Re failed!\n");
	}
	return ret;
}

int sh_reset_tidmap()
{
	struct sh_algo *algo = &g_algo;
	struct sh_mbh *mbh = algo->mbh;
	sh_tidmap_t tm;
	while (mbh) {
		mbh->cur_linkset = mbh->cfg_linkset;
		sh_mbh_get_tidmap(mbh, mbh->cfg_linkset, &tm);
		sh_tidmap_set(&tm);
		mbh = mbh->next;
	}
	return SH_OK;
}

static void sh_mbh_invalidate_stats(struct sh_mbh *mbh)
{
	int i;
	sh_link_t *sl;

	for (i = 0; i < MAX_MLO_LINKS; i++) {
		sl = &mbh->link[i];
		sl->stats.flags = 0;
	}

	if (mbh->bsta)
		sh_mbh_invalidate_stats(mbh->bsta);
}

static int sh_algo_prep(struct sh_mbh *mbh)
{
	/* check stats, etc */
	SMARTHAUL_TRACE("mbh %d current anchor link %d, linkset %x\n",
			mbh->ix, mbh->anchor_link, mbh->cur_linkset);
	return g_algo.ls_algo->prep(mbh);
}

static uint8 sh_algo_expand_linkset(struct sh_mbh *mbh)
{
	ASSERT(mbh->can_linkset > mbh->cur_linkset);
	mbh->cur_linkset = mbh->can_linkset;
	mbh->change_cycle = mbh->algo->cycle;
	return mbh->can_linkset;
}

static int sh_algo_expand_score(struct sh_mbh *mbh)
{
	struct sh_algo *algo = &g_algo;
	uint8 idle_linkset = mbh->cfg_linkset & (~mbh->cur_linkset);
	int tot_score = 0;

	/* do not expand if changed too soon */
	if (algo->cycle - mbh->change_cycle < algo->info.min_cycle)
		return 0;

	mbh->can_linkset = mbh->cur_linkset;
	for (int i = 0; i < MAX_MLO_LINKS; i++) {
		int score;
		uint8 b = 1 << i;

		if ((b & idle_linkset) == 0)
			continue;

		score = algo->ls_algo->expand_score(mbh, i);
		if (score > 0)
			mbh->can_linkset |= b;

		tot_score += score;

	}

	SMARTHAUL_TRACE("mbh %d expand score %d\n", mbh->ix, tot_score);
	return tot_score;
}

static uint8 sh_algo_reduce_linkset(struct sh_mbh *mbh)
{
	ASSERT(mbh->can_linkset < mbh->cur_linkset);
	mbh->cur_linkset = mbh->can_linkset;
	mbh->change_cycle = mbh->algo->cycle;
	return mbh->can_linkset;
}

static int sh_algo_reduce_score(struct sh_mbh *mbh)
{
	struct sh_algo *algo = &g_algo;
	int tot_score = 0;
	uint8 busy_linkset = mbh->cur_linkset;

	/* debounce, mitigate ping pong */
	if (algo->cycle - mbh->change_cycle < algo->info.min_cycle)
		return 0;

	/* never exclude the anchor link */
	busy_linkset &= ~(1 << mbh->anchor_link);

	mbh->can_linkset = mbh->cur_linkset;
	for (int i = 0; i < MAX_MLO_LINKS; i++) {
		uint8 b = 1 << i;
		if ((b & busy_linkset) == 0)
			continue;

		SMARTHAUL_TRACE("check link %d\n", i);
		int score = algo->ls_algo->reduce_score(mbh, i);
		if (score > 0)
			mbh->can_linkset &= ~b;

		tot_score += score;

	}

	SMARTHAUL_TRACE("mbh %d reduce score %d\n", mbh->ix, tot_score);
	return tot_score;
}

static int sh_ls_prep_1(struct sh_mbh *mbh)
{
	return SH_OK;
}

static int sh_ls_expand_score_1(struct sh_mbh *mbh, uint8 link_id)
{
	sh_algo_info_t *info = &mbh->algo->info;
	struct sh_link *sl = &mbh->link[link_id];
	int score = 0;

	uint8 tx_sum, sum;
	char *nvram_str;
	int i;
	/* expand if a link is idle enough */

	int rssi_threshold;
	rssi_threshold = nvram_get_int("sh_algo_rssi_lo") ? nvram_get_int("sh_algo_rssi_lo") : -82;

	if (sl->band_ix == 2
		&& mbh->link[0].stats.rssi > rssi_threshold
		&& mbh->link[1].stats.rssi > rssi_threshold) {
		SMARTHAUL_INFO("rssi of 6G(%d) & 5G(%d) are higher than %d, skip 2G expand score calculate\n",
				mbh->link[0].stats.rssi, mbh->link[1].stats.rssi, rssi_threshold);
		goto exit;
	}
	else if (nvram_get_int("sh_slgo_add_2g") == 1) {
		score = 100;
		goto exit;
	}

	if (sl->stats.txop > info->txop_hi)
		score = sl->stats.txop - info->txop_hi;

	if (sl->stats.retry_flag == TRUE){
		sum = 0;
		nvram_str = nvram_get("sh_algo_tx_sum");
		tx_sum = nvram_str ? strtoul(nvram_str, NULL, 0) : 20;

		for (i = 0; i < mbh->link_cnt ; i++){
			sum += mbh->link[i].stats.tx;
			sum += mbh->link[i].stats.inbss;
		}

		if (sum > tx_sum) {	/* still transfering data */
			score = 0;
			SMARTHAUL_TRACE("Transfring data, high retry link%d can't be added back\n", link_id);
		}
		else {
			sl->stats.retry_flag = FALSE;
			SMARTHAUL_TRACE("Stop transfring data, high retry link%d is added back\n", link_id);
		}					
	}

exit:
	SMARTHAUL_TRACE("mbh %d link %d, score %d\n", mbh->ix, link_id, score);
	return score;
}

static int sh_ls_reduce_score_1(struct sh_mbh *mbh, uint8 link_id)
{
	sh_algo_info_t *info = &mbh->algo->info;
	struct sh_link *sl = &mbh->link[link_id];
	int score = 0;
	uint32 obss_score;
	uint32 algo_obss;
	uint8 algo_retry;
	char *nvram_str;
	bool low_obss = FALSE;
	bool low_retry = FALSE;

#if defined(RTCONFIG_HND_ROUTER_BE_4916)
	nvram_str = nvram_get("sh_algo_obss");
	algo_obss = nvram_str ? strtoul(nvram_str, NULL, 0) : 0;
	nvram_str = nvram_get("sh_algo_retry");
	algo_retry = nvram_str ? strtoul(nvram_str, NULL, 0) : 0;
#else
	algo_obss = 0;
	algo_retry = 0;
#endif

	int rssi_threshold;
	rssi_threshold = nvram_get_int("sh_algo_rssi_hi") ? nvram_get_int("sh_algo_rssi_hi") : -77;

	if (sl->band_ix == 2
		&& mbh->link[0].stats.rssi > rssi_threshold
		&& mbh->link[1].stats.rssi > rssi_threshold) {
		SMARTHAUL_INFO("rssi of 6G(%d) & 5G(%d) are higher than %d, remove 2G instantly\n",
				mbh->link[0].stats.rssi, mbh->link[1].stats.rssi, rssi_threshold);
		score = 100;
		goto exit;
	}
	else if (nvram_get_int("sh_slgo_add_2g") == 1){
		goto exit;
	}
	/* obss checking */
	if (algo_obss == 0){ /* default mode */
		if (sl->band_ix == 6){
			nvram_str = nvram_get("sh_algo_obss_6G");
#if defined(BQ16_PRO) 
			algo_obss = nvram_str ? strtoul(nvram_str, NULL, 0) : 6;
#else
			algo_obss = nvram_str ? strtoul(nvram_str, NULL, 0) : 3;
#endif
			if (sl->stats.obss * algo_obss < sl->stats.tx + sl->stats.inbss){
				SMARTHAUL_TRACE("link %d obss is too low with multiplier %d\n", link_id, algo_obss);
				low_obss = TRUE;
			}
		}
		/* if obss is low, removing this link from BH will not help */
		else {
			nvram_str = nvram_get("sh_algo_obss_5G");
			algo_obss = nvram_str ? strtoul(nvram_str, NULL, 0) : 3;
			if (sl->stats.obss * algo_obss < sl->stats.tx + sl->stats.inbss){
				SMARTHAUL_TRACE("link %d obss is too low with multiplier %d\n", link_id, algo_obss);
				low_obss = TRUE;
			}
		}
	}
#if defined(RTCONFIG_HND_ROUTER_BE_4916)
	/* Only BRCM models support nocat mode and retry check */
	else { /* obss + nocat mode */
		obss_score = (sl->stats.obss + sl->stats.nocat) * algo_obss / 100;
		if (obss_score < sl->stats.tx + sl->stats.inbss){
			SMARTHAUL_TRACE("link %d obss+nocat is too low with multiplier %d%%\n", link_id, algo_obss);
			low_obss = TRUE;
		}
	}
	/* retry checking */
	if (algo_retry != 0 && link_id == 2){ /* Only check link 2 retry with non-zero algo_retry */
		if(sl->stats.retries < algo_retry){
			SMARTHAUL_TRACE("link %d have low retry\n", link_id);
        	low_retry = TRUE;
		}

		if (low_obss == TRUE && low_retry == TRUE && link_id == 2){
			SMARTHAUL_TRACE("link %d have low retry & low obss, skip txop checking\n", link_id);
			goto exit;
		}
	}
	else {
		if (low_obss == TRUE){
			SMARTHAUL_TRACE("link %d have low obss, skip txop checking\n", link_id);
			goto exit;
		}
	}

#endif
	if (sl->stats.txop < info->txop_lo){
		score = info->txop_lo - sl->stats.txop;
		if (algo_retry != 0 && sl->stats.retries > algo_retry && link_id == 2){
			SMARTHAUL_TRACE("link%d is marked as high retry link\n", link_id);
			sl->stats.retry_flag = TRUE;
		}
		else 
			sl->stats.retry_flag = FALSE;
	}

exit:
	SMARTHAUL_TRACE("mbh %d link %d, score %d\n", mbh->ix, link_id, score);
	return score;
}

static uint8 sh_ls_next_anchor_link(struct sh_mbh *mbh)
{
	struct sh_algo *algo = &g_algo;
	uint i, best_score = 0;
	uint8 best_link = 0;

	ASSERT(mbh->is_bap);

#if defined (RTCONFIG_HND_ROUTER_BE_4916)
	/* if no stats update, keep the last anchor */
	/*if (algo->cycle != mbh->stats_cycle) {
		SMARTHAUL_TRACE("mbh %x stats not updated, %d vs %d\n", mbh->ix,
				mbh->stats_cycle, algo->cycle);
		return mbh->anchor_link;
	}*/

	/* get the link with the highest score, here just use total traffic */
	for (i = 0; i < MAX_MLO_LINKS; i++) {
		sh_link_t *sl = &mbh->link[i];
		uint score;

		if (!sl->valid)
			continue;

		if (!(sl->stats.flags & SH_STATS_FLAG_SCB)) {
			SMARTHAUL_TRACE("mbh %x link %d scb stats not updated\n", mbh->ix, i);
			goto exit;
		}

		if ((sl->stats.ntx < sl->stats.last_ntx) || (sl->stats.nrx < sl->stats.last_nrx)) {
			SMARTHAUL_TRACE("mbh %x link %d scb stats overflow\n", mbh->ix, i);
			goto exit;
		}

		/* find the highest utilized link */
		score = sl->stats.ntx - sl->stats.last_ntx + sl->stats.nrx - sl->stats.last_nrx;
		SMARTHAUL_TRACE("mbh %x link %d scb score is %d (%d - %d + %d - %d)\n", mbh->ix, i, score,
						sl->stats.ntx, sl->stats.last_ntx, sl->stats.nrx, sl->stats.last_nrx);
		if (score > best_score) {
			best_score = score;
			best_link = i;
		}
	}

	SMARTHAUL_TRACE("mbh %x best_link=%d best_score=%d\n", mbh->ix, best_link, best_score);
	if (best_score && (best_link != mbh->anchor_link)) {
		SMARTHAUL_INFO("switch anchor_link from %d to %d\n", mbh->anchor_link, best_link);
		mbh->anchor_link = best_link;
	}
#else
	/* select the best link depends on models */
	mbh->anchor_link = best_link;
#endif
exit:
	return mbh->anchor_link;
}

static int sh_ls_prep_2(struct sh_mbh *mbh)
{
	struct sh_algo *algo = &g_algo;

	/* if anchor_band is not fixed, find the best link to be the next anchor */
	if (algo->info.anchor_band) {
		SMARTHAUL_TRACE("nvram fixed anchor_band %d, skip dynamic selection\n",
				algo->info.anchor_band);
		return SH_OK;
	}

	(void)sh_ls_next_anchor_link(mbh);
	return SH_OK;
}

static int sh_ls_expand_score_2(struct sh_mbh *mbh, uint8 link_id)
{
	return sh_ls_expand_score_1(mbh, link_id);
}

static int sh_ls_reduce_score_2(struct sh_mbh *mbh, uint8 link_id)
{
	int score = sh_ls_reduce_score_1(mbh, link_id);

	if (score) {
		/* TBD: add more filters */

		/* if the rest of linkset cannot support the ntx+nrx of this link, set score to 0 */

		/* maybe check obss of the bsta stats? */
	}
	//SMARTHAUL_TRACE("mbh %d link %d, score %d\n", mbh->ix, link_id, score);
	return score;
}

void sigtermHandler(int sig)
{
	sh_reset_tidmap();
	exit(0);
}

void timerHandler(int sig){
	static int disabled = 0;
	if (nvram_get_int("smarthaul_enable") == 1) {
		sh_collect_stats();
		disabled = 0;
	}
	else if (disabled == 0) {
		SMARTHAUL_SYSLOG("Smarthaul algorithm not enable\n");
		sh_reset_tidmap();
		disabled = 1;
	}
}

int smarthaul_main(void)
{
	smarthaul_t *smarthaul = &g_smarthaul;
	int ret;
	char *nvram_val;

	sleep(15); /* sleep for a while, wait for mlo init */

	signal(SIGALRM, timerHandler);
	signal(SIGTERM, sigtermHandler);

	/* set debug log level */
	if ((nvram_val = nvram_get("smarthaul_msglevel"))) {
		smarthaul_debug_level = (uint)strtoul(nvram_val, NULL, 0);
		smarthaul_debug_level |= SMARTHAUL_DEFAULT_LEVEL;
		SMARTHAUL_INFO("smarthaul debug level %x\n", smarthaul_debug_level);
	}

	/* Check if this device is CAP or not */
	if(nvram_get_int("re_mode") == 1){
		SMARTHAUL_ERROR("Smart Haul only work on CAP!\n");
		return 0;
	}

	SMARTHAUL_TRACE("Start initializing smarthaul\n");
	ret = sh_init(smarthaul);
	
	if (ret != SH_OK)
		exit(0);

	while (1)
	{
		pause();
	}
	return 0;
}

sh_algo_info_t *sh_algo_info_get(void)
{
	return &g_algo.info;
}
