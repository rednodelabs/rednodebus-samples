# RedNodeBus Samples

This repository includes a list of samples integrating OpenThread + RedNodeBus stack, based on [Zephyr OS](https://www.zephyrproject.org/).

The C and CXX compiler identification used for these samples is GNU 10.3.1.

## Prerequisites
The following tools are required:
* Git
* Python 3
* West [Zephyr’s meta-tool](https://docs.zephyrproject.org/latest/guides/west/index.html)
* [J-Link utilities](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)

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

## RNB OpenThread RCP Co-Processor
Sample code for the RCP architecture supported by OpenThread.

> For more information on this design, see [OpenThread](https://openthread.io/platforms#platform_designs)

This sample has been developed to be used with the following boards:

### nrf52840dk_nrf52840 board
#### With USB interface:
```
west build -p -b nrf52840dk_nrf52840 samples/coprocessor -- -DDTC_OVERLAY_FILE=usb.overlay -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf overlay-usb.conf"
```
```
west flash
```
#### With UART interface:
```
west build -p -b nrf52840dk_nrf52840 samples/coprocessor -- -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf"
```
```
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
You can find more details [here][j-link-ob].
[j-link-ob]: https://wiki.segger.com/J-Link_OB_SAM3U_NordicSemi#Hardware_flow_control_support

### nrf52840dongle_nrf52840 board
#### With USB interface:
```
west build -p -b nrf52840dongle_nrf52840 samples/coprocessor -- -DDTC_OVERLAY_FILE=usb.overlay -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf overlay-usb.conf"
```
```
west flash
```

## RNB Node
Sample code for the simplest wireless node integrating RedNodeBus + OpenThread stack.

This sample has been developed to be used with the following boards:

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
west flash
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
west flash
```

## CoAP Client
CoAP client sample code integrating RedNodeBus + OpenThread stack.

This sample has been developed to be used with the following boards:

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
In case a previous OT configuration has been programmed in the used board, we recommend to erase the flash completely.
In this way, we can assure the new OT configuration will be written correctly:
```
nrfjprog -e
```
Once the flash has been completely erased, the new firmware image can be flashed:
```
west flash
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
west flash
```

To test the coap client, the coap_server.py script located in the script folder can be used.

The IPv4 address (converted to an IPv6 equivalent) of the machine running the server needs to be specified 
in the coap_clients_util.c file:
```
#if HARDCODED_UNICAST_IP
static struct sockaddr_in6 unique_local_addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(COAP_PORT),
        .sin6_addr.s6_addr = { 0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0xb2, 0x85 },
        .sin6_scope_id = 0U
};
```
For example, if the server runs in the following local IP: 
```
192.168.178.133
```
First it needs to be converted to IPv6 using an [IPv4 to IPv6 converter](https://iplocation.io/ipv4-to-ipv6/), which gives:
```
c0a8:b285
```
Then, the prefix `2001:db8:1:ffff::` must be added, resulting in the following IP: 
```
2001:0db8:0001:ffff:0000:0000:c0a8:b285
```

## RedNodeBus + OpenThread Border Router (RNB OTBR)

In order to run the RedNodeBus services in the edge platform, RedNodeLabs provides a
docker container based on the [OpenThread Border Router (OTBR)](https://openthread.io/guides/border-router)
for Raspberry Pi (models 3B+, 4).

This container offers the OpenThread network services in combination with the RedNodeBus API based on MQTT.

Please, consider that this service requires to purchase a license per edge device.
For further details, contact RedNodeLabs.

### Raspberry Pi setup
Follow the instructions described in [OpenThread](https://openthread.io/guides/border-router/docker)
to set up your Raspberry Pi and install Docker tool.

### Run RNB OTBR Docker
Firstly, pull the RNB OTBR from RedNodeLabs docker repository:
```
docker pull rednodelabs/otbr:dev-0.9.4
```

RNB OTBR requires the [RCP](###coprocessor) node in order to form a Thread network and offer RedNodeBus services.

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
docker run --sysctl "net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1" -p 1883:1883 -p 3000:3000 --dns=127.0.0.1 -it -v /home/pi/rnl_certs:/app/config -v /dev/ttyACM0:/dev/ttyACM0 --privileged rednodelabs/otbr:dev-0.9.4
```

Notice that the first time you connect the Raspberry Pi it will require Internet access to download the unique device certificate. Please consider to persist these files in a secure and safe place for each edge device. For further details regarding the provisioning process, please read [RedNodeLabs Device Provisioing](PROVISIONING.md).

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

On the Raspberry Pi running RNB OTBR Docker, open a browser window and navigate to 127.0.0.1:3000.
If RNB OTBR Docker is running correctly, the RNB OTBR Web GUI loads.

### MQTT API Specification

Corresponding version of the API documentation can be downloaded [here](https://netorgft3728920-my.sharepoint.com/:b:/g/personal/info_rednodelabs_com/EWfUvttx3k9Cst-vFau9MhEB8-xD40dfypIrbi8DLmcEJg). 