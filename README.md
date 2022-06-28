![logo](misc/logo.png)

# RedNodeBus Samples

This repository includes a list of samples integrating OpenThread + RedNodeBus (RNB) stack, based on [Zephyr OS](https://www.zephyrproject.org/).

## Prerequisites
The following tools are required:
* Git
* Python 3
* West [Zephyr’s meta-tool](https://docs.zephyrproject.org/latest/guides/west/index.html)
* [J-Link utilities](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)

The C and CXX compiler identification used for these samples is GNU 10.3.1.

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

To test the system, flash either the [RNB Node](#rnb-node), the [CoAP Client](#coap-client) or the [Echo Client](#echo-client) sample in the wireless nodes, and run the 
[RNB OTBR](#rednodebus--openthread-border-router-rnb-otbr) docker. Once running, interact with the system using the [MQTT API](#mqtt-api-specification).

## RNB Lib Configuration
In case a previous OpenThread (OT) configuration has been programmed in the board, we recommend erasing the flash completely.
This way, we can assure that the new OT configuration will be written correctly:
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

To enable RedNodeBus ranging diagnostic, add the overlay to `OVERLAY_CONFIG` when building the sample projects as following: 
`../common/overlay-rednodebus-ranging-diagnostic.conf`. For building the radio coprocessor (RCP) this is not required.

## RNB Node
Sample code for the wireless node integrating RedNodeBus + OpenThread stack.

This sample has been developed to be used with the following boards:

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dk_nrf52840 board
```
west build -p -b nrf52840dk_nrf52840s samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dongle_nrf52840 board
```
west build -p -b nrf52840dongle_nrf52840 samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

## CoAP Client
Sample code for the wireless node integrating RedNodeBus + OpenThread stack with a CoAP client.

This sample has been developed to be used with the following boards:

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dk_nrf52840 board
```
west build -p -b nrf52840dk_nrf52840 samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dongle_nrf52840 board
```
west build -p -b nrf52840dongle_nrf52840 samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### Testing the CoAP Client
To test the CoAP client, the `coap_server.py` file located in the `script` folder can be used.

The IPv4 address (converted to an IPv6 equivalent) of the machine running the server needs to be specified 
in the coap_clients_util.c file:
```
#if HARDCODED_UNICAST_IP
static struct sockaddr_in6 unique_local_addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(COAP_PORT),
        .sin6_addr.s6_addr = { 0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0xac, 0x11, 0x00, 0x01 },
        .sin6_scope_id = 0U
};
```
For example, if the server runs in the following local IP: 
```
172.17.0.1
```
First, we need to take the last 32 bits from the conversion to IPv6 using an [IPv4 to IPv6 converter](https://iplocation.io/ipv4-to-ipv6/):
```
ac11:0001
```
Then, the prefix `2001:db8:1:ffff::` must be added, resulting in the following IP: 
```
2001:0db8:0001:ffff:0000:0000:ac11:0001
```

Using the `coap_server.py` script in the same machine running the RNB OTBR docker (or one reachable through the IP network), 
the CoAP messages generated when pressing a button in the board can be received.

## Echo Client
Sample code for the wireless node integrating RedNodeBus + OpenThread stack with a UDP client socket.

This sample has been developed to be used with the following boards:

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev samples/echo_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev samples/echo_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dk_nrf52840 board
```
west build -p -b nrf52840dk_nrf52840 samples/echo_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dongle_nrf52840 board
```
west build -p -b nrf52840dongle_nrf52840 samples/echo_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### Testing the Echo Client
To test the echo client, the `udp_server.py` file located in the `script` folder can be used.

Using the `udp_server.py` script in the same machine running the RNB OTBR docker (or one reachable through the IP network), 
the echo service can be tested. 

Similarly as in the [CoAP Client](#coap-client), the IPv6-equivalent of the IPv4 of the machine running the `udp_server.py` 
can be specified in the `common.h` file:
```
#define CONFIG_NET_CONFIG_PEER_IPV6_ADDR "2001:0db8:0001:ffff:0000:0000:ac11:0001"
```

## RedNodeBus + OpenThread Border Router (RNB OTBR)

To run the RedNodeBus services in the edge platform, RedNodeLabs provides a
docker container based on the [OpenThread Border Router (OTBR)](https://openthread.io/guides/border-router)
for Raspberry Pi (models 3B+, 4).

This container offers the OpenThread network services in combination with the RedNodeBus API based on MQTT.

Please, consider that this service requires getting a license. For further details, contact RedNodeLabs.

### Setting-Up the Raspberry Pi
Follow the instructions described in [OpenThread](https://openthread.io/guides/border-router/docker#raspberry_pi_setup)
to set up your Raspberry Pi and install Docker tool.

### Setting-Up the RNB OTBR Docker
Firstly, pull the RNB OTBR from RedNodeLabs docker repository:
```
docker pull rednodelabs/otbr:dev-0.9.8
```

> The version of the RNB OTBR Docker must match the version of the samples flashed in the nodes, i.e. v0.9.8, otherwise they will not be compatible!

RNB OTBR requires our radio coprocessor (RCP) sample in order to form a Thread network and offer the RedNodeBus services. 
This sample has been developed to be used with the following boards:

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
##### Mass Storage Device known issue (only for UART interface)
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

##### Hardware Flow Control detection (only for UART interface)
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
After building and flashing, attach the RCP device to the Raspberry Pi running RNB OTBR Docker via UART or USB.
Determine the serial port name for the RCP device by checking /dev:

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

Start RNB OTBR Docker, referencing the RCP's serial port and the folder where the credentials are stored. 
For example, if the RCP is mounted at `/dev/ttyACM0` and the certificates are in `/home/pi/rnl_certs`:
```
docker run --sysctl "net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1" -p 1883:1883 -p 3000:3000 --dns=127.0.0.1 -it -v /home/pi/rnl_certs:/app/config -v /dev/ttyACM0:/dev/ttyACM0 --privileged rednodelabs/otbr:dev-0.9.8
```

Notice that the first time you connect the Raspberry Pi it will require Internet access to download the unique device certificate. 
For further details regarding the provisioning process, please read [RedNodeLabs Device Provisioing](PROVISIONING.md).

Upon success, you should have output similar to this:
```
WARNING: Localhost DNS setting (--dns=127.0.0.1) may fail in containers.
RADIO_URL: spinel+hdlc+uart:///dev/ttyACM0
TUN_INTERFACE_NAME: wpan0
NAT64_PREFIX: 64:ff9b::/96
AUTO_PREFIX_ROUTE: true
AUTO_PREFIX_SLAAC: true
Current platform is ubuntu
* Applying /etc/sysctl.d/10-console-messages.conf ...
kernel.printk = 4 4 1 7
* Applying /etc/sysctl.d/10-ipv6-privacy.conf ...
net.ipv6.conf.all.use_tempaddr = 2
net.ipv6.conf.default.use_tempaddr = 2
* Applying /etc/sysctl.d/10-kernel-hardening.conf ...
kernel.kptr_restrict = 1
* Applying /etc/sysctl.d/10-link-restrictions.conf ...
fs.protected_hardlinks = 1
fs.protected_symlinks = 1
* Applying /etc/sysctl.d/10-magic-sysrq.conf ...
kernel.sysrq = 176
* Applying /etc/sysctl.d/10-network-security.conf ...
net.ipv4.conf.default.rp_filter = 1
net.ipv4.conf.all.rp_filter = 1
net.ipv4.tcp_syncookies = 1
* Applying /etc/sysctl.d/10-ptrace.conf ...
kernel.yama.ptrace_scope = 1
* Applying /etc/sysctl.d/10-zeropage.conf ...
vm.mmap_min_addr = 65536
* Applying /etc/sysctl.d/60-otbr-ip-forward.conf ...
net.ipv6.conf.all.forwarding = 1
net.ipv4.ip_forward = 1
* Applying /etc/sysctl.conf ...
* Starting userspace NAT64 tayga             [ OK ]
/usr/sbin/service
* Starting domain name service... bind9      [ OK ]
/usr/sbin/service
* dbus is not running
* Starting system message bus dbus           [ OK ]
...fail!
otWeb[155]: border router web started on wpan0
otbr-agent[224]: Thread interface wpan0
otbr-agent[224]: Thread is down
otbr-agent[224]: Check if Thread is up: OK
otbr-agent[224]: Stop publishing service
otbr-agent[224]: PSKc is not initialized
otbr-agent[224]: Check if PSKc is initialized: OK
otbr-agent[224]: Initialize OpenThread Border Router Agent: OK
otbr-agent[224]: Border router agent started.
```

RNB OTBR Docker is now running. Leave this terminal window open and running in the background. 
If you quit the process or close the window, RNB OTBR Docker will go down.

On the Raspberry Pi running RNB OTBR Docker, open a browser and go to `127.0.0.1:3000`.
If the Docker is running correctly, the management Web GUI loads and the MQTT API can be used to interact with the system.

### MQTT API Specification

Corresponding version of the API documentation can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/Eau4K5sZcTVBv6vB9MY4OO0BhtaAsPStyznbmRZ81zcBTQ?e=chDzPi). 