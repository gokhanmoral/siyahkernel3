/* linux/arch/arm/mach-exynos/include/mach/gpio-midas.h
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS4 - MIDAS GPIO lib
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_GPIO_P11_H
#define __ASM_ARCH_GPIO_P11_H __FILE__

#if defined(CONFIG_MACH_P11_01_BD)
#include "gpio-rev01-p11.h"
#elif defined(CONFIG_MACH_P11_02_BD)
#include "gpio-rev02-p11.h"
#elif defined(CONFIG_MACH_P11_04_BD)
#include "gpio-rev04-p11.h"
#endif

#endif /* __ASM_ARCH_GPIO_P11_H */
