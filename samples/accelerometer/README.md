# Accelerometer
Sample code for the wireless node integrating RedNodeBus + OpenThread stack with a UDP client socket sending the accelerometer values. Compatible with decawave_dwm1001_dev and qorvo_dwm3001c_dev.

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### qorvo_dwm3001c_dev board
```
west build -p -b qorvo_dwm3001c_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

## Testing the Accelerometer
To test the echo client, the `accelerometer.py` file located in the `script` folder can be used.

Using the `accelerometer.py` script in the same machine running the RNB OTBR docker (or one reachable through the IP network), the echo service can be tested.

The IPv4 address (converted to an IPv6 equivalent) of the machine running the `accelerometer.py` can be specified in the `common.h` file:

For example, if the server runs in the following local IP (default Docker IP):
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

Finally, we specify it in the corresponding define in `common.h`:
```
#define CONFIG_NET_CONFIG_PEER_IPV6_ADDR "2001:0db8:0001:ffff:0000:0000:ac11:0001"
```

## Accelerometer Configuration
It is possible to set different configurations for the accelerometer. To change them, add the necesary configs in the file `prj.conf`. Check the available configurations for in `zephyr/drivers/sensor/lis2dh/Kconfig`. By default, the accelerometer is only activer if the board acts as a TAG.