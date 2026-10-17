#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
#
# Public key to dtsi converter.
#
# Copyright (c) Siemens AG, 2022
#

from argparse import ArgumentParser, FileType
from os.path import basename, splitext
from Cryptodome.PublicKey import RSA
from Cryptodome.Util.number import inverse

def int_to_bytestr(n, length=None):
    if not length:
        length = (n.bit_length() + 7) // 8
    byte_array = n.to_bytes(length, 'big')
    return ' '.join(['{:02x}'.format(byte) for byte in byte_array])

ap = ArgumentParser(description='Public key to dtsi converter')

ap.add_argument('--hash', '-H', default='sha256',
                help='hash to be used with key (default: sha256)')
ap.add_argument('--required-conf', '-c', action='store_true',
                help='mark key required for configuration')
ap.add_argument('--required-image', '-i', action='store_true',
                help='mark key required for image')
ap.add_argument('--spl', '-s', action='store_true',
                help='mark key for usage in SPL')
ap.add_argument('key_file', metavar='KEY_FILE', type=FileType('r'),
                help='key file (formats: X.509, PKCS#1, OpenSSH)')
ap.add_argument('dtsi_file', metavar='DTSI_FILE', type=FileType('w'),
                help='dtsi output file')

args = ap.parse_args()

key_name, _ = splitext(basename(args.key_file.name))

key_data = args.key_file.read()
key = RSA.importKey(key_data)

r_squared = (2**key.size_in_bits())**2 % key.n
n0_inverse = 2**32 - inverse(key.n, 2**32)

out = args.dtsi_file
out.write('/ {\n')
out.write('\tsignature {\n')
out.write('\t\tkey-{} {{\n'.format(key_name))
out.write('\t\t\tkey-name-hint = "{}";\n'.format(key_name))
out.write('\t\t\talgo = "{},rsa{}";\n'.format(args.hash, key.size_in_bits()))
out.write('\t\t\trsa,num-bits = <{}>;\n'.format(key.size_in_bits()))
out.write('\t\t\trsa,modulus = [{}];\n'.format(int_to_bytestr(key.n)))
out.write('\t\t\trsa,exponent = [{}];\n'.format(int_to_bytestr(key.e, 8)))
out.write('\t\t\trsa,r-squared = [{}];\n'.format(int_to_bytestr(r_squared)))
out.write('\t\t\trsa,n0-inverse = <0x{:x}>;\n'.format(n0_inverse))
if args.required_conf:
    out.write('\t\t\trequired = "conf";\n')
elif args.required_image:
    out.write('\t\t\trequired = "image";\n')
if args.spl:
    out.write('\t\t\tu-boot,dm-spl;\n')
out.write('\t\t};\n')
out.write('\t};\n')
out.write('};\n')
