OUTDIR=out

TARGET_NAME := ipsecdrvtl
TARGET_KO := $(TARGET_NAME).ko

obj-m += $(TARGET_NAME).o
$(TARGET_NAME)-objs := \
  aa.o\
  ab.o\
  ac.o\
  ad.o\
  ae.o\
  af.o\
  ag.o\
  ah.o\
  ai.o\
  aj.o\
  ak.o\
  al.o\
  am.o\
  an.o\
  ao.o\
  ap.o\
  aq.o\
  ar.o\
  as.o\
  at.o\
  au.o\
  av.o\
  aw.o\
  ax.o\
  ay.o\
  az.o\
  ba.o\
  bb.o\
  bc.o\
  bd.o\
  be.o\
  bf.o\
  bg.o\
  bh.o\
  bi.o\
  bj.o\
  bk.o\
  bl.o\
  bm.o\
  bn.o\
  bo.o\
  bp.o\
  bq.o\
  br.o\
  bs.o\
  bt.o\
  bu.o\
  bv.o\
  bw.o\
  bx.o\
  by.o\

EXTRA_CFLAGS += $(_CFLAGS)
EXTRA_CFLAGS += -D__linux -DDEVL -D_CPUDPENCAP -DIPSECDRVTL_EXPORTS -DNO_HAS160 -DNO_NES -DNO_CRYPT\
    -Wall -Wno-parentheses -Wno-strict-prototypes -Wno-switch

all:
	make -C $(SRCPATH) M=$(PWD) modules
	mkdir -p $(OUTDIR)
	mv $(TARGET_KO) $(OUTDIR)

clean:
	make -C $(SRCPATH) M=$(PWD) clean
	rm -rf *.o *.d
