/*
 *
 * Copyright (C) 2022, Broadband Forum
 * Copyright (C) 2022, Snom Technology GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file usp_record.c
 *
 * Functions to dynamically create and populate a USP Record of Connect or
 * Disconnect type.
 *
 */
#include "usp_record.h"

#include "msg_handler.h"  // For AGENT_CURRENT_PROTOCOL_VERSION
#include "usp-record.pb-c.h"

/*********************************************************************//**
**
** USPREC_WebSocketConnect_Create
**
** Creates a WebSocket Connect record
**
** \param   cont_endpoint_id - Controller endpoint_id to send the record to
** \param   msi - pointer to structure in which to return the connect record
**
** \return  None
**
**************************************************************************/
void USPREC_WebSocketConnect_Create(char *cont_endpoint_id, mtp_send_item_t *msi)
{
    UspRecord__Record rec;
    UspRecord__WebSocketConnectRecord websock;
    unsigned char *buf;
    int len;
    int size;

    // Fill in the USP Record structure
    // NOTE: No need to dynamically allocate structure members, as structure is stack allocated and will not be freed with usp_record__record__free_unpacked()
    usp_record__record__init(&rec);
    rec.version = AGENT_CURRENT_PROTOCOL_VERSION;
    rec.to_id = cont_endpoint_id;
    rec.from_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    rec.payload_security = USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT;
    rec.record_type_case = USP_RECORD__RECORD__RECORD_TYPE_WEBSOCKET_CONNECT;
    rec.websocket_connect = &websock;

    // Fill in the Websocket Connect Record structure
    usp_record__web_socket_connect_record__init(&websock);

    // Serialize the protobuf record structure into a buffer
    len = usp_record__record__get_packed_size(&rec);
    buf = USP_MALLOC(len);
    size = usp_record__record__pack(&rec, buf);
    USP_ASSERT(size == len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    // Prepare the MTP item information now it is serialized.
    MTP_EXEC_MtpSendItem_Init(msi);
    msi->usp_msg_type = INVALID_USP_MSG_TYPE;
    msi->content_type = kMtpContentType_ConnectRecord;
    msi->pbuf = buf;  // Ownership of the serialized USP Record is passed back to the caller
    msi->pbuf_len = len;
}

/*********************************************************************//**
**
** USPREC_MqttConnect_Create
**
** Creates an MQTT Connect record
**
** \param   cont_endpoint_id - Controller endpoint_id to send the record to
** \param   mqtt_version - MQTT protocol version that was used to connect to the MQTT broker
** \param   agent_topic - MQTT topic that the agent actually subscribed to
** \param   msi - pointer to structure in which to return the connect record
**
** \return  None
**
**************************************************************************/
void USPREC_MqttConnect_Create(char *cont_endpoint_id, mqtt_protocolver_t mqtt_version, char *agent_topic, mtp_send_item_t *msi)
{
    UspRecord__Record rec;
    UspRecord__MQTTConnectRecord mqtt;
    unsigned char *buf;
    int len;
    int size;

    // Fill in the USP Record structure
    // NOTE: No need to dynamically allocate structure members, as structure is stack allocated and will not be freed with usp_record__record__free_unpacked()
    usp_record__record__init(&rec);
    rec.version = AGENT_CURRENT_PROTOCOL_VERSION;
    rec.to_id = cont_endpoint_id;
    rec.from_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    rec.payload_security = USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT;
    rec.record_type_case = USP_RECORD__RECORD__RECORD_TYPE_MQTT_CONNECT;
    rec.mqtt_connect = &mqtt;

    // Fill in the MQTT Connect Record structure
    usp_record__mqttconnect_record__init(&mqtt);
    mqtt.subscribed_topic = agent_topic;

    switch (mqtt_version)
    {
        case kMqttProtocol_5_0:
            mqtt.version = USP_RECORD__MQTTCONNECT_RECORD__MQTTVERSION__V5;
            break;

        case kMqttProtocol_3_1_1:
        case kMqttProtocol_3_1:
            mqtt.version = USP_RECORD__MQTTCONNECT_RECORD__MQTTVERSION__V3_1_1;
            break;

        default:
            TERMINATE_BAD_CASE(mqtt_version);
            break;
    }

    // Serialize the protobuf record structure into a buffer
    len = usp_record__record__get_packed_size(&rec);
    buf = USP_MALLOC(len);
    size = usp_record__record__pack(&rec, buf);
    USP_ASSERT(size == len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    // Prepare the MTP item information now it is serialized.
    MTP_EXEC_MtpSendItem_Init(msi);
    msi->usp_msg_type = INVALID_USP_MSG_TYPE;
    msi->content_type = kMtpContentType_ConnectRecord;
    msi->pbuf = buf;  // Ownership of the serialized USP Record is passed back to the caller
    msi->pbuf_len = len;
}

/*********************************************************************//**
**
** USPREC_StompConnect_Create
**
** Creates a STOMP Connect record
**
** \param   cont_endpoint_id - Controller endpoint_id to send the record to
** \param   destination - STOMP destination that agent subscribed to
** \param   msi - pointer to structure in which to return the connect record
**
** \return  None
**
**************************************************************************/
void USPREC_StompConnect_Create(char *cont_endpoint_id, char *destination, mtp_send_item_t *msi)
{
    UspRecord__Record rec;
    UspRecord__STOMPConnectRecord stomp;
    unsigned char *buf;
    int len;
    int size;

    // Fill in the USP Record structure
    // NOTE: No need to dynamically allocate structure members, as structure is stack allocated and will not be freed with usp_record__record__free_unpacked()
    usp_record__record__init(&rec);
    rec.version = AGENT_CURRENT_PROTOCOL_VERSION;
    rec.to_id = cont_endpoint_id;
    rec.from_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    rec.payload_security = USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT;
    rec.record_type_case = USP_RECORD__RECORD__RECORD_TYPE_STOMP_CONNECT;
    rec.stomp_connect = &stomp;

    // Fill in the STOMP Connect Record structure
    usp_record__stompconnect_record__init(&stomp);
    stomp.subscribed_destination = destination;
    stomp.version = USP_RECORD__STOMPCONNECT_RECORD__STOMPVERSION__V1_2;  // Hardcoded to v1.2, the only supported version

    // Serialize the protobuf record structure into a buffer
    len = usp_record__record__get_packed_size(&rec);
    buf = USP_MALLOC(len);
    size = usp_record__record__pack(&rec, buf);
    USP_ASSERT(size == len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    // Prepare the MTP item information now it is serialized.
    MTP_EXEC_MtpSendItem_Init(msi);
    msi->usp_msg_type = INVALID_USP_MSG_TYPE;
    msi->content_type = kMtpContentType_ConnectRecord;
    msi->pbuf = buf;  // Ownership of the serialized USP Record is passed back to the caller
    msi->pbuf_len = len;
}

/*********************************************************************//**
**
** USPREC_Disconnect_Create
**
** Creates a Disconnect record
**
** \param   content_type - indicates whether the disconnect record is to close an E2E session or not
** \param   cont_endpoint_id - Controller endpoint_id to send the record to
** \param   reason_code - USP error code for reason to disconnect
** \param   reason_str - pointer to textual description of reason to disconnect
** \param   msi - pointer to structure in which to return the connect record
**
** \return  None
**
**************************************************************************/
void USPREC_Disconnect_Create(mtp_content_type_t content_type, char *cont_endpoint_id, uint32_t reason_code, char *reason_str, mtp_send_item_t *msi)
{
    UspRecord__Record rec;
    UspRecord__DisconnectRecord disc;
    unsigned char *buf;
    int len;
    int size;

    USP_ASSERT(reason_str != NULL);

    // If a received USP record fails to unpack, then we don't know which controller sent it
    // But we still want to close the connection, so still create a disconnect record, just with an empty controller endpoint_id
    if (cont_endpoint_id == NULL)
    {
        cont_endpoint_id = "";
    }

    // Fill in the USP Record structure
    // NOTE: No need to dynamically allocate structure members, as structure is stack allocated and will not be freed with usp_record__record__free_unpacked()
    usp_record__record__init(&rec);
    rec.version = AGENT_CURRENT_PROTOCOL_VERSION;
    rec.to_id = cont_endpoint_id;
    rec.from_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    rec.payload_security = USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT;
    rec.record_type_case = USP_RECORD__RECORD__RECORD_TYPE_DISCONNECT;
    rec.disconnect = &disc;

    // Fill in the Disconnect Record structure
    usp_record__disconnect_record__init(&disc);
    disc.reason = reason_str;
    disc.reason_code = reason_code;

    // Serialize the protobuf record structure into a buffer
    len = usp_record__record__get_packed_size(&rec);
    buf = USP_MALLOC(len);
    size = usp_record__record__pack(&rec, buf);
    USP_ASSERT(size == len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    // Prepare the MTP item information now it is serialized.
    MTP_EXEC_MtpSendItem_Init(msi);
    msi->usp_msg_type = INVALID_USP_MSG_TYPE;
    msi->content_type = content_type;
    msi->pbuf = buf;  // Ownership of the serialized USP Record passes to the queue, unless an error is returned.
    msi->pbuf_len = len;
}

