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

#ifndef _UTIL_H
#define _UTIL_H 1

#include "cctalk.h"

#include <unistd.h>
#include <stdint.h>

/*
 * write(2) wrapper that attempts to write the whole buffer.
 * Returns either the original count or -1 to signal failure.
 * The timeout parameter specifies how long can the function
 * block until it is allowed to write next byte.
 */
ssize_t xwrite(int fd, const void *buf, size_t count, int timeout);

/*
 * read(2) wrapper that attempts to fill the whole buffer.
 * Returns either the original count or -1 to signal failure.
 * The timeout parameter specifies how long can the function
 * block until it is allowed to write next byte.
 */
ssize_t xread(int fd, void *buf, size_t count, int timeout);

/* Compute the "simple" ccTalk checksum.
 * Returns either the original count or -1 to signal failure. */
uint8_t crc_simple(struct cctalk_message *msg, const void *data);

/* Compute CRC-16-CCITT checksum while modifying the header source
 * field that will hold one half of the checksum.  Stupid ccTalk. */
uint8_t crc_16_ccitt(struct cctalk_message *msg, const void *data);

#endif				/* !_UTIL_H */
