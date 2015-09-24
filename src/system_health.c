/**
 * librouteros - src/system_health.c
 * Copyright (C) 2009  Florian octo Forster
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the License is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Authors:
 *   Florian octo Forster <octo at verplant.org>
 *   Mariusz Bialonczyk <manio@skyboo.net>
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
	ros_system_health_handler_t handler;
	void *user_data;
};
typedef struct rt_internal_data_s rt_internal_data_t;

/*
 * Private functions
 */
static int rt_reply_to_system_health (const ros_reply_t *r, /* {{{ */
		ros_system_health_t *ret)
{
	if (r == NULL)
		return (EINVAL);

	if (strcmp ("re", ros_reply_status (r)) != 0)
		return (rt_reply_to_system_health (ros_reply_next (r), ret));

	ret->voltage = sstrtod (ros_reply_param_val_by_key (r, "voltage"));
	ret->temperature = sstrtod (ros_reply_param_val_by_key (r, "temperature"));

	return (0);
} /* }}} int rt_reply_to_system_health */

static int sh_internal_handler (ros_connection_t *c, /* {{{ */
		const ros_reply_t *r, void *user_data)
{
	ros_system_health_t sys_health;
	rt_internal_data_t *internal_data;
	int status;

	memset (&sys_health, 0, sizeof (sys_health));
	status = rt_reply_to_system_health (r, &sys_health);
	if (status != 0)
		return (status);

	internal_data = user_data;

	status = internal_data->handler (c, &sys_health, internal_data->user_data);

	return (status);
} /* }}} int sh_internal_handler */

/*
 * Public functions
 */
int ros_system_health (ros_connection_t *c, /* {{{ */
		ros_system_health_handler_t handler, void *user_data)
{
	rt_internal_data_t data;

	if ((c == NULL) || (handler == NULL))
		return (EINVAL);

	data.handler = handler;
	data.user_data = user_data;

	return (ros_query (c, "/system/health/print",
				/* args_num = */ 0, /* args = */ NULL,
				sh_internal_handler, &data));
} /* }}} int ros_system_health */

/* vim: set ts=2 sw=2 noet fdm=marker : */
