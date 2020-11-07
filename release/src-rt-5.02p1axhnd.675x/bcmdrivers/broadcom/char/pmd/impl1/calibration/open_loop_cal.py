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

#from add_api import *
from telnet_api import *

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


def main(log_file, board_serial_number='', configuration_data=''):
    #crt.Screen.Synchronous = True

    # setting up user configuration_data
    onu_to_tx_att_db = float( configuration_data.onu_to_tx_att_db )

    # opening a log file
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    start_time = time.time( )
    log_file_header( log_file, "open_loop_cal_log" )
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    # starting calibration
    echo( "$$$ open loop calibration starts  $$$############### ", 1 )

    # Run firmware biasctr
    send2shell( "laser msg --set ae 2 0006" )
    # Wait for process to end
    OPC( )

    read_internal_runtime(log_file)

    # meassuring optical power level bias

    # get optical power level
    opm_dbm = float( measure_tx_optical_power_dbm() ) + abs( float( onu_to_tx_att_db ) )

    # calculating micro watt value and target
    op_uw = 1000 * (10 ** (float( opm_dbm ) / 10))
    log_file.write( "bias\noptical power read in dBm=%f  \toptical power in uW =%f \n" % (opm_dbm, op_uw) )

    # setting new Hex values
    op_uw_hex = format( int( round( op_uw ) ), '04x' )
    send2shell( "laser msg --set b0 2 %s" % (op_uw_hex) )

    send2shell( "laser msg --set ae 2 0007" )
    # Wait for process to end
    OPC( )

    read_internal_runtime(log_file)

    # modulation

    # Run firmware modctr
    send2shell( "laser msg --set ae 2 0008" )
    # Wait for process to end
    OPC( )

    read_internal_runtime(log_file)

    # meassuring optical power level modulation

    # get optical power level
    opm_dbm_mod = float( measure_tx_optical_power_dbm() ) + abs( float( onu_to_tx_att_db ) )

    # calculating micro watt value and target
    op_uw_mod = 1000 * (10 ** (float( opm_dbm_mod ) / 10))
    log_file.write(
        "modulation\noptical power read in dBm=%f  \toptical power in uW =%f \n" % (opm_dbm_mod, op_uw_mod) )

    # setting new Hex values
    op_uw_hex = format( int( round( op_uw_mod ) ), '04x' )
    send2shell( "laser msg --set b1 2 %s" % (op_uw_hex) )

    send2shell( "laser msg --set ae 2 0009" )
    # Wait for process to end
    OPC( )

    read_internal_runtime(log_file)
	
	# reading calibration results
    data = read_calibration_results( )

    biasctrl = data.biasctrl
    modctrl = data.modctrl

    log_file.write( "final ctrls\nbiasctrl=%s \t modctrl=%s\n" % (biasctrl, modctrl) )
    # calibration complete message
    echo( "$$$   open loop calibration complete  $$$###############", 1 )

    # timing the script
    end_time = time.time( )
    log_file.write( "\ntime of script is %f seconds\n" % (end_time - start_time) )
