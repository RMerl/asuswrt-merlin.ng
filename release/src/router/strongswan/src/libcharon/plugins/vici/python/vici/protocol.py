import io
import socket
import struct

from collections import namedtuple
from collections import OrderedDict

from .compat import iteritems
from .exception import DeserializationException


class Transport(object):
    HEADER_LENGTH = 4
    MAX_SEGMENT = 512 * 1024

    def __init__(self, sock):
        self.socket = sock

    def send(self, packet):
        self.socket.sendall(struct.pack("!I", len(packet)) + packet)

    def receive(self):
        raw_length = self._recvall(self.HEADER_LENGTH)
        length, = struct.unpack("!I", raw_length)
        payload = self._recvall(length)
        return payload

    def close(self):
        self.socket.shutdown(socket.SHUT_RDWR)
        self.socket.close()

    def _recvall(self, count):
        """Ensure to read count bytes from the socket"""
        data = b""
        while len(data) < count:
            buf = self.socket.recv(count - len(data))
            if not buf:
                raise socket.error('Connection closed')
            data += buf
        return data


class Packet(object):
    CMD_REQUEST = 0         # Named request message
    CMD_RESPONSE = 1        # Unnamed response message for a request
    CMD_UNKNOWN = 2         # Unnamed response if requested command is unknown
    EVENT_REGISTER = 3      # Named event registration request
    EVENT_UNREGISTER = 4    # Named event de-registration request
    EVENT_CONFIRM = 5       # Unnamed confirmation for event (de-)registration
    EVENT_UNKNOWN = 6       # Unnamed response if event (de-)registration failed
    EVENT = 7               # Named event message

    ParsedPacket = namedtuple(
        "ParsedPacket",
        ["response_type", "payload"]
    )

    ParsedEventPacket = namedtuple(
        "ParsedEventPacket",
        ["response_type", "event_type", "payload"]
    )

    @classmethod
    def _named_request(cls, request_type, request, message=None):
        request = request.encode("UTF-8")
        payload = struct.pack("!BB", request_type, len(request)) + request
        if message is not None:
            return payload + message
        else:
            return payload

    @classmethod
    def request(cls, command, message=None):
        return cls._named_request(cls.CMD_REQUEST, command, message)

    @classmethod
    def register_event(cls, event_type):
        return cls._named_request(cls.EVENT_REGISTER, event_type)

    @classmethod
    def unregister_event(cls, event_type):
        return cls._named_request(cls.EVENT_UNREGISTER, event_type)

    @classmethod
    def parse(cls, packet):
        stream = FiniteStream(packet)
        response_type, = struct.unpack("!B", stream.read(1))

        if response_type == cls.EVENT:
            length, = struct.unpack("!B", stream.read(1))
            event_type = stream.read(length)
            return cls.ParsedEventPacket(response_type, event_type, stream)
        else:
            return cls.ParsedPacket(response_type, stream)


class Message(object):
    SECTION_START = 1       # Begin a new section having a name
    SECTION_END = 2         # End a previously started section
    KEY_VALUE = 3           # Define a value for a named key in the section
    LIST_START = 4          # Begin a named list for list items
    LIST_ITEM = 5           # Define an unnamed item value in the current list
    LIST_END = 6            # End a previously started list

    @classmethod
    def serialize(cls, message):
        def encode_named_type(marker, name):
            name = name.encode("UTF-8")
            return struct.pack("!BB", marker, len(name)) + name

        def encode_blob(value):
            if not isinstance(value, bytes):
                value = str(value).encode("UTF-8")
            return struct.pack("!H", len(value)) + value

        def serialize_list(lst):
            segment = bytes()
            for item in lst:
                segment += struct.pack("!B", cls.LIST_ITEM) + encode_blob(item)
            return segment

        def serialize_dict(d):
            segment = bytes()
            for key, value in iteritems(d):
                if isinstance(value, dict):
                    segment += (
                        encode_named_type(cls.SECTION_START, key)
                        + serialize_dict(value)
                        + struct.pack("!B", cls.SECTION_END)
                    )
                elif isinstance(value, list):
                    segment += (
                        encode_named_type(cls.LIST_START, key)
                        + serialize_list(value)
                        + struct.pack("!B", cls.LIST_END)
                    )
                else:
                    segment += (
                        encode_named_type(cls.KEY_VALUE, key)
                        + encode_blob(value)
                    )
            return segment

        return serialize_dict(message)

    @classmethod
    def deserialize(cls, stream):
        def decode_named_type(stream):
            length, = struct.unpack("!B", stream.read(1))
            return stream.read(length).decode("UTF-8")

        def decode_blob(stream):
            length, = struct.unpack("!H", stream.read(2))
            return stream.read(length)

        def decode_list_item(stream):
            marker, = struct.unpack("!B", stream.read(1))
            while marker == cls.LIST_ITEM:
                yield decode_blob(stream)
                marker, = struct.unpack("!B", stream.read(1))

            if marker != cls.LIST_END:
                raise DeserializationException(
                    "Expected end of list at {pos}".format(pos=stream.tell())
                )

        section = OrderedDict()
        section_stack = []
        while stream.has_more():
            element_type, = struct.unpack("!B", stream.read(1))
            if element_type == cls.SECTION_START:
                section_name = decode_named_type(stream)
                new_section = OrderedDict()
                section[section_name] = new_section
                section_stack.append(section)
                section = new_section

            elif element_type == cls.LIST_START:
                list_name = decode_named_type(stream)
                section[list_name] = [item for item in decode_list_item(stream)]

            elif element_type == cls.KEY_VALUE:
                key = decode_named_type(stream)
                section[key] = decode_blob(stream)

            elif element_type == cls.SECTION_END:
                if len(section_stack):
                    section = section_stack.pop()
                else:
                    raise DeserializationException(
                        "Unexpected end of section at {pos}".format(
                            pos=stream.tell()
                        )
                    )

        if len(section_stack):
            raise DeserializationException("Expected end of section")
        return section


class FiniteStream(io.BytesIO):
    def __len__(self):
        return len(self.getvalue())

    def has_more(self):
        return self.tell() < len(self)
