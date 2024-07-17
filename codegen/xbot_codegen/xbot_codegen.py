import json
import cbor2

# Supported types for raw encoding
# we can also encode arrays of these basic types.
raw_encoding_valid_types = [
    "char",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "float",
    "double",
]


# Convert a binary string to a value we can use in our header file.
def binary2c_array(data):
    result = "{\n"
    # Spread across lines, so that we don't have one huge line
    for line in range(0, len(data), 8):
        result += "  "
        for idx, b in enumerate(data[line:line + 8], start=line):
            result += f"0x{b:02X}"
            if idx < (len(data) - 1):
                result += ", "
        result += "\n"
    result += "};"
    return result


# Convert names to CamelCase for use in function names.
def toCamelCase(name):
    return ''.join(x for x in name if not x.isspace())


def loadService(path: str) -> dict:
    # Fetch the service definition
    with open(path) as f:
        json_service = json.load(f)

    # Build the dict for code generation.
    service = {
        "class_name": toCamelCase(json_service["type"]) + "Base",
        "service_json": json.dumps(json_service, indent=2),
        "service_cbor": cbor2.dumps(json_service)
    }

    # Transform the input definitions
    additional_includes = []
    inputs = []
    for json_input in json_service["inputs"]:
        # Convert to valid C++ function name
        input_name = toCamelCase(json_input['name'])
        input_id = int(json_input['id'])
        callback_name = f"On{input_name}Changed"
        custom_decoder_code = None
        # Handle array types (type[length])
        if "[" in json_input["type"] and "]" in json_input["type"]:
            # Split the type definition at the [, validate and get max length
            type, _, rest = json_input["type"].rpartition("[")
            # Rest needs to end with "]", it needs to be something like 123]
            if not rest.endswith("]") or type not in raw_encoding_valid_types:
                raise Exception(f"Illegal data type: {type}!")
            max_length = int(rest.replace("]", ""))
            input = {
                "id": input_id,
                "name": input_name,
                "type": type,
                "is_array": True,
                "max_length": max_length,
                "callback_name": callback_name
            }
        else:
            # Not an array type
            type = json_input["type"]
            if type not in raw_encoding_valid_types:
                raise Exception(f"Illegal data type: {type}!")
            if json_input.get("encoding") == "zcbor":
                # Add additional include for the decoder
                include_name = f"<{json_input['type']}_decode.h>"
                if include_name not in additional_includes:
                    additional_includes.append(include_name)
                # Generate the decoding code
                custom_decoder_code = ("{\n"
                                       f"   struct {json_input['type']} result;\n"
                                       f"   if(cbor_decode_{json_input['type']}(static_cast<const uint8_t*>(payload), header->payload_size, &result, nullptr) == ZCBOR_SUCCESS) {{\n"
                                       f"       return {callback_name}(result);\n"
                                       "    } else {\n"
                                       "        return false;\n"
                                       "    }\n"
                                       "}"
                                       )
            input = {
                "id": input_id,
                "name": input_name,
                "type": type,
                "is_array": False,
                "callback_name": callback_name,
                "custom_decoder_code": custom_decoder_code
            }

        inputs.append(input)
    service["inputs"] = inputs

    # Transform the output definitions
    outputs = []
    for json_output in json_service["outputs"]:
        # Convert to valid C++ function name
        output_name = toCamelCase(json_output['name'])
        output_id = int(json_output['id'])
        method_name = f"Send{output_name}"
        custom_encoder_code = None
        # Handle array types (type[length])
        if "[" in json_output["type"] and "]" in json_output["type"]:
            # Split the type definition at the [, validate and get max length
            type, _, rest = json_output["type"].rpartition("[")
            # Rest needs to end with "]", it needs to be something like 123]
            if not rest.endswith("]") or type not in raw_encoding_valid_types:
                raise Exception(f"Illegal data type: {type}!")
            max_length = int(rest.replace("]", ""))
            output = {
                "id": output_id,
                "name": output_name,
                "type": type,
                "is_array": True,
                "max_length": max_length,
                "method_name": method_name
            }
        else:
            # Not an array type
            if json_output.get("encoding") == "zcbor":
                # Add additional include for the decoder
                include_name = f"<{json_output['type']}_encode.h>"
                if include_name not in additional_includes:
                    additional_includes.append(include_name)
                # Generate the decoding code
                custom_encoder_code = ("{\n"
                                       f"   size_t encoded_len = 0;\n"
                                       f"   if(cbor_encode_{json_output['type']}(scratch_buffer, sizeof(scratch_buffer), &data, &encoded_len) == ZCBOR_SUCCESS) {{\n"
                                       f"       return SendData({output_id}, scratch_buffer, encoded_len);\n"
                                       "    } else {\n"
                                       "        return false;\n"
                                       "    }\n"
                                       "}"
                                       )
            type = json_output["type"]
            if type not in raw_encoding_valid_types:
                raise Exception(f"Illegal data type: {type}!")
            output = {
                "id": output_id,
                "name": output_name,
                "type": type,
                "is_array": False,
                "method_name": method_name,
                "custom_encoder_code": custom_encoder_code
            }

        outputs.append(output)
    service["outputs"] = outputs
    service["additional_includes"] = additional_includes
    return service
