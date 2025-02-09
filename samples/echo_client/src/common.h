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

struct data {
	const char *proto;

	struct {
		int sock;
		/* Work controlling udp data sending */
		struct k_work_delayable recv;
		struct k_work_delayable transmit;
		uint32_t expecting;
		uint32_t counter;
		uint32_t mtu;
	} udp;

	struct {
		int sock;
		uint32_t expecting;
		uint32_t received;
		uint32_t counter;
	} tcp;
};
struct data_rnb {
	const char *proto;

	struct {
		struct rednodebus_user_payload_params rnb_user_payload_params;
		/* Work controlling rnb data sending */
		struct k_work_delayable recv;
		struct k_work_delayable transmit;
		uint32_t expecting;
		uint32_t counter;
		uint32_t mtu;
	} rnb_user_payload;
};

struct configs {
	struct data ipv6;
};
struct configs_rnb {
	struct data_rnb rnb;
};

#if !defined(CONFIG_NET_CONFIG_PEER_IPV6_ADDR)
#define CONFIG_NET_CONFIG_PEER_IPV6_ADDR "2001:0db8:0001:ffff:0000:0000:ac11:0001"
#endif

extern const char lorem_ipsum[];
extern const int ipsum_len;
extern struct configs conf;
extern struct configs_rnb conf_rnb;

int start_udp(void);
int process_udp(void);
void stop_udp(void);

int start_tcp(void);
int process_tcp(void);
void stop_tcp(void);

int start_rnb_user_payload(void);
int process_rnb_user_payload(void);
void stop_rnb_user_payload(void);
void wait_rnb_user_payload_reply(void);
void rnb_utils_handle_user_payload(const uint8_t user_payload_length,
				   const struct rednodebus_user_event_params_user_payload *user_payload);

#if defined(CONFIG_NET_VLAN)
int init_vlan(void);
#else
static inline int init_vlan(void)
{
	return 0;
}
#endif
