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

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def calc_temp2apdvol(log_file, apd_vol_cal, temp_cal, apd_vbr_coeff_high_temp, apd_vbr_coeff_low_temp, apd_vbr_temp_boundary, apd_vol_step):
    '''
    apd voltage dependence on temperature is modeled as a piece-wise linear curve, the combination of two linear curves:
    - at increasing temperature with respect to a defined temperature point
    - at decreasing temperature with respect to to the same defined temperature point
    - pivot of the curve is determined based on the calibrated apd voltage
    apd_vol_cal: apd calibrated voltage (find vbr@(rssi==(apd_value=0.0285)), then subtract apd_step_down_voltage (each step is 47mV)
    temp_cal: temperature based on thermistor near bosa
    apd_vbr_coeff_high_temp: apd vbr voltage coefficient in high temperture range (V/C)
    apd_vbr_coeff_low_temp: apd vbr voltage coefficient in high temperture range (V/C)
    apd_vbr_temp_boundary: temperature point defining the boundary between vbr temperature coefficents low and high (e.g. 25C)
    apd_vol_step: apd regulator step resultion in Volts,
    '''
    dac_min = 0
    dac_max = 0x3ff
    diff_temp = temp_cal-apd_vbr_temp_boundary
    if diff_temp>=0:
        apd_vol_at_origin = apd_vol_cal - (apd_vbr_coeff_high_temp/apd_vol_step)*diff_temp
    else:
        apd_vol_at_origin = apd_vol_cal - (apd_vbr_coeff_low_temp/apd_vol_step)*diff_temp

    # logging
    params_str = 'parameters: apd_vol_cal=%d [dac], temp_cal=%d [C], apd_vbr_coeff_high_temp=%.3f [V/C], apd_vbr_coeff_low_temp=%.3f [V/C, apd_vbr_temp_boundary=%d [C], apd_vol_step=%.3f [V]'  %(apd_vol_cal, temp_cal, apd_vbr_coeff_high_temp, apd_vbr_coeff_low_temp, apd_vbr_temp_boundary, apd_vol_step)
    print 'calculating temperature to apd voltage table'
    print params_str
    log_file.write('calculating temperature to apd voltage table')
    log_file.write( params_str )
    
    # from -40C to apd_vbr_temp_boundary
    temp2apdvol = [0 for temp in range(-40,120)]
    k = 0
    apd_vol = apd_vol_at_origin - (apd_vbr_coeff_low_temp/apd_vol_step)*(apd_vbr_temp_boundary+40)
    if (apd_vol<dac_min):
        warning_str = 'required apd voltage @-40C is lower than regulator minimum voltage by %.2f Volt (consider changing regulator feedback network)' %((apd_vol-dac_min)*apd_vol_step)
        print warning_str
        print 'clipping values that are lower than minimum regulator voltage'
        log_file.write( warning_str )
        log_file.write( 'clipping values that are lower than minimum regulator voltage' )

    dac_inc = apd_vbr_coeff_low_temp/apd_vol_step
    for temp in range(-40,apd_vbr_temp_boundary):
        temp2apdvol[k] = min(dac_max,max(dac_min,int(round(apd_vol))))
        apd_vol += dac_inc
        k += 1

    # from -40C to apd_vbr_temp_boundary
    apd_vol = apd_vol_at_origin
    dac_inc = apd_vbr_coeff_high_temp/apd_vol_step
    for temp in range(apd_vbr_temp_boundary,120):
        temp2apdvol[k] = min(dac_max,max(dac_min,int(round(apd_vol))))
        apd_vol += dac_inc
        k += 1
    apd_vol -= dac_inc
    if (apd_vol>dac_max):
        warning_str = 'required apd voltage @120C is higher than regulator maximum voltage by %.2f Volt (consider changing regulator feedback network)' %((apd_vol-dac_max)*apd_vol_step)
        print warning_str
        print 'clipping values that are higher than maximum regulator voltage'
        log_file.write(warning_str)
        log_file.write( 'clipping values that are higher than maximum regulator voltage' )

    return temp2apdvol


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def main(log_file,board_serial_number, configuration_data):


    #crt.Screen.Synchronous = True
 
    
    #opening a log file
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    start_time=time.time()
    log_file_header( log_file, "apd_cal_log" )
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    #starting calibration 
    echo("$$$ apd calibration starts  $$$############### ",1)


    #Run apd calibration
    send2shell("laser msg --set ae 2 0002")

    #Wait for process to end
    OPC()
    read_internal_runtime(log_file)

    #Read external temperature 
    T0=int(write_read("laser msg  --get 11 2","-msg"),16)
    T0_hex=format(T0,'x') 
    send2shell("laser calibration --set 13 "+ T0_hex)
    log_file.write("Temperature value is %d\t\n" %(T0))

    # calculate apd temperatute to voltage table
    data = read_calibration_results( )
    apd_vol_cal = int(data.apd_vol,16)
    temp_cal = T0
    apd_vbr_coeff_high_temp = float(configuration_data.apd_vbr_coeff_high_temp)
    apd_vbr_coeff_low_temp = float(configuration_data.apd_vbr_coeff_low_temp)
    apd_vbr_temp_boundary = int(configuration_data.apd_vbr_temp_boundary)
    apd_vol_step = float(configuration_data.apd_vol_step)
    temp2apdvol_table = calc_temp2apdvol(log_file, apd_vol_cal, temp_cal, apd_vbr_coeff_high_temp, apd_vbr_coeff_low_temp, apd_vbr_temp_boundary, apd_vol_step )

    # store table in flash
    temp2apdvol_str = ",".join([str(x) for x in temp2apdvol_table])
    send2shell('laser temp2apd --set ' + temp2apdvol_str)
    log_file.write("\ntemp2apd table:%s\n\n" %(temp2apdvol_str))

    #timing the script
    end_time=time.time()
    log_file.write("\ntime of script is %f seconds\n" %(end_time-start_time))

    echo("$$$ apd calibration finished  $$$############### ",1)

        

