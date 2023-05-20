# CoAP Client
Sample code for the wireless node integrating RedNodeBus + OpenThread stack with a CoAP client. Compatible with `decawave_dwm1001_dev`,
`qorvo_dwm3001c_dev`, `insightsip_isp3010_dev`, `nrf52833dk_nrf52833`, `nrf52840dk_nrf52840`, `nrf52840dongle_nrf52840`, `nrf52832_mdk`, `rnl_w1` and `rnl_w1x` as BOARD_NAME.

```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

## Testing the CoAP Client
To test the CoAP client, the `coap_server.py` file located in the `script` folder can be used.

The IPv4 address (converted to an IPv6 equivalent) of the machine running the server needs to be specified in the coap_clients_util.c file:
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

Using the `coap_server.py` script in the same machine running the RNB OTBR docker (or one reachable through the IP network), the CoAP messages generated when pressing a button in the board can be received.
