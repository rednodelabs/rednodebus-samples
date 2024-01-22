/* rnb_utils.c - RedNodeBus utilities code for node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(rnb_utils, LOG_LEVEL_INF);

#include <zephyr.h>
#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <net/sntp.h>

#include <net/ieee802154_radio.h>
#include <net/openthread.h>
#include <openthread/platform/radio.h>
#include <openthread/ip6.h>
#include <openthread/thread.h>

#include <dk_buttons_and_leds.h>

#include "rnb_utils.h"
#include "rnb_leds.h"

#define REDNODEBUS_UTILS_STACK_SIZE 512
#define REDNODEBUS_UTILS_EVENT_BUFFER_SIZE 8
#define REDNODEBUS_UTILS_THREAD_PRIORITY 3
#define REDNODEBUS_SNTP_STACK_SIZE 1024
#define CONFIG_IPV6_ADDR_SNTP "2001:0db8:0001:ffff:0000:0000:ac13:0002"
#define CONFIG_IPV6_ADDR_SNTP_PORT 123
#define REDNODEBUS_SNTP_THREAD_PRIORITY 4
#define REDNODEBUS_SNTP_UPDATE_TRIGGER "CLOCK"
#define REDNODEBUS_UTILS_EUID_BYTE_LENGTH 6
#define REDNODEBUS_SIMULATED_DISCONNECTION 0
#define REDNODEBUS_SIMULATED_DISCONNECTION_PERIOD_S 10
#define REDNODEBUS_SIMULATED_DISCONNECTION_THREAD_PRIORITY 0

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) ((const struct ieee802154_radio_api *const)(dev)->api)

struct rednodebus_utils_event
{
	void *fifo_reserved; /* 1st word reserved for use by fifo. */
	const struct device *dev;
	enum rednodebus_user_event rnb_user_event;
	struct rednodebus_user_event_params rnb_user_event_params;
	bool in_use;
};

/* RedNodeBus utils stack. */
static K_THREAD_STACK_DEFINE(rnb_utils_stack, REDNODEBUS_UTILS_STACK_SIZE);
#if REDNODEBUS_SIMULATED_DISCONNECTION
static K_THREAD_STACK_DEFINE(rnb_simulated_disconnection_stack, REDNODEBUS_UTILS_STACK_SIZE);
#endif

/* RedNodeBus utils thread control block. */
static struct k_thread rnb_utils_thread;
#if REDNODEBUS_SIMULATED_DISCONNECTION
static struct k_thread rnb_simulated_disconnection_thread;
#endif

/* RedNodeBus utils event fifo queue. */
static struct k_fifo rnb_utils_event_fifo;

/* RedNodeBus SNTP thread. */
static struct k_thread rnb_sntp_thread;

/* RedNodeBus SNTP stack. */
static K_THREAD_STACK_DEFINE(rnb_sntp_stack, REDNODEBUS_SNTP_STACK_SIZE);

/* SNTP variables. */
static struct sntp_time sntp_time;
static int64_t sntp_reference_uptime;
static bool sntp_synchronized;
static K_SEM_DEFINE(sntp_sem, 0, 1);

#if !defined(CONFIG_REDNODEBUS_WATCHDOG) && defined(CONFIG_WATCHDOG)
#define REDNODEBUS_WDT_STACK_SIZE 256
#define REDNODEBUS_WDT_THREAD_PRIORITY 5

/* RedNodeBus SNTP thread. */
static struct k_thread rnb_wdt_thread;

/* RedNodeBus SNTP stack. */
static K_THREAD_STACK_DEFINE(rnb_wdt_stack, REDNODEBUS_WDT_STACK_SIZE);

static void rnb_wdt_process_thread(void *arg1, void *arg2, void *arg3);
#endif

/* RedNodeBus OT start work. */
static struct k_work ot_init_work;
static struct k_work ot_deinit_work;
static struct k_work ot_start_work;
static struct k_work ot_stop_work;

static struct rednodebus_utils_event rnb_utils_events[REDNODEBUS_UTILS_EVENT_BUFFER_SIZE];
__WEAK struct rednodebus_user_config rnb_user_config = {.role = REDNODEBUS_USER_ROLE_UNDEFINED, .sync_active_period_ms = 2000, .sync_sleep_period_ms = 10000};
static struct rednodebus_user_runtime_config rnb_user_runtime_config;
static bool rnb_configured;
static bool rnb_started;
static bool rnb_connected;
static bool rnb_initialized = false;
static bool ot_started;

static void print_rnb_state(const uint8_t state);
static void print_rnb_role(const uint8_t role);
static void print_rnb_uwb_mode(const uint8_t uwb_mode);
static void print_rnb_ranging_mode(const uint8_t ranging_mode);
static void handle_rnb_user_event(const struct device *dev,
				  const enum rednodebus_user_event evt,
				  void *event_params);
static void handle_rnb_user_rxtx_signal(const struct device *dev,
					enum rednodebus_user_rxtx_signal sig,
					bool active);
static void process_rnb_utils_event(const struct device *dev,
				    enum rednodebus_user_event evt,
				    const struct rednodebus_user_event_params *rnb_user_event_params);
static void rnb_utils_process_thread(void *arg1, void *arg2, void *arg3);
#if REDNODEBUS_SIMULATED_DISCONNECTION
static void rnb_simulated_disconnection_process_thread(void *arg1, void *arg2, void *arg3);
#endif
static void ot_init_work_handler(struct k_work *item);
static void ot_deinit_work_handler(struct k_work *item);
static void ot_start_work_handler(struct k_work *item);
static void ot_stop_work_handler(struct k_work *item);

static void ot_init_work_handler(struct k_work *item)
{
	ARG_UNUSED(item);

	otError error;
	struct openthread_context *ctx;

	if (rnb_started)
	{
		return;
	}

	ctx = openthread_get_default_context();

	openthread_api_mutex_lock(ctx);

	error = otIp6SetEnabled(ctx->instance, true);

	if (error == OT_ERROR_NONE)
	{
		rnb_started = true;
	}

#if defined(CONFIG_SOC_NRF52840)
	// Max allowed RADIO output power for nRF52840 SoC. For more info, refer to datasheet
	otPlatRadioSetTransmitPower(ctx->instance, 8); // +8 dBm
#else
	// Max allowed RADIO output power for nRF52832 SoC. For more info, refer to datasheet
	otPlatRadioSetTransmitPower(ctx->instance, 4); // +4 dBm
#endif

	openthread_api_mutex_unlock(ctx);
}

static void ot_deinit_work_handler(struct k_work *item)
{
	otError error;
	struct openthread_context *ctx;

	if (!rnb_started)
	{
		return;
	}

	ctx = openthread_get_default_context();

	openthread_api_mutex_lock(ctx);

	error = otThreadSetEnabled(ctx->instance, false);

	if (error == OT_ERROR_NONE)
	{
		ot_started = false;
	}

	error = otIp6SetEnabled(ctx->instance, false);

	if (error == OT_ERROR_NONE)
	{
		rnb_started = false;
	}

	openthread_api_mutex_unlock(ctx);
}

static void ot_start_work_handler(struct k_work *item)
{
	ARG_UNUSED(item);
	int ret;

	if (rnb_connected && !ot_started)
	{
		ret = openthread_start(openthread_get_default_context());

		if (ret == 0)
		{
			ot_started = true;
		}
	}
}

static void ot_stop_work_handler(struct k_work *item)
{
	ARG_UNUSED(item);
	struct openthread_context *ctx;

	if (ot_started)
	{
		ctx = openthread_get_default_context();

		openthread_api_mutex_lock(ctx);

		otThreadSetEnabled(ctx->instance, false);

		openthread_api_mutex_unlock(ctx);

		ot_started = false;
	}
}

__WEAK void rnb_utils_handle_new_state(const struct rednodebus_user_event_params_state *state)
{
	/* Intentionally empty. */
}

__WEAK void rnb_utils_handle_user_payload(const uint8_t user_payload_length,
					  const struct rednodebus_user_event_params_user_payload *user_payload)
{
	/* Example receiving a command via user payload. */
	if (user_payload_length == 5)
	{
		char sntp_trigger_bfr[] = REDNODEBUS_SNTP_UPDATE_TRIGGER;
		if (!memcmp(user_payload, sntp_trigger_bfr, 5))
		{
			k_sem_give(&sntp_sem);
			LOG_INF("SNTP Update Trigger Received");
		}
	}
}

void rnb_utils_get_euid(uint64_t *euid)
{
	/*
	 * NRF_FICR->DEVICEADDR[] is array of 32-bit words.
	 * NRF_FICR->DEVICEADDR yields type (unit32_t*)
	 * Cast: (uint64_t*) NRF_FICR->DEVICEADDR yields type (unit64_t*)
	 * Dereferencing: *(uint64_t*) NRF_FICR->DEVICEADDR yields type uint64_t
	 *
	 * Nordic doc asserts upper two bytes read all ones.
	 */
	uint64_t my_euid = *((uint64_t *)NRF_FICR->DEVICEADDR);

	// Mask off upper bytes, to match over-the-air length of 6 bytes.
	my_euid &= (((uint64_t)1) << (REDNODEBUS_UTILS_EUID_BYTE_LENGTH * 8)) - 1;

	memcpy(euid, &my_euid, REDNODEBUS_UTILS_EUID_BYTE_LENGTH);
}

void rnb_utils_stop()
{
	if (rnb_initialized)
	{
		k_work_submit(&ot_deinit_work);
	}
}

void rnb_utils_start()
{
	if (rnb_initialized)
	{
		k_work_submit(&ot_init_work);
	}
}

static void print_rnb_state(const uint8_t state)
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
				  const enum rednodebus_user_event evt,
				  void *event_params)
{
	const struct rednodebus_user_event_params *rnb_user_event_params = (const struct rednodebus_user_event_params *)event_params;
	const uint8_t event_size = sizeof(rnb_user_event_params->event_params_length) + rnb_user_event_params->event_params_length;

	for (uint8_t i = 0; i < ARRAY_SIZE(rnb_utils_events); i++)
	{
		if (rnb_utils_events[i].in_use)
		{
			continue;
		}

		rnb_utils_events[i].in_use = true;

		rnb_utils_events[i].dev = dev;

		rnb_utils_events[i].rnb_user_event = evt;

		memcpy(&rnb_utils_events[i].rnb_user_event_params,
		       rnb_user_event_params,
		       event_size);

		k_fifo_put(&rnb_utils_event_fifo, &rnb_utils_events[i]);

		return;
	}

	LOG_ERR("Not enough events allocated for rnb user event id %u", evt);
}

static void handle_rnb_user_rxtx_signal(const struct device *dev,
					enum rednodebus_user_rxtx_signal sig,
					bool active)
{
	switch (sig)
	{
	case REDNODEBUS_USER_RX_SIGNAL:
		rnb_leds_set_rx(active);
		rnb_leds_set_tx(false);
		break;
	case REDNODEBUS_USER_TX_SIGNAL:
		rnb_leds_set_tx(active);
		rnb_leds_set_rx(false);
		break;
	default:
		break;
	}
}

static void process_rnb_utils_event(const struct device *dev,
				    enum rednodebus_user_event evt,
				    const struct rednodebus_user_event_params *rnb_user_event_params)
{
	const struct rednodebus_user_event_params_state *state;
	const struct rednodebus_user_event_params_user_payload *user_payload;
	int ret;

	switch (evt)
	{
	case REDNODEBUS_USER_EVENT_NEW_STATE:
		LOG_DBG("RNB user event new state");

		state = &rnb_user_event_params->state;

		print_rnb_state(state->state);

		LOG_INF("RNB ID: 0x%04X", state->rnb_id);

		if (state->state > REDNODEBUS_USER_BUS_STATE_UNSYNCHRONIZED)
		{
			if (state->role != REDNODEBUS_USER_ROLE_UNDEFINED)
			{
				print_rnb_role(state->role);
			}

			LOG_INF("RNB period ms: %u", state->period_ms);
		}

		if (state->state == REDNODEBUS_USER_BUS_STATE_STOPPED)
		{
			if (!rnb_configured)
			{
				ret = REDNODEBUS_API(dev)->init_rnb_user_config(dev, &rnb_user_config);

				if (ret != 0)
				{
					LOG_ERR("Error in init_rnb_config");

					return;
				}

				ret = REDNODEBUS_API(dev)->update_rnb_user_runtime_config(dev, &rnb_user_runtime_config);

				if (ret != 0)
				{
					LOG_ERR("Error in update_rnb_runtime_config");

					return;
				}

				rnb_configured = true;

				rnb_utils_start();
			}
			else
			{
				// In order to reset RNB_ID
				// struct rednodebus_user_settings rnb_user_settings;
				// rnb_user_settings.rnb_id = 0;
				// ret = REDNODEBUS_API(dev)->set_rnb_user_settings(dev, &rnb_user_settings);
			}
		}
		else if (state->state == REDNODEBUS_USER_BUS_STATE_CONNECTED)
		{
			rnb_connected = true;

			k_work_submit(&ot_start_work);
		}
		else if (state->state == REDNODEBUS_USER_BUS_STATE_UNSYNCHRONIZED)
		{
			k_work_submit(&ot_stop_work);

			rnb_connected = false;
		}

		if ((state->ranging_mode != REDNODEBUS_USER_RANGING_MODE_DISABLED) &&
		    (state->state == REDNODEBUS_USER_BUS_STATE_CONNECTED))
		{
			print_rnb_uwb_mode(state->uwb_mode);
			print_rnb_ranging_mode(state->ranging_mode);
		}

		rnb_utils_handle_new_state(state);

		break;
	case REDNODEBUS_USER_EVENT_USER_PAYLOAD:
		user_payload = &rnb_user_event_params->user_payload;

		rnb_utils_handle_user_payload(rnb_user_event_params->event_params_length, user_payload);
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

		process_rnb_utils_event(rnb_utils_event->dev,
					rnb_utils_event->rnb_user_event,
					&rnb_utils_event->rnb_user_event_params);

		rnb_utils_event->in_use = false;
	}
}

static void rnb_sntp_process_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	struct sntp_ctx ctx;
	struct sockaddr_in6 addr6;

	int rv;

	/* ipv6 */
	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(CONFIG_IPV6_ADDR_SNTP_PORT);
	inet_pton(AF_INET6, CONFIG_IPV6_ADDR_SNTP, &addr6.sin6_addr);

	sntp_synchronized = false;

	while (1)
	{
		if (is_rnb_connected())
		{
			rv = sntp_init(&ctx, (struct sockaddr *)&addr6,
				       sizeof(struct sockaddr_in6));
			if (rv < 0)
			{
				LOG_ERR("Failed to init SNTP IPv6 ctx: %d", rv);
			}
			else
			{
				LOG_INF("Sending SNTP IPv6 request...");
				rv = sntp_query(&ctx, 4 * MSEC_PER_SEC, &sntp_time);
				if (rv < 0)
				{
					LOG_ERR("SNTP IPv6 request: %d", rv);
				}
				else
				{
					sntp_reference_uptime = k_uptime_get();
					sntp_synchronized = true;
					LOG_INF("time since Epoch: high word: %u, low word: %u, fraction: %u",
						(uint32_t)(sntp_time.seconds >> 32), (uint32_t)sntp_time.seconds, sntp_time.fraction);
				}
			}
			sntp_close(&ctx);
		}
		if (sntp_synchronized)
		{
			k_sem_take(&sntp_sem, K_FOREVER);
		}
		else
		{
			k_sleep(K_MSEC(10000));
		}
	}
}

#if REDNODEBUS_SIMULATED_DISCONNECTION
static void rnb_simulated_disconnection_process_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	bool is_connected;

	while (!is_rnb_connected())
	{
		k_sleep(K_SECONDS(1));
	}

	is_connected = is_rnb_connected();

	while (1)
	{
		if (is_connected)
		{
			rnb_utils_stop();

			LOG_INF("RedNodeBus stopped");
		}
		else
		{
			rnb_utils_start();

			LOG_INF("RedNodeBus started");
		}

		is_connected = !is_connected;

		k_sleep(K_SECONDS(REDNODEBUS_SIMULATED_DISCONNECTION_PERIOD_S));
	}
}
#endif

bool is_rnb_connected(void)
{
	return rnb_connected;
}

int64_t rnb_get_unix_time(void)
{
	int64_t unix_time_seconds = -1;

	if (sntp_synchronized)
	{
		unix_time_seconds = ((k_uptime_get() - sntp_reference_uptime) / 1000) + sntp_time.seconds;
	}

	return unix_time_seconds;
}

#if defined(CONFIG_DK_LIBRARY)
static void on_button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & DK_BTN1_MSK)
	{
		if (rnb_started)
		{
			rnb_utils_stop();
		}
		else
		{
			rnb_utils_start();
		}
	}
}
#endif

int init_rnb(void)
{
	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	uint64_t euid = 0;
	struct ieee802154_config ieee802154_config;
	int ret;

	ret = rnb_leds_init();
	if (ret)
	{
		LOG_WRN("Cannot init RedNodeBus LEDs");
	}

#if defined(CONFIG_DK_LIBRARY) && !defined(CONFIG_COAP_UTILS)
	ret = dk_buttons_init(on_button_changed);
	if (ret)
	{
		LOG_ERR("Cannot init buttons (error: %d)", ret);
		return ret;
	}
#endif

	rnb_utils_get_euid(&euid);

	LOG_INF("EUID 0x%04X%04X", (uint32_t)(euid >> 32), (uint32_t)euid);

	rnb_configured = false;
	rnb_started = false;
	rnb_connected = false;
	ot_started = false;

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

	k_thread_create(&rnb_sntp_thread,
			rnb_sntp_stack,
			REDNODEBUS_SNTP_STACK_SIZE,
			rnb_sntp_process_thread,
			NULL,
			NULL,
			NULL,
			K_PRIO_PREEMPT(REDNODEBUS_SNTP_THREAD_PRIORITY),
			0,
			K_NO_WAIT);

	k_thread_name_set(&rnb_sntp_thread, "rnb_sntp_thread");

#if REDNODEBUS_SIMULATED_DISCONNECTION
	k_thread_create(&rnb_simulated_disconnection_thread,
			rnb_simulated_disconnection_stack,
			REDNODEBUS_UTILS_STACK_SIZE,
			rnb_simulated_disconnection_process_thread,
			NULL,
			NULL,
			NULL,
			K_PRIO_PREEMPT(REDNODEBUS_SIMULATED_DISCONNECTION_THREAD_PRIORITY),
			0,
			K_NO_WAIT);

	k_thread_name_set(&rnb_simulated_disconnection_thread, "rnb_simulated_disconnection_thread");
#endif

#if !defined(CONFIG_REDNODEBUS_WATCHDOG) && defined(CONFIG_WATCHDOG)
	k_thread_create(&rnb_wdt_thread,
			rnb_wdt_stack,
			REDNODEBUS_WDT_STACK_SIZE,
			rnb_wdt_process_thread,
			NULL,
			NULL,
			NULL,
			K_PRIO_PREEMPT(REDNODEBUS_WDT_THREAD_PRIORITY),
			0,
			K_NO_WAIT);

	k_thread_name_set(&rnb_wdt_thread, "rnb_wdt_thread");
#endif

	k_work_init(&ot_init_work, ot_init_work_handler);
	k_work_init(&ot_deinit_work, ot_deinit_work_handler);
	k_work_init(&ot_start_work, ot_start_work_handler);
	k_work_init(&ot_stop_work, ot_stop_work_handler);

	ieee802154_config.rnb_user_event_handler = handle_rnb_user_event;
	REDNODEBUS_API(dev)->configure(dev, REDNODEBUS_CONFIG_USER_EVENT_HANDLER, &ieee802154_config);

	ieee802154_config.rnb_user_rxtx_signal_handler = handle_rnb_user_rxtx_signal;
	REDNODEBUS_API(dev)->configure(dev, REDNODEBUS_CONFIG_USER_RXTX_SIGNAL_HANDLER, &ieee802154_config);

	rnb_initialized = true;

	return 0;
}

#if !defined(CONFIG_REDNODEBUS_WATCHDOG) && defined(CONFIG_WATCHDOG)
#include <zephyr/drivers/watchdog.h>

#define WATCHDOG_PERIOD 20000

static int wdt_channel_id;

static void nrf5_wdt_start(void)
{
	const struct device *wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
	int err;

	if (!device_is_ready(wdt))
	{
		LOG_ERR("%s: device not ready.\n", wdt->name);
		return;
	}

	struct wdt_timeout_cfg wdt_config = {
	    /* Reset SoC when watchdog timer expires. */
	    .flags = WDT_FLAG_RESET_SOC,

	    /* Expire watchdog after max window */
	    .window.min = 0U,
	    .window.max = WATCHDOG_PERIOD,
	};

	wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	if (wdt_channel_id < 0)
	{
		LOG_ERR("Watchdog install error\n");
		return;
	}

	err = wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG);
	if (err < 0)
	{
		LOG_ERR("Watchdog setup error\n");
		return;
	}
}

static void rnb_wdt_process_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	const struct device *wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));

	nrf5_wdt_start();

	while (1)
	{
		wdt_feed(wdt, wdt_channel_id);
		k_sleep(K_MSEC(WATCHDOG_PERIOD / 4));
	}
}

#endif /* CONFIG_WATCHDOG */
