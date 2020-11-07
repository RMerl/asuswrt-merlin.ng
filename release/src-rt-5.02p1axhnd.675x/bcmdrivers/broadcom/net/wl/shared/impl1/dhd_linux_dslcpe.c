/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

/* This is to customize dhd_linux for DSLCPE */

#include <typedefs.h>
#include <linuxver.h>
#include <board.h>
#include <osl.h>

#if defined(BCM_BLOG)
#include <linux/blog.h>
#include <bcm_mcast.h>
#endif
#include <epivers.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <dngl_stats.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_flowring.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <dhdioctl.h>
#include <sdiovar.h>
#include <bcmmsgbuf.h>
#include <dhd_pcie.h>

#include <dhd_linux_dslcpe.h>

char mfg_firmware_path[MOD_PARAM_PATHLEN];

module_param_string(mfg_firmware_path, mfg_firmware_path, MOD_PARAM_PATHLEN, 0660);

#if defined(BCM_DHD_RUNNER) && !defined(BCM_COUNTER_EXTSTATS)
extern void  dhd_clear_stats(struct net_device *net);
#endif /* BCM_DHD_RUNNER */

void
update_firmware_path(struct dhd_bus *bus, char *pfw_path)
{
	int nodhdhdr_flag = 0;
#if defined(BCM_DHD_RUNNER)
	dhd_helper_status_t  dhd_rnr_status;
#endif

	DHD_ERROR(("%s: pfw_path = %s\n", __FUNCTION__, pfw_path));
	if ((pfw_path == NULL) || (strstr(pfw_path, ".bea") != NULL)) {
		return;
	}

	switch(bus->sih->chip){
	CASE_BCM43602_CHIP: 
	    strcat((char *)pfw_path, "/43602");
	    break;
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
#if defined(BCM4363_CHIP_ID) && !defined(ENABLE_AIRIQ)
	case BCM4363_CHIP_ID:
#endif /* BCM4363_CHIP_ID */
	    strcat((char *)pfw_path, "/4366");
	    break;
#if defined(BCM4363_CHIP_ID) && defined(ENABLE_AIRIQ)
	case BCM4363_CHIP_ID:
	    strcat((char *)pfw_path, "/4363");
		break;
#endif
	default: DHD_ERROR(("%s: no CHIP ID matches\n", __FUNCTION__));
	}

	switch(bus->sih->chiprev){
	case 0:
	    strcat((char *)pfw_path, "a0");
	    break;
	case 1:
	    strcat((char *)pfw_path, "a1");
	    break;
	case 2:
	    strcat((char *)pfw_path, "b0");
	    break;
	case 3:
	    strcat((char *)pfw_path, "a3");
	    break;
	case 4:
	    strcat((char *)pfw_path, "c0");
	    break;
	default: DHD_ERROR(("%s: no CHIP REV matches\n", __FUNCTION__));;
	}

#if defined(BCM_DHDHDR) && defined(BCM_DHD_RUNNER)
	if (dhd_runner_do_iovar(bus->dhd->runner_hlp,
				DHD_RNR_IOVAR_STATUS,
				0, (char*)(&dhd_rnr_status),
				sizeof(dhd_rnr_status)) == BCME_OK) {
		if ((dhd_rnr_status.sup_features.txoffl == 1) &&
		    (dhd_rnr_status.sup_features.dhdhdr == 0)) {
			DHD_ERROR(("%s: DHDHDR not supported by runner, using nodhdhdr dongle\n", __FUNCTION__));
			nodhdhdr_flag = 1;
		}
	}
#endif /* BCM_DHD_RUNNER && BCM_DHDHDR */

	if (BCM4365_CHIP(bus->sih->chip) ||
	    (BCM43602_CHIP(bus->sih->chip) && (bus->sih->chiprev == 3))) {
		if (nodhdhdr_flag)
			strcat((char*)pfw_path, "/rtecdc_nodhdhdr.bin");
		else
			strcat((char*)pfw_path, "/rtecdc.bin");
	} else {
		strcat((char*)pfw_path, "/rtecdc.bin");
	}
}

int dhd_vars_adjust(struct dhd_bus *bus, char *memblock, uint *total_len)
{
	int bcmerror = BCME_OK;
	if(bus->vars) {
		uint len=*total_len;	
		uint i=len-1;
		char *locbufp=bus->vars;
		while(i>=0 && memblock[i]=='\0') i--;
		len=i+2;

		for(i=0; i < bus->varsz; i++, locbufp++){
		        if(*locbufp == '\0')
			        break;
			while(*locbufp != '\0') {
			        i++;
				locbufp++;
			}
		}

		if ((len+i) < MAX_NVRAMBUF_SIZE) {
			memcpy(memblock+len,bus->vars,i);
			*total_len=len+i;
		} else {
			DHD_ERROR(("%s:nvram size %d is bigger than max:%d \n",__FUNCTION__,len+bus->varsz,MAX_NVRAMBUF_SIZE));
			bcmerror = BCME_ERROR;
		}
	}
	return bcmerror;
}


/*
 * Get the mutxmax nvram variable
 * For 6838/6848/47189 platforms set the default value (0) if not available
 */
char*
dhd_get_nvram_mutxmax(dhd_pub_t *dhd)
{
	char *var = nvram_get("mutxmax");

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM947189)
	/* Add a check for 4366chip if needed */
	if (var == NULL) {
	    /* Update with default value, so later code can access it */
	    nvram_set("mutxmax", "0");
	    DHD_ERROR(("mutxmax nvram is not set, using platform default \r\n"));
	    var = nvram_get("mutxmax");
	}
#endif /* CONFIG_BCM96838 || CONFIG_BCM96848 || CONFIG_BCM947189 */

	return var;
}

/*
 * Check and update mutxmax var for dongle
 */
int 
dhd_check_and_set_mutxmax(dhd_pub_t *dhd, char *memblock, uint* len)
{
#define MUTXMAXBUFLEN        12
	char *var = NULL;
	char mutxmaxbuf[MUTXMAXBUFLEN]="mutxmax=";
	char *locbufp;
	int mutxmax;

	/* If mutxmax nvram is present, need to update the nvramvars */
	/* skip if it is set to auto mode (default) */
	var = nvram_get("mutxmax");
	if (var != NULL) {
	    mutxmax = bcm_strtoul(var, NULL, 0);
	    if (mutxmax != -1) {
	        if((*len)+12 >= MAX_NVRAMBUF_SIZE) {
	            DHD_ERROR(("%s: nvram size is too big \n", __FUNCTION__));
	            return BCME_ERROR;
	        }

	        /* memblock delimiter is END\0 string */
	        /* Move END by 12bytes to fit the mutxmax var */
	        locbufp = memblock+(*len)-4;
	        memcpy(locbufp+MUTXMAXBUFLEN, locbufp, 4);

	        /* append mutxmax var to the list */
	        sprintf(mutxmaxbuf+strlen(mutxmaxbuf), "0x%1x", mutxmax);
	        memcpy(locbufp, mutxmaxbuf, MUTXMAXBUFLEN);
	        (*len) = (*len) + MUTXMAXBUFLEN;
	    }
	}

	return  BCME_OK;
}

int 
dhd_check_and_set_mac(dhd_pub_t *dhd, char *memblock, uint* len)
{
	char *locbufp=memblock;
	const char macaddr00[]="macaddr=00:00:00:00:00:00";
	const char macaddr11[]="macaddr=11:11:11:11:11:11";
	char macaddrbuf[26]="macaddr=";
	int i,j;
	int appendMac=1;
	int replaceMac=0;
	struct ether_addr mac;

	for(i=0; i < (*len); i++, locbufp++){
		if(*locbufp == '\0')
			break;
		if(memcmp(locbufp, macaddrbuf, 7) == 0) {
			appendMac = 0;
			if(memcmp(locbufp, macaddr00, 25) == 0 || memcmp(locbufp, macaddr11, 25) == 0)
				replaceMac = 1;
			break;
		}
		while(*locbufp != '\0') {
			i++;
			locbufp++;
		}
	}
	
	if(appendMac && (*len)+26>= MAX_NVRAMBUF_SIZE) {
		DHD_ERROR(("%s: nvram size is too big \n", __FUNCTION__));
		return BCME_ERROR;
	} else if(appendMac || replaceMac) {
		printf("Replace or append with internal Mac Address\n");
		dhd_get_cfe_mac(dhd, &mac);
		for (j = 0; j < (ETH_ALEN-1); j++)
			sprintf(macaddrbuf+strlen(macaddrbuf),"%2.2X:",mac.octet[j]);
		sprintf(macaddrbuf+strlen(macaddrbuf),"%2.2X",mac.octet[j]);
		memcpy(memblock+i, macaddrbuf, 25);
		if(appendMac)
			(*len) = (*len) + 26;
	}

	return  BCME_OK;
}

void 
dhd_get_cfe_mac(dhd_pub_t *dhd, struct ether_addr *mac)
{
	unsigned long ulId = (unsigned long)('w'<<24) + (unsigned long)('l'<<16) + dhd_get_instance(dhd);	
	kerSysGetMacAddress(mac->octet, ulId);
}

/* reset DHD counters
 * - Reset interface counters
 * - Reset DHD public counters
 * - Reset bus counters
 * - Reset net rx_dropped count
*/
void
dhd_reset_cnt(struct net_device *net) 
{
	dhd_pub_t *dhdp = dhd_dev_get_dhdpub(net);
	int ifidx;

	/* Get the interface index */
	ifidx = dhd_dev_get_ifidx(net);
	if (ifidx == DHD_BAD_IF) {
		/* Set to main interface */
		ifidx = 0;
	}

	/* Clear the interface statistics */
	dhd_if_clear_stats(dhdp, ifidx);
#if defined(BCM_DHD_RUNNER) && !defined(BCM_COUNTER_EXTSTATS)
	/* Clear interface b, c, d, m stats */
	dhd_clear_stats(net); 
#endif /* BCM_DHD_RUNNER */

	/* Clear the net stack rx_dropped pkts count as well */
	/* This count gets added by the network stack for stats */ 
	atomic_long_set(&net->rx_dropped, 0);

	/* Clear DHD public counters */
	dhdp->tx_packets = dhdp->rx_packets = 0;
	dhdp->tx_errors = dhdp->rx_errors = 0;
	dhdp->tx_ctlpkts = dhdp->rx_ctlpkts = 0;
	dhdp->tx_ctlerrs = dhdp->rx_ctlerrs = 0;
	dhdp->tx_dropped = 0;
	dhdp->rx_dropped = 0;
	dhdp->tx_pktgetfail = 0;
	dhdp->rx_pktgetfail = 0;
	dhdp->rx_readahead_cnt = 0;
	dhdp->tx_realloc = 0;
	dhdp->wd_dpc_sched = 0;
	memset(&dhdp->dstats, 0, sizeof(dhdp->dstats));
	dhd_bus_clearcounts(dhdp);
#ifdef PROP_TXSTATUS
	/* clear proptxstatus related counters */
	dhd_wlfc_clear_counts(dhdp);
#endif /* PROP_TXSTATUS */

	return;
}

/* Process DSLCPE private IOCTLS:
 * 
 * returns  not supported for unknown IOCTL's
*/
int 
dhd_priv_ioctl(struct net_device *net, struct ifreq *ifr, int cmd) 
{
	int	error = 0;
	int	isup = 0;
	
	switch (cmd) {
		case SIOCGLINKSTATE:
			if (net->flags&IFF_UP) isup = 1;
			if (copy_to_user((void*)(int*)ifr->ifr_data, (void*)&isup,
				sizeof(int))) {
				error = -EFAULT;
			}
			break;
			
		case SIOCSCLEARMIBCNTR:
			dhd_reset_cnt(net);
 			break;

		default:
			/* All other private IOCTLs not supported yet */
			error = -EOPNOTSUPP;
			break;
	}

	return error;
}


enum {
#if defined(PKTC)
	IOV_PKTC = 1,
	IOV_PKTCBND,
#endif /* PKTC */
	IOV_DSLCPE_END
};

const bcm_iovar_t dhd_dslcpe_iovars[] = {
#if defined(PKTC)
	{"pktc", 		IOV_PKTC,	0, IOVT_BOOL, 0	},
	{"pktcbnd",		IOV_PKTCBND,0, IOVT_UINT32, 0 },
#endif
	{NULL, 0, 0, 0, 0 }
};


int
dhd_dslcpe_iovar_op(dhd_pub_t *dhdp, const char *name,
                 void *params, int plen, void *arg, int len, bool set)
{
	const bcm_iovar_t *vi = NULL;
	int bcmerror = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 int_val3 = 0;
	bool bool_val = 0;
	int val_size;
	uint32 actionid;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(name);
	ASSERT(len >= 0);

	/* Get MUST have return space */
	ASSERT(set || (arg && len));

	/* Set does NOT take qualifiers */
	ASSERT(!set || (!params && !plen));

	DHD_INFO(("%s: %s %s, len %d plen %d\n", __FUNCTION__,
	         name, (set ? "set" : "get"), len, plen));

	/* Look up var locally; if not found pass to host driver */
	if ((vi = bcm_iovar_lookup(dhd_dslcpe_iovars, name)) == NULL) {
		bcmerror = BCME_NOTFOUND;
		goto exit;
	}

	/* set up 'params' pointer in case this is a set command so that
	 * the convenience int and bool code can be common to set and get
	 */
	if (params == NULL) {
		params = arg;
		plen = len;
	}

	if (vi->type == IOVT_VOID)
		val_size = 0;
	else if (vi->type == IOVT_BUFFER)
		val_size = len;
	else
		/* all other types are integer sized */
		val_size = sizeof(int);

	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);
	
	if ((bcmerror = bcm_iovar_lencheck(vi, arg, len, IOV_ISSET(actionid))) != 0)
		goto exit;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (plen >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val2));

	if (plen >= (int)sizeof(int_val) * 3)
		bcopy((void*)((uintptr)params + 2 * sizeof(int_val)), &int_val3, sizeof(int_val3));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
#if defined(PKTC)
	case IOV_SVAL(IOV_PKTC):
		dhdp->pktc = int_val;
		break;
	case IOV_GVAL(IOV_PKTC):
		int_val = (int32)dhdp->pktc;
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_PKTCBND):
		dhdp->pktcbnd = int_val;
		break;
	case IOV_GVAL(IOV_PKTCBND):
		int_val = (int32)dhdp->pktcbnd;
		bcopy(&int_val, arg, val_size);
		break;
#endif /* PKTC */
	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

exit:
	return bcmerror;
} /* dhd_dslcpe_iovar_op */

#if defined(DSLCPE_PWLCS)
/*
 * PWLCS (PCIe WireLess Card Status)
 *
 * Detects PCIe Wireless card probe failures (failed to initialize, crash during initialization)
 * Recovers the system by rebooting on failures (done in the driver loading script)
 * Reboots until a configurable maximum boot failures and declares the card is bad.
 * Bad card gets initialized with a dummy network interface to the upper layers
 * System will continue to use dummy interface until a configurable max dummy boots.
 * After max dummy boots, system will try to boot the wireless card normally again.
 *
 * Helper functions
 * - To detect card is bad (based on number of card boot failures)
 * - Use dummy network interface on a bad card
 *
 * Configuration - using Scratch Pad as Storage area
 *
 * ----- KEY ----    -SCOPE-    ------ VALUES ------             - DEFAULT -
 *  pwlcsmaxcrd      global     0: disable, n: enable n cards         0
 *  pwlcsmaxfbc      global     > 0                                   3
 *  plwcsmaxdbc      global     > 0                                   1
 *  pwlcspcie%d      perslot    GOOD,                               GOOD
 *                              BOOT1,BOOT2,..,BOOT[MAXFBC]
 *                              BAD1,BAD2,.....,BAD[MAXDBC]
 *
 */

/* Local defines */
#define PWLCS_MAXCRD_DEF               0
#define PWLCS_MAXFBC_DEF               3
#define PWLCS_MAXDBC_DEF               1


#define PWLCS_ENABLE_ID                0
#define PWLCS_MAXFBC_ID                1
#define PWLCS_MAXDBC_ID                2
#define PWLCS_STATUS_ID                3
#define PWLCS_TESTBC_ID                3

#define PWLCS_PSPKEY_MAX_SIZE          16
#define PWLCS_PSPDATA_MAX_SIZE         16

#define PWLCS_MAXCRD_PSPKEY            "pwlcsmaxcrd"
#define PWLCS_MAXFBC_PSPKEY            "pwlcsmaxfbc"
#define PWLCS_MAXDBC_PSPKEY            "pwlcsmaxdbc"
#define PWLCS_STATUS_PSPKEY_FMT        "pwlcspcie%d"
#define PWLCS_STATUS_PSPDATA_FMT       "%s"
#define PWLCS_CNT_PSPDATA_FMT          "%d"
#define PWLCS_TEST_PSPKEY_FMT          "pwlcststbc%d"

#define PWLCS_BC_GOOD                  0
#define PWLCS_BC_BAD                   (-1)
#define PWLCS_STATUS_STR_GOOD          "GOOD"
#define PWLCS_STATUS_STR_BOOT          "BOOT"
#define PWLCS_STATUS_STR_BAD           "BAD"

#define PWLCS_STATUS_BOOT_FMT          PWLCS_STATUS_STR_BOOT"%d"
#define PWLCS_STATUS_BAD_FMT           PWLCS_STATUS_STR_BAD"%d"

#define PWLCS_TEST_WF_PCIE(u)          (1ul << (2+(u)))
#define PWLCS_TEST_MASK                (0x3C)
#define PWLCS_TEST_BAD_WLADDR           0x1280

#define DHD_DUMMYIF_DEV_PRIV_SIZE	(sizeof(dhd_dummyif_dev_priv_t))
#define DHD_DUMMYIF_DEV_STATS(dev)	(((dhd_dummyif_dev_priv_t *)DEV_PRIV(dev))->stats)

/* Local Macros */
#if defined(DSLCPE_PWLCS_DBG)
#define PWLCS_PR(fmt, args...)	       printk(fmt, ##args)
#else /* !DSLCPE_PWLCS_DBG */
#define PWLCS_PR(fmt, args...)	       do {} while (0)
#endif /* !DSLCPE_PWLCS_DBG */


/* Local structure */
typedef struct pwlcs_cb {
	int unit;       /* dhd unit number */
	int enable;     /* 0: Disabled, 1: Enabled */
	int max_fbc;    /* Max number of failed boots before the card is declared bad */
	int max_dbc;    /* Max number of dummy if boots before fbc resets */
	int cur_bc;     /* Current boot count */
} pwlcs_cb_t;

typedef struct dhd_dummyif_dev_priv {
	struct net_device_stats stats;
} dhd_dummyif_dev_priv_t;


/********************************
 * Local function declerations 
 ********************************/
static int dhd_pwlcs_bc2status(pwlcs_cb_t *pwlcs, int bc, char *status, int len);
static int dhd_pwlcs_status2bc(pwlcs_cb_t *pwlcs, int *bc, char *status, int len);

static int dhd_pwlcs_get_enable(pwlcs_cb_t *pwlcs);
static int dhd_pwlcs_get_maxfbc(pwlcs_cb_t *pwlcs);
static int dhd_pwlcs_get_maxdbc(pwlcs_cb_t *pwlcs);

static int dhd_pwlcs_get_bc(pwlcs_cb_t *pwlcs);
static int dhd_pwlcs_set_bc(pwlcs_cb_t *pwlcs, int bc);

static int dhd_dummyif_start_xmit(struct sk_buff *skb, struct net_device *dev);
static int dhd_dummyif_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
static struct net_device_stats* dhd_dummyif_get_stats(struct net_device *dev);

/********************************
 * Local variables 
 ********************************/
static pwlcs_cb_t dhd_pwlcs_cb[FWDER_MAX_UNIT] = {};
static const struct net_device_ops netdev_dummyif_ops =
{
	.ndo_start_xmit = dhd_dummyif_start_xmit,
	.ndo_get_stats = dhd_dummyif_get_stats,
	.ndo_do_ioctl = dhd_dummyif_do_ioctl
};


/********************************
 * Local functions 
 ********************************/
/* Get card status string given boot count */
int
dhd_pwlcs_bc2status(pwlcs_cb_t *pwlcs, int bc, char *status, int len)
{
	if (bc == PWLCS_BC_GOOD) {
	    strncpy(status, PWLCS_STATUS_STR_GOOD, len);
	} else if (bc > pwlcs->max_fbc) {
	    strncpy(status, PWLCS_STATUS_STR_BAD, len);
	    len = strlen(status);
	    status[len] = '0' + bc - pwlcs->max_fbc;
	    status[len + 1] = 0;
	} else {
	    strncpy(status, PWLCS_STATUS_STR_BOOT, len);
	    len = strlen(status);
	    status[len] = '0' + bc;
	    status[len + 1] = 0;
	}

	PWLCS_PR("  %s(%d): [%d] => [%s]\n", __FUNCTION__, pwlcs->unit, bc, status);
	return 0;
}

/* Get card boot count given card status string */
int
dhd_pwlcs_status2bc(pwlcs_cb_t *pwlcs, int *bc, char *status, int len)
{
	int cnt;

	if (strncmp(status, PWLCS_STATUS_STR_GOOD, strlen(PWLCS_STATUS_STR_GOOD)) == 0) {
	    /* Initializing */
	    cnt = PWLCS_BC_GOOD;
	} else if (strncmp(status, PWLCS_STATUS_STR_BAD, strlen(PWLCS_STATUS_STR_BAD)) == 0) {
	    /* Booting with Dummy If */
	    sscanf(status, PWLCS_STATUS_BAD_FMT, &cnt);
	    cnt += pwlcs->max_fbc;
	} else if (strncmp(status, PWLCS_STATUS_STR_BOOT, strlen(PWLCS_STATUS_STR_BOOT)) == 0) {
	    /* Booting with failure */
	    sscanf(status, PWLCS_STATUS_BOOT_FMT, &cnt);
	} else {
	    /* Default assumes Good */
	    cnt = PWLCS_BC_GOOD;
	}

	if (bc) *bc = cnt;

	PWLCS_PR("  %s(%d): [%s] => [%d]\n", __FUNCTION__, pwlcs->unit, status, cnt);

	return cnt;
}

/* Get card pwlcs enable status */
int
dhd_pwlcs_get_enable(pwlcs_cb_t *pwlcs)
{
	char *val;
	int enable = PWLCS_MAXCRD_DEF;

	val = nvram_get(PWLCS_MAXCRD_PSPKEY);

	if (val) {
	    sscanf(val, PWLCS_CNT_PSPDATA_FMT, &enable);
	    /* filter out junk settings */
	    if (enable < 0) enable = 0;
	}

	PWLCS_PR(" %s(%d) Enable [%d]\n", __FUNCTION__, pwlcs->unit, enable);

	return enable;
}

/* Get card pwlcs max boot fail count */
int
dhd_pwlcs_get_maxfbc(pwlcs_cb_t *pwlcs)
{
	char *val;
	int maxfbc = PWLCS_MAXFBC_DEF;

	/* Get the key value from PSP */
	val = nvram_get(PWLCS_MAXFBC_PSPKEY);

	if (val) {
	    sscanf(val, PWLCS_CNT_PSPDATA_FMT, &maxfbc);
	    /* filter out junk settings */
	    if (maxfbc < 0) maxfbc = PWLCS_MAXFBC_DEF;
	}

	PWLCS_PR(" %s(%d) MAXFBC [%d]\n", __FUNCTION__, pwlcs->unit, maxfbc);

	return maxfbc;
}

/* Get card pwlcs max dummy if boot count */
int
dhd_pwlcs_get_maxdbc(pwlcs_cb_t *pwlcs)
{
	char *val;
	int maxdbc = PWLCS_MAXDBC_DEF;

	/* Get the key value from PSP */
	val = nvram_get(PWLCS_MAXDBC_PSPKEY);
	if (val) {
	    sscanf(val, PWLCS_CNT_PSPDATA_FMT, &maxdbc);
	    /* filter out junk settings */
	    if (maxdbc < 0) maxdbc = PWLCS_MAXDBC_DEF;
	}

	PWLCS_PR(" %s(%d) MAXDBC [%d]\n", __FUNCTION__, pwlcs->unit, maxdbc);

	return maxdbc;
}

/* Get given card fail boot count */
int
dhd_pwlcs_get_bc(pwlcs_cb_t *pwlcs)
{
	char key[PWLCS_PSPKEY_MAX_SIZE];
	char *val;
	int bc = PWLCS_BC_GOOD;

	/* Prepare the PSP key information */
	snprintf(key, sizeof(key), PWLCS_STATUS_PSPKEY_FMT, pwlcs->unit);

	/* Get the key value from PSP */
	val = nvram_get(key);
	if (val) {
	    bc = dhd_pwlcs_status2bc(pwlcs, NULL, val, strlen(val));
	}

	PWLCS_PR("%s(%d) CURBC [%d] CURSTS [%s]\n", __FUNCTION__, pwlcs->unit, bc, val);

	return bc;
}

/* Set given card fail boot count */
int
dhd_pwlcs_set_bc(pwlcs_cb_t *pwlcs, int bc)
{
	char key[PWLCS_PSPKEY_MAX_SIZE];
	char buff[PWLCS_PSPDATA_MAX_SIZE];
	int len = sizeof(buff);

	/* Prepare the PSP key information */
	snprintf(key, sizeof(key), PWLCS_STATUS_PSPKEY_FMT, pwlcs->unit);

	if (dhd_pwlcs_bc2status(pwlcs, bc, buff, len) != 0) {
	    return -1;
	}
	len = strlen(buff);

	/* Get the key value from PSP */
	nvram_set(key, buff);

	PWLCS_PR("%s(%d) CURBC [%d] CURSTS [%s]\n", __FUNCTION__, pwlcs->unit, bc, buff);

	return 0;
}

static int
dhd_dummyif_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	PWLCS_PR("PWLCS: %s\n", __FUNCTION__);
	PKTFREE(NULL, skb, FALSE);
	return 0;
}


static int
dhd_dummyif_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	PWLCS_PR("PWLCS: %s\n", __FUNCTION__);
	return 0;
}

static struct net_device_stats*
dhd_dummyif_get_stats(struct net_device *dev)
{
	PWLCS_PR("PWLCS: %s\n", __FUNCTION__);
	return &DHD_DUMMYIF_DEV_STATS(dev);
}




/* External Functions */
/* Initialize the card pwlcs
   
   return
     0:  card status good/disabled/nothing to do
    -1:  card status bad, use dummy interface
    +ve: boot count (keep card initialization)
*/
int
dhd_pwlcs_init_bc(int unit)
{
	pwlcs_cb_t *pwlcs = NULL;
	int status;

	if (unit >= FWDER_MAX_UNIT) {
	    PWLCS_PR("%s(%d): exceeds max [%d]\n", __FUNCTION__, unit, FWDER_MAX_UNIT);
	    return PWLCS_BC_GOOD;
	}

	pwlcs = &dhd_pwlcs_cb[unit];
	pwlcs->unit = unit;

	/* Check if pwlcs is enabled for this radio or not */
	pwlcs->enable = dhd_pwlcs_get_enable(pwlcs);
	if (!pwlcs->enable) {
	    /* Nothing to do, Just return GOOD status */
	    PWLCS_PR("%s(%d): not enabled\n", __FUNCTION__, unit);
	    return PWLCS_BC_GOOD;
	}

	pwlcs->max_fbc = dhd_pwlcs_get_maxfbc(pwlcs);
	pwlcs->max_dbc = dhd_pwlcs_get_maxdbc(pwlcs);
	pwlcs->cur_bc = dhd_pwlcs_get_bc(pwlcs);

	if (pwlcs->max_dbc) {
	    if (pwlcs->cur_bc >= (pwlcs->max_fbc + pwlcs->max_dbc)) {
	        /* reset the boot count if max dbc is reached */
	        pwlcs->cur_bc = PWLCS_BC_GOOD;
	    }
	}

	if (pwlcs->cur_bc >= pwlcs->max_fbc) {
	    /* Card status is BAD */
	    PWLCS_PR("%s(%d): BAD Card\n", __FUNCTION__, unit);
	    status = PWLCS_BC_BAD;
	} else {
	    status = pwlcs->cur_bc;
	}

	/* Update the boot count */
	if ((pwlcs->cur_bc > pwlcs->max_fbc) && (pwlcs->max_dbc == 0)) {
	    /* No dbc, keep it permanantly in BAD state */
	    dhd_pwlcs_set_bc(pwlcs, pwlcs->cur_bc);
	} else {
	    dhd_pwlcs_set_bc(pwlcs, pwlcs->cur_bc + 1);
	}

	/* Card status is Good or Booting */
	PWLCS_PR("%s(%d): Card boot count [%d] status [%d]\n", __FUNCTION__, unit, pwlcs->cur_bc, status);
	return status;
}

/*
 * Reset boot count (called on successful card initialization)
 *
 * Return
 *  0 : Success
 * -ve: Failure
 */
int
dhd_pwlcs_reset_bc(int unit)
{
	pwlcs_cb_t *pwlcs = NULL;

	if (unit >= FWDER_MAX_UNIT) {
	    PWLCS_PR("%s: unit %d exceeds max %d\n", __FUNCTION__, unit, FWDER_MAX_UNIT);
	    return PWLCS_BC_GOOD;
	}

	pwlcs = &dhd_pwlcs_cb[unit];

	if (!pwlcs->enable) {
	    /* Nothing to do, Just return GOOD status */
	    PWLCS_PR("%s(%d) not enabled\n", __FUNCTION__, unit);
	    return PWLCS_BC_GOOD;
	}

	dhd_pwlcs_set_bc(pwlcs, PWLCS_BC_GOOD);

	return PWLCS_BC_GOOD;
}


#if defined(DSLCPE_PWLCS_TEST)
/* Test boot count
  - Cause insmod to crash using one of the three methods if enabled
    * read an non-implemented PCIe core address
    * Write an non-implemented PCIe core address
    * Access NULL pointer
  - Test can be enabled from CFE using "WLAN Feature" or
    PSP control key "pcie[X]_pwlcs_tst" 
*/
int
dhd_pwlcs_test_bc(int unit, struct pci_dev *pdev)
{
	char key[PWLCS_PSPKEY_MAX_SIZE];
	char *val;
	int test_cnt = 0;
	int cnt;
	uint32 badaddr = PWLCS_TEST_BAD_WLADDR;
	pwlcs_cb_t *pwlcs = NULL;

	PWLCS_PR("%s(%d)\n", __FUNCTION__, unit);

	if (unit >= FWDER_MAX_UNIT) {
	    PWLCS_PR("%s(%d) exceeds max [%d]\n", __FUNCTION__, unit, FWDER_MAX_UNIT);
	    return PWLCS_BC_GOOD;
	}

	pwlcs = &dhd_pwlcs_cb[unit];

	if (!pwlcs->enable) {
	    /* Nothing to do, Just return GOOD status */
	    PWLCS_PR("%s(%d) not enabled\n", __FUNCTION__, unit);
	    return PWLCS_BC_GOOD;
	}

	cnt = dhd_pwlcs_get_bc(pwlcs);

	/* Prepare the PSP key information */
	snprintf(key, sizeof(key), PWLCS_TEST_PSPKEY_FMT, unit);

	/* Get the key value from PSP */
	val = nvram_get(key);

	if (val) {
	    PWLCS_PR("%s: [%s] = [%s]\n", __FUNCTION__, key, val);
	    sscanf(val, "%d", &test_cnt);
	}

	/* Using unused bits 2,3 of wlan feature for test purpose
	 * -------------------------------
	 * |7|6|5|4|  3   |  2   | 1 | 0 |
	 * -------------------------------
	 * | | | | |PWLCS1|PWLCS0|MGF|NIC|
	 * -------------------------------
	 */
	if (((kerSysGetWlanFeature() & PWLCS_TEST_MASK) & PWLCS_TEST_WF_PCIE(unit)) ||
	    (test_cnt >= cnt)) {
		/* cause bad card condition until the fail count reaches test count */
		uint32 val;

		/* Read of Offset 0x1280 on the device should cause external abort */
		pci_read_config_dword(pdev, badaddr, &val);
		DHD_ERROR(("%s: pci_read_config_dword(0x%x) = 0x%x\r\n", __FUNCTION__, badaddr, val));

#if 0
		/* Write of Offset 0x1280 on the device should cause external abort */
		val = 0xBAD;
		pci_write_config_dword(pdev, badaddr, val);
		DHD_ERROR(("%s: pci_write_config_dword(0x%x) = 0x%x\r\n", __FUNCTION__, badaddr, val));

		/* Null pointer access */
		((dhd_dummyif_dev_priv_t*)0)->stats.rx_packets++;
		DHD_ERROR(("%s: (0x%p)\n", __FUNCTION__, &((dhd_dummyif_dev_priv_t*)0)->stats.rx_packets));
#endif
	}

	return 0;
}
#endif /* DSLCPE_PWLCS_TEST */

struct net_device *
dhd_pwlcs_register_dummyif(char *ifname, bool need_rtnl_lock)
{
	struct net_device *dev = NULL;
	int err;
	ASSERT(ifname);
	dev = alloc_etherdev(DHD_DUMMYIF_DEV_PRIV_SIZE);
	if (!dev) {
		DHD_ERROR(("%s: alloc wlif failed\n", __FUNCTION__));
		return NULL;
	}
	strncpy(dev->name, ifname, sizeof(dev->name));
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31))
	dev->hard_start_xmit = dhd_dummyif_start_xmit;
	dev->do_ioctl = dhd_dummyif_do_ioctl;
	dev->get_stats = dhd_dummyif_get_stats;
#else
	dev->netdev_ops = &netdev_dummyif_ops;
#endif
	if (need_rtnl_lock)
		err = register_netdev(dev);
	else
		err = register_netdevice(dev);
	if (err != 0) {
		DHD_ERROR(("%s, register_netdev failed for %s\n",
			__FUNCTION__, dev->name));
		free_netdev(dev);
		dev = NULL;
	} else {
		PWLCS_PR("%s: Successfully registered dummy if (%s)\n", __FUNCTION__, dev->name);
	}
	return dev;
}

void
dhd_pwlcs_unregister_dummyif(struct net_device *dev)
{
	if (dev) {
		unregister_netdev(dev);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
		free(dev->priv);
		free(dev);
#else
		free_netdev(dev);
#endif /* 2.6.24 */
	}
	return;
}

#endif /* DSLCPE_PWLCS */
