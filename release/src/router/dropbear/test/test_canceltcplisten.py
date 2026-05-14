"""
Tests cancel-tcpip-forward
"""
from test_dropbear import *

import asyncssh
import asyncio
import socket
import errno
import random

async def run(addr, port):
    async with asyncssh.connect(addr, port = port) as conn:

        fwds = []
        COUNT=4

        def connect(host, port):
            print(f"connection from {host}:{port}")

        for x in range(COUNT):
            server = await conn.start_server(connect, '', 0,
                                     encoding='utf-8')
            print(f"opened fwd {server} port {server.get_port()}")
            fwds.append(server)

        # Check that they can be connected
        for f in fwds:
            c = socket.create_connection(("localhost", f.get_port()))

        # In case list ordering is important
        random.shuffle(fwds)

        for f in fwds:
            print(f"close fwd {f}")
            f.close()
            await f.wait_closed()

            # Check it's closed
            try:
                c = socket.create_connection(("localhost", f.get_port()))
                assert False, f"Expected {f} port {f.get_port()} to be closed"
            except OSError as e:
                assert e.errno == errno.ECONNREFUSED

def test_canceltcplisten(request, dropbear):
    opt = request.config.option
    host = opt.remote or LOCALADDR
    port = int(opt.port)

    asyncio.run(run(host, port))
