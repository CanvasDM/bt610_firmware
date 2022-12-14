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

# Application Version 1.23.85 (July 30, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input, high current inputs, pressure inputs and ultrasonic input, and I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

### Changed

-19469 Add button press PHY change timeout
-19464 Tamper Switch LED off when in self mode
-19465 Purge advertising event queue during client disconnection
-19470 Adjust BT510 Interval & Duration business rules

### Fixed
-19476 Aggregation Count not reporting back correctly
-19446 Simultaneous LEDs cause lockup
-19473 Event Id not correct for AC current
-19460 advertising interval not saving after reset when value is higher 4sec

### Known Issues / Limitations

-19282 Sensor name length being misreported 
-19249 Normal, BT610 Advertising Issue (not advertising, or phy change) 
-19248 Normal, Connection Failure, Unhandled ATT code 0x1d + Ignoring unexpected request. 
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark 
-18806 BT610 will indicate the RX ring buffer is full 

### Resource Documents

- BT6ApiParams v1.67
- BT610 Comm Spec v1.0
- User Guide v1.0

# Application Version 1.22.5 (July 22, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input, high current inputs, pressure inputs and ultrasonic input, and I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

### Changed

-18828 Connection timeout removal

### Fixed

### Known Issues / Limitations

-19282 Sensor name length being misreported 
-19249 Normal, BT610 Advertising Issue (not advertising, or phy change) 
-19248 Normal, Connection Failure, Unhandled ATT code 0x1d + Ignoring unexpected request. 
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark 
-18806 BT610 will indicate the RX ring buffer is full 

### Resource Documents

- BT6ApiParams v1.64
- BT610 Comm Spec v1.0
- User Guide v1.0

# Application Version 1.21.9 (July 21, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input, high current inputs, pressure inputs and ultrasonic input, and I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

-19137 Add parameter write error feedback mechanism

### Changed

-19423 Set Advertising Interval minimum to 500ms
-19420 Inherit BT510 Advertising Interval & Duration relationship 

### Fixed

-19380 Normal, BT610 lockup after removing power

### Known Issues / Limitations

-19282 Sensor name length being misreported 
-19249 Normal, BT610 Advertising Issue (not advertising, or phy change) 
-19248 Normal, Connection Failure, Unhandled ATT code 0x1d + Ignoring unexpected request. 
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark 
-18806 BT610 will indicate the RX ring buffer is full 

### Resource Documents

- BT6ApiParams v1.63
- BT610 Comm Spec v1.0
- User Guide v1.0

# Application Version 1.20.13 (July 16, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input, high current inputs, pressure inputs and ultrasonic input, and I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

### Changed

### Fixed

-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 
-19238 Correct potential divide by zero issue in AD module 
-18814 BT610 Stops Advertising 
-19303 Move BLE callbacks to ROM 

### Known Issues / Limitations

-19282 Sensor name length being misreported 
-19249 Normal, BT610 Advertising Issue (not advertising, or phy change) 
-19248 Normal, Connection Failure, Unhandled ATT code 0x1d + Ignoring unexpected request. 
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark 
-18806 BT610 will indicate the RX ring buffer is full 
-18728 Missing scan response 

### Resource Documents

- BT6ApiParams v1.60
- BT610 Comm Spec v1.0
- User Guide v1.0

# Application Version 1.19.69 (July 14, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input. 
With some functional support for high current inputs, pressure inputs and ultrasonic input, and I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

### Changed

### Fixed

-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 
-19238 Correct potential divide by zero issue in AD module 
-18814 BT610 Stops Advertising 
-19303 Move BLE callbacks to ROM 

### Known Issues / Limitations

-19282 Sensor name length being misreported 
-19249 Normal, BT610 Advertising Issue (not advertising, or phy change) 
-19248 Normal, Connection Failure, Unhandled ATT code 0x1d + Ignoring unexpected request. 
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark 
-18806 BT610 will indicate the RX ring buffer is full 
-18728 Missing scan response 

### Resource Documents

- BT6ApiParams v1.58
- BT610 Comm Spec v1.0
- User Guide v1.0

# Application Version 1.18.87 (July 1, 2021)


### Summary

This release of the BT610 includes the following configurable features: analog inputs, digital inputs, thermistors, digital outputs. With some functional support for high current inputs, pressure inputs and ultrasonic input. 
With some functional support for high current inputs, pressure inputs and ultrasonic input, and I2C/SPI/UART configuration. 

### Target Hardware

The hardware is R5.0

### Added

### Changed

### Fixed

-19145 Runtime fault occurs 
-19272 AC current sensor not reporting 
-19278 Coded PHY adverts indicated as 1M type
-19270 BT610 lockup 
-19285 Measurements are done multiple times 
-19243 Battery interval not turning off 

### Known Issues / Limitations

-19282 Sensor name length being misreported 
-19249 Normal, BT610 Advertising Issue (not advertising, or phy change) 
-19248 Normal, Connection Failure, Unhandled ATT code 0x1d + Ignoring unexpected request. 
-19238 Correct potential divide by zero issue in AD module 
-18881 internal timestamp seems to be rolling over around the 36 1/2 hour mark 
-18814 BT610 Stops Advertising 
-18806 BT610 will indicate the RX ring buffer is full 
-18728 Missing scan response 
-18826 ASSERTION FAIL in /zephyr/subsys/bluetooth/controller/ll_sw/nordic/lll/lll_adv_aux.c:367 

### Resource Documents

- BT6ApiParams v1.58
- BT610 Comm Spec v1.0
- User Guide v1.0

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

