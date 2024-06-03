#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "shutils.h"
#include "rtstate.h"
#include "shared.h"

#define SWITCH_DEV   "/dev/rtkswitch"
#define MII_IFNAME      "switch0"

#define ASUS_SWITCH_PHY_ENABLE 0x0003
#define ASUS_SWITCH_PHY_DISABLE 0x0004

#define NR_WANLAN_PORT 5
enum {
	LAN1_PORT = 0,
	LAN2_PORT = 1,
	LAN3_PORT = 2,
	LAN4_PORT = 3,
	WAN_PORT  = 4,
	EXT_PORT1 = 5, //CPU PORT MAX 1G
	EXT_PORT2 = 6, //CPU PORT MAX 2.5G
};

static int LanWanPort[NR_WANLAN_PORT] = {3, 2, 1, 0, 4};

void default_LANWANPartition() {

    eval("swconfig", "dev", MII_IFNAME, "set", "enable_vlan", "1"); // enable vlan
    eval("swconfig", "dev", MII_IFNAME, "vlan","1","set", "ports", "0 1 2 3 6"); // set lan port
    eval("swconfig", "dev", MII_IFNAME, "vlan","2","set", "ports", "4 5"); // set wan port

	int i = 0, lan_portsmask = 0, wan_portsmask = 0;
	for(i = 0; i < NR_WANLAN_PORT-1; i++) {
		lan_portsmask |= (1U << LanWanPort[i]);
	}
	nvram_set_int("lan_portsmask", lan_portsmask);

	wan_portsmask = 1U << LanWanPort[WAN_PORT];
	nvram_set_int("wan_portsmask", wan_portsmask);
}

void config_esw_LANWANPartition()
{
	int stb = 0, wans_lanport = 0;
	int i = 0;
	int lan_portsmask = 0, wan_portsmask = 0, wanlan_portsmask = 0;
	unsigned char lan_ports_def[NR_WANLAN_PORT-1];
	char lan_ports[16];
	char wanlan_ports[16];// For DUAL WAN

	stb = nvram_get_int("switch_stb_x");
	wans_lanport = nvram_get_int("wans_lanport");

	memset(lan_ports_def, 1, sizeof(lan_ports_def));

	switch(stb) {
		case 1:
			lan_ports_def[LAN1_PORT] = 0;
			break;
		case 2:
			lan_ports_def[LAN2_PORT] = 0;
			break;
		case 3:
			lan_ports_def[LAN3_PORT] = 0;
			break;
		case 4:
			lan_ports_def[LAN4_PORT] = 0;
			break;
		case 5:
			lan_ports_def[LAN1_PORT] = 0;
			lan_ports_def[LAN2_PORT] = 0;
			break;
		case 6:
			lan_ports_def[LAN3_PORT] = 0;
			lan_ports_def[LAN4_PORT] = 0;
			break;
		default:
			break;
	}

//checking if lan as wan?
	if((get_wans_dualwan() & WANSCAP_LAN) && wans_lanport)
		lan_ports_def[wans_lanport-1] = 0;

	memset(lan_ports, 0, sizeof(lan_ports));
	memset(wanlan_ports, 0, sizeof(wanlan_ports));

	for(i = 0; i < NR_WANLAN_PORT-1; i++) {
		if(lan_ports_def[i]) {
			if(strlen(lan_ports))
				snprintf(lan_ports+strlen(lan_ports), sizeof(lan_ports)-strlen(lan_ports), " %d", LanWanPort[i]);
			else
				snprintf(lan_ports, sizeof(lan_ports), "%d", LanWanPort[i]);

			lan_portsmask |= (1U << LanWanPort[i]);
		}
	}

	nvram_set_int("lan_portsmask", lan_portsmask);


    eval("swconfig", "dev", MII_IFNAME, "set", "enable_vlan", "1"); // enable vlan


	if((get_wans_dualwan() & WANSCAP_LAN) && wans_lanport){
		snprintf(wanlan_ports, sizeof(wanlan_ports), "%d %dt", LanWanPort[wans_lanport-1], EXT_PORT2);
		eval("swconfig", "dev", MII_IFNAME, "vlan","3","set", "ports", wanlan_ports); // set lan port
		wanlan_portsmask = 1U << LanWanPort[wans_lanport-1];
		nvram_set_int("wanlan_portsmask", wanlan_portsmask);
		snprintf(lan_ports+strlen(lan_ports), sizeof(lan_ports)-strlen(lan_ports), " %dt", EXT_PORT2); //Add CPU port
	} else {
		snprintf(lan_ports+strlen(lan_ports), sizeof(lan_ports)-strlen(lan_ports), " %d", EXT_PORT2); //Add CPU port
	}

    eval("swconfig", "dev", MII_IFNAME, "vlan","1","set", "ports", lan_ports); // set lan port

	wan_portsmask = 1U << LanWanPort[WAN_PORT];
	nvram_set_int("wan_portsmask", wan_portsmask);

    return;
}

int rtkswitch_ioctl(int val, int val2)
{
	dbG("val %d, val2 %d\n", val, val2);
#if 0
	switch (val) {
	case 8:
		config_rtl_esw_LANWANPartition(val2);
		break;

	default:
		break;
	}
#endif
	return 0;
}

int config_rtkswitch(int argc, char *argv[])
{
	int val = 0;
	int val2 = 0;

	if(argc < 2)
		return -1;

	if(argc >= 3)
		val2 = (int) strtol(argv[2], NULL, 0);

	val = (int) strtol(argv[1], NULL, 0);

	return rtkswitch_ioctl(val, val2);
}

typedef struct {
	unsigned int link[5];
	unsigned int speed[5];
	unsigned int duplex[5];
} phyState;

static void mt7622_rtl8367s_get_port_status(phyState *pS) {
	int fd = -1;

	if(pS == NULL)
		return;
  
	fd = open(SWITCH_DEV, O_RDONLY);

	if (fd < 0) {
		perror(SWITCH_DEV);
		return;
	}

	memset(pS, 0, sizeof(phyState));

	if (ioctl(fd, 0, pS) < 0) {
		perror(SWITCH_DEV" ioctl");
		close(fd);
		return;
	}
	
	close(fd);
}

static void rtk_set_phy_enable(int portmask) {
	int fd = -1;

	fd = open(SWITCH_DEV, O_RDONLY);

	if (fd < 0) {
		perror(SWITCH_DEV);
		return;
	}

	if (ioctl(fd, ASUS_SWITCH_PHY_ENABLE, &portmask) < 0) {
		perror(SWITCH_DEV" ioctl");
		close(fd);
		return;
	}

	close(fd);
}

static void rtk_set_phy_disable(int portmask) {
	int fd = -1;

	fd = open(SWITCH_DEV, O_RDONLY);

	if (fd < 0) {
		perror(SWITCH_DEV);
		return;
	}

	if (ioctl(fd, ASUS_SWITCH_PHY_DISABLE, &portmask) < 0) {
		perror(SWITCH_DEV" ioctl");
		close(fd);
		return;
	}

	close(fd);
}

void ATE_mt7622_rtl8367s_esw_port_status(void) {
	phyState pS;

	mt7622_rtl8367s_get_port_status(&pS);

	printf("W0=%C;L1=%C;L2=%C;L3=%C;L4=%C;\n",
			(pS.link[4] == 1) ? (pS.speed[4] == 2) ? 'G' : 'M': 'X',
			(pS.link[3] == 1) ? (pS.speed[3] == 2) ? 'G' : 'M': 'X',
			(pS.link[2] == 1) ? (pS.speed[2] == 2) ? 'G' : 'M': 'X',
			(pS.link[1] == 1) ? (pS.speed[1] == 2) ? 'G' : 'M': 'X',
			(pS.link[0] == 1) ? (pS.speed[0] == 2) ? 'G' : 'M': 'X');
	return;
}

int rtkswitch_WanPort_linkUp(void)
{
	rtk_set_phy_enable(1U << LanWanPort[WAN_PORT]);
	return 0;
}

int rtkswitch_WanPort_linkDown(void)
{
	rtk_set_phy_disable(1U << LanWanPort[WAN_PORT]);
	return 0;
}

int rtkswitch_LanPort_linkUp(void)
{
	int portsmask = 0;
	int i = 0;

	for(i = 0; i < NR_WANLAN_PORT-1; i++) {
		portsmask |= (1U << LanWanPort[i]);
	}
	rtk_set_phy_enable(portsmask);

    return 0;
}

int rtkswitch_LanPort_linkDown(void)
{
	int portsmask = 0;
	int i = 0;

	for(i = 0; i < NR_WANLAN_PORT-1; i++) {
		portsmask |= (1U << LanWanPort[i]);
	}

	rtk_set_phy_disable(portsmask);

    return 0;
}

int rtkswitch_AllPort_linkUp(void)
{
	int portsmask = 0;
	int i = 0;

	for(i = 0; i < NR_WANLAN_PORT; i++) {
		portsmask |= (1U << LanWanPort[i]);
	}
	rtk_set_phy_enable(portsmask);

    return 0;
}

int rtkswitch_AllPort_linkDown(void)
{
	int portsmask = 0;
	int i = 0;

	for(i = 0; i < NR_WANLAN_PORT; i++) {
		portsmask |= (1U << LanWanPort[i]);
	}
	rtk_set_phy_disable(portsmask);

    return 0;
}

unsigned int rtkswitch_wanPort_phyStatus(int wan_unit)
{
	int i = 0;
	phyState pS;

	mt7622_rtl8367s_get_port_status(&pS);

	switch (get_dualwan_by_unit(wan_unit))
	{
		case WANS_DUALWAN_IF_WAN:
			return pS.speed[WAN_PORT];
		break;
		case WANS_DUALWAN_IF_LAN:
			for(i = 0; i < NR_WANLAN_PORT-1; i++) {
				if((nvram_get_int("wanlan_portsmask") & 1U<<LanWanPort[i]) )
					return pS.link[LanWanPort[i]];
			}
		break;
		default:
			return 0;
	}
	return 0;
}

unsigned int rtkswitch_lanPorts_phyStatus(void)
{
	int lan_portsmask = 0, i = 0;
	phyState pS;

	mt7622_rtl8367s_get_port_status(&pS);

	lan_portsmask = nvram_get_int("lan_portsmask");

	for(i = 0; i < NR_WANLAN_PORT-1; i++) {
		if((lan_portsmask & 1U<<LanWanPort[i]) && pS.link[LanWanPort[i]])
			return 1;
	}

	return 0;
}

#define GPIOLIB_DIR	"/sys/class/gpio"
//MAX is 512 - GPIO MAX(103)
#define GPIO_BASE 409

/* Export specified GPIO
 * @return:
 * 	0:	success
 *  otherwise:	fail
 */
static int __export_gpio(uint32_t gpio)
{
	char gpio_path[PATH_MAX], export_path[PATH_MAX], gpio_str[] = "999XXX";

	if (!d_exists(GPIOLIB_DIR)) {
		_dprintf("%s does not exist!\n", __func__);
		return -1;
	}
	snprintf(gpio_path, sizeof(gpio_path),"%s/gpio%d", GPIOLIB_DIR, gpio);
	if (d_exists(gpio_path))
		return 0;

	snprintf(export_path, sizeof(export_path), "%s/export", GPIOLIB_DIR);
	snprintf(gpio_str, sizeof(gpio_str), "%d", gpio);
	f_write_string(export_path, gpio_str, 0, 0);

	return 0;
}

int ralink_gpio_write_bit(int gpio, int value)
{
	char path[PATH_MAX], val_str[10];

	gpio += GPIO_BASE;
	snprintf(val_str, sizeof(val_str), "%d", !!value);
	snprintf(path, sizeof(path), "%s/gpio%d/value", GPIOLIB_DIR, gpio);
	f_write_string(path, val_str, 0, 0);
	return -1;
}

int ralink_gpio_read_bit(int gpio)
{
	char path[PATH_MAX], value[10];

	gpio += GPIO_BASE;
	snprintf(path, sizeof(path), "%s/gpio%d/value", GPIOLIB_DIR, gpio);
	f_read_string(path, value, sizeof(value));

	return safe_atoi(value);
	return -1;
}

int ralink_gpio_init(unsigned int gpio, int dir)
{
	char path[PATH_MAX], v[10], *dir_str = "in";

	gpio += GPIO_BASE;
	if (dir == GPIO_DIR_OUT) {
		dir_str = "out";		/* output, low voltage */
		*v = '\0';
		snprintf(path, sizeof(path), "%s/gpio%d/value", GPIOLIB_DIR, gpio);
		if (f_read_string(path, v, sizeof(v)) > 0 && safe_atoi(v) == 1)
			dir_str = "high";	/* output, high voltage */
	}

	__export_gpio(gpio);
	snprintf(path, sizeof(path), "%s/gpio%d/direction", GPIOLIB_DIR, gpio);
	f_write_string(path, dir_str, 0, 0);

	return 0;
}

int ralink_gpio_init2(unsigned int gpio, int dir)
{
	char path[PATH_MAX], *dir_str = "in";

	gpio += GPIO_BASE;
	if (dir == GPIO_DIR_OUT) {
		dir_str = "high";	/* output, high voltage */
	}

	__export_gpio(gpio);
	snprintf(path, sizeof(path), "%s/gpio%d/direction", GPIOLIB_DIR, gpio);
	f_write_string(path, dir_str, 0, 0);

	return 0;
}

