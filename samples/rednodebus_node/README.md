# RedNodeBus Node
Sample code for the wireless node integrating RedNodeBus + OpenThread stack. Compatible with decawave_dwm1001_dev,
qorvo_dwm3001c_dev, insightsip_isp3010_dev, nrf52833dk_nrf52833, nrf52840dk_nrf52840 and nrf52840dongle_nrf52840.

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### qorvo_dwm3001c_dev board
```
west build -p -b qorvo_dwm3001c_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### nrf52840dk_nrf52840 board
```
west build -p -b nrf52840dk_nrf52840 . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

### nrf52840dongle_nrf52840 board
```
west build -p -b nrf52840dongle_nrf52840 . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf"
```

## Ranging diagnostic

### decawave_dwm1001_dev board
```
west build -p -b decawave_dwm1001_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```

### qorvo_dwm3001c_dev board
```
west build -p -b qorvo_dwm3001c_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```

### insightsip_isp3010_dev board
```
west build -p -b insightsip_isp3010_dev . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```

### nrf52840dk_nrf52840 board
```
west build -p -b nrf52840dk_nrf52840 . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```

### nrf52840dongle_nrf52840 board
```
west build -p -b nrf52840dongle_nrf52840 . -- -DOVERLAY_CONFIG="overlay-ot-rnb.conf ../common/overlay-rnb-ranging-diagnostic.conf"
```
