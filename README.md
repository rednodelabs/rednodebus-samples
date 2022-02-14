# RedNodeBus Samples

This repository includes a list of samples integrating OpenThread + RedNodeBus stack, based on [Zephyr OS](https://www.zephyrproject.org/).


## Prerequisites
The following tools are required:
* Git
* Python 3
* West [Zephyr’s meta-tool](https://docs.zephyrproject.org/latest/guides/west/index.html)
* [J-Link utilities](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)


## Init
The project needs to be initialized as usual using **west**.
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

Before building the samples, do not forget to set the environment variables:
```
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=<PATH>
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
#### With UART interface:
```
west build -p -b nrf52840dk_nrf52840 samples/coprocessor -- -DOVERLAY_CONFIG="overlay-rcp-rnb.conf overlay-vendor_hook-rnb.conf"
```
```
west flash
```
##### Mass Storage Device known issue
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

##### Hardware Flow Control detection
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

> For more information regarding RedNodeBus API, see [ieee802154_radio.h](https://github.com/rednodelabs/zephyr/blob/main/include/net/ieee802154_radio.h)

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

> For more information regarding RedNodeBus API, see [ieee802154_radio.h](https://github.com/rednodelabs/zephyr/blob/main/include/net/ieee802154_radio.h)

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev samples/coap_client -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
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

## RedNodeBus + OpenThread Border Router (RNB OTBR)

In order to run the RedNodeBus services in the edge platform, RedNodeLabs provides a
docker container based on the [OpenThread Border Router (OTBR)](https://openthread.io/guides/border-router)
for Raspberry Pi (models 3B+, 4).

This container offers the OpenThread network services in combination with the RedNodeBus API based on MQTT.

### Raspberry Pi setup
Please follow the instructions described in [OpenThread](https://openthread.io/guides/border-router/docker)
to set up your Raspberry Pi and install Docker tool.

### Run RNB OTBR Docker
Firstly, pull the RNB OTBR from RedNodeLabs docker repository:
```
docker pull rednodelabs/otbr:dev-0.9.1
```

RNB OTBR requires the [RCP](###coprocessor) node in order to form a Thread network and offer RedNodeBus services.

After building and flashing, attach the RCP device to the Raspberry Pi running RNB OTBR Docker via UART (J-Link USB).
Determine the serial port name for the RCP device by checking /dev:

```
$ ls /dev/tty*
/dev/ttyACMO
```

Start RNB OTBR Docker, referencing the RCP's serial port. For example, if the RCP is mounted at /dev/ttyACM0:
```
docker run --sysctl "net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1" -p 1883:1883 -p 3000:3000 --dns=127.0.0.1 -it -v /dev/ttyACM0:/dev/ttyACM0 --privileged rednodelabs/otbr:dev-0.9.1
```

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

RNB OTBR Docker is now running. Leave this terminal window open and running in the background. If you quit the process or close the window, RNB OTBR Docker will go down.

On the Raspberry Pi running RNB OTBR Docker, open a browser window and navigate to 127.0.0.1:3000.
If RNB OTBR Docker is running correctly, the RNB OTBR Web GUI loads.
