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

import time, os, string, sys

# (*) if visa import is needed
import visa
import serial
import math
import serial.tools.list_ports
import numpy as np

tx_opm_handle = None
rx_opm_handle = None
attenuator_handle = None
prologix_serial = None
dca_handle = None
rx_opm_type = None
tx_opm_type = None
rx_opm_ch = ''
tx_opm_ch = ''
att_ch = ''
is_prologix = None
is_golight = None

golight_va160s_commands = {
    'product_name': [ 0x00, 0x52, 0x44, 0x50, 0x4E ],
    'block_light': [0x00, 0x53, 0x54, 0x53, 0x54, 0x01, 0x00 ],
    'unblock_light': [0x00, 0x53, 0x54, 0x53, 0x54, 0x01, 0x01 ],
    'set_atten' : [0x00, 0x53, 0x54, 0x41, 0x54, 0x01 ]
    }

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# loading the crt object
#def Init_cal(obj):
#    global crt
#    crt = obj
#    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def get_borad_serial_number():
    '''
    Prompt the user for board serial number
    '''
    board_serial_number = ''
    # board_serial_number=crt.Dialog.Prompt("Please enter board serial number(integer):", "serial number", "")
    return board_serial_number

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class device_address_attributes( object ):
    def __init__(self, addr_str):

        # parse device_address to address and channel
        address_fields = addr_str.split('::')
        last_field = address_fields[-1].lower()
        if ( 'ch' in last_field ):
            self.device_address = string.join(address_fields[:-1],'::')
            self.ch_no = last_field.split('ch')[1]
        else:
            self.device_address = addr_str
            self.ch_no = ''


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def connect_to_prologix():

    global prologix_serial

    # get a list of COM ports
    ports_list = serial.tools.list_ports.comports()

    # iterate list
    print 'looking for COM port connected to PROLIGIX USB-GPIB ...\n'
    for port in ports_list:
        port_name = port.device
        hwid = port.hwid
        print 'checking port %s ...\n' %(port_name)
        if ( 'usb' in hwid.lower() ):
            # test if connected to prologix
            try:
                prologix_serial = serial.Serial(port_name,timeout=1)
                if (prologix_serial.is_open):
                    prologix_serial.write('++ver\r')
                    retstr = prologix_serial.readline()
                    if ('prologix' in retstr.lower()):
                        break
                    else:
                        prologix_serial.close()
                else:
                    print 'cannot open %s, access is denied\n' %(port_name)
            except serial.SerialException:
                print 'error opening %s\n' %(port_name)

    if (prologix_serial.is_open):
        print 'prologix is connected to %s\n' %(port_name)
    else:
        sys.exit('terminate script: cannot find prologix')

    return 0

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def golight_gen_command(command_array):
    n = len(command_array)
    command_array = [0xAA, n] + command_array
    chksum = sum(command_array) % 256
    command_array = command_array + [chksum]
    return command_array

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def connect_to_golight():

    global golight_serial

    # golight idetntifcation message
    command_array = golight_gen_command(golight_va160s_commands.get('product_name'))

    # get a list of COM ports
    ports_list = serial.tools.list_ports.comports()

    # iterate list
    print 'looking for COM port connected to GOLIGHT ...\n'
    for port in ports_list:
        port_name = port.device
        hwid = port.hwid
        print 'checking port %s ...\n' %(port_name)
        if ( 'usb' in hwid.lower() ):
            # test if connected to golight
            try:
                golight_serial = serial.Serial(port_name,timeout=1)
                if (golight_serial.is_open):
                    golight_serial.write(command_array)
                    time.sleep( 0.1 )
                    nbytes = golight_serial.in_waiting
                    ret_array = golight_serial.read(nbytes)
                    if ('va160s' in ret_array.lower()):
                        break
                    else:
                        golight_serial.close()
                else:
                    print 'cannot open %s, access is denied\n' %(port_name)
            except serial.SerialException:
                print 'error opening %s\n' %(port_name)

    if (golight_serial.is_open):
        print 'golight is connected to %s\n' %(port_name)
    else:
        sys.exit('terminate script: cannot find golight')

    return 0


# attenuator functions
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def connect_to_attenuator(device_address):
    '''
    Connects to a device via visa and return a handle for the device
    :rtype: object
    '''

    global prologix_serial
    global attenuator_handle
    global is_prologix
    global is_golight
    global attx_opm_handlet_ch

    flag = False

    # parse device_address to address and channel
    device_addr_attr = device_address_attributes(device_address)
    device_address = device_addr_attr.device_address     
    att_ch = device_addr_attr.ch_no

    print 'connecting to attenuator, address=%s, ch=%s ...\n' %(device_address,att_ch)
    
    # resolve address
    is_prologix = False
    is_golight = False
    if ('PROLOGIX' in device_address):
        # search which COM port is connected to prologix
        connect_to_prologix()
        is_prologix = True
        # extract attenuator gpib address
        gpib_str = device_address.split('::')[1]
        gpib_addr = int(gpib_str.replace( 'GPIB', '' ))
        # configure prologix
        prologix_serial.write('++mode 1\r++addr %d\r++auto 1\r' %(gpib_addr))
        # send identification command to attenuator
        prologix_serial.write('*idn?\r')
        retstr = prologix_serial.readline()
        if ( 'jds' in retstr.lower() ):
            flag = True
    elif ('GOLIGHT' in device_address):
        # search which COM port is connected to GOLIGHT attenuator
        connect_to_golight()
        is_golight = True
        return
    else:
        # visa
        retstr = None
        rm = visa.ResourceManager('C:\\Windows\\System32\\agvisa32.dll')
        attenuator_handle = rm.open_resource( device_address )
        if (attenuator_handle!=None):
            flag = True

    if (flag):
        print ('connected to JDS attenuator\n')
    else:
        sys.exit('%s\nterminate script: cannot connect to JDS JDS attenuator' %(retstr))

    return 


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def block_incoming_light():
    '''
    Blocks all incoming light to DUT
    '''

    global prologix_serial
    global golight_serial
    global attenuator_handle
    global att_ch

    command_str = ':OUTPut%s 0' %(att_ch)

    try:
        #att_handle.query( att_block_commend )
        if (is_prologix):
            prologix_serial.write(command_str + '\r')
        elif (is_golight):
            command_array = golight_gen_command(golight_va160s_commands.get('block_light'))
            golight_serial.write(command_array)
        else:
            attenuator_handle.write( command_str )
    except:
        pass
    # wait for opartion complete
    time.sleep( 1 )

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def unblock_incoming_light():
    '''
    Unblocks incoming light to DUT
    '''

    global prologix_serial
    global golight_serial
    global attenuator_handle
    global att_ch

    command_str = ':OUTPut%s 1' %(att_ch)

    try:        
        if (is_prologix):
            prologix_serial.write(command_str + '\r')
        elif (is_golight):
            command_array = golight_gen_command(golight_va160s_commands.get('unblock_light'))
            golight_serial.write(command_array)
        else:
            #att_handle.query( att_block_commend )
            attenuator_handle.write( command_str )
    except:
        pass
    # wait for opartion complete
    time.sleep( 1 )

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def set_attenuation_level(attnuation_level):
    '''
    Sets an attenuation level at the OLT input
    according to "attnuation_level" value
    '''

    global prologix_serial
    global golight_serial
    global attenuator_handle
    global att_ch

    command_str = ':INPUT%s:ATT %f' % (att_ch,attnuation_level)

    try:
        if (is_prologix):
            prologix_serial.write(command_str + '\r')
        elif (is_golight):
            # attenuation single precision float (float32) in little endian ordering
            att_array = [ord(x) for x in list(np.float32(attnuation_level).tobytes())]
            command_array = golight_va160s_commands.get('set_atten') + att_array
            command_array = golight_gen_command(command_array)
            golight_serial.write(command_array)
        else:
            #att_handle.query( att_block_commend )
            attenuator_handle.write( command_str )
    except:
        pass
    # wait for opartion complete
    time.sleep( 1 )
    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



# opm functions
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def connect_to_opm_keysight(device_address, direction, wavelength_nm):
    '''
    Connects to a device via visa and return a handle for the device
    :rtype: object    
    '''

    global tx_opm_handle
    global rx_opm_handle
    global tx_opm_type
    global rx_opm_type
    global tx_opm_ch
    global rx_opm_ch

    # parse device_address to address and channel
    device_addr_attr = device_address_attributes(device_address)
    device_address = device_addr_attr.device_address     
    ch_no = device_addr_attr.ch_no
    
    print 'connecting to keysight optical power meter, address=%s, ch=%s ...\n' %(device_address,ch_no)


    rm = visa.ResourceManager('C:\\Windows\\System32\\agvisa32.dll')
    handle = rm.open_resource( device_address )
    if ( direction == 'rx' ):
        rx_opm_handle = handle
        rx_opm_type = 'keysight'
        rx_opm_ch = ch_no
    else:
        tx_opm_handle = handle
        tx_opm_type = 'keysight'
        tx_opm_ch = ch_no
    
    # set measurement mode and averaging
    command_str = 'INITIATE%s:CONTINUOUS 0;SENSE%s:POWER:ATIM 50MS' % (ch_no,ch_no) 
    handle.write( command_str )
    
    # set wavelength
    command_str = 'SENSE%s:POWER:WAVELENGTH %snm' %(ch_no,wavelength_nm)
    handle.write( command_str )

    print 'connected\n\n'

    return


def connect_to_opm_thorlabs(device_address, direction, wavelength_nm ):
    '''
    Connects to a device via visa and return a handle for the device
    :rtype: object    
    '''

    global rx_opm_handle
    global rx_opm_type
    global tx_opm_handle
    global tx_opm_type


    print 'connecting to thorlabs 1ch optical power meter, address=%s ...\n' %(device_address)

    rm = visa.ResourceManager( )
    handle = rm.open_resource( device_address )
    if ( direction == 'rx' ):
        rx_opm_handle = handle
        rx_opm_type = 'thorlabs'
    else:
        tx_opm_handle = handle
        tx_opm_type = 'thorlabs'

    # set averaging time to ~60msec
    command_str = 'average:count 20'
    handle.write( command_str )

    # set wavelength
    command_str = 'correction:wavelength ' + wavelength_nm
    handle.write( command_str )

    # set units W
    command_str = 'POW:UNIT W'
    handle.write( command_str )

    print 'connected\n\n'

    return
    
	
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



# DCA functions
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def connect_to_dca(device_address):
    '''
    Connects to a device via visa and return a handle for the device
    :rtype: object
    '''

    global dca_handle

    rm = visa.ResourceManager('C:\\Windows\\System32\\agvisa32.dll')
    dca_handle = rm.open_resource( device_address )

    return


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def measure_tx_optical_power_dbm():
    '''
    Meassures the optical tx power level in dBm.
    '''

    global tx_opm_handle
    global tx_opm_type
    global tx_opm_ch


    if (tx_opm_type == 'keysight'):
        # optical power meter, read from optical channel command
        command_str = 'READ%s:POWER?' % (tx_opm_ch)
        optical_power_level = tx_opm_handle.query( command_str )
        # Perform second read do to a bug in our optical device
        optical_power_level = tx_opm_handle.query( command_str )
    else:
        # thorlabs
        command_str = 'READ?'
        optical_power_level_W = float( tx_opm_handle.query( command_str ) )
        optical_power_level = 10*math.log10(1000*optical_power_level_W)

    return optical_power_level


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def measure_rx_optical_power_dbm():
    '''
    Meassures the optical rx power level in dBm.
    '''

    global rx_opm_handle
    global rx_opm_type
    global rx_opm_ch

    if (rx_opm_type == 'keysight'):
        # optical power meter, read from optical channel command
        command_str = 'READ%s:POWER?' % (rx_opm_ch)
        optical_power_level = rx_opm_handle.query( command_str )
        # Perform second read do to a bug in our optical device
        optical_power_level = rx_opm_handle.query( command_str )
    else:
        # thorlabs
        command_str = 'READ?'
        optical_power_level_W = float( rx_opm_handle.query( command_str ) )
        optical_power_level = 10*math.log10(1000*optical_power_level_W)



    return optical_power_level
    # for debugging menual opm optical_power=crt.Dialog.Prompt("Please enter optical power:", "optical power", "")


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def measure_extinction_ratio():
    '''
    Meassures Extinction Ratio [dB].
    '''

    global dca_handle

    # clear disaply
    command_str = 'ACQUIRE:CDISPLAY'
    dca_handle.write( command_str )
    time.sleep( 10 )
    # measure er
    command_str = 'MEASURE:EYE:ERATIO?'
    er_db = dca_handle.query( command_str )

    return er_db
    #extinction_ratio=crt.Dialog.Prompt("Please enter extinction ratio:", "extinction ratio", "")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def search_text(file_data, search):
    '''
    Gets the value of the text searched from the text file
    :rtype: object
    '''
    value = [line for line in file_data if search in line]
    value = value[0].replace( search, '' ).strip( '\n' )
    return value


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Configuration( object ):
    def __init__(self, data):
        self.opm_type = data[0]
        self.rx_opm_address = data[1]
        self.tx_opm_address = data[2]
        self.attenuator_address = data[3]
        self.dca_address = data[4]
        self.onu_to_tx_att_db = data[5]
        self.onu_to_rx_opm_att_db = data[6]
        self.olt_to_onu_att1_db = data[7]
        self.olt_to_onu_att2_db = data[8]

        self.apd_vbr_rssi = data[9]
        self.apd_step_down_voltage = data[10]
        self.apd_vol_step = data[11]
        self.apd_vbr_coeff_high_temp = data[12]
        self.apd_vbr_coeff_low_temp = data[13]
        self.apd_vbr_temp_boundary = data[14]
        self.max_mpd = data[15]
        self.min_mpd = data[16]
        self.max_biasctl = data[17]
        self.max_modctl = data[18]
        self.max_eyesafety = data[19]
        self.min_eyesafety = data[20]
        self.max_rogue = data[21]
        self.min_rogue = data[22]
        self.los_assert_th_op_dbm = data[23]
        self.los_deassert_th_op_dbm = data[24]
        self.avrg_power = data[25]
        self.er_target = data[26]
        self.run_er = data[27]
        self.min_optical_power = data[28]
        self.board = data[29]
        self.cal_apd = data[30]
        self.cal_dcd = data[31]
        self.los_type = data[32]
        self.tx_wavelength_nm = data[33]
        self.rx_wavelength_nm = data[34]
        self.prbs_mode = data[35]
        self.thermistor_type = data[36]
        self.cal_mode = data[37]
        self.tracking_compensation_enable0 = data[38]
        self.tracking_compensation_enable1 = data[39]
        self.tracking_compensation_coeff1_q8 = data[40]
        self.tracking_compensation_coeff2_q8 = data[41]

        er_linear = 10 ** ( float(self.er_target) / 10 )
        avrg_power_uW = 10 ** (float(self.avrg_power) / 10 ) * 1000
        self.L0_uW = 2 * avrg_power_uW / (1 + er_linear)
        self.L1_uW = er_linear * self.L0_uW
        self.apd_stepdown = int(round(float(self.apd_step_down_voltage)/float(self.apd_vol_step)))


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def load_configuration():
    '''
    Gets the opm and the attenuator addresses
    and the values of the attenuation
    from the conficuration.ini file
    '''
    # load directory path
    dirname = os.path.dirname( os.path.realpath( __file__ ) )
    name = dirname + "\\configuration.ini"

    # trying to open an excisting text file
    try:
        value_txt_file = open( name, 'r+' )  # Trying to open data file
        file_data = value_txt_file.readlines( )
        # getting opm type and address
        opm_type = search_text( file_data, "opm_type=" )
        rx_opm_address = search_text( file_data, "rx_opm_address=" )
        tx_opm_address = search_text( file_data, "tx_opm_address=" )
        # getting attenuator address
        attenuator_address = search_text( file_data, "attenuator_address=" )
        # getting DCA address
        dca_address = search_text( file_data, "dca_address=" )
        # getting attenuation values address
        onu_to_tx_att_db = search_text( file_data, "onu_to_tx_att_db=" )
        onu_to_rx_opm_att_db = search_text( file_data, "onu_to_rx_opm_att_db=" )
        olt_to_onu_att1_db = search_text( file_data, "olt_to_onu_att1_db=" )
        olt_to_onu_att2_db = search_text( file_data, "olt_to_onu_att2_db=" )

        # apd parameters
        apd_vbr_rssi = search_text( file_data, "apd_vbr_rssi=" )
        apd_step_down_voltage = search_text( file_data, "apd_step_down_voltage=" )
        apd_vol_step = search_text( file_data, "apd_vol_step=" )
        apd_vbr_coeff_high_temp = search_text( file_data, "apd_vbr_coeff_high_temp=" )
        apd_vbr_coeff_low_temp = search_text( file_data, "apd_vbr_coeff_low_temp=" )
        apd_vbr_temp_boundary = search_text( file_data, "apd_vbr_temp_boundary=" )

        # mpd parameters
        max_mpd = search_text( file_data, "max_mpd_level_value=" )
        min_mpd = search_text( file_data, "min_mpd_level_value=" )

        # bias and mod parameters
        max_biasctl = search_text( file_data, "max_biasctl=" )
        max_modctl = search_text( file_data, "max_modctl=" )

        # eyesafety and rogue parameters
        max_eyesafety = search_text( file_data, "max_eyesafety_thr=" )
        min_eyesafety = search_text( file_data, "min_eyesafety_thr=" )
        max_rogue = search_text( file_data, "max_rogue_dark_level=" )
        min_rogue = search_text( file_data, "min_rogue_dark_level=" )

        # los parameters
        los_assert_th_op_dbm= search_text( file_data, "los_assert_th_op_dbm=" )
        los_deassert_th_op_dbm= search_text( file_data, "los_deassert_th_op_dbm=" )

        # average power and er parameters
        avrg_power = search_text( file_data, "avrg_power=" )
        er_target = search_text( file_data, "er_target_value=" )
        prbs_mode = search_text( file_data, "cal_prbs_mode=" )
        run_er = search_text( file_data, "run_extinction_ratio=" )

        tracking_compensation_enable0 = search_text(file_data, "tracking_compensation_enable0=" )
        tracking_compensation_enable1 = search_text(file_data, "tracking_compensation_enable1=" )
        tracking_compensation_coeff1_q8 = search_text(file_data, "tracking_compensation_coeff1_q8=" )
        tracking_compensation_coeff2_q8 = search_text(file_data, "tracking_compensation_coeff2_q8=" )

        min_optical_power = search_text( file_data, "min_optical_power=" )
        board = search_text( file_data, "board_type=" )
        cal_apd = search_text( file_data, "cal_apd=" )
        cal_dcd = search_text( file_data, "cal_dcd=" )
        los_type = search_text( file_data, "los_type=" )

        cal_mode = search_text( file_data, "cal_mode=" )

        # wavelength
        tx_wavelength_nm = search_text( file_data, "tx_wavelength_nm=" )
        rx_wavelength_nm = search_text( file_data, "rx_wavelength_nm=" )
        thermistor_type = search_text( file_data, "thermistor_type=" )
    except:
        #crt.Dialog.MessageBox( "cannot find correct configuration file Quit execut" )
        print( "cannot find correct configuration file Quit execution " + name )
        #crt.Quit( )
        # creating a class of configuration data
    configuration_data = Configuration(
            [opm_type, rx_opm_address, tx_opm_address, attenuator_address, dca_address,
             onu_to_tx_att_db, onu_to_rx_opm_att_db, olt_to_onu_att1_db,
             olt_to_onu_att2_db,
             apd_vbr_rssi, apd_step_down_voltage, apd_vol_step, apd_vbr_coeff_high_temp, apd_vbr_coeff_low_temp , apd_vbr_temp_boundary,
             max_mpd, min_mpd,
             max_biasctl,
             max_modctl,
             max_eyesafety, min_eyesafety,
             max_rogue, min_rogue,
             los_assert_th_op_dbm, los_deassert_th_op_dbm,
             avrg_power,
             er_target, run_er,
             min_optical_power,
             board,
             cal_apd,
             cal_dcd,
             los_type,
			 tx_wavelength_nm, rx_wavelength_nm,
             prbs_mode, thermistor_type, cal_mode,
             tracking_compensation_enable0, tracking_compensation_enable1, tracking_compensation_coeff1_q8, tracking_compensation_coeff2_q8])

    return configuration_data

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
