#include <rc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include "rc_ipsec.h"

/* for struct utsname */
#include <sys/utsname.h>

#ifdef IPSEC_DEBUG
#define DBG(args) _dprintf args
#endif

#define wan_prefix(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)

void rc_ipsec_topology_set();

/*ike version : v1, v2, auto:ike*/
char ikev[IKE_TYPE_MAX_NUM][SZ_MIN] = {
    {"ike"},
    {"ikev1"},
    {"ikev2"},
};
/*ike default: 3des-sha1-modp1536, aes128-sha1-modp2048*/
char encryp[ENCRYPTION_TYPE_MAX_NUM][SZ_MIN] = {
    {"des"},   
    {"3des"},
    {"aes128"},
    {"aes192"},
    {"aes256"},
};

/*hash default is sha1*/
char hash[HASH_TYPE_MAX_NUM][SZ_MIN] = {
    {"md5"},
    {"sha1"},
    {"sha256"},
    {"sha384"},
    {"sha512"},
};

/*DH: default group is modp1536,modp2048*/
char dh_group[DH_GROUP_MAX_NUM][SZ_MIN] = {
/*Regular Groups*/
    {"modp768"},      /*DH Group 1  : 768bits*/
    {"modp1024"},     /*DH Group 2  : 1024bits*/
    {"modp1536"},     /*DH Group 5  : 1536bits*/
    {"modp2048"},     /*DH Group 14 : 2048bits*/
    {"modp3072"},     /*DH Group 15 : 3072bits*/
    {"modp4096"},     /*DH Group 16 : 4096bits*/
    {"modp6144"},     /*DH Group 17 : 6144bits*/
    {"modp8192"},     /*DH Group 18 : 8192bits*/
/*Modulo Rrime Groups with Prime Order Sbugroup*/
    {"modp1024s160"}, /*DH Group 22 : 1024bits*/
    {"modp2048s224"}, /*DH Group 23 : 2048bits*/
    {"modp2048s256"}, /*DH Group 24 : 2048bits*/
};

static ipsec_samba_t samba_prof;
static ipsec_samba_t pre_samba_prof;

static ipsec_prof_t prof[2][MAX_PROF_NUM];
static pki_ca_t ca_tab[CA_FILES_MAX_NUM];

int get_active_wan_unit(void)
{
    int active_wan_unit = 0;
#ifdef RTCONFIG_DUALWAN
    int connected = 0;
    if(nvram_match("wans_mode", "lb")){
        for(active_wan_unit = WAN_UNIT_FIRST; active_wan_unit < WAN_UNIT_MAX; active_wan_unit++){
            if(is_wan_connect(active_wan_unit)){
                connected = 1;
                break;
            }
        }

        if(!connected)
            active_wan_unit = WAN_UNIT_FIRST;
    }
    else
#endif
        active_wan_unit = wan_primary_ifunit();

    return active_wan_unit;
}

/*param 1: char *p_end , IN : the src of string buf */
/*param 2: char *p_tmp , OUT: the dest of string buf*/
/*param 3: int *nsize_shifft , OUT: the size of shifft*/
void ipsec_profile_str_parse(char *p_end, char *p_tmp, int *nsize_shifft)
{
    int i = 1;
    while(*p_end != '\0' && *p_end != '>'){
        *p_tmp = *p_end;
        p_tmp++;
        p_end++;
        i++;
    }
    *p_tmp = '\0';
    *p_end = '\0';
    *nsize_shifft = i;
    return;
}

static int ipsec_ike_type_idx_get(char *data);
static int ipsec_esp_type_idx_get(char *data);
/*param 1: char *p_end , IN : the src of string buf */
/*param 2: int *nsize_shifft , OUT: nsize_shifft: OUTPUT*/
int ipsec_profile_int_parse(int ike_esp_type, char *p_end, int *nsize_shifft)
{
    int i = 1, i_value = -1;
    char *p_head = p_end;
    if('n' == *p_head){ i_value = 0; }
    while(*p_end++ != '>'){ i++; }
    *(p_end - 1) = '\0';
    *nsize_shifft = i;
    if(0 != i_value){
        if(FLAG_IKE_ENCRYPT == ike_esp_type){
            i_value = ipsec_ike_type_idx_get(strndup(p_head, i));
        } else if(FLAG_ESP_HASH == ike_esp_type){
            i_value = ipsec_esp_type_idx_get(strndup(p_head, i));
        } else if(FLAG_KEY_CHAG_VER == ike_esp_type){
            if(('a' == *p_head) && ('u' == *(p_head + 1))){
               i_value = IKE_TYPE_AUTO; 
            } else {
               i_value = atoi(strndup(p_head, (size_t)i));
            }
        } else if(FLAG_NONE == ike_esp_type){
            i_value = atoi(strndup(p_head, (size_t)i));
        }
    }
    return i_value;
}

/* function compares the two strings s1 and s2.*/
/* if s1 is fouund to be less than, to match, or be greater than s2.*/
/* the last character of all type of ike and esp is different. */
/* that's why just compare last character */
int reverse_str_cmp(char *s1, char *s2)
{
    int n = 0;
    int sz_s1 = strlen(s1), sz_s2 = strlen(s2);
    if(sz_s1 != sz_s2) {
        return 0;
    }
    n = sz_s1;
    while(0 != n){
        if(s1[n] != s2[n]){
            return 0;
        }else{
            return n;
        }
        n--;
    }
    return sz_s2;
}

int ipsec_ike_type_idx_get(char *data)
{
    int i = 0;
    encryption_type_t ike_type_idx = ENCRYPTION_TYPE_MAX_NUM; /*default : auto*/
    if(0 == strcmp(data, "auto")){
        return ENCRYPTION_TYPE_MAX_NUM;
    }
    if(strlen(data) == strlen("des")){
        return ENCRYPTION_TYPE_DES;
    }
    for(i = ENCRYPTION_TYPE_3DES; i < ENCRYPTION_TYPE_MAX_NUM; i++){
        if(0 == reverse_str_cmp(data, encryp[i])){
            continue;
        }else{
            return i;
        }
    }
    return ike_type_idx;
}

int ipsec_esp_type_idx_get(char *data)
{
    int i = 0;
    encryption_type_t ike_type_idx = HASH_TYPE_MAX_NUM; /*default : auto*/
    if(0 == strcmp(data, "auto")){
        return HASH_TYPE_MAX_NUM;
    }
    if(strlen(data) == strlen("md5")){
        return HASH_TYPE_MD5;
    }
    for(i = HASH_TYPE_SHA1; i < HASH_TYPE_MAX_NUM; i++){
        if(0 == reverse_str_cmp(data, hash[i])){
            continue;
        }else{
            return i;
        }
    }
    return ike_type_idx;
}

void ipsec_samba_prof_fill(char *p_data)
{
    int i = 1;
    char *p_end = NULL, *p_tmp = NULL;
    p_end = p_data;

	p_end += i;/*to shifft next '>'*/
    /*DNS1*/
    p_tmp = &(samba_prof.dns1[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
//DBG(("dns1:%s\n", samba_prof.dns1));
    p_end += i;/*to shifft next '>'*/
    /*DNS2*/
    p_tmp = &(samba_prof.dns2[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
//DBG(("dns2:%s\n", samba_prof.dns2));
    p_end += i ;/*to shifft next '>'*/
    /*NBIOS1*/
    p_tmp = &(samba_prof.nbios1[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
//DBG(("nbios1:%s\n", samba_prof.nbios1));
    p_end += i;/*to shifft next '>'*/
    /*NBIOS2*/
    p_tmp = &(samba_prof.nbios2[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
//DBG(("nbios2:%s\n", samba_prof.nbios2));
    //p_end += i;
    return;
}

void ipsec_prof_fill(int prof_idx, char *p_data, ipsec_prof_type_t prof_type)
{
    int i = 1;
    char *p_end = NULL, *p_tmp = NULL, *ptr=NULL;;
    p_end = p_data;

    /*vpn_type*/
    prof[prof_type][prof_idx].vpn_type = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*profilename*/    
    p_tmp = &(prof[prof_type][prof_idx].profilename[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i ;
    /*remote_gateway_method*/
    p_tmp = &(prof[prof_type][prof_idx].remote_gateway_method[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i ;
    /*remote gateway*/
    p_tmp = &(prof[prof_type][prof_idx].remote_gateway[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i ;
    /*local public interface*/
    p_tmp = &(prof[prof_type][prof_idx].local_public_interface[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i ;
    /*local public ip*/
    p_tmp = &(prof[prof_type][prof_idx].local_pub_ip[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i ;
    /*auth_method*/
    prof[prof_type][prof_idx].auth_method = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                  p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*auth method value -- psk password or private key of rsa*/
    p_tmp = &(prof[prof_type][prof_idx].auth_method_key[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
	/*p_end+1 for skipping '<'*/
    p_end += 1;
    /*to parse local subnet e.g.192.168.2.1/24> [local_port] 0>*/
    p_tmp = &(prof[prof_type][prof_idx].local_subnet[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
	 /*to replace '<' to ',' e.g. 192.168.3.1/16,192.168.2.1/24*/
	while ((ptr=strchr(p_tmp, '<'))!=NULL) *ptr = ',';
	
    p_end += i;
    /*local port*/
    prof[prof_type][prof_idx].local_port = (uint16_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                  p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*remote subnet*/
    /*to parse remote subnet e.g. <192.168.3.1/16>3600>*/
    /*p_end+1 for skipping '<'*/
    p_end += 1;
    p_tmp = &(prof[prof_type][prof_idx].remote_subnet[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    /*to replace '<' to ',' e.g. 192.168.3.1/16,192.168.2.1/24*/
	while ((ptr=strchr(p_tmp, '<'))!=NULL) *ptr = ',';
    
    p_end += i ;
    /*remote port*/
    prof[prof_type][prof_idx].remote_port = (uint16_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                     p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*tunnel type: transport or tunnel*/
    p_tmp = &(prof[prof_type][prof_idx].tun_type[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
    /*virtual ip en */
    p_tmp = &(prof[prof_type][prof_idx].virtual_ip_en[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
	
    /*virtual ip subnet*/
    p_tmp = &(prof[prof_type][prof_idx].virtual_subnet[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
	/* if virtual_subnet=x.x.x, convert to x.x.x.0/24 */
	if(0 != strcmp(p_tmp, "") && 0 == strstr(p_tmp, "/"))	
		strcat(prof[prof_type][prof_idx].virtual_subnet, ".0/24");
	
    p_end += i;
    /*accessible_networks*/
    prof[prof_type][prof_idx].accessible_networks = (uint8_t)ipsec_profile_int_parse(
                                                          FLAG_NONE, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*ike*/
    prof[prof_type][prof_idx].ike = (uint8_t)ipsec_profile_int_parse(FLAG_KEY_CHAG_VER,
                                                          p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*encryption_p1*/
    prof[prof_type][prof_idx].encryption_p1 = (uint8_t)ipsec_profile_int_parse(
                                                 FLAG_IKE_ENCRYPT, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*hash_p1*/
    prof[prof_type][prof_idx].hash_p1 = (uint8_t)ipsec_profile_int_parse(
                                           FLAG_ESP_HASH, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*exchange*/
    prof[prof_type][prof_idx].exchange = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*local id*/
    p_tmp = &(prof[prof_type][prof_idx].local_id[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
    /*remote_id*/
    p_tmp = &(prof[prof_type][prof_idx].remote_id[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
    /*keylife_p1*/
    prof[prof_type][prof_idx].keylife_p1 = (uint32_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                  p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*xauth*/
    prof[prof_type][prof_idx].xauth = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                 p_end, &i);
    p_end += i;
    /*xauth_account*/
    p_tmp = &(prof[prof_type][prof_idx].xauth_account[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
    /*xauth_password*/
    p_tmp = &(prof[prof_type][prof_idx].xauth_password[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
    /*xauth_server_type,USER auth: auth2meth for IKEv2*/
    p_tmp = &(prof[prof_type][prof_idx].rightauth2_method[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
    p_end += i;
    /* leftauth_method = rightauth2_method if leftauth_method is not given */
    snprintf(prof[prof_type][prof_idx].leftauth_method, sizeof(prof[prof_type][prof_idx].leftauth_method), "%s", prof[prof_type][prof_idx].rightauth2_method);

    /*traversal*/
    prof[prof_type][prof_idx].traversal = (uint16_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                 p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*ike_isakmp*/
    prof[prof_type][prof_idx].ike_isakmp_port = (uint16_t)ipsec_profile_int_parse(
                                                    FLAG_NONE, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*ike_isakmp_nat*/
    prof[prof_type][prof_idx].ike_isakmp_nat_port = (uint16_t)ipsec_profile_int_parse(
                                                        FLAG_NONE, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*ipsec_dpd*/
    prof[prof_type][prof_idx].ipsec_dpd = (uint16_t)ipsec_profile_int_parse(
                                              FLAG_NONE, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*dead_peer_detection*/
    prof[prof_type][prof_idx].dead_peer_detection = (uint16_t)ipsec_profile_int_parse(
                                                        FLAG_NONE, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*encryption_p2*/
    prof[prof_type][prof_idx].encryption_p2 = (uint16_t)ipsec_profile_int_parse(
                                                  FLAG_IKE_ENCRYPT, p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*hash_p2*/
    prof[prof_type][prof_idx].hash_p2 = (uint16_t)ipsec_profile_int_parse(FLAG_ESP_HASH,
                                                               p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*keylife_p2: IPSEC phase 2*/
    prof[prof_type][prof_idx].keylife_p2 = (uint32_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                  p_end, &i);
    p_end += i; /*to shifft next '>'*/
    /*keyingtries*/
    prof[prof_type][prof_idx].keyingtries = (uint16_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                   p_end, &i);
    p_end += i; /*to shifft next '>'*/
	/*samba settings*/
	p_tmp = &(prof[prof_type][prof_idx].samba_settings[0]);
    ipsec_profile_str_parse(p_end, p_tmp, &i);
	while ((ptr=strchr(p_tmp, '<'))!=NULL) *ptr = '>';  /*to replace '<' to '>' e.g. >1.1.1.1>2.2.2.2>3.3.3.3>4.4.4.4*/

    p_end += i; /*to shifft next '>'*/
    /*ipsec_conn_en*/

    if(!strstr(p_end, ">"))
    {
        prof[prof_type][prof_idx].ipsec_conn_en = atoi(p_end);
    }
    else
    {
        /* there are more parameters to be parsed */
        prof[prof_type][prof_idx].ipsec_conn_en = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                                   p_end, &i);
        p_end += i;

        p_tmp = &(prof[prof_type][prof_idx].leftauth_method[0]);
        ipsec_profile_str_parse(p_end, p_tmp, &i);
        p_end += i;

        p_tmp = &(prof[prof_type][prof_idx].leftcert[0]);
        ipsec_profile_str_parse(p_end, p_tmp, &i);
        p_end += i;
        
        p_tmp = &(prof[prof_type][prof_idx].leftsendcert[0]);
        ipsec_profile_str_parse(p_end, p_tmp, &i);
        p_end += i;
        
        p_tmp = &(prof[prof_type][prof_idx].leftkey[0]);
        ipsec_profile_str_parse(p_end, p_tmp, &i);
        p_end += i;
        
        p_tmp = &(prof[prof_type][prof_idx].eap_identity[0]);
        ipsec_profile_str_parse(p_end, p_tmp, &i);
		/*the last one doesn't need to parse ">".*/
    }

    /*the end of profile*/
    return;
}

void ipsec_prof_fill_ext(int prof_idx, char *p_data, ipsec_prof_type_t prof_type)
{
	int i = 1;
    char *p_end = NULL;
    p_end = p_data;

    /*encryption_p1_ext*/
    prof[prof_type][prof_idx].encryption_p1_ext = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
    p_end += i; /*to shifft next '>'*/

	/*hash_p1_ext*/
    prof[prof_type][prof_idx].hash_p1_ext = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
	p_end += i; /*to shifft next '>'*/

	/*dh_group*/
    prof[prof_type][prof_idx].dh_group = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
	p_end += i; /*to shifft next '>'*/

	 /*encryption_p2_ext*/
    prof[prof_type][prof_idx].encryption_p2_ext = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
    p_end += i; /*to shifft next '>'*/

	/*hash_p2_ext*/
    prof[prof_type][prof_idx].hash_p2_ext = (uint8_t)ipsec_profile_int_parse(FLAG_NONE,
                                                               p_end, &i);
	p_end += i; /*to shifft next '>'*/

	/*pfs_group*/
    prof[prof_type][prof_idx].pfs_group= atoi(p_end); /*the last one doesn't need to parse ">".*/

	/*the end of profile*/
	return;
}

int pre_ipsec_prof_set()
{
    char buf[SZ_MIN], buf_ext[SZ_MIN];
    char *p_tmp = NULL, buf1[SZ_BUF];
	char *p_tmp_ext = NULL, buf1_ext[SZ_BUF];
    int i, rc = 0, prof_count = 0;

    p_tmp = &buf1[0];
	p_tmp_ext = &buf1_ext[0];
    memset(p_tmp, 0, sizeof(char) * SZ_MIN);    
	memset(p_tmp_ext, 0, sizeof(char) * SZ_MIN); 
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
	    for(i = 1; i <= MAX_PROF_NUM; i++){
			if(PROF_SVR == prof_count){
	        	sprintf(&buf[0], "ipsec_profile_%d", i);
				sprintf(&buf_ext[0], "ipsec_profile_%d_ext", i);
			}
			else if(PROF_CLI == prof_count){
				sprintf(&buf[0], "ipsec_profile_client_%d", i);
				sprintf(&buf_ext[0], "ipsec_profile_client_%d_ext", i);
			}

			if(strlen(nvram_safe_get(&buf[0])) > 0 && strlen(nvram_safe_get(&buf_ext[0])) > 0){
				strlcpy(p_tmp, nvram_safe_get(&buf[0]), sizeof(buf1));
				strlcpy(p_tmp_ext, nvram_safe_get(&buf_ext[0]), sizeof(buf1_ext));
				/*to avoid nvram that it has not been inited ready*/
				if(0 != *p_tmp){
					ipsec_prof_fill(i-1, p_tmp,prof_count);
					if(0 != *p_tmp_ext)
					ipsec_prof_fill_ext(i-1, p_tmp_ext,prof_count);
					rc = 1;
				}
			}
	    }
	}

    return rc;
}

int pre_ipsec_samba_prof_set()
{
	char buf[SZ_BUF];
    char *p_tmp = NULL;
    int rc = 0;
	int i;
    memset((ipsec_samba_t *)&samba_prof, 0, sizeof(ipsec_samba_t));
    p_tmp = &buf[0];
    memset(p_tmp, '\0', sizeof(char) * SZ_BUF);
	
	for(i = 0; i < MAX_PROF_NUM; i++){
		if((0 != strcmp(prof[PROF_SVR][i].samba_settings, "null")) && IPSEC_CONN_EN_UP == prof[PROF_SVR][i].ipsec_conn_en
			&& VPN_TYPE_HOST_NET == prof[PROF_SVR][i].vpn_type){
				strcpy(p_tmp, prof[PROF_SVR][i].samba_settings);
        //DBG(("pre_ipsec_samba_prof_set#%s\n", p_tmp));
        if('\0' != *p_tmp){
            ipsec_samba_prof_fill(p_tmp);
            rc = 1;
        }
				break;
		}
    }
    //DBG(("ipsec_samba#%s[p_tmp]\n", p_tmp));
	/*DBG(("dns1:%s\n, dns2:%s\n, nbns1=%s\n, nbns2=%s\n",
     samba_prof.dns1, samba_prof.dns2, samba_prof.nbios1, samba_prof.nbios2));*/
    return rc;
}


void rc_ipsec_conf_set()
{
    int rc = 0;

    rc = pre_ipsec_prof_set();
    if(0 < rc) {
        rc_ipsec_topology_set();
    }
    return;
}

/*
void rc_ipsec_secrets_init()
{
    FILE *fp = NULL;

    fp = fopen("/tmp/etc/ipsec.secrets", "w");

    fprintf(fp,"#/etc/ipsec.secrets set\n"
                " : PSK 1234567\n\n"
                "test_secret : XAUTH 1234\n"
                "tingya_secret : EAP 1234\n");
    if(NULL != fp){
        fclose(fp);
    }
    return;
}
*/

void rc_strongswan_conf_set()
{
	FILE *fp;
	char *user;
	int rc;

	fp = fopen("/etc/strongswan.conf", "w");
	if (fp == NULL)
		return;

	user = nvram_safe_get("http_username");
	if (*user == '\0')
		user = "admin";

	fprintf(fp,
		"charon {\n"
		"	user = %s\n"
		"	threads = %d\n"
		"	send_vendor_id = yes\n"
		"	interfaces_ignore = %s\n"
		"	starter { load_warning = no }\n"
		"	load_modular = yes\n"
		"	i_dont_care_about_security_and_use_aggressive_mode_psk = yes\n"
		"	plugins {\n"
		"		include strongswan.d/charon/*.conf\n"
		"	}\n"
		"	filelog {\n"
		"		charon {\n"
		"			path = /var/log/strongswan.charon.log\n"
		"			time_format = %%b %%e %%T\n"
		"			default = %d\n"
		"			append = no\n"
		"			flush_line = yes\n"
		"		}\n"
		"	}\n",
		user,
		nvram_get_int("ipsec_threads_num") ? : 8,
		nvram_safe_get("lan_ifname"),
		nvram_get_int("ipsec_log_level") ? : 1);

	rc = pre_ipsec_samba_prof_set();
	if (rc != 0) {
		if (('n' != samba_prof.dns1[0]) && ('\0' != samba_prof.dns1[0]))
			fprintf(fp,"	dns1=%s\n", samba_prof.dns1);
		if (('n' != samba_prof.dns2[0]) && ('\0' != samba_prof.dns2[0]))
			fprintf(fp,"	dns2=%s\n", samba_prof.dns2);
		if (('n' != samba_prof.nbios1[0]) && ('\0' != samba_prof.nbios1[0]))
			fprintf(fp,"	nbns1=%s\n", samba_prof.nbios1);
		if (('n' != samba_prof.nbios2[0]) && ('\0' != samba_prof.nbios2[0]))
			fprintf(fp,"	nbns2=%s\n", samba_prof.nbios2);
	}

	fprintf(fp, "}\n");
	fclose(fp);

	run_postconf("strongswan","/etc/strongswan.conf");
}

void rc_ipsec_ca2ipsecd_cp(FILE *fp, uint32_t idx)
{
    fprintf(fp, "cp -r %s%d_asusCert.pem /tmp/etc/ipsec.d/cacerts/\n"
                "cp -r %s%d_svrCert.pem /tmp/etc/ipsec.d/certs/\n"
                "cp -r %s%d_svrKey.pem /tmp/etc/ipsec.d/private/\n"
                "cp -r %s%d_cliCert.pem /tmp/etc/ipsec.d/certs/\n"
                "cp -r %s%d_cliKey.pem /tmp/etc/ipsec.d/private/\n"
                "echo %s > %s%d_p12.pwd\n",
                FILE_PATH_CA_ETC, idx, FILE_PATH_CA_ETC, idx,
                FILE_PATH_CA_ETC, idx, FILE_PATH_CA_ETC, idx,
                FILE_PATH_CA_ETC, idx, ca_tab[idx].p12_pwd,
                FILE_PATH_CA_ETC, idx);
    return;
}


void rc_ipsec_pki_gen_exec(uint32_t idx)
{
    FILE *fp = NULL;
	char *argv[3];
	argv[0] = "/bin/sh";
	argv[1] = FILE_PATH_CA_GEN_SH"&";
	argv[2] = NULL;

    fp = fopen(FILE_PATH_CA_GEN_SH, "a+w");

    if(NULL != fp){
        rc_ipsec_ca2ipsecd_cp(fp, idx);
        fclose(fp);
        DBG(("to run %s in the background!\n", FILE_PATH_CA_GEN_SH));
        //system("."FILE_PATH_CA_GEN_SH"&");
        _eval(argv, NULL, 0, NULL);
    }
    return;
}

void rc_ipsec_start(FILE *fp)
{
    if(NULL != fp){
        fprintf(fp,"ipsec start > /dev/null 2>&1 \n");
#if defined(RTCONFIG_QUICKSEC)
		fprintf(fp, "quicksecpm -f /tmp/quicksecpm.xml -O /tmp/quicksecpm.log -d\n");
#endif
    }
    return;
}

void rc_ipsec_up(FILE *fp, int prof_idx, ipsec_prof_type_t prof_type)
{
    if((NULL != fp) && ('\0' != prof[prof_type][prof_idx].profilename[0])){
        //fprintf(fp, "ipsec up %s & \n", prof[prof_type][prof_idx].profilename);
#if defined(RTCONFIG_QUICKSEC)
		//fprintf(fp, "quicksecpm -f /tmp/%s.xml -d\n", prof[prof_type][prof_idx].profilename);
#endif
    }
    return;
}

void rc_ipsec_down(FILE *fp, int prof_idx, ipsec_prof_type_t prof_type)
{
    if((NULL != fp) && ('\0' != prof[prof_type][prof_idx].profilename[0])){
        fprintf(fp, "ipsec down %s &\n", prof[prof_type][prof_idx].profilename);
    }
    return;
}

void rc_ipsec_restart(FILE *fp)
{
    if(NULL != fp){
        /*to do this command after ipsec reload command has been exec*/
        fprintf(fp, "ipsec restart > /dev/null 2>&1\n"
        		"sleep 3 > /dev/null 2>&1\n");
    }
    return;
}

void rc_ipsec_reload(FILE *fp)
{
    if(NULL != fp){
        /* to do this command after ipsec config has been modified */
        /* and ipsec pthread has already run                       */
        fprintf(fp, "ipsec reload > /dev/null 2>&1\n"
        		"sleep 1 > /dev/null 2>&1\n");
    }
#if defined(RTCONFIG_QUICKSEC)
	killall("quicksecpm", SIGHUP);
#endif
	
    return;
}

void rc_ipsec_rereadall(FILE *fp)
{
    if(NULL != fp){
        fprintf(fp, "ipsec rereadall > /dev/null 2>&1\n"
        		"sleep 1 > /dev/null 2>&1\n");
    }
    return;
}

void rc_ipsec_stop(FILE *fp)
{
    if(NULL != fp){
        /*Disabled ipsec*/
        fprintf(fp, "ipsec stop > /dev/null 2>&1\n"
        		"sleep 1 > /dev/null 2>&1\n");
#if defined(RTCONFIG_QUICKSEC)
		//fprintf(fp, "killall quicksecpm\n");
#endif
    }
#if defined(RTCONFIG_QUICKSEC)	
	if (pids("quicksecpm"))
		killall("quicksecpm", SIGTERM);
#endif
    return;
}

int rc_ipsec_ca_fileidx_check()
{
    char awk_cmd[SZ_BUF], str_buf[SZ_MIN];
    FILE *fp = NULL;
    uint32_t file_idx_32bits = 0, i_tmp = 0;
	char *argv[3];
	
	argv[0] = "/bin/sh";
	argv[1] = FILE_PATH_CA_CHECK_SH;
	argv[2] = NULL;
    
    fp = fopen(FILE_PATH_CA_CHECK_SH, "w");
    
    sprintf(awk_cmd, "awk '{for ( x = 0; x < %d; x++ )"
                     "{ if($1 == x\"_cliKey.pem\")"
                     "{ print x}}}' %s > %s",CA_FILES_MAX_NUM,
                      FILE_PATH_CA_PKI_TEMP, FILE_PATH_CA_TEMP);
    fprintf(fp, "ls %s > %s\n%s\n", FILE_PATH_CA_ETC, 
                                    FILE_PATH_CA_PKI_TEMP, awk_cmd);
    if(NULL != fp){
        fclose(fp);
    }
	chmod(FILE_PATH_CA_CHECK_SH, 0777);
    //system("."FILE_PATH_CA_CHECK_SH);
    _eval(argv, NULL, 0, NULL);
	
    fp = fopen(FILE_PATH_CA_TEMP, "r");
    if(NULL == fp){
        return file_idx_32bits;
    }
    memset(str_buf, 0, SZ_MIN);
    while(EOF != fscanf(fp, "%s", str_buf)){
        i_tmp = atoi(str_buf);
        file_idx_32bits |= (uint32_t)(1 << i_tmp);
        memset(str_buf, 0, SZ_MIN);
    }
    if(NULL != fp){
        fclose(fp);
    }
    return file_idx_32bits;
}

int rc_ipsec_ca_fileidx_available_get()
{
    unsigned int idx = 0;
    uint32_t file_idx_32bits = 0;

    file_idx_32bits = rc_ipsec_ca_fileidx_check();
    DBG(("__rc_ipsec_ca_fileidx_available_get:0x%x\n", file_idx_32bits));
    for(idx = 0; idx < CA_FILES_MAX_NUM; idx++){
        if(0 == (file_idx_32bits & (uint32_t)(1 << idx))){
            file_idx_32bits |= (uint32_t)(1 << idx);
            DBG(("file_idx_32bits:0x%x\n", file_idx_32bits));
            return idx;
        }
    }
    return CA_FILES_MAX_NUM;
}

/*return value : ca_file's index*/
int rc_ipsec_ca_txt_parse()
{
    char buf[SZ_BUF];
    char *p_tmp = NULL, *p_buf = NULL; 
    int file_idx = 0;
    /*ca index star from 0*/
    sprintf(buf, "%s", nvram_safe_get("ca_manage_profile"));
    p_buf = &buf[0];
   
    file_idx = rc_ipsec_ca_fileidx_available_get();

    if(CA_FILES_MAX_NUM == file_idx){
        DBG(("ca files reach the maxmum numbers(%d)", file_idx));
        return file_idx;
    }
 
    p_tmp = (char *)&(ca_tab[file_idx].p12_pwd[0]);
    while('>' != *p_buf){
        *p_tmp = *p_buf;
        p_tmp++;
        p_buf++;
    }
    *p_tmp = '\0';
    
    p_tmp = (char *)&(ca_tab[file_idx].ca_txt[0]);
    while('>' != *(++p_buf)){
        *p_tmp = *p_buf;
        p_tmp++;
    }
    *p_tmp = '\0';

    p_tmp = (char *)&(ca_tab[file_idx].san[0]);
    while('\0' != *(++p_buf)){
        *p_tmp = *p_buf;
        p_tmp++;
    }
    *p_tmp = '\0';
    /*to re-parsing root CA cert for server,client cert*/
    p_tmp = (char *)&(ca_tab[file_idx].ca_cert[0]);
    p_buf = (char *)&(ca_tab[file_idx].ca_txt[0]);

    memset(p_tmp, 0, SZ_BUF);
    while('\0' != *p_buf){
        if(('C' == *p_buf) && ('N' == *(p_buf + 1)) && ('=' == *(p_buf + 2))){
            break;
        }
        *p_tmp++ = *p_buf++;
    }

    *p_tmp = '\0';
    return file_idx;
}

int rc_ipsec_ca_gen()
{
    FILE *fp = NULL;
    int file_idx = 0;

    fp = fopen(FILE_PATH_CA_GEN_SH,"w");
    file_idx = rc_ipsec_ca_txt_parse();
    fprintf(fp,"#%s\npki --gen --outform pem > "FILE_PATH_CA_ETC"%d_ca.pem \n"
               "pki --self --in "FILE_PATH_CA_ETC"%d_ca.pem --dn \"%s\""
               " --ca --outform pem > "FILE_PATH_CA_ETC"%d_asusCert.pem\n"
               "pki --gen --outform pem > "FILE_PATH_CA_ETC"%d_svrKey.pem\n"
               "pki --pub --in "FILE_PATH_CA_ETC"%d_svrKey.pem | "
               "pki --issue --cacert "FILE_PATH_CA_ETC"%d_asusCert.pem"
               " --cakey "FILE_PATH_CA_ETC"%d_ca.pem --dn \"%s CN=%s\""
               " --san=%s --outform pem > "FILE_PATH_CA_ETC"%d_svrCert.pem\n"
               "pki --gen --outform pem > "FILE_PATH_CA_ETC"%d_cliKey.pem\n"
               "pki --pub --in "FILE_PATH_CA_ETC"%d_cliKey.pem | "
               "pki --issue --cacert "FILE_PATH_CA_ETC"%d_asusCert.pem"
               " --cakey "FILE_PATH_CA_ETC"%d_ca.pem --dn \"%s CN=@client\" "
               "--san=@client --outform pem > %s%d_cliCert.pem\n"
               "export RANDFILE="FILE_PATH_CA_ETC".rnd\n"
               "openssl pkcs12 -export -inkey %s%d_cliKey.pem "
               " -in %s%d_cliCert.pem -name \"client Cert\" "
               " -certfile %s%d_asusCert.pem -caname \"ASUS Root CA\""
               " -out %s%d_cliCert.p12 -password pass:%s\n",
               FILE_PATH_CA_GEN_SH, file_idx, 
               file_idx, ca_tab[file_idx].ca_txt,
               file_idx,file_idx, file_idx, file_idx,
               file_idx, ca_tab[file_idx].ca_cert, ca_tab[file_idx].san,
               ca_tab[file_idx].san, file_idx, file_idx, file_idx, file_idx,
               file_idx, ca_tab[file_idx].ca_cert, FILE_PATH_CA_ETC, file_idx,
               FILE_PATH_CA_ETC, file_idx, FILE_PATH_CA_ETC, file_idx,
               FILE_PATH_CA_ETC, file_idx, FILE_PATH_CA_ETC, file_idx,
               ca_tab[file_idx].p12_pwd);
    if(NULL != fp){
        fclose(fp);
    }
	chmod(FILE_PATH_CA_GEN_SH, 0777);
    return file_idx;
}

void rc_ipsec_ca_import(uint32_t ca_type, FILE *fp)
{
#if 0
    for(i = 1; i <= MAX_PROF_NUM; i++){
        memset(ca_prof_name, 0, sizeof(char) * SZ_MIN);
        sprintf(ca_prof_name, "ipsec_ca_%d", i);
        if(NULL != nvram_safe_get(ca_prof_name)){
            memset(cert_name, 0, sizeof(char) * SZ_MIN);
            sprintf(cert_name, "/tmp/etc/ipsec.d/certs/ipsec_cert_%d.pem", i);
            fp = fopen(cert_name, "w");

            fprintf(fp, "%s", nvram_safe_get(ca_prof_name));
            if(NULL != fp){
                fclose(fp);
            }
        }
    }
#endif
    switch(ca_type){
    case CA_TYPE_CACERT:
    break;
    case CA_TYPE_CERT:
    break;
    case CA_TYPE_PRIVATE_KEY:
    break;
    case CA_TYPE_PKS12:
        /*to get verify code(password)*/
    break;
    }

    return; 
}

void rc_ipsec_cert_import(char *asus_cert, char *ipsec_cli_cert,
                          char *ipsec_cli_key, char *pks12)
{
    char *p_file = NULL;
    char cmd[SZ_BUF], tmp[SZ_MIN];
    if(NULL != asus_cert){
        memset(cmd, 0, sizeof(char) * SZ_BUF);
        sprintf(&cmd[0], "cp -r %s /tmp/etc/ipsec.d/cacerts/", asus_cert);
        system(cmd);
    }
    if(NULL != ipsec_cli_cert){
        memset(cmd, 0, sizeof(char) * SZ_BUF);
        sprintf(&cmd[0], "cp -r %s /tmp/etc/ipsec.d/certs/", ipsec_cli_cert);
        system(cmd);
    }
    if(NULL != ipsec_cli_key){
        memset(cmd, 0, sizeof(char) * SZ_BUF);
        sprintf(&cmd[0], "cp -r %s /tmp/etc/ipsec.d/private/", ipsec_cli_key);
        system(cmd);
    }
    if(NULL != pks12){
        if(NULL != nvram_safe_get("ca_manage_profile")){
            DBG(("pks12:%s", pks12));
            p_file = &tmp[0];
            memset(p_file, '\0', sizeof(char) * SZ_MIN);
            strncpy(p_file, pks12, strlen(pks12) - 4); /*4 : strlen(p12)*/
            sprintf(cmd, "echo %s > "FILE_PATH_CA_ETC"%s.pwd", 
                    nvram_safe_get("ca_manage_profile"), p_file);
            system(cmd);
            memset(cmd, 0, sizeof(char) * SZ_BUF);
            sprintf(&cmd[0], "openssl pkcs12 -in %s%s -clcerts -out "
                             "/tmp/etc/ipsec.d/certs/%s.pem -password pass:%s", 
                             FILE_PATH_CA_ETC, pks12, p_file, 
                             nvram_safe_get("ca_manage_profile"));
            DBG((cmd));
            system(cmd);
            memset(cmd, 0, sizeof(char) * SZ_BUF);
            sprintf(&cmd[0], "openssl pkcs12 -in %s%s -cacerts -out "
                             "/tmp/etc/ipsec.d/cacerts/%s.pem"
                             " -password pass:%s",
                             FILE_PATH_CA_ETC, pks12, p_file,
                             nvram_safe_get("ca_manage_profile"));
            DBG((cmd));
            system(cmd);
            memset(cmd, 0, sizeof(char) * SZ_BUF);
            sprintf(&cmd[0], "openssl pkcs12 -in %s%s -out "
                    "/tmp/etc/ipsec.d/certs/%s.pem -nodes -password pass:%s",
                     FILE_PATH_CA_ETC, pks12, p_file,
                     nvram_safe_get("ca_manage_profile"));
            DBG((cmd));
            system(cmd);
        }
    }
    return;
}

void rc_ipsec_ca_export(char *verify_pwd)
{
    char cmd[SZ_BUF];
    if(NULL != verify_pwd){
        sprintf(&cmd[0], "openssl pkcs12 -export -inkey ipsec_cliKey.pem "
                " -in ipsec_cliCert.pem -name \"IPSEC client\" "
                " -certfile asusCert.pem -caname \"ASUS Root CA\""
                " -out ipsec_cliCert.p12");
        system(cmd);
        system(verify_pwd);
    }
    return;
}

void rc_ipsec_gen_cert(int skip_checking)
{
    FILE *fp = NULL;
    struct utsname uts;
    char device_cn[64] = {0};
    char ddns_name[128] = {0}, remote_id[128] = {0}, prefix[16] = {0};
    int ca_lifetime = 2200;

    if((skip_checking == 0) && check_if_file_exist(FILE_PATH_CA_ETC FILE_NAME_CERT_PEM)&&check_if_file_exist(FILE_PATH_CA_ETC FILE_NAME_SVR_PRIVATE_KEY)&&check_if_file_exist(FILE_PATH_CA_ETC FILE_NAME_SVR_CERT_PEM)){
        DBG(("CA files are ready there.\n"));
        return;
    }

    if((skip_checking == 0) && (!nvram_match("ntp_ready", "1")))
    {
        DBG(("NTP is not synced yet, skip generating CA files.\n"));
        return;
    }

    DBG(("Generate CA files.\n"));
    uname(&uts);
    snprintf(device_cn, sizeof(device_cn), "%s", uts.nodename);
    if(strlen(device_cn) == 0){
        snprintf(device_cn, sizeof(device_cn), "%s", nvram_safe_get("odmpid"));
        if(strlen(device_cn) == 0)
        {
            snprintf(device_cn, sizeof(device_cn), "%s", nvram_safe_get("productid"));
        }
    }

    if(nvram_get_int("ipsec_ca_lifetime") > 0){
        ca_lifetime = nvram_get_int("ipsec_ca_lifetime");
    }

    snprintf(ddns_name, sizeof(ddns_name), "%s", nvram_safe_get("ddns_hostname_x"));
    if(strlen(ddns_name) == 0 )
    {
        snprintf(prefix, sizeof(prefix), "wan%d_", get_active_wan_unit());
        snprintf(remote_id, sizeof(remote_id), "%s", nvram_pf_safe_get(prefix, "ipaddr"));
        if(strlen(remote_id) == 0){
            DBG(("[Error]wan ip is not set yet, no any CAs will be created.\n"));
            return;
        }
    }else
        strlcpy(remote_id, ddns_name, sizeof(remote_id));

    fp = fopen(FILE_PATH_CA_ETC"generate.sh", "w");
    if(NULL != fp){
        fprintf(fp, "#!/bin/sh\n\n");
        fprintf(fp, "pki --gen --size 2048 --outform pem > %s%s\n"
                    "pki --self --in %s%s --dn \"C=TW,O=ASUS,CN=ASUS %s Root CA\" --ca --lifetime %d --outform pem > %s%s\n"
                    "pki --gen --size 2048 --outform pem > %s%s\n"
                    "pki --pub --in %s%s | pki --issue --cacert %s%s --cakey %s%s --dn \"C=TW,O=ASUS,CN=%s\" --san=\"%s\" --lifetime %d --outform pem > %s%s\n\n"
                    "openssl x509 -in %s%s -outform der -out %s%s\n\n",
                    FILE_PATH_CA_ETC, FILE_NAME_CA_PRIVATE_KEY,
                    FILE_PATH_CA_ETC, FILE_NAME_CA_PRIVATE_KEY, trimNL(device_cn), ca_lifetime, FILE_PATH_CA_ETC, FILE_NAME_CERT_PEM,
                    FILE_PATH_CA_ETC, FILE_NAME_SVR_PRIVATE_KEY,
                    FILE_PATH_CA_ETC, FILE_NAME_SVR_PRIVATE_KEY, FILE_PATH_CA_ETC, FILE_NAME_CERT_PEM, FILE_PATH_CA_ETC, FILE_NAME_CA_PRIVATE_KEY, remote_id, remote_id, ca_lifetime, FILE_PATH_CA_ETC, FILE_NAME_SVR_CERT_PEM,
                    FILE_PATH_CA_ETC, FILE_NAME_CERT_PEM, FILE_PATH_CA_ETC, FILE_NAME_CERT_DER
                    );
        fclose(fp);
    }
    chmod(FILE_PATH_CA_ETC"generate.sh", 0777);
    system(FILE_PATH_CA_ETC"generate.sh");
}

void rc_ipsec_ca_init( )
{
    FILE *fp = NULL;

    fp = fopen(FILE_PATH_CA_ETC"ca_init.sh", "w");
    if(NULL != fp){
        fprintf(fp, "#!/bin/sh\n\n");
        fprintf(fp, "cp -r %sasusCert.pem /tmp/etc/ipsec.d/cacerts/\n"
                    "cp -r %ssvrCert.pem /tmp/etc/ipsec.d/certs/\n"
                    "cp -r %ssvrKey.pem /tmp/etc/ipsec.d/private/\n",
                    FILE_PATH_CA_ETC, FILE_PATH_CA_ETC, FILE_PATH_CA_ETC);
        fclose(fp);
    }
    chmod(FILE_PATH_CA_ETC"ca_init.sh", 0777);
    system(FILE_PATH_CA_ETC"ca_init.sh");
}

void rc_ipsec_conf_default_init()
{
	FILE *fp;

	fp = fopen("/etc/ipsec.conf", "w");
	if (fp == NULL)
		return;

	/* default */
	fprintf(fp,
		"config setup\n"
		"conn %%default\n"
		"	ikelifetime=60m\n"
		"	keylife=20m\n"
		"	rekeymargin=3m\n"
		"	keyingtries=1\n"
		"	keyexchange=ike\n"
		"\n");

	fclose(fp);
}

void rc_ipsec_psk_xauth_rw_init()
{
	FILE *fp;

	fp = fopen("/etc/ipsec.conf", "a+w");
	if (fp == NULL)
		return;

	/* also supports iOS PSK and Shrew on Windows */
	fprintf(fp,
		"conn android_xauth_psk\n"
		"	keyexchange=ikev1\n"
		"	left=%%defaultroute\n"
		"	leftauth=psk\n"
		"	leftsubnet=0.0.0.0/0\n"
		"	right=%%any\n"
		"	rightauth=psk\n"
		"	rightauth2=xauth\n"
		"	rightsourceip=10.2.1.0/24\n"
		"	auto=add\n"
		"\n");

	fclose(fp);
}

void rc_ipsec_secrets_set()
{
	char ipsec_client_list_name[SZ_MIN] = {0}, buf[SZ_MAX] = {0}, s_tmp[SZ_MAX] = {0};
	char auth2meth[SZ_MIN] = {0};
#ifdef RTCONFIG_INSTANT_GUARD
	char ig_client_list[1024] = {0}, ig_client_buf[128] = {0};
	char *desc = NULL, *ts = NULL, *active = NULL;
#endif
	char ipsec_client_list_buf[1024] = {0}, ig_client_list_tmp[1024] = {0};
	char *name = NULL, *passwd = NULL;
	char word[1024] = {0}, *word_next = NULL;
	int i,prof_count = 0, unit;
	FILE *fp = NULL;
	//char word[80], *next;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	fp = fopen("/tmp/etc/ipsec.secrets", "w");
	if(!fp)
	{
		DBG(("Cannot open ipsec.secrets!\n"));
		return;
	}
	fprintf(fp,"#/etc/ipsec.secrets\n\n");

	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
		for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_DOWN != prof[prof_count][i].ipsec_conn_en){
				if((0 != strcmp(prof[prof_count][i].auth_method_key, "null")) &&
					('\0' != prof[prof_count][i].auth_method_key[0]) &&
					((1 == prof[prof_count][i].auth_method) || (0 == prof[prof_count][i].auth_method))){
					if(strcmp(prof[prof_count][i].local_public_interface,"wan") == 0){
						strcpy(prof[prof_count][i].local_pub_ip,nvram_safe_get("wan0_ipaddr"));
					}
					else if(strcmp(prof[prof_count][i].local_public_interface,"wan2") == 0){
						strcpy(prof[prof_count][i].local_pub_ip,nvram_safe_get("wan1_ipaddr"));
					}
					else if(strcmp(prof[prof_count][i].local_public_interface,"usb") == 0) {
						for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
							if (dualwan_unit__usbif(unit)) {
								wan_prefix(unit, prefix);
								strcpy(prof[prof_count][i].local_pub_ip,nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
								break;
							}
						}
					}
					else{
						strcpy(prof[prof_count][i].local_pub_ip,nvram_safe_get("lan_ipaddr"));
					}
					fprintf(fp,"\n %s : %s %s\n\n"
						/*fprintf(fp,"\n %s %s : %s %s\n\n"
						, ((0 == strcmp(prof[prof_count][i].local_id, "null") ||
						('\0' == prof[prof_count][i].local_id[0])) ?
						((('\0' == prof[prof_count][i].local_pub_ip[0]) ||
						('n' == prof[prof_count][i].local_pub_ip[0])) ? "\%any" :
						prof[prof_count][i].local_pub_ip ) : prof[prof_count][i].local_id)*/
						, ((0 == strcmp(prof[prof_count][i].remote_id, "null") ||
						('\0' == prof[prof_count][i].remote_id[0])) ?
						((('\0' == prof[prof_count][i].remote_gateway[0]) ||
						('n' == prof[prof_count][i].remote_gateway[0])) ? "\%any" :
						prof[prof_count][i].remote_gateway) : prof[prof_count][i].remote_id)
						, ((0 == prof[prof_count][i].auth_method) ? "RSA" : "PSK")
						, prof[prof_count][i].auth_method_key);
				}
				/*second-factor auth*/
				if((IKE_TYPE_V1 == prof[prof_count][i].ike) &&
				(IPSEC_AUTH2_TYP_CLI == prof[prof_count][i].xauth)){
					fprintf(fp, "#cli[%d]\n %s : XAUTH %s\n", i, prof[prof_count][i].xauth_account
							, prof[prof_count][i].xauth_password);
				}else if((IKE_TYPE_V2 == prof[prof_count][i].ike) &&
						(IPSEC_AUTH2_TYP_CLI == prof[prof_count][i].xauth)){
					fprintf(fp, "#cli[%d]\n %s : EAP %s\n", i, prof[prof_count][i].xauth_account
							, prof[prof_count][i].xauth_password);
				}

				memset(ipsec_client_list_name, 0, sizeof(char) * SZ_MIN);
				snprintf(ipsec_client_list_name, sizeof(ipsec_client_list_name), "ipsec_client_list_%d", i+1);
#ifdef RTCONFIG_INSTANT_GUARD
				if(nvram_get_int("ipsec_ig_enable") == 1)
				{
					strlcpy(ig_client_list, nvram_safe_get("ig_client_list"), sizeof(ig_client_list));
				}
#endif
				//if(NULL != nvram_safe_get(ipsec_client_list_name)){
				if(strlen(nvram_safe_get(ipsec_client_list_name)) > 0
#ifdef RTCONFIG_INSTANT_GUARD
				|| strlen(ig_client_list) > 0
#endif
				){
					memset(buf, 0, sizeof(char) * SZ_MAX);
					memset(s_tmp, 0, sizeof(char) * SZ_MAX);
					memset(ipsec_client_list_buf, 0, sizeof(ipsec_client_list_buf));

					if((VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type) &&
						(prof[prof_count][i].ike == IKE_TYPE_V2) &&
						(prof[prof_count][i].auth_method == 0)){
						snprintf(s_tmp, sizeof(s_tmp), ": RSA %s\n", prof[prof_count][i].leftkey);
					}

					if(nvram_get_int("ipsec_server_enable") == 1){
						strlcpy(buf, nvram_safe_get(ipsec_client_list_name), sizeof(buf));
						foreach_60(word,buf, word_next){
							if((vstrsep(word, ">", &name, &passwd)) != 2)
								continue;

							snprintf(ipsec_client_list_buf, sizeof(ipsec_client_list_buf), "\n%s : %s %s"
									, name, (IKE_TYPE_V2 == prof[prof_count][i].ike) ? "EAP" : "XAUTH", passwd);
							strlcat(s_tmp, ipsec_client_list_buf, sizeof(s_tmp));
						}
					}
#ifdef RTCONFIG_INSTANT_GUARD
					if(nvram_get_int("ipsec_ig_enable") == 1){
						strlcpy(ig_client_list, nvram_safe_get("ig_client_list"), sizeof(ig_client_list));
						memset(ig_client_list_tmp, 0, sizeof(ig_client_list_tmp));
						memset(ig_client_buf, 0, sizeof(ig_client_buf));
						foreach_60(word, ig_client_list, word_next){
							if((vstrsep(word, ">", &name, &passwd, &desc, &ts, &active)) != 5)
								continue;
							if(active != NULL && !strcmp(active, "1")){
								snprintf(ig_client_buf, sizeof(ig_client_buf), "\n%s : %s %s"
										, name, (IKE_TYPE_V2 == prof[prof_count][i].ike) ? "EAP" : "XAUTH", passwd);
								strlcat(ig_client_list_tmp, ig_client_buf, sizeof(ig_client_list_tmp));
							}
						}
					}
#endif
					fprintf(fp, "\n#ipsec_client_list_%d\n\n%s%s\n", i+1, s_tmp, ig_client_list_tmp);
				}
			}
		}
	}
	if(NULL != fp){
		fclose(fp);
	}
	return;
}

void ipsec_conf_local_set(FILE *fp, int prof_idx, ipsec_prof_type_t prof_type)
{
	char left_ipaddr[16];
	char tmp_str[12];
	int unit = 0;
	char word[80], *next;
	/*char lan_class[32];
	
	ip2class(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), lan_class);
	if(0 != strcmp(prof[prof_type][prof_idx].local_subnet,lan_class))
		strcpy(prof[prof_type][prof_idx].local_subnet,lan_class);*/
	
	left_ipaddr[0]='\0';
	if(0 == strcmp(prof[prof_type][prof_idx].local_public_interface, "wan")){
		if(ipv6_enabled())
			strcpy(left_ipaddr, "");	
		else
		strcpy(left_ipaddr, nvram_safe_get("wan0_ipaddr"));
	}
	else if(0 == strcmp(prof[prof_type][prof_idx].local_public_interface, "wan2")){
		strcpy(left_ipaddr, nvram_safe_get("wan1_ipaddr"));
	}
	else if(0 == strcmp(prof[prof_type][prof_idx].local_public_interface, "lan")){
		
		strcpy(left_ipaddr, nvram_safe_get("lan_ipaddr"));
	}
	else if(0 == strcmp(prof[prof_type][prof_idx].local_public_interface, "usb")){
		tmp_str[0]='\0';
		foreach(word, nvram_safe_get("wan_ifnames"), next) {
			if (0 == strcmp(word,"usb")){
				break;
			}
			unit ++;
		}
		sprintf(tmp_str,"wan%d_ipaddr", unit);
		strcpy(left_ipaddr, nvram_safe_get(tmp_str));
	}
	
	if(0 != strlen(left_ipaddr) && 0 != strcmp(left_ipaddr,"0.0.0.0"))
		fprintf(fp, "  left=%s\n  #receive web value#left=%s\n", left_ipaddr, prof[prof_type][prof_idx].local_pub_ip);
	else
		fprintf(fp, "  left=%%defaultroute\n  #receive web value#left=%s\n", prof[prof_type][prof_idx].local_pub_ip);
	
	
	fprintf(fp, "  leftsubnet=%s\n"
                "  leftfirewall=%s\n  #interface=%s\n"
                , (VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type) ?
                  "0.0.0.0/0" : prof[prof_type][prof_idx].local_subnet
                , ((VPN_TYPE_NET_NET_CLI == prof[prof_type][prof_idx].vpn_type) || 
                   (prof[prof_type][prof_idx].traversal == 1)) ? "yes" : "no"
                , prof[prof_type][prof_idx].local_public_interface
           );
    if((0 != prof[prof_type][prof_idx].local_port) && (nvram_get_int("ipsec_isakmp_port") != prof[prof_type][prof_idx].local_port) &&
       (nvram_get_int("ipsec_nat_t_port") != prof[prof_type][prof_idx].local_port)){
        fprintf(fp, "  leftprotoport=17/%d\n", prof[prof_type][prof_idx].local_port);
    }
    if(IKE_TYPE_AUTO != prof[prof_type][prof_idx].ike){
        fprintf(fp, "  leftauth=%s\n"
                , (prof[prof_type][prof_idx].auth_method == 1) ? "psk" :
                   ((prof[prof_type][prof_idx].ike == IKE_TYPE_V2) ?
                     prof[prof_type][prof_idx].leftauth_method : "pubkey")
               );
    }
    if((VPN_TYPE_NET_NET_CLI == prof[prof_type][prof_idx].vpn_type) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V1) && 
       (IPSEC_AUTH2_TYP_DIS != prof[prof_type][prof_idx].xauth)){
        fprintf(fp, "  rightauth2=xauth\n");
    }else if((VPN_TYPE_NET_NET_CLI == prof[prof_type][prof_idx].vpn_type) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V2) && 
       (IPSEC_AUTH2_TYP_DIS != prof[prof_type][prof_idx].xauth)){
        fprintf(fp, "  rightauth2=%s\n", prof[prof_type][prof_idx].rightauth2_method);
    }
    if((0 != strcmp(prof[prof_type][prof_idx].local_id, "null")) && 
       ('\0' != prof[prof_type][prof_idx].local_id[0])){
        fprintf(fp, "  leftid=%s\n", prof[prof_type][prof_idx].local_id);
    }
	
    if((VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V2) &&
       (prof[prof_type][prof_idx].auth_method == 0)){
        fprintf(fp, "  leftcert=%s\n", prof[prof_type][prof_idx].leftcert);
    }
    if((VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V2) &&
       (prof[prof_type][prof_idx].auth_method == 0)){
        fprintf(fp, "  #leftsendcert is the key point for iOS devices\n");
        fprintf(fp, "  leftsendcert=%s\n", prof[prof_type][prof_idx].leftsendcert);
    }
    if((VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V2) &&
       (prof[prof_type][prof_idx].auth_method == 0)){
        fprintf(fp, "  eap_identity=%s\n", prof[prof_type][prof_idx].eap_identity);
    }

    return;
}

void ipsec_conf_remote_set(FILE *fp, int prof_idx, ipsec_prof_type_t prof_type)
{
    if((VPN_TYPE_NET_NET_SVR == prof[prof_type][prof_idx].vpn_type) || 
       (VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type)){
        fprintf(fp, "  right=%%any\n");
    }else{
        fprintf(fp, "  right=%s\n", prof[prof_type][prof_idx].remote_gateway);
    }

    if((0 != prof[prof_type][prof_idx].remote_port) && (nvram_get_int("ipsec_isakmp_port") != prof[prof_type][prof_idx].remote_port) 
        && (nvram_get_int("ipsec_nat_t_port") != prof[prof_type][prof_idx].remote_port)){
        fprintf(fp, "  rightprotoport=17/%d\n", prof[prof_type][prof_idx].remote_port);
    }

    if(VPN_TYPE_HOST_NET != prof[prof_type][prof_idx].vpn_type){
        fprintf(fp, "  rightsubnet=%s\n",prof[prof_type][prof_idx].remote_subnet);
    }
    if(IKE_TYPE_AUTO != prof[prof_type][prof_idx].ike){
        fprintf(fp, "  rightauth=%s\n"
                  , (prof[prof_type][prof_idx].auth_method == 1) ? "psk" :
                     ((prof[prof_type][prof_idx].ike == IKE_TYPE_V2) ?
                     prof[prof_type][prof_idx].rightauth2_method : "pubkey")
               );
    }
    if(((VPN_TYPE_NET_NET_SVR == prof[prof_type][prof_idx].vpn_type) ||
        (VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type)) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V1) && 
       (IPSEC_AUTH2_TYP_DIS != prof[prof_type][prof_idx].xauth)){
        fprintf(fp, "  rightauth2=xauth\n");
    }else if(((VPN_TYPE_NET_NET_SVR == prof[prof_type][prof_idx].vpn_type) ||
        (VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type)) &&
       (prof[prof_type][prof_idx].ike == IKE_TYPE_V2) &&
       (IPSEC_AUTH2_TYP_DIS != prof[prof_type][prof_idx].xauth)){
        fprintf(fp, "  rightauth2=%s\n", prof[prof_type][prof_idx].rightauth2_method);
    }
    if(VPN_TYPE_HOST_NET == prof[prof_type][prof_idx].vpn_type){
        if((0 != strcmp(prof[prof_type][prof_idx].virtual_subnet, "null")) &&
           ('\0' != prof[prof_type][prof_idx].virtual_subnet[0])){
            fprintf(fp, "#sourceip_en=%s\n  rightsourceip=%s\n"
                 , prof[prof_type][prof_idx].virtual_ip_en, prof[prof_type][prof_idx].virtual_subnet);
	    fprintf(fp, "  rightdns=%s\n", nvram_safe_get("lan_ipaddr"));
        }
    }
    if((0 != strcmp(prof[prof_type][prof_idx].remote_id, "null")) &&
       ('\0' != prof[prof_type][prof_idx].remote_id[0])){
        fprintf(fp, "  rightid=%s\n", prof[prof_type][prof_idx].remote_id);
    }
	
    return;
}

char* get_ike_esp_bit_convert(char *str, int size, int maxNum, int n, int type)
{
	int i;
	char tmpStr[12];
	memset(str, 0, size);
	for(i = 0; i < maxNum; i++){
		if((n >> i) & 0x1 ){
			if(type == FLAG_IKE_ENCRYPT)
				sprintf(tmpStr, "-%s", encryp[i]);
			else if(type == FLAG_ESP_HASH)
				sprintf(tmpStr, "-%s", hash[i]);
			else if(type == FLAG_DH_GROUP)
				sprintf(tmpStr, "-%s", dh_group[i]);
			strcat(str, tmpStr);
		}
	}
	return str;
}
char* get_ike_esp_bit_convert1(char *str, int size, int n1, int n2, int n3)
{
	int i,j,k;
	char tmpStr[SZ_MIN];
	memset(str, 0, size);
	memset(tmpStr, 0, sizeof(tmpStr));
	for(i = 0; i < ENCRYPTION_TYPE_MAX_NUM; i++){
		for(j = 0; j < HASH_TYPE_MAX_NUM; j++){
			if(n3 !=0){
				for(k = 0; k < DH_GROUP_MAX_NUM; k++){
					if(((n1 >> i) & 0x1) && ((n2 >> j) & 0x1) && ((n3 >> k) & 0x1)){
						sprintf(tmpStr, ",%s-%s-%s", encryp[i], hash[j], dh_group[k]);
						strcat(str, tmpStr);
					}
				}
			}
			else{
				if(((n1 >> i) & 0x1) && ((n2 >> j) & 0x1)){
					sprintf(tmpStr, ",%s-%s", encryp[i], hash[j]);
					strcat(str, tmpStr);
				}
			}
		}
	}
	return str;
}

void ipsec_conf_phase1_set(FILE *fp, int prof_idx, ipsec_prof_type_t prof_type)
{
	char str[1024];
	memset(str, 0, sizeof(str));
    fprintf(fp, "  ikelifetime=%d\n", prof[prof_type][prof_idx].keylife_p1);
	fprintf(fp, "  ike=%s!\n", get_ike_esp_bit_convert1(str, sizeof(str), prof[prof_type][prof_idx].encryption_p1_ext, prof[prof_type][prof_idx].hash_p1_ext, prof[prof_type][prof_idx].dh_group) + 1);
	//fprintf(fp, "  ike=%s", get_ike_esp_bit_convert(str, sizeof(str), ENCRYPTION_TYPE_MAX_NUM, prof[prof_type][prof_idx].encryption_p1_ext, FLAG_IKE_ENCRYPT) + 1);
	//fprintf(fp, "%s", get_ike_esp_bit_convert(str, sizeof(str), HASH_TYPE_MAX_NUM, prof[prof_type][prof_idx].hash_p1_ext, FLAG_ESP_HASH));
	//fprintf(fp, "%s\n", get_ike_esp_bit_convert(str, sizeof(str), DH_GROUP_MAX_NUM, prof[prof_type][prof_idx].dh_group, FLAG_DH_GROUP));

    /*if((ENCRYPTION_TYPE_MAX_NUM != prof[prof_type][prof_idx].encryption_p1) &&
       (HASH_TYPE_MAX_NUM != prof[prof_type][prof_idx].hash_p1)){
        fprintf(fp,"  ike=%s-%s-%s\n", encryp[prof[prof_type][prof_idx].encryption_p1]
                  , hash[prof[prof_type][prof_idx].hash_p1], dh_group[DH_GROUP_14]);
    } else if((ENCRYPTION_TYPE_MAX_NUM == prof[prof_type][prof_idx].encryption_p1) && 
              (HASH_TYPE_MAX_NUM != prof[prof_type][prof_idx].hash_p1)){
        fprintf(fp,"  ike=%s-%s-%s,%s-%s-%s!\n", encryp[ENCRYPTION_TYPE_AES128]
                  , hash[prof[prof_type][prof_idx].hash_p1], dh_group[DH_GROUP_14]
                  , encryp[ENCRYPTION_TYPE_3DES]
                  , hash[prof[prof_type][prof_idx].hash_p1], dh_group[DH_GROUP_14]);
    } else if((ENCRYPTION_TYPE_MAX_NUM != prof[prof_type][prof_idx].encryption_p1) && 
              (HASH_TYPE_MAX_NUM == prof[prof_type][prof_idx].hash_p1)){
        fprintf(fp,"  ike=%s-%s-%s,%s-%s-%s!\n", encryp[prof[prof_type][prof_idx].encryption_p1]
                  , hash[HASH_TYPE_SHA1], dh_group[DH_GROUP_14]
                  , encryp[prof[prof_type][prof_idx].encryption_p1]
                  , hash[HASH_TYPE_SHA256], dh_group[DH_GROUP_14]);
    }*/
    return;
}

void ipsec_conf_phase2_set(FILE *fp, int prof_idx, ipsec_prof_type_t prof_type)
{
	char str[1024];
    fprintf(fp, "  keylife=%d\n", prof[prof_type][prof_idx].keylife_p2);
	fprintf(fp, "  esp=%s!\n", get_ike_esp_bit_convert1(str, sizeof(str), prof[prof_type][prof_idx].encryption_p2_ext, prof[prof_type][prof_idx].hash_p2_ext, prof[prof_type][prof_idx].pfs_group) + 1);
	//fprintf(fp, "  esp=%s", get_ike_esp_bit_convert(str, sizeof(str), ENCRYPTION_TYPE_MAX_NUM, prof[prof_type][prof_idx].encryption_p2_ext, FLAG_IKE_ENCRYPT) + 1);
	//fprintf(fp, "%s\n", get_ike_esp_bit_convert(str, sizeof(str), HASH_TYPE_MAX_NUM, prof[prof_type][prof_idx].hash_p2_ext, FLAG_ESP_HASH));

    /*if((ENCRYPTION_TYPE_MAX_NUM != prof[prof_type][prof_idx].encryption_p2) && 
       (HASH_TYPE_MAX_NUM != prof[prof_type][prof_idx].hash_p2)){
        fprintf(fp,"  esp=%s-%s\n", encryp[prof[prof_type][prof_idx].encryption_p2]
                  , hash[prof[prof_type][prof_idx].hash_p2]);
    } else if((ENCRYPTION_TYPE_MAX_NUM == prof[prof_type][prof_idx].encryption_p2) &&
              (HASH_TYPE_MAX_NUM != prof[prof_type][prof_idx].hash_p2)){
        fprintf(fp,"  esp=%s-%s,%s-%s!\n", encryp[ENCRYPTION_TYPE_AES128]
                  , hash[prof[prof_type][prof_idx].hash_p2]
                  , encryp[ENCRYPTION_TYPE_3DES]
                  , hash[prof[prof_type][prof_idx].hash_p2]);
    } else if((ENCRYPTION_TYPE_MAX_NUM != prof[prof_type][prof_idx].encryption_p2) &&
              (HASH_TYPE_MAX_NUM == prof[prof_type][prof_idx].hash_p2)){
        fprintf(fp,"  esp=%s-%s,%s-%s!\n", encryp[prof[prof_type][prof_idx].encryption_p2]
                  , hash[HASH_TYPE_SHA1]
                  , encryp[prof[prof_type][prof_idx].encryption_p2]
                  , hash[HASH_TYPE_SHA256]);
    }*/
    return;
}
void rc_ipsec_topology_set()
{
    int i,prof_count = 0;
    FILE *fp = NULL;
    char *p_tmp = NULL, buf[SZ_MIN];
    p_tmp = &buf[0];

    char *s_tmp = NULL, buf1[SZ_BUF];
    s_tmp = &buf1[0];

    memset(p_tmp, 0, sizeof(char) * SZ_MIN);
    fp = fopen("/tmp/etc/ipsec.conf", "w");
    fprintf(fp,"conn %%default\n  keyexchange=ikev1\n  authby=secret\n  ike=aes256-sha1-modp1024\n");

	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
	    for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_DOWN != prof[prof_count][i].ipsec_conn_en){
			if(PROF_SVR == prof_count)
	        	sprintf(&buf[0], "ipsec_profile_%d", i+1);
			else if(PROF_CLI == prof_count)
				sprintf(&buf[0], "ipsec_profile_client_%d", i+1);
	        memset(s_tmp, '\0', SZ_BUF);
	        if(NULL != nvram_safe_get(&buf[0])){
	            strcpy(s_tmp, nvram_safe_get(&buf[0]));
	        }
	        if('\0' == *s_tmp){
	            memset((ipsec_prof_t *)&prof[prof_count][i], 0, sizeof(ipsec_prof_t));
	            prof[prof_count][i].ipsec_conn_en = IPSEC_CONN_EN_DEFAULT;
	            continue;
	        }
	        if(VPN_TYPE_NET_NET_SVR == prof[prof_count][i].vpn_type){
            	fprintf(fp,"#Net-to-Net VPN SVR[prof#%d]:%s\n\n", i, s_tmp);
	        }
			else if(VPN_TYPE_NET_NET_CLI == prof[prof_count][i].vpn_type){
            	fprintf(fp,"#Net-to-Net VPN CLI[prof#%d]:%s\n\n", i, s_tmp);
	        }
			else if(VPN_TYPE_NET_NET_PEER == prof[prof_count][i].vpn_type){
            	fprintf(fp,"#Net-to-Net PEER[prof#%d]:%s\n\n", i, s_tmp);
	        }
			else if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type){
            	fprintf(fp,"#Host-to-NET[prof#%d]:%s\n\n", i, s_tmp);
	        } 
			else {
            	continue;
        	}
	        fprintf(fp,"\nconn %s\n", prof[prof_count][i].profilename);
	        if(VPN_TYPE_HOST_NET != prof[prof_count][i].vpn_type){
            fprintf(fp,"##enforced UDP encapsulation (forceencaps=yes)\n"
                       "  keyingtries=%d\n  type=%s\n", prof[prof_count][i].keyingtries
                      , prof[prof_count][i].tun_type);
        	}
	        if(IKE_TYPE_AUTO == prof[prof_count][i].ike){
            	fprintf(fp,"  keyexchange=ikev1\n");
	        }
			else{
	            fprintf(fp,"  keyexchange=%s\n", ikev[prof[prof_count][i].ike]);
        	}
		    if(IKE_AGGRESSIVE_MODE == prof[prof_count][i].exchange){
            	fprintf(fp,"  aggressive=yes\n");
        	}

	        ipsec_conf_local_set(fp, i, prof_count);
	        ipsec_conf_remote_set(fp, i, prof_count);
			
	        if(VPN_TYPE_HOST_NET != prof[prof_count][i].vpn_type){
	            ipsec_conf_phase1_set(fp, i, prof_count);
	            ipsec_conf_phase2_set(fp, i, prof_count);
        	}
			else	{
				fprintf(fp,"  ike=%s-%s-%s\n", encryp[ENCRYPTION_TYPE_AES256], hash[HASH_TYPE_SHA1], dh_group[DH_GROUP_2]);
				fprintf(fp,"  dpdtimeout=30s\n");	
			}
			if(DPD_CLEAR == prof[prof_count][i].dead_peer_detection)
				fprintf(fp,"  dpdaction=clear\n");
			else if(DPD_HOLD == prof[prof_count][i].dead_peer_detection)
				fprintf(fp,"  dpdaction=hold\n");
			else if(DPD_RESTART == prof[prof_count][i].dead_peer_detection)
				fprintf(fp,"  dpdaction=restart\n");
			
			if(DPD_NONE != prof[prof_count][i].dead_peer_detection)
				fprintf(fp,"  dpddelay=%ds\n", prof[prof_count][i].ipsec_dpd);
			
	        if(VPN_TYPE_NET_NET_CLI == prof[prof_count][i].vpn_type || VPN_TYPE_NET_NET_PEER == prof[prof_count][i].vpn_type)
				fprintf(fp,"  auto=start\n");
			else
		        fprintf(fp,"  auto=add\n\n");
	    }
	}
	}

    if(NULL != fp){
        fclose(fp);
        run_postconf("ipsec","/etc/ipsec.conf");
    }
    return;
}
void rc_ipsec_nvram_convert_check(void)
{
	int i, prof_count = 0;
	char buf[SZ_MIN], buf_ext[SZ_MIN], tmpStr[SZ_MIN];
	char *nv=NULL, *nvp=NULL, *b=NULL;
	char *encryption_p1=NULL, *hash_p1=NULL, *dh_group=NULL;
	char *encryption_p2=NULL, *hash_p2=NULL, *pfs_group=NULL;
	
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
	    for(i = 1; i <= MAX_PROF_NUM; i++){
			if(PROF_SVR == prof_count){
				sprintf(&buf[0], "ipsec_profile_%d", i);
				sprintf(&buf_ext[0], "ipsec_profile_%d_ext", i);
			}
			else if(PROF_CLI == prof_count){
				sprintf(&buf[0], "ipsec_profile_client_%d", i);
				sprintf(&buf_ext[0], "ipsec_profile_client_%d_ext", i);
			}
			if(0 != strcmp(nvram_safe_get(&buf[0]), "")){
				if(0 == strcmp(nvram_safe_get(&buf_ext[0]),""))	/* if there no ext nvram, filled them. */
				{
				nvp = strdup(nvram_safe_get(&buf[0]));
				b = strsep(&nvp, ">");
				if(0 == strcmp(b, "4"))
					nvram_set(&buf_ext[0], "0>0>0>0>0>0");	/* none */
				else
					nvram_set(&buf_ext[0], "6>6>255>6>6>255");
				}
				else	/* if there is ext nvram but format not match, replace them. */
				{
					nv = nvp = strdup(nvram_safe_get(&buf_ext[0]));
					if (vstrsep(nv, ">", &encryption_p1, &hash_p1, &dh_group, &encryption_p2, &hash_p2, &pfs_group) == 6)
						continue;
					sprintf(tmpStr, "%s>%s>%s>%s>%s>255", encryption_p1, hash_p1, dh_group, encryption_p2, hash_p2);
					nvram_set(&buf_ext[0], tmpStr);
				}
			}

		}
	}
}
void rc_ipsec_config_init(void)
{
    memset((ipsec_samba_t *)&samba_prof, 0, sizeof(ipsec_samba_t));
    memset((ipsec_prof_t *)&prof[0][0], 0, sizeof(ipsec_prof_t) * MAX_PROF_NUM);
    //memset((pki_ca_t *)&ca_tab[0], 0, sizeof(pki_ca_t) * CA_FILES_MAX_NUM);
	memset((ipsec_samba_t *)&pre_samba_prof, 0, sizeof(ipsec_samba_t));
	if(!d_exists("/etc/ipsec.d") || !d_exists("/etc/strongswan.d"))
		system("cp -rf /usr/etc/* /tmp/etc/");
    system("mkdir -p /jffs/ca_files");
    /*ipsec.conf init*/    
    rc_ipsec_conf_default_init();
    rc_ipsec_psk_xauth_rw_init();
    /*ipsec.secrets init*/
    if(nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
         || nvram_get_int("ipsec_ig_enable")
#endif
        )
		rc_ipsec_set(IPSEC_INIT,PROF_ALL);
    //rc_ipsec_secrets_set();
    //rc_ipsec_conf_set();
    /*ipsec pki shell script default generate*/
    //rc_ipsec_ca_default_gen();
    //rc_ipsec_pki_gen_exec();
    //rc_ipsec_ca_init();
    /*ca import*/
    //rc_ipsec_ca_import();
    return;
}
#if 0
static int cur_bitmap_en_scan()
{
    uint32_t cur_bitmap_en = 0, i = 0,prof_count = 0;
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
	    for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_UP == prof[prof_count][i].ipsec_conn_en){
	        	cur_bitmap_en |= (uint32_t)(1 << i);
	        }
	    }
	}
    return cur_bitmap_en;
}
#endif

void get_bitmap_scan(int *cur_bitmap_en)
{
	uint32_t i = 0,prof_count = 0;
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
	    for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_UP == prof[prof_count][i].ipsec_conn_en){
	        	cur_bitmap_en[prof_count] |= (uint32_t)(1 << i);
	        }
	    }
	}
	return;
}
void run_ipsec_firewall_scripts(void)
{
	char *argv[3];
	argv[0] = "/bin/sh";
	argv[1] = FILE_PATH_IPSEC_IPTABLES_RULE;
	argv[2] = NULL;

	chmod(FILE_PATH_IPSEC_IPTABLES_RULE, 0777);
	_eval(argv, NULL, 0, NULL);
	return;
}

#if defined(RTCONFIG_QUICKSEC)
static qs_virtual_ip_t 
get_virtual_ip_format(char *virtual_subnet)
{
	char *ip, *mask;
	int i=0;
	int ip_1, ip_2, ip_3, ip_4;
	int mask_1, mask_2, mask_3, mask_4, mask_tmp;
	int ip_start_1, ip_start_2, ip_start_3, ip_start_4;
	int ip_end_1, ip_end_2, ip_end_3, ip_end_4;
	
	qs_virtual_ip_t virtual_ip;
	memset(&virtual_ip, 0, sizeof(qs_virtual_ip_t));

	ip = strtok(virtual_subnet, "/");
	mask = strtok(NULL, "/");
	//DBG(("ip=%s,mask=%s\n", ip, mask));
	
	ip_1 = atoi(strtok(ip, "."));
	ip_2 = atoi(strtok(NULL, "."));
	ip_3 = atoi(strtok(NULL, "."));
	ip_4 = atoi(strtok(NULL, "."));
	
	mask_tmp = atoi(mask);
	int bitpatten = 0xff00;
	
	if( mask_tmp >= 8){
		mask_1 = 255;
		mask_tmp -= 8;
	}
	else{
		i = mask_1;
		while (i > 0){
			bitpatten = bitpatten >> 1;
			i--;
		}
		mask_1 = bitpatten & 0xff;	
	}
	
	if( mask_tmp >= 8){
		mask_2 = 255;
		mask_tmp -= 8;
	}
	else{
		i = mask_2;
		while (i > 0){
			bitpatten = bitpatten >> 1;
			i--;
		}
		mask_2 = bitpatten & 0xff;	
	}
	
	if( mask_tmp >= 8){
		mask_3 = 255;
		mask_tmp -= 8;
	}
	else{
		i = mask_3;
		int bitpatten = 0xff00; 
		while (i > 0){
			bitpatten = bitpatten >> 1;
			i--;
		}
		
		mask_3 = bitpatten & 0xff;	
	}
	//DBG(("mask_tmp=%d\n",mask_tmp));
	if( mask_tmp >= 8){
		mask_4 = 255;
	}
	else{
		i = mask_tmp;
		while (i > 0){
			bitpatten = bitpatten >> 1;
			i--;
		}
		mask_4 = bitpatten & 0xff;	
	}
	
	//DBG(("%d %d %d %d\n", mask_1, mask_2, mask_3, mask_4));
	//DBG(("%d %d %d %d\n", ip_1, ip_2, ip_3, ip_4));
	
	ip_start_1 = ip_1 & mask_1;
	ip_start_2 = ip_2 & mask_2;
	ip_start_3 = ip_3 & mask_3;
	ip_start_4 = (ip_4 & mask_4) + 1;
	
	ip_end_1 = ip_1 | (~ mask_1 & 0xff);
	ip_end_2 = ip_2 | (~ mask_2 & 0xff);
	ip_end_3 = ip_3 | (~ mask_3 & 0xff);
	ip_end_4 = (ip_4 | (~ mask_4 & 0xff)) - 1;
	
	sprintf(virtual_ip.ip_start, "%d.%d.%d.%d", ip_start_1, ip_start_2, ip_start_3, ip_start_4);
	sprintf(virtual_ip.ip_end, "%d.%d.%d.%d", ip_end_1, ip_end_2, ip_end_3, ip_end_4);
	sprintf(virtual_ip.subnet, "%d.%d.%d.%d", mask_1, mask_2, mask_3, mask_4);
	//DBG(("start_IP=%s, end_IP=%s, subnet=%s\n", ip_start, ip_end, subnet_mask));

	return virtual_ip;
}

void rc_ipsec_topology_set_XML()
{
	int i,prof_count = 0;
    FILE *fp = NULL;
    char *p_tmp = NULL, buf[SZ_MIN];
    p_tmp = &buf[0];
	char flag_buf[SZ_64BUF], alg_buf[SZ_64BUF];
    char *s_tmp = NULL, buf1[SZ_BUF];
	char alg_enc[SZ_MIN], alg_hash[SZ_MIN];
    s_tmp = &buf1[0];
	qs_virtual_ip_t virtual_ip;
	char lan_class[32];
	char *subnet, *subnet_total;

    memset(p_tmp, 0, sizeof(char) * SZ_MIN);
	memset(&virtual_ip, 0, sizeof(qs_virtual_ip_t));
	memset(alg_buf, 0, sizeof(char)* SZ_64BUF);
	   
	if((fp = fopen("/tmp/quicksecpm.xml", "w")) == NULL){
		DBG(("OPEN %s FAIL!!\n", "/tmp/quicksecpm.xml"));
		return;
	}
	fprintf(fp,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
	fprintf(fp,"<!DOCTYPE quicksec PUBLIC \"quicksec:dtd\" \"quicksec.dtd\">\n");
	fprintf(fp,"<quicksec>\n");
	fprintf(fp,"  <params>\n");
	
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
	    for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_DOWN != prof[prof_count][i].ipsec_conn_en){
				if(PROF_SVR == prof_count)
		        	sprintf(&buf[0], "ipsec_profile_%d", i+1);
				else if(PROF_CLI == prof_count)
					sprintf(&buf[0], "ipsec_profile_client_%d", i+1);
		        memset(s_tmp, '\0', SZ_BUF);
		        if(NULL != nvram_safe_get(&buf[0])){
		            strcpy(s_tmp, nvram_safe_get(&buf[0]));
		        }
		        if('\0' == *s_tmp){
		            memset((ipsec_prof_t *)&prof[prof_count][i], 0, sizeof(ipsec_prof_t));
		            prof[prof_count][i].ipsec_conn_en = IPSEC_CONN_EN_DEFAULT;
		            continue;
		        }
				
				if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type)
				{
					/* virtual server settings */
					fprintf(fp,"    <address-pool");
					if(0 != strlen(samba_prof.dns1) && 0 != strlen(samba_prof.dns2))
						fprintf(fp," dns=\"%s;%s\"", samba_prof.dns1, samba_prof.dns2);
					else if(0 != strlen(samba_prof.dns1))
						fprintf(fp," dns=\"%s\"", samba_prof.dns1);
					else if(0 != strlen(samba_prof.dns2))
						fprintf(fp," dns=\"%s\"", samba_prof.dns2);
						
					if(0 != strlen(samba_prof.nbios1) && 0 != strlen(samba_prof.nbios2))
						fprintf(fp," wins=\"%s;%s\"", samba_prof.nbios1, samba_prof.nbios2);
					else if(0 != strlen(samba_prof.nbios1))
						fprintf(fp," wins=\"%s\"", samba_prof.nbios1);
					else if(0 != strlen(samba_prof.nbios2))
						fprintf(fp," wins=\"%s\"", samba_prof.nbios2);
					fprintf(fp,">\n");
					ip2class(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), lan_class);
					if(0 != strcmp(prof[prof_count][i].local_subnet,lan_class))
						strncpy(prof[prof_count][i].local_subnet, lan_class, 32);
					fprintf(fp,"      <subnet>%s</subnet>\n", prof[prof_count][i].local_subnet);
					strcpy(buf, prof[prof_count][i].virtual_subnet);
					virtual_ip = get_virtual_ip_format(buf);
					//DBG(("virtual_subnet=%s\n",prof[prof_count][i].virtual_subnet));
					fprintf(fp,"      <address netmask=\"%s\">%s-%s</address>\n", virtual_ip.subnet, virtual_ip.ip_start,virtual_ip.ip_end);					
					fprintf(fp,"    </address-pool>\n");
				}			
			}
		}
	}

	fprintf(fp,"    <auth-domain>\n");
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
		for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_DOWN != prof[prof_count][i].ipsec_conn_en){
				if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type)
					fprintf(fp,"      <remote-secret id-type=\"email\" id=\"hostcli@ipsec.com\">%s</remote-secret>\n", prof[prof_count][i].auth_method_key);
				else
					fprintf(fp,"      <remote-secret id-type=\"email\" id=\"%s\">%s</remote-secret>\n", prof[prof_count][i].remote_id, prof[prof_count][i].auth_method_key);

				/* support xauth server */
				if(IPSEC_AUTH2_TYP_SVR == prof[prof_count][i].xauth){
					char ipsec_client_list_name[SZ_MIN], xauth_buf[SZ_MAX], xauth_tmp[SZ_MAX];
					char *p_str = NULL, *p_str1 = NULL;
					char *delim = "\n";
					char *pch;
					int idx = 0;
					memset(ipsec_client_list_name, 0, sizeof(char) * SZ_MIN);
					sprintf(ipsec_client_list_name, "ipsec_client_list_%d", i+1);

					/* xauth account/password */
					if(NULL != nvram_safe_get(ipsec_client_list_name)){
						p_str = &xauth_buf[0];
						p_str1 = &xauth_tmp[0];
						memset(xauth_buf, 0, sizeof(char) * SZ_MAX);
						memset(xauth_tmp, 0, sizeof(char) * SZ_MAX);
						strcpy(xauth_buf, nvram_safe_get(ipsec_client_list_name));
						while('\0' != *p_str){
							if('<' == *p_str){
								*p_str = '\n';
							}
							if('>' == *p_str){
								*p_str1 = *p_str++ = '\n';
								
								p_str1 +=  1;
							}
							*p_str1++ = *p_str++;
						}
						p_str1 = '\0';
						pch = strtok(xauth_tmp,delim);
						while (pch != NULL)
						{
							if(idx % 2 == 0)
								fprintf(fp,"        <password user-name=\"%s",pch);
							else
								fprintf(fp,"\" password=\"%s\"/>\n",pch);
							pch = strtok (NULL, delim);

							idx++;
						}  
					}
				}
			}
		}
	}
	fprintf(fp,"    </auth-domain>\n");	
	fprintf(fp,"  </params>\n");
	fprintf(fp,"  <policy>\n");
	
	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
		for(i = 0; i < MAX_PROF_NUM; i++){
			if(IPSEC_CONN_EN_DOWN != prof[prof_count][i].ipsec_conn_en){
				fprintf(fp,"  <!-- %s config -->\n", prof[prof_count][i].profilename);	
				fprintf(fp,"    <tunnel name=\"%s\"", prof[prof_count][i].profilename);

				flag_buf[0] = '\0';
				if(VPN_TYPE_NET_NET_SVR == prof[prof_count][i].vpn_type || VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type)
					strcat_r(flag_buf," dont-initiate",flag_buf);
				if(VPN_TYPE_NET_NET_PEER == prof[prof_count][i].vpn_type || VPN_TYPE_NET_NET_CLI == prof[prof_count][i].vpn_type)
					strcat_r(flag_buf," auto",flag_buf);	
				if(IPSEC_AUTH2_TYP_SVR == prof[prof_count][i].xauth)
					strcat_r(flag_buf," xauth-methods",flag_buf);
				if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type)
					strcat_r(flag_buf," require-cfgmode allow-cfgmode",flag_buf);
				if(IKE_AGGRESSIVE_MODE == prof[prof_count][i].exchange)
					strcat_r(flag_buf," aggressive-mode",flag_buf);
				if(0 == strcmp(prof[prof_count][i].tun_type,"transport"))
					strcat_r(flag_buf," transport",flag_buf);
				
				if(0 != strlen(flag_buf))
					fprintf(fp," flags=\"%s\"", flag_buf + 1);

				flag_buf[0] = '\0';
				DBG(("enc2=%d, hash2=%d\n",prof[prof_count][i].encryption_p2, prof[prof_count][i].hash_p2));

				if(ENCRYPTION_TYPE_AES128 == prof[prof_count][i].encryption_p2)
					strcat_r(flag_buf,"aes",flag_buf);
				else if(ENCRYPTION_TYPE_3DES == prof[prof_count][i].encryption_p2)
					strcat_r(flag_buf,"3des",flag_buf);	

				if(HASH_TYPE_SHA1 == prof[prof_count][i].hash_p2)
					strcat_r(flag_buf," sha1",flag_buf);
				else if(HASH_TYPE_SHA256 == prof[prof_count][i].hash_p2)
					strcat_r(flag_buf," sha2",flag_buf);
				DBG(("flag_buf=%s\n", flag_buf));
				if(0 != strlen(flag_buf))
					fprintf(fp," transform=\"esp %s\"", flag_buf);	
				if(0 != prof[prof_count][i].keylife_p1)
					fprintf(fp," ike-life=\"%d\"", prof[prof_count][i].keylife_p1);	
				fprintf(fp,">\n");	
				if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type)
					fprintf(fp,"      <identity id-type=\"email\" id=\"hostsrv@ipsec.com\">\n");
				else
					fprintf(fp,"      <identity id-type=\"email\" id=\"%s\">\n", prof[prof_count][i].local_id);
				fprintf(fp,"        <local-secret>%s</local-secret>\n", prof[prof_count][i].auth_method_key);
				fprintf(fp,"      </identity>\n");
				if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type)
					fprintf(fp,"      <identity type=\"remote\" id-type=\"email\" id=\"hostcli@ipsec.com\"/>\n");
				else
					fprintf(fp,"      <identity type=\"remote\" id-type=\"email\" id=\"%s\" flags=\"enforce-identity\"/>\n", prof[prof_count][i].remote_id);

				DBG(("enc=%d, hash=%d\n",prof[prof_count][i].encryption_p1, prof[prof_count][i].hash_p1));
				flag_buf[0] = '\0';
				
				if(ENCRYPTION_TYPE_AES128 == prof[prof_count][i].encryption_p1)
					strcat_r(flag_buf,"aes",flag_buf);
				else if(ENCRYPTION_TYPE_3DES == prof[prof_count][i].encryption_p1)
					strcat_r(flag_buf,"3des",flag_buf);					
				
				
				if(HASH_TYPE_SHA1 == prof[prof_count][i].hash_p1)
					strcat_r(flag_buf," sha1",flag_buf);
				else if(HASH_TYPE_SHA256 == prof[prof_count][i].hash_p1)
					strcat_r(flag_buf," sha2",flag_buf);
				if(0 != strlen(flag_buf))
					fprintf(fp,"      <ike-algorithms>%s</ike-algorithms>\n", flag_buf);
				
				if(ENCRYPTION_TYPE_AES128 == prof[prof_count][i].encryption_p1)
					fprintf(fp,"      <algorithm-properties algorithm=\"aes\" min-key-size=\"128\" max-key-size=\"128\" default-key-size=\"128\"/>\n");
				if(HASH_TYPE_SHA256 == prof[prof_count][i].hash_p1)
					fprintf(fp,"      <algorithm-properties algorithm=\"sha2\" min-key-size=\"256\" max-key-size=\"256\" default-key-size=\"256\"/>\n");
				
				if(0 != prof[prof_count][i].keylife_p2)
					fprintf(fp,"      <life type=\"seconds\">%d</life>\n", prof[prof_count][i].keylife_p2);	
				if(0 != strcmp(prof[prof_count][i].remote_gateway, "null"))
					fprintf(fp,"        <peer>%s</peer>\n", prof[prof_count][i].remote_gateway);
				fprintf(fp,"      <local-ip>%s</local-ip>\n", prof[prof_count][i].local_pub_ip);
				fprintf(fp,"      <ike-versions>%d</ike-versions>\n", prof[prof_count][i].ike);
				if(0 != prof[prof_count][i].dead_peer_detection)
					fprintf(fp,"      <dpd-timeout>%d</dpd-timeout>\n", prof[prof_count][i].ipsec_dpd);
				fprintf(fp,"    </tunnel>\n");
				if(VPN_TYPE_HOST_NET != prof[prof_count][i].vpn_type){
					fprintf(fp,"    <rule to-tunnel=\"%s\">\n", prof[prof_count][i].profilename);
					//fprintf(fp,"      <src>ipv4(%s)</src>\n",prof[prof_count][i].local_subnet);
					//fprintf(fp,"      <dst>ipv4(%s)</dst>\n",prof[prof_count][i].remote_subnet);
					fprintf(fp,"      <src>");
					subnet_total = strdup(prof[prof_count][i].local_subnet);
					while((subnet = strsep(&subnet_total, ",")) != NULL){
						if(NULL != strstr(subnet, ":"))
							fprintf(fp,"ipv6(%s),", subnet);
						else
							fprintf(fp,"ipv4(%s),", subnet);
				    }
					fprintf(fp,"</src>\n");
					fprintf(fp,"      <dst>");
					subnet_total = strdup(prof[prof_count][i].remote_subnet);
					while((subnet = strsep(&subnet_total, ",")) != NULL){
						if(NULL != strstr(subnet, ":"))
							fprintf(fp,"ipv6(%s),", subnet);
						else
							fprintf(fp,"ipv4(%s),", subnet);
				    }
					fprintf(fp,"</dst>\n");
					fprintf(fp,"    </rule>\n");
				}

				if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type){
					fprintf(fp,"    <rule from-tunnel=\"%s\"/>\n", prof[prof_count][i].profilename);
				}
				else{
					fprintf(fp,"    <rule from-tunnel=\"%s\">\n", prof[prof_count][i].profilename);
					//fprintf(fp,"      <src>ipv4(%s)</src>\n", prof[prof_count][i].remote_subnet);
					//fprintf(fp,"      <dst>ipv4(%s)</dst>\n", prof[prof_count][i].local_subnet);
					fprintf(fp,"      <src>");
					subnet_total = strdup(prof[prof_count][i].remote_subnet);
					while((subnet = strsep(&subnet_total, ",")) != NULL){
						if(NULL != strstr(subnet, ":"))
							fprintf(fp,"ipv6(%s),", subnet);
						else
							fprintf(fp,"ipv4(%s),", subnet);
				    }
					fprintf(fp,"</src>\n");
					fprintf(fp,"      <dst>");
					subnet_total = strdup(prof[prof_count][i].local_subnet);
					while((subnet = strsep(&subnet_total, ",")) != NULL){
						if(NULL != strstr(subnet, ":"))
							fprintf(fp,"ipv6(%s),", subnet);
						else
							fprintf(fp,"ipv4(%s),", subnet);
				    }
					fprintf(fp,"</dst>\n");
					fprintf(fp,"    </rule>\n");
				}
			}
		}
	}
		
	fprintf(fp,"    <rule>\n");
	fprintf(fp,"      <src>ipv4(0.0.0.0/0)</src>\n");
	fprintf(fp,"      <dst>ipv4(0.0.0.0/0)</dst>\n");	
	fprintf(fp,"    </rule>\n");
	fprintf(fp,"  </policy>\n");
	fprintf(fp,"</quicksec>\n");
	fclose(fp);
    return;
}

#endif

void rc_ipsec_set(ipsec_conn_status_t conn_status, ipsec_prof_type_t prof_type)
{
    static bool ipsec_start_en = FALSE;
    static int32_t pre_bitmap_en[2] = {0};
    uint32_t i = 0; //, cur_bitmap_en= 0;
	int prof_count = 0;
    FILE *fp = NULL, *fp1 = NULL;
    char interface[4];
	char *argv[3];
	int unit = 0;
	char word[80], *next;
	uint32_t cur_bitmap_en_p[2]={0};
	char *local_subnet, *local_subnet_total;
	int prof_i = 0, is_duplicate = 0;
	char tmpStr[20];
/* moved to firewall
	int isakmp_port = nvram_get_int("ipsec_isakmp_port");
	int nat_t_port = nvram_get_int("ipsec_nat_t_port");
*/

	argv[0] = "/bin/sh";
	argv[1] = FILE_PATH_IPSEC_SH;
	argv[2] = NULL;

    rc_ipsec_nvram_convert_check();
    rc_ipsec_conf_set();
    rc_ipsec_secrets_set();
    rc_ipsec_gen_cert(0);
    rc_ipsec_ca_init();
    rc_strongswan_conf_set();
#if defined(RTCONFIG_QUICKSEC)
	rc_ipsec_topology_set_XML();
#endif
	if ((fp = fopen(FILE_PATH_IPSEC_SH, "w")) == NULL){
		DBG(("OPEN %s FAIL!!\n", FILE_PATH_IPSEC_SH));
		return;
	}

	if ((fp1 = fopen(FILE_PATH_IPSEC_IPTABLES_RULE, "w")) == NULL){
		DBG(("OPEN %s FAIL!!\n", FILE_PATH_IPSEC_IPTABLES_RULE));
		return;
	}
		
//#if 0
    if(FALSE == ipsec_start_en){
        rc_ipsec_start(fp);
        ipsec_start_en = TRUE;
        if(IPSEC_INIT == conn_status){
            fprintf(fp, "\nsleep 7 > /dev/null 2>&1 \n");
        }
    }
//#endif
    //cur_bitmap_en = cur_bitmap_en_scan();
	get_bitmap_scan((int *) cur_bitmap_en_p);
	//DBG(("rc_ipsec_down_stat>>>> 0x%x, 0x%x\n", cur_bitmap_en_p[0], cur_bitmap_en_p[1]));

	if(cur_bitmap_en_p[0] != 0)
		nvram_set_int("ipsec_client_enable",1);
	else
		nvram_set_int("ipsec_client_enable",0);
	
	if((nvram_get_int("ipsec_server_enable") == 1 || nvram_get_int("ipsec_client_enable") == 1 )
#ifdef RTCONFIG_INSTANT_GUARD
         || nvram_get_int("ipsec_ig_enable")
#endif
        ){
		/*if(IPSEC_INIT == conn_status){
			if (!pids("starter") && !pids("charon"))
				rc_ipsec_start(fp);
			else	{
				rc_ipsec_stop(fp);
				rc_ipsec_start(fp);
			}
		}
		ipsec_start_en = TRUE;*/
#if defined(RTCONFIG_QUICKSEC)		
		modprobe("ah4");
		modprobe("esp4");
		modprobe("ipcomp");
		modprobe("xfrm4_tunnel");
		modprobe("xfrm_user");
#endif
		/* ipsec must be restart if strongswan.conf changed, or it will not apply the new settings. */
		if((TRUE == ipsec_start_en) && (IPSEC_INIT != conn_status)&& (0 != strcmp(pre_samba_prof.dns1, samba_prof.dns1) || 0 != strcmp(pre_samba_prof.dns2, samba_prof.dns2) || 
				0 != strcmp(pre_samba_prof.nbios1, samba_prof.nbios1) || 0 != strcmp(pre_samba_prof.nbios2, samba_prof.nbios2))){
			pre_samba_prof = samba_prof;
			fprintf(fp, "\nsleep 2 > /dev/null 2>&1 \n");
			rc_ipsec_restart(fp);
		}
		else{
			rc_ipsec_rereadall(fp);
			rc_ipsec_reload(fp);
		}
	}
	
	//rc_ipsec_rereadall(fp);
	//rc_ipsec_reload(fp);

	for(prof_count = PROF_CLI; prof_count < PROF_ALL; prof_count++){
		DBG(("rc_ipsec_down_stat>>>> 0x%x,prof_count=%d\n", pre_bitmap_en[prof_count],prof_count));
    	for(i = 0; i < MAX_PROF_NUM; i++){
			if(0 != strlen(prof[prof_count][i].profilename)) {
				if(strcmp(prof[prof_count][i].local_public_interface,"wan") == 0){
					strcpy(interface,nvram_safe_get("wan0_gw_ifname"));
				}
				else if(strcmp(prof[prof_count][i].local_public_interface,"wan2") == 0){
					strcpy(interface,nvram_safe_get("wan1_gw_ifname"));
				}
				else if(strcmp(prof[prof_count][i].local_public_interface,"lan") == 0) 
				strcpy(interface,"br0");
				else if(strcmp(prof[prof_count][i].local_public_interface,"usb") == 0){ 
					foreach(word, nvram_safe_get("wan_ifnames"), next) {
						if (0 == strcmp(word,"usb")){
							break;
						}
						unit ++;
					}
					strcpy(interface,get_wan_ifname(unit));
				}
				if(IPSEC_CONN_EN_UP == prof[prof_count][i].ipsec_conn_en){
					if(0 != strcmp(interface,"")){
						if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type){
							/* Host to Net */
							for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
								sprintf(tmpStr,"wan%d_gw_ifname",unit);
								if(0 != strlen(nvram_safe_get(tmpStr)))
									strcpy(interface,nvram_safe_get(tmpStr));
								/* moved to firewall
								fprintf(fp1, "iptables -D INPUT -i %s --protocol esp -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -D INPUT -i %s --protocol ah -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -D INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, isakmp_port);
								fprintf(fp1, "iptables -D INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, nat_t_port);
								fprintf(fp1, "iptables -I INPUT -i %s --protocol esp -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -I INPUT -i %s --protocol ah -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -I INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, isakmp_port);
								fprintf(fp1, "iptables -I INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, nat_t_port);
								fprintf(fp1, "iptables -D OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -D OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -D OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, isakmp_port);
								fprintf(fp1, "iptables -D OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, nat_t_port);
								fprintf(fp1, "iptables -I OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -I OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
								fprintf(fp1, "iptables -I OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, isakmp_port);
								fprintf(fp1, "iptables -I OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, nat_t_port);
								fprintf(fp1, "iptables -D INPUT -i %s -s %s -j ACCEPT\n",interface, prof[prof_count][i].virtual_subnet);
								fprintf(fp1, "iptables -I INPUT -i %s -s %s -j ACCEPT\n",interface, prof[prof_count][i].virtual_subnet);
								*/
#ifdef RTCONFIG_BCMARM
								/* mark connect to bypass CTF */
								if (nvram_match("ctf_disable", "0")){
									fprintf(fp1, "iptables -t mangle -D FORWARD -i %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
									fprintf(fp1, "iptables -t mangle -A FORWARD -i %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
									fprintf(fp1, "iptables -t mangle -D FORWARD -o %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
									fprintf(fp1, "iptables -t mangle -A FORWARD -o %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
								}
#endif
							}
							/* to fix host-to-net some android device can't access some website. ex. www.sogi.com.tw */
							fprintf(fp1, "iptables -t mangle -D FORWARD -m policy --pol ipsec --dir in -p tcp -m tcp --tcp-flags SYN,RST SYN -m tcpmss --mss 1361:1536 -j TCPMSS --set-mss 1360\n");
							fprintf(fp1, "iptables -t mangle -D FORWARD -m policy --pol ipsec --dir out -p tcp -m tcp --tcp-flags SYN,RST SYN -m tcpmss --mss 1361:1536 -j TCPMSS --set-mss 1360\n");
							fprintf(fp1, "iptables -t mangle -A FORWARD -m policy --pol ipsec --dir in -p tcp -m tcp --tcp-flags SYN,RST SYN -m tcpmss --mss 1361:1536 -j TCPMSS --set-mss 1360\n");
							fprintf(fp1, "iptables -t mangle -A FORWARD -m policy --pol ipsec --dir out -p tcp -m tcp --tcp-flags SYN,RST SYN -m tcpmss --mss 1361:1536 -j TCPMSS --set-mss 1360\n");
						}
						else{
							/* Net to Net */
							/* moved to firewall
							fprintf(fp1, "iptables -D INPUT -i %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -D INPUT -i %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -D INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, isakmp_port);
							fprintf(fp1, "iptables -D INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, nat_t_port);
							fprintf(fp1, "iptables -I INPUT -i %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -I INPUT -i %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -I INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, isakmp_port);
							fprintf(fp1, "iptables -I INPUT -i %s -p udp --dport %d -j ACCEPT\n", interface, nat_t_port);
							fprintf(fp1, "iptables -D OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -D OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -D OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, isakmp_port);
							fprintf(fp1, "iptables -D OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, nat_t_port);
							fprintf(fp1, "iptables -I OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -I OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp1, "iptables -I OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, isakmp_port);
							fprintf(fp1, "iptables -I OUTPUT -o %s -p udp --sport %d -j ACCEPT\n", interface, nat_t_port);
							*/
							local_subnet_total = strdup(prof[prof_count][i].local_subnet);
							while((local_subnet = strsep(&local_subnet_total, ",")) != NULL){
								/* moved to firewall
								fprintf(fp1, "iptables -t nat -D POSTROUTING -s %s -m policy --dir out --pol ipsec -j ACCEPT\n", local_subnet);
								fprintf(fp1, "iptables -t nat -I POSTROUTING -s %s -m policy --dir out --pol ipsec -j ACCEPT\n", local_subnet);
								*/
#ifdef RTCONFIG_BCMARM
								/* mark connect to bypass CTF */
								if (nvram_match("ctf_disable", "0")){
									fprintf(fp1, "iptables -t mangle -D FORWARD -i %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
									fprintf(fp1, "iptables -t mangle -A FORWARD -i %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
									fprintf(fp1, "iptables -t mangle -D FORWARD -o %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
									fprintf(fp1, "iptables -t mangle -A FORWARD -o %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
								}
#endif
						    }
						}
						
					}
				}
		if(((IPSEC_START == conn_status) || (IPSEC_INIT == conn_status)) &&  
	           (IPSEC_CONN_EN_UP == prof[prof_count][i].ipsec_conn_en)){

	            if(((uint32_t)(1 << i)) != (pre_bitmap_en[prof_count] & ((uint32_t)(1 << i)))){
						//cur_bitmap_en = cur_bitmap_en_scan();
						get_bitmap_scan((int *) cur_bitmap_en_p);
						if(0 != strcmp(interface,"")){
							/* moved to firewall
							fprintf(fp, "iptables -D INPUT -i %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D INPUT -i %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D INPUT -i %s -p udp --dport 500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D INPUT -i %s -p udp --dport 4500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I INPUT -i %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I INPUT -i %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I INPUT -i %s -p udp --dport 500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I INPUT -i %s -p udp --dport 4500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D OUTPUT -o %s -p udp --sport 500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -D OUTPUT -o %s -p udp --sport 4500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I OUTPUT -o %s -p udp --sport 500 -j ACCEPT\n", interface);
							fprintf(fp, "iptables -I OUTPUT -o %s -p udp --sport 4500 -j ACCEPT\n", interface);
							*/

						if(VPN_TYPE_HOST_NET == prof[prof_count][i].vpn_type){
							/* moved to firewall
							fprintf(fp, "iptables -D INPUT -i %s -s %s -j ACCEPT\n",interface,prof[prof_count][i].virtual_subnet);
							fprintf(fp, "iptables -I INPUT -i %s -s %s -j ACCEPT\n",interface,prof[prof_count][i].virtual_subnet);
							*/
#ifdef RTCONFIG_BCMARM
							/* mark connect to bypass CTF */
							/*if (nvram_match("ctf_disable", "0")){
								fprintf(fp, "iptables -t mangle -D FORWARD -i %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
								fprintf(fp, "iptables -t mangle -A FORWARD -i %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
								fprintf(fp, "iptables -t mangle -D FORWARD -o %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
								fprintf(fp, "iptables -t mangle -A FORWARD -o %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].virtual_subnet);
							}*/
#endif							
						}
						else{
							local_subnet_total = strdup(prof[prof_count][i].local_subnet);
							while((local_subnet = strsep(&local_subnet_total, ",")) != NULL){
								/* moved to firewall
								fprintf(fp, "iptables -t nat -D POSTROUTING -s %s -m policy --dir out --pol ipsec -j ACCEPT\n", local_subnet);
								fprintf(fp, "iptables -t nat -I POSTROUTING -s %s -m policy --dir out --pol ipsec -j ACCEPT\n", local_subnet);
								*/
#ifdef RTCONFIG_BCMARM
								/* mark connect to bypass CTF */
								/*if (nvram_match("ctf_disable", "0")){
									fprintf(fp, "iptables -t mangle -D FORWARD -i %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
									fprintf(fp, "iptables -t mangle -A FORWARD -i %s -d %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
									fprintf(fp, "iptables -t mangle -D FORWARD -o %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
									fprintf(fp, "iptables -t mangle -A FORWARD -o %s -s %s -j MARK --set-mark 0x01/0x7\n", interface, prof[prof_count][i].local_subnet);
								}*/
#endif									
						    }
							
						}
						}
						if(VPN_TYPE_NET_NET_PEER == prof[prof_count][i].vpn_type || VPN_TYPE_NET_NET_CLI == prof[prof_count][i].vpn_type)
                rc_ipsec_up(fp, i, prof_count);
            }
				}
				else if((IPSEC_STOP == conn_status) &&
		         (IPSEC_CONN_EN_DOWN == prof[prof_count][i].ipsec_conn_en)){
    				for(prof_i = 0; prof_i < MAX_PROF_NUM; prof_i++){
						while((0 == strcmp(prof[prof_count][i].local_public_interface, prof[prof_count][prof_i].local_public_interface)) && 
							(i != prof_i) && (1 == prof[prof_count][prof_i].ipsec_conn_en)){
							is_duplicate = 1;
							break;
						}
    				}
					if(0 == is_duplicate){
						/* moved to firewall
						fprintf(fp, "iptables -D INPUT -i %s --protocol esp -j ACCEPT\n", interface);
						fprintf(fp, "iptables -D INPUT -i %s --protocol ah -j ACCEPT\n", interface);
						fprintf(fp, "iptables -D INPUT -i %s -p udp --dport 500 -j ACCEPT\n", interface);
						fprintf(fp, "iptables -D INPUT -i %s -p udp --dport 4500 -j ACCEPT\n", interface); 
						fprintf(fp, "iptables -D OUTPUT -o %s --protocol esp -j ACCEPT\n", interface);
						fprintf(fp, "iptables -D OUTPUT -o %s --protocol ah -j ACCEPT\n", interface);
						fprintf(fp, "iptables -D OUTPUT -o %s -p udp --sport 500 -j ACCEPT\n", interface);
						fprintf(fp, "iptables -D OUTPUT -o %s -p udp --sport 4500 -j ACCEPT\n", interface);
						*/
					}
					rc_ipsec_down(fp, i, prof_count);
				}

				if((0 == nvram_get_int("ipsec_server_enable")) && (PROF_SVR == prof_count)
#ifdef RTCONFIG_INSTANT_GUARD
                 && (0 == nvram_get_int("ipsec_ig_enable"))
#endif
                 )
				rc_ipsec_down(fp, i, prof_count);
			
			}
		}
		pre_bitmap_en[prof_count] = cur_bitmap_en_p[prof_count];
	}

	if((IPSEC_STOP == conn_status) && (TRUE == ipsec_start_en) && 
		(0 == cur_bitmap_en_p[PROF_CLI] && 0 == cur_bitmap_en_p[PROF_SVR])){
        rc_ipsec_stop(fp);
        ipsec_start_en = FALSE;
	}else if((0 == nvram_get_int("ipsec_server_enable")) && (0 == nvram_get_int("ipsec_client_enable"))
#ifdef RTCONFIG_INSTANT_GUARD
			&& (0 == nvram_get_int("ipsec_ig_enable"))
#endif
	)
    {
        rc_ipsec_stop(fp);
		ipsec_start_en = FALSE;
	}
	
	if(NULL != fp1){
        fclose(fp1);
    }
	
    if(NULL != fp){
        fclose(fp);
    }
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ60XX)
	reinit_ecm(-1);
#endif
	DBG(("rc_ipsec_down_stat<<<< CLI: 0x%x, SVR: 0x%x\n", cur_bitmap_en_p[PROF_CLI],cur_bitmap_en_p[PROF_SVR]));
	chmod(FILE_PATH_IPSEC_SH, 0777);
	_eval(argv, NULL, 0, NULL);
	DBG(("rc_ipsec_down_stat<<<< CLI: 0x%x, SVR: 0x%x\n", cur_bitmap_en_p[PROF_CLI],cur_bitmap_en_p[PROF_SVR]));
	run_ipsec_firewall_scripts();
    return;
}

