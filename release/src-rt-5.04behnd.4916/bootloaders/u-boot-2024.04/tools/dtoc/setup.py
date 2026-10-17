# SPDX-License-Identifier: GPL-2.0+

from distutils.core import setup
setup(name='dtoc',
      version='1.0',
      license='GPL-2.0+',
      scripts=['dtoc'],
      packages=['dtoc'],
      package_dir={'dtoc': ''},
      package_data={'dtoc': ['README']},
      classifiers=['Environment :: Console',
                   'Topic :: Software Development :: Embedded Systems'])
