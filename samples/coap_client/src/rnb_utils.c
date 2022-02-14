/* rnb.c - RedNodeBus specific code for node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/ieee802154_radio.h>

LOG_MODULE_REGISTER(rnb, CONFIG_RNB_UTILS_LOG_LEVEL);

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) \
		((const struct ieee802154_radio_api * const)(dev)->api)


void handle_radio_rnb_user_event(const struct device *dev,
      enum rednodebus_user_event evt,
      void *event_params)
{
	ARG_UNUSED(dev);

	const struct rednodebus_user_event_params *rnb_user_event_params =
			(const struct rednodebus_user_event_params *) event_params;

	switch (evt)
	{
	case REDNODEBUS_USER_EVENT_NEW_STATE:
		LOG_INF("RNB user event");
		LOG_DBG("RNB state %u", rnb_user_event_params->state);
		LOG_DBG("RNB role %u", rnb_user_event_params->role);
		LOG_DBG("RNB period ms %u", rnb_user_event_params->period_ms);
		LOG_DBG("RNB UWB mode %u", rnb_user_event_params->uwb_mode);
		LOG_DBG("RNB ranging mode %u", rnb_user_event_params->ranging_mode);
		break;
	default:
		LOG_WRN("RNB user event unknown");
		break;
	}
}

int init_rnb(void)
{
	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	struct ieee802154_config ieee802154_config;
	struct rednodebus_user_config rnb_user_config;

	ieee802154_config.rnb_user_event_handler = handle_radio_rnb_user_event;
	REDNODEBUS_API(dev)->configure(dev, REDNODEBUS_CONFIG_USER_EVENT_HANDLER, &ieee802154_config);

	rnb_user_config.ranging_enabled = true;
	rnb_user_config.ranging_period_ms = 0;
	REDNODEBUS_API(dev)->configure_rnb(dev, &rnb_user_config);

	REDNODEBUS_API(dev)->start_rnb(dev, REDNODEBUS_USER_ROLE_UNDEFINED);

	return 0;
}
