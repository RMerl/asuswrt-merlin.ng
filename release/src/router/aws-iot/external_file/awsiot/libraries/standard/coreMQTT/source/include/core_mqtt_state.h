/*
 * coreMQTT v1.1.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file core_mqtt_state.h
 * @brief Function to keep state of MQTT PUBLISH packet deliveries.
 */
#ifndef CORE_MQTT_STATE_H
#define CORE_MQTT_STATE_H

#include "core_mqtt.h"

/**
 * @ingroup mqtt_constants
 * @brief Initializer value for an #MQTTStateCursor_t, indicating a search
 * should start at the beginning of a state record array
 */
#define MQTT_STATE_CURSOR_INITIALIZER    ( ( size_t ) 0 )

/**
 * @ingroup mqtt_basic_types
 * @brief Cursor for iterating through state records.
 */
typedef size_t MQTTStateCursor_t;

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this section, this enum is private.
 *
 * @brief Value indicating either send or receive.
 */
typedef enum MQTTStateOperation
{
    MQTT_SEND,
    MQTT_RECEIVE
} MQTTStateOperation_t;
/** @endcond */

/**
 * @fn MQTTStatus_t MQTT_ReserveState( MQTTContext_t * pMqttContext, uint16_t packetId, MQTTQoS_t qos );
 * @brief Reserve an entry for an outgoing QoS 1 or Qos 2 publish.
 *
 * @param[in] pMqttContext Initialized MQTT context.
 * @param[in] packetId The ID of the publish packet.
 * @param[in] qos 1 or 2.
 *
 * @return MQTTSuccess, MQTTNoMemory, or MQTTStateCollision.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
MQTTStatus_t MQTT_ReserveState( MQTTContext_t * pMqttContext,
                                uint16_t packetId,
                                MQTTQoS_t qos );
/** @endcond */

/**
 * @fn MQTTPublishState_t MQTT_CalculateStatePublish( MQTTStateOperation_t opType, MQTTQoS_t qos )
 * @brief Calculate the new state for a publish from its qos and operation type.
 *
 * @param[in] opType Send or Receive.
 * @param[in] qos 0, 1, or 2.
 *
 * @return The calculated state.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
MQTTPublishState_t MQTT_CalculateStatePublish( MQTTStateOperation_t opType,
                                               MQTTQoS_t qos );
/** @endcond */

/**
 * @fn MQTTStatus_t MQTT_UpdateStatePublish( MQTTContext_t * pMqttContext, uint16_t packetId, MQTTStateOperation_t opType, MQTTQoS_t qos, MQTTPublishState_t * pNewState );
 * @brief Update the state record for a PUBLISH packet.
 *
 * @param[in] pMqttContext Initialized MQTT context.
 * @param[in] packetId ID of the PUBLISH packet.
 * @param[in] opType Send or Receive.
 * @param[in] qos 0, 1, or 2.
 * @param[out] pNewState Updated state of the publish.
 *
 * @return #MQTTBadParameter, #MQTTIllegalState, #MQTTStateCollision or
 * #MQTTSuccess.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
MQTTStatus_t MQTT_UpdateStatePublish( MQTTContext_t * pMqttContext,
                                      uint16_t packetId,
                                      MQTTStateOperation_t opType,
                                      MQTTQoS_t qos,
                                      MQTTPublishState_t * pNewState );
/** @endcond */

/**
 * @fn MQTTPublishState_t MQTT_CalculateStateAck( MQTTPubAckType_t packetType, MQTTStateOperation_t opType, MQTTQoS_t qos );
 * @brief Calculate the state from a PUBACK, PUBREC, PUBREL, or PUBCOMP.
 *
 * @param[in] packetType PUBACK, PUBREC, PUBREL, or PUBCOMP.
 * @param[in] opType Send or Receive.
 * @param[in] qos 1 or 2.
 *
 * @return The calculated state.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
MQTTPublishState_t MQTT_CalculateStateAck( MQTTPubAckType_t packetType,
                                           MQTTStateOperation_t opType,
                                           MQTTQoS_t qos );
/** @endcond */

/**
 * @fn MQTTStatus_t MQTT_UpdateStateAck( MQTTContext_t * pMqttContext, uint16_t packetId, MQTTPubAckType_t packetType, MQTTStateOperation_t opType, MQTTPublishState_t * pNewState );
 * @brief Update the state record for an ACKed publish.
 *
 * @param[in] pMqttContext Initialized MQTT context.
 * @param[in] packetId ID of the ack packet.
 * @param[in] packetType PUBACK, PUBREC, PUBREL, or PUBCOMP.
 * @param[in] opType Send or Receive.
 * @param[out] pNewState Updated state of the publish.
 *
 * @return #MQTTBadParameter if an invalid parameter is passed;
 * #MQTTBadResponse if the packet from the network is not found in the records;
 * #MQTTIllegalState if the requested update would result in an illegal transition;
 * #MQTTSuccess otherwise.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
MQTTStatus_t MQTT_UpdateStateAck( MQTTContext_t * pMqttContext,
                                  uint16_t packetId,
                                  MQTTPubAckType_t packetType,
                                  MQTTStateOperation_t opType,
                                  MQTTPublishState_t * pNewState );
/** @endcond */

/**
 * @fn uint16_t MQTT_PubrelToResend( const MQTTContext_t * pMqttContext, MQTTStateCursor_t * pCursor, MQTTPublishState_t * pState );
 * @brief Get the packet ID of next pending PUBREL ack to be resent.
 *
 * This function will need to be called to get the packet for which a PUBREL
 * need to be sent when a session is reestablished. Calling this function
 * repeatedly until packet id is 0 will give all the packets for which
 * a PUBREL need to be resent in the correct order.
 *
 * @param[in] pMqttContext Initialized MQTT context.
 * @param[in,out] pCursor Index at which to start searching.
 * @param[out] pState State indicating that PUBREL packet need to be sent.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
uint16_t MQTT_PubrelToResend( const MQTTContext_t * pMqttContext,
                              MQTTStateCursor_t * pCursor,
                              MQTTPublishState_t * pState );
/** @endcond */

/**
 * @brief Get the packet ID of next pending publish to be resent.
 *
 * This function will need to be called to get the packet for which a publish
 * need to be sent when a session is reestablished. Calling this function
 * repeatedly until packet id is 0 will give all the packets for which
 * a publish need to be resent in the correct order.
 *
 * @param[in] pMqttContext Initialized MQTT context.
 * @param[in,out] pCursor Index at which to start searching.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // For this example assume this function returns an outgoing unacknowledged
 * // QoS 1 or 2 publish from its packet identifier.
 * MQTTPublishInfo_t * getPublish( uint16_t packetID );
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * MQTTStateCursor_t cursor = MQTT_STATE_CURSOR_INITIALIZER;
 * bool sessionPresent;
 * uint16_t packetID;
 * MQTTPublishInfo_t * pResendPublish = NULL;
 * MQTTConnectInfo_t connectInfo = { 0 };
 *
 * // This is assumed to have been initialized before the call to MQTT_Connect().
 * MQTTContext_t * pContext;
 *
 * // Set clean session to false to attempt session resumption.
 * connectInfo.cleanSession = false;
 * connectInfo.pClientIdentifier = "someClientID";
 * connectInfo.clientIdentifierLength = strlen( connectInfo.pClientIdentifier );
 * connectInfo.keepAliveSeconds = 60;
 * // Optional connect parameters are not relevant to this example.
 *
 * // Create an MQTT connection. Use 100 milliseconds as a timeout.
 * status = MQTT_Connect( pContext, &connectInfo, NULL, 100, &sessionPresent );
 *
 * if( status == MQTTSuccess )
 * {
 *      if( sessionPresent )
 *      {
 *          // Loop while packet ID is nonzero.
 *          while( ( packetID = MQTT_PublishToResend( pContext, &cursor ) ) != 0 )
 *          {
 *              // Assume this function will succeed.
 *              pResendPublish = getPublish( packetID );
 *              // Set DUP flag.
 *              pResendPublish->dup = true;
 *              status = MQTT_Publish( pContext, pResendPublish, packetID );
 *
 *              if( status != MQTTSuccess )
 *              {
 *                  // Application can decide how to handle a failure.
 *              }
 *          }
 *      }
 *      else
 *      {
 *          // The broker did not resume a session, so we can clean up the
 *          // list of outgoing publishes.
 *      }
 * }
 * @endcode
 */
/* @[declare_mqtt_publishtoresend] */
uint16_t MQTT_PublishToResend( const MQTTContext_t * pMqttContext,
                               MQTTStateCursor_t * pCursor );
/* @[declare_mqtt_publishtoresend] */

/**
 * @fn const char * MQTT_State_strerror( MQTTPublishState_t state );
 * @brief State to string conversion for state engine.
 *
 * @param[in] state The state to convert to a string.
 *
 * @return The string representation of the state.
 */

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this definition, this function is private.
 */
const char * MQTT_State_strerror( MQTTPublishState_t state );
/** @endcond */

#endif /* ifndef CORE_MQTT_STATE_H */
