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

import math
from telnet_api import *


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


def main(log_file, board_serial_number='', configuration_data=''):
    #crt.Screen.Synchronous = True

    # setting up user configuration_data
    onu_to_tx_att_db = float( configuration_data.onu_to_tx_att_db )
    target_op_dbm = float( configuration_data.avrg_power )
    prbs_mode = int( configuration_data.prbs_mode )

    # opening a log file
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    start_time = time.time( )
    log_file_header( log_file, "Average_op_cal_log" )
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    # starting calibration
    if 1 == prbs_mode:
        send2shell("laser general --set 2 8148 4 00000001")
        send2shell("laser general --get 2 8148 4")
        echo(" prbs based average power tuning", 1)
        echo("$$$ PRBS average optical power calibration starts  $$$############### ", 1)
        # start DUT prbs transmission
        start_prbs()
        burst_correction_factor_db = 0;
    else:
        echo(" burst based average power tuning", 1)
        echo("$$$ Burst average optical power calibration starts  $$$############### ", 1)
        if int(configuration_data.board) == 4:
            msstop = 9344;
            frame_length = 9720;
        else:
            msstop = 18688;
            frame_length = 19440;
        # start DUT bursts transmission
        start_burst(int(configuration_data.board), msstop)
        burst_correction_factor_db = 10 * math.log10(frame_length / float(msstop - 256))

    log_file.write( "Tune average op\ntarget_op_dbm %f dBm \tburst_correction_factor_db %f \n" % (target_op_dbm, burst_correction_factor_db) )	

    while True:
        #Operate FW
        send2shell("laser msg --set ae 2 000d")

        #Wait for process to end
        OPC()

        read_internal_runtime(log_file)
        send2shell("laser msg --get 17 50")

        # get optical power level
        opm_dbm = float( measure_tx_optical_power_dbm() ) + abs( float( onu_to_tx_att_db ) )
        opm_dbm = opm_dbm + burst_correction_factor_db

        # calculating micro watt value and target
        op_uw = 1000 * (10 ** (float( opm_dbm ) / 10))
        log_file.write( "Tune average op\noptical power read in dBm=%f  \toptical power in uW =%f \n" % (opm_dbm, op_uw) )		

        # setting new Hex values
        final_op_hex = format( int( round( op_uw ) ), '04x' )
        send2shell( "laser msg --set b3 2 %s" % (final_op_hex) )
        send2shell( "laser msg --set b4 2 %s" % (final_op_hex) )

        if abs(opm_dbm - target_op_dbm) < 0.2:
            break

    if 1 == prbs_mode:
        # stop DUT prbs bursts transmission
        stop_prbs()
        send2shell("laser general --set 2 8148 4 00000000")
        send2shell("laser general --get 2 8148 4")
    else:
        # stop DUT bursts transmission
        stop_burst( )

    # calibration complete message
    echo( "$$$   Average optical power calibration completee  $$$###############", 1 )

    # timing the script
    end_time = time.time( )
    log_file.write( "\ntime of script is %f seconds\n" % (end_time - start_time) )
