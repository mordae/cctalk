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

#ifndef _CCTALK_DEVICE_H
#define _CCTALK_DEVICE_H 1

#ifndef _CCTALK_H
# error "This file cannot be included directly, include <cctalk.h> instead."
#endif

#include "host.h"

/* Specific peer device. */
struct cctalk_device {
	/* Host this device can be reached through. */
	const struct cctalk_host *host;

	/* Detected comms version of the device. */
	uint16_t version;

	/* Address to use when communicating with the device. */
	uint8_t id;
};

/* Scan the peer device and prepare above structure.
 * You may not free the host before the device. */
struct cctalk_device *cctalk_device_scan(const struct cctalk_host *host,
                                         uint8_t id);

/* Free the device structure. */
void cctalk_device_free(struct cctalk_device *device);

/* Declare some generic methods. */
#define cctalk_declare_method(name, ...) \
	struct cctalk_message * \
	name(const struct cctalk_device *device, ##__VA_ARGS__)

cctalk_declare_method(cctalk_request_comms_revision);
cctalk_declare_method(cctalk_reset_device);
cctalk_declare_method(cctalk_modify_inhibit_status, uint16_t mask);
cctalk_declare_method(cctalk_modify_master_inhibit_status, int on);

#endif				/* !_CCTALK_DEVICE_H */