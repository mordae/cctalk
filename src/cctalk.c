/*
 * Copyright (C) 2013  Jan Dvorak <mordae@anilinux.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "cctalk.h"

#include <error.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

static const struct option longopts[] = {
	{"help",     0, 0, 'h'},
	{"version",  0, 0, 'V'},
	{"device",   1, 0, 'd'},
	{"simple",   0, 0, 's'},
	{"ccitt",    0, 0, 'c'},
	{"host-id",  1, 0, 'i'},
	{"timeout",  1, 0, 't'},

	{0, 0, 0, 0},
};

static const char optstring[] = "hVscd:i:t:";

static char *device = NULL;
static enum cctalk_crc_mode crc_mode = CCTALK_CRC_SIMPLE;
static uint8_t host_id = 1;
static int timeout = 1000;

static int do_version(int argc, char **argv)
{
	printf("cctalk %s\n", VERSION);
	return 0;
}

static int do_help(int argc, char **argv)
{
	puts("cctalk [--device=/dev/ttyUSB0] peer-id msg-code arg...");
	puts("Send request to a ccTalk compatible device.");
	puts("");
	puts("ACTIONS:");
	puts("  --help, -h     Display this help.");
	puts("  --version, -V  Display version information.");
	puts("");
	puts("OPTIONS:");
	puts("  --simple, -s   Use the default 8-bit checksums.");
	puts("  --ccitt, -c    Use 16-bit checksums.");
	puts("  --timeout, -t 1000");
	puts("                 Set communication timeout in milliseconds.");
	puts("  --host-id, -i 1");
	puts("                 Change address used by the host.");
	puts("  --device, -d /dev/ttyUSB0");
	puts("                 Select serial line device to communicate on.");
	puts("");
	puts("Report bugs to Jan Dvorak <mordae@anilinux.org>.");
	return 0;
}

static int do_talk(int argc, char **argv)
{
	struct cctalk_message *msg;
	struct cctalk_host *host;
	uint8_t fields[257] = {0};
	int i;

	if (argc < 2)
		error(1, 0, "not enough parameters specified");

	for (i = 0; i < argc && i < 257; i++)
		fields[i] = atoi(argv[i]);

	if (NULL == (host = cctalk_host_new(device)))
		error(1, errno, "failed to open device %s", device);

	host->crc_mode = crc_mode;
	host->id = host_id;

	if (-1 == cctalk_send(host, fields[0], fields[1], argc - 2, fields + 2))
		error(1, errno, "message could not be sent");

	if (NULL == (msg = cctalk_recv(host)))
		error(1, errno, "no message received");

	printf("%s: status=%i, source=%i, destination=%i\ndata:",
	       (msg->header ? "error" : "success"), msg->header,
	       (crc_mode == CCTALK_CRC_SIMPLE ? msg->source : 1),
	       msg->destination);

	for (i = 0; i < msg->length; i++)
		printf(" %i", msg->data[i]);

	printf("\n");

	free(msg);
	cctalk_host_free(host);

	return 0;
}

int main(int argc, char **argv)
{
	int result, c, idx = 0;
	int (*action)(int argc, char **argv) = do_talk;

	while (-1 != (c = getopt_long(argc, argv, optstring, longopts, &idx)))
		switch (c) {
			case 'h':
				action = do_help;
				break;

			case 'V':
				action = do_version;
				break;

			case 'd':
				free(device);
				device = strdup(optarg);
				break;

			case 'i':
				host_id = atoi(optarg);
				break;

			case 's':
				crc_mode = CCTALK_CRC_SIMPLE;
				break;

			case 'c':
				crc_mode = CCTALK_CRC_CCITT;
				break;

			case 't':
				timeout = atoi(optarg);
				break;

			case '?':
				return 1;
		}

	if (NULL == device)
		device = strdup("/dev/ttyUSB0");

	result = action(argc - optind, argv + optind);

	free(device);
	return result;
}
