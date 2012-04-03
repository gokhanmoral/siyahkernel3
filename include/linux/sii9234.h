/*
 * Copyright (C) 2011 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _SII9234_H_
#define _SII9234_H_

#ifdef __KERNEL__

#define	CONFIG_SAMSUNG_WORKAROUND_HPD_GLANCE

struct sii9234_platform_data {
	u8 power_state;
	int ddc_i2c_num;
	void (*init)(void);
	void (*mhl_sel)(bool enable);
	void (*hw_onoff)(bool on);
	void (*hw_reset)(void);
	void (*enable_vbus)(bool enable);
	void (*vbus_present)(bool on);
	struct i2c_client *mhl_tx_client;
	struct i2c_client *tpi_client;
	struct i2c_client *hdmi_rx_client;
	struct i2c_client *cbus_client;
};

extern void sii9234_mhl_detection_sched(void);

#endif

#ifdef	CONFIG_SAMSUNG_WORKAROUND_HPD_GLANCE
extern	void mhl_hpd_handler(bool onoff);
#endif

#ifdef	CONFIG_SAMSUNG_USE_11PIN_CONNECTOR
extern	int	max77693_muic_get_status1_adc1k_value(void);
#endif

#endif /* _SII9234_H_ */
