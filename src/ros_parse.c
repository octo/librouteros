/**
 * librouteros - src/ros_parse.c
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
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <errno.h>
#include <assert.h>

#include "routeros_api.h"

_Bool sstrtob (const char *str) /* {{{ */
{
	if (str == NULL)
		return (false);

	if (strcasecmp ("true", str) == 0)
		return (true);
	return (false);
} /* }}} _Bool sstrtob */

unsigned int sstrtoui (const char *str) /* {{{ */
{
	unsigned int ret;
	char *endptr;

	if (str == NULL)
		return (0);

	errno = 0;
	endptr = NULL;
	ret = (unsigned int) strtoul (str, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
		return (0);

	return (ret);
} /* }}} unsigned int sstrtoui */

uint64_t sstrtoui64 (const char *str) /* {{{ */
{
	uint64_t ret;
	char *endptr;

	if (str == NULL)
		return (0);

	errno = 0;
	endptr = NULL;
	ret = (uint64_t) strtoull (str, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
		return (0);

	return (ret);
} /* }}} uint64_t sstrtoui64 */

double sstrtod (const char *str) /* {{{ */
{
	double ret;
	char *endptr;

	if (str == NULL)
		return (NAN);

	errno = 0;
	endptr = NULL;
	ret = strtod (str, &endptr);
	if ((endptr == str) || (errno != 0))
		return (NAN);

	return (ret);
} /* }}} double sstrtod */

int sstrto_rx_tx_counters (const char *str, /* {{{ */
		uint64_t *rx, uint64_t *tx)
{
	const char *ptr;
	char *endptr;

	if ((rx == NULL) || (tx == NULL))
		return (EINVAL);

	*rx = 0;
	*tx = 0;

	if (str == NULL)
		return (EINVAL);

	ptr = str;
	errno = 0;
	endptr = NULL;
	*rx = (uint64_t) strtoull (ptr, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
	{
		*rx = 0;
		return (EIO);
	}

	assert (endptr != NULL);
	if ((*endptr != '/') && (*endptr != ','))
		return (EIO);

	ptr = endptr + 1;
	errno = 0;
	endptr = NULL;
	*tx = (uint64_t) strtoull (ptr, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
	{
		*rx = 0;
		*tx = 0;
		return (EIO);
	}

	return (0);
} /* }}} int sstrto_rx_tx_counters */

/* have_hour is initially set to false and later set to true when the first
 * colon is found. It is used to determine whether the number before the colon
 * is hours or minutes. External code should use the sstrtodate() macro. */
uint64_t _sstrtodate (const char *str, _Bool have_hour) /* {{{ */
{
	uint64_t ret;
	char *endptr;

	if ((str == NULL) || (*str == 0))
		return (0);

	/* Example string: 6w6d18:33:07 */
	errno = 0;
	endptr = NULL;
	ret = (uint64_t) strtoull (str, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
		return (0);

	switch (*endptr)
	{
		case 'y': ret *= 365 * 86400; break;
		case 'w': ret *=   7 * 86400; break;
		case 'd': ret *=       86400; break;
		case ':': ret *= have_hour ? 60 : 3600; have_hour = true; break;
	}

	return (ret + _sstrtodate (endptr + 1, have_hour));
} /* }}} uint64_t _sstrtodate */

/* vim: set ts=2 sw=2 noet fdm=marker : */
