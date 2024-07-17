# xbot-service Library

This library can be used to easily implement services compatible with the xbot framework.

There is a code generator for service interfaces and also pre-defined interfaces for reuse.

All platform specific code can be found in the `src/portable` directory. For Linux there is an example implementation.
You can provide a custom implementation for your platform.

This library is embedded friendly, it does not use heap allocations at all and keeps used resources at a minimum.
