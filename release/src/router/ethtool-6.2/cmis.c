/**
 * Description:
 *
 * This module adds CMIS support to ethtool. The changes are similar to
 * the ones already existing in qsfp.c, but customized to use the memory
 * addresses and logic as defined in the specification's document.
 *
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include "internal.h"
#include "sff-common.h"
#include "cmis.h"
#include "netlink/extapi.h"

/* The maximum number of supported Banks. Relevant documents:
 * [1] CMIS Rev. 5, page. 128, section 8.4.4, Table 8-40
 */
#define CMIS_MAX_BANKS	4
#define CMIS_CHANNELS_PER_BANK	8
#define CMIS_MAX_CHANNEL_NUM	(CMIS_MAX_BANKS * CMIS_CHANNELS_PER_BANK)

/* We are not parsing further than Page 11h. */
#define CMIS_MAX_PAGES	18

struct cmis_memory_map {
	const __u8 *lower_memory;
	const __u8 *upper_memory[CMIS_MAX_BANKS][CMIS_MAX_PAGES];
#define page_00h upper_memory[0x0][0x0]
#define page_01h upper_memory[0x0][0x1]
#define page_02h upper_memory[0x0][0x2]
};

#define CMIS_PAGE_SIZE		0x80
#define CMIS_I2C_ADDRESS	0x50

static struct {
	const char *str;
	int offset;
	__u8 value;	/* Alarm is on if (offset & value) != 0. */
} cmis_aw_mod_flags[] = {
	{ "Module temperature high alarm",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_HALARM_STATUS },
	{ "Module temperature low alarm",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_LALARM_STATUS },
	{ "Module temperature high warning",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_HWARN_STATUS },
	{ "Module temperature low warning",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_LWARN_STATUS },

	{ "Module voltage high alarm",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_HALARM_STATUS },
	{ "Module voltage low alarm",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_LALARM_STATUS },
	{ "Module voltage high warning",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_HWARN_STATUS },
	{ "Module voltage low warning",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_LWARN_STATUS },

	{ NULL, 0, 0 },
};

static struct {
	const char *fmt_str;
	int offset;
	int adver_offset;	/* In Page 01h. */
	__u8 adver_value;	/* Supported if (offset & value) != 0. */
} cmis_aw_chan_flags[] = {
	{ "Laser bias current high alarm   (Chan %d)",
	  CMIS_TX_BIAS_AW_HALARM_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_BIAS_MON_MASK },
	{ "Laser bias current low alarm    (Chan %d)",
	  CMIS_TX_BIAS_AW_LALARM_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_BIAS_MON_MASK },
	{ "Laser bias current high warning (Chan %d)",
	  CMIS_TX_BIAS_AW_HWARN_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_BIAS_MON_MASK },
	{ "Laser bias current low warning  (Chan %d)",
	  CMIS_TX_BIAS_AW_LWARN_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_BIAS_MON_MASK },

	{ "Laser tx power high alarm   (Channel %d)",
	  CMIS_TX_PWR_AW_HALARM_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_PWR_MON_MASK },
	{ "Laser tx power low alarm    (Channel %d)",
	  CMIS_TX_PWR_AW_LALARM_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_PWR_MON_MASK },
	{ "Laser tx power high warning (Channel %d)",
	  CMIS_TX_PWR_AW_HWARN_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_PWR_MON_MASK },
	{ "Laser tx power low warning  (Channel %d)",
	  CMIS_TX_PWR_AW_LWARN_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_TX_PWR_MON_MASK },

	{ "Laser rx power high alarm   (Channel %d)",
	  CMIS_RX_PWR_AW_HALARM_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_RX_PWR_MON_MASK },
	{ "Laser rx power low alarm    (Channel %d)",
	  CMIS_RX_PWR_AW_LALARM_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_RX_PWR_MON_MASK },
	{ "Laser rx power high warning (Channel %d)",
	  CMIS_RX_PWR_AW_HWARN_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_RX_PWR_MON_MASK },
	{ "Laser rx power low warning  (Channel %d)",
	  CMIS_RX_PWR_AW_LWARN_OFFSET,
	  CMIS_DIAG_CHAN_ADVER_OFFSET, CMIS_RX_PWR_MON_MASK },

	{ NULL, 0, 0, 0 },
};

static void cmis_show_identifier(const struct cmis_memory_map *map)
{
	sff8024_show_identifier(map->lower_memory, CMIS_ID_OFFSET);
}

static void cmis_show_connector(const struct cmis_memory_map *map)
{
	sff8024_show_connector(map->page_00h, CMIS_CTOR_OFFSET);
}

static void cmis_show_oui(const struct cmis_memory_map *map)
{
	sff8024_show_oui(map->page_00h, CMIS_VENDOR_OUI_OFFSET);
}

/**
 * Print the revision compliance. Relevant documents:
 * [1] CMIS Rev. 3, pag. 45, section 1.7.2.1, Table 18
 * [2] CMIS Rev. 4, pag. 81, section 8.2.1, Table 8-2
 */
static void cmis_show_rev_compliance(const struct cmis_memory_map *map)
{
	__u8 rev = map->lower_memory[CMIS_REV_COMPLIANCE_OFFSET];
	int major = (rev >> 4) & 0x0F;
	int minor = rev & 0x0F;

	printf("\t%-41s : Rev. %d.%d\n", "Revision compliance", major, minor);
}

/**
 * Print information about the device's power consumption.
 * Relevant documents:
 * [1] CMIS Rev. 3, pag. 59, section 1.7.3.9, Table 30
 * [2] CMIS Rev. 4, pag. 94, section 8.3.9, Table 8-18
 * [3] QSFP-DD Hardware Rev 5.0, pag. 22, section 4.2.1
 */
static void cmis_show_power_info(const struct cmis_memory_map *map)
{
	float max_power = 0.0f;
	__u8 base_power = 0;
	__u8 power_class;

	/* Get the power class (first 3 most significat bytes) */
	power_class = (map->page_00h[CMIS_PWR_CLASS_OFFSET] >> 5) & 0x07;

	/* Get the base power in multiples of 0.25W */
	base_power = map->page_00h[CMIS_PWR_MAX_POWER_OFFSET];
	max_power = base_power * 0.25f;

	printf("\t%-41s : %d\n", "Power class", power_class + 1);
	printf("\t%-41s : %.02fW\n", "Max power", max_power);
}

/**
 * Print the cable assembly length, for both passive copper and active
 * optical or electrical cables. The base length (bits 5-0) must be
 * multiplied with the SMF length multiplier (bits 7-6) to obtain the
 * correct value. Relevant documents:
 * [1] CMIS Rev. 3, pag. 59, section 1.7.3.10, Table 31
 * [2] CMIS Rev. 4, pag. 94, section 8.3.10, Table 8-19
 */
static void cmis_show_cbl_asm_len(const struct cmis_memory_map *map)
{
	static const char *fn = "Cable assembly length";
	float mul = 1.0f;
	float val = 0.0f;

	/* Check if max length */
	if (map->page_00h[CMIS_CBL_ASM_LEN_OFFSET] == CMIS_6300M_MAX_LEN) {
		printf("\t%-41s : > 6.3km\n", fn);
		return;
	}

	/* Get the multiplier from the first two bits */
	switch (map->page_00h[CMIS_CBL_ASM_LEN_OFFSET] & CMIS_LEN_MUL_MASK) {
	case CMIS_MULTIPLIER_00:
		mul = 0.1f;
		break;
	case CMIS_MULTIPLIER_01:
		mul = 1.0f;
		break;
	case CMIS_MULTIPLIER_10:
		mul = 10.0f;
		break;
	case CMIS_MULTIPLIER_11:
		mul = 100.0f;
		break;
	default:
		break;
	}

	/* Get base value from first 6 bits and multiply by mul */
	val = (map->page_00h[CMIS_CBL_ASM_LEN_OFFSET] & CMIS_LEN_VAL_MASK);
	val = (float)val * mul;
	printf("\t%-41s : %0.2fm\n", fn, val);
}

/**
 * Print the length for SMF fiber. The base length (bits 5-0) must be
 * multiplied with the SMF length multiplier (bits 7-6) to obtain the
 * correct value. Relevant documents:
 * [1] CMIS Rev. 3, pag. 63, section 1.7.4.2, Table 39
 * [2] CMIS Rev. 4, pag. 99, section 8.4.2, Table 8-27
 */
static void cmis_print_smf_cbl_len(const struct cmis_memory_map *map)
{
	static const char *fn = "Length (SMF)";
	float mul = 1.0f;
	float val = 0.0f;

	if (!map->page_01h)
		return;

	/* Get the multiplier from the first two bits */
	switch (map->page_01h[CMIS_SMF_LEN_OFFSET] & CMIS_LEN_MUL_MASK) {
	case CMIS_MULTIPLIER_00:
		mul = 0.1f;
		break;
	case CMIS_MULTIPLIER_01:
		mul = 1.0f;
		break;
	default:
		break;
	}

	/* Get base value from first 6 bits and multiply by mul */
	val = (map->page_01h[CMIS_SMF_LEN_OFFSET] & CMIS_LEN_VAL_MASK);
	val = (float)val * mul;
	printf("\t%-41s : %0.2fkm\n", fn, val);
}

/**
 * Print relevant signal integrity control properties. Relevant documents:
 * [1] CMIS Rev. 3, pag. 71, section 1.7.4.10, Table 46
 * [2] CMIS Rev. 4, pag. 105, section 8.4.10, Table 8-34
 */
static void cmis_show_sig_integrity(const struct cmis_memory_map *map)
{
	if (!map->page_01h)
		return;

	/* CDR Bypass control: 2nd bit from each byte */
	printf("\t%-41s : ", "Tx CDR bypass control");
	printf("%s\n", YESNO(map->page_01h[CMIS_SIG_INTEG_TX_OFFSET] & 0x02));

	printf("\t%-41s : ", "Rx CDR bypass control");
	printf("%s\n", YESNO(map->page_01h[CMIS_SIG_INTEG_RX_OFFSET] & 0x02));

	/* CDR Implementation: 1st bit from each byte */
	printf("\t%-41s : ", "Tx CDR");
	printf("%s\n", YESNO(map->page_01h[CMIS_SIG_INTEG_TX_OFFSET] & 0x01));

	printf("\t%-41s : ", "Rx CDR");
	printf("%s\n", YESNO(map->page_01h[CMIS_SIG_INTEG_RX_OFFSET] & 0x01));
}

/**
 * Print relevant media interface technology info. Relevant documents:
 * [1] CMIS Rev. 3
 * --> pag. 61, section 1.7.3.14, Table 36
 * --> pag. 64, section 1.7.4.3, 1.7.4.4
 * [2] CMIS Rev. 4
 * --> pag. 97, section 8.3.14, Table 8-24
 * --> pag. 98, section 8.4, Table 8-25
 * --> page 100, section 8.4.3, 8.4.4
 */
static void cmis_show_mit_compliance(const struct cmis_memory_map *map)
{
	static const char *cc = " (Copper cable,";

	printf("\t%-41s : 0x%02x", "Transmitter technology",
	       map->page_00h[CMIS_MEDIA_INTF_TECH_OFFSET]);

	switch (map->page_00h[CMIS_MEDIA_INTF_TECH_OFFSET]) {
	case CMIS_850_VCSEL:
		printf(" (850 nm VCSEL)\n");
		break;
	case CMIS_1310_VCSEL:
		printf(" (1310 nm VCSEL)\n");
		break;
	case CMIS_1550_VCSEL:
		printf(" (1550 nm VCSEL)\n");
		break;
	case CMIS_1310_FP:
		printf(" (1310 nm FP)\n");
		break;
	case CMIS_1310_DFB:
		printf(" (1310 nm DFB)\n");
		break;
	case CMIS_1550_DFB:
		printf(" (1550 nm DFB)\n");
		break;
	case CMIS_1310_EML:
		printf(" (1310 nm EML)\n");
		break;
	case CMIS_1550_EML:
		printf(" (1550 nm EML)\n");
		break;
	case CMIS_OTHERS:
		printf(" (Others/Undefined)\n");
		break;
	case CMIS_1490_DFB:
		printf(" (1490 nm DFB)\n");
		break;
	case CMIS_COPPER_UNEQUAL:
		printf("%s unequalized)\n", cc);
		break;
	case CMIS_COPPER_PASS_EQUAL:
		printf("%s passive equalized)\n", cc);
		break;
	case CMIS_COPPER_NF_EQUAL:
		printf("%s near and far end limiting active equalizers)\n", cc);
		break;
	case CMIS_COPPER_F_EQUAL:
		printf("%s far end limiting active equalizers)\n", cc);
		break;
	case CMIS_COPPER_N_EQUAL:
		printf("%s near end limiting active equalizers)\n", cc);
		break;
	case CMIS_COPPER_LINEAR_EQUAL:
		printf("%s linear active equalizers)\n", cc);
		break;
	}

	if (map->page_00h[CMIS_MEDIA_INTF_TECH_OFFSET] >= CMIS_COPPER_UNEQUAL) {
		printf("\t%-41s : %udb\n", "Attenuation at 5GHz",
		       map->page_00h[CMIS_COPPER_ATT_5GHZ]);
		printf("\t%-41s : %udb\n", "Attenuation at 7GHz",
		       map->page_00h[CMIS_COPPER_ATT_7GHZ]);
		printf("\t%-41s : %udb\n", "Attenuation at 12.9GHz",
		       map->page_00h[CMIS_COPPER_ATT_12P9GHZ]);
		printf("\t%-41s : %udb\n", "Attenuation at 25.8GHz",
		       map->page_00h[CMIS_COPPER_ATT_25P8GHZ]);
	} else if (map->page_01h) {
		printf("\t%-41s : %.3lfnm\n", "Laser wavelength",
		       (((map->page_01h[CMIS_NOM_WAVELENGTH_MSB] << 8) |
			  map->page_01h[CMIS_NOM_WAVELENGTH_LSB]) * 0.05));
		printf("\t%-41s : %.3lfnm\n", "Laser wavelength tolerance",
		       (((map->page_01h[CMIS_WAVELENGTH_TOL_MSB] << 8) |
			  map->page_01h[CMIS_WAVELENGTH_TOL_LSB]) * 0.005));
	}
}

/**
 * Print relevant info about the maximum supported fiber media length
 * for each type of fiber media at the maximum module-supported bit rate.
 * Relevant documents:
 * [1] CMIS Rev. 3, page 64, section 1.7.4.2, Table 39
 * [2] CMIS Rev. 4, page 99, section 8.4.2, Table 8-27
 */
static void cmis_show_link_len(const struct cmis_memory_map *map)
{
	cmis_print_smf_cbl_len(map);
	if (!map->page_01h)
		return;
	sff_show_value_with_unit(map->page_01h, CMIS_OM5_LEN_OFFSET,
				 "Length (OM5)", 2, "m");
	sff_show_value_with_unit(map->page_01h, CMIS_OM4_LEN_OFFSET,
				 "Length (OM4)", 2, "m");
	sff_show_value_with_unit(map->page_01h, CMIS_OM3_LEN_OFFSET,
				 "Length (OM3 50/125um)", 2, "m");
	sff_show_value_with_unit(map->page_01h, CMIS_OM2_LEN_OFFSET,
				 "Length (OM2 50/125um)", 1, "m");
}

/**
 * Show relevant information about the vendor. Relevant documents:
 * [1] CMIS Rev. 3, page 56, section 1.7.3, Table 27
 * [2] CMIS Rev. 4, page 91, section 8.2, Table 8-15
 */
static void cmis_show_vendor_info(const struct cmis_memory_map *map)
{
	const char *clei;

	sff_show_ascii(map->page_00h, CMIS_VENDOR_NAME_START_OFFSET,
		       CMIS_VENDOR_NAME_END_OFFSET, "Vendor name");
	cmis_show_oui(map);
	sff_show_ascii(map->page_00h, CMIS_VENDOR_PN_START_OFFSET,
		       CMIS_VENDOR_PN_END_OFFSET, "Vendor PN");
	sff_show_ascii(map->page_00h, CMIS_VENDOR_REV_START_OFFSET,
		       CMIS_VENDOR_REV_END_OFFSET, "Vendor rev");
	sff_show_ascii(map->page_00h, CMIS_VENDOR_SN_START_OFFSET,
		       CMIS_VENDOR_SN_END_OFFSET, "Vendor SN");
	sff_show_ascii(map->page_00h, CMIS_DATE_YEAR_OFFSET,
		       CMIS_DATE_VENDOR_LOT_OFFSET + 1, "Date code");

	clei = (const char *)(map->page_00h + CMIS_CLEI_START_OFFSET);
	if (*clei && strncmp(clei, CMIS_CLEI_BLANK, CMIS_CLEI_LEN))
		sff_show_ascii(map->page_00h, CMIS_CLEI_START_OFFSET,
			       CMIS_CLEI_END_OFFSET, "CLEI code");
}

/* Print the current Module State. Relevant documents:
 * [1] CMIS Rev. 5, pag. 57, section 6.3.2.2, Figure 6-3
 * [2] CMIS Rev. 5, pag. 60, section 6.3.2.3, Figure 6-4
 * [3] CMIS Rev. 5, pag. 107, section 8.2.2, Table 8-6
 */
static void cmis_show_mod_state(const struct cmis_memory_map *map)
{
	__u8 mod_state;

	mod_state = (map->lower_memory[CMIS_MODULE_STATE_OFFSET] &
		     CMIS_MODULE_STATE_MASK) >> 1;
	printf("\t%-41s : 0x%02x", "Module State", mod_state);
	switch (mod_state) {
	case CMIS_MODULE_STATE_MODULE_LOW_PWR:
		printf(" (ModuleLowPwr)\n");
		break;
	case CMIS_MODULE_STATE_MODULE_PWR_UP:
		printf(" (ModulePwrUp)\n");
		break;
	case CMIS_MODULE_STATE_MODULE_READY:
		printf(" (ModuleReady)\n");
		break;
	case CMIS_MODULE_STATE_MODULE_PWR_DN:
		printf(" (ModulePwrDn)\n");
		break;
	case CMIS_MODULE_STATE_MODULE_FAULT:
		printf(" (ModuleFault)\n");
		break;
	default:
		printf(" (reserved or unknown)\n");
		break;
	}
}

/* Print the Module Fault Information. Relevant documents:
 * [1] CMIS Rev. 5, pag. 64, section 6.3.2.12
 * [2] CMIS Rev. 5, pag. 115, section 8.2.10, Table 8-15
 */
static void cmis_show_mod_fault_cause(const struct cmis_memory_map *map)
{
	__u8 mod_state, fault_cause;

	mod_state = (map->lower_memory[CMIS_MODULE_STATE_OFFSET] &
		     CMIS_MODULE_STATE_MASK) >> 1;
	if (mod_state != CMIS_MODULE_STATE_MODULE_FAULT)
		return;

	fault_cause = map->lower_memory[CMIS_MODULE_FAULT_OFFSET];
	printf("\t%-41s : 0x%02x", "Module Fault Cause", fault_cause);
	switch (fault_cause) {
	case CMIS_MODULE_FAULT_NO_FAULT:
		printf(" (No fault detected / not supported)\n");
		break;
	case CMIS_MODULE_FAULT_TEC_RUNAWAY:
		printf(" (TEC runaway)\n");
		break;
	case CMIS_MODULE_FAULT_DATA_MEM_CORRUPTED:
		printf(" (Data memory corrupted)\n");
		break;
	case CMIS_MODULE_FAULT_PROG_MEM_CORRUPTED:
		printf(" (Program memory corrupted)\n");
		break;
	default:
		printf(" (reserved or unknown)\n");
		break;
	}
}

/* Print the current Module-Level Controls. Relevant documents:
 * [1] CMIS Rev. 5, pag. 58, section 6.3.2.2, Table 6-12
 * [2] CMIS Rev. 5, pag. 111, section 8.2.6, Table 8-10
 */
static void cmis_show_mod_lvl_controls(const struct cmis_memory_map *map)
{
	printf("\t%-41s : ", "LowPwrAllowRequestHW");
	printf("%s\n", ONOFF(map->lower_memory[CMIS_MODULE_CONTROL_OFFSET] &
			     CMIS_LOW_PWR_ALLOW_REQUEST_HW_MASK));
	printf("\t%-41s : ", "LowPwrRequestSW");
	printf("%s\n", ONOFF(map->lower_memory[CMIS_MODULE_CONTROL_OFFSET] &
			     CMIS_LOW_PWR_REQUEST_SW_MASK));
}

static void cmis_parse_dom_power_type(const struct cmis_memory_map *map,
				      struct sff_diags *sd)
{
	sd->rx_power_type = map->page_01h[CMIS_DIAG_TYPE_OFFSET] &
			    CMIS_RX_PWR_TYPE_MASK;
	sd->tx_power_type = map->page_01h[CMIS_DIAG_CHAN_ADVER_OFFSET] &
			    CMIS_TX_PWR_MON_MASK;
}

static void cmis_parse_dom_mod_lvl_monitors(const struct cmis_memory_map *map,
					    struct sff_diags *sd)
{
	sd->sfp_voltage[MCURR] = OFFSET_TO_U16_PTR(map->lower_memory,
						   CMIS_CURR_VCC_OFFSET);
	sd->sfp_temp[MCURR] = (__s16)OFFSET_TO_U16_PTR(map->lower_memory,
						       CMIS_CURR_TEMP_OFFSET);
}

static void cmis_parse_dom_mod_lvl_thresh(const struct cmis_memory_map *map,
					  struct sff_diags *sd)
{
	/* Page is not present in IOCTL path. */
	if (!map->page_02h)
		return;
	sd->supports_alarms = 1;

	sd->sfp_voltage[HALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						   CMIS_VCC_HALRM_OFFSET);
	sd->sfp_voltage[LALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						   CMIS_VCC_LALRM_OFFSET);
	sd->sfp_voltage[HWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						   CMIS_VCC_HWARN_OFFSET);
	sd->sfp_voltage[LWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						   CMIS_VCC_LWARN_OFFSET);

	sd->sfp_temp[HALRM] = (__s16)OFFSET_TO_U16_PTR(map->page_02h,
						       CMIS_TEMP_HALRM_OFFSET);
	sd->sfp_temp[LALRM] = (__s16)OFFSET_TO_U16_PTR(map->page_02h,
						       CMIS_TEMP_LALRM_OFFSET);
	sd->sfp_temp[HWARN] = (__s16)OFFSET_TO_U16_PTR(map->page_02h,
						       CMIS_TEMP_HWARN_OFFSET);
	sd->sfp_temp[LWARN] = (__s16)OFFSET_TO_U16_PTR(map->page_02h,
						       CMIS_TEMP_LWARN_OFFSET);
}

static __u8 cmis_tx_bias_mul(const struct cmis_memory_map *map)
{
	switch (map->page_01h[CMIS_DIAG_CHAN_ADVER_OFFSET] &
		CMIS_TX_BIAS_MUL_MASK) {
	case CMIS_TX_BIAS_MUL_1:
		return 0;
	case CMIS_TX_BIAS_MUL_2:
		return 1;
	case CMIS_TX_BIAS_MUL_4:
		return 2;
	}

	return 0;
}

static void
cmis_parse_dom_chan_lvl_monitors_bank(const struct cmis_memory_map *map,
				      struct sff_diags *sd, int bank)
{
	const __u8 *page_11h = map->upper_memory[bank][0x11];
	int i;

	if (!page_11h)
		return;

	for (i = 0; i < CMIS_CHANNELS_PER_BANK; i++) {
		__u8 tx_bias_offset, rx_power_offset, tx_power_offset;
		int chan = bank * CMIS_CHANNELS_PER_BANK + i;
		__u8 bias_mul = cmis_tx_bias_mul(map);

		tx_bias_offset = CMIS_TX_BIAS_OFFSET + i * sizeof(__u16);
		rx_power_offset = CMIS_RX_PWR_OFFSET + i * sizeof(__u16);
		tx_power_offset = CMIS_TX_PWR_OFFSET + i * sizeof(__u16);

		sd->scd[chan].bias_cur = OFFSET_TO_U16_PTR(page_11h,
							   tx_bias_offset);
		sd->scd[chan].bias_cur >>= bias_mul;
		sd->scd[chan].rx_power = OFFSET_TO_U16_PTR(page_11h,
							   rx_power_offset);
		sd->scd[chan].tx_power = OFFSET_TO_U16_PTR(page_11h,
							   tx_power_offset);
	}
}

static void cmis_parse_dom_chan_lvl_monitors(const struct cmis_memory_map *map,
					     struct sff_diags *sd)
{
	int i;

	for (i = 0; i < CMIS_MAX_BANKS; i++)
		cmis_parse_dom_chan_lvl_monitors_bank(map, sd, i);
}

static void cmis_parse_dom_chan_lvl_thresh(const struct cmis_memory_map *map,
					   struct sff_diags *sd)
{
	__u8 bias_mul = cmis_tx_bias_mul(map);

	if (!map->page_02h)
		return;

	sd->bias_cur[HALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_BIAS_HALRM_OFFSET);
	sd->bias_cur[HALRM] >>= bias_mul;
	sd->bias_cur[LALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_BIAS_LALRM_OFFSET);
	sd->bias_cur[LALRM] >>= bias_mul;
	sd->bias_cur[HWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_BIAS_HWARN_OFFSET);
	sd->bias_cur[HWARN] >>= bias_mul;
	sd->bias_cur[LWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_BIAS_LWARN_OFFSET);
	sd->bias_cur[LWARN] >>= bias_mul;

	sd->tx_power[HALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_PWR_HALRM_OFFSET);
	sd->tx_power[LALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_PWR_LALRM_OFFSET);
	sd->tx_power[HWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_PWR_HWARN_OFFSET);
	sd->tx_power[LWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_TX_PWR_LWARN_OFFSET);

	sd->rx_power[HALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_RX_PWR_HALRM_OFFSET);
	sd->rx_power[LALRM] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_RX_PWR_LALRM_OFFSET);
	sd->rx_power[HWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_RX_PWR_HWARN_OFFSET);
	sd->rx_power[LWARN] = OFFSET_TO_U16_PTR(map->page_02h,
						CMIS_RX_PWR_LWARN_OFFSET);
}

static void cmis_parse_dom(const struct cmis_memory_map *map,
			   struct sff_diags *sd)
{
	cmis_parse_dom_power_type(map, sd);
	cmis_parse_dom_mod_lvl_monitors(map, sd);
	cmis_parse_dom_mod_lvl_thresh(map, sd);
	cmis_parse_dom_chan_lvl_monitors(map, sd);
	cmis_parse_dom_chan_lvl_thresh(map, sd);
}

/* Print module-level monitoring values. Relevant documents:
 * [1] CMIS Rev. 5, page 110, section 8.2.5, Table 8-9
 */
static void cmis_show_dom_mod_lvl_monitors(const struct sff_diags *sd)
{
	PRINT_TEMP("Module temperature", sd->sfp_temp[MCURR]);
	PRINT_VCC("Module voltage", sd->sfp_voltage[MCURR]);
}

/* Print channel Tx laser bias current. Relevant documents:
 * [1] CMIS Rev. 5, page 165, section 8.9.4, Table 8-79
 */
static void
cmis_show_dom_chan_lvl_tx_bias_bank(const struct cmis_memory_map *map,
				    const struct sff_diags *sd, int bank)
{
	const __u8 *page_11h = map->upper_memory[bank][0x11];
	int i;

	if (!page_11h)
		return;

	for (i = 0; i < CMIS_CHANNELS_PER_BANK; i++) {
		int chan = bank * CMIS_CHANNELS_PER_BANK + i;
		char fmt_str[80];

		snprintf(fmt_str, 80, "%s (Channel %d)",
			 "Laser tx bias current", chan + 1);
		PRINT_BIAS(fmt_str, sd->scd[chan].bias_cur);
	}
}

static void cmis_show_dom_chan_lvl_tx_bias(const struct cmis_memory_map *map,
					   const struct sff_diags *sd)
{
	int i;

	if(!(map->page_01h[CMIS_DIAG_CHAN_ADVER_OFFSET] &
	     CMIS_TX_BIAS_MON_MASK))
		return;

	for (i = 0; i < CMIS_MAX_BANKS; i++)
		cmis_show_dom_chan_lvl_tx_bias_bank(map, sd, i);
}

/* Print channel Tx average optical power. Relevant documents:
 * [1] CMIS Rev. 5, page 165, section 8.9.4, Table 8-79
 */
static void
cmis_show_dom_chan_lvl_tx_power_bank(const struct cmis_memory_map *map,
				     const struct sff_diags *sd, int bank)
{
	const __u8 *page_11h = map->upper_memory[bank][0x11];
	int i;

	if (!page_11h)
		return;

	for (i = 0; i < CMIS_CHANNELS_PER_BANK; i++) {
		int chan = bank * CMIS_CHANNELS_PER_BANK + i;
		char fmt_str[80];

		snprintf(fmt_str, 80, "%s (Channel %d)",
			 "Transmit avg optical power", chan + 1);
		PRINT_xX_PWR(fmt_str, sd->scd[chan].tx_power);
	}
}

static void cmis_show_dom_chan_lvl_tx_power(const struct cmis_memory_map *map,
					    const struct sff_diags *sd)
{
	int i;

	if (!sd->tx_power_type)
		return;

	for (i = 0; i < CMIS_MAX_BANKS; i++)
		cmis_show_dom_chan_lvl_tx_power_bank(map, sd, i);
}

/* Print channel Rx input optical power. Relevant documents:
 * [1] CMIS Rev. 5, page 165, section 8.9.4, Table 8-79
 */
static void
cmis_show_dom_chan_lvl_rx_power_bank(const struct cmis_memory_map *map,
				     const struct sff_diags *sd, int bank)
{
	const __u8 *page_11h = map->upper_memory[bank][0x11];
	int i;

	if (!page_11h)
		return;

	for (i = 0; i < CMIS_CHANNELS_PER_BANK; i++) {
		int chan = bank * CMIS_CHANNELS_PER_BANK + i;
		char *rx_power_str;
		char fmt_str[80];

		if (!sd->rx_power_type)
			rx_power_str = "Receiver signal OMA";
		else
			rx_power_str = "Rcvr signal avg optical power";

		snprintf(fmt_str, 80, "%s (Channel %d)", rx_power_str,
			 chan + 1);
		PRINT_xX_PWR(fmt_str, sd->scd[chan].rx_power);
	}
}

static void cmis_show_dom_chan_lvl_rx_power(const struct cmis_memory_map *map,
					    const struct sff_diags *sd)
{
	int i;

	if(!(map->page_01h[CMIS_DIAG_CHAN_ADVER_OFFSET] & CMIS_RX_PWR_MON_MASK))
		return;

	for (i = 0; i < CMIS_MAX_BANKS; i++)
		cmis_show_dom_chan_lvl_rx_power_bank(map, sd, i);
}

static void cmis_show_dom_chan_lvl_monitors(const struct cmis_memory_map *map,
					    const struct sff_diags *sd)
{
	cmis_show_dom_chan_lvl_tx_bias(map, sd);
	cmis_show_dom_chan_lvl_tx_power(map, sd);
	cmis_show_dom_chan_lvl_rx_power(map, sd);
}

/* Print module-level flags. Relevant documents:
 * [1] CMIS Rev. 5, page 109, section 8.2.4, Table 8-8
 */
static void cmis_show_dom_mod_lvl_flags(const struct cmis_memory_map *map)
{
	int i;

	for (i = 0; cmis_aw_mod_flags[i].str; i++) {
		printf("\t%-41s : %s\n", cmis_aw_mod_flags[i].str,
		       map->lower_memory[cmis_aw_mod_flags[i].offset] &
		       cmis_aw_mod_flags[i].value ? "On" : "Off");
	}
}

/* Print channel-level flags. Relevant documents:
 * [1] CMIS Rev. 5, page 162, section 8.9.3, Table 8-77
 * [1] CMIS Rev. 5, page 164, section 8.9.3, Table 8-78
 */
static void cmis_show_dom_chan_lvl_flags_chan(const struct cmis_memory_map *map,
					      int bank, int chan)
{
	const __u8 *page_11h = map->upper_memory[bank][0x11];
	int i;

	for (i = 0; cmis_aw_chan_flags[i].fmt_str; i++) {
		char str[80];

		if (!(map->page_01h[cmis_aw_chan_flags[i].adver_offset] &
		      cmis_aw_chan_flags[i].adver_value))
			continue;

		snprintf(str, 80, cmis_aw_chan_flags[i].fmt_str, chan + 1);
		printf("\t%-41s : %s\n", str,
		       page_11h[cmis_aw_chan_flags[i].offset] & chan ?
		       "On" : "Off");
	}
}

static void
cmis_show_dom_chan_lvl_flags_bank(const struct cmis_memory_map *map,
				  int bank)
{
	const __u8 *page_11h = map->upper_memory[bank][0x11];
	int i;

	if (!page_11h)
		return;

	for (i = 0; i < CMIS_CHANNELS_PER_BANK; i++) {
		int chan = bank * CMIS_CHANNELS_PER_BANK + i;

		cmis_show_dom_chan_lvl_flags_chan(map, bank, chan);
	}
}

static void cmis_show_dom_chan_lvl_flags(const struct cmis_memory_map *map)
{
	int i;

	for (i = 0; i < CMIS_MAX_BANKS; i++)
		cmis_show_dom_chan_lvl_flags_bank(map, i);
}


static void cmis_show_dom(const struct cmis_memory_map *map)
{
	struct sff_diags sd = {};

	/* Diagnostic information is only relevant when the module memory
	 * model is paged and not flat.
	 */
	if (map->lower_memory[CMIS_MEMORY_MODEL_OFFSET] &
	    CMIS_MEMORY_MODEL_MASK)
		return;

	cmis_parse_dom(map, &sd);

	cmis_show_dom_mod_lvl_monitors(&sd);
	cmis_show_dom_chan_lvl_monitors(map, &sd);
	cmis_show_dom_mod_lvl_flags(map);
	cmis_show_dom_chan_lvl_flags(map);
	if (sd.supports_alarms)
		sff_show_thresholds(sd);
}

static void cmis_show_all_common(const struct cmis_memory_map *map)
{
	cmis_show_identifier(map);
	cmis_show_power_info(map);
	cmis_show_connector(map);
	cmis_show_cbl_asm_len(map);
	cmis_show_sig_integrity(map);
	cmis_show_mit_compliance(map);
	cmis_show_link_len(map);
	cmis_show_vendor_info(map);
	cmis_show_rev_compliance(map);
	cmis_show_mod_state(map);
	cmis_show_mod_fault_cause(map);
	cmis_show_mod_lvl_controls(map);
	cmis_show_dom(map);
}

static void cmis_memory_map_init_buf(struct cmis_memory_map *map,
				     const __u8 *id)
{
	/* Lower Memory and Page 00h are always present.
	 *
	 * Offset into Upper Memory is between page size and twice the page
	 * size. Therefore, set the base address of each page to base address
	 * plus page size multiplied by the page number.
	 */
	map->lower_memory = id;
	map->page_00h = id;

	/* Page 01h is only present when the module memory model is paged and
	 * not flat.
	 */
	if (map->lower_memory[CMIS_MEMORY_MODEL_OFFSET] &
	    CMIS_MEMORY_MODEL_MASK)
		return;

	map->page_01h = id + CMIS_PAGE_SIZE;
}

void cmis_show_all_ioctl(const __u8 *id)
{
	struct cmis_memory_map map = {};

	cmis_memory_map_init_buf(&map, id);
	cmis_show_all_common(&map);
}

static void cmis_request_init(struct ethtool_module_eeprom *request, u8 bank,
			      u8 page, u32 offset)
{
	request->offset = offset;
	request->length = CMIS_PAGE_SIZE;
	request->page = page;
	request->bank = bank;
	request->i2c_address = CMIS_I2C_ADDRESS;
	request->data = NULL;
}

static int cmis_num_banks_get(const struct cmis_memory_map *map,
			      int *p_num_banks)
{
	switch (map->page_01h[CMIS_PAGES_ADVER_OFFSET] &
		CMIS_BANKS_SUPPORTED_MASK) {
	case CMIS_BANK_0_SUPPORTED:
		*p_num_banks = 1;
		break;
	case CMIS_BANK_0_1_SUPPORTED:
		*p_num_banks = 2;
		break;
	case CMIS_BANK_0_3_SUPPORTED:
		*p_num_banks = 4;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int
cmis_memory_map_init_pages(struct cmd_context *ctx,
			   struct cmis_memory_map *map)
{
	struct ethtool_module_eeprom request;
	int num_banks, i, ret;

	/* Lower Memory and Page 00h are always present.
	 *
	 * Offset into Upper Memory is between page size and twice the page
	 * size. Therefore, set the base address of each page to its base
	 * address minus page size.
	 */
	cmis_request_init(&request, 0, 0x0, 0);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;
	map->lower_memory = request.data;

	cmis_request_init(&request, 0, 0x0, CMIS_PAGE_SIZE);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;
	map->page_00h = request.data - CMIS_PAGE_SIZE;

	/* Pages 01h and 02h are only present when the module memory model is
	 * paged and not flat.
	 */
	if (map->lower_memory[CMIS_MEMORY_MODEL_OFFSET] &
	    CMIS_MEMORY_MODEL_MASK)
		return 0;

	cmis_request_init(&request, 0, 0x1, CMIS_PAGE_SIZE);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;
	map->page_01h = request.data - CMIS_PAGE_SIZE;

	cmis_request_init(&request, 0, 0x2, CMIS_PAGE_SIZE);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;
	map->page_02h = request.data - CMIS_PAGE_SIZE;

	/* Bank 0 of Page 11h provides lane-specific registers for the first 8
	 * lanes, and each additional Banks provides support for an additional
	 * 8 lanes. Only initialize supported Banks.
	 */
	ret = cmis_num_banks_get(map, &num_banks);
	if (ret < 0)
		return ret;

	for (i = 0; i < num_banks; i++) {
		cmis_request_init(&request, i, 0x11, CMIS_PAGE_SIZE);
		ret = nl_get_eeprom_page(ctx, &request);
		if (ret < 0)
			return ret;
		map->upper_memory[i][0x11] = request.data - CMIS_PAGE_SIZE;
	}

	return 0;
}

int cmis_show_all_nl(struct cmd_context *ctx)
{
	struct cmis_memory_map map = {};
	int ret;

	ret = cmis_memory_map_init_pages(ctx, &map);
	if (ret < 0)
		return ret;
	cmis_show_all_common(&map);

	return 0;
}
