//
// Created by clemens on 4/23/24.
//

/*[[[cog
import cog
from xbot_codegen import toCamelCase, loadService

service = loadService(service_file)

cog.outl(f'#include "{service["class_name"]}.hpp"')

]]]*/
#include "ServiceTemplateBase.hpp"
//[[[end]]]
#include <cstring>
#include <ulog.h>
#include "Lock.hpp"
#include "portable/system.hpp"

/*[[[cog
cog.outl(f"bool {service['class_name']}::handlePacket(const xbot::comms::datatypes::XbotHeader *header, const void *payload) {{")
]]]*/
bool ServiceTemplateBase::handlePacket(const xbot::comms::datatypes::XbotHeader *header, const void *payload) {
//[[[end]]]
    if (header->message_type == xbot::comms::datatypes::MessageType::DATA) {
        // Call the callback for this input
        switch (header->arg2) {
            /*[[[cog
            for i in service['inputs']:
                cog.outl(f"case {i['id']}:");
                if i['is_array']:
                    cog.outl(f"if(header->payload_size % sizeof({i['type']}) != 0) {{");
                    cog.outl("    ULOG_ARG_ERROR(&service_id_, \"Invalid data size\");");
                    cog.outl("    return false;");
                    cog.outl("}");
                    cog.outl(f"return {i['callback_name']}(static_cast<const {i['type']}*>(payload), header->payload_size/sizeof({i['type']}));");
                else:
                    if i['custom_decoder_code']:
                        cog.outl(i['custom_decoder_code'])
                    else:
                        cog.outl(f"if(header->payload_size != sizeof({i['type']})) {{");
                        cog.outl("    ULOG_ARG_ERROR(&service_id_, \"Invalid data size\");");
                        cog.outl("    return false;");
                        cog.outl("}");
                        cog.outl(f"return {i['callback_name']}(*static_cast<const {i['type']}*>(payload));");
            ]]]*/
            case 0:
            if(header->payload_size % sizeof(char) != 0) {
                ULOG_ARG_ERROR(&service_id_, "Invalid data size");
                return false;
            }
            return OnExampleInput1Changed(static_cast<const char*>(payload), header->payload_size/sizeof(char));
            case 1:
            if(header->payload_size != sizeof(uint32_t)) {
                ULOG_ARG_ERROR(&service_id_, "Invalid data size");
                return false;
            }
            return OnExampleInput2Changed(*static_cast<const uint32_t*>(payload));
            //[[[end]]]
            default:
                return false;
        }
    }
    return false;
}
/*[[[cog
cog.outl(f"bool {service['class_name']}::advertiseService() {{")
]]]*/
bool ServiceTemplateBase::advertiseService() {
//[[[end]]]
    static_assert(sizeof(scratch_buffer)>80+sizeof(SERVICE_DESCRIPTION_CBOR), "scratch_buffer too small for service description. increase size");

    size_t index = 0;
    // Build CBOR payload
    // 0xA4 = object with 4 entries
    scratch_buffer[index++] = 0xA4;
    // Key1
    // 0x62 = text(3)
    scratch_buffer[index++] = 0x63;
    scratch_buffer[index++] = 'n';
    scratch_buffer[index++] = 'i';
    scratch_buffer[index++] = 'd';
    // 0x84 = array with 4 entries (4x32 = 128 bit; our ID length)
    scratch_buffer[index++] = 0x84;

    uint8_t id[16];
    if(!xbot::comms::system::getNodeId(id, 16)) {
        ULOG_ARG_ERROR(&service_id_, "Error fetching node ID");

        return false;
    }
    for(size_t i = 0; i < sizeof(id); i+=4) {
        // 0x1A == 32 bit unsigned, positive
        scratch_buffer[index++] = 0x1A;
        scratch_buffer[index++] = id[i+0];
        scratch_buffer[index++] = id[i+1];
        scratch_buffer[index++] = id[i+2];
        scratch_buffer[index++] = id[i+3];
    }

    // Key2
    // 0x62 = text(3)
    scratch_buffer[index++] = 0x63;
    scratch_buffer[index++] = 's';
    scratch_buffer[index++] = 'i';
    scratch_buffer[index++] = 'd';

    // 0x19 == 16 bit unsigned, positive
    scratch_buffer[index++] = 0x19;
    scratch_buffer[index++] = (service_id_>>8) & 0xFF;
    scratch_buffer[index++] = service_id_ & 0xFF;


    // Key2
    // 0x68 = text(8)
    scratch_buffer[index++] = 0x68;
    scratch_buffer[index++] = 'e';
    scratch_buffer[index++] = 'n';
    scratch_buffer[index++] = 'd';
    scratch_buffer[index++] = 'p';
    scratch_buffer[index++] = 'o';
    scratch_buffer[index++] = 'i';
    scratch_buffer[index++] = 'n';
    scratch_buffer[index++] = 't';

    // Get the IP address
    char address[16]{};
    uint16_t port = 0;

    if(!xbot::comms::sock::getEndpoint(&udp_socket_, address, sizeof(address), &port)) {
        ULOG_ARG_ERROR(&service_id_, "Error fetching socket address");
        return false;
    }

    size_t len = strlen(address);
    if(len >= 16) {
        ULOG_ARG_ERROR(&service_id_, "Got invalid address");
        return false;
    }
    // Object with 2 entries (ip, port)
    scratch_buffer[index++] = 0xA2;
    // text(2) = "ip"
    scratch_buffer[index++] = 0x62;
    scratch_buffer[index++] = 'i';
    scratch_buffer[index++] = 'p';
    scratch_buffer[index++] = 0x60 + len;
    strncpy(reinterpret_cast<char*>(scratch_buffer+index), address, len);
    index += len;
    scratch_buffer[index++] = 0x64;
    scratch_buffer[index++] = 'p';
    scratch_buffer[index++] = 'o';
    scratch_buffer[index++] = 'r';
    scratch_buffer[index++] = 't';
    // 0x19 == 16 bit unsigned, positive
    scratch_buffer[index++] = 0x19;
    scratch_buffer[index++] = (port>>8) & 0xFF;
    scratch_buffer[index++] = port & 0xFF;

    // Key3
    // 0x64 = text(4)
    scratch_buffer[index++] = 0x64;
    scratch_buffer[index++] = 'd';
    scratch_buffer[index++] = 'e';
    scratch_buffer[index++] = 's';
    scratch_buffer[index++] = 'c';

    memcpy(scratch_buffer+index, SERVICE_DESCRIPTION_CBOR, sizeof(SERVICE_DESCRIPTION_CBOR));
    index+=sizeof(SERVICE_DESCRIPTION_CBOR);


    xbot::comms::datatypes::XbotHeader header{};
    if(reboot) {
        header.flags = 1;
    } else {
        header.flags = 0;
    }
    header.message_type = xbot::comms::datatypes::MessageType::SERVICE_ADVERTISEMENT;
    header.payload_size = index;
    header.protocol_version = 1;
    header.arg1 = 0;
    header.arg2 = 0;
    header.sequence_no = sd_sequence_++;
    header.timestamp = xbot::comms::system::getTimeMicros();

    // Reset reboot on rollover
    if(sd_sequence_==0) {
        reboot = false;
    }

    xbot::comms::packet::PacketPtr ptr = xbot::comms::packet::allocatePacket();
    xbot::comms::packet::packetAppendData(ptr, &header, sizeof(header));
    xbot::comms::packet::packetAppendData(ptr, scratch_buffer, header.payload_size);
    return xbot::comms::sock::transmitPacket(&udp_socket_, ptr, xbot::config::sd_multicast_address, xbot::config::multicast_port);
}

/*[[[cog
# Generate send function implementations.
for output in service["outputs"]:
    if output['is_array']:
        cog.outl(f"bool {service['class_name']}::{output['method_name']}(const {output['type']}* data, uint32_t length) {{")
        cog.outl(f"    return SendData({output['id']}, data, length*sizeof({output['type']}));")
        cog.outl("}")
    else:
        if output['custom_encoder_code']:
            cog.outl(f"bool {service['class_name']}::{output['method_name']}(const {output['type']} &data) {{")
            cog.outl(output['custom_encoder_code'])
            cog.outl("}")
        else:
            cog.outl(f"bool {service['class_name']}::{output['method_name']}(const {output['type']} &data) {{")
            cog.outl(f"    return SendData({output['id']}, &data, sizeof({output['type']}));")
            cog.outl("}")
]]]*/
bool ServiceTemplateBase::SendExampleOutput1(const char* data, uint32_t length) {
    return SendData(0, data, length*sizeof(char));
}
bool ServiceTemplateBase::SendExampleOutput2(const uint32_t &data) {
    return SendData(1, &data, sizeof(uint32_t));
}
//[[[end]]]
