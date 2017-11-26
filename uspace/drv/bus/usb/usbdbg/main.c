/*
 * Copyright (c) 2017 Petr Manek
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup drvusbdbg
 * @{
 */
/**
 * @file
 * Main routines of USB debug device driver.
 */
#include <errno.h>
#include <usb/debug.h>
#include <usb/dev/driver.h>

#include "usbdbg.h"
#include "device.h"

#define NAME "usbdbg"

static int device_add(usb_device_t *dev)
{
	usb_log_info("Adding device '%s'", usb_device_get_name(dev));

	int err;

	usb_dbg_dev_t *dbg_dev;
	if ((err = usb_dbg_dev_create(dev, &dbg_dev)))
		return err;

	/* TODO: Register device in some list. */
	/* TODO: Register device DDF function. */

	return EOK;
}

static int device_remove(usb_device_t *dev)
{
	usb_log_info("Removing device '%s'", usb_device_get_name(dev));

	usb_dbg_dev_t *dbg_dev = usb_dbg_dev_get(dev);

	/* TODO: Make sure nothing is going on with the device. */
	/* TODO: Unregister device DDF function. */
	/* TODO: Remove device from list */

	usb_dbg_dev_destroy(dbg_dev);

	return EOK;
}

static int device_gone(usb_device_t *dev)
{
	usb_log_info("Device '%s' gone.", usb_device_get_name(dev));

	usb_dbg_dev_t *dbg_dev = usb_dbg_dev_get(dev);

	/* TODO: Make sure nothing is going on with the device. */
	/* TODO: Unregister device DDF function. */
	/* TODO: Remove device from list */

	usb_dbg_dev_destroy(dbg_dev);

	return EOK;
}

static int function_online(ddf_fun_t *fun)
{
	return ddf_fun_online(fun);
}

static int function_offline(ddf_fun_t *fun)
{
	return ddf_fun_offline(fun);
}

/** USB debug driver ops. */
static const usb_driver_ops_t dbg_driver_ops = {
	.device_add = device_add,
	.device_rem = device_remove,
	.device_gone = device_gone,
	.function_online = function_online,
	.function_offline = function_offline
};

/** USB debug driver. */
static const usb_driver_t dbg_driver = {
	.name = NAME,
	.ops = &dbg_driver_ops,
	.endpoints = NULL
};

int main(int argc, char *argv[])
{
	printf(NAME ": USB debug device driver.\n");

	log_init(NAME);

	return usb_driver_main(&dbg_driver);
}

/**
 * @}
 */
