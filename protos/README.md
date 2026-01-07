This directory is the canonical location for all service definitions used by
the android emulator, both qemu2 and qemu-next. The services are defined using
[gRPC](https://grpc.io/).

## Services

Below is a summary of the available services:

### Core Services

* **Adb**: Enables interaction with the running Adb service inside the
emulator. This is usually only available to containerized emulators.
* **EmulatorController**: A comprehensive service to control many aspects of
the emulator, including sensors, clipboard, battery, GPS, input events (touch,
mouse, keyboard), phone calls, SMS, and getting the emulator status and
screenshots.
* **EmulatorStats**: Allows querying the emulator's runtime information
including memory usage, CPU usage and etc.
* **SnapshotService**: Manages snapshots, allowing to list, insert, store, and
retrieve them.
* **UiController**: Manages the emulator's user interface, like showing/hiding
extended controls and setting the theme.
* **Rtc**: An RTC service lets you interact with the emulator through WebRTC.

### DEPRECATED Services

These services are deprecated and will not be ported to qemu-next.

* **Waterfall**: Provides services for pushing/pulling files, executing
commands, and port forwarding between the host and the device.
* **PortForwarder**: A service for managing port forwarding sessions via
`Waterfall`.
* **EmulatedBluetoothService**: Registers an emulated bluetooth device.
Interactions with Bluetooth should now be done using Netsim and Bumble
(<https://google.github.io/bumble/>).
* **GattDeviceService**: Represents a bluetooth Gatt device. The android
emulator will connect to this service to read/write and observe characteristics
if the emulator establishes a connection to this device.

### Incubating Services

These services are under development and might change in the future.

* **AvdService**: Inspects and retrieves general information about the running
AVD.
* **CarService**: Forwards events to and from the Android car HAL.
* **Modem**: Interacts with the emulated modem for things like SMS, phone
calls, and cellular information.
* **ScreenRecording**: Manages screen recording sessions.
* **SensorService**: Interacts with the emulator's sensor module, allowing to
set, get, and stream sensor data.
* **VirtualSceneService**: Manages posters and animations in the virtual scene.

### Testing Services

* **TestEcho**: A simple echo service for testing gRPC behavior.
* **TestRunner**: A service for running various IPC tests.

