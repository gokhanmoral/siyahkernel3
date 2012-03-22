# bcmdhd
DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                     \
        -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP -DBCMFILEIMAGE  \
        -DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST -DBDC -DTOE    \
        -DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG               \
        -DCUSTOMER_HW_SAMSUNG -DOOB_INTR_ONLY                                 \
        -DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
        -DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
        -DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT                             \
        -DEMBEDDED_PLATFORM -DPNO_SUPPORT

# distinguish between the 43xx chip
ifeq ($(CONFIG_BCM4334),m)
DHDCFLAGS += -DHW_OOB -DBCM4334_CHECK_CHIP_REV
DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
DHDCFLAGS += -DPROP_TXSTATUS
endif
ifeq ($(CONFIG_BCM4334),y)
DHDCFLAGS += -DHW_OOB -DBCM4334_CHECK_CHIP_REV
DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
DHDCFLAGS += -DPROP_TXSTATUS
endif

ifeq ($(CONFIG_BCM4330),m)
DHDCFLAGS += -DCONFIG_CONTROL_PM -DPOWER_ON_DELAY_4330
endif

ifeq ($(CONFIG_BCM4330),y)
DHDCFLAGS += -DCONFIG_CONTROL_PM -DPOWER_ON_DELAY_4330
endif

ifeq ($(CONFIG_MACH_U1),y)
DHDCFLAGS += -DUSE_CID_CHECK -DWRITE_MACADDR -DU1_MACADDR
endif

ifeq ($(CONFIG_TARGET_LOCALE_KOR),y)
DHDCFLAGS += -DUSE_CID_CHECK -DRDWR_MACADDR -DU1_MACADDR
DHDCFLAGS += -DROAM_ENABLE -DROAM_API
endif

ifeq ($(CONFIG_MACH_U1_KOR_KT), y)
DHDCFLAGS += -DVLAN_MODE_OFF
DHDCFLAGS += -DKEEP_ALIVE_PACKET_PERIOD_30_SEC
DHDCFLAGS += -DFULL_ROAMING_SCAN_PERIOD_60_SEC
endif

# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
endif

ifneq ($(findstring GlobalConfig, $(wildcard $(srctree)/include/sec_feature/*)),)
DHDCFLAGS += -DUSE_SECFEATURE
endif
ifeq ($(SEC_REGION),CHN)
DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
endif

# For SLP feature
ifeq ($(CONFIG_SLP),y)
DHDCFLAGS += -DSLP_PATH
endif

# For Debug
EXTRA_CFLAGS += $(DHDCFLAGS) -DWL_CFG80211
EXTRA_CFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(src)/src\"

EXTRA_CFLAGS += -I$(src)/src/include/
EXTRA_CFLAGS += -I$(src)/src/dhd/sys/
EXTRA_CFLAGS += -I$(src)/src/dongle/
EXTRA_CFLAGS += -I$(src)/src/bcmsdio/sys/
EXTRA_CFLAGS += -I$(src)/src/wl/sys/
EXTRA_CFLAGS += -I$(src)/src/shared/

EXTRA_LDFLAGS += --strip-debug
KBUILD_CFLAGS += -I$(LINUXDIR)/include -I$(shell pwd)

obj-m   += dhd.o

dhd-y := src/bcmsdio/sys/bcmsdh.o	src/bcmsdio/sys/bcmsdh_linux.o \
	 src/bcmsdio/sys/bcmsdh_sdmmc.o	src/bcmsdio/sys/bcmsdh_sdmmc_linux.o \
	 src/dhd/sys/dhd_bta.o		src/dhd/sys/dhd_cdc.o \
	 src/dhd/sys/dhd_common.o	src/dhd/sys/dhd_custom_gpio.o \
	 src/dhd/sys/dhd_custom_sec.o \
	 src/dhd/sys/dhd_linux.o	src/dhd/sys/dhd_linux_sched.o \
	 src/dhd/sys/dhd_cfg80211.o	src/dhd/sys/dhd_sdio.o \
	 src/shared/aiutils.o		src/shared/bcmevent.o \
	 src/shared/bcmutils.o		src/shared/bcmwifi.o \
	 src/shared/hndpmu.o		src/shared/linux_osl.o \
	 src/shared/sbutils.o		src/shared/siutils.o \
	 src/wl/sys/wl_android.o	src/wl/sys/wl_cfg80211.o \
	 src/wl/sys/wl_cfgp2p.o		src/wl/sys/wldev_common.o \
	 src/wl/sys/wl_linux_mon.o

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -rf *.o *.ko *.mod.c *~ .*.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin \
	src/bcmsdio/sys/*.o \
	src/bcmsdio/sys/*.o.cmd \
	src/bcmsdio/sys/.*.o.cmd \
        src/dhd/sys/*.o \
	src/dhd/sys/*.o.cmd \
        src/dhd/sys/.*.o.cmd \
        src/shared/*.o \
	src/shared/*.o.cmd \
        src/shared/.*.o.cmd \
        src/wl/sys/*.o \
        src/wl/sys/*.o.cmd \
        src/wl/sys/.*.o.cmd

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
