#ifndef CMIS_H__
#define CMIS_H__

/* Identifier and revision compliance (Page 0) */
#define CMIS_ID_OFFSET				0x00
#define CMIS_REV_COMPLIANCE_OFFSET		0x01
#define CMIS_MEMORY_MODEL_OFFSET		0x02
#define CMIS_MEMORY_MODEL_MASK			0x80

/* Global Status Information (Page 0) */
#define CMIS_MODULE_STATE_OFFSET		0x03
#define CMIS_MODULE_STATE_MASK			0x0E
#define CMIS_MODULE_STATE_MODULE_LOW_PWR	0x01
#define CMIS_MODULE_STATE_MODULE_PWR_UP		0x02
#define CMIS_MODULE_STATE_MODULE_READY		0x03
#define CMIS_MODULE_STATE_MODULE_PWR_DN		0x04
#define CMIS_MODULE_STATE_MODULE_FAULT		0x05

/* Module Flags (Page 0) */
#define CMIS_VCC_AW_OFFSET			0x09
#define CMIS_VCC_LWARN_STATUS			0x80
#define CMIS_VCC_HWARN_STATUS			0x40
#define CMIS_VCC_LALARM_STATUS			0x20
#define CMIS_VCC_HALARM_STATUS			0x10
#define CMIS_TEMP_AW_OFFSET			0x09
#define CMIS_TEMP_LWARN_STATUS			0x08
#define CMIS_TEMP_HWARN_STATUS			0x04
#define CMIS_TEMP_LALARM_STATUS			0x02
#define CMIS_TEMP_HALARM_STATUS			0x01

#define CMIS_MODULE_TYPE_OFFSET			0x55
#define CMIS_MT_MMF				0x01
#define CMIS_MT_SMF				0x02

/* Module-Level Monitors (Page 0) */
#define CMIS_CURR_TEMP_OFFSET			0x0E
#define CMIS_CURR_VCC_OFFSET			0x10

/* Module Global Controls (Page 0) */
#define CMIS_MODULE_CONTROL_OFFSET		0x1A
#define CMIS_LOW_PWR_ALLOW_REQUEST_HW_MASK	0x40
#define CMIS_LOW_PWR_REQUEST_SW_MASK		0x10

/* Module Fault Information (Page 0) */
#define CMIS_MODULE_FAULT_OFFSET		0x29
#define CMIS_MODULE_FAULT_NO_FAULT		0x00
#define CMIS_MODULE_FAULT_TEC_RUNAWAY		0x01
#define CMIS_MODULE_FAULT_DATA_MEM_CORRUPTED	0x02
#define CMIS_MODULE_FAULT_PROG_MEM_CORRUPTED	0x03

#define CMIS_CTOR_OFFSET			0xCB

/* Vendor related information (Page 0) */
#define CMIS_VENDOR_NAME_START_OFFSET		0x81
#define CMIS_VENDOR_NAME_END_OFFSET		0x90

#define CMIS_VENDOR_OUI_OFFSET			0x91

#define CMIS_VENDOR_PN_START_OFFSET		0x94
#define CMIS_VENDOR_PN_END_OFFSET		0xA3

#define CMIS_VENDOR_REV_START_OFFSET		0xA4
#define CMIS_VENDOR_REV_END_OFFSET		0xA5

#define CMIS_VENDOR_SN_START_OFFSET		0xA6
#define CMIS_VENDOR_SN_END_OFFSET		0xB5

#define CMIS_DATE_YEAR_OFFSET			0xB6
#define CMIS_DATE_VENDOR_LOT_OFFSET		0xBC

/* CLEI Code (Page 0) */
#define CMIS_CLEI_START_OFFSET			0xBE
#define CMIS_CLEI_END_OFFSET			0xC7
#define CMIS_CLEI_BLANK				"          "
#define CMIS_CLEI_LEN				0x0A

/* Cable assembly length */
#define CMIS_CBL_ASM_LEN_OFFSET			0xCA
#define CMIS_6300M_MAX_LEN			0xFF

/* Cable length with multiplier */
#define CMIS_MULTIPLIER_00			0x00
#define CMIS_MULTIPLIER_01			0x40
#define CMIS_MULTIPLIER_10			0x80
#define CMIS_MULTIPLIER_11			0xC0
#define CMIS_LEN_MUL_MASK			0xC0
#define CMIS_LEN_VAL_MASK			0x3F

/* Module power characteristics */
#define CMIS_PWR_CLASS_OFFSET			0xC8
#define CMIS_PWR_MAX_POWER_OFFSET		0xC9
#define CMIS_PWR_CLASS_MASK			0xE0
#define CMIS_PWR_CLASS_1			0x00
#define CMIS_PWR_CLASS_2			0x01
#define CMIS_PWR_CLASS_3			0x02
#define CMIS_PWR_CLASS_4			0x03
#define CMIS_PWR_CLASS_5			0x04
#define CMIS_PWR_CLASS_6			0x05
#define CMIS_PWR_CLASS_7			0x06
#define CMIS_PWR_CLASS_8			0x07

/* Copper cable attenuation */
#define CMIS_COPPER_ATT_5GHZ			0xCC
#define CMIS_COPPER_ATT_7GHZ			0xCD
#define CMIS_COPPER_ATT_12P9GHZ			0xCE
#define CMIS_COPPER_ATT_25P8GHZ			0xCF

/* Cable assembly lane */
#define CMIS_CABLE_ASM_NEAR_END_OFFSET		0xD2
#define CMIS_CABLE_ASM_FAR_END_OFFSET		0xD3

/* Media interface technology */
#define CMIS_MEDIA_INTF_TECH_OFFSET		0xD4
#define CMIS_850_VCSEL				0x00
#define CMIS_1310_VCSEL				0x01
#define CMIS_1550_VCSEL				0x02
#define CMIS_1310_FP				0x03
#define CMIS_1310_DFB				0x04
#define CMIS_1550_DFB				0x05
#define CMIS_1310_EML				0x06
#define CMIS_1550_EML				0x07
#define CMIS_OTHERS				0x08
#define CMIS_1490_DFB				0x09
#define CMIS_COPPER_UNEQUAL			0x0A
#define CMIS_COPPER_PASS_EQUAL			0x0B
#define CMIS_COPPER_NF_EQUAL			0x0C
#define CMIS_COPPER_F_EQUAL			0x0D
#define CMIS_COPPER_N_EQUAL			0x0E
#define CMIS_COPPER_LINEAR_EQUAL		0x0F

/*-----------------------------------------------------------------------
 * Upper Memory Page 0x01: contains advertising fields that define properties
 * that are unique to active modules and cable assemblies.
 * GlobalOffset = 2 * 0x80 + LocalOffset
 */

/* Supported Link Length (Page 1) */
#define CMIS_SMF_LEN_OFFSET			0x84
#define CMIS_OM5_LEN_OFFSET			0x85
#define CMIS_OM4_LEN_OFFSET			0x86
#define CMIS_OM3_LEN_OFFSET			0x87
#define CMIS_OM2_LEN_OFFSET			0x88

/* Wavelength (Page 1) */
#define CMIS_NOM_WAVELENGTH_MSB			0x8A
#define CMIS_NOM_WAVELENGTH_LSB			0x8B
#define CMIS_WAVELENGTH_TOL_MSB			0x8C
#define CMIS_WAVELENGTH_TOL_LSB			0x8D

/* Supported Pages Advertising (Page 1) */
#define CMIS_PAGES_ADVER_OFFSET			0x8E
#define CMIS_BANKS_SUPPORTED_MASK		0x03
#define CMIS_BANK_0_SUPPORTED			0x00
#define CMIS_BANK_0_1_SUPPORTED			0x01
#define CMIS_BANK_0_3_SUPPORTED			0x02

/* Module Characteristics Advertising (Page 1) */
#define CMIS_DIAG_TYPE_OFFSET			0x97
#define CMIS_RX_PWR_TYPE_MASK			0x10

/* Supported Monitors Advertisement (Page 1) */
#define CMIS_DIAG_CHAN_ADVER_OFFSET		0xA0
#define CMIS_TX_BIAS_MON_MASK			0x01
#define CMIS_TX_PWR_MON_MASK			0x02
#define CMIS_RX_PWR_MON_MASK			0x04
#define CMIS_TX_BIAS_MUL_MASK			0x18
#define CMIS_TX_BIAS_MUL_1			0x00
#define CMIS_TX_BIAS_MUL_2			0x08
#define CMIS_TX_BIAS_MUL_4			0x10

/* Signal integrity controls */
#define CMIS_SIG_INTEG_TX_OFFSET		0xA1
#define CMIS_SIG_INTEG_RX_OFFSET		0xA2

/*-----------------------------------------------------------------------
 * Upper Memory Page 0x02: Optional Page that informs about module-defined
 * thresholds for module-level and lane-specific threshold crossing monitors.
 */

/* Module-Level Monitor Thresholds (Page 2) */
#define CMIS_TEMP_HALRM_OFFSET			0x80
#define CMIS_TEMP_LALRM_OFFSET			0x82
#define CMIS_TEMP_HWARN_OFFSET			0x84
#define CMIS_TEMP_LWARN_OFFSET			0x86
#define CMIS_VCC_HALRM_OFFSET			0x88
#define CMIS_VCC_LALRM_OFFSET			0x8A
#define CMIS_VCC_HWARN_OFFSET			0x8C
#define CMIS_VCC_LWARN_OFFSET			0x8E

/* Lane-Related Monitor Thresholds (Page 2) */
#define CMIS_TX_PWR_HALRM_OFFSET		0xB0
#define CMIS_TX_PWR_LALRM_OFFSET		0xB2
#define CMIS_TX_PWR_HWARN_OFFSET		0xB4
#define CMIS_TX_PWR_LWARN_OFFSET		0xB6
#define CMIS_TX_BIAS_HALRM_OFFSET		0xB8
#define CMIS_TX_BIAS_LALRM_OFFSET		0xBA
#define CMIS_TX_BIAS_HWARN_OFFSET		0xBC
#define CMIS_TX_BIAS_LWARN_OFFSET		0xBE
#define CMIS_RX_PWR_HALRM_OFFSET		0xC0
#define CMIS_RX_PWR_LALRM_OFFSET		0xC2
#define CMIS_RX_PWR_HWARN_OFFSET		0xC4
#define CMIS_RX_PWR_LWARN_OFFSET		0xC6

/*-----------------------------------------------------------------------
 * Upper Memory Page 0x11: Optional Page that contains lane dynamic status
 * bytes.
 */

/* Media Lane-Specific Flags (Page 0x11) */
#define CMIS_TX_PWR_AW_HALARM_OFFSET		0x8B
#define CMIS_TX_PWR_AW_LALARM_OFFSET		0x8C
#define CMIS_TX_PWR_AW_HWARN_OFFSET		0x8D
#define CMIS_TX_PWR_AW_LWARN_OFFSET		0x8E
#define CMIS_TX_BIAS_AW_HALARM_OFFSET		0x8F
#define CMIS_TX_BIAS_AW_LALARM_OFFSET		0x90
#define CMIS_TX_BIAS_AW_HWARN_OFFSET		0x91
#define CMIS_TX_BIAS_AW_LWARN_OFFSET		0x92
#define CMIS_RX_PWR_AW_HALARM_OFFSET		0x95
#define CMIS_RX_PWR_AW_LALARM_OFFSET		0x96
#define CMIS_RX_PWR_AW_HWARN_OFFSET		0x97
#define CMIS_RX_PWR_AW_LWARN_OFFSET		0x98

/* Media Lane-Specific Monitors (Page 0x11) */
#define CMIS_TX_PWR_OFFSET			0x9A
#define CMIS_TX_BIAS_OFFSET			0xAA
#define CMIS_RX_PWR_OFFSET			0xBA

#define YESNO(x) (((x) != 0) ? "Yes" : "No")
#define ONOFF(x) (((x) != 0) ? "On" : "Off")

void cmis_show_all_ioctl(const __u8 *id);

int cmis_show_all_nl(struct cmd_context *ctx);

#endif /* CMIS_H__ */
