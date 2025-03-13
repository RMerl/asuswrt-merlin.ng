#!/usr/bin/env python

from distutils.core import setup, Extension

setup(	name="smbus",
	version="1.1",
	description="Python bindings for Linux SMBus access through i2c-dev",
	author="Mark M. Hoffman",
	author_email="mhoffman@lightlink.com",
	maintainer="Mark M. Hoffman",
	maintainer_email="linux-i2c@vger.kernel.org",
	license="GPLv2",
	url="https://i2c.wiki.kernel.org/index.php/I2C_Tools",
	ext_modules=[Extension(
		"smbus",
		["smbusmodule.c"],
		extra_compile_args=['-I../include'],
		extra_link_args=['-L../lib', '-li2c']
	)]
)
