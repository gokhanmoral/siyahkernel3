##############################################################################
# COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2011 ALL RIGHTS RESERVED
# 2009.02 - Currently managed by  JongMinKim <jm45.kim@samsung.com> , SungHwanYun <sunghwan.yun@samsung.com>
##############################################################################
# VERSION&DATE	: Version 1.00	2009/02
##############################################################################

MOD_NAME = j4fs

ifneq ($(KERNELRELEASE), )

EXTRA_CFLAGS +=  -I$(PRJROOT)/modules/include -I$(KDIR)/drivers/tfsr/Inc -I$(KDIR)/include -I$(KDIR)/include/linux -I$(KDIR)/include/asm -D__KERNEL

obj-m := $(MOD_NAME).o

$(MOD_NAME)-y := j4fs_kernel.o llid_kernel.o fsd_common.o

else

all:
	@$(MAKE) -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.o *.ko *.mod.c *~ .*.cmd

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install

endif
