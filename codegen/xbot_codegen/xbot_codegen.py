import json
import cbor2


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
    inputs = []
    for json_input in json_service["inputs"]:
        # Convert to valid C++ function name
        input_name = toCamelCase(json_input['name'])
        input_id = int(json_input['id'])
        # Handle array types (type[length])
        if "[" in json_input["type"] and "]" in json_input["type"]:
            # Split the type definition at the [, validate and get max length
            type, _, rest = json_input["type"].rpartition("[")
            # Rest needs to end with "]", it needs to be something like 123]
            if not rest.endswith("]"):
                raise Exception(f"Illegal data type: {type}!")
            max_length = int(rest.replace("]", ""))
            input = {
                "id": input_id,
                "name": input_name,
                "type": type,
                "is_array": True,
                "max_length": max_length,
                "callback_name": f"On{input_name}Changed"
            }
        else:
            # Not an array type
            type = json_input["type"]
            input = {
                "id": input_id,
                "name": input_name,
                "type": type,
                "is_array": False,
                "callback_name": f"On{input_name}Changed"
            }

        inputs.append(input)
    service["inputs"] = inputs

    # Transform the output definitions
    outputs = []
    for json_output in json_service["outputs"]:
        # Convert to valid C++ function name
        output_name = toCamelCase(json_output['name'])
        output_id = int(json_output['id'])
        # Handle array types (type[length])
        if "[" in json_output["type"] and "]" in json_output["type"]:
            # Split the type definition at the [, validate and get max length
            type, _, rest = json_output["type"].rpartition("[")
            # Rest needs to end with "]", it needs to be something like 123]
            if not rest.endswith("]"):
                raise Exception(f"Illegal data type: {type}!")
            max_length = int(rest.replace("]", ""))
            output = {
                "id": output_id,
                "name": output_name,
                "type": type,
                "is_array": True,
                "max_length": max_length,
                "method_name": f"Send{output_name}"
            }
        else:
            # Not an array type
            type = json_output["type"]
            output = {
                "id": output_id,
                "name": output_name,
                "type": type,
                "is_array": False,
                "method_name": f"Send{output_name}"
            }

        outputs.append(output)
    service["outputs"] = outputs
    return service
