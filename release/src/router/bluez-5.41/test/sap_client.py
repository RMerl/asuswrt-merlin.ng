""" Copyright (C) 2010-2011 ST-Ericsson SA """

""" Author: Szymon Janc <szymon.janc@tieto.com> for ST-Ericsson. """

""" This program is free software; you can redistribute it and/or modify """
""" it under the terms of the GNU General Public License as published by """
""" the Free Software Foundation; either version 2 of the License, or """
""" (at your option) any later version. """

""" This program is distributed in the hope that it will be useful, """
""" but WITHOUT ANY WARRANTY; without even the implied warranty of """
""" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the """
""" GNU General Public License for more details. """

""" You should have received a copy of the GNU General Public License """
""" along with this program; if not, write to the Free Software """
""" Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA """

from array import array
from bluetooth import *
import time
import re

class SAPParam:
    """ SAP Parameter Class """

    MaxMsgSize = 0x00
    ConnectionStatus = 0x01
    ResultCode = 0x02
    DisconnectionType = 0x03
    CommandAPDU = 0x04
    ResponseAPDU = 0x05
    ATR = 0x06
    CardReaderStatus = 0x07
    StatusChange = 0x08
    TransportProtocol = 0x09
    CommandAPDU7816 = 0x10

    def __init__(self, name, id, value = None):
        self.name = name
        self.id = id
        self.value = value

    def _padding(self,  buf):
        pad = array('B')
        while ( (len(buf) + len(pad)) % 4 ) != 0:
            pad.append(0)
        return pad

    def _basicCheck(self,  buf):
        if len(buf) < 4 or (len(buf) % 4) != 0 or buf[1] != 0:
                return (-1,  -1)
        if buf[0] != self.id:
            return (-1,  -1)
        plen = buf[2] * 256 + buf[3] + 4
        if plen > len(buf):
            return (-1,  -1)
        pad = plen
        while (pad % 4) != 0:
            if buf[pad] != 0:
                return (-1,  -1)
            pad+=1
        return (plen,  pad)

    def getID(self):
        return self.id

    def getValue(self):
        return self.value

    def getContent(self):
        return "%s(id=0x%.2X), value=%s \n" %  (self.name,  self.id, self.value)

    def serialize(self):
        a = array('B', '\00\00\00\00')
        a[0] = self.id
        a[1] = 0	# reserved
        a[2] = 0	# length
        a[3] = 1	# length
        a.append(self.value)
        a.extend(self._padding(a))
        return a

    def deserialize(self,  buf):
        p = self._basicCheck(buf)
        if p[0] == -1:
            return -1
        self.id = buf[0]
        self.value = buf[4]
        return p[1]


class SAPParam_MaxMsgSize(SAPParam):
    """MaxMsgSize Param """

    def __init__(self,  value = None):
        SAPParam.__init__(self,"MaxMsgSize",  SAPParam.MaxMsgSize, value)
        self.__validate()

    def __validate(self):
        if self.value > 0xFFFF:
             self.value = 0xFFFF

    def serialize(self):
        a = array('B', '\00\00\00\00')
        a[0] = self.id
        a[3] = 2
        a.append(self.value / 256)
        a.append(self.value % 256)
        a.extend(self._padding(a))
        return a

    def deserialize(self,  buf):
        p = self._basicCheck(buf)
        if p[0] == -1 :
            return -1
        self.value = buf[4] * 256 + buf[5]
        return p[1]

class SAPParam_CommandAPDU(SAPParam):
    def __init__(self,  value = None):
        if value is None:
            SAPParam.__init__(self, "CommandAPDU",  SAPParam.CommandAPDU, array('B'))
        else:
            SAPParam.__init__(self, "CommandAPDU",  SAPParam.CommandAPDU, array('B', value))

    def serialize(self):
        a = array('B', '\00\00\00\00')
        a[0] = self.id
        plen = len(self.value)
        a[2] = plen / 256
        a[3] = plen % 256
        a.extend(self.value)
        a.extend(self._padding(a))
        return a

    def deserialize(self,  buf):
        p = self._basicCheck(buf)
        if p[0] == -1:
            return -1
        self.value = buf[4:p[0]]
        return p[1]

class SAPParam_ResponseAPDU(SAPParam_CommandAPDU):
    """ResponseAPDU Param """

    def __init__(self,  value = None):
        if value is None:
            SAPParam.__init__(self, "ResponseAPDU",  SAPParam.ResponseAPDU, array('B'))
        else:
            SAPParam.__init__(self, "ResponseAPDU",  SAPParam.ResponseAPDU, array('B', value))

class SAPParam_ATR(SAPParam_CommandAPDU):
    """ATR Param """

    def __init__(self,  value = None):
        if value is None:
            SAPParam.__init__(self, "ATR",  SAPParam.ATR, array('B'))
        else:
            SAPParam.__init__(self, "ATR",  SAPParam.ATR, array('B', value))

class SAPParam_CommandAPDU7816(SAPParam_CommandAPDU):
    """Command APDU7816 Param."""

    def __init__(self,  value = None):
        if value is None:
            SAPParam.__init__(self, "CommandAPDU7816",  SAPParam.CommandAPDU7816, array('B'))
        else:
            SAPParam.__init__(self, "CommandAPDU7816",  SAPParam.CommandAPDU7816, array('B', value))


class SAPParam_ConnectionStatus(SAPParam):
    """Connection status Param."""

    def __init__(self,  value = None):
        SAPParam.__init__(self,"ConnectionStatus",  SAPParam.ConnectionStatus, value)
        self.__validate()

    def __validate(self):
        if self.value is not None and self.value not in (0x00,  0x01,  0x02,  0x03,  0x04):
            print "Warning. ConnectionStatus value in reserved range (0x%x)" % self.value

    def deserialize(self,  buf):
        ret = SAPParam.deserialize(self, buf)
        if ret == -1:
            return -1
        self.__validate()
        return ret

class SAPParam_ResultCode(SAPParam):
    """ Result Code Param """

    def __init__(self,  value = None):
        SAPParam.__init__(self,"ResultCode",  SAPParam.ResultCode, value)
        self.__validate()

    def __validate(self):
        if self.value is not None and self.value not in (0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07):
            print "Warning. ResultCode value in reserved range (0x%x)" % self.value

    def deserialize(self,  buf):
        ret = SAPParam.deserialize(self, buf)
        if ret == -1:
            return -1
        self.__validate()
        return ret

class SAPParam_DisconnectionType(SAPParam):
    """Disconnection Type Param."""

    def __init__(self,  value = None):
        SAPParam.__init__(self,"DisconnectionType",  SAPParam.DisconnectionType, value)
        self.__validate()

    def __validate(self):
        if self.value is not None and self.value not in (0x00,  0x01):
            print "Warning. DisconnectionType value in reserved range (0x%x)" % self.value

    def deserialize(self,  buf):
        ret = SAPParam.deserialize(self, buf)
        if ret == -1:
            return -1
        self.__validate()
        return ret

class SAPParam_CardReaderStatus(SAPParam_CommandAPDU):
    """Card reader Status Param."""

    def __init__(self,  value = None):
        if value is None:
            SAPParam.__init__(self, "CardReaderStatus",  SAPParam.CardReaderStatus, array('B'))
        else:
            SAPParam.__init__(self, "CardReaderStatus",  SAPParam.CardReaderStatus, array('B', value))

class SAPParam_StatusChange(SAPParam):
    """Status Change Param """

    def __init__(self,  value = None):
        SAPParam.__init__(self,"StatusChange",  SAPParam.StatusChange, value)

    def __validate(self):
        if self.value is not None and self.value not in (0x00,  0x01,  0x02,  0x03,  0x04,  0x05):
            print "Warning. StatusChange value in reserved range (0x%x)" % self.value

    def deserialize(self,  buf):
        ret = SAPParam.deserialize(self, buf)
        if ret == -1:
            return -1
        self.__validate()
        return ret

class SAPParam_TransportProtocol(SAPParam):
    """Transport Protocol Param """

    def __init__(self,  value = None):
        SAPParam.__init__(self,"TransportProtocol",  SAPParam.TransportProtocol, value)
        self.__validate()

    def __validate(self):
        if self.value is not None and self.value not in (0x00,  0x01):
            print "Warning. TransportProtoco value in reserved range (0x%x)" % self.value

    def deserialize(self,  buf):
        ret = SAPParam.deserialize(self, buf)
        if ret == -1:
            return -1
        self.__validate()
        return ret

class SAPMessage:

    CONNECT_REQ = 0x00
    CONNECT_RESP = 0x01
    DISCONNECT_REQ = 0x02
    DISCONNECT_RESP =0x03
    DISCONNECT_IND = 0x04
    TRANSFER_APDU_REQ = 0x05
    TRANSFER_APDU_RESP = 0x06
    TRANSFER_ATR_REQ = 0x07
    TRANSFER_ATR_RESP = 0x08
    POWER_SIM_OFF_REQ = 0x09
    POWER_SIM_OFF_RESP = 0x0A
    POWER_SIM_ON_REQ = 0x0B
    POWER_SIM_ON_RESP = 0x0C
    RESET_SIM_REQ = 0x0D
    RESET_SIM_RESP = 0x0E
    TRANSFER_CARD_READER_STATUS_REQ = 0x0F
    TRANSFER_CARD_READER_STATUS_RESP = 0x10
    STATUS_IND = 0x11
    ERROR_RESP = 0x12
    SET_TRANSPORT_PROTOCOL_REQ = 0x13
    SET_TRANSPORT_PROTOCOL_RESP = 0x14

    def __init__(self,  name,  id):
        self.name = name
        self.id = id
        self.params = []
        self.buf = array('B')

    def _basicCheck(self,  buf):
        if len(buf) < 4 or (len(buf) % 4) != 0 :
            return False

        if buf[0] != self.id:
            return False

        return True

    def getID(self):
        return self.id

    def getContent(self):
        s = "%s(id=0x%.2X) " % (self.name,  self.id)
        if len( self.buf): s = s + "[%s]" % re.sub("(.{2})", "0x\\1 " , self.buf.tostring().encode("hex").upper(), re.DOTALL)
        s = s + "\n\t"
        for p in self.params:
            s = s + "\t" + p.getContent()
        return s

    def getParams(self):
        return self.params

    def addParam(self,  param):
        self.params.append(param)

    def serialize(self):
        ret = array('B', '\00\00\00\00')
        ret[0] = self.id
        ret[1] = len(self.params)
        ret[2] = 0	# reserved
        ret[3] = 0	# reserved
        for p in self.params:
            ret.extend(p.serialize())

        self.buf = ret
        return ret

    def deserialize(self,  buf):
        self.buf = buf
        return len(buf) == 4 and buf[1] == 0 and self._basicCheck(buf)


class SAPMessage_CONNECT_REQ(SAPMessage):
    def __init__(self,  MaxMsgSize = None):
        SAPMessage.__init__(self,"CONNECT_REQ",  SAPMessage.CONNECT_REQ)
        if MaxMsgSize is not None:
            self.addParam(SAPParam_MaxMsgSize(MaxMsgSize))

    def _validate(self):
        if len(self.params) == 1:
            if self.params[0].getID() == SAPParam.MaxMsgSize:
                return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []
        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_MaxMsgSize()
            if p.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p)
                return self._validate()

        return False

class SAPMessage_CONNECT_RESP(SAPMessage):
    def __init__(self,  ConnectionStatus = None,  MaxMsgSize = None):
        SAPMessage.__init__(self,"CONNECT_RESP",  SAPMessage.CONNECT_RESP)
        if ConnectionStatus is not None:
            self.addParam(SAPParam_ConnectionStatus(ConnectionStatus))
            if MaxMsgSize is not None:
                self.addParam(SAPParam_MaxMsgSize(MaxMsgSize))

    def _validate(self):
        if len(self.params) > 0:
            if self.params[0] .getID() == SAPParam.ConnectionStatus:
                if self.params[0].getValue() ==  0x02:
                    if len(self.params) == 2:
                        return True
                else:
                    if len(self.params) == 1:
                        return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []

        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_ConnectionStatus()
            r = p.deserialize(buf[4:])
            if  r != -1:
                self.addParam(p)
                if buf[1] == 2:
                    p = SAPParam_MaxMsgSize()
                    r = p.deserialize(buf[4+r:])
                    if r != -1:
                        self.addParam(p)

                return self._validate()

        return False

class SAPMessage_DISCONNECT_REQ(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"DISCONNECT_REQ",  SAPMessage.DISCONNECT_REQ)

class SAPMessage_DISCONNECT_RESP(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"DISCONNECT_RESP",  SAPMessage.DISCONNECT_RESP)

class SAPMessage_DISCONNECT_IND(SAPMessage):
    def __init__(self,  Type = None):
        SAPMessage.__init__(self,"DISCONNECT_IND",  SAPMessage.DISCONNECT_IND)
        if Type is not None:
            self.addParam(SAPParam_DisconnectionType(Type))

    def _validate(self):
        if len(self.params) == 1:
            if self.params[0].getID() == SAPParam.DisconnectionType:
                return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []
        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_DisconnectionType()
            if p.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p)
                return self._validate()

        return False


class SAPMessage_TRANSFER_APDU_REQ(SAPMessage):
    def __init__(self,  APDU = None,  T = False):
        SAPMessage.__init__(self,"TRANSFER_APDU_REQ",  SAPMessage.TRANSFER_APDU_REQ)
        if APDU is not None:
            if T :
                self.addParam(SAPParam_CommandAPDU(APDU))
            else:
                self.addParam(SAPParam_CommandAPDU7816(APDU))

    def _validate(self):
        if len(self.params) == 1:
            if self.params[0].getID() == SAPParam.CommandAPDU or self.params[0].getID() == SAPParam.CommandAPDU7816:
                return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []
        if SAPMessage._basicCheck(self,  buf):

            p = SAPParam_CommandAPDU()
            p2 = SAPParam_CommandAPDU7816()
            if p.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p)
                return self._validate()
            elif p2.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p2)
                return self._validate()

        return False

class SAPMessage_TRANSFER_APDU_RESP(SAPMessage):
    def __init__(self,  ResultCode = None,  Response = None):
        SAPMessage.__init__(self,"TRANSFER_APDU_RESP",  SAPMessage.TRANSFER_APDU_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))
            if Response is not None:
                self.addParam(SAPParam_ResponseAPDU(Response))

    def _validate(self):
        if len(self.params) > 0:
            if self.params[0] .getID() == SAPParam.ResultCode:
                if self.params[0].getValue() == 0x00:
                    if len(self.params) == 2:
                        return True
                else:
                    if len(self.params) == 1:
                        return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []

        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_ResultCode()
            r = p.deserialize(buf[4:])
            if  r != -1:
                self.addParam(p)
                if buf[1] == 2:
                    p = SAPParam_ResponseAPDU()
                    r = p.deserialize(buf[4+r:])
                    if r != -1:
                        self.addParam(p)

                return self._validate()

        return False

class SAPMessage_TRANSFER_ATR_REQ(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"TRANSFER_ATR_REQ",  SAPMessage.TRANSFER_ATR_REQ)

class SAPMessage_TRANSFER_ATR_RESP(SAPMessage):
    def __init__(self,  ResultCode = None,  ATR = None):
        SAPMessage.__init__(self,"TRANSFER_ATR_RESP",  SAPMessage.TRANSFER_ATR_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))
            if ATR is not None:
                self.addParam(SAPParam_ATR(ATR))

    def _validate(self):
        if len(self.params) > 0:
            if self.params[0] .getID() == SAPParam.ResultCode:
                if self.params[0].getValue() == 0x00:
                    if len(self.params) == 2:
                        return True
                else:
                    if len(self.params) == 1:
                        return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []

        if SAPMessage._basicCheck(self,  buf):

            p = SAPParam_ResultCode()
            r = p.deserialize(buf[4:])

            if  r != -1:

                self.addParam(p)
                if buf[1] == 2:

                    p = SAPParam_ATR()
                    r = p.deserialize(buf[4+r:])
                    if r != -1:
                        self.addParam(p)

                return self._validate()

        return False

class SAPMessage_POWER_SIM_OFF_REQ(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"POWER_SIM_OFF_REQ",  SAPMessage.POWER_SIM_OFF_REQ)

class SAPMessage_POWER_SIM_OFF_RESP(SAPMessage):
    def __init__(self,  ResultCode = None):
        SAPMessage.__init__(self,"POWER_SIM_OFF_RESP",  SAPMessage.POWER_SIM_OFF_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))

    def _validate(self):
        if len(self.params) == 1:
            if self.params[0].getID() == SAPParam.ResultCode:
                return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []
        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_ResultCode()
            if p.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p)
                return self._validate()

        return False

class SAPMessage_POWER_SIM_ON_REQ(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"POWER_SIM_ON_REQ",  SAPMessage.POWER_SIM_ON_REQ)

class SAPMessage_POWER_SIM_ON_RESP(SAPMessage_POWER_SIM_OFF_RESP):
    def __init__(self,  ResultCode = None):
        SAPMessage.__init__(self,"POWER_SIM_ON_RESP",  SAPMessage.POWER_SIM_ON_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))

class SAPMessage_RESET_SIM_REQ(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"RESET_SIM_REQ",  SAPMessage.RESET_SIM_REQ)

class SAPMessage_RESET_SIM_RESP(SAPMessage_POWER_SIM_OFF_RESP):
    def __init__(self,  ResultCode = None):
        SAPMessage.__init__(self,"RESET_SIM_RESP",  SAPMessage.RESET_SIM_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))

class SAPMessage_STATUS_IND(SAPMessage):
    def __init__(self,  StatusChange = None):
        SAPMessage.__init__(self,"STATUS_IND",  SAPMessage.STATUS_IND)
        if StatusChange is not None:
            self.addParam(SAPParam_StatusChange(StatusChange))

    def _validate(self):
        if len(self.params) == 1:
            if self.params[0].getID() == SAPParam.StatusChange:
                return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []
        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_StatusChange()
            if p.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p)
                return self._validate()

        return False

class SAPMessage_TRANSFER_CARD_READER_STATUS_REQ(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"TRANSFER_CARD_READER_STATUS_REQ",  SAPMessage.TRANSFER_CARD_READER_STATUS_REQ)

class SAPMessage_TRANSFER_CARD_READER_STATUS_RESP(SAPMessage):
    def __init__(self,  ResultCode = None,  Status = None):
        SAPMessage.__init__(self,"TRANSFER_CARD_READER_STATUS_RESP",  SAPMessage.TRANSFER_CARD_READER_STATUS_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))
            if Status is not None:
                self.addParam(SAPParam_CardReaderStatus(Status))

    def _validate(self):
        if len(self.params) > 0:
            if self.params[0] .getID() == SAPParam.ResultCode:
                if self.params[0].getValue() == 0x00:
                    if len(self.params) == 2:
                        return True
                else:
                    if len(self.params) == 1:
                        return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []

        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_ResultCode()
            r = p.deserialize(buf[4:])
            if  r != -1:
                self.addParam(p)
                if buf[1] == 2:
                    p = SAPParam_CardReaderStatus()
                    r = p.deserialize(buf[4+r:])
                    if r != -1:
                        self.addParam(p)

                return self._validate()

        return False

class SAPMessage_ERROR_RESP(SAPMessage):
    def __init__(self):
        SAPMessage.__init__(self,"ERROR_RESP",  SAPMessage.ERROR_RESP)


class SAPMessage_SET_TRANSPORT_PROTOCOL_REQ(SAPMessage):
    def __init__(self,  protocol = None):
        SAPMessage.__init__(self,"SET_TRANSPORT_PROTOCOL_REQ",  SAPMessage.SET_TRANSPORT_PROTOCOL_REQ)
        if protocol is not None:
            self.addParam(SAPParam_TransportProtocol(protocol))

    def _validate(self):
        if len(self.params) == 1:
            if self.params[0].getID() == SAPParam.TransportProtocol:
                return True
        return False

    def deserialize(self,  buf):
        self.buf = buf
        self.params[:] = []
        if SAPMessage._basicCheck(self,  buf):
            p = SAPParam_TransportProtocol()
            if p.deserialize(buf[4:]) == len(buf[4:]):
                self.addParam(p)
                return self._validate()

        return False

class SAPMessage_SET_TRANSPORT_PROTOCOL_RESP(SAPMessage_POWER_SIM_OFF_RESP):
    def __init__(self,  ResultCode = None):
        SAPMessage.__init__(self,"SET_TRANSPORT_PROTOCOL_RESP",  SAPMessage.SET_TRANSPORT_PROTOCOL_RESP)
        if ResultCode is not None:
            self.addParam(SAPParam_ResultCode(ResultCode))


class SAPClient:

    CONNECTED = 1
    DISCONNECTED = 0

    uuid = "0000112D-0000-1000-8000-00805F9B34FB"
    bufsize = 1024
    timeout = 20
    state = DISCONNECTED

    def __init__(self,  host = None,  port = None):
        self.sock = None

        if host is None or is_valid_address(host):
            self.host = host
        else:
            raise BluetoothError ("%s is not a valid BT address." % host)
            self.host = None
            return

        if port is None:
            self.__discover()
        else:
            self.port = port

        self.__connectRFCOMM()

    def __del__(self):
        self.__disconnectRFCOMM()

    def __disconnectRFCOMM(self):
        if self.sock is not None:
            self.sock.close()
            self.state = self.DISCONNECTED

    def __discover(self):
        service_matches = find_service(self.uuid, self.host)

        if len(service_matches) == 0:
            raise BluetoothError ("No SAP service found")
            return

        first_match = service_matches[0]
        self.port = first_match["port"]
        self.host = first_match["host"]

        print "SAP Service found on %s(%s)" % first_match["name"] % self.host

    def __connectRFCOMM(self):
        self.sock=BluetoothSocket( RFCOMM )
        self.sock.connect((self.host, self.port))
        self.sock.settimeout(self.timeout)
        self.state = self.CONNECTED

    def __sendMsg(self, msg):
        if isinstance(msg,  SAPMessage):
            s = msg.serialize()
            print "\tTX: " + msg.getContent()
            return self.sock.send(s.tostring())

    def __rcvMsg(self,  msg):
        if isinstance(msg,  SAPMessage):
            print "\tRX Wait: %s(id = 0x%.2x)" % (msg.name, msg.id)
            data = self.sock.recv(self.bufsize)
            if data:
                if msg.deserialize(array('B',data)):
                    print "\tRX: len(%d) %s" % (len(data), msg.getContent())
                    return msg
                else:
                    print "msg: %s" % array('B',data)
                    raise BluetoothError ("Message deserialization failed.")
            else:
                raise BluetoothError ("Timeout. No data received.")

    def connect(self):
        self.__connectRFCOMM()

    def disconnect(self):
        self.__disconnectRFCOMM()

    def isConnected(self):
        return self.state

    def proc_connect(self):
        try:
            self.__sendMsg(SAPMessage_CONNECT_REQ(self.bufsize))
            params = self.__rcvMsg(SAPMessage_CONNECT_RESP()).getParams()

            if params[0].getValue() in (0x00,  0x04):
                pass
            elif params[0].getValue() == 0x02:
                self.bufsize = params[1].getValue()

                self.__sendMsg(SAPMessage_CONNECT_REQ(self.bufsize))
                params = self.__rcvMsg(SAPMessage_CONNECT_RESP()).getParams()

                if params[0].getValue() not in (0x00,  0x04):
                    return False
            else:
                return False

            params = self.__rcvMsg(SAPMessage_STATUS_IND()).getParams()
            if params[0].getValue() == 0x00:
                return False
            elif params[0].getValue() == 0x01:
                """OK, Card reset"""
                return self.proc_transferATR()
            elif params[0].getValue() == 0x02:
                """T0 not supported"""
                if self.proc_transferATR():
                    return self.proc_setTransportProtocol(1)
                else:
                    return False
            else:
                return False
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_disconnectByClient(self, timeout=0):
        try:
            self.__sendMsg(SAPMessage_DISCONNECT_REQ())
            self.__rcvMsg(SAPMessage_DISCONNECT_RESP())
            time.sleep(timeout) # let srv to close rfcomm
            self.__disconnectRFCOMM()
            return True
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_disconnectByServer(self, timeout=0):
        try:
            params = self.__rcvMsg(SAPMessage_DISCONNECT_IND()).getParams()

            """graceful"""
            if params[0].getValue() == 0x00:
                if not self.proc_transferAPDU():
                    return False

            return self.proc_disconnectByClient(timeout)

        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_transferAPDU(self,  apdu = "Sample APDU command"):
        try:
            self.__sendMsg(SAPMessage_TRANSFER_APDU_REQ(apdu))
            params = self.__rcvMsg(SAPMessage_TRANSFER_APDU_RESP()).getParams()
            return True
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_transferATR(self):
        try:
            self.__sendMsg(SAPMessage_TRANSFER_ATR_REQ())
            params = self.__rcvMsg(SAPMessage_TRANSFER_ATR_RESP()).getParams()
            return True
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_powerSimOff(self):
        try:
            self.__sendMsg(SAPMessage_POWER_SIM_OFF_REQ())
            params = self.__rcvMsg(SAPMessage_POWER_SIM_OFF_RESP()).getParams()
            return True
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_powerSimOn(self):
        try:
            self.__sendMsg(SAPMessage_POWER_SIM_ON_REQ())
            params = self.__rcvMsg(SAPMessage_POWER_SIM_ON_RESP()).getParams()
            if params[0].getValue() == 0x00:
                return self.proc_transferATR()

            return True
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_resetSim(self):
        try:
            self.__sendMsg(SAPMessage_RESET_SIM_REQ())
            params = self.__rcvMsg(SAPMessage_RESET_SIM_RESP()).getParams()
            if params[0].getValue() == 0x00:
                return self.proc_transferATR()

            return True
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_reportStatus(self):
        try:
            params = self.__rcvMsg(SAPMessage_STATUS_IND()).getParams()
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_transferCardReaderStatus(self):
        try:
            self.__sendMsg(SAPMessage_TRANSFER_CARD_READER_STATUS_REQ())
            params = self.__rcvMsg(SAPMessage_TRANSFER_CARD_READER_STATUS_RESP()).getParams()
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_errorResponse(self):
        try:
            """ send malformed message, no mandatory maxmsgsize parameter"""
            self.__sendMsg(SAPMessage_CONNECT_REQ())

            params = self.__rcvMsg(SAPMessage_ERROR_RESP()).getParams()
        except BluetoothError , e:
            print "Error. " +str(e)
            return False

    def proc_setTransportProtocol(self,  protocol = 0):
        try:
            self.__sendMsg(SAPMessage_SET_TRANSPORT_PROTOCOL_REQ(protocol))
            params = self.__rcvMsg(SAPMessage_SET_TRANSPORT_PROTOCOL_RESP()).getParams()

            if params[0].getValue() == 0x00:
                params = self.__rcvMsg(SAPMessage_STATUS_IND()).getParams()
                if params[0].getValue() in (0x01,  0x02):
                    return self.proc_transferATR()
                else:
                    return True
                    """return False ???"""
            elif params[0].getValue == 0x07:
                """not supported"""
                return True
                """return False ???"""
            else:
                return False

        except BluetoothError , e:
            print "Error. " +str(e)
            return False

if __name__ == "__main__":
    pass
