/*
 * coreMQTT v1.1.2
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
 * @file core_mqtt.h
 * @brief User-facing functions of the MQTT 3.1.1 library.
 */
#ifndef CORE_MQTT_H
#define CORE_MQTT_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* MQTT_DO_NOT_USE_CUSTOM_CONFIG allows building the MQTT library
 * without a custom config. If a custom config is provided, the
 * MQTT_DO_NOT_USE_CUSTOM_CONFIG macro should not be defined. */
#ifndef MQTT_DO_NOT_USE_CUSTOM_CONFIG
    /* Include custom config file before other headers. */
    #include "core_mqtt_config.h"
#endif

/* Include config defaults header to get default values of configs not
 * defined in core_mqtt_config.h file. */
#include "core_mqtt_config_defaults.h"

/* Include MQTT serializer library. */
#include "core_mqtt_serializer.h"

/* Include transport interface. */
#include "transport_interface.h"

/**
 * @cond DOXYGEN_IGNORE
 * The current version of this library.
 */
#define MQTT_LIBRARY_VERSION    "v1.1.2"
/** @endcond */

/**
 * @ingroup mqtt_constants
 * @brief Invalid packet identifier.
 *
 * Zero is an invalid packet identifier as per MQTT v3.1.1 spec.
 */
#define MQTT_PACKET_ID_INVALID    ( ( uint16_t ) 0U )

/* Structures defined in this file. */
struct MQTTPubAckInfo;
struct MQTTContext;
struct MQTTDeserializedInfo;

/**
 * @ingroup mqtt_callback_types
 * @brief Application provided function to query the current time in
 * milliseconds.
 *
 * @return The current time in milliseconds.
 */
typedef uint32_t (* MQTTGetCurrentTimeFunc_t )( void );

/**
 * @ingroup mqtt_callback_types
 * @brief Application callback for receiving incoming publishes and incoming
 * acks.
 *
 * @note This callback will be called only if packets are deserialized with a
 * result of #MQTTSuccess or #MQTTServerRefused. The latter can be obtained
 * when deserializing a SUBACK, indicating a broker's rejection of a subscribe.
 *
 * @param[in] pContext Initialized MQTT context.
 * @param[in] pPacketInfo Information on the type of incoming MQTT packet.
 * @param[in] pDeserializedInfo Deserialized information from incoming packet.
 */
typedef void (* MQTTEventCallback_t )( struct MQTTContext * pContext,
                                       struct MQTTPacketInfo * pPacketInfo,
                                       struct MQTTDeserializedInfo * pDeserializedInfo );

/**
 * @ingroup mqtt_enum_types
 * @brief Values indicating if an MQTT connection exists.
 */
typedef enum MQTTConnectionStatus
{
    MQTTNotConnected, /**< @brief MQTT Connection is inactive. */
    MQTTConnected     /**< @brief MQTT Connection is active. */
} MQTTConnectionStatus_t;

/**
 * @ingroup mqtt_enum_types
 * @brief The state of QoS 1 or QoS 2 MQTT publishes, used in the state engine.
 */
typedef enum MQTTPublishState
{
    MQTTStateNull = 0,  /**< @brief An empty state with no corresponding PUBLISH. */
    MQTTPublishSend,    /**< @brief The library will send an outgoing PUBLISH packet. */
    MQTTPubAckSend,     /**< @brief The library will send a PUBACK for a received PUBLISH. */
    MQTTPubRecSend,     /**< @brief The library will send a PUBREC for a received PUBLISH. */
    MQTTPubRelSend,     /**< @brief The library will send a PUBREL for a received PUBREC. */
    MQTTPubCompSend,    /**< @brief The library will send a PUBCOMP for a received PUBREL. */
    MQTTPubAckPending,  /**< @brief The library is awaiting a PUBACK for an outgoing PUBLISH. */
    MQTTPubRecPending,  /**< @brief The library is awaiting a PUBREC for an outgoing PUBLISH. */
    MQTTPubRelPending,  /**< @brief The library is awaiting a PUBREL for an incoming PUBLISH. */
    MQTTPubCompPending, /**< @brief The library is awaiting a PUBCOMP for an outgoing PUBLISH. */
    MQTTPublishDone     /**< @brief The PUBLISH has been completed. */
} MQTTPublishState_t;

/**
 * @ingroup mqtt_enum_types
 * @brief Packet types used in acknowledging QoS 1 or QoS 2 publishes.
 */
typedef enum MQTTPubAckType
{
    MQTTPuback, /**< @brief PUBACKs are sent in response to a QoS 1 PUBLISH. */
    MQTTPubrec, /**< @brief PUBRECs are sent in response to a QoS 2 PUBLISH. */
    MQTTPubrel, /**< @brief PUBRELs are sent in response to a PUBREC. */
    MQTTPubcomp /**< @brief PUBCOMPs are sent in response to a PUBREL. */
} MQTTPubAckType_t;

/**
 * @ingroup mqtt_enum_types
 * @brief The status codes in the SUBACK response to a subscription request.
 */
typedef enum MQTTSubAckStatus
{
    MQTTSubAckSuccessQos0 = 0x00, /**< @brief Success with a maximum delivery at QoS 0. */
    MQTTSubAckSuccessQos1 = 0x01, /**< @brief Success with a maximum delivery at QoS 1. */
    MQTTSubAckSuccessQos2 = 0x02, /**< @brief Success with a maximum delivery at QoS 2. */
    MQTTSubAckFailure = 0x80      /**< @brief Failure. */
} MQTTSubAckStatus_t;

/**
 * @ingroup mqtt_struct_types
 * @brief An element of the state engine records for QoS 1 or Qos 2 publishes.
 */
typedef struct MQTTPubAckInfo
{
    uint16_t packetId;               /**< @brief The packet ID of the original PUBLISH. */
    MQTTQoS_t qos;                   /**< @brief The QoS of the original PUBLISH. */
    MQTTPublishState_t publishState; /**< @brief The current state of the publish process. */
} MQTTPubAckInfo_t;

/**
 * @ingroup mqtt_struct_types
 * @brief A struct representing an MQTT connection.
 */
typedef struct MQTTContext
{
    /**
     * @brief State engine records for outgoing publishes.
     */
    MQTTPubAckInfo_t outgoingPublishRecords[ MQTT_STATE_ARRAY_MAX_COUNT ];

    /**
     * @brief State engine records for incoming publishes.
     */
    MQTTPubAckInfo_t incomingPublishRecords[ MQTT_STATE_ARRAY_MAX_COUNT ];

    /**
     * @brief The transport interface used by the MQTT connection.
     */
    TransportInterface_t transportInterface;

    /**
     * @brief The buffer used in sending and receiving packets from the network.
     */
    MQTTFixedBuffer_t networkBuffer;

    /**
     * @brief The next available ID for outgoing MQTT packets.
     */
    uint16_t nextPacketId;

    /**
     * @brief Whether the context currently has a connection to the broker.
     */
    MQTTConnectionStatus_t connectStatus;

    /**
     * @brief Function used to get millisecond timestamps.
     */
    MQTTGetCurrentTimeFunc_t getTime;

    /**
     * @brief Callback function used to give deserialized MQTT packets to the application.
     */
    MQTTEventCallback_t appCallback;

    /**
     * @brief Timestamp of the last packet sent by the library.
     */
    uint32_t lastPacketTime;

    /**
     * @brief Whether the library sent a packet during a call of #MQTT_ProcessLoop or
     * #MQTT_ReceiveLoop.
     */
    bool controlPacketSent;

    /* Keep alive members. */
    uint16_t keepAliveIntervalSec; /**< @brief Keep Alive interval. */
    uint32_t pingReqSendTimeMs;    /**< @brief Timestamp of the last sent PINGREQ. */
    bool waitingForPingResp;       /**< @brief If the library is currently awaiting a PINGRESP. */
} MQTTContext_t;

/**
 * @ingroup mqtt_struct_types
 * @brief Struct to hold deserialized packet information for an #MQTTEventCallback_t
 * callback.
 */
typedef struct MQTTDeserializedInfo
{
    uint16_t packetIdentifier;          /**< @brief Packet ID of deserialized packet. */
    MQTTPublishInfo_t * pPublishInfo;   /**< @brief Pointer to deserialized publish info. */
    MQTTStatus_t deserializationResult; /**< @brief Return code of deserialization. */
} MQTTDeserializedInfo_t;

/**
 * @brief Initialize an MQTT context.
 *
 * This function must be called on a #MQTTContext_t before any other function.
 *
 * @note The #MQTTGetCurrentTimeFunc_t function for querying time must be defined. If
 * there is no time implementation, it is the responsibility of the application
 * to provide a dummy function to always return 0, provide 0 timeouts for
 * all calls to #MQTT_Connect, #MQTT_ProcessLoop, and #MQTT_ReceiveLoop and configure
 * the #MQTT_RECV_POLLING_TIMEOUT_MS and #MQTT_SEND_RETRY_TIMEOUT_MS configurations
 * to be 0. This will result in loop functions running for a single iteration, and
 * #MQTT_Connect relying on #MQTT_MAX_CONNACK_RECEIVE_RETRY_COUNT to receive the CONNACK packet.
 *
 * @param[in] pContext The context to initialize.
 * @param[in] pTransportInterface The transport interface to use with the context.
 * @param[in] getTimeFunction The time utility function to use with the context.
 * @param[in] userCallback The user callback to use with the context to
 * notify about incoming packet events.
 * @param[in] pNetworkBuffer Network buffer provided for the context.
 *
 * @return #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSuccess otherwise.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Function for obtaining a timestamp.
 * uint32_t getTimeStampMs();
 * // Callback function for receiving packets.
 * void eventCallback(
 *      MQTTContext_t * pContext,
 *      MQTTPacketInfo_t * pPacketInfo,
 *      MQTTDeserializedInfo_t * pDeserializedInfo
 * );
 * // Network send.
 * int32_t networkSend( NetworkContext_t * pContext, const void * pBuffer, size_t bytes );
 * // Network receive.
 * int32_t networkRecv( NetworkContext_t * pContext, void * pBuffer, size_t bytes );
 *
 * MQTTContext_t mqttContext;
 * TransportInterface_t transport;
 * MQTTFixedBuffer_t fixedBuffer;
 * uint8_t buffer[ 1024 ];
 *
 * // Clear context.
 * memset( ( void * ) &mqttContext, 0x00, sizeof( MQTTContext_t ) );
 *
 * // Set transport interface members.
 * transport.pNetworkContext = &someTransportContext;
 * transport.send = networkSend;
 * transport.recv = networkRecv;
 *
 * // Set buffer members.
 * fixedBuffer.pBuffer = buffer;
 * fixedBuffer.size = 1024;
 *
 * status = MQTT_Init( &mqttContext, &transport, getTimeStampMs, eventCallback, &fixedBuffer );
 *
 * if( status == MQTTSuccess )
 * {
 *      // Do something with mqttContext. The transport and fixedBuffer structs were
 *      // copied into the context, so the original structs do not need to stay in scope.
 * }
 * @endcode
 */
/* @[declare_mqtt_init] */
MQTTStatus_t MQTT_Init( MQTTContext_t * pContext,
                        const TransportInterface_t * pTransportInterface,
                        MQTTGetCurrentTimeFunc_t getTimeFunction,
                        MQTTEventCallback_t userCallback,
                        const MQTTFixedBuffer_t * pNetworkBuffer );
/* @[declare_mqtt_init] */

/**
 * @brief Establish an MQTT session.
 *
 * This function will send MQTT CONNECT packet and receive a CONNACK packet. The
 * send and receive from the network is done through the transport interface.
 *
 * The maximum time this function waits for a CONNACK is decided in one of the
 * following ways:
 * 1. If @p timeoutMs is greater than 0:
 *    #MQTTContext_t.getTime is used to ensure that the function does not wait
 *    more than @p timeoutMs for CONNACK.
 * 2. If @p timeoutMs is 0:
 *    The network receive for CONNACK is retried up to the number of times
 *    configured by #MQTT_MAX_CONNACK_RECEIVE_RETRY_COUNT.
 *
 * @note If a dummy #MQTTGetCurrentTimeFunc_t was passed to #MQTT_Init, then a
 * timeout value of 0 MUST be passed to the API, and the #MQTT_RECV_POLLING_TIMEOUT_MS
 * and #MQTT_SEND_RETRY_TIMEOUT_MS timeout configurations MUST be set to 0.
 *
 * @param[in] pContext Initialized MQTT context.
 * @param[in] pConnectInfo MQTT CONNECT packet information.
 * @param[in] pWillInfo Last Will and Testament. Pass NULL if Last Will and
 * Testament is not used.
 * @param[in] timeoutMs Maximum time in milliseconds to wait for a CONNACK packet.
 * A zero timeout makes use of the retries for receiving CONNACK as configured with
 * #MQTT_MAX_CONNACK_RECEIVE_RETRY_COUNT.
 * @param[out] pSessionPresent Whether a previous session was present.
 * Only relevant if not establishing a clean session.
 *
 * @return #MQTTNoMemory if the #MQTTContext_t.networkBuffer is too small to
 * hold the MQTT packet;
 * #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSendFailed if transport send failed;
 * #MQTTRecvFailed if transport receive failed for CONNACK;
 * #MQTTNoDataAvailable if no data available to receive in transport until
 * the @p timeoutMs for CONNACK;
 * #MQTTSuccess otherwise.
 *
 * @note This API may spend more time than provided in the timeoutMS parameters in
 * certain conditions as listed below:
 *
 * 1. Timeouts are incorrectly configured - If the timeoutMS is less than the
 *    transport receive timeout and if a CONNACK packet is not received within
 *    the transport receive timeout, the API will spend the transport receive
 *    timeout (which is more time than the timeoutMs). It is the case of incorrect
 *    timeout configuration as the timeoutMs parameter passed to this API must be
 *    greater than the transport receive timeout. Please refer to the transport
 *    interface documentation for more details about timeout configurations.
 *
 * 2. Partial CONNACK packet is received right before the expiry of the timeout - It
 *    is possible that first two bytes of CONNACK packet (packet type and remaining
 *    length) are received right before the expiry of the timeoutMS. In that case,
 *    the API makes one more network receive call in an attempt to receive the remaining
 *    2 bytes. In the worst case, it can happen that the remaining 2 bytes are never
 *    received and this API will end up spending timeoutMs + transport receive timeout.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * MQTTConnectInfo_t connectInfo = { 0 };
 * MQTTPublishInfo_t willInfo = { 0 };
 * bool sessionPresent;
 * // This is assumed to have been initialized before calling this function.
 * MQTTContext_t * pContext;
 *
 * // True for creating a new session with broker, false if we want to resume an old one.
 * connectInfo.cleanSession = true;
 * // Client ID must be unique to broker. This field is required.
 * connectInfo.pClientIdentifier = "someClientID";
 * connectInfo.clientIdentifierLength = strlen( connectInfo.pClientIdentifier );
 *
 * // The following fields are optional.
 * // Value for keep alive.
 * connectInfo.keepAliveSeconds = 60;
 * // Optional username and password.
 * connectInfo.pUserName = "someUserName";
 * connectInfo.userNameLength = strlen( connectInfo.pUserName );
 * connectInfo.pPassword = "somePassword";
 * connectInfo.passwordLength = strlen( connectInfo.pPassword );
 *
 * // The last will and testament is optional, it will be published by the broker
 * // should this client disconnect without sending a DISCONNECT packet.
 * willInfo.qos = MQTTQoS0;
 * willInfo.pTopicName = "/lwt/topic/name";
 * willInfo.topicNameLength = strlen( willInfo.pTopicName );
 * willInfo.pPayload = "LWT Message";
 * willInfo.payloadLength = strlen( "LWT Message" );
 *
 * // Send the connect packet. Use 100 ms as the timeout to wait for the CONNACK packet.
 * status = MQTT_Connect( pContext, &connectInfo, &willInfo, 100, &sessionPresent );
 *
 * if( status == MQTTSuccess )
 * {
 *      // Since we requested a clean session, this must be false
 *      assert( sessionPresent == false );
 *
 *      // Do something with the connection.
 * }
 * @endcode
 */
/* @[declare_mqtt_connect] */
MQTTStatus_t MQTT_Connect( MQTTContext_t * pContext,
                           const MQTTConnectInfo_t * pConnectInfo,
                           const MQTTPublishInfo_t * pWillInfo,
                           uint32_t timeoutMs,
                           bool * pSessionPresent );
/* @[declare_mqtt_connect] */

/**
 * @brief Sends MQTT SUBSCRIBE for the given list of topic filters to
 * the broker.
 *
 * @param[in] pContext Initialized MQTT context.
 * @param[in] pSubscriptionList List of MQTT subscription info.
 * @param[in] subscriptionCount The number of elements in pSubscriptionList.
 * @param[in] packetId Packet ID generated by #MQTT_GetPacketId.
 *
 * @return #MQTTNoMemory if the #MQTTContext_t.networkBuffer is too small to
 * hold the MQTT packet;
 * #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSendFailed if transport write failed;
 * #MQTTSuccess otherwise.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * MQTTSubscribeInfo_t subscriptionList[ NUMBER_OF_SUBSCRIPTIONS ] = { 0 };
 * uint16_t packetId;
 * // This context is assumed to be initialized and connected.
 * MQTTContext_t * pContext;
 * // This is assumed to be a list of filters we want to subscribe to.
 * const char * filters[ NUMBER_OF_SUBSCRIPTIONS ];
 *
 * // Set each subscription.
 * for( int i = 0; i < NUMBER_OF_SUBSCRIPTIONS; i++ )
 * {
 *      subscriptionList[ i ].qos = MQTTQoS0;
 *      // Each subscription needs a topic filter.
 *      subscriptionList[ i ].pTopicFilter = filters[ i ];
 *      subscriptionList[ i ].topicFilterLength = strlen( filters[ i ] );
 * }
 *
 * // Obtain a new packet id for the subscription.
 * packetId = MQTT_GetPacketId( pContext );
 *
 * status = MQTT_Subscribe( pContext, &subscriptionList[ 0 ], NUMBER_OF_SUBSCRIPTIONS, packetId );
 *
 * if( status == MQTTSuccess )
 * {
 *      // We must now call MQTT_ReceiveLoop() or MQTT_ProcessLoop() to receive the SUBACK.
 *      // If the broker accepts the subscription we can now receive publishes
 *      // on the requested topics.
 * }
 * @endcode
 */
/* @[declare_mqtt_subscribe] */
MQTTStatus_t MQTT_Subscribe( MQTTContext_t * pContext,
                             const MQTTSubscribeInfo_t * pSubscriptionList,
                             size_t subscriptionCount,
                             uint16_t packetId );
/* @[declare_mqtt_subscribe] */

/**
 * @brief Publishes a message to the given topic name.
 *
 * @param[in] pContext Initialized MQTT context.
 * @param[in] pPublishInfo MQTT PUBLISH packet parameters.
 * @param[in] packetId packet ID generated by #MQTT_GetPacketId.
 *
 * @return #MQTTNoMemory if pBuffer is too small to hold the MQTT packet;
 * #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSendFailed if transport write failed;
 * #MQTTSuccess otherwise.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * MQTTPublishInfo_t publishInfo;
 * uint16_t packetId;
 * // This context is assumed to be initialized and connected.
 * MQTTContext_t * pContext;
 *
 * // QoS of publish.
 * publishInfo.qos = MQTTQoS1;
 * publishInfo.pTopicName = "/some/topic/name";
 * publishInfo.topicNameLength = strlen( publishInfo.pTopicName );
 * publishInfo.pPayload = "Hello World!";
 * publishInfo.payloadLength = strlen( "Hello World!" );
 *
 * // Packet ID is needed for QoS > 0.
 * packetId = MQTT_GetPacketId( pContext );
 *
 * status = MQTT_Publish( pContext, &publishInfo, packetId );
 *
 * if( status == MQTTSuccess )
 * {
 *      // Since the QoS is > 0, we will need to call MQTT_ReceiveLoop()
 *      // or MQTT_ProcessLoop() to process the publish acknowledgments.
 * }
 * @endcode
 */
/* @[declare_mqtt_publish] */
MQTTStatus_t MQTT_Publish( MQTTContext_t * pContext,
                           const MQTTPublishInfo_t * pPublishInfo,
                           uint16_t packetId );
/* @[declare_mqtt_publish] */

/**
 * @brief Sends an MQTT PINGREQ to broker.
 *
 * @param[in] pContext Initialized and connected MQTT context.
 *
 * @return #MQTTNoMemory if pBuffer is too small to hold the MQTT packet;
 * #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSendFailed if transport write failed;
 * #MQTTSuccess otherwise.
 */
/* @[declare_mqtt_ping] */
MQTTStatus_t MQTT_Ping( MQTTContext_t * pContext );
/* @[declare_mqtt_ping] */

/**
 * @brief Sends MQTT UNSUBSCRIBE for the given list of topic filters to
 * the broker.
 *
 * @param[in] pContext Initialized MQTT context.
 * @param[in] pSubscriptionList List of MQTT subscription info.
 * @param[in] subscriptionCount The number of elements in pSubscriptionList.
 * @param[in] packetId packet ID generated by #MQTT_GetPacketId.
 *
 * @return #MQTTNoMemory if the #MQTTContext_t.networkBuffer is too small to
 * hold the MQTT packet;
 * #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSendFailed if transport write failed;
 * #MQTTSuccess otherwise.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * MQTTSubscribeInfo_t unsubscribeList[ NUMBER_OF_SUBSCRIPTIONS ] = { 0 };
 * uint16_t packetId;
 * // This context is assumed to be initialized and connected.
 * MQTTContext_t * pContext;
 * // This is assumed to be a list of filters we want to unsubscribe from.
 * const char * filters[ NUMBER_OF_SUBSCRIPTIONS ];
 *
 * // Set information for each unsubscribe request.
 * for( int i = 0; i < NUMBER_OF_SUBSCRIPTIONS; i++ )
 * {
 *      unsubscribeList[ i ].pTopicFilter = filters[ i ];
 *      unsubscribeList[ i ].topicFilterLength = strlen( filters[ i ] );
 *
 *      // The QoS field of MQTT_SubscribeInfo_t is unused for unsubscribing.
 * }
 *
 * // Obtain a new packet id for the unsubscribe request.
 * packetId = MQTT_GetPacketId( pContext );
 *
 * status = MQTT_Unsubscribe( pContext, &unsubscribeList[ 0 ], NUMBER_OF_SUBSCRIPTIONS, packetId );
 *
 * if( status == MQTTSuccess )
 * {
 *      // We must now call MQTT_ReceiveLoop() or MQTT_ProcessLoop() to receive the UNSUBACK.
 *      // After this the broker should no longer send publishes for these topics.
 * }
 * @endcode
 */
/* @[declare_mqtt_unsubscribe] */
MQTTStatus_t MQTT_Unsubscribe( MQTTContext_t * pContext,
                               const MQTTSubscribeInfo_t * pSubscriptionList,
                               size_t subscriptionCount,
                               uint16_t packetId );
/* @[declare_mqtt_unsubscribe] */

/**
 * @brief Disconnect an MQTT session.
 *
 * @param[in] pContext Initialized and connected MQTT context.
 *
 * @return #MQTTNoMemory if the #MQTTContext_t.networkBuffer is too small to
 * hold the MQTT packet;
 * #MQTTBadParameter if invalid parameters are passed;
 * #MQTTSendFailed if transport send failed;
 * #MQTTSuccess otherwise.
 */
/* @[declare_mqtt_disconnect] */
MQTTStatus_t MQTT_Disconnect( MQTTContext_t * pContext );
/* @[declare_mqtt_disconnect] */

/**
 * @brief Loop to receive packets from the transport interface. Handles keep
 * alive.
 *
 * @note Passing a timeout value of 0 will run the loop for a single iteration.
 *
 * @note If a dummy timer function, #MQTTGetCurrentTimeFunc_t, is passed to the library,
 * then the keep-alive mechanism is not supported by the #MQTT_ProcessLoop API.
 * In that case, the #MQTT_ReceiveLoop API function should be used instead.
 *
 * @param[in] pContext Initialized and connected MQTT context.
 * @param[in] timeoutMs Minimum time in milliseconds that the receive loop will
 * run, unless an error occurs.
 *
 * @note Calling this function blocks the calling context for a time period that
 * depends on the passed @p timeoutMs, the configuration macros, #MQTT_RECV_POLLING_TIMEOUT_MS
 * and #MQTT_SEND_RETRY_TIMEOUT_MS, and the underlying transport interface implementation
 * timeouts, unless an error occurs. The blocking period also depends on the execution time of the
 * #MQTTEventCallback_t callback supplied to the library. It is recommended that the supplied
 * #MQTTEventCallback_t callback does not contain blocking operations to prevent potential
 * non-deterministic blocking period of the #MQTT_ProcessLoop API call.
 *
 * @return #MQTTBadParameter if context is NULL;
 * #MQTTRecvFailed if a network error occurs during reception;
 * #MQTTSendFailed if a network error occurs while sending an ACK or PINGREQ;
 * #MQTTBadResponse if an invalid packet is received;
 * #MQTTKeepAliveTimeout if the server has not sent a PINGRESP before
 * #MQTT_PINGRESP_TIMEOUT_MS milliseconds;
 * #MQTTIllegalState if an incoming QoS 1/2 publish or ack causes an
 * invalid transition for the internal state machine;
 * #MQTTSuccess on success.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * uint32_t timeoutMs = 100;
 * // This context is assumed to be initialized and connected.
 * MQTTContext_t * pContext;
 *
 * while( true )
 * {
 *      status = MQTT_ProcessLoop( pContext, timeoutMs );
 *
 *      if( status != MQTTSuccess )
 *      {
 *          // Determine the error. It's possible we might need to disconnect
 *          // the underlying transport connection.
 *      }
 *      else
 *      {
 *          // Other application functions.
 *      }
 * }
 * @endcode
 */
/* @[declare_mqtt_processloop] */
MQTTStatus_t MQTT_ProcessLoop( MQTTContext_t * pContext,
                               uint32_t timeoutMs );
/* @[declare_mqtt_processloop] */

/**
 * @brief Loop to receive packets from the transport interface. Does not handle
 * keep alive.
 *
 * @note Passing a timeout value of 0 will run the loop for a single iteration.
 * If a dummy #MQTTGetCurrentTimeFunc_t was passed to #MQTT_Init, then the timeout
 * value passed to the API MUST be 0, and the #MQTT_RECV_POLLING_TIMEOUT_MS
 * and #MQTT_SEND_RETRY_TIMEOUT_MS timeout configurations MUST be set to 0.
 *
 * @param[in] pContext Initialized and connected MQTT context.
 * @param[in] timeoutMs Minimum time in milliseconds that the receive loop will
 * run, unless an error occurs.
 *
 * @note Calling this function blocks the calling context for a time period that
 * depends on the passed @p timeoutMs, the configuration macros, #MQTT_RECV_POLLING_TIMEOUT_MS
 * and #MQTT_SEND_RETRY_TIMEOUT_MS, and the underlying transport interface implementation
 * timeouts, unless an error occurs. The blocking period also depends on the execution time of the
 * #MQTTEventCallback_t callback supplied to the library. It is recommended that the supplied
 * #MQTTEventCallback_t callback does not contain blocking operations to prevent potential
 * non-deterministic blocking period of the #MQTT_ReceiveLoop API call.
 *
 * @return #MQTTBadParameter if context is NULL;
 * #MQTTRecvFailed if a network error occurs during reception;
 * #MQTTSendFailed if a network error occurs while sending an ACK or PINGREQ;
 * #MQTTBadResponse if an invalid packet is received;
 * #MQTTIllegalState if an incoming QoS 1/2 publish or ack causes an
 * invalid transition for the internal state machine;
 * #MQTTSuccess on success.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * MQTTStatus_t status;
 * uint32_t timeoutMs = 100;
 * uint32_t keepAliveMs = 60 * 1000;
 * // This context is assumed to be initialized and connected.
 * MQTTContext_t * pContext;
 *
 * while( true )
 * {
 *      status = MQTT_ReceiveLoop( pContext, timeoutMs );
 *
 *      if( status != MQTTSuccess )
 *      {
 *          // Determine the error. It's possible we might need to disconnect
 *          // the underlying transport connection.
 *      }
 *      else
 *      {
 *          // Since this function does not send pings, the application may need
 *          // to in order to comply with keep alive.
 *          if( ( pContext->getTime() - pContext->lastPacketTime ) > keepAliveMs )
 *          {
 *              status = MQTT_Ping( pContext );
 *          }
 *
 *          // Other application functions.
 *      }
 * }
 * @endcode
 */
/* @[declare_mqtt_receiveloop] */
MQTTStatus_t MQTT_ReceiveLoop( MQTTContext_t * pContext,
                               uint32_t timeoutMs );
/* @[declare_mqtt_receiveloop] */

/**
 * @brief Get a packet ID that is valid according to the MQTT 3.1.1 spec.
 *
 * @param[in] pContext Initialized MQTT context.
 *
 * @return A non-zero number.
 */
/* @[declare_mqtt_getpacketid] */
uint16_t MQTT_GetPacketId( MQTTContext_t * pContext );
/* @[declare_mqtt_getpacketid] */

/**
 * @brief A utility function that determines whether the passed topic filter and
 * topic name match according to the MQTT 3.1.1 protocol specification.
 *
 * @param[in] pTopicName The topic name to check.
 * @param[in] topicNameLength Length of the topic name.
 * @param[in] pTopicFilter The topic filter to check.
 * @param[in] topicFilterLength Length of topic filter.
 * @param[out] pIsMatch This is filled with the whether there
 * exists a match or not.
 *
 * @note The API assumes that the passed topic name is valid to meet the
 * requirements of the MQTT 3.1.1 specification. Invalid topic names (for example,
 * containing wildcard characters) should not be passed to the function.
 * Also, the API checks validity of topic filter for wildcard characters ONLY if
 * the passed topic name and topic filter do not have an exact string match.
 *
 * @return Returns one of the following:
 * - #MQTTBadParameter, if any of the input parameters is invalid.
 * - #MQTTSuccess, if the matching operation was performed.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Variables used in this example.
 * const char * pTopic = "topic/match/1";
 * const char * pFilter = "topic/#";
 * MQTTStatus_t status = MQTTSuccess;
 * bool match = false;
 *
 * status = MQTT_MatchTopic( pTopic, strlen( pTopic ), pFilter, strlen( pFilter ), &match );
 * // Our parameters were valid, so this will return success.
 * assert( status == MQTTSuccess );
 *
 * // For this specific example, we already know this value is true. This
 * // check is placed here as an example for use with variable topic names.
 * if( match )
 * {
 *      // Application can decide what to do with the matching topic name.
 * }
 * @endcode
 */
MQTTStatus_t MQTT_MatchTopic( const char * pTopicName,
                              const uint16_t topicNameLength,
                              const char * pTopicFilter,
                              const uint16_t topicFilterLength,
                              bool * pIsMatch );

/**
 * @brief Parses the payload of an MQTT SUBACK packet that contains status codes
 * corresponding to topic filter subscription requests from the original
 * subscribe packet.
 *
 * Each return code in the SUBACK packet corresponds to a topic filter in the
 * SUBSCRIBE Packet being acknowledged.
 * The status codes can be one of the following:
 *  - 0x00 - Success - Maximum QoS 0
 *  - 0x01 - Success - Maximum QoS 1
 *  - 0x02 - Success - Maximum QoS 2
 *  - 0x80 - Failure
 * Refer to #MQTTSubAckStatus_t for the status codes.
 *
 * @param[in] pSubackPacket The SUBACK packet whose payload is to be parsed.
 * @param[out] pPayloadStart This is populated with the starting address
 * of the payload (or return codes for topic filters) in the SUBACK packet.
 * @param[out] pPayloadSize This is populated with the size of the payload
 * in the SUBACK packet. It represents the number of topic filters whose
 * SUBACK status is present in the packet.
 *
 * @return Returns one of the following:
 * - #MQTTBadParameter if the input SUBACK packet is invalid.
 * - #MQTTSuccess if parsing the payload was successful.
 *
 * <b>Example</b>
 * @code{c}
 *
 * // Global variable used in this example.
 * // This is assumed to be the subscription list in the original SUBSCRIBE packet.
 * MQTTSubscribeInfo_t pSubscribes[ NUMBER_OF_SUBSCRIPTIONS ];
 *
 * // MQTT_GetSubAckStatusCodes is intended to be used from the application
 * // callback that is called by the library in MQTT_ProcessLoop or MQTT_ReceiveLoop.
 * void eventCallback(
 *      MQTTContext_t * pContext,
 *      MQTTPacketInfo_t * pPacketInfo,
 *      MQTTDeserializedInfo_t * pDeserializedInfo
 * )
 * {
 *      MQTTStatus_t status = MQTTSuccess;
 *      uint8_t * pCodes;
 *      size_t numCodes;
 *
 *      if( pPacketInfo->type == MQTT_PACKET_TYPE_SUBACK )
 *      {
 *          status = MQTT_GetSubAckStatusCodes( pPacketInfo, &pCodes, &numCodes );
 *
 *          // Since the pointers to the payload and payload size are not NULL, and
 *          // we use the packet info struct passed to the app callback (verified
 *          // to be valid by the library), this function must return success.
 *          assert( status == MQTTSuccess );
 *          // The server must send a response code for each topic filter in the
 *          // original SUBSCRIBE packet.
 *          assert( numCodes == NUMBER_OF_SUBSCRIPTIONS );
 *
 *          for( int i = 0; i < numCodes; i++ )
 *          {
 *              // The only failure code is 0x80 = MQTTSubAckFailure.
 *              if( pCodes[ i ] == MQTTSubAckFailure )
 *              {
 *                  // The subscription failed, we may want to retry the
 *                  // subscription in pSubscribes[ i ] outside of this callback.
 *              }
 *              else
 *              {
 *                  // The subscription was granted, but the maximum QoS may be
 *                  // lower than what was requested. We can verify the granted QoS.
 *                  if( pSubscribes[ i ].qos != pCodes[ i ] )
 *                  {
 *                      LogWarn( (
 *                          "Requested QoS %u, but granted QoS %u for %s",
 *                          pSubscribes[ i ].qos, pCodes[ i ], pSubscribes[ i ].pTopicFilter
 *                      ) );
 *                  }
 *              }
 *          }
 *      }
 *      // Handle other packet types.
 * }
 * @endcode
 */
/* @[declare_mqtt_getsubackstatuscodes] */
MQTTStatus_t MQTT_GetSubAckStatusCodes( const MQTTPacketInfo_t * pSubackPacket,
                                        uint8_t ** pPayloadStart,
                                        size_t * pPayloadSize );
/* @[declare_mqtt_getsubackstatuscodes] */

/**
 * @brief Error code to string conversion for MQTT statuses.
 *
 * @param[in] status The status to convert to a string.
 *
 * @return The string representation of the status.
 */
/* @[declare_mqtt_status_strerror] */
const char * MQTT_Status_strerror( MQTTStatus_t status );
/* @[declare_mqtt_status_strerror] */

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* ifndef CORE_MQTT_H */
