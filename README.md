![logo](misc/logo.png)

# RedNodeBus Samples

This repository includes a list of samples integrating OpenThread with the RedNodeBus (RNB) Lib, providing an extremely reliable, energy-efficient and real-time positioning and data bus based on [Zephyr OS](https://www.zephyrproject.org/).

## Prerequisites
Read the [Zephyr’s Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) to prepare your environment with the required tools and dependencies.

The [Zephyr SDK](https://github.com/zephyrproject-rtos/sdk-ng) version used for these samples is `0.15.2`.

Remember to set the required environment variables:
```
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
```

The project needs to be initialized using **west**.
```
mkdir zephyr-workspace
cd zephyr-workspace
```
```
west init -m git@github.com:rednodelabs/rednodebus-samples
```
```
west update
```

> To update to a newer release, remember to perform both a `git pull` in the `rednodebus-samples` folder inside `zephyr-workspace` and a `west update` to update the dependencies.

## RNB Lib
The documentation of the RNB Lib can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EWNBbUEbA5tLpctNqfadtgkBzdcLokwpXBGhRz0bFCw9QQ?e=5FbUsv).

To test the system, check the `samples` folder and flash either the [RNB Node](samples/rednodebus_node/README.md), the [CoAP Client](samples/coap_client/README.md), the [Echo Client](samples/echo_client/README.md), the [Accelerometer](samples/accelerometer/README.md) or the [Socket Test](samples/socket_test/README.md) sample in the wireless nodes, and run the [RNB OTBR](#integrating-rednodebus-with-the-openthread-border-router) docker. Once running, interact with the system using the [MQTT API](#mqtt-api-documentation).

All samples, except the `coprocessor`, `battery` and `accelerometer`, are compatible with the following boards:
- `decawave_dwm1001_dev` (data bus and ranging)
- `qorvo_dwm3001c_dev` (data bus and ranging)
- `insightsip_isp3010_dev` (data bus and ranging)
- `nrf52840dk_nrf52840` (only data bus)
- `nrf52840dongle_nrf52840` (only data bus)
- `nrf52833dk_nrf52833` (only data bus)
- `nrf52832_mdk` (only data bus)
- `rnl_w1` (data bus and ranging)
- `rnl_w1x` (only data bus)

The `battery` sample is compatible with the following boards:
- `rnl_w1` (data bus and ranging)
- `rnl_w1x` (only data bus)

The `accelerometer` sample is compatible with the following boards:
- `decawave_dwm1001_dev` (data bus and ranging)
- `qorvo_dwm3001c_dev` (data bus and ranging)
- `rnl_w1` (data bus and ranging)
- `rnl_w1x` (only data bus)

The `coprocessor` sample is compatible with the following boards:
- `nrf52840dk_nrf52840`
- `nrf52833dk_nrf52833`
- `nrf52840dongle_nrf52840`
- `qorvo_dwm3001c_dev`
- `rnl_w1`

The fastest way to start is programming the simplest sample in the nodes ([RNB Node](samples/rednodebus_node/README.md)). Compiled hex files of the samples are available for some supported boards in [rednodebus-release](https://github.com/rednodelabs/rednodebus-release/tree/main/hex). To check supported boards and building flags, the `sample.yaml` files of each sample can be used as reference.

In general, other custom boards using the `nRF52832_QFAA` (with or without `decawave_dw1000`), `nRF52833_QIAA` (with or without `qorvo_dw3000`) and `nRF52840_QIAA` SOCs should work with the library; but only the boards mentioned aboved have been tested. Included [board's devicetrees](boards) can be used as a base for porting to other boards.

## Integrating RedNodeBus with the OpenThread Border Router

To run the RedNodeBus services in the edge platform, RedNodeLabs provides a docker container based on the [OpenThread Border Router (OTBR)](https://openthread.io/guides/border-router) for Raspberry Pi (models 3B+, 4). It may also work with other `linux/arm64` platforms. Alternatively, the docker is also available for the `linux/amd64` platform, by specifying it in the docker compose file, so it can be run on a regular Linux laptop.

> A 64-bit OS is required, such as [Raspberry Pi OS (64-bit)](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit).

This container offers the OpenThread network services in combination with the RedNodeBus API based on MQTT.

Please, consider that this service requires getting a license. For further details, contact [RedNodeLabs](mailto:info@rednodelabs.com).

### Setting-Up the Raspberry Pi
Follow the instructions described in [OpenThread](https://openthread.io/guides/border-router/docker#raspberry_pi_setup) to set up your Raspberry Pi and install Docker tool.


### RNB OTBR Docker
In order to launch and interface (via MQTT) with the RedNodeBus system a docker is available in the [RedNodeLabs docker hub repository](https://hub.docker.com/repository/docker/rednodelabs/rnb-otbr).

RNB OTBR requires a device connected and programmed with our [Radio Co-Processor (RCP) sample](samples/coprocessor/README.md) in order to form a Thread network and offer the RedNodeBus services. This sample has been developed to be used with multiple boards. You can directly download the binaries from [rednodebus-release](https://github.com/rednodelabs/rednodebus-release/tree/main/hex) for some boards and default configurations, e.g. USB and UART through the on-board debugger. If other configurations are required, e.g. different UART pins, follow the build instructions [here](samples/coprocessor/README.md).

After building and flashing the RCP, attach it to the Raspberry Pi via UART or USB.

> If using the RCP via USB, the serial port name will be automatically recognized and there is no need to find it manually. If using UART the rigt port, e.g. `/dev/ttyACM0`, needs to be specified later on.

Once you have requested your customer information and received your claim certificates (`certs.tar`), copy the files to the Raspberry Pi and unzip them in a folder:
```
mkdir /home/pi/rnl_certs
tar -xvzf certs.tar -C /home/pi/rnl_certs
```
This folder should be mounted always as a volume in `/app/config` when running the docker container.

### RNB OTBR WEB UI Docker
In order to visualize and interact with the RedNodeBus services, an optional web UI is also provided as a docker service in the [RedNodeLabs docker hub repository](https://hub.docker.com/repository/docker/rednodelabs/rnb-otbr-web-ui).

The following docker compose template can be used to launch both RNB OTBR and RNB OTBR WEB UI services:

``` yaml
version: '3.8'
services:
  rnb-otbr:
    image: rednodelabs/rnb-otbr:TAG
    platform: linux/arm64
    deploy:
      restart_policy:
        condition: always
    ports:
      - "1883:1883"
    privileged: true
    sysctls:
      - net.ipv6.conf.all.disable_ipv6=0
      - net.ipv6.conf.all.forwarding=1
      - net.ipv4.conf.all.forwarding=1
      - net.netfilter.nf_conntrack_udp_timeout=1200
      - net.netfilter.nf_conntrack_udp_timeout_stream=1200
    volumes:
      - /home/pi/rnl_certs:/app/config
      - /dev:/dev
    environment:
      - RCP_FD=/dev/ttyACM0
      - MQTT_HOST=localhost
      - MQTT_PORT=1883
    networks:
      rnb-otbr-nw:
        ipv4_address: 172.19.0.2

  rnb-otbr-web-ui:
    image: rednodelabs/rnb-otbr-web-ui:TAG
    platform: linux/arm64
    deploy:
      restart_policy:
        condition: always
    ports:
      - "3000:3000"
    environment:
      - MQTT_HOST=rnb-otbr
      - MQTT_PORT=1883
    networks:
      rnb-otbr-nw:
        ipv4_address: 172.19.0.3

networks:
  rnb-otbr-nw:
    driver: bridge
    ipam:
     config:
       - subnet: 172.19.0.0/16
```

Please, replace the `TAG` labels with the right ones (i.e. `vX.X.X`) in the `image` fields and specify the correct path of the volume with the certificates (`/home/pi/rnl_certs` in the example). If UART is used, replace `/dev/ttyACM0` with the right RCP file descriptor in the `RCP_FD` environment variable (not required if USB is used).

> TAG has the format `vX.X.X`, check the tag of the current release in Github.

> The TAG of the RNB OTBR Docker must match the version of the samples flashed in all nodes in the system, otherwise there might be incompatibilities!

> In the sample yaml, the port `1883` is specified to deploy the MQTT broker. If the port is already used in the host, the docker initialization will fail. In that case, a different port can be binded. Similarly, port `3000` is used for the web UI, it can be changed to a different one. In both cases the chosen port needs to be specified to access externally the MQTT and web interfaces from the host.

> The first time you connect the Raspberry Pi it will require Internet access to download the unique device certificate. It is also required the first time you connect a new node to request its license. For further details regarding the provisioning process, please read [RedNodeLabs Device Provisioning](PROVISIONING.md).

In order to launch the services, save the template as a yaml (`docker-compose.yml`) file and run docker compose as follows:
```
docker compose up --force-recreate
```

On the Raspberry Pi running the docker containers, open a browser and go to `127.0.0.1:3000` to acces the web UI and connect the wireless nodes programmed with the RedNodeBus samples.

> The web UI can be accessed from a different machine in the same local network as the Raspberry Pi, e.g. going to `raspberrypi:3000`, to avoid the load of the graphical interface in the Pi.

The docker log file is stored in the mounted volume, i.e. `/home/pi/rnl_certs/log/syslog`.

### RNB OTBR DFU Service
We offer Device Firmware Upgrade (DFU) licenses for automatically managing the firmware of the RCP attached to the Raspberry Pi, in order to facilitate the deployment of the service and tp avoid version mismatches between the docker service and the firmware running in the RCP. If you acquire the license for the DFU service, the firmware will be automatically checked when the docker starts, and the right image will be flashed if not already present in the device.

We currently support the following boards:
- `nrf52840dk_nrf52840` (USB and UART interface via on-board debugger)
- `nrf52833dk_nrf52833` (USB and UART interface via on-board debugger)
- `qorvo_dwm3001c_dev` (USB only)
- `rnl_w1` (USB only)

The board needs to be flashed with the `coprocessor_smp_svr` available in [rednodebus-release](https://github.com/rednodelabs/rednodebus-release/tree/main/hex) for the supported boards and interfaces. Please use the following command to flash the corresponding image:
```
west flash --hex-file path/to/image/merged.hex
```

In order to activate the service in the docker, use the following docker compose template:

``` yaml
version: '3.8'
services:
  rnb-otbr:
    image: rednodelabs/rnb-otbr:TAG
    platform: linux/arm64
    deploy:
      restart_policy:
        condition: always
    ports:
      - "1883:1883"
    privileged: true
    sysctls:
      - net.ipv6.conf.all.disable_ipv6=0
      - net.ipv6.conf.all.forwarding=1
      - net.ipv4.conf.all.forwarding=1
      - net.netfilter.nf_conntrack_udp_timeout=1200
      - net.netfilter.nf_conntrack_udp_timeout_stream=1200
    volumes:
      - /home/pi/rnl_certs:/app/config
      - /dev:/dev
    environment:
      - RCP_FD=/dev/ttyACM0
      - MQTT_HOST=localhost
      - MQTT_PORT=1883
      - RNB_OTBR_DFU=1
      - RNB_OTBR_DFU_BOARD=BOARD_NAME
      - RNB_OTBR_DFU_UART=0
      - RNB_OTBR_DFU_RANGING_DIAG=0
    networks:
      rnb-otbr-nw:
        ipv4_address: 172.19.0.2


  rnb-otbr-web-ui:
    image: rednodelabs/rnb-otbr-web-ui:TAG
    platform: linux/arm64
    deploy:
      restart_policy:
        condition: always
    ports:
      - "3000:3000"
    environment:
      - MQTT_HOST=rnb-otbr
      - MQTT_PORT=1883
    networks:
      rnb-otbr-nw:
        ipv4_address: 172.19.0.3

networks:
  rnb-otbr-nw:
    driver: bridge
    ipam:
     config:
       - subnet: 172.19.0.0/16
```
Please, replace the `TAG` labels with the right ones (i.e. `vX.X.X`) in the `image` fields and specify the correct path of the volume with the certificates (`/home/pi/rnl_certs` in the example). If UART is used, replace `/dev/ttyACM0` with the right RCP file descriptor in the `RCP_FD` environment variable (not required if USB is used) and change `RNB_OTBR_DFU_UART` to `1`. Also replace `BOARD_NAME` with the right name of your board in the `RNB_OTBR_DFU_BOARD` variable, e.g. `nrf52840dk_nrf52840`. If ranging diagnostics are desired set `RNB_OTBR_DFU_RANGING_DIAG` to `1` (only for testing purposes, all other nodes in the system must be compiled with this option for the system to operate properly).

## MQTT API

The MQTT API can be also used to interact with the system. For further details, read the API documentation.

The corresponding version can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EedNfrVqV_xDj_XAf63u8q4BvzvoVIRBdpE716DOCL1veQ?e=FsqdFA).

## Documentation

- [RNB Lib](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EWNBbUEbA5tLpctNqfadtgkBzdcLokwpXBGhRz0bFCw9QQ?e=5FbUsv)
- [MQTT API](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EedNfrVqV_xDj_XAf63u8q4BvzvoVIRBdpE716DOCL1veQ?e=FsqdFA)
