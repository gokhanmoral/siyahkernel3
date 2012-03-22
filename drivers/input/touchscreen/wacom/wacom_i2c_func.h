/*
 *  wacom_i2c_func.h - Wacom G5 Digitizer Controller (I2C bus)
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef COOR_WORK_AROUND
extern unsigned char screen_rotate;
extern unsigned char user_hand;
#endif

extern int wacom_i2c_master_send(struct i2c_client *client, const char *buf,
				 int count, unsigned short addr);
extern int wacom_i2c_master_recv(struct i2c_client *client, char *buf,
				 int count, unsigned short addr);
extern int wacom_i2c_test(struct wacom_i2c *wac_i2c);
extern int wacom_i2c_coord(struct wacom_i2c *wac_i2c);
extern int wacom_i2c_query(struct wacom_i2c *wac_i2c);
extern void check_emr_device(bool bOn);
extern void forced_release(struct wacom_i2c *wac_i2c);
