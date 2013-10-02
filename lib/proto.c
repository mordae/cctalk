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
#include "util.h"

#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/fcntl.h>

static int setup_serial_line(int fd)
{
	struct termios tio = {
		.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD,
		.c_iflag = 0,
		.c_oflag = 0,
		.c_lflag = ICANON,
	};

	if (-1 == tcflush(fd, TCIFLUSH))
		return -1;

	return tcsetattr(fd, TCSANOW, &tio);
}

struct cctalk_host *cctalk_host_new(const char *path)
{
	struct cctalk_host *host;
	int fd;

	if (-1 == (fd = open(path, O_RDWR | O_NOCTTY)))
		return NULL;

	if (-1 == setup_serial_line(fd))
		return NULL;

	host = malloc(sizeof(*host));
	host->fd = fd;
	host->host_id = 1;
	host->crc_mode = CCTALK_CRC_SIMPLE;
	host->timeout = 1000;

	return host;
}

void cctalk_host_free(struct cctalk_host *host)
{
	if (NULL == host)
		return;

	close(host->fd);
	free(host);
}

int cctalk_send(struct cctalk_host *host, uint8_t destination,
                enum cctalk_method method, size_t length, void *data)
{
	uint8_t checksum;

	struct cctalk_message header = {
		.destination = destination,
		.length = length,
		.source = host->host_id,
		.header = method,
	};

	if (CCTALK_CRC_CCITT == host->crc_mode)
		checksum = crc_16_ccitt(&header, data);
	else
		checksum = crc_simple(&header, data);

	if (-1 == xwrite(host->fd, &header, sizeof(header), host->timeout))
		return -1;

	if (-1 == xwrite(host->fd, data, length, host->timeout))
		return -1;

	if (-1 == xwrite(host->fd, &checksum, 1, host->timeout))
		return -1;

	return 0;
}

struct cctalk_message *cctalk_recv(struct cctalk_host *host, int timeout)
{
	struct cctalk_message *msg;
	struct cctalk_message header;
	uint8_t data[256] = {0};
	uint8_t checksum, our_checksum;

	if (-1 == xread(host->fd, &header, sizeof(header), host->timeout))
		return NULL;

	if (-1 == xread(host->fd, data, header.length, host->timeout))
		return NULL;

	if (-1 == xread(host->fd, &checksum, 1, host->timeout))
		return NULL;

	if (CCTALK_CRC_CCITT == host->crc_mode)
		our_checksum = crc_16_ccitt(&header, data);
	else
		our_checksum = crc_simple(&header, data);

	if (checksum != our_checksum)
		return NULL;

	msg = malloc(sizeof(header) + header.length + 1);
	memcpy(msg, &header, sizeof(header));
	memcpy(msg->data, data, header.length + 1);

	return msg;
}
