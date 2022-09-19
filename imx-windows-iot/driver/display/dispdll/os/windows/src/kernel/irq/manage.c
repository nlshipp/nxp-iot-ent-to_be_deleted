// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 1992, 1998-2006 Linus Torvalds, Ingo Molnar
 * Copyright (C) 2005-2006 Thomas Gleixner
 * Copyright 2022 NXP
 *
 * This file contains driver APIs to the irq subsystem.
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#include "internals.h"

#include "boot\dts\freescale\board.h"

/**
 *	disable_irq_nosync - disable an irq
 *	@irq: Interrupt to disable
 */
void disable_irq_nosync(unsigned int irq)
{
	disable_irq(irq);
}

/**
 *	disable_irq - disable an irq
 *	@irq: Interrupt to disable
 */
void disable_irq(unsigned int irq)
{
	if (irq >= NR_IRQS) {
		return;
	}

	irq_disable(&irq_desc[irq]);
}

/**
 *	enable_irq - enable handling of an irq
 *	@irq: Interrupt to enable
 */
void enable_irq(unsigned int irq)
{
	if (irq >= NR_IRQS) {
		return;
	}

	irq_enable(&irq_desc[irq]);
}

/**
 *	free_irq - free an interrupt allocated with request_irq
 *	@irq: Interrupt line to free
 *	@dev_id: Device identity to free
 *
 *	Remove an interrupt handler. The handler is removed and interrupt line is disabled.
 *
 *	Returns the devname argument passed to request_irq.
 */
const void *free_irq(unsigned int irq, void *dev_id)
{
	if (irq >= NR_IRQS) {
		return NULL;
	}

	irq_disable(&irq_desc[irq]);

	irq_desc[irq].handler = NULL;
	irq_desc[irq].dev_id = NULL;

	return irq_desc[irq].name;
}

/**
 *	request_irq - Add a handler for an interrupt line
 *	@irq:	The interrupt line to allocate
 *	@handler:	Function to be called when the IRQ occurs.
 *	@flags:	Handling flags
 *	@name:	Name of the device generating this interrupt
 *	@dev:	A cookie passed to the handler function
 *
 *	This call allocates interrupt resources and enables the interrupt line and IRQ handling.
 */
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	const char *name, void *dev)
{
	if (irq >= NR_IRQS) {
		return -1;
	}

	irq_desc[irq].handler = handler;
	irq_desc[irq].dev_id = dev;

	irq_enable(&irq_desc[irq]);

	return 0;
}
