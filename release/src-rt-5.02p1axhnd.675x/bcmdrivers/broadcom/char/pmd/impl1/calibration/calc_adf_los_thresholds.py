# $language = "python"
# $interface = "1.0"

'''
<:copyright-BRCM:2019:proprietary:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
'''

################################################################################################
###################                    importing and setting up             ####################
################################################################################################
from telnet_api import *


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class adf_configuration( object ):
    def __init__(self, data):
        self.los_assert_th = data[0]
        self.los_deassert_th = data[1]
        self.lb_bucket_sz = data[2]
        self.lb_assert_th = data[3]
        

def main(log_file, board_serial_number='', configuration_data=''):
    #crt.Screen.Synchronous = True


    # opening a log file
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    start_time = time.time( )
    log_file_header( log_file, "RSSI_LOS_cal_log" )
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    # starting calibration
    echo( "$$$ RSSI-LOS calibration starts  $$$############### ", 1 )

    # reading rssi factors
    data = read_calibration_results( )
    rssi_a_factor = data.rssi_a_factor
    rssi_b_factor = data.rssi_b_factor
    rssi_c_factor = data.rssi_c_factor
    print "rssi_a_factor=%.2F, rssi_b_factor=%.2F, rssi_c_factor=%.2F \n" %(rssi_a_factor,rssi_b_factor,rssi_c_factor)

    # computing los assert/de-assert threshold
    lambda_zero = 0.00849783423170002452661434309779
    scaling_exp = 10;
    scaling_exp_pow = 1.0 / 2**(23 - scaling_exp)

    op_dbm = float(configuration_data.los_assert_th_op_dbm)
    op_1_16_uw = 16*1000*(10**(op_dbm/10.0))
    print "op_1_16_uw=%f\n" %(op_1_16_uw)
    x = (op_1_16_uw-rssi_a_factor)/rssi_b_factor
    diff_sample = x/lambda_zero/scaling_exp_pow
    adf_los_assert_th = int(diff_sample)
    
    op_dbm = float(configuration_data.los_deassert_th_op_dbm)
    op_1_16_uw = 16*1000*(10**(op_dbm/10.0))
    print "op_1_16_uw=%f\n" %(op_1_16_uw)
    x = (op_1_16_uw-rssi_a_factor)/rssi_b_factor
    diff_sample = x/lambda_zero/scaling_exp_pow
    adf_los_deassert_th = int(diff_sample)
    print "adf_los_assert_th=%d, adf_los_deassert_th=%d\n" %(adf_los_assert_th,adf_los_deassert_th)

    # set assert and deassert threshold in FW
    adf_los_thresholds = (adf_los_deassert_th << 16) | adf_los_assert_th;
    adf_los_thresholds_str = format( adf_los_thresholds, '08x' )
    send2shell( "laser msg --set be 4 %s" %adf_los_thresholds_str )

    # set leaky bucket size and assert threshold (bucket size > assert threshold):
    #    bucket size:      bits 15-8
    #    assert threshold: bits 7-0
    # below: bucket size = 16, assert threshold = 15
    # increasing bucket size and assert threshold:
    #    1. will increase new LOS condition detection time.
    #    2. allow los assert and de-assert thresholds to be closer with lower false alarm rate
    lb_bucket_sz = 16
    lb_assert_th = lb_bucket_sz - 1
    print "adf_lb_bucket_sz=%d, adf_lb_assert_th=%d\n" %(lb_bucket_sz,lb_assert_th)
    lb_params = (lb_bucket_sz << 8) | lb_assert_th
    lb_params_str =  format( lb_params, '04x' )
    send2shell( "laser msg --set bf 2 %s" %lb_params_str )

    # create adf configuration data class
    adf_configuration_data = adf_configuration([adf_los_assert_th, adf_los_deassert_th, lb_bucket_sz, lb_assert_th])

    # calibration complete message
    echo( "$$$   RSSI-LOS calibration completee  $$$###############", 1 )

    # timing the script
    end_time = time.time( )
    log_file.write( "\ntime of script is %f seconds\n" % (end_time - start_time) )

    return adf_configuration_data


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if __name__ == "__main__":
    main( )

