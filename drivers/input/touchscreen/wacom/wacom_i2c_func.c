/*
 *  wacom_i2c_func.c - Wacom G5 Digitizer Controller (I2C bus)
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

#include <linux/wacom_i2c.h>
#include "wacom_i2c_flash.h"
#ifdef COOR_WORK_AROUND
#include "wacom_i2c_coord_table.h"

#define WACOM_OFFSET_X (-60)
#define WACOM_OFFSET_Y (-100)

/* For firmware algorithm */
#define CAL_PITCH 100
#define MAX_COORD_X 11392
#define MAX_COORD_Y 7120
#define LATTICE_SIZE_X ((MAX_COORD_X / CAL_PITCH)+2)
#define LATTICE_SIZE_Y ((MAX_COORD_Y / CAL_PITCH)+2)
#endif

#if defined(CONFIG_MACH_P4)
#define MAX_COORD_X WACOM_POSX_MAX
#define MAX_COORD_Y WACOM_POSY_MAX
#endif

#define CONFIG_SAMSUNG_KERNEL_DEBUG_USER

/* block wacom coordinate print */
/* extern int sec_debug_level(void); */
#ifdef SEC_DVFS_LOCK
#define SEC_DVFS_LOCK_TIMEOUT 3
static struct delayed_work dvfs_dwork;
static void free_dvfs_lock(struct work_struct *work)
{
	exynos_cpufreq_lock_free(DVFS_LOCK_ID_PEN);
}
static void set_dvfs_lock(bool on)
{
	static bool enable;

	if (on) {
		cancel_delayed_work(&dvfs_dwork);
		if (!enable) {
			unsigned int cpufreq_level;
			if (!exynos_cpufreq_get_level(800000, &cpufreq_level)) {
				exynos_cpufreq_lock(DVFS_LOCK_ID_PEN,
					cpufreq_level);
				enable = true;
			}
		}
	} else {
		if (enable)
			schedule_delayed_work(&dvfs_dwork,
				SEC_DVFS_LOCK_TIMEOUT * HZ);
		enable = false;
	}
}
#endif

void forced_release(struct wacom_i2c *wac_i2c)
{
#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
	printk(KERN_DEBUG "[E-PEN] %s\n", __func__);
#endif
	input_report_abs(wac_i2c->input_dev,
		ABS_PRESSURE, 0);
	input_report_key(wac_i2c->input_dev,
		BTN_STYLUS, 0);
	input_report_key(wac_i2c->input_dev,
		BTN_TOUCH, 0);
	input_report_key(wac_i2c->input_dev,
		wac_i2c->tool, 0);
	input_sync(wac_i2c->input_dev);

	wac_i2c->pen_prox = 0;
	wac_i2c->pen_pressed = 0;
	wac_i2c->side_pressed = 0;

#ifdef SEC_DVFS_LOCK
	set_dvfs_lock(false);
#endif

}

int wacom_i2c_test(struct wacom_i2c *wac_i2c)
{
	int ret, i;
	char buf, test[10];
	buf = COM_QUERY;

	ret = i2c_master_send(wac_i2c->client, &buf, sizeof(buf));
	if (ret > 0)
		printk(KERN_INFO "[E-PEN]: buf:%d, sent:%d\n", buf, ret);
	else {
		printk(KERN_ERR "[E-PEN]: Digitizer is not active\n");
		return -1;
	}

	ret = i2c_master_recv(wac_i2c->client, test, sizeof(test));
	if (ret >= 0) {
		for (i = 0; i < 8; i++)
			printk(KERN_INFO "[E-PEN]: %d\n", test[i]);
	} else {
		printk(KERN_ERR "[E-PEN]: Digitizer does not reply\n");
		return -1;
	}

	return 0;
}

int wacom_i2c_master_send(struct i2c_client *client,
			  const char *buf, int count, unsigned short addr)
{
	int ret;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;

	msg.addr = addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = (char *)buf;

	ret = i2c_transfer(adap, &msg, 1);

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return (ret == 1) ? count : ret;
}

int wacom_i2c_master_recv(struct i2c_client *client, char *buf,
			  int count, unsigned short addr)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	int ret;

	msg.addr = addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(adap, &msg, 1);

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return (ret == 1) ? count : ret;
}

int wacom_i2c_query(struct wacom_i2c *wac_i2c)
{
	struct wacom_features *wac_feature = wac_i2c->wac_feature;
	struct i2c_msg msg[2];
	int ret;
	char buf;
	u8 data[9];
	int i = 0;
	int query_limit = 10;

#ifdef SEC_DVFS_LOCK
	INIT_DELAYED_WORK(&dvfs_dwork, free_dvfs_lock);
#endif

	buf = COM_QUERY;

	msg[0].addr = wac_i2c->client->addr;
	msg[0].flags = 0x00;
	msg[0].len = 1;
	msg[0].buf = (u8 *)&buf;

	msg[1].addr = wac_i2c->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = COM_QUERY_NUM;
	msg[1].buf = (u8 *) data;

	do {
		ret = i2c_transfer(wac_i2c->client->adapter, msg, 2);
		printk(KERN_INFO "[E-PEN]: %s: %dth ret of wacom query=%d\n",
		       __func__, i, ret);

		i++;
		if (ret < 0)
			continue;

		wac_feature->fw_version = ((u16) data[7] << 8) + (u16) data[8];
		if (wac_feature->fw_version != 0xFF
		    && wac_feature->fw_version != 0xFFFF
		    && wac_feature->fw_version != 0x0) {
			break;
		} else {
			printk(KERN_NOTICE
			       "[E-PEN]: %X, %X, %X, %X, %X, %X, %X, %X, %X fw=0x%x\n",
			       data[0], data[1], data[2], data[3], data[4],
			       data[5], data[6], data[7], data[8],
			       wac_feature->fw_version);
		}

		i++;
	} while (i < query_limit);

#if defined(CONFIG_MACH_Q1_BD) || defined(CONFIG_MACH_P4)
	wac_feature->x_max = (u16) MAX_COORD_X;
	wac_feature->y_max = (u16) MAX_COORD_Y;
#else
	wac_feature->x_max = ((u16) data[1] << 8) + (u16) data[2];
	wac_feature->y_max = ((u16) data[3] << 8) + (u16) data[4];
#endif
	wac_feature->pressure_max = (u16) data[6] + ((u16) data[5] << 8);

#if defined(COOR_WORK_AROUND)
	if (i == 10 || ret < 0) {
		printk(KERN_NOTICE "[E-PEN]: COOR_WORK_AROUND is applied\n");
		printk(KERN_NOTICE
		       "[E-PEN]: %X, %X, %X, %X, %X, %X, %X, %X, %X\n", data[0],
		       data[1], data[2], data[3], data[4], data[5], data[6],
		       data[7], data[8]);
		wac_feature->x_max = (u16) COOR_WORK_AROUND_X_MAX;
		wac_feature->y_max = (u16) COOR_WORK_AROUND_Y_MAX;
		wac_feature->pressure_max = (u16) COOR_WORK_AROUND_PRESSURE_MAX;
		wac_feature->fw_version = 0xFF;
	}
#endif

	printk(KERN_NOTICE "[E-PEN]: x_max=0x%X\n", wac_feature->x_max);
	printk(KERN_NOTICE "[E-PEN]: y_max=0x%X\n", wac_feature->y_max);
	printk(KERN_NOTICE "[E-PEN]: pressure_max=0x%X\n",
	       wac_feature->pressure_max);
	printk(KERN_NOTICE "[E-PEN]: fw_version=0x%X (d7:0x%X,d8:0x%X)\n",
	       wac_feature->fw_version, data[7], data[8]);
	printk(KERN_NOTICE "[E-PEN]: %X, %X, %X, %X, %X, %X, %X, %X, %X\n",
	       data[0], data[1], data[2], data[3], data[4], data[5], data[6],
	       data[7], data[8]);

	if ((i == 10) && (ret < 0))
		return ret;

	return 0;
}

#ifdef COOR_WORK_AROUND
void wacom_i2c_coord_offset(u16 *coordX, u16 *coordY)
{
	u16 ix, iy;
	u16 dXx_0, dXy_0, dXx_1, dXy_1;
	int D0, D1, D2, D3, D;

	ix = (u16) (((*coordX)) / CAL_PITCH);
	iy = (u16) (((*coordY)) / CAL_PITCH);

	dXx_0 = *coordX - (ix * CAL_PITCH);
	dXx_1 = CAL_PITCH - dXx_0;

	dXy_0 = *coordY - (iy * CAL_PITCH);
	dXy_1 = CAL_PITCH - dXy_0;

	if (*coordX <= MAX_COORD_X) {
		D0 = tableX[user_hand][screen_rotate][ix +
						      (iy * LATTICE_SIZE_X)] *
		    (dXx_1 + dXy_1);
		D1 = tableX[user_hand][screen_rotate][ix + 1 +
						      iy * LATTICE_SIZE_X] *
		    (dXx_0 + dXy_1);
		D2 = tableX[user_hand][screen_rotate][ix +
						      (iy +
						       1) * LATTICE_SIZE_X] *
		    (dXx_1 + dXy_0);
		D3 = tableX[user_hand][screen_rotate][ix + 1 +
						      (iy +
						       1) * LATTICE_SIZE_X] *
		    (dXx_0 + dXy_0);
		D = (D0 + D1 + D2 + D3) / (4 * CAL_PITCH);

		if (((int)*coordX + D) > 0)
			*coordX += D;
		else
			*coordX = 0;
	}

	if (*coordY <= MAX_COORD_Y) {
		D0 = tableY[user_hand][screen_rotate][ix +
						      (iy * LATTICE_SIZE_X)] *
		    (dXy_1 + dXx_1);
		D1 = tableY[user_hand][screen_rotate][ix + 1 +
						      iy * LATTICE_SIZE_X] *
		    (dXy_1 + dXx_0);
		D2 = tableY[user_hand][screen_rotate][ix +
						      (iy +
						       1) * LATTICE_SIZE_X] *
		    (dXy_0 + dXx_1);
		D3 = tableY[user_hand][screen_rotate][ix + 1 +
						      (iy +
						       1) * LATTICE_SIZE_X] *
		    (dXy_0 + dXx_0);
		D = (D0 + D1 + D2 + D3) / (4 * CAL_PITCH);

		if (((int)*coordY + D) > 0)
			*coordY += D;
		else
			*coordY = 0;
	}
}

void wacom_i2c_coord_average(unsigned short *CoordX, unsigned short *CoordY,
			     int bFirstLscan)
{
	unsigned char i;
	unsigned int work;
	unsigned char ave_step = 4, ave_shift = 2;
	static unsigned short Sum_X, Sum_Y;
	static unsigned short AveBuffX[4], AveBuffY[4];
	static unsigned char AvePtr;
	static unsigned char bResetted;

	if (bFirstLscan == 0)
		bResetted = 0;
	else {
		if (bFirstLscan && (bResetted == 0)) {
			AvePtr = 0;

			ave_step = 4;
			ave_shift = 2;

			for (i = 0; i < ave_step; i++) {
				AveBuffX[i] = *CoordX;
				AveBuffY[i] = *CoordY;
			}
			Sum_X = (unsigned short)*CoordX << ave_shift;
			Sum_Y = (unsigned short)*CoordY << ave_shift;
			bResetted = 1;
		} else if (bFirstLscan) {
			Sum_X = Sum_X - AveBuffX[AvePtr] + (*CoordX);
			AveBuffX[AvePtr] = *CoordX;
			work = Sum_X >> ave_shift;
			*CoordX = (unsigned int)work;

			Sum_Y = Sum_Y - AveBuffY[AvePtr] + (*CoordY);
			AveBuffY[AvePtr] = (*CoordY);
			work = Sum_Y >> ave_shift;
			*CoordY = (unsigned int)work;

			if (++AvePtr >= ave_step)
				AvePtr = 0;
		}
	}

}
#endif

static bool wacom_i2c_coord_range(u16 *x, u16 *y)
{
#if 0
/*
#if defined(CONFIG_MACH_P4)
*/
	*x -= WACOM_POSX_OFFSET;
	*y -= WACOM_POSY_OFFSET;
#endif
	if ((*x <= WACOM_POSX_MAX) &&
		(*y <= WACOM_POSY_MAX))
		return true;

	return false;
}

int wacom_i2c_coord(struct wacom_i2c *wac_i2c)
{
	bool prox = false;
	int ret = 0;
	u8 *data;
	int rubber, stylus;
	static u16 x, y, pressure;
	static u16 tmp;
	int rdy = 0;

	data = wac_i2c->wac_feature->data;
	ret = i2c_master_recv(wac_i2c->client, data, COM_COORD_NUM);

	if (ret >= 0) {
#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
		pr_debug("[E-PEN] %x, %x, %x, %x, %x, %x, %x\n",
			 data[0], data[1], data[2], data[3], data[4], data[5],
			 data[6]);
#endif

		if (data[0] & 0x80) {
			/* enable emr device */
			if (!wac_i2c->pen_prox) {

				wac_i2c->pen_prox = 1;

				if (data[0] & 0x40)
					wac_i2c->tool = BTN_TOOL_RUBBER;
				else
					wac_i2c->tool = BTN_TOOL_PEN;
#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
				pr_debug("[E-PEN] is in(%d)\n", wac_i2c->tool);
#endif
			}

			prox = !!(data[0] & 0x10);
			stylus = !!(data[0] & 0x20);
			rubber = !!(data[0] & 0x40);
			rdy = !!(data[0] & 0x80);

			x = ((u16) data[1] << 8) + (u16) data[2];
			y = ((u16) data[3] << 8) + (u16) data[4];
			pressure = ((u16) data[5] << 8) + (u16) data[6];

#ifdef WACOM_IMPORT_FW_ALGO
			/* Change Position to Active Area */
			if (x <= origin_offset[0])
				x = 0;
			else
				x = x - origin_offset[0];
			if (y <= origin_offset[1])
				y = 0;
			else
				y = y - origin_offset[1];
#ifdef COOR_WORK_AROUND
			wacom_i2c_coord_offset(&x, &y);
			wacom_i2c_coord_average(&x, &y, rdy);
#endif
#endif

			if (wac_i2c->wac_pdata->x_invert)
				x = wac_i2c->wac_feature->x_max - x;
			if (wac_i2c->wac_pdata->y_invert)
				y = wac_i2c->wac_feature->y_max - y;

			if (wac_i2c->wac_pdata->xy_switch) {
				tmp = x;
				x = y;
				y = tmp;
			}
#ifdef COOR_WORK_AROUND
			/* Add offset */
			x = x + tilt_offsetX[user_hand][screen_rotate];
			y = y + tilt_offsetY[user_hand][screen_rotate];
#endif
			if (wacom_i2c_coord_range(&x, &y)) {
				input_report_abs(wac_i2c->input_dev, ABS_X, x);
				input_report_abs(wac_i2c->input_dev, ABS_Y, y);
				input_report_abs(wac_i2c->input_dev,
						 ABS_PRESSURE, pressure);
				input_report_key(wac_i2c->input_dev,
						 BTN_STYLUS, stylus);
				input_report_key(wac_i2c->input_dev, BTN_TOUCH,
						 prox);
				input_report_key(wac_i2c->input_dev,
						 wac_i2c->tool, 1);
				input_sync(wac_i2c->input_dev);

				if (prox && !wac_i2c->pen_pressed) {
#ifdef SEC_DVFS_LOCK
					set_dvfs_lock(true);
#endif
#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
					printk(KERN_DEBUG
						"[E-PEN] is pressed(%d,%d,%d)(%d)\n",
						x, y, pressure, wac_i2c->tool);
#else
					printk(KERN_DEBUG "[E-PEN] pressed\n");
#endif

				} else if (!prox && wac_i2c->pen_pressed) {
#ifdef SEC_DVFS_LOCK
					set_dvfs_lock(false);
#endif
#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
					printk(KERN_DEBUG
						"[E-PEN] is released(%d,%d,%d)(%d)\n",
						x, y, pressure, wac_i2c->tool);
#else
					printk(KERN_DEBUG "[E-PEN] released\n");
#endif
				}

				wac_i2c->pen_pressed = prox;

#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
				if (stylus && !wac_i2c->side_pressed)
					printk(KERN_DEBUG "[E-PEN] side on");
				else if (!stylus && wac_i2c->side_pressed)
					printk(KERN_DEBUG "[E-PEN] side off");
#endif
				wac_i2c->side_pressed = stylus;
			}
#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
			else
			   printk("[E-PEN] raw data x=0x%x, y=0x%x\n",
			   x, y);
#endif
		} else {
#ifdef COOR_WORK_AROUND
			/* enable emr device */
			wacom_i2c_coord_average(0, 0, 0);
#endif

			if (wac_i2c->pen_prox) {
				/* input_report_abs(wac->input_dev,
				ABS_X, x); */
				/* input_report_abs(wac->input_dev,
				ABS_Y, y); */
				input_report_abs(wac_i2c->input_dev,
						 ABS_PRESSURE, 0);
				input_report_key(wac_i2c->input_dev,
						 BTN_STYLUS, 0);
				input_report_key(wac_i2c->input_dev, BTN_TOUCH,
						 0);
				input_report_key(wac_i2c->input_dev,
						 wac_i2c->tool, 0);
				input_sync(wac_i2c->input_dev);

#if defined(CONFIG_SAMSUNG_KERNEL_DEBUG_USER)
				if (wac_i2c->pen_pressed
					|| wac_i2c->side_pressed)
					printk(KERN_DEBUG "[E-PEN] is out");
				else
					printk(KERN_DEBUG "[E-PEN] is out");
#endif
			}
			wac_i2c->pen_prox = 0;
			wac_i2c->pen_pressed = 0;
			wac_i2c->side_pressed = 0;

#ifdef SEC_DVFS_LOCK
			set_dvfs_lock(false);
#endif
		}
	} else {
		printk(KERN_ERR "[E-PEN]: failed to read i2c\n");
		return -1;
	}

	return 0;
}

