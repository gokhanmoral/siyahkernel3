/* linux/arch/arm/plat-s5p/mdnie.h
 *
 * mDNIe Platform Specific Header Definitions.
 *
 * Copyright (c) 2011 Samsung Electronics
 * InKi Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _MDNIE_H_
#define _MDNIE_H_

#define END_CMD		0xffff

enum mdnie_mode {
	CMC623_MODE_MIN = 0,
	CMC623_MODE_UI = CMC623_MODE_MIN,
	CMC623_MODE_VIDEO,
	CMC623_MODE_CAMERA,
	CMC623_MODE_VT,
	CMC623_MODE_DMB,
	CMC623_MODE_GALLERY,
	CMC623_MODE_MAX = CMC623_MODE_GALLERY
};

struct mdnie_tables {
	unsigned short *tables[CMC623_MODE_MAX + 1];
	unsigned short *cabc_tables[CMC623_MODE_MAX + 1];
};

#endif /* _MDNIE_H_ */
