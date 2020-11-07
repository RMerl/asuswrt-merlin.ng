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
import time, sys, os

dirname = os.getcwd()
sys.path.insert(0, dirname)

from telnet_api import *
from thermistor_tbl import *
from datetime import datetime
import version

# Loading user setup configuration


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def main():
    # open telent connection
    Init_api()

    # creating a time stamp
    timestamp = datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d--%H-%M-%S')

    board_serial_number = None
    board_serial_number = get_borad_serial_number()

    # Creating a directory for the log files
    dir_full_path = dirname + "\\SN%s %s log files\\" % (board_serial_number, timestamp)
    if not os.path.exists(dir_full_path):
        os.makedirs(dir_full_path)

        # opening main log file
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    start_time = time.time()
    log_file_name = dir_full_path + "main_log_file.txt"
    log_file = open_log_file(log_file_name, board_serial_number)
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    # import all scripts
    import apd_cal, \
        open_loop_cal, \
        mpd_cal, \
        eyesafety_cal, \
        rogue_cal, \
        tracking_cal_first_burst_targets, \
        tracking_cal_tracking_targets, \
        rssi_cal, \
        los_cal, \
        calc_adf_los_thresholds, \
        flash_log, \
        avrg_op_cal, \
        extinction_ratio_cal, \
        dcd_cal

    pmd_type = int(read_reg("laser general --get 0 02a8 4"), 16)
    print (pmd_type)

    # load and define user parameters and log the parameters 
    configuration_data = load_configuration()
    log_str = '\n\nconfiguration.ini parameters and derived parameters:\n'
    for attr in dir(configuration_data):
        ret_val = attr.find('__')
        if ret_val == 0:
            # skip internal python attributes (starting with __
            continue
        log_str = log_str + '%s = %r\n' % (attr, getattr(configuration_data, attr))
    log_file.write(log_str + '\n\n')

    opm_type = configuration_data.opm_type
    rx_opm_address = configuration_data.rx_opm_address
    tx_opm_address = configuration_data.tx_opm_address
    attenuator_address = configuration_data.attenuator_address
    dca_address = configuration_data.dca_address
    onu_to_tx_att_db = configuration_data.onu_to_tx_att_db
    onu_to_rx_opm_att_db = configuration_data.onu_to_rx_opm_att_db
    olt_to_onu_att1_db = configuration_data.olt_to_onu_att1_db
    olt_to_onu_att2_db = configuration_data.olt_to_onu_att2_db
    apd_vbr_rssi_config = configuration_data.apd_vbr_rssi
    apd_stepdown_config = configuration_data.apd_stepdown
    max_mpd_config = configuration_data.max_mpd
    min_mpd_config = configuration_data.min_mpd
    avrg_power_config = configuration_data.avrg_power
    L0_uW_config = configuration_data.L0_uW
    L1_uW_config = configuration_data.L1_uW
    er_target_config = configuration_data.er_target
    max_biasctl_config = configuration_data.max_biasctl
    max_modctl_config = configuration_data.max_modctl
    min_op_config = configuration_data.min_optical_power
    max_eyesafety_config = configuration_data.max_eyesafety
    min_eyesafety_config = configuration_data.min_eyesafety
    max_rogue_config = configuration_data.max_rogue
    min_rogue_config = configuration_data.min_rogue
    los_assert_th_op_dbm = configuration_data.los_assert_th_op_dbm
    los_deassert_th_op_dbm = configuration_data.los_deassert_th_op_dbm
    cal_apd = int(configuration_data.cal_apd)
    cal_dcd = int(configuration_data.cal_dcd)
    los_type = configuration_data.los_type

    run_extinction_ratio = configuration_data.run_er
    cal_prbs_mode = int(configuration_data.prbs_mode)
    board_type = configuration_data.board
    tx_wavelength_nm = configuration_data.tx_wavelength_nm
    rx_wavelength_nm = configuration_data.rx_wavelength_nm
    opm_type = configuration_data.opm_type
    thermistor_type = configuration_data.thermistor_type
    cal_mode_str = configuration_data.cal_mode
    cal_mode = int(cal_mode_str)

    echo("$$$  starting calibration  $$$############### ", 1)

    # Coordinated Universal Time (UTC)
    utc_time_stamp = datetime.utcnow().strftime("%y%m%d_%H%M")
    script_cl = version.CALIBRATION_SCRIPTS_CL.split()[1]
    calibration_manifest = '%s.%sL.%s;%s;%s' % (version.BRCM_VERSION, version.BRCM_RELEASE, version.BRCM_EXTRAVERSION,
                                               script_cl, utc_time_stamp)
    send2shell("laser calibration --start %s" % calibration_manifest)

    if int(thermistor_type) == 1:
        thermistor_val = Vishay_NTHS0603N01N1003FE
        print ("thermistor_type = Vishay_NTHS0603N01N1003FE")
    elif int(thermistor_type) == 2:
        thermistor_val = TDK_NTCG104EF104FT1X
        print ("thermistor_type = TDK_NTCG104EF104FT1X")
    elif int(thermistor_type) == 3:
        thermistor_val = MURATA_NCP15WF104F03RC
        print ("thermistor_type = MURATA_NCP15WF104F03RC")
    else:
        thermistor_val = Vishay_NTHS0603N01N1003FE
        print ("thermistor_type not valid, using default= Vishay_NTHS0603N01N1003FE")

    send2shell("laser res2temp --set %s" % (thermistor_val))

    # converting apd mpd bias and mod value to firmware useable values

    # apd default 0.285 calculation example 65536*0.285=18677.76 =>round=>hex value=48F5
    apd_str = format(int(round(float(apd_vbr_rssi_config) * 65536)), '04x')
    apd_stepdown_str = format(int(apd_stepdown_config), '02x')

    # mpd_level default 1800  example hex(1800) = 0708
    mpd_str = format(int(max_mpd_config) * 2 ** 16 + int(min_mpd_config), '08x')

    # L0 default -11dBm example (10^(-11/10))*1000=79=> hex(79)=4f
    L0_str = format(int(round(L0_uW_config)), '04x')

    # modctrl default 3dBm  example (10^(3/10))*1000=1995=> hex(1995)=07cb
    L1_str = format(int(round(L1_uW_config)), '04x')

    # legacy parameter - not used
    not_used1_str = '0000'

    # rogue limits

    # extinction ratio default 13dB  
    er_str = format(int(round((10 ** (float(er_target_config) / 10)) * 256)), '04x')

    # bias current limits
    bias_str = format(int(max_biasctl_config), '04x')

    # modulation current limits
    mod_str = format(int(max_modctl_config), '04x')

    # min optical power limit
    min_op_str = format(int(min_op_config), '04x')

    # eyesafety threshold limits
    eyesafety_str = format(int(max_eyesafety_config) * 2 ** 8 + int(min_eyesafety_config), '04x')

    # rogue threshold limits
    rogue_str = format(int(max_rogue_config) * 2 ** 8 + int(min_rogue_config), '04x')

    # formatting string to set
    user_calibration_set = (
            apd_str +
            mpd_str +
            L0_str + L1_str +
            not_used1_str +
            er_str +
            bias_str + mod_str +
            min_op_str +
            eyesafety_str +
            rogue_str +
            apd_stepdown_str + '00'
    )

    # connecting to opm and attenuator
    connect_to_attenuator(attenuator_address)
    if (opm_type == 'keysight'):
        connect_to_opm_keysight(tx_opm_address, 'tx', tx_wavelength_nm)
        connect_to_opm_keysight(rx_opm_address, 'rx', rx_wavelength_nm)
    else:
        connect_to_opm_thorlabs(tx_opm_address, 'tx', tx_wavelength_nm)
        connect_to_opm_thorlabs(rx_opm_address, 'rx', rx_wavelength_nm)

    if int(run_extinction_ratio) == 1:
        connect_to_dca(dca_address)

    block_incoming_light()

    # run scripts

    # stop statistic collection
    send2shell("laser calibration --set 61 0")

    # work in open loop
    send2shell("laser calibration --set 60 1")

    if int(board_type) == 1:
        # FHBB
        send2shell("laser msg --set 8f 2 0003")
        send2shell("laser msg --set 91 4 0003ffff")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 3")
        send2shell("laser calibration --set 26 0 3ffff")
        send2shell("laser calibration --set 24 0 0 0")
    elif int(board_type) == 2:
        # FHBBST
        send2shell("laser msg --set 8f 2 0005")
        send2shell("laser msg --set 91 4 00003fff")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 5")
        send2shell("laser calibration --set 26 0 3fff")
        send2shell("laser calibration --set 24 0 0 0")
    elif int(board_type) == 3:
        # 68910 GPON
        # send2shell( "laser msg --set 8f 2 0006" )
        # send2shell( "laser msg --set 91 4 000003ff" )
        # send2shell( "laser calibration --set 25 6" )
        # send2shell( "laser calibration --set 26 3ff" )
        send2shell("laser msg --set 8f 2 0000")
        send2shell("laser msg --set 91 4 00000000")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 0")
        send2shell("laser calibration --set 26 0 0")
        send2shell("laser calibration --set 24 0 0 0")
    elif int(board_type) == 4:
        # 68910 XGPON
        send2shell("laser msg --set 8f 2 0000")
        send2shell("laser msg --set 91 4 00000000")
        # send2shell( "laser msg --set 98 2 0000" )
        send2shell("laser msg --set 98 2 0009")
        send2shell("laser calibration --set 25 1 0")
        send2shell("laser calibration --set 26 1 0")
        # send2shell( "laser calibration --set 24 1 0 0" )
        send2shell("laser calibration --set 24 1 9 0")
        # 68910 10EPON
        send2shell("laser calibration --set 25 0 0")
        send2shell("laser calibration --set 26 0 0")
        send2shell("laser calibration --set 24 0 9 0")
    elif int(board_type) == 5:
        # 6836 GPON
        send2shell("laser msg --set 8f 2 0004")
        send2shell("laser msg --set 91 4 0001ffff")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 4")
        send2shell("laser calibration --set 26 0 1ffff")
        send2shell("laser calibration --set 24 0 0 0")
    elif int(board_type) == 6:
        # 63158 GPON (combo DSL + GPON)
        send2shell("laser msg --set 8f 2 0004")
        send2shell("laser msg --set 91 4 00000fff")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 4")
        send2shell("laser calibration --set 26 0 fff")
        send2shell("laser calibration --set 24 0 0 0")
    elif int(board_type) == 7:
        # 6846 GPON
        send2shell("laser msg --set 8f 2 0004")
        send2shell("laser msg --set 91 4 0003ffff")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 4")
        send2shell("laser calibration --set 26 0 3ffff")
        send2shell("laser calibration --set 24 0 0 0")
    elif int(board_type) == 8:
        # 6836 GPON - DCA 1.25G filter
        send2shell("laser msg --set 8f 2 0000")
        send2shell("laser msg --set 91 4 00000000")
        send2shell("laser msg --set 98 2 3f1c")
        send2shell("laser calibration --set 25 0 0")
        send2shell("laser calibration --set 26 0 0")
        send2shell("laser calibration --set 24 0 1c 3f")
    elif int(board_type) == 9:
        # 6878 GPON - DCA 1.25G filter
        send2shell("laser msg --set 8f 2 0003")
        send2shell("laser msg --set 91 4 00007fff")
        send2shell("laser msg --set 98 2 0000")
        send2shell("laser calibration --set 25 0 3")
        send2shell("laser calibration --set 26 0 7fff")
        send2shell("laser calibration --set 24 0 0 0")
    else:
        echo(" ERROR: BORAD TYPE NOT DETERMINE !!!", 1)
        return

    # set calibration parameters according to configuration.ini data
    send2shell("laser msg --set af %d %s" % (len(user_calibration_set) / 2, user_calibration_set))

    print "Calibration mode = " + cal_mode_str
    # init calibration process
    send2shell("laser msg --set ae 2 0001")
    # Wait for process to end
    OPC()

    # RX calibration
    if ((cal_mode & 1) > 0):
        # Run apd calibration
        if (cal_apd == 1):
            call_script(apd_cal, start_time, log_file, board_serial_number, configuration_data)

    # TX calibration
    if ((cal_mode & 2) > 0):
        # Run open_loop calibration
        call_script(open_loop_cal, start_time, log_file, board_serial_number, configuration_data)

        # Run MPD calibration
        call_script(mpd_cal, start_time, log_file, board_serial_number)

        # Run DCD calibration
        if (cal_dcd == 1):
            call_script(dcd_cal, start_time, log_file, board_serial_number, configuration_data)

        # Run laser driver - tracking targets calibration
        call_script(tracking_cal_tracking_targets, start_time, log_file, board_serial_number, configuration_data)

        # Run Tune average optical power
        call_script(avrg_op_cal, start_time, log_file, board_serial_number, configuration_data)

        # Run Tune extinction ratio
        if (int(run_extinction_ratio)) == 1:
            # Unblock incoming light to DUT
            unblock_incoming_light()
            # Set attenuator attenuation level for incoming light level to DUT to -27dBm
            set_attenuation_level(float(olt_to_onu_att1_db))
            call_script(extinction_ratio_cal, start_time, log_file, board_serial_number, configuration_data)

        die_temp_ref = laser_msg_get(0x7, 2)

        # Run laser driver - tracking targets calibration
        if cal_prbs_mode == 1:
            call_script(tracking_cal_tracking_targets, start_time, log_file, board_serial_number, configuration_data)

        # Run laser driver - first burst targets calibration
        call_script(tracking_cal_first_burst_targets, start_time, log_file, board_serial_number)

        # Run eyesafety threshold validation
        call_script(eyesafety_cal, start_time, log_file, board_serial_number)

        # Run rogue threshold validation
        call_script(rogue_cal, start_time, log_file, board_serial_number)

    # RX calibration
    if ((cal_mode & 1) > 0):
        # Run RSSI calibration
        call_script(rssi_cal, start_time, log_file, board_serial_number, configuration_data)

        # Run LOS calibration
        if (los_type == 'lia_los'):
            # LIA-LOS
            call_script(los_cal, start_time, log_file, board_serial_number, configuration_data)
        else:
            # RSSI-LOS
            adf_los_configuration_data = calc_adf_los_thresholds.main(log_file, board_serial_number, configuration_data)
            adf_los_assert_th = format(adf_los_configuration_data.los_assert_th, '0x')
            adf_los_deassert_th = format(adf_los_configuration_data.los_deassert_th, '0x')
            adf_lb_bucket_sz = format(adf_los_configuration_data.lb_bucket_sz, '0x')
            adf_lb_assert_th = format(adf_los_configuration_data.lb_assert_th, '0x')

    # reading calibration results
    data = read_calibration_results()

    apd_vol = data.apd_vol
    vga = data.vga
    dac = data.dac
    tia = data.tia
    calibctrl = data.calibctrl
    fb_level_0_dac_ref = data.fb_level_0_dac_ref
    fb_level_1_dac_ref = data.fb_level_1_dac_ref
    tracking_level_0_dac_ref = data.tracking_level_0_dac_ref
    tracking_level_1_dac_ref = data.tracking_level_1_dac_ref
    biasctrl = data.biasctrl
    modctrl = data.modctrl
    esc_thr = data.esc_thr
    rssi_a_factor = data.rssi_a_factor
    rssi_b_factor = data.rssi_b_factor
    rssi_c_factor = data.rssi_c_factor
    lia_los_assert_thr = data.los_assert_thr
    lia_los_deassert_thr = data.los_deassert_thr
    lia_sat_pos_high = data.sat_pos_high
    lia_sat_pos_low = data.sat_pos_low
    lia_sat_neg_high = data.sat_neg_high
    lia_sat_neg_low = data.sat_neg_low
    tx_power = data.tx_power
    bias0 = data.bias0
    dcyctl = data.dcyctl

    log_file.write("\n\ncalibration results:\n")

    if ((cal_mode & 2) > 0):
        log_file.write("tia=%s \t vga=%s \t dac=%s \t  calibctrl=%s \n" % (tia, vga, dac, calibctrl))
        log_file.write(
            "fb_level_0_dac_ref=%s \t fb_level_1_dac_ref=%s  \ntracking_level_0_dac_ref=%s \t tracking_level_1_dac_ref=%s\n" % (
                fb_level_0_dac_ref, fb_level_1_dac_ref, tracking_level_0_dac_ref, tracking_level_1_dac_ref))
        log_file.write("biasctrl=%s \t modctrl=%s\n" % (biasctrl, modctrl))
        log_file.write("dcyctl=%s \n" % (dcyctl))
        log_file.write("esc_thr=%s \n" % (esc_thr))
        log_file.write("tx_power=%s \t bias0=%s\n" % (tx_power, bias0))

    if ((cal_mode & 1) > 0):
        log_file.write("apd_vol=%s \n" % (apd_vol))
        log_file.write("rssi_a_factor=%s \t rssi_b_factor=%s \t rssi_c_factor=%s\n" % (
            rssi_a_factor, rssi_b_factor, rssi_c_factor))
        if (los_type == 'lia_los'):
            # lia-los
            log_file.write(
                "lia-los:los_assert_thr=%s \t los_deassert_thr=%s \t sat_pos_high=%s \t  sat_pos_low=%s \t sat_neg_high=%s \t  sat_neg_low=%s \n" % (
                    lia_los_assert_thr, lia_los_deassert_thr, lia_sat_pos_high, lia_sat_pos_low, lia_sat_neg_high,
                    lia_sat_neg_low))
        else:
            # adf-los
            log_file.write(
                "adf-los:los_assert_thr=%s \t los_deassert_thr=%s \t lb_bucket_sz=%s \t lb_assert_th=%s \n" % (
                    adf_los_assert_th, adf_los_deassert_th, adf_lb_bucket_sz, adf_lb_assert_th))

    # Stop calibration process
    send2shell("laser msg  --set ae 2 000f")
    # wait for complete
    OPC()

    # restore statistic collection
    send2shell("laser calibration --set 61 1")

    # work in close loop
    send2shell("laser calibration --set 60 0")

    # Burn results to flash

    # TX calibration
    if ((cal_mode & 2) > 0):
        # Open loop
        send2shell("laser calibration --set 4 %s" % (biasctrl))
        send2shell("laser calibration --set 5 %s" % (modctrl))

        # DCD control
        if (cal_dcd == 1):
            send2shell("laser calibration --set 27 %s" % (dcyctl))

        # MPD
        send2shell("laser calibration --set 7 %s %s" % (tia, vga))
        send2shell("laser calibration --set 20 %s" % (dac))
        send2shell("laser calibration --set 28 %s" % (calibctrl))

        # Tracking
        send2shell("laser calibration --set 2 %s" % (fb_level_0_dac_ref))
        send2shell("laser calibration --set 3 %s" % (fb_level_1_dac_ref))
        send2shell("laser calibration --set 18 %s" % (tracking_level_0_dac_ref))
        send2shell("laser calibration --set 19 %s" % (tracking_level_1_dac_ref))

        # Eyesafety
        send2shell("laser calibration --set 16 %s" % (esc_thr))

        # TX_POWER
        send2shell("laser calibration --set 29 %s" % (tx_power))

        # BIAS0
        send2shell("laser calibration --set 30 %s" % (bias0))

        # Tracking compensation
        enable_mask = int(configuration_data.tracking_compensation_enable0) | \
                      (int(configuration_data.tracking_compensation_enable1) << 1)
        send2shell("laser calibration --set 37 %s %x %x %x" % (enable_mask, die_temp_ref,
                                                               int(configuration_data.tracking_compensation_coeff1_q8),
                                                               int(configuration_data.tracking_compensation_coeff2_q8)))

        compensation_val = enable_mask | (die_temp_ref << 8) | \
                           (int(configuration_data.tracking_compensation_coeff1_q8) << 16) | \
                           (int(configuration_data.tracking_compensation_coeff2_q8) << 24)
        send2shell("laser msg --set c0 4 %08x" % (compensation_val))

    # RX calibration
    if ((cal_mode & 1) > 0):
        # APD
        # DEPRECATED send2shell("laser calibration --set 6 0 %s" % (apd_vol))
        # RSSI
        send2shell("laser calibration --set 10 %s" % (rssi_a_factor))
        send2shell("laser calibration --set 11 %s" % (rssi_b_factor))
        send2shell("laser calibration --set 12 %s" % (rssi_c_factor))

        # LOS
        if (los_type == 'lia_los'):
            # LIA-LOS
            send2shell("laser calibration --set 21 %s %s" % (lia_los_assert_thr, lia_los_deassert_thr))
            send2shell("laser calibration --set 22 %s %s" % (lia_sat_pos_high, lia_sat_pos_low))
            send2shell("laser calibration --set 23 %s %s" % (lia_sat_neg_high, lia_sat_neg_low))
        else:
            # RSSI-LOS
            send2shell("laser calibration --set 35 %s %s" % (adf_los_assert_th, adf_los_deassert_th))
            send2shell("laser calibration --set 36 %s %s" % (adf_lb_assert_th, adf_lb_bucket_sz))

    # Run results logging
    call_script(flash_log, start_time, log_file, board_serial_number)

    send2shell("laser calibration --stop %s" % calibration_manifest)

    echo("$$$  calibration complete $$$############### ", 1)

    # timing the script
    end_time = time.time()
    log_file.write("\n\ntime of script is %f seconds\n" % ((end_time - start_time)))
    log_file.close()

    close_connection()


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# function for calling each sub_script
def call_script(script_name, start_time, log_file, board_serial_number, configuration_data=None):
    process_start_time = time.time()
    if (configuration_data == None):
        script_name.main(log_file, board_serial_number)
    else:
        script_name.main(log_file, board_serial_number, configuration_data)

    # read calibration results (debug)
    send2shell("laser msg --get 17 50")

    # timing the script
    process_time = time.time()
    log_file.write("\n %s process time is %f seconds\n" % (str(script_name), (process_time - process_start_time)))
    echo("$$$  starting next procedure  $$$############### ", 1)


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if __name__ == "__main__":
    main()
