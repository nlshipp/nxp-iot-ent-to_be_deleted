/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LINUX_REGMAP_H
#define __LINUX_REGMAP_H

/*
 * Register map access API
 *
 * Copyright 2011 Wolfson Microelectronics plc
 * Copyright 2022 NXP
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 */

#include <linux/delay.h>
#include <linux/err.h>
#include "os/windows/src/i2c_comm.h"

struct i2c_client;

struct regmap {
	struct iotarget_handles io_target;
	NTSTATUS(*regmap_write)(struct iotarget_handles *tgt, UINT8 reg_addr, PVOID buffer, ULONG buffer_length);
	NTSTATUS(*regmap_read)(struct iotarget_handles *tgt, UINT8 reg_addr, PVOID buffer, ULONG buffer_length);
};

struct reg_sequence {
    unsigned int reg;
    unsigned int def;
    unsigned int delay_us;
};

struct regmap *devm_regmap_init_i2c(struct i2c_client *i2c);

void regmap_release_i2c(struct regmap *map);

int regmap_write(struct regmap *map, unsigned int reg, unsigned int val);
int regmap_bulk_write(struct regmap *map, unsigned int reg, const void *val, size_t val_count);
int regmap_read(struct regmap *map, unsigned int reg, unsigned int *val);
int regmap_bulk_read(struct regmap *map, unsigned int reg, void *val, size_t val_count);
int regmap_update_bits(struct regmap *map, unsigned int reg, unsigned int mask, unsigned int val);
int regmap_register_patch(struct regmap *map, const struct reg_sequence *regs, int num_regs);

#endif
