#ifndef __S6E8AA0_PARAM_H__
#define __S6E8AA0_PARAM_H__

#define GAMMA_PARAM_SIZE	26
#define ELVSS_PARAM_SIZE	3


enum {
	ELVSS_MIN = 0,
	ELVSS_1,
	ELVSS_2,
	ELVSS_MAX,
	ELVSS_STATUS_MAX,
};

enum {
	GAMMA_30CD,
	GAMMA_40CD,
	GAMMA_50CD,
	GAMMA_60CD,
	GAMMA_70CD,
	GAMMA_80CD,
	GAMMA_90CD,
	GAMMA_100CD,
	GAMMA_110CD,
	GAMMA_120CD,
	GAMMA_130CD,
	GAMMA_140CD,
	GAMMA_150CD,
	GAMMA_160CD,
	GAMMA_170CD,
	GAMMA_180CD,
	GAMMA_190CD,
	GAMMA_200CD,
	GAMMA_210CD,
	GAMMA_220CD,
	GAMMA_230CD,
	GAMMA_240CD,
	GAMMA_250CD,
	GAMMA_260CD,
	GAMMA_270CD,
	GAMMA_280CD,
	GAMMA_290CD,	/* 300CD */
	GAMMA_MAX
};

static const unsigned char SEQ_APPLY_LEVEL_2_KEY[] = {
	0xFC,
	0x5A, 0x5A
};

static const unsigned char SEQ_APPLY_LEVEL_2[] = {
	0xF0,
	0x5A, 0x5A
};

static const unsigned char SEQ_APPLY_MTP_KEY_ENABLE[] = {
	0xF1,
	0x5A, 0x5A
};

static const unsigned char SEQ_SLEEP_OUT[] = {
	0x11,
	0x00, 0x00
};

static const unsigned char SEQ_DISPLAY_CONDITION_SET[] = {
	0xF2,
	0x80, 0x03, 0x0D
};

static const unsigned char SEQ_GAMMA_UPDATE[] = {
	0xF7, 0x03,
	0x00
};

static const unsigned char SEQ_ELVSS_CONTROL[] = {
	0xB1,
	0x04, 0x00
};

static const unsigned char SEQ_DISPLAY_ON[] = {
	0x29,
	0x00, 0x00
};

static const unsigned char SEQ_DISPLAY_OFF[] = {
	0x28,
	0x00, 0x00
};

static const unsigned char SEQ_STANDBY_ON[] = {
	0x01,
	0x00, 0x00
};

static const unsigned char SEQ_ACL_ON[] = {
	0xC0, 0x01,
	0x00
};

static const unsigned char SEQ_ACL_OFF[] = {
	0xC0, 0x00,
	0x00
};

static const unsigned char SEQ_ELVSS_32[] = {
	0xB1,
	0x04, 0x9F
};

static const unsigned char SEQ_ELVSS_34[] = {
	0xB1,
	0x04, 0x9D
};

static const unsigned char SEQ_ELVSS_38[] = {
	0xB1,
	0x04, 0x99
};

static const unsigned char SEQ_ELVSS_47[] = {
	0xB1,
	0x04, 0x90
};

static const unsigned char *ELVSS_TABLE[] = {
	SEQ_ELVSS_32,
	SEQ_ELVSS_34,
	SEQ_ELVSS_38,
	SEQ_ELVSS_47,
};

#endif /* __S6E8AA0_PARAM_H__ */
