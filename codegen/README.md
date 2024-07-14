# xbot_comms Codegen
This folder include the code generator for the service template.

In order to use it, declare the service in your CMakeLists.txt and inherit from the genrerated base class.

The code generator will generate:
- Callbacks for input channels
- Send methods for output channels
- All IO handling and threading including service discovery


Look at the EchoService for an example.
