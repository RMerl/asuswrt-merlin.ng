from test_dropbear import *
import signal
import queue
import socket
import os
from pathlib import Path

# Tests for server side authentication

# Requires keyfile and authorized_keys set up in github action build.yml
@pytest.mark.skipif('DBTEST_IN_ACTION' not in os.environ, reason="DBTEST_IN_ACTION not set")
def test_pubkeyinfo(request, dropbear):
	kf = str(Path.home() / ".ssh/id_dropbear_key2")
	r = dbclient(request, "-i", kf, "echo -n $SSH_PUBKEYINFO", capture_output=True)
	# stop at first space
	assert r.stdout.decode() == "key2"

@pytest.mark.skipif('DBTEST_IN_ACTION' not in os.environ, reason="DBTEST_IN_ACTION not set")
def test_pubkeyinfo_special(request, dropbear):
	kf = str(Path.home() / ".ssh/id_dropbear_key3")
	r = dbclient(request, "-i", kf, "echo -n $SSH_PUBKEYINFO", capture_output=True)
	# comment contains special characters so the SSH_PUBKEYINFO should not be set
	assert r.stdout.decode() == ""

@pytest.mark.skipif('DBTEST_IN_ACTION' not in os.environ, reason="DBTEST_IN_ACTION not set")
def test_pubkeyinfo_okchar(request, dropbear):
	kf = str(Path.home() / ".ssh/id_dropbear_key4")
	r = dbclient(request, "-i", kf, "echo -n $SSH_PUBKEYINFO", capture_output=True)
	# comment contains special characters so the SSH_PUBKEYINFO should not be set
	assert r.stdout.decode() == "key4,char"
