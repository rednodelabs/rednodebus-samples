#!/bin/bash

IMAGE='./build/zephyr/zephyr.hex'

IDS="$(nrfjprog --ids)"

for i in $(echo ${IDS} | tr "\n" "\n")
do
  echo "${i}"
  nrfjprog -f nrf52 --program ${IMAGE} --sectorerase -s ${i}
  nrfjprog -f nrf52 --reset -s ${i}
done
