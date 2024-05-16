/*
 * Copyright (c) 2024 Jiri Svoboda
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

/** @addtogroup ata_bd
 * @{
 */
/** @file ATA driver definitions.
 */

#ifndef __ATA_BD_H__
#define __ATA_BD_H__

#include <ata/ata.h>
#include <ata/ata_hw.h>
#include <ddf/driver.h>
#include <fibril_synch.h>
#include <stdbool.h>
#include <stdint.h>

#define NAME "ata_bd"

/** ATA hardware resources */
typedef struct {
	uintptr_t cmd;	/**< Command block base address. */
	uintptr_t ctl;	/**< Control block base address. */
	int irq;	/**< IRQ */
} ata_hwres_t;

/** ATA controller */
typedef struct ata_ctrl {
	/** DDF device */
	ddf_dev_t *dev;
	/** I/O base address of the command registers */
	uintptr_t cmd_physical;
	/** I/O base address of the control registers */
	uintptr_t ctl_physical;

	/** Command registers */
	ata_cmd_t *cmd;
	/** Control registers */
	ata_ctl_t *ctl;
	/** IRQ (-1 if not used) */
	int irq;
	/** IRQ handle */
	cap_irq_handle_t ihandle;

	/** Synchronize controller access */
	fibril_mutex_t lock;
	/** Value of status register read by interrupt handler */
	uint8_t irq_status;

	/** Libata ATA channel */
	ata_channel_t *channel;
	struct ata_fun *fun[2];
} ata_ctrl_t;

typedef struct ata_fun {
	ddf_fun_t *fun;
	void *charg;
} ata_fun_t;

extern errno_t ata_ctrl_init(ata_ctrl_t *, ata_hwres_t *);
extern errno_t ata_ctrl_remove(ata_ctrl_t *);
extern errno_t ata_ctrl_gone(ata_ctrl_t *);

#endif

/** @}
 */
