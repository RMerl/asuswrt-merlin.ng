

"""
    demo_dynamic.py                                    v2b

    This program demonstrates Python's use of the dynamic
    language support additions to LTC, namely access to LTC
    constants, struct and union sizes, and the binding of a
    math package to LTC.  Also provided are simple code
    fragments to illustrate how one might write a Python
    wrapper for LTC and how an app might call the wrapper.
    This or a similar model should work for Ruby and other
    dynamic languages.

    This instance uses Python's ctypes and requires a single
    .dylib linking together LTC and a math library.  Building
    a single .dylib is needed because LTC wants a fairly tight
    relationship between itself and the mathlib.  (ctypes can
    load multiple .dylibs, but it does not support this level
    of tight coupling between otherwise independent libraries.)

    My .dylib was created on OSX/macOS with the following:
        sudo make -j5 -f makefile.shared                        \
            CFLAGS="-DUSE_TFM -DTFM_DESC -I/usr/local/include"  \
            EXTRALIBS=/usr/local/lib/libtfm.a  install

    For python 2.7.12 on Ubuntu Xenial the following worked for
    me (without MPI support):
        sudo make -f makefile.shared install PREFIX="/usr"

    Reminder: you don't need to bind in a math library unless
              you are going to use LTC functions that need a
              mathlib.  For example, public key crypto requires
              a mathlib; hashing and symmetric encryption do not.

    ------

    This code was originally written for Python 2.7 with the
    ctypes standard library.  This version is modified to run
    under both Python 2.7 and 3.6.

    Arguably the biggest change for Python3 has to do with
    strings.  Under Python2, native strings are ASCII bytes and
    passing them to LTC is natural and requires no conversion.
    Under Python3 all native strings are Unicode which requires
    they be converted to bytes before use by LTC.

    Note the following for Python3.
        - ASCII keys, IVs and other string arguments must be
          'bytes'.  Define them with a 'b' prefix or convert
          via the 'bytes()' function.
        - "strings" returned from LTC are bytes and conversion
          to Unicode might be necessary for proper printing.
          If so, use <string>.decode('utf-8').
        - The Python2 'print' statement becomes a function in
          Python3 which requires parenthesis, eg. 'print()'.

    NB: Unicode is achieved under Python2 by either defining
        a Unicode string with a 'u' prefix or passing ASCII
        strings thru the 'unicode()' function.

    Larry Bugbee
    March 2014      v1
    August 2017     v2b

"""


import sys
from ctypes import *
from ctypes.util import find_library

# switches to enable/disable selected output
SHOW_ALL_CONSTANTS      = True
SHOW_ALL_SIZES          = True
SHOW_SELECTED_CONSTANTS = True
SHOW_SELECTED_SIZES     = True
SHOW_BUILD_OPTIONS_ALGS = True
SHOW_SHA256_EXAMPLE     = True
SHOW_CHACHA_EXAMPLE     = True

print(' ')
print('  demo_dynamic.py')

def inprint(s, indent=0):
    "prints strings indented, including multline strings"
    for line in s.split('\n'):
        print(' '*indent + line)

#-------------------------------------------------------------------------------
# load the .dylib

libname = 'tomcrypt'
libpath = find_library(libname)
print(' ')
print('  path to library %s: %s' % (libname, libpath))

LTC = cdll.LoadLibrary(libpath)
print('  loaded: %s' % LTC)
print(' ')


#-------------------------------------------------------------------------------
# get list of all supported constants followed by a list of all
# supported sizes.  One alternative: these lists may be parsed
# and used as needed.

if SHOW_ALL_CONSTANTS:
    print('-'*60)
    print('  all supported constants and their values:')

    # get size to allocate for constants output list
    str_len = c_int(0)
    ret = LTC.crypt_list_all_constants(None, byref(str_len))
    print('    need to allocate %d bytes to build list \n' % str_len.value)

    # allocate that size and get (name, size) pairs, each pair
    # separated by a newline char.
    names_sizes = c_buffer(str_len.value)
    ret = LTC.crypt_list_all_constants(names_sizes, byref(str_len))
    print(names_sizes.value.decode("utf-8"))
    print(' ')


if SHOW_ALL_SIZES:
    print('-'*60)
    print('  all supported sizes:')

    # get size to allocate for sizes output list
    str_len = c_int(0)
    ret = LTC.crypt_list_all_sizes(None, byref(str_len))
    print('    need to allocate %d bytes to build list \n' % str_len.value)

    # allocate that size and get (name, size) pairs, each pair
    # separated by a newline char.
    names_sizes = c_buffer(str_len.value)
    ret = LTC.crypt_list_all_sizes(names_sizes, byref(str_len))
    print(names_sizes.value.decode("utf-8"))
    print(' ')


#-------------------------------------------------------------------------------
# get individually named constants and sizes

if SHOW_SELECTED_CONSTANTS:
    print('-'*60)
    print('\n  selected constants:')

    names = [
        b'ENDIAN_LITTLE',
        b'ENDIAN_64BITWORD',
        b'PK_PUBLIC',
        b'LTC_MILLER_RABIN_REPS',
        b'CTR_COUNTER_BIG_ENDIAN',
    ]
    for name in names:
        const_value = c_int(0)
        rc = LTC.crypt_get_constant(name, byref(const_value))
        value = const_value.value
        print('    %-25s  %d' % (name.decode("utf-8"), value))
    print(' ')

if SHOW_SELECTED_SIZES:
    print('-'*60)
    print('\n  selected sizes:')

    names = [
        b'rijndael_key',
        b'rsa_key',
        b'symmetric_CTR',
        b'twofish_key',
        b'ecc_point',
        b'gcm_state',
        b'sha512_state',
    ]
    for name in names:
        size_value = c_int(0)
        rc = LTC.crypt_get_size(name, byref(size_value))
        value = size_value.value
        print('    %-25s  %d' % (name.decode("utf-8"), value))
    print(' ')


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
# LibTomCrypt exposes one interesting string that can be accessed
# via Python's ctypes module, "crypt_build_settings", which
# provides a list of this build's compiler switches and supported
# algorithms.  If someday LTC exposes other interesting strings,
# they can be found with:
#   nm /usr/local/lib/libtomcrypt.dylib | grep " D "

def get_named_string(lib, name):
    return c_char_p.in_dll(lib, name).value.decode("utf-8")

if SHOW_BUILD_OPTIONS_ALGS:
    print('-'*60)
    print('This is a string compiled into LTC showing compile')
    print('options and algorithms supported by this build \n')
#    print(get_named_string(LTC, 'crypt_build_settings'))
    inprint(get_named_string(LTC, 'crypt_build_settings'), 4)


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
# here is an example of how Python code can be written to access
# LTC's implementation of SHA256 and ChaCha,

# - - - - - - - - - - - - -
# definitions

from binascii import hexlify, unhexlify

def _err2str(err):
    # define return type
    errstr = LTC.error_to_string
    errstr.restype = c_char_p
    # get and return err string
    return errstr(err)

def _get_size(name):
    size = c_int(0)
    rc = LTC.crypt_get_size(bytes(name), byref(size))
    if rc != 0:
        raise Exception('LTC.crypt_get_size(%s) rc = %d' % (name, rc))
    return size.value

def _get_constant(name):
    constant = c_int(0)
    rc = LTC.crypt_get_constant(bytes(name), byref(constant))
    if rc != 0:
        raise Exception('LTC.crypt_get_constant(%s) rc = %d' % (name, rc))
    return constant.value

CRYPT_OK = _get_constant(b'CRYPT_OK')

class SHA256(object):
    def __init__(self):
        self.state = c_buffer(_get_size(b'sha256_state'))
        LTC.sha256_init(byref(self.state))
    def update(self, data):
        LTC.sha256_process(byref(self.state), data, len(data))
    def digest(self):
        md = c_buffer(32)
        LTC.sha256_done(byref(self.state), byref(md))
        return md.raw

class ChaCha(object):
    def __init__(self, key, rounds):
        self.state   = c_buffer(_get_size(b'chacha_state'))
        self.counter = c_int(1)
        err = LTC.chacha_setup(byref(self.state), key, len(key), rounds)
        if err != CRYPT_OK:
            raise Exception('LTC.chacha_setup(), err = %d, "%s"' % (err, _err2str(err)))
    def set_iv32(self, iv):
        err = LTC.chacha_ivctr32(byref(self.state), iv, len(iv), byref(self.counter))
        if err != CRYPT_OK:
            raise Exception('LTC.chacha_ivctr32(), err = %d, "%s"' % (err, _err2str(err)))
    def crypt(self, datain):
        dataout = c_buffer(len(datain))
        err = LTC.chacha_crypt(byref(self.state), datain, len(datain), byref(dataout))
        if err != CRYPT_OK:
            raise Exception('LTC.chacha_crypt(), err = %d, "%s"' % (err, _err2str(err)))
        return dataout.raw

# - - - - - - - - - - - - -
# a SHA256 app fragment

if SHOW_SHA256_EXAMPLE:
    print('-'*60)
    data = b'hello world'               # we want bytes, not Unicode

    sha256 = SHA256()
    sha256.update(data)
    md = sha256.digest()

    template = '\n  the SHA256 digest for "%s" is %s \n'
    print(template % (data, hexlify(md)))

# - - - - - - - - - - - - -
# a ChaCha app fragment

if SHOW_CHACHA_EXAMPLE:
    print('-'*60)
    key     = b'hownowbrowncow\x00\x00' # exactly 16 or 32 bytes
    rounds  = 12                        # common values: 8, 12, 20
    iv      = b'123456789012'           # exactly 12 bytes
    plain   = b'Kilroy was here, there, and everywhere!'

    cha = ChaCha(key, rounds)
    cha.set_iv32(iv)
    cipher = cha.crypt(plain)

    template = '\n  ChaCha%d ciphertext   for "%s" is "%s"'
    print(template % (rounds, plain, hexlify(cipher)))

    cha.set_iv32(iv)                    # reset to decrypt
    decrypted = cha.crypt(cipher)

    template = '  ChaCha%d decoded text for "%s" is "%s" \n'
    print(template % (rounds, plain, decrypted.decode("utf-8")))

# Footnote: Keys should be erased fm memory as soon as possible after use,
# and that includes Python.  For a tip on how to do that in Python, see
# http://buggywhip.blogspot.com/2010/12/erase-keys-and-credit-card-numbers-in.html

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
