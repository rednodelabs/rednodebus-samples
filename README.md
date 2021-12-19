# RedNodeBus

## Init
```
mkdir zephyr-workspace
cd zephyr-workspace
west init -m git@github.com:rednodelabs/rednodebus.git
west update
```


## Build

### echo-server
```
west build -p -b nrf52840dk_nrf52840 samples/echo_server -- -DOVERLAY_CONFIG="overlay-rednodebus.conf overlay-rednodebus-logger.conf"
```
or
```
west build -p -b nrf52840dongle_nrf52840 samples/echo_server -- -DOVERLAY_CONFIG="overlay-rednodebus.conf overlay-rednodebus-logger.conf"
```
or
```
west build -p -b nrf52840dongle_nrf52840_april samples/echo_server -- -DOVERLAY_CONFIG="overlay-rednodebus.conf overlay-rednodebus-logger.conf"
```
For RTT debugging, add:
```
overlay-rednodebus-debug.conf
```
For OT, add:
```
CONFIG_NET_L2_IEEE802154=n
CONFIG_REDNODEBUS_EMPTY_PAYLOAD=n
CONFIG_REDNODEBUS_REDNODERANGING_MAX_REPORTER_COUNT=2
overlay-rednodebus-ot.conf
```

### echo-client
```
west build -p -b decawave_dwm1001_dev samples/echo_client -- -DOVERLAY_CONFIG="overlay-rednodebus.conf"
```
or
```
west build -p -b insightsip_isp3010_dev samples/echo_client -- -DOVERLAY_CONFIG="overlay-rednodebus.conf"
```
For RTT debugging, add:
```
overlay-rednodebus-debug.conf
```
For OT, add:
```
CONFIG_NET_L2_IEEE802154=n
CONFIG_REDNODEBUS_EMPTY_PAYLOAD=n
CONFIG_REDNODEBUS_REDNODERANGING_MAX_REPORTER_COUNT=2
overlay-rednodebus-ot.conf
```

### coprocessor
```
west build -p -b nrf52840dk_nrf52840 samples/coprocessor -- -DOVERLAY_CONFIG="overlay-rednodebus-rcp.conf overlay-rednodebus-vendor_hook.conf overlay-rednodelabs-usb.conf overlay-logging.conf"
```

### ot-br-posix
```
./script/bootstrap
BACKBONE_ROUTER=0 OTBR_OPTIONS='-DOT_SPINEL_RESET_CONNECTION=ON -DOT_THREAD_VERSION=1.1 -DOT_BACKBONE_ROUTER=OFF -DOT_DAEMON=ON -DOT_FULL_LOGS=ON' ./script/setup
```
```
sudo ./build/otbr/src/agent/otbr-agent -I wpan0 -v 'spinel+hdlc+uart:///dev/serial/by-id/usb-RedNodeLabs_Thread_Co-Processor_0-if00?uart-baudrate=1000000' -d 6
```
```
sudo ./build/otbr/third_party/openthread/repo/src/posix/ot-ctl log level 5
```
