/* socket-test.c - Networking accelerometer UDP socket client */

/*
 * Copyright (c) 2022 RedNodeLabs UG.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The accelerometer application is acting as a client that is run in Zephyr OS,
 * and the server is run in the host acting as a server.
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_accelerometer_sample, LOG_LEVEL_DBG);

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

#if defined(CONFIG_REDNODEBUS)
#include "rnb_utils.h"
#endif

#include "common.h"

#define INVALID_SOCK (-1)

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | \
		    NET_EVENT_L4_DISCONNECTED)

#define MTU_SIZE 1280

static uint8_t rnb_role;

void rnb_utils_handle_new_state(const struct rednodebus_user_event_state *event_state)
{
	rnb_role = event_state->role;
}

int32_t packet_buffer[10];
const int packet_len = sizeof(packet_buffer);

const struct device *sensor;

struct configs conf = {
    .ipv6 = {
	.proto = "IPv6",
	.udp.sock = INVALID_SOCK},
};

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

static void fetch_and_display(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG)
	{
		overrun = "[OVERRUN] ";
		rc = 0;
	}
	if (rc == 0)
	{
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}

	if (rc < 0)
	{
		LOG_INF("ERROR: Accelerometer data failed: %d\n", rc);
	}
	else
	{
		packet_buffer[4] = accel[0].val1;
		packet_buffer[5] = accel[0].val2;
		packet_buffer[6] = accel[1].val1;
		packet_buffer[7] = accel[1].val2;
		packet_buffer[8] = accel[2].val1;
		packet_buffer[9] = accel[2].val2;

		send_udp_data(&conf.ipv6);

		LOG_DBG("#%u EUID 0x%04X%04X @ %u ms; %sx_acc: %d, y_acc: %0d, z_acc: %d\n",
			count, packet_buffer[1], packet_buffer[0], k_uptime_get_32(), overrun,
			packet_buffer[4], packet_buffer[6], packet_buffer[8]);
	}
}

static void trigger_handler(const struct device *dev,
			    const struct sensor_trigger *trig)
{
	fetch_and_display(dev);
}

static void accelerometer()
{
	sensor = DEVICE_DT_GET_ANY(st_lis2dh);

	if (sensor == NULL)
	{
		LOG_ERR("Accelerometer not found\n");
		return;
	}
	if (!device_is_ready(sensor))
	{
		LOG_ERR("Device %s is not ready\n", sensor->name);
		return;
	}

	struct sensor_trigger trig;
	int rc;

	trig.type = SENSOR_TRIG_DATA_READY;
	trig.chan = SENSOR_CHAN_ACCEL_XYZ;

	rc = sensor_trigger_set(sensor, &trig, trigger_handler);
	if (rc != 0)
	{
		LOG_ERR("Failed to set trigger: %d\n", rc);
		return;
	}
}

static void start_client(void)
{
	int ret;

	if (rnb_role == REDNODEBUS_USER_ROLE_TAG)
	{
		ret = start_udp();
		if (ret == 0)
		{
			accelerometer();
		}
		else
		{
			LOG_ERR("UDP init failed");
			stop_udp();
		}
	}
}

void main(void)
{
#if defined(CONFIG_REDNODEBUS)
	init_rnb();

	while (!is_rnb_connected())
	{
		k_sleep(K_MSEC(1000));
	}
#endif

	init_app();

	k_thread_priority_set(k_current_get(), THREAD_PRIORITY);

	start_client();
}
