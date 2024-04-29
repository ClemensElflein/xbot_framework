# xBot Framework

## Bridging the Gap in Robotics Development

Building robust and reusable robotics systems can be challenging. The Robot Operating System (ROS) provides powerful middleware for abstracting communication between high-level components, but struggles at the hardware level, requiring specialized firmware to interface with low-level devices. Existing solutions like microROS introduce a direct dependency on ROS, complicating debugging.

The **xBot Framework** offers a lightweight, independent solution for interfacing directly with sensors and actuators without adding additional dependencies. This new framework integrates hardware components seamlessly into the broader robotics ecosystem.

### Features

- **⚙️ Hardware Communication Simplified:** Define services in JSON format to generate a C++ code template, allowing for easy communication with hardware components such as ESCs, IMUs, and GPIOs.
- **🧩 Lightweight and Portable:** The framework has no dependencies and avoids dynamic memory allocation, making it ideal for microcontrollers, but can also be used on Linux systems.
- **🖥 Service-Based Architecture:** Each sensor and actuator is implemented as a service, described by a JSON interface and implemented as C++ service classes. This modular design allows for reusable hardware communication.
- **🔍 Service Discovery:** The framework includes automatic service discovery, making it easy to detect and manage all connected boards.
- **🌐 Control Hub:** A versatile hub provides:
    - **Web UI:** Visually explore devices, monitor data, and test actuators.
    - **Firmware Update:** Update the firmware on your board directly through the web interface using our included Ethernet bootloader.
    - **REST API:** Discover devices programmatically.
    - **Optional ROS Bridge:** Automatically maps discovered devices to ROS topics, offering configurable integration with ROS-based systems.

### Status and Contributions

The xBot Framework is currently a work in progress, and we welcome any input or feedback. If you'd like to contribute, please read our contributing guidelines and check out our issue tracker.
