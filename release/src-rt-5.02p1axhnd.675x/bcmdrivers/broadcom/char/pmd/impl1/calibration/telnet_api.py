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

from _codecs import register
import time, os, telnetlib, re, sys
from calibration_functions import *

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# loading the crt object
# def Init_api(obj):
#    global crt
#    crt = obj
#    return

HOST = "192.168.1.1"
user = "admin"
password = "admin"


def Init_api():
    global tn

    tn = telnetlib.Telnet(HOST)

    tn.read_until("Login: ", 3)
    tn.write(user + '\r\n')

    tn.read_until("Password: ", 3)
    tn.write(password + '\r\n')

    tn.read_until(" >", 3)

    tn.write("cat /proc/pmd/calibration &\r\n")

    tn.read_until(" >", 3)

    print "*** Telnet connection to " + HOST + " is established ***\n"

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def close_connection():
    global tn

    tn.close()
    del tn


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
def send2shell(command, prompt=" > ", delay=0.05):
    '''
    Sends command to shell
    waits for string " >" and pause for "delay" time
    '''
    global command_response

    tn.read_very_eager()  # clean buffer
    tn.write(command + '\r\n')
    command_response = tn.read_until(prompt, 4)
    print command_response

    result = command_response.find(prompt)
    if (result == -1):
        print "ERR: command (" + command + ") ended in timeout"

    return command_response


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


def write_read(order, text=" >", delay=0.05, timeout=5, word_clip_num=0):
    '''
    This function is emptying the memory
    sending the "order" and reading data before "text" or " >" by default or till timeout is reached
    sleep is the time(in seconds) between the command sent to the time it is read
    word_clip_num is number of words needed to be cliped to get the text
    '''
    tn.read_very_eager()  # clean buffer
    tn.write(order + '\r\n')
    # time.sleep( delay )
    response = tn.read_until(text, timeout)  # reading data after command till text
    response = response.replace(order + '\r\n', "")
    response = response.replace('\r\n', ' ')
    response = response.split(' ')

    back = response[word_clip_num]  # clips the text to get the number

    return back


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


def write_read_all_text(order, text=" >", delay=0.05, timeout=5):
    '''
    This function is emptying the memory
    sending the "order" and reading data before "text" or " >" by default or till timeout is reached
    sleep is the time(in seconds) between the command sent to the time it is read
    word_clip_num is number of words needed to be cliped to get the text
    '''
    tn.read_very_eager()  # clean buffer
    tn.write(order + '\r\n')
    text_back = tn.read_until(text, timeout)  # reading data after commend till text

    return text_back


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def echo(text, flag=0):
    '''
    this function allow echoing to screen according to flag
    flag is 0 by default which means no printing
    flag 1 echos the text sent
    '''
    if flag == 0:
        return
    else:
        tn.write(
            "echo ###############################################" + text + '\r')  # sending basic value to pmd
    ret = tn.read_until(" >", 2)
    print ret
    tn.write('\r\n')  # sends command and carriage return
    tn.read_until(" >", 2)

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def start_prbs():
    """
    This function starts continues prbs
    """
    tn.read_very_eager()  # clean buffer
    tn.write("sh" + '\r\n')
    tn.read_until("#", 2)
    time.sleep(0.05)
    tn.write("bs /misc prbs gpon 31 0" + '\r')
    tn.read_until("#", 2)
    time.sleep(0.05)
    tn.write("exit" + '\r\n')
    tn.read_until(" >", 2)
    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def stop_prbs():
    '''
    This function stops continues prbs
    '''
    tn.read_very_eager()  # clean buffer
    tn.write("sh" + '\r\n')
    tn.read_until("#", 2)
    time.sleep(0.05)
    tn.write("bs /misc prbs none 31 0" + '\r')
    tn.read_until("#", 2)
    time.sleep(0.05)
    tn.write("exit" + '\r\n')
    tn.read_until(" >", 2)
    tn.write("laser msg --set 95 2 0000" + '\r')
    tn.read_until(" >", 2)
    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def start_burst(board_type, msstop=2304):
    '''
    start DUT bursts transmission
    '''
    tn.read_very_eager()  # clean buffer
    tn.write("sh" + '\r\n')
    tn.read_until("#", 2)
    time.sleep(0.05)
    msstop_hex = format(int(msstop), '04x')
    if board_type == 4:
        tn.write(
            "bs /b/c gpon misc_tx={enable=yes,overhead=aaaaaaaaaaaaaaaa,overhead_len=8,overhead_repetition=2,overhead_repetition_len=0,msstart=100,msstop=%s}\r" % (
                msstop_hex))
    else:
        tn.write(
            "bs /b/c gpon misc_tx={enable=yes,overhead=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,overhead_len=10,overhead_repetition=0,overhead_repetition_len=0,msstart=100,msstop=%s}\r" % (
                msstop_hex))
    tn.read_until("#", 2)
    time.sleep(0.05)
    tn.write("exit" + '\r\n')
    tn.read_until(" >", 2)
    # work around a SW bug
    send2shell("laser msg --set 84 2 0001")
    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def stop_burst(board_type=0):
    '''
    stop DUT bursts transmission
    '''
    tn.read_very_eager()  # clean buffer
    tn.write("sh" + '\r\n')
    tn.read_until("#", 2)
    time.sleep(0.05)
    if board_type == 4:
        tn.write(
            "bs /b/c gpon misc_tx={enable=no,overhead=aaaaaaaaaaaaaaaa,overhead_len=8,overhead_repetition=2,overhead_repetition_len=0,msstart=100,msstop=2000}\r")
    else:
        tn.write(
            "bs /b/c gpon misc_tx={enable=no,overhead=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,overhead_len=10,overhead_repetition=0,overhead_repetition_len=0,msstart=100,msstop=2000}" + '\r')
    tn.read_until("#", 2)
    time.sleep(0.05)
    tn.write("exit" + '\r\n')
    tn.read_until(" >", 2)
    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


def read_reg(order, prompt=" >"):
    '''
    This function reads reg value
    '''
    tn.read_very_eager()  # clean buffer
    tn.write(order + '\r\n')
    result = tn.read_until(prompt, 3)
    result = result.replace(order + '\r\n', "")
    result = result.replace('\r\n', ' ')
    result = result.split(' ')
    try:
        string1 = result[0]
    except:
        string1 = "error reading reg!! " + string1

    return string1


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def set_regul():
    '''
    this function reset the regulator in case it is not active
    '''
    # Read regulator data
    regulator = int(read_reg("laser general --get 0 404 4"))

    if regulator != 2:
        send2shell("laser general --set 0 1a4 4 00000080")
    return regulator


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def get_regulator_status():
    '''
    this function gets the regulator status
    '''
    # Read regulator data
    regulator = int(read_reg("laser general --get 0 404 4"))

    return regulator


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def renable_regulator_status():
    '''
    this function checks the ragulator status,
    trys to reset it if necessary
    if it succeeds returns True else returns False
    '''
    # Read regulator data
    regulator = int(write_read("laser general --get 0 404 4"))
    if regulator != 2:
        send2shell("laser general --set 0 1a4 4 00000080")
    regulator = int(write_read("laser general --get 0 404 4"))

    if regulator == 2:
        return True
    else:
        return False


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def OPC():
    '''
    Waits for oparation compelete signal
    '''
    global command_response

    result = command_response.find("PMD: Calibration ")
    if (result == -1):
        response = tn.read_until("PMD: Calibration ", 15)
        result = response.find("PMD: Calibration ")
    else:
        response = command_response

    if (result == -1):
        print "\nERR: Calibration timeout\n\n\n"
        sys.exit(1)
    else:
        response = response + tn.read_very_eager()
        print response
        result = response.find("state over")
        if (result == -1):
            result = response.find("failure")
            if (result == -1):
                print "\nERR: Calibration timeout\n\n\n"
            else:
                print "\nERR: Calibration failure\n\n\n"
            sys.exit(1)
        else:
            print "\nWe are good!!\n\n\n"

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def laser_msg_get(msg_id, msg_len, ret_format='hex'):
    '''
    Reads laser msg get result, format = {'hex','float','dec'}

    Sends get command to crt and unwrap the values according to the data structure

    '''
    # Read results and save to flash
    command_str = 'laser msg --get %x %d' % (msg_id, msg_len)
    n_iter = 0
    while n_iter < 10:
        registers_value = send2shell(command_str)
        registers_value = registers_value.replace(command_str + '\r\n', '')
        registers_value = registers_value.replace('\r\n', ' ')
        registers_value = registers_value.split(' ')
        try:
            ret_list = []
            for k in range((msg_len + 2) / 4):
                if ret_format == 'hex':
                    t_val = int(registers_value[k], 16)
                elif ret_format == 'dec':
                    t_val = int(registers_value[k])
                elif ret_format == 'float':
                    t_val = float(registers_value[k])
                else:
                    raise ValueError('ret_format=%s is not supported' % (ret_format))
                    return -1
                ret_list.append(t_val)
            if len(ret_list) == 1:
                return ret_list[0]
            else:
                return ret_list
        except:
            n_iter += 1

    print 'cannot parse correctly returned string'
    return -1


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def read_calibration_results():
    '''
    Reads firmware calibration results

    Sends get command to crt and unwrap the values according to the data structure

    '''
    # Read results and save to flash
    registers_value = send2shell("laser msg --get 17 50")
    registers_value = registers_value.replace("laser msg --get 17 50\r\n", "")
    registers_value = registers_value.replace('\r\n', ' ')
    registers_value = registers_value.split(' ')
    # print registers_value

    # Save results (count from last array)
    apd_vol = registers_value[0][0:4]
    vga = '00' + registers_value[0][4:6]
    dac = '00' + registers_value[0][6:8]
    tia = '00' + registers_value[1][0:2]
    calibctrl = '00' + registers_value[1][2:4]
    fb_level_0_dac_ref = registers_value[1][4:8]
    fb_level_1_dac_ref = registers_value[2][0:4]
    tracking_level_0_dac_ref = registers_value[2][4:8]
    tracking_level_1_dac_ref = registers_value[3][0:4]
    biasctrl = registers_value[3][4:8]
    modctrl = registers_value[4][0:4]
    esc_thr = registers_value[4][4:8]
    rssi_a_factor = registers_value[5][0:8]
    rssi_b_factor = registers_value[6][0:8]
    rssi_c_factor = registers_value[7][0:8]
    los_assert_thr = registers_value[8][0:2]
    los_deassert_thr = registers_value[8][2:4]
    sat_pos_high = registers_value[8][4:6]
    sat_pos_low = registers_value[8][6:8]
    sat_neg_high = registers_value[9][0:2]
    sat_neg_low = registers_value[9][2:4]
    tx_power = registers_value[9][4:8]
    bias0 = registers_value[10][0:4]
    bias_delta_i = registers_value[10][4:8]
    slope_efficiency = registers_value[11][0:4]
    # rogue_dark = registers_value[11][4:8]
    dcyctl = registers_value[12][0:4]
    # calculate rssi a b c value
    rssi_a_factor = (float(twos_comp(int(rssi_a_factor, 16), 32))) / 256
    rssi_b_factor = (float(twos_comp(int(rssi_b_factor, 16), 32))) / 256
    rssi_c_factor = (float(twos_comp(int(rssi_c_factor, 16), 32))) / 256

    # converts data to a class object
    data = CalibrationResults([apd_vol,
                               vga,
                               dac,
                               tia,
                               calibctrl,
                               fb_level_0_dac_ref,
                               fb_level_1_dac_ref,
                               tracking_level_0_dac_ref,
                               tracking_level_1_dac_ref,
                               biasctrl,
                               modctrl,
                               esc_thr,
                               rssi_a_factor,
                               rssi_b_factor,
                               rssi_c_factor,
                               los_assert_thr,
                               los_deassert_thr,
                               sat_pos_high,
                               sat_pos_low,
                               sat_neg_high,
                               sat_neg_low,
                               tx_power,
                               bias0,
                               bias_delta_i,
                               slope_efficiency,
                               dcyctl])

    return data


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CalibrationResults(object):
    def __init__(self, data):
        """

        :rtype: object with fields:
                    apd_vol
                    vga
                    dac
                    tia
                    calibctrl
                    fb_level_0_dac_ref
                    fb_level_1_dac_ref
                    tracking_level_0_dac_ref
                    tracking_level_1_dac_ref
                    biasctrl
                    modctrl
                    esc_thr,
                    rssi_a_factor,
                    rssi_b_factor,
                    rssi_c_factor,
                    los_assert_thr,
                    los_deassert_thr,
                    sat_pos_high,
                    sat_pos_low,
                    sat_neg_high,
                    sat_neg_low,
                    tx_power,
                    bias0,
                    bias_delta_i,
                    slope_efficiency
                    dcyctl

        """
        self.apd_vol = data[0]
        self.vga = data[1]
        self.dac = data[2]
        self.tia = data[3]
        self.calibctrl = data[4]
        self.fb_level_0_dac_ref = data[5]
        self.fb_level_1_dac_ref = data[6]
        self.tracking_level_0_dac_ref = data[7]
        self.tracking_level_1_dac_ref = data[8]
        self.biasctrl = data[9]
        self.modctrl = data[10]
        self.esc_thr = data[11]
        self.rssi_a_factor = data[12]
        self.rssi_b_factor = data[13]
        self.rssi_c_factor = data[14]
        self.los_assert_thr = data[15]
        self.los_deassert_thr = data[16]
        self.sat_pos_high = data[17]
        self.sat_pos_low = data[18]
        self.sat_neg_high = data[19]
        self.sat_neg_low = data[20]
        self.tx_power = data[21]
        self.bias0 = data[22]
        self.bias_delta_i = data[23]
        self.slope_efficiency = data[24]
        self.dcyctl = data[25]


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def logger(log_file, txt):
    '''
    Writes text to log file
    '''
    log_file.write(txt + "\n")
    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def open_log_file(file_name_location, board_serial_number):
    '''
    Opens a log file with wanted syntax
    '''
    log_file = open(file_name_location, 'w+')
    log_file.write("Board serial number:%s\n" % (board_serial_number))
    return log_file


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def log_file_header(log_file, header_text):
    """
    creats a script header in the log file
    :type log_file: object
    """
    log_file.write("\n##################   %s   ################ \n\n\n" % (header_text))

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def twos_comp(val, bits):
    """compute the 2's compliment of int value val"""
    if (val & (1 << (bits - 1))) != 0:  # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)  # compute negative value
    return val  # return positive value as is


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def read_internal_runtime(log_file):
    # Read internal runtime
    T0 = int(write_read("laser general --get 0 844 4"), 16)
    T0_hex = format(T0, 'x')
    log_file.write("Internal FW runtime is %d\t\n" % (T0))

    return
