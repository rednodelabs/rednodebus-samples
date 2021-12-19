/* main.c - RedNodeBus node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(rednodebus_node, LOG_LEVEL_INF);

#include "rnb.h"


void main(void)
{
    LOG_INF("RedNodeBus node sample");

#if defined(CONFIG_REDNODEBUS)
    init_rnb();
#endif

}
