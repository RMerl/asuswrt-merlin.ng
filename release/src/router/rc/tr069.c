/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include "rc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> //for inet_ntop()
#include <netdb.h>	// for struct addrinfo

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2)
#define JFFS_TR_FOLDER		"/jffs/tr"
#endif
#define TR_FOLDER			"/tmp/tr"
#define DEFAULT_TR_XML		"/usr/tr/tr.xml"
#define TR_XML_FILE			"tr.xml"
#define TR_XML_BAK_FILE		"tr.xml.bak"
#define TR_XML_TAR_FILE			"tr.xml.tz"
#define TR_XML_BAK_TAR_FILE		"tr.xml.bak.tz"
#define DHCPC_LEASE_LIST	"/tmp/dhcpc_lease_list"
#define TR_CONFIG_FILE		TR_FOLDER"/tr.conf"
#define TR_LOG_FILE			TR_FOLDER"/tr.log"
#define CA_CERT_FILE		TR_FOLDER"/ca.pem"
#define CLIENT_CERT_FILE	TR_FOLDER"/client.crt"
#define CLIENT_KEY_FILE		TR_FOLDER"/client.key"
//#define IPDRDOC_FOLDER		TR_FOLDER"/IPDRDoc"

int generate_tr_config()
{
	FILE *fp;
	
	/* Generate tr configuration file */
	if (!(fp = fopen(TR_CONFIG_FILE, "w"))) {
		perror(TR_CONFIG_FILE);
		return -1;
	}

	fprintf(fp, "#TCPAddress = 0.0.0.0\n");
	fprintf(fp, "TCPAddress = :: #or IPv6_Addr\n");
	fprintf(fp, "TCPChallenge = Basic\n");
	fprintf(fp, "TCPNotifyInterval = 0\n");
	fprintf(fp, "UDPAddress = 0.0.0.0\n");
	fprintf(fp, "UDPPort = 7547\n");
	fprintf(fp, "UDPNotifyInterval = 20\n");
	fprintf(fp, "#CLIAddress = 0.0.0.0\n");
	fprintf(fp, "CLIAddress = ::\n");
	fprintf(fp, "CLIPort = 1234\n");
	fprintf(fp, "CLITimeout = 10\n");
	fprintf(fp, "CLIDocRoot = /usr/tr/doc/html\n");
	fprintf(fp, "CLIIndex = index.html\n");
	fprintf(fp, "MaxListener = 3\n");
	fprintf(fp, "CACert = %s\n", CA_CERT_FILE);
	fprintf(fp, "ClientCert = %s\n", CLIENT_CERT_FILE);
	fprintf(fp, "ClientKey = %s\n", CLIENT_KEY_FILE);
	fprintf(fp, "SSLPassword= password\n");
	fprintf(fp, "Init = tr.xml\n");
	//fprintf(fp, "#At the moment, all statis inform parameter MUST be constant value parameter\n");
#ifdef RTCONFIG_TR181
	fprintf(fp, "InformParameter = Device.DeviceInfo.SpecVersion\n");
	fprintf(fp, "InformParameter = Device.DeviceInfo.HardwareVersion\n");
	fprintf(fp, "InformParameter = Device.DeviceInfo.SoftwareVersion\n");
	fprintf(fp, "InformParameter = Device.DeviceInfo.ProvisioningCode\n");
	fprintf(fp, "InformParameter = Device.ManagementServer.ConnectionRequestURL\n");
	fprintf(fp, "InformParameter = Device.ManagementServer.ParameterKey\n");
	//fprintf(fp, "#InformParameter = Device.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.1.ExternalIPAddress\n");
#else	/* TR098 */
	fprintf(fp, "InformParameter = InternetGatewayDevice.DeviceSummary\n");
	fprintf(fp, "InformParameter = InternetGatewayDevice.DeviceInfo.SpecVersion\n");
	fprintf(fp, "InformParameter = InternetGatewayDevice.DeviceInfo.HardwareVersion\n");
	fprintf(fp, "InformParameter = InternetGatewayDevice.DeviceInfo.SoftwareVersion\n");
	fprintf(fp, "InformParameter = InternetGatewayDevice.DeviceInfo.ProvisioningCode\n");
	fprintf(fp, "InformParameter = InternetGatewayDevice.ManagementServer.ConnectionRequestURL\n");
	fprintf(fp, "InformParameter = InternetGatewayDevice.ManagementServer.ParameterKey\n");
	//fprintf(fp, "#InformParameter = InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.1.ExternalIPAddress\n");
#endif
	fprintf(fp, "LogFileName = %s\n", TR_LOG_FILE);
	fprintf(fp, "LogAutoRotate = true\n");
	fprintf(fp, "LogBackup = 5\n");
	fprintf(fp, "LogLevel = DEBUG\n");
	if(nvram_get_int("TR_debug") == 1)
		fprintf(fp, "LogMode = BOTH\n");
	else
		fprintf(fp, "LogMode = NONE\n");
	fprintf(fp, "LogLimit = 1MB\n");
	fprintf(fp, "TaskQueueLenLimit = 64\n");
	fprintf(fp, "Upload = 1:1 Vendor Configuration File:/tmp/Settings_%s.CFG\n", nvram_safe_get("productid"));
	fprintf(fp, "Upload = 0:2 Vendor Log File:/tmp/syslog.log\n");
	//fprintf(fp, "Upload = 0:X 00256D 3GPP Performance File:pm.xml\n");
	fprintf(fp, "Download = 1 Firmware Upgrade Image:/tmp/firmware.trx\n");
	fprintf(fp, "Download = 2 Web Content:web.html\n");
	fprintf(fp, "Download = 3 Vendor Configuration File:/tmp/settings_u.prf\n");
#ifdef RTCONFIG_SFEXPRESS
	fprintf(fp, "Download = 4 Openvpn Client1 File:/tmp/openvpn_file\n");
	fprintf(fp, "Download = 5 Openvpn Client2 File:/tmp/openvpn_file\n");
	fprintf(fp, "Download = 6 Openvpn Client3 File:/tmp/openvpn_file\n");
#endif
	//fprintf(fp, "Download = 4 Tone File:tone.file\n");
	//fprintf(fp, "Download = 5 Ringer File:ringer.file\n");
	//fprintf(fp, "Download = X Worksys DUInstallDownload: duin.pkg\n");
	//fprintf(fp, "Download = X Worksys DUUpdateDownload: duup.pkg\n");
	//fprintf(fp, "Download = X 00256D 3GPP Configuration File:cm.xml\n");
	fprintf(fp, "TrustTargetFileName = false\n");
	fprintf(fp, "CheckIdleInterval = 2\n");
	fprintf(fp, "DownloadRetryInterval = 5\n");
	fprintf(fp, "DownloadMaxRetries = 3\n");
#if 0
	fprintf(fp, "#CustomedEvent = 1:X 123456 Event1\n");
	fprintf(fp, "CustomedEvent = 1: X 00256D 3GPP Reprovision Required: SecGW\n");
	fprintf(fp, "CustomedEvent = 2: X 00256D 3GPP Reprovision Required: FAPGW\n");
#endif
	fprintf(fp, "SessionTimeout = 10\n");
	fprintf(fp, "Expect100Continue = false\n");
	fprintf(fp, "HTTPChunk = true\n");
	fprintf(fp, "KAISpan=5\n");
	fprintf(fp, "StunRepeat = 2\n");
#if 0
	fprintf(fp, "WIBRepeat = 5\n");
	fprintf(fp, "WIBSpan = 5\n");
	fprintf(fp, "WIBReInterval = 10\n");
	fprintf(fp, "ResponseStatus1 = Device.DeviceInfo.SoftwareVersion\n");
	fprintf(fp, "ResponseStatus1 = Device.MultiInstanceSample\n");
#endif
	//fprintf(fp, "IPDRDoc = %s\n", IPDRDOC_FOLDER);

	fclose(fp);

	return 0;
}

int check_xml_file_version(char *def_file, char *run_file)
{
	FILE *fp_def, *fp_run;
	int res = 0;
	char buf[128];
	char def_ver_info[16] = "", run_ver_info[16] = "";
	char *p;

	/* get version info from the xml file of default */
	if ((fp_def = fopen(def_file, "r")) == NULL) {
		_dprintf("*** can't open file(%s)\n", def_file);
		return res;
	}
	
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf), fp_def) != NULL) {
#ifdef RTCONFIG_TR181
		if(strstr(buf, "'Device'")) {
#else	/* TR098 */
		if(strstr(buf, "'InternetGatewayDevice'")) {
#endif
			if((p =strstr(buf, "arg="))) {
				sscanf(p, "arg='%[^']", def_ver_info);
				//logmessage("TR-069 agent", "def_ver_info: %s, len: %d", def_ver_info, strlen(def_ver_info));
				break;
			}
		}
	}
	fclose(fp_def);

	/* get version info from the xml file of runtime */
	if ((fp_run = fopen(run_file, "r")) == NULL) {
		_dprintf("*** can't open file(%s)\n", run_file);
		return res;
	}
	
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf), fp_run) != NULL) {
#ifdef RTCONFIG_TR181
		if(strstr(buf, "'Device'")) {
#else	/* TR098 */
		if(strstr(buf, "'InternetGatewayDevice'")) {
#endif
			if((p =strstr(buf, "arg="))) {
				sscanf(p, "arg='%[^']", run_ver_info);
				//logmessage("TR-069 agent", "def_ver_info: %s, len: %d", run_ver_info, strlen(run_ver_info));
				break;
			}
		}
	}
	fclose(fp_run);

	//logmessage("TR-069 agent", "def_ver_info: %s, run_ver_info: %s", def_ver_info, run_ver_info);

	if (!strcmp(def_ver_info, run_ver_info)) {
		res = 1;
		logmessage("TR-069 agent", "version match");
	}
	else
		logmessage("TR-069 agent", "version dismatch");

	return res;
}

char *convert_cert(const char *name, char *buf)
{
	char *value;
	int len, i;

	value = nvram_safe_get(name);

	len = strlen(value);

	for (i=0; (i < len); i++) {
		if (value[i] == '>')
			buf[i] = '\n';
		else
			buf[i] = value[i];
	}

	buf[i] = '\0';

	return buf;
}

void save_cert_from_nvram()
{
	char buf[3000];
	FILE *fp = NULL;

	/* write ca cert from nvram */
	if (check_if_file_exist(CA_CERT_FILE))
		unlink(CA_CERT_FILE);

	if(!nvram_is_empty("tr_ca_cert")) {
		fp = fopen(CA_CERT_FILE, "w");
		chmod(CA_CERT_FILE, S_IRUSR|S_IWUSR);
		fprintf(fp, "%s", convert_cert("tr_ca_cert", buf));
		fclose(fp);
	}

	/* write client cert from nvram */
	if (check_if_file_exist(CLIENT_CERT_FILE))
		unlink(CLIENT_CERT_FILE);

	if(!nvram_is_empty("tr_client_cert")) {
		fp = fopen(CLIENT_CERT_FILE, "w");
		chmod(CLIENT_CERT_FILE, S_IRUSR|S_IWUSR);
		fprintf(fp, "%s", convert_cert("tr_client_cert", buf));
		fclose(fp);
	}

	/* write client key from nvram */
	if (check_if_file_exist(CLIENT_KEY_FILE))
		unlink(CLIENT_KEY_FILE);
	
	if(!nvram_is_empty("tr_client_key")) {
		fp = fopen(CLIENT_KEY_FILE, "w");
		chmod(CLIENT_KEY_FILE, S_IRUSR|S_IWUSR);
		fprintf(fp, "%s", convert_cert("tr_client_key", buf));
		fclose(fp);
	}
}

char *skip_blanks( const char *str )
{
    while( *str && ( ( unsigned char ) *str ) < 33 ) {
        str++;
    }

    return ( char * )str;
}
/*   
******************************
To convert url into ip  
******************************
URL type:
     * Domain name:port/
     * Domain name/
     * [ipv6_addr(:)]:port/
     * [ipv6_addr(:)]/
     * ipv4_addr:port/
     * ipv4_addr/
*/
extern int url2ip(const char *u , struct addrinfo **res);
int url2ip(const char *u , struct addrinfo **res)
{
    char url[1025];
    char *proto = "http";
    char *host = NULL;
    char hostname[257]; /*The HTTP server host address*/
    char port[10];  /*The HTTP server port*/
    char *c;

    u = skip_blanks( u );
    snprintf( url, sizeof( url ), "%s", u );
    c = strstr( url, "://" );

    if( c ) {
        *c = '\0';
        host = c + 3;

        if( strcasecmp( url, "https" ) == 0 ) {
            proto = "https";
        } else if( strcasecmp( url, "http" ) ) {
                _dprintf("Unsupported protocol: %s", url );
                return -1;
            }
    } else {
        host = url;
    }

    c = strchr( host, '/' );

    if( c ) {
        *c = '\0';  
    } 

    if( *host == '[' ) { /* [ipv6_addr] */
        char *c1 = NULL;
        host++;
        c = strchr( host, ']' );

        if( c == NULL ) {
            _dprintf("[] not match\n");
            return -1;
        }

        *c = '\0';
        snprintf( hostname, sizeof(hostname), "%s", host );
        c++;

        if( ( c1 = strchr( c, ':' ) ) != NULL ) {
           snprintf( port, sizeof( port ), "%s", c1 + 1 );

        } else if( strcasecmp( proto, "https" ) == 0 ) {
            snprintf( port, sizeof( port ), "443" );   /* The default port for HTTPS */
        } else {
            snprintf(port, sizeof( port ), "80" );
        }
    } else {
        /* Domain name and ipv4_addr */
        c = strchr( host, ':' );

        if( c ) {
            snprintf( port, sizeof( port ), "%s", c + 1 );
            *c = '\0';

        } else if( strcasecmp( proto, "https" ) == 0 ) {
            snprintf( port, sizeof( port ), "443" );   /* The default port for HTTPS */
        } else {
            snprintf( port, sizeof(port ), "80" );
        }
         snprintf( hostname, sizeof(hostname), "%s", host );
    }

    _dprintf("sherry_debug1 hostname=%s\n,port=%s\n", hostname, port );

   /* A host name or address be converted to ip */
    int ret;
    struct addrinfo hints;
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( ( ret = getaddrinfo( hostname, port, &hints, res ) ) != 0 ) {
    	_dprintf("Get server(%s) address information failed: %s!", host, gai_strerror( ret ) );
    	return -1;
    }

    return 0;
}

void
stop_tr(void)
{
	killall_tk("tr069");
	char logaccept[32];
	if(nvram_get_int("tr_stop") != 1){
		if (nvram_get_int("tr_conn_port")) {
			_dprintf("sherry_debug3 tr_conn_port !=NULL\n");
			if(strlen(nvram_safe_get("tr_acs_url_tmp"))){
				_dprintf("sherry_debug4 tr_acs_url_tmp !=NULL\n");
				struct addrinfo *res, *ressave;
	        	int ret;
				ret=url2ip(nvram_safe_get("tr_acs_url_tmp"), &res);
				ressave = res;
				if( (ret != -1) && (res!=NULL) && (res->ai_addr !=NULL) ){
	        		do {
	        			if(res->ai_family == AF_INET){//ipv4
	                        char ipv4str[INET_ADDRSTRLEN];
	                        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)(res->ai_addr))->sin_addr),ipv4str, INET_ADDRSTRLEN) == NULL)
	                           	perror("inet_ntop"); 
	                        _dprintf("sherry_debug2 ipstr =%s\n", ipv4str);
	                        /* Determine the log type */
						if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
							strcpy(logaccept, "logaccept");
						else strcpy(logaccept, "ACCEPT");
	                        eval("iptables", "-t", "filter", "-D", "INPUT", "-s", ipv4str, "-p", "tcp", "-m", "tcp", "--dport", nvram_safe_get("tr_conn_port") ,"-j", logaccept);

	                    }else if(res->ai_family == AF_INET6){
	                        char ipv6str[INET6_ADDRSTRLEN];
	                        if(inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr),ipv6str, INET6_ADDRSTRLEN) == NULL)
	                            perror("inet_ntop"); 
	                        _dprintf("sherry_debug2 ipstr =%s\n", ipv6str);
	                        //TODO
	                        //add ipv6 addr to ip6tables
	                    }
	           
	        		} while( ( res = res->ai_next ) != NULL );

	       			freeaddrinfo( ressave );
	       		}
	       	}
	       	eval("iptables", "-t", "filter", "-D", "INPUT", "-p", "udp", "--dport", nvram_safe_get("tr_conn_port"), "-j", logaccept);
		}
	}
	nvram_set("tr_stop", "1"); /*add for ui disable tr firt and then enable tr, will do restart_tr ,
								iptables will be delete twice*/
}

int 
start_tr(void)
{
	char *tr_argv[] = { "/sbin/tr069", "-d", TR_FOLDER, NULL};
	char tr_xml_path[64];
	pid_t pid;
	int ret;

	if (!nvram_get_int("tr_enable") || !nvram_invmatch("tr_acs_url", ""))
		return 0;

	if (getpid() != 1) {
		notify_rc_after_wait("start_tr");
		return 0;
	}

	/* Shut down previous instance if any */
	//stop_tr();
	killall_tk("tr069");

	/* Check tr folder whether exists or not */
	if (!check_if_dir_exist(TR_FOLDER))
		mkdir(TR_FOLDER, 0744);

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2)
	/* Check tr folder on jffs whether exists or not */
	if (!check_if_dir_exist(JFFS_TR_FOLDER))
		mkdir(JFFS_TR_FOLDER, 0744);

	/* copy /jffs/tr/tr.xml to /tmp/tr and then delete it */
	if (check_if_file_exist(JFFS_TR_FOLDER"/"TR_XML_FILE)) {
		eval("cp", JFFS_TR_FOLDER"/"TR_XML_FILE, TR_FOLDER);
		unlink(JFFS_TR_FOLDER"/"TR_XML_FILE);
	}

	/* copy /jffs/tr/tr.xml.bak to /tmp/tr and then delete it */
	if (check_if_file_exist(JFFS_TR_FOLDER"/"TR_XML_BAK_FILE)) {
		eval("cp", JFFS_TR_FOLDER"/"TR_XML_BAK_FILE, TR_FOLDER);
		unlink(JFFS_TR_FOLDER"/"TR_XML_BAK_FILE);
	}

	/* untar /jffs/tr/tr.xml.tz to /tmp/tr */
	if (check_if_file_exist(JFFS_TR_FOLDER"/"TR_XML_TAR_FILE))
		eval("tar", "-zxf", JFFS_TR_FOLDER"/"TR_XML_TAR_FILE);

	/* untar /jffs/tr/tr.xml.bak.tz to /tmp/tr */
	if (check_if_file_exist(JFFS_TR_FOLDER"/"TR_XML_BAK_TAR_FILE))
		eval("tar", "-zxf", JFFS_TR_FOLDER"/"TR_XML_BAK_TAR_FILE);
#endif

	/* Check IPDRDoc folder whether exists or not */
	//if (!check_if_dir_exist(IPDRDOC_FOLDER));
	//	mkdir(IPDRDOC_FOLDER, 0744);

	/* Check tr.xml whether exists or not */
	sprintf(tr_xml_path, "%s/%s", TR_FOLDER, TR_XML_FILE);
	if (!check_if_file_exist(tr_xml_path))
		eval("cp", DEFAULT_TR_XML, tr_xml_path);
	else
	{
		if (!check_xml_file_version(DEFAULT_TR_XML, tr_xml_path))
			eval("cp", DEFAULT_TR_XML, tr_xml_path);
	}

	/* Generate tr configuration file */
	ret = generate_tr_config();
	if (ret < 0)
		return ret;

	/* write cert and key to file */
	save_cert_from_nvram();

	/* add 7547 port iptables*/
	if (nvram_get_int("tr_conn_port")) {
		_dprintf("sherry_debug3 tr_conn_port !=NULL\n");
		char logaccept[32];
		if(strlen(nvram_safe_get("tr_acs_url"))){
			_dprintf("sherry_debug4 tr_acs_url !=NULL\n");
			struct addrinfo *res, *ressave;
        	
        	int ret;
			ret=url2ip(nvram_safe_get("tr_acs_url"), &res);
			ressave = res;
			if( (ret != -1) && (res!=NULL) && (res->ai_addr !=NULL) ){
        		do {
        			if(res->ai_family == AF_INET){//ipv4
                        char ipv4str[INET_ADDRSTRLEN];
                        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)(res->ai_addr))->sin_addr),ipv4str, INET_ADDRSTRLEN) == NULL)
                           	perror("inet_ntop"); 
                        _dprintf("sherry_debug2 ipstr =%s\n", ipv4str);
                        /* Determine the log type */
					if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
						strcpy(logaccept, "logaccept");
					else strcpy(logaccept, "ACCEPT");
                        eval("iptables", "-t", "filter", "-A", "INPUT", "-s", ipv4str, "-p", "tcp", "-m", "tcp", "--dport", nvram_safe_get("tr_conn_port") ,"-j", logaccept);

                    }else if(res->ai_family == AF_INET6){
                        char ipv6str[INET6_ADDRSTRLEN];
                        if(inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr),ipv6str, INET6_ADDRSTRLEN) == NULL)
                            perror("inet_ntop"); 
                        _dprintf("sherry_debug2 ipstr =%s\n", ipv6str);
                        //TODO
                        //add ipv6 addr to ip6tables
                    }
           
        		} while( ( res = res->ai_next ) != NULL );

       			freeaddrinfo( ressave );
       		}
       	}
       	eval("iptables", "-t", "filter", "-A", "INPUT", "-p", "udp", "--dport", nvram_safe_get("tr_conn_port"), "-j", logaccept);
	}

	nvram_set("tr_stop", "0"); /*add for ui disable tr firt and then enable tr, will do restart_tr ,
								iptables will be delete twice*/

	/* Execute tr agent */
	ret = _eval(tr_argv, NULL, 0, &pid);

	if (pids("tr069"))
		logmessage("TR-069 agent", "daemon is started");

	nvram_set("tr_acs_url_tmp", nvram_safe_get("tr_acs_url"));

	return ret;
}

int check_udhcpc_lease_exist(char *check_str)
{
	FILE *fp;
	char buf[128];
	int res = 0;

	if (!(fp = fopen(DHCPC_LEASE_LIST, "r")))
		return 0;

	while(fgets(buf, sizeof(buf), fp) != NULL) {
		if(strstr(buf, check_str) != NULL)
			res = 1;	
	}

	fclose(fp);

	return res;
}

int
dhcpc_lease_main(int argc, char **argv)
{
	char *oui = safe_getenv("DNSMASQ_CPEWAN_OUI");
	char *serial = safe_getenv("DNSMASQ_CPEWAN_SERIAL");
	char *class = safe_getenv("DNSMASQ_CPEWAN_CLASS");
	int hn_exist = 1;

	if(argc != 5 && argc != 4)
		return -1;

	if(argc == 4)
		hn_exist = 0;
	else if(argc == 5)
		hn_exist = 1;
	else
		return -1;

	_dprintf("%s():: %s, %s, %s, %s\n", __FUNCTION__, argv[1], argv[2], argv[3], hn_exist ? argv[4]: "No hostname");

//	if((oui && *oui) && (serial && *serial)) {
	if(!pids("tr069")) {	/* write the info of manageable device into the file */		
		if(!strcmp(argv[1], "add") || !strcmp(argv[1], "old")) {
			if((oui && *oui) && (serial && *serial)) {
				char check_str[128];

				snprintf(check_str, sizeof(check_str), "%s,%s,%s,%s", oui, serial, class, argv[2]);

				if(!check_udhcpc_lease_exist(check_str)) {
					FILE *fp;

					if (!(fp = fopen(DHCPC_LEASE_LIST, "a")))
						return -1;
		
					/* write oui, serial, class */
					_dprintf("%s():: %s %s %s %s\n", __FUNCTION__, argv[1], oui, serial, class);
					fprintf(fp, "%s,%s,%s,%s\n", oui, serial, class, argv[2]);

					fclose(fp);
				}
			}
		}
	}
	else	/* send the info of manageable device to tr069 agent */
	{
		char cmd[256];
		char cmdhost[256];

		memset(cmd, 0 ,sizeof(cmd));
		memset(cmdhost, 0 ,sizeof(cmdhost));

		if(!strcmp(argv[1], "add") || !strcmp(argv[1], "old")) {
			snprintf(cmdhost, sizeof(cmdhost), "/sbin/sendtocli http://127.0.0.1:1234/hostshost/device \"action=add&ipaddr=%s&hostname=%s\"", argv[3], hn_exist ? argv[4] : "");
				

			if((oui && *oui) && (serial && *serial)) {
				char check_str[128];

				snprintf(check_str, sizeof(check_str), "%s,%s,%s,%s", oui, serial, class, argv[2]);

				if(!check_udhcpc_lease_exist(check_str)) {
					FILE *fp;

					if (!(fp = fopen(DHCPC_LEASE_LIST, "a")))
						return -1;
		
					/* write oui, serial, class */
					_dprintf("%s():: %s %s %s %s\n", __FUNCTION__, argv[1], oui, serial, class);
					fprintf(fp, "%s,%s,%s,%s\n", oui, serial, class, argv[2]);

					fclose(fp);
				}

				_dprintf("%s():: %s %s %s %s\n", __FUNCTION__, oui, serial, class, argv[2]);
				snprintf(cmd, sizeof(cmd), "/sbin/sendtocli http://127.0.0.1:1234/manageable/device \"action=add&oui=%s&serial=%s&class=%s\"", oui, serial, class);
			}
		}
		else if(!strcmp(argv[1], "del")) {
			snprintf(cmdhost, sizeof(cmdhost), "/sbin/sendtocli http://127.0.0.1:1234/hostshost/device \"action=del&ipaddr=%s&hostname=%s\"", argv[3], hn_exist ? argv[4] : "");

			FILE *fp;
			char buf[128]; 
			char *oui_str = NULL, *serial_str = NULL, *class_str = NULL, *hwaddr_str = NULL;
						
			if (!(fp = fopen(DHCPC_LEASE_LIST, "r")))
				return -1;

			while(fgets(buf, sizeof(buf), fp) != NULL) {
				if(strstr(buf, argv[2]) != NULL) {
					if ((vstrsep(buf, ",", &oui_str, &serial_str, &class_str, &hwaddr_str) != 4))
						continue;
					_dprintf("%s():: oui: %s, serial: %s, class: %s, hwaddr: %s\n", oui_str, serial_str, class_str, hwaddr_str);
					snprintf(cmd, sizeof(cmd), "/sbin/sendtocli http://127.0.0.1:1234/manageable/device \"action=del&oui=%s&serial=%s&class=%s\"", oui_str, serial_str, class_str);
					break;
				}
			}

			fclose(fp);				
		}	
			
		if(strlen(cmd)) {
			_dprintf("%s():: %s\n", __FUNCTION__, cmd);
			system(cmd);
		}

	}

	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
}
