# RedNodeBus Node
Sample code for the wireless node integrating RedNodeBus + OpenThread stack. Compatible with `decawave_dwm1001_dev`,
`qorvo_dwm3001c_dev`, `insightsip_isp3010_dev`, `nrf52833dk_nrf52833`, `nrf52840dk_nrf52840`, `nrf52840dongle_nrf52840`, `nrf52832_mdk`, `rnl_w1` and `rnl_w1x` as BOARD_NAME.

```
west build -p -b BOARD_NAME . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
