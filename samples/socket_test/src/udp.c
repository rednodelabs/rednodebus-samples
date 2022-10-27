/* udp.c - UDP specific code for echo client */

/*
 * Copyright (c) 2017 Intel Corporation.
 * Copyright (c) 2018 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(net_socket_test_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/socket.h>
#include <net/tls_credentials.h>
#include <random/rand32.h>

#include "common.h"

int send_udp_data(struct data *data)
{
	int ret;
	static int packets_sent;

	memcpy(&lorem_ipsum[UID_CHARS + SESSION_RAND_CHARS], &packets_sent, sizeof(packets_sent));

	do {
		//data->udp.transmitting = sys_rand32_get() % ipsum_len;
		data->udp.transmitting = UDP_TRANSMISSION_BYTES % ipsum_len;
	} while (data->udp.transmitting == 0U ||
		 data->udp.transmitting > data->udp.mtu);

	ret = send(data->udp.sock, lorem_ipsum, data->udp.transmitting, 0);

	LOG_DBG("%s UDP: Sent %d bytes", data->proto, data->udp.transmitting);

	packets_sent ++;

	return ret < 0 ? -EIO : 0;
}

static int start_udp_proto(struct data *data, struct sockaddr *addr,
			   socklen_t addrlen)
{
	int ret;

	data->udp.sock = socket(addr->sa_family, SOCK_DGRAM, IPPROTO_UDP);

	if (data->udp.sock < 0) {
		LOG_ERR("Failed to create UDP socket (%s): %d", data->proto,
			errno);
		return -errno;
	}

	/* Call connect so we can use send and recv. */
	ret = connect(data->udp.sock, addr, addrlen);
	if (ret < 0) {
		LOG_ERR("Cannot connect to UDP remote (%s): %d", data->proto,
			errno);
		ret = -errno;
	}

	return ret;
}

int start_udp(void)
{
	int ret = 0;
	struct sockaddr_in6 addr6;

	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = htons(PEER_PORT);
		inet_pton(AF_INET6, CONFIG_NET_CONFIG_PEER_IPV6_ADDR,
			  &addr6.sin6_addr);

		ret = start_udp_proto(&conf.ipv6, (struct sockaddr *)&addr6,
				      sizeof(addr6));
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

void stop_udp(void)
{
	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		if (conf.ipv6.udp.sock >= 0) {
			(void)close(conf.ipv6.udp.sock);
		}
	}
}
