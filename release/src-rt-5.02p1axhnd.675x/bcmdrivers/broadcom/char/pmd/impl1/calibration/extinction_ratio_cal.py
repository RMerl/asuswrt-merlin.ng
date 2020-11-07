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

from telnet_api import *


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



def main(log_file, board_serial_number='', configuration_data=''):
    #crt.Screen.Synchronous = True

    # setting up user configuration_data
    er_target = float( configuration_data.er_target )
    prbs_mode = int( configuration_data.prbs_mode )

    # opening a log file
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    start_time = time.time( )
    log_file_header( log_file, "Extinction_ratio_cal_log" )
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    # starting calibration
    if 1 == prbs_mode:
        send2shell("laser general --set 2 8148 4 00000001")
        send2shell("laser general --get 2 8148 4")
        echo(" prbs based ER tuning", 1)
        # start DUT prbs transmission
        echo( "$$$ PRBS extinction ratio calibration starts  $$$############### ", 1 )
        er_delta = 0.2
        start_prbs()
    else:
        echo(" burst based ER tuning", 1)
        # start DUT bursts transmission
        echo( "$$$ Burst extinction ratio calibration starts  $$$############### ", 1 )
        er_delta = 1
        if int(configuration_data.board) == 4: 
            start_burst(int(configuration_data.board), 9344)
        else:
            start_burst(int(configuration_data.board), 18688)

    for x in range(0, 9):
        #Operate FW
        send2shell("laser msg --set ae 2 000e")

        # Wait for process to end
        OPC( )

        read_internal_runtime(log_file)
        send2shell("laser msg --get 17 50")

        # get extinction ratio level
        er_level = float( measure_extinction_ratio() )
        extinction_ratio = (10 ** (float( er_level ) / 10)) * 256 
        log_file.write( "tune extinction ratio\nextinction_ratio read %f  \t linear extinction_ratio in Q8.8 %f \n" % (er_level, extinction_ratio) )

        if abs(er_level - er_target) < er_delta:
            break

        # setting new Hex values
        er_hex = format( int( round( extinction_ratio ) ), '04x' )
        send2shell( "laser msg --set b6 2 %s" % (er_hex) )

    if 1 == prbs_mode:
        # stop DUT prbs transmission
        stop_prbs( )
        send2shell("laser general --set 2 8148 4 00000000")
        send2shell("laser general --get 2 8148 4")
    else:
        # stop DUT bursts transmission
        stop_burst( )

    # calibration complete message
    echo( "$$$   Extinction ratio calibration complete  $$$###############", 1 )

    # timing the script
    end_time = time.time( )
    log_file.write( "\ntime of script is %f seconds\n" % (end_time - start_time) )
