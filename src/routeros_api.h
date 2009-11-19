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

struct mt_connection_s;
typedef struct mt_connection_s mt_connection_t;

struct mt_reply_s;
typedef struct mt_reply_s mt_reply_t;

typedef int (*mt_reply_handler_t) (mt_connection_t *c, const mt_reply_t *r,
		void *user_data);

/*
 * Connection handling
 */
mt_connection_t *mt_connect (const char *node, const char *service,
		const char *username, const char *password);
int mt_disconnect (mt_connection_t *con);

/* 
 * Command execution
 */
int mt_query (mt_connection_t *c,
		const char *command,
		size_t args_num, const char * const *args,
		mt_reply_handler_t handler, void *user_data);

/* 
 * Reply handling
 */
const mt_reply_t *mt_reply_next (const mt_reply_t *r);
int mt_reply_num (const mt_reply_t *r);

const char *mt_reply_status (const mt_reply_t *r);

/* Receiving reply parameters */
const char *mt_reply_param_key_by_index (const mt_reply_t *r,
		unsigned int index);
const char *mt_reply_param_val_by_index (const mt_reply_t *r,
		unsigned int index);
const char *mt_reply_param_val_by_key (const mt_reply_t *r, const char *key);

/* vim: set ts=2 sw=2 noet fdm=marker : */
