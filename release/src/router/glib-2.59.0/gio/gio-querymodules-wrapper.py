#!/usr/bin/env python3

import os
import subprocess
import sys

if not os.environ.get('DESTDIR'):
  print('GIO module cache creation...')
  subprocess.call([sys.argv[1], sys.argv[2]])
