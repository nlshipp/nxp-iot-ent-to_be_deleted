/* SPDX-License-Identifier: GPL-2.0 */
/* interrupt.h */
/*
 * Modifications Copyright 2022 NXP
 */
#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <linux/bitops.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>

typedef irqreturn_t(*irq_handler_t)(int, void *);

const void *free_irq(unsigned int irq, void *dev_id);

/**
 * request_irq - Add a handler for an interrupt line
 * @irq:	The interrupt line to allocate
 * @handler:	Function to be called when the IRQ occurs.
 *		Primary handler for threaded interrupts
 *		If NULL, the default primary handler is installed
 * @flags:	Handling flags
 * @name:	Name of the device generating this interrupt
 * @dev:	A cookie passed to the handler function
 *
 * This call allocates an interrupt and establishes a handler; see
 * the documentation for request_threaded_irq() for details.
 */
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	const char *name, void *dev);

bool irq_handle(int irq);

extern void disable_irq_nosync(unsigned int irq);
extern void disable_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);
#endif
