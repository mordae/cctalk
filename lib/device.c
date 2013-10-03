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

#include <stdlib.h>

struct cctalk_device *cctalk_device_scan(const struct cctalk_host *host,
                                         uint8_t id)
{
	struct cctalk_device *dev = calloc(1, sizeof(*dev));
	dev->host = host;
	dev->id = id;

	struct cctalk_message *reply = cctalk_request_comms_revision(dev);

	if (NULL == reply || 3 != reply->length) {
		free(dev);
		free(reply);
		return NULL;
	}

	dev->version = (reply->data[1] << 8) | reply->data[2];
	free(reply);
	return dev;
}

void cctalk_device_free(struct cctalk_device *dev)
{
	if (NULL == dev)
		return;

	free(dev);
}

#define cctalk_define_method(name, method) \
	struct cctalk_message *name(const struct cctalk_device *dev) \
	{ \
		if (-1 == cctalk_send(dev->host, dev->id, method, 0, NULL)) \
			return NULL; \
		return cctalk_recv(dev->host); \
	} \
	struct cctalk_message *name(const struct cctalk_device *dev)

cctalk_define_method(cctalk_request_comms_revision,
                     CCTALK_METHOD_REQUEST_COMMS_REVISION);

cctalk_define_method(cctalk_reset_device,
                     CCTALK_METHOD_RESET_DEVICE);

struct cctalk_message *
cctalk_modify_inhibit_status(const struct cctalk_device *dev, uint16_t mask)
{
	uint8_t data[] = {mask & 0xff, mask >> 8};

	enum cctalk_method method = CCTALK_METHOD_MODIFY_INHIBIT_STATUS;
	if (-1 == cctalk_send(dev->host, dev->id, method, sizeof(data), data))
		return NULL;

	return cctalk_recv(dev->host);
}

struct cctalk_message *
cctalk_modify_master_inhibit_status(const struct cctalk_device *dev, int on)
{
	uint8_t data[] = {on ? 1 : 0};

	enum cctalk_method method = CCTALK_METHOD_MODIFY_MASTER_INHIBIT_STATUS;
	if (-1 == cctalk_send(dev->host, dev->id, method, sizeof(data), data))
		return NULL;

	return cctalk_recv(dev->host);
}
