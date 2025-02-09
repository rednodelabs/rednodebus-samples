/* rnb-user-payload.c - RedNodeBus payload */

/*
 * Copyright (c) 2025 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(net_socket_test_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/ieee802154_radio.h>

#include "common.h"

/* Convenience defines for RADIO */
#define REDNODEBUS_API(dev) ((const struct ieee802154_radio_api *const)(dev)->api)

int send_rnb_data(struct data_rnb *data)
{
	const struct device *dev = device_get_binding(CONFIG_IEEE802154_NRF5_DRV_NAME);
	int ret;

	if(data->rnb_user_payload.transmitting == 0 || data->rnb_user_payload.transmitting > data->rnb_user_payload.mtu)
	{
		LOG_ERR("%s USER PAYLOAD: Invalid data size %d", data->proto, data->rnb_user_payload.transmitting);
		ret = -EINVAL;
		goto exit;
	}

	ret = REDNODEBUS_API(dev)->send_rnb_user_payload(dev, &data->rnb_user_payload.rnb_user_payload_params, data->rnb_user_payload.transmitting);

	if (ret == 0)
	{
		LOG_DBG("%s USER PAYLOAD: Sent %d bytes", data->proto, data->rnb_user_payload.transmitting);
		LOG_HEXDUMP_DBG(&data->rnb_user_payload.rnb_user_payload_params.user_payload, data->rnb_user_payload.transmitting, "Data");
	}
	else
	{
		LOG_ERR("%s USER PAYLOAD: error sending %d", data->proto, ret);
	}

exit:
	return ret < 0 ? -EIO : 0;
}

int start_rnb_user_payload(void)
{
	return 0;
}

void stop_rnb_user_payload(void)
{
}
