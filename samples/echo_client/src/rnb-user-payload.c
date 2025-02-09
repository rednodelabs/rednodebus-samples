/* rnb-user-payload.c - RedNodeBus payload */

/*
 * Copyright (c) 2025 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(net_echo_client_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/ieee802154_radio.h>

#include <random/rand32.h>

#include "common.h"

#define RECV_BUF_SIZE 1280
#define RNB_SLEEP K_MSEC(150)
#define RNB_WAIT K_SECONDS(20)

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) ((const struct ieee802154_radio_api *const)(dev)->api)

K_SEM_DEFINE(wait_rnb_data, 0, 1);

static char recv_buf[RECV_BUF_SIZE];
static int received;

static int send_rnb_data(struct data_rnb *data)
{
	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	int ret;
	uint16_t rnb_user_payload_params_length;

	do
	{
		data->rnb_user_payload.expecting = sys_rand32_get() % (data->rnb_user_payload.mtu + 1);
	} while (data->rnb_user_payload.expecting == 0U);

	memcpy(&data->rnb_user_payload.rnb_user_payload_params.user_payload, lorem_ipsum, data->rnb_user_payload.expecting);

	rnb_user_payload_params_length = data->rnb_user_payload.expecting;

	ret = REDNODEBUS_API(dev)->send_rnb_user_payload(dev, &data->rnb_user_payload.rnb_user_payload_params, rnb_user_payload_params_length);

	if (ret == 0)
	{
		LOG_DBG("%s USER PAYLOAD: Sent %d bytes", data->proto, data->rnb_user_payload.expecting);
		LOG_HEXDUMP_DBG(&data->rnb_user_payload.rnb_user_payload_params.user_payload, data->rnb_user_payload.expecting, "Data");
	}
	else
	{
		LOG_ERR("%s USER PAYLOAD: error sending %d", data->proto, ret);
	}

	k_work_reschedule(&data->rnb_user_payload.recv, RNB_WAIT);

	return ret < 0 ? -EIO : 0;
}

static int compare_rnb_data(struct data_rnb *data, const char *buf, uint32_t received)
{
	if (received != data->rnb_user_payload.expecting)
	{
		LOG_ERR("%s USER PAYLOAD: Invalid amount of data received:, received %d vs expected %d", data->proto, received, data->rnb_user_payload.expecting);
		return -EIO;
	}

	if (memcmp(buf, lorem_ipsum, received) != 0)
	{
		LOG_ERR("%s USER PAYLOAD: Invalid data received", data->proto);
		return -EIO;
	}

	return 0;
}

static void wait_reply(struct k_work *work)
{
	/* This means that we did not receive response in time. */
	struct data_rnb *data = CONTAINER_OF(work, struct data_rnb, rnb_user_payload.recv);

	LOG_ERR("%s USER PAYLOAD: Data packet not received", data->proto);

	/* Send a new packet at this point */
	send_rnb_data(data);
}

static void wait_transmit(struct k_work *work)
{
	struct data_rnb *data = CONTAINER_OF(work, struct data_rnb, rnb_user_payload.transmit);

	send_rnb_data(data);
}

static int start_rnb_proto(struct data_rnb *data)
{
	int ret = 0;

	k_work_init_delayable(&data->rnb_user_payload.recv, wait_reply);
	k_work_init_delayable(&data->rnb_user_payload.transmit, wait_transmit);

	received = 0;

	return ret;
}

static int process_rnb_proto(struct data_rnb *data)
{
	int ret;

	if (received == 0)
	{
		return -EIO;
	}
	if (received < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			ret = 0;
		}
		else
		{
			ret = -errno;
		}
		return ret;
	}

	ret = compare_rnb_data(data, recv_buf, received);
	if (ret != 0)
	{
		LOG_WRN("%s: Received and compared %d bytes, data "
			"mismatch",
			data->proto, received);
		return 0;
	}

	/* Correct response received */
	LOG_DBG("%s: Received and compared %d bytes, all ok",
		data->proto, received);

	if (++data->rnb_user_payload.counter % 1000 == 0U)
	{
		LOG_INF("%s USER PAYLOAD: Exchanged %u packets", data->proto,
			data->rnb_user_payload.counter);
	}

	k_work_cancel_delayable(&data->rnb_user_payload.recv);

	ret = send_rnb_data(data);

	return ret;
}

int start_rnb_user_payload(void)
{
	int ret = 0;

	ret = start_rnb_proto(&conf_rnb.rnb);
	if (ret < 0)
	{
		return ret;
	}

	ret = send_rnb_data(&conf_rnb.rnb);
	if (ret < 0)
	{
		return ret;
	}

	return ret;
}

int process_rnb_user_payload(void)
{
	int ret;

	ret = process_rnb_proto(&conf_rnb.rnb);

	return ret;
}

void stop_rnb_user_payload(void)
{
}

void wait_rnb_user_payload_reply(void)
{
	/* Wait for the reply. */
	k_sem_take(&wait_rnb_data, K_FOREVER);
}

void rnb_utils_handle_user_payload(const uint8_t user_payload_length,
				   const struct rednodebus_user_event_params_user_payload *user_payload)
{
	memcpy(recv_buf, &user_payload->user_payload, user_payload_length);

	received = user_payload_length;

	k_sem_give(&wait_rnb_data);
}
