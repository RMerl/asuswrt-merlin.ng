# SPDX-License-Identifier: GPL-2.0+

from distutils.core import setup
setup(name='patman',
      version='1.0',
      license='GPL-2.0+',
      scripts=['patman'],
      packages=['patman'],
      package_dir={'patman': ''},
      package_data={'patman': ['README']},
      classifiers=['Environment :: Console',
                   'Topic :: Software Development'])
