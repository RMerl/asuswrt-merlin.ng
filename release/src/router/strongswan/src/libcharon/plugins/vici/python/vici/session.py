import collections
import socket

from .exception import SessionException, CommandException, EventUnknownException
from .protocol import Transport, Packet, Message


class Session(object):
    def __init__(self, sock=None):
        if sock is None:
            sock = socket.socket(socket.AF_UNIX)
            sock.connect("/var/run/charon.vici")
        self.handler = SessionHandler(Transport(sock))

    def version(self):
        """Retrieve daemon and system specific version information.

        :return: daemon and system specific version information
        :rtype: dict
        """
        return self.handler.request("version")

    def stats(self):
        """Retrieve IKE daemon statistics and load information.

        :return: IKE daemon statistics and load information
        :rtype: dict
        """
        return self.handler.request("stats")

    def reload_settings(self):
        """Reload strongswan.conf settings and any plugins supporting reload.
        """
        self.handler.request("reload-settings")

    def initiate(self, sa):
        """Initiate an SA.

        :param sa: the SA to initiate
        :type sa: dict
        :return: generator for logs emitted as dict
        :rtype: generator
        """
        return self.handler.streamed_request("initiate", "control-log", sa)

    def terminate(self, sa):
        """Terminate an SA.

        :param sa: the SA to terminate
        :type sa: dict
        :return: generator for logs emitted as dict
        :rtype: generator
        """
        return self.handler.streamed_request("terminate", "control-log", sa)

    def redirect(self, sa):
        """Redirect an IKE_SA.

        :param sa: the SA to redirect
        :type sa: dict
        """
        self.handler.request("redirect", sa)

    def install(self, policy):
        """Install a trap, drop or bypass policy defined by a CHILD_SA config.

        :param policy: policy to install
        :type policy: dict
        """
        self.handler.request("install", policy)

    def uninstall(self, policy):
        """Uninstall a trap, drop or bypass policy defined by a CHILD_SA config.

        :param policy: policy to uninstall
        :type policy: dict
        """
        self.handler.request("uninstall", policy)

    def list_sas(self, filters=None):
        """Retrieve active IKE_SAs and associated CHILD_SAs.

        :param filters: retrieve only matching IKE_SAs (optional)
        :type filters: dict
        :return: generator for active IKE_SAs and associated CHILD_SAs as dict
        :rtype: generator
        """
        return self.handler.streamed_request("list-sas", "list-sa", filters)

    def list_policies(self, filters=None):
        """Retrieve installed trap, drop and bypass policies.

        :param filters: retrieve only matching policies (optional)
        :type filters: dict
        :return: generator for installed trap, drop and bypass policies as dict
        :rtype: generator
        """
        return self.handler.streamed_request("list-policies", "list-policy",
                                             filters)

    def list_conns(self, filters=None):
        """Retrieve loaded connections.

        :param filters: retrieve only matching configuration names (optional)
        :type filters: dict
        :return: generator for loaded connections as dict
        :rtype: generator
        """
        return self.handler.streamed_request("list-conns", "list-conn",
                                             filters)

    def get_conns(self):
        """Retrieve connection names loaded exclusively over vici.

        :return: connection names
        :rtype: dict
        """
        return self.handler.request("get-conns")

    def list_certs(self, filters=None):
        """Retrieve loaded certificates.

        :param filters: retrieve only matching certificates (optional)
        :type filters: dict
        :return: generator for loaded certificates as dict
        :rtype: generator
        """
        return self.handler.streamed_request("list-certs", "list-cert", filters)

    def load_conn(self, connection):
        """Load a connection definition into the daemon.

        :param connection: connection definition
        :type connection: dict
        """
        self.handler.request("load-conn", connection)

    def unload_conn(self, name):
        """Unload a connection definition.

        :param name: connection definition name
        :type name: dict
        """
        self.handler.request("unload-conn", name)

    def load_cert(self, certificate):
        """Load a certificate into the daemon.

        :param certificate: PEM or DER encoded certificate
        :type certificate: dict
        """
        self.handler.request("load-cert", certificate)

    def load_key(self, private_key):
        """Load a private key into the daemon.

        :param private_key: PEM or DER encoded key
        """
        self.handler.request("load-key", private_key)

    def load_shared(self, secret):
        """Load a shared IKE PSK, EAP or XAuth secret into the daemon.

        :param secret: shared IKE PSK, EAP or XAuth secret
        :type secret: dict
        """
        self.handler.request("load-shared", secret)

    def flush_certs(self, filter=None):
        """Flush the volatile certificate cache.

        Flush the certificate stored temporarily in the cache. The filter
        allows to flush only a certain type of certificates, e.g. CRLs.

        :param filter: flush only certificates of a given type (optional)
        :type filter: dict
        """
        self.handler.request("flush-certs", filter)

    def clear_creds(self):
        """Clear credentials loaded over vici.

        Clear all loaded certificate, private key and shared key credentials.
        This affects only credentials loaded over vici, but additionally
        flushes the credential cache.
        """
        self.handler.request("clear-creds")

    def load_pool(self, pool):
        """Load a virtual IP pool.

        Load an in-memory virtual IP and configuration attribute pool.
        Existing pools with the same name get updated, if possible.

        :param pool: virtual IP and configuration attribute pool
        :type pool: dict
        """
        return self.handler.request("load-pool", pool)

    def unload_pool(self, pool_name):
        """Unload a virtual IP pool.

        Unload a previously loaded virtual IP and configuration attribute pool.
        Unloading fails for pools with leases currently online.

        :param pool_name: pool by name
        :type pool_name: dict
        """
        self.handler.request("unload-pool", pool_name)

    def get_pools(self, options):
        """Retrieve loaded pools.

        :param options: filter by name and/or retrieve leases (optional)
        :type options: dict
        :return: loaded pools
        :rtype: dict
        """
        return self.handler.request("get-pools", options)

    def listen(self, event_types):
        """Register and listen for the given events.

        :param event_types: event types to register
        :type event_types: list
        :return: generator for streamed event responses as (event_type, dict)
        :rtype: generator
        """
        return self.handler.listen(event_types)


class SessionHandler(object):
    """Handles client command execution requests over vici."""

    def __init__(self, transport):
        self.transport = transport

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
        elif response.response_type != Packet.EVENT_CONFIRM:
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
                        errmsg=command_response["errmsg"]
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

        self._register_unregister(event_stream_type, True);

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
                            pass
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
            self._register_unregister(event_stream_type, False);

        # evaluate command result, if any
        if "success" in command_response:
            if command_response["success"] != b"yes":
                raise CommandException(
                    "Command failed: {errmsg}".format(
                        errmsg=command_response["errmsg"]
                    )
                )

    def listen(self, event_types):
        """Register and listen for the given events.

        :param event_types: event types to register
        :type event_types: list
        :return: generator for streamed event responses as (event_type, dict)
        :rtype: generator
        """
        for event_type in event_types:
            self._register_unregister(event_type, True)

        try:
            while True:
                response = Packet.parse(self.transport.receive())
                if response.response_type == Packet.EVENT:
                    try:
                        yield response.event_type, Message.deserialize(response.payload)
                    except GeneratorExit:
                        break

        finally:
            for event_type in event_types:
                self._register_unregister(event_type, False)
