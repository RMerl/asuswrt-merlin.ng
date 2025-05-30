/******************************************************************************
 * Copyright (C) 2021 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
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
 *
 * Module Description:
 *
 *                      !!! DO NOT EDIT THIS FILE DIRECTLY !!!
 *
 * This module was generated automatically with RDB source input files.
 * You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://confluence.broadcom.com/pages/viewpage.action?spaceKey=BCGRDB&title=RDB+%28Confluence%29+Home
 *
 * Date:             Generated on               Tue Nov 30 14:58:39 2021
 *                   Full Compile MD5 Checksum  76b8a32dee8db5f48c3a89e0a6278540
 *                     (minus title and desc)
 *                   MD5 Checksum               7dcf8d7f58b10ccb563b99b1683d60ae
 *
 * lock_release:     n/a
 *
 * Command Line:     /tools/dvtsw/current/Linux/combo_header.pl --multi --style=hydra --output_file=bchp /lwork/CPE/ni889281/bld5_1/depot/CommEngine/widgets/tools/hal_generator/BCM6888_A0_MACSEC/BCM6888_A0/config/ru_gen/BCM6888_A0.rdb
 *
 * Compiled with:    RDB Utility                combo_header.pl
 *                   RDB.pm                     3676
 *                   unknown                    unknown
 *                   Perl Interpreter           5.026000
 *                   Operating System           linux
 *                   Script Source              r_3688/Linux/combo_header.pl
 *                   DVTSWVER                   LOCAL r_3688/Linux/combo_header.pl
 *
 *
 *******************************************************************************/

/****************************************************************************
 * Register info BCHP_REGISTER(offset, width, access, name).
 ***************************************************************************/
BCHP_REGISTER(0x83700004, 32,  RW, ESEC_PORT_0, ESEC_CONFIG)
BCHP_REGISTER(0x8370000c, 32,  RW, ESEC_PORT_0, ESEC_STATUS)
BCHP_REGISTER(0x83700014, 32,  RW, ESEC_PORT_0, ESEC_IDF_STATUS)
BCHP_REGISTER(0x8370001c, 32,  RW, ESEC_PORT_0, ESEC_ICF_STATUS)
BCHP_REGISTER(0x83700024, 32,  RW, ESEC_PORT_0, ESEC_ODF_STATUS)
BCHP_REGISTER(0x8370002c, 32,  RW, ESEC_PORT_0, ESEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83700034, 32,  RW, ESEC_PORT_0, ESEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83700400, 64,  RW, ISEC_PORT_0, ISEC_PP_CTRL)
BCHP_REGISTER(0x8370040c, 32,  RW, ISEC_PORT_0, ISEC_RUD_MGMT_RULE_CTRL)
BCHP_REGISTER(0x83700414, 32,  RO, ISEC_PORT_0, ISEC_AES_ICV_FAIL_CNT)
BCHP_REGISTER(0x8370041c, 32,  RO, ISEC_PORT_0, ISEC_MTU_FAIL_CNT)
BCHP_REGISTER(0x83700424, 32,  RW, ISEC_PORT_0, ISEC_PDF_STATUS)
BCHP_REGISTER(0x8370042c, 32,  RW, ISEC_PORT_0, ISEC_PCF_BANK0_STATUS)
BCHP_REGISTER(0x83700434, 32,  RW, ISEC_PORT_0, ISEC_PCF_BANK1_STATUS)
BCHP_REGISTER(0x8370043c, 32,  RW, ISEC_PORT_0, ISEC_CB_STR_FIFO_STATUS)
BCHP_REGISTER(0x83700444, 32,  RW, ISEC_PORT_0, ISEC_IDF_STATUS)
BCHP_REGISTER(0x8370044c, 32,  RW, ISEC_PORT_0, ISEC_ICF_STATUS)
BCHP_REGISTER(0x83700454, 32,  RW, ISEC_PORT_0, ISEC_ODF_STATUS)
BCHP_REGISTER(0x8370045c, 32,  RW, ISEC_PORT_0, ISEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83700464, 32,  RW, ISEC_PORT_0, ISEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83700804, 32,  RW, MACSEC_PORT_0, MACSEC_CTRL)
BCHP_REGISTER(0x8370080c, 32,  RW, MACSEC_PORT_0, MACSEC_ESEC_ISEC_STATUS)
BCHP_REGISTER(0x83701004, 32,  RW, ESEC_PORT_1, ESEC_CONFIG)
BCHP_REGISTER(0x8370100c, 32,  RW, ESEC_PORT_1, ESEC_STATUS)
BCHP_REGISTER(0x83701014, 32,  RW, ESEC_PORT_1, ESEC_IDF_STATUS)
BCHP_REGISTER(0x8370101c, 32,  RW, ESEC_PORT_1, ESEC_ICF_STATUS)
BCHP_REGISTER(0x83701024, 32,  RW, ESEC_PORT_1, ESEC_ODF_STATUS)
BCHP_REGISTER(0x8370102c, 32,  RW, ESEC_PORT_1, ESEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83701034, 32,  RW, ESEC_PORT_1, ESEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83701400, 64,  RW, ISEC_PORT_1, ISEC_PP_CTRL)
BCHP_REGISTER(0x8370140c, 32,  RW, ISEC_PORT_1, ISEC_RUD_MGMT_RULE_CTRL)
BCHP_REGISTER(0x83701414, 32,  RO, ISEC_PORT_1, ISEC_AES_ICV_FAIL_CNT)
BCHP_REGISTER(0x8370141c, 32,  RO, ISEC_PORT_1, ISEC_MTU_FAIL_CNT)
BCHP_REGISTER(0x83701424, 32,  RW, ISEC_PORT_1, ISEC_PDF_STATUS)
BCHP_REGISTER(0x8370142c, 32,  RW, ISEC_PORT_1, ISEC_PCF_BANK0_STATUS)
BCHP_REGISTER(0x83701434, 32,  RW, ISEC_PORT_1, ISEC_PCF_BANK1_STATUS)
BCHP_REGISTER(0x8370143c, 32,  RW, ISEC_PORT_1, ISEC_CB_STR_FIFO_STATUS)
BCHP_REGISTER(0x83701444, 32,  RW, ISEC_PORT_1, ISEC_IDF_STATUS)
BCHP_REGISTER(0x8370144c, 32,  RW, ISEC_PORT_1, ISEC_ICF_STATUS)
BCHP_REGISTER(0x83701454, 32,  RW, ISEC_PORT_1, ISEC_ODF_STATUS)
BCHP_REGISTER(0x8370145c, 32,  RW, ISEC_PORT_1, ISEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83701464, 32,  RW, ISEC_PORT_1, ISEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83701804, 32,  RW, MACSEC_PORT_1, MACSEC_CTRL)
BCHP_REGISTER(0x8370180c, 32,  RW, MACSEC_PORT_1, MACSEC_ESEC_ISEC_STATUS)
BCHP_REGISTER(0x83702004, 32,  RW, ESEC_PORT_2, ESEC_CONFIG)
BCHP_REGISTER(0x8370200c, 32,  RW, ESEC_PORT_2, ESEC_STATUS)
BCHP_REGISTER(0x83702014, 32,  RW, ESEC_PORT_2, ESEC_IDF_STATUS)
BCHP_REGISTER(0x8370201c, 32,  RW, ESEC_PORT_2, ESEC_ICF_STATUS)
BCHP_REGISTER(0x83702024, 32,  RW, ESEC_PORT_2, ESEC_ODF_STATUS)
BCHP_REGISTER(0x8370202c, 32,  RW, ESEC_PORT_2, ESEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83702034, 32,  RW, ESEC_PORT_2, ESEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83702400, 64,  RW, ISEC_PORT_2, ISEC_PP_CTRL)
BCHP_REGISTER(0x8370240c, 32,  RW, ISEC_PORT_2, ISEC_RUD_MGMT_RULE_CTRL)
BCHP_REGISTER(0x83702414, 32,  RO, ISEC_PORT_2, ISEC_AES_ICV_FAIL_CNT)
BCHP_REGISTER(0x8370241c, 32,  RO, ISEC_PORT_2, ISEC_MTU_FAIL_CNT)
BCHP_REGISTER(0x83702424, 32,  RW, ISEC_PORT_2, ISEC_PDF_STATUS)
BCHP_REGISTER(0x8370242c, 32,  RW, ISEC_PORT_2, ISEC_PCF_BANK0_STATUS)
BCHP_REGISTER(0x83702434, 32,  RW, ISEC_PORT_2, ISEC_PCF_BANK1_STATUS)
BCHP_REGISTER(0x8370243c, 32,  RW, ISEC_PORT_2, ISEC_CB_STR_FIFO_STATUS)
BCHP_REGISTER(0x83702444, 32,  RW, ISEC_PORT_2, ISEC_IDF_STATUS)
BCHP_REGISTER(0x8370244c, 32,  RW, ISEC_PORT_2, ISEC_ICF_STATUS)
BCHP_REGISTER(0x83702454, 32,  RW, ISEC_PORT_2, ISEC_ODF_STATUS)
BCHP_REGISTER(0x8370245c, 32,  RW, ISEC_PORT_2, ISEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83702464, 32,  RW, ISEC_PORT_2, ISEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83702804, 32,  RW, MACSEC_PORT_2, MACSEC_CTRL)
BCHP_REGISTER(0x8370280c, 32,  RW, MACSEC_PORT_2, MACSEC_ESEC_ISEC_STATUS)
BCHP_REGISTER(0x83703004, 32,  RW, ESEC_PORT_3, ESEC_CONFIG)
BCHP_REGISTER(0x8370300c, 32,  RW, ESEC_PORT_3, ESEC_STATUS)
BCHP_REGISTER(0x83703014, 32,  RW, ESEC_PORT_3, ESEC_IDF_STATUS)
BCHP_REGISTER(0x8370301c, 32,  RW, ESEC_PORT_3, ESEC_ICF_STATUS)
BCHP_REGISTER(0x83703024, 32,  RW, ESEC_PORT_3, ESEC_ODF_STATUS)
BCHP_REGISTER(0x8370302c, 32,  RW, ESEC_PORT_3, ESEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83703034, 32,  RW, ESEC_PORT_3, ESEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83703400, 64,  RW, ISEC_PORT_3, ISEC_PP_CTRL)
BCHP_REGISTER(0x8370340c, 32,  RW, ISEC_PORT_3, ISEC_RUD_MGMT_RULE_CTRL)
BCHP_REGISTER(0x83703414, 32,  RO, ISEC_PORT_3, ISEC_AES_ICV_FAIL_CNT)
BCHP_REGISTER(0x8370341c, 32,  RO, ISEC_PORT_3, ISEC_MTU_FAIL_CNT)
BCHP_REGISTER(0x83703424, 32,  RW, ISEC_PORT_3, ISEC_PDF_STATUS)
BCHP_REGISTER(0x8370342c, 32,  RW, ISEC_PORT_3, ISEC_PCF_BANK0_STATUS)
BCHP_REGISTER(0x83703434, 32,  RW, ISEC_PORT_3, ISEC_PCF_BANK1_STATUS)
BCHP_REGISTER(0x8370343c, 32,  RW, ISEC_PORT_3, ISEC_CB_STR_FIFO_STATUS)
BCHP_REGISTER(0x83703444, 32,  RW, ISEC_PORT_3, ISEC_IDF_STATUS)
BCHP_REGISTER(0x8370344c, 32,  RW, ISEC_PORT_3, ISEC_ICF_STATUS)
BCHP_REGISTER(0x83703454, 32,  RW, ISEC_PORT_3, ISEC_ODF_STATUS)
BCHP_REGISTER(0x8370345c, 32,  RW, ISEC_PORT_3, ISEC_TAG_FIFO_STATUS)
BCHP_REGISTER(0x83703464, 32,  RW, ISEC_PORT_3, ISEC_CW_FIFO_STATUS)
BCHP_REGISTER(0x83703804, 32,  RW, MACSEC_PORT_3, MACSEC_CTRL)
BCHP_REGISTER(0x8370380c, 32,  RW, MACSEC_PORT_3, MACSEC_ESEC_ISEC_STATUS)
BCHP_REGISTER(0x83704004, 32,  RW, ESEC_GENERAL, ESEC_EGRESS_MTU_FOR_MGMT_PKT)
BCHP_REGISTER(0x8370400c, 32,  RW, ESEC_GENERAL, ESEC_CTRL)
BCHP_REGISTER(0x83704014, 32,  RW, ESEC_GENERAL, ESEC_PN_THD)
BCHP_REGISTER(0x83704018, 64,  RW, ESEC_GENERAL, ESEC_XPN_THD)
BCHP_REGISTER(0x83704024, 32,  RW, ESEC_GENERAL, ESEC_SVTAG_ETYPE)
BCHP_REGISTER(0x8370402c, 32,  RW, ESEC_GENERAL, ESEC_VXLANSEC_DEST_PORT_NO)
BCHP_REGISTER(0x83704030, 64,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_KEY_REG0)
BCHP_REGISTER(0x83704038, 64,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_KEY_REG1)
BCHP_REGISTER(0x83704040, 64,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_KEY_REG2)
BCHP_REGISTER(0x83704048, 64,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_KEY_REG3)
BCHP_REGISTER(0x83704050, 64,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_IV_REG0)
BCHP_REGISTER(0x8370405c, 32,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_IV_REG1)
BCHP_REGISTER(0x83704060, 64,  RW, ESEC_GENERAL, ESEC_FIPS_DATA_REG0)
BCHP_REGISTER(0x83704068, 64,  RW, ESEC_GENERAL, ESEC_FIPS_DATA_REG1)
BCHP_REGISTER(0x83704074, 32,  RW, ESEC_GENERAL, ESEC_FIPS_OVR_CONTROL)
BCHP_REGISTER(0x83704078, 64,  RO, ESEC_GENERAL, ESEC_FIPS_ENC_DATA_REG0)
BCHP_REGISTER(0x83704080, 64,  RO, ESEC_GENERAL, ESEC_FIPS_ENC_DATA_REG1)
BCHP_REGISTER(0x83704088, 64,  RO, ESEC_GENERAL, ESEC_FIPS_ICV_REG0)
BCHP_REGISTER(0x83704090, 64,  RO, ESEC_GENERAL, ESEC_FIPS_ICV_REG1)
BCHP_REGISTER(0x8370409c, 32,  RW, ESEC_GENERAL, ESEC_FIPS_STATUS)
BCHP_REGISTER(0x837040a4, 32,  RW, ESEC_GENERAL, ESEC_INTR_ENABLE)
BCHP_REGISTER(0x837040ac, 32,  RO, ESEC_GENERAL, ESEC_IDF_INTR)
BCHP_REGISTER(0x837040b4, 32,  RO, ESEC_GENERAL, ESEC_ICF_INTR)
BCHP_REGISTER(0x837040bc, 32,  RO, ESEC_GENERAL, ESEC_ODF_INTR)
BCHP_REGISTER(0x837040c4, 32,  RO, ESEC_GENERAL, ESEC_TAG_FIFO_INTR)
BCHP_REGISTER(0x837040cc, 32,  RO, ESEC_GENERAL, ESEC_CW_FIFO_INTR)
BCHP_REGISTER(0x837040d4, 32,  RW, ESEC_GENERAL, ESEC_SC_TABLE_ECC_STATUS)
BCHP_REGISTER(0x837040dc, 32,  RW, ESEC_GENERAL, ESEC_SA_TABLE_ECC_STATUS)
BCHP_REGISTER(0x837040e4, 32,  RW, ESEC_GENERAL, ESEC_HASH_TABLE_ECC_STATUS)
BCHP_REGISTER(0x837040ec, 32,  RW, ESEC_GENERAL, ESEC_ERROR_INTR_STATUS)
BCHP_REGISTER(0x837040f4, 32,  RW, ESEC_GENERAL, ESEC_ERROR_INTR_ENABLE)
BCHP_REGISTER(0x837040f8, 64,  RW, ESEC_GENERAL, ESEC_SPARE)
BCHP_REGISTER(0x83704400, 64,  RW, ISEC_GENERAL, ISEC_CTRL)
BCHP_REGISTER(0x8370440c, 32,  RW, ISEC_GENERAL, ISEC_PN_EXPIRE_THD)
BCHP_REGISTER(0x83704410, 64,  RW, ISEC_GENERAL, ISEC_XPN_EXPIRE_THD)
BCHP_REGISTER(0x8370441c, 32,  RW, ISEC_GENERAL, ISEC_SVTAG_CTRL)
BCHP_REGISTER(0x83704424, 32,  RW, ISEC_GENERAL, ISEC_ETYPE_MAX_LEN)
BCHP_REGISTER(0x83704428, 64,  RW, ISEC_GENERAL, ISEC_TPID_0)
BCHP_REGISTER(0x83704434, 32,  RW, ISEC_GENERAL, ISEC_TPID_1)
BCHP_REGISTER(0x83704438, 64,  RW, ISEC_GENERAL, ISEC_MPLS_ETYPE)
BCHP_REGISTER(0x83704444, 32,  RW, ISEC_GENERAL, ISEC_PBB_TPID)
BCHP_REGISTER(0x8370444c, 32,  RW, ISEC_GENERAL, ISEC_IPV4_ETYPE)
BCHP_REGISTER(0x83704454, 32,  RW, ISEC_GENERAL, ISEC_IPV6_ETYPE)
BCHP_REGISTER(0x8370445c, 32,  RW, ISEC_GENERAL, ISEC_PTP_ETYPE)
BCHP_REGISTER(0x83704464, 32,  RW, ISEC_GENERAL, ISEC_NIV_ETYPE)
BCHP_REGISTER(0x8370446c, 32,  RW, ISEC_GENERAL, ISEC_PE_ETYPE)
BCHP_REGISTER(0x83704474, 32,  RW, ISEC_GENERAL, ISEC_UDP_PROTOCOL)
BCHP_REGISTER(0x8370447c, 32,  RW, ISEC_GENERAL, ISEC_TCP_PROTOCOL)
BCHP_REGISTER(0x83704484, 32,  RW, ISEC_GENERAL, ISEC_VXLANSEC_DEST_PORT_NO)
BCHP_REGISTER(0x8370448c, 32,  RW, ISEC_GENERAL, ISEC_PTP_DEST_PORT_NO)
BCHP_REGISTER(0x83704494, 32,  RW, ISEC_GENERAL, ISEC_OUT_DESTPORT_NO)
BCHP_REGISTER(0x83704498, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_0)
BCHP_REGISTER(0x837044a0, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_1)
BCHP_REGISTER(0x837044a8, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_2)
BCHP_REGISTER(0x837044b0, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_3)
BCHP_REGISTER(0x837044b8, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_4)
BCHP_REGISTER(0x837044c0, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_5)
BCHP_REGISTER(0x837044c8, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_6)
BCHP_REGISTER(0x837044d0, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_DA_7)
BCHP_REGISTER(0x837044d8, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_ETYPE_0)
BCHP_REGISTER(0x837044e0, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_CFG_ETYPE_1)
BCHP_REGISTER(0x837044e8, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_DA_RANGE_LOW)
BCHP_REGISTER(0x837044f0, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_DA_RANGE_HIGH)
BCHP_REGISTER(0x837044f8, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_DA_ETYPE_1st)
BCHP_REGISTER(0x83704500, 64,  RW, ISEC_GENERAL, ISEC_MGMTRULE_DA_ETYPE_2nd)
BCHP_REGISTER(0x8370450c, 32,  RW, ISEC_GENERAL, ISEC_PAD_CTRL)
BCHP_REGISTER(0x83704514, 32,  RW, ISEC_GENERAL, ISEC_AES_ICV_FAIL_STATUS)
BCHP_REGISTER(0x83704518, 64,  RO, ISEC_GENERAL, ISEC_AES_CALC_ICV_REG0)
BCHP_REGISTER(0x83704520, 64,  RW, ISEC_GENERAL, ISEC_AES_CALC_ICV_REG1)
BCHP_REGISTER(0x83704528, 64,  RO, ISEC_GENERAL, ISEC_AES_RCV_ICV_REG0)
BCHP_REGISTER(0x83704530, 64,  RW, ISEC_GENERAL, ISEC_AES_RCV_ICV_REG1)
BCHP_REGISTER(0x83704538, 64,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_KEY_REG0)
BCHP_REGISTER(0x83704540, 64,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_KEY_REG1)
BCHP_REGISTER(0x83704548, 64,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_KEY_REG2)
BCHP_REGISTER(0x83704550, 64,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_KEY_REG3)
BCHP_REGISTER(0x83704558, 64,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_IV_REG0)
BCHP_REGISTER(0x83704564, 32,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_IV_REG1)
BCHP_REGISTER(0x83704568, 64,  RW, ISEC_GENERAL, ISEC_FIPS_DATA_REG0)
BCHP_REGISTER(0x83704570, 64,  RW, ISEC_GENERAL, ISEC_FIPS_DATA_REG1)
BCHP_REGISTER(0x8370457c, 32,  RW, ISEC_GENERAL, ISEC_FIPS_OVR_CONTROL)
BCHP_REGISTER(0x83704580, 64,  RO, ISEC_GENERAL, ISEC_FIPS_ENC_DATA_REG0)
BCHP_REGISTER(0x83704588, 64,  RO, ISEC_GENERAL, ISEC_FIPS_ENC_DATA_REG1)
BCHP_REGISTER(0x83704590, 64,  RO, ISEC_GENERAL, ISEC_FIPS_ICV_REG0)
BCHP_REGISTER(0x83704598, 64,  RO, ISEC_GENERAL, ISEC_FIPS_ICV_REG1)
BCHP_REGISTER(0x837045a4, 32,  RW, ISEC_GENERAL, ISEC_FIPS_STATUS)
BCHP_REGISTER(0x837045ac, 32,  RW, ISEC_GENERAL, ISEC_SER_CONTROL)
BCHP_REGISTER(0x837045b0, 64,  RW, ISEC_GENERAL, ISEC_SER_SCAN_CONFIG)
BCHP_REGISTER(0x837045bc, 32,  RO, ISEC_GENERAL, ISEC_SER_SCAN_STATUS)
BCHP_REGISTER(0x837045c0, 64,  RW, ISEC_GENERAL, ISEC_SPTCAM_SER_STATUS)
BCHP_REGISTER(0x837045c8, 64,  RW, ISEC_GENERAL, ISEC_SCTCAM_SER_STATUS)
BCHP_REGISTER(0x837045d0, 64,  RW, ISEC_GENERAL, SP_CAM_BIST_CONFIG_A)
BCHP_REGISTER(0x837045dc, 32,  RO, ISEC_GENERAL, SP_CAM_BIST_STATUS_A)
BCHP_REGISTER(0x837045e0, 64,  RW, ISEC_GENERAL, SP_CAM_BIST_CONFIG_B)
BCHP_REGISTER(0x837045ec, 32,  RO, ISEC_GENERAL, SP_CAM_BIST_STATUS_B)
BCHP_REGISTER(0x837045f0, 64,  RW, ISEC_GENERAL, SP_CAM_BIST_CONFIG_C)
BCHP_REGISTER(0x837045fc, 32,  RO, ISEC_GENERAL, SP_CAM_BIST_STATUS_C)
BCHP_REGISTER(0x83704600, 64,  RW, ISEC_GENERAL, SC_CAM_BIST_CONFIG)
BCHP_REGISTER(0x8370460c, 32,  RO, ISEC_GENERAL, SC_CAM_BIST_STATUS)
BCHP_REGISTER(0x83704610, 64,  RW, ISEC_GENERAL, ISEC_INTR_ENABLE)
BCHP_REGISTER(0x8370461c, 32,  RO, ISEC_GENERAL, ISEC_PDF_INTR)
BCHP_REGISTER(0x83704624, 32,  RO, ISEC_GENERAL, ISEC_PCF_BANK0_INTR)
BCHP_REGISTER(0x8370462c, 32,  RO, ISEC_GENERAL, ISEC_PCF_BANK1_INTR)
BCHP_REGISTER(0x83704634, 32,  RO, ISEC_GENERAL, ISEC_CB_STR_FIFO_INTR)
BCHP_REGISTER(0x8370463c, 32,  RO, ISEC_GENERAL, ISEC_IDF_INTR)
BCHP_REGISTER(0x83704644, 32,  RO, ISEC_GENERAL, ISEC_ICF_INTR)
BCHP_REGISTER(0x8370464c, 32,  RO, ISEC_GENERAL, ISEC_ODF_INTR)
BCHP_REGISTER(0x83704654, 32,  RO, ISEC_GENERAL, ISEC_TAG_FIFO_INTR)
BCHP_REGISTER(0x8370465c, 32,  RO, ISEC_GENERAL, ISEC_CW_FIFO_INTR)
BCHP_REGISTER(0x83704664, 32,  RW, ISEC_GENERAL, ISEC_SC_TABLE_ECC_STATUS)
BCHP_REGISTER(0x8370466c, 32,  RW, ISEC_GENERAL, ISEC_SA_TABLE_ECC_STATUS)
BCHP_REGISTER(0x83704674, 32,  RW, ISEC_GENERAL, ISEC_HASH_TABLE_ECC_STATUS)
BCHP_REGISTER(0x8370467c, 32,  RW, ISEC_GENERAL, ISEC_SP_POLICY_ECC_STATUS)
BCHP_REGISTER(0x83704684, 32,  RW, ISEC_GENERAL, ISEC_SP_MAP_ECC_STATUS)
BCHP_REGISTER(0x8370468c, 32,  RW, ISEC_GENERAL, ISEC_ERROR_INTR_STATUS)
BCHP_REGISTER(0x83704694, 32,  RW, ISEC_GENERAL, ISEC_ERROR_INTR_ENABLE)
BCHP_REGISTER(0x8370469c, 32,  RW, ISEC_GENERAL, ISEC_MISC_CTRL)
BCHP_REGISTER(0x837046a0, 64,  RW, ISEC_GENERAL, ISEC_SPARE)
BCHP_REGISTER(0x83704804, 32,  RO, MACSEC_GENERAL, MACSEC_VERSION_ID)
BCHP_REGISTER(0x8370480c, 32,  RW, MACSEC_GENERAL, MACSEC_GEN_CTRL)
BCHP_REGISTER(0x83704814, 32,  RW, MACSEC_GENERAL, MACSEC_TDM_WRAP_PTR)
BCHP_REGISTER(0x83704818, 64,  RW, MACSEC_GENERAL, MACSEC_SECTAG_ETYPE)
BCHP_REGISTER(0x83704824, 32,  RW, MACSEC_GENERAL, MACSEC_TIME_TICK)
BCHP_REGISTER(0x83704828, 64,  RW, MACSEC_GENERAL, MACSEC_TIME_TICK_CTRL)
BCHP_REGISTER(0x83704830, 64,  RO, MACSEC_GENERAL, MACSEC_CURRENT_TIMER)
BCHP_REGISTER(0x8370483c, 32,  RW, MACSEC_GENERAL, MACSEC_HW_RESET_CONTROL)
BCHP_REGISTER(0x83704844, 32,  RW, MACSEC_GENERAL, MACSEC_ECC_CTRL)
BCHP_REGISTER(0x83704848, 64,  RW, MACSEC_GENERAL, MACSEC_INTR_ENABLE)
BCHP_REGISTER(0x83704850, 64,  RW, MACSEC_GENERAL, MACSEC_INTR_STATUS)
BCHP_REGISTER(0x8370485c, 32,  RW, MACSEC_GENERAL, MACSEC_MIB_INTR_ENABLE)
BCHP_REGISTER(0x83704864, 32,  RO, MACSEC_GENERAL, MACSEC_MIB_INTR_STATUS)
BCHP_REGISTER(0x83704868, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_1)
BCHP_REGISTER(0x83704870, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_2)
BCHP_REGISTER(0x83704878, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_3)
BCHP_REGISTER(0x83704880, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_4)
BCHP_REGISTER(0x83704888, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_5)
BCHP_REGISTER(0x83704890, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_6)
BCHP_REGISTER(0x83704898, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_7)
BCHP_REGISTER(0x837048a0, 64,  RW, MACSEC_GENERAL, MACSEC_MEM_CTRL_8)
BCHP_REGISTER(0x837048a8, 64,  RO, MACSEC_GENERAL, MACSEC_INTR_FIFO_COUNT)
BCHP_REGISTER(0x837048b4, 32,  RW, MACSEC_GENERAL, ESEC_MIB_MISC_ECC_STATUS)
BCHP_REGISTER(0x837048bc, 32,  RW, MACSEC_GENERAL, ESEC_MIB_SC_UnCtrl_ECC_STATUS)
BCHP_REGISTER(0x837048c4, 32,  RW, MACSEC_GENERAL, ESEC_MIB_SC_Ctrl_ECC_STATUS)
BCHP_REGISTER(0x837048cc, 32,  RW, MACSEC_GENERAL, ESEC_MIB_SC_ECC_STATUS)
BCHP_REGISTER(0x837048d4, 32,  RW, MACSEC_GENERAL, ESEC_MIB_SA_ECC_STATUS)
BCHP_REGISTER(0x837048dc, 32,  RW, MACSEC_GENERAL, ISEC_SPTCAM_HIT_COUNT_ECC_STATUS)
BCHP_REGISTER(0x837048e4, 32,  RW, MACSEC_GENERAL, ISEC_SCTCAM_HIT_COUNT_ECC_STATUS)
BCHP_REGISTER(0x837048ec, 32,  RW, MACSEC_GENERAL, ISEC_PORT_COUNTERS_ECC_STATUS)
BCHP_REGISTER(0x837048f4, 32,  RW, MACSEC_GENERAL, ISEC_MIB_SP_UNCTRL_ECC_STATUS)
BCHP_REGISTER(0x837048fc, 32,  RW, MACSEC_GENERAL, ISEC_MIB_SP_CTRL_1_ECC_STATUS)
BCHP_REGISTER(0x83704904, 32,  RW, MACSEC_GENERAL, ISEC_MIB_SP_CTRL_2_ECC_STATUS)
BCHP_REGISTER(0x8370490c, 32,  RW, MACSEC_GENERAL, ISEC_MIB_SC_ECC_STATUS)
BCHP_REGISTER(0x83704914, 32,  RW, MACSEC_GENERAL, ISEC_MIB_SA_ECC_STATUS)
BCHP_REGISTER(0x8370491c, 32,  RW, MACSEC_GENERAL, MACSEC_MIB_MODE)
BCHP_REGISTER(0x83704920, 64,  RW, MACSEC_GENERAL, MACSEC_SPARE)
BCHP_REGISTER(0x83705000, 32,  RW, ETH_R2SBUS_BRIDGE, SBUS_ID)
BCHP_REGISTER(0x83705004, 32,  RW, ETH_R2SBUS_BRIDGE, TIMEOUT)
BCHP_REGISTER(0x83705008, 32,  RO, ETH_R2SBUS_BRIDGE, TIMEOUT_STATUS)
BCHP_REGISTER(0x83705010, 32,  RW, ETH_R2SBUS_BRIDGE, DIRECT_WRITE_DATA_HIGH)
BCHP_REGISTER(0x83705014, 32,  RO, ETH_R2SBUS_BRIDGE, DIRECT_READ_DATA_HIGH)
BCHP_REGISTER(0x83705018, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_COMMAND)
BCHP_REGISTER(0x8370501c, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_COMMAND)
BCHP_REGISTER(0x83705020, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA0)
BCHP_REGISTER(0x83705024, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA1)
BCHP_REGISTER(0x83705028, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA2)
BCHP_REGISTER(0x8370502c, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA3)
BCHP_REGISTER(0x83705030, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA4)
BCHP_REGISTER(0x83705034, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA5)
BCHP_REGISTER(0x83705038, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA6)
BCHP_REGISTER(0x8370503c, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA7)
BCHP_REGISTER(0x83705040, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA8)
BCHP_REGISTER(0x83705044, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA9)
BCHP_REGISTER(0x83705048, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA10)
BCHP_REGISTER(0x8370504c, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA11)
BCHP_REGISTER(0x83705050, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA12)
BCHP_REGISTER(0x83705054, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA13)
BCHP_REGISTER(0x83705058, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA14)
BCHP_REGISTER(0x8370505c, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA15)
BCHP_REGISTER(0x83705060, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA16)
BCHP_REGISTER(0x83705064, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT0_WRITE_DATA17)
BCHP_REGISTER(0x83705070, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA0)
BCHP_REGISTER(0x83705074, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA1)
BCHP_REGISTER(0x83705078, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA2)
BCHP_REGISTER(0x8370507c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA3)
BCHP_REGISTER(0x83705080, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA4)
BCHP_REGISTER(0x83705084, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA5)
BCHP_REGISTER(0x83705088, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA6)
BCHP_REGISTER(0x8370508c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA7)
BCHP_REGISTER(0x83705090, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA8)
BCHP_REGISTER(0x83705094, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA9)
BCHP_REGISTER(0x83705098, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA10)
BCHP_REGISTER(0x8370509c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA11)
BCHP_REGISTER(0x837050a0, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA12)
BCHP_REGISTER(0x837050a4, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA13)
BCHP_REGISTER(0x837050a8, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA14)
BCHP_REGISTER(0x837050ac, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA15)
BCHP_REGISTER(0x837050b0, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA16)
BCHP_REGISTER(0x837050b4, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT0_READ_DATA17)
BCHP_REGISTER(0x837050c0, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA0)
BCHP_REGISTER(0x837050c4, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA1)
BCHP_REGISTER(0x837050c8, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA2)
BCHP_REGISTER(0x837050cc, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA3)
BCHP_REGISTER(0x837050d0, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA4)
BCHP_REGISTER(0x837050d4, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA5)
BCHP_REGISTER(0x837050d8, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA6)
BCHP_REGISTER(0x837050dc, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA7)
BCHP_REGISTER(0x837050e0, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA8)
BCHP_REGISTER(0x837050e4, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA9)
BCHP_REGISTER(0x837050e8, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA10)
BCHP_REGISTER(0x837050ec, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA11)
BCHP_REGISTER(0x837050f0, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA12)
BCHP_REGISTER(0x837050f4, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA13)
BCHP_REGISTER(0x837050f8, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA14)
BCHP_REGISTER(0x837050fc, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA15)
BCHP_REGISTER(0x83705100, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA16)
BCHP_REGISTER(0x83705104, 32,  RW, ETH_R2SBUS_BRIDGE, INDIRECT1_WRITE_DATA17)
BCHP_REGISTER(0x83705110, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA0)
BCHP_REGISTER(0x83705114, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA1)
BCHP_REGISTER(0x83705118, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA2)
BCHP_REGISTER(0x8370511c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA3)
BCHP_REGISTER(0x83705120, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA4)
BCHP_REGISTER(0x83705124, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA5)
BCHP_REGISTER(0x83705128, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA6)
BCHP_REGISTER(0x8370512c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA7)
BCHP_REGISTER(0x83705130, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA8)
BCHP_REGISTER(0x83705134, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA9)
BCHP_REGISTER(0x83705138, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA10)
BCHP_REGISTER(0x8370513c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA11)
BCHP_REGISTER(0x83705140, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA12)
BCHP_REGISTER(0x83705144, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA13)
BCHP_REGISTER(0x83705148, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA14)
BCHP_REGISTER(0x8370514c, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA15)
BCHP_REGISTER(0x83705150, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA16)
BCHP_REGISTER(0x83705154, 32,  RO, ETH_R2SBUS_BRIDGE, INDIRECT1_READ_DATA17)

/* End of File */
