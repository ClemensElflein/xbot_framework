//
// Created by clemens on 4/23/24.
//

/*[[[cog
import cog
import xbot_codegen

service = xbot_codegen.loadService(service_file)

#Generate include guard
cog.outl(f"#ifndef {service['class_name'].upper()}_HPP")
cog.outl(f"#define {service['class_name'].upper()}_HPP")

]]]*/
#ifndef SERVICETEMPLATEBASE_HPP
#define SERVICETEMPLATEBASE_HPP
//[[[end]]]

#include <xbot-service/Service.hpp>

/*[[[cog
for include in service['additional_includes']:
    cog.outl(f"#include {include}")
]]]*/
//[[[end]]]

/*[[[cog
cog.outl(f"class {service['class_name']} : public xbot::service::Service {{")
]]]*/
class ServiceTemplateBase : public xbot::service::Service {
//[[[end]]]
public:
    /*[[[cog
    cog.outl("#ifdef XBOT_ENABLE_STATIC_STACK")
    cog.outl(f"explicit {service['class_name']}(uint16_t service_id, uint32_t tick_rate_micros, void* stack, size_t stack_size)")
    cog.outl("#else")
    cog.outl(f"explicit {service['class_name']}(uint16_t service_id, uint32_t tick_rate_micros)")
    cog.outl("#endif")
    ]]]*/
    #ifdef XBOT_ENABLE_STATIC_STACK
    explicit ServiceTemplateBase(uint16_t service_id, uint32_t tick_rate_micros, void* stack, size_t stack_size)
    #else
    explicit ServiceTemplateBase(uint16_t service_id, uint32_t tick_rate_micros)
    #endif
    //[[[end]]]
    #ifdef XBOT_ENABLE_STATIC_STACK
        : Service(service_id, tick_rate_micros, stack, stack_size) {
    #else
        : Service(service_id, tick_rate_micros, nullptr, 0) {
    #endif
    }

    /*[[[cog
    cog.outl(f"\u002f*\n{service['service_json']}\n*\u002f")
    cog.out("static constexpr unsigned char SERVICE_DESCRIPTION_CBOR[] = ");
    cog.outl(xbot_codegen.binary2c_array(service["service_cbor"]));
    ]]]*/
    /*
    {
      "type": "ServiceTemplate",
      "version": 1,
      "inputs": [
        {
          "id": 0,
          "name": "ExampleInput1",
          "type": "char[100]"
        },
        {
          "id": 1,
          "name": "ExampleInput2",
          "type": "uint32_t"
        }
      ],
      "outputs": [
        {
          "id": 0,
          "name": "ExampleOutput1",
          "type": "char[100]"
        },
        {
          "id": 1,
          "name": "ExampleOutput2",
          "type": "uint32_t"
        }
      ],
      "registers": [
        {
          "id": 0,
          "name": "Register1",
          "type": "char[42]"
        },
        {
          "id": 1,
          "name": "Register2",
          "type": "uint32_t"
        }
      ]
    }
    */
    static constexpr unsigned char SERVICE_DESCRIPTION_CBOR[] = {
      0xA5, 0x64, 0x74, 0x79, 0x70, 0x65, 0x6F, 0x53, 
      0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x54, 0x65, 
      0x6D, 0x70, 0x6C, 0x61, 0x74, 0x65, 0x67, 0x76, 
      0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x01, 0x66, 
      0x69, 0x6E, 0x70, 0x75, 0x74, 0x73, 0x82, 0xA3, 
      0x62, 0x69, 0x64, 0x00, 0x64, 0x6E, 0x61, 0x6D, 
      0x65, 0x6D, 0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 
      0x65, 0x49, 0x6E, 0x70, 0x75, 0x74, 0x31, 0x64, 
      0x74, 0x79, 0x70, 0x65, 0x69, 0x63, 0x68, 0x61, 
      0x72, 0x5B, 0x31, 0x30, 0x30, 0x5D, 0xA3, 0x62, 
      0x69, 0x64, 0x01, 0x64, 0x6E, 0x61, 0x6D, 0x65, 
      0x6D, 0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 
      0x49, 0x6E, 0x70, 0x75, 0x74, 0x32, 0x64, 0x74, 
      0x79, 0x70, 0x65, 0x68, 0x75, 0x69, 0x6E, 0x74, 
      0x33, 0x32, 0x5F, 0x74, 0x67, 0x6F, 0x75, 0x74, 
      0x70, 0x75, 0x74, 0x73, 0x82, 0xA3, 0x62, 0x69, 
      0x64, 0x00, 0x64, 0x6E, 0x61, 0x6D, 0x65, 0x6E, 
      0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x4F, 
      0x75, 0x74, 0x70, 0x75, 0x74, 0x31, 0x64, 0x74, 
      0x79, 0x70, 0x65, 0x69, 0x63, 0x68, 0x61, 0x72, 
      0x5B, 0x31, 0x30, 0x30, 0x5D, 0xA3, 0x62, 0x69, 
      0x64, 0x01, 0x64, 0x6E, 0x61, 0x6D, 0x65, 0x6E, 
      0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x4F, 
      0x75, 0x74, 0x70, 0x75, 0x74, 0x32, 0x64, 0x74, 
      0x79, 0x70, 0x65, 0x68, 0x75, 0x69, 0x6E, 0x74, 
      0x33, 0x32, 0x5F, 0x74, 0x69, 0x72, 0x65, 0x67, 
      0x69, 0x73, 0x74, 0x65, 0x72, 0x73, 0x82, 0xA3, 
      0x62, 0x69, 0x64, 0x00, 0x64, 0x6E, 0x61, 0x6D, 
      0x65, 0x69, 0x52, 0x65, 0x67, 0x69, 0x73, 0x74, 
      0x65, 0x72, 0x31, 0x64, 0x74, 0x79, 0x70, 0x65, 
      0x68, 0x63, 0x68, 0x61, 0x72, 0x5B, 0x34, 0x32, 
      0x5D, 0xA3, 0x62, 0x69, 0x64, 0x01, 0x64, 0x6E, 
      0x61, 0x6D, 0x65, 0x69, 0x52, 0x65, 0x67, 0x69, 
      0x73, 0x74, 0x65, 0x72, 0x32, 0x64, 0x74, 0x79, 
      0x70, 0x65, 0x68, 0x75, 0x69, 0x6E, 0x74, 0x33, 
      0x32, 0x5F, 0x74
    };
    //[[[end]]]


private:
    uint32_t sd_sequence_ = 0;
    bool reboot = true;
    bool handleData(uint16_t target_id, const void *payload, size_t length) override final;
    bool advertiseService() override final;
    bool isConfigured() override final;
    void clearConfiguration() override final;
    bool setRegister(uint16_t target_id, const void *payload,
                          size_t length)override final;

protected:
    /*[[[cog
    # Generate callback functions for each input.
    for input in service["inputs"]:
        if input['is_array']:
            cog.outl(f"virtual bool {input['callback_name']}(const {input['type']}* new_value, uint32_t length) = 0;")
        else:
            cog.outl(f"virtual bool {input['callback_name']}(const {input['type']} &new_value) = 0;")
    ]]]*/
    virtual bool OnExampleInput1Changed(const char* new_value, uint32_t length) = 0;
    virtual bool OnExampleInput2Changed(const uint32_t &new_value) = 0;
    //[[[end]]]

    /*[[[cog
    # Generate send functions for each output.
    for output in service["outputs"]:
        if output['is_array']:
            cog.outl(f"bool {output['method_name']}(const {output['type']}* data, uint32_t length);")
        else:
            cog.outl(f"bool {output['method_name']}(const {output['type']} &data);")
    ]]]*/
    bool SendExampleOutput1(const char* data, uint32_t length);
    bool SendExampleOutput2(const uint32_t &data);
    //[[[end]]]
    
    /*[[[cog
    # Generate register struct

    for register in service["registers"]:
      cog.outl("struct {");
      if register['is_array']:
          cog.outl(f"{register['type']} value[{register ['max_length']}];")
          cog.outl("size_t length = 0;")
      else:
          cog.outl(f"{register['type']} value;")
      cog.outl("bool valid = false;")
      cog.outl(f"}} {register['name']};");
    ]]]*/
    struct {
    char value[42];
    size_t length = 0;
    bool valid = false;
    } Register1;
    struct {
    uint32_t value;
    bool valid = false;
    } Register2;
    //[[[end]]]
};



#endif

