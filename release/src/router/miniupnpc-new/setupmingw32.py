#! /usr/bin/env python
# vim: tabstop=8 shiftwidth=8 expandtab
# $Id: setupmingw32.py,v 1.13 2020/10/15 22:27:08 nanard Exp $
# the MiniUPnP Project (c) 2007-2020 Thomas Bernard
# https://miniupnp.tuxfamily.org/ or http://miniupnp.free.fr/
#
# python script to build the miniupnpc module under windows (using mingw32)
#
import sys

if (sys.version_info.major * 10 +  sys.version_info.minor) >= 35:
        compat_lib = ["legacy_stdio_definitions"]
else:
        compat_lib = []

try:
        from setuptools import setup, Extension
except ImportError:
        from distutils.core import setup, Extension
from distutils import sysconfig
sysconfig.get_config_vars()["OPT"] = ''
sysconfig.get_config_vars()["CFLAGS"] = ''
setup(name="miniupnpc",
      version=open('VERSION').read().strip(),
      author='Thomas BERNARD',
      author_email='miniupnp@free.fr',
      license=open('LICENSE').read(),
      url='http://miniupnp.free.fr/',
      description='miniUPnP client',
      ext_modules=[
         Extension(name="miniupnpc", sources=["miniupnpcmodule.c"],
                   libraries=["ws2_32", "iphlpapi"] + compat_lib,
                   extra_objects=["miniupnpc.lib"])
      ])

