"""
Tests opening and closing several (up to 4) channels concurrently.
"""
from test_dropbear import *

import asyncssh
import asyncio
import random

async def run(addr, port):
    async with asyncssh.connect(addr, port = port) as conn:

        chans = []
        MAX=4

        for x in range(10000):
            if len(chans) < MAX:
                pipes = await conn.open_session(command = "df")
                chans.append(pipes)
                l = len(chans)
                print(f" add, len {l}")

            if random.random() < 0.2:
                i = random.randrange(0, len(chans))
                l = len(chans)
                print(f" del {i}/{l}")
                del chans[i]

def test_concurrent(request, dropbear):
    opt = request.config.option
    host = opt.remote or LOCALADDR
    port = int(opt.port)

    asyncio.run(run(host, port))
