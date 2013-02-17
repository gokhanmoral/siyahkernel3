obj-$(CONFIG_VIDEO_FIMC)	+= fimc/
obj-$(CONFIG_VIDEO_JPEG)	+= jpeg/
obj-$(CONFIG_VIDEO_JPEG_V2X)	+= jpeg_v2x/
obj-$(CONFIG_VIDEO_TVOUT)	+= tvout/
obj-$(CONFIG_VIDEO_MFC5X)	+= mfc5x/

ifeq ($(CONFIG_ARCH_EXYNOS4), y)
obj-$(CONFIG_VIDEO_FIMG2D3X)	+= fimg2d3x-exynos4/
obj-$(CONFIG_VIDEO_FIMG2D4X)	+= fimg2d4x-exynos4/
else
obj-$(CONFIG_VIDEO_FIMG2D3X)	+= fimg2d3x/
obj-$(CONFIG_VIDEO_FIMG2D4X)	+= fimg2d4x/
endif

ifeq ($(CONFIG_MALI_UMP_R3P1_LSI), y)
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= ump_r3p1/
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= mali_r3p1/
else ifeq ($(CONFIG_MACH_U1), y)
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= ump/
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= mali/
else ifeq ($(CONFIG_MACH_SMDKC210), y)
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= ump/
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= mali/
else ifeq ($(CONFIG_MACH_SMDKV310), y)
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= ump/
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= mali/
else
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= ump/
obj-$(CONFIG_VIDEO_MALI400MP_UMP)	+= mali/
endif

obj-$(CONFIG_VIDEO_TSI)		+= tsi/

EXTRA_CFLAGS += -Idrivers/media/video
