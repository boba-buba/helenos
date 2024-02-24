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

/** @addtogroup libpcap
 * @{
 */
/** @file
 * @brief Client side of the pcapctl. Functions are called from the app pcapctl
 */

#include <errno.h>
#include <async.h>
#include <str.h>
#include <stdlib.h>

#include "pcapctl_dump.h"
#include "pcapdump_iface.h"

/** Finish an async exchange on the pcapctl session
 *
 * @param exch  Exchange to be finished
 */
static void pcapctl_dump_exchange_end(async_exch_t *exch)
{
	async_exchange_end(exch);
}

errno_t pcapctl_dump_init(pcapctl_sess_t *sess)
{
	errno_t rc;
	char *svc_name;
	category_id_t pcap_cat;
	size_t count;
	service_id_t *pcap_svcs = NULL;

	rc = loc_category_get_id("pcap", &pcap_cat, 0);
	if (rc != EOK) {
		printf("Error resolving category 'pcap'.\n");
		return rc;
	}

	rc = loc_category_get_svcs(pcap_cat, &pcap_svcs, &count);
	if (rc != EOK) {
		printf("Error resolving list of pcap services.\n");
		return rc;
	}
	assert((count > 0) && "TODO: not implemented when no services are available\n");

	rc = loc_service_get_name(pcap_svcs[0], &svc_name); // Note: for now [0], because only one driver is in pcap_svcs and there is no need to find particular
	if (rc != EOK) {
		printf("Error getting service name.\n");
		goto error;
	}
	printf("Using device: %s\n", svc_name);

	async_sess_t *new_session = loc_service_connect(pcap_svcs[0], INTERFACE_PCAP_CONTROL, 0);
	if (new_session == NULL) {
		printf("Error connecting to service.\n");
		rc =  EREFUSED;
		goto error;
	}
	sess->sess = new_session;
	rc = EOK;
error:
	free(pcap_svcs);
	return rc;
}

/** Starting a new session for pcapctl
 *
 * @param name Name of the file to dump packets to
 * @param sess session to start
 * @return EOK on success or an error code
 */
errno_t pcapctl_dump_start(const char *name, pcapctl_sess_t *sess)
{
	errno_t rc;
	async_exch_t *exch = async_exchange_begin(sess->sess);

	size_t size = str_size(name);
	aid_t req = async_send_0(exch, PCAP_CONTROL_SET_START, NULL);

	rc = async_data_write_start(exch, (void *) name, size);

	pcapctl_dump_exchange_end(exch);

	if (rc != EOK) {
		async_forget(req);
		return rc;
	}

	errno_t retval;
	async_wait_for(req, &retval);
	return retval;
}

/** Finish current session for pcapctl
 *
 * @param sess Session to finish
 * @return EOK on success or an error code
 */
errno_t pcapctl_dump_stop(pcapctl_sess_t *sess)
{
	errno_t rc;
	async_exch_t *exch = async_exchange_begin(sess->sess);
	rc = async_req_0_0(exch, PCAP_CONTROL_SET_STOP);

	pcapctl_dump_exchange_end(exch);
	return rc;
}

/** @}
 */