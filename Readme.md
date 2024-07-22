# xBot Framework

## Disclaimer

Important: The xBot Framework is a very early Work-In-Progress (WIP) project. It is not ready for production use, and
the codebase is subject to change without notice. Use it at your own risk, and be prepared for significant modifications
and updates as the project evolves.

## Bridging the Gap in Robotics Development

Building robust and reusable robotics systems can be challenging. The Robot Operating System (ROS) provides powerful
middleware for abstracting communication between high-level components, but struggles at the hardware level, requiring
specialized firmware to interface with low-level devices. Existing solutions like microROS introduce a direct dependency
on ROS which complicates debugging, bloats the low-level firmware and leads to tight coupling between high-level
application specific code and low-level drivers.

The **xBot Framework** offers a lightweight, independent solution for interfacing directly with sensors and actuators
without adding heavy dependencies. This new framework helps integrate hardware components seamlessly into the broader
robotics ecosystem.

### Features

- **🖥 Service-Based Architecture:** Low level features (e.g. sensors or actuators) are implemented as a service,
  described by a JSON interface
  and implemented as C++ classes. This modular design allows for reusable hardware communication. Service
  implementations can be done either on a microcontroller or a Linux system, since the communication is abstracted from
  the actual operating system.
- **⚙️ Hardware Communication Simplified:** Define services in JSON format to generate a C++ code template, allowing for
  easy communication with hardware components such as ESCs, IMUs, and GPIOs.
- **🧩 Lightweight and Portable:** The framework has minimal dependencies and avoids dynamic memory allocation, making it
  ideal for microcontrollers, but can also be used on Linux systems.
- **🔍 Service Discovery:** The framework includes automatic service discovery, making it easy to connect your low-level
  services to your application specific high-level code.
- **⚙️ Runtime:** The Runtime:
    - **Web UI:** Visually explore devices, monitor data, and test actuators. (Status: Planned)
    - **Firmware Update:** Update the firmware on your board directly through the web interface using our included
      Ethernet bootloader. (Status: Planned)
    - **REST API:** Discover services programmatically. (Status: Working)
    - **Plugin System:** Write an application-specific plugin which gets loaded by the runtime to bridge to your
      high-level code.
- **🚀 Performant Serialization:** Data is transmitted schemaless and binary packed. This leads to less traffic and fast
  serialization times.

## Repository Structure

This repository contains all parts of the xbot_framework.

### /libxbot-service

Use this library to provide a service to the system. An example would be publishing IMU data or providing motor control
services.
The library will take care of advertising your service and connecting to the runtime.

### /libxbot-service-interface

Use the **libxbot-service-interface** library to connect to a specific service. For example if there is an IMU service
on your network, and you want to receive its data (or bridge to ROS), include **libxbot-service-interface** in your
project to use your services.

### /include

Files in this directory are needed on both sides (service and service interface). E.g. message header definitions.

### /codegen

This folder contains the code generation part of the **xbot_framework**. Code generation is done from service.json files
which describe inputs, outputs and registers of services.

The code generator will generate callbacks for inputs and sending methods for the outputs. It will also generate all
code necessary for service discovery and configuration.

### /ext

All dependencies are included here as submodules. Not every dependency is needed by every part of the software.

## Status and Contributions

The xBot Framework is currently a work in progress, and we welcome any input or feedback. If you'd like to contribute,
please read our contributing guidelines and check out our issue tracker.
