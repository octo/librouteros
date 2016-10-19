/**
 * librouteros - src/ros_parse.h
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

#ifndef ROS_PARSE_H
#define ROS_PARSE_H 1

_Bool sstrtob (const char *str);

unsigned int sstrtoui (const char *str);
uint64_t sstrtoui64 (const char *str);

double sstrtod (const char *str);

int sstrto_rx_tx_counters (const char *str, uint64_t *rx, uint64_t *tx);

uint64_t _sstrtodate (const char *str, _Bool have_hour);
#define sstrtodate(str) _sstrtodate((str), 0)

#endif /* ROS_PARSE_H */

/* vim: set ts=2 sw=2 noet fdm=marker : */
