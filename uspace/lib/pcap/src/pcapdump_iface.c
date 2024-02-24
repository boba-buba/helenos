/*
 * Copyright (c) 2023 Nataliia Korop
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

/**
 * @addtogroup libpcap
 * @{
 */
/**
 * @file
 * @brief Server side of the pcapctl
 */

#include <async.h>
#include <errno.h>
#include <fibril_synch.h>

#include "pcapdump_iface.h"

FIBRIL_MUTEX_INITIALIZE(to_dump_mutex);

static void pcapdump_start_srv(ipc_call_t *icall, pcap_iface_t *iface)
{
	char *data;
	size_t size;
	errno_t rc = async_data_write_accept((void **) &data, false, 0, 0, 0, &size);
	if (rc != EOK) {
		async_answer_0(icall, rc);
		return;
	}

	/** When try to start when already started, close current and starts new */
	if (iface->to_dump == true) {
		iface->fini();
	}
	iface->init((const char *)data);

	fibril_mutex_lock(&to_dump_mutex);
	iface->to_dump = true;
	fibril_mutex_unlock(&to_dump_mutex);

	async_answer_0(icall, rc);
}

static void pcapdump_stop_srv(ipc_call_t *icall, pcap_iface_t *iface)
{
	/** If want to stop, when already stopped, do nothing */
	if (iface->to_dump == false) {
		async_answer_0(icall, EOK);
		return;
	}

	fibril_mutex_lock(&to_dump_mutex);
	iface->to_dump = false;
	fibril_mutex_unlock(&to_dump_mutex);

	iface->fini();
	async_answer_0(icall, EOK);
}

static void pcapdump_conn(ipc_call_t *icall, void *arg)
{
	pcap_iface_t *iface = (pcap_iface_t *)arg;

	assert((iface != NULL) && "pcapdump requires pcap interface\n");

	/* Accept connection */
	async_accept_0(icall);

	while (true) {
		ipc_call_t call;
		async_get_call(&call);
		sysarg_t method = ipc_get_imethod(&call);
		if (!method) {
			/* The other side has hung up */
			async_answer_0(&call, EOK);
			break;
		}
		switch (method) {
		case PCAP_CONTROL_SET_START:
			pcapdump_start_srv(&call, iface);
			break;
		case PCAP_CONTROL_SET_STOP:
			pcapdump_stop_srv(&call, iface);
			break;
		default:
			async_answer_0(&call, EINVAL);
			break;
		}
	}
}

errno_t pcapdump_init(pcap_iface_t *iface)
{
	port_id_t port;
	errno_t rc;

	rc = pcap_iface_init(iface);

	if (rc != EOK) {
		printf("Failed creating pcap interface: %s", str_error(rc));
		return rc;
	}

	rc = async_create_port(INTERFACE_PCAP_CONTROL,
	    pcapdump_conn, iface, &port);
	if (rc != EOK) {
		return rc;
	}
	return EOK;
}

/** Dumping function for driver
 *
 * Called every time, the packet is sent/recieved by the device
 *
 * @param iface Dumping interface
 * @param data The packet
 * @param size Size of the packet
 *
 */
void pcapdump_packet(pcap_iface_t *iface, const void *data, size_t size)
{

	if (iface == NULL) {
		return;
	}

	if (!iface->to_dump) {
		return;
	}

	iface->add_packet(data, size);
}

/** @}
 */