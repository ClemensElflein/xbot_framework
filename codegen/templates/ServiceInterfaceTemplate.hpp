// @formatter:off
// clang-format off
//
// Created by clemens on 4/23/24.
//

/*[[[cog
import cog
import xbot_codegen

service = xbot_codegen.loadService(service_file)

#Generate include guard
cog.outl(f"#ifndef {service['interface_class_name'].upper()}_HPP")
cog.outl(f"#define {service['interface_class_name'].upper()}_HPP")

]]]*/
#ifndef SERVICETEMPLATEINTERFACEBASE_HPP
#define SERVICETEMPLATEINTERFACEBASE_HPP
//[[[end]]]

#include <xbot-service-interface/ServiceInterfaceBase.hpp>
#include <xbot-service-interface/XbotServiceInterface.hpp>

/*[[[cog
for include in service['additional_includes']:
    cog.outl(f"#include {include}")
]]]*/
//[[[end]]]

/*[[[cog
cog.outl(f"class {service['interface_class_name']} : public xbot::serviceif::ServiceInterfaceBase {{")
]]]*/
class ServiceTemplateInterfaceBase : public xbot::serviceif::ServiceInterfaceBase {
//[[[end]]]
public:
    /*[[[cog
    cog.outl(f"explicit {service['interface_class_name']}(uint16_t service_id, xbot::serviceif::Context ctx) : ServiceInterfaceBase(service_id, \"{service['type']}\", {service['version']}, ctx) {{}}")
    ]]]*/
    explicit ServiceTemplateInterfaceBase(uint16_t service_id, Context ctx) : ServiceInterfaceBase(service_id, "ServiceTemplate", 1, ctx) {}
    //[[[end]]]


    /*[[[cog
    # Generate send functions for each input.
    for input in service["inputs"]:
        if input['is_array']:
            cog.outl(f"bool {input['method_name']}(const {input['type']}* data, uint32_t length);")
        else:
            cog.outl(f"bool {input['method_name']}(const {input['type']} &data);")
    ]]]*/
    bool SendExampleInput1(const char* data, uint32_t length);
    bool SendExampleInput2(const uint32_t &data);
    //[[[end]]]

    /*[[[cog
    # Generate send functions for each register.
    for register in service["registers"]:
        if register['is_array']:
            cog.outl(f"bool {register['method_name']}(const {register['type']}* data, uint32_t length);")
        else:
            cog.outl(f"bool {register['method_name']}(const {register['type']} &data);")
    ]]]*/
    bool SetRegisterRegister1(const char* data, uint32_t length);
    bool SetRegisterRegister2(const uint32_t &data);
    //[[[end]]]

protected:
    /*[[[cog
    # Generate callback functions for each service output.
    for output in service["outputs"]:
        if output['is_array']:
            cog.outl(f"virtual void {output['callback_name']}(const {output['type']}* new_value, uint32_t length) = 0;")
        else:
            cog.outl(f"virtual void {output['callback_name']}(const {output['type']} &new_value) = 0;")
    ]]]*/
    virtual void OnExampleOutput1Changed(const char* new_value, uint32_t length) = 0;
    virtual void OnExampleOutput2Changed(const uint32_t &new_value) = 0;
    //[[[end]]]



private:
	void OnData(const std::string &uid, uint64_t timestamp, uint16_t target_id, const void *payload, size_t buflen) final;
  void OnServiceConnected(const std::string &uid) override;
  void OnTransactionStart(uint64_t timestamp) override;
  void OnTransactionEnd() override;
  void OnServiceDisconnected(const std::string &uid) override;
};



#endif

