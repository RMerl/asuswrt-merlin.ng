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
 */
#include <stdio.h>
#include <string.h>
#include <bcmnvram.h>
#include <net/if_arp.h>
#include <shutils.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <etioctl.h>
#include <rc.h>
typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>
#if defined(RTCONFIG_REALTEK)
#include "../../../shared/sysdeps/realtek/realtek.h"
//TODO
#include "../../../../../realtek/rtl819x/linux-3.10/drivers/net/wireless/rtl8192cd/ieee802_mib.h"
#include "mib_adapter/rtk_wifi_drvmib.h"
#include "../../../../../realtek/rtl819x/linux-3.10/include/generated/autoconf.h"
#else
#include <ctype.h>
#endif
#include <wlutils.h>
#include <shared.h>

extern void set_led(int wl0_stage, int wl1_stage);

static char *ATE_REALTEK_FACTORY_MODE_STR()       // return "ATEMODE"
{
	char atemode[8];
	sprintf(atemode, "%s%s%s%s%s%s%s%s", "A", "T", "E", "M", "O", "D", "E", "\0");
	return strdup(atemode);
}

int IS_ATE_FACTORY_MODE(void)
{
	char *mode_str;
	int ret;
	mode_str = ATE_REALTEK_FACTORY_MODE_STR();
	ret = strcmp(nvram_safe_get(mode_str), "1");
	free(mode_str);
	return (ret == 0);
}

void platform_start_ate_mode(void)
{
}

void ate_commit_bootlog(char *err_code)
{
//	int ate_ret_offset = HW_SETTING_OFFSET;

	nvram_set("Ate_power_on_off_enable", err_code);
	nvram_commit();

#if 0	/* don't need the below now */
	/* using the last byte of territoryCode to save ate ret */
	ate_ret_offset += sizeof(PARAM_HEADER_T);
	ate_ret_offset += (int)(&((struct hw_setting *)0)->modelName);
	ate_ret_offset -= 1;

	rtk_flash_write(err_code, ate_ret_offset, 1);
#endif
}

#if defined(RTCONFIG_CONCURRENTREPEATER)
#if defined(RPAC53) || defined(RPAC55)
int set_off_led(led_state_t *led)
{
#if  defined(RPAC53)
	switch (led->id) {
	case LED_POWER:
		led_control(LED_POWER_RED, LED_OFF);
		led_control(LED_POWER, LED_OFF);
		break;
	case LED_2G:
		led_control(LED_2G_ORANGE, LED_OFF);
		led_control(LED_2G_GREEN, LED_OFF);
		led_control(LED_2G_RED, LED_OFF);
		break;
	case LED_5G:
		led_control(LED_5G_ORANGE, LED_OFF);
		led_control(LED_5G_GREEN, LED_OFF);
		led_control(LED_5G_RED, LED_OFF);
		break;
	case LED_LAN:
		led_control(LED_LAN, LED_OFF);
		break;
	default:
		dbG("Not support the LED ID:%d\n", led->id);
	}
#elif defined(RPAC55)
	switch (led->id) {
	case LED_POWER:
		led_control(LED_POWER, LED_OFF);
		led_control(LED_POWER_RED, LED_OFF);
		break;
	case LED_WIFI:
		led_control(LED_WIFI, LED_OFF);
		break;
	case LED_SIG1:
		led_control(LED_SIG1, LED_OFF);
		break;
	case LED_SIG2:
		led_control(LED_SIG2, LED_OFF);
		break;
	default:
		dbG("Not support the LED ID:%d\n", led->id);
	}
#endif
	led->state = LED_OFF;
	return 0;
}

int set_on_led(led_state_t *led)
{
#if defined(RPAC53)
	switch (led->id) {
	case LED_POWER:
		if (led->color == LED_GREEN)
			led_control(LED_POWER, LED_ON);
		else if (led->color == LED_RED)
			led_control(LED_POWER_RED, LED_ON);
		break;
	case LED_2G:
		if (led->color == LED_RED)
			led_control(LED_2G_RED, LED_ON);
		else if (led->color == LED_GREEN)
			led_control(LED_2G_GREEN, LED_ON);
		else if (led->color == LED_ORANGE)
			led_control(LED_2G_ORANGE, LED_ON);
		break;
	case LED_5G:
		if (led->color == LED_RED)
			led_control(LED_5G_RED, LED_ON);
		else if (led->color == LED_GREEN)
			led_control(LED_5G_GREEN, LED_ON);
		else if (led->color == LED_ORANGE)
			led_control(LED_5G_ORANGE, LED_ON);
		break;
	case LED_LAN:
		led_control(LED_LAN, LED_ON);
		break;
	default:
		dbG("Not support the LED ID:%d\n", led->id);
	}
#elif defined(RPAC55)
	switch (led->id) {
	case LED_POWER:
		if (led->color == LED_BLUE)
			led_control(LED_POWER, LED_ON);
		else if (led->color == LED_RED)
			led_control(LED_POWER_RED, LED_ON);
		break;
	case LED_WIFI:
		led_control(LED_WIFI, LED_ON);
		break;
	case LED_SIG1:
		led_control(LED_SIG1, LED_ON);
		break;
	case LED_SIG2:
		led_control(LED_SIG2, LED_ON);
		break;
	default:
		dbG("Not support the LED ID:%d\n", led->id);
	}
#endif
	led->state = LED_ON;
	return 0;
}
void update_gpiomode(int gpio, int mode)
{
	char path[PATH_MAX], val_str[64];
 
	sprintf(val_str, "gpiomode %d %d", gpio, mode);
	sprintf(path, "/proc/asus_ate");
	f_write_string(path, val_str, 0, 0);
}
#endif
#endif

int setAllLedOn(void)
{	
	rtklog("%s\n",__FUNCTION__);
#if defined(RPAC68U)
	set_led(LED_ON_ALL, LED_ON_ALL);
#elif defined(RPAC53)
	led_control(LED_POWER, LED_ON);
	led_control(LED_WAN, LED_ON);
	led_control(LED_LAN, LED_ON);
	led_control(LED_USB, LED_ON);

	update_gpiomode(14, 1);
	led_control(LED_POWER_RED, LED_ON);
	led_control(LED_2G_ORANGE, LED_ON);
	led_control(LED_2G_GREEN, LED_ON);
	led_control(LED_2G_RED, LED_ON);
	led_control(LED_5G_ORANGE, LED_ON);
	led_control(LED_5G_GREEN, LED_ON);
	led_control(LED_5G_RED, LED_ON);
#elif defined(RPAC55)
	led_control(LED_POWER, LED_ON);
	led_control(LED_POWER_RED, LED_ON);
	led_control(LED_WIFI, LED_ON);
	led_control(LED_SIG1, LED_ON);
	led_control(LED_SIG2, LED_ON);
#endif
	puts("1");
	return 0;
}

int setAllLedOff(void)
{
	rtklog("%s\n",__FUNCTION__);
#if defined(RPAC68U)
	set_led(LED_OFF_ALL, LED_OFF_ALL);
#elif defined(RPAC53)
	led_control(LED_POWER, LED_OFF);
	led_control(LED_WAN, LED_OFF);
	led_control(LED_LAN, LED_OFF);
	led_control(LED_USB, LED_OFF);

	update_gpiomode(14, 1);
	led_control(LED_POWER_RED, LED_OFF);
	led_control(LED_2G_ORANGE, LED_OFF);
	led_control(LED_2G_GREEN, LED_OFF);
	led_control(LED_2G_RED, LED_OFF);
	led_control(LED_5G_ORANGE, LED_OFF);
	led_control(LED_5G_GREEN, LED_OFF);
	led_control(LED_5G_RED, LED_OFF);
#elif defined(RPAC55)
	led_control(LED_POWER, LED_OFF);
	led_control(LED_POWER_RED, LED_OFF);
	led_control(LED_WIFI, LED_OFF);
	led_control(LED_SIG1, LED_OFF);
	led_control(LED_SIG2, LED_OFF);
#endif
	puts("1");
	return 0;
}

#ifdef RPAC53
int setAllGreenLedOn(void)
{	
	rtklog("%s\n",__FUNCTION__);

	led_control(LED_POWER, LED_ON);
	led_control(LED_LAN, LED_ON);
	update_gpiomode(14, 1);

	led_control(LED_2G_GREEN, LED_ON);
	led_control(LED_5G_GREEN, LED_ON);
	
	puts("1");
	return 0;
}

int setAllOrangeLedOn(void)
{	
	rtklog("%s\n",__FUNCTION__);

	led_control(LED_2G_ORANGE, LED_ON);
	led_control(LED_5G_ORANGE, LED_ON);

	puts("1");
	return 0;
}
#endif
#if defined(RPAC53) || defined(RPAC55)
int setAllRedLedOn(void)
{	
	rtklog("%s\n",__FUNCTION__);

	/* Turn off other lights.*/
#if defined(RPAC55)
	led_control(LED_POWER, LED_OFF);
	led_control(LED_WIFI, LED_OFF);
	led_control(LED_SIG1, LED_OFF);
	led_control(LED_SIG2, LED_OFF);
#endif

	led_control(LED_POWER_RED, LED_ON);
#if defined(RPAC53)
	led_control(LED_2G_RED, LED_ON);
	led_control(LED_5G_RED, LED_ON);
#endif

	puts("1");
	return 0;
}
#endif
#ifdef RPAC55
int setAllBlueLedOn(void)
{	
	rtklog("%s\n",__FUNCTION__);

	/* Turn off other lights.*/
	led_control(LED_POWER_RED, LED_OFF);

	led_control(LED_POWER, LED_ON);
	led_control(LED_WIFI, LED_ON);
	led_control(LED_SIG1, LED_ON);
	led_control(LED_SIG2, LED_ON);

	puts("1");
	return 0;
}
#endif

int setMAC_2G(const char *mac)
{
	rtklog("%s\n",__FUNCTION__);
	char ea[ETHER_ADDR_LEN];
	int offset = HW_SETTING_OFFSET;
	int offset_nic0 = HW_SETTING_OFFSET;
	if (mac==NULL || !isValidMacAddr(mac))
		return 0;

	if (!IS_ATE_FACTORY_MODE())
		return 0;

	if (ether_atoe(mac, ea))
	{
		offset += sizeof(PARAM_HEADER_T);
		offset += (int)(&((struct hw_setting *)0)->wlan);
		offset += sizeof(struct hw_wlan_setting);
		offset += (int)(&((struct hw_wlan_setting *)0)->macAddr);
		rtk_flash_write(ea,offset,6);

		/* Set NIC0 MAC address */
		offset_nic0 += sizeof(PARAM_HEADER_T);
		offset_nic0 += (int)(&((struct hw_setting *)0)->nic0Addr);
		rtk_flash_write(ea, offset_nic0, 6);

		/* Set Guest Network MAC address */
		offset += 6; // 1st Guest Network MAC address
		ea[5] += 1;
		rtk_flash_write(ea, offset, 6);
		offset += 6; // 2nd Guest Network MAC address
		ea[5] += 1;
		rtk_flash_write(ea, offset, 6);
		offset += 6; // 3rd Guest Network MAC address
		ea[5] += 1;
		rtk_flash_write(ea, offset, 6);

		getMAC_2G();
	}
	return 1;
}

int setMAC_5G(const char *mac)
{
	rtklog("%s\n",__FUNCTION__);
	char ea[ETHER_ADDR_LEN];
	int offset = HW_SETTING_OFFSET;
	int offset_nic1 = HW_SETTING_OFFSET;

	if (mac==NULL || !isValidMacAddr(mac))
		return 0;

	if (!IS_ATE_FACTORY_MODE())
		return 0;

	if (ether_atoe(mac, ea))
	{
		offset += sizeof(PARAM_HEADER_T);
		offset += (int)(&((struct hw_setting *)0)->wlan);
		offset += (int)(&((struct hw_wlan_setting *)0)->macAddr);
		rtk_flash_write(ea,offset,6);

		/* Set NIC1 MAC address */
		offset_nic1 += sizeof(PARAM_HEADER_T);
		offset_nic1 += (int)(&((struct hw_setting *)0)->nic1Addr);
		rtk_flash_write(ea, offset_nic1, 6);

		/* Set Guest Network MAC address */
		offset += 6; // 1st Guest Network MAC address
		ea[5] += 1;
		rtk_flash_write(ea, offset, 6);
		offset += 6; // 2nd Guest Network MAC address
		ea[5] += 1;
		rtk_flash_write(ea, offset, 6);
		offset += 6; // 3rd Guest Network MAC address
		ea[5] += 1;
		rtk_flash_write(ea, offset, 6);

		getMAC_5G();
	}
	return 1;
}

#ifdef RPAC55
int setMAC_BT(const char *mac)
{
	rtklog("%s\n",__FUNCTION__);
	char ea[ETHER_ADDR_LEN];
	int offset = BLUETOOTH_HW_SETTING_OFFSET;
	if (mac==NULL || !isValidMacAddr(mac))
		return 0;

	if (!IS_ATE_FACTORY_MODE())
		return 0;

	if (ether_atoe(mac, ea))
	{
		offset += sizeof(PARAM_HEADER_T);
		offset += (int)(&(((BLUETOOTH_HW_SETTING_T *)0)->btAddr));
		rtk_flash_write(ea,offset,6);

		getMAC_BT();
	}
	return 1;
}
int getMAC_BT(const char *mac)
{
	rtklog("%s\n",__FUNCTION__);
	unsigned char buffer[6];
	char macaddr[18];
	int offset = BLUETOOTH_HW_SETTING_OFFSET;
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));

	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&(((BLUETOOTH_HW_SETTING_T *)0)->btAddr));
	rtk_flash_read(buffer,offset,6);

	ether_etoa(buffer, macaddr);
	puts(macaddr);
}
#endif

int setCountryCode_2G(const char *cc)
{
	int rd_offset_2g = HW_SETTING_OFFSET;
	int rd_offset_5g = HW_SETTING_OFFSET;
	int cc_offset = HW_SETTING_OFFSET;
	int i = 0, num = sizeof(reg_domain)/sizeof(reg_domain_t);
	unsigned char code_2g = 0;
	unsigned char code_5g = 0;

	if (cc == NULL)
		return 0;

	if (!IS_ATE_FACTORY_MODE())
		return 0;

	while (i < num) {
		if (strcmp(cc, reg_domain[i].name) == 0) {
			code_2g = reg_domain[i].band_2G;
			code_5g = reg_domain[i].band_5G;
			break;
		}
		i++;
	}

	if ((code_2g < DOMAIN_FCC || code_2g >= DOMAIN_MAX) || (code_5g < DOMAIN_FCC || code_5g >= DOMAIN_MAX))
		return 0;

	/* write country code */
	cc_offset += sizeof(PARAM_HEADER_T);
	cc_offset += (int)(&((struct hw_setting *)0)->countryCode);	
	rtk_flash_write(cc, cc_offset, 2);

	/* write regDomain for 2G */
	rd_offset_2g += sizeof(PARAM_HEADER_T);
	rd_offset_2g += (int)(&((struct hw_setting *)0)->wlan);
	rd_offset_2g += sizeof(struct hw_wlan_setting);
	rd_offset_2g += (int)(&((struct hw_wlan_setting *)0)->regDomain);	
	rtk_flash_write(&code_2g, rd_offset_2g, 1);
	nvram_set("wl0_country_code", cc);
	
	/* write regDomain for 5G */
	rd_offset_5g += sizeof(PARAM_HEADER_T);
	rd_offset_5g += (int)(&((struct hw_setting *)0)->wlan);
	rd_offset_5g += (int)(&((struct hw_wlan_setting *)0)->regDomain);
	rtk_flash_write(&code_5g, rd_offset_5g, 1);
	nvram_set("wl1_country_code", cc);

	puts(cc);
	return 1;
}

int setCountryCode_5G(const char *cc)
{
	return 1;
}

int setTerritoryCode(const char *tcode)
{
	unsigned char tc_buf[5];
	int tc_offset = HW_SETTING_OFFSET;

	memset(tc_buf, 0, sizeof(tc_buf));
	tc_offset += sizeof(PARAM_HEADER_T);
	tc_offset += (int)(&((struct hw_setting *)0)->territoryCode);

	/* special case
	 * if tcode == "FFFFF", Write FF, FF, FF, FF, FF to OFFSET_TERRITORY_CODE
	 */
	if (!strcmp(tcode, "FFFFF")) {
		memset(tc_buf, 0x0, sizeof(tc_buf));
		rtk_flash_write(tc_buf, tc_offset, 5);
		nvram_unset("territory_code");

		return 0;
	}

	/* [A-Z][A-Z]/[0-9][0-9] */
	if (tcode[2] != '/' ||
	    !isupper(tcode[0]) || !isupper(tcode[1]) ||
	    !isdigit(tcode[3]) || !isdigit(tcode[4]))
	{
		return -1; //only check 5 bytes??
	}

	if (!IS_ATE_FACTORY_MODE())
		return -1;

	rtk_flash_write(tcode, tc_offset, 5);
	nvram_set("territory_code", tcode);
	
	return 0;
}

int getTerritoryCode()
{
	//puts(nvram_safe_get("territory_code"));
	//return 0;
	unsigned char tc_buf[6];
	int tc_offset = HW_SETTING_OFFSET;

	memset(tc_buf, 0, sizeof(tc_buf));
	tc_offset += sizeof(PARAM_HEADER_T);
	tc_offset += (int)(&((struct hw_setting *)0)->territoryCode);

	rtk_flash_read(tc_buf, tc_offset, 5);
	if ((unsigned char)tc_buf[0] != 0x0)
		puts(tc_buf);
	return 0;
}

int setUsb3p0Enable()
{
	usb3_enable(1);
	nvram_set("usb_usb3", "1");
	puts("1");
	return 0;
}

int setUsb3p0Disable()
{
	usb3_enable(0);
	nvram_set("usb_usb3", "0");
	puts("1");
	return 0;
}

int setSN(const char *SN)
{
}

int setMN(const char *MN)
{
	char modelname[16];
	int mn_offset = HW_SETTING_OFFSET;

	mn_offset += sizeof(PARAM_HEADER_T);
	mn_offset += (int)(&((struct hw_setting *)0)->modelName);

	if(MN==NULL || !is_valid_hostname(MN))
		return 0;

	if (!IS_ATE_FACTORY_MODE())
		return 0;

	memset(modelname, 0, sizeof(modelname));
	strncpy(modelname, MN, sizeof(modelname) -1);
	rtk_flash_write(modelname, mn_offset, sizeof(modelname));

	nvram_set("odmpid", modelname);
	puts(nvram_safe_get("odmpid"));
	return 1;
}

int setPIN(const char *pin)
{
	rtklog("%s\n",__FUNCTION__);
	int offset = HW_SETTING_OFFSET;

	if (!IS_ATE_FACTORY_MODE())
		return 0;

	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	offset += (int)(&((struct hw_wlan_setting *)0)->wscPin);
	if (pincheck(pin))
	{
		rtk_flash_write(pin,offset,8);
		offset += sizeof(struct hw_wlan_setting);
		rtk_flash_write(pin,offset,8);
		char PIN[9];
		memset(PIN, 0, 9);
		memcpy(PIN, pin, 8);
		puts(PIN);
		return 1;
	}
	return 0;	
}

int getBootVer()
{
	FILE *fp;
	char buf[32], out[64];

	system("echo 'bootver 1' > /proc/asus_ate");
	fp = popen("cat /proc/asus_ate", "r");
	if (fp) {
		fgets(buf, sizeof(buf),fp);
		pclose(fp);
	}
	sprintf(out, "%s-%s", nvram_safe_get("productid"), buf);
	puts(out);
	return 0;
}

int getMAC_2G()
{
	rtklog("%s\n",__FUNCTION__);
	unsigned char buffer[6];
	char macaddr[18];
	int offset = HW_SETTING_OFFSET;
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));

	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	offset += sizeof(struct hw_wlan_setting);
	offset += (int)(&((struct hw_wlan_setting *)0)->macAddr);
	rtk_flash_read(buffer,offset,6);

	ether_etoa(buffer, macaddr);
	puts(macaddr);
	
	return 0;
}

int getMAC_5G()
{
	rtklog("%s\n",__FUNCTION__);
	unsigned char buffer[6];
	char macaddr[18];
	int offset = HW_SETTING_OFFSET;
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));
	
	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	offset += (int)(&((struct hw_wlan_setting *)0)->macAddr);
	rtk_flash_read(buffer,offset,6);
	ether_etoa(buffer, macaddr);
	puts(macaddr);
	return 0;
}

int getCountryCode_2G()
{
	char country_code[3];
	int cc_offset = HW_SETTING_OFFSET;

	memset(country_code, 0, sizeof(country_code));
	cc_offset += sizeof(PARAM_HEADER_T);
	cc_offset += (int)(&((struct hw_setting *)0)->countryCode);

	rtk_flash_read(country_code, cc_offset, 2);

	if (country_code[0] == 0x0)	// 0x0 is default
		;
	else
		puts(country_code);
	
	return 0;
}

int getCountryCode_5G()
{
	return 0;
}

int getSN(void)
{
}

int getMN(void)
{
	puts(nvram_safe_get("odmpid"));
	return 0;
}

/** @brief Get device model name form flash.
 *
 *  @param modelname IN/OUT. Return model name to caller. 
 *
 *  @param length IN. Param modelname is length. Prevent to overflow.
 *
 *  @return 0 is normal. Others is error.
 */
int getflashMN(char *modelname, int length)
{
	int mn_offset = HW_SETTING_OFFSET;
 
	mn_offset += sizeof(PARAM_HEADER_T);
	mn_offset += (int)(&((struct hw_setting *)0)->modelName);
 
	memset(modelname, 0, length);

	if (length > 16)
		length = 16; /* hw-setting modelname size is 16. */

	rtk_flash_read(modelname, mn_offset, length);
	modelname[length - 1] = '\0';

	return 0;
}

int getPIN()
{
	rtklog("%s\n",__FUNCTION__);
	unsigned char PIN[9];
	memset(PIN, 0, sizeof(PIN));
	int offset = HW_SETTING_OFFSET;
	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	offset += (int)(&((struct hw_wlan_setting *)0)->wscPin);
	rtk_flash_read(PIN,offset,8);
	puts(PIN);
	return 0;
}

int GetPhyStatus(int verbose)
{
	FILE *fp;
	char out[64], output[64] = "";
	char *b;
	char phystatus[5][4];
	int i;

	system("echo 'physt 1' > /proc/asus_ate");
	fp = popen("cat /proc/asus_ate", "r");
	if (fp) {
		fgets(out, sizeof(out),fp);
		pclose(fp);
	}

	for (i = 0, b = strtok(out, ";"); b != NULL; b = strtok(NULL, ";"), i++)
		snprintf(phystatus[i], sizeof(phystatus[i])/sizeof(char) - 1, "%s", index(b, '=')+1);
	
#if defined(RPAC53)
	sprintf(output, "L1=%s;", phystatus[4]);
#elif defined(RPAC55)
	sprintf(output, "L1=%s;", phystatus[0]);
#else
	sprintf(output, "L1=%s;L2=%s;L3=%s;L4=%s;L5=%s;", 
			phystatus[0], phystatus[1], phystatus[2], phystatus[3], phystatus[4]);
#endif

	puts(output);

	return 1;
}

#if 1
unsigned int get_channel_list(int band, char *chList, char *countryCode)
{
	int i = 0, num = sizeof(reg_domain)/sizeof(reg_domain_t);
	unsigned char code = 0;
	struct channel_list *ch = NULL;
	char tmp[8];

	ch = ((band == WLAN_2G)?reg_channel_2_4g:reg_channel_5g_full_band);

	while (i < num) {
		if (strcmp(countryCode, reg_domain[i].name) == 0) {
			code = ((band == WLAN_2G)?reg_domain[i].band_2G:reg_domain[i].band_5G);
			break;
		}
		i++;
	}

	if (code) {
		for(i=0; i<ch[code-1].len; i++) {
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",ch[code-1].channel[i]);
			strcat(chList,tmp);
			if(i != ch[code-1].len - 1)
				strcat(chList,",");
		}
		//puts(chList);
	}

	return code;
}
#endif

int Get_ChannelList_2G(void)
{
	rtklog("%s\n",__FUNCTION__);
	unsigned char countryCode[3];
	char chList[256]={0};
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	int unit = 0;
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	memset(countryCode, 0, sizeof(countryCode));
	strncpy(countryCode, nvram_safe_get(strcat_r(prefix, "country_code", tmp)), 2);
	rtklog("countryCode:%s\n",countryCode);

#if 1
	get_channel_list(WLAN_2G, chList, countryCode);
	puts(chList);

/*
	printf("\n\n======================================\n");
	printf("chList[%s]\n", chList);
	printf("======================================\n\n\n");
*/
#elif 0
{
	int i = 0, num = sizeof(reg_domain)/sizeof(reg_domain_t);
	unsigned char code = 0;

	while (i < num) {
		if (strcmp(countryCode, reg_domain[i].name) == 0) {
			code = reg_domain[i].band_2G;
			break;
		}
		i++;
	}

	if (code) {
		for(i=0; i<reg_channel_2_4g[code-1].len; i++) {
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",reg_channel_2_4g[code-1].channel[i]);
			strcat(chList,tmp);
			if(i != reg_channel_2_4g[code-1].len - 1)
				strcat(chList,",");
		}
		puts(chList);
	}
}
#else
	if(rtk_get_channel_list_via_country(countryCode,chList,WLAN_2G)==0)
	{
		puts(chList);
	}
#endif

	return 1;
}

int Get_ChannelList_5G(void)
{
	rtklog("%s\n",__FUNCTION__);
	unsigned char countryCode[3];
	char chList[256]={0};
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	int unit = 1;
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	memset(countryCode, 0, sizeof(countryCode));
	strncpy(countryCode, nvram_safe_get(strcat_r(prefix, "country_code", tmp)), 2);
	rtklog("countryCode:%s\n",countryCode);

#if 1
	get_channel_list(WLAN_5G, chList, countryCode);
	puts(chList);
#elif 0
{
	int i = 0, num = sizeof(reg_domain)/sizeof(reg_domain_t);
	unsigned char code = 0;
	//struct channel_list *ch = reg_channel_5g_full_band;

	while (i < num) {
		if (strcmp(countryCode, reg_domain[i].name) == 0) {
			code = reg_domain[i].band_2G;
			break;
		}
		i++;
	}

	if (code) {
		for(i=0; i<reg_channel_5g_full_band[code-1].len; i++) {
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",reg_channel_5g_full_band[code-1].channel[i]);
			strcat(chList,tmp);
			if(i != reg_channel_5g_full_band[code-1].len - 1)
				strcat(chList,",");
		}
		puts(chList);
	}
}
#else
	if(rtk_get_channel_list_via_country(countryCode,chList,WLAN_5G)==0)
	{
		puts(chList);
	}
#endif
	return 1;
}

void Get_fail_ret(void)
{
#if 0	/* don't need the below now */
	unsigned char ate_ret_buf[2];
	int ate_ret_offset = HW_SETTING_OFFSET;

	/* using the last byte of territoryCode to save ate ret */
	memset(ate_ret_buf, 0, sizeof(ate_ret_buf));
	ate_ret_offset += sizeof(PARAM_HEADER_T);
	ate_ret_offset += (int)(&((struct hw_setting *)0)->modelName);
	ate_ret_offset -= 1;

	rtk_flash_read(ate_ret_buf, ate_ret_offset, 1);
	puts(ate_ret_buf);
#endif
}

void Get_fail_reboot_log(void)
{
}

void Get_fail_dev_log(void)
{
}

void set_factory_mode()
{
	char *mode_str;

	if (!nvram_match("ateCommand_flag", "1"))
		return;

	mode_str = ATE_REALTEK_FACTORY_MODE_STR();
	nvram_set(mode_str,"1");
	nvram_commit();
	free(mode_str);
}

