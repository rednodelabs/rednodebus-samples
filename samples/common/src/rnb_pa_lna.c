/* rnb_pa_lna.c - RedNodeBus PA LNA code for node */

/*
 * Copyright (c) 2024 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <net/ieee802154_radio.h>
#include <drivers/gpio.h>
#if defined(CONFIG_BOARD_FANSTEL_BU840XE)
#include <nrfx_gpiote.h>
#endif

#if defined(CONFIG_BOARD_FANSTEL_BU840XE)
#define PA_LNA_CTX_PIN DT_GPIO_PIN(DT_NODELABEL(pa_lna), ctx_gpios)
#define PA_LNA_CRX_PIN DT_GPIO_PIN(DT_NODELABEL(pa_lna), crx_gpios)
#define PA_LNA_CHL_PIN DT_GPIO_PIN(DT_NODELABEL(pa_lna), chl_gpios)
#define PA_LNA_CPS_PIN DT_GPIO_PIN(DT_NODELABEL(pa_lna), cps_gpios)
#endif

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) ((const struct ieee802154_radio_api *const)(dev)->api)

int rnb_pa_lna_init(bool enabled)
{
	int ret = 0;

#if defined(CONFIG_BOARD_FANSTEL_BU840XE)
	const struct device *dev;
	struct rednodebus_pa_lna_config pa_lna_config;

	nrf_gpio_cfg_output(PA_LNA_CTX_PIN);
	nrf_gpio_cfg_output(PA_LNA_CRX_PIN);
	nrf_gpio_cfg_output(PA_LNA_CHL_PIN);
	nrf_gpio_cfg_output(PA_LNA_CPS_PIN);

	nrf_gpio_pin_clear(PA_LNA_CTX_PIN);
	nrf_gpio_pin_clear(PA_LNA_CRX_PIN);
	nrf_gpio_pin_set(PA_LNA_CHL_PIN);

	if (enabled)
	{
		nrf_gpio_pin_clear(PA_LNA_CPS_PIN);
	}
	else
	{
		nrf_gpio_pin_set(PA_LNA_CPS_PIN);
	}

	pa_lna_config.ctx_pin = PA_LNA_CTX_PIN;
	pa_lna_config.crx_pin = PA_LNA_CRX_PIN;
	pa_lna_config.enabled = enabled;

	dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	ret = REDNODEBUS_API(dev)->init_rnb_pa_lna_config(dev, &pa_lna_config);
#endif

	return ret;
}
