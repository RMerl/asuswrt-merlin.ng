/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
* 
*    Copyright (c) 2023 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

#ifndef __BCM_VOICE_DT_BINDINGS_H__
#define __BCM_VOICE_DT_BINDINGS_H__

#define VD_NOT_CONNECTED                        0
/* Return codes. */
#define VD_SUCCESS                              0
#define VD_BOARD_ID_NOT_FOUND                   1
#define VD_VALUE_NOT_DEFINED                    2
#define VD_BOARD_ID_NOT_SET                     3
#define VD_MAX_CHANNELS_EXCEEDED                4
#define VD_NO_ZSI_ON_BOARD_ERR                  5
#define VD_MAX_ITEM_EXCEEDED                    6
#define VD_SUCCESS_LAST                         7
#define VD_INTERRUPT_ALREADY_DEFINED            8
#define VD_GPIO_ALREADY_USED_AS_INTERRUPT       9
#define VD_INVALID_INTERRUPT                    10
#define VD_INVALID_GPIO                         11
#define VD_INVALID_INTF                         12
#define VD_CHANNELS_NOT_DEFINED                 13

#define VD_NOT_DEFINED                          0xffff


/* General-purpose flag definitions (rename as appropriate) */
#define FLAG_DSP_APMHAL_ENABLE           (1 << 0)
#define FLAG_DSP_PCMHAL_ENABLE           (1 << 1)
#define FLAG_ISI_SUPPORT                 (1 << 2)
#define FLAG_ZSI_SUPPORT                 (1 << 3)
#define FLAG_THALASSA_SUPPORT            (1 << 4)
#define FLAG_DAISY_CHAIN                 (1 << 5)
#define FLAG_MODNAME_TESTNAME5           (1 << 6)
#define FLAG_MODNAME_TESTNAME6           (1 << 7)
#define FLAG_MODNAME_TESTNAME7           (1 << 8)
#define FLAG_MODNAME_TESTNAME8           (1 << 9)

/*
** Device-specific definitions
*/

#define  VD_NONE                        (-1)
#define  VD_IDECT1                       0 /* Do not move this around, otherwise rebuild dect_driver.bin */
#define  VD_EDECT1                       1
#define  VD_SILABS_3050                  2 /* SILAB IDs till VD_SILABS_MAXID. Do NOT mess up Zarlink and Silab IDs. Put SILAB IDs before Zarlink's always*/
#define  VD_SILABS_3215                  3
#define  VD_SILABS_3216                  4
#define  VD_SILABS_3217                  5
#define  VD_SILABS_32172_ISI             6
#define  VD_SILABS_32173_ISI             7
#define  VD_SILABS_32176                 8
#define  VD_SILABS_32178                 9
#define  VD_SILABS_3226                  10
#define  VD_SILABS_32260                 11
#define  VD_SILABS_32261                 12
#define  VD_SILABS_32267                 13
#define  VD_SILABS_3239                  14
#define  VD_SILABS_32392                 15
#define  VD_SILABS_32182_ISI             16
#define  VD_SILABS_32184                 17
#define  VD_SILABS_32280_ISI             18
#define  VD_SILABS_32281_ISI             19
#define  VD_SILABS_32282                 20
#define  VD_SILABS_32284_ISI             21
#define  VD_SILABS_32286                 22
#define  VD_SILABS_32192_ISI             23
#define  VD_SILABS_32291_ISI             24
#define  VD_SILABS_32293                 25
#define  VD_SILABS_32295_ISI             26
#define  VD_SILABS_32297                 27
#define  VD_SILABS_MAXID                 28
                                         
/* ZARLINK IDs till VD_MAX */           
#define  VD_ZARLINK_88010                50
#define  VD_ZARLINK_88221                51
#define  VD_ZARLINK_88266                52
#define  VD_ZARLINK_88276                53
#define  VD_ZARLINK_88506                54
#define  VD_ZARLINK_88536                55
#define  VD_ZARLINK_88264                56
#define  VD_ZARLINK_89010                57
#define  VD_ZARLINK_89116                58
#define  VD_ZARLINK_89316                59
#define  VD_ZARLINK_9530                 60
#define  VD_ZARLINK_9540                 61
#define  VD_ZARLINK_9541                 62
#define  VD_ZARLINK_89136                63
#define  VD_ZARLINK_89336                64
#define  VD_ZARLINK_88601                65
#define  VD_ZARLINK_88701                66
#define  VD_ZARLINK_88702_ZSI            67
#define  VD_ZARLINK_9662                 68
#define  VD_ZARLINK_9661                 69
#define  VD_ZARLINK_88801                70
#define  VD_ZARLINK_9672_ZSI             71
#define  VD_ZARLINK_9642_ZSI             72
#define  VD_ZARLINK_9642_ZSI_IB          73
#define  VD_ZARLINK_9642_ZSI_TB          74
#define  VD_ZARLINK_9641_ZSI             75
#define  VD_ZARLINK_9652_ZSI_IB          76
#define  VD_ZARLINK_9622_FB              77
#define  VD_ZARLINK_9622_IB              78
#define  VD_ZARLINK_9632_IB              79
#define  VD_ZARLINK_9643_ZSI_IB          80
#define  VD_ZARLINK_9653_IB              81
#define  VD_MAX                          82
                                         
/*
** VOICE_DEVICE_PROFILE
 */
#define  VD_FLYBACK                      0
#define  VD_FLYBACK_TH                   1
#define  VD_BUCKBOOST                    2
#define  VD_TRIPLE_BUCKBOOST             3
#define  VD_INVBOOST                     4
#define  VD_INVBOOST_TH                  5
#define  VD_QCUK                         6
#define  VD_FIXEDRAIL                    7
#define  VD_MASTERSLAVE_FB               8
#define  VD_MASTERSLAVE_IB               9
#define  VD_FB_TSS                       10
#define  VD_FB_TSS_ISO                   11
#define  VD_PMOS_BUCK_BOOST              12
#define  VD_LCQCUK                       13
#define  VD_LCCB                         14
#define  VD_LCUB                         15
/*
** VD_AUDIO_TYPE
 */
#define  VD_VDTYPE_NONE                (-1)
#define  VD_VDTYPE_PCM                  0
#define  VD_VDTYPE_APM                  1
#define  VD_VDTYPE_DECT                 2
#define  VD_VDTYPE_AUDIO                3

/*
** VD_RESET_PIN
 */
#define  VD_RESET_NOT_REQUIRED         (-1)
#define  VD_RESET_FXS1                  0
#define  VD_RESET_FXS2                  1
#define  VD_RESET_FXS3                  3
#define  VD_RESET_FXO                   VD_RESET_FXS3
#define  VD_RESET_FXS4                  (VD_RESET_FXO+1)

/*
** VD_VCTYPE
 */
#define  VD_VCTYPE_NONE                (-1)
#define  VD_VCTYPE_SLIC                 0
#define  VD_VCTYPE_DAA                  1
#define  VD_VCTYPE_DECT                 2
#define  VD_VCTYPE_AUDIO                3

/*
** VD_VC_SAMPLE_COMP
 */
#define  VD_VC_COMP_LINEAR             0 
#define  VD_VC_COMP_ALAW               1
#define  VD_VC_COMP_ULAW               2

/*
** VD_VC_SAMPLE_RATE
 */
#define  VD_VC_8KHZ                    0
#define  VD_VC_16KHZ                   1


/*
** VD_SPI_SIGNAL
 */
#define  VD_SPI_SS_NOT_REQUIRED       (-1)
#define  VD_SPI_SS_B1                  0
#define  VD_SPI_SS_B2                  1
#define  VD_SPI_SS_B3                  2
#define  VD_SPI_SS_B4                  3
#define  VD_SPI_SS_ZSI_ISI             7

/*
** VD_VC_STATUS
*/
#define  VD_VC_NONE                  (-1)
#define  VD_VC_ACTIVE                 0
#define  VD_VC_INACTIVE               1

#endif
