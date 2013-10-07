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

#ifndef _CCTALK_HOST_H
#define _CCTALK_HOST_H 1

#ifndef _CCTALK_H
# error "This file cannot be included directly, include <cctalk.h> instead."
#endif

#include <stdint.h>
#include <stdio.h>

#include "enum.h"

/* Available crc modes. */
enum cctalk_crc_mode {
	/* Simple 256-complement, 1 byte checksum after every message. */
	CCTALK_CRC_SIMPLE = 0,

	/* Full CRC-16-CCITT checksum using source address field and one
	 * more byte after every message. */
	CCTALK_CRC_CCITT = 1,
};

/* ccTalk host context for communication over serial line. */
struct cctalk_host {
	/* Serial line descriptor. */
	int fd;

	/* Host identifier. */
	uint8_t id;

	/* Selected CRC mode to send and to expect. */
	enum cctalk_crc_mode crc_mode;

	/* Read/write timeout in milliseconds. */
	int timeout;
};

/* Single message with variable-length payload. */
struct cctalk_message {
	uint8_t destination;
	uint8_t length;
	uint8_t source;
	uint8_t header;
	uint8_t data[0];
} __attribute__((__packed__));


/*
 * Create ccTalk host context using specified serial port.
 *
 * You can freely tune the options in the returned structure
 * before you send any messages.
 */
struct cctalk_host *cctalk_host_new(const char *path);

/* Destroy the ccTalk host context. */
void cctalk_host_free(struct cctalk_host *host);

/* Send message via given ccTalk host. */
int cctalk_send(const struct cctalk_host *host, uint8_t destination,
                enum cctalk_method method, void *data, size_t length);

/*
 * Receive single message via given ccTalk host.
 * Returns NULL if no data arrives for more than timeout milliseconds.
 */
struct cctalk_message *cctalk_recv(const struct cctalk_host *host);

/* Receive message and return it's status.
 * Returns -1 if no data arrives for more than timeout milliseconds. */
int cctalk_recv_status(const struct cctalk_host *host);

/* Receive message data and return it's status.
 * Returns -1 if no data arrives for more than timeout milliseconds.
 * If fewer than requested bytes arrives, buffer is nil padded. */
int cctalk_recv_data(const struct cctalk_host *host, uint8_t *buf, size_t len);


#endif				/* !_CCTALK_HOST_H */
