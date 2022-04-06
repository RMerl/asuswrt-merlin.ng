from test_dropbear import *
import signal
import queue
import socket

# Tests for various edge cases of SSH channels and connection service

def test_exitcode(request, dropbear):
	r = dbclient(request, "exit 44")
	assert r.returncode == 44

@pytest.mark.xfail(reason="Not yet implemented", strict=True)
def test_signal(request, dropbear):
	r = dbclient(request, "kill -FPE $$")
	assert r.returncode == -signal.SIGFPE

@pytest.mark.parametrize("size", [0, 1, 2, 100, 5000, 200_000])
def test_roundtrip(request, dropbear, size):
	dat = os.urandom(size)
	r = dbclient(request, "cat", input=dat, capture_output=True)
	r.check_returncode()
	assert r.stdout == dat

@pytest.mark.parametrize("size", [0, 1, 2, 100, 20001, 41234])
def test_read_pty(request, dropbear, size):
	# testcase for
	# https://bugs.openwrt.org/index.php?do=details&task_id=1814
	# https://github.com/mkj/dropbear/pull/85
	# From Yousong Zhou
	# Fixed Oct 2021
	#
	#$ ssh -t my.router cat /tmp/bigfile | wc
	#Connection to my.router closed.
	#  0       1   14335 <- should be 20001

	# Write the file. No newlines etc which could confuse ptys
	dat = random_alnum(size)
	r = dbclient(request, "tmpf=`mktemp`; echo $tmpf; cat > $tmpf", input=dat, capture_output=True, text=True)
	tmpf = r.stdout.rstrip()
	r.check_returncode()
	# Read with a pty, this is what is being tested.
	# Timing/buffering is subtle, we seem to need to cat a file from disk to hit it.
	m, s = pty.openpty()
	r = dbclient(request, "-t", f"cat {tmpf}; rm {tmpf}", stdin=s, capture_output=True)
	r.check_returncode()
	assert r.stdout.decode() == dat

@pytest.mark.parametrize("fd", [1, 2])
def test_bg_sleep(request, fd, dropbear):
	# https://lists.ucc.asn.au/pipermail/dropbear/2006q1/000362.html
	# Rob Landley "Is this a bug?" 24 Mar 2006
	# dbclient user@system "sleep 10& echo hello"
	#
	# It should return right after printing hello, but it doesn't.  It waits until
	# the child process exits.

	# failure is TimeoutExpired
	redir = "" if fd == 1 else " >&2 "
	r = dbclient(request, f"sleep 10& echo hello {redir}",
		capture_output=True, timeout=2, text=True)
	r.check_returncode()
	st = r.stdout if fd == 1 else r.stderr

	if fd == 2 and 'accepted unconditionally' in st:
		# ignore hostkey warning, a bit of a hack
		assert st.endswith("\n\nhello\n")
	else:
		assert st.rstrip() == "hello"


def test_idle(request, dropbear):
	# Idle test, -I 1 should make it return before the 2 second timeout
	r = dbclient(request, "-I", "1", "echo zong; sleep 10",
		capture_output=True, timeout=2, text=True)
	r.check_returncode()
	assert r.stdout.rstrip() == "zong"

@pytest.mark.parametrize("size", [1, 4000, 40000])
def test_netcat(request, dropbear, size):
	opt = request.config.option
	if opt.remote:
		pytest.xfail("don't know netcat address for remote")

	dat1 = os.urandom(size)
	dat2 = os.urandom(size)
	with HandleTcp(3344, 1, dat2) as tcp:
		r = dbclient(request, "-B", "localhost:3344", input=dat1, capture_output=True)
		r.check_returncode()
		assert r.stdout == dat2
		assert tcp.inbound() == dat1

@pytest.mark.parametrize("size", [1, 4000, 40000])
@pytest.mark.parametrize("fwd_flag", "LR")
def test_tcpflushout(request, dropbear, size, fwd_flag):
	""" Tests that an opened TCP connection prevent a SSH session from being closed
	until that TCP connection has finished transferring
	"""
	opt = request.config.option
	if opt.remote:
		pytest.xfail("don't know address for remote")

	dat1 = os.urandom(size)
	dat2 = os.urandom(size)
	q = queue.Queue()
	with HandleTcp(3344, timeout=1, response=q) as tcp:

		r = dbclient(request, f"-{fwd_flag}", "7788:localhost:3344", "sleep 0.1; echo -n done",
			text=True, background=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
		# time to let the listener start
		time.sleep(0.1)
		# open a tcp connection
		c = socket.create_connection(("localhost", 7788))

		# wait for the shell to finish. sleep a bit longer in case it exits.
		assert r.stdout.read(4) == "done"
		time.sleep(0.1)

		# now the shell has finished, we can write on the tcp socket
		c.sendall(dat2)
		c.shutdown(socket.SHUT_WR)
		q.put(dat1)

		# return a tcp response
		q.put(None)
		# check hasn't exited
		assert r.poll() == None

		# read the response
		assert readall_socket(c) == dat1
		c.close()
		assert tcp.inbound() == dat2
		# check has exited, allow time for dbclient to exit
		time.sleep(0.1)
		assert r.poll() == 0
