/**
 * libmikrotik - src/mikrotik.h
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
 **/

#define ROUTEROS_API_PORT "8728"

struct ros_connection_s;
typedef struct ros_connection_s ros_connection_t;

struct ros_reply_s;
typedef struct ros_reply_s ros_reply_t;

typedef int (*ros_reply_handler_t) (ros_connection_t *c, const ros_reply_t *r,
		void *user_data);

/*
 * Connection handling
 */
ros_connection_t *ros_connect (const char *node, const char *service,
		const char *username, const char *password);
int ros_disconnect (ros_connection_t *con);

/* 
 * Command execution
 */
int ros_query (ros_connection_t *c,
		const char *command,
		size_t args_num, const char * const *args,
		ros_reply_handler_t handler, void *user_data);

/* 
 * Reply handling
 */
const ros_reply_t *ros_reply_next (const ros_reply_t *r);
int ros_reply_num (const ros_reply_t *r);

const char *ros_reply_status (const ros_reply_t *r);

/* Receiving reply parameters */
const char *ros_reply_param_key_by_index (const ros_reply_t *r,
		unsigned int index);
const char *ros_reply_param_val_by_index (const ros_reply_t *r,
		unsigned int index);
const char *ros_reply_param_val_by_key (const ros_reply_t *r, const char *key);

/* vim: set ts=2 sw=2 noet fdm=marker : */
