#!/usr/bin/env python3

import os
import sys
import time
import psutil

from pathlib import Path


want_name = "dropbear"
# Walks up the parent process tree, prints a r-xp line of /proc/pid/maps when
# it finds the wanted name

def main():

	try:
		for p in psutil.Process().parents():
			print(p.pid, file=sys.stderr)
			print(p.name(), file=sys.stderr)
			print(p.cmdline(), file=sys.stderr)

			if want_name in p.name():
				with (Path('/proc') / str(p.pid) / "maps").open() as f:
					for i, l in enumerate(f, 1):
						if ' r-xp ' in l:
							print(l.rstrip())
							break
				return

		raise RuntimeError(f"Couldn't find parent {want_name} process")
	except Exception as e:
		print(psutil.Process().parents())
		for p in psutil.Process().parents():
			print(p.name())
		print(e)
		# time.sleep(100)
		raise

if __name__ == "__main__":
	main()
