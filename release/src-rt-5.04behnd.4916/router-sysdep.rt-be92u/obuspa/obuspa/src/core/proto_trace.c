/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file proto_trace.c
 *
 * Functions for pretty printing a USP message in protobuf debug format
 *
 */

#include <string.h>
#include <protobuf-c/protobuf-c.h>
#include <inttypes.h>  // For PRIu64

#include "common_defs.h"
#include "proto_trace.h"

// Number of spaces to use for each indentation block when printing messages in protobuf format
#define INDENTATION 2

//------------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void PrintProtobufCMessageRecursive(ProtobufCMessage *msg, int indent);
void PrintProtobufFieldRecursive(const ProtobufCFieldDescriptor *fields, void *p_value, int indent);

/*********************************************************************//**
**
** PROTO_TRACE_ProtobufMessage
**
** Prints a USP protobuf message structure in Protobuf debug format
**
** \param   base - pointer to structure containing USP protocol buffer message to print (eg &usp->base)
**
** \return  None
**
**************************************************************************/
void PROTO_TRACE_ProtobufMessage(ProtobufCMessage *base)
{
    // Exit if protocol trace not enabled
    if (enable_protocol_trace==false)
    {
        return;
    }

    PrintProtobufCMessageRecursive(base, 0);
    USP_PROTOCOL("\n");
}

/*********************************************************************//**
**
** PrintProtobufCMessageRecursive
**
** Prints a generic protocol buffer C message structure in Protobuf debug format
** NOTE: This function is called recursively
**
** \param   msg - pointer to structure containing protocol buffer message to print
** \param   indent - indentation level (used by recursion to format the printout)
**
** \return  None
**
**************************************************************************/
void PrintProtobufCMessageRecursive(ProtobufCMessage *msg, int indent)
{
    int i, j;
    unsigned n_fields;
    unsigned offset;
    unsigned quantifier_offset;
    const ProtobufCFieldDescriptor *fields;
    void *p_value;
    void *p_quantifier;
    int quantifier;
    void **p_array;

    // Exit if structure contains unexpected NULL pointers. Should not be necessary, but makes code safer.
    if ((msg == NULL) || (msg->descriptor == NULL))
    {
        return;
    }

    n_fields = msg->descriptor->n_fields;
    for (i=0; i<n_fields; i++)
    {
        fields = &msg->descriptor->fields[i];
        offset = fields->offset;
        p_value = (void *) (((char *)msg) + offset);

        quantifier_offset = fields->quantifier_offset;
        p_quantifier = (void *) (((char *)msg) + quantifier_offset);
        quantifier = *((int *)p_quantifier);

        // If field is a selector between different messages, then parse only the message which was selected
        if (fields->flags & PROTOBUF_C_FIELD_FLAG_ONEOF)
        {
            for (j=0; j< msg->descriptor->n_fields; j++)
            {
                fields = &msg->descriptor->fields[j];
                if (fields->id == quantifier)
                {
                    PrintProtobufFieldRecursive(fields, p_value, indent);
                    return; // intentional -  the selector fields terminate the list of fields for a message
                }
            }
        }

        switch(fields->label)
        {
            case PROTOBUF_C_LABEL_REPEATED:
                // Print arrays of types
                p_array = *((void ***)p_value);
                for (j=0; j<quantifier; j++)
                {
                    PrintProtobufFieldRecursive(fields, &p_array[j], indent);
                }
                break;

            case PROTOBUF_C_LABEL_OPTIONAL:
                // Only print optional elements, if they were present
                if (quantifier)
                {
                    PrintProtobufFieldRecursive(fields, p_value, indent);
                }
                break;


            default:
            case PROTOBUF_C_LABEL_REQUIRED:
                // Ordinary type, or wrapped message
                PrintProtobufFieldRecursive(fields, p_value, indent);
                break;
        }
    }
}

/*********************************************************************//**
**
** PrintProtobufFieldRecursive
**
** Prints a generic protocol buffer C field
** NOTE: This function is called recursively, and always via PrintProtobufCMessageRecursive()
**
** \param   fields - pointer to field info to print out
** \param   p_value - pointer to the variable containing the field to printout
** \param   indent - indentation level (used by recursion to format the printout)
**
** \return  None
**
**************************************************************************/
void PrintProtobufFieldRecursive(const ProtobufCFieldDescriptor *fields, void *p_value, int indent)
{
    int i;
    int index;
    ProtobufCEnumDescriptor *enum_desc;
    ProtobufCEnumValue *ev;
    char *str;

    // Exit if structure contains unexpected NULL pointers. Should not be necessary, but makes code safer.
    if ((fields == NULL) || (p_value == NULL))
    {
        return;
    }

    switch(fields->type)
    {
        #define INDENT_CHAR ""
        case PROTOBUF_C_TYPE_INT32:      /**< int32 */
        case PROTOBUF_C_TYPE_SINT32:     /**< signed int32 */
        case PROTOBUF_C_TYPE_SFIXED32:   /**< signed int32 (4 bytes) */
            USP_PROTOCOL("%*s%s: %d", indent, INDENT_CHAR, fields->name, *((int32_t *)p_value));
            break;

        case PROTOBUF_C_TYPE_INT64:      /**< int64 */
        case PROTOBUF_C_TYPE_SINT64:     /**< signed int64 */
        case PROTOBUF_C_TYPE_SFIXED64:   /**< signed int64 (8 bytes) */
            USP_PROTOCOL("%*s%s: %"PRId64, indent, INDENT_CHAR, fields->name, *((int64_t *)p_value));
            break;

        case PROTOBUF_C_TYPE_UINT32:     /**< unsigned int32 */
        case PROTOBUF_C_TYPE_FIXED32:    /**< unsigned int32 (4 bytes) */
            USP_PROTOCOL("%*s%s: %u", indent, INDENT_CHAR, fields->name, *((uint32_t *)p_value));
            break;

        case PROTOBUF_C_TYPE_UINT64:     /**< unsigned int64 */
        case PROTOBUF_C_TYPE_FIXED64:    /**< unsigned int64 (8 bytes) */
            USP_PROTOCOL("%*s%s: %"PRIu64, indent, INDENT_CHAR, fields->name, *((uint64_t *)p_value));
            break;

        case PROTOBUF_C_TYPE_FLOAT:      /**< float */
            USP_PROTOCOL("%*s%s: %f", indent, INDENT_CHAR, fields->name, *((float *)p_value));
            break;

        case PROTOBUF_C_TYPE_DOUBLE:     /**< double */
            USP_PROTOCOL("%*s%s: %lf", indent, INDENT_CHAR, fields->name, *((double *)p_value));
            break;

        case PROTOBUF_C_TYPE_BOOL:       /**< boolean */
            USP_PROTOCOL("%*s%s: %s", indent, INDENT_CHAR, fields->name, (*((protobuf_c_boolean *)p_value)) ? "true" : "false");
            break;

        case PROTOBUF_C_TYPE_ENUM:       /**< enumerated type */
            index = *((int *)p_value);
            enum_desc = (ProtobufCEnumDescriptor *) fields->descriptor;
            for (i=0; i < enum_desc->n_values; i++)
            {
                ev = (ProtobufCEnumValue *) &enum_desc->values[i];
                if (ev->value == index)
                {
                    USP_PROTOCOL("%*s%s: %s", indent, INDENT_CHAR, fields->name, ev->name);
                    break;
                }
            }
            break;

        case PROTOBUF_C_TYPE_STRING:     /**< UTF-8 or ASCII string */
            str = *((char **)p_value);
            if (str != NULL)
            {
                USP_PROTOCOL("%*s%s: \"%s\"", indent, INDENT_CHAR, fields->name, str);
            }
            break;

        case PROTOBUF_C_TYPE_BYTES:      /**< arbitrary byte sequence */
            USP_PROTOCOL("%*s%s[%zu]", indent, INDENT_CHAR, fields->name, ((ProtobufCBinaryData *) p_value)->len);
            // We do not print the content out here, only its length. This will get triggered by USP Agent when printing out a USP Record, for the encapsulated USP Message.
            break;

        case PROTOBUF_C_TYPE_MESSAGE:    /**< nested message */
            USP_PROTOCOL("%*s%s {", indent, INDENT_CHAR, fields->name);
            PrintProtobufCMessageRecursive( *((ProtobufCMessage **)p_value), indent+INDENTATION );
            USP_PROTOCOL("%*s}", indent, INDENT_CHAR);
            break;

        default:
            // Unknown field. This should never occur. Just ignore it.
            break;
    }
}


