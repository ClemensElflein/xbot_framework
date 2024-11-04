// @formatter:off
// clang-format off
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
#include <xbot-service/Lock.hpp>
#include <xbot-service/portable/system.hpp>
#include <xbot-service/Io.hpp>


/*[[[cog
cog.outl(f"constexpr unsigned char {service['class_name']}::SERVICE_DESCRIPTION_CBOR[];")
]]]*/
//[[[end]]]


/*[[[cog
cog.outl(f"bool {service['class_name']}::handleData(uint16_t target_id, const void *payload, size_t length) {{")
]]]*/
bool ServiceTemplateBase::handleData(uint16_t target_id, const void *payload, size_t length) {
//[[[end]]]

        // Call the callback for this input
        switch (target_id) {
            /*[[[cog
            for i in service['inputs']:
                cog.outl(f"case {i['id']}:");
                if i['is_array']:
                    cog.outl(f"if(length % sizeof({i['type']}) != 0) {{");
                    cog.outl("    ULOG_ARG_ERROR(&service_id_, \"Invalid data size\");");
                    cog.outl("    return false;");
                    cog.outl("}");
                    cog.outl(f"return {i['callback_name']}(static_cast<const {i['type']}*>(payload), length/sizeof({i['type']}));");
                else:
                    if i['custom_decoder_code']:
                        cog.outl(i['custom_decoder_code'])
                    else:
                        cog.outl(f"if(length != sizeof({i['type']})) {{");
                        cog.outl("    ULOG_ARG_ERROR(&service_id_, \"Invalid data size\");");
                        cog.outl("    return false;");
                        cog.outl("}");
                        cog.outl(f"return {i['callback_name']}(*static_cast<const {i['type']}*>(payload));");
            ]]]*/
            case 0:
            if(length % sizeof(char) != 0) {
                ULOG_ARG_ERROR(&service_id_, "Invalid data size");
                return false;
            }
            return OnExampleInput1Changed(static_cast<const char*>(payload), length/sizeof(char));
            case 1:
            if(length != sizeof(uint32_t)) {
                ULOG_ARG_ERROR(&service_id_, "Invalid data size");
                return false;
            }
            return OnExampleInput2Changed(*static_cast<const uint32_t*>(payload));
            //[[[end]]]
            default:
                return false;
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
    // 0xA4 = object with 3 entries
    scratch_buffer[index++] = 0xA3;
    // Key1
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

    if(!xbot::service::Io::getEndpoint(address, sizeof(address), &port)) {
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


    xbot::datatypes::XbotHeader header{};
    if(reboot) {
        header.flags = 1;
    } else {
        header.flags = 0;
    }
    header.message_type = xbot::datatypes::MessageType::SERVICE_ADVERTISEMENT;
    header.payload_size = index;
    header.protocol_version = 1;
    header.arg1 = 0;
    header.arg2 = 0;
    header.sequence_no = sd_sequence_++;
    header.timestamp = xbot::service::system::getTimeMicros();

    // Reset reboot on rollover
    if(sd_sequence_==0) {
        reboot = false;
    }

    xbot::service::packet::PacketPtr ptr = xbot::service::packet::allocatePacket();
    xbot::service::packet::packetAppendData(ptr, &header, sizeof(header));
    xbot::service::packet::packetAppendData(ptr, scratch_buffer, header.payload_size);
    return xbot::service::Io::transmitPacket(ptr, xbot::config::sd_multicast_address, xbot::config::multicast_port);
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

/*[[[cog
    # Generate configured check
    cog.outl(f"bool {service['class_name']}::isConfigured() {{")
    for register in service["registers"]:
        cog.outl(f"if(!this->{register['name']}.valid) {{return false;}}")
    cog.outl("return true;")
    cog.outl("}")
]]]*/
bool ServiceTemplateBase::isConfigured() {
if(!this->Register1.valid) {return false;}
if(!this->Register2.valid) {return false;}
return true;
}
//[[[end]]]

/*[[[cog
    # Generate config reset
    cog.outl(f"void {service['class_name']}::clearConfiguration() {{")
    for register in service["registers"]:
        cog.outl(f"this->{register['name']}.valid = false;")
    cog.outl("}")
]]]*/
void ServiceTemplateBase::clearConfiguration() {
this->Register1.valid = false;
this->Register2.valid = false;
}
//[[[end]]]


/*[[[cog
cog.outl(f"bool {service['class_name']}::setRegister(uint16_t target_id, const void *payload, size_t length) {{")
]]]*/
bool ServiceTemplateBase::setRegister(uint16_t target_id, const void *payload, size_t length) {
  //[[[end]]]


  // Call the callback for this input
  switch (target_id) {
    /*[[[cog
    for r in service['registers']:
        cog.outl(f"case {r['id']}:");
        if r['is_array']:
            cog.outl(f"if(length % sizeof({r['type']}) != 0 || length > sizeof({r['name']}.value)) {{");
            cog.outl("    ULOG_ARG_ERROR(&service_id_, \"Invalid data size\");");
            cog.outl("    return false;");
            cog.outl("}");
            cog.outl(f"{r['name']}.length = length/sizeof({r['type']});")
            cog.outl(f"memcpy(&{r['name']}.value, payload, length);")
            cog.outl(f"{r['name']}.valid = true;")
            cog.outl("return true;")
        else:
            cog.outl(f"if(length != sizeof({r['name']}.value)) {{");
            cog.outl("    ULOG_ARG_ERROR(&service_id_, \"Invalid data size\");");
            cog.outl("    return false;");
            cog.outl("}");
            cog.outl(f"memcpy(&{r['name']}.value, payload, length);")
            cog.outl(f"{r['name']}.valid = true;")
            cog.outl("return true;")
    ]]]*/
    case 0:
    if(length % sizeof(char) != 0 || length > sizeof(Register1.value)) {
        ULOG_ARG_ERROR(&service_id_, "Invalid data size");
        return false;
    }
    Register1.length = length/sizeof(char);
    memcpy(&Register1.value, payload, length);
    Register1.valid = true;
    return true;
    case 1:
    if(length != sizeof(Register2.value)) {
        ULOG_ARG_ERROR(&service_id_, "Invalid data size");
        return false;
    }
    memcpy(&Register2.value, payload, length);
    Register2.valid = true;
    return true;
    //[[[end]]]
    default:
      return false;
  }
  return false;
}