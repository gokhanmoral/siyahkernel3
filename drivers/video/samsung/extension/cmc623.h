/*
 * CMC623 Setting table for SLP7 Machine.
 *
 * Author: InKi Dae  <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "mdnie.h"

#if 1
#define SCR_C8_TN	0x0000	/* kb R	SCR */
#define SCR_C9_TN	0x3000	/* gc R */
#define SCR_CA_TN	0xffff	/* rm R */
#define SCR_CB_TN	0xffd0	/* yw R */
#define SCR_CC_TN	0x0040	/* kb G */
#define SCR_CD_TN	0xffff	/* gc G */
#define SCR_CE_TN	0x0000	/* rm G */
#define SCR_CF_MSB16_TN	0xf000	/* yw G */
#define SCR_D0_TN	0x00a0	/* kb B */
#define SCR_D1_TN	0x00ff	/* gc B */
#define SCR_D2_TN	0x00ff	/* rm B */
#define SCR_D3_TN	0x00d0	/* yw B */
#define SCR_CC_DMB_TN	0x0040	/* kb G */

static const unsigned short cmc623_video_tn[] = {
	0x0000, 0x0000,   /*BANK 0*/
	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0008, 0x0020,
	0x0030, 0x0000,   /*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xaf00,   /*SCR RrCr*/
	0x00e2, 0x00b7,   /*SCR RgCg*/
	0x00e3, 0x00bc,   /*SCR RbCb*/
	0x00e4, 0x00af,   /*SCR GrMr*/
	0x00e5, 0xb700,   /*SCR GgMg*/
	0x00e6, 0x00bc,   /*SCR GbMb*/
	0x00e7, 0x00af,   /*SCR BrYr*/
	0x00e8, 0x00b7,   /*SCR BgYg*/
	0x00e9, 0xbc00,   /*SCR BbYb*/
	0x00ea, 0x00af,   /*SCR KrWr*/
	0x00eb, 0x00b7,   /*SCR KgWg*/
	0x00ec, 0x00bc,   /*SCR KbWb*/
	0x00ff, 0x0000,   /*Mask Release*/
	END_CMD, 0x0000,
#if 0

	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0060,		/* SCR LABC */
	0x002c, 0x0fff,		/* DNR bypass 0x003C */
	0x002d, 0x1900,		/* DNR bypass 0x0a08 */
	0x002e, 0x0000,		/* DNR bypass 0x1010 */
	0x002f, 0x00ff,		/* DNR bypass 0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
#endif
};

static const unsigned short cmc623_video_cabc_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0070,		/* SCR LABC CABC */
	0x002c, 0x0fff,		/* DNR bypass 0x003C */
	0x002d, 0x1900,		/* DNR bypass 0x0a08 */
	0x002e, 0x0000,		/* DNR bypass 0x1010 */
	0x002f, 0x00ff,		/* DNR bypass 0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x0072, 0x0000,		/* CABC Dgain */
	0x0073, 0x0000,
	0x0074, 0x0000,
	0x0075, 0x0000,
	0x007C, 0x0002,		/* Dynamic LCD */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_camera_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0060,		/* SCR LABC */
	0x002c, 0x0fff,		/* DNR bypass 0x003C */
	0x002d, 0x1900,		/* DNR bypass 0x0a08 */
	0x002e, 0x0000,		/* DNR bypass 0x1010 */
	0x002f, 0x00ff,		/* DNR bypass 0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_camera_cabc_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0070,		/* SCR LABC CABC */
	0x002c, 0x0fff,		/* DNR bypass {0x003C */
	0x002d, 0x1900,		/* DNR bypass {0x0a08 */
	0x002e, 0x0000,		/* DNR bypass {0x1010 */
	0x002f, 0x00ff,		/* DNR bypass {0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x0072, 0x0000,		/* CABC Dgain */
	0x0073, 0x0000,
	0x0074, 0x0000,
	0x0075, 0x0000,
	0x007C, 0x0002,		/* Dynamic LCD */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_ui_tn[] = {
	0x0000, 0x0000,   /*BANK 0*/
	0x0008, 0x0020,   /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,   /*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xaf00,   /*SCR RrCr*/
	0x00e2, 0x00b7,   /*SCR RgCg*/
	0x00e3, 0x00bc,   /*SCR RbCb*/
	0x00e4, 0x00af,   /*SCR GrMr*/
	0x00e5, 0xb700,   /*SCR GgMg*/
	0x00e6, 0x00bc,   /*SCR GbMb*/
	0x00e7, 0x00af,   /*SCR BrYr*/
	0x00e8, 0x00b7,   /*SCR BgYg*/
	0x00e9, 0xbc00,   /*SCR BbYb*/
	0x00ea, 0x00af,   /*SCR KrWr*/
	0x00eb, 0x00b7,   /*SCR KgWg*/
	0x00ec, 0x00bc,   /*SCR KbWb*/
	0x00ff, 0x0000,   /*Mask Release*/
	END_CMD, 0x0000,
};

static const unsigned short cmc623_ui_cabc_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0070,		/* SCR LABC CABC */
	0x002c, 0x0fff,		/* DNR bypass 0x003C */
	0x002d, 0x1900,		/* DNR bypass 0x0a08 */
	0x002e, 0x0000,		/* DNR bypass 0x1010 */
	0x002f, 0x00ff,		/* DNR bypass 0x0400 */
	0x003A, 0x0009,		/* HDTR (DE D) CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x0072, 0x0000,		/* CABC Dgain */
	0x0073, 0x0000,
	0x0074, 0x0000,
	0x0075, 0x0000,
	0x007C, 0x0002,		/* Dynamic LCD */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_vt_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0060,		/* SCR LABC */
	0x002c, 0x0fff,		/* DNR bypass,dir_th   0x003c */
	0x002d, 0x19ff,		/* DNR dir_num,decont7 0x0a08 */
	0x002e, 0xff16,		/* DNR decont5,mask_th 0x1010 */
	0x002F, 0x0000,		/* DNR block_th        0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_vt_cabc_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0070,		/* SCR LABC CABC */
	0x002c, 0x0fff,		/* DNR bypass,dir_th 0x003c */
	0x002d, 0x19ff,		/* DNR dir_num,decont7 0x0a08 */
	0x002e, 0xff16,		/* DNR decont5,mask_th 0x1010 */
	0x002F, 0x0000,		/* DNR block_th 0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x0072, 0x0000,		/* CABC Dgain */
	0x0073, 0x0000,
	0x0074, 0x0000,
	0x0075, 0x0000,
	0x007C, 0x0002,		/* Dynamic LCD */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_dmb_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0060,		/* SCR LABC */
	0x002c, 0x003c,		/* DNR bypass,dir_th   0x003c */
	0x002d, 0x0a08,		/* DNR dir_num,decont7 0x0a08 */
	0x002e, 0x1010,		/* DNR decont5,mask_th 0x1010 */
	0x002F, 0x0400,		/* DNR block_th        0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_DMB_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_dmb_cabc_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0070,		/* SCR LABC CABC */
	0x002c, 0x003c,		/* DNR bypass,dir_th   0x003c */
	0x002d, 0x0a08,		/* DNR dir_num,decont7 0x0a08 */
	0x002e, 0x1010,		/* DNR decont5,mask_th 0x1010 */
	0x002F, 0x0400,		/* DNR block_th        0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x0072, 0x0000,		/* CABC Dgain */
	0x0073, 0x0000,
	0x0074, 0x0000,
	0x0075, 0x0000,
	0x007C, 0x0002,		/* Dynamic LCD */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_DMB_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_gallery_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0060,		/* SCR LABC */
	0x002c, 0x0fff,		/* DNR bypass 0x003C */
	0x002d, 0x1900,		/* DNR bypass 0x0a08 */
	0x002e, 0x0000,		/* DNR bypass 0x1010 */
	0x002f, 0x00ff,		/* DNR bypass 0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};

static const unsigned short cmc623_gallery_cabc_tn[] = {
	0x0000, 0x0000,		/* BANK 0 */
	0x0001, 0x0070,		/* SCR LABC CABC */
	0x002c, 0x0fff,		/* DNR bypass 0x003C */
	0x002d, 0x1900,		/* DNR bypass 0x0a08 */
	0x002e, 0x0000,		/* DNR bypass 0x1010 */
	0x002f, 0x00ff,		/* DNR bypass 0x0400 */
	0x003A, 0x000D,		/* HDTR DE CS */
	0x003B, 0x03ff,		/* DE SHARPNESS */
	0x003C, 0x0000,		/* NOISE LEVEL */
	0x0042, 0x0000,		/* DE TH (MAX DIFF) */
	0x0072, 0x0000,		/* CABC Dgain */
	0x0073, 0x0000,
	0x0074, 0x0000,
	0x0075, 0x0000,
	0x007C, 0x0002,		/* Dynamic LCD */
	0x00c8, SCR_C8_TN,
	0x00c9, SCR_C9_TN,
	0x00ca, SCR_CA_TN,
	0x00cb, SCR_CB_TN,
	0x00cc, SCR_CC_TN,
	0x00cd, SCR_CD_TN,
	0x00ce, SCR_CE_TN,
	0x00d0, SCR_D0_TN,
	0x00d1, SCR_D1_TN,
	0x00d2, SCR_D2_TN,
	0x00d3, SCR_D3_TN,
	END_CMD, 0x0000,
};
#endif

const struct mdnie_tables cmc623_tables = {
	.tables[CMC623_MODE_UI]		= (unsigned short *)&cmc623_ui_tn,
	.tables[CMC623_MODE_VIDEO]	= (unsigned short *)&cmc623_video_tn,
	.tables[CMC623_MODE_CAMERA]	= (unsigned short *)&cmc623_camera_tn,
	.tables[CMC623_MODE_VT]		= (unsigned short *)&cmc623_vt_tn,
	.tables[CMC623_MODE_DMB]	= (unsigned short *)&cmc623_dmb_tn,
	.tables[CMC623_MODE_GALLERY]	= (unsigned short *)&cmc623_gallery_tn,

	/* not used yet. */
	.cabc_tables[CMC623_MODE_UI]		=
		(unsigned short *)&cmc623_ui_cabc_tn,
	.cabc_tables[CMC623_MODE_VIDEO]		=
		(unsigned short *)&cmc623_video_cabc_tn,
	.cabc_tables[CMC623_MODE_CAMERA]	=
		(unsigned short *)&cmc623_camera_cabc_tn,
	.cabc_tables[CMC623_MODE_VT]		=
		(unsigned short *)&cmc623_vt_cabc_tn,
	.cabc_tables[CMC623_MODE_DMB]		=
		(unsigned short *)&cmc623_dmb_cabc_tn,
	.cabc_tables[CMC623_MODE_GALLERY]	=
		(unsigned short *)&cmc623_gallery_cabc_tn,
};
