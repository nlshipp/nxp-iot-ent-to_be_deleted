/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Modifications Copyright 2022 NXP
 */
#ifndef _ASM_GENERIC_BUG_H
#define _ASM_GENERIC_BUG_H

#ifndef WARN_ON
extern void __warn(const char* file, const char* func, int line);
#define WARN_ON(condition) do {						\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__warn(__FILE__,__FUNCTION__, __LINE__);	\
	unlikely(__ret_warn_on);						\
} while(0);
#endif

#endif
