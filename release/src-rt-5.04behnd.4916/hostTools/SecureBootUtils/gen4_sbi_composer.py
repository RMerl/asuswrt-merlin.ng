#!/usr/local/bin/python3

#
# From ./build/prebuild_checks.mk
#   python3 version must be 3.7.0 or greater
#

import os
import random
import string
import subprocess
import argparse
import struct



SBI_VER_GEN4 = 0x04
SBI_AUTH_HDR_VER = 0x01

SBI_UNAUTH_MGC_NUM_1 = 0x0002CE92
SBI_UNAUTH_MGC_NUM_2 = 0x00023769

CERT_ROOM_VERSION = 0x01
MFG_MAT_ROOM_VERSION = 0x01
FLD_MAT_ROOM_VERSION = 0x01

'''
enum room_type {
   CERTIFICATE = 0xAAA1,
   MFG_MATERIALS = 0xAAA2,
   FLD_MATERIALS = 0xAAA3,
};
'''
CERTIFICATE = 0xAAA1
MFG_MATERIALS = 0xAAA2
FLD_MATERIALS = 0xAAA3

'''
enum sec_action {
	NO_ACTION = 0xAC01,
	MOVE_TO_MFG = 0xAC02,
	MOVE_TO_FLD = 0xAC03,
};
'''
NO_ACTION = 0xAC01
MOVE_TO_MFG = 0xAC02
MOVE_TO_FLD = 0xAC03

'''
enum sec_config {
	IS_SBI_ENCRYPTED = (1 << 0),
};
'''
IS_SBI_ENCRYPTED = 0x1

'''
enum SEC_STATE {
   MODE_UNSECURE = 0x1,
   MODE_MFG_SECURE = 0x2,
   MODE_FLD_SECURE = 0x4,
};
'''
MODE_UNSECURE = 0x1
MODE_MFG_SECURE = 0x2
MODE_FLD_SECURE = 0x4



def generate_random_string(length):
    letters = string.ascii_letters
    random_string = ''.join(random.choice(letters) for _ in range(length))
    return random_string



def save_to_file(content, filename):
    with open(filename, 'wb') as f:
        f.write(content)



def prepare_arguments():

    print("Gen4 SBI Composer: prepare arguments")

    parser = argparse.ArgumentParser(description='Process some parameters.')
    parser.add_argument('--key_map', type=str, help='Path to Key Map Configuration File')
    parser.add_argument('--eligibility', type=str, help='Eligibility options, one of the following: MODE_UNSECURE / MODE_MFG_SECURE / MODE_FLD_SECURE / MODE_MFG_FLD_SECURE')
    parser.add_argument('--action', type=str, help='Action options, one of the following: NO_ACTION / MOVE_TO_MFG / MOVE_TO_FLD')
    parser.add_argument('--certificate', type=str, help='Path to the certificate file, obtained from Broadcom')
    parser.add_argument('--mfg_rot', type=str, help='Key Name Hint to be used for MFG SBI Signing (priv) and Verification (pub)')
    parser.add_argument('--mfg_encryptor_args', type=str, help='Options used for encryption')
    parser.add_argument('--mfg_roe_ek', type=str, help='AES EK used to encrypt MFG SBI or Key Store Blob when composing Security Switch SBI')
    parser.add_argument('--mfg_roe_iv', type=str, help='AES IV used to encrypt MFG SBI or Key Store Blob when composing Security Switch SBI')
    parser.add_argument('--fld_rot', type=str, help='Key Name Hint to be used for FLD SBI Signing (priv) and Verification (pub)')
    parser.add_argument('--fld_encryptor_args', type=str, help='Options used for encryption')
    parser.add_argument('--fld_roe_ek', type=str, help='AES EK used to encrypt FLD SBI')
    parser.add_argument('--fld_roe_iv', type=str, help='AES EK used to encrypt FLD SBI')
    parser.add_argument('--blob', type=str, help='Path to the payload file, ARM BL or Key Store Blob when composing Security Switch SBI')
    parser.add_argument('--encryptor', type=str, help='Path to BASH script, invoked by this utility to encrypt content')
    parser.add_argument('--signer', type=str, help='Path to BASH script, invoked by this utility to sign content')
    parser.add_argument('--output', type=str, help='Result file, full path including the folders')
    parser.add_argument('--build_dir', type=str, help='Path to build directory, used by this utility to store intermediate data and computations')
    parser.add_argument('--crc_calculator', type=str, help='Path to CRC Calculator')

    print("Gen4 SBI Composer: arguments - ", parser.parse_args())

    return parser.parse_args()



def create_sbi_unauth_hdr(args, sbi_auth_hdr_sbi_payload):

    print("Gen4 SBI Composer: prepare sbi_unauth_hdr")

    sandbox_file = args.build_dir + "/" + generate_random_string(32)

    fmt = '<' + 'IIIIIIII'
    fmt_no_crc = '<' + 'IIIIIII'

    magic_1 = SBI_UNAUTH_MGC_NUM_1
    magic_2 = SBI_UNAUTH_MGC_NUM_2

    ver = SBI_VER_GEN4

    # Gen4 Unsecured SBI is Backward Compatible to Gen3 Boot Flow
    sbi_len_extra = (512 + 4)

    eligible = MODE_UNSECURE
    if ('MODE_UNSECURE' == args.eligibility) and (('MOVE_TO_MFG' == args.action) or ('MOVE_TO_FLD' == args.action)):
        sbi_len_extra += (512)
    elif 'MODE_MFG_SECURE' == args.eligibility:
        sbi_len_extra += (512)
        eligible = MODE_MFG_SECURE
    elif 'MODE_FLD_SECURE' == args.eligibility:
        sbi_len_extra += (512)
        eligible = MODE_FLD_SECURE
    elif 'MODE_MFG_FLD_SECURE' == args.eligibility:
        sbi_len_extra += (512)
        eligible = (MODE_MFG_SECURE | MODE_FLD_SECURE)

    hdr_len = struct.calcsize(fmt)

    sbi_len = hdr_len + len(sbi_auth_hdr_sbi_payload) + sbi_len_extra

    action = NO_ACTION
    if 'MOVE_TO_MFG' == args.action:
            action = MOVE_TO_MFG
    elif 'MOVE_TO_FLD' == args.action:
            action = MOVE_TO_FLD

    save_to_file(struct.pack(fmt_no_crc, magic_1, magic_2, ver, eligible, hdr_len, sbi_len, action), sandbox_file)
    crc = (subprocess.run([args.crc_calculator, sandbox_file], capture_output=True, text=True)).stdout

    sbi_unauth_hdr = struct.pack(fmt, magic_1, magic_2, ver, eligible, hdr_len, sbi_len, action, int(crc, 16))
 
    print("Gen4 SBI Composer: sbi_unauth_hdr - ready")

    if os.path.exists(sandbox_file):
        os.remove(sandbox_file)

    return sbi_unauth_hdr



def __create_certificate_room(args):

    print("Gen4 SBI Composer: prepare certificate_room")

    if (not os.path.exists(args.certificate)):
        print("Gen4 SBI Composer: certificate file was not found by specified path - ", args.certificate)
        exit(1)

    with open(args.certificate, 'rb') as f:
        data = f.read()

    fmt = '<' + 'HHHH' + str(len(data)) + 's'

    room_type = CERTIFICATE
    room_ver = CERT_ROOM_VERSION
    room_len = struct.calcsize(fmt)
    room_config = IS_SBI_ENCRYPTED

    room = struct.pack(fmt, room_type, room_ver, room_len, room_config, data)

    print("Gen4 SBI Composer: prepare certificate_room - ready")

    return room



def __create_mfg_materials_room(args):

    print("Gen4 SBI Composer: prepare mfg_materials_room")

    fmt = '<' + 'HHHHII'

    room_type = MFG_MATERIALS
    room_ver = MFG_MAT_ROOM_VERSION
    room_len = struct.calcsize(fmt)
    room_rot = int(''.join(hex(ord(char))[2:] for char in args.mfg_rot), 16)
    room_roe = 0
    if (len(args.mfg_roe_ek)):
        room_roe = int(''.join(hex(ord(char))[2:] for char in args.mfg_roe_ek), 16)

    if (room_roe):
         room_config = IS_SBI_ENCRYPTED
    else:
         room_config = 0x0000

    room = struct.pack(fmt, room_type, room_ver, room_len, room_config, room_rot, room_roe)

    print("Gen4 SBI Composer: prepare mfg_materials_room - ready")

    return room



def __create_fld_materials_room(args):

    print("Gen4 SBI Composer: prepare fld_materials_room")

    fmt = '<' + 'HHHHII'

    room_type = FLD_MATERIALS
    room_ver = FLD_MAT_ROOM_VERSION
    room_len = struct.calcsize(fmt)
    room_rot = int(''.join(hex(ord(char))[2:] for char in args.fld_rot), 16)
    room_roe = 0
    if (len(args.fld_roe_ek)):
        room_roe = int(''.join(hex(ord(char))[2:] for char in args.fld_roe_ek), 16)

    if (room_roe):
         room_config = IS_SBI_ENCRYPTED
    else:
         room_config = 0x0000

    room = struct.pack(fmt, room_type, room_ver, room_len, room_config, room_rot, room_roe)

    print("Gen4 SBI Composer: prepare fld_materials_room - ready")

    return room



def create_sbi_auth_hdr(args, sbi_payload):

    print("Gen4 SBI Composer: prepare sbi_auth_hdr")

    fmt = '<' + 'IIIIIIIIIII'

    ver = SBI_AUTH_HDR_VER
    # hdr_len
    # auth_len
    certificate_offs = 0
    mfg_materials_offs = 0
    fld_materials_offs = 0
    reserved_0_offs = 0
    reserved_1_offs = 0
    reserved_2_offs = 0
    reserved_3_offs = 0
    reserved_4_offs = 0

    rooms = struct.pack('')

    if ('MODE_UNSECURE' == args.eligibility) and (('MOVE_TO_MFG' == args.action) or ('MOVE_TO_FLD' == args.action)):
        certificate_offs = 0x2C
        rooms += __create_certificate_room(args)
    elif ('MODE_MFG_SECURE' == args.eligibility):
        mfg_materials_offs = 0x2C
        rooms += __create_mfg_materials_room(args)
    elif ('MODE_FLD_SECURE' == args.eligibility):
        fld_materials_offs = 0x2C
        rooms += __create_fld_materials_room(args)
    elif ('MODE_MFG_FLD_SECURE' == args.eligibility):
        mfg_materials_offs = 0x2C
        fld_materials_offs = 0x3C
        rooms += __create_mfg_materials_room(args)
        rooms += __create_fld_materials_room(args)

    hdr_len = struct.calcsize(fmt) + len(rooms)

    auth_len = 0
    if not (('MODE_UNSECURE' == args.eligibility) and ('NO_ACTION' == args.action)):
        auth_len = hdr_len + len(sbi_payload)

    if (len(rooms)):
        fmt += str(len(rooms)) + 's'
        sbi_auth_hdr = struct.pack(fmt, ver, hdr_len, auth_len, certificate_offs, mfg_materials_offs, fld_materials_offs, 
                                reserved_0_offs, reserved_1_offs, reserved_2_offs, reserved_3_offs, reserved_4_offs, 
                                rooms)
    else:
        sbi_auth_hdr = struct.pack(fmt, ver, hdr_len, auth_len, certificate_offs, mfg_materials_offs, fld_materials_offs, 
                                reserved_0_offs, reserved_1_offs, reserved_2_offs, reserved_3_offs, reserved_4_offs)

    print("Gen4 SBI Composer: sbi_auth_hdr - ready")

    return sbi_auth_hdr



def create_sbi_payload(args):

    print("Gen4 SBI Composer: prepare sbi_payload")

    if (not os.path.exists(args.blob)):
        print("Gen4 SBI Composer: payload file was not found by specified path - ", args.blob)
        exit(1)

    sandbox_file = args.build_dir + "/" + generate_random_string(32)

    if (len(args.mfg_roe_ek)):
        result = subprocess.run([args.encryptor, args.key_map, args.mfg_encryptor_args, args.mfg_roe_ek, args.mfg_roe_iv, args.blob, sandbox_file], capture_output=True, text=True)
        if (result.returncode):
            print("Gen4 SBI Composer: utility failure - ", result.stderr)
            exit(1)
        print(result.stdout)
        with open(sandbox_file, 'rb') as f:
            data = f.read()
    elif (len(args.fld_roe_ek)):
        result = subprocess.run([args.encryptor, args.key_map, args.fld_encryptor_args, args.fld_roe_ek, args.fld_roe_iv, args.blob, sandbox_file], capture_output=True, text=True)
        if (result.returncode):
            print("Gen4 SBI Composer: utility failure - ", result.stderr)
            exit(1)
        print(result.stdout)
        with open(sandbox_file, 'rb') as f:
            data = f.read()
    else:
        with open(args.blob, 'rb') as f:
            data = f.read()

    fmt = '<' + str(len(data)) + 's'

    sbi_payload = struct.pack(fmt, data)

    print("Gen4 SBI Composer: sbi_payload - ready")

    if os.path.exists(sandbox_file):
        os.remove(sandbox_file)

    return sbi_payload



def create_sbi_trailer(args, sbi_auth_hdr_sbi_payload):

    print("Gen4 SBI Composer: prepare sbi_trailer")

    sandbox_file = args.build_dir + "/" + generate_random_string(32)
    signature_file = args.build_dir + "/" + generate_random_string(32)

    save_to_file(sbi_auth_hdr_sbi_payload, sandbox_file)

    # Calculate CRC over Authenticated Header and the Payload
    crc = (subprocess.run([args.crc_calculator, sandbox_file], capture_output=True, text=True)).stdout

    # Prepare MFG Signature if needed, add/place padding if needed
    mfg_signature = bytes()
    if (args.mfg_rot):
        result = subprocess.run([args.signer, args.key_map, args.mfg_rot, sandbox_file, signature_file], capture_output=True, text=True)
        if (result.returncode):
            print("Gen4 SBI Composer: utility failure - ", result.stderr)
            exit(1)
        print(result.stdout)
        with open(signature_file, 'rb') as f:
            mfg_signature = f.read()

    padding_len = (512 - len(mfg_signature))
    if (padding_len):
        mfg_signature += bytearray(b'\x00' * padding_len)

    # Prepare FLD Signature if needed, add/place padding if needed
    fld_signature = bytes()
    if (args.fld_rot):
        result = subprocess.run([args.signer, args.key_map, args.fld_rot, sandbox_file, signature_file], capture_output=True, text=True)
        if (result.returncode):
            print("Gen4 SBI Composer: utility failure - ", result.stderr)
            exit(1)
        print(result.stdout)
        with open(signature_file, 'rb') as f:
            fld_signature = f.read()

    padding_len = (512 - len(fld_signature))
    if (padding_len):
        fld_signature += bytearray(b'\x00' * padding_len)

    # Compose Trailer according to requested Eligibility and Actions
    if ('MODE_UNSECURE' == args.eligibility) and ('NO_ACTION' == args.action):
        fmt = '<' + str(len(fld_signature)) + 's' + 'I'
        sbi_trailer = struct.pack(fmt, fld_signature, int(crc, 16))
    else:
        fmt = '<' + str(len(fld_signature)) + 's' + 'I' + str(len(mfg_signature)) + 's'
        sbi_trailer = struct.pack(fmt, fld_signature, int(crc, 16), mfg_signature)

    print("Gen4 SBI Composer: sbi_trailer - ready")

    if os.path.exists(sandbox_file):
        os.remove(sandbox_file)

    if os.path.exists(signature_file):
        os.remove(signature_file)

    return sbi_trailer



def compose_sbi(args):

    sbi_payload = create_sbi_payload(args)
    sbi_auth_hdr = create_sbi_auth_hdr(args, sbi_payload)
    sbi_unauth_hdr = create_sbi_unauth_hdr(args, sbi_auth_hdr + sbi_payload)
    sbi_trailer = create_sbi_trailer(args, sbi_auth_hdr + sbi_payload)
    sbi = sbi_unauth_hdr + sbi_auth_hdr + sbi_payload + sbi_trailer

    return sbi



def main():
    '''
    This utility creates Gen4 SBI Files
        - Unsecured Mode
        - Manufacture Secured Mode
        - Field Secured Mode
    '''

    print("Gen4 SBI Composer: started")

    # Parse command line arguments
    args = prepare_arguments()

    # Do some initial arguments check
    if (not args.eligibility) or (not args.action) or (not args.blob) or (not args.output) or (not args.build_dir) or (not args.crc_calculator):
        print("Gen4 SBI Composer: missing arguments, use '-h' for options")
        exit(1)

    if (not os.path.exists(args.crc_calculator)):
        print("Gen4 SBI Composer: crc calculator utility was not found by specified path - ", args.crc_calculator)
        exit(1)

    if os.path.exists(args.output):
        os.remove(args.output)

    # Compose the Gen4 SBI File
    sbi = compose_sbi(args)

    # Save the Gen4 SBI File
    save_to_file(sbi, args.output)

    if ((1024 < len(sbi)) and (os.path.exists(args.output))):
        print("Gen4 SBI Composer: completed")
    else:
        print("Gen4 SBI Composer: failed")
        exit(1)



if __name__ == '__main__':
    main()
