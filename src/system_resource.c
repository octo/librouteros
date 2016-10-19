/**
 * librouteros - src/system_resource.c
 * Copyright (C) 2009  Florian octo Forster
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:
 *   Florian octo Forster <octo at verplant.org>
 **/

#ifndef _ISOC99_SOURCE
# define _ISOC99_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
#endif

#include "config.h"

#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include "routeros_api.h"
#include "ros_parse.h"

/*
 * Private data types
 */
struct rt_internal_data_s
{
	ros_system_resource_handler_t handler;
	void *user_data;
};
typedef struct rt_internal_data_s rt_internal_data_t;

/*
 * Private functions
 */
static int rt_reply_to_system_resource (const ros_reply_t *r, /* {{{ */
		ros_system_resource_t *ret)
{
	if (r == NULL)
		return (EINVAL);

	if (strcmp ("re", ros_reply_status (r)) != 0)
		return (rt_reply_to_system_resource (ros_reply_next (r), ret));

	ret->uptime = sstrtodate (ros_reply_param_val_by_key (r, "uptime"));

	ret->version = ros_reply_param_val_by_key (r, "version");
	ret->architecture_name = ros_reply_param_val_by_key (r, "architecture-name");
	ret->board_name = ros_reply_param_val_by_key (r, "board-name");

	ret->cpu_model = ros_reply_param_val_by_key (r, "cpu");
	ret->cpu_count = sstrtoui (ros_reply_param_val_by_key (r, "cpu-count"));
	ret->cpu_load = sstrtoui (ros_reply_param_val_by_key (r, "cpu-load"));
	ret->cpu_frequency = sstrtoui64 (ros_reply_param_val_by_key (r, "cpu-frequency")) * 1000000;

	/* One "kilobyte" is 1024 bytes, according to "janisk", see
	 * <http://forum.mikrotik.com/viewtopic.php?f=2&t=37943> */
	ret->free_memory = sstrtoui64 (ros_reply_param_val_by_key (r, "free-memory")) * 1024;
	ret->total_memory = sstrtoui64 (ros_reply_param_val_by_key (r, "total-memory")) * 1024;

	ret->free_hdd_space = sstrtoui64 (ros_reply_param_val_by_key (r, "free-hdd-space")) * 1024;
	ret->total_hdd_space = sstrtoui64 (ros_reply_param_val_by_key (r, "total-hdd-space")) * 1024;

	ret->write_sect_since_reboot = sstrtoui64 (ros_reply_param_val_by_key (r, "write-sect-since-reboot"));
	ret->write_sect_total = sstrtoui64 (ros_reply_param_val_by_key (r, "write-sect-total"));
	ret->bad_blocks = sstrtoui64 (ros_reply_param_val_by_key (r, "bad-blocks"));

	return (0);
} /* }}} int rt_reply_to_system_resource */

static int sr_internal_handler (ros_connection_t *c, /* {{{ */
		const ros_reply_t *r, void *user_data)
{
	ros_system_resource_t sys_res;
	rt_internal_data_t *internal_data;
	int status;

	memset (&sys_res, 0, sizeof (sys_res));
	status = rt_reply_to_system_resource (r, &sys_res);
	if (status != 0)
		return (status);

	internal_data = user_data;

	status = internal_data->handler (c, &sys_res, internal_data->user_data);

	return (status);
} /* }}} int sr_internal_handler */

/*
 * Public functions
 */
int ros_system_resource (ros_connection_t *c, /* {{{ */
		ros_system_resource_handler_t handler, void *user_data)
{
	rt_internal_data_t data;

	if ((c == NULL) || (handler == NULL))
		return (EINVAL);

	data.handler = handler;
	data.user_data = user_data;

	return (ros_query (c, "/system/resource/print",
				/* args_num = */ 0, /* args = */ NULL,
				sr_internal_handler, &data));
} /* }}} int ros_system_resource */

/* vim: set ts=2 sw=2 noet fdm=marker : */
