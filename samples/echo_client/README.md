# Echo Client
Sample code for the wireless node integrating RedNodeBus + OpenThread stack with a UDP client socket. Compatible with `decawave_dwm1001_dev`,
`qorvo_dwm3001c_dev`, `insightsip_isp3010_dev`, `nrf52833dk_nrf52833`, `nrf52840dk_nrf52840`, `nrf52840dongle_nrf52840`, `nrf52832_mdk`, `rnl_w1` and `rnl_w1x` as BOARD_NAME.

```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

## Testing the Echo Client
To test the echo client, the `echo_server.py` file located in the `script` folder can be used.

Using the `echo_server.py` script in the same machine running the RNB OTBR docker (or one reachable through the IP network), the echo service can be tested.

The IPv4 address (converted to an IPv6 equivalent) of the machine running the `echo_server.py` can be specified in the `common.h` file:

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

## Using the RNB socket
To use the native RedNodeBus socket instead of the UDP socket, add the `overlay-rnb-user-payload.conf` to the compilation:
```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf overlay-rnb-user-payload.conf"
```

And use the `echo_server_rnb.py` script instead.
