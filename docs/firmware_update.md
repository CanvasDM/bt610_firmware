# Firmware Updates

## Prerequisites
1. [mcumgr CLI](https://github.com/apache/mynewt-mcumgr#command-line-tool) (cross platform)
2. BT610 running firmware part number 480-00099 v2.x or greater. 
3. Terminal program: Putty (Windows,Linux,macOS), Teraterm (Windows), Serial (macOS)

## Update Zephyr App Via UART

1. Connect terminal program to console UART and turn off log messages. Log messages output by the firmware can interfere with the firmware transfer process.

    Issue command:
    ```
    log halt
    ```

2. Disconnect the terminal program from the console UART and transfer the update file to the BT610 using the mcumgr CLI via the console UART.

    ```
    # Linux/macOS

    mcumgr -t 20 -r 3 --conntype serial --connstring dev=/dev/tty.usbserial-A908JLEI,mtu=512 image upload ../../BT610_v2.0.0.bin

    # Windows

    mcumgr -t 20 -r 3 --conntype serial --connstring dev=COM4,mtu=512 image upload C:\BT610_v2.0.0.bin

    ```
    Depending on the size of the update file, the transfer can take some time.

2. List the images to obtain the hash of the update image in slot 1

    ```
    # Linux/macOS

    mcumgr --conntype serial --connstring dev=/dev/tty.usbserial-A908JLEI image list

    # Windows

    mcumgr --conntype serial --connstring dev=COM4 image list

    ```

    Response should look like
    ```
    Images:
    image=0 slot=0
        version: 2.0.0
        bootable: true
        flags: active confirmed
        hash: 292df381866bf65cab8f007897e3bcd8e936d5e37ba78183162e6f5fe1085b03
    image=0 slot=1
        version: 2.0.0
        bootable: true
        flags:
        hash: e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276
    Split status: N/A (0)
    ```

3. Test the image in slot 1. This sets the image in slot 1 to be swapped and booted.

    ```
    # Linux/macOS

    mcumgr --conntype serial --connstring dev=/dev/tty.usbserial-A908JLEI image test e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276

    # Windows

    mcumgr --conntype serial --connstring dev=COM4 image test e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276

    ```

    Response should look like
    ```
    Images:
    image=0 slot=0
        version: 2.0.0
        bootable: true
        flags: active confirmed
        hash: 292df381866bf65cab8f007897e3bcd8e936d5e37ba78183162e6f5fe1085b03
    image=0 slot=1
        version: 2.0.0
        bootable: true
        flags: pending
        hash: e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276
    Split status: N/A (0)
    ```
    Note the `flags` for slot 1 are now set to pending.

4. Issue a reset to swap to the slot 1 image and boot it. This can take some time to complete.

    ```
    # Linux/macOS

    mcumgr --conntype serial --connstring dev=/dev/tty.usbserial-A908JLEI reset

    # Windows

    mcumgr --conntype serial --connstring dev=COM4 image reset

    ```

5. Re-connect the terminal program to the console UART to monitor when the new image boots. Once it boots, issue the turn off logging in preparation for the last step.

    Issue command:
    ```
    log halt
    ```

6. Confirm the image. If the new image is not confirmed, the image will be swapped back to slot 1 on the next reboot.

    ```
    # Linux/macOS

    mcumgr --conntype serial --connstring dev=/dev/tty.usbserial-A908JLEI image confirm

    # Windows

    mcumgr --conntype serial --connstring dev=COM4 image confirm

    ```

## Update Zephyr App Via BLE (mcumgr CLI)

Using mcumgr CLI and BLE is only supported on Linux or macOS.

1. Transfer the update file to the BT610 using the mcumgr CLI via BLE.

    ```
    mcumgr -t 20 -r 3 --conntype ble --connstring ctlr_name=hci0,peer_name='BT610-0303848' image upload /../../bt610_v2.0.10.bin

    ```
    Depending on the size of the update file, the transfer can take some time.

2. List the images to obtain the hash of the update image in slot 1

    ```
    mcumgr --conntype ble --connstring ctlr_name=hci0,peer_name='BT610-0303848' image list

    ```

    Response should look like
    ```
    Images:
    image=0 slot=0
        version: 2.0.0
        bootable: true
        flags: active confirmed
        hash: 292df381866bf65cab8f007897e3bcd8e936d5e37ba78183162e6f5fe1085b03
    image=0 slot=1
        version: 2.0.0
        bootable: true
        flags:
        hash: e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276
    Split status: N/A (0)
    ```

3. Test the image in slot 1. This sets the image in slot 1 to be swapped and booted.

    ```
    mcumgr --conntype ble --connstring ctlr_name=hci0,peer_name='BT610-0303848' image test e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276

    ```

    Response should look like
    ```
    Images:
    image=0 slot=0
        version: 2.0.0
        bootable: true
        flags: active confirmed
        hash: 292df381866bf65cab8f007897e3bcd8e936d5e37ba78183162e6f5fe1085b03
    image=0 slot=1
        version: 2.0.0
        bootable: true
        flags: pending
        hash: e378dde02fe58825fe0b620926ec932f0a4aaaa82857e897e40f7486d2011276
    Split status: N/A (0)
    ```
    Note the `flags` for slot 1 are now set to pending.

4. Issue a reset to swap to the slot 1 image and boot it. This can take some time to complete.

    ```
    mcumgr --conntype ble --connstring ctlr_name=hci0,peer_name='BT610-0303848' reset

    ```

6. Confirm the image once it has booted. If the new image is not confirmed, the image will be swapped back to slot 1 on the next reboot.

    ```
    mcumgr --conntype ble --connstring ctlr_name=hci0,peer_name='BT610-0303848' image confirm

    ```