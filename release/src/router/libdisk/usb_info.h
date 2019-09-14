#ifndef _USB_INFO_H_
#define _USB_INFO_H_

#include <linux/version.h>
#include <rtconfig.h>

#define DEBUG_USB

#ifdef DEBUG_USB
#define usb_dbg(fmt, args...) do{ \
		FILE *fp = fopen("/tmp/usb.log", "a+"); \
		if(fp){ \
			fprintf(fp, "[usb_dbg: %s] ", __FUNCTION__); \
			fprintf(fp, fmt, ## args); \
			fclose(fp); \
		} \
	}while(0)
#else
#define usb_dbg _dprintf
#endif

#define foreach_58(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, ":")], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, ":")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ':'); \
	     strlen(word); \
	     next = next ? &next[strspn(next, ":")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, ":")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ':'))

#define MAX_WAIT_FILE 5

#define SYS_MODULE "/sys/module"
#define SYS_BLOCK "/sys/block"
#define SYS_TTY "/sys/class/tty"
#define SYS_NET "/sys/class/net"
#if (LINUX_KERNEL_VERSION >= KERNEL_VERSION(3,6,0)) || \
    defined(HND_ROUTER) || defined(RTCONFIG_LANTIQ)
#define SYS_USB "/sys/class/usbmisc"
#else
#define SYS_USB "/sys/class/usb"
#endif
#define SYS_SG "/sys/class/scsi_generic"
#define USB_DEVICE_PATH "/sys/bus/usb/devices"
#define USB_DRIVER_PATH "/sys/bus/usb/drivers"
#define SYS_RNDIS_PATH "/sys/module/rndis_host/drivers/usb:rndis_host"
#define SYS_CDCETH_PATH "/sys/module/cdc_ether/drivers/usb:cdc_ether"
#define SYS_NCM_PATH "/sys/module/cdc_ncm/drivers/usb:cdc_ncm"

#include <rtstate.h>

#if !defined(RTCONFIG_ALPINE) && !defined(RTCONFIG_LANTIQ)
#define USB_XHCI_PORT_1 get_usb_xhci_port(0)
#define USB_XHCI_PORT_2 get_usb_xhci_port(1)
#define USB_EHCI_PORT_1 get_usb_ehci_port(0)
#define USB_EHCI_PORT_2 get_usb_ehci_port(1)
#define USB_OHCI_PORT_1 get_usb_ohci_port(0)
#define USB_OHCI_PORT_2 get_usb_ohci_port(1)
#define USB_EHCI_PORT_3 get_usb_ehci_port(2)
#define USB_OHCI_PORT_3 get_usb_ohci_port(2)
#endif

#if defined(RTCONFIG_M2_SSD)
#define M2_SSD_PORT USB_EHCI_PORT_3
#endif
#ifdef BCM_MMC
#define SDCARD_PORT USB_EHCI_PORT_3
#endif

enum {
	DEVICE_TYPE_PRINTER=0,
	DEVICE_TYPE_MODEM,
	DEVICE_TYPE_DISK,
	DEVICE_TYPE_SG,
	DEVICE_TYPE_CD,
	DEVICE_TYPE_BECEEM,
	DEVICE_TYPE_UNKNOWN,
};

extern char *supported_types[];

extern int find_str_host_info(const char *str, int *host, int *channel, int *id, int *lun);
extern int find_disk_host_info(const char *dev, int *host, int *channel, int *id, int *lun);

extern char *get_device_type_by_interface(const char *usb_interface, char *type, const int type_size, char *dev, const int dev_size);
extern char *get_device_type_by_node(const char *usb_node, char *type, const int type_size, char *dev, const int dev_size);
extern int get_device_type_by_device(const char *device_name);
extern char *get_usb_node_by_string(const char *target_string, char *ret, const int ret_size);
extern char *get_usb_node_by_device(const char *device_name, char *buf, const int buf_size);
extern char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size);
extern char *get_usb_port_by_device(const char *device_name, char *buf, const int buf_size);
extern char *get_interface_by_string(const char *target_string, char *ret, const int ret_size);
extern char *get_interface_by_device(const char *device_name, char *buf, const int buf_size);
extern char *get_path_by_node(const char *usb_node, char *buf, const int buf_size);

extern unsigned int get_usb_vid(const char *usb_node);
extern unsigned int get_usb_pid(const char *usb_node);
extern char *get_usb_manufacturer(const char *usb_node, char *buf, const int buf_size);
extern char *get_usb_product(const char *usb_node, char *buf, const int buf_size);
extern char *get_usb_serial(const char *usb_node, char *buf, const int buf_size);
extern char *get_usb_speed(const char *usb_node, char *buf, const int buf_size);
extern int get_usb_interface_number(const char *usb_node);
extern int get_usb_interface_order(const char *interface_name);
extern char *get_usb_interface_class(const char *interface_name, char *buf, const int buf_size);
extern char *get_usb_interface_subclass(const char *interface_name, char *buf, const int buf_size);
extern int get_interface_numendpoints(const char *interface_name);
extern int get_interface_Int_endpoint(const char *interface_name);

extern int check_hotplug_action(const char *action);
extern int set_usb_common_nvram(const char *action, const char *device_name, const char *usb_node, const char *known_type);
extern int detect_usb_devices();
extern void unset_usb_nvram(char *port_path);

#ifdef RTCONFIG_USB_MODEM
extern void clean_modem_state(int modem_unit, int flag);

#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,6,36)
extern int hadWWANModule(void);
#endif
extern int hadOptionModule(void);
extern int hadSerialModule(void);
extern int hadACMModule(void);
extern int hadRNDISModule(void);
extern int isTTYNode(const char *device_name);
extern int isSerialNode(const char *device_name);
extern int isACMNode(const char *device_name);
extern int isSerialInterface(const char *interface_name, const int specifics, const unsigned int vid, const unsigned int pid);
extern int isACMInterface(const char *interface_name, const int specifics, const unsigned int vid, const unsigned int pid);
extern int isRNDISInterface(const char *interface_name, const unsigned int vid, const unsigned int pid);
extern int isQMIInterface(const char *interface_name);
extern int isGOBIInterface(const char *interface_name);
extern int isCDCETHInterface(const char *interface_name);
extern int isNCMInterface(const char *interface_name);
extern int isASIXInterface(const char *interface_name);
#ifdef RTCONFIG_USB_BECEEM
extern int isGCTInterface(const char *interface_name);
extern int hadBeceemModule();
extern int hadGCTModule(void);
extern int isBeceemNode(const char *device_name);
#endif
extern int is_usb_modem_ready(int wan_type);
#endif

#ifdef RTCONFIG_USB_PRINTER
extern int hadPrinterModule();
extern int hadPrinterInterface(const char *usb_node);
extern int isPrinterInterface(const char *interface_name);
#endif
extern int isStorageInterface(const char *interface_name);
extern int isStorageDevice(const char *device_name);
#if defined(RTCONFIG_USB_MODEM) || defined(RTCONFIG_USB_CDROM)
extern int isCDROMDevice(const char *device_name);
#endif
#if defined(RTCONFIG_M2_SSD)
extern int isM2SSDDevice(const char *device_name);
#else
static inline int isM2SSDDevice(__attribute__ ((unused)) const char *device_name) { return 0; }
#endif
#ifdef BCM_MMC
extern int isMMCDevice(const char *device_name);
#endif

extern char *find_sg_of_device(const char *device_name, char *buf, const int buf_size);

#ifdef RTCONFIG_INTERNAL_GOBI
extern char *get_gobi_portpath();
#endif

#endif	/* !_USB_INFO_H_ */
