# Coprocessor
The Coprocessor sample demonstrates how to implement OpenThread's RCP inside the Zephyr environment. Compatible with `nrf52840dk_nrf52840`, `nrf52840dongle_nrf52840`,
`nrf52833dk_nrf52833` and `qorvo_dwm3001c_dev` as BOARD_NAME.

#### With USB interface:
```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf overlay-vendor_hook-rnb.conf overlay-usb-rnb.conf"
```

#### With USB interface and ranging diagnostic
```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf overlay-vendor_hook-rnb.conf overlay-usb-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```

#### With UART interface:
```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf overlay-vendor_hook-rnb.conf"
```
#### With UART interface and ranging diagnostic
```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf overlay-vendor_hook-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```

#### Mass Storage Device known issue (only for UART interface through the on-board debugger)
Depending on your version, due to a known issue in SEGGER's J-Link firmware, you might experience data corruption or data drops if you use the serial port. You can avoid this issue by disabling the Mass Storage Device.

#### Disabling the Mass Storage Device on Linux
1. Connect the DK to your machine with a USB cable.
2. Run `JLinkExe` to connect to the target. For example:
```
JLinkExe -device NRF52840_XXAA -if SWD -speed 4000 -autoconnect 1 -SelectEmuBySN <SEGGER_ID>
```
3. Run the following command:
```
MSDDisable
```
4. Power cycle the DK.

#### Hardware Flow Control detection (only for UART interface through the on-board debugger)
By default, SEGGER J-Link automatically detects at runtime whether the target is using Hardware Flow Control (HWFC).

The automatic HWFC detection is done by driving P0.07 (Clear to Send - CTS) from the interface MCU and evaluating the state of P0.05 (Request to Send - RTS) when the first data is sent or received. If the state of P0.05 (RTS) is high, it is assumed that HWFC is not used.

To avoid potential race conditions, you can force HWFC and bypass the runtime auto-detection.

#### Disabling the HWFC detection on Linux
1. Connect the DK to your machine with a USB cable.
2. Run `JLinkExe` to connect to the target. For example:
```
JLinkExe -device NRF52840_XXAA -if SWD -speed 4000 -autoconnect 1 -SelectEmuBySN <SEGGER_ID>
```
3. Run the following command:
```
SetHWFC Force
```
4. Power cycle the DK.
