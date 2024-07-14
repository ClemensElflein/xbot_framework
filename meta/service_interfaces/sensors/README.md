Well known interfaces for sensors.

## imu_service.json
An interface for an IMU with up to 9DOF.

### Inputs: None
### Outputs:
- Data (ID 0): 9 floats with the following meaning:
  - acceleration (x/y/z) in m/sec
  - rotation (x/y/z) in rad/sec
  - magnetic field (x/y/z) in utesla

### Registers:
- Update Rate (ID 0): Update rate in Hz
- Enabled Channels (ID 1): Used to get and set the enabled channels. It's a bitfield with the lower 9 bits set according to the enabled channels.
- Available Channels (ID 2, Read Only): Used to get the available channels. It's a bitfield with the lower 9 bits set according to the available channels.
