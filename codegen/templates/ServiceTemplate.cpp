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
cog.outl(f"void {service['class_name']}::advertiseService() {{")
]]]*/
void ServiceTemplateBase::advertiseService() {
//[[[end]]]
    static_assert(sizeof(sd_buffer)>80+sizeof(SERVICE_DESCRIPTION_CBOR), "sd_buffer too small for service description. increase size");
    xbot::comms::Lock lk{sd_buffer_mutex};
    size_t index = 0;
    // Build CBOR payload
    // 0xA3 = object with 4 entries
    sd_buffer[index++] = 0xA3;
    // Key1
    // 0x62 = text(3)
    sd_buffer[index++] = 0x63;
    sd_buffer[index++] = 'n';
    sd_buffer[index++] = 'i';
    sd_buffer[index++] = 'd';
    // 0x84 = array with 4 entries (4x32 = 128 bit; our ID length)
    sd_buffer[index++] = 0x84;

    uint8_t id[16];
    if(!getNodeId(id, 16)) {
        ULOG_ARG_ERROR(&service_id_, "Error fetching node ID");
        return;
    }
    for(size_t i = 0; i < sizeof(id); i+=4) {
        // 0x1A == 32 bit unsigned, positive
        sd_buffer[index++] = 0x1A;
        sd_buffer[index++] = id[i+0];
        sd_buffer[index++] = id[i+1];
        sd_buffer[index++] = id[i+2];
        sd_buffer[index++] = id[i+3];
    }

    // Key2
    // 0x62 = text(3)
    sd_buffer[index++] = 0x63;
    sd_buffer[index++] = 's';
    sd_buffer[index++] = 'i';
    sd_buffer[index++] = 'd';

    // 0x19 == 16 bit unsigned, positive
    sd_buffer[index++] = 0x19;
    sd_buffer[index++] = (service_id_>>8) & 0xFF;
    sd_buffer[index++] = service_id_ & 0xFF;


    // Key2
    // 0x68 = text(8)
    sd_buffer[index++] = 0x68;
    sd_buffer[index++] = 'e';
    sd_buffer[index++] = 'n';
    sd_buffer[index++] = 'd';
    sd_buffer[index++] = 'p';
    sd_buffer[index++] = 'o';
    sd_buffer[index++] = 'i';
    sd_buffer[index++] = 'n';
    sd_buffer[index++] = 't';

    // Get the IP address
    char address[16]{};
    uint16_t port = 0;

    if(!xbot::comms::socketGetEndpoint(udp_socket_, address, sizeof(address), &port)) {
        ULOG_ARG_ERROR(&service_id_, "Error fetching socket address");
        return;
    }

    size_t len = strlen(address);
    if(len >= 16) {
        ULOG_ARG_ERROR(&service_id_, "Got invalid address");
        return;
    }
    // Object with 2 entries (ip, port)
    sd_buffer[index++] = 0xA2;
    // text(2) = "ip"
    sd_buffer[index++] = 0x62;
    sd_buffer[index++] = 'i';
    sd_buffer[index++] = 'p';
    sd_buffer[index++] = 0x60 + len;
    strncpy(reinterpret_cast<char*>(sd_buffer+index), address, len);
    index += len;
    sd_buffer[index++] = 0x64;
    sd_buffer[index++] = 'p';
    sd_buffer[index++] = 'o';
    sd_buffer[index++] = 'r';
    sd_buffer[index++] = 't';
    // 0x19 == 16 bit unsigned, positive
    sd_buffer[index++] = 0x19;
    sd_buffer[index++] = (port>>8) & 0xFF;
    sd_buffer[index++] = port & 0xFF;

    // Key3
    // 0x64 = text(4)
    sd_buffer[index++] = 0x64;
    sd_buffer[index++] = 'd';
    sd_buffer[index++] = 'e';
    sd_buffer[index++] = 's';
    sd_buffer[index++] = 'c';

    memcpy(sd_buffer+index, SERVICE_DESCRIPTION_CBOR, sizeof(SERVICE_DESCRIPTION_CBOR));
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
    header.timestamp = getTimeMicros();

    // Reset reboot on rollover
    if(sd_sequence_==0) {
        reboot = false;
    }

    xbot::comms::PacketPtr ptr = xbot::comms::allocatePacket();
    packetAppendData(ptr, &header, sizeof(header));
    packetAppendData(ptr, sd_buffer, header.payload_size);
    socketTransmitPacket(udp_socket_, ptr, xbot::comms::config::sd_multicast_address, xbot::comms::config::multicast_port);
}

/*[[[cog
# Generate send function implementations.
for output in service["outputs"]:
    if output['is_array']:
        cog.outl(f"bool {service['class_name']}::{output['method_name']}(const {output['type']}* data, uint32_t length) {{")
        cog.outl(f"    return SendData({output['id']}, data, length*sizeof({output['type']}));")
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
