/*
 * Copyright (c) 2017 Intel Corporation.
 * Copyright (c) 2018 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <net/ieee802154_radio.h>

/* Value of 0 will cause the IP stack to select next free port */
#define MY_PORT 0

#define PEER_PORT 4242

#if IS_ENABLED(CONFIG_NET_TC_THREAD_PREEMPTIVE)
#define THREAD_PRIORITY K_PRIO_PREEMPT(8)
#else
#define THREAD_PRIORITY K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1)
#endif

struct data
{
	const char *proto;

	struct
	{
		int sock;
		uint32_t transmitting;
		uint32_t mtu;
	} udp;
};

struct data_rnb {
	const char *proto;

	struct {
		struct rednodebus_user_payload_params rnb_user_payload_params;
		uint32_t transmitting;
		uint32_t mtu;
	} rnb_user_payload;
};

struct configs
{
	struct data ipv6;
};
struct configs_rnb {
	struct data_rnb rnb;
};

#if !defined(CONFIG_NET_CONFIG_PEER_IPV6_ADDR)
#define CONFIG_NET_CONFIG_PEER_IPV6_ADDR "2001:0db8:0001:ffff:0000:0000:ac11:0001"
#endif

extern int32_t packet_buffer[];
extern const int packet_len;
extern struct configs conf;

int start_udp(void);
void stop_udp(void);
int send_udp_data(struct data *data);

int start_rnb_user_payload(void);
void stop_rnb_user_payload(void);
int send_rnb_data(struct data_rnb *data);
