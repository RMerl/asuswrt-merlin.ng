from pathlib import Path
import sys

from test_dropbear import *

def test_reexec(request, dropbear):
	"""
	Tests that two consecutive connections have different address layouts.
	This indicates that re-exec makes ASLR work
	"""
	map_script = (Path(request.node.fspath).parent / "parent_dropbear_map.py").resolve()
	# run within the same venv, for python deps
	activate = own_venv_command()
	cmd = f"{activate}; {map_script}"
	print(cmd)
	r = dbclient(request, cmd, capture_output=True, text=True)
	map1 = r.stdout.rstrip()
	print(r.stderr, file=sys.stderr)
	r.check_returncode()

	r = dbclient(request, cmd, capture_output=True, text=True)
	map2 = r.stdout.rstrip()
	print(r.stderr, file=sys.stderr)
	r.check_returncode()

	print(map1)
	print(map2)
	# expect something like
	# "563174d59000-563174d5d000 r--p 00000000 00:29 4242372                    /home/matt/src/dropbear/build/dropbear"
	assert map1.endswith('/dropbear') or map1.endswith('/dropbearmulti')
	a1 = map1.split()[0]
	a2 = map2.split()[0]
	print(a1)
	print(a2)
	# relocation addresses should differ
	assert a1 != a2

