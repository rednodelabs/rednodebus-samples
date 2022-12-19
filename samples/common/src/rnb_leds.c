/* rnb_leds.c - RedNodeBus LEDs code for node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <drivers/gpio.h>

#if DT_NODE_EXISTS(DT_ALIAS(rnb_tx_led))
static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(DT_ALIAS(rnb_tx_led), gpios);
#endif

#if DT_NODE_EXISTS(DT_ALIAS(rnb_rx_led))
static const struct gpio_dt_spec led_rx = GPIO_DT_SPEC_GET(DT_ALIAS(rnb_rx_led), gpios);
#endif

int rnb_leds_init()
{
	int err = 0;
#if DT_NODE_EXISTS(DT_ALIAS(rnb_tx_led))
	err = gpio_pin_configure_dt(&led_tx, GPIO_OUTPUT_INACTIVE);
	if (err < 0)
	{
		return err;
	}
#endif
#if DT_NODE_EXISTS(DT_ALIAS(rnb_rx_led))
	err = gpio_pin_configure_dt(&led_rx, GPIO_OUTPUT_INACTIVE);
	if (err < 0)
	{
		return err;
	}
#endif
	return err;
}

void rnb_leds_set_tx(bool active)
{
#if DT_NODE_EXISTS(DT_ALIAS(rnb_tx_led))
	gpio_pin_set_dt(&led_tx, active);
#endif
}

void rnb_leds_set_rx(bool active)
{
#if DT_NODE_EXISTS(DT_ALIAS(rnb_rx_led))
	gpio_pin_set_dt(&led_rx, active);
#endif
}
