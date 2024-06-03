
/******* VERSION 0 FORMAT **********/
#define BP_DDR_SPEED_MASK               0x1f
#define BP_DDR_SPEED_SHIFT              0
#define BP_DDR_SPEED_SAFE               0
#define BP_DDR_SPEED_400_6_6_6		1	/* DDR3-800E  */
#define BP_DDR_SPEED_533_7_7_7		2	/* DDR3-1066F */
#define BP_DDR_SPEED_533_8_8_8		3	/* DDR3-1066G */
#define BP_DDR_SPEED_667_9_9_9		4	/* DDR3-1333H */
#define BP_DDR_SPEED_667_10_10_10	5	/* DDR3-1333J */
#define BP_DDR_SPEED_800_10_10_10	6	/* DDR3-1600J */
#define BP_DDR_SPEED_800_11_11_11	7	/* DDR3-1600K */
#define BP_DDR_SPEED_1067_11_11_11	8	/* DDR3-2133K */
#define BP_DDR_SPEED_1067_12_12_12	9	/* DDR3-2133L */
#define BP_DDR_SPEED_1067_13_13_13	10	/* DDR3-2133M */
#define BP_DDR_SPEED_1067_14_14_14	11	/* DDR3-2133N */ 
#define BP_DDR_SPEED_933_10_10_10	12	/* DDR3-1866J */
#define BP_DDR_SPEED_933_11_11_11	13	/* DDR3-1866K */ 
#define BP_DDR_SPEED_933_12_12_12	14	/* DDR3-1866L */
#define BP_DDR_SPEED_933_13_13_13	15	/* DDR3-1866M */
#define BP_DDR_SPEED_1067_15_15_15	16	/* DDR4-2133P */
#define BP_DDR_SPEED_1067_16_16_16	17	/* DDR4-2133R */
#define BP_DDR_SPEED_1200_17_17_17	18	/* DDR4-2400T */
#define BP_DDR_SPEED_1333_18_18_18	19	/* DDR4-2666U */
#define BP_DDR_SPEED_1333_19_19_19	20	/* DDR4-2666V */
#define BP_DDR_SPEED_1600_22_22_22	21	/* DDR4-3200AA */
#define BP_DDR_SPEED_1467_21_21_21	22	/* DDR4-2933Y */
#define BP_DDR_SPEED_CUSTOM_1           27  /* customized freq 1000MHz */
#define BP_DDR_SPEED_CUSTOM_2           28
#define BP_DDR_SPEED_CUSTOM_3           29
#define BP_DDR_SPEED_CUSTOM_4           30

#define BP_DDR_DEVICE_WIDTH_MASK        0xe0
#define BP_DDR_DEVICE_WIDTH_SHIFT       5
#define BP_DDR_DEVICE_WIDTH_8           (0 << BP_DDR_DEVICE_WIDTH_SHIFT)
#define BP_DDR_DEVICE_WIDTH_16          (1 << BP_DDR_DEVICE_WIDTH_SHIFT)
#define BP_DDR_DEVICE_WIDTH_32          (2 << BP_DDR_DEVICE_WIDTH_SHIFT)

#define BP_DDR_TOTAL_SIZE_MASK          0xf00
#define BP_DDR_TOTAL_SIZE_SHIFT         8
#define BP_DDR_TOTAL_SIZE_64MB          (1 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_128MB         (2 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_256MB         (3 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_512MB         (4 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_1024MB        (5 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_2048MB        (6 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_4096MB        (7 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_8192MB        (8 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_16GB          (9 << BP_DDR_TOTAL_SIZE_SHIFT)

#define BP_DDR_SSC_CONFIG_MASK          0xf000
#define BP_DDR_SSC_CONFIG_SHIFT         12
#define BP_DDR_SSC_CONFIG_NONE          (0 << BP_DDR_SSC_CONFIG_SHIFT)
#define BP_DDR_SSC_CONFIG_1             (1 << BP_DDR_SSC_CONFIG_SHIFT)
#define BP_DDR_SSC_CONFIG_2             (2 << BP_DDR_SSC_CONFIG_SHIFT)
#define BP_DDR_SSC_CONFIG_CUSTOM        (3 << BP_DDR_SSC_CONFIG_SHIFT)

#define BP_DDR_TEMP_MASK                0x30000
#define BP_DDR_TEMP_SHIFT               16
#define BP_DDR_TEMP_NORMAL              (0 << BP_DDR_TEMP_SHIFT)
#define BP_DDR_TEMP_EXTENDED_SRT        (1 << BP_DDR_TEMP_SHIFT)
#define BP_DDR_TEMP_EXTENDED_ASR        (2 << BP_DDR_TEMP_SHIFT)

#define BP_DDR_TOTAL_WIDTH_MASK         0xc0000
#define BP_DDR_TOTAL_WIDTH_SHIFT        18
#define BP_DDR_TOTAL_WIDTH_16BIT        (0 << BP_DDR_TOTAL_WIDTH_SHIFT)
#define BP_DDR_TOTAL_WIDTH_32BIT        (1 << BP_DDR_TOTAL_WIDTH_SHIFT)
#define BP_DDR_TOTAL_WIDTH_8BIT         (2 << BP_DDR_TOTAL_WIDTH_SHIFT)

#define BP_DDR_TYPE_MASK                0x300000
#define BP_DDR_TYPE_SHIFT               20
#define BP_DDR_TYPE_DDR3                (0 << BP_DDR_TYPE_SHIFT)
#define BP_DDR_TYPE_DDR4                (1 << BP_DDR_TYPE_SHIFT)

#define BP_DDR_MCBSEL_FORMAT_MASK       0x7000000
#define BP_DDR_MCBSEL_FORMAT_SHIFT      24
#define BP_DDR_MCBSEL_FORMAT_VER0       (0 << BP_DDR_MCBSEL_FORMAT_SHIFT) /* DDR3/4 */
#define BP_DDR_MCBSEL_FORMAT_VER1       (1 << BP_DDR_MCBSEL_FORMAT_SHIFT) /* LPDDR4/5 */

/* Vtt termination settings. Vtt termination is required in the board design for
   Address and Control line to improve the signal integrity when it need to support
   DDR at high clock. For slow DDR, Vtt termination is not used. It can be no termination
   at all with direct connection through serial resistor from DDR chip to the PHY or
   with passive terminatin of a pull-up and pull-down resistors.

   In low-end 2 layer board design, Vtt usually is not used as DDR runs slow so PCB_2LAYER
   option is essetnially Vtt dsiabled. But Vtt option is not determined by the layer of PCB.
   Keep this PCB definition only for compatiblity reason and new design should use VTT setting.
 */

#if defined(BP_DDR_SUPPORT_2L_PCB)
#define BP_DDR_PCB_MASK                 0x20000000
#define BP_DDR_PCB_SHIFT                29
#define BP_DDR_PCB_MULTI_LAYER          (0 << BP_DDR_PCB_SHIFT)
#define BP_DDR_PCB_2LAYER               (1 << BP_DDR_PCB_SHIFT)
#endif

#if defined(BP_DDR_SUPPORT_VTT)
#define BP_DDR_VTT_MASK                 0x30000000
#define BP_DDR_VTT_SHIFT                28
#define BP_DDR_VTT_EN                   (0 << BP_DDR_VTT_SHIFT)  /* Vtt enabled */
#define BP_DDR_VTT_EN_DR                (3 << BP_DDR_VTT_SHIFT)  /* Vtt enabled with dual rank ddr support */
#define BP_DDR_VTT_DIS_NOTERM           (2 << BP_DDR_VTT_SHIFT)  /* Vtt disabled with no AC termination */
#if defined(BP_DDR_SUPPORT_VTT_DIS_PASVTERM)
#define BP_DDR_VTT_DIS_PASVTERM	        (1 << BP_DDR_VTT_SHIFT)  /* Vtt disabled with passive AC termination */
#endif
#endif

#define BP_DDR_CONFIG_MASK              (~(BP_DDR_CONFIG_DEBUG|BP_DDR_CONFIG_OVERRIDE))
#define BP_DDR_CONFIG_DEBUG             (1 << 30)
#define BP_DDR_CONFIG_OVERRIDE          (1 << 31)

/********* VERSION 1 FORMAT ***********/
#define BP1_DDR_SPEED_MASK       0x1f
#define BP1_DDR_SPEED_SHIFT      0
#define BP1_DDR_SPEED_SAFE       0
#define BP1_DDR_SPEED_1333_24_24_24  1   /* LPDDR4-1333 */
#define BP1_DDR_SPEED_1600_28_29_29  2   /* LPDDR4-1600 */
#define BP1_DDR_SPEED_1866_32_34_34  3   /* LPDDR4-1866 */
#define BP1_DDR_SPEED_2133_36_39_39  4   /* LPDDR4-2133 */
#define BP1_DDR_SPEED_1600_29_29_29  5   /* LPDDR5-1600 */
#define BP1_DDR_SPEED_1867_34_34_34  6   /* LPDDR5-1867 */
#define BP1_DDR_SPEED_2133_39_39_39  7   /* LPDDR5-2133 */
#define BP1_DDR_SPEED_2400_44_44_44  8   /* LPDDR5-2400 */
#define BP1_DDR_SPEED_2750_50_50_50  9   /* LPDDR5-2750 */
#define BP1_DDR_SPEED_3000_54_54_54  10  /* LPDDR5-3000 */
#define BP1_DDR_SPEED_3200_58_58_58  11  /* LPDDR5-3200 */

/* bits 5-7 */
#define BP1_DDR_WIDTH_MASK   0xe0
#define BP1_DDR_WIDTH_SHIFT  5
#define BP1_DDR_WIDTH_16BIT  (0 << BP1_DDR_WIDTH_SHIFT)
#define BP1_DDR_WIDTH_32BIT  (1 << BP1_DDR_WIDTH_SHIFT)
#define BP1_DDR_WIDTH_64BIT  (2 << BP1_DDR_WIDTH_SHIFT)

/* bits 8-11 */
#define BP1_DDR_TOTAL_SIZE_MASK     0xf00
#define BP1_DDR_TOTAL_SIZE_SHIFT        8
#define BP1_DDR_TOTAL_SIZE_2Gb      (1 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_4Gb      (2 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_8Gb      (3 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_12Gb     (4 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_16Gb      (5 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_24Gb      (6 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_32Gb      (7 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_48Gb      (8 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_64Gb      (9 << BP1_DDR_TOTAL_SIZE_SHIFT)
#define BP1_DDR_TOTAL_SIZE_96Gb      (10 << BP1_DDR_TOTAL_SIZE_SHIFT)

/* bits 12-13 */
#define BP1_DDR_SSC_CONFIG_MASK     0x3000
#define BP1_DDR_SSC_CONFIG_SHIFT        12
#define BP1_DDR_SSC_CONFIG_NONE     (0 << BP1_DDR_SSC_CONFIG_SHIFT)
#define BP1_DDR_SSC_CONFIG_1            (1 << BP1_DDR_SSC_CONFIG_SHIFT)
#define BP1_DDR_SSC_CONFIG_2            (2 << BP1_DDR_SSC_CONFIG_SHIFT)
#define BP1_DDR_SSC_CONFIG_CUSTOM   (3 << BP1_DDR_SSC_CONFIG_SHIFT)

/* bits 14-15 */
#define BP1_DDR_TYPE_MASK           0xc000
#define BP1_DDR_TYPE_SHIFT          14
#define BP1_DDR_TYPE_LPDDR4         (0 << BP1_DDR_TYPE_SHIFT)
#define BP1_DDR_TYPE_LPDDR5         (1 << BP1_DDR_TYPE_SHIFT)
#define BP1_DDR_TYPE_LPDDR4X        (2 << BP1_DDR_TYPE_SHIFT)
#define BP1_DDR_TYPE_LPDDR5X        (3 << BP1_DDR_TYPE_SHIFT)

/* bit 16 */
#define BP1_DDR_RANK_MASK           0x10000
#define BP1_DDR_RANK_SHIFT          16
#define BP1_DDR_SINGLE_RANK         (0 << BP1_DDR_RANK_SHIFT)
#define BP1_DDR_DUAL_RANK           (1 << BP1_DDR_RANK_SHIFT)

/* bits 17-18 */
#define BP1_DDR_EXT_TEMP_MASK       0x60000
#define BP1_DDR_EXT_TEMP_SHIFT      17
#define BP1_DDR_1X_REFRESH          (0 << BP1_DDR_EXT_TEMP_SHIFT)
#define BP1_DDR_05X_REFRESH         (1 << BP1_DDR_EXT_TEMP_SHIFT)
#define BP1_DDR_025X_REFRESH        (2 << BP1_DDR_EXT_TEMP_SHIFT)

/* bits 24-26 */
#define BP1_DDR_MCBSEL_FORMAT_MASK  0x7000000
#define BP1_DDR_MCBSEL_FORMAT_SHIFT 24
#define BP1_DDR_MCBSEL_FORMAT_VER0  (0 << BP1_DDR_MCBSEL_FORMAT_SHIFT) /* DDR3/4 */
#define BP1_DDR_MCBSEL_FORMAT_VER1  (1 << BP1_DDR_MCBSEL_FORMAT_SHIFT) /* LPDDR4/5 */

/* bit 30 */
#define BP1_DDR_CONFIG_DEBUG            BP_DDR_CONFIG_DEBUG
/* bit 31 */
#define BP1_DDR_CONFIG_OVERRIDE     BP_DDR_CONFIG_OVERRIDE
#define BP1_DDR_CONFIG_MASK         (~(BP1_DDR_CONFIG_DEBUG|BP1_DDR_CONFIG_OVERRIDE))

