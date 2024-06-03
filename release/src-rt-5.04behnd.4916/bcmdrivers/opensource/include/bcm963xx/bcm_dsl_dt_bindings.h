#if !defined(_BCM_DSL_DT_BINDINGS_H_)
#define _BCM_DSL_DT_BINDINGS_H_

#define DSL_AFE_DEFAULT                  0

#define DSL_AFE_CHIP_INT                 (1 << 28)
#define DSL_AFE_CHIP_6505                (2 << 28)
#define DSL_AFE_CHIP_6306                (3 << 28)
#define DSL_AFE_CHIP_CH0                 (4 << 28)
#define DSL_AFE_CHIP_CH1                 (5 << 28)
#define DSL_AFE_CHIP_GFAST               (6 << 28)
#define DSL_AFE_CHIP_GFAST0              (6 << 28)
#define DSL_AFE_CHIP_GFAST_CH0           (7 << 28)
#define DSL_AFE_CHIP_GFAST1              (8 << 28)
#define DSL_AFE_CHIP_GFAST_CH1           (9 << 28)

#define DSL_AFE_LD_ISIL1556              (1 << 21)
#define DSL_AFE_LD_6301                  (2 << 21)
#define DSL_AFE_LD_6302                  (3 << 21)
#define DSL_AFE_LD_6303                  (4 << 21)
#define DSL_AFE_LD_6304                  (5 << 21)
#define DSL_AFE_LD_6305                  (6 << 21)
#define DSL_AFE_LD_6307                  (7 << 21)

#define DSL_AFE_LD_REV_6303_VR5P3        (1 << 18)

#define DSL_AFE_FE_ANNEXA                (1 << 15)
#define DSL_AFE_FE_ANNEXB                (2 << 15)
#define DSL_AFE_FE_ANNEXJ                (3 << 15)
#define DSL_AFE_FE_ANNEXBJ               (4 << 15)
#define DSL_AFE_FE_ANNEXM                (5 << 15)
#define DSL_AFE_FE_ANNEXC                (6 << 15)

#define DSL_AFE_FE_AVMODE_COMBO          (0 << 13)
#define DSL_AFE_FE_AVMODE_ADSL           (1 << 13)
#define DSL_AFE_FE_AVMODE_VDSL           (2 << 13)

/* VDSL only */
#define DSL_AFE_FE_REV_ISIL_REV1         (1 << 8)
#define DSL_AFE_FE_REV_12_20             DSL_AFE_FE_REV_ISIL_REV1
#define DSL_AFE_FE_REV_12_21             (2 << 8)

/* Combo */
#define DSL_AFE_FE_REV_6302_REV1         (1 << 8)
#define DSL_AFE_FE_REV_6302_REV_7_12     (1 << 8)
#define DSL_AFE_FE_REV_6302_REV_7_2_21   (2 << 8)

#define DSL_AFE_FE_REV_6302_REV_7_2_1    (3 << 8)
#define DSL_AFE_FE_REV_6302_REV_7_2      (4 << 8)
#define DSL_AFE_FE_REV_6302_REV_7_2_UR2  (5 << 8)
#define DSL_AFE_FE_REV_6302_REV_7_2_2    (6 << 8)
#define DSL_AFE_FE_REV_6302_REV_7_2_30    (7 << 8)
#define DSL_AFE_6302_6306_REV_A_12_40    (8 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_30    (9 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_20    (1 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_40    (1 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_60    (1 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_50    (2 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_35    (3 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_70    (3 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_75    (4 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_50      (1 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_51      (2 << 8)
#define DSL_AFE_FE_REV_6304_REV_12_4_40      (1 << 8)
#define DSL_AFE_FE_REV_6304_REV_12_4_45      (2 << 8)
#define DSL_AFE_FE_REV_6304_REV_12_4_60      (1 << 8)
#define DSL_AFE_FE_REV_6305_REV_12_5_60_1    (1 << 8)
#define DSL_AFE_FE_REV_6305_REV_12_5_60_2    (2 << 8)
#define DSL_AFE_FE_REV_6304_REV_12_4_80      (4 << 8)
#define DSL_AFE_FE_REV_6305_REV_12_5_80      (4 << 8)
#define DSL_AFE_FE_REV_6303_146__REV_12_3_80 (3 << 8)
#define DSL_AFE_FE_REV_6303_146__REV_12_3_85 (4 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_82      (5 << 8)
#define DSL_AFE_FE_REV_6303_REV_12_3_72      (8 << 8)
#define DSL_AFE_FE_REV_6307_REV_12_7_60      (1 << 8)
#define DSL_AFE_FE_REV_6307_REV_12_7_60_1    (1 << 8)
#define DSL_AFE_FE_REV_6307_REV_12_7_60_2    (2 << 8)
#define DSL_AFE_FE_REV_6307_REV_12_7_63_1    (5 << 8)
#define DSL_AFE_FE_REV_6307_REV_12_7_63_2    (6 << 8)


/* ADSL only*/
#define DSL_AFE_FE_REV_6302_REV_5_2_1    (1 << 8)
#define DSL_AFE_FE_REV_6302_REV_5_2_2    (2 << 8)
#define DSL_AFE_FE_REV_6302_REV_5_2_3    (3 << 8)
#define DSL_AFE_FE_REV_6301_REV_5_1_1    (1 << 8)
#define DSL_AFE_FE_REV_6301_REV_5_1_2    (2 << 8)
#define DSL_AFE_FE_REV_6301_REV_5_1_3    (3 << 8)
#define DSL_AFE_FE_REV_6301_REV_5_1_4    (4 << 8)

#define DSL_AFE_FE_COAX                  (1 << 7)

#define DSL_AFE_FE_RNC                   (1 << 6)
#define DSL_AFE_FE_8dBm                  (1 << 5)
#define DSL_AFE_FE_AVG                   (1 << 4)

#define DSL_AFE_VDSLCTL_0                0
#define DSL_AFE_VDSLCTL_1                1
#define DSL_AFE_VDSLCTL_2                2
#define DSL_AFE_VDSLCTL_3                3
#define DSL_AFE_VDSLCTL_4                4
#define DSL_AFE_VDSLCTL_5                5

#endif
