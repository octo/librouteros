/**
 * librouteros - src/ros_parse.h
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

#ifndef ROS_PARSE_H
#define ROS_PARSE_H 1

_Bool sstrtob (const char *str);

unsigned int sstrtoui (const char *str);
uint64_t sstrtoui64 (const char *str);

double sstrtod (const char *str);

int sstrto_rx_tx_counters (const char *str, uint64_t *rx, uint64_t *tx);

#endif /* ROS_PARSE_H */

/* vim: set ts=2 sw=2 noet fdm=marker : */
