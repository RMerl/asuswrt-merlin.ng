#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <sys/ioctl.h>
#include <errno.h>
//#include <net/if.h>
//#include <linux/wireless.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
//#include <wlutils.h>
#include <linux_gpio.h>
#include <etioctl.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"
#include <sys/mman.h>
#include <trxhdr.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlioctl.h>

/*******************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include "realtek/realtek.h"
#include "realtek_common.h"
#include <wlscan.h>
//#include "realtek.h"
/*******************/
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif
//#define RTK_TEST
const char WIF_5G[]	= "wl1";
const char WIF_2G[]	= "wl0";
const char VXD_5G[]	= "wl1-vxd";
const char VXD_2G[]	= "wl0-vxd";
#define TXPWR_THRESHOLD_1	25
#define TXPWR_THRESHOLD_2	50
#define TXPWR_THRESHOLD_3	88
#define TXPWR_THRESHOLD_4	100

#define CONFIG_RTL_11AC_SUPPORT
#define CONFIG_RTL_92D_SUPPORT
/********************************************************************/
unsigned int hw_setting_offset;

int rtk_file_lock(int fd)
{
	int ret;
	struct flock lock;
	lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
try_again:
	ret = fcntl(fd,F_SETLKW,&lock);
	if(ret == -1) {
		if (errno == EINTR) {
			goto try_again;
		}
	}
	return ret;
}

int rtk_file_unlock(int fd)
{
	close(fd);
}

int isFileExist(char *file_name)
{
	struct stat status;

	if ( stat(file_name, &status) < 0)
		return 0;

	return 1;
}

int getPid_fromFile(char *file_name)
{
	FILE *fp;
	char *pidfile = file_name;
	int result = -1;
	
	fp= fopen(pidfile, "r");
	if (!fp) {
        	printf("can not open:%s\n", file_name);
		return -1;
   	}
	fscanf(fp,"%d",&result);
	fclose(fp);
	
	return result;
}

int rtk_get_channel_list_via_country(char* country_code,char* list,wlan_band band)
{
	rtklog("%s\n",__FUNCTION__);
	int i, j, num, ret = -1;
	int regDomain;
	char tmp[8];
	if(country_code == NULL||list == NULL)
	{
		return ret;
	}
	if(band !=0 && band != 1)
	{
		return ret;
	}
	num = sizeof(countryIEArray)/sizeof(COUNTRY_IE_ELEMENT);
	rtklog("num:%d\n",num);
	for(i=0;i<num;i++)
	{
		if(!strcasecmp(country_code,countryIEArray[i].countryA2))
		{
			if(band == WLAN_5G)
			{
				regDomain = countryIEArray[i].A_Band_Region;
				rtklog("regDomain:%d\n",regDomain);
				if (regDomain == DOMAIN_ETSI || regDomain == DOMAIN_MKK || nvram_contains_word("rc_support", "dfs"))
				{ 
					for(j=0;j<reg_channel_5g_full_band[regDomain-1].len;j++)
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d",reg_channel_5g_full_band[regDomain-1].channel[j]);
						strcat(list,tmp);
						if(j != reg_channel_5g_full_band[regDomain-1].len - 1)
							strcat(list,",");
					}
				} else {
					for(j=0;j<reg_channel_5g_not_dfs_band[regDomain-1].len;j++)
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d",reg_channel_5g_not_dfs_band[regDomain-1].channel[j]);
						strcat(list,tmp);
						if(j != reg_channel_5g_not_dfs_band[regDomain-1].len - 1)
							strcat(list,",");
					}					
				}
			}
			else if(band == WLAN_2G)
			{
				regDomain = countryIEArray[i].G_Band_Region;
				rtklog("regDomain:%d\n",regDomain);
				for(j=0;j<reg_channel_2_4g[regDomain-1].len;j++)
				{
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d",reg_channel_2_4g[regDomain-1].channel[j]);
					strcat(list,tmp);
					if(j != reg_channel_2_4g[regDomain-1].len - 1)
						strcat(list,",");
				}
			}
			ret = 0;
			break;
		}
	}
	return ret;
}

int rtk_web_get_channel_list(char* country_code,wl_uint32_list_t *list,wlan_band band)
{
//	cprintf("%s\n",__FUNCTION__);
	int i, j, num, ret = -1;
	int regDomain;
	char tmp[8];
	if(country_code == NULL||list == NULL)
	{
		return ret;
	}
	if(band !=0 && band != 1)
	{
		return ret;
	}
	num = sizeof(countryIEArray)/sizeof(COUNTRY_IE_ELEMENT);
	//cprintf("num:%d country_code=%s\n",num,country_code);
	for(i=0;i<num;i++)
	{
		if(!strcasecmp(country_code,countryIEArray[i].countryA2))
		{
		//	cprintf("band:%d i=%d\n",band,i);
			if(band == WLAN_5G)
			{
				regDomain = countryIEArray[i].A_Band_Region;
				//cprintf("regDomain:%d\n",regDomain);
				list->count=reg_channel_5g_full_band[regDomain-1].len;
				for(j=0;j<reg_channel_5g_full_band[regDomain-1].len;j++)
				{
					list->element[j]=reg_channel_5g_full_band[regDomain-1].channel[j];	
				//	cprintf(" %d",list->element[j]);
				}
			}
			else if(band == WLAN_2G)
			{
				regDomain = countryIEArray[i].G_Band_Region;
				//cprintf("regDomain:%d\n",regDomain);				
				list->count=reg_channel_2_4g[regDomain-1].len;
				for(j=0;j<reg_channel_2_4g[regDomain-1].len;j++)
				{
					list->element[j]=reg_channel_2_4g[regDomain-1].channel[j];					
					//cprintf(" %d",list->element[j]);
				}
			}
			ret = 0;
			break;
		}
	}
	return ret;
}

void read_hw_setting_offset(void) 
{
	FILE *hwpart_proc;
	hwpart_proc = fopen ( "/proc/flash/hwpart", "r" );
	if ( hwpart_proc != NULL )
	{
		 char buf[16];
		 unsigned int hw_setting_off=0;

		 fgets(buf, sizeof(buf), hwpart_proc);        /* eat line */
		 sscanf(buf, "%x",&hw_setting_off);
#ifndef CONFIG_MTD_NAND
		if(hw_setting_off == 0)
			hw_setting_off = hw_setting_offset;
#endif
		 hw_setting_offset = hw_setting_off;
		 fclose(hwpart_proc);		 
		 rtk_printf("read_hw_setting_offset = %x \n",hw_setting_offset);
	}
}

#ifdef HW_SETTING_CHECKSUM
void restore_setting(void)
{
	int fh, hwconf_len = sizeof(HW_SETTING_T);
	unsigned char *data = malloc(hwconf_len+1);// one byte for checksum

#ifdef CONFIG_MTD_NAND
	fh = open(FLASH_DEVICE_NAME, O_RDWR);
#else
	fh = open(FLASH_DEVICE_NAME, O_RDWR);
#endif

	if ( fh == -1 )	{
		free(data);
		rtk_printf("open file error\n");
		return;
	}

	lseek(fh, HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T), SEEK_SET);
	if (read(fh, data, hwconf_len+1) != (hwconf_len+1)) {
		free(data);
		close(fh);
		return;
	}

	if ( !CHECKSUM_OK(data, hwconf_len+1) ) {
#ifdef NAND_DUAL_SETTING
		printf("\n\n==> checksum error, resotre from /hw_setting/hw_backup.bin\n\n");
		system("cp /hw_setting/hw_backup.bin /hw_setting/hw.bin");
#else
		rtk_printf("\n\n==> checksum error!\n\n");
#endif
	}

	free(data);
	close(fh);
}
#endif /* HW_SETTING_CHECKSUM */

int rtk_flash_read(char *buf, int offset, int len)
{
	rtklog("%s\n",__FUNCTION__);
	int fh;
	int ok=1;

#ifdef HW_SETTING_CHECKSUM
	restore_setting();
#endif

#ifdef CONFIG_MTD_NAND
	fh = open(FLASH_DEVICE_NAME, O_RDWR|O_CREAT);
#else
	fh = open(FLASH_DEVICE_NAME, O_RDWR);
#endif

	if ( fh == -1 )
	{
		rtk_printf("open file error\n");
		return 0;
	}

	lseek(fh, offset, SEEK_SET);

	if ( read(fh, buf, len) != len)
		ok = 0;

	close(fh);

	return ok;
}

int rtk_flash_write(char *buf, int offset, int len)
{
	rtklog("%s\n",__FUNCTION__);
	int fh;
	int ok=1;
#ifdef CONFIG_MTD_NAND
	fh = open(FLASH_DEVICE_NAME, O_RDWR|O_CREAT);
#else
	fh = open(FLASH_DEVICE_NAME, O_RDWR);
#endif
	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( write(fh, buf, len) != len)
		ok = 0;

#ifdef HW_SETTING_CHECKSUM//checksum
	if (ok) {
		int hwconf_len = sizeof(HW_SETTING_T);
		unsigned char checksum, *data = malloc(hwconf_len+1);

		lseek(fh, HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T), SEEK_SET);
		if (read(fh, data, hwconf_len) != hwconf_len)
			ok = 0;

		if (ok) {
			checksum = CHECKSUM(data, hwconf_len);
			lseek(fh, HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T), SEEK_SET);
			data[hwconf_len]=checksum;
			if ( write(fh, data, hwconf_len+1) != (hwconf_len+1)) {
				ok = 0;
			}

			//rtk_printf("==> checksum=0x%x\n", checksum);
			free(data);
		}
	}
#endif /* HW_SETTING_CHECKSUM */

	close(fh);
	sync();

#ifdef NAND_DUAL_SETTING//dual setting
	system("cp /hw_setting/hw.bin /hw_setting/hw_backup.bin");

	/* copy hw.bin to backup partition */
	{
		struct stat st;
		if (0 == stat("/hw_setting2", &st)) {
			system("cp /hw_setting/hw.bin /hw_setting2/hw.bin");
		}
	}
#endif

	return ok;
}

int read_hw_setting(char *buf)
{
	PARAM_HEADER_T header;
	if(rtk_flash_read(&header,HW_SETTING_OFFSET,sizeof(PARAM_HEADER_T))==0){
		rtklog("Read wlan hw setting header failed\n");
		return -1;
	}
	if(memcmp(header.signature, HW_SETTING_HEADER_TAG, TAG_LEN)){
		rtklog("Invild wlan hw setting signature!\n");
		return -1;
	}
	if(rtk_flash_read(buf,HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T),header.len)==0){
		rtklog("Read wlan hw setting to memory failed\n");
		return -1;
	}
	return 0;
}

int read_hw_setting_length()
{
	PARAM_HEADER_T header;
	int len;
	if(rtk_flash_read(&header,HW_SETTING_OFFSET,sizeof(PARAM_HEADER_T))==0){
		rtklog("Read wlan hw setting header failed\n");
		return -1;
	}
	if(memcmp(header.signature, HW_SETTING_HEADER_TAG, TAG_LEN)){
		rtklog("Invild wlan hw setting signature %s!\n",header.signature);
		return -1;
	}
	len = header.len;
	return len;
}

static int hex_to_string(unsigned char *hex,char *str,int len)
{
	int i;
	char *d,*s;
	const static char hexdig[] = "0123456789abcdef";
	if(hex == NULL||str == NULL)
		return -1;
	d = str;
	s = hex;
	
	for(i = 0;i < len;i++,s++){
		*d++ = hexdig[(*s >> 4) & 0xf];
		*d++ = hexdig[*s & 0xf];
	}
	*d = 0;
	return 0;
}
#if defined(CONFIG_RTL_11AC_SUPPORT)

#define B1_G1	40
#define B1_G2	48

#define B2_G1	56
#define B2_G2	64

#define B3_G1	104
#define B3_G2	112
#define B3_G3	120
#define B3_G4	128
#define B3_G5	136
#define B3_G6	144

#define B4_G1	153
#define B4_G2	161
#define B4_G3	169
#define B4_G4	177

void assign_diff_AC(unsigned char* pMib, unsigned char* pVal)
{
	int x=0, y=0;

	memset((pMib+35), pVal[0], (B1_G1-35));
	memset((pMib+B1_G1), pVal[1], (B1_G2-B1_G1));
	memset((pMib+B1_G2), pVal[2], (B2_G1-B1_G2));
	memset((pMib+B2_G1), pVal[3], (B2_G2-B2_G1));
	memset((pMib+B2_G2), pVal[4], (B3_G1-B2_G2));
	memset((pMib+B3_G1), pVal[5], (B3_G2-B3_G1));
	memset((pMib+B3_G2), pVal[6], (B3_G3-B3_G2));
	memset((pMib+B3_G3), pVal[7], (B3_G4-B3_G3));
	memset((pMib+B3_G4), pVal[8], (B3_G5-B3_G4));
	memset((pMib+B3_G5), pVal[9], (B3_G6-B3_G5));
	memset((pMib+B3_G6), pVal[10], (B4_G1-B3_G6));
	memset((pMib+B4_G1), pVal[11], (B4_G2-B4_G1));
	memset((pMib+B4_G2), pVal[12], (B4_G3-B4_G2));
	memset((pMib+B4_G3), pVal[13], (B4_G4-B4_G3));

}
void assign_diff_AC_hex_to_string(unsigned char* pmib,char* str,int len)
{
	char mib_buf[MAX_5G_CHANNEL_NUM_MIB];
	memset(mib_buf,0,sizeof(mib_buf));
	assign_diff_AC(mib_buf, pmib);
	hex_to_string(mib_buf,str,MAX_5G_CHANNEL_NUM_MIB);
}
#endif

int set_led_type(HW_WLAN_SETTING_Tp phw)
{
	unsigned char tmpbuff[100] = {0};    
#ifdef HAVE_RTK_DUAL_BAND_SUPPORT
	HW_WLAN_SETTING_Tp phw_5g = (HW_WLAN_SETTING_Tp)((unsigned char *)phw+sizeof(HW_WLAN_SETTING_T));
#endif

	sprintf(tmpbuff,"iwpriv wlan0 set_mib led_type=%d",phw->ledType);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
#ifdef HAVE_RTK_DUAL_BAND_SUPPORT
	sprintf(tmpbuff,"iwpriv wlan1 set_mib led_type=%d",phw_5g->ledType);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
#endif
	return 0;
}

int set_mimo_tr_mode(unsigned char board_ver, char* interface)
{
	char tmpbuff[1024];
	unsigned int MIMO_TR_mode;

	if(board_ver == 1)
		MIMO_TR_mode = 5;   // 3T3R
	else if(board_ver == 2)
		MIMO_TR_mode = 3;   // 2T2R
	else if(board_ver == 3)
		MIMO_TR_mode = 2;   // 2T4R
	else
		MIMO_TR_mode = 5;   // 3T3R

	sprintf(tmpbuff,"iwpriv %s set_mib MIMO_TR_mode=%d",interface,MIMO_TR_mode);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	return 0;
}

int set_tx_calibration(HW_WLAN_SETTING_Tp phw,char* interface,int txpower)
{
	char tmpbuff[1024],p[MAX_5G_CHANNEL_NUM_MIB*2+1];
	int intVal = 0;
	int i = 0;

	if(!phw)
		return -1;
	
	if(txpower == TXPWR_THRESHOLD_4)
		intVal = 0;
	else if(txpower >= TXPWR_THRESHOLD_3)
		intVal = 1;
	else if(txpower >= TXPWR_THRESHOLD_2)
		intVal = 6;
	else if(txpower >= TXPWR_THRESHOLD_1)
		intVal = 12;
	else if (txpower >= 1)
		intVal = 17;
	else
		intVal = 99; // Special case. Reduce txpower to 1.
	
	sprintf(tmpbuff,"iwpriv %s set_mib ther=%d",interface,phw->Ther);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	sprintf(tmpbuff,"iwpriv %s set_mib xcap=%d",interface,phw->xCap);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	if(phw->regDomain == 3 || phw->regDomain == 6)/*CE or JP*/
	{
		sprintf(tmpbuff,"iwpriv %s set_mib adaptivity_enable=1",interface);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	}
	else
	{
		sprintf(tmpbuff,"iwpriv %s set_mib adaptivity_enable=0",interface);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	}
	if(phw->regDomain == 6)/*JP*/
	{
		sprintf(tmpbuff,"iwpriv %s set_mib Carrier_Sense_enable=1",interface);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);		
	}
	else
	{
		sprintf(tmpbuff,"iwpriv %s set_mib Carrier_Sense_enable=0",interface);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);		
	}
	if(!strcmp(interface,"wl1"))/*5G*/
	{
		if(phw->regDomain == 3 || phw->regDomain == 6 || nvram_contains_word("rc_support", "dfs")) /*CE or JP*/
		{
			sprintf(tmpbuff,"iwpriv %s set_mib disable_DFS=0",interface);
			system(tmpbuff);
			rtk_printf("%s\n",tmpbuff);
		}
		else
		{
			sprintf(tmpbuff,"iwpriv %s set_mib disable_DFS=1",interface);
			system(tmpbuff);
			rtk_printf("%s\n",tmpbuff);
		}
	}
	else
	{
		sprintf(tmpbuff,"iwpriv %s set_mib disable_DFS=1",interface);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	}
#if 1	
	sprintf(tmpbuff,"iwpriv %s set_mib regdomain=%d",interface,phw->regDomain);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	sprintf(tmpbuff,"iwpriv %s set_mib led_type=%d",interface,phw->ledType);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	sprintf(tmpbuff,"iwpriv %s set_mib tssi1=%d",interface,phw->TSSI1);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	sprintf(tmpbuff,"iwpriv %s set_mib tssi2=%d",interface,phw->TSSI2);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	sprintf(tmpbuff,"iwpriv %s set_mib trswitch=%d",interface,phw->trswitch);
 	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
#endif

	if (intVal && access_point_mode()) {
		for (i = 0; i < MAX_2G_CHANNEL_NUM_MIB; i++) {
			if (phw->pwrlevelCCK_A[i] != 0)
				if ((phw->pwrlevelCCK_A[i] - intVal) >= 1)
					phw->pwrlevelCCK_A[i] -= intVal;
				else
					phw->pwrlevelCCK_A[i] = 1;

			if (phw->pwrlevelCCK_B[i] != 0)
				if ((phw->pwrlevelCCK_B[i] - intVal) >= 1)
					phw->pwrlevelCCK_B[i] -= intVal;
				else
					phw->pwrlevelCCK_B[i] = 1;
#if defined(RPAC68U)
			if (phw->pwrlevelCCK_C[i] != 0)
				if ((phw->pwrlevelCCK_C[i] - intVal) >= 1)
					phw->pwrlevelCCK_C[i] -= intVal;
				else
					phw->pwrlevelCCK_C[i] = 1;

			if (phw->pwrlevelCCK_D[i] != 0)
				if ((phw->pwrlevelCCK_D[i] - intVal) >= 1)
					phw->pwrlevelCCK_D[i] -= intVal;
				else
					phw->pwrlevelCCK_D[i] = 1;
#endif				
			if (phw->pwrlevelHT40_1S_A[i] != 0)
				if ((phw->pwrlevelHT40_1S_A[i] - intVal) >= 1)
					phw->pwrlevelHT40_1S_A[i] -= intVal;
				else
					phw->pwrlevelHT40_1S_A[i] = 1;

			if (phw->pwrlevelHT40_1S_B[i] != 0)
				if ((phw->pwrlevelHT40_1S_B[i] - intVal) >= 1)
					phw->pwrlevelHT40_1S_B[i] -= intVal;
				else
					phw->pwrlevelHT40_1S_B[i] = 1;
#if defined(RPAC68U)
			if (phw->pwrlevelHT40_1S_C[i] != 0)
				if ((phw->pwrlevelHT40_1S_C[i] - intVal) >= 1)
					phw->pwrlevelHT40_1S_C[i] -= intVal;
				else
					phw->pwrlevelHT40_1S_C[i] = 1;

			if (phw->pwrlevelHT40_1S_D[i] != 0)
				if ((phw->pwrlevelHT40_1S_D[i] - intVal) >= 1)
					phw->pwrlevelHT40_1S_D[i] -= intVal;
				else
					phw->pwrlevelHT40_1S_D[i] = 1;
#endif				
		}		
		hex_to_string(phw->pwrlevelCCK_A, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelCCK_A=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);

		hex_to_string(phw->pwrlevelCCK_B, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelCCK_B=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);
#if defined(RPAC68U)		
		hex_to_string(phw->pwrlevelCCK_C, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelCCK_C=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);

		hex_to_string(phw->pwrlevelCCK_D, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelCCK_D=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);
#endif
		hex_to_string(phw->pwrlevelHT40_1S_A, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelHT40_1S_A=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);

		hex_to_string(phw->pwrlevelHT40_1S_B, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelHT40_1S_B=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);
#if defined(RPAC68U)		
		hex_to_string(phw->pwrlevelHT40_1S_C, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelHT40_1S_C=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);

		hex_to_string(phw->pwrlevelHT40_1S_D, p, MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevelHT40_1S_D=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);
#endif
		for (i = 0; i < MAX_5G_CHANNEL_NUM_MIB; i++) {
			if (phw->pwrlevel5GHT40_1S_A[i] != 0)
				if ((phw->pwrlevel5GHT40_1S_A[i] - intVal) >= 1)
					phw->pwrlevel5GHT40_1S_A[i] -= intVal;
				else
					phw->pwrlevel5GHT40_1S_A[i] = 1;

			if (phw->pwrlevel5GHT40_1S_B[i] != 0)
				if ((phw->pwrlevel5GHT40_1S_B[i] - intVal) >= 1)
					phw->pwrlevel5GHT40_1S_B[i] -= intVal;
				else
					phw->pwrlevel5GHT40_1S_B[i] = 1;
#if defined(RPAC68U)
			if (phw->pwrlevel5GHT40_1S_C[i] != 0)
				if ((phw->pwrlevel5GHT40_1S_C[i] - intVal) >= 1)
					phw->pwrlevel5GHT40_1S_C[i] -= intVal;
				else
					phw->pwrlevel5GHT40_1S_C[i] = 1;

			if (phw->pwrlevel5GHT40_1S_D[i] != 0)
				if ((phw->pwrlevel5GHT40_1S_D[i] - intVal) >= 1)
					phw->pwrlevel5GHT40_1S_D[i] -= intVal;
				else
					phw->pwrlevel5GHT40_1S_D[i] = 1;
#endif
		}
		hex_to_string(phw->pwrlevel5GHT40_1S_A, p, MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevel5GHT40_1S_A=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);

		hex_to_string(phw->pwrlevel5GHT40_1S_B, p, MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevel5GHT40_1S_B=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);
#if defined(RPAC68U)
		hex_to_string(phw->pwrlevel5GHT40_1S_C, p, MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevel5GHT40_1S_C=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);

		hex_to_string(phw->pwrlevel5GHT40_1S_D, p, MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff, "iwpriv %s set_mib pwrlevel5GHT40_1S_D=%s", interface, p);
		system(tmpbuff);
		rtk_printf("%s\n", tmpbuff);
#endif		
	}
	else{
		hex_to_string(phw->pwrlevelCCK_A - intVal,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelCCK_A=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);

		hex_to_string(phw->pwrlevelCCK_B,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelCCK_B=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);

#if defined(RPAC68U)
		hex_to_string(phw->pwrlevelCCK_C - intVal,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelCCK_C=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	
		hex_to_string(phw->pwrlevelCCK_D,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelCCK_D=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
#endif
		hex_to_string(phw->pwrlevelHT40_1S_A,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelHT40_1S_A=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	
		hex_to_string(phw->pwrlevelHT40_1S_B,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelHT40_1S_B=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
#if defined(RPAC68U)
		hex_to_string(phw->pwrlevelHT40_1S_C,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelHT40_1S_C=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	
		hex_to_string(phw->pwrlevelHT40_1S_D,p,MAX_2G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevelHT40_1S_D=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
#endif		
		hex_to_string(phw->pwrlevel5GHT40_1S_A,p,MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevel5GHT40_1S_A=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	
		hex_to_string(phw->pwrlevel5GHT40_1S_B,p,MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevel5GHT40_1S_B=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);

#if defined(RPAC68U)
		hex_to_string(phw->pwrlevel5GHT40_1S_C,p,MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevel5GHT40_1S_C=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
	
		hex_to_string(phw->pwrlevel5GHT40_1S_D,p,MAX_5G_CHANNEL_NUM_MIB);
		sprintf(tmpbuff,"iwpriv %s set_mib pwrlevel5GHT40_1S_D=%s",interface,p);
		system(tmpbuff);
		rtk_printf("%s\n",tmpbuff);
#endif
	}
	
	hex_to_string(phw->pwrdiffHT40_2S,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiffHT40_2S=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
		
	hex_to_string(phw->pwrdiffHT20,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiffHT20=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	hex_to_string(phw->pwrdiffOFDM,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiffOFDM=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

#if 1
#if defined(RPAC68U)
	hex_to_string(phw->pwrdiff_40BW3S_20BW3S_A,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW3S_20BW3S_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff_40BW3S_20BW3S_B,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW3S_20BW3S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	hex_to_string(phw->pwrdiff_20BW1S_OFDM1T_C,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_20BW1S_OFDM1T_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff_40BW2S_20BW2S_C,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW2S_20BW2S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff_40BW3S_20BW3S_C,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW3S_20BW3S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff_20BW1S_OFDM1T_D,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_20BW1S_OFDM1T_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff_40BW2S_20BW2S_D,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW2S_20BW2S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff_40BW3S_20BW3S_D,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW3S_20BW3S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW3S_20BW3S_A,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW3S_20BW3S_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW3S_20BW3S_B,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW3S_20BW3S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW3S_160BW3S_A,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW3S_160BW3S_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW3S_160BW3S_B,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW3S_160BW3S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_20BW1S_OFDM1T_C,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_20BW1S_OFDM1T_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW2S_20BW2S_C,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW2S_20BW2S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW3S_20BW3S_C,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW3S_20BW3S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW1S_160BW1S_C,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW1S_160BW1S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW2S_160BW2S_C,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW2S_160BW2S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW3S_160BW3S_C,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW3S_160BW3S_C=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_20BW1S_OFDM1T_D,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_20BW1S_OFDM1T_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW2S_20BW2S_D,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW2S_20BW2S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW3S_20BW3S_D,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW3S_20BW3S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW1S_160BW1S_D,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW1S_160BW1S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW2S_160BW2S_D,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW2S_160BW2S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW3S_160BW3S_D,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW3S_160BW3S_D=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff); 	
#endif
#endif //end #if 1
	
#ifdef CONFIG_RTL_92D_SUPPORT
#if defined(RPAC68U) || defined(RPAC55)
	hex_to_string(phw->pwrdiff5GHT40_2S,p,MAX_5G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff5GHT40_2S=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	hex_to_string(phw->pwrdiff5GHT20,p,MAX_5G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff5GHT20=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	hex_to_string(phw->pwrdiff5GOFDM,p,MAX_5G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff5GOFDM=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
#endif
#endif

#ifdef CONFIG_RTL_11AC_SUPPORT
	hex_to_string(phw->pwrdiff_20BW1S_OFDM1T_A,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_20BW1S_OFDM1T_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	hex_to_string(phw->pwrdiff_40BW2S_20BW2S_A,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW2S_20BW2S_A=%s",interface,p);
	system(tmpbuff);	
	rtk_printf("%s\n",tmpbuff);
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_20BW1S_OFDM1T_A,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_20BW1S_OFDM1T_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW2S_20BW2S_A,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW2S_20BW2S_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);

	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW1S_160BW1S_A,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW1S_160BW1S_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW2S_160BW2S_A,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW2S_160BW2S_A=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	hex_to_string(phw->pwrdiff_20BW1S_OFDM1T_B,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_20BW1S_OFDM1T_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	hex_to_string(phw->pwrdiff_40BW2S_20BW2S_B,p,MAX_2G_CHANNEL_NUM_MIB);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_40BW2S_20BW2S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);	
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_20BW1S_OFDM1T_B,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_20BW1S_OFDM1T_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_40BW2S_20BW2S_B,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_40BW2S_20BW2S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW1S_160BW1S_B,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW1S_160BW1S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);
	
	assign_diff_AC_hex_to_string(phw->pwrdiff_5G_80BW2S_160BW2S_B,p,MAX_5G_DIFF_NUM);
	sprintf(tmpbuff,"iwpriv %s set_mib pwrdiff_5G_80BW2S_160BW2S_B=%s",interface,p);
	system(tmpbuff);
	rtk_printf("%s\n",tmpbuff);	
#endif
	return 0;
}


int set_wifi_mac(char* buf)
{
	unsigned int offset, wifi_count;
	unsigned char tmpbuff[64];
	unsigned char mactmp[ETH_ALEN+1];
	int idx;

	memset(mactmp,0,ETH_ALEN+1);
	/* ethernet mac */
	
#if 0
	idx = 0;
	offset = HW_SETTING_ETHMAC_OFFSET + idx*ETH_ALEN;
	memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);
	sprintf(tmpbuff,"ifconfig br-lan hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
 	system(tmpbuff);

	memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);
	sprintf(tmpbuff,"ifconfig eth0 hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
 	system(tmpbuff);
    
	idx = 1;
	offset = HW_SETTING_ETHMAC_OFFSET + idx*ETH_ALEN;
	memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);
	sprintf(tmpbuff,"ifconfig eth1 hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
 	system(tmpbuff);
#endif
#if !defined(HAVE_RTK_EFUSE)
	/* wifi mac */
	idx = 0;
	offset = HW_WLAN_SETTING_OFFSET + sizeof(struct hw_wlan_setting) * idx;
	memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);	
	sprintf(tmpbuff,"ifconfig wlan0 hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
	//rtk_printf("cmd=%s\n",tmpbuff);
	system(tmpbuff);
//#if defined(HAVE_WIFI_MBSSID)
#if 0
	for(wifi_count=0;wifi_count<HAVE_WIFI_MBSSID;wifi_count++){
		memcpy(mactmp,(unsigned char *)(buf+offset+ETH_ALEN*(wifi_count+1)),ETH_ALEN);	
		sprintf(tmpbuff,"ifconfig wlan0-%d hw ether %02x%02x%02x%02x%02x%02x",wifi_count+1,mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
		//rtk_printf("cmd=%s\n",tmpbuff);
		system(tmpbuff);
	}
#endif
#ifdef HAVE_RTK_DUAL_BAND_SUPPORT
	idx = 1;
	offset = HW_WLAN_SETTING_OFFSET + sizeof(struct hw_wlan_setting) * idx;
	memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);	
	sprintf(tmpbuff,"ifconfig wlan1 hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
	//rtk_printf("cmd=%s\n",tmpbuff);
	system(tmpbuff);
//#if defined(HAVE_WIFI_MBSSID)
#if 0
	for(wifi_count=0;wifi_count<HAVE_WIFI_MBSSID;wifi_count++){
		memcpy(mactmp,(unsigned char *)(buf+offset+ETH_ALEN*(wifi_count+1)),ETH_ALEN);   
		sprintf(tmpbuff,"ifconfig wlan1-%d hw ether %02x%02x%02x%02x%02x%02x",wifi_count+1,mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
		//rtk_printf("cmd=%s\n",tmpbuff);
		system(tmpbuff);
	}
#endif	//MBSSID
#endif	//HAVE_RTK_DUAL_BAND_SUPPORT
#endif	//HAVE_RTK_EFUSE
}

int set_ethernet_mac(char* buf)
{
	unsigned int offset;
    unsigned char tmpbuff[64];
    unsigned char mactmp[ETH_ALEN+1];
    int idx;

    memset(mactmp,0,ETH_ALEN+1);
    /* ethernet mac */
    
    idx = 0;
    offset = HW_SETTING_ETHMAC_OFFSET + idx*ETH_ALEN;
    memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);
    sprintf(tmpbuff,"ifconfig br-lan hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
    system(tmpbuff);

    memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);
    sprintf(tmpbuff,"ifconfig eth0 hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
    system(tmpbuff);

    idx = 1;
    offset = HW_SETTING_ETHMAC_OFFSET + idx*ETH_ALEN;
    memcpy(mactmp,(unsigned char *)(buf+offset),ETH_ALEN);
    sprintf(tmpbuff,"ifconfig eth1 hw ether %02x%02x%02x%02x%02x%02x",mactmp[0],mactmp[1],mactmp[2],mactmp[3],mactmp[4],mactmp[5]);
    system(tmpbuff);

}
/*********************************************************************/

#define GPIOLIB_DIR     "/sys/class/gpio"
 
	/* Export specified GPIO
  	 * @return:
 	 *      0:      success
	 *  otherwise:  fail
	 */
static int __export_gpio(uint32_t gpio)
{
	char gpio_path[PATH_MAX], export_path[PATH_MAX], gpio_str[] = "999XXX";
 
	if (!d_exists(GPIOLIB_DIR)) {
		rtklog("%s does not exist!\n", __func__);
		return -1;
	}
	sprintf(gpio_path, "%s/gpio%d", GPIOLIB_DIR, gpio);
	if (d_exists(gpio_path))
		return 0;
	
	rtklog("export gpio %d\n", gpio);
	sprintf(export_path, "%s/export", GPIOLIB_DIR);
	sprintf(gpio_str, "%d", gpio);
	f_write_string(export_path, gpio_str, 0, 0);

	return 0;
}

uint32_t gpio_dir(uint32_t gpio, int dir)
{
	char path[PATH_MAX], v[10], *dir_str = "in";

#ifdef RPAC53
	char val_str[32];
	sprintf(val_str, "config 5 w");
	sprintf(path, "/proc/wl0/gpio_ctrl");
	if (gpio == (nvram_get_int("led_2g_red_gpio") & 0xff)) {

		if (dir == GPIO_DIR_OUT)
			f_write_string(path, val_str, 0, 0);

		return 0;
	}
#endif

	if (dir == GPIO_DIR_OUT) {
		dir_str = "out";                /* output, low voltage */
		*v = '\0';
		sprintf(path, "%s/gpio%d/value", GPIOLIB_DIR, gpio);
		if (f_read_string(path, v, sizeof(v)) > 0 && atoi(v) == 1)
		dir_str = "high";       /* output, high voltage */
	}

	__export_gpio(gpio);
	sprintf(path, "%s/gpio%d/direction", GPIOLIB_DIR, gpio);
	f_write_string(path, dir_str, 0, 0);

	return 0;
}

#define swapportstatus(x) \
{ \
    unsigned int data = *(unsigned int*)&(x); \
    data = ((data & 0x000c0000) >> 18) |    \
           ((data & 0x00030000) >> 14) |    \
           ((data & 0x0000c000) >> 10) |    \
           ((data & 0x00003000) >>  6) |    \
	   ((data & 0x00000c00) >>  2);     \
    *(unsigned int*)&(x) = data;            \
}

//extern uint32_t gpio_read(void);
//extern void gpio_write(uint32_t bitvalue, int en);
void gpio_write(uint32_t bitvalue, int en)
{
	//rtklog("%s\n",__FUNCTION__);
	int led_wps_gpio = 0xff;
	int btn_rst_gpio = 0xff;
	led_wps_gpio = nvram_get_int("led_wps_gpio");
	btn_rst_gpio = nvram_get_int("btn_rst_gpio");
	char cmd[64] = {0};
	char path[PATH_MAX], val_str[32];
	//rtklog("%s bitvalue:%d,en:%d\n",__FUNCTION__,bitvalue,en);
	if(bitvalue == (led_wps_gpio & 0xff))
	{
		if(en)
		{
			sprintf(cmd,"echo 1 > /proc/gpio");
		}
		else
		{
			sprintf(cmd,"echo 0 > /proc/gpio");
		}
		doSystem(cmd);
	}
#ifdef RPAC53
	else if (bitvalue == (nvram_get_int("led_2g_red_gpio") & 0xff)) {

		sprintf(val_str, "config 5 w");
		sprintf(path, "/proc/wl0/gpio_ctrl");
		f_write_string(path, val_str, 0, 0);

		if (!en)
			sprintf(val_str, "set 5 1");
		else
			sprintf(val_str, "set 5 0");

		f_write_string(path, val_str, 0, 0);
	}
#endif
	else {
		sprintf(val_str, "%d", !!en);
		sprintf(path, "%s/gpio%d/value", GPIOLIB_DIR, bitvalue);
		f_write_string(path, val_str, 0, 0);
	}
}

uint32_t get_gpio(uint32_t gpio)
{
	FILE *fp;
	int btn_rst_gpio;
	int btn_wps_gpio;
	int ret = 0;
#ifdef RTK_DEBUG
	int type = 0;
#endif
	char path[PATH_MAX], value[10];
	btn_rst_gpio = nvram_get_int("btn_rst_gpio");
	btn_wps_gpio = nvram_get_int("btn_wps_gpio");
#ifdef RPAC68U
	if(gpio == (btn_rst_gpio & 0xff))
	{
#ifdef RTK_DEBUG
		type = 1;
#endif
		fp=fopen("/proc/load_default","r");
		if(fp)
		{
			fscanf(fp,"%d",&ret);
			fclose(fp); 
		}
	}
	else if(gpio == (btn_wps_gpio & 0xff))
#else
	if(gpio == (btn_wps_gpio & 0xff))
#endif
	{
#ifdef RTK_DEBUG
		type = 2;
#endif
		fp=fopen("/proc/gpio","r");
		if(fp)
		{
			fscanf(fp,"%d",&ret);
			fclose(fp); 
		}
	}
#ifdef RPAC53
	else if (gpio == (nvram_get_int("led_2g_red_gpio") & 0xff)) {
		fp = fopen("/proc/wl0/gpio_ctrl", "r");
		if (fp) {
			fscanf(fp,"%d",&ret);
			fclose(fp); 
		}
		return ret;
	}
#endif
	else {
		sprintf(path, "%s/gpio%d/value", GPIOLIB_DIR, gpio);
		f_read_string(path, value, sizeof(value));
		return atoi(value);
	}

	if(ret)
	{
		ret = 0;
	}
	else
	{
		ret  = 1;
	}
	return ret;
}


uint32_t set_gpio(uint32_t gpio, uint32_t value)
{
	gpio_write(gpio, value);
	return 0;
}

#if 0//def RTCONFIG_BCMFA
int get_fa_rev(void)
{
	//TODO
	return 0;
}

int get_fa_dump(void)
{
	//TODO
	return 0;
}
#endif

int get_switch_model(void)
{
	//TODO
	return SWITCH_UNKNOWN;
}

int robo_ioctl(int fd, int write, int page, int reg, uint32_t *value)
{
	//TODO
	return 0;
}

int phy_ioctl(int fd, int write, int phy, int reg, uint32_t *value)
{
	//TODO
	return 0;
}

// !0: connected
//  0: disconnected


uint32_t get_phy_status(uint32_t portmask)
{
#if 0
	int fd;
	int value=0;
	int    args1[0];
	 unsigned int args[4]={0};
	int i=0;
	int wanflag=1; //to do
 	 int sockfd;
	 struct ifreq ifr;

	args[1] =(unsigned int)args1;
	args[2] =0;
	args[3] =&value;

	 if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	 {
		perror("fatal error socket\n");
		return -3;
	 }
		
	if(wanflag){
 		strcpy((char*)&ifr.ifr_name, "eth1");
		  args[0] = RTL8651_IOCTL_GETWANLINKSTATUS;
	}
	else{
		strcpy((char*)&ifr.ifr_name, "eth0");
		  args[0] = RTL8651_IOCTL_GETLANLINKSTATUS;
	}	
	((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;
	if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
 	{
		close(sockfd);
		return -1;
	}
	close(sockfd);
  	//_dprintf("value:%d,[%s]:[%d].\n",value,__FUNCTION__,__LINE__);
  	return value;	
#endif		
}
#define RTL_WAN_LINKSTATUS	"proc/eth1/link_status"

uint32_t rtkswitch_wanPort_phyStatus(uint32_t wan_unit)
{
	FILE *fp;
	unsigned int value=0;
	char buff[32];
	if(wan_unit==0)
	{
		if((fp= fopen(RTL_WAN_LINKSTATUS, "r"))==NULL)
			return value;
		fgets(buff, 32, fp);
		value =atoi(buff);
		fclose(fp);

		
	}
	return value;
}

void rtl_init_qos_patch( void)
{
	int qos_enabled=0;
	char qos_value[8]={0};
	int fastpath_enabled=0;
	int hwnat_enabled=0;
	int connbyte_match=1;
	char tmp_buffer[128]={0};
	char *buf;
	char *g;
	char *p;
	char *desc, *addr, *port, *prio, *transferred, *proto;
	int class_num;
	int i, inuse;
	
	int v4v6_ok;
	//_dprintf("qos init patch\n");
	//to do fastpath
	if(nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") == 0)
	{
		qos_enabled =1;
	}
	else
		qos_enabled =0;
	
	if(qos_enabled){
		#if 0
		g = buf = strdup(nvram_safe_get("qos_rulelist"));
		while (g) {

			/* ASUSWRT
			qos_rulelist :
				desc>addr>port>proto>transferred>prio

				addr  : (source) IP or MAC or IP-range
				port  : dest port
				proto : tcp, udp, tcp/udp, any , (icmp, igmp)
				transferred : min:max
				prio  : 0-4, 0 is the highest
	  		*/

			if ((p = strsep(&g, "<")) == NULL) break;
			if((vstrsep(p, ">", &desc, &addr, &port, &proto, &transferred, &prio)) != 6) continue;
			class_num = atoi(prio);
			if ((class_num < 0) || (class_num > 4)) continue;

			char *tmp_trans, *q_min, *q_max;
			long min, max ;

			sprintf(tmp, "%s", transferred);
			tmp_trans = tmp;
			q_min = strsep(&tmp_trans, "~");
			q_max = tmp_trans;

			if (strcmp(transferred,"") == 0){
				sprintf(conn, "%s", "");
			}
			else{
				//match connbytes
				connbyte_match =1;
				break;
			}
			

		}
		
		
		if(connbyte_match==0)
		{
			fastpath_enabled =1;
			hwnat_enabled =1;
		}
		else
		#endif
		{
			fastpath_enabled =0;
			hwnat_enabled =0;
		}
	}
	else
	{
		fastpath_enabled =1;
		hwnat_enabled =1;
	}
	memset(tmp_buffer,0,128);
	sprintf(tmp_buffer, "%s %d  %s","echo",qos_enabled,">/proc/qos");
	system(tmp_buffer);
	
	memset(tmp_buffer,0,128);
	sprintf(tmp_buffer, "%s %d  %s","echo",fastpath_enabled,"> /proc/fast_nat");
	system(tmp_buffer);
	
	memset(tmp_buffer,0,128);
	sprintf(tmp_buffer, "%s %d  %s","echo",hwnat_enabled,"> /proc/hw_nat");
	system(tmp_buffer);
	
}

// 2bit per port (0-4(5)*2 shift)
// 0: 10 Mbps
// 1: 100 Mbps
// 2: 1000 Mbps
uint32_t get_phy_speed(uint32_t portmask)
{
	//TODO
	return 0;
}

uint32_t set_phy_ctrl(uint32_t portmask, int ctrl)
{
	//TODO
	return 0;
}

#define IMAGE_HEADER "HDR0"
#define MAX_VERSION_LEN 64
#define MAX_PID_LEN 12
#define MAX_HW_COUNT 4

int fwChecksumOk(char *data, int len)
{
	unsigned short sum=0;
	int i;

	for (i=0; i<len; i+=2) {
#ifdef _LITTLE_ENDIAN_
		sum += WORD_SWAP( *((unsigned short *)&data[i]) );
#else
		sum += *((unsigned short *)&data[i]);
#endif

	}
	return( (sum==0) ? 1 : 0);
}

unsigned int get_radio_status(char *ifname)
{
	char buf[64];
	FILE *fp;
	int len;
	char *pt1,*pt2;

	sprintf(buf, "iwpriv %s get_mib func_off", ifname);

	fp = popen(buf, "r");
	if (fp) {
		memset(buf, 0, sizeof(buf));
		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (len > 1) {
			buf[len-1] = '\0';
			pt1 = strstr(buf, "get_mib:");
			if (pt1) 
			{
				pt2 = pt1 + strlen("get_mib:");
				if (!strchr(pt2, '1'))
					return 1;
			}
		}
	}
	return 0;
}

int get_radio(int unit, int subunit)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";

	if (subunit > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	
	return get_radio_status(nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
}

void set_radio(int on, int unit, int subunit)
{
	//int led = (!unit)? LED_2G:LED_5G, onoff = (!on)? LED_OFF:LED_ON;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX", ifname[]="wlXXXXXX";

	if (!nvram_get_int("wlready")) return;

	if (subunit > 0)
	{
#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_CONCURRENTREPEATER)
		if (sw_mode() == SW_MODE_REPEATER)	/* repeater mode */
			return;
#endif
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	}	
	else
	{   
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	}
	strcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

	if (*ifname != '\0')
	{   
	   	//doSystem("ifconfig %s %s",ifname, on?"up":"down");
		doSystem("iwpriv %s set_mib func_off=%d", ifname, on?0:1);
		if (!on && subunit == 0) // root ap only.
			doSystem("iwpriv %s del_sta all", ifname); // kick all STA.
		if (unit == 0)/* for 2.4G led */
			doSystem("echo 'led %d 255' > /proc/asus_ate", on ? 3: 0);	/* 0 is for off, 3 is for on, 255 is do nothing */
		else if (unit == 1) 	/* for 5G led */
			doSystem("echo 'led 255 %d' > /proc/asus_ate", on ? 3: 0);
	}
}


///////////////////////////////////////////////////////////////


/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}
#ifdef RTK_TEST
int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
	int i=0,j=0;
	for(i=1;i<6;i++)
	{
		pInfo[i].aid=i;
		pInfo[i].addr[0]=0;
		pInfo[i].flag=STA_INFO_FLAG_ASOC;
		pInfo[i].network=BAND_11BG|BAND_11N;
		for(j=1;j<6;j++)
			pInfo[i].addr[j]=i+j;
		pInfo[i].link_time=i*60;
		pInfo[i].tx_packets=i*600;
		pInfo[i].rx_packets=i*2000;		
	}
	pInfo[3].flag|=STA_INFO_FLAG_ASLEEP;
	pInfo[4].network=BAND_5G_11ANAC;
	return 0;
}

int getWlBssInfo(char *interface, bss_info *pInfo)
{
	int i=5,j=0;	
	strcpy(pInfo->ssid,interface);
	strcat(pInfo->ssid,"_test_ssid");
	pInfo->bssid[0]=0;
	for(j=1;j<6;j++)
		pInfo->bssid[j]=i+j;
	pInfo->state=STATE_SCANNING;	
	pInfo->channel=123;
	pInfo->rssi=54;
	pInfo->sq=25;
	return 0;
}


int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
	return 0;
}
int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
	int i=0,j=0;
	pStatus->number=15;
	for(i=0;i<15;i++)
	{
		pStatus->bssdb[i].bdBssId[0]=0;
		for(j=1;i<6;j++)
			pStatus->bssdb[i].bdBssId[j]=i*j;
		pStatus->bssdb[i].bdSsId.Octet="ssidtest";
		pStatus->bssdb[i].bdSsId.Length=strlen("ssidtest");
		pStatus->bssdb[i].ChannelNumber=23;
	}
	return 0;
}

#else
int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
    int skfd=0;
    struct iwreq wrq;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);
    memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0){
    	close( skfd );
		return -1;
	}
    close( skfd );
    return 0;
}

int getWlBssInfo(char *interface, bss_info *pInfo)
{

    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
      /* If no wireless name : no wireless extensions */
      {
      	 close( skfd );
        return -1;
      }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(bss_info);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0){
    	 close( skfd );
	return -1;
	}
    close( skfd );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyRequest(char *interface, int *pStatus)
{

    int skfd=0;
    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0){
    	close( skfd );
	return -1;
	}
    close( skfd );

    if ( result == 0xff )
    	*pStatus = -1;
    else
	*pStatus = (int) result;

	return 0;

}


int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
    int skfd=0,i=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0){
    	close( skfd );
	return -1;
	}
	/*cprintf("%s:%d interface=%s pStatus->number=%d sizeof(SS_STATUS_T)=%d\n",__FUNCTION__,__LINE__,interface,pStatus->number,sizeof(SS_STATUS_T));
	for(i=0;i<500;i++)
	{
		cprintf("%02x",((char*)pStatus)[i]);
	}*/
	close( skfd );
	
    return 0;
}
#endif
int rtk_getAuthMacList(char *interface,void *buff)
{
	WLAN_STA_INFO_T staInfoList[MAX_STA_NUM+1]={0};
	struct maclist* authMacList=(struct maclist*)buff;
	int i=0;
	authMacList->count=0;
	if(getWlStaInfo(interface,staInfoList)<0)
	{
		printf("%s getWlStaInfo fail!\n",interface);
		return -1;
	}
	for(i=1;i<=MAX_STA_NUM; i++)
	{
		if(staInfoList[i].aid && (staInfoList[i].flag & STA_INFO_FLAG_ASOC))
		{
			memcpy(&(authMacList->ea[authMacList->count]),staInfoList[i].addr,sizeof(staInfoList[i].addr));
			authMacList->count++;
		}
	}
	return 0;
}

//get rtk sta info via mac address 
int rtk_getStaInfo(char *interface,char *macAddr,WLAN_STA_INFO_Tp pRtk_sta_info)
{
	WLAN_STA_INFO_T staInfoList[MAX_STA_NUM+1]={0};
	int i=0;
	if(getWlStaInfo(interface,staInfoList)<0)
	{
		printf("%s getWlStaInfo fail!\n",interface);
		return -1;
	}
	for(i=1;i<=MAX_STA_NUM; i++)
	{
		if(staInfoList[i].aid && (staInfoList[i].flag & STA_INFO_FLAG_ASOC))
		{
			if(memcmp(macAddr,staInfoList[i].addr,sizeof(staInfoList[i].addr))==0)
			{				
				//sta->flags|=WL_STA_AUTHO;
				memcpy(pRtk_sta_info,&(staInfoList[i]),sizeof(WLAN_STA_INFO_T));
				return 1;
			}
		}
	}
	
		
	return 0;
}

int getMonitorStaRssi(char *interface, char *buff, int len)
{
	int skfd = 0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd == -1)
		return -1;

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		/* If no wireless name : no wireless extensions */
		close( skfd );
		return -1;
	}

	wrq.u.data.pointer = (void *)buff;
	wrq.u.data.length = len;

	if (iw_get_ext(skfd, interface, SIOCGIWRTLMONITORSTARSSI, &wrq) < 0) {
		close( skfd );
		return -1;
	}

	close( skfd );
	return 0;
}

int getAclInfo(char *interface, WLAN_ACL_INFO_Tp pRtk_acl_info)
{
	int skfd = 0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd == -1)
		return -1;

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		/* If no wireless name : no wireless extensions */
		close( skfd );
		return -1;
	}

	wrq.u.data.pointer = (caddr_t)pRtk_acl_info;
	wrq.u.data.length = sizeof(WLAN_ACL_INFO_T);
	memset(pRtk_acl_info, 0, sizeof(WLAN_ACL_INFO_T));

	if (iw_get_ext(skfd, interface, SIOCGIWRTLACLINFO, &wrq) < 0) {
		close( skfd );
		return -1;
	}

	close( skfd );
	return 0;
}

int getAclList(char *interface, WLAN_ACL_CLIENT_LIST_Tp pRtk_acl_client_list)
{
	int skfd = 0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd == -1)
		return -1;

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		/* If no wireless name : no wireless extensions */
		close( skfd );
		return -1;
	}

	wrq.u.data.pointer = (caddr_t)pRtk_acl_client_list;
	wrq.u.data.length = sizeof(WLAN_ACL_CLIENT_LIST_T);
	memset(pRtk_acl_client_list, 0, sizeof(WLAN_ACL_CLIENT_LIST_T));

	if (iw_get_ext(skfd, interface, SIOCGIWRTLACLCLIENTLIST, &wrq) < 0) {
		close( skfd );
		return -1;
	}

	close( skfd );
	return 0;
}

void set_11ac_txrate(WLAN_STA_INFO_Tp pInfo,char* txrate)
{
	char channelWidth=0;//20M 0,40M 1,80M 2
	char shortGi=0;
	char rate_idx=pInfo->txOperaRates-0xA0;
	if(!txrate)return;
/*
	TX_USE_40M_MODE		= BIT(0),
	TX_USE_SHORT_GI		= BIT(1),
	TX_USE_80M_MODE		= BIT(2)
*/
	if(pInfo->ht_info & 0x4)
		channelWidth=2;
	else if(pInfo->ht_info & 0x1)
		channelWidth=1;
	else
		channelWidth=0;
	if(pInfo->ht_info & 0x2)
		shortGi=1;

	sprintf(txrate, "%d", VHT_MCS_DATA_RATE[channelWidth][shortGi][rate_idx]*5);
}

void dump_sta_info(WLAN_STA_INFO_Tp staInfoList)
{
	int i=0,hr=0,min=0,sec=0;
	char mode_buf[64]={0},outbuf[512]={0};
	rtk_printf("%s:%d\n",__FUNCTION__,__LINE__);
	for(i=0;i<MAX_STA_NUM;i++)
	{
		if (staInfoList[i].addr[0]||staInfoList[i].addr[1]||staInfoList[i].addr[2]
			||staInfoList[i].addr[3]||staInfoList[i].addr[4]||staInfoList[i].addr[5])
		{
			if(staInfoList[i].network& BAND_5G_11AC)
				sprintf(mode_buf, "%s", (" 11ac"));
			else if(staInfoList[i].network & BAND_11N)
				sprintf(mode_buf, "%s", (" 11n"));
			else if (staInfoList[i].network & BAND_11G)
				sprintf(mode_buf,"%s",  (" 11g"));	
			else if (staInfoList[i].network & BAND_11B)
				sprintf(mode_buf, "%s", (" 11b"));
			else if (staInfoList[i].network& BAND_11A)
				sprintf(mode_buf, "%s", (" 11a"));	
			else
				sprintf(mode_buf, "%s", (" ---"));
			hr = staInfoList[i].link_time / 3600;
			min = (staInfoList[i].link_time % 3600) / 60;
			sec = staInfoList[i].link_time - hr * 3600 - min * 60;
			
			rtk_printf("aid=%x,flag=%x txOperaRates=%x ht_info=%x\n",staInfoList[i].aid,staInfoList[i].flag,staInfoList[i].txOperaRates,
				staInfoList[i].ht_info);
			rtk_printf("%s ",wl_ether_etoa((struct ether_addr*)&(staInfoList[i].addr)));
			
			rtk_printf("%-8s%-12lu%-12lu%-15s   %02d:%02d:%02d\n",
				mode_buf,staInfoList[i].tx_packets, staInfoList[i].rx_packets,\
				((staInfoList[i].flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
				hr, min, sec);
			
		}
	}
	rtk_printf("%s:%d\n",__FUNCTION__,__LINE__);
}

int
wl_ioctl(char *name, int cmd, void *buf, int len)
{
	char *buff=(char*)buf;
	int retval=0;
	//rtk_printf("%s:%d\n",__FUNCTION__,__LINE__);
	switch(cmd)
	{
		case WLC_GET_VAR:
			//rtk_printf("%s:%d name=%s\n",__FUNCTION__,__LINE__,name);
			if(strcmp(buff,"rtk_sta_info")==0)
			{
				//get all sta info
				//rtk_printf("%s:%d\n",__FUNCTION__,__LINE__);
				if(getWlStaInfo(name,(WLAN_STA_INFO_Tp)buf)<0)
				{
					rtk_printf("%s getWlStaInfo fail!\n",name);
					return -1;
				}
				//dump_sta_info((WLAN_STA_INFO_Tp)buf);
				//rtk_printf("%s:%d\n",__FUNCTION__,__LINE__);
			}
			else if(strcmp(buff,"sta_info")==0)
			{
				WLAN_STA_INFO_T rtk_sta_info={0};
				sta_info_t *sta = (sta_info_t *)buf;
				retval=rtk_getStaInfo(name,(char*)((char*)buf+strlen((char*)buf) + 1),&rtk_sta_info);
				if(retval<0)
				{
					rtk_printf("%s rtk_getStaInfo fail!\n",name);
					return -1;
				}
				if(retval==1)//find
					sta->flags|=WL_STA_AUTHO;
				else //not find
					sta->flags&=(~WL_STA_AUTHO);
				
			}
			else if(strcmp(buff,"authe_sta_list")==0)
			{
				if(rtk_getAuthMacList(name,buf)<0)
				{
					rtk_printf("%s rtk_getAuthMacList fail!\n",name);
					return -1;
				}				
			}
			else if(strcmp(buff,"autho_sta_list")==0)
			{
				if(rtk_getAuthMacList(name,buf)<0)
				{
					rtk_printf("%s rtk_getAuthMacList fail!\n",name);
					return -1;
				}	
			}
			else if (strcmp(buff, "sta_txrx_bytes") == 0)
			{
				char mac[6] = {0};
				memcpy(mac, (char *)buf+strlen(buf)+1, ETHER_ADDR_LEN);
				WLAN_STA_INFO_T rtk_sta_info={0};
				retval=rtk_getStaInfo(name, mac,&rtk_sta_info);
				*((unsigned long long *)buf) = rtk_sta_info.tx_bytes + rtk_sta_info.rx_bytes;
			}
			else
			{
				rtk_printf("invalid command!cmd=%d,buf=%s\n",cmd,buff);\
				return -1;
			}
			break;
		case WLC_GET_ASSOCLIST:
			if(rtk_getAuthMacList(name,buf)<0)
			{
				rtk_printf("%s rtk_getAuthMacList fail!\n",name);
				return -1;
			}	
			break;
		case WLC_GET_RSSI:
		{
			
			WLAN_STA_INFO_T rtk_sta_info={0};
			
			scb_val_t* pscb_val=buf;
			
			retval=rtk_getStaInfo(name,(char*)&(pscb_val->ea),&rtk_sta_info);
			
			if(retval<=0)
			{
				rtk_printf("%s rtk_getStaInfo fail!%d\n",name,retval);
				return -1;
			}
			pscb_val->val=rtk_sta_info.rssi;
			
			break;
		}
		case WLC_GET_BSSID:
		{
			bss_info bss={0};
			if(getWlBssInfo(name,&bss)<0)
			{
				rtk_printf("%s getWlBssInfo fail!\n",name);
				return -1;
			}
			memcpy(buf,bss.bssid,sizeof(bss.bssid));
			break;
		}
		case WLC_GET_BSS_INFO:
		{
			bss_info bss={0};
			if(getWlBssInfo(name,&bss)<0)
			{
				rtk_printf("%s getWlBssInfo fail!\n",name);
				return -1;
			}
			memcpy(buf,&bss,sizeof(bss));			
			break;
		}
		case WLC_SCAN :
		{
			int status=0;
			if(getWlSiteSurveyRequest(name,&status)<0)
			{
				rtk_printf("%s getWlSiteSurveyRequest fail!\n",name);
				return -1;
			}
			//cprintf("%s:%d status=%d\n",__FUNCTION__,__LINE__,status);
			if(status!=0)
				return -1;
			break;
		}
		case WLC_SCAN_RESULTS:
		{
			SS_STATUS_T scan_status={0};
			apinf_t *apinfos=buf;
			int i=0,wait=0;
			while(wait++<15)
			{
				scan_status.number = 0;
				if(getWlSiteSurveyResult(name,&scan_status)<0)
				{
					rtk_printf("%s getWlSiteSurveyRequest fail!\n",name);
					return -1;
				}
				//list->count=scan_status.number;
				//cprintf("%s:%d scan_status.number=%d\n",__FUNCTION__,__LINE__,scan_status.number);
				if(scan_status.number==0xff)
				{
					sleep(1);
					continue;
				}
				break;
			}
			if(scan_status.number==0xff)
			{
				rtk_printf("%s getWlSiteSurveyRequest fail!\n",name);
				return -1;
			}
			if(scan_status.number>APINFO_MAX)
				scan_status.number=APINFO_MAX;
			bzero(apinfos,sizeof(apinf_t)*APINFO_MAX);
			for(i=0;i<scan_status.number;i++)
			{
				rtklog("%s:%d bdBssId=%02x:%02x:%02x:%02x:%02x:%02x\n",__FUNCTION__,__LINE__,
					scan_status.bssdb[i].bdBssId[0],
					scan_status.bssdb[i].bdBssId[1],
					scan_status.bssdb[i].bdBssId[2],
					scan_status.bssdb[i].bdBssId[3],
					scan_status.bssdb[i].bdBssId[4],
					scan_status.bssdb[i].bdBssId[5]);
				
				sprintf(apinfos[i].BSSID,"%02x:%02x:%02x:%02x:%02x:%02x",
					scan_status.bssdb[i].bdBssId[0],
					scan_status.bssdb[i].bdBssId[1],
					scan_status.bssdb[i].bdBssId[2],
					scan_status.bssdb[i].bdBssId[3],
					scan_status.bssdb[i].bdBssId[4],
					scan_status.bssdb[i].bdBssId[5]);
				memcpy(apinfos[i].SSID,scan_status.bssdb[i].bdSsIdBuf,SSID_LEN);
				//apinfos[i].SSID_len=strlen(list->bss_info[i].SSID);
				apinfos[i].ctl_ch=scan_status.bssdb[i].ChannelNumber;	
				apinfos[i].channel=scan_status.bssdb[i].ChannelNumber;
				//apinfos[i].length = sizeof(wl_bss_info_t);
				apinfos[i].RSSI_Quality=scan_status.bssdb[i].rssi;
				if ((scan_status.bssdb[i].bdCap & cPrivacy) == 0)
				{
					apinfos[i].wep=0;
					apinfos[i].wpa=0;
				}
				else {
					if (scan_status.bssdb[i].bdTstamp[0] == 0)						
						apinfos[i].wep=1;					
					else {
						int wpa_exist = 0, idx = 0;
						apinfos[i].wpa=1;
						if (scan_status.bssdb[i].bdTstamp[0] & 0x0000ffff) {
							
							if (((scan_status.bssdb[i].bdTstamp[0] & 0x0000f000) >> 12) == 0x4)
								apinfos[i].wid.key_mgmt=WPA_KEY_MGMT_PSK_;
							else if(((scan_status.bssdb[i].bdTstamp[0] & 0x0000f000) >> 12) == 0x2)
								apinfos[i].wid.key_mgmt=WPA_KEY_MGMT_IEEE8021X_;
							

							if (((scan_status.bssdb[i].bdTstamp[0] & 0x00000f00) >> 8) == 0x5)
								apinfos[i].wid.pairwise_cipher=(WPA_CIPHER_TKIP_|WPA_CIPHER_CCMP_);
							else if (((scan_status.bssdb[i].bdTstamp[0] & 0x00000f00) >> 8) == 0x4)
								apinfos[i].wid.pairwise_cipher=(WPA_CIPHER_CCMP_);
							else if (((scan_status.bssdb[i].bdTstamp[0] & 0x00000f00) >> 8) == 0x1)
								apinfos[i].wid.pairwise_cipher=(WPA_CIPHER_TKIP_);
						}
						if (scan_status.bssdb[i].bdTstamp[0] & 0xffff0000) {
							
							if (((scan_status.bssdb[i].bdTstamp[0] & 0xf0000000) >> 28) == 0x4)
								apinfos[i].wid.key_mgmt=WPA_KEY_MGMT_PSK2_;
							else if (((scan_status.bssdb[i].bdTstamp[0] & 0xf0000000) >> 28) == 0x2)
								apinfos[i].wid.key_mgmt=WPA_KEY_MGMT_IEEE8021X2_;
							if (((scan_status.bssdb[i].bdTstamp[0] & 0x0f000000) >> 24) == 0x5)
								apinfos[i].wid.pairwise_cipher=(WPA_CIPHER_TKIP_|WPA_CIPHER_CCMP_);
							else if (((scan_status.bssdb[i].bdTstamp[0] & 0x0f000000) >> 24) == 0x4)
								apinfos[i].wid.pairwise_cipher=(WPA_CIPHER_CCMP_);
							else if (((scan_status.bssdb[i].bdTstamp[0] & 0x0f000000) >> 24) == 0x1)
								apinfos[i].wid.pairwise_cipher=(WPA_CIPHER_TKIP_);
						}
					}
				}

				if ((scan_status.bssdb[i].network & (BAND_11G|BAND_11B | BAND_11N))==(BAND_11G|BAND_11B | BAND_11N))
					apinfos[i].NetworkType=Ndis802_11OFDM24_N;
				else if ((scan_status.bssdb[i].network & (BAND_11G|BAND_11B))==(BAND_11G|BAND_11B))
					apinfos[i].NetworkType=Ndis802_11OFDM24;
				else if(scan_status.bssdb[i].network & (BAND_5G_11AC))
					apinfos[i].NetworkType=Ndis802_11OFDM5_VHT;
				else if((scan_status.bssdb[i].network & (BAND_11A | BAND_11N))==(BAND_11A | BAND_11N))
					apinfos[i].NetworkType=Ndis802_11OFDM5_N;				
				else if (scan_status.bssdb[i].network&BAND_11B)
					apinfos[i].NetworkType=Ndis802_11DS;				
				else if(scan_status.bssdb[i].network & BAND_11A)
					apinfos[i].NetworkType=Ndis802_11OFDM5;						
				else
					apinfos[i].NetworkType=Ndis802_11NetworkTypeMax;	
				
				//cprintf("%s:%d ssid=%s scan_status.bssdb[i].network=%d apinfos[i].NetworkType=%d\n",__FUNCTION__,__LINE__,apinfos[i].SSID,scan_status.bssdb[i].network,apinfos[i].NetworkType);
			}
			
			break;
		}
		case WLC_GET_VALID_CHANNELS:
		{
			char tmp[256], prefix[] = "wlXXXXXXXXXX_";
			char*ifname;
			
			unsigned char countryCode[3]={0};
			char chList[256]={0};
			snprintf(prefix, sizeof(prefix), "wl%d_", 0);
			ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			if(strcmp(ifname,name)==0)
			{
				strncpy(countryCode, nvram_safe_get(strcat_r(prefix, "country_code", tmp)), 2);
				if(!countryCode[0])
				{
					strcpy(countryCode,DEF_COUNTRY_CODE);
					nvram_set(strcat_r(prefix, "country_code", tmp),DEF_COUNTRY_CODE);
				}
				
				if(rtk_web_get_channel_list(countryCode,(wl_uint32_list_t *)buf,WLAN_2G)!=0)
				{
					return -1;
				}
			}
			snprintf(prefix, sizeof(prefix), "wl%d_", 1);
			ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			if(strcmp(ifname,name)==0)
			{
				strncpy(countryCode, nvram_safe_get(strcat_r(prefix, "country_code", tmp)), 2);
				if(!countryCode[0])
					strcpy(countryCode,"CN");
				if(rtk_web_get_channel_list(countryCode,(wl_uint32_list_t *)buf,WLAN_5G)!=0)
				{
					return -1;
				}
			}
			break;
		}
		case WLC_GET_INSTANCE:
		{
			int *unit_cur=buf;
			//cprintf("%s:%d wlc_band=%s name=%s\n",__FUNCTION__,__LINE__,nvram_safe_get("wlc_band"),name);
			(*unit_cur)= atoi(nvram_safe_get("wlc_band"));
			//cprintf("%s:%d unit_cur=%d %p\n",__FUNCTION__,__LINE__,*unit_cur,unit_cur);
			break;
		}
		case WLC_GET_RATE:
		{
			int * retv=buf;
			char txrate[64]={0};
			int rateid=0,i=0;
			
			WLAN_STA_INFO_T wlan_sta[MAX_STA_NUM+1]= {0};
			WLAN_STA_INFO_Tp pInfo=wlan_sta;
		    rtk_printf("%s:%d name=%s\n",__FUNCTION__,__LINE__,name);
			if(getWlStaInfo(name,wlan_sta)<0)
			{
				rtk_printf("%s getWlStaInfo fail!\n",name);
				return -1;
			}
			
			//dump_sta_info(wlan_sta);
			for(i=0;i<MAX_STA_NUM+1;i++)
			{
				if(wlan_sta[i].aid && (wlan_sta[i].flag & STA_INFO_FLAG_ASOC))
				{
					pInfo=&wlan_sta[i];
					break;
				}
			}
			
			rtk_printf("%s:%d i=%d txOperaRates=%x\n",__FUNCTION__,__LINE__,i,pInfo->txOperaRates);
			if(pInfo->txOperaRates >= 0xA0) {
				//sprintf(txrate, "%d", pInfo->acTxOperaRate); 
				set_11ac_txrate(pInfo, txrate);
			} else if((pInfo->txOperaRates & 0x80) != 0x80){	
				if(pInfo->txOperaRates%2){
					sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5"); 
				}else{
					sprintf(txrate, "%d",pInfo->txOperaRates/2); 
				}
			}else{
				if((pInfo->ht_info & 0x1)==0){ //20M
					if((pInfo->ht_info & 0x2)==0){//long
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
								break;
							}
						}
					}else if((pInfo->ht_info & 0x2)==0x2){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}else if((pInfo->ht_info & 0x1)==0x1){//40M
					if((pInfo->ht_info & 0x2)==0){//long
						
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
								break;
							}
						}
					}else if((pInfo->ht_info & 0x2)==0x2){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}
				
			}
			 rtk_printf("%s:%d txrate=%s\n",__FUNCTION__,__LINE__,txrate);
			(*retv)=atoi(txrate);
			rtk_printf("%s:%d retv=%d\n",__FUNCTION__,__LINE__,*retv);
			break;
		}
		case WLC_GET_MACLIST:
		{
			int i=0;
			struct maclist *maclist = buf;
			WLAN_ACL_CLIENT_LIST_T acl_client_list;
			WLAN_ACL_CLIENT_LIST_Tp p_acl_client_list = &acl_client_list;
			memset(p_acl_client_list, 0, sizeof(WLAN_ACL_CLIENT_LIST_T));

			if (getAclList(name, p_acl_client_list) < 0) {
				rtk_printf("%s getAclList fail!\n", name);
			}
			maclist->count = p_acl_client_list->count;
			for (i=0; i<p_acl_client_list->count; i++) {
				rtk_printf("%s:%d MAC: %02x %02x %02x %02x %02x %02x\n", __FUNCTION__, __LINE__
					p_acl_client_list->macAddr[i][0],
					p_acl_client_list->macAddr[i][1],
					p_acl_client_list->macAddr[i][2],
					p_acl_client_list->macAddr[i][3],
					p_acl_client_list->macAddr[i][4],
					p_acl_client_list->macAddr[i][5]);
				memcpy(&(maclist->ea[i]), p_acl_client_list->macAddr[i], MAC_ADDR_LEN);
			}
			break;
		}
		case WLC_GET_MACMODE:
		{
			WLAN_ACL_INFO_T acl_info;
			WLAN_ACL_INFO_Tp p_acl_info = &acl_info;
			memset(p_acl_info, 0, sizeof(WLAN_ACL_INFO_T));

			if (getAclInfo(name, p_acl_info) < 0)
			{
				rtk_printf("%s getAclInfo fail!\n", name);
				return -1;
			}
			*((int *)buf) = acl_info.mode;
			break;
		}
		case WLC_SET_MACMODE:
			break;
		case WLC_GET_RADIO:
			break;
		case WLC_GET_SSID:
			break;
		case WLC_GET_MONITOR_STA_RSSI:
		{
			if (getMonitorStaRssi(name, buff, len) < 0)
			{
				rtk_printf("%s getMonitorStaRssi fail!\n", name);
				return -1;
			}
			break;
		}
		default:
			rtk_printf("invalid command!cmd=%d\n",cmd);
			return -1;
	}
	return 0;
}

/**
 * Generate interface name based on @band and @subunit. (@subunit is NOT y in wlX.Y)
 * @band:
 * @subunit:
 * @buf:
 * @return:
 */
char *__get_wlifname(int band, int subunit, char *buf)
{
	if (!buf)
		return buf;

	strcpy(buf, (!band) ? WIF_2G : WIF_5G);
	if (subunit) {
		sprintf(buf + strlen(buf), ".%d", subunit);
	}

	return buf;
}

/**
 * Generate VAP interface name of wlX.Y for Guest network, Free Wi-Fi, and Facebook Wi-Fi
 * @x:		X of wlX.Y, aka unit
 * @y:		Y of wlX.Y
 * @buf:	Pointer to buffer of VAP interface name. Must greater than or equal to IFNAMSIZ
 * @return:
 * 	NULL	Invalid @buf
 * 	""	Invalid parameters
 *  otherwise	VAP interface name of wlX.Y
 */
char *get_wlxy_ifname(int x, int y, char *buf)
{
	int i, sidx;
	char prefix[sizeof("wlX.Yxxx")];

	if (!buf)
		return buf;

	if (x < 0 || y < 0 || y >= MAX_NO_MSSID)
		return "";

	if (y == 0) {
		__get_wlifname(x, 0, buf);
		return buf;
	}

	*buf = '\0';
	for (i = 1, sidx = 1; i < MAX_NO_MSSID; ++i) {
		if (i == y) {
			__get_wlifname(x, sidx, buf);
			break;
		}

		snprintf(prefix, sizeof(prefix), "wl%d.%d_", x, i);
		if (nvram_pf_match(prefix, "bss_enabled", "1"))
			sidx++;
	}

	return buf;
}

int psta_exist_except(int unit)
{
	//TODO
	return 0;
}

char *get_lan_hwaddr(void)
{
	/* TODO: handle exceptional model */
	return nvram_safe_get("et0macaddr");
}

char *get_wan_hwaddr(void)
{
	/* TODO: handle exceptional model */
        return nvram_safe_get("et1macaddr");
}

char *get_2g_hwaddr(void)
{
        return get_wan_hwaddr();
}

char *get_label_mac()
{
	return get_2g_hwaddr();
}

int get_channel_list_via_country(int unit, const char *country_code, char *buffer, int len)
{
	//TODO
	return -1;
}

int get_channel_list_via_driver(int unit, char *buffer, int len)
{
	//TODO
	return -1;
}

char *get_wififname(int band)
{
	const char *wif[] = { WIF_2G, WIF_5G };
	if (band < 0 || band >= ARRAY_SIZE(wif)) {
		rtk_printf("%s: Invalid wl%d band!\n", __func__, band);
		band = 0;
	}
	return (char*) wif[band];
}
