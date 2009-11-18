/**
 * libmikrotik - src/main.c
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "routeros_api.h"

/*
 * Private structures
 */
struct mt_connection_s
{
	int fd;
};

struct mt_reply_s
{
	unsigned int params_num;
	char *status;
	char **keys;
	char **values;

	mt_reply_t *next;
};

/*
 * Private functions
 */
static mt_reply_t *reply_alloc (void) /* {{{ */
{
	mt_reply_t *r;

	r = malloc (sizeof (*r));
	if (r == NULL)
		return (NULL);

	memset (r, 0, sizeof (*r));
	r->keys = NULL;
	r->values = NULL;
	r->next = NULL;

	return (r);
} /* }}} mt_reply_s *reply_alloc */

static void reply_free (mt_reply_t *r) /* {{{ */
{
	mt_reply_t *next;

	if (r == NULL)
		return;

	next = r->next;

	free (r->keys);
	free (r->values);
	free (r);

	reply_free (next);
} /* }}} void reply_free */

static int buffer_init (char **ret_buffer, size_t *ret_buffer_size) /* {{{ */
{
	if ((ret_buffer == NULL) || (ret_buffer_size == NULL))
		return (EINVAL);

	if (*ret_buffer_size < 1)
		return (EINVAL);

	return (0);
} /* }}} int buffer_init */

static int buffer_add (char **ret_buffer, size_t *ret_buffer_size, /* {{{ */
		const char *string)
{
	char *buffer;
	size_t buffer_size;
	size_t string_size;
	size_t req_size;

	if ((ret_buffer == NULL) || (ret_buffer_size == NULL) || (string == NULL))
		return (EINVAL);

	buffer = *ret_buffer;
	buffer_size = *ret_buffer_size;

	string_size = strlen (string);
	if (string_size == 0)
		return (EINVAL);

	if (string_size >= 0x10000000)
		req_size = 5 + string_size;
	else if (string_size >= 0x200000)
		req_size = 4 + string_size;
	else if (string_size >= 0x4000)
		req_size = 3 + string_size;
	else if (string_size >= 0x80)
		req_size = 2 + string_size;
	else
		req_size = 1 + string_size;

	if (*ret_buffer_size < req_size)
		return (ENOMEM);

	if (string_size >= 0x10000000)
	{
		buffer[0] = 0xF0;
		buffer[1] = (string_size >> 24) & 0xff;
		buffer[2] = (string_size >> 16) & 0xff;
		buffer[3] = (string_size >>  8) & 0xff;
		buffer[4] = (string_size      ) & 0xff;
		buffer += 5;
		buffer_size -= 5;
	}
	else if (string_size >= 0x200000)
	{
		buffer[0] = (string_size >> 24) & 0x1f;
		buffer[0] |= 0xE0;
		buffer[1] = (string_size >> 16) & 0xff;
		buffer[2] = (string_size >>  8) & 0xff;
		buffer[3] = (string_size      ) & 0xff;
		buffer += 4;
		buffer_size -= 4;
	}
	else if (string_size >= 0x4000)
	{
		buffer[0] = (string_size >> 16) & 0x3f;
		buffer[0] |= 0xC0;
		buffer[1] = (string_size >>  8) & 0xff;
		buffer[2] = (string_size      ) & 0xff;
		buffer += 3;
		buffer_size -= 3;
	}
	else if (string_size >= 0x80)
	{
		buffer[0] = (string_size >>  8) & 0x7f;
		buffer[0] |= 0x80;
		buffer[1] = (string_size      ) & 0xff;
		buffer += 2;
		buffer_size -= 2;
	}
	else /* if (string_size <= 0x7f) */
	{
		buffer[0] = (char) string_size;
		buffer += 1;
		buffer_size -= 1;
	}

	assert (buffer_size >= string_size);
	memcpy (buffer, string, string_size);
	buffer += string_size;
	buffer_size -= string_size;

	*ret_buffer = buffer;
	*ret_buffer_size = buffer_size;

	return (0);
} /* }}} int buffer_add */

static int buffer_end (char **ret_buffer, size_t *ret_buffer_size) /* {{{ */
{
	if ((ret_buffer == NULL) || (ret_buffer_size == NULL))
		return (EINVAL);

	if (*ret_buffer_size < 1)
		return (EINVAL);

	/* Add empty word. */
	(*ret_buffer)[0] = 0;
	(*ret_buffer)++;
	(*ret_buffer_size)--;

	return (0);
} /* }}} int buffer_end */

static int buffer_decode_next (char **ret_buffer, size_t *ret_buffer_size, /* {{{ */
		char *dst, size_t *dst_size)
{
	uint8_t *buffer;
	size_t buffer_size;
	size_t req_size;

	if ((ret_buffer == NULL) || (ret_buffer_size == NULL) || (dst_size == NULL))
		return (EINVAL);

	buffer = (uint8_t *) (*ret_buffer);
	buffer_size = *ret_buffer_size;

	if (buffer_size < 1)
		return (EINVAL);

	/* Calculate `req_size' and update `buffer' and `buffer_size'. */
	if (buffer[0] == 0xF0) /* {{{ */
	{
		if (buffer_size < (0x10000000 + 5))
			return (EPROTO);
		req_size = (buffer[1] << 24)
			| (buffer[2] << 16)
			| (buffer[3] << 8)
			| buffer[4];
		buffer += 5;
		buffer_size -= 5;
	}
	else if ((buffer[0] & 0xE0) == 0xE0)
	{
		if (buffer_size < (0x200000 + 4))
			return (EPROTO);
		req_size = ((buffer[0] & 0x1F) << 24)
			| (buffer[1] << 16)
			| (buffer[2] << 8)
			| buffer[3];
		buffer += 4;
		buffer_size -= 4;
	}
	else if ((buffer[0] & 0xC0) == 0xC0)
	{
		if (buffer_size < (0x4000 + 3))
			return (EPROTO);
		req_size = ((buffer[0] & 0x3F) << 16)
			| (buffer[1] << 8)
			| buffer[2];
		buffer += 3;
		buffer_size -= 3;
	}
	else if ((buffer[0] & 0x80) == 0x80)
	{
		if (buffer_size < (0x80 + 2))
			return (EPROTO);
		req_size = ((buffer[0] & 0x7F) << 8) | buffer[1];
		buffer += 2;
		buffer_size -= 2;
	}
	else if ((buffer[0] & 0x80) == 0)
	{
		req_size = buffer[0];
		buffer += 1;
		buffer_size -= 1;
	}
	else
	{
		/* First nibble is `F' but second nibble is not `0'. */
		return (EPROTO);
	} /* }}} */

	if (buffer_size < req_size)
		return (EPROTO);

	if (dst == NULL)
	{
		*dst_size = (req_size + 1);
		return (0);
	}

	if (*dst_size <= req_size)
		return (ENOMEM);

	memcpy (dst, buffer, req_size);
	dst[req_size] = 0;
	*dst_size = (req_size + 1);

	assert (buffer_size >= req_size);
	buffer += req_size;
	buffer_size -= req_size;

	*ret_buffer = buffer;
	*ret_buffer_size = buffer_size;
	
	return (0);
} /* }}} int buffer_decode_next */

/*
 * Public functions
 */
mt_connection_t *mt_connect (const char *node, const char *service,
		const char *username, const char *password);
int mt_disconnect (mt_connection_t *con);

int mt_query (mt_connection_t *c,
		const char *command,
		size_t args_num, const char * const *args,
		mt_reply_handler_t *handler, void *user_data)
{
	char buffer[4096];
	char *buffer_ptr;
	size_t buffer_size;

	size_t i;
	int status;

	buffer_ptr = buffer;
	buffer_size = sizeof (buffer);

	status = buffer_init (&buffer_ptr, &buffer_size);
	if (status != 0)
		return (status);

	status = buffer_add (&buffer_ptr, &buffer_size, command);
	if (status != 0)
		return (status);

	for (i = 0; i < args_num; i++)
	{
		if (args[i] == NULL)
			return (EINVAL);

		status = buffer_add (&buffer_ptr, &buffer_size, args[i]);
		if (status != 0)
			return (status);
	}

	status = buffer_end (&buffer_ptr, &buffer_size);
	if (status != 0)
		return (status);

	/* FIXME: Send out the buffer and read back results. */
}

const mt_reply_t *mt_reply_next (const mt_reply_t *r) /* {{{ */
{
	if (r == NULL)
		return (NULL);

	return (r->next);
} /* }}} mt_reply_t *mt_reply_next */

int mt_reply_num (const mt_reply_t *r) /* {{{ */
{
	int ret;
	const mt_reply_t *ptr;

	ret = 0;
	for (ptr = r; ptr != NULL; ptr = ptr->next)
		ret++;

	return (ret);
} /* }}} int mt_reply_num */

const char *mt_reply_param_key_by_index (const mt_reply_t *r, /* {{{ */
		unsigned int index)
{
	if (r == NULL)
		return (NULL);

	if (index >= r->params_num)
		return (NULL);

	return (r->keys[index]);
} /* }}} char *mt_reply_param_key_by_index */

const char *mt_reply_param_val_by_index (const mt_reply_t *r, /* {{{ */
		unsigned int index)
{
	if (r == NULL)
		return (NULL);

	if (index >= r->params_num)
		return (NULL);

	return (r->values[index]);
} /* }}} char *mt_reply_param_key_by_index */

const char *mt_reply_param_val_by_key (const mt_reply_t *r, /* {{{ */
		const char *key)
{
	unsigned int i;

	if ((r == NULL) || (key == NULL))
		return (NULL);

	for (i = 0; i < r->params_num; i++)
		if (strcmp (r->keys[i], key) == 0)
			return (r->values[i]);

	return (NULL);
} /* }}} char *mt_reply_param_val_by_key */

/* vim: set ts=2 sw=2 noet fdm=marker : */
