/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * Authors: Adam Hampson <ahampson@sta.samsung.com>
 *          Erik Gilling <konkers@android.com>
 *
 * Additional contributions by : Shankar Bandal <shankar.b@samsung.com>
 *                               Dharam Kumar <dharam.kr@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/sii9234.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/device.h>

/*temp*/
#ifndef CONFIG_SII9234_CBUS
#define CONFIG_SII9234_CBUS
#endif

#ifdef CONFIG_SII9234_CBUS
#ifndef CONFIG_SII9234_RCP
#define CONFIG_SII9234_RCP	1
#endif
#endif

#ifndef CONFIG_SS_FACTORY
#define CONFIG_SS_FACTORY	1
#endif

#ifdef CONFIG_SII9234_RCP
#include <linux/sii9234_rcp.h>
#include <linux/input.h>
#endif

#define CONFIG_MHL_DEBUG

#undef pr_debug
#ifdef CONFIG_MHL_DEBUG
#define pr_debug(fmt, ...)					\
do {								\
	if (unlikely(mhl_dbg_flag == 1)) {			\
		printk(KERN_INFO fmt, ##__VA_ARGS__);		\
	}							\
} while (0)
int mhl_dbg_flag;
#else
#define pr_debug(fmt, ...)
#endif

#define T_WAIT_TIMEOUT_RGND_INT		2000
#define T_WAIT_TIMEOUT_DISC_INT		1000
#define T_WAIT_TIMEOUT_RSEN_INT		200

#define T_SRC_VBUS_CBUS_TO_STABLE	200
#define T_SRC_WAKE_PULSE_WIDTH_1	19
#define T_SRC_WAKE_PULSE_WIDTH_2	60
#define T_SRC_WAKE_TO_DISCOVER		500
#define T_SRC_VBUS_CBUS_T0_STABLE	500

#define T_SRC_CBUS_FLOAT		50
#define T_HPD_WIDTH			110
#define T_SRC_RXSENSE_DEGLITCH		110
#define T_SRC_CBUS_DEGLITCH		2
#define T_TPKT_SENDER_TIMEOUT		100

/* MHL TX Addr 0x72 Registers */
#define MHL_TX_IDL_REG			0x02
#define MHL_TX_IDH_REG			0x03
#define MHL_TX_REV_REG			0x04
#define MHL_TX_SRST			0x05
#define MHL_TX_INTR1_REG		0x71
#define MHL_TX_INTR2_REG		0x72
#define MHL_TX_INTR3_REG		0x73
#define MHL_TX_INTR4_REG		0x74
#define MHL_TX_INTR1_ENABLE_REG		0x75
#define MHL_TX_INTR2_ENABLE_REG		0x76
#define MHL_TX_INTR3_ENABLE_REG		0x77
#define MHL_TX_INTR4_ENABLE_REG		0x78
#define MHL_TX_INT_CTRL_REG		0x79
#define MHL_TX_TMDS_CCTRL		0x80

#define MHL_TX_DISC_CTRL1_REG		0x90
#define MHL_TX_DISC_CTRL2_REG		0x91
#define MHL_TX_DISC_CTRL3_REG		0x92
#define MHL_TX_DISC_CTRL4_REG		0x93

/* There doesn't seem to be any documentation for CTRL5 but it looks like
 * it is some sort of pull up control register
 */
#define MHL_TX_DISC_CTRL5_REG		0x94
#define MHL_TX_DISC_CTRL6_REG		0x95
#define MHL_TX_DISC_CTRL7_REG		0x96
#define MHL_TX_DISC_CTRL8_REG		0x97
#define MHL_TX_STAT1_REG		0x98
#define MHL_TX_STAT2_REG		0x99

#define MHL_TX_MHLTX_CTL1_REG		0xA0
#define MHL_TX_MHLTX_CTL2_REG		0xA1
#define MHL_TX_MHLTX_CTL4_REG		0xA3
#define MHL_TX_MHLTX_CTL6_REG		0xA5
#define MHL_TX_MHLTX_CTL7_REG		0xA6

/* MHL TX SYS STAT Registers */
#define MHL_TX_SYSSTAT_REG		0x09

/* MHL TX SYS STAT Register Bits */
#define RSEN_STATUS			(1<<2)

/* MHL TX INTR4 Register Bits */
#define RGND_READY_INT			(1<<6)
#define VBUS_LOW_INT			(1<<5)
#define CBUS_LKOUT_INT			(1<<4)
#define MHL_DISC_FAIL_INT		(1<<3)
#define MHL_EST_INT			(1<<2)

/* MHL TX INTR4_ENABLE 0x78 Register Bits */
#define RGND_READY_MASK			(1<<6)
#define CBUS_LKOUT_MASK			(1<<4)
#define MHL_DISC_FAIL_MASK		(1<<3)
#define MHL_EST_MASK			(1<<2)

/* MHL TX INTR1 Register Bits*/
#define HPD_CHANGE_INT			(1<<6)
#define RSEN_CHANGE_INT			(1<<5)

/* MHL TX INTR1_ENABLE 0x75 Register Bits*/
#define HPD_CHANGE_INT_MASK		(1<<6)
#define RSEN_CHANGE_INT_MASK		(1<<5)

/* CBUS_INT_1_ENABLE: CBUS Transaction Interrupt #1 Mask */
#define CBUS_INTR1_ENABLE_REG		0x09
#define CBUS_INTR2_ENABLE_REG		0x1F

/* CBUS Interrupt Status Registers*/
#define CBUS_INT_STATUS_1_REG		0x08
#define CBUS_INT_STATUS_2_REG		0x1E

/* CBUS INTR1 STATUS Register bits */
#define MSC_RESP_ABORT			(1<<6)
#define MSC_REQ_ABORT			(1<<5)
#define MSC_REQ_DONE			(1<<4)
#define MSC_MSG_RECD			(1<<3)
#define CBUS_DDC_ABORT			(1<<2)

/* CBUS INTR1 STATUS 0x09 Enable Mask*/
#define MSC_RESP_ABORT_MASK		(1<<6)
#define MSC_REQ_ABORT_MASK		(1<<5)
#define MSC_REQ_DONE_MASK		(1<<4)
#define MSC_MSG_RECD_MASK		(1<<3)
#define CBUS_DDC_ABORT_MASK		(1<<2)

/* CBUS INTR2 STATUS Register bits */
#define WRT_STAT_RECD			(1<<3)
#define SET_INT_RECD			(1<<2)
#define WRT_BURST_RECD			(1<<0)

/* CBUS INTR2 STATUS 0x1F Enable Mask*/
#define WRT_STAT_RECD_MASK		(1<<3)
#define SET_INT_RECD_MASK		(1<<2)
#define WRT_BURST_RECD_MASK		(1<<0)

#define CBUS_INTR_STATUS_1_ENABLE_MASK\
			(MSC_RESP_ABORT_MASK |\
			MSC_REQ_ABORT_MASK |\
			MSC_REQ_DONE_MASK |\
			MSC_MSG_RECD_MASK |\
			CBUS_DDC_ABORT_MASK)

#define CBUS_INTR_STATUS_2_ENABLE_MASK\
			(WRT_STAT_RECD_MASK |\
			SET_INT_RECD_MASK)

#define MHL_INT_EDID_CHG		(1<<1)

#define MHL_RCHANGE_INT			0x20
#define MHL_DCHANGE_INT			0x21
#define MHL_INT_DCAP_CHG		(1<<0)
#define MHL_INT_DSCR_CHG		(1<<1)
#define MHL_INT_REQ_WRT			(1<<2)
#define MHL_INT_GRT_WRT			(1<<3)

/* CBUS Control Registers*/
/* Retry count for all MSC commands*/
#define MSC_RETRY_FAIL_LIM_REG		0x1D

/* reason for MSC_REQ_ABORT interrupt on CBUS */
#define MSC_REQ_ABORT_REASON_REG	0x0D
#define MSC_RESP_ABORT_REASON_REG	0x0E

/* MSC Requestor Abort Reason Register bits*/
#define ABORT_BY_PEER			(1<<7)
#define UNDEF_CMD			(1<<3)
#define TIMEOUT				(1<<2)
#define PROTO_ERROR			(1<<1)
#define MAX_FAIL			(1<<0)

#define REG_CBUS_INTR_STATUS		0x08
/* Responder aborted DDC command at translation layer */
#define BIT_DDC_ABORT			(1<<2)
/* Responder sent a VS_MSG packet (response data or command.) */
#define BIT_MSC_MSG_RCV			(1<<3)
/* Responder sent ACK packet (not VS_MSG) */
#define BIT_MSC_XFR_DONE		(1<<4)
/* Command send aborted on TX side */
#define BIT_MSC_XFR_ABORT		(1<<5)
#define BIT_MSC_ABORT			(1<<6)

/* Set HPD came from Downstream, */
#define SET_HPD_DOWNSTREAM		(1<<6)

/* MHL TX DISC1 Register Bits */
#define DISC_EN				(1<<0)

/* MHL TX DISC2 Register Bits */
#define SKIP_GND			(1<<6)
#define ATT_THRESH_SHIFT		0x04
#define ATT_THRESH_MASK			(0x03 << ATT_THRESH_SHIFT)
#define USB_D_OEN			(1<<3)
#define DEGLITCH_TIME_MASK		0x07
#define DEGLITCH_TIME_2MS		0
#define DEGLITCH_TIME_4MS		1
#define DEGLITCH_TIME_8MS		2
#define DEGLITCH_TIME_16MS		3
#define DEGLITCH_TIME_40MS		4
#define DEGLITCH_TIME_50MS		5
#define DEGLITCH_TIME_60MS		6
#define DEGLITCH_TIME_128MS		7

#define DISC_CTRL3_COMM_IMME		(1<<7)
#define DISC_CTRL3_FORCE_MHL		(1<<6)
#define DISC_CTRL3_FORCE_USB		(1<<4)
#define DISC_CTRL3_USB_EN		(1<<3)

/* MHL TX DISC4 0x93 Register Bits*/
#define CBUS_DISC_PUP_SEL_SHIFT		6
#define CBUS_DISC_PUP_SEL_MASK		(3<<CBUS_DISC_PUP_SEL_SHIFT)
#define CBUS_DISC_PUP_SEL_10K		(2<<CBUS_DISC_PUP_SEL_SHIFT)
#define CBUS_DISC_PUP_SEL_OPEN		(0<<CBUS_DISC_PUP_SEL_SHIFT)
#define CBUS_IDLE_PUP_SEL_SHIFT		4
#define CBUS_IDLE_PUP_SEL_MASK		(3<<CBUS_IDLE_PUP_SEL_SHIFT)
#define CBUS_IDLE_PUP_SEL_OPEN		(0<<CBUS_IDLE_PUP_SEL_SHIFT)

/* MHL TX DISC5 0x94 Register Bits */
#define CBUS_MHL_PUP_SEL_MASK		0x03
#define CBUS_MHL_PUP_SEL_5K		0x01
#define CBUS_MHL_PUP_SEL_OPEN		0x00

/* MHL Interrupt Registers */
#define CBUS_MHL_INTR_REG_0		0xA0
#define CBUS_MHL_INTR_REG_1		0xA1
#define CBUS_MHL_INTR_REG_2		0xA2
#define CBUS_MHL_INTR_REG_3		0xA3

/* MHL Status Registers */
#define CBUS_MHL_STATUS_REG_0		0xB0
#define CBUS_MHL_STATUS_REG_1		0xB1
#define CBUS_MHL_STATUS_REG_2		0xB2
#define CBUS_MHL_STATUS_REG_3		0xB3

/* MHL TX DISC6 0x95 Register Bits */
#define USB_D_OVR			(1<<7)
#define USB_ID_OVR			(1<<6)
#define DVRFLT_SEL			(1<<5)
#define BLOCK_RGND_INT			(1<<4)
#define SKIP_DEG			(1<<3)
#define CI2CA_POL			(1<<2)
#define CI2CA_WKUP			(1<<1)
#define SINGLE_ATT			(1<<0)

/* MHL TX DISC7 0x96 Register Bits
 *
 * Bits 7 and 6 are labeled as reserved but seem to be related to toggling
 * the CBUS signal when generating the wake pulse sequence.
 */
#define USB_D_ODN			(1<<5)
#define VBUS_CHECK			(1<<2)
#define RGND_INTP_MASK			0x03
#define RGND_INTP_OPEN			0
#define RGND_INTP_2K			1
#define RGND_INTP_1K			2
#define RGND_INTP_SHORT			3

/* TPI Addr 0x7A Registers */
#define TPI_DPD_REG			0x3D

#define TPI_PD_TMDS			(1<<5)
#define TPI_PD_OSC_EN			(1<<4)
#define TPI_TCLK_PHASE			(1<<3)
#define TPI_PD_IDCK			(1<<2)
#define TPI_PD_OSC			(1<<1)
#define TPI_PD				(1<<0)

#define CBUS_CONFIG_REG			0x07
#define CBUS_LINK_CONTROL_2_REG		0x31

/* HDMI RX Registers */
#define HDMI_RX_TMDS0_CCTRL1_REG	0x10
#define HDMI_RX_TMDS_CLK_EN_REG		0x11
#define HDMI_RX_TMDS_CH_EN_REG		0x12
#define HDMI_RX_PLL_CALREFSEL_REG	0x17
#define HDMI_RX_PLL_VCOCAL_REG		0x1A
#define HDMI_RX_EQ_DATA0_REG		0x22
#define HDMI_RX_EQ_DATA1_REG		0x23
#define HDMI_RX_EQ_DATA2_REG		0x24
#define HDMI_RX_EQ_DATA3_REG		0x25
#define HDMI_RX_EQ_DATA4_REG		0x26
#define HDMI_RX_TMDS_ZONE_CTRL_REG	0x4C
#define HDMI_RX_TMDS_MODE_CTRL_REG	0x4D

/* MHL SideBand Channel(MSC) Registers */
#define CBUS_MSC_COMMAND_START_REG	0x12
#define CBUS_MSC_OFFSET_REG		0x13
#define CBUS_MSC_FIRST_DATA_OUT_REG	0x14
#define CBUS_MSC_SECOND_DATA_OUT_REG	0x15
#define CBUS_MSC_FIRST_DATA_IN_REG	0x16
#define CBUS_MSC_SECOND_DATA_IN_REG	0x17
#define CBUS_MSC_MSG_CMD_IN_REG		0x18
#define CBUS_MSC_MSG_DATA_IN_REG	0x19
#define CBUS_MSC_WRITE_BURST_LEN	0x20
/*Turn content streaming ON.*/
#define CBUS_MSC_RAP_CONTENT_ON		0x10
/*Turn content streaming OFF.*/
#define CBUS_MSC_RAP_CONTENT_OFF	0x11
/*Register Bits for CBUS_MSC_COMMAND_START_REG */
#define START_BIT_MSC_RESERVED		(1<<0)
#define START_BIT_MSC_MSG		(1<<1)
#define START_BIT_READ_DEVCAP		(1<<2)
#define START_BIT_WRITE_STAT_INT	(1<<3)
#define START_BIT_WRITE_BURST		(1<<4)

/* MHL Device Capability Register offsets */
#define DEVCAP_DEV_STATE		0x00
#define DEVCAP_MHL_VERSION		0x01
#define DEVCAP_DEV_CAT			0x02
#define DEVCAP_ADOPTER_ID_H		0x03
#define DEVCAP_ADOPTER_ID_L		0x04
#define DEVCAP_VID_LINK_MODE		0x05
#define DEVCAP_AUD_LINK_MODE		0x06
#define DEVCAP_VIDEO_TYPE		0x07
#define DEVCAP_LOG_DEV_MAP		0x08
#define DEVCAP_BANDWIDTH		0x09
#define DEVCAP_DEV_FEATURE_FLAG		0x0A
#define DEVCAP_DEVICE_ID_H		0x0B
#define DEVCAP_DEVICE_ID_L		0x0C
#define DEVCAP_SCRATCHPAD_SIZE		0x0D
#define DEVCAP_INT_STAT_SIZE		0x0E
#define DEVCAP_RESERVED			0x0F

#define MHL_DEV_CATEGORY_POW_BIT	(1<<4)

/* Device Capability Ready Bit */
#define MHL_STATUS_DCAP_READY		(1<<0)

#define MHL_FEATURE_RCP_SUPPORT		(1<<0)
#define MHL_FEATURE_RAP_SUPPORT		(1<<1)
#define MHL_FEATURE_SP_SUPPORT		(1<<2)

#define MHL_STATUS_CLK_MODE_PACKED_PIXEL	0x02
#define MHL_STATUS_CLK_MODE_NORMAL	0x03

#define MHL_STATUS_PATH_ENABLED		0x08
#define MHL_STATUS_PATH_DISABLED	0x00

#define MHL_STATUS_REG_CONNECTED_RDY	0x30

#define CBUS_PKT_BUF_COUNT		18

/* page0:0x79 HPD Control Register */
#define	MHL_HPD_OUT_OVR_EN	(1<<4)
#define	MHL_HPD_OUT_OVR_VAL	(1<<5)

enum page_num {
	PAGE0 = 0,
	PAGE1,
	PAGE2,
	PAGE3
};

enum rgnd_state {
	RGND_UNKNOWN = 0,
	RGND_OPEN,
	RGND_1K,
	RGND_2K,
	RGND_SHORT
};

enum mhl_state {
	STATE_DISCONNECTED = 0,
	STATE_DISCOVERY_FAILED,
	STATE_CBUS_LOCKOUT,
	STATE_ESTABLISHED,
};

enum msc_subcommand {
	/* MSC_MSG Sub-Command codes */
	MSG_RCP =	0x10,
	MSG_RCPK =	0x11,
	MSG_RCPE =	0x12,
	MSG_RAP =	0x20,
	MSG_RAPK =	0x21,
};

enum cbus_command {
	CBUS_IDLE =	0x00,
	CBUS_ACK =		0x33,
	CBUS_NACK =		0x35,
	CBUS_WRITE_STAT  =	0x60 | 0x80,
	CBUS_SET_INT     =	0x60,
	CBUS_READ_DEVCAP =	0x61,
	CBUS_GET_STATE =	0x62,
	CBUS_GET_VENDOR_ID =	0x63,
	CBUS_SET_HPD =		0x64,
	CBUS_CLR_HPD =		0x65,
	CBUS_SET_CAP_ID =	0x66,
	CBUS_GET_CAP_ID =	0x67,
	CBUS_MSC_MSG =		0x68,
	CBUS_GET_SC1_ERR_CODE =	0x69,
	CBUS_GET_DDC_ERR_CODE =	0x6A,
	CBUS_GET_MSC_ERR_CODE =	0x6B,
	CBUS_WRITE_BURST =	0x6C,
	CBUS_GET_SC3_ERR_CODE =	0x6D,
};

enum mhl_status_type {
	NO_MHL_STATUS = 0x00,
	MHL_INIT_DONE,
	MHL_WAITING_RGND_DETECT,
	MHL_CABLE_CONNECT,
	MHL_DISCOVERY_START,
	MHL_DISCOVERY_END,
	MHL_DISCOVERY_SUCCESS,
	MHL_DISCOVERY_FAIL,
	MHL_RSEN_GLITCH,
	MHL_RSEN_LOW,
};

struct mhl_tx_status_type {
	u8 intr4_mask_value;
	u8 intr1_mask_value;
	u8 intr_cbus1_mask_value;
	u8 intr_cbus2_mask_value;
	enum mhl_status_type	mhl_status;
	u8 linkmode;
	u8 connected_ready;
	bool cbus_connected;
	bool sink_hpd;
	bool rgnd_1k;
	u8 rsen_check_available;
};

static struct device *sii9244_mhldev;

static struct workqueue_struct *sii9234_wq;

static inline bool mhl_state_is_error(enum mhl_state state)
{
	return state == STATE_DISCOVERY_FAILED ||
		state == STATE_CBUS_LOCKOUT;
}
/* Structure for holding MSC command data */
struct cbus_packet {
	enum cbus_command command;
	u8 offset;
	u8 data[2];
	u8 status;
};

struct device_cap {
	u8 mhl_ver;
	u8 dev_type;
	u16 adopter_id;
	u8 vid_link_mode;
	u8 aud_link_mode;
	u8 video_type;
	u8 log_dev_map;
	u8 bandwidth;
	u16 device_id;
	u8 scratchpad_size;
	u8 int_stat_size;

	bool rcp_support;
	bool rap_support;
	bool sp_support;
};

struct sii9234_data {
	struct sii9234_platform_data	*pdata;
	wait_queue_head_t		wq;
	struct work_struct		sii9234_int_work;

	bool				claimed;
	enum mhl_state			state;
	enum rgnd_state			rgnd;
	bool				rsen;
	atomic_t			is_callback_scheduled;

	struct mutex			lock;
	struct mutex			cbus_lock;
	struct cbus_packet		cbus_pkt;
	struct cbus_packet		cbus_pkt_buf[CBUS_PKT_BUF_COUNT];
	struct device_cap		devcap;
	struct mhl_tx_status_type	mhl_status_value;
#ifdef CONFIG_SII9234_RCP
	u8 error_key;
	struct input_dev		*input_dev;
#endif
};

static u8 sii9234_tmds_control(struct sii9234_data *sii9234, bool enable);
static void __sii9234_power_down(struct sii9234_data *sii9234);

#ifdef CONFIG_SII9234_CBUS
static bool cbus_command_request(struct sii9234_data *sii9234,
				 enum cbus_command command,
				 u8 offset, u8 data);
static void cbus_command_response(struct sii9234_data *sii9234);
#endif

#ifdef	CONFIG_SAMSUNG_USE_11PIN_CONNECTOR
static int is_mhl_cable_connected(void)
{
	return	max77693_muic_get_status1_adc1k_value();
}
#endif

static	int mhl_onoff_ex(struct sii9234_data *sii9234, bool onoff)
{
	int ret = 0;

	if (!sii9234 || !sii9234->pdata) {
		printk(KERN_ERR	"[ERR] %s() getting resource is failed\n",
			__func__);
		return -ENXIO;
	}

	mutex_lock(&sii9234->lock);
	if (sii9234->pdata->power_state == onoff) {
		pr_info("%s() mhl already %s\n",
			__func__, onoff ? "on" : "off");
		ret = -ENXIO;
		goto exit;
	} else
		pr_info("%s(%s)\n", __func__, onoff ? "on" : "off");

	sii9234->pdata->power_state = onoff; /*save power state*/

	if (sii9234->pdata->mhl_sel)
		sii9234->pdata->mhl_sel(onoff);

	if (onoff) {
		if (sii9234->pdata->hw_onoff)
				sii9234->pdata->hw_onoff(1);

		if (sii9234->pdata->hw_reset)
			sii9234->pdata->hw_reset();

	} else {
		pr_info("%s() call __sii9234_power_down\n", __func__);

#ifdef	CONFIG_SAMSUNG_WORKAROUND_HPD_GLANCE
		mhl_hpd_handler(false);
#endif

		__sii9234_power_down(sii9234);

		if (sii9234->pdata->hw_onoff)
			sii9234->pdata->hw_onoff(0);
		else
			printk(KERN_ERR	"[ERROR] %s() hw_onoff is NULL\n",
				__func__);

#ifdef	CONFIG_SAMSUNG_USE_11PIN_CONNECTOR
		ret = is_mhl_cable_connected();
		if (ret == 1) {
			pr_info("%s() mhl still inserted, retry discovery\n",
				__func__);
			queue_work(sii9234_wq, &(sii9234->sii9234_int_work));
		} else if (ret == 0) {
			atomic_set(&sii9234->is_callback_scheduled, false);
			pr_info("%s() mhl cable is removed\n", __func__);
		} else {
			atomic_set(&sii9234->is_callback_scheduled, false);
			pr_err("[ERROR] %s() is_mhl_cable_connected error : %d\n",
				__func__, ret);
		}
#else
		atomic_set(&sii9234->is_callback_scheduled, false);
#endif
	}

exit:
	mutex_unlock(&sii9234->lock);
	return ret;
}

static int mhl_tx_write_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 value)
{
	int ret;
	ret = i2c_smbus_write_byte_data(sii9234->pdata->mhl_tx_client, offset,
					value);
	if (ret < 0)
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);

	return ret;
}

static int mhl_tx_read_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 *value)
{
	int ret;

	if (!value)
		return -EINVAL;

	ret = i2c_smbus_write_byte(sii9234->pdata->mhl_tx_client, offset);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	ret = i2c_smbus_read_byte(sii9234->pdata->mhl_tx_client);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	*value = ret & 0x000000FF;

	return 0;
}

static int mhl_tx_set_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 mask)
{
	int ret;
	u8 value;

	ret = mhl_tx_read_reg(sii9234, offset, &value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	value |= mask;

	ret = mhl_tx_write_reg(sii9234, offset, value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	return ret;
}

static int mhl_tx_clear_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 mask)
{
	int ret;
	u8 value;

	ret = mhl_tx_read_reg(sii9234, offset, &value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	value &= ~mask;

	ret = mhl_tx_write_reg(sii9234, offset, value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	return ret;
}

static int tpi_write_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 value)
{
	int ret;
	ret = i2c_smbus_write_byte_data(sii9234->pdata->tpi_client, offset,
					value);
	if (ret < 0)
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);

	return ret;
}

static int tpi_read_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 *value)
{
	int ret;

	if (!value) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return -EINVAL;
	}

	ret = i2c_smbus_write_byte(sii9234->pdata->tpi_client, offset);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	ret = i2c_smbus_read_byte(sii9234->pdata->tpi_client);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	*value = ret & 0x000000FF;

	return 0;
}

static int hdmi_rx_write_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 value)
{
	int ret;
	ret = i2c_smbus_write_byte_data(sii9234->pdata->hdmi_rx_client, offset,
			value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
	}
	return ret;
}

static int hdmi_rx_read_reg(struct sii9234_data *sii9234, unsigned int offset,
			    u8 *value)
{
	int ret;

	if (!value) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return -EINVAL;
	}
	ret = i2c_smbus_write_byte(sii9234->pdata->hdmi_rx_client, offset);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	ret = i2c_smbus_read_byte(sii9234->pdata->hdmi_rx_client);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	*value = ret & 0x000000FF;

	return 0;
}

static int cbus_write_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 value)
{
	int ret;
	ret = i2c_smbus_write_byte_data(sii9234->pdata->cbus_client, offset,
					value);
	if (ret < 0)
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);

	return ret;
}

static int cbus_read_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 *value)
{
	int ret;

	if (!value) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return -EINVAL;
	}
	ret = i2c_smbus_write_byte(sii9234->pdata->cbus_client, offset);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	ret = i2c_smbus_read_byte(sii9234->pdata->cbus_client);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	*value = ret & 0x000000FF;

	return 0;
}

static int cbus_set_reg(struct sii9234_data *sii9234, unsigned int offset,
		u8 mask)
{
	int ret;
	u8 value;

	ret = cbus_read_reg(sii9234, offset, &value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	value |= mask;

	ret = cbus_write_reg(sii9234, offset, value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}
	return ret;
}

static int mhl_wake_toggle(struct sii9234_data *sii9234,
		unsigned long high_period,
		unsigned long low_period)
{
	int ret;

	/* These bits are not documented. */
	ret =
	    mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL7_REG, (1<<7) | (1<<6));
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	usleep_range(high_period * USEC_PER_MSEC, high_period * USEC_PER_MSEC);

	ret =
	    mhl_tx_clear_reg(sii9234, MHL_TX_DISC_CTRL7_REG, (1<<7) | (1<<6));
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	usleep_range(low_period * USEC_PER_MSEC, low_period * USEC_PER_MSEC);

	return 0;
}

static int mhl_send_wake_pulses(struct sii9234_data *sii9234)
{
	int ret;

	ret = mhl_wake_toggle(sii9234, T_SRC_WAKE_PULSE_WIDTH_1,
			T_SRC_WAKE_PULSE_WIDTH_1);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = mhl_wake_toggle(sii9234, T_SRC_WAKE_PULSE_WIDTH_1,
			T_SRC_WAKE_PULSE_WIDTH_2);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = mhl_wake_toggle(sii9234, T_SRC_WAKE_PULSE_WIDTH_1,
			T_SRC_WAKE_PULSE_WIDTH_1);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = mhl_wake_toggle(sii9234, T_SRC_WAKE_PULSE_WIDTH_1,
			      T_SRC_WAKE_TO_DISCOVER);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	return 0;
}

static int sii9234_cbus_reset(struct sii9234_data *sii9234)
{
	int ret;
	int idx;

	/* Reset CBUS */
	ret = mhl_tx_set_reg(sii9234, MHL_TX_SRST, 0x03);
	if (ret < 0)
		return ret;

	usleep_range(2000, 3000);

	ret = mhl_tx_clear_reg(sii9234, MHL_TX_SRST, 0x03);
	if (ret < 0)
		return ret;

	for (idx = 0 ; idx < 4 ; idx++) {
		cbus_write_reg(sii9234, 0xE0 + idx, 0xFF);
		cbus_write_reg(sii9234, 0xF0 + idx, 0xFF);
	}

	/* Adjust interrupt mask everytime reset is performed.*/
	ret = cbus_write_reg(sii9234, CBUS_INTR1_ENABLE_REG, 0);
	if (ret < 0) {
		printk(KERN_ERR	"[ERROR] %s() write CBUS_INTR1_ENABLE_REG fail\n",
			__func__);
		return ret;
	}

	ret = cbus_write_reg(sii9234, CBUS_INTR2_ENABLE_REG, 0);
	if (ret < 0) {
		printk(KERN_ERR	"[ERROR] %s() write CBUS_INTR2_ENABLE_REG fail\n",
			__func__);
		return ret;
	}

	return 0;
}

/* require to chek mhl imformation of samsung in cbus_init_register*/
static int sii9234_cbus_init(struct sii9234_data *sii9234)
{
	u8 value;

	pr_debug("sii9234: cbus init\n");
	cbus_write_reg(sii9234, 0x07, 0xF2);

	cbus_write_reg(sii9234, 0x40, 0x03);
	cbus_write_reg(sii9234, 0x42, 0x06);
	cbus_write_reg(sii9234, 0x36, 0x0C);
	cbus_write_reg(sii9234, 0x3D, 0xFD);

	cbus_write_reg(sii9234, 0x1C, 0x01);
	cbus_write_reg(sii9234, 0x1D, 0x0F);

	cbus_write_reg(sii9234, 0x44, 0x02);

	cbus_write_reg(sii9234, 0x31, 0x0D);
	cbus_write_reg(sii9234, 0x22, 0x01);
	cbus_write_reg(sii9234, 0x30, 0x01);

	/* Setup our devcap*/
	cbus_write_reg(sii9234, 0x80, 0x00);/*To meet cts 6.3.10.1 spec*/

	cbus_write_reg(sii9234, 0x81, 0x11);
	cbus_write_reg(sii9234, 0x82, 0x02);
	cbus_write_reg(sii9234, 0x83, 0);
	cbus_write_reg(sii9234, 0x84, 0);
	cbus_write_reg(sii9234, 0x85, 0x01);
	cbus_write_reg(sii9234, 0x86, 0x01);
	cbus_write_reg(sii9234, 0x87, 0);
	cbus_write_reg(sii9234, 0x88, (1<<1) | (1<<2) | (1<<3) | (1<<7));
	cbus_write_reg(sii9234, 0x89, 0x0F);
	cbus_write_reg(sii9234, 0x8A, (1<<0) | (1<<1) | (1<<2));
	cbus_write_reg(sii9234, 0x8B, 0);
	cbus_write_reg(sii9234, 0x8C, 0);
	cbus_write_reg(sii9234, 0x8D, 16);
	cbus_write_reg(sii9234, 0x8E, 0x33);
	cbus_write_reg(sii9234, 0x8F, 0);

	cbus_read_reg(sii9234, 0x31, &value);
	value |= 0x0C;
	cbus_write_reg(sii9234, 0x31, value);

	cbus_write_reg(sii9234, 0x30, 0x01);

	cbus_read_reg(sii9234, 0x3C, &value);
	value &= ~0x38;
	value |= 0x30;
	cbus_write_reg(sii9234, 0x3C, value);

	cbus_read_reg(sii9234, 0x22, &value);
	value &= ~0x0F;
	value |= 0x0D;
	cbus_write_reg(sii9234, 0x22, value);

	cbus_read_reg(sii9234, 0x2E, &value);
	value |= 0x15;
	cbus_write_reg(sii9234, 0x2E, value);

	cbus_write_reg(sii9234, CBUS_INTR1_ENABLE_REG, 0);
	cbus_write_reg(sii9234, CBUS_INTR2_ENABLE_REG, 0);

	return 0;
}

static void force_usb_id_switch_open(struct sii9234_data *sii9234)
{
	/*Disable CBUS discovery*/
	mhl_tx_clear_reg(sii9234, MHL_TX_DISC_CTRL1_REG, (1<<0));
	/*Force USB ID switch to open*/
	mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL6_REG, USB_ID_OVR);

	/* siliconimage recommanded value is 0xA6 But make detection error */
	mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL3_REG, 0x86);
/*	mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL3_REG, 0xA6); */
	/*Force upstream HPD to 0 when not in MHL mode.*/
	mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG, (1<<4) | (1<<5));
}

static void release_usb_id_switch_open(struct sii9234_data *sii9234)
{
	msleep(T_SRC_CBUS_FLOAT);
	/* clear USB ID switch to open*/
	mhl_tx_clear_reg(sii9234, MHL_TX_DISC_CTRL6_REG, USB_ID_OVR);
	/* Enable CBUS discovery*/
	mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL1_REG, (1<<0));
}

#ifdef CONFIG_SII9234_CBUS
static void cbus_req_abort_error(struct sii9234_data *sii9234)
{
	u8 abort_reason = 0;

	pr_debug("sii9234: MSC Request Aborted:");

	cbus_read_reg(sii9234, MSC_REQ_ABORT_REASON_REG, &abort_reason);
	cbus_write_reg(sii9234, MSC_REQ_ABORT_REASON_REG, 0xff);

	if (abort_reason) {
		if (abort_reason & BIT_MSC_XFR_ABORT) {
			cbus_read_reg(sii9234, MSC_REQ_ABORT_REASON_REG,
							&abort_reason);
			pr_cont("ABORT_REASON_REG = %d\n", abort_reason);
			cbus_write_reg(sii9234, MSC_REQ_ABORT_REASON_REG, 0xff);
		}
		if (abort_reason & BIT_MSC_ABORT) {
			cbus_read_reg(sii9234, BIT_MSC_ABORT, &abort_reason);
			pr_cont("BIT_MSC_ABORT = %d\n", abort_reason);
			cbus_write_reg(sii9234, BIT_MSC_ABORT, 0xff);
		}
		if (abort_reason & ABORT_BY_PEER)
			pr_cont(" Peer Sent an ABORT");
		if (abort_reason & UNDEF_CMD)
			pr_cont(" Undefined Opcode");
		if (abort_reason & TIMEOUT)
			pr_cont(" Requestor Translation layer Timeout");
		if (abort_reason & PROTO_ERROR)
			pr_cont(" Protocol Error");
		if (abort_reason & MAX_FAIL) {
			u8 msc_retry_thr_val = 0;
			pr_cont(" Retry Threshold exceeded");
			cbus_read_reg(sii9234,
					MSC_RETRY_FAIL_LIM_REG,
					&msc_retry_thr_val);
			pr_cont("Retry Threshold value is:%d",
					msc_retry_thr_val);
		}
	}
	pr_cont("\n");
}

static void cbus_resp_abort_error(struct sii9234_data *sii9234)
{
	u8 abort_reason = 0;

	pr_debug("sii9234: MSC Response Aborted:");
	cbus_read_reg(sii9234, MSC_RESP_ABORT_REASON_REG, &abort_reason);
	cbus_write_reg(sii9234, MSC_RESP_ABORT_REASON_REG, 0xFF);

	if (abort_reason) {
		if (abort_reason & ABORT_BY_PEER)
			pr_cont(" Peer Sent an ABORT");
		if (abort_reason & UNDEF_CMD)
			pr_cont(" Undefined Opcode");
		if (abort_reason & TIMEOUT)
			pr_cont(" Requestor Translation layer Timeout");
	}
		pr_cont("\n");
}

static bool cbus_ddc_abort_error(struct sii9234_data *sii9234)
{
	u8 val1, val2;

	/* clear the ddc abort counter */
	cbus_write_reg(sii9234, 0x29, 0xFF);
	cbus_read_reg(sii9234, 0x29, &val1);
	usleep_range(3000, 4000);
	cbus_read_reg(sii9234, 0x29, &val2);
	if (val2 > (val1 + 50)) {
		pr_debug("Applying DDC Abort Safety(SWA 18958)\n)");
		mhl_tx_set_reg(sii9234, MHL_TX_SRST, (1<<3));
		mhl_tx_clear_reg(sii9234, MHL_TX_SRST, (1<<3));
		force_usb_id_switch_open(sii9234);
		release_usb_id_switch_open(sii9234);
		mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL1_REG, 0xD0);
		sii9234_tmds_control(sii9234, false);
		/* Disconnect and notify to OTG */
		return true;
	}
	pr_debug("sii9234: DDC abort interrupt\n");

	return false;
}

#ifdef CONFIG_SII9234_RCP
static void rcp_uevent_report(struct sii9234_data *sii9234, u8 key)
{
#ifdef MHL_RCP_FRAMEWORK
	char tempkey[32];
	char *envp[] = { tempkey, NULL };

	struct kobject *kobj = &sii9234->pdata->cbus_client->dev.kobj;

	snprintf(tempkey, sizeof(tempkey),
			"MHL_RCP=%s", code[key].name);
	kobject_uevent_env(kobj, KOBJ_CHANGE, envp);
#else
	pr_info("rcp_uevent_report key: %d\n", key);
	input_report_key(sii9234->input_dev, (unsigned int)key+1, 1);
	input_report_key(sii9234->input_dev, (unsigned int)key+1, 0);
	input_sync(sii9234->input_dev);
#endif
}

static void cbus_process_rcp_key(struct sii9234_data *sii9234, u8 key)
{
	if (code[key].valid) {
		/* Report the key */
		rcp_uevent_report(sii9234, key);
		/* Send the RCP ack */
		cbus_command_request(sii9234, CBUS_MSC_MSG,
						MSG_RCPK, key);
	} else {
		sii9234->error_key = key;
		/*
		* Send a RCPE(RCP Error Message) to Peer followed by
		* RCPK with old key-code so that initiator(TV) can
		* recognize failed key code.error code = 0x01 means
		* Ineffective key code was received.
		* See Table 21.(PRM)for details.
		*/
		cbus_command_request(sii9234, CBUS_MSC_MSG, MSG_RCPE, 0x01);
	}
}

static void cbus_process_rap_key(struct sii9234_data *sii9234, u8 key)
{
	if (CBUS_MSC_RAP_CONTENT_ON == key)
		sii9234_tmds_control(sii9234, true);
	else if (CBUS_MSC_RAP_CONTENT_OFF == key)
		sii9234_tmds_control(sii9234, false);

	cbus_command_request(sii9234, CBUS_MSC_MSG, MSG_RAPK, 0x00);
}

/*
 * Incoming MSC_MSG : RCP/RAP/RCPK/RCPE/RAPK commands
 *
 * Process RCP key codes and the send supported keys to userspace.
 * If a key is not supported then an error ack is sent to the peer.  Note
 * that by default all key codes are supported.
 *
 * An alternate method might be to decide the validity of the key in the
 * driver itself.  However, the driver does not have any criteria to which
 * to make this decision.
 */
static void cbus_handle_msc_msg(struct sii9234_data *sii9234)
{
	u8 cmd_code, key;

	mutex_lock(&sii9234->cbus_lock);
	if (sii9234->state != STATE_ESTABLISHED) {
		pr_debug("sii9234: invalid MHL state\n");
		mutex_unlock(&sii9234->cbus_lock);
		return;
	}

	cbus_read_reg(sii9234, CBUS_MSC_MSG_CMD_IN_REG, &cmd_code);
	cbus_read_reg(sii9234, CBUS_MSC_MSG_DATA_IN_REG, &key);

	pr_debug("sii9234: cmd_code:%d, key:%d\n", cmd_code, key);

	switch (cmd_code) {
	case MSG_RCP:
		pr_debug("sii9234: RCP Arrived. KEY CODE:%d\n", key);
		mutex_unlock(&sii9234->cbus_lock);
		cbus_process_rcp_key(sii9234, key);
		return;
	case MSG_RAP:
		pr_debug("sii9234: RAP Arrived\n");
		mutex_unlock(&sii9234->cbus_lock);
		cbus_process_rap_key(sii9234, key);
		return;
	case MSG_RCPK:
		pr_debug("sii9234: RCPK Arrived\n");
		break;
	case MSG_RCPE:
		pr_debug("sii9234: RCPE Arrived\n");
		break;
	case MSG_RAPK:
		pr_debug("sii9234: RAPK Arrived\n");
		break;
	default:
		pr_debug("sii9234: MAC error\n");
		mutex_unlock(&sii9234->cbus_lock);
		cbus_command_request(sii9234, CBUS_GET_MSC_ERR_CODE, 0, 0);
		return;
	}
	mutex_unlock(&sii9234->cbus_lock);
}
#endif	/* CONFIG_SII9234_RCP */

void mhl_path_enable(struct sii9234_data *sii9234, bool path_en)
{
	pr_debug("sii9234: mhl_path_enable MHL_STATUS_PATH %s\n",
					path_en ? "ENABLED" : "DISABLED");

	if (path_en)
		sii9234->mhl_status_value.linkmode |= MHL_STATUS_PATH_ENABLED;
	else
		sii9234->mhl_status_value.linkmode &= ~MHL_STATUS_PATH_ENABLED;

	cbus_command_request(sii9234, CBUS_WRITE_STAT, CBUS_LINK_CONTROL_2_REG,
					sii9234->mhl_status_value.linkmode);
}


static void cbus_handle_wrt_stat_recd(struct sii9234_data *sii9234)
{
	u8 status_reg0, status_reg1;

	pr_debug("sii9234: CBUS WRT_STAT_RECD\n");

	/*
	* The two MHL status registers need to read to ensure that the MSC is
	* ready to receive the READ_DEVCAP command.
	* The READ_DEVCAP command is need to determine the dongle power state
	* and whether RCP, RCPE, RCPK, RAP, and RAPE are supported.
	*
	* Note that this is not documented properly in the PRM.
	*/

	cbus_read_reg(sii9234, CBUS_MHL_STATUS_REG_0, &status_reg0);
	cbus_write_reg(sii9234, CBUS_MHL_STATUS_REG_0, 0xFF);

	cbus_read_reg(sii9234, CBUS_MHL_STATUS_REG_1, &status_reg1);
	cbus_write_reg(sii9234, CBUS_MHL_STATUS_REG_1, 0xFF);

	pr_debug("sii9234: STATUS_REG0 : [%d];STATUS_REG1 : [%d]\n",
			status_reg0, status_reg1);

	if (!(sii9234->mhl_status_value.linkmode &
				MHL_STATUS_PATH_ENABLED) &&
				(MHL_STATUS_PATH_ENABLED & status_reg1)) {
		mhl_path_enable(sii9234, true);
	} else if ((sii9234->mhl_status_value.linkmode &
				MHL_STATUS_PATH_ENABLED) &&
				!(MHL_STATUS_PATH_ENABLED & status_reg1)) {
		mhl_path_enable(sii9234, false);
	}

	if (status_reg0 & MHL_STATUS_DCAP_READY) {
		pr_debug("sii9234: DEV CAP READY\n");
		cbus_command_request(sii9234, CBUS_READ_DEVCAP,
						DEVCAP_DEV_CAT, 0x00);
		cbus_command_request(sii9234, CBUS_READ_DEVCAP,
						DEVCAP_DEV_FEATURE_FLAG, 0x00);
	}
}

static void cbus_handle_edid_change(struct sii9234_data *sii9234)
{
		/* Enable Overriding HPD OUT */
		mhl_tx_set_reg(sii9234, MHL_TX_INT_CTRL_REG, (1<<4));

		/*
		 * As per HDMI specification to indicate EDID change
		 * in TV (or sink), we need to toggle HPD line.
		 */

		/* HPD OUT = Low */
		mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG, (1<<5));

		/* A SET_HPD command shall not follow a CLR_HPD command
		 * within less than THPD_WIDTH(50ms).
		 */
		msleep(T_HPD_WIDTH);

		/* HPD OUT = High */
		mhl_tx_set_reg(sii9234, MHL_TX_INT_CTRL_REG, (1<<5));

		/* Disable Overriding of HPD OUT */
		mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG, (1<<4));
}

static void cbus_handle_set_int_recd(struct sii9234_data *sii9234)
{
	u8 intr_reg0, intr_reg1, value;

	/* read and clear interrupt*/
	cbus_read_reg(sii9234, CBUS_MHL_INTR_REG_0, &intr_reg0);
	cbus_write_reg(sii9234, CBUS_MHL_INTR_REG_0, intr_reg0);

	cbus_read_reg(sii9234, CBUS_MHL_INTR_REG_1, &intr_reg1);
	cbus_write_reg(sii9234, CBUS_MHL_INTR_REG_1, intr_reg1);

	cbus_read_reg(sii9234, CBUS_MHL_INTR_REG_2, &value);
	cbus_write_reg(sii9234, CBUS_MHL_INTR_REG_2, value);

	cbus_read_reg(sii9234, CBUS_MHL_INTR_REG_3, &value);
	cbus_write_reg(sii9234, CBUS_MHL_INTR_REG_3, value);

	pr_debug("sii9234: INTR_REG0 : [%d]; INTR_REG1 : [%d]\n",
			intr_reg0, intr_reg1);

	if (intr_reg0 & MHL_INT_DCAP_CHG) {
		pr_debug("sii9234: MHL_INT_DCAP_CHG\n");
		cbus_command_request(sii9234, CBUS_READ_DEVCAP,
				DEVCAP_DEV_CAT, 0x00);
		cbus_command_request(sii9234, CBUS_READ_DEVCAP,
				DEVCAP_DEV_FEATURE_FLAG, 0x00);
	}

	if (intr_reg0 & MHL_INT_DSCR_CHG)
		pr_debug("sii9234:  MHL_INT_DSCR_CHG\n");

	if (intr_reg0 & MHL_INT_REQ_WRT) {
		pr_debug("sii9234:  MHL_INT_REQ_WRT\n");
		cbus_command_request(sii9234, CBUS_SET_INT,
					 MHL_RCHANGE_INT, MHL_INT_GRT_WRT);
	}

	if (intr_reg0 & MHL_INT_GRT_WRT)
		pr_debug("sii9234:  MHL_INT_GRT_WRT\n");

	if (intr_reg1 & MHL_INT_EDID_CHG) {
		pr_debug("sii9234: MHL_INT_EDID_CHG\n");

		cbus_handle_edid_change(sii9234);
	}
}
#endif /* CONFIG_SII9234_CBUS */

static int sii9234_power_init(struct sii9234_data *sii9234)
{
	int ret;

	/* Force the SiI9234 into the D0 state. */
	ret = tpi_write_reg(sii9234, TPI_DPD_REG, 0x3F);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Enable TxPLL Clock */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_TMDS_CLK_EN_REG, 0x01);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Enable Tx Clock Path & Equalizer */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_TMDS_CH_EN_REG, 0x15);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Power Up TMDS */
	ret = mhl_tx_write_reg(sii9234, 0x08, 0x35);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	return 0;
}

static int sii9234_hdmi_init(struct sii9234_data *sii9234)
{
	int ret;
	/* Analog PLL Control
	 * bits 5:4 = 2b00 as per characterization team.
	 */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_TMDS0_CCTRL1_REG, 0xC1);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* PLL Calrefsel */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_PLL_CALREFSEL_REG, 0x03);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* VCO Cal */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_PLL_VCOCAL_REG, 0x20);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Auto EQ */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_EQ_DATA0_REG, 0x8A);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Auto EQ */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_EQ_DATA1_REG, 0x6A);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Auto EQ */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_EQ_DATA2_REG, 0xAA);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Auto EQ */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_EQ_DATA3_REG, 0xCA);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Auto EQ */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_EQ_DATA4_REG, 0xEA);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Manual zone */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_TMDS_ZONE_CTRL_REG, 0xA0);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* PLL Mode Value */
	ret = hdmi_rx_write_reg(sii9234, HDMI_RX_TMDS_MODE_CTRL_REG, 0x00);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

/* need to test */
/*	ret = mhl_tx_write_reg(sii9234, MHL_TX_TMDS_CCTRL, 0x34); */
	ret = mhl_tx_write_reg(sii9234, MHL_TX_TMDS_CCTRL, 0x24);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = hdmi_rx_write_reg(sii9234, 0x45, 0x44);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Rx PLL BW ~ 4MHz */
	ret = hdmi_rx_write_reg(sii9234, 0x31, 0x0A);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	/* Analog PLL Control
	 * bits 5:4 = 2b00 as per characterization team.
	 */
	return 0;
}

static int sii9234_mhl_tx_ctl_int(struct sii9234_data *sii9234)
{
	int ret;

	ret = mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL1_REG, 0xD0);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL2_REG, 0xFC);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL4_REG, 0xEB);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL7_REG, 0x0C);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	return 0;
}

static void __sii9234_power_down(struct sii9234_data *sii9234)
{
	disable_irq_nosync(sii9234->pdata->mhl_tx_client->irq);

	if (sii9234->claimed) {
		if (sii9234->pdata->vbus_present)
			sii9234->pdata->vbus_present(false);
	}

	sii9234->state = STATE_DISCONNECTED;
	sii9234->claimed = false;

	/* power down mhl intenal modules */
	tpi_write_reg(sii9234, TPI_DPD_REG, 0);
}

int rsen_state_timer_out(struct sii9234_data *sii9234)
{
	int ret = 0;
	u8 value;

	ret = mhl_tx_read_reg(sii9234, MHL_TX_SYSSTAT_REG, &value);
	if (ret < 0) {
		printk(KERN_ERR
			"[ERROR] %s() read MHL_TX_SYSSTAT_REG\n", __func__);
		goto err_exit;
	}

	sii9234->rsen = value & RSEN_STATUS;

	if (value & RSEN_STATUS) {
		pr_info("sii9234: MHL cable connected.. RESN High\n");
	} else {
		pr_info("sii9234: RSEN lost\n");
		/* Once RSEN loss is confirmed,we need to check
		 * based on cable status and chip power status,whether
		 * it is SINK Loss(HDMI cable not connected, TV Off)
		 * or MHL cable disconnection
		 * TODO: Define the below mhl_disconnection()
		 */
		msleep(T_SRC_RXSENSE_DEGLITCH);
		ret = mhl_tx_read_reg(sii9234, MHL_TX_SYSSTAT_REG, &value);
		if (ret < 0) {
			printk(KERN_ERR
				"[ERROR] %s() read MHL_TX_SYSSTAT_REG\n",
				__func__);
			goto err_exit;
		}

		pr_info("sys_stat: %x ~\n", value);
		if ((value & RSEN_STATUS) == 0)	{
			printk(KERN_INFO
				"%s() RSEN Really LOW ~\n", __func__);
			/*To meet CTS 3.3.22.2 spec*/
			sii9234_tmds_control(sii9234, false);
			force_usb_id_switch_open(sii9234);
			release_usb_id_switch_open(sii9234);
			ret = -1;
			goto err_exit;
		}
	}
	return ret;

err_exit:
	printk(KERN_ERR	"[ERROR] %s() err_exit\n", __func__);
	/*turn off mhl and change usb_sel to usb*/
	mhl_onoff_ex(sii9234, false);
	return ret;
}

void sii9234_mhl_detection_sched(void)
{
	struct sii9234_data *sii9234 = dev_get_drvdata(sii9244_mhldev);
	printk(KERN_INFO "%s()\n", __func__);

	if (!sii9234 || !sii9234->pdata) {
		printk(KERN_ERR
			"[ERR] %s() getting resource is failed\n", __func__);
		return;
	}

	mutex_lock(&sii9234->lock);
	if (atomic_read(&sii9234->is_callback_scheduled) == false) {
		atomic_set(&sii9234->is_callback_scheduled, true);
		queue_work(sii9234_wq, &(sii9234->sii9234_int_work));
	} else {
		pr_debug("sii9234: detection callback is already scheduled.\n");
	}
	mutex_unlock(&sii9234->lock);
}
EXPORT_SYMBOL(sii9234_mhl_detection_sched);

static void sii9234_detection_callback(struct work_struct *p)
{
	struct sii9234_data *sii9234;
	int ret;
	u8 value;

	printk(KERN_INFO "%s()\n", __func__);

	sii9234 = container_of(p, struct sii9234_data, sii9234_int_work);
	if (sii9234 == NULL) {
		printk(KERN_ERR "[ERROR] %s() sii9234 error!!\n", __func__);
		return;
	}

	mhl_onoff_ex(sii9234, true);

	mutex_lock(&sii9234->lock);
	sii9234->mhl_status_value.linkmode = MHL_STATUS_CLK_MODE_NORMAL;
	sii9234->rgnd = RGND_UNKNOWN;
	sii9234->state = STATE_DISCONNECTED;
	sii9234->rsen = false;
	memset(sii9234->cbus_pkt_buf, 0x00, sizeof(sii9234->cbus_pkt_buf));

	/* Set the board configuration so the  SiI9234 has access to the
	 * external connector.
	 */
	if (sii9234->pdata == NULL) {
		printk(KERN_ERR "[ERROR] %s():%d sii9234->pdata error!!\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = sii9234_power_init(sii9234);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d sii9234_power_init failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = sii9234_cbus_reset(sii9234);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d sii9234_cbus_reset failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = sii9234_hdmi_init(sii9234);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d sii9234_hdmi_init failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = sii9234_mhl_tx_ctl_int(sii9234);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d sii9234_mhl_tx_ctl_init failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* Enable HDCP Compliance safety */
	ret = mhl_tx_write_reg(sii9234, 0x2B, 0x01);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d mhl_tx_write_reg failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* CBUS discovery cycle time for each drive and float = 150us */
	ret = mhl_tx_read_reg(sii9234, MHL_TX_DISC_CTRL1_REG, &value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d mhl_tx_read_reg failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	value &= ~(1 << 3);
	value |= (1 << 2);

	ret = mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL1_REG, value);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d mhl_tx_write_reg failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* Clear bit 6 (reg_skip_rgnd) */
	ret = mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL2_REG,
			(1<<7) /* Reserved Bit */ |
			2 << ATT_THRESH_SHIFT |
			DEGLITCH_TIME_50MS);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d  Clear bit 6 failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* Changed from 66 to 65 for 94[1:0] = 01 = 5k reg_cbusmhl_pup_sel */
	/* 1.8V CBUS VTH & GND threshold */
	/*To meet CTS 3.3.7.2 spec*/
	ret = mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL5_REG, 0x77);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write MHL_TX_DISC_CTRL5_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* set bit 2 and 3, which is Initiator Timeout */
	ret = cbus_read_reg(sii9234, CBUS_LINK_CONTROL_2_REG, &value);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d  read CBUS_LINK_CONTROL_2_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	value |= 0x0C;

	ret = cbus_write_reg(sii9234, CBUS_LINK_CONTROL_2_REG, value);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d  write CBUS_LINK_CONTROL_2_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL6_REG, 0xA0);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write MHL_TX_MHLTX_CTL6_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* RGND & single discovery attempt (RGND blocking) */
	ret = mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL6_REG, 0x71);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write MHL_TX_DISC_CTRL6_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* Use VBUS path of discovery state machine */
	ret = mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL8_REG, 0x0);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write MHL_TX_DISC_CTRL8_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL6_REG, USB_ID_OVR);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d set MHL_TX_DISC_CTRL6_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* To allow RGND engine to operate correctly.
	 * When moving the chip from D2 to D0 (power up, init regs)
	 * the values should be
	 * 94[1:0] = 01  reg_cbusmhl_pup_sel[1:0] should be set for 5k
	 * 93[7:6] = 10  reg_cbusdisc_pup_sel[1:0] should be
	 * set for 10k (default)
	 * 93[5:4] = 00  reg_cbusidle_pup_sel[1:0] = open (default)
	 */
	/* siliconimage recommanded value is 0xA6 But make detection error */
	ret = mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL3_REG, 0x86);
/*	ret = mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL3_REG, 0xA6); */
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d set MHL_TX_DISC_CTRL3_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* change from CC to 8C to match 5K */
	/*To meet CTS 3.3.72 spec*/
	ret = mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL4_REG, 0x8C);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d set MHL_TX_DISC_CTRL4_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* Force upstream HPD to 0 when not in MHL mode */
	mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG, MHL_HPD_OUT_OVR_VAL);
	mhl_tx_set_reg(sii9234, MHL_TX_INT_CTRL_REG, MHL_HPD_OUT_OVR_EN);

	/* Configure the interrupt as active high */
	ret =
	    mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG, (1 << 2) | (1 << 1));
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d clear MHL_TX_INT_CTRL_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	msleep(25);

#if 0
	/* release usb_id switch */
	ret = mhl_tx_clear_reg(sii9234, MHL_TX_DISC_CTRL6_REG, USB_ID_OVR);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d clear MHL_TX_DISC_CTRL6_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}
#endif

	ret = mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL1_REG, 0x27);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%dwrite MHL_TX_DISC_CTRL1_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	sii9234_cbus_init(sii9234);

	/* Enable Auto soft reset on SCDT = 0 */
	ret = mhl_tx_write_reg(sii9234, 0x05, 0x04);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d write 0x05 failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	/* HDMI Transcode mode enable */
	ret = mhl_tx_write_reg(sii9234, 0x0D, 0x1C);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%dHDMI Transcode mode enable failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_INTR4_ENABLE_REG,
			       RGND_READY_MASK | CBUS_LKOUT_MASK |
			       MHL_DISC_FAIL_MASK | MHL_EST_MASK);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write MHL_TX_INTR4_ENABLE_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_INTR1_ENABLE_REG,
						RSEN_CHANGE_INT_MASK);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%dwrite MHL_TX_INTR1_ENABLE_REG failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	ret = cbus_write_reg(sii9234, CBUS_INTR1_ENABLE_REG,
					CBUS_INTR_STATUS_1_ENABLE_MASK);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s() cbus register init failed !\n",
		       __func__);
		goto unhandled;
	}

	ret = cbus_write_reg(sii9234, CBUS_INTR2_ENABLE_REG,
					CBUS_INTR_STATUS_2_ENABLE_MASK);
	if (ret < 0)
		goto unhandled;

/* this point is very importand before megsure RGND impedance*/
	force_usb_id_switch_open(sii9234);

	mhl_tx_clear_reg(sii9234, MHL_TX_DISC_CTRL4_REG,
					(1<<7) | (1<<6) | (1<<5) | (1<<4));
/*	mhl_tx_clear_reg(sii9234, MHL_TX_DISC_CTRL5_REG, (1<<1) | (1<<0));*/
	release_usb_id_switch_open(sii9234);
/*end of this*/
	pr_debug("sii9234: waiting for RGND measurement\n");
	enable_irq(sii9234->pdata->mhl_tx_client->irq);

	/* SiI9244 Programmer's Reference Section 2.4.3
	 * State : RGND Ready
	 */
	mutex_unlock(&sii9234->lock);
	ret = wait_event_timeout(sii9234->wq,
				 ((sii9234->rgnd != RGND_UNKNOWN) ||
				  mhl_state_is_error(sii9234->state)),
				 msecs_to_jiffies(T_WAIT_TIMEOUT_RGND_INT));

	mutex_lock(&sii9234->lock);
	if (ret == 0 || mhl_state_is_error(sii9234->state)) {
		printk(KERN_INFO "%s():%d wait RGND Ready failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	if (sii9234->rgnd != RGND_1K) {
		printk(KERN_ERR "[ERROR] %s():%d rgnd impedance failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	mutex_unlock(&sii9234->lock);

	pr_debug("sii9234: waiting for detection\n");
	ret = wait_event_timeout(sii9234->wq,
				 sii9234->state != STATE_DISCONNECTED,
				 msecs_to_jiffies(T_WAIT_TIMEOUT_DISC_INT));

	mutex_lock(&sii9234->lock);
	if (ret == 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d wait detection state failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	if (sii9234->state == STATE_DISCONNECTED) {
		printk(KERN_ERR
		       "[ERROR] %s():%d sii9234->state is STATE_DISCONNECTED !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	if (sii9234->state == STATE_DISCOVERY_FAILED) {
		printk(KERN_ERR
		       "[ERROR] %s():%d STATE_DISCOVERY_FAILED failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	if (mhl_state_is_error(sii9234->state)) {
		printk(KERN_ERR "[ERROR] %s():%d mhl_state_is_error failed !\n",
		       __func__, __LINE__);
		goto unhandled;
	}

	mutex_unlock(&sii9234->lock);
	pr_debug("sii9234: waiting for RSEN_INT\n");
	ret = wait_event_timeout(sii9234->wq, sii9234->rsen,
				 msecs_to_jiffies(T_WAIT_TIMEOUT_RSEN_INT));
	mutex_lock(&sii9234->lock);
	if (ret == 0) {
		/*CTS 3.3.14.3 Discovery;Sink Never Drives MHL+/- HIGH*/
		/*MHL SPEC 8.2.1.1.1;Transition SRC4-SRC7*/
		if (rsen_state_timer_out(sii9234) < 0) {
			pr_info("sii9234: rsen_state_timer_out\n");
			goto unhandled;
		}
	}

	if (!sii9234->rsen) {
		ret = mhl_tx_read_reg(sii9234, MHL_TX_SYSSTAT_REG, &value);
		pr_debug("sii9234: Recheck RSEN value\n");
		if (!(ret && (value & RSEN_STATUS))) {
			msleep(T_SRC_RXSENSE_DEGLITCH);
			pr_debug("sii9234: RSEN is low -> retry once\n");

			ret = mhl_tx_read_reg(sii9234, MHL_TX_SYSSTAT_REG,
								&value);
			if (!(ret && (value & RSEN_STATUS))) {
				pr_debug("sii9234: RSEN is still low\n");
				goto unhandled;
			}

		}
		sii9234->rsen = value & RSEN_STATUS;
	}

	pr_info("sii9234: connection established\n");
	sii9234->claimed = true;
#ifdef	CONFIG_SAMSUNG_WORKAROUND_HPD_GLANCE
	mhl_hpd_handler(true);
#endif

	if (sii9234->pdata->vbus_present)
		sii9234->pdata->vbus_present(true);

	mutex_unlock(&sii9234->lock);

	return;

 unhandled:
	pr_info("sii9234: Detection failed");
	if (sii9234->state == STATE_DISCONNECTED)
		pr_cont(" (timeout)");
	else if (sii9234->state == STATE_DISCOVERY_FAILED)
		pr_cont(" (discovery failed)");
	else if (sii9234->state == STATE_CBUS_LOCKOUT)
		pr_cont(" (cbus_lockout)");
	pr_cont("\n");

	/*mhl spec: 8.3.3, if discovery failed, must retry discovering*/
	if ((sii9234->state == STATE_DISCOVERY_FAILED) &&
						 (sii9234->rgnd == RGND_1K)) {
		pr_cont("Discovery failed but RGND_1K impedence"
					" restart detection_callback");

		if (sii9234->pdata->hw_reset)
			sii9234->pdata->hw_reset();

		queue_work(sii9234_wq, &(sii9234->sii9234_int_work));
		mutex_unlock(&sii9234->lock);
	} else {
		printk(KERN_INFO	"%s() mhl power off\n", __func__);
		mutex_unlock(&sii9234->lock);
		mhl_onoff_ex(sii9234, false);
	}

	pr_debug("sii9234: dectection callback finished\n");
	return;
}

#ifdef	CONFIG_SII9234_CBUS

static void save_cbus_pkt_to_buffer(struct sii9234_data *sii9234)
{
	int index;

	for (index = 0; index < CBUS_PKT_BUF_COUNT; index++)
		if (sii9234->cbus_pkt_buf[index].status == false)
			break;

	if (index == CBUS_PKT_BUF_COUNT) {
		pr_debug("sii9234: Error save_cbus_pkt Buffer Full\n");
		index -= 1; /*adjust index*/
	}

	pr_debug("sii9234: save_cbus_pkt_to_buffer index = %d\n", index);
	memcpy(&sii9234->cbus_pkt_buf[index], &sii9234->cbus_pkt,
					sizeof(struct cbus_packet));
	sii9234->cbus_pkt_buf[index].status = true;
}

static void cbus_command_response(struct sii9234_data *sii9234)
{
	bool unsupported_key_received = false;
	bool req_write_status = false;
	u8 value, offset = 0;

	mutex_lock(&sii9234->cbus_lock);
	pr_debug("sii9234: cbus_command_response\n");

	switch (sii9234->cbus_pkt.command) {
	case CBUS_MSC_MSG:
		pr_debug("sii9234: cbus_command_response Received"
					" ACK for CBUS_MSC_MSG\n");
#ifdef CONFIG_SII9234_RCP
		if (sii9234->cbus_pkt.data[0] == MSG_RCPE &&
					sii9234->cbus_pkt.data[1] == 0x01) {
			sii9234->cbus_pkt.command = CBUS_IDLE;
			mutex_unlock(&sii9234->cbus_lock);
			cbus_command_request(sii9234, CBUS_MSC_MSG, MSG_RCPK,
							sii9234->error_key);
			return;
		}
#endif
		break;
	case CBUS_WRITE_STAT:
		pr_debug("sii9234: cbus_command_response"
					"CBUS_WRITE_STAT\n");
		cbus_read_reg(sii9234, CBUS_MSC_FIRST_DATA_IN_REG,
					&sii9234->cbus_pkt.data[0]);
		break;
	case CBUS_SET_INT:
		pr_debug("sii9234: cbus_command_response CBUS_SET_INT\n");
		if (sii9234->cbus_pkt.offset == MHL_RCHANGE_INT &&
				sii9234->cbus_pkt.data[0] == MHL_INT_DSCR_CHG) {
			/*Write burst final step... Req->GRT->Write->DSCR*/
			pr_debug("sii9234: MHL_RCHANGE_INT &"
						"MHL_INT_DSCR_CHG\n");
		} else if (sii9234->cbus_pkt.offset == MHL_RCHANGE_INT &&
				sii9234->cbus_pkt.data[0] == MHL_INT_DCAP_CHG) {
			pr_debug("sii9234: MHL_RCHANGE_INT &"
						"MHL_INT_DCAP_CHG\n");
			sii9234->cbus_pkt.command = CBUS_IDLE;
			mutex_unlock(&sii9234->cbus_lock);
			cbus_command_request(sii9234, CBUS_WRITE_STAT,
						MHL_STATUS_REG_CONNECTED_RDY,
						MHL_STATUS_DCAP_READY);
			return;
		}
		break;
	case CBUS_WRITE_BURST:
		pr_debug("sii9234: cbus_command_response"
						"MHL_WRITE_BURST\n");
		sii9234->cbus_pkt.command = CBUS_IDLE;
		mutex_unlock(&sii9234->cbus_lock);
		cbus_command_request(sii9234, CBUS_SET_INT,
					 MHL_RCHANGE_INT, MHL_INT_DSCR_CHG);
		return;
	case CBUS_READ_DEVCAP:
		pr_debug("sii9234: cbus_command_response"
					" CBUS_READ_DEVCAP\n");
		cbus_read_reg(sii9234, CBUS_MSC_FIRST_DATA_IN_REG,
								 &value);
		switch (sii9234->cbus_pkt.offset) {
		case DEVCAP_MHL_VERSION:
			sii9234->devcap.mhl_ver = value;
			pr_debug("sii9234: MHL_VERSION: %X\n", value);
			break;
		case DEVCAP_DEV_CAT:
			if (value & MHL_DEV_CATEGORY_POW_BIT)
				pr_debug("sii9234: CAT=POWERED");
			else
				pr_debug("sii9234: CAT=UNPOWERED");
				break;
		case DEVCAP_ADOPTER_ID_H:
			sii9234->devcap.adopter_id = (value & 0xFF) << 0x8;
			pr_debug("sii9234: DEVCAP_ADOPTER_ID_H = %X\n", value);
			break;
		case DEVCAP_ADOPTER_ID_L:
			sii9234->devcap.adopter_id |= value & 0xFF;
			pr_debug("sii9234: DEVCAP_ADOPTER_ID_L = %X\n", value);
			break;
		case DEVCAP_VID_LINK_MODE:
			sii9234->devcap.vid_link_mode = 0x3F & value;
			pr_debug("sii9234: MHL_CAP_VID_LINK_MODE = %d\n",
					 sii9234->devcap.vid_link_mode);
			break;
		case DEVCAP_AUD_LINK_MODE:
			sii9234->devcap.aud_link_mode = 0x03 & value;
			pr_debug("sii9234: DEVCAP_AUD_LINK_MODE =%d\n",
					sii9234->devcap.aud_link_mode);
			break;
		case DEVCAP_VIDEO_TYPE:
			sii9234->devcap.video_type = 0x8F & value;
			pr_debug("sii9234: DEVCAP_VIDEO_TYPE =%d\n",
					sii9234->devcap.video_type);
			break;
		case DEVCAP_LOG_DEV_MAP:
			sii9234->devcap.log_dev_map = value;
			pr_debug("sii9234: DEVCAP_LOG_DEV_MAP =%d\n",
					sii9234->devcap.log_dev_map);
			break;
		case DEVCAP_BANDWIDTH:
			sii9234->devcap.bandwidth = value;
			pr_debug("sii9234: DEVCAP_BANDWIDTH =%d\n",
						sii9234->devcap.bandwidth);
			break;
		case DEVCAP_DEV_FEATURE_FLAG:
			if ((value & MHL_FEATURE_RCP_SUPPORT) == 0)
				pr_debug("sii9234: FEATURE_FLAG=RCP");

			if ((value & MHL_FEATURE_RAP_SUPPORT) == 0)
				pr_debug("sii9234: FEATURE_FLAG=RAP\n");

			if ((value & MHL_FEATURE_SP_SUPPORT) == 0)
				pr_debug("sii9234: FEATURE_FLAG=SP\n");
			break;
		case DEVCAP_DEVICE_ID_H:
			sii9234->devcap.device_id = (value & 0xFF) << 0x8;
			pr_info("sii9234: DEVICE_ID_H=0x%x\n", value);
			offset = DEVCAP_DEVICE_ID_L;
			break;
		case DEVCAP_DEVICE_ID_L:
			sii9234->devcap.device_id |= value & 0xFF;
			pr_info("sii9234: DEVICE_ID_L=0x%x\n", value);
			break;
		case DEVCAP_SCRATCHPAD_SIZE:
			  sii9234->devcap.scratchpad_size = value;
			  pr_debug("sii9234: DEVCAP_SCRATCHPAD_SIZE =%d\n",
					sii9234->devcap.scratchpad_size);
			  break;
		case DEVCAP_INT_STAT_SIZE:
			  sii9234->devcap.int_stat_size = value;
			  pr_debug("sii9234: DEVCAP_INT_STAT_SIZE =%d\n",
					sii9234->devcap.int_stat_size);
			  break;
		case DEVCAP_RESERVED:
			pr_info("sii9234: DEVCAP_RESERVED : %d\n", value);
			break;
		case DEVCAP_DEV_STATE:
			pr_debug("sii9234: DEVCAP_DEV_STATE\n");
			break;
		default:
			pr_debug("sii9234: DEVCAP DEFAULT\n");
			 break;
		}
		break;
	default:
		pr_debug("sii9234: error: cbus_command_response"
			" cannot handle : %d\n", sii9234->cbus_pkt.command);
	}

	sii9234->cbus_pkt.command = CBUS_IDLE;
	mutex_unlock(&sii9234->cbus_lock);

#ifdef CONFIG_SII9234_RCP
	if (unsupported_key_received)
		cbus_command_request(sii9234, CBUS_MSC_MSG, MSG_RCPK,
						sii9234->error_key);
#endif

	if (offset)
		cbus_command_request(sii9234, CBUS_READ_DEVCAP,
						offset, 0x00);

	if (req_write_status)
		cbus_command_request(sii9234, CBUS_WRITE_STAT,
					MHL_STATUS_REG_CONNECTED_RDY,
						MHL_STATUS_DCAP_READY);
}

#ifdef DEBUG_MHL
static void cbus_command_response_dbg_msg(struct sii9234_data *sii9234,
							 u8 index)
{
	/*Added to debugcbus_pkt_buf*/
	pr_info("sii9234: cbus_pkt_buf[index].command = %d,"
			"sii9234->cbus_pkt.command = %d\n",
					sii9234->cbus_pkt_buf[index].command,
					sii9234->cbus_pkt.command);
	pr_info("sii9234: cbus_pkt_buf[index].data[0] = %d,"
			"sii9234->cbus_pkt.data[0] =  %d\n",
					sii9234->cbus_pkt_buf[index].data[0],
					sii9234->cbus_pkt.data[0]);

	pr_info("sii9234: cbus_pkt_buf[index].data[1] = %d,"
			 "sii9234->cbus_pkt.data[1] = %d\n",
					sii9234->cbus_pkt_buf[index].data[1],
					sii9234->cbus_pkt.data[1]);
	pr_info("sii9234: cbus_pkt_buf[index].offset = %d,"
			"sii9234->cbus_pkt.offset = %d\n",
					sii9234->cbus_pkt_buf[index].offset,
					sii9234->cbus_pkt.offset);
}
#endif

static void cbus_command_response_all(struct sii9234_data *sii9234)
{
	u8 index;
	struct cbus_packet cbus_pkt_process_buf[CBUS_PKT_BUF_COUNT];
	pr_debug("sii9234: cbus_command_response_all for All requests\n");

	/*take bkp of cbus_pkt_buf*/
	memcpy(cbus_pkt_process_buf, sii9234->cbus_pkt_buf,
			sizeof(cbus_pkt_process_buf));

	/*clear cbus_pkt_buf  to hold next request*/
	memset(sii9234->cbus_pkt_buf, 0x00, sizeof(sii9234->cbus_pkt_buf));

	/*process all previous requests*/
	for (index = 0; index < CBUS_PKT_BUF_COUNT; index++) {
		if (cbus_pkt_process_buf[index].status == true) {
			memcpy(&sii9234->cbus_pkt, &cbus_pkt_process_buf[index],
						sizeof(struct cbus_packet));
			cbus_command_response(sii9234);
#ifdef DEBUG_MHL
			/*print cbus_cmd messg*/
			cbus_command_response_dbg_msg(sii9234, index);
#endif
		}
	}
}

static bool cbus_command_request(struct sii9234_data *sii9234,
				 enum cbus_command command, u8 offset, u8 data)
{
	u8 start_bit = 0;

	mutex_lock(&sii9234->cbus_lock);
	if (sii9234->state != STATE_ESTABLISHED) {
		pr_debug("sii9234: cbus_command_request without establish\n");
		pr_debug("sii9234: ==> command:0x%X, offset:0x%X",
							command, offset);
			mutex_unlock(&sii9234->cbus_lock);
			return -EINVAL;
	}

	sii9234->cbus_pkt.command = command;
	sii9234->cbus_pkt.offset = offset;
	if (command == CBUS_MSC_MSG)
		sii9234->cbus_pkt.data[0] = offset;
	else
		sii9234->cbus_pkt.data[0] = data;

	pr_debug("sii9234: %s() Sending MSC_MSG SubCommand=%d"
				", key-code=%d\n", __func__,
				sii9234->cbus_pkt.offset,
				sii9234->cbus_pkt.data[0]);

	cbus_write_reg(sii9234, CBUS_MSC_OFFSET_REG,
						sii9234->cbus_pkt.offset);
	cbus_write_reg(sii9234, CBUS_MSC_FIRST_DATA_OUT_REG,
					sii9234->cbus_pkt.data[0]);

	switch (sii9234->cbus_pkt.command) {
	case CBUS_SET_INT:
		pr_debug("sii9234: cbus_command_request"
						" CBUS_SET_INT\n");
		start_bit = START_BIT_WRITE_STAT_INT;
		break;
	case CBUS_WRITE_STAT:
		pr_debug("sii9234: cbus_command_request"
						" CBUS_WRITE_STAT\n");
		start_bit = START_BIT_WRITE_STAT_INT;
		break;
	case CBUS_MSC_MSG:
		/*treat offset as data[0] in case of CBUS_MSC_MSG*/
		sii9234->cbus_pkt.data[0] = offset;
		sii9234->cbus_pkt.data[1] = data;
		pr_debug("sii9234: cbus_command_request CBUS_MSC_MSG"
					" SubCommand=%d,key-code=%d\n",
					sii9234->cbus_pkt.command,
					sii9234->cbus_pkt.data[1]);

		cbus_write_reg(sii9234, CBUS_MSC_SECOND_DATA_OUT_REG,
					sii9234->cbus_pkt.data[1]);
		cbus_write_reg(sii9234, CBUS_MSC_OFFSET_REG,
					sii9234->cbus_pkt.command);
		start_bit = START_BIT_MSC_MSG;
		break;
	case CBUS_READ_DEVCAP:
		pr_debug("sii9234: cbus_command_request CBUS_READ_DEVCAP\n");
		start_bit = START_BIT_READ_DEVCAP;
		break;
	case CBUS_WRITE_BURST:
		pr_debug("sii9234: cbus_command_request CBUS_WRITE_BURST\n");
		start_bit = START_BIT_WRITE_BURST;
		break;
	case CBUS_GET_STATE:
	case CBUS_GET_VENDOR_ID:
	case CBUS_SET_HPD:
	case CBUS_CLR_HPD:
	case CBUS_GET_MSC_ERR_CODE:
	case CBUS_GET_SC3_ERR_CODE:
	case CBUS_GET_SC1_ERR_CODE:
	case CBUS_GET_DDC_ERR_CODE:
		cbus_write_reg(sii9234, CBUS_MSC_OFFSET_REG,
					sii9234->cbus_pkt.command);
		start_bit = START_BIT_MSC_RESERVED;
		break;
	default:
		pr_debug("sii9234: error send cbus command fail\n");
		mutex_unlock(&sii9234->cbus_lock);
		return false;
	}

	pr_debug("sii9234: startbit = %d\n", start_bit);
	cbus_write_reg(sii9234, CBUS_MSC_COMMAND_START_REG, start_bit);
	save_cbus_pkt_to_buffer(sii9234);
	mutex_unlock(&sii9234->cbus_lock);

	return true;
}
#endif /* CONFIG_SII9234_CBUS */

static u8 sii9234_tmds_control(struct sii9234_data *sii9234, bool enable)
{
	u8 ret = -1;

	if (enable) {
		ret = mhl_tx_set_reg(sii9234, MHL_TX_TMDS_CCTRL, (1<<4));
		if (ret < 0) {
			printk(KERN_ERR	"%s() mhl_tx_set_reg fail\n", __func__);
			return ret;
		}
		pr_debug("sii9234: %s() MHL HPD High, enabled TMDS\n",
			__func__);
		ret = mhl_tx_set_reg(sii9234, MHL_TX_INT_CTRL_REG,
			MHL_HPD_OUT_OVR_EN | MHL_HPD_OUT_OVR_VAL);
	} else {
		ret = mhl_tx_clear_reg(sii9234, MHL_TX_TMDS_CCTRL, (1<<4));
		if (ret < 0) {
			printk(KERN_ERR
				"%s() mhl_tx_clear_reg fail\n", __func__);
			return ret;
		}
/*
		ret = mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG,
							(1<<4) | (1<<5));
*/
		ret = mhl_tx_clear_reg(sii9234, MHL_TX_INT_CTRL_REG,
							MHL_HPD_OUT_OVR_VAL);
		if (ret < 0) {
			printk(KERN_ERR
				"%s() clear MHL_TX_INT_CTRL_REG fail\n",
				__func__);
			return ret;
		}
		ret = mhl_tx_set_reg(sii9234, MHL_TX_INT_CTRL_REG,
							MHL_HPD_OUT_OVR_EN);
		if (ret < 0) {
			printk(KERN_ERR
				"%s() set MHL_TX_INT_CTRL_REG fail\n",
				__func__);
			return ret;
		}
		pr_info("sii9234 MHL HPD low, disabled TMDS\n");
	}

	return ret;
}

static irqreturn_t sii9234_irq_thread(int irq, void *data)
{
	struct sii9234_data *sii9234 = data;
	int ret;
	u8 intr1, intr4, value;
	u8 intr1_en, intr4_en;
	bool	retry_detection = false;
#ifdef	CONFIG_SII9234_CBUS
	u8 cbus_intr1, cbus_intr2;
#endif
	u8 mhl_poweroff = 0;
	void (*cbus_resp_callback)(struct sii9234_data *) = NULL;

	mutex_lock(&sii9234->lock);
	ret = mhl_tx_read_reg(sii9234, MHL_TX_INTR1_REG, &intr1);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d read MHL_TX_INTR1_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = mhl_tx_read_reg(sii9234, MHL_TX_INTR4_REG, &intr4);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d read MHL_TX_INTR4_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = mhl_tx_read_reg(sii9234, MHL_TX_INTR1_ENABLE_REG, &intr1_en);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d read MHL_TX_INTR1_ENABLE_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = mhl_tx_read_reg(sii9234, MHL_TX_INTR4_ENABLE_REG, &intr4_en);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d read MHL_TX_INTR4_ENABLE_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

#ifdef CONFIG_SII9234_CBUS
	ret = cbus_read_reg(sii9234, CBUS_INT_STATUS_1_REG, &cbus_intr1);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d read CBUS_INT_STATUS_1_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = cbus_read_reg(sii9234, CBUS_INT_STATUS_2_REG, &cbus_intr2);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d read CBUS_INT_STATUS_2_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = cbus_write_reg(sii9234, CBUS_INT_STATUS_1_REG, 0xFF);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write CBUS_INT_STATUS_1_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = cbus_write_reg(sii9234, CBUS_INT_STATUS_2_REG, 0xFF);
	if (ret < 0) {
		printk(KERN_ERR
		       "[ERROR] %s():%d write CBUS_INT_STATUS_2_REG failed !\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	pr_debug("sii9234: irq intr1(%02x/%02x)"
		" intr4(%02x/%02x) cbus(%02x/%02x\n)",
			intr1, intr1_en,
			intr4, intr4_en,
			cbus_intr1, cbus_intr2);
#endif

	if (intr4 & RGND_READY_INT) {
		ret = mhl_tx_read_reg(sii9234, MHL_TX_STAT2_REG, &value);
		if (ret < 0) {
			dev_err(&sii9234->pdata->mhl_tx_client->dev,
					"STAT2 reg, err %d\n", ret);
			goto err_exit;
		}

		switch (value & RGND_INTP_MASK) {
		case RGND_INTP_OPEN:
			pr_debug("sii9234: RGND Open\n");
			sii9234->rgnd = RGND_OPEN;
			memset(sii9234->cbus_pkt_buf, 0x00,
					sizeof(sii9234->cbus_pkt_buf));
			break;
		case RGND_INTP_1K:
			pr_debug("sii9234: RGND 1K\n");
/*After applying RGND patch, there is some issue about discovry failure
this poin is add to fix that problem*/
			ret = mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL4_REG,
				0x8C);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d write MHL_TX_DISC_CTRL4_REG failed !\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}

			mhl_tx_write_reg(sii9234, MHL_TX_DISC_CTRL5_REG, 0x77);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d write MHL_TX_DISC_CTRL5_REG failed !\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}

			mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL6_REG, 0x05);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d write MHL_TX_DISC_CTRL6_REG failed !\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}

			usleep_range(T_SRC_VBUS_CBUS_TO_STABLE * USEC_PER_MSEC,
				T_SRC_VBUS_CBUS_TO_STABLE  * USEC_PER_MSEC);

			ret = mhl_send_wake_pulses(sii9234);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d mhl_send_wake_pulses failed !\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}
			sii9234->rgnd = RGND_1K;

			cbus_write_reg(sii9234, CBUS_INTR1_ENABLE_REG, 0);
			cbus_write_reg(sii9234, CBUS_INTR2_ENABLE_REG, 0);
			break;

		case RGND_INTP_2K:
			pr_debug("sii9234: RGND 2K\n");
			sii9234->rgnd = RGND_2K;
			memset(sii9234->cbus_pkt_buf, 0x00,
					sizeof(sii9234->cbus_pkt_buf));
			break;
		case RGND_INTP_SHORT:
			pr_debug("sii9234: RGND Short\n");
			sii9234->rgnd = RGND_SHORT;
			memset(sii9234->cbus_pkt_buf, 0x00,
					sizeof(sii9234->cbus_pkt_buf));
			break;
		};

		if (sii9234->rgnd != RGND_1K) {
			printk(KERN_INFO
				"%s() rgnd is not 1K. power off\n", __func__);
			mhl_poweroff = 1; /*Power down mhl chip*/
			goto err_exit;
		}
	}

	if (intr1 & RSEN_CHANGE_INT) {
		ret = mhl_tx_read_reg(sii9234, MHL_TX_SYSSTAT_REG, &value);
		if (ret < 0) {
			pr_info("sii9234: MHL_TX_SYSSTAT_REG read error\n");
			goto err_exit;
		}
		sii9234->rsen = value & RSEN_STATUS;

		if (value & RSEN_STATUS) {
			pr_info("sii9234: MHL cable connected.. RSEN High\n");
		} else {
			pr_info("sii9234: RSEN lost -");
			/* Once RSEN loss is confirmed,we need to check
			 * based on cable status and chip power status,whether
			 * it is SINK Loss(HDMI cable not connected, TV Off)
			 * or MHL cable disconnection
			 */
			/* mhl_disconnection(); */
			msleep(T_SRC_RXSENSE_DEGLITCH);
			ret = mhl_tx_read_reg(sii9234, MHL_TX_SYSSTAT_REG,
								 &value);
			pr_cont(" sys_stat: %x\n", value);

			if ((value & RSEN_STATUS) == 0) {
				if (sii9234->claimed == true)
					retry_detection = true;

				memset(sii9234->cbus_pkt_buf, 0x00,
					sizeof(sii9234->cbus_pkt_buf));

				printk(KERN_INFO
					"%s() RSEN lost. tmds disable, power down\n",
					__func__);
				sii9234_tmds_control(sii9234, false);
				mhl_poweroff = 1; /*Power down mhl chip */
				goto i2c_error_exit;
			}
		}
	}

	if (intr4 & CBUS_LKOUT_INT) {
		pr_debug("sii9234: CBUS Lockout Interrupt\n");
		sii9234->state = STATE_CBUS_LOCKOUT;
	}

	if (intr4 & MHL_DISC_FAIL_INT) {
		printk(KERN_ERR	"[ERROR]	sii9234: MHL_DISC_FAIL_INT\n");
		sii9234->state = STATE_DISCOVERY_FAILED;
		memset(sii9234->cbus_pkt_buf, 0x00,
					sizeof(sii9234->cbus_pkt_buf));
	}

	if (intr4 & MHL_EST_INT) {
		pr_debug("sii9234: mhl est interrupt %d\n", value);
		/* work_around code for 1K & USB_OPEN TEST */
		if (!sii9234->rsen) {
			pr_info("sii9234: EST_INT without RSEN_HIGH\n");
			mhl_poweroff = 1;
			goto err_exit;
		}

		/* discovery override */
		ret = mhl_tx_write_reg(sii9234, MHL_TX_MHLTX_CTL1_REG, 0x10);
		if (ret < 0) {
			printk(KERN_ERR
			       "[ERROR] %s():%d write MHL_TX_MHLTX_CTRL1_REG failed !\n",
			       __func__, __LINE__);
			goto i2c_error_exit;
		}

		/* increase DDC translation layer timer (byte mode) */
		ret = cbus_write_reg(sii9234, 0x07, 0x32);
		if (ret < 0) {
			printk(KERN_ERR
			       "[ERROR] %s():%d cbus_write_reg failed !\n",
			       __func__, __LINE__);
			goto i2c_error_exit;
		}
		ret = cbus_set_reg(sii9234, 0x44, 1 << 1);
		if (ret < 0) {
			printk(KERN_ERR
			       "[ERROR] %s():%d cbus_set_reg failed !\n",
			       __func__, __LINE__);
			goto i2c_error_exit;
		}

		/* Keep the discovery enabled. Need RGND interrupt */
		ret = mhl_tx_set_reg(sii9234, MHL_TX_DISC_CTRL1_REG, (1 << 0));
		if (ret < 0) {
			printk(KERN_ERR
			       "[ERROR] %s():%d mhl_tx_set_reg failed !\n",
			       __func__, __LINE__);
			goto i2c_error_exit;
		}

		sii9234->state = STATE_ESTABLISHED;
#ifdef	CONFIG_SII9234_CBUS
		cbus_command_request(sii9234, CBUS_SET_INT,
					 MHL_RCHANGE_INT, MHL_INT_DCAP_CHG);

		mhl_tx_write_reg(sii9234, MHL_TX_INTR1_ENABLE_REG,
				RSEN_CHANGE_INT_MASK | HPD_CHANGE_INT_MASK);

		cbus_write_reg(sii9234, CBUS_INTR1_ENABLE_REG,
					CBUS_INTR_STATUS_1_ENABLE_MASK);
		cbus_write_reg(sii9234, CBUS_INTR2_ENABLE_REG,
					CBUS_INTR_STATUS_2_ENABLE_MASK);
#endif
	}

	if (intr1 & HPD_CHANGE_INT) {
		ret = cbus_read_reg(sii9234, MSC_REQ_ABORT_REASON_REG, &value);

		if (value & SET_HPD_DOWNSTREAM) {
			pr_info("sii9234: hpd high\n");
			/* Downstream HPD Highi */
#ifdef CONFIG_SII9234_CBUS
			/* Do we need to send HPD upstream using
			 * Register 0x79(page0)? Is HPD need to be overriden??
			 *      TODO: See if we need code for overriding HPD OUT
			 *      as per Page 0,0x79 Register
			 */
			sii9234->mhl_status_value.sink_hpd = true;
			ret = cbus_command_request(sii9234, CBUS_WRITE_STAT,
					CBUS_LINK_CONTROL_2_REG,
					sii9234->mhl_status_value.linkmode);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d cbus_command_request failed !\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}
#endif

			/* Enable TMDS */
			ret = sii9234_tmds_control(sii9234, true);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d set sii9234_tmds_control\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}
			pr_debug("sii9234: %s() MHL HPD High, enabled TMDS\n",
				__func__);

		} else {
			pr_info("sii9234: hpd low\n");
			/*Downstream HPD Low*/

			/* Similar to above comments.
			 * TODO:Do we need to override HPD OUT value
			 * and do we need to disable TMDS here?
			 */
			sii9234->mhl_status_value.sink_hpd = false;
			/* Disable TMDS */
			ret = sii9234_tmds_control(sii9234, false);
			if (ret < 0) {
				printk(KERN_ERR
				       "[ERROR] %s():%d set sii9234_tmds_control\n",
				       __func__, __LINE__);
				goto i2c_error_exit;
			}
		}
	}

#ifdef	CONFIG_SII9234_CBUS
	/*
	 * Process CBUS interrupts only when MHL connection has been
	 * established
	 */
	if (sii9234->state == STATE_ESTABLISHED) {

		if (cbus_intr1 & MSC_RESP_ABORT)
			cbus_resp_abort_error(sii9234);

		if (cbus_intr1 & MSC_REQ_ABORT)
			cbus_req_abort_error(sii9234);

		if ((cbus_intr1 & CBUS_DDC_ABORT) ||
					 (cbus_intr1 & MSC_RESP_ABORT)) {
			pr_debug("sii9234: CBUS DDC abort\n");
			if (cbus_ddc_abort_error(sii9234)) {
				printk(KERN_ERR "[ERROR] %s():%d cbus_ddc_abort_error\n",
					__func__, __LINE__);
				if (sii9234->claimed == true)
					retry_detection = true;
				mhl_poweroff = 1; /*Power down mhl chip */
				goto i2c_error_exit;
			}
		}

		if (cbus_intr1 & MSC_REQ_DONE) {
			pr_debug("sii9234: CBUS cmd ACK Received\n");
			cbus_resp_callback = cbus_command_response_all;
		}
#ifdef CONFIG_SII9234_RCP
		if (cbus_intr1 & MSC_MSG_RECD) {
			pr_debug("sii9234: MSC MSG Received\n");
			cbus_handle_msc_msg(sii9234);
		}
#endif

		if (cbus_intr1 & MSC_REQ_DONE) {
			pr_debug("sii9234: MSC_REQ_DONE(ACK) Received\n");
			cbus_read_reg(sii9234, CBUS_MHL_INTR_REG_1, &value);
			if (!(value & MHL_INT_EDID_CHG))
				cbus_resp_callback = cbus_command_response_all;
		}

		/* ignore WRT_STAT_RECD interrupt when we get HPD CHANGE */
		if (cbus_intr2 & WRT_STAT_RECD && intr1 == 0)
			cbus_handle_wrt_stat_recd(sii9234);

		if (cbus_intr2 & SET_INT_RECD)
			cbus_handle_set_int_recd(sii9234);
	}
#endif /* CONFIG_SII9234_CBUS */

 err_exit:
	ret = mhl_tx_write_reg(sii9234, MHL_TX_INTR1_REG, intr1);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d write MHL_TX_INTR1_REG\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = mhl_tx_write_reg(sii9234, MHL_TX_INTR4_REG, intr4);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d write MHL_TX_INTR4_REG\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}
 #ifdef	CONFIG_SII9234_CBUS
	ret = cbus_write_reg(sii9234, CBUS_INT_STATUS_1_REG, cbus_intr1);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d write CBUS_INT_STATUS_1_REG\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}

	ret = cbus_write_reg(sii9234, CBUS_INT_STATUS_2_REG, cbus_intr2);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d write CBUS_INT_STATUS_2_REG\n",
		       __func__, __LINE__);
		goto i2c_error_exit;
	}
#endif

i2c_error_exit:
	mutex_unlock(&sii9234->lock);

	pr_info("sii9234: wake_up\n");
	wake_up(&sii9234->wq);

	if (cbus_resp_callback)
		cbus_resp_callback(sii9234);

	if (mhl_poweroff) {
		printk(KERN_INFO	"%s() mhl_poweroff\n", __func__);
		mhl_onoff_ex(sii9234, false);
	}
/* need to more test */
/*
	if (retry_detection) {
		printk(KERN_INFO	"%s() retry_detection\n", __func__);

		sii9234_mhl_detection_sched();

	}
*/
	return IRQ_HANDLED;
}

#ifdef CONFIG_SS_FACTORY
#define SII_ID 0x92
static ssize_t sysfs_check_mhl_command(struct class *class,
	struct class_attribute *attr, char *buf)
{
	int size;
	u8 sii_id = 0;
	struct sii9234_data *sii9234 = dev_get_drvdata(sii9244_mhldev);

	if (sii9234->pdata->hw_onoff)
		sii9234->pdata->hw_onoff(1);

	if (sii9234->pdata->hw_reset)
		sii9234->pdata->hw_reset();

	mhl_tx_read_reg(sii9234, MHL_TX_IDH_REG, &sii_id);
	pr_info("sel_show sii_id: %X\n", sii_id);

	if (sii9234->pdata->hw_onoff)
		sii9234->pdata->hw_onoff(1);

	size = snprintf(buf, 10, "%d\n", sii_id == SII_ID ? 1 : 0);

	return size;

}

static CLASS_ATTR(test_result, 0664 , sysfs_check_mhl_command, NULL);
#endif /*config_ss_factory*/

static ssize_t sysfs_ddc_i2c_num_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	int size;
	struct sii9234_data *sii9234 = dev_get_drvdata(sii9244_mhldev);

	pr_info("ddc_i2c_num : %d\n", sii9234->pdata->ddc_i2c_num);
	size = sprintf(buf, "DDC %d\n", sii9234->pdata->ddc_i2c_num);

	return size;
}

static CLASS_ATTR(ddc_i2c_num, 0664 , sysfs_ddc_i2c_num_show, NULL);

static ssize_t sysfs_mhl_read_reg_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	pr_info("sysfs_mhl_read_reg_show\n");
	return sprintf(buf, "sysfs_mhl_read_reg_show\n");
}

static ssize_t sysfs_mhl_read_reg_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	struct sii9234_data *sii9234 = dev_get_drvdata(sii9244_mhldev);
	enum page_num pn;
	unsigned int offset;
	int ret;
	u8 value = 0;
	char dest[10];
	char *buffer = (char *)buf;

	if (size > 10) {
		pr_info("Error : Unsupported format\n");
		return size;
	}

	if (strnicmp(buf, "mhl", 3) == 0) {
		printk(KERN_INFO	"%s() - mhl start\n", __func__);
		sii9234_mhl_detection_sched();
	}

	strcpy(dest, strsep(&buffer, ":"));
	ret = kstrtouint(dest, 0, &offset);
	if (ret != 0) {
		pr_info("Error : Page number\n");
		return size;
	}
	pn = (enum page_num)offset;

	strcpy(dest, buffer);
	ret = kstrtouint(dest, 0, &offset);
	if (ret || offset > 0xff) {
		pr_info("Error : Offset number\n");
		return size;
	}

	switch (pn) {
	case PAGE0:
		mhl_tx_read_reg(sii9234, offset, &value);
		break;
	case PAGE1:
		tpi_read_reg(sii9234, offset, &value);
		break;
	case PAGE2:
		hdmi_rx_read_reg(sii9234, offset, &value);
		break;
	case PAGE3:
		cbus_read_reg(sii9234, offset, &value);
		break;
	default:
		pr_info("\nError : Out of the page number range\n");
		return size;
	}
	pr_info("[sii9234] MHL register Page%d:0x%02x = 0x%02x\n", pn, offset,
		value);

	return size;
}

static DEVICE_ATTR(mhl_read_reg, S_IRUGO | S_IWUSR,
		   sysfs_mhl_read_reg_show, sysfs_mhl_read_reg_store);

#ifdef CONFIG_MHL_DEBUG
module_param_named(mhl_dbg_flag, mhl_dbg_flag, uint, 0644);
#endif

static int __devinit sii9234_mhl_tx_i2c_probe(struct i2c_client *client,
					      const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct sii9234_data *sii9234;
#ifdef CONFIG_SII9234_RCP
	struct input_dev *input;
#endif
	int ret;
#ifdef CONFIG_SS_FACTORY
	struct class *sec_mhl;
#endif

#ifdef CONFIG_MHL_DEBUG
	mhl_dbg_flag = 1;
#endif

	printk(KERN_INFO "%s()\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		printk(KERN_ERR "[ERROR] %s() i2c no function\n", __func__);
		return -EIO;
	}

	sii9234 = kzalloc(sizeof(struct sii9234_data), GFP_KERNEL);
	if (!sii9234) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	atomic_set(&sii9234->is_callback_scheduled, false);
#ifdef CONFIG_SII9234_RCP
	input = input_allocate_device();
	if (!input) {
		dev_err(&client->dev, "failed to allocate input device.\n");
		ret = -ENOMEM;
		goto err_exit0;
	}
#endif

	sii9234->pdata = client->dev.platform_data;
	if (!sii9234->pdata) {
		ret = -EINVAL;
		printk(KERN_ERR "[ERROR] %s() platform_data is NULL\n",
		       __func__);
		goto err_exit1;
	}
	sii9234->pdata->mhl_tx_client = client;

	init_waitqueue_head(&sii9234->wq);
	mutex_init(&sii9234->lock);
	mutex_init(&sii9234->cbus_lock);

	i2c_set_clientdata(client, sii9234);
	sii9244_mhldev = &client->dev;

	sii9234->pdata->init();

	ret = device_create_file(&(client->dev), &dev_attr_mhl_read_reg);
	if (ret < 0)
		dev_err(&(client->dev), "failed to add sysfs entries\n");

	sii9234_wq = create_singlethread_workqueue("sii9234_wq");
	if (!sii9234_wq) {
		printk(KERN_ERR
		       "[ERROR] %s() workqueue create fail\n", __func__);
		ret = -ENOMEM;
		goto err_exit1b;
	}
	INIT_WORK(&(sii9234->sii9234_int_work), sii9234_detection_callback);

	pr_info("create mhl sysfile\n");

	sec_mhl = class_create(THIS_MODULE, "mhl");
	if (IS_ERR(sec_mhl)) {
		pr_err("Failed to create class(sec_mhl)!\n");
		ret = -ENOMEM;
		goto err_exit1c;
	}

	ret = class_create_file(sec_mhl, &class_attr_ddc_i2c_num);
	if (ret) {
		pr_err("Failed to create device file in sysfs entries!\n");
		ret = -ENOMEM;
		goto err_exit2;
	}
#ifdef CONFIG_SS_FACTORY
	ret = class_create_file(sec_mhl, &class_attr_test_result);
	if (ret) {
		pr_err("Failed to create device file in sysfs entries!\n");
		ret = -ENOMEM;
		goto err_exit2a;
	}
#endif

	sii9234->cbus_pkt.command = CBUS_IDLE;
	sii9234->cbus_pkt.offset = DEVCAP_DEV_STATE;

#ifdef CONFIG_SII9234_RCP
	/* indicate that we generate key events */
	set_bit(EV_KEY, input->evbit);
	bitmap_fill(input->keybit, KEY_MAX);

	sii9234->input_dev = input;
	input_set_drvdata(input, sii9234);
	input->name = "sii9234_rcp";
	input->id.bustype = BUS_I2C;

	ret = input_register_device(input);
	if (ret < 0) {
		dev_err(&client->dev, "fail to register input device\n");
		goto err_exit2b;
	}
#endif

	client->irq = gpio_to_irq(GPIO_MHL_INT);
	ret = request_threaded_irq(client->irq, NULL, sii9234_irq_thread,
				   IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
				   "sii9234", sii9234);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s() register thread fail\n",
		       __func__);
		ret = -ENOMEM;
		goto err_exit2c;
	}
	disable_irq(client->irq);

	return 0;

 err_exit2c:
#ifdef CONFIG_SII9234_RCP
	input_unregister_device(input);
#endif

 err_exit2b:
#ifdef CONFIG_SS_FACTORY
	class_remove_file(sec_mhl, &class_attr_test_result);
#endif

 err_exit2a:
	class_remove_file(sec_mhl, &class_attr_ddc_i2c_num);

 err_exit2:
	class_destroy(sec_mhl);

 err_exit1c:
	destroy_workqueue(sii9234_wq);

 err_exit1b:
	device_remove_file(&(client->dev), &dev_attr_mhl_read_reg);

	mutex_destroy(&sii9234->lock);
	mutex_destroy(&sii9234->cbus_lock);

 err_exit1:
#ifdef CONFIG_SII9234_RCP
	input_free_device(input);
#endif
 err_exit0:
	kfree(sii9234);

	return ret;
}

static int __devinit sii9234_tpi_i2c_probe(struct i2c_client *client,
					   const struct i2c_device_id *id)
{
	struct sii9234_platform_data *pdata = client->dev.platform_data;
	if (!pdata) {
		printk(KERN_ERR "[ERROR] %s() fail\n", __func__);
		return -EINVAL;
	}
	pdata->tpi_client = client;
	return 0;
}

static int __devinit sii9234_hdmi_rx_i2c_probe(struct i2c_client *client,
					       const struct i2c_device_id *id)
{
	struct sii9234_platform_data *pdata = client->dev.platform_data;
	if (!pdata) {
		printk(KERN_ERR "[ERROR] %s() fail\n", __func__);
		return -EINVAL;
	}

	pdata->hdmi_rx_client = client;
	return 0;
}

static int __devinit sii9234_cbus_i2c_probe(struct i2c_client *client,
					    const struct i2c_device_id *id)
{
	struct sii9234_platform_data *pdata = client->dev.platform_data;
	if (!pdata) {
		printk(KERN_ERR "[ERROR] %s() fail\n", __func__);
		return -EINVAL;
	}
	pdata->cbus_client = client;
	return 0;
}

static int __devexit sii9234_mhl_tx_remove(struct i2c_client *client)
{
	return 0;
}

static int __devexit sii9234_tpi_remove(struct i2c_client *client)
{
	return 0;
}

static int __devexit sii9234_hdmi_rx_remove(struct i2c_client *client)
{
	return 0;
}

static int __devexit sii9234_cbus_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id sii9234_mhl_tx_id[] = {
	{"sii9234_mhl_tx", 0},
	{}
};

static const struct i2c_device_id sii9234_tpi_id[] = {
	{"sii9234_tpi", 0},
	{}
};

static const struct i2c_device_id sii9234_hdmi_rx_id[] = {
	{"sii9234_hdmi_rx", 0},
	{}
};

static const struct i2c_device_id sii9234_cbus_id[] = {
	{"sii9234_cbus", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sii9234_mhl_tx_id);
MODULE_DEVICE_TABLE(i2c, sii9234_tpi_id);
MODULE_DEVICE_TABLE(i2c, sii9234_hdmi_rx_id);
MODULE_DEVICE_TABLE(i2c, sii9234_cbus_id);

static struct i2c_driver sii9234_mhl_tx_i2c_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "sii9234_mhl_tx",
		   },
	.id_table = sii9234_mhl_tx_id,
	.probe = sii9234_mhl_tx_i2c_probe,
	.remove = __devexit_p(sii9234_mhl_tx_remove),
	.command = NULL,
};

static struct i2c_driver sii9234_tpi_i2c_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "sii9234_tpi",
		   },
	.id_table = sii9234_tpi_id,
	.probe = sii9234_tpi_i2c_probe,
	.remove = __devexit_p(sii9234_tpi_remove),
};

static struct i2c_driver sii9234_hdmi_rx_i2c_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "sii9234_hdmi_rx",
		   },
	.id_table = sii9234_hdmi_rx_id,
	.probe = sii9234_hdmi_rx_i2c_probe,
	.remove = __devexit_p(sii9234_hdmi_rx_remove),
};

static struct i2c_driver sii9234_cbus_i2c_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "sii9234_cbus",
		   },
	.id_table = sii9234_cbus_id,
	.probe = sii9234_cbus_i2c_probe,
	.remove = __devexit_p(sii9234_cbus_remove),
};

static int __init sii9234_init(void)
{
	int ret;

	ret = i2c_add_driver(&sii9234_mhl_tx_i2c_driver);
	if (ret < 0) {
		printk(KERN_ERR "[ERROR] %s():%d failed !\n", __func__,
		       __LINE__);
		return ret;
	}

	ret = i2c_add_driver(&sii9234_tpi_i2c_driver);
	if (ret < 0)
		goto err_exit1;

	ret = i2c_add_driver(&sii9234_hdmi_rx_i2c_driver);
	if (ret < 0)
		goto err_exit2;

	ret = i2c_add_driver(&sii9234_cbus_i2c_driver);
	if (ret < 0)
		goto err_exit3;

	return 0;

 err_exit3:
	i2c_del_driver(&sii9234_hdmi_rx_i2c_driver);
 err_exit2:
	i2c_del_driver(&sii9234_tpi_i2c_driver);
 err_exit1:
	i2c_del_driver(&sii9234_mhl_tx_i2c_driver);
	pr_err("i2c_add_driver fail\n");
	return ret;
}

static void __exit sii9234_exit(void)
{
	i2c_del_driver(&sii9234_cbus_i2c_driver);
	i2c_del_driver(&sii9234_hdmi_rx_i2c_driver);
	i2c_del_driver(&sii9234_tpi_i2c_driver);
	i2c_del_driver(&sii9234_mhl_tx_i2c_driver);
}

module_init(sii9234_init);
module_exit(sii9234_exit);
