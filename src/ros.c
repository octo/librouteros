/**
 * libmikrotik - src/ros.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <getopt.h>

#include "routeros_api.h"

#if !__GNUC__
# define __attribute__(x) /**/
#endif

static const char *opt_username = "admin";

static int result_handler (ros_connection_t *c, const ros_reply_t *r, /* {{{ */
		void *user_data)
{
	unsigned int i;

	if (r == NULL)
		return (0);

	printf ("Status: %s\n", ros_reply_status (r));

	for (i = 0; /* true */; i++)
	{
		const char *key;
		const char *val;

		key = ros_reply_param_key_by_index (r, i);
		val = ros_reply_param_val_by_index (r, i);

		if ((key == NULL) || (val == NULL))
		{
			if (key != NULL)
				fprintf (stderr, "val is NULL but key is %s!\n", key);
			if (val != NULL)
				fprintf (stderr, "key is NULL but val is %s!\n", val);
			break;
		}

		printf ("  Param %u: %s = %s\n", i, key, val);
	}

	printf ("===\n");

	return (result_handler (c, ros_reply_next (r), user_data));
} /* }}} int result_handler */

static void regtable_dump (const ros_registration_table_t *r) /* {{{ */
{
	if (r == NULL)
		return;

	printf ("=== %s / %s ===\n", r->interface, r->radio_name);
	printf ("Mode:           %12s\n",
			r->ap ? (r->wds ? "AP with WDS" : "Access point") : "Station");
	printf ("Rate:           %7g Mbps / %7g Mbps\n", r->rx_rate, r->tx_rate);
	printf ("Packets:        %12"PRIu64" / %12"PRIu64"\n",
			r->rx_packets, r->tx_packets);
	printf ("Bytes:          %12"PRIu64" / %12"PRIu64"\n",
			r->rx_bytes, r->tx_bytes);
	printf ("Frames          %12"PRIu64" / %12"PRIu64"\n",
			r->rx_frames, r->tx_frames);
	printf ("Frame Bytes:    %12"PRIu64" / %12"PRIu64"\n",
			r->rx_frame_bytes, r->tx_frame_bytes);
	printf ("HW Frames:      %12"PRIu64" / %12"PRIu64"\n",
			r->rx_hw_frames, r->tx_hw_frames);
	printf ("HW Frame Bytes: %12"PRIu64" / %12"PRIu64"\n",
			r->rx_hw_frame_bytes, r->tx_hw_frame_bytes);
	printf ("Quality:        %10g %% / %10g %%\n",
			r->rx_ccq, r->tx_ccq);
	printf ("Signal str.:    %8g dBm / %8g dBm\n",
		r->rx_signal_strength, r->tx_signal_strength);
	printf ("Signal / noise: %8g dBm\n", r->signal_to_noise);
	printf ("==========\n");

	regtable_dump (r->next);
} /* }}} void regtable_dump */

static int regtable_handler (__attribute__((unused)) ros_connection_t *c, /* {{{ */
		const ros_registration_table_t *r,
		__attribute__((unused)) void *user_data)
{
	regtable_dump (r);
	return (0);
} /* }}} int regtable_handler */

static void interface_dump (const ros_interface_t *i) /* {{{ */
{
	if (i == NULL)
		return;

	printf ("=== %s ===\n"
			"Type:    %12s\n"
			"Comment: %12s\n"
			"Bytes:   %12"PRIu64" / %12"PRIu64"\n"
			"Packets: %12"PRIu64" / %12"PRIu64"\n"
			"Errors:  %12"PRIu64" / %12"PRIu64"\n"
			"Drops:   %12"PRIu64" / %12"PRIu64"\n"
			"MTU:     %12u\n"
			"L2 MTU:  %12u\n"
			"Running: %12s\n"
			"Dynamic: %12s\n"
			"Enabled: %12s\n"
			"==========\n",
			i->name, i->type, i->comment,
			i->rx_bytes, i->tx_bytes,
			i->rx_packets, i->tx_packets,
			i->rx_errors, i->tx_errors,
			i->rx_drops, i->tx_drops,
			i->mtu, i->l2mtu,
			i->running ? "true" : "false",
			i->dynamic ? "true" : "false",
			i->enabled ? "true" : "false");

	interface_dump (i->next);
} /* }}} void interface_dump */

static int interface_handler (__attribute__((unused)) ros_connection_t *c, /* {{{ */
		const ros_interface_t *i,
		__attribute__((unused)) void *user_data)
{
	interface_dump (i);
	return (0);
} /* }}} int interface_handler */

static int system_resource_handler (__attribute__((unused)) ros_connection_t *c, /* {{{ */
		const ros_system_resource_t *r, __attribute__((unused)) void *user_data)
{
	uint64_t used_memory;
	uint64_t used_hdd_space;

	if (r == NULL)
		return (EINVAL);

	used_memory = r->total_memory - r->free_memory;
	used_hdd_space = r->total_hdd_space - r->free_hdd_space;

	printf ("====== System resources ======\n"
			"Uptime:        %10.2f days\n"
			"RouterOS version:  %11s\n"
			"Architecture name: %11s\n"
			"Board name:    %15s\n"
			"CPU model:     %15s\n"
			"CPU count:     %15u\n"
			"CPU load:      %15u\n"
			"CPU frequency: %11g MHz\n"
			"Memory free:   %10"PRIu64" Byte (%4.1f %%)\n"
			"Memory used:   %10"PRIu64" Byte (%4.1f %%)\n"
			"Memory total:  %10"PRIu64" Byte\n"
			"Space free:    %10"PRIu64" Byte (%4.1f %%)\n"
			"Space used:    %10"PRIu64" Byte (%4.1f %%)\n"
			"Space total:   %10"PRIu64" Byte\n"
			"Sectors written: %13"PRIu64" (%"PRIu64")\n"
			"Bad blocks:    %15"PRIu64"\n"
			"==============================\n",
			((double) r->uptime) / 86400.0, r->version,
			r->architecture_name, r->board_name,
			r->cpu_model, r->cpu_count, r->cpu_load,
			((double) r->cpu_frequency) / 1000000.0,
			r->free_memory,
			100.0 * (((double) r->free_memory) / ((double) r->total_memory)),
			used_memory,
			100.0 * (((double) used_memory) / ((double) r->total_memory)),
			r->total_memory,
			r->free_hdd_space,
			100.0 * (((double) r->free_hdd_space) / ((double) r->total_hdd_space)),
			used_hdd_space,
			100.0 * (((double) used_hdd_space) / ((double) r->total_hdd_space)),
			r->total_hdd_space,
			r->write_sect_since_reboot,
			r->write_sect_total,
			r->bad_blocks);

	return (0);
} /* }}} int system_resource_handler */

static char *read_password (void) /* {{{ */
{
	FILE *tty;
	struct termios old_flags;
	struct termios new_flags;
	int status;
	char buffer[1024];
	size_t buffer_len;
	char *passwd;

	tty = fopen ("/dev/tty", "w+");
	if (tty == NULL)
	{
		fprintf (stderr, "Unable to open /dev/tty: %s\n",
				strerror (errno));
		return (NULL);
	}

	fprintf (tty, "Password for user %s: ", opt_username);
	fflush (tty);

	memset (&old_flags, 0, sizeof (old_flags));
	tcgetattr (fileno (tty), &old_flags);
	new_flags = old_flags;
	/* clear ECHO */
	new_flags.c_lflag &= ~ECHO;
	/* set ECHONL */
	new_flags.c_lflag |= ECHONL;

	status = tcsetattr (fileno (tty), TCSANOW, &new_flags);
	if (status != 0)
	{
		fprintf (stderr, "tcsetattr failed: %s\n", strerror (errno));
		fclose (tty);
		return (NULL);
	}

	if (fgets (buffer, sizeof (buffer), tty) == NULL)
	{
		fprintf (stderr, "fgets failed: %s\n", strerror (errno));
		fclose (tty);
		return (NULL);
	}
	buffer[sizeof (buffer) - 1] = 0;
	buffer_len = strlen (buffer);

	status = tcsetattr (fileno (tty), TCSANOW, &old_flags);
	if (status != 0)
		fprintf (stderr, "tcsetattr failed: %s\n", strerror (errno));

	fclose (tty);
	tty = NULL;

	while ((buffer_len > 0) && ((buffer[buffer_len-1] == '\n') || (buffer[buffer_len-1] == '\r')))
	{
		buffer_len--;
		buffer[buffer_len] = 0;
	}
	if (buffer_len == 0)
		return (NULL);

	passwd = malloc (strlen (buffer) + 1);
	if (passwd == NULL)
		return (NULL);
	memcpy (passwd, buffer, strlen (buffer) + 1);
	memset (buffer, 0, sizeof (buffer));

	return (passwd);
} /* }}} char *read_password */

static void exit_usage (void) /* {{{ */
{
	printf ("Usage: ros [options] <host> <command> [args]\n"
			"\n"
			"OPTIONS:\n"
			"  -u <user>       Use <user> to authenticate.\n"
			"  -h              Display this help message.\n"
			"\n");
	if (ros_version () == ROS_VERSION)
		printf ("Using librouteros %s\n", ROS_VERSION_STRING);
	else
		printf ("Using librouteros %s (%s)\n",
				ros_version_string (), ROS_VERSION_STRING);
	printf ("Copyright (c) 2009 by Florian Forster\n");

	exit (EXIT_SUCCESS);
} /* }}} void exit_usage */

int main (int argc, char **argv) /* {{{ */
{
	ros_connection_t *c;
	char *passwd;
	const char *host;
	const char *command;

	int option;

	while ((option = getopt (argc, argv, "u:h?")) != -1)
	{
		switch (option)
		{
			case 'u':
				opt_username = optarg;
				break;

			case 'h':
			case '?':
			default:
				exit_usage ();
				break;
		}
	}

	if ((argc - optind) < 2)
		exit_usage ();

	host = argv[optind];
	command = argv[optind+1];

	passwd = read_password ();
	if (passwd == NULL)
		exit (EXIT_FAILURE);

	c = ros_connect (host, ROUTEROS_API_PORT,
			opt_username, passwd);
	memset (passwd, 0, strlen (passwd));
	if (c == NULL)
	{
		fprintf (stderr, "ros_connect failed: %s\n", strerror (errno));
		exit (EXIT_FAILURE);
	}

	if (command[0] == '/')
	{
		ros_query (c, command,
				(size_t) (argc - (optind + 2)), (const char * const *) (argv + optind + 2),
				result_handler, /* user data = */ NULL);
	}
	else if (strcmp ("interface", command) == 0)
	{
		ros_interface (c, interface_handler, /* user data = */ NULL);
	}
	else if (strcmp ("registration-table", command) == 0)
	{
		ros_registration_table (c, regtable_handler, /* user data = */ NULL);
	}
	else if (strcmp ("system-resource", command) == 0)
	{
		ros_system_resource (c, system_resource_handler, /* user data = */ NULL);
	}
	else
	{
		fprintf (stderr, "Unknown built-in command %s. "
				"Are you missing a leading slash?\n", command);
	}

	ros_disconnect (c);

	return (0);
} /* }}} int main */

/* vim: set ts=2 sw=2 noet fdm=marker : */
