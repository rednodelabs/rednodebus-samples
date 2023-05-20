/* socket-test.c - Networking battery UDP socket client */

/*
 * Copyright (c) 2022 RedNodeLabs UG.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The battery application is acting as a client that is run in Zephyr OS,
 * and the server is run in the host acting as a server.
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_battery_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <syscalls/rand32.h>
#include <net/socket.h>
#include <net/net_mgmt.h>
#include <net/net_event.h>
#include <net/net_conn_mgr.h>
#include <net/ieee802154_radio.h>

#include <device.h>
#include <drivers/sensor.h>

#include "battery.h"

#if defined(CONFIG_REDNODEBUS)
#include "rnb_utils.h"
#endif

#include "common.h"

#define INVALID_SOCK (-1)

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | \
		    NET_EVENT_L4_DISCONNECTED)

#define MTU_SIZE 1280

static uint8_t rnb_role;
int32_t packet_buffer[5];
const int packet_len = sizeof(packet_buffer);

const struct device *sensor;

struct configs conf = {
    .ipv6 = {
	.proto = "IPv6",
	.udp.sock = INVALID_SOCK},
};

#if defined(CONFIG_REDNODEBUS)
void rnb_utils_handle_new_state(const struct rednodebus_user_event_state *event_state)
{
	rnb_role = event_state->role;
}
#endif

static void init_app(void)
{
	conf.ipv6.udp.mtu = MTU_SIZE;

#if defined(CONFIG_REDNODEBUS)
	uint64_t euid = 0;
	rnb_utils_get_euid(&euid);

	packet_buffer[0] = (uint32_t)euid;
	packet_buffer[1] = (uint32_t)(euid >> 32);

	int session_rand = sys_rand32_get();
	packet_buffer[2] = session_rand;
#endif
}

static void battery_send_value()
{
	int batt_mV = battery_sample();

	if (batt_mV < 0)
	{
		LOG_ERR("Failed to read battery voltage: %d\n",
			batt_mV);
	}
	else
	{
		LOG_DBG("Battery: : %d mV", batt_mV);

		packet_buffer[4] = batt_mV;

		send_udp_data(&conf.ipv6);
	}
}

static void start_client(void)
{
	int ret;

	ret = start_udp();
	if (ret == 0)
	{
		while (1)
		{
			ret = battery_measure_enable(true);

			if (ret != 0)
			{
				LOG_ERR("Failed initialize battery measurement: %d\n", ret);
				return;
			}

			// Wait until the value at the ADC pin is stable
			k_sleep(K_MSEC(100));

			battery_send_value();
			battery_measure_enable(false);

			k_sleep(K_MSEC(UDP_TRANSMISSION_PERIOD_SECONDS * 1000));
		}
	}
	else
	{
		LOG_ERR("UDP init failed");
		stop_udp();
	}
}

void main(void)
{
	k_thread_priority_set(k_current_get(), THREAD_PRIORITY);

#if defined(CONFIG_REDNODEBUS)
	init_rnb();

	while (!is_rnb_connected())
	{
		k_sleep(K_MSEC(1000));
	}
#endif

	init_app();

	start_client();
}
