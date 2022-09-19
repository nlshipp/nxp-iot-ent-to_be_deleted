/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * i2c.h - definitions for the Linux i2c bus interface
 * Copyright (C) 1995-2000 Simon G. Vogl
 * Copyright (C) 2013-2019 Wolfram Sang <wsa@kernel.org>
 * Copyright 2022 NXP
 *
 * With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi> and
 * Frodo Looijaard <frodol@dds.nl>
 */
#ifndef _LINUX_I2C_H
#define _LINUX_I2C_H

#include <linux/device.h>	/* for struct device */

struct i2c_client {
	unsigned short addr;		/* chip address - NOTE: 7bit	*/
								/* addresses are stored in the	*/
								/* _LOWER_ 7 bits		*/
	struct device dev;		/* the device structure		*/
	int irq;			/* irq issued by device		*/
	LARGE_INTEGER connection_id;		/* I2C connection id from the RESOURCE HUB */
	bool is_initialized;
};

static inline void *i2c_get_clientdata(const struct i2c_client *client)
{
	return dev_get_drvdata(&client->dev);
}

static inline void i2c_set_clientdata(struct i2c_client *client, void *data)
{
	dev_set_drvdata(&client->dev, data);
}


#endif /* _LINUX_I2C_H */
