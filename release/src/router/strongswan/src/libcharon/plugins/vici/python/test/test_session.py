import pytest
import socket
import struct
from collections import OrderedDict

from vici.session import Session
from vici.protocol import Transport, Packet, Message, FiniteStream
from vici.exception import DeserializationException


class MockedServer(object):

    def __init__(self, sock):
        self.transport = Transport(sock)

    def send(self, kind, name=None, message=None):
        if name is None:
            payload = struct.pack("!B", kind)
        else:
            name = name.encode("UTF-8")
            payload = struct.pack("!BB", kind, len(name)) + name
        if message is not None:
            payload += Message.serialize(message)
        self.transport.send(payload)

    def recv(self):
        stream = FiniteStream(self.transport.receive())
        kind, length = struct.unpack("!BB", stream.read(2))
        name = stream.read(length)
        data = stream.read()
        if len(data):
            return kind, name, Message.deserialize(data)
        return kind, name


class TestSession(object):

    events = [
        OrderedDict([('event', b'1')]),
        OrderedDict([('event', b'2')]),
        OrderedDict([('event', b'3')]),
    ]

    def interconnect(self):
        c, s = socket.socketpair(socket.AF_UNIX)
        return Session(c), MockedServer(s)

    def test_request(self):
        c, s = self.interconnect()

        s.send(Packet.CMD_RESPONSE)
        assert c.request("doit") == {}
        assert s.recv() == (Packet.CMD_REQUEST, b"doit")

        s.send(Packet.CMD_RESPONSE, message={"hey": b"hou"})
        assert c.request("heyhou") == {"hey": b"hou"}
        assert s.recv() == (Packet.CMD_REQUEST, b"heyhou")

    def test_streamed(self):
        c, s = self.interconnect()

        s.send(Packet.EVENT_CONFIRM)
        for e in self.events:
            s.send(Packet.EVENT, name="stream", message=e)
        s.send(Packet.CMD_RESPONSE)
        s.send(Packet.EVENT_CONFIRM)

        assert list(c.streamed_request("streamit", "stream")) == self.events
        assert s.recv() == (Packet.EVENT_REGISTER, b"stream")
        assert s.recv() == (Packet.CMD_REQUEST, b"streamit")
        assert s.recv() == (Packet.EVENT_UNREGISTER, b"stream")

    def test_timeout(self):
        c, s = self.interconnect()

        s.send(Packet.EVENT_CONFIRM)
        s.send(Packet.EVENT_CONFIRM)
        for e in self.events:
            s.send(Packet.EVENT, name="event", message=e)

        r = []
        i = 0
        for name, msg in c.listen(["xyz", "event"], timeout=0.1):
            if name is None:
                i += 1
                if i > 2:
                    s.send(Packet.EVENT, name="event", message={"late": b'1'})
                    s.send(Packet.EVENT_CONFIRM)
                    s.send(Packet.EVENT_CONFIRM)
                    break
            else:
                assert name == b"event"
                r.append(msg)

        assert s.recv() == (Packet.EVENT_REGISTER, b"xyz")
        assert s.recv() == (Packet.EVENT_REGISTER, b"event")
        assert s.recv() == (Packet.EVENT_UNREGISTER, b"xyz")
        assert s.recv() == (Packet.EVENT_UNREGISTER, b"event")

        assert r == self.events
