/* udp.c - UDP specific code for echo client */

/*
 * Copyright (c) 2017 Intel Corporation.
 * Copyright (c) 2018 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(net_echo_client_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/socket.h>
#include <random/rand32.h>

#include "common.h"

#define RECV_BUF_SIZE 1280
#define UDP_SLEEP K_MSEC(150)
#define UDP_WAIT K_SECONDS(60)

static char recv_buf[RECV_BUF_SIZE];

static int send_udp_data(struct data *data)
{
	int ret;

	do {
		data->udp.expecting = sys_rand32_get() % ipsum_len;
	} while (data->udp.expecting == 0U ||
		 data->udp.expecting > data->udp.mtu);

	ret = send(data->udp.sock, lorem_ipsum, data->udp.expecting, 0);

	LOG_DBG("%s UDP: Sent %d bytes", data->proto, data->udp.expecting);

	k_work_reschedule(&data->udp.recv, UDP_WAIT);

	return ret < 0 ? -EIO : 0;
}

static int compare_udp_data(struct data *data, const char *buf, uint32_t received)
{
	if (received != data->udp.expecting) {
		LOG_ERR("Invalid amount of data received: UDP %s", data->proto);
		return -EIO;
	}

	if (memcmp(buf, lorem_ipsum, received) != 0) {
		LOG_ERR("Invalid data received: UDP %s", data->proto);
		return -EIO;
	}

	return 0;
}

static void wait_reply(struct k_work *work)
{
	/* This means that we did not receive response in time. */
	struct data *data = CONTAINER_OF(work, struct data, udp.recv);

	LOG_ERR("UDP %s: Data packet not received", data->proto);

	/* Send a new packet at this point */
	send_udp_data(data);
}

static void wait_transmit(struct k_work *work)
{
	struct data *data = CONTAINER_OF(work, struct data, udp.transmit);

	send_udp_data(data);
}

static int start_udp_proto(struct data *data, struct sockaddr *addr,
			   socklen_t addrlen)
{
	int ret;

	k_work_init_delayable(&data->udp.recv, wait_reply);
	k_work_init_delayable(&data->udp.transmit, wait_transmit);

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

static int process_udp_proto(struct data *data)
{
	int ret, received;

	received = recv(data->udp.sock, recv_buf, sizeof(recv_buf),
			MSG_DONTWAIT);

	if (received == 0) {
		return -EIO;
	}
	if (received < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			ret = 0;
		} else {
			ret = -errno;
		}
		return ret;
	}

	ret = compare_udp_data(data, recv_buf, received);
	if (ret != 0) {
		LOG_WRN("%s UDP: Received and compared %d bytes, data "
			"mismatch", data->proto, received);
		return 0;
	}

	/* Correct response received */
	LOG_DBG("%s UDP: Received and compared %d bytes, all ok",
		data->proto, received);

	if (++data->udp.counter % 1000 == 0U) {
		LOG_INF("%s UDP: Exchanged %u packets", data->proto,
			data->udp.counter);
	}

	k_work_cancel_delayable(&data->udp.recv);

	/* Do not flood the link if we have also TCP configured */
	if (IS_ENABLED(CONFIG_NET_TCP)) {
		k_work_reschedule(&data->udp.transmit, UDP_SLEEP);
		ret = 0;
	} else {
		ret = send_udp_data(data);
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

	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		ret = send_udp_data(&conf.ipv6);
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

int process_udp(void)
{
	int ret = 0;

	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		ret = process_udp_proto(&conf.ipv6);
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

void stop_udp(void)
{
	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		k_work_cancel_delayable(&conf.ipv6.udp.recv);
		k_work_cancel_delayable(&conf.ipv6.udp.transmit);

		if (conf.ipv6.udp.sock >= 0) {
			(void)close(conf.ipv6.udp.sock);
		}
	}
}
