import subprocess
import os
import pty
import tempfile
import logging
import time
import socketserver
import threading
import queue

import pytest

LOCALADDR="127.0.5.5"

@pytest.fixture(scope="module")
def dropbear(request):
	opt = request.config.option
	if opt.remote:
		yield None
		return

	# split so that "dropbearmulti dropbear" works
	args = opt.dropbear.split() + [
		"-p", LOCALADDR, # bind locally only
		"-r", opt.hostkey,
		"-p", opt.port,
		"-F", "-E",
		]
	p = subprocess.Popen(args, stderr=subprocess.PIPE, text=True)
	# Wait until it has started listening
	for l in p.stderr:
		if "Not backgrounding" in l:
			break
	# Check it's still running
		assert p.poll() is None
	# Ready
	yield p
	p.terminate()
	print("Terminated dropbear. Flushing output:")
	for l in p.stderr:
		print(l.rstrip())
	print("Done")

def dbclient(request, *args, **kwargs):
	opt = request.config.option
	host = opt.remote or LOCALADDR
	# split so that "dropbearmulti dbclient" works
	base_args = opt.dbclient.split() + ["-y", host, "-p", opt.port]
	if opt.user:
		base_args.extend(['-l', opt.user])
	full_args = base_args + list(args)
	bg = kwargs.get("background")
	if "background" in kwargs:
		del kwargs["background"]
	if bg:
		return subprocess.Popen(full_args, **kwargs)
	else:
		kwargs.setdefault("timeout", 10)
		# wait for response
		return subprocess.run(full_args, **kwargs)

def own_venv_command():
	""" Returns a command to run as a prefix to get the same venv
	as the current running Python. Returns '' on not a virtualenv
	"""
	try:
		venv = os.environ['VIRTUAL_ENV']
	except KeyError:
		return ""

	# note: bash/zsh unix specific
	return f"source {venv}/bin/activate"

class HandleTcp(socketserver.ThreadingMixIn, socketserver.TCPServer):

	# override TCPServer's default, avoids TIME_WAIT
	allow_reuse_addr = True

	""" Listens for a single incoming request, sends a response if given,
	and returns the inbound data.
	Reponse can be a queue object, in which case each item in the queue will
	be sent as a response, until it receives a None item.
	"""
	def __init__(self, port, timeout, response=None):
		super().__init__(('localhost', port), self.Handler)
		self.port = 	port
		self.timeout = timeout
		self.response = response
		self.sink = None

	class Handler(socketserver.StreamRequestHandler):
		def handle(self):
			if isinstance(self.server.response, queue.Queue):
				while True:
					i = self.server.response.get()
					if i is None:
						break
					self.wfile.write(i)
			elif self.server.response:
				self.wfile.write(self.server.response)
			assert self.server.sink is None, ">1 request sent to handler"
			self.server.sink = self.rfile.read()

	def __enter__(self):
		self.server_thread = threading.Thread(target=self.serve_forever)
		self.server_thread.daemon = True
		self.server_thread.start()
		return self

	def __exit__(self, *exc_stuff):
		self.shutdown()
		self.server_thread.join()

	def inbound(self):
		""" Returns the data sent to the socket """
		return self.sink

def readall_socket(sock):
	b = []
	while True:
		i = sock.recv(4096)
		if not i:
			break
		b.append(i)
	return b''.join(b)

# returns a str
def random_alnum(size):
	r = os.urandom(500 + size*5)
	return bytes(i for i in r if bytes((i,)).isalnum())[:size].decode()


