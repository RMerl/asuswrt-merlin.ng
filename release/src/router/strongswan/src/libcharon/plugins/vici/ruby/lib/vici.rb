##
# The Vici module implements a native ruby client side library for the
# strongSwan VICI protocol. The Connection class provides a high-level
# interface to issue requests or listen for events.
#
#  Copyright (C) 2014 Martin Willi
#  Copyright (C) 2014 revosec AG
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.

module Vici

  ##
  # Vici specific exception all others inherit from
  class Error < StandardError
  end

  ##
  # Error while parsing a vici message from the daemon
  class ParseError < Error
  end

  ##
  # Error while encoding a vici message from ruby data structures
  class EncodeError < Error
  end

  ##
  # Error while exchanging messages over the vici Transport layer
  class TransportError < Error
  end

  ##
  # Generic vici command execution error
  class CommandError < Error
  end

  ##
  # Error if an issued vici command is unknown by the daemon
  class CommandUnknownError < CommandError
  end

  ##
  # Error if a command failed to execute in the daemon
  class CommandExecError < CommandError
  end

  ##
  # Generic vici event handling error
  class EventError < Error
  end

  ##
  # Tried to register to / unregister from an unknown vici event
  class EventUnknownError < EventError
  end

  ##
  # Exception to raise from an event listening closure to stop listening
  class StopEventListening < Exception
  end


  ##
  # The Message class provides the low level encoding and decoding of vici
  # protocol messages. Directly using this class is usually not required.
  class Message

    SECTION_START = 1
    SECTION_END = 2
    KEY_VALUE = 3
    LIST_START = 4
    LIST_ITEM = 5
    LIST_END = 6

    def initialize(data = "")
      if data == nil
        @root = Hash.new()
      elsif data.is_a?(Hash)
        @root = data
      else
        @encoded = data
      end
    end

    ##
    # Get the raw byte encoding of an on-the-wire message
    def encoding
      if @encoded == nil
        @encoded = encode(@root)
      end
      @encoded
    end

    ##
    # Get the root element of the parsed ruby data structures
    def root
      if @root == nil
        @root = parse(@encoded)
      end
      @root
    end

    private

    def encode_name(name)
      [name.length].pack("c") << name
    end

    def encode_value(value)
      if value.class != String
        value = value.to_s
      end
      [value.length].pack("n") << value
    end

    def encode_kv(encoding, key, value)
      encoding << KEY_VALUE << encode_name(key) << encode_value(value)
    end

    def encode_section(encoding, key, value)
      encoding << SECTION_START << encode_name(key)
      encoding << encode(value) << SECTION_END
    end

    def encode_list(encoding, key, value)
      encoding << LIST_START << encode_name(key)
      value.each do |item|
        encoding << LIST_ITEM << encode_value(item)
      end
      encoding << LIST_END
    end

    def encode(node)
      encoding = ""
      node.each do |key, value|
        case value.class
          when String, Fixnum, true, false
            encoding = encode_kv(encoding, key, value)
          else
            if value.is_a?(Hash)
              encoding = encode_section(encoding, key, value)
            elsif value.is_a?(Array)
              encoding = encode_list(encoding, key, value)
            else
              encoding = encode_kv(encoding, key, value)
            end
        end
      end
      encoding
    end

    def parse_name(encoding)
      len = encoding.unpack("c")[0]
      name = encoding[1, len]
      return encoding[(1 + len)..-1], name
    end

    def parse_value(encoding)
      len = encoding.unpack("n")[0]
      value = encoding[2, len]
      return encoding[(2 + len)..-1], value
    end

    def parse(encoding)
      stack = [Hash.new]
      list = nil
      while encoding.length != 0 do
        type = encoding.unpack("c")[0]
        encoding = encoding[1..-1]
        case type
          when SECTION_START
            encoding, name = parse_name(encoding)
            stack.push(stack[-1][name] = Hash.new)
          when SECTION_END
            if stack.length() == 1
              raise ParseError, "unexpected section end"
            end
            stack.pop()
          when KEY_VALUE
            encoding, name = parse_name(encoding)
            encoding, value = parse_value(encoding)
            stack[-1][name] = value
          when LIST_START
            encoding, name = parse_name(encoding)
            stack[-1][name] = []
            list = name
          when LIST_ITEM
            raise ParseError, "unexpected list item" if list == nil
            encoding, value = parse_value(encoding)
            stack[-1][list].push(value)
          when LIST_END
            raise ParseError, "unexpected list end" if list == nil
            list = nil
          else
            raise ParseError, "invalid type: #{type}"
        end
      end
      if stack.length() > 1
        raise ParseError, "unexpected message end"
      end
      stack[0]
    end
  end


  ##
  # The Transport class implements to low level segmentation of packets
  # to the underlying transport stream.  Directly using this class is usually
  # not required.
  class Transport

    CMD_REQUEST = 0
    CMD_RESPONSE = 1
    CMD_UNKNOWN = 2
    EVENT_REGISTER = 3
    EVENT_UNREGISTER = 4
    EVENT_CONFIRM = 5
    EVENT_UNKNOWN = 6
    EVENT = 7

    ##
    # Create a transport layer using a provided socket for communication.
    def initialize(socket)
      @socket = socket
      @events = Hash.new
    end

    ##
    # Receive data from socket, until len bytes read
    def recv_all(len)
      encoding = ""
      while encoding.length < len do
        data = @socket.recv(len - encoding.length)
        if data.empty?
          raise TransportError, "connection closed"
        end
        encoding << data
      end
      encoding
    end

    ##
    # Send data to socket, until all bytes sent
    def send_all(encoding)
      len = 0
      while len < encoding.length do
        len += @socket.send(encoding[len..-1], 0)
      end
    end

    ##
    # Write a packet prefixed by its length over the transport socket. Type
    # specifies the message, the optional label and message get appended.
    def write(type, label, message)
      encoding = ""
      if label
        encoding << label.length << label
      end
      if message
        encoding << message.encoding
      end
      send_all([encoding.length + 1, type].pack("Nc") + encoding)
    end

    ##
    # Read a packet from the transport socket. Returns the packet type, and
    # if available in the packet a label and the contained message.
    def read
      len = recv_all(4).unpack("N")[0]
      encoding = recv_all(len)
      type = encoding.unpack("c")[0]
      len = 1
      case type
        when CMD_REQUEST, EVENT_REGISTER, EVENT_UNREGISTER, EVENT
          label = encoding[2, encoding[1].unpack("c")[0]]
          len += label.length + 1
        when CMD_RESPONSE, CMD_UNKNOWN, EVENT_CONFIRM, EVENT_UNKNOWN
          label = nil
        else
          raise TransportError, "invalid message: #{type}"
      end
      if encoding.length == len
        return type, label, Message.new
      end
      return type, label, Message.new(encoding[len..-1])
    end

    def dispatch_event(name, message)
      @events[name].each do |handler|
        handler.call(name, message)
      end
    end

    def read_and_dispatch_event
      type, label, message = read
      p
      if type == EVENT
        dispatch_event(label, message)
      else
        raise TransportError, "unexpected message: #{type}"
      end
    end

    def read_and_dispatch_events
      loop do
        type, label, message = read
        if type == EVENT
          dispatch_event(label, message)
        else
          return type, label, message
        end
      end
    end

    ##
    # Send a command with a given name, and optionally a message. Returns
    # the reply message on success.
    def request(name, message = nil)
      write(CMD_REQUEST, name, message)
      type, label, message = read_and_dispatch_events
      case type
        when CMD_RESPONSE
          return message
        when CMD_UNKNOWN
          raise CommandUnknownError, name
        else
          raise CommandError, "invalid response for #{name}"
      end
    end

    ##
    # Register a handler method for the given event name
    def register(name, handler)
      write(EVENT_REGISTER, name, nil)
      type, label, message = read_and_dispatch_events
      case type
        when EVENT_CONFIRM
          if @events.has_key?(name)
            @events[name] += [handler]
          else
            @events[name] = [handler];
          end
        when EVENT_UNKNOWN
          raise EventUnknownError, name
        else
          raise EventError, "invalid response for #{name} register"
      end
    end

    ##
    # Unregister a handler method for the given event name
    def unregister(name, handler)
      write(EVENT_UNREGISTER, name, nil)
      type, label, message = read_and_dispatch_events
      case type
        when EVENT_CONFIRM
          @events[name] -= [handler]
        when EVENT_UNKNOWN
          raise EventUnknownError, name
        else
          raise EventError, "invalid response for #{name} unregister"
      end
    end
  end


  ##
  # The Connection class provides the high-level interface to monitor, configure
  # and control the IKE daemon. It takes a connected stream-oriented Socket for
  # the communication with the IKE daemon.
  #
  # This class takes and returns ruby objects for the exchanged message data.
  # * Sections get encoded as Hash, containing other sections as Hash, or
  # * Key/Values, where the values are Strings as Hash values
  # * Lists get encoded as Arrays with String values
  # Non-String values that are not a Hash nor an Array get converted with .to_s
  # during encoding.
  class Connection

    def initialize(socket = nil)
      if socket == nil
        socket = UNIXSocket.new("/var/run/charon.vici")
      end
      @transp = Transport.new(socket)
    end

    ##
    # List matching loaded connections. The provided closure is invoked
    # for each matching connection.
    def list_conns(match = nil, &block)
      call_with_event("list-conns", Message.new(match), "list-conn", &block)
    end

    ##
    # List matching active SAs. The provided closure is invoked for each
    # matching SA.
    def list_sas(match = nil, &block)
      call_with_event("list-sas", Message.new(match), "list-sa", &block)
    end

    ##
    # List matching installed policies. The provided closure is invoked
    # for each matching policy.
    def list_policies(match, &block)
      call_with_event("list-policies", Message.new(match), "list-policy",
                      &block)
    end

    ##
    # List matching loaded certificates. The provided closure is invoked
    # for each matching certificate definition.
    def list_certs(match = nil, &block)
      call_with_event("list-certs", Message.new(match), "list-cert", &block)
    end

    ##
    # Load a connection into the daemon.
    def load_conn(conn)
      check_success(@transp.request("load-conn", Message.new(conn)))
    end

    ##
    # Unload a connection from the daemon.
    def unload_conn(conn)
      check_success(@transp.request("unload-conn", Message.new(conn)))
    end

    ##
    # Get the names of connections managed by vici.
    def get_conns()
      @transp.request("get-conns").root
    end

    ##
    # Flush credential cache.
    def flush_certs(match = nil)
      check_success(@transp.request("flush-certs", Message.new(match)))
    end

    ##
    # Clear all loaded credentials.
    def clear_creds()
      check_success(@transp.request("clear-creds"))
    end

    ##
    # Load a certificate into the daemon.
    def load_cert(cert)
      check_success(@transp.request("load-cert", Message.new(cert)))
    end

    ##
    # Load a private key into the daemon.
    def load_key(key)
      check_success(@transp.request("load-key", Message.new(key)))
    end

    ##
    # Load a shared key into the daemon.
    def load_shared(shared)
      check_success(@transp.request("load-shared", Message.new(shared)))
    end

    ##
    # Load a virtual IP / attribute pool
    def load_pool(pool)
      check_success(@transp.request("load-pool", Message.new(pool)))
    end

    ##
    # Unload a virtual IP / attribute pool
    def unload_pool(pool)
      check_success(@transp.request("unload-pool", Message.new(pool)))
    end

    ##
    # Get the currently loaded pools.
    def get_pools(options)
      @transp.request("get-pools", Message.new(options)).root
    end

    ##
    # Initiate a connection. The provided closure is invoked for each log line.
    def initiate(options, &block)
      check_success(call_with_event("initiate", Message.new(options),
                    "control-log", &block))
    end

    ##
    # Terminate a connection. The provided closure is invoked for each log line.
    def terminate(options, &block)
      check_success(call_with_event("terminate", Message.new(options),
                    "control-log", &block))
    end

    ##
    # Redirect an IKE_SA.
    def redirect(options)
      check_success(@transp.request("redirect", Message.new(options)))
    end

    ##
    # Install a shunt/route policy.
    def install(policy)
      check_success(@transp.request("install", Message.new(policy)))
    end

    ##
    # Uninstall a shunt/route policy.
    def uninstall(policy)
      check_success(@transp.request("uninstall", Message.new(policy)))
    end

    ##
    # Reload strongswan.conf settings.
    def reload_settings
      check_success(@transp.request("reload-settings", nil))
    end

    ##
    # Get daemon statistics and information.
    def stats
      @transp.request("stats", nil).root
    end

    ##
    # Get daemon version information
    def version
      @transp.request("version", nil).root
    end

    ##
    # Listen for a set of event messages. This call is blocking, and invokes
    # the passed closure for each event received. The closure receives the
    # event name and the event message as argument. To stop listening, the
    # closure may raise a StopEventListening exception, the only caught
    # exception.
    def listen_events(events, &block)
      self.class.instance_eval do
        define_method(:listen_event) do |label, message|
          block.call(label, message.root)
        end
      end
      events.each do |event|
        @transp.register(event, method(:listen_event))
      end
      begin
        loop do
          @transp.read_and_dispatch_event
        end
      rescue StopEventListening
      ensure
        events.each do |event|
          @transp.unregister(event, method(:listen_event))
        end
      end
    end

    ##
    # Issue a command request, but register for a specific event while the
    # command is active. VICI uses this mechanism to stream potentially large
    # data objects continuously. The provided closure is invoked for all
    # event messages.
    def call_with_event(command, request, event, &block)
      self.class.instance_eval do
        define_method(:call_event) do |label, message|
          block.call(message.root)
        end
      end
      @transp.register(event, method(:call_event))
      begin
        reply = @transp.request(command, request)
      ensure
        @transp.unregister(event, method(:call_event))
      end
      reply
    end

    ##
    # Check if the reply of a command indicates "success", otherwise raise a
    # CommandExecError exception
    def check_success(reply)
      root = reply.root
      if root["success"] != "yes"
        raise CommandExecError, root["errmsg"]
      end
      root
    end
  end
end
