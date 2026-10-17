#!/usr/bin/env python3
## SPDX-License-Identifier: GPL-2.0-only
#
# EFI variable store utilities.
#
# (c) 2020 Paulo Alcantara <palcantara@suse.de>
#

import os
import struct
import uuid
import time
import zlib
import argparse
from OpenSSL import crypto

# U-Boot variable store format (version 1)
UBOOT_EFI_VAR_FILE_MAGIC = 0x0161566966456255

# UEFI variable attributes
EFI_VARIABLE_NON_VOLATILE = 0x1
EFI_VARIABLE_BOOTSERVICE_ACCESS = 0x2
EFI_VARIABLE_RUNTIME_ACCESS = 0x4
EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS = 0x10
EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS = 0x20
EFI_VARIABLE_READ_ONLY = 1 << 31
NV_BS = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS
NV_BS_RT = NV_BS | EFI_VARIABLE_RUNTIME_ACCESS
NV_BS_RT_AT = NV_BS_RT | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS
DEFAULT_VAR_ATTRS = NV_BS_RT

# vendor GUIDs
EFI_GLOBAL_VARIABLE_GUID = '8be4df61-93ca-11d2-aa0d-00e098032b8c'
EFI_IMAGE_SECURITY_DATABASE_GUID = 'd719b2cb-3d3a-4596-a3bc-dad00e67656f'
EFI_CERT_TYPE_PKCS7_GUID = '4aafd29d-68df-49ee-8aa9-347d375665a7'
WIN_CERT_TYPE_EFI_GUID = 0x0ef1
WIN_CERT_REVISION = 0x0200

var_attrs = {
        'NV': EFI_VARIABLE_NON_VOLATILE,
        'BS': EFI_VARIABLE_BOOTSERVICE_ACCESS,
        'RT': EFI_VARIABLE_RUNTIME_ACCESS,
        'AT': EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS,
        'RO': EFI_VARIABLE_READ_ONLY,
        'AW': EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS,
}

var_guids = {
        'EFI_GLOBAL_VARIABLE_GUID': EFI_GLOBAL_VARIABLE_GUID,
        'EFI_IMAGE_SECURITY_DATABASE_GUID': EFI_IMAGE_SECURITY_DATABASE_GUID,
}

class EfiStruct:
    # struct efi_var_file
    var_file_fmt = '<QQLL'
    var_file_size = struct.calcsize(var_file_fmt)
    # struct efi_var_entry
    var_entry_fmt = '<LLQ16s'
    var_entry_size = struct.calcsize(var_entry_fmt)
    # struct efi_time
    var_time_fmt = '<H6BLh2B'
    var_time_size = struct.calcsize(var_time_fmt)
    # WIN_CERTIFICATE
    var_win_cert_fmt = '<L2H'
    var_win_cert_size = struct.calcsize(var_win_cert_fmt)
    # WIN_CERTIFICATE_UEFI_GUID
    var_win_cert_uefi_guid_fmt = var_win_cert_fmt+'16s'
    var_win_cert_uefi_guid_size = struct.calcsize(var_win_cert_uefi_guid_fmt)

class EfiVariable:
    def __init__(self, size, attrs, time, guid, name, data):
        self.size = size
        self.attrs = attrs
        self.time = time
        self.guid = guid
        self.name = name
        self.data = data

def calc_crc32(buf):
    return zlib.crc32(buf) & 0xffffffff

class EfiVariableStore:
    def __init__(self, infile):
        self.infile = infile
        self.efi = EfiStruct()
        if os.path.exists(self.infile) and os.stat(self.infile).st_size > self.efi.var_file_size:
            with open(self.infile, 'rb') as f:
                buf = f.read()
                self._check_header(buf)
                self.ents = buf[self.efi.var_file_size:]
        else:
            self.ents = bytearray()

    def _check_header(self, buf):
        hdr = struct.unpack_from(self.efi.var_file_fmt, buf, 0)
        magic, crc32 = hdr[1], hdr[3]

        if magic != UBOOT_EFI_VAR_FILE_MAGIC:
            print("err: invalid magic number: %s"%hex(magic))
            exit(1)
        if crc32 != calc_crc32(buf[self.efi.var_file_size:]):
            print("err: invalid crc32: %s"%hex(crc32))
            exit(1)

    def _get_var_name(self, buf):
        name = ''
        for i in range(0, len(buf) - 1, 2):
            if not buf[i] and not buf[i+1]:
                break
            name += chr(buf[i])
        return ''.join([chr(x) for x in name.encode('utf_16_le') if x]), i + 2

    def _next_var(self, offs=0):
        size, attrs, time, guid = struct.unpack_from(self.efi.var_entry_fmt, self.ents, offs)
        data_fmt = str(size)+"s"
        offs += self.efi.var_entry_size
        name, namelen = self._get_var_name(self.ents[offs:])
        offs += namelen
        data = struct.unpack_from(data_fmt, self.ents, offs)[0]
        # offset to next 8-byte aligned variable entry
        offs = (offs + len(data) + 7) & ~7
        return EfiVariable(size, attrs, time, uuid.UUID(bytes_le=guid), name, data), offs

    def __iter__(self):
        self.offs = 0
        return self

    def __next__(self):
        if self.offs < len(self.ents):
            var, noffs = self._next_var(self.offs)
            self.offs = noffs
            return var
        else:
            raise StopIteration

    def __len__(self):
        return len(self.ents)

    def _set_var(self, guid, name_data, size, attrs, tsec):
        ent = struct.pack(self.efi.var_entry_fmt,
                          size,
                          attrs,
                          tsec,
                          uuid.UUID(guid).bytes_le)
        ent += name_data
        self.ents += ent

    def del_var(self, guid, name, attrs):
        offs = 0
        while offs < len(self.ents):
            var, loffs = self._next_var(offs)
            if var.name == name and str(var.guid) == guid:
                if var.attrs != attrs:
                    print("err: attributes don't match")
                    exit(1)
                self.ents = self.ents[:offs] + self.ents[loffs:]
                return
            offs = loffs
        print("err: variable not found")
        exit(1)

    def set_var(self, guid, name, data, size, attrs):
        offs = 0
        while offs < len(self.ents):
            var, loffs = self._next_var(offs)
            if var.name == name and str(var.guid) == guid:
                if var.attrs != attrs:
                    print("err: attributes don't match")
                    exit(1)
                # make room for updating var
                self.ents = self.ents[:offs] + self.ents[loffs:]
                break
            offs = loffs

        tsec = int(time.time()) if attrs & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS else 0
        nd = name.encode('utf_16_le') + b"\x00\x00" + data
        # U-Boot variable format requires the name + data blob to be 8-byte aligned
        pad = ((len(nd) + 7) & ~7) - len(nd)
        nd += bytes([0] * pad)

        return self._set_var(guid, nd, size, attrs, tsec)

    def save(self):
        hdr = struct.pack(self.efi.var_file_fmt,
                          0,
                          UBOOT_EFI_VAR_FILE_MAGIC,
                          len(self.ents) + self.efi.var_file_size,
                          calc_crc32(self.ents))

        with open(self.infile, 'wb') as f:
            f.write(hdr)
            f.write(self.ents)

def parse_attrs(attrs):
    v = DEFAULT_VAR_ATTRS
    if attrs:
        v = 0
        for i in attrs.split(','):
            v |= var_attrs[i.upper()]
    return v

def parse_data(val, vtype):
    if not val or not vtype:
        return None, 0
    fmt = { 'u8': '<B', 'u16': '<H', 'u32': '<L', 'u64': '<Q' }
    if vtype.lower() == 'file':
        with open(val, 'rb') as f:
            data = f.read()
            return data, len(data)
    if vtype.lower() == 'str':
        data = val.encode('utf-8')
        return data, len(data)
    if vtype.lower() == 'nil':
        return None, 0
    i = fmt[vtype.lower()]
    return struct.pack(i, int(val)), struct.calcsize(i)

def parse_args(args):
    name = args.name
    attrs = parse_attrs(args.attrs)
    guid = args.guid if args.guid else EFI_GLOBAL_VARIABLE_GUID

    if name.lower() == 'db' or name.lower() == 'dbx':
        name = name.lower()
        guid = EFI_IMAGE_SECURITY_DATABASE_GUID
        attrs = NV_BS_RT_AT
    elif name.lower() == 'pk' or name.lower() == 'kek':
        name = name.upper()
        guid = EFI_GLOBAL_VARIABLE_GUID
        attrs = NV_BS_RT_AT

    data, size = parse_data(args.data, args.type)
    return guid, name, attrs, data, size

def cmd_set(args):
    env = EfiVariableStore(args.infile)
    guid, name, attrs, data, size = parse_args(args)
    env.set_var(guid=guid, name=name, data=data, size=size, attrs=attrs)
    env.save()

def print_var(var):
    print(var.name+':')
    print("    "+str(var.guid)+' '+''.join([x for x in var_guids if str(var.guid) == var_guids[x]]))
    print("    "+'|'.join([x for x in var_attrs if var.attrs & var_attrs[x]])+", DataSize = %s"%hex(var.size))
    hexdump(var.data)

def cmd_print(args):
    env = EfiVariableStore(args.infile)
    if not args.name and not args.guid and not len(env):
        return

    found = False
    for var in env:
        if not args.name:
            if args.guid and args.guid != str(var.guid):
                continue
            print_var(var)
            found = True
        else:
            if args.name != var.name or (args.guid and args.guid != str(var.guid)):
                continue
            print_var(var)
            found = True

    if not found:
        print("err: variable not found")
        exit(1)

def cmd_del(args):
    env = EfiVariableStore(args.infile)
    attrs = parse_attrs(args.attrs)
    guid = args.guid if args.guid else EFI_GLOBAL_VARIABLE_GUID
    env.del_var(guid, args.name, attrs)
    env.save()

def pkcs7_sign(cert, key, buf):
    with open(cert, 'r') as f:
        crt = crypto.load_certificate(crypto.FILETYPE_PEM, f.read())
    with open(key, 'r') as f:
        pkey = crypto.load_privatekey(crypto.FILETYPE_PEM, f.read())

    PKCS7_BINARY = 0x80
    PKCS7_DETACHED = 0x40
    PKCS7_NOATTR = 0x100

    bio_in = crypto._new_mem_buf(buf)
    p7 = crypto._lib.PKCS7_sign(crt._x509, pkey._pkey, crypto._ffi.NULL, bio_in,
                                PKCS7_BINARY|PKCS7_DETACHED|PKCS7_NOATTR)
    bio_out = crypto._new_mem_buf()
    crypto._lib.i2d_PKCS7_bio(bio_out, p7)
    return crypto._bio_to_string(bio_out)

# UEFI 2.8 Errata B "8.2.2 Using the EFI_VARIABLE_AUTHENTICATION_2 descriptor"
def cmd_sign(args):
    guid, name, attrs, data, _ = parse_args(args)
    attrs |= EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS
    efi = EfiStruct()

    tm = time.localtime()
    etime = struct.pack(efi.var_time_fmt,
                        tm.tm_year, tm.tm_mon, tm.tm_mday,
                        tm.tm_hour, tm.tm_min, tm.tm_sec,
                        0, 0, 0, 0, 0)

    buf = name.encode('utf_16_le') + uuid.UUID(guid).bytes_le + attrs.to_bytes(4, byteorder='little') + etime
    if data:
        buf += data
    sig = pkcs7_sign(args.cert, args.key, buf)

    desc = struct.pack(efi.var_win_cert_uefi_guid_fmt,
                       efi.var_win_cert_uefi_guid_size + len(sig),
                       WIN_CERT_REVISION,
                       WIN_CERT_TYPE_EFI_GUID,
                       uuid.UUID(EFI_CERT_TYPE_PKCS7_GUID).bytes_le)

    with open(args.outfile, 'wb') as f:
        if data:
            f.write(etime + desc + sig + data)
        else:
            f.write(etime + desc + sig)

def main():
    ap = argparse.ArgumentParser(description='EFI variable store utilities')
    subp = ap.add_subparsers(help="sub-command help")

    printp = subp.add_parser('print', help='get/list EFI variables')
    printp.add_argument('--infile', '-i', required=True, help='file to save the EFI variables')
    printp.add_argument('--name', '-n', help='variable name')
    printp.add_argument('--guid', '-g', help='vendor GUID')
    printp.set_defaults(func=cmd_print)

    setp = subp.add_parser('set', help='set EFI variable')
    setp.add_argument('--infile', '-i', required=True, help='file to save the EFI variables')
    setp.add_argument('--name', '-n', required=True, help='variable name')
    setp.add_argument('--attrs', '-a', help='variable attributes (values: nv,bs,rt,at,ro,aw)')
    setp.add_argument('--guid', '-g', help="vendor GUID (default: %s)"%EFI_GLOBAL_VARIABLE_GUID)
    setp.add_argument('--type', '-t', help='variable type (values: file|u8|u16|u32|u64|str)')
    setp.add_argument('--data', '-d', help='data or filename')
    setp.set_defaults(func=cmd_set)

    delp = subp.add_parser('del', help='delete EFI variable')
    delp.add_argument('--infile', '-i', required=True, help='file to save the EFI variables')
    delp.add_argument('--name', '-n', required=True, help='variable name')
    delp.add_argument('--attrs', '-a', help='variable attributes (values: nv,bs,rt,at,ro,aw)')
    delp.add_argument('--guid', '-g', help="vendor GUID (default: %s)"%EFI_GLOBAL_VARIABLE_GUID)
    delp.set_defaults(func=cmd_del)

    signp = subp.add_parser('sign', help='sign time-based EFI payload')
    signp.add_argument('--cert', '-c', required=True, help='x509 certificate filename in PEM format')
    signp.add_argument('--key', '-k', required=True, help='signing certificate filename in PEM format')
    signp.add_argument('--name', '-n', required=True, help='variable name')
    signp.add_argument('--attrs', '-a', help='variable attributes (values: nv,bs,rt,at,ro,aw)')
    signp.add_argument('--guid', '-g', help="vendor GUID (default: %s)"%EFI_GLOBAL_VARIABLE_GUID)
    signp.add_argument('--type', '-t', required=True, help='variable type (values: file|u8|u16|u32|u64|str|nil)')
    signp.add_argument('--data', '-d', help='data or filename')
    signp.add_argument('--outfile', '-o', required=True, help='output filename of signed EFI payload')
    signp.set_defaults(func=cmd_sign)

    args = ap.parse_args()
    if hasattr(args, "func"):
        args.func(args)
    else:
        ap.print_help()

def group(a, *ns):
    for n in ns:
        a = [a[i:i+n] for i in range(0, len(a), n)]
    return a

def join(a, *cs):
    return [cs[0].join(join(t, *cs[1:])) for t in a] if cs else a

def hexdump(data):
    toHex = lambda c: '{:02X}'.format(c)
    toChr = lambda c: chr(c) if 32 <= c < 127 else '.'
    make = lambda f, *cs: join(group(list(map(f, data)), 8, 2), *cs)
    hs = make(toHex, '  ', ' ')
    cs = make(toChr, ' ', '')
    for i, (h, c) in enumerate(zip(hs, cs)):
        print ('    {:010X}: {:48}  {:16}'.format(i * 16, h, c))

if __name__ == '__main__':
    main()
