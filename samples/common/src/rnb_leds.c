/* rnb_leds.c - RedNodeBus LEDs code for node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

struct gpio_pin
{
	const char *const port;
	const uint8_t pin;
	const gpio_flags_t flags;
};

static uint8_t rnb_tx_led_idx = 0;

#if DT_NODE_EXISTS(DT_ALIAS(rnb_tx_led))
static uint8_t rnb_rx_led_idx = 1;
#else
static uint8_t rnb_rx_led_idx = 0;
#endif

static const struct gpio_pin led_pins[] = {
#if DT_NODE_EXISTS(DT_ALIAS(rnb_tx_led))
    {DT_GPIO_LABEL(DT_ALIAS(rnb_tx_led), gpios),
     DT_GPIO_PIN(DT_ALIAS(rnb_tx_led), gpios),
     DT_GPIO_FLAGS(DT_ALIAS(rnb_tx_led), gpios)},
#endif
#if DT_NODE_EXISTS(DT_ALIAS(rnb_rx_led))
    {DT_GPIO_LABEL(DT_ALIAS(rnb_rx_led), gpios),
     DT_GPIO_PIN(DT_ALIAS(rnb_rx_led), gpios),
     DT_GPIO_FLAGS(DT_ALIAS(rnb_rx_led), gpios)},
#endif
};

static const struct device *led_devs[ARRAY_SIZE(led_pins)];

static ALWAYS_INLINE void rnb_leds_set_led(uint8_t led_index, uint8_t value)
{
	if ((led_index < ARRAY_SIZE(led_pins)) && led_devs[led_index])
	{
		gpio_pin_set(led_devs[led_index], led_pins[led_index].pin, value);
	}
}

int rnb_leds_init()
{
	int err = -ENODEV;

	for (uint8_t i = 0; i < ARRAY_SIZE(led_pins); i++)
	{
		led_devs[i] = device_get_binding(led_pins[i].port);
		if (!led_devs[i])
		{
			return -ENODEV;
		}

		err = gpio_pin_configure(led_devs[i], led_pins[i].pin, GPIO_OUTPUT_INACTIVE | led_pins[i].flags);
		if (err)
		{
			led_devs[i] = NULL;

			return err;
		}

		err = 0;
	}

	return err;
}

void rnb_leds_set_tx(bool active)
{
	rnb_leds_set_led(rnb_tx_led_idx, active ? 1 : 0);
}

void rnb_leds_set_rx(bool active)
{
	rnb_leds_set_led(rnb_rx_led_idx, active ? 1 : 0);
}
