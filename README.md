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
west init -m git@github.com:rednodelabs/rednodebus-samples.git
```
```
west update
```

> To update to a newer release, remember to perform both a `git pull` in the `rednodebus-samples` folder inside `zephyr-workspace` and a `west update` to update the dependencies.

## RNB Lib Documentation
The documentation of the RNB Lib can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EVdIIvr2xPhOuW6Um5m6NfEBySMIM3ZZPjGyQ8J50eUucA?e=HRdwOT).

To test the system, flash either the [RNB Node](samples/rednodebus_node/README.md), the [CoAP Client](samples/coap_client/README.md), the [Echo Client](samples/echo_client/README.md), the [Accelerometer](samples/accelerometer/README.md) or the [Socket Test](samples/socket_test/README.md) sample in the wireless nodes, and run the [RNB OTBR](#rednodebus--openthread-border-router-rnb-otbr) docker. Once running, interact with the system using the [MQTT API](#mqtt-api-specification).

## Integrating RedNodeBus with the OpenThread Border Router (RNB OTBR)

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

If other configurations are required, e.g. different UART pins or baudrates, follow the build instructions [here](samples/coprocessor/README.md).

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

> The web UI can be accessed from a different machine in the same local network as the Raspberry Pi, e.g. entering `raspberrypi:3000`, to avoid the load of the graphical interface in the Pi.

The docker log file is stored in the mounted volume, i.e. `/home/pi/rnl_certs/log/syslog`.

## MQTT API Documentation

The MQTT API can be also used to interact with the system. For further details, read the API documentation.

The corresponding version can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EdGv-n0aNT5Glp43ALY1OkgBFfxalbSmV6y2VABwifQULw?e=5ndMeu).
