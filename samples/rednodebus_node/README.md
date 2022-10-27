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
west build -p -b nrf52840dk_nrf52840 samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
```
nrfjprog -e
west flash
```

### nrf52840dongle_nrf52840 board
```
west build -p -b nrf52840dongle_nrf52840 samples/rednodebus_node -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```
