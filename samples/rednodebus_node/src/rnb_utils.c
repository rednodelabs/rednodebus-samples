/* rnb.c - RedNodeBus specific code for node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(rednodebus_node, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/ieee802154_radio.h>

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) \
		((const struct ieee802154_radio_api * const)(dev)->api)

void print_rnb_state(const uint8_t state)
{
	switch (state)
	{
	case REDNODEBUS_USER_BUS_STATE_CONNECTED:
		LOG_INF("RNB state: CONNECTED");
		break;
	case REDNODEBUS_USER_BUS_STATE_SYNCHRONIZED:
		LOG_INF("RNB state: SYNCHRONIZED");
		break;
	case REDNODEBUS_USER_BUS_STATE_UNSYNCHRONIZED:
		LOG_INF("RNB state: UNSYNCHRONIZED");
		break;
	case REDNODEBUS_USER_BUS_STATE_STOPPED:
		LOG_INF("RNB state: STOPPED");
		break;
	default:
		LOG_WRN("RNB state: unknown");
		break;
	}
}

void print_rnb_role(const uint8_t role)
{
	switch (role)
	{
	case REDNODEBUS_USER_ROLE_TAG:
		LOG_INF("RNB role: TAG");
		break;
	case REDNODEBUS_USER_ROLE_ANCHOR:
		LOG_INF("RNB role: ANCHOR");
		break;
	case REDNODEBUS_USER_ROLE_UNDEFINED:
		LOG_INF("RNB role: UNDEFINED");
		break;
	default:
		LOG_WRN("RNB role: unknown");
		break;
	}
}

void print_rnb_uwb_mode(const uint8_t uwb_mode)
{
	switch (uwb_mode)
	{
	case REDNODEBUS_USER_UWB_MODE_HIGH_RATE_CTXP:
		LOG_INF("RNB UWB mode: HIGH_RATE_CTXP");
		break;
	case REDNODEBUS_USER_UWB_MODE_LONG_RANGE_CTXP:
		LOG_INF("RNB UWB mode: LONG_RANGE_CTXP");
		break;
	case REDNODEBUS_USER_UWB_MODE_HIGH_RATE_STXP:
		LOG_INF("RNB UWB mode: HIGH_RATE_STXP");
		break;
	case REDNODEBUS_USER_UWB_MODE_LONG_RANGE_STXP:
		LOG_INF("RNB UWB mode: LONG_RANGE_STXP");
		break;
	case REDNODEBUS_USER_UWB_MODE_NONE:
		LOG_INF("RNB UWB mode: MODE_NONE");
		break;
	default:
		LOG_WRN("RNB UWB mode: unknown");
		break;
	}
}

void print_rnb_ranging_mode(const uint8_t ranging_mode)
{
	switch (ranging_mode)
	{
	case REDNODEBUS_USER_RANGING_MODE_2W_T2A:
		LOG_INF("RNB ranging mode: 2W_T2A");
		break;
	case REDNODEBUS_USER_RANGING_MODE_2W_A2T:
		LOG_INF("RNB ranging mode: 2W_A2T");
		break;
	case REDNODEBUS_USER_RANGING_MODE_3W_A2T:
		LOG_INF("RNB ranging mode: HIGH_RATE_STXP");
		break;
	case REDNODEBUS_USER_RANGING_MODE_3W_A2A:
		LOG_INF("RNB ranging mode: 3W_A2T");
		break;
	case REDNODEBUS_USER_RANGING_MODE_DISABLED:
		LOG_INF("RNB ranging mode: DISABLED");
		break;
	default:
		LOG_WRN("RNB ranging mode: unknown");
		break;
	}
}

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
		LOG_DBG("RNB user event");

		print_rnb_state(rnb_user_event_params->state);

		if (rnb_user_event_params->state > REDNODEBUS_USER_BUS_STATE_UNSYNCHRONIZED)
		{
			if (rnb_user_event_params->role != REDNODEBUS_USER_ROLE_UNDEFINED)
			{
				print_rnb_role(rnb_user_event_params->role);
			}

			LOG_INF("RNB period ms %u", rnb_user_event_params->period_ms);
		}

		if (rnb_user_event_params->ranging_mode != REDNODEBUS_USER_RANGING_MODE_DISABLED)
		{
			print_rnb_uwb_mode(rnb_user_event_params->uwb_mode);
			print_rnb_ranging_mode(rnb_user_event_params->ranging_mode);
		}
		break;
	default:
		LOG_WRN("RNB user event: unknown");
		break;
	}
}

int init_rnb(void)
{
	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	struct ieee802154_config ieee802154_config;
	struct rednodebus_user_config rnb_user_config;
	const uint8_t network_id = 0;

	ieee802154_config.rnb_user_event_handler = handle_radio_rnb_user_event;
	REDNODEBUS_API(dev)->configure(dev, REDNODEBUS_CONFIG_USER_EVENT_HANDLER, &ieee802154_config);

	rnb_user_config.sync_active_period_ms = 100;
	rnb_user_config.sync_sleep_period_ms = 1000;
	rnb_user_config.ranging_enabled = true;
	rnb_user_config.ranging_period_ms = 0;
	REDNODEBUS_API(dev)->configure_rnb(dev, &rnb_user_config);

	REDNODEBUS_API(dev)->start_rnb(dev, network_id, REDNODEBUS_USER_ROLE_UNDEFINED);

	return 0;
}
