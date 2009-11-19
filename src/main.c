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

#ifndef _ISOC99_SOURCE
# define _ISOC99_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "routeros_api.h"


/* FIXME */
char *strdup (const char *);

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
static int read_exact (int fd, void *buffer, size_t buffer_size) /* {{{ */
{
	char *buffer_ptr;
	size_t have_bytes;

	if ((fd < 0) || (buffer == NULL))
		return (EINVAL);

	if (buffer_size == 0)
		return (0);

	buffer_ptr = buffer;
	have_bytes = 0;
	while (have_bytes < buffer_size)
	{
		size_t want_bytes;
		ssize_t status;

		want_bytes = buffer_size - have_bytes;
		errno = 0;
		status = read (fd, buffer_ptr, want_bytes);
		if (status < 0)
		{
			if (errno == EAGAIN)
				continue;
			else
				return (status);
		}

		assert (status <= want_bytes);
		have_bytes += status;
		buffer_ptr += status;
	}

	return (0);
} /* }}} int read_exact */

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

static int reply_add_keyval (mt_reply_t *r, const char *key, /* {{{ */
		const char *val)
{
	char **tmp;

	tmp = realloc (r->keys, (r->params_num + 1) * sizeof (*tmp));
	if (tmp == NULL)
		return (ENOMEM);
	r->keys = tmp;

	tmp = realloc (r->values, (r->params_num + 1) * sizeof (*tmp));
	if (tmp == NULL)
		return (ENOMEM);
	r->values = tmp;

	r->keys[r->params_num] = strdup (key);
	if (r->keys[r->params_num] == NULL)
		return (ENOMEM);

	r->values[r->params_num] = strdup (val);
	if (r->values[r->params_num] == NULL)
	{
		free (r->keys[r->params_num]);
		r->keys[r->params_num] = NULL;
		return (ENOMEM);
	}

	r->params_num++;
	return (0);
} /* }}} int reply_add_keyval */

static void reply_free (mt_reply_t *r) /* {{{ */
{
	mt_reply_t *next;
	unsigned int i;

	if (r == NULL)
		return;

	next = r->next;

	for (i = 0; i < r->params_num; i++)
	{
		free (r->keys[i]);
		free (r->values[i]);
	}

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

static int send_command (mt_connection_t *c, /* {{{ */
		const char *command,
		size_t args_num, const char * const *args)
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

	buffer_ptr = buffer;
	while (buffer_size > 0)
	{
		ssize_t bytes_written;

		errno = 0;
		bytes_written = write (c->fd, buffer_ptr, buffer_size);
		if (bytes_written < 0)
		{
			if (errno == EAGAIN)
				continue;
			else
				return (errno);
		}
		assert (bytes_written <= buffer_size);

		buffer_ptr += bytes_written;
		buffer_size -= bytes_written;
	} /* while (buffer_size > 0) */

	return (0);
} /* }}} int send_command */

static int read_word (mt_connection_t *c, /* {{{ */
		char *buffer, size_t *buffer_size)
{
	size_t req_size;
	uint8_t word_length[5];
	int status;

	if ((buffer == NULL) || (*buffer_size < 1))
		return (EINVAL);

	/* read one byte from the socket */
	status = read_exact (c->fd, word_length, 1);
	if (status != 0)
		return (status);

	/* Calculate `req_size' */
	if (((unsigned char) buffer[0]) == 0xF0) /* {{{ */
	{
		status = read_exact (c->fd, &word_length[1], 4);
		if (status != 0)
			return (status);

		req_size = (buffer[1] << 24)
			| (buffer[2] << 16)
			| (buffer[3] << 8)
			| buffer[4];
	}
	else if ((buffer[0] & 0xE0) == 0xE0)
	{
		status = read_exact (c->fd, &word_length[1], 3);
		if (status != 0)
			return (status);

		req_size = ((buffer[0] & 0x1F) << 24)
			| (buffer[1] << 16)
			| (buffer[2] << 8)
			| buffer[3];
	}
	else if ((buffer[0] & 0xC0) == 0xC0)
	{
		status = read_exact (c->fd, &word_length[1], 2);
		if (status != 0)
			return (status);

		req_size = ((buffer[0] & 0x3F) << 16)
			| (buffer[1] << 8)
			| buffer[2];
	}
	else if ((buffer[0] & 0x80) == 0x80)
	{
		status = read_exact (c->fd, &word_length[1], 1);
		if (status != 0)
			return (status);

		req_size = ((buffer[0] & 0x7F) << 8)
			| buffer[1];
	}
	else if ((buffer[0] & 0x80) == 0)
	{
		req_size = (size_t) word_length[0];
	}
	else
	{
		/* First nibble is `F' but second nibble is not `0'. */
		return (EPROTO);
	} /* }}} */

	if (*buffer_size < req_size)
		return (ENOMEM);

	/* Empty word. This ends a `sentence' and must therefore be valid. */
	if (req_size == 0)
	{
		buffer[0] = 0;
		*buffer_size = 0;
		return (0);
	}

	status = read_exact (c->fd, buffer, req_size);
	if (status != 0)
		return (status);
	*buffer_size = req_size;

	return (0);
} /* }}} int buffer_decode_next */

static mt_reply_t *receive_reply (mt_connection_t *c) /* {{{ */
{
	char buffer[4096];
	size_t buffer_size;
	int status;

	mt_reply_t *head;
	mt_reply_t *tail;

	head = NULL;
	tail = NULL;

	while (42)
	{
		buffer_size = sizeof (buffer) - 1;
		status = read_word (c, buffer, &buffer_size);
		if (status != 0)
			break;
		assert (buffer_size < sizeof (buffer));
		buffer[buffer_size] = 0;

		/* Empty word means end of reply */
		if (buffer_size == 0)
			break;

		if (buffer[0] == '!') /* {{{ */
		{
			mt_reply_t *tmp;

			tmp = reply_alloc ();
			if (tmp == NULL)
			{
				status = ENOMEM;
				break;
			}

			tmp->status = strdup (&buffer[1]);
			if (tmp->status == NULL)
			{
				reply_free (tmp);
				status = ENOMEM;
				break;
			}

			if (tail == NULL)
			{
				head = tmp;
				tail = tmp;
			}
			else
			{
				tail->next = tmp;
				tail = tmp;
			}
		} /* }}} if (buffer[0] == '!') */
		else if (buffer[0] == '=') /* {{{ */
		{
			char *key = &buffer[1];
			char *val;

			key = &buffer[1];
			val = strchr (key, '=');
			if (val == NULL)
			{
				fprintf (stderr, "Ignoring misformed word: %s\n", buffer);
				continue;
			}
			*val = 0;
			val++;

			reply_add_keyval (tail, key, val);
		} /* }}} if (buffer[0] == '=') */
		else
		{
			printf ("Ignoring unknown word: %s\n", buffer);
		}
	} /* while (42) */
	
	if (status != 0)
	{
		reply_free (head);
		return (NULL);
	}

	return (head);
} /* }}} mt_reply_t *receive_reply */

static int create_socket (const char *node, const char *service) /* {{{ */
{
	struct addrinfo  ai_hint;
	struct addrinfo *ai_list;
	struct addrinfo *ai_ptr;
	int status;

	memset (&ai_hint, 0, sizeof (ai_hint));
#ifdef AI_ADDRCONFIG
	ai_hint.ai_flags |= AI_ADDRCONFIG;
#endif
	ai_hint.ai_family = AF_UNSPEC;
	ai_hint.ai_socktype = SOCK_STREAM;

	ai_list = NULL;
	status = getaddrinfo (node, service, &ai_hint, &ai_list);
	if (status != 0)
		return (-1);
	assert (ai_list != NULL);

	for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next)
	{
		int fd;
		int status;

		fd = socket (ai_ptr->ai_family, ai_ptr->ai_socktype,
				ai_ptr->ai_protocol);
		if (fd < 0)
			continue;

		status = connect (fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
		if (status != 0)
		{
			close (fd);
			continue;
		}

		return (fd);
	}

	freeaddrinfo (ai_list);

	return (-1);
} /* }}} int create_socket */

/*
 * Public functions
 */
mt_connection_t *mt_connect (const char *node, const char *service, /* {{{ */
		const char *username, const char *password)
{
	int fd;
	mt_connection_t *c;

	if ((node == NULL) || (username == NULL) || (password == NULL))
		return (NULL);

	fd = create_socket (node, (service != NULL) ? service : ROUTEROS_API_PORT);
	if (fd < 0)
		return (NULL);

	c = malloc (sizeof (*c));
	if (c == NULL)
	{
		close (fd);
		return (NULL);
	}
	memset (c, 0, sizeof (*c));

	c->fd = fd;

	return (c);
} /* }}} mt_connection_t *mt_connect */

int mt_disconnect (mt_connection_t *c) /* {{{ */
{
	if (c == NULL)
		return (EINVAL);

	if (c->fd >= 0)
	{
		close (c->fd);
		c->fd = -1;
	}

	free (c);

	return (0);
} /* }}} int mt_disconnect */

int mt_query (mt_connection_t *c, /* {{{ */
		const char *command,
		size_t args_num, const char * const *args,
		mt_reply_handler_t *handler, void *user_data)
{
	int status;
	mt_reply_t *r;

	status = send_command (c, command, args_num, args);
	if (status != 0)
		return (status);

	r = receive_reply (c);
	if (r == NULL)
		return (EPROTO);

	/* Call the callback function with the data we received. */
	status = (*handler) (c, r, user_data);

	/* Free the allocated memory ... */
	reply_free (r);

	/* ... and return. */
	return (status);
} /* }}} int mt_query */

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
