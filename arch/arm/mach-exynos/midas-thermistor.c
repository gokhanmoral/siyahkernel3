/*
 * midas-thermistor.c - thermistor of MIDAS Project
 *
 * Copyright (C) 2011 Samsung Electrnoics
 * SangYoung Son <hello.son@samsung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <mach/midas-thermistor.h>
#ifdef CONFIG_SEC_THERMISTOR
#include <mach/sec_thermistor.h>
#endif

#ifdef CONFIG_S3C_ADC
#if defined(CONFIG_MACH_M0)
static struct adc_table_data ap_adc_temper_table_battery[] = {
	{  165,	 800 },
	{  171,	 790 },
	{  177,	 780 },
	{  183,	 770 },
	{  189,	 760 },
	{  196,	 750 },
	{  202,	 740 },
	{  208,	 730 },
	{  214,	 720 },
	{  220,	 710 },
	{  227,	 700 },
	{  237,	 690 },
	{  247,	 680 },
	{  258,	 670 },
	{  269,	 660 },
	{  281,	 650 },
	{  296,	 640 },
	{  311,	 630 },
	{  326,	 620 },
	{  341,	 610 },
	{  356,	 600 },
	{  370,	 590 },
	{  384,	 580 },
	{  398,	 570 },
	{  412,	 560 },
	{  427,	 550 },
	{  443,	 540 },
	{  457,	 530 },
	{  471,	 520 },
	{  485,	 510 },
	{  498,	 500 },
	{  507,	 490 },
	{  516,	 480 },
	{  525,	 470 },
	{  535,	 460 },
	{  544,	 450 },
	{  553,	 440 },
	{  562,	 430 },
	{  579,	 420 },
	{  596,	 410 },
	{  613,	 400 },
	{  630,	 390 },
	{  648,	 380 },
	{  665,	 370 },
	{  684,	 360 },
	{  702,	 350 },
	{  726,	 340 },
	{  750,	 330 },
	{  774,	 320 },
	{  798,	 310 },
	{  821,	 300 },
	{  844,	 290 },
	{  867,	 280 },
	{  891,	 270 },
	{  914,	 260 },
	{  937,	 250 },
	{  960,	 240 },
	{  983,	 230 },
	{ 1007,	 220 },
	{ 1030,	 210 },
	{ 1054,	 200 },
	{ 1083,	 190 },
	{ 1113,	 180 },
	{ 1143,	 170 },
	{ 1173,	 160 },
	{ 1202,	 150 },
	{ 1232,	 140 },
	{ 1262,	 130 },
	{ 1291,	 120 },
	{ 1321,	 110 },
	{ 1351,	 100 },
	{ 1357,	  90 },
	{ 1363,	  80 },
	{ 1369,	  70 },
	{ 1375,	  60 },
	{ 1382,	  50 },
	{ 1402,	  40 },
	{ 1422,	  30 },
	{ 1442,	  20 },
	{ 1462,	  10 },
	{ 1482,	   0 },
	{ 1519,	 -10 },
	{ 1528,	 -20 },
	{ 1546,	 -30 },
	{ 1563,	 -40 },
	{ 1587,	 -50 },
	{ 1601,	 -60 },
	{ 1614,	 -70 },
	{ 1625,  -80 },
	{ 1641,  -90 },
	{ 1663, -100 },
	{ 1678, -110 },
	{ 1693, -120 },
	{ 1705, -130 },
	{ 1720, -140 },
	{ 1736, -150 },
	{ 1751, -160 },
	{ 1767, -170 },
	{ 1782, -180 },
	{ 1798, -190 },
	{ 1815, -200 },
};
#elif defined(CONFIG_MACH_C1)
static struct adc_table_data ap_adc_temper_table_battery[] = {
	{ 305,  650 },
	{ 566,  430 },
	{ 1494,   0 },
	{ 1571, -50 },
};
#elif defined(CONFIG_MACH_S2PLUS)
static struct adc_table_data ap_adc_temper_table_battery[] = {
	{ 305,  650 },
	{ 566,  430 },
	{ 1494,   0 },
	{ 1571, -50 },
};
#else	/* sample */
static struct adc_table_data ap_adc_temper_table_battery[] = {
	{ 305,  650 },
	{ 566,  430 },
	{ 1494,   0 },
	{ 1571, -50 },
};
#endif

int convert_adc(int adc_data, int channel)
{
	int adc_value;
	int low, mid, high;
	struct adc_table_data *temper_table = NULL;
	pr_debug("%s\n", __func__);

	low = mid = high = 0;
	switch (channel) {
	case 1:
		temper_table = ap_adc_temper_table_battery;
		high = ARRAY_SIZE(ap_adc_temper_table_battery) - 1;
		break;
	case 2:
		temper_table = ap_adc_temper_table_battery;
		high = ARRAY_SIZE(ap_adc_temper_table_battery) - 1;
		break;
	default:
		pr_info("%s: not exist temper table for ch(%d)\n", __func__,
							channel);
		return -EINVAL;
		break;
	}

	/* Out of table range */
	if (adc_data <= temper_table[low].adc) {
		adc_value = temper_table[low].value;
		return adc_value;
	} else if (adc_data >= temper_table[high].adc) {
		adc_value = temper_table[high].value;
		return adc_value;
	}

	while (low <= high) {
		mid = (low + high) / 2;
		if (temper_table[mid].adc > adc_data)
			high = mid - 1;
		else if (temper_table[mid].adc < adc_data)
			low = mid + 1;
		else
			break;
	}
	adc_value = temper_table[mid].value;

	/* high resolution */
	if (adc_data < temper_table[mid].adc)
		adc_value = temper_table[mid].value +
			((temper_table[mid-1].value - temper_table[mid].value) *
			(temper_table[mid].adc - adc_data) /
			(temper_table[mid].adc - temper_table[mid-1].adc));
	else
		adc_value = temper_table[mid].value -
			((temper_table[mid].value - temper_table[mid+1].value) *
			(adc_data - temper_table[mid].adc) /
			(temper_table[mid+1].adc - temper_table[mid].adc));

	pr_debug("%s: adc data(%d), adc value(%d)\n", __func__,
					adc_data, adc_value);
	return adc_value;

}
#endif

#ifdef CONFIG_SEC_THERMISTOR
static struct sec_therm_adc_table temper_table_ap[] = {
	{  165,	 800 },
	{  171,	 790 },
	{  177,	 780 },
	{  183,	 770 },
	{  189,	 760 },
	{  196,	 750 },
	{  202,	 740 },
	{  208,	 730 },
	{  214,	 720 },
	{  220,	 710 },
	{  227,	 700 },
	{  237,	 690 },
	{  247,	 680 },
	{  258,	 670 },
	{  269,	 660 },
	{  281,	 650 },
	{  296,	 640 },
	{  311,	 630 },
	{  326,	 620 },
	{  341,	 610 },
	{  356,	 600 },
	{  370,	 590 },
	{  384,	 580 },
	{  398,	 570 },
	{  412,	 560 },
	{  427,	 550 },
	{  443,	 540 },
	{  457,	 530 },
	{  471,	 520 },
	{  485,	 510 },
	{  498,	 500 },
	{  507,	 490 },
	{  516,	 480 },
	{  525,	 470 },
	{  535,	 460 },
	{  544,	 450 },
	{  553,	 440 },
	{  562,	 430 },
	{  579,	 420 },
	{  596,	 410 },
	{  613,	 400 },
	{  630,	 390 },
	{  648,	 380 },
	{  665,	 370 },
	{  684,	 360 },
	{  702,	 350 },
	{  726,	 340 },
	{  750,	 330 },
	{  774,	 320 },
	{  798,	 310 },
	{  821,	 300 },
	{  844,	 290 },
	{  867,	 280 },
	{  891,	 270 },
	{  914,	 260 },
	{  937,	 250 },
	{  960,	 240 },
	{  983,	 230 },
	{ 1007,	 220 },
	{ 1030,	 210 },
	{ 1054,	 200 },
	{ 1083,	 190 },
	{ 1113,	 180 },
	{ 1143,	 170 },
	{ 1173,	 160 },
	{ 1202,	 150 },
	{ 1232,	 140 },
	{ 1262,	 130 },
	{ 1291,	 120 },
	{ 1321,	 110 },
	{ 1351,	 100 },
	{ 1357,	  90 },
	{ 1363,	  80 },
	{ 1369,	  70 },
	{ 1375,	  60 },
	{ 1382,	  50 },
	{ 1402,	  40 },
	{ 1422,	  30 },
	{ 1442,	  20 },
	{ 1462,	  10 },
	{ 1482,	   0 },
	{ 1519,	 -10 },
	{ 1528,	 -20 },
	{ 1546,	 -30 },
	{ 1563,	 -40 },
	{ 1587,	 -50 },
	{ 1601,	 -60 },
	{ 1614,	 -70 },
	{ 1625,  -80 },
	{ 1641,  -90 },
	{ 1663, -100 },
	{ 1678, -110 },
	{ 1693, -120 },
	{ 1705, -130 },
	{ 1720, -140 },
	{ 1736, -150 },
	{ 1751, -160 },
	{ 1767, -170 },
	{ 1782, -180 },
	{ 1798, -190 },
	{ 1815, -200 },
};

static struct sec_therm_platform_data sec_therm_pdata = {
	.adc_channel	= 1,
	.adc_arr_size	= ARRAY_SIZE(temper_table_ap),
	.adc_table	= temper_table_ap,
	.polling_interval = 30 * 1000, /* msecs */
};

struct platform_device sec_device_thermistor = {
	.name = "sec-thermistor",
	.id = -1,
	.dev.platform_data = &sec_therm_pdata,
};
#endif

#ifdef CONFIG_STMPE811_ADC
/* temperature table for ADC ch7 */
static struct adc_table_data temper_table_battery[] = {
	{	1856, -20	},
	{	1846, -19	},
	{	1835, -18	},
	{	1823, -17	},
	{	1811, -16	},
	{	1799, -15	},
	{	1786, -14	},
	{	1773, -13	},
	{	1759, -12	},
	{	1744, -11	},
	{	1730, -10	},
	{	1714, -9	},
	{	1699, -8	},
	{	1683, -7	},
	{	1666, -6	},
	{	1649, -5	},
	{	1631, -4	},
	{	1613, -3	},
	{	1595, -2	},
	{	1576, -1	},
	{	1556, 0		},
	{	1536, 1		},
	{	1516, 2		},
	{	1496, 3		},
	{	1475, 4		},
	{	1454, 5		},
	{	1432, 6		},
	{	1410, 7		},
	{	1388, 8		},
	{	1366, 9		},
	{	1343, 10	},
	{	1320, 11	},
	{	1297, 12	},
	{	1274, 13	},
	{	1250, 14	},
	{	1227, 15	},
	{	1204, 16	},
	{	1180, 17	},
	{	1156, 18	},
	{	1133, 19	},
	{	1109, 20	},
	{	1086, 21	},
	{	1062, 22	},
	{	1039, 23	},
	{	1015, 24	},
	{	992,  25	},
	{	969,  26	},
	{	947,  27	},
	{	924,  28	},
	{	902,  29	},
	{	880,  30	},
	{	858,  31	},
	{	836,  32	},
	{	815,  33	},
	{	794,  34	},
	{	773,  35	},
	{	753,  36	},
	{	733,  37	},
	{	713,  38	},
	{	694,  39	},
	{	675,  40	},
	{	657,  41	},
	{	638,  42	},
	{	621,  43	},
	{	603,  44	},
	{	586,  45	},
	{	569,  46	},
	{	553,  47	},
	{	537,  48	},
	{	522,  49	},
	{	507,  50	},
	{	492,  51	},
	{	477,  52	},
	{	463,  53	},
	{	450,  54	},
	{	436,  55	},
	{	423,  56	},
	{	411,  57	},
	{	399,  58	},
	{	387,  59	},
	{	375,  60	},
	{	364,  61	},
	{	353,  62	},
	{	342,  63	},
	{	332,  64	},
	{	322,  65	},
	{	312,  66	},
	{	303,  67	},
	{	294,  68	},
	{	285,  69	},
	{	276,  70	},
	{	268,  71	},
	{	260,  72	},
	{	252,  73	},
	{	244,  74	},
	{	237,  75	},
	{	230,  76	},
	{	223,  77	},
	{	216,  78	},
	{	210,  79	},
	{	204,  80	},
};

struct stmpe811_platform_data stmpe811_pdata = {
	.adc_table_ch4 = temper_table_battery,
	.table_size_ch4 = ARRAY_SIZE(temper_table_battery),
	.adc_table_ch7 = temper_table_battery,
	.table_size_ch7 = ARRAY_SIZE(temper_table_battery),

	.irq_gpio = GPIO_ADC_INT,
};
#endif

