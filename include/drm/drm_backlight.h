/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

enum drm_bl_class_type {
	BL_BACKLIGHT_CLASS,
	BL_LCD_CLASS,
};

extern int drm_bl_register(struct device *dev, int type);
extern void drm_bl_unregister(struct device *dev);
extern void drm_bl_dpms(int mode);
