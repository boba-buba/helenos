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
 *  @brief pcap inteface: Dumping interface for the device which packets we want to dump
 */

#include <errno.h>
#include <str.h>
#include "pcap_dumper.h"

/** Initialize writing to .pcap file.
 *
 * @param writer    Interface for working with .pcap file
 * @param filename  Name of the file for dumping packets
 * @return          EOK on success or an error code
 *
 */
static errno_t pcap_writer_to_file_init(pcap_writer_t *writer, const char *filename)
{
	errno_t rc;
	writer->data = fopen(filename, "a");
	if (writer->data == NULL) {
		rc = EINVAL;
		return rc;
	}
	pcap_writer_add_header(writer);

	rc = EOK;
	return rc;
}

static size_t pcap_file_w32(pcap_writer_t *writer, uint32_t data)
{
	return fwrite(&data, 1, 4, (FILE *)writer->data);
}

static size_t pcap_file_w16(pcap_writer_t *writer, uint16_t data)
{
	return fwrite(&data, 1, 2, (FILE *)writer->data);
}

static size_t pcap_file_wbuffer(pcap_writer_t *writer, const void *data, size_t size)
{
	assert(writer->data);
	return fwrite(data, 1, size, (FILE *)writer->data);
}

static void pcap_file_close(pcap_writer_t *writer)
{
	fclose((FILE *)writer->data);
	writer->data = NULL;
}

static pcap_writer_ops_t file_ops = {
	.open = &pcap_writer_to_file_init,
	.write_u32 = &pcap_file_w32,
	.write_u16 = &pcap_file_w16,
	.write_buffer = &pcap_file_wbuffer,
	.close = &pcap_file_close
};

static size_t pcap_short_file_w32(pcap_writer_t *writer, uint32_t data)
{
	return fwrite(&data, 1, 4, (FILE *)writer->data);
}

static size_t pcap_short_file_w16(pcap_writer_t *writer, uint16_t data)
{
	return fwrite(&data, 1, 2, (FILE *)writer->data);
}

static size_t pcap_short_file_wbuffer(pcap_writer_t *writer, const void *data, size_t size)
{
	return fwrite(data, 1, size < 60 ? size : 60, (FILE *)writer->data);
}

static void pcap_short_file_close(pcap_writer_t *writer)
{
	fclose((FILE *)writer->data);
}

static pcap_writer_ops_t short_file_ops = {
	.open = &pcap_writer_to_file_init,
	.write_u32 = &pcap_short_file_w32,
	.write_u16 = &pcap_short_file_w16,
	.write_buffer = &pcap_short_file_wbuffer,
	.close = &pcap_short_file_close

};

errno_t pcap_dumper_start(struct pcap_dumper *dumper, const char *name)
{
	fibril_mutex_lock(&dumper->mutex);

	/** When try to start when already started, close current and starts new */
	if (dumper->to_dump) {
		pcap_dumper_stop(dumper);
	}
	errno_t rc = dumper->writer.ops->open(&dumper->writer, name);
	if (rc == EOK) {
		dumper->to_dump = true;
	} else {
		printf("Failed creating pcap dumper: %s", str_error(rc));
	}
	fibril_mutex_unlock(&dumper->mutex);
	return rc;
}

errno_t pcap_dumper_set_ops(struct pcap_dumper *dumper, const char *name)
{
	fibril_mutex_lock(&dumper->mutex);
	errno_t rc = EOK;
	if (!str_cmp(name, "short_file")) {
		dumper->writer.ops = &short_file_ops;
	} else if (!str_cmp(name, "full_file")) {
		dumper->writer.ops = &file_ops;
	} else {
		rc = EINVAL;
	}
	fibril_mutex_unlock(&dumper->mutex);
	return rc;
}

void pcap_dumper_add_packet(struct pcap_dumper *dumper, const void *data, size_t size)
{
	fibril_mutex_lock(&dumper->mutex);

	if (!dumper->to_dump) {
		fibril_mutex_unlock(&dumper->mutex);
		return;
	}

	pcap_writer_add_packet(&dumper->writer, data, size);
	fibril_mutex_unlock(&dumper->mutex);
}

void pcap_dumper_stop(struct pcap_dumper *dumper)
{
	fibril_mutex_lock(&dumper->mutex);

	/** If want to stop, when already stopped, do nothing */
	if (!dumper->to_dump) {
		fibril_mutex_unlock(&dumper->mutex);
		return;
	}
	dumper->to_dump = false;
	dumper->writer.ops->close(&dumper->writer);
	fibril_mutex_unlock(&dumper->mutex);
}

/** Initialize interface for dumping packets
 *
 * @param dumper Device dumping interface
 *
 */
errno_t pcap_dumper_init(pcap_dumper_t *dumper)
{
	fibril_mutex_initialize(&dumper->mutex);
	dumper->to_dump = false;
	dumper->writer.ops = NULL;
	return EOK;
}

/** @}
 */
