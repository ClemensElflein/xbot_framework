//
// Created by clemens on 4/23/24.
//

/*[[[cog
import cog
from xbot_codegen import toCamelCase, loadService

service = loadService(service_file)

cog.outl(f'#include <{service["interface_class_name"]}.hpp>')

]]]*/
#include <ServiceTemplateInterfaceBase.hpp>
//[[[end]]]
#include <cstring>
#include <spdlog/spdlog.h>
/*[[[cog
cog.outl(f"void {service['interface_class_name']}::OnData(const std::string &uid, uint64_t timestamp, uint16_t target_id, const void *payload, size_t length) {{")
]]]*/
void ServiceTemplateInterfaceBase::OnData(const std::string &uid, uint64_t timestamp, uint16_t target_id, const void *payload, size_t length) {
//[[[end]]]
    // Call the callback for this input
        switch (target_id) {
            /*[[[cog
            for o in service['outputs']:
                cog.outl(f"case {o['id']}:");
                if o['is_array']:
                    cog.outl(f"if(length % sizeof({o['type']}) != 0) {{");
                    cog.outl("    spdlog::error(\"Invalid data size\");");
                    cog.outl("    return;");
                    cog.outl("}");
                    cog.outl(f"{o['callback_name']}(static_cast<const {o['type']}*>(payload), length/sizeof({o['type']}));");
                else:
                    cog.outl(f"if(length != sizeof({o['type']})) {{");
                    cog.outl("    spdlog::error(\"Invalid data size\");");
                    cog.outl("    return;");
                    cog.outl("}");
                    cog.outl(f"{o['callback_name']}(*static_cast<const {o['type']}*>(payload));");
                cog.outl(f"break;");
            ]]]*/
            case 0:
            if(length % sizeof(char) != 0) {
                spdlog::error("Invalid data size");
                return;
            }
            OnExampleOutput1Changed(static_cast<const char*>(payload), length/sizeof(char));
            break;
            case 1:
            if(length != sizeof(uint32_t)) {
                spdlog::error("Invalid data size");
                return;
            }
            OnExampleOutput2Changed(*static_cast<const uint32_t*>(payload));
            break;
            //[[[end]]]
            default:
                return;
        }
        return;
}


/*[[[cog
# Generate send function implementations.
for input in service["inputs"]:
    if input['is_array']:
        cog.outl(f"bool {service['interface_class_name']}::{input['method_name']}(const {input['type']}* data, uint32_t length) {{")
        cog.outl(f"    return SendData({input['id']}, data, length*sizeof({input['type']}));")
        cog.outl("}")
    else:
        cog.outl(f"bool {service['interface_class_name']}::{input['method_name']}(const {input['type']} &data) {{")
        cog.outl(f"    return SendData({input['id']}, &data, sizeof({input['type']}));")
        cog.outl("}")
]]]*/
bool ServiceTemplateInterfaceBase::SendExampleInput1(const char* data, uint32_t length) {
    return SendData(0, data, length*sizeof(char));
}
bool ServiceTemplateInterfaceBase::SendExampleInput2(const uint32_t &data) {
    return SendData(1, &data, sizeof(uint32_t));
}
//[[[end]]]

/*[[[cog
cog.outl(f"void {service['interface_class_name']}::OnServiceConnected(const std::string &uid) {{}};")
cog.outl(f"void {service['interface_class_name']}::OnTransactionStart(uint64_t timestamp) {{}};")
cog.outl(f"void {service['interface_class_name']}::OnTransactionEnd() {{}};")
cog.outl(f"void {service['interface_class_name']}::OnServiceDisconnected(const std::string &uid) {{}};")
]]]*/
void ServiceTemplateInterfaceBase::OnServiceConnected(const std::string &uid) {};
void ServiceTemplateInterfaceBase::OnTransactionStart(uint64_t timestamp) {};
void ServiceTemplateInterfaceBase::OnTransactionEnd() {};
void ServiceTemplateInterfaceBase::OnServiceDisconnected(const std::string &uid) {};
//[[[end]]]