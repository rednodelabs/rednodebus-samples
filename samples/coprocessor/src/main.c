/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <logging/log.h>
#include <net/ieee802154_radio.h>
#include <zephyr/device.h>

#if defined(CONFIG_REDNODEBUS)
#include "rnb_leds.h"
#include "rnb_pa_lna.h"
#endif /* CONFIG_REDNODEBUS */

LOG_MODULE_REGISTER(coprocessor_sample, CONFIG_OT_COPROCESSOR_LOG_LEVEL);

#if defined(CONFIG_REDNODEBUS)
/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) ((const struct ieee802154_radio_api *const)(dev)->api)

static void handle_rnb_user_rxtx_signal(const struct device *dev,
					enum rednodebus_user_rxtx_signal sig,
					bool active);

static void handle_rnb_user_rxtx_signal(const struct device *dev,
					enum rednodebus_user_rxtx_signal sig,
					bool active)
{
	switch (sig)
	{
	case REDNODEBUS_USER_RX_SIGNAL:
		rnb_leds_set_tx(active);
		break;
	case REDNODEBUS_USER_TX_SIGNAL:
		rnb_leds_set_rx(active);
		break;
	default:
		break;
	}
}
#endif /* CONFIG_REDNODEBUS */

#if defined(CONFIG_REDNODEBUS_WATCHDOG)
const struct device *rnb_get_wdt(void)
{
	const struct device *wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
	return wdt;
}
#endif /* CONFIG_REDNODEBUS_WATCHDOG */

void main(void)
{
#if defined(CONFIG_REDNODEBUS)
	int ret;

	ret = rnb_leds_init();
	if (ret != 0)
	{
		LOG_WRN("Cannot init RedNodeBus LEDs");
	}

#if defined(CONFIG_BOARD_FANSTEL_BU840XE)
	ret = rnb_pa_lna_init(true);
	if (ret != 0)
	{
		LOG_WRN("Cannot init RedNodeBus PA LNA");
	}
#endif

	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	struct ieee802154_config ieee802154_config;

	ieee802154_config.rnb_user_rxtx_signal_handler = handle_rnb_user_rxtx_signal;
	REDNODEBUS_API(dev)->configure(dev, REDNODEBUS_CONFIG_USER_RXTX_SIGNAL_HANDLER, &ieee802154_config);
#endif /* CONFIG_REDNODEBUS */
}
