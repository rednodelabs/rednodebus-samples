/* rnb_utils.c - RedNodeBus utilities code for node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(rnb_utils, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/ieee802154_radio.h>
#include <net/openthread.h>
#include <openthread/platform/radio.h>

#include "rnb_utils.h"

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) ((const struct ieee802154_radio_api *const)(dev)->api)

struct rednodebus_utils_event
{
	void *fifo_reserved; /* 1st word reserved for use by fifo. */
	const struct device *dev;
	enum rednodebus_user_event event;
	struct rednodebus_user_event_params params;
	bool in_use;
};

/* RedNodeBus utils stack. */
static K_KERNEL_STACK_MEMBER(rnb_utils_stack, REDNODEBUS_UTILS_STACK_SIZE);

/* RedNodeBus utils thread control block. */
static struct k_thread rnb_utils_thread;

/* RedNodeBus utils event fifo queue. */
static struct k_fifo rnb_utils_event_fifo;

static struct rednodebus_utils_event rnb_utils_events[REDNODEBUS_UTILS_EVENT_BUFFER_SIZE];
static bool rnb_configured;
static struct rednodebus_user_config rnb_user_config;
static struct rednodebus_user_runtime_config rnb_user_runtime_config;
static bool rnb_connected;

static void print_rnb_state(const uint8_t state);
static void print_rnb_role(const uint8_t role);
static void print_rnb_uwb_mode(const uint8_t uwb_mode);
static void print_rnb_ranging_mode(const uint8_t ranging_mode);
static void handle_rnb_user_event(const struct device *dev,
				  enum rednodebus_user_event evt,
				  void *event_params);
static void process_rnb_utils_event(const struct device *dev,
				    const enum rednodebus_user_event event,
				    const struct rednodebus_user_event_params *params);
static void rnb_utils_process_thread(void *arg1, void *arg2, void *arg3);

static void print_rnb_state(const uint8_t state)
{
	switch (state)
	{
	case REDNODEBUS_USER_BUS_STATE_CONNECTED:
		rnb_connected = true;

		LOG_INF("RNB state: CONNECTED");
		break;
	case REDNODEBUS_USER_BUS_STATE_SYNCHRONIZED:
		LOG_INF("RNB state: SYNCHRONIZED");
		break;
	case REDNODEBUS_USER_BUS_STATE_UNSYNCHRONIZED:
		LOG_INF("RNB state: UNSYNCHRONIZED");
		break;
	case REDNODEBUS_USER_BUS_STATE_STOPPED:
		rnb_connected = false;

		LOG_INF("RNB state: STOPPED");
		break;
	default:
		LOG_WRN("RNB state: unknown");
		break;
	}
}

static void print_rnb_role(const uint8_t role)
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

static void print_rnb_uwb_mode(const uint8_t uwb_mode)
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

static void print_rnb_ranging_mode(const uint8_t ranging_mode)
{
	switch (ranging_mode)
	{
	case REDNODEBUS_USER_RANGING_MODE_2W_T2A:
		LOG_INF("RNB ranging mode: 2W_T2A");
		break;
	case REDNODEBUS_USER_RANGING_MODE_2W_A2T:
		LOG_INF("RNB ranging mode: 2W_A2T");
		break;
	case REDNODEBUS_USER_RANGING_MODE_2W_A2A:
		LOG_INF("RNB ranging mode: 2W_A2A");
		break;
	case REDNODEBUS_USER_RANGING_MODE_3W_A2T:
		LOG_INF("RNB ranging mode: 3W_A2T");
		break;
	case REDNODEBUS_USER_RANGING_MODE_3W_A2A:
		LOG_INF("RNB ranging mode: 3W_A2A");
		break;
	case REDNODEBUS_USER_RANGING_MODE_DISABLED:
		LOG_INF("RNB ranging mode: DISABLED");
		break;
	default:
		LOG_WRN("RNB ranging mode: unknown");
		break;
	}
}

static void handle_rnb_user_event(const struct device *dev,
				  enum rednodebus_user_event evt,
				  void *event_params)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(rnb_utils_events); i++)
	{
		if (rnb_utils_events[i].in_use)
		{
			continue;
		}

		rnb_utils_events[i].in_use = true;

		rnb_utils_events[i].dev = dev;
		rnb_utils_events[i].event = evt;

		memcpy(&rnb_utils_events[i].params,
		       event_params,
		       sizeof(struct rednodebus_user_event_params));

		k_fifo_put(&rnb_utils_event_fifo, &rnb_utils_events[i]);

		return;
	}

	LOG_ERR("Not enough events allocated for rnb user event %u", evt);
}

static void process_rnb_utils_event(const struct device *dev,
				    const enum rednodebus_user_event event,
				    const struct rednodebus_user_event_params *params)
{
	int ret;

	switch (event)
	{
	case REDNODEBUS_USER_EVENT_NEW_STATE:
		LOG_DBG("RNB user event new state");

		print_rnb_state(params->state);

		if (params->state > REDNODEBUS_USER_BUS_STATE_UNSYNCHRONIZED)
		{
			if (params->role != REDNODEBUS_USER_ROLE_UNDEFINED)
			{
				print_rnb_role(params->role);
			}

			LOG_INF("RNB period ms %u", params->period_ms);
		}
		else if (params->state == REDNODEBUS_USER_BUS_STATE_STOPPED)
		{
			if (!rnb_configured)
			{
				ret = REDNODEBUS_API(dev)->init_rnb_config(dev, &rnb_user_config);

				if (ret != 0)
				{
					LOG_ERR("Error in init_rnb_config");

					return;
				}

				ret = REDNODEBUS_API(dev)->update_rnb_runtime_config(dev, &rnb_user_runtime_config);

				if (ret != 0)
				{
					LOG_ERR("Error in update_rnb_runtime_config");

					return;
				}

				rnb_configured = true;

				openthread_start(openthread_get_default_context());

#if defined(CONFIG_SOC_NRF52840)
				// Max allowed RADIO output power for nRF52840 SoC. For more info, refer to datasheet
				otPlatRadioSetTransmitPower(openthread_get_default_context()->instance, 8); // +8 dBm
#else
				// Max allowed RADIO output power for nRF52832 SoC. For more info, refer to datasheet
				otPlatRadioSetTransmitPower(openthread_get_default_context()->instance, 4); // +4 dBm
#endif
			}
		}

		if ((params->ranging_mode != REDNODEBUS_USER_RANGING_MODE_DISABLED) &&
		    (params->state == REDNODEBUS_USER_BUS_STATE_CONNECTED))
		{
			print_rnb_uwb_mode(params->uwb_mode);
			print_rnb_ranging_mode(params->ranging_mode);
		}
		break;
	default:
		LOG_WRN("RNB user event: unknown");
		break;
	}
}

static void rnb_utils_process_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	struct rednodebus_utils_event *rnb_utils_event;

	while (1)
	{
		rnb_utils_event = k_fifo_get(&rnb_utils_event_fifo, K_FOREVER);

		process_rnb_utils_event(rnb_utils_event->dev, rnb_utils_event->event, &rnb_utils_event->params);

		rnb_utils_event->in_use = false;
	}
}

int init_rnb(void)
{
	rnb_configured = false;
	rnb_connected = false;

	rnb_user_config.network_id = 0;
	rnb_user_config.role = REDNODEBUS_USER_ROLE_UNDEFINED;
	rnb_user_config.sync_active_period_ms = 2000;
	rnb_user_config.sync_sleep_period_ms = 10000;

	rnb_user_runtime_config.energy_save_mode_enabled = true;
#if defined(CONFIG_REDNODERANGING)
	rnb_user_runtime_config.ranging_enabled = true;
#else
	rnb_user_runtime_config.ranging_enabled = false;
#endif
	rnb_user_runtime_config.ranging_period_ms = 0;

	k_fifo_init(&rnb_utils_event_fifo);

	k_thread_create(&rnb_utils_thread,
			rnb_utils_stack,
			REDNODEBUS_UTILS_STACK_SIZE,
			rnb_utils_process_thread,
			NULL,
			NULL,
			NULL,
			K_PRIO_PREEMPT(REDNODEBUS_UTILS_THREAD_PRIORITY),
			0,
			K_NO_WAIT);

	k_thread_name_set(&rnb_utils_thread, "rnb_utils_thread");

	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	struct ieee802154_config ieee802154_config;

	ieee802154_config.rnb_user_event_handler = handle_rnb_user_event;
	REDNODEBUS_API(dev)->configure(dev, REDNODEBUS_CONFIG_USER_EVENT_HANDLER, &ieee802154_config);

	return 0;
}

bool is_rnb_connected(void)
{
	return rnb_connected;
}
