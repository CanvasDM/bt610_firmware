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

