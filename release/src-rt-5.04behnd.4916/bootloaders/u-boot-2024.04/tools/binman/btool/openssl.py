# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for openssl

openssl provides a number of features useful for signing images

Documentation is at https://www.coreboot.org/CBFS

Source code is at https://www.openssl.org/
"""

import hashlib

from binman import bintool
from u_boot_pylib import tools


VALID_SHAS = [256, 384, 512, 224]
SHA_OIDS = {256:'2.16.840.1.101.3.4.2.1',
            384:'2.16.840.1.101.3.4.2.2',
            512:'2.16.840.1.101.3.4.2.3',
            224:'2.16.840.1.101.3.4.2.4'}

class Bintoolopenssl(bintool.Bintool):
    """openssl tool

    This bintool supports creating new openssl certificates.

    It also supports fetching a binary openssl

    Documentation about openssl is at https://www.openssl.org/
    """
    def __init__(self, name):
        super().__init__(
            name, 'openssl cryptography toolkit',
            version_regex=r'OpenSSL (.*) \(', version_args='version')

    def x509_cert(self, cert_fname, input_fname, key_fname, cn, revision,
                  config_fname):
        """Create a certificate

        Args:
            cert_fname (str): Filename of certificate to create
            input_fname (str): Filename containing data to sign
            key_fname (str): Filename of .pem file
            cn (str): Common name
            revision (int): Revision number
            config_fname (str): Filename to write fconfig into

        Returns:
            str: Tool output
        """
        indata = tools.read_file(input_fname)
        hashval = hashlib.sha512(indata).hexdigest()
        with open(config_fname, 'w', encoding='utf-8') as outf:
            print(f'''[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
CN                     = {cert_fname}

[ v3_ca ]
basicConstraints       = CA:true
1.3.6.1.4.1.294.1.3    = ASN1:SEQUENCE:swrv
1.3.6.1.4.1.294.1.34   = ASN1:SEQUENCE:sysfw_image_integrity

[ swrv ]
swrv = INTEGER:{revision}

[ sysfw_image_integrity ]
shaType                = OID:2.16.840.1.101.3.4.2.3
shaValue               = FORMAT:HEX,OCT:{hashval}
imageSize              = INTEGER:{len(indata)}
''', file=outf)
        args = ['req', '-new', '-x509', '-key', key_fname, '-nodes',
                '-outform', 'DER', '-out', cert_fname, '-config', config_fname,
                '-sha512']
        return self.run_cmd(*args)

    def x509_cert_sysfw(self, cert_fname, input_fname, key_fname, sw_rev,
                  config_fname, req_dist_name_dict, firewall_cert_data):
        """Create a certificate to be booted by system firmware

        Args:
            cert_fname (str): Filename of certificate to create
            input_fname (str): Filename containing data to sign
            key_fname (str): Filename of .pem file
            sw_rev (int): Software revision
            config_fname (str): Filename to write fconfig into
            req_dist_name_dict (dict): Dictionary containing key-value pairs of
            req_distinguished_name section extensions, must contain extensions for
            C, ST, L, O, OU, CN and emailAddress
            firewall_cert_data (dict):
              - auth_in_place (int): The Priv ID for copying as the
                specific host in firewall protected region
              - num_firewalls (int): The number of firewalls in the
                extended certificate
              - certificate (str): Extended firewall certificate with
                the information for the firewall configurations.

        Returns:
            str: Tool output
        """
        indata = tools.read_file(input_fname)
        hashval = hashlib.sha512(indata).hexdigest()
        with open(config_fname, 'w', encoding='utf-8') as outf:
            print(f'''[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
C                      = {req_dist_name_dict['C']}
ST                     = {req_dist_name_dict['ST']}
L                      = {req_dist_name_dict['L']}
O                      = {req_dist_name_dict['O']}
OU                     = {req_dist_name_dict['OU']}
CN                     = {req_dist_name_dict['CN']}
emailAddress           = {req_dist_name_dict['emailAddress']}

[ v3_ca ]
basicConstraints       = CA:true
1.3.6.1.4.1.294.1.3    = ASN1:SEQUENCE:swrv
1.3.6.1.4.1.294.1.34   = ASN1:SEQUENCE:sysfw_image_integrity
1.3.6.1.4.1.294.1.35   = ASN1:SEQUENCE:sysfw_image_load
1.3.6.1.4.1.294.1.37   = ASN1:SEQUENCE:firewall

[ swrv ]
swrv = INTEGER:{sw_rev}

[ sysfw_image_integrity ]
shaType                = OID:2.16.840.1.101.3.4.2.3
shaValue               = FORMAT:HEX,OCT:{hashval}
imageSize              = INTEGER:{len(indata)}

[ sysfw_image_load ]
destAddr = FORMAT:HEX,OCT:00000000
authInPlace = INTEGER:{hex(firewall_cert_data['auth_in_place'])}

[ firewall ]
numFirewallRegions = INTEGER:{firewall_cert_data['num_firewalls']}
{firewall_cert_data['certificate']}
''', file=outf)
        args = ['req', '-new', '-x509', '-key', key_fname, '-nodes',
                '-outform', 'DER', '-out', cert_fname, '-config', config_fname,
                '-sha512']
        return self.run_cmd(*args)

    def x509_cert_rom(self, cert_fname, input_fname, key_fname, sw_rev,
                  config_fname, req_dist_name_dict, cert_type, bootcore,
                  bootcore_opts, load_addr, sha):
        """Create a certificate

        Args:
            cert_fname (str): Filename of certificate to create
            input_fname (str): Filename containing data to sign
            key_fname (str): Filename of .pem file
            sw_rev (int): Software revision
            config_fname (str): Filename to write fconfig into
            req_dist_name_dict (dict): Dictionary containing key-value pairs of
            req_distinguished_name section extensions, must contain extensions for
            C, ST, L, O, OU, CN and emailAddress
            cert_type (int): Certification type
            bootcore (int): Booting core
            bootcore_opts(int): Booting core option, lockstep (0) or split (2) mode
            load_addr (int): Load address of image
            sha (int): Hash function

        Returns:
            str: Tool output
        """
        indata = tools.read_file(input_fname)
        hashval = hashlib.sha512(indata).hexdigest()
        with open(config_fname, 'w', encoding='utf-8') as outf:
            print(f'''
[ req ]
 distinguished_name     = req_distinguished_name
 x509_extensions        = v3_ca
 prompt                 = no
 dirstring_type         = nobmp

 [ req_distinguished_name ]
C                      = {req_dist_name_dict['C']}
ST                     = {req_dist_name_dict['ST']}
L                      = {req_dist_name_dict['L']}
O                      = {req_dist_name_dict['O']}
OU                     = {req_dist_name_dict['OU']}
CN                     = {req_dist_name_dict['CN']}
emailAddress           = {req_dist_name_dict['emailAddress']}

 [ v3_ca ]
 basicConstraints = CA:true
 1.3.6.1.4.1.294.1.1 = ASN1:SEQUENCE:boot_seq
 1.3.6.1.4.1.294.1.2 = ASN1:SEQUENCE:image_integrity
 1.3.6.1.4.1.294.1.3 = ASN1:SEQUENCE:swrv
# 1.3.6.1.4.1.294.1.4 = ASN1:SEQUENCE:encryption
 1.3.6.1.4.1.294.1.8 = ASN1:SEQUENCE:debug

 [ boot_seq ]
 certType = INTEGER:{cert_type}
 bootCore = INTEGER:{bootcore}
 bootCoreOpts = INTEGER:{bootcore_opts}
 destAddr = FORMAT:HEX,OCT:{load_addr:08x}
 imageSize = INTEGER:{len(indata)}

 [ image_integrity ]
 shaType = OID:{SHA_OIDS[sha]}
 shaValue = FORMAT:HEX,OCT:{hashval}

 [ swrv ]
 swrv = INTEGER:{sw_rev}

# [ encryption ]
# initalVector = FORMAT:HEX,OCT:TEST_IMAGE_ENC_IV
# randomString = FORMAT:HEX,OCT:TEST_IMAGE_ENC_RS
# iterationCnt = INTEGER:TEST_IMAGE_KEY_DERIVE_INDEX
# salt = FORMAT:HEX,OCT:TEST_IMAGE_KEY_DERIVE_SALT

 [ debug ]
 debugUID = FORMAT:HEX,OCT:0000000000000000000000000000000000000000000000000000000000000000
 debugType = INTEGER:4
 coreDbgEn = INTEGER:0
 coreDbgSecEn = INTEGER:0
''', file=outf)
        args = ['req', '-new', '-x509', '-key', key_fname, '-nodes',
                '-outform', 'DER', '-out', cert_fname, '-config', config_fname,
                '-sha512']
        return self.run_cmd(*args)

    def x509_cert_rom_combined(self, cert_fname, input_fname, key_fname, sw_rev,
                  config_fname, req_dist_name_dict, load_addr, sha, total_size, num_comps,
                  sysfw_inner_cert_ext_boot_sequence_string, dm_data_ext_boot_sequence_string,
                  imagesize_sbl, hashval_sbl, load_addr_sysfw, imagesize_sysfw,
                  hashval_sysfw, load_addr_sysfw_data, imagesize_sysfw_data,
                  hashval_sysfw_data, sysfw_inner_cert_ext_boot_block,
                  dm_data_ext_boot_block, bootcore_opts):
        """Create a certificate

        Args:
            cert_fname (str): Filename of certificate to create
            input_fname (str): Filename containing data to sign
            key_fname (str): Filename of .pem file
            sw_rev (int): Software revision
            config_fname (str): Filename to write fconfig into
            req_dist_name_dict (dict): Dictionary containing key-value pairs of
            req_distinguished_name section extensions, must contain extensions for
            C, ST, L, O, OU, CN and emailAddress
            cert_type (int): Certification type
            bootcore (int): Booting core
            load_addr (int): Load address of image
            sha (int): Hash function
            bootcore_opts (int): Booting core option, lockstep (0) or split (2) mode

        Returns:
            str: Tool output
        """
        indata = tools.read_file(input_fname)
        hashval = hashlib.sha512(indata).hexdigest()
        sha_type = SHA_OIDS[sha]
        with open(config_fname, 'w', encoding='utf-8') as outf:
            print(f'''
[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
C                      = {req_dist_name_dict['C']}
ST                     = {req_dist_name_dict['ST']}
L                      = {req_dist_name_dict['L']}
O                      = {req_dist_name_dict['O']}
OU                     = {req_dist_name_dict['OU']}
CN                     = {req_dist_name_dict['CN']}
emailAddress           = {req_dist_name_dict['emailAddress']}

[ v3_ca ]
basicConstraints = CA:true
1.3.6.1.4.1.294.1.3=ASN1:SEQUENCE:swrv
1.3.6.1.4.1.294.1.9=ASN1:SEQUENCE:ext_boot_info

[swrv]
swrv=INTEGER:{sw_rev}

[ext_boot_info]
extImgSize=INTEGER:{total_size}
numComp=INTEGER:{num_comps}
sbl=SEQUENCE:sbl
sysfw=SEQUENCE:sysfw
sysfw_data=SEQUENCE:sysfw_data
{sysfw_inner_cert_ext_boot_sequence_string}
{dm_data_ext_boot_sequence_string}

[sbl]
compType = INTEGER:1
bootCore = INTEGER:16
compOpts = INTEGER:{bootcore_opts}
destAddr = FORMAT:HEX,OCT:{load_addr:08x}
compSize = INTEGER:{imagesize_sbl}
shaType  = OID:{sha_type}
shaValue = FORMAT:HEX,OCT:{hashval_sbl}

[sysfw]
compType = INTEGER:2
bootCore = INTEGER:0
compOpts = INTEGER:0
destAddr = FORMAT:HEX,OCT:{load_addr_sysfw:08x}
compSize = INTEGER:{imagesize_sysfw}
shaType  = OID:{sha_type}
shaValue = FORMAT:HEX,OCT:{hashval_sysfw}

[sysfw_data]
compType = INTEGER:18
bootCore = INTEGER:0
compOpts = INTEGER:0
destAddr = FORMAT:HEX,OCT:{load_addr_sysfw_data:08x}
compSize = INTEGER:{imagesize_sysfw_data}
shaType  = OID:{sha_type}
shaValue = FORMAT:HEX,OCT:{hashval_sysfw_data}

{sysfw_inner_cert_ext_boot_block}

{dm_data_ext_boot_block}
        ''', file=outf)
        args = ['req', '-new', '-x509', '-key', key_fname, '-nodes',
                '-outform', 'DER', '-out', cert_fname, '-config', config_fname,
                '-sha512']
        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for openssl

        This installs the openssl package using the apt utility.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched and now installed, None if a method
            other than FETCH_BIN was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        return self.apt_install('openssl')
