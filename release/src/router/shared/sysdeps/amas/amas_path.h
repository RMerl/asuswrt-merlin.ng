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



#define OB_OFF			1
#define OB_AVALIABLE	2
#define OB_REQ			3
#define OB_LOCKED		4
#define OB_SUCCESS		5

#define SS_OFF              0
#define SS_KEY				1
#define SS_KEYACK			2
#define SS_SECURITY			3
#define SS_SUCCESS			4
#define SS_FAIL         	5
#define SS_KEYFAIL			6
#define SS_SECURITYFAIL		7
#define SS_TIMEOUT          8
#define SS_KEY_FIN			9
#define	SS_KEYACK_FIN		10
#define SS_SECURITY_FIN		11
#define SS_SUCCESS_FIN		12
#define SS_OBD_FIN          13


#define HASH_LEN			32
#define GEN_KEY_LEN			32
