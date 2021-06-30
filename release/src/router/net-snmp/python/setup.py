from distutils.core import setup, Extension
from setuptools import setup, Extension, find_packages
import os
import re
import sys

intree=0

args = sys.argv[:]
for arg in args:
    if arg.find('--basedir=') == 0:
        basedir = arg.split('=')[1]
        sys.argv.remove(arg)
        intree=1

if intree:
    netsnmp_libs = os.popen(basedir+'/net-snmp-config --libs').read()
    libdir = os.popen(basedir+'/net-snmp-config --build-lib-dirs '+basedir).read()
    incdir = os.popen(basedir+'/net-snmp-config --build-includes '+basedir).read() + " " + os.popen(basedir+'/net-snmp-config --base-cflags '+basedir).read()
    libs = re.findall(r"(?:^|\s+)-l(\S+)", netsnmp_libs)
    libdirs = re.findall(r"(?:^|\s+)-L(\S+)", libdir)
    incdirs = re.findall(r"(?:^|\s+)-I(\S+)", incdir)
else:
    netsnmp_libs = os.popen('net-snmp-config --libs').read()
    libdirs = re.findall(r"(?:^|\s+)-L(\S+)", netsnmp_libs)
    incdirs = []
    libs = re.findall(r"(?:^|\s+)-l(\S+)", netsnmp_libs)

setup(
    name="netsnmp-python", version="1.0a1",
    description = 'The Net-SNMP Python Interface',
    author = 'G. S. Marzot',
    author_email = 'giovanni.marzot@sparta.com',
    url = 'http://www.net-snmp.org',
    license="BSD",
    packages=find_packages(),
    test_suite = "netsnmp.tests.test",

    ext_modules = [
       Extension("netsnmp.client_intf", ["netsnmp/client_intf.c"],
                 library_dirs=libdirs,
                 include_dirs=incdirs,
                 libraries=libs )
       ]
    )
