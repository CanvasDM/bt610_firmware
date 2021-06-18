# Release Notes

This document provides current and previous release notes for the Sentrius BT610.

Release notes are a summary of changes, new and enhanced features, resolved issues, and known issues that are not
resolved in this version. This set of release notes applies to both the firmware and support documents.

## Third Party

This project makes use of the following third-party components:

| Name                     | License         | Link                                                     |
| -------------------------| ----------------| ---------------------------------------------------------|
| Zephyr Project RTOS      | Zephyr Project  | https://github.com/zephyrproject-rtos/zephyr             |
| mcumgr                   | Apache          | https://github.com/apache/mynewt-mcumgr                  |
| NRF SDK                  | Nordic          | https://github.com/nrfconnect/sdk-nrf                    |
| mcuboot                  | Apache          | https://github.com/mcu-tools/mcuboot                     |


# Firmware Versions

# Application Version 1.17.7 (June 18, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input. 

This release does not provide support for the following: I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

### Changed

### Fixed

### Known Issues / Limitations

-19145 Runtime fault occurs
-19184 Correct invalid value handling for config Type
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark
-18814 BT610 Stops Advertising 
-18806 BT610 will indicate the RX ring buffer is full 
-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 

### Resource Documents

- BT6ApiParams v1.54
- BT610 Comm Spec v1.0
- User Guide v1.0


# Application Version 1.16.29 (June 8, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input. 

This release does not provide support for the following: I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added


### Changed
-19071 Allow device to return to shelf mode 

### Fixed
-19129 Thermistor Config being reset 
-19122 Alarms enable disable control 
-19114 Events not being advertised 

### Known Issues / Limitations

-18814 BT610 Stops Advertising 
-18806 BT610 will indicate the RX ring buffer is full 
-18728 Missing scan response 
-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 

### Resource Documents

- BT6ApiParams v1.54
- BT610 Comm Spec v1.0
- User Guide v1.0


# Application Version 1.15.37 (May 26, 2021)


### Summary

This release of the BT610 that includes the following configurable features: Analog inputs, digital inputs, thermistors, digital outputs, pressure inputs, ultrasonic input.
Also this release does not provide support for: I2C/SPI/UART configuration.

### Target Hardware

The hardware is R5.0

### Added
Ultrasonic and pressure configuration.

### Changed
-18979 Change order of the flags

### Fixed
-18989 prepare log return values are not populated
-18978 length of device name missing
-18668 The SetRtc Time API Message Needs Clarity
-18664 SetParameter message response in OpenRPC does not match what is being sent.

### Known Issues / Limitations

-18814 BT610 Stops Advertising 
-18806 BT610 will indicate the RX ring buffer is full 
-18728 Missing scan response 
-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 

### Resource Documents

- BT6ApiParams v1.38
- BT610 Comm Spec v1.0
- User Guide v1.0


# Application Version 1.11.1 (April 21, 2021)


### Summary

This is the first major release of the BT610 that includes the following configurable features: Analog inputs, digital inputs, thermistors, digital outputs.
Also this release does not provide support for: pressure inputs, ultrasonic input, or I2C/SPI/UART configuration.

### Target Hardware

The hardware is R2.0

### Added

### Changed

### Fixed

### Known Issues / Limitations

-18814 BT610 Stops Advertising 
-18806 BT610 will indicate the RX ring buffer is full 
-18728 Missing scan response 
-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 

### Resource Documents

- BT6ApiParams v1.38
- BT610 Comm Spec v1.0
- User Guide v1.0

