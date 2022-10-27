![logo](misc/logo.png)

# RedNodeBus Samples

This repository includes a list of samples integrating OpenThread + RedNodeBus (RNB) stack, based on [Zephyr OS](https://www.zephyrproject.org/).

## Prerequisites
Read the [Zephyr’s Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) to prepare your environment with the required tools and dependencies.

The [Zephyr SDK](https://github.com/zephyrproject-rtos/sdk-ng) version used for these samples is `0.14.2`.

Remember to set the required environment variables:
```
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
```

## Init
The project needs to be initialized using **west**.
```
mkdir zephyr-workspace
cd zephyr-workspace
```
```
west init -m git@github.com:rednodelabs/rednodebus-samples.git
```
```
west update
```

> To update to a newer release, remember to perform both a `git pull` in the `rednodebus-samples` folder inside `zephyr-workspace` and a `west update` to update the dependencies.

## RNB Lib Configuration
In case a previous OpenThread (OT) configuration has been programmed in the board, we recommend erasing the flash completely. This way, we can assure that the new OT configuration will be written correctly:
```
nrfjprog -e
```

To configure the board to act as an anchor, modify the following line in the `rnb_utils.c` file of the sample you are using:
```
rnb_user_config.role = REDNODEBUS_USER_ROLE_ANCHOR;
```
Similarly, to configure the board to act as a tag:
```
rnb_user_config.role = REDNODEBUS_USER_ROLE_TAG;
```

To use different OT credentials, specify them in `samples/common/overlay-ot-defaults.conf`.

To enable the RedNodeBus ranging diagnostic, add the overlay to `OVERLAY_CONFIG` when building the sample projects as following:
`../common/overlay-rednodebus-ranging-diagnostic.conf`. Note that this is mainly for debugging purposes, since the power consumption will increase.

To test the system, flash either the [RNB Node](samples/rednodebus_node/README.md), the [CoAP Client](samples/coap_client/README.md), the [Echo Client](samples/echo_client/README.md) or the [Socket Test](samples/socket_test/README.md) sample in the wireless nodes, and run the [RNB OTBR](#rednodebus--openthread-border-router-rnb-otbr) docker. Once running, interact with the system using the [MQTT API](#mqtt-api-specification).

## RedNodeBus + OpenThread Border Router (RNB OTBR)

To run the RedNodeBus services in the edge platform, RedNodeLabs provides a docker container based on the [OpenThread Border Router (OTBR)](https://openthread.io/guides/border-router) for Raspberry Pi (models 3B+, 4).

> A 64-bit OS is required, such as [Raspberry Pi OS (64-bit)](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit)

This container offers the OpenThread network services in combination with the RedNodeBus API based on MQTT.

Please, consider that this service requires getting a license. For further details, contact [RedNodeLabs](mailto:info@rednodelabs.com).

### Setting-Up the Raspberry Pi
Follow the instructions described in [OpenThread](https://openthread.io/guides/border-router/docker#raspberry_pi_setup) to set up your Raspberry Pi and install Docker tool.

### Setting-Up the RNB OTBR Docker
Firstly, pull the RNB OTBR from [RedNodeLabs docker hub repository](https://hub.docker.com/repository/docker/rednodelabs/rnb-otbr). Please specify the desired TAG:
```
docker pull rednodelabs/rnb-otbr:TAG
```

> TAG has the format `vX.X.X`, check the tag of the current release in Github.

> The TAG of the RNB OTBR Docker must match the version of the samples flashed in the nodes, otherwise there might be incompatibilities!

RNB OTBR requires our radio coprocessor (RCP) sample in order to form a Thread network and offer the RedNodeBus services. This sample has been developed to be used with the multiple boards. You can directly download the binaries from [rednodebus-release](https://github.com/rednodelabs/rednodebus-release/tree/main/hex) for some boards and default configurations.

If other configurations are required, e.g. different UART pins or baudrates, follow the build instructions below:

### nrf52840dk_nrf52840 board
#### With USB interface:
```
west build -p -b nrf52840dk_nrf52840 samples/coprocessor -- -DDTC_OVERLAY_FILE=usb.overlay -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf overlay-usb.conf"
```
```
nrfjprog -e
west flash
```
#### With UART interface:
```
west build -p -b nrf52840dk_nrf52840 samples/coprocessor -- -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf"
```
```
nrfjprog -e
west flash
```

##### Mass Storage Device known issue (only for UART interface through the on-board debugger)
Depending on your version, due to a known issue in SEGGER's J-Link firmware, you might experience data corruption or data drops if you use the serial port. You can avoid this issue by disabling the Mass Storage Device.

###### Disabling the Mass Storage Device on Linux
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

##### Hardware Flow Control detection (only for UART interface through the on-board debugger)
By default, SEGGER J-Link automatically detects at runtime whether the target is using Hardware Flow Control (HWFC).

The automatic HWFC detection is done by driving P0.07 (Clear to Send - CTS) from the interface MCU and evaluating the state of P0.05 (Request to Send - RTS) when the first data is sent or received. If the state of P0.05 (RTS) is high, it is assumed that HWFC is not used.

To avoid potential race conditions, you can force HWFC and bypass the runtime auto-detection.

###### Disabling the HWFC detection on Linux
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

### nrf52840dongle_nrf52840 board
#### With USB interface:
```
west build -p -b nrf52840dongle_nrf52840 samples/coprocessor -- -DDTC_OVERLAY_FILE=usb.overlay -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf overlay-usb.conf"
```

### Running the RNB OTBR Docker
After building and flashing, attach the RCP device to the Raspberry Pi running RNB OTBR Docker via UART or USB. Determine the serial port name for the RCP device by checking /dev:

```
$ ls /dev/tty*
/dev/ttyACMO
```

Once you have requested your customer information and received your claim certificates (certs.tar), copy the files to the Raspberry Pi and unzip them in a folder:
```
mkdir /home/pi/rnl_certs
tar -xvzf certs.tar -C /home/pi/rnl_certs
```
This folder should be mounted always as `/app/config` volume when running the docker container.

Start the RNB OTBR Docker, referencing the RCP's serial port and the folder where the credentials are stored. For example, if the RCP is mounted at `/dev/ttyACM0` and the certificates are in `/home/pi/rnl_certs`:
```
docker run --sysctl "net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1" -p 1883:1883 -it -v /home/pi/rnl_certs:/app/config -v /dev/ttyACM0:/dev/ttyACM0 --privileged rednodelabs/rnb-otbr:TAG
```

> The MQTT API uses requires using the port 1883. If the port is already used in the host, the docker initialization will fail. Disable services in the host that might be using it, such as Mosquitto.

> The first time you connect the Raspberry Pi it will require Internet access to download the unique device certificate. For further details regarding the provisioning process, please read [RedNodeLabs Device Provisioing](PROVISIONING.md).

### Running the RNB OTBR WEB UI Docker

In order to visualize and control the RedNodeBus services, a web UI is also provided as a docker service in the [RedNodeLabs docker hub repository](https://hub.docker.com/repository/docker/rednodelabs/rnb-otbr-web-ui).

The following docker compose template can be used to launch both RNB OTBR and RNB OTBR WEB UI services:

``` yaml
version: '3.8'
services:
  rnb-otbr:
    image: rednodelabs/rnb-otbr:TAG
    platform: linux/arm64
    ports:
      - "1883:1883"
    privileged: true
    sysctls:
      - net.ipv6.conf.all.disable_ipv6=0
      - net.ipv6.conf.all.forwarding=1
      - net.ipv4.conf.all.forwarding=1
    volumes:
      - /home/pi/rnl_certs:/app/config
      - /dev/ttyACM0:/dev/ttyACM0
    environment:
      - RADIO_URL=spinel+hdlc+uart:///dev/ttyACM0?uart-baudrate=1000000&uart-flow-control

  rnb-otbr-web-ui:
    image: rednodelabs/rnb-otbr-web-ui:TAG
    platform: linux/arm64
    ports:
      - "3000:3000"
    environment:
      - WEB_UI_PORT=3000
      - MQTT_HOST=rnb-otbr
      - MQTT_PORT=1883
```

Please, specify the right TAGs and desired parameters, such as the path of the volume with the certificates and the right serial port name.

In order to launch the services, save the template as a yaml file and run docker compose as follows:
```
docker compose up
```

Remember to bring it down before relaunching it:
In order to launch the services, save the template as a yaml file and run docker compose as follows:
```
docker compose down
```

On the Raspberry Pi running the docker containers, open a browser and go to `127.0.0.1:3000` to acces the web UI.

### MQTT API

The MQTT API can be also used to interact with the system. For further details, read the API documentation.

The corresponding version can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EX_PFtej2_lBsVAJyeMG_XIBFQ4_hwrBCuJRwgMOGRqE0g?e=TseCKu).
