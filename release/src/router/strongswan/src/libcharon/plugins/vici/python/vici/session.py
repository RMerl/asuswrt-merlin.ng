import socket
import platform

from .exception import SessionException, CommandException, EventUnknownException
from .protocol import Transport, Packet, Message, RECV_TIMEOUT_DEFAULT
from .command_wrappers import CommandWrappers


class Session(CommandWrappers, object):
    def __init__(self, sock=None):
        """Establish a session with an IKE daemon.

        By default, the session will connect to the `/var/run/charon.vici` Unix
        domain socket.

        If there is a need to connect a socket in another location or set
        specific settings on the socket (like a timeout), create and connect
        a socket and pass it to the `sock` parameter.

        .. note::

            In case a timeout is set on the socket, the internal read code
            will temporarily disable it after receiving the first byte to avoid
            partial read corruptions.

        :param sock: socket connected to the IKE daemon (optional)
        :type sock: socket.socket
        """
        if sock is None:
            if platform.system() == "Windows":
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect(('127.0.0.1', 4502))
            else:
                sock = socket.socket(socket.AF_UNIX)
                sock.connect("/var/run/charon.vici")
        self.transport = Transport(sock)

    def _communicate(self, packet):
        """Send packet over transport and parse response.

        :param packet: packet to send
        :type packet: :py:class:`vici.protocol.Packet`
        :return: parsed packet in a tuple with message type and payload
        :rtype: :py:class:`collections.namedtuple`
        """
        self.transport.send(packet)
        return Packet.parse(self.transport.receive())

    def _register_unregister(self, event_type, register):
        """Register or unregister for the given event.

        :param event_type: event to register
        :type event_type: str
        :param register: whether to register or unregister
        :type register: bool
        """
        if register:
            packet = Packet.register_event(event_type)
        else:
            packet = Packet.unregister_event(event_type)
        response = self._communicate(packet)
        if response.response_type == Packet.EVENT_UNKNOWN:
            raise EventUnknownException(
                "Unknown event type '{event}'".format(event=event_type)
            )
        while response.response_type == Packet.EVENT:
            response = Packet.parse(self.transport.receive())
        if response.response_type != Packet.EVENT_CONFIRM:
            raise SessionException(
                "Unexpected response type {type}, "
                "expected '{confirm}' (EVENT_CONFIRM)".format(
                    type=response.response_type,
                    confirm=Packet.EVENT_CONFIRM,
                )
            )

    def request(self, command, message=None):
        """Send request with an optional message.

        :param command: command to send
        :type command: str
        :param message: message (optional)
        :type message: str
        :return: command result
        :rtype: dict
        """
        if message is not None:
            message = Message.serialize(message)
        packet = Packet.request(command, message)
        response = self._communicate(packet)

        if response.response_type != Packet.CMD_RESPONSE:
            raise SessionException(
                "Unexpected response type {type}, "
                "expected '{response}' (CMD_RESPONSE)".format(
                    type=response.response_type,
                    response=Packet.CMD_RESPONSE
                )
            )

        command_response = Message.deserialize(response.payload)
        if "success" in command_response:
            if command_response["success"] != b"yes":
                raise CommandException(
                    "Command failed: {errmsg}".format(
                        errmsg=command_response["errmsg"].decode("UTF-8")
                    )
                )

        return command_response

    def streamed_request(self, command, event_stream_type, message=None):
        """Send command request and collect and return all emitted events.

        :param command: command to send
        :type command: str
        :param event_stream_type: event type emitted on command execution
        :type event_stream_type: str
        :param message: message (optional)
        :type message: str
        :return: generator for streamed event responses as dict
        :rtype: generator
        """
        if message is not None:
            message = Message.serialize(message)

        self._register_unregister(event_stream_type, True)

        try:
            packet = Packet.request(command, message)
            self.transport.send(packet)
            exited = False
            while True:
                response = Packet.parse(self.transport.receive())
                if response.response_type == Packet.EVENT:
                    if not exited:
                        try:
                            yield Message.deserialize(response.payload)
                        except GeneratorExit:
                            exited = True
                else:
                    break

            if response.response_type == Packet.CMD_RESPONSE:
                command_response = Message.deserialize(response.payload)
            else:
                raise SessionException(
                    "Unexpected response type {type}, "
                    "expected '{response}' (CMD_RESPONSE)".format(
                        type=response.response_type,
                        response=Packet.CMD_RESPONSE
                    )
                )

        finally:
            self._register_unregister(event_stream_type, False)

        # evaluate command result, if any
        if "success" in command_response:
            if command_response["success"] != b"yes":
                raise CommandException(
                    "Command failed: {errmsg}".format(
                        errmsg=command_response["errmsg"].decode("UTF-8")
                    )
                )

    def listen(self, event_types, timeout=RECV_TIMEOUT_DEFAULT):
        """Register and listen for the given events.

        If a timeout is given, the generator produces a (None, None) tuple
        if no event has been received for that time. This allows the caller
        to either abort by breaking from the generator, or perform periodic
        tasks while staying registered within listen(), and then continue
        waiting for more events.

        :param event_types: event types to register
        :type event_types: list
        :param timeout: timeout to wait for events, in fractions of a second
        :type timeout: float
        :return: generator for streamed event responses as (event_type, dict)
        :rtype: generator
        """
        for event_type in event_types:
            self._register_unregister(event_type, True)

        try:
            while True:
                try:
                    response = Packet.parse(self.transport.receive(timeout))
                except socket.timeout:
                    yield None, None
                    continue
                if response.response_type == Packet.EVENT:
                    try:
                        msg = Message.deserialize(response.payload)
                        yield response.event_type, msg
                    except GeneratorExit:
                        break

        finally:
            for event_type in event_types:
                self._register_unregister(event_type, False)
