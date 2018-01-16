//amas_bhctrl connection status
#define ETH 		0x01  	//ethernet
#define WL_2G 		0x02  	//2G
#define WL_5G 		0x04	//5G
#define WL_5G_1 	0x08	//5G-1

/*
Ethernet selection rule:
ETHERNET_PLUGIN: ethernet connected to P-AP and CAP. 
ETHERNET_HOP : ethernet connected to P-AP and CAP, ethernet hop count smaller than wireless.
We add nvram [amas_ethernet] for config ethernet selection rule.
*/
enum {
		NONE=0,
		ETHERNET_NONE,
		ETHERNET_PLUGIN,
		ETHERNET_HOP,
}; 

typedef struct maclist_r {
	uint count;					/* number of MAC addresses */
	struct ether_addr ea[1];	/* variable length array of MAC addresses */
} maclist_re;
