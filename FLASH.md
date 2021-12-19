# Flashing RedNodeLabs firmware

## Getting Started

The RedNodeLabs embedded platform is based on the Nordic Semiconductor [nRF5 microcontrollers family](https://www.nordicsemi.com/Products).

Our firmware development, programming and debugging process uses the toolchain that Nordic Semiconductor
provides for this purpose: [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools).

The toolchain is available for the platforms: Windows, Linux and macOS.

The main components we use from the toolchain are:

+ nrfjprog tool for programming through SEGGER J-LINK programmers and debuggers.
+ SEGGER J-Link software.

For some specific dev-boards or USB dongles that do not integrate a debugger,
we require additional tools, as the specified below:
+ [nRF Connect Programmer](https://infocenter.nordicsemi.com/topic/ug_nc_programmer/UG/nrf_connect_programmer/ncp_introduction.html)
is required to flash the [nRF52840 dongle](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-Dongle).
+ [Adafruit_nRF52_nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil) is required to flash the
[April USB Dongle 52840](https://wiki.aprbrother.com/en/BleUsbDongle.html#hardware-description-for-dongle-52840).


## Install

### nRF Command Line Tools

Please refer to the [documentation](https://infocenter.nordicsemi.com/topic/ug_nrf_cltools/UG/cltools/nrf_installation.html)
provided by Nordic Semiconductor to install this toolchain.

### nRF Connect Programmer

Please refer to the [documentation](https://infocenter.nordicsemi.com/topic/ug_nc_programmer/UG/common/nrf_connect_app_installing.html)
provided by Nordic Semiconductor to install this toolkit.

### adafruit-nrfutil

Please refer to the [documentation](https://github.com/adafruit/Adafruit_nRF52_nrfutil)
provided by Adafruit to install the tool.


## Flash

Once the required toolchains have successfully installed in the computer,
we can proceed to flash the firmware.

For that, please prepare the firmware image (*firmware.hex*) for each target platform under a known path in the computer.

Please follow the next sections depending on the target hardware.

### Boards with integrated debugger

Boards to be flashed following these steps:
+ [nRF52840-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK)
+ [DWM1001-DEV](https://www.qorvo.com/products/p/DWM1001-DEV)

```
nrfjprog -f nrf52 --program firmware.hex --sectorerase
nrfjprog -f nrf52 --reset
```

### Boards without integrated debugger

#### [nRF52840 dongle](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-Dongle)

Please refer to the [documentation](https://infocenter.nordicsemi.com/topic/ug_nrf52840_dongle/UG/nrf52840_Dongle/programming.html)
provided by Nordic Semiconductor to flash this USB dongle.

#### [April USB dongle 52840](https://wiki.aprbrother.com/en/BleUsbDongle.html#hardware-description-for-dongle-52840)

Firstly, please generate the *dfu-package.zip* with the firmware image:

```
adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application firmware.hex dfu-package.zip
```

Once generated, plug in the USB Dongle in the computer while pressing with a thin stick the SW1 button.
Now the USB dongle should have been entered the bootloader mode and its *SERIAL_PORT* should be enumerated.
Once the *SERIAL_PORT* associated to the USB dongle is known, please execute the following command:

```
adafruit-nrfutil dfu serial --package dfu-package.zip -p SERIAL_PORT -b 115200
```

#### [ISP3010](https://www.insightsip.com/products/combo-smart-modules/isp3010)

This board can be connected to a [nRF52840-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK)
and be then flashed following the [previously defined step](#Boards-with-integrated-debugger).
