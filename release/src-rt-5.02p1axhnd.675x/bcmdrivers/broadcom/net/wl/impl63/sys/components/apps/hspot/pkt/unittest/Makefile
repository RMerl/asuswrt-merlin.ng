# Makefile for hotspot packet encoder & decoder module.
#
# Copyright (C) 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile xxxxxx 2014-11-14 06:00:44Z Dima $

#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# Src folder path
SRC_PATH = ../../../../../../main/src
# Components folder path
COMPONENTS_PATH = ../../../../../components

WLAN_ComponentsInUse += bcmwifi math
include $(SRC_PATH)/makefiles/WLAN_Common.mk

# Source file path
SOURCE = ../bcm_encode.c ../bcm_decode.c ../bcm_encode_ie.c ../bcm_decode_ie.c \
	 ../bcm_encode_gas.c ../bcm_decode_gas.c ../bcm_encode_hspot_anqp.c \
	 ../bcm_decode_hspot_anqp.c ../bcm_encode_anqp.c ../bcm_decode_anqp.c \
	 ../bcm_encode_qos.c ../bcm_decode_qos.c ../bcm_encode_wnm.c ../bcm_decode_wnm.c \
	 ../bcm_decode_p2p.c $(SRC_PATH)/shared/bcmwifi/src/bcmwifi_channels.c ../../util/trace.c

# Header file path
INCLUDE = $(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR) ../../include .. .

# Unit tests file path
TEST_SRC = main.c test_bcm_enc_dec.c test_bcm_enc_dec_ie.c test_bcm_enc_dec_gas.c \
	   test_bcm_enc_dec_anqp.c test_bcm_enc_dec_hspot_anqp.c test_bcm_enc_dec_qos.c \
	   test_bcm_enc_dec_wnm.c

# Check tool directory path (from the unit test source directory)
CHECK_ROOT_PATH = $(SRC_PATH)/unittest

#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Central make file path
include $(CHECK_ROOT_PATH)/unittest.mk
