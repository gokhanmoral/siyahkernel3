/*
 * Copyright (c) 2004-2011 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/ip.h>
#include "core.h"
#include "wmi_btcoex.h"
#include "debug.h"

static inline struct sk_buff *ath6kl_wmi_btcoex_get_new_buf(u32 size)
{
	struct sk_buff *skb;

	skb = ath6kl_buf_alloc(size);
	if (!skb)
		return NULL;

	skb_put(skb, size);
	if (size)
		memset(skb->data, 0, size);

	return skb;
}

static int ath6kl_get_wmi_cmd(int nl_cmd)
{
	int wmi_cmd = 0;
	switch (nl_cmd) {
	case NL80211_WMI_SET_BT_STATUS:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT status\n");
		wmi_cmd = WMI_SET_BT_STATUS_CMDID;
		break;

	case NL80211_WMI_SET_BT_PARAMS:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT params\n");
		wmi_cmd = WMI_SET_BT_PARAMS_CMDID;
		break;

	case NL80211_WMI_SET_BT_FT_ANT:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT FT antenna\n");
		wmi_cmd = WMI_SET_BTCOEX_FE_ANT_CMDID;
		break;

	case NL80211_WMI_SET_COLOCATED_BT_DEV:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT collocated dev\n");
		wmi_cmd = WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMDID;
		break;

	case NL80211_WMI_SET_BT_INQUIRY_PAGE_CONFIG:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT inquiry page\n");
		wmi_cmd = WMI_SET_BTCOEX_BTINQUIRY_PAGE_CONFIG_CMDID;
		break;

	case NL80211_WMI_SET_BT_SCO_CONFIG:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT sco config\n");
		wmi_cmd = WMI_SET_BTCOEX_SCO_CONFIG_CMDID;
		break;

	case NL80211_WMI_SET_BT_A2DP_CONFIG:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT a2dp config\n");
		wmi_cmd = WMI_SET_BTCOEX_A2DP_CONFIG_CMDID;
		break;

	case NL80211_WMI_SET_BT_ACLCOEX_CONFIG:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT acl config\n");
		wmi_cmd = WMI_SET_BTCOEX_ACLCOEX_CONFIG_CMDID;
		break;

	case NL80211_WMI_SET_BT_DEBUG:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT bt debug\n");
		wmi_cmd = WMI_SET_BTCOEX_DEBUG_CMDID;
		break;

	case NL80211_WMI_SET_BT_OPSTATUS:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Set BT op status\n");
		wmi_cmd = WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMDID;
		break;

	case NL80211_WMI_GET_BT_CONFIG:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Get BT config\n");
		wmi_cmd = WMI_GET_BTCOEX_CONFIG_CMDID;
		break;

	case NL80211_WMI_GET_BT_STATS:
		ath6kl_dbg(ATH6KL_DBG_WMI, "Get BT status\n");
		wmi_cmd = WMI_GET_BTCOEX_STATS_CMDID;
		break;
	}
	return wmi_cmd;
}

int ath6kl_wmi_send_btcoex_cmd(struct wmi *wmi,
			u8 *buf, int len)
{
	struct sk_buff *skb;
	u32 nl_cmd;
	int wmi_cmd;

	nl_cmd = *(u32 *)buf;
	buf += sizeof(u32);
	len -= sizeof(u32);
	wmi_cmd = ath6kl_get_wmi_cmd(nl_cmd);
	if (wmi_cmd == 0)
		return -ENOMEM;

	skb = ath6kl_wmi_btcoex_get_new_buf(len);
	if (!skb)
		return -ENOMEM;

	memcpy(skb->data, buf, len);

	return ath6kl_wmi_cmd_send(wmi, 0, skb,
			(enum wmi_cmd_id)wmi_cmd,
			NO_SYNC_WMIFLAG);
}
