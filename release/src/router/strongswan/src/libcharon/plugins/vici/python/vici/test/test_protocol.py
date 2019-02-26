import pytest

from ..protocol import Packet, Message, FiniteStream
from ..exception import DeserializationException


class TestPacket(object):
    # test data definitions for outgoing packet types
    cmd_request = b"\x00\x0c" b"command_type"
    cmd_request_msg = b"\x00\x07" b"command" b"payload"
    event_register = b"\x03\x0a" b"event_type"
    event_unregister = b"\x04\x0a" b"event_type"

    # test data definitions for incoming packet types
    cmd_response = b"\x01" b"reply"
    cmd_unknown = b"\x02"
    event_confirm = b"\x05"
    event_unknown = b"\x06"
    event = b"\x07\x03" b"log" b"message"

    def test_request(self):
        assert Packet.request("command_type") == self.cmd_request
        assert Packet.request("command", b"payload") == self.cmd_request_msg

    def test_register_event(self):
        assert Packet.register_event("event_type") == self.event_register

    def test_unregister_event(self):
        assert Packet.unregister_event("event_type") == self.event_unregister

    def test_parse(self):
        parsed_cmd_response = Packet.parse(self.cmd_response)
        assert parsed_cmd_response.response_type == Packet.CMD_RESPONSE
        assert parsed_cmd_response.payload.getvalue() == self.cmd_response

        parsed_cmd_unknown = Packet.parse(self.cmd_unknown)
        assert parsed_cmd_unknown.response_type == Packet.CMD_UNKNOWN
        assert parsed_cmd_unknown.payload.getvalue() == self.cmd_unknown

        parsed_event_confirm = Packet.parse(self.event_confirm)
        assert parsed_event_confirm.response_type == Packet.EVENT_CONFIRM
        assert parsed_event_confirm.payload.getvalue() == self.event_confirm

        parsed_event_unknown = Packet.parse(self.event_unknown)
        assert parsed_event_unknown.response_type == Packet.EVENT_UNKNOWN
        assert parsed_event_unknown.payload.getvalue() == self.event_unknown

        parsed_event = Packet.parse(self.event)
        assert parsed_event.response_type == Packet.EVENT
        assert parsed_event.payload.getvalue() == self.event


class TestMessage(object):
    """Message (de)serialization test."""

    # data definitions for test of de(serialization)
    # serialized messages holding a section
    ser_sec_unclosed = b"\x01\x08unclosed"
    ser_sec_single = b"\x01\x07section\x02"
    ser_sec_nested = b"\x01\x05outer\x01\x0asubsection\x02\x02"

    # serialized messages holding a list
    ser_list_invalid = b"\x04\x07invalid\x05\x00\x02e1\x02\x03sec\x06"
    ser_list_0_item = b"\x04\x05empty\x06"
    ser_list_1_item = b"\x04\x01l\x05\x00\x02e1\x06"
    ser_list_2_item = b"\x04\x01l\x05\x00\x02e1\x05\x00\x02e2\x06"

    # serialized messages with key value pairs
    ser_kv_pair = b"\x03\x03key\x00\x05value"
    ser_kv_zero = b"\x03\x0azerolength\x00\x00"

    # deserialized messages holding a section
    des_sec_single = { "section": {} }
    des_sec_nested = { "outer": { "subsection": {} } }

    # deserialized messages holding a list
    des_list_0_item = { "empty": [] }
    des_list_1_item = { "l": [ b"e1" ] }
    des_list_2_item = { "l": [ b"e1", b"e2" ] }

    # deserialized messages with key value pairs
    des_kv_pair = { "key": b"value" }
    des_kv_zero = { "zerolength": b"" }

    def test_section_serialization(self):
        assert Message.serialize(self.des_sec_single) == self.ser_sec_single
        assert Message.serialize(self.des_sec_nested) == self.ser_sec_nested

    def test_list_serialization(self):
        assert Message.serialize(self.des_list_0_item) == self.ser_list_0_item
        assert Message.serialize(self.des_list_1_item) == self.ser_list_1_item
        assert Message.serialize(self.des_list_2_item) == self.ser_list_2_item

    def test_key_serialization(self):
        assert Message.serialize(self.des_kv_pair) == self.ser_kv_pair
        assert Message.serialize(self.des_kv_zero) == self.ser_kv_zero

    def test_section_deserialization(self):
        single = Message.deserialize(FiniteStream(self.ser_sec_single))
        nested = Message.deserialize(FiniteStream(self.ser_sec_nested))

        assert single == self.des_sec_single
        assert nested == self.des_sec_nested

        with pytest.raises(DeserializationException):
            Message.deserialize(FiniteStream(self.ser_sec_unclosed))

    def test_list_deserialization(self):
        l0 = Message.deserialize(FiniteStream(self.ser_list_0_item))
        l1 = Message.deserialize(FiniteStream(self.ser_list_1_item))
        l2 = Message.deserialize(FiniteStream(self.ser_list_2_item))

        assert l0 == self.des_list_0_item
        assert l1 == self.des_list_1_item
        assert l2 == self.des_list_2_item

        with pytest.raises(DeserializationException):
            Message.deserialize(FiniteStream(self.ser_list_invalid))

    def test_key_deserialization(self):
        pair = Message.deserialize(FiniteStream(self.ser_kv_pair))
        zerolength = Message.deserialize(FiniteStream(self.ser_kv_zero))

        assert pair == self.des_kv_pair
        assert zerolength == self.des_kv_zero

    def test_roundtrip(self):
        message = {
            "key1": "value1",
            "section1": {
                "sub-section": {
                    "key2": b"value2",
                },
                "list1": [ "item1", "item2" ],
            },
        }
        serialized_message = FiniteStream(Message.serialize(message))
        deserialized_message = Message.deserialize(serialized_message)

        # ensure that list items and key values remain as undecoded bytes
        deserialized_section = deserialized_message["section1"]
        assert deserialized_message["key1"] == b"value1"
        assert deserialized_section["sub-section"]["key2"] == b"value2"
        assert deserialized_section["list1"] == [ b"item1", b"item2" ]
